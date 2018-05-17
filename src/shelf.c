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

#include "../include/shelf.h"
#include "../include/shelf_profiler.h"


Elf_Desc *Elf_Open(const char *path) {
    Elf_Desc *desc;
    long path_length;
    struct stat file_stat;

    /* Function pointers we'll set later to handle different endianesses */
    uint16_t (*read_word)(const unsigned char *src);
    uint32_t (*read_dword)(const unsigned char *src);
    uint64_t (*read_qword)(const unsigned char *src);
    
    PROFILER_IN();

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

    PROFILER_ROUT(desc, "Elf_Desc: %p");

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

    PROFILER_ROUT(NULL, "Elf_Desc: %p");
}

// ssize_t Elf_Write(desc *desc, const char *path) {
//     return (ssize_t) 0;
// }

void Elf_Close(Elf_Desc **desc) {
    PROFILER_IN();

    if (!(*desc))
        PROFILER_ERR("NULL pointer passed.");

    if ((*desc)->e_phdr != NULL) {
        free((*desc)->e_phdr);
        (*desc)->e_phdr = NULL;
    }

    if ((*desc)->e_shdr != NULL) {
        free((*desc)->e_shdr);
        (*desc)->e_shdr = NULL;
    }

    if ((*desc)->e_mmapped) {
        munmap((*desc)->e_rawdata, (*desc)->e_size);
        (*desc)->e_mmapped = 0;
    }

    if ((*desc)->e_malloced) {
        free((*desc)->e_rawdata);
        (*desc)->e_malloced = 0;
    }

    if ((*desc)->e_fd) {
        close((*desc)->e_fd);
        (*desc)->e_fd = 0;
    }

    free((*desc)->filepath);
    free((*desc));
    (*desc) = NULL;

    PROFILER_OUT();
}


/*
 * Getterinos for header members
 */
Elf64_Ehdr *esh_get_hdr(Elf_Desc *desc) {
    assert(desc != NULL);
    return &(desc->e_hdr);
}

uint8_t  esh_get_class(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_ident[EI_CLASS], "%u");
}

uint8_t  esh_get_data(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_ident[EI_DATA], "%u");
}

uint8_t  esh_get_osabi(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_ident[EI_OSABI], "%u");
}

uint8_t  esh_get_osabiversion(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_ident[EI_ABIVERSION], "%u");
}

uint16_t esh_get_type(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_type, "%u");
}

uint16_t esh_get_machine(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_machine, "%u");
}

uint32_t esh_get_version(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_version, "%u");
}

uint64_t esh_get_entry(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_entry, "%lu");
}

uint64_t esh_get_phoff(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_phoff, "%lu");
}

uint64_t esh_get_shoff(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_shoff, "%lu");
}

uint32_t esh_get_flags(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_flags, "%u");
}

uint16_t esh_get_ehsize(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_ehsize, "%u");
}

uint16_t esh_get_phentsize(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_phentsize, "%u");
}

uint16_t esh_get_phnum(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_phnum, "%u");
}

uint16_t esh_get_shentsize(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_shentsize, "%u");
}

uint16_t esh_get_shnum(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_shnum, "%u");
}

uint16_t esh_get_shstrndx(Elf_Desc *desc) {
    PROFILER_IN();
    assert(desc != NULL);
    PROFILER_ROUT(desc->e_hdr.e_shstrndx, "%u");
}

/*
 * Setterinos for header members
 */
void esh_set_class(Elf_Desc *desc, uint8_t class) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_ident[EI_CLASS] = class;
    PROFILER_OUT();
}

void esh_set_data(Elf_Desc *desc, uint8_t data) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_ident[EI_DATA] = data;
    PROFILER_OUT();
}

void esh_set_osabi(Elf_Desc *desc, uint8_t osabi) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_ident[EI_OSABI] = osabi;
    PROFILER_OUT();
}

void esh_set_osabiversion(Elf_Desc *desc, uint8_t osabiversion) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_ident[EI_ABIVERSION] = osabiversion;
    PROFILER_OUT();
}

void esh_set_type(Elf_Desc *desc, uint16_t type) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_type = type;
    PROFILER_OUT();
}

void esh_set_machine(Elf_Desc *desc, uint16_t machine) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_machine = machine;
    PROFILER_OUT();
}

void esh_set_version(Elf_Desc *desc, uint32_t version) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_version = version;
    PROFILER_OUT();
}

void esh_set_entry(Elf_Desc *desc, uint64_t entry) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_entry = entry;
    PROFILER_OUT();
}

void esh_set_phoff(Elf_Desc *desc, uint64_t phoff) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_phoff = phoff;
    PROFILER_OUT();
}

