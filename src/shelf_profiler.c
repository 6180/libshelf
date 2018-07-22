#include <stdlib.h>
#include <string.h>

#include "../include/shelf_profiler.h"


int profiler_depth = 0;
int profiler_level = 0;

/*
 * This is really hacky but it reduces a full path like
 *   '/home/eevee/foo/bar/baz.h' to bar/baz.h. There are probably much cleaner
 *   ways to do this. If you know one feel free to call me an idiot and share.
 */
char *clean_filename(const char *file)
{
    char *cpy = strdup(file);
    char *s = cpy + strlen(cpy);

    int sep_count = 0;

    while (sep_count != 2) {
        sep_count += (*--s == '/');
        if (s == file) {
            s--;
            break;
        }
    }

    strncpy(buffer, s + 1, BUFFER_LENGTH - 1);
    free(cpy);
    return buffer;
}

void set_profiler_level(int level)
{
    profiler_level = level;
}

int get_profiler_level()
{
    return profiler_level;
}
