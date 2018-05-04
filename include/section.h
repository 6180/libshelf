#ifndef ELF_SECTION_736AB8
#define ELF_SECTION_736AB8

#include "elf.h"


/* Elf section descriptor. */
typedef struct shelf_sect {
    char *name;         /* Cached name. */
    Elf64_Shdr *shdr;   /* Associated Elf64_Shdr for this section. */
    int index;          /* Index in sht. */
    void *data;         /* Pointer to sections data cache. */

    struct shelf_sect *prev;  /* Pointer to next section in list. */
    struct shelf_sect *next;  /* Pointer to previous section in list. */
} shelfsect_t;

/* Function for retrieving section headers. */
extern shelfsect_t *create_section(char *name); // TODO
extern shelfsect_t *get_section_by_name(Elf_Desc *desc, char *name); // TODO
extern shelfsect_t *get_section_by_index(Elf_Desc *desc, uint32_t index); // TODO
extern shelfsect_t *get_sections_by_type(Elf_Desc *desc, uint32_t type); // TODO
extern shelfsect_t *get_section_from_symbol(Elf_Desc *desc); // TODO
extern shelfsect_t *get_parent_section(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *get_parent_section_by_foffset(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *get_section_list(Elf_Desc *desc); // TODO
extern shelfsect_t *get_tail_section(Elf_Desc *desc); // TODO

/* Functions for reading/writing section data. */
extern void        *get_section_data(Elf_Desc *desc, Elf64_Shdr shdr); // TODO
extern int         *write_section_data(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern int         *append_data_to_section(Elf_Desc *desc, void *data, size_t len); // TODO
extern int         *fill_section(Elf_Desc *desc, Elf64_Shdr shdr, char c, size_t size); // TODO

/* Functions for adding and removing sections. */
extern shelfsect_t *add_section(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *add_runtime_section(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern int         *remove_section(Elf_Desc *desc, char *name); // TODO
extern int         *shift_section(Elf_Desc *desc, Elf64_Addr addr); // TODO
extern int         *swap_sections(Elf_Desc *desc, Elf64_Shdr shdr_a, Elf64_Shdr shdr_b); // TODO

/* Misc. */
extern void        free_shelfsect(shelfsect_t *sect);

#endif // ELF_SECTION_736AB8