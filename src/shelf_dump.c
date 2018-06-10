#include <stdio.h>
#include <stdlib.h>

#include "../include/shelf.h"
#include "../include/shelf_dump.h"
#include "../include/shelf_profiler.h"


void shelf_dump_ident(const shelfobj_t *desc)
{
    PROFILER_IN();

    if (desc == NULL)
        PROFILER_ERR("Null pointer passed");

    printf("E_Ident:\n");
    printf("\tMagic: %02x %02x %02x %02x\n",
        desc-> e_ident[EI_MAG0],
        desc-> e_ident[EI_MAG1],
        desc-> e_ident[EI_MAG2],
        desc-> e_ident[EI_MAG3]
    );
    printf("\tClass: %s\n", get_elf_class_str(desc->e_ident[EI_CLASS]));
    printf("\tData: %s\n", get_data_encoding_str(desc->e_ident[EI_DATA]));
    printf("\tVersion: %s\n", get_elf_version_str(desc->e_ident[EI_VERSION]));
    printf("\tOS/ABI: %s\n", get_osabi_str(desc->e_ident[EI_OSABI]));
    printf("\tOS/ABI Version: %d\n", desc->e_ident[EI_ABIVERSION]);

    PROFILER_OUT();
}

void shelf_dump_header(const shelfobj_t *desc)
{
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Header()\n");
        exit(-1);
    }

    printf("Elf Header:\n");
    printf("  Magic:                             %02x %02x %02x %02x\n",
        desc-> e_ident[EI_MAG0],
        desc-> e_ident[EI_MAG1],
        desc-> e_ident[EI_MAG2],
        desc-> e_ident[EI_MAG3]
    );
    printf("  Class:                             %s\n", get_elf_class_str(desc->e_ident[EI_CLASS]));
    printf("  Data:                              %s\n", get_data_encoding_str(desc->e_ident[EI_DATA]));
    printf("  Version:                           %s\n", get_elf_version_str(desc->e_ident[EI_VERSION]));
    printf("  OS/ABI:                            %s\n", get_osabi_str(desc->e_ident[EI_OSABI]));
    printf("  OS/ABI Version:                    %d\n", desc->e_ident[EI_ABIVERSION]);
    printf("  Type:                              %s\n", get_file_type_str(desc->hdr.e_type));
    printf("  Machine:                           %s\n", get_machine_str(desc->hdr.e_machine));
    printf("  Version:                           %s\n", get_elf_version_str(desc->hdr.e_version));

    if (desc->ei_class != 2) {
        printf("  Entry:                             0x%1$08x (%1$u)\n", (unsigned int) desc->hdr.e_entry);
        printf("  Program Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->hdr.e_phoff);
        printf("  Section Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->hdr.e_shoff);
    } else {
        printf("  Entry:                             0x%lx\n", desc->hdr.e_entry);
        printf("  Program Header Offset:             0x%1$lx (%1$lu)\n", desc->hdr.e_phoff);
        printf("  Section Header Offset:             0x%1$lx (%1$lu)\n", desc->hdr.e_shoff);
    }

    printf("  Flags:                             0x%1$x (%1$d)\n", desc->hdr.e_flags);
    printf("  Header Size:                       0x%1$x (%1$d)\n", desc->hdr.e_ehsize);
    printf("  Size of program headers:           0x%1$x (%1$d)\n", desc->hdr.e_phentsize);
    printf("  Number of program headers:         %d\n", desc->hdr.e_phnum);
    printf("  Size of section headers:           0x%1$x (%1$d)\n", desc->hdr.e_shentsize);
    printf("  Number of section headers:         %d\n", desc->hdr.e_shnum);
    printf("  Section header string table index: %u\n", desc->hdr.e_shstrndx);
}

void shelf_dump_program_headers(const shelfobj_t *desc)
{
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Program_Headers()\n");
        exit(-1);
    }

    printf("Program Headers:\n\n");

    if (desc->ei_class != 2) {
        printf("  Type      Offset     VirtAddr  PhysAddr\n"
               "            FileSize   MemSize   Flags  Align\n"
               "---------------------------------------------------\n");
    } else {
        printf("  Type      Offset             VirtAddr           PhysAddr\n"
               "            FileSize           MemSize            Flags  Align\n"
               "--------------------------------------------------------------------\n");
    }

    for (uint16_t i = 0; i < desc->hdr.e_phnum; i++) {
        if (desc->ei_class != 2) { // 32-bit
            printf("  %-10s0x%08x 0x%08x 0x%08x\n"
                   "            0x%08x 0x%08x %s  %#x\n",
                get_phdr_type_str(desc->pht[i].p_type),
                (uint32_t) desc->pht[i].p_offset,
                (uint32_t) desc->pht[i].p_vaddr,
                (uint32_t) desc->pht[i].p_paddr,
                (uint32_t) desc->pht[i].p_filesz,
                (uint32_t) desc->pht[i].p_memsz,
                get_phdr_flags_str(desc->pht[i].p_flags),
                (uint32_t) desc->pht[i].p_align
            );
        } else { // 64-bit
            printf("  %-10s0x%016lx 0x%016lx 0x%016lx\n"
                   "            0x%016lx 0x%016lx %s  0x%lx\n",
                get_phdr_type_str(desc->pht[i].p_type),
                desc->pht[i].p_offset,
                desc->pht[i].p_vaddr,
                desc->pht[i].p_paddr,
                desc->pht[i].p_filesz,
                desc->pht[i].p_memsz,
                get_phdr_flags_str(desc->pht[i].p_flags),
                desc->pht[i].p_align
            );
        }
    }
}

