CFLAGS = -std=gnu11 -lm -Wall -Wextra -O3 -pedantic -g3 -fopenmp
LFLAGS = -pthread

.PHONY: all
all: delannoy_serial delannoy_parallel


.PHONY: clean
clean:
	$(RM) delannoy_serial
	$(RM) delannoy_parallel

	
delannoy_serial: delannoy_serial.c
	gcc $(CFLAGS) $^ -o delannoy_serial $(LFLAGS)

delannoy_parallel: delannoy_parallel.c
	gcc $(CFLAGS) $^ -o delannoy_parallel $(LFLAGS)
