CC=gcc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

.PHONY: all
all: pi_seq pi_mpi heat_stencil_1D_seq heat_stencil_1D_mpi

pi_seq: pi_seq.c
	@$(CC) $(CC_FLAGS) $< -o pi_seq

pi_mpi: pi_mpi.c
	@$(CC) $(CC_FLAGS) $< -o pi_mpi

heat_stencil_1D_seq: heat_stencil_1D_seq.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_1D_seq

heat_stencil_1D_mpi: heat_stencil_1D_mpi.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_1D_mpi

.PHONY: clean
clean:
	@rm pi_seq pi_mpi heat_stencil_1D_seq heat_stencil_1D_mpi

	