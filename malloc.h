#ifndef MALLOC_H
#define MALLOC_H 1
#include <stdint.h>

#define MAX_CALLCHAIN 20
#define MAX_LOC_LEN   512
struct malloc_event {
   int ev_size;
   uint64_t rdt;
   uint64_t addr;
   int tid;
   int pid;
   int cpu;
   int alloc_size;
   int type;
   uint64_t alloc_ip;
   int callchain_size;
   uint64_t callchain[];
};

#endif
