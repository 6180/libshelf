#include <stdio.h>

#include "elf.h"
#include "elf_constants.h"

int main(void) {
    printf("Hello, World!\n");

    Elf_Desc *desc = Elf_Open("./lib/libelfbutcher.so");

    if (desc != NULL) {
        printf("Huzzah!\n");
        Elf_Dump_Ident(desc);
    }

    printf("Goodbye, World!\n");
}
