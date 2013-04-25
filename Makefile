CFLAGS=-Wall -g -ggdb3 -O0 -I ../parser/utils -I ../parser -I ../module
LDFLAGS=-lnuma

OBJECTS = symbol.o merge.o

.PHONY: all clean
all: makefile.dep ldlib.so merge

makefile.dep: *.[Cch]
	for i in *.[Cc]; do gcc -MM "$${i}" ${CFLAGS}; done > $@
	
-include makefile.dep

ldlib.so: ldlib.c
	g++ -fPIC ${CFLAGS} -c ldlib.c
	g++ -shared -Wl,-soname,libpmalloc.so -o ldlib.so ldlib.o -ldl -lpthread

merge: ${OBJECTS} ../parser/utils/rbtree.o

../parser/utils/rbtree.o:
	make -C ../parser

clean:
	rm -f *.o *.so merge makefile.dep

