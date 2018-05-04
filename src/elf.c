#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/elf.h"


Elf_Desc *Elf_Open(const char *path) {
    Elf_Desc *desc;
    long path_length;
    struct stat file_stat;

    /* Function pointers we'll set later to handle different endianesses */
    uint16_t (*read_word)(const unsigned char *src);
    uint32_t (*read_dword)(const unsigned char *src);
    uint64_t (*read_qword)(const unsigned char *src);
    
    stat(path, &file_stat);

    /* Make sure this is a regular file. */
    if (!S_ISREG(file_stat.st_mode)) 
        return NULL;

    /* Make sure we can read the file. */
    if (access(path, R_OK) != 0) 
        return NULL;

    /* Allocate memory for the elf descriptor struct and set the filepath. */
    desc = calloc(1, sizeof(*desc));
    path_length = strlen(path);
    desc->filepath = malloc(path_length + 1);
    strncpy(desc->filepath, path, path_length);

    /* Check if we can write to the file. */
    if (access(desc->filepath, W_OK) == 0)
        desc->e_writable = 1;

    /* Initialize fields we know at this point. */
    desc->e_size = file_stat.st_size;
    desc->e_readable = 1;
    desc->e_magic = ELF_MAGIC;

    /* Before trying to open make sure the file is at least as large as an elf header. */
    if (desc->e_size < 52) {
        printf("File is smaller than the smallest valid elf header.\n");
    }

    if (desc->e_writable) 
        desc->e_fd = open(desc->filepath, O_RDWR);
    else 
        desc->e_fd = open(desc->filepath, O_RDONLY);

    /* We can't open with the permissions detected earlier for some reason. */
    if (desc->e_fd == -1) {
        printf("Failed opening file\n");
        desc->e_fd = 0;
        goto error;
    }
    
    desc->e_rawdata = mmap(
        NULL,
        desc->e_size,
        PROT_READ,
        MAP_PRIVATE,
        desc->e_fd,
        0
    );

    // printf("1\n");

    if (desc->e_rawdata == MAP_FAILED) {
        printf("mmap() failed.\n");
        goto error;
    }


    desc->e_mmapped = 1;
    desc->e_ident = desc->e_rawdata;

    desc->e_class = desc->e_ident[EI_CLASS];
    desc->e_encoding = desc->e_ident[EI_DATA];

    if (desc->e_encoding != 2) { // little-endian
        read_word = read_word_le;
        read_dword = read_dword_le;
        read_qword = read_qword_le;
    } else {                     // big-endian
        read_word = read_word_be;
        read_dword = read_dword_be;
        read_qword = read_qword_be;
    }


    size_t offset = EI_NIDENT;

    if (desc->e_class == 1) { // 32 bit
        memcpy(desc->e_hdr.e_ident, desc->e_rawdata, EI_NIDENT);
        desc->e_hdr.e_type = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_machine = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_version = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_entry = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_phoff = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_shoff = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_flags = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_ehsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_phentsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_phnum = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shentsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shnum = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shstrndx = read_word(desc->e_rawdata + offset); offset += 2;
    } else if (desc->e_class == 2) {  // 64 bit
        memcpy(desc->e_hdr.e_ident, desc->e_rawdata, EI_NIDENT);
        desc->e_hdr.e_type = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_machine = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_version = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_entry = read_qword(desc->e_rawdata + offset); offset += 8;
        desc->e_hdr.e_phoff = read_qword(desc->e_rawdata + offset); offset += 8;
        desc->e_hdr.e_shoff = read_qword(desc->e_rawdata + offset); offset += 8;
        desc->e_hdr.e_flags = read_dword(desc->e_rawdata + offset); offset += 4;
        desc->e_hdr.e_ehsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_phentsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_phnum = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shentsize = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shnum = read_word(desc->e_rawdata + offset); offset += 2;
        desc->e_hdr.e_shstrndx = read_word(desc->e_rawdata + offset); offset += 2;
    }

    desc->e_phdr = malloc(desc->e_hdr.e_phnum * sizeof(Elf64_Phdr));

    if (desc->e_phdr == NULL) {
        printf("malloc() for phdr table failed.\n");
        goto error;
    }

    unsigned char *ph_base = desc->e_rawdata + desc->e_hdr.e_phoff;

    for (size_t i = 0; i < desc->e_hdr.e_phnum; i++) {
        if (desc->e_class != 2) { // 32-bit
            desc->e_phdr[i].p_type =   read_dword(ph_base + i * desc->e_hdr.e_phentsize);
            desc->e_phdr[i].p_offset = read_dword(ph_base + i * desc->e_hdr.e_phentsize + 4);
            desc->e_phdr[i].p_vaddr =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 8);
            desc->e_phdr[i].p_paddr =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 12);
            desc->e_phdr[i].p_filesz = read_dword(ph_base + i * desc->e_hdr.e_phentsize + 16);
            desc->e_phdr[i].p_memsz =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 20);
            desc->e_phdr[i].p_flags =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 24);
            desc->e_phdr[i].p_align =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 28);
        } else { // 64-bit
            desc->e_phdr[i].p_type =   read_dword(ph_base + i * desc->e_hdr.e_phentsize);
            desc->e_phdr[i].p_flags =  read_dword(ph_base + i * desc->e_hdr.e_phentsize + 4);
            desc->e_phdr[i].p_offset = read_dword(ph_base + i * desc->e_hdr.e_phentsize + 8);
            desc->e_phdr[i].p_vaddr =  read_qword(ph_base + i * desc->e_hdr.e_phentsize + 16);
            desc->e_phdr[i].p_paddr =  read_qword(ph_base + i * desc->e_hdr.e_phentsize + 24);
            desc->e_phdr[i].p_filesz = read_qword(ph_base + i * desc->e_hdr.e_phentsize + 32);
            desc->e_phdr[i].p_memsz =  read_qword(ph_base + i * desc->e_hdr.e_phentsize + 40);
            desc->e_phdr[i].p_align =  read_qword(ph_base + i * desc->e_hdr.e_phentsize + 48);
        }
    }

    desc->e_shdr = malloc(desc->e_hdr.e_shnum * sizeof(Elf64_Shdr));

    if (desc->e_shdr == NULL) {
        printf("malloc() for shdr table failed.\n");
        goto error;
    }

    unsigned char *sh_base = desc->e_rawdata + desc->e_hdr.e_shoff;

    for (size_t i = 0; i < desc->e_hdr.e_shnum; i++) {
        if (desc->e_class != 2) { // 32-bit
            // TODO: Do this thingy boi
        } else { // 64-bit
            desc->e_shdr[i].sh_name =      read_dword(sh_base + i * desc->e_hdr.e_shentsize);
            desc->e_shdr[i].sh_type =      read_dword(sh_base + i * desc->e_hdr.e_shentsize + 4);
            desc->e_shdr[i].sh_flags =     read_qword(sh_base + i * desc->e_hdr.e_shentsize + 8);
            desc->e_shdr[i].sh_addr =      read_qword(sh_base + i * desc->e_hdr.e_shentsize + 16);
            desc->e_shdr[i].sh_offset =    read_qword(sh_base + i * desc->e_hdr.e_shentsize + 24);
            desc->e_shdr[i].sh_size =      read_qword(sh_base + i * desc->e_hdr.e_shentsize + 32);
            desc->e_shdr[i].sh_link =      read_dword(sh_base + i * desc->e_hdr.e_shentsize + 40);
            desc->e_shdr[i].sh_info =      read_dword(sh_base + i * desc->e_hdr.e_shentsize + 44);
            desc->e_shdr[i].sh_addralign = read_qword(sh_base + i * desc->e_hdr.e_shentsize + 48);
            desc->e_shdr[i].sh_entsize =   read_qword(sh_base + i * desc->e_hdr.e_shentsize + 56);
        }
    }    

    return desc;

