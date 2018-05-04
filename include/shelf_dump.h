#ifndef ELF_5CD73F63
#define ELF_5CD73F63

#include "shelf.h"

extern void Elf_Dump_Ident(const Elf_Desc *desc);
extern void Elf_Dump_Header(const Elf_Desc *desc);
extern void Elf_Dump_Program_Headers(const Elf_Desc *desc);
extern void Elf_Dump_Section_Headers(const Elf_Desc *desc);

#endif // ELF_5CD73F63
