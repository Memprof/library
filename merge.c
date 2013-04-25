#include "merge.h"
#include "symbol.h"
#include "malloc.h"
#include "list.h"
#include "rbtree.h"

static struct stat {
   int nb_parsed_lines;
   int nb_failed_lines;
   int line_num;
} stats;
struct list *files_to_parse = NULL;
struct list *functions_to_ignore = NULL;
rbtree events;

void usage(int error) {
   fprintf(error?stderr:stdout, "Usage : ./parse files\n");
   exit(error);
}



void parse_options(int argc, char** argv) {
   if(argc < 2) 
      usage(-1);

   int c;
   for(c = 1; c < argc; c++) {
      if(!strcmp(argv[c], "--ignore")) {
         functions_to_ignore = list_add(functions_to_ignore, argv[c+1]);
         c++;
      } else 
         files_to_parse = list_add(files_to_parse, argv[c]);
   }

   events = rbtree_create();
}


/**
570231244602293 callchain
/home/b218/malloc_hook/ldlib.so(_Z11print_tracem+0x48) [0x7fda331a9166]
/home/b218/malloc_hook/ldlib.so(malloc+0x184) [0x7fda331a93c3]
  /usr/local/apache2/lib/libapr-1.so.0(apr_allocator_alloc+0x1d8) [0x7fda328cdd98]
   /usr/local/apache2/lib/libaprutil-1.so.0(apr_bucket_alloc_create_ex+0xe) [0x7fda32d0f78e]
    /usr/local/apache2/lib/libaprutil-1.so.0(apr_bucket_alloc_create+0x1e) [0x7fda32d0f80e]
     /usr/local/apache2/bin/httpd() [0x46ce82]
      /lib/libpthread.so.0(+0x68ba) [0x7fda3225f8ba]
       /lib/libc.so.6(clone+0x6d) [0x7fda31dc302d]
570231244602293 tid 110237 cpu 0 size 8192 addr 13c0bf0 type 1
**/

int is_callchain_start(char *line) {
   uint64_t time;
   char *after_dec = read_dec(line, &time);
   if(!after_dec)
      return 0;
   return !strcmp(after_dec, " callchain\n");
}

static int get_tid(char *file) {
   int tid = 0, steps = 1;
   char *end = file + strlen(file) - 1;
   while(*end >= '0' && *end <= '9') {
      tid += (*end - '0')*steps;
      steps *= 10;
      end--;
   }
   return tid;
}
void parse_file(char *file) {
   FILE *data = fopen(file, "r");
   if(!data)
      die("Cannot open file %s\n", file);
   
   int current_tid = get_tid(file);
   printf("#Parsing file %s tid %d\n", file, current_tid);

   char line[MAX_BUFF_LEN];
   int in_callchain = 0;
   struct malloc_event *e = calloc(1, sizeof(*e) + MAX_CALLCHAIN*sizeof(uint64_t));

   stats.line_num = 0;
   while(fgets(line, sizeof(line), data)) {
      stats.nb_parsed_lines++;
      stats.line_num++;

      if(is_callchain_start(line)) {
         if(in_callchain)
            die("File bugged: is_callchain_start true at %s:%d\n", file, stats.line_num);
         in_callchain = 1;
      } else if(in_callchain && (strstr(line, "ldlib.so") || strstr(line, "libstreamflow"))) {
         continue;
      } else if(sscanf(line, "%lu pid %d cpu %d size %d addr %lx type %d\n", 
               &e->rdt, &e->pid, &e->cpu, &e->alloc_size, &e->addr, &e->type) == 6) {
         in_callchain = 0;

         if(e->ev_size == 0) {
            /* If this happens, try to set PID_HACK to 1 in the malloc_hook/ldlib.c file */
            fprintf(stderr, "File bugged: unexpected of of callchain at %s:%d (ev_size=%d)\n", file, stats.line_num, e->ev_size);
            continue;
         }
         e->tid = current_tid;
         if(e->alloc_ip == 0) {
            fprintf(stderr, "File bugged: cannot find IP of allocation at %s:%d\n", file, stats.line_num);
            continue;
         }

         rbtree_insert(events, (void*)e->rdt, e, pointer_cmp); /* Stored in the wrong order to use a LIFO in script/parse */
         e = calloc(1, sizeof(*e) + MAX_CALLCHAIN*sizeof(uint64_t));
      } else if(in_callchain) {
         int ignore_line = 0;
         char *to_ignore;
         list_foreach(functions_to_ignore, to_ignore) {
            if(strstr(line, to_ignore)) {
               ignore_line = 1;
               break;
            }
         }
         if(ignore_line)
            continue;

         uint64_t ip;
         get_loc(line, &ip);
         if(in_callchain == 1) {
            in_callchain = 2;
            e->alloc_ip = ip;
            e->callchain_size = 0;
            e->ev_size = sizeof(*e);
         } else {
            e->callchain[e->callchain_size++] = ip;
            e->ev_size += sizeof(uint64_t);
         }
      } else {
         goto error;
      }
      continue;
error:
      stats.nb_failed_lines++;
   }

}

FILE *dump;
int print_tree(void *key, void *value) {
   struct malloc_event *e = (struct malloc_event *)value;
   
   int ret = fwrite(e, e->ev_size, 1, dump);
   if(ret != 1)
      die("fwrite fail: %s (%d)\n", strerror(ferror(dump)), ret);
   return 0;
}

int main(int argc, char** argv) {
   parse_options(argc, argv);

   char *file;
   list_foreach(files_to_parse, file) {
      parse_file(file);
   }

   dump = fopen("data.processed.raw", "w");
   if(!dump) 
      die("Cannot write to data.processed.raw: %s\n", strerror(errno));
   rbtree_print(events, print_tree);
   fclose(dump);
	return 0;
}
