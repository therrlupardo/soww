CC=gcc
NVCC=nvcc
MPICC=mpicc

CFLAGS=-O2
OMPFLAGS=-fopenmp
CUDAFLAGS=-arch=sm_35


LIBS=-lm
CUDALIBS=-lcudart
STDLIBS=-lstdc++
PTHREADSLIBS=-lpthread
OPENCLLIBS=-lOpenCL

PROGRAMS= cuda mpi openmp mpi+openmp

all: ${PROGRAMS}

openmp: openmp.c
	${CC} ${CFLAGS} ${OMPFLAGS} $< -o $@ ${LIBS}

run:
	make openmp
	export OMP_NUM_THREADS=8
	./openmp
clean:
	rm *.o ${PROGRAMS}