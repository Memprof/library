#ifndef MERGE_H
#define MERGE_H 1

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>


#define ___unused __attribute__((unused))
#define die(msg, args...) \
   do {                         \
      fprintf(stderr,"(%s,%d) " msg "\n", __FUNCTION__ , __LINE__, ##args); \
      exit(-1);                 \
   } while(0)

#define MAX_BUFF_LEN 1024



static char *read_number(char *l, uint64_t *v, int base) {
   uint64_t val = 0;
   char *line = l;
   do {
      val = val * base;
      char c = *line;
      if(c >= '0' && c <= '9') {
         val += (c - '0');
      } else if(base == 16 && c >= 'a' && c <= 'f') {
         val += (c - 'a' + 10);
      } else if(c == ' ' || c == ']') {
         if(v) *v = val / base;
         return line;
      } else {
         //die("Invalid char '%c' in line %s\n", c, l);
         return NULL;
      }
      line++;
   } while(1);
   return NULL;
}
static ___unused char* read_hex(char *l, uint64_t *v) {
   return read_number(l, v, 16);
}
static ___unused char* read_dec(char *l, uint64_t *v) {
   return read_number(l, v, 10);
}

#endif