error:
    fflush(stdout);

    if (desc->e_phdr != NULL) {
        free(desc->e_phdr);
        desc->e_phdr = NULL;
    }

    if (desc->e_mmapped) {
        munmap(desc->e_rawdata, desc->e_size);
        desc->e_mmapped = 0;
    }

    if (desc->e_fd) {
        close(desc->e_fd);
        desc->e_fd = 0;
    }

    free(desc->filepath);
    free(desc);

    return NULL;
}

// ssize_t Elf_Write(desc *desc, const char *path) {
//     return (ssize_t) 0;
// }

void Elf_Close(Elf_Desc *desc) {
    if (!desc)
        return;

    if (desc->e_phdr != NULL)
        free(desc->e_phdr);

    if (desc->e_shdr != NULL)
        free(desc->e_shdr);

    if (desc->e_mmapped)
        munmap(desc->e_rawdata, desc->e_size);

    if (desc->e_malloced)
        free(desc->e_rawdata);

    if (desc->e_fd)
        close(desc->e_fd);

    free(desc->filepath);
    free(desc);

    desc = NULL;
}


/*
 * Getterinos for header members
 */
Elf64_Ehdr *esh_get_hdr(Elf_Desc *desc) {
    assert(desc != NULL);
    return &(desc->e_hdr);
}

