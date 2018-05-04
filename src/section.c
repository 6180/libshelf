#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/elf.h"
#include "../include/section.h"


shelfsect_t *create_section(char *name) {
    shelfsect_t *new_sect;

    if (name == NULL) {
        printf("NULL pointer passed to create_section()\n");
        exit(-1);
    }

    new_sect = calloc(1, sizeof(shelfsect_t));
    new_sect->name = strdup(name);

    return new_sect;
}