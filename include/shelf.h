#ifndef SHELF_B8FA07
#define SHELF_B8FA07


#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "shelf_constants.h"

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
 * e_phentsize: This member holds the size of one entry in the file's program
 *   header table; all entries are the same size.
 * e_phnum: This member holds the number of entries in the program header table.
 *   Thus the product of e_phent_size and ph_num gives teh tables size in bytes.
 *   If a file has no program header table this member holds zero.
 * e_shentsize: This member holds the size of one entry in the section header
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

typedef Elf64_Ehdr shelf_Ehdr;

/*
 * An executable or shared object file's program header table is an array of
 * structures, each describing a segment or other information the system needs
 * to prepare the program for execution. An object file segment contains one or
 * more sections. Program headers are meaningful only for executable and shared
 * object files. A program specifies its own program header size with the ELF
 * header's e_phentsize and e_phnum members. The ELF program header is described
 * by the type Elf32_Phdr or Elf64_Phdr depending on the architecture.
 * 
 * p_type: This member of the structure indicates what kind of segment this
 *   array element describes or how to interpret the array element's information.
 * p_offset: This member holds the offset from the beginning of the file at
 *   which the first byte of the segment resides.
 * p_vaddr: This member holds the virtual address at which the first byte of
 *   this segment resides in memory.
 * p_paddr: On systems for which physical addressing is relevant, this member is
 *   reserved for the segment's physical address. Under BSD this member is not
 *   used and must be zero.
 * p_filesz: This member holds the number of bytes in the file image of the
 *   segment. It may be zero.
 * p_memsz: This member holds the number of bytes in the memory image of the
 *   segment. It may be zero.
 * p_flags: This member holds a bit-mask of flags relevant to the segment.
 * p_align: This member holds the value to which the segments are aligned in
 *   memory and in the file. Loadable process segments must have congruent
 *   values for p_vaddr and p_offset, modulo to the page size. Values of zero
 *   and one mean no alignment is required.Otherwise, p_align should be a
 *   positive, integral power of two, and p_vaddr should equal p_offset, modulo
 *   p_align.
 */
typedef struct Elf32_Phdr {
    uint32_t   p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    uint32_t   p_filesz;
    uint32_t   p_memsz;
    uint32_t   p_flags;
    uint32_t   p_align;
} Elf32_Phdr;

typedef struct Elf64_Phdr {
    uint32_t   p_type;
    uint32_t   p_flags;
    Elf64_Off  p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    uint64_t   p_filesz;
    uint64_t   p_memsz;
    uint64_t   p_align;
} Elf64_Phdr;

typedef Elf64_Phdr shelf_Phdr;

/*
 * A file's section header table lets one locate all the file's sections. The
 * section header table is an array of Elf32_Shdr or Elf64_Shdr structures. The
 * ELF header's e_shoff member gives the byte offset from the beginning of the
 * file to the section header table. e_shnum holds the number of entries the
 * section header table contains. e_shentsize holds the size in bytes of each entry.
 * 
 * sh_name: This member specifies the name of the section. It is an index into
 *   the section header string table section, giving the location of a
 *   null-terminated string.
 * sh_type: This member categorizes the sections contents and properties.
 * sh_flags: This member holds bitflags that can be used to describe
 *   miscellaneous attributes.
 * sh_addr: if this section appears in the memory image of a process, this
 *   member holds the address at which the first byte of the section should
 *   reside. Otherwise this member holds zero.
 * sh_offset: This members value holds the offset in bytes from the beginning
 *   of the file to the first byte in the section. One section type, SHT_NOBITS,
 *   occupies no space in the file and it's sh_offset member locates the
 *   conceptual placement in the file.
 * sh_size: This member holds the sections size in bytes. Unless the sections
 *   type is SHT_NOBITS, the section occupies sh_size bytes in t he file. A
 *   section of type SHT_NOBITS may have a non-zero size but occupies no space
 *   in the file.
 * sh_link: This memebr holds a section header table index link, whose 
 *   interpretation depends on the section type.
 * sh_info: This member holds extra information, whose interpretation depends
 *   on the section type.
 * sh_addralign: Some sections have address alignment constraints. If a section
 *   holds a doubleword, the system must ensure doubleword alignment for the
 *   entire section. That is, the value of sh_addr must be congruent to zero,
 *   modulo the value of sh_addralign. Only zero and positive integral powers
 *   of two are allowed. The value of 0 or 1 means the section has no alignment
 *   constraints.
 * sh_entsize: Some sections hold a table of fixed-size entries such as a symbol
 *   table. for such a section, this member gives the size in bytes for each
 *   entry. This member contains zero if the section does not hold a table of
 *   fixed-size entries.
 */
typedef struct Elf32_Shdr {
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint32_t   sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off  sh_offset;
    uint32_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint32_t   sh_addralign;
    uint32_t   sh_entsize;
} Elf32_Shdr;

typedef struct {
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint64_t   sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off  sh_offset;
    uint64_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint64_t   sh_addralign;
    uint64_t   sh_entsize;
} Elf64_Shdr;

typedef Elf64_Shdr shelf_Shdr;

/* 
 * Elf section descriptor.
 */