uint8_t  esh_get_class(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_ident[EI_CLASS];
}

uint8_t  esh_get_data(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_ident[EI_DATA];
}

uint8_t  esh_get_osabi(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_ident[EI_OSABI];
}

uint8_t  esh_get_osabiversion(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_ident[EI_ABIVERSION];
}

uint16_t esh_get_type(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_type;
}

uint16_t esh_get_machine(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_machine;
}

uint32_t esh_get_version(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_version;
}

uint64_t esh_get_entry(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_entry;
}

uint64_t esh_get_phoff(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_phoff;
}

uint64_t esh_get_shoff(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_shoff;
}

uint32_t esh_get_flags(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_flags;
}

uint16_t esh_get_ehsize(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_ehsize;
}

uint16_t esh_get_phentsize(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_phentsize;
}

uint16_t esh_get_phnum(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_phnum;
}

uint16_t esh_get_shentsize(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_shentsize;
}

uint16_t esh_get_shnum(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_shnum;
}

uint16_t esh_get_shstrndx(Elf_Desc *desc) {
    assert(desc != NULL);
    return desc->e_hdr.e_shstrndx;
}

/*
 * Setterinos for header members
 */
void esh_set_class(Elf_Desc *desc, uint8_t class) {
    assert(desc != NULL);
    desc->e_ident[EI_CLASS] = class;
}

void esh_set_data(Elf_Desc *desc, uint8_t data) {
    assert(desc != NULL);
    desc->e_ident[EI_DATA] = data;
}

void esh_set_osabi(Elf_Desc *desc, uint8_t osabi) {
    assert(desc != NULL);
    desc->e_ident[EI_OSABI] = osabi;
}

void esh_set_osabiversion(Elf_Desc *desc, uint8_t osabiversion) {
    assert(desc != NULL);
    desc->e_ident[EI_ABIVERSION] = osabiversion;
}

void esh_set_type(Elf_Desc *desc, uint16_t type) {
    assert(desc != NULL);
    desc->e_hdr.e_type = type;
}

void esh_set_machine(Elf_Desc *desc, uint16_t machine) {
    assert(desc != NULL);
    desc->e_hdr.e_machine = machine;
}

void esh_set_version(Elf_Desc *desc, uint32_t version) {
    assert(desc != NULL);
    desc->e_hdr.e_version = version;
}

void esh_set_entry(Elf_Desc *desc, uint64_t entry) {
    assert(desc != NULL);
    desc->e_hdr.e_entry = entry;
}

void esh_set_phoff(Elf_Desc *desc, uint64_t phoff) {
    assert(desc != NULL);
    desc->e_hdr.e_phoff = phoff;
}

void esh_set_shoff(Elf_Desc *desc, uint64_t shoff) {
    assert(desc != NULL);
    desc->e_hdr.e_shoff = shoff;
}

void esh_set_flags(Elf_Desc *desc, uint32_t flags) {
    assert(desc != NULL);
    desc->e_hdr.e_flags = flags;
}

void esh_set_ehsize(Elf_Desc *desc, uint16_t ehsize) {
    assert(desc != NULL);
    desc->e_hdr.e_ehsize = ehsize;
}

void esh_set_phentsize(Elf_Desc *desc, uint16_t phentsize) {
    assert(desc != NULL);
    desc->e_hdr.e_phentsize = phentsize;
}

void esh_set_phnum(Elf_Desc *desc, uint16_t phnum) {
    assert(desc != NULL);
    desc->e_hdr.e_phnum = phnum;
}

void esh_set_shentsize(Elf_Desc *desc, uint16_t shentsize) {
    assert(desc != NULL);
    desc->e_hdr.e_shentsize = shentsize;
}

void esh_set_shnum(Elf_Desc *desc, uint16_t shnum) {
    assert(desc != NULL);
    desc->e_hdr.e_shnum = shnum;
}

void esh_set_shstrndx(Elf_Desc *desc, uint16_t shstrndx) {
    assert(desc != NULL);
    desc->e_hdr.e_shstrndx = shstrndx;
}


