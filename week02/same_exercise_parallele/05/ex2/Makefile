CFLAGS= -std=gnu11 -lm -Wall -Wextra -O3 -pedantic -g3 -fopenmp
LFLAGS = -pthread

.PHONY: all
all: heat_stencil_2D heat_stencil_2D_serial


.PHONY: clean
clean:
	$(RM) heat_stencil_2D
	$(RM) heat_stencil_2D_serial
	
heat_stencil_2D: heat_stencil_2D.c
	gcc $(CFLAGS) $^ -o heat_stencil_2D $(LFLAGS)

heat_stencil_2D_serial: heat_stencil_2D_serial.c
	gcc $(CFLAGS) $^ -o heat_stencil_2D_serial $(LFLAGS)
