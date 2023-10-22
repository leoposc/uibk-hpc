CC=gcc
MPICC=mpicc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic

COMMON_OBJ = common/common.o
SEQ_OBJ = heat_stencil_2D_seq.o $(COMMON_OBJ)
MPI_OBJ = heat_stencil_2D_mpi.o $(COMMON_OBJ)

SEQ = heat_stencil_2D_seq
MPI = heat_stencil_2D_mpi

.PHONY: all
all: $(SEQ) $(MPI)

.PHONY: clean
clean:
	rm -f $(COMMON_OBJ) $(SEQ_OBJ) $(MPI_OBJ) $(SEQ) $(MPI)

$(SEQ): $(SEQ_OBJ)
	@$(CC) $(CC_FLAGS) -o $@ $^

$(MPI): $(MPI_OBJ)
	@$(MPICC) $(CC_FLAGS) -o $@ $^

%.o: %.c common.h
	$(CC) $(CFLAGS) -c -o $@ $<