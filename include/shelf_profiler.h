#ifndef SHELF_PROFILER_77828342
#define SHELF_PROFILER_77828342

#include <libgen.h>
#include <string.h>

/* Constants for coloring terminal output. */
#define C_NRM  "\x1B[0m"
#define C_RED  "\x1B[31m"
#define C_GRN  "\x1B[32m"
#define C_YEL  "\x1B[33m"
#define C_BLU  "\x1B[34m"
#define C_MAG  "\x1B[35m"
#define C_CYN  "\x1B[36m"
#define C_WHT  "\x1B[37m

/* Temporary buffer to avoid some allocations. */
#define BUF_LENGTH 1024
static char buf[BUF_LENGTH];

/* Bit flags for `profiler_level` */
#define PROFILE_NONE  0
#define PROFILE_ERROR (1 << 0)
#define PROFILE_FUNCS (1 << 1)
#define PROFILE_ALLOC (1 << 2)
#define PROFILE_DEBUG (1 << 3)

int profiler_depth; /* How deep are we in function trace depth? */
int profiler_level; /* How verbose should the profiler be? */

/* Believe it or not this sets the `profiler_level` variable. */
extern void set_profiler_level(int level);
/* You already know what it do. */
extern int get_profiler_level();

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

char *clean_filename(const char *file);

#define PROFILER_DEBUG(fmt, args...) do {                                \
    if ((profiler_level & PROFILE_DEBUG) > 0) {                          \
        printf("[D]<%s@%s:%d>: " fmt,                                    \
               clean_filename(__FILE__), __FUNCTION__, __LINE__, ##args  \
        );                                                               \
    }                                                                    \
} while (0)

#define PROFILER_IN() do {                                               \
    if ((profiler_level & PROFILE_FUNCS) > 0) {                          \
        memset(buf, ' ', profiler_depth);                                \
        buf[profiler_depth] = '\0';                                      \ 
        profiler_depth++;                                                \
        printf(buf);                                                     \
        printf(C_GRN "->%d <%s@%s>\n" C_NRM,                             \
               profiler_depth, __FUNCTION__, clean_filename(__FILE__));  \
    }                                                                    \
} while (0)

#define PROFILER_OUT() do {                                              \
    if ((profiler_level & PROFILE_FUNCS) > 0) {                          \
        profiler_depth--;                                                \
        memset(buf, ' ', profiler_depth);                                \
        buf[profiler_depth] = '\0';                                      \ 
        printf("%s", buf);                                                     \
        printf(C_MAG "<-%d <%s@%s>\n" C_NRM,                             \
               profiler_depth, __FUNCTION__, clean_filename(__FILE__)    \
        );                                                               \
    }                                                                    \
} while (0)

#define PROFILER_ROUT(ret, fmt) do {                                              \
    if ((profiler_level & PROFILE_FUNCS) > 0) {                          \
        profiler_depth--;                                                \
        memset(buf, ' ', profiler_depth);                                \
        buf[profiler_depth] = '\0';                                      \ 
        printf(buf);                                                     \
        printf(C_MAG "<-%d <%s@%s> " C_NRM,                              \
               profiler_depth, __FUNCTION__, clean_filename(__FILE__)    \
        );                                                               \
        snprintf(buf, BUF_LENGTH, "%s \0", fmt);    \ 
        printf("(" fmt ")\n", ret);   \
    }                                                                    \
} while (0)

#endif // SHELF_PROFILER_77828342
