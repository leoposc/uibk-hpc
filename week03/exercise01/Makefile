CC=gcc
MPICC=mpicc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic

.PHONY: all
all: heat_stencil_2D_seq heat_stencil_2D_mpi

heat_stencil_2D_seq: heat_stencil_2D_seq.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_seq

heat_stencil_2D_mpi: heat_stencil_2D_mpi.c
	@$(MPICC) $(CC_FLAGS) $< -o heat_stencil_2D_mpi

.PHONY: clean
clean:
	@rm heat_stencil_2D_seq
	@rm heat_stencil_2D_mpi

.PHONY: run
run: all
	@echo "Sequential:"
	@./heat_stencil_2D_seq
	@echo
