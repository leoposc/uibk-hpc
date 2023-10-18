CC=gcc
CC_FLAGS=-O2 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

.PHONY: all
all: heat_stencil_2D_serial


.PHONY: clean
clean:
	$(RM) heat_stencil_2D_serial
	
heat_stencil_2D_serial: heat_stencil_2D_serial.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_2D_serial