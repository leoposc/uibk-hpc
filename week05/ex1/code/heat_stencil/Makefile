MPICC=mpicc
CFLAGS=-O3 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi -ffast-math -g -fsanitize=address,leak,undefined

SRCDIR=src
BINDIR=bin

MPI_PROGRAMS=heat_stencil_2D_mpi_nb

all: $(MPI_PROGRAMS) $(SEQ_PROGRAMS)

$(BINDIR)/%: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

heat_stencil_2D_mpi_nb: $(BINDIR)/heat_stencil_2D_mpi_nb

clean:
	rm -rf $(BINDIR)
