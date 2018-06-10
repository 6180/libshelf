#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/shelf.h"
#include "../include/shelf_profiler.h"
#include "../include/section.h"


shelfsect_t *create_section(char *name)
{
    shelfsect_t *new_sect;

    if (name == NULL) {
        printf("NULL pointer passed to create_section()\n");
        exit(-1);
    }

    new_sect = calloc(1, sizeof(shelfsect_t));
    new_sect->name = strdup(name);

    return new_sect;
}

shelfsect_t *get_section_by_name(shelfobj_t *desc, char *name)
{
    shelfsect_t *ret = 0;

    PROFILER_IN();

    /* sanity checks. */
    if (desc == NULL || name == NULL)
        PROFILER_RERR("NULL argument passed to get_section_by_name()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    for (uint32_t i = 0; i < desc->hdr.e_shnum; i++) {
        if (!strcmp(name, desc->sect_list[i].name)) {
            ret = &desc->sect_list[i];
            break;
        }
    }

    PROFILER_ROUT(ret, "shelfsect_t: %p");
}

void free_shelfsect(shelfsect_t *sect)
{
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

void load_section_list(shelfobj_t *desc)
{
    shelf_Shdr *cur_shdr;

    PROFILER_IN();
    
    uint32_t section_count = desc->hdr.e_shnum;
    // pointer to the beginning of the shstrtab data in file
    char *strtab = (char *)(desc->data + desc->sht[desc->hdr.e_shstrndx].sh_offset);

    desc->sect_list = calloc(1, section_count * sizeof(shelfsect_t));

    for (uint32_t i = 0; i < section_count; i++) {
        cur_shdr = &desc->sht[i];
        // copy name from shstrtab for current shdr
        desc->sect_list[i].name = strdup(strtab + cur_shdr->sh_name);
        desc->sect_list[i].shdr = cur_shdr;
        desc->sect_list[i].index = i;
        // TODO: possibly load section contents here instead of lazy loading
    }

    PROFILER_OUT();
}

