#include "../include/shelf_profiler.h"


int profiler_depth = 0;
int profiler_level = 0;

/* 
 * This is really hacky but it reduces a full path like 
 *   '/home/eevee/foo/bar/baz.h' to bar/baz.h. There are probably much cleaner
 *   ways to do this. If you know one please share~
 */
char *clean_filename(const char *file) {
    /* __FILE__ expands to a const * const char. The strdup removes 
      the const pointer qualification. */ 
    char *_dirname = basename(dirname(strdup(file)));
    char *_basename = basename(strdup(file));
    int tmplen = strlen(_dirname); 
    strncpy(buf, _dirname, BUF_LENGTH);
    buf[tmplen] = '/';        
    strncpy(buf + tmplen + 1, _basename, BUF_LENGTH - tmplen - 2);
    return buf;
}   

void set_profiler_level(int level) {
    profiler_level = level;
}

int get_profiler_level() {
    return profiler_level;
}
