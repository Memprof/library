#include "merge.h"
#include "rbtree.h"
#include "symbol.h"

char *find_ip(char *line, uint64_t *ip) {
   char *l = line;
cont:
   while(*l && *l != '[')
      l++;
   /** This is an ugly hack for postgres: postgres has the good idea to name its processes
    * 'postgres: postgres test [local] authentication'. The [local] should be skipped.
    * Note that find_func does not work on such processes but I don't care (IP is resolved by MemProf).
    */
   if(!(*l) || !*(l+1))
      return NULL;
   if(*(l+1)!='0') { //0x
      l++;
      goto cont;
   }
   l+= 3;

   l = read_hex(l, ip);
   
   return ++l;
}

//unsafe because of find_lib
void get_loc(char *line, uint64_t *ip) {
   *ip = 0;
   line = find_ip(line, ip);
   return;
}