const char* get_elf_class_str(unsigned int elf_class) {
    static char buf[32];

    switch (elf_class) {
        case ELFCLASSNONE: return "none";
        case ELFCLASS32:   return "ELF32";
        case ELFCLASS64:   return "ELF64";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", elf_class);
            return buf;
        }
    }
}

const char *get_data_encoding_str(unsigned int encoding) {
    static char buf[32];

    switch (encoding) {
        case ELFDATANONE: return "none";
        case ELFDATA2LSB: return "2's compliment, little-endian";
        case ELFDATA2MSB: return "2's compliment, big-endian";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", encoding);
            return buf;
        }
    }
}

const char *get_elf_version_str(unsigned int version) {
    static char buf[32];

    switch (version) {
        case EV_NONE:    return "none";
        case EV_CURRENT: return "current (1)";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", version);
            return buf;
        }
    }
}

const char *get_osabi_str(unsigned int osabi) {
    static char buf[32];

    switch (osabi) {
        case ELFOSABI_SYSV:    return "UNIX System V ABI";
        case ELFOSABI_HPUX:    return "Hewlett-Packard HP-UX";
        case ELFOSABI_NETBSD:  return "NetBSD";
        case ELFOSABI_LINUX:   return "Linux";
        case ELFOSABI_SOLARIS: return "Sun Solaris";
        case ELFOSABI_AIX:     return "AIX";
        case ELFOSABI_IRIX:    return "IRIX";
        case ELFOSABI_FREEBSD: return "FreeBSD";
        case ELFOSABI_TRU64:   return "Compaq TRU64 UNIX";
        case ELFOSABI_MODESTO: return "Novell Modesto";
        case ELFOSABI_OPENBSD: return "Open BSD";
        case ELFOSABI_OPENVMS: return "Open VMS";
        case ELFOSABI_NSK:     return "Hewlett-Packard Non-Stop Kernel";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", osabi);
            return buf;
        }
    }
}

const char *get_file_type_str(unsigned int file_type) {
    static char buf[32];

    switch (file_type) {
        case ET_NONE: return "NONE (None)";
        case ET_REL:  return "REL (Relocatable file)";
        case ET_EXEC: return "EXEC (Executable file)";
        case ET_DYN:  return "DYN (Shared object file)";
        case ET_CORE: return "CORE (Core file)";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", file_type);
            return buf;
        }
    }
}

