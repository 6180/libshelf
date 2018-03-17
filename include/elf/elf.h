#ifndef ELF_B8FA07
#define ELF_B8FA07


#include <stdint.h>

typedef Elf_Byte unsigned char

typedef Elf32_Addr     uint32_t
typedef Elf32_Off      uint32_t
typedef Elf32_Section  uint16_t
typedef Elf32_Versym   uint16_t
typedef Elf32_Half     uint16_t
typedef Elf32_Sword    int32_t
typedef Elf32_Word     uint32_t
typedef Elf32_Sxword   int64_t
typedef Elf32_Xword    uint64_t

typedef Elf64_Addr     uint64_t
typedef Elf64_Off      uint64_t
typedef Elf64_Section  uint16_t
typedef Elf64_Versym   uint16_t
typedef Elf64_Half     uint16_t
typedef Elf64_Sword    int32_t
typedef Elf64_Word     uint32_t
typedef Elf64_Sxword   int64_t
typedef Elf64_Xword    uint64_t


#include "elf_constants.h"


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
typedef struct {
    unsigned char e_ident[EI_NIDENT];
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

typedef struct {
    unsigned char e_ident[EI_NIDENT];
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

#endif // ELF_B8FA07
