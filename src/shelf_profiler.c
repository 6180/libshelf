#include "../include/shelf_profiler.h"


int profiler_depth = 0;
int profiler_level = 0;


void set_profiler_level(int level) {
    profiler_level = level;
}

int get_profiler_level() {
    return profiler_level;
}
