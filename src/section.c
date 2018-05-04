#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/shelf.h"
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

void free_shelfsect(shelfsect_t *sect) {
    if (sect == NULL) {
        printf("NULL pointer passed to free_shelfsect()\n");
        exit(-1);
    }

    if (sect->name) {
        free(sect->name);
        sect->name = NULL;
    }

    if (sect->shdr) {
        free(sect->shdr);
        sect->shdr = NULL;
    }

    if (sect->data) {
        free(sect->data);
        sect->data = NULL;
    }

    free(sect);
    sect = NULL;
}

