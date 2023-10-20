CC=gcc
MPICC=mpicc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic

.PHONY: all
all: heat_stencil_1D_mpi

heat_stencil_1D_mpi: heat_stencil_1D_mpi.c
	@$(MPICC) $(CC_FLAGS) $< -o heat_stencil_1D_mpi

.PHONY: clean
clean:
	@rm heat_stencil_1D_mpi

