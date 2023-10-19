CC=gcc
MPICC=mpicc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

.PHONY: all
all: heat_stencil_2D_serial heat_stencil_2D_mpi heat_stencil_2D_parallel


.PHONY: clean
clean:
	$(RM) heat_stencil_2D_serial heat_stencil_2D_parallel
	
heat_stencil_2D_serial: heat_stencil_2D_serial.c
	@$(MPICC) $(CC_FLAGS) $< -o heat_stencil_2D_serial

heat_stencil_2D_mpi: heat_stencil_2D_mpi.c
	@$(MPICC) $(CC_FLAGS) $< -o heat_stencil_2D_mpi

heat_stencil_2D_parallel: heat_stencil_2D_parallel.c
	@$(MPICC) $(CC_FLAGS) $< -o heat_stencil_2D_parallel