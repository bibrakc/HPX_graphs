# Makefile for src-parallel

SHELL = /bin/bash


### Set your desired C compiler and any necessary flags.  Note that CoMD
### uses some c99 features.  You can also set flags for optimization and
### specify paths to include files that the compiler can't find on its
### own.  If you need any -L or -l switches to get C standard libraries
### (such as -lm for the math library) put them in C_LIB.
CC = cc
CFLAGS = --std=gnu99
#CFLAGS = --std=c99
OPTFLAGS = -g -O2
INCLUDES = 
C_LIB = -lm
C_LIB  += $(shell pkg-config --libs --static hpx)
CFLAGS += $(shell pkg-config --cflags hpx)


### A place to specify any other include or library switches your
### platform requires.
OTHER_LIB = 
OTHER_INCLUDE =




#########################################
### Below here, it is pitch black.  
### You are likely to be eaten by a grue.
##########################################

# clear all suffixes
.SUFFIXES:
# list only those that we use 
.SUFFIXES: .c .o

.PHONY: DEFAULT clean distclean depend

BIN_DIR=./bin


# Set executable name and add includes & libraries for MPI if needed.
bfs_VARIANT = sssp-hpx

bfs_EXE = ${BIN_DIR}/${bfs_VARIANT}

LDFLAGS += ${C_LIB} ${OTHER_LIB}
CFLAGS  += ${OPTFLAGS} ${INCLUDES} ${OTHER_INCLUDE}


SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)


DEFAULT: ${bfs_EXE}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

${bfs_EXE}: ${BIN_DIR}  ${OBJECTS} 
	${CC} ${CFLAGS} -o ${bfs_EXE} ${OBJECTS} ${LDFLAGS}


${BIN_DIR}:
	@if [ ! -d ${BIN_DIR} ]; then mkdir -p ${BIN_DIR} ; fi

clean:
	rm -f *.o 

distclean: clean
	rm -f ${bfs_EXE} 
	rm -rf html latex
