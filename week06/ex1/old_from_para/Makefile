CFLAGS= -std=gnu11 -lm -Wall -Wextra -O3 -pedantic -g3 -fopenmp 
LFLAGS = -pthread -lm

.PHONY: all
all: Nbody Nbody_par


.PHONY: clean
clean:
	$(RM) Nbody
	$(RM) Nbody_par

	
Nbody: Nbody.c
	gcc $(CFLAGS) $^ -o Nbody $(LFLAGS)

Nbody_par: Nbody_par.c
	gcc $(CFLAGS) $^ -o Nbody_par $(LFLAGS)

run:
	./Nbody 5000 100
	./Nbody_par 5000 100
	gnuplot particle.plt