void esh_set_shoff(Elf_Desc *desc, uint64_t shoff) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_shoff = shoff;
    PROFILER_OUT();
}

void esh_set_flags(Elf_Desc *desc, uint32_t flags) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_flags = flags;
    PROFILER_OUT();
}

void esh_set_ehsize(Elf_Desc *desc, uint16_t ehsize) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_ehsize = ehsize;
    PROFILER_OUT();
}

void esh_set_phentsize(Elf_Desc *desc, uint16_t phentsize) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_phentsize = phentsize;
    PROFILER_OUT();
}

void esh_set_phnum(Elf_Desc *desc, uint16_t phnum) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_phnum = phnum;
    PROFILER_OUT();
}

void esh_set_shentsize(Elf_Desc *desc, uint16_t shentsize) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_shentsize = shentsize;
    PROFILER_OUT();
}

void esh_set_shnum(Elf_Desc *desc, uint16_t shnum) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_shnum = shnum;
    PROFILER_OUT();
}

void esh_set_shstrndx(Elf_Desc *desc, uint16_t shstrndx) {
    PROFILER_IN();
    assert(desc != NULL);
    desc->e_hdr.e_shstrndx = shstrndx;
    PROFILER_OUT();
}

#define STR_CASE(case_num, str) case case_num: strncpy(buf, str, sizeof(buf)); break;