typedef struct shelf_sect {
    char *name;         /* Cached name. */
    shelf_Shdr *shdr;   /* Associated Elf64_Shdr for this section. */
    int index;          /* Index in sht. */
    void *data;         /* Pointer to sections data cache. */

    struct shelf_sect *prev;  /* Pointer to next section in list. */
    struct shelf_sect *next;  /* Pointer to previous section in list. */
} shelfsect_t;


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
    unsigned char *e_rawdata;

    /* Pointer to the beginning of the elf header. Usually the same as e_rawdata. */
    unsigned char *e_ident;
   
    /* 1 - 32-bit
     * 2 - 64-bit */
    unsigned char e_class;
    
    /* 1 - little-endian
     * 2 - big-endian */
    unsigned char e_encoding;

    /* Internally we'll just use the 64-bit structures to simplify implementation */
    Elf64_Ehdr e_hdr;
    Elf64_Phdr *e_phdr;
    Elf64_Shdr *e_shdr;

    /* Misc flagerinos. */
    unsigned e_readable:1;  /* File is readable. */
    unsigned e_writable:1;  /* File is Writable. */
    unsigned e_dirty:1;     /* e_rawdata has been modified. */
    unsigned e_mmapped:1;   /* e_rawdata is mmapped */
    unsigned e_malloced:1;  /* e_rawdata is allocated on yonder heap */
} Elf_Desc;

/*
 * Elf Object structure.
 */
typedef struct s_elfobj {
    shelf_Ehdr *hdr;
    shelf_Shdr *sht;
    Elf64_Phdr *pht;
    shelfsect_t *sect_list;

    int fd;
    char *filename;
    struct stat file_stat;
    int type;
    int writable;
    time_t load_time;
    char hdr_corrupt;
    char read;
    char mmapped;
    char malloced;
    char stripped;

} shelfobj_t;


/* 
 * Functions for creating and managing struct Elf_Desc objects. 
 */
extern Elf_Desc *Elf_Open(const char *path);
// extern ssize_t Elf_Write(Elf_Desc *elf_desc, const char *path);
extern void Elf_Close(Elf_Desc *desc);


/*
 * Accessor functions for individual header fields
 */
extern Elf64_Ehdr *elf_get_hdr(Elf_Desc *desc);

extern uint8_t  elf_get_class(Elf_Desc *desc);
extern uint8_t  elf_get_data(Elf_Desc *desc);
extern uint8_t  elf_get_osabi(Elf_Desc *desc);
extern uint8_t  elf_get_osabiversion(Elf_Desc *desc);
extern uint16_t elf_get_type(Elf_Desc *desc);
extern uint16_t elf_get_machine(Elf_Desc *desc);
extern uint32_t elf_get_version(Elf_Desc *desc);
extern uint64_t elf_get_entry(Elf_Desc *desc);
extern uint64_t elf_get_phoff(Elf_Desc *desc);
extern uint64_t elf_get_shoff(Elf_Desc *desc);
extern uint32_t elf_get_flags(Elf_Desc *desc);
extern uint16_t elf_get_ehsize(Elf_Desc *desc);
extern uint16_t elf_get_phentsize(Elf_Desc *desc);
extern uint16_t elf_get_phnum(Elf_Desc *desc);
extern uint16_t elf_get_shentsize(Elf_Desc *desc);
extern uint16_t elf_get_shnum(Elf_Desc *desc);
extern uint16_t elf_get_shstrndx(Elf_Desc *desc);

extern void elf_set_class(Elf_Desc *desc, uint8_t class);
extern void elf_set_data(Elf_Desc *desc, uint8_t data);
extern void elf_set_osabi(Elf_Desc *desc, uint8_t osabi);
extern void elf_set_osabiversion(Elf_Desc *desc, uint8_t osabiversion);
extern void elf_set_type(Elf_Desc *desc, uint16_t type);
extern void elf_set_machine(Elf_Desc *desc, uint16_t machine);
extern void elf_set_version(Elf_Desc *desc, uint32_t version);
extern void elf_set_entry(Elf_Desc *desc, uint64_t entry);
extern void elf_set_phoff(Elf_Desc *desc, uint64_t phoff);
extern void elf_set_shoff(Elf_Desc *desc, uint64_t shoff);
extern void elf_set_flags(Elf_Desc *desc, uint32_t flags);
extern void elf_set_ehsize(Elf_Desc *desc, uint16_t ehsize);
extern void elf_set_phentsize(Elf_Desc *desc, uint16_t phentsize);
extern void elf_set_phnum(Elf_Desc *desc, uint16_t phentnum);
extern void elf_set_shentsize(Elf_Desc *desc, uint16_t shentsize);
extern void elf_set_shnum(Elf_Desc *desc, uint16_t shentnum);
extern void elf_set_shstrndx(Elf_Desc *desc, uint16_t shstrndx);


/*
 * Helper functions for getting user friendly names for some header fields.
 */
extern const char *get_elf_class_str(unsigned int elf_class);
extern const char *get_data_encoding_str(unsigned int encoding);
extern const char *get_elf_version_str(unsigned int version);
extern const char *get_osabi_str(unsigned int osabi);
extern const char *get_file_type_str(unsigned int file_type);
extern const char *get_machine_str(unsigned int machine);
extern const char *get_phdr_type_str(unsigned int type);
extern const char *get_phdr_flags_str(unsigned int flags);
extern const char *get_shdr_type_str(uint32_t type);
extern const char *get_shdr_flags_str(unsigned int flags);


uint16_t read_word_le(const unsigned char *src);
uint16_t read_word_be(const unsigned char *src);
uint32_t read_dword_le(const unsigned char *src);
uint32_t read_dword_be(const unsigned char *src);
uint64_t read_qword_le(const unsigned char *src);
uint64_t read_qword_be(const unsigned char *src);

#endif // SHELF_B8FA07
