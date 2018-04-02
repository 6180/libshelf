#ifndef ELF_B8FA07
#define ELF_B8FA07


#include <stdint.h>
#include <sys/types.h>

#include "elf_constants.h"

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef uint16_t Elf32_Versym;
typedef uint16_t Elf32_Half;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf32_Xword;

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Section;
typedef uint16_t Elf64_Versym;
typedef uint16_t Elf64_Half;
typedef int32_t  Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Xword;


/*
 * An ELF header resides at the beginning of an ELF object file and holds
 * information describing the file's organization.
 * 
 * e_ident: This member holds machine independant data with which to decode and
 *   interpret a file.
 * e_type: This member identifies the object file type.
 * e_machine: This member identifies the required architecture for a file.
 * e_version: This member identifies the object file version.
 * e_entry: This member gives the virtual address to which the system first
 *   transfers control, thus starting the process. If the file holds no
 *   associated entry point, this member holds zero.
 * e_phoff: This member holds the program header table's offset in bytes. If the
 *   file has no program header table, this member holds zero.
 * e_shoff: This member holds the section table's offset in bytes. If the file
 *   has no section header table, this member holds zero.
 * e_flags: This member holds processor-specific flags associated with the file.
 *   Flag names take the form EF_machine_flag.
 * e_ehsize: This member holds the ELF header's size in bytes.
 * e_phent_size: This member holds the size of one entry in the file's program
 *   header table; all entries are the same size.
 * e_phnum: This member holds the number of entries in the program header table.
 *   Thus the product of e_phent_size and ph_num gives teh tables size in bytes.
 *   If a file has no program header table this member holds zero.
 * e_shent_size: This member holds the size of one entry in the section header
 *   table; all entries are the same size.
 * e_shnum: This member holds the number of entries in the section header table.
 *   If a file has no section header table this member holds zero.
 * e_shstrndx: This member holds the section header table index of the entry
 *   associated with the section name string table. If the file has no section
 *   name string table, this member holds the value SH_UNDEF.
 */
typedef struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;

} Elf32_Ehdr;

typedef struct Elf64_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;

} Elf64_Ehdr;

#define ELF_HEADER_MAXLEN 64

/*
 * Decriptor for elf objects.
 */
typedef struct Elf_Desc {
    /* Size of the ELF object on-disk. */
    off_t e_size;

    /* Path for the file associated with this object. */
    char *filepath;
    
    /* File descriptor for reading or writing this object to or from disk. */
    int e_fd;
    
    /* Pointer to the beginning of the raw file if loaded in memory. */
    char *e_rawdata;

    /* Pointer to the beginning of the elf header. Usually the same as e_rawdata. */
    char *e_ident;
   
    /* 1 - 32-bit
     * 2 - 64-bit */
    unsigned char e_class;
    
    /* 1 - little-endian
     * 2 - big-endian */
    unsigned char e_encoding;

    union e_hdr {
        Elf32_Ehdr ehdr32;
        Elf64_Ehdr ehdr64;
    } e_hdr;

    /* Misc flagerinos. */
    unsigned e_readable:1;  /* File is readable. */
    unsigned e_writable:1;  /* File is Writable. */
    unsigned e_dirty:1;     /* e_rawdata has been modified. */
    unsigned e_mmapped:1;   /* e_rawdata is mmapped */

    uint64_t e_magic; /* Magic number for debugging. */
} Elf_Desc;

#define ELF_MAGIC 0x2A1D3297AFCED291ULL;

extern Elf_Desc *Elf_Open(const char *path);
// extern ssize_t Elf_Write(Elf_Desc *elf_desc, const char *path);
extern void Elf_Close(Elf_Desc *desc);
extern void Elf_Dump_Ident(Elf_Desc *desc);
extern void Elf_Dump_Header(Elf_Desc *desc);

static const char *get_elf_class(unsigned int elf_class);
static const char *get_data_encoding(unsigned int encoding);
static const char *get_elf_version(unsigned int version);
static const char *get_osabi_name(unsigned int osabi);
static const char *get_file_type(unsigned int file_type);
static const char *get_machine_name(unsigned int machine);

static uint16_t read_word_le(const char *src);
static uint16_t read_word_be(const char *src);
static uint32_t read_dword_le(const char *src);
static uint32_t read_dword_be(const char *src);
static uint64_t read_qword_le(const char *src);
static uint64_t read_qword_be(const char *src);


#endif // ELF_B8FA07
