#include "merge.h"
#include "rbtree.h"
#include "symbol.h"

char *find_ip(char *line, uint64_t *ip) {
   char *l = line;
cont:
   while(*l && *l != '[')
      l++;
   if(!(*l) || !*(l+1))
      return NULL;
   /* 
    * This is an ugly hack for postgres: postgres has the good idea to name its processes
    * 'postgres: postgres test [local] authentication'. The [local] should be skipped.
    */
   if(*(l+1)!='0') { //0x
      l++;
      goto cont;
   }
   l+= 3;

   l = read_hex(l, ip);
   
   return ++l;
}

void get_loc(char *line, uint64_t *ip) {
   *ip = 0;
   line = find_ip(line, ip);
   return;
}