const char *get_machine_str(unsigned int machine) {
    static char buf[32];

    switch (machine) {
        case 0:   return"No machine";
        case 1:   return"AT&T WE 32100";
        case 2:   return"SPARC";
        case 3:   return"Intel 80386";
        case 4:   return"Motorola 68000";
        case 5:   return"Motorola 88000";
        case 7:   return"Intel 80860";
        case 8:   return"MIPS I Architecture";
        case 9:   return"IBM System/370 Processor";
        case 10:  return "MIPS RS3000 Little-endian";
        case 15:  return "Hewlett-Packard PA-RISC";
        case 17:  return "Fujitsu VPP500";
        case 18:  return "Enhanced instruction set SPARC";
        case 19:  return "Intel 80960";
        case 20:  return "PowerPC";
        case 21:  return "64-bit PowerPC";
        case 22:  return "IBM System/390 Processor";
        case 36:  return "NEC V800";
        case 37:  return "Fujitsu FR20";
        case 38:  return "TRW RH-32";
        case 39:  return "Motorola RCE";
        case 40:  return "Advanced RISC Machines ARM";
        case 41:  return "Digital Alpha";
        case 42:  return "Hitachi SH";
        case 43:  return "SPARC Version 9";
        case 44:  return "Siemens TriCore embedded processor";
        case 45:  return "Argonaut RISC Core, Argonaut Technologies Inc.";
        case 46:  return "Hitachi H8/300";
        case 47:  return "Hitachi H8/300H";
        case 48:  return "Hitachi H8S";
        case 49:  return "Hitachi H8/500";
        case 50:  return "Intel IA-64 processor architecture";
        case 51:  return "Stanford MIPS-X";
        case 52:  return "Motorola ColdFire";
        case 53:  return "Motorola M68HC12";
        case 54:  return "Fujitsu MMA Multimedia Accelerator";
        case 55:  return "Siemens PCP";
        case 56:  return "Sony nCPU embedded RISC processor";
        case 57:  return "Denso NDR1 microprocessor";
        case 58:  return "Motorola Star*Core processor";
        case 59:  return "Toyota ME16 processor";
        case 60:  return "STMicroelectronics ST100 processor";
        case 61:  return "Advanced Logic Corp. TinyJ embedded processor family";
        case 62:  return "AMD x86-64 architecture";
        case 63:  return "Sony DSP Processor";
        case 64:  return "Digital Equipment Corp. PDP-10";
        case 65:  return "Digital Equipment Corp. PDP-11";
        case 66:  return "Siemens FX66 microcontroller";
        case 67:  return "STMicroelectronics ST9+ 8/16 bit microcontroller";
        case 68:  return "STMicroelectronics ST7 8-bit microcontroller";
        case 69:  return "Motorola MC68HC16 Microcontroller";
        case 70:  return "Motorola MC68HC11 Microcontroller";
        case 71:  return "Motorola MC68HC08 Microcontroller";
        case 72:  return "Motorola MC68HC05 Microcontroller";
        case 73:  return "Silicon Graphics SVx";
        case 74:  return "STMicroelectronics ST19 8-bit microcontroller";
        case 75:  return "Digital VAX";
        case 76:  return "Axis Communications 32-bit embedded processor";
        case 77:  return "Infineon Technologies 32-bit embedded processor";
        case 78:  return "Element 14 64-bit DSP Processor";
        case 79:  return "LSI Logic 16-bit DSP Processor";
        case 80:  return "Donald Knuth's educational 64-bit processor";
        case 81:  return "Harvard University machine-independent object files";
        case 82:  return "SiTera Prism";
        case 83:  return "Atmel AVR 8-bit microcontroller";
        case 84:  return "Fujitsu FR30";
        case 85:  return "Mitsubishi D10V";
        case 86:  return "Mitsubishi D30V";
        case 87:  return "NEC v850";
        case 88:  return "Mitsubishi M32R";
        case 89:  return "Matsushita MN10300";
        case 90:  return "Matsushita MN10200";
        case 91:  return "picoJava";
        case 92:  return "OpenRISC 32-bit embedded processor";
        case 93:  return "ARC Cores Tangent-A5";
        case 94:  return "Tensilica Xtensa Architecture";
        case 95:  return "Alphamosaic VideoCore processor";
        case 96:  return "Thompson Multimedia General Purpose Processor";
        case 97:  return "National Semiconductor 32000 series";
        case 98:  return "Tenor Network TPC processor";
        case 99:  return "Trebia SNP 1000 processor";
        case 100: return "STMicroelectronics (www.st.com) ST200 microcontroller";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", machine);
            return buf;
        }
    }
}

const char *get_phdr_type_str(unsigned int type) {
    switch (type) {
        case PT_NULL:         return "NULL";
        case PT_LOAD:         return "LOAD";
        case PT_DYNAMIC:      return "DYNAMIC";
        case PT_INTERP:       return "INTERP";
        case PT_NOTE:         return "NOTE";
        case PT_SHLIB:        return "SHLIB";
        case PT_PHDR:         return "PHDR";
        case PT_TLS:          return "TLS";
        case PT_LOOS:         return "LOOS";
        case PT_HIOS:         return "HIOS";
        case PT_LOPROC:       return "LOPROC";
        case PT_HIPROC:       return "HIPROC";
        case PT_GNU_EH_FRAME: return "EH_FRAME";
        case PT_GNU_STACK:    return "STACK";
        case PT_GNU_RELRO:    return "RELRO";
        default:              return "INVALID";
    }
}

const char *get_phdr_flags_str(unsigned int flags) {
    static char buf[4] = {0};

    buf[0] = (flags & PF_R) ? 'R' : '-';
    buf[1] = (flags & PF_W) ? 'W' : '-';
    buf[2] = (flags & PF_X) ? 'X' : '-';
    buf[3] = '\0'; // Just in case fam.

    return buf;
}

