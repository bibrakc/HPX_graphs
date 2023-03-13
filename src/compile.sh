#!/bin/bash
  
echo "Compiling sssp-hpx.c"

cc -std=gnu99 -D_POSIX_C_SOURCE=200809L -I/N/u/bchandio/BigRed200/installs/hpx/include \
  -I/N/u/bchandio/BigRed200/installs/hpx/lib/libffi-3.99999/include -g -O2 \
  -c sssp_hpx.c -o main.o -L/N/u/bchandio/BigRed200/installs/hpx/lib \
  -L/N/u/bchandio/BigRed200/installs/hpx/lib/../lib64 -lhpx -lrt -lstdc++ \
  -Wl,-rpath,/N/u/bchandio/BigRed200/installs/hpx/lib -pthread -lm -lcityhash \
  -Wl,-rpath,/N/u/bchandio/BigRed200/installs/hpx/lib -pthread -lm -lcityhash \
  -lurcu-qsbr -lurcu-cds -ljemalloc -lffi -Wl,-rpath -Wl,--enable-new-dtags \
  -lmpi -lpthread -lrt



cc  -D_POSIX_C_SOURCE=200809L -I/N/u/bchandio/BigRed200/installs/mpich/include \
    -I/N/u/bchandio/BigRed200/installs/hpx/include \
    -I/N/u/bchandio/BigRed200/installs/hpx/lib/libffi-3.99999/include \
    -o sssp_hpx.out main.o -L/N/u/bchandio/BigRed200/installs/hpx/lib \
    -L/N/u/bchandio/BigRed200/installs/hpx/lib/../lib64 -lhpx -lrt -lstdc++ \
    -Wl,-rpath,/N/u/bchandio/BigRed200/installs/hpx/lib -pthread -lm \
    -lcityhash -Wl,-rpath,/N/u/bchandio/BigRed200/installs/hpx/lib -pthread \
    -lm -lcityhash -lurcu-qsbr -lurcu-cds -ljemalloc -lffi -Wl,-rpath \
    -Wl,--enable-new-dtags -lmpi -lpthread -lrt


rm main.o