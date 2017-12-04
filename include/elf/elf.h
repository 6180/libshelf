#ifndef ELF_639918_H
#define ELF_639918_H

#include <stdint.h>

/*
 * Constants for indexing into e_ident
 */
#define EI_MAG0 0
#define EI_MAG0 1
#define EI_MAG0 2
#define EI_MAG0 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_PAD 7

/*
 * A files first 4 bytes hold a "magic number" identifying the file as an ELF
 * object file.
 * 
 * ELFMAG0: e_ident[EI_MAG0]
 * ELFMAG1: e_ident[EI_MAG1]
 * ELFMAG2: e_ident[EI_MAG2]
 * ELFMAG3: e_ident[EI_MAG3]
 */
#define ELFMAG0 '\x7f'
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

/*
 * Byte e_ident[EI_CLASS] identifies the file's class or capacity.
 * 
 * ELFCLASSNONE: Invalid class.
 * ELFCLASS32:   32-bit objects.
 * ELFCLASS64:   64-bit objects.  
 */
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

/*
 * Byte e_ident[EI_DATA] identifies the data encoding of the processor-specific
 * data in the object file.
 * 
 * ELFDATANONE: Invalid data encoding.
 * ELFDATA2LSB: 2's compliment values with the least significant byte
 *   occupying the lowest memory address (little endian).
 * ELFDATA2MSB: 2's compliment values with the most significant byte
 *   occupying the lowest memory address (big endian).
 */
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/*
 * Byte e_ident[EI_VERSION] identifies the ELF header version number.
 * 
 * EV_NONE: Invalid version.
 * EV_CURRENT: Current version.
 */
#define EV_NONE 0
#define EV_CURRENT

/*
 * Currently defined vlaues for e_machine.
 */
#define EM_NONE 0         /* No machine */
#define EM_M32 1          /* AT&T WE 32100 */
#define EM_SPARC 2        /* SPARC */
#define EM_386 3          /* Intel 80386 */
#define EM_68K 4          /* Motorola 68000 */
#define EM_88K 5          /* Motorola 88000 */
#define EM_860 7          /* Intel 80860 */
#define EM_MIPS 8         /* MIPS I Architecture */
#define EM_S370 9         /* IBM System/370 Processor */
#define EM_MIPS_RS3_LE 10 /* MIPS RS3000 Little-endian */
#define EM_PARISC 15      /* Hewlett-Packard PA-RISC */
#define EM_VPP500 17      /* Fujitsu VPP500 */
#define EM_SPARC32PLUS 18 /* Enhanced instruction set SPARC */
#define EM_960 19         /* Intel 80960 */
#define EM_PPC 20         /* PowerPC */
#define EM_PPC64 21       /* 64-bit PowerPC */
#define EM_S390 22        /* IBM System/390 Processor */
#define EM_V800 36        /* NEC V800 */
#define EM_FR20 37        /* Fujitsu FR20 */
#define EM_RH32 38        /* TRW RH-32 */
#define EM_RCE 39         /* Motorola RCE */
#define EM_ARM 40         /* Advanced RISC Machines ARM */
#define EM_ALPHA 41       /* Digital Alpha */
#define EM_SH 42          /* Hitachi SH */
#define EM_SPARCV9 43     /* SPARC Version 9 */
#define EM_TRICORE 44     /* Siemens TriCore embedded processor */
#define EM_ARC 45         /* Argonaut RISC Core, Argonaut Technologies Inc. */
#define EM_H8_300 46      /* Hitachi H8/300 */
#define EM_H8_300H 47     /* Hitachi H8/300H */
#define EM_H8S 48         /* Hitachi H8S */
#define EM_H8_500 49      /* Hitachi H8/500 */
#define EM_IA_64 50       /* Intel IA-64 processor architecture */
#define EM_MIPS_X 51      /* Stanford MIPS-X */
#define EM_COLDFIRE 52    /* Motorola ColdFire */
#define EM_68HC12 53      /* Motorola M68HC12 */
#define EM_MMA 54         /* Fujitsu MMA Multimedia Accelerator */
#define EM_PCP 55         /* Siemens PCP */
#define EM_NCPU 56        /* Sony nCPU embedded RISC processor */
#define EM_NDR1 57        /* Denso NDR1 microprocessor */
#define EM_STARCORE 58    /* Motorola Star*Core processor */
#define EM_ME16 59        /* Toyota ME16 processor */
#define EM_ST100 60       /* STMicroelectronics ST100 processor */
#define EM_TINYJ 61       /* Advanced Logic Corp. TinyJ embedded processor family */
#define EM_X86_64 62      /* AMD x86-64 architecture */
#define EM_PDSP 63        /* Sony DSP Processor */
#define EM_PDP10 64       /* Digital Equipment Corp. PDP-10 */
#define EM_PDP11 65       /* Digital Equipment Corp. PDP-11 */
#define EM_FX66 66        /* Siemens FX66 microcontroller */
#define EM_ST9PLUS 67     /* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7 68         /* STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16 69      /* Motorola MC68HC16 Microcontroller */
#define EM_68HC11 70      /* Motorola MC68HC11 Microcontroller */
#define EM_68HC08 71      /* Motorola MC68HC08 Microcontroller */
#define EM_68HC05 72      /* Motorola MC68HC05 Microcontroller */
#define EM_SVX 73         /* Silicon Graphics SVx */
#define EM_ST19 74        /* STMicroelectronics ST19 8-bit microcontroller */
#define EM_VAX 75         /* Digital VAX */
#define EM_CRIS 76        /* Axis Communications 32-bit embedded processor */
#define EM_JAVELIN 77     /* Infineon Technologies 32-bit embedded processor */
#define EM_FIREPATH 78    /* Element 14 64-bit DSP Processor */
#define EM_ZSP 79         /* LSI Logic 16-bit DSP Processor */
#define EM_MMIX 80        /* Donald Knuth's educational 64-bit processor */
#define EM_HUANY 81       /* Harvard University machine-independent object files */
#define EM_PRISM 82       /* SiTera Prism */
#define EM_AVR 83         /* Atmel AVR 8-bit microcontroller */
#define EM_FR30 84        /* Fujitsu FR30 */
#define EM_D10V 85        /* Mitsubishi D10V */
#define EM_D30V 86        /* Mitsubishi D30V */
#define EM_V850 87        /* NEC v850 */
#define EM_M32R 88        /* Mitsubishi M32R */
#define EM_MN10300 89     /* Matsushita MN10300 */
#define EM_MN10200 90     /* Matsushita MN10200 */
#define EM_PJ 91          /* picoJava */
#define EM_OPENRISC 92    /* OpenRISC 32-bit embedded processor */
#define EM_ARC_A5 93      /* ARC Cores Tangent-A5 */
#define EM_XTENSA 94      /* Tensilica Xtensa Architecture */
#define EM_VIDEOCORE 95   /* Alphamosaic VideoCore processor */
#define EM_TMM_GPP 96     /* Thompson Multimedia General Purpose Processor */
#define EM_NS32K 97       /* National Semiconductor 32000 series */
#define EM_TPC 98         /* Tenor Network TPC processor */
#define EM_SNP1K 99       /* Trebia SNP 1000 processor */
#define EM_ST200 100      /* STMicroelectronics (www.st.com) ST200 microcontroller */

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
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phent_size;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr_t;

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phent_size;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr_t;

#endif
