#ifndef SHELF_SECTION_736AB8
#define SHELF_SECTION_736AB8

#include "shelf.h"


/* Function for retrieving sections. */
extern shelfsect_t *create_section(char *name);
extern shelfsect_t *get_section_by_name(shelfobj_t *desc, char *name); // TODO
extern shelfsect_t *get_section_by_index(shelfobj_t *desc, uint32_t index); // TODO
extern shelfsect_t *get_sections_by_type(shelfobj_t *desc, uint32_t type); // TODO
extern shelfsect_t *get_section_from_symbol(shelfobj_t *desc); // TODO
extern shelfsect_t *get_parent_section(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *get_parent_section_by_foffset(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *get_section_list(shelfobj_t *desc); // TODO
extern shelfsect_t *get_tail_section(shelfobj_t *desc); // TODO

/* Functions for reading/writing section data. */
extern void        *get_section_data(shelfobj_t *desc, Elf64_Shdr shdr); // TODO
extern int         *write_section_data(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern int         *append_data_to_section(shelfobj_t *desc, void *data, size_t len); // TODO
extern int         *fill_section(shelfobj_t *desc, Elf64_Shdr shdr, char c, size_t size); // TODO

/* Functions for adding and removing sections. */
extern shelfsect_t *add_section(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern shelfsect_t *add_runtime_section(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern int         *remove_section(shelfobj_t *desc, char *name); // TODO
extern int         *shift_section(shelfobj_t *desc, Elf64_Addr addr); // TODO
extern int         *swap_sections(shelfobj_t *desc, Elf64_Shdr shdr_a, Elf64_Shdr shdr_b); // TODO

/* Misc. */
extern void        free_shelfsect(shelfsect_t *sect);
void               load_section_list(shelfobj_t *desc);

#endif // SHELF_SECTION_736AB8