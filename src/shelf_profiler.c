#include "../include/shelf_profiler.h"


int profiler_depth = 0;
int profiler_level = 0;

/*
 * This is really hacky but it reduces a full path like
 *   '/home/eevee/foo/bar/baz.h' to bar/baz.h. There are probably much cleaner
 *   ways to do this. If you know one please share~
 */
// char *clean_filename(const char *file) {
//     /* __FILE__ expands to a const * const char. The strdup removes
//       the const pointer qualification. */
//     char *tmp_str = strdup(file);
//     char *tmp_base = tmp_str; // basename and dirname will modify the pointer we
//     char *_dirname = basename(dirname(strdup(file)));
//     char *_basename = basename(strdup(file));
//     int tmplen = strlen(_dirname);
//     strncpy(buffer, _dirname, BUFFER_LENGTH);
//     buffer[tmplen] = '/';
//     strncpy(buffer + tmplen + 1, _basename, BUFFER_LENGTH - tmplen - 2);
//     return buffer;
// }

char *clean_filename(const char *file) {
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

    strncpy(buffer, s + 1, BUFFER_LENGTH);
    free(cpy);
    return buffer;
}

void set_profiler_level(int level) {
    profiler_level = level;
}

int get_profiler_level() {
    return profiler_level;
}