const char* get_elf_class_str(unsigned int elf_class) {
    static char buf[32];

    PROFILER_IN();

    switch (elf_class) {
        STR_CASE(ELFCLASSNONE, "none");
        STR_CASE(ELFCLASS32,   "ELF32");
        STR_CASE(ELFCLASS64,   "ELF64");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", elf_class);
            break;
        }
    }
    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_data_encoding_str(unsigned int encoding) {
    static char buf[32];

    PROFILER_IN();

    switch (encoding) {
        STR_CASE(ELFDATANONE, "none");
        STR_CASE(ELFDATA2LSB, "2's compliment, little-endian");
        STR_CASE(ELFDATA2MSB, "2's compliment, big-endian");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", encoding);
            break;
        }
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_elf_version_str(unsigned int version) {
    static char buf[32];

    PROFILER_IN();

    switch (version) {
        STR_CASE(EV_NONE,    "none");
        STR_CASE(EV_CURRENT, "current (1");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", version);
            break;
        }
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_osabi_str(unsigned int osabi) {
    static char buf[32];

    PROFILER_IN();

    switch (osabi) {
        STR_CASE(ELFOSABI_SYSV,    "UNIX System V ABI");
        STR_CASE(ELFOSABI_HPUX,    "Hewlett-Packard HP-UX");
        STR_CASE(ELFOSABI_NETBSD,  "NetBSD");
        STR_CASE(ELFOSABI_LINUX,   "Linux");
        STR_CASE(ELFOSABI_SOLARIS, "Sun Solaris");
        STR_CASE(ELFOSABI_AIX,     "AIX");
        STR_CASE(ELFOSABI_IRIX,    "IRIX");
        STR_CASE(ELFOSABI_FREEBSD, "FreeBSD");
        STR_CASE(ELFOSABI_TRU64,   "Compaq TRU64 UNIX");
        STR_CASE(ELFOSABI_MODESTO, "Novell Modesto");
        STR_CASE(ELFOSABI_OPENBSD, "Open BSD");
        STR_CASE(ELFOSABI_OPENVMS, "Open VMS");
        STR_CASE(ELFOSABI_NSK,     "Hewlett-Packard Non-Stop Kernel");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", osabi);
            break;
        }
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_file_type_str(unsigned int file_type) {
    static char buf[64];

    PROFILER_IN();

    switch (file_type) {
        STR_CASE(ET_NONE, "NONE (None");
        STR_CASE(ET_REL,  "REL (Relocatable file");
        STR_CASE(ET_EXEC, "EXEC (Executable file");
        STR_CASE(ET_DYN,  "DYN (Shared object file");
        STR_CASE(ET_CORE, "CORE (Core file");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", file_type);
            break;
        }
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_machine_str(unsigned int machine) {
    static char buf[64];

    PROFILER_IN();

    switch (machine) {
        STR_CASE(0,   "No machine");
        STR_CASE(1,   "AT&T WE 32100");
        STR_CASE(2,   "SPARC");
        STR_CASE(3,   "Intel 80386");
        STR_CASE(4,   "Motorola 68000");
        STR_CASE(5,   "Motorola 88000");
        STR_CASE(7,   "Intel 80860");
        STR_CASE(8,   "MIPS I Architecture");
        STR_CASE(9,   "IBM System/370 Processor");
        STR_CASE(10,  "MIPS RS3000 Little-endian");
        STR_CASE(15,  "Hewlett-Packard PA-RISC");
        STR_CASE(17,  "Fujitsu VPP500");
        STR_CASE(18,  "Enhanced instruction set SPARC");
        STR_CASE(19,  "Intel 80960");
        STR_CASE(20,  "PowerPC");
        STR_CASE(21,  "64-bit PowerPC");
        STR_CASE(22,  "IBM System/390 Processor");
        STR_CASE(36,  "NEC V800");
        STR_CASE(37,  "Fujitsu FR20");
        STR_CASE(38,  "TRW RH-32");
        STR_CASE(39,  "Motorola RCE");
        STR_CASE(40,  "Advanced RISC Machines ARM");
        STR_CASE(41,  "Digital Alpha");
        STR_CASE(42,  "Hitachi SH");
        STR_CASE(43,  "SPARC Version 9");
        STR_CASE(44,  "Siemens TriCore embedded processor");
        STR_CASE(45,  "Argonaut RISC Core, Argonaut Technologies Inc.");
        STR_CASE(46,  "Hitachi H8/300");
        STR_CASE(47,  "Hitachi H8/300H");
        STR_CASE(48,  "Hitachi H8S");
        STR_CASE(49,  "Hitachi H8/500");
        STR_CASE(50,  "Intel IA-64 processor architecture");
        STR_CASE(51,  "Stanford MIPS-X");
        STR_CASE(52,  "Motorola ColdFire");
        STR_CASE(53,  "Motorola M68HC12");
        STR_CASE(54,  "Fujitsu MMA Multimedia Accelerator");
        STR_CASE(55,  "Siemens PCP");
        STR_CASE(56,  "Sony nCPU embedded RISC processor");
        STR_CASE(57,  "Denso NDR1 microprocessor");
        STR_CASE(58,  "Motorola Star*Core processor");
        STR_CASE(59,  "Toyota ME16 processor");
        STR_CASE(60,  "STMicroelectronics ST100 processor");
        STR_CASE(61,  "Advanced Logic Corp. TinyJ embedded processor family");
        STR_CASE(62,  "AMD x86-64 architecture");
        STR_CASE(63,  "Sony DSP Processor");
        STR_CASE(64,  "Digital Equipment Corp. PDP-10");
        STR_CASE(65,  "Digital Equipment Corp. PDP-11");
        STR_CASE(66,  "Siemens FX66 microcontroller");
        STR_CASE(67,  "STMicroelectronics ST9+ 8/16 bit microcontroller");
        STR_CASE(68,  "STMicroelectronics ST7 8-bit microcontroller");
        STR_CASE(69,  "Motorola MC68HC16 Microcontroller");
        STR_CASE(70,  "Motorola MC68HC11 Microcontroller");
        STR_CASE(71,  "Motorola MC68HC08 Microcontroller");
        STR_CASE(72,  "Motorola MC68HC05 Microcontroller");
        STR_CASE(73,  "Silicon Graphics SVx");
        STR_CASE(74,  "STMicroelectronics ST19 8-bit microcontroller");
        STR_CASE(75,  "Digital VAX");
        STR_CASE(76,  "Axis Communications 32-bit embedded processor");
        STR_CASE(77,  "Infineon Technologies 32-bit embedded processor");
        STR_CASE(78,  "Element 14 64-bit DSP Processor");
        STR_CASE(79,  "LSI Logic 16-bit DSP Processor");
        STR_CASE(80,  "Donald Knuth's educational 64-bit processor");
        STR_CASE(81,  "Harvard University machine-independent object files");
        STR_CASE(82,  "SiTera Prism");
        STR_CASE(83,  "Atmel AVR 8-bit microcontroller");
        STR_CASE(84,  "Fujitsu FR30");
        STR_CASE(85,  "Mitsubishi D10V");
        STR_CASE(86,  "Mitsubishi D30V");
        STR_CASE(87,  "NEC v850");
        STR_CASE(88,  "Mitsubishi M32R");
        STR_CASE(89,  "Matsushita MN10300");
        STR_CASE(90,  "Matsushita MN10200");
        STR_CASE(91,  "picoJava");
        STR_CASE(92,  "OpenRISC 32-bit embedded processor");
        STR_CASE(93,  "ARC Cores Tangent-A5");
        STR_CASE(94,  "Tensilica Xtensa Architecture");
        STR_CASE(95,  "Alphamosaic VideoCore processor");
        STR_CASE(96,  "Thompson Multimedia General Purpose Processor");
        STR_CASE(97,  "National Semiconductor 32000 series");
        STR_CASE(98,  "Tenor Network TPC processor");
        STR_CASE(99,  "Trebia SNP 1000 processor");
        STR_CASE(100, "STMicroelectronics (www.st.com) ST200 microcontroller");
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", machine);
            break;
        }
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_phdr_type_str(unsigned int type) {
    static char buf[32];

    PROFILER_IN();

    switch (type) {
        STR_CASE(PT_NULL,         "NULL");
        STR_CASE(PT_LOAD,         "LOAD");
        STR_CASE(PT_DYNAMIC,      "DYNAMIC");
        STR_CASE(PT_INTERP,       "INTERP");
        STR_CASE(PT_NOTE,         "NOTE");
        STR_CASE(PT_SHLIB,        "SHLIB");
        STR_CASE(PT_PHDR,         "PHDR");
        STR_CASE(PT_TLS,          "TLS");
        STR_CASE(PT_LOOS,         "LOOS");
        STR_CASE(PT_HIOS,         "HIOS");
        STR_CASE(PT_LOPROC,       "LOPROC");
        STR_CASE(PT_HIPROC,       "HIPROC");
        STR_CASE(PT_GNU_EH_FRAME, "EH_FRAME");
        STR_CASE(PT_GNU_STACK,    "STACK");
        STR_CASE(PT_GNU_RELRO,    "RELRO");
        default: strncpy(buf, "INVALID", sizeof(buf)); break;
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_phdr_flags_str(unsigned int flags) {
    static char buf[4] = {0};

    PROFILER_IN();

    buf[0] = (flags & PF_R) ? 'R' : '-';
    buf[1] = (flags & PF_W) ? 'W' : '-';
    buf[2] = (flags & PF_X) ? 'X' : '-';
    buf[3] = '\0'; // Just in case fam.

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_shdr_type_str(uint32_t type) {
    static char buf[32];

    PROFILER_IN();

    switch (type) {
        STR_CASE(SHT_NULL,           "NULL");
        STR_CASE(SHT_PROGBITS,       "PROGBITS");
        STR_CASE(SHT_SYMTAB,         "SYMTAB");
        STR_CASE(SHT_STRTAB,         "STRTAB");
        STR_CASE(SHT_RELA,           "RELA");
        STR_CASE(SHT_HASH,           "HASH");
        STR_CASE(SHT_DYNAMIC,        "DYNAMIC");
        STR_CASE(SHT_NOTE,           "NOTE");
        STR_CASE(SHT_NOBITS,         "NOBITS");
        STR_CASE(SHT_REL,            "REL");
        STR_CASE(SHT_SHLIB,          "SHLIB");
        STR_CASE(SHT_DYNSYM,         "DYNSYM");
        STR_CASE(SHT_INIT_ARRAY,     "INIT_ARRAY");
        STR_CASE(SHT_FINI_ARRAY,     "FINI_ARRAY");
        STR_CASE(SHT_PREINIT_ARRAY,  "PREINIT_ARRAY");
        STR_CASE(SHT_GROUP,          "GROUP");
        STR_CASE(SHT_SYMTAB_SHNDX,   "SYMTAB_SHNDX");
        STR_CASE(SHT_LOOS,           "LOOS");
        STR_CASE(SHT_GNU_ATTRIBUTES, "GNU_ATTRIBUTES");
        STR_CASE(SHT_GNU_HASH,       "GNU_HASH");
        STR_CASE(SHT_GNU_LIBLIST,    "GNU_LIBLIST");
        STR_CASE(SHT_CHECKSUM,       "CHECKSUM");
        STR_CASE(SHT_LOSUNW,         "LOSUNW");
        STR_CASE(SHT_SUNW_COMDAT,    "SUNW_COMDAT");
        STR_CASE(SHT_SUNW_syminfo,   "SUNW_syminfo");
        STR_CASE(SHT_GNU_verdef,     "GNU_verdef");
        STR_CASE(SHT_GNU_verneed,    "GNU_verneed");
        STR_CASE(SHT_GNU_versym,     "GNU_versym");
        STR_CASE(SHT_LOPROC,         "LOPROC");
        STR_CASE(SHT_HIPROC,         "HIPROC");
        STR_CASE(SHT_LOUSER,         "LOUSER");
        STR_CASE(SHT_HIUSER,         "HIUSER");
        default: strncpy(buf, "INVALID", sizeof(buf)); break;
    }

    PROFILER_ROUT(buf, "\"%s\"");
}

const char *get_shdr_flags_str(unsigned int sh_flags) {
    static char buf[32];
    char *p = buf;
    unsigned int flag;

    PROFILER_IN();

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

    PROFILER_ROUT(buf, "\"%s\"");
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
