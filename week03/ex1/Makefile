CC=gcc
MPICC=mpicc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

.PHONY: all
all: heat_stencil_2D_serial heat_stencil_2D_mpi heat_stencil_2D_parallel heat_stencil_2D_parallel_nb


.PHONY: clean
clean:
	$(RM) heat_stencil_2D_serial heat_stencil_2D_parallel heat_stencil_2D_mpi heat_stencil_2D_parallel_nb
	
heat_stencil_2D_serial: heat_stencil_2D_serial.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_serial

heat_stencil_2D_mpi: heat_stencil_2D_mpi.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_mpi

heat_stencil_2D_parallel: heat_stencil_2D_parallel.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_parallel

heat_stencil_2D_parallel_nb: heat_stencil_2D_parallel_nb.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_parallel_nb