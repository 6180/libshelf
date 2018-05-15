#include <stdio.h>
#include <stdlib.h>

#include "../include/shelf.h"
#include "../include/shelf_dump.h"


void Shelf_Dump_Ident(const Elf_Desc *desc) {
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
    printf("\tClass: %s\n", get_elf_class_str(desc->e_ident[EI_CLASS]));
    printf("\tData: %s\n", get_data_encoding_str(desc->e_ident[EI_DATA]));
    printf("\tVersion: %s\n", get_elf_version_str(desc->e_ident[EI_VERSION]));
    printf("\tOS/ABI: %s\n", get_osabi_str(desc->e_ident[EI_OSABI]));
    printf("\tOS/ABI Version: %d\n", desc->e_ident[EI_ABIVERSION]);
}

void Shelf_Dump_Header(const Elf_Desc *desc) {
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
    printf("  Type:                              %s\n", get_file_type_str(desc->e_hdr.e_type));
    printf("  Machine:                           %s\n", get_machine_str(desc->e_hdr.e_machine));
    printf("  Version:                           %s\n", get_elf_version_str(desc->e_hdr.e_version));

    if (desc->e_class != 2) {
        printf("  Entry:                             0x%1$08x (%1$u)\n", (unsigned int) desc->e_hdr.e_entry);
        printf("  Program Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->e_hdr.e_phoff);
        printf("  Section Header Offset:             0x%1$x (%1$u)\n", (unsigned int) desc->e_hdr.e_shoff);
    } else {
        printf("  Entry:                             0x%lx\n", desc->e_hdr.e_entry);
        printf("  Program Header Offset:             0x%1$lx (%1$lu)\n", desc->e_hdr.e_phoff);
        printf("  Section Header Offset:             0x%1$lx (%1$lu)\n", desc->e_hdr.e_shoff);
    }

    printf("  Flags:                             0x%1$x (%1$d)\n", desc->e_hdr.e_flags);
    printf("  Header Size:                       0x%1$x (%1$d)\n", desc->e_hdr.e_ehsize);
    printf("  Size of program headers:           0x%1$x (%1$d)\n", desc->e_hdr.e_phentsize);
    printf("  Number of program headers:         %d\n", desc->e_hdr.e_phnum);
    printf("  Size of section headers:           0x%1$x (%1$d)\n", desc->e_hdr.e_shentsize);
    printf("  Number of section headers:         %d\n", desc->e_hdr.e_shnum);
    printf("  Section header string table index: %u\n", desc->e_hdr.e_shstrndx);
}

void Shelf_Dump_Program_Headers(const Elf_Desc *desc) {
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Program_Headers()\n");
        exit(-1);
    }

    printf("Program Headers:\n\n");

    if (desc->e_class != 2) {
        printf("  Type      Offset     VirtAddr  PhysAddr\n"
               "            FileSize   MemSize   Flags  Align\n"
               "---------------------------------------------------\n");
    } else {
        printf("  Type      Offset             VirtAddr           PhysAddr\n"
               "            FileSize           MemSize            Flags  Align\n"
               "--------------------------------------------------------------------\n");
    }

    for (uint16_t i = 0; i < desc->e_hdr.e_phnum; i++) {
        if (desc->e_class != 2) { // 32-bit
            printf("  %-10s0x%08x 0x%08x 0x%08x\n"
                   "            0x%08x 0x%08x %s  %#x\n",
                get_phdr_type_str(desc->e_phdr[i].p_type),
                (uint32_t) desc->e_phdr[i].p_offset,
                (uint32_t) desc->e_phdr[i].p_vaddr,
                (uint32_t) desc->e_phdr[i].p_paddr,
                (uint32_t) desc->e_phdr[i].p_filesz,
                (uint32_t) desc->e_phdr[i].p_memsz,
                get_phdr_flags_str(desc->e_phdr[i].p_flags),
                (uint32_t) desc->e_phdr[i].p_align                
            );
        } else { // 64-bit
            printf("  %-10s0x%016lx 0x%016lx 0x%016lx\n"
                   "            0x%016lx 0x%016lx %s  0x%lx\n",
                get_phdr_type_str(desc->e_phdr[i].p_type),
                desc->e_phdr[i].p_offset,
                desc->e_phdr[i].p_vaddr,
                desc->e_phdr[i].p_paddr,
                desc->e_phdr[i].p_filesz,
                desc->e_phdr[i].p_memsz,
                get_phdr_flags_str(desc->e_phdr[i].p_flags),
                desc->e_phdr[i].p_align
            );
        }
    }
}

void Shelf_Dump_Section_Headers(const Elf_Desc *desc){
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Section_Headers()\n");
        exit(-1);
    }

    printf("Section Headers:\n\n");
    
    if (desc->e_class != 2) { // 32-bit
        printf("  [Nr] Name       Type       Address    Offset\n"
               "       Size       EntSize    Flags  Link  Info  Align\n"
               "------------------------------------------------------\n");
        
        for (uint16_t i = 0; i < desc->e_hdr.e_shnum; i++) {
            printf("  [%-2d] %-10s %-10s 0x%08x 0x%08x\n"
                    "       0x%08x 0x%08x %-5.5s\n",
                i,
                // get_shdr_name(desc, desc->e_shdr[i].sh_name),
                desc->e_rawdata + desc->e_shdr[desc->e_hdr.e_shstrndx].sh_offset + desc->e_shdr[i].sh_name,
                get_shdr_type_str(desc->e_shdr[i].sh_type),
                (uint32_t) desc->e_shdr[i].sh_addr,
                (uint32_t) desc->e_shdr[i].sh_offset,
                (uint32_t) desc->e_shdr[i].sh_size,
                (uint32_t) desc->e_shdr[i].sh_entsize,
                get_shdr_flags_str(desc->e_shdr[i].sh_flags)
            );
        }
    } else { // 64-bit
        printf("  [Nr] Name               Type               Address            Offset\n"
            "       Size               EntSize            Flags  Link  Info  Align\n"
            "--------------------------------------------------------------------------\n");
        
        for (uint16_t i = 0; i < desc->e_hdr.e_shnum; i++) {
            printf("  [%-2d] %-18s %-18s 0x%016lx 0x%08lx\n"
                    "       0x%016lx 0x%016lx %-5.5s\n",
                i,
                // get_shdr_name(desc, desc->e_shdr[i].sh_name),
                desc->e_rawdata + desc->e_shdr[desc->e_hdr.e_shstrndx].sh_offset + desc->e_shdr[i].sh_name,
                get_shdr_type_str(desc->e_shdr[i].sh_type),
                desc->e_shdr[i].sh_addr,
                desc->e_shdr[i].sh_offset,
                desc->e_shdr[i].sh_size,
                desc->e_shdr[i].sh_entsize,
                get_shdr_flags_str(desc->e_shdr[i].sh_flags)
            );
        }
    }
}
