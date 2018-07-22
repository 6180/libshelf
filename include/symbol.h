#ifndef SHELF_SYMBOL_DDD0C4
#define SHELF_SYMBOL_DDD0C4

#include "shelf.h"

int		    elfsh_init_symbol_hashtables(shelfsect_t *desc);
shelfsym_t	*elfsh_get_symbol_by_name(shelfsect_t *desc, char *name);
char		*elfsh_reverse_symbol(shelfsect_t *desc, Elf64_Addr sym_value, Elf64_Addr *offset);
char		*elfsh_get_symbol_name(shelfsect_t *desc, shelfsym_t *s);
void		*elfsh_get_symtab(shelfsect_t *desc, int *num);
shelfsym_t	*elfsh_get_symbol_by_value(shelfsect_t *desc, Elf64_Addr vaddr, int *off, int mode);
shelfsym_t	*elfsh_get_dynsymbol_by_value(shelfsect_t *desc, Elf64_Addr vaddr, int *off, int mode);
int		    elfsh_strip(shelfsect_t *desc);
int		    elfsh_set_symbol_name(shelfsect_t *desc, shelfsym_t *s, char *name);
int		    elfsh_shift_symtab(shelfsect_t *desc, Elf64_Addr lim, int inc);
int		    elfsh_insert_sectsym(shelfsect_t *desc, shelfsect_t *sect);
int		    elfsh_get_symbol_foffset(shelfsect_t *desc, shelfsym_t *sym);
int		    elfsh_insert_symbol(shelfsect_t *sect, shelfsym_t *sym, char *name);
int		    elfsh_insert_funcsym(shelfsect_t *desc, char *name, Elf64_Addr vaddr, 
                             uint32_t sz, uint32_t sctidx);

#endif // SHELF_SYMBOL_DDD0C4
