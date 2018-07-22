#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shelf.h"
#include "shelf_profiler.h"
#include "section.h"


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
    shelfsect_t *ret = NULL;

    PROFILER_IN();

    /* sanity checks. */
    if (desc == NULL || name == NULL)
        PROFILER_RERR("NULL argument passed to get_section_by_name()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    for (size_t i = 0; i < desc->hdr.e_shnum; i++) {
        if (!strcmp(name, desc->sect_list[i].name)) {
            ret = &desc->sect_list[i];
            break;
        }
    }

    PROFILER_ROUT(ret, "shelfsect_t: %p");
}

shelfsect_t *get_section_by_index(shelfobj_t *desc, uint32_t index)
{
    shelfsect_t *ret = NULL;

    PROFILER_IN();

    if (desc == NULL || index >= desc->hdr.e_shnum)
        PROFILER_RERR("Bad argument passed to get_section_by_index()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    for (size_t i = 0; i < desc->hdr.e_shnum; i++) {
        if (desc->sect_list[i].index == index) {
            ret = &desc->sect_list[i];
            break;
        }
    }

    PROFILER_ROUT(ret, "shelfsect_t: %p");    
}

shelfsect_t **get_sections_by_type(shelfobj_t *desc, uint32_t type)
{
    shelfsect_t **sections = NULL;
    uint32_t matches = 0;

    PROFILER_IN();

    if (desc == NULL)
        PROFILER_RERR("Null argument passed to get_sections_by_type()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    // Count the matches first instead of using a dynamic collection
    for (size_t i = 0; i < desc->hdr.e_shnum; i++) {
        if (desc->sht[i].sh_type == type)
            matches++;
    }

    sections = malloc((matches + 1) * sizeof(shelfsect_t*));
    // NULL pointer terminated array
    sections[matches] = NULL;
    size_t idx = 0;

    // Populate sections array with pointers to matching sections
    for (size_t i = 0; i < desc->hdr.e_shnum; i++) {
        if (desc->sht[i].sh_type == type)
            sections[idx++] = &(desc->sect_list[i]);
    }

    PROFILER_ROUT(sections, "shelfsect_t**: %p");
}

shelfsect_t *get_parent_section(shelfobj_t *desc, Elf64_Addr addr)
{
        PROFILER_IN();

    if (desc == NULL)
        PROFILER_RERR("Null argument passed to get_parent_section()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    for (size_t i = 1; i < desc->hdr.e_shnum; i++) {
        shelfsect_t *cur_sect = &(desc->sect_list[i]); 
        
        if (cur_sect->shdr->sh_addr <= addr && 
            cur_sect->shdr->sh_addr + cur_sect->shdr->sh_size >= addr) {
            
            PROFILER_ROUT(cur_sect, "shelfsect_t *: %p");
        }
    }
}

shelfsect_t *get_parent_section_by_foffset(shelfobj_t *desc, Elf64_Addr addr)
{
    PROFILER_IN();

    if (desc == NULL)
        PROFILER_RERR("Null argument passed to get_parent_section_by_foffset()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    for (size_t i = 1; i < desc->hdr.e_shnum; i++) {
        shelfsect_t *cur_sect = &(desc->sect_list[i]); 
        
        if (cur_sect->shdr->sh_offset <= addr && 
            cur_sect->shdr->sh_offset + cur_sect->shdr->sh_size >= addr) {
            
            PROFILER_ROUT(cur_sect, "shelfsect_t *: %p");
        }
    }

    PROFILER_ROUT(NULL, "shelfsect_t *: %p");
}

shelfsect_t *get_section_list(shelfobj_t *desc)
{
    PROFILER_IN();

    if (desc == NULL)
        PROFILER_RERR("Null argument passed to get_section_list()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);
    
    PROFILER_ROUT(desc->sect_list, "shelfsect_t *: %p");
}

shelfsect_t *get_tail_section(shelfobj_t *desc)
{
    PROFILER_IN();

    if (desc == NULL)
        PROFILER_RERR("Null argument passed to get_tail_section()\n", NULL);

    if (desc->sect_list == NULL)
        load_section_list(desc);

    PROFILER_ROUT(&(desc->sect_list[desc->hdr.e_shnum - 1]), "shelfsect_t: %p");
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

