#ifndef ELF_5CD73F63
#define ELF_5CD73F63

#include "shelf.h"

extern void shelf_dump_ident(const shelfobj_t *desc);
extern void shelf_dump_header(const shelfobj_t *desc);
extern void shelf_dump_program_headers(const shelfobj_t *desc);
extern void shelf_dump_section_headers(const shelfobj_t *desc);

#endif // ELF_5CD73F63