void shelf_dump_section_headers(const shelfobj_t *desc)
{
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Section_Headers()\n");
        exit(-1);
    }

    printf("Section Headers:\n\n");

    if (desc->ei_class != 2) { // 32-bit
        printf("  [Nr] Name       Type       Address    Offset\n"
               "       Size       EntSize    Flags  Link  Info  Align\n"
               "------------------------------------------------------\n");

        for (uint16_t i = 0; i < desc->hdr.e_shnum; i++) {
            printf("  [%-2d] %-10s %-10s 0x%08x 0x%08x\n"
                    "       0x%08x 0x%08x %-5.5s\n",
                i,
                // get_shdr_name(desc, desc->e_shdr[i].sh_name),
                desc->data + desc->sht[desc->hdr.e_shstrndx].sh_offset + desc->sht[i].sh_name,
                get_shdr_type_str(desc->sht[i].sh_type),
                (uint32_t) desc->sht[i].sh_addr,
                (uint32_t) desc->sht[i].sh_offset,
                (uint32_t) desc->sht[i].sh_size,
                (uint32_t) desc->sht[i].sh_entsize,
                get_shdr_flags_str(desc->sht[i].sh_flags)
            );
        }
    } else { // 64-bit
        printf("  [Nr] Name               Type               Address            Offset\n"
               "       Size               EntSize            Flags  Link  Info  Align\n"
               "--------------------------------------------------------------------------\n"
        );

        for (uint16_t i = 0; i < desc->hdr.e_shnum; i++) {
            printf("  [%-2d] %-18s %-18s 0x%016lx 0x%08lx\n"
                    "       0x%016lx 0x%016lx %-5.5s\n",
                i,
                desc->data + desc->sht[desc->hdr.e_shstrndx].sh_offset + desc->sht[i].sh_name,
                get_shdr_type_str(desc->sht[i].sh_type),
                desc->sht[i].sh_addr,
                desc->sht[i].sh_offset,
                desc->sht[i].sh_size,
                desc->sht[i].sh_entsize,
                get_shdr_flags_str(desc->sht[i].sh_flags)
            );
        }
    }
}