const char *get_shdr_type_str(uint32_t type) {
    switch (type) {
        case SHT_NULL:           return "NULL";
        case SHT_PROGBITS:       return "PROGBITS";
        case SHT_SYMTAB:         return "SYMTAB";
        case SHT_STRTAB:         return "STRTAB";
        case SHT_RELA:           return "RELA";
        case SHT_HASH:           return "HASH";
        case SHT_DYNAMIC:        return "DYNAMIC";
        case SHT_NOTE:           return "NOTE";
        case SHT_NOBITS:         return "NOBITS";
        case SHT_REL:            return "REL";
        case SHT_SHLIB:          return "SHLIB";
        case SHT_DYNSYM:         return "DYNSYM";
        case SHT_INIT_ARRAY:     return "INIT_ARRAY";
        case SHT_FINI_ARRAY:     return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:  return "PREINIT_ARRAY";
        case SHT_GROUP:          return "GROUP";
        case SHT_SYMTAB_SHNDX:   return "SYMTAB_SHNDX";
        case SHT_LOOS:           return "LOOS";
        case SHT_GNU_ATTRIBUTES: return "GNU_ATTRIBUTES";
        case SHT_GNU_HASH:       return "GNU_HASH";
        case SHT_GNU_LIBLIST:    return "GNU_LIBLIST";
        case SHT_CHECKSUM:       return "CHECKSUM";
        case SHT_LOSUNW:         return "LOSUNW";
        case SHT_SUNW_COMDAT:    return "SUNW_COMDAT";
        case SHT_SUNW_syminfo:   return "SUNW_syminfo";
        case SHT_GNU_verdef:     return "GNU_verdef";
        case SHT_GNU_verneed:    return "GNU_verneed";
        case SHT_GNU_versym:     return "GNU_versym";
        case SHT_LOPROC:         return "LOPROC";
        case SHT_HIPROC:         return "HIPROC";
        case SHT_LOUSER:         return "LOUSER";
        case SHT_HIUSER:         return "HIUSER";
        default:                 return "INVALID";
    }
}

const char *get_shdr_flags_str(unsigned int sh_flags) {
    static char buf[32];
    char *p = buf;
    unsigned int flag;

    memset(buf, 0, sizeof(buf));

    for (size_t i = 0; i <= 31; i++) {
        flag = sh_flags & (1 << i);

        switch (flag) {
            case SHF_WRITE:            *p++ = 'W'; break;
            case SHF_ALLOC:            *p++ = 'A'; break;
            case SHF_EXECINSTR:        *p++ = 'X'; break;
            case SHF_MERGE:            *p++ = 'M'; break;
            case SHF_STRINGS:          *p++ = 'S'; break;
            case SHF_INFO_LINK:        *p++ = 'I'; break;
            case SHF_LINK_ORDER:       *p++ = 'L'; break;
            case SHF_OS_NONCONFORMING: *p++ = 'O'; break;
            case SHF_GROUP:            *p++ = 'G'; break;
            case SHF_TLS:              *p++ = 'T'; break;
            case SHF_EXCLUDE:          *p++ = 'E'; break;
        }
    }

    return buf;
}

uint16_t read_word_le(const unsigned char *src) {
    uint16_t ret = 0;
    ret |= src[0];
    ret |= src[1] << 8;
    return ret;
}

uint16_t read_word_be(const unsigned char *src) {
    uint16_t ret = 0;
    ret |= src[1];
    ret |= src[0] << 8;
    return ret;
}

uint32_t read_dword_le(const unsigned char *src) {
    uint32_t ret = 0;
    ret |= src[0];
    ret |= src[1] << 8;
    ret |= src[2] << 16;
    ret |= src[3] << 24;
    return ret;
}

uint32_t read_dword_be(const unsigned char *src) {
    uint32_t ret = 0;
    ret |= src[3];
    ret |= src[2] << 8;
    ret |= src[1] << 16;
    ret |= src[0] << 24;
    return ret;
}

uint64_t read_qword_le(const unsigned char *src) {
    uint64_t ret = 0;
    ret |= (uint64_t)src[0];
    ret |= (uint64_t)src[1] << 8;
    ret |= (uint64_t)src[2] << 16;
    ret |= (uint64_t)src[3] << 24;
    ret |= (uint64_t)src[4] << 32;
    ret |= (uint64_t)src[5] << 40;
    ret |= (uint64_t)src[6] << 48;
    ret |= (uint64_t)src[7] << 56;
    return ret;
}

uint64_t read_qword_be(const unsigned char *src) {
    uint64_t ret = 0;
    ret |= (uint64_t)src[3];
    ret |= (uint64_t)src[2] << 8;
    ret |= (uint64_t)src[1] << 16;
    ret |= (uint64_t)src[0] << 24;
    ret |= (uint64_t)src[0] << 32;
    ret |= (uint64_t)src[0] << 40;
    ret |= (uint64_t)src[0] << 48;
    ret |= (uint64_t)src[0] << 56;
    return ret;
}
