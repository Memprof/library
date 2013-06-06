MemProf Library
===============

The Memprof Library. Used to track malloc and free.


WARNING: To compile the library, you must have a copy of the kernel module in ../module and a copy of the parser in ../parser. See https://github.com/Memprof to get the module and the parser.

```bash
make 
sudo insmod ../module/memprof.ko 
echo b > /proc/memprof_cntl 
LD_PRELOAD=./ldlib.so <app> 
echo e > /proc/memprof_cntl 
cat /proc/memprof_ibs > ibs.raw 
cat /proc/memprof_perf > perf.raw 
./merge /tmp/data.* 
../parser/parse ibs.raw --data data.processed.raw --perf perf.raw [options, e.g. -M] 
```

Notes
=====
* The library will write text files in /tmp at the end of the run. Make sure that you have the rights to write in /tmp.

* By default, the library uses backtrace() to collect callchains. If your application is configured to use frame pointers (i.e., compiled with -fno-omit-frame-pointers), then you can enable frame pointers; it will speed up the data collection (see ldlib.c).
