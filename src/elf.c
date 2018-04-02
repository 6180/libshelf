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
    uint16_t (*read_word)(const char *src);
    uint32_t (*read_dword)(const char *src);
    uint64_t (*read_qword)(const char *src);
    
    stat(path, &file_stat);

    /* Make sure this is a regular file. */
    if (!S_ISREG(file_stat.st_mode)) 
        return NULL;

    /* Make sure we can read the file. */
    if (access(path, R_OK) != 0) 
        return NULL;

    /* Allocate memory for the elf descriptor struct and set the filepath. */
    desc = calloc(1, sizeof(desc));
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
        printf("Failed opening file\ni");
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
    } else {                   // big-endian
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

    return desc;

error:
    fflush(stdout);

    if (desc->e_mmapped)
        munmap(desc->e_rawdata, desc->e_size);

    if (desc->e_fd)
        close(desc->e_fd);

    free(desc->filepath);
    free(desc);

    return NULL;
}

// ssize_t Elf_Write(desc *desc, const char *path) {
//     return (ssize_t) 0;
// }

void Elf_Close(Elf_Desc *desc) {
    (void) desc;
}

void Elf_Dump_Ident(Elf_Desc *desc) {
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Ident()\n");
        exit(-1);
    }

    printf("E_Ident:\n");
    printf("\tMagic: %02x %02x %02x %02x\n",
        desc-> e_ident[EI_MAG0],
        desc-> e_ident[EI_MAG1],
        desc-> e_ident[EI_MAG2],
        desc-> e_ident[EI_MAG3]        
    );
    printf("\tClass: %s\n", get_elf_class(desc->e_ident[EI_CLASS]));
    printf("\tData: %s\n", get_data_encoding(desc->e_ident[EI_DATA]));
    printf("\tVersion: %s\n", get_elf_version(desc->e_ident[EI_VERSION]));
    printf("\tOS/ABI: %s\n", get_osabi_name(desc->e_ident[EI_OSABI]));
    printf("\tOS/ABI Version: %d\n", desc->e_ident[EI_ABIVERSION]);
}

void Elf_Dump_Header(Elf_Desc *desc) {
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Header()\n");
        exit(-1);
    }

    printf("Elf Header:\n");
    printf("\tMagic:                             %02x %02x %02x %02x\n",
        desc-> e_ident[EI_MAG0],
        desc-> e_ident[EI_MAG1],
        desc-> e_ident[EI_MAG2],
        desc-> e_ident[EI_MAG3]        
    );
    printf("\tClass:                             %s\n", get_elf_class(desc->e_ident[EI_CLASS]));
    printf("\tData:                              %s\n", get_data_encoding(desc->e_ident[EI_DATA]));
    printf("\tVersion:                           %s\n", get_elf_version(desc->e_ident[EI_VERSION]));
    printf("\tOS/ABI:                            %s\n", get_osabi_name(desc->e_ident[EI_OSABI]));
    printf("\tOS/ABI Version:                    %d\n", desc->e_ident[EI_ABIVERSION]);
    printf("\tType:                              %s\n", get_file_type(desc->e_hdr.e_type));
    printf("\tMachine:                           %s\n", get_machine_name(desc->e_hdr.e_machine));
    printf("\tVersion:                           %s\n", get_elf_version(desc->e_hdr.e_version));

    if (desc->e_class != 2) {
        printf("\tEntry:                             0x%1$08x (%1$u)\n", (unsigned int) desc->e_hdr.e_entry);
        printf("\tProgram Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->e_hdr.e_phoff);
        printf("\tSection Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->e_hdr.e_shoff);
    } else {
        printf("\tEntry:                             0x%1$016lx\n", desc->e_hdr.e_entry);
        printf("\tProgram Header Offset:             0x%1$lx (%1$lu)\n", desc->e_hdr.e_phoff);
        printf("\tSection Header Offset:             0x%1$lx (%1$lu)\n", desc->e_hdr.e_shoff);
    }

    printf("\tFlags:                             0x%1$x (%1$d)\n", desc->e_hdr.e_flags);
    printf("\tHeader Size:                       0x%1$04x (%1$d)\n", desc->e_hdr.e_ehsize);
    printf("\tSize of program headers:           0x%1$x (%1$d)\n", desc->e_hdr.e_phentsize);
    printf("\tNumber of program headers:         %d\n", desc->e_hdr.e_phnum);
    printf("\tSize of section headers:           0x%1$x (%1$d)\n", desc->e_hdr.e_shentsize);
    printf("\tNumber of section headers:         %d\n", desc->e_hdr.e_shnum);
    printf("\tSection header string table index: %u\n", desc->e_hdr.e_shstrndx);
}

static const char* get_elf_class(unsigned int elf_class) {
    static char buf[32];

    switch (elf_class) {
        case ELFCLASSNONE: return "none";
        case ELFCLASS32:    return "ELF32";
        case ELFCLASS64:    return "ELF64";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", elf_class);
            return buf;
        }
    }
}

static const char *get_data_encoding(unsigned int encoding) {
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

static const char *get_elf_version(unsigned int version) {
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

static const char *get_osabi_name(unsigned int osabi) {
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

static const char *get_file_type(unsigned int file_type) {
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

static const char *get_machine_name(unsigned int machine) {
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

static uint16_t read_word_le(const char *src) {
    uint16_t ret = 0;
    ret |= src[0];
    ret |= src[1] << 8;
    return ret;
}

static uint16_t read_word_be(const char *src) {
    uint16_t ret = 0;
    ret |= src[1];
    ret |= src[0] << 8;
    return ret;
}

static uint32_t read_dword_le(const char *src) {
    uint32_t ret = 0;
    ret |= src[0];
    ret |= src[1] << 8;
    ret |= src[2] << 16;
    ret |= src[3] << 24;
    return ret;
}

static uint32_t read_dword_be(const char *src) {
    uint32_t ret = 0;
    ret |= src[3];
    ret |= src[2] << 8;
    ret |= src[1] << 16;
    ret |= src[0] << 24;
    return ret;
}

static uint64_t read_qword_le(const char *src) {
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

static uint64_t read_qword_be(const char *src) {
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
