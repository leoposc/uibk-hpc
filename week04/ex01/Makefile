MPICC=mpicc
GCC=gcc
CFLAGS=-O3 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

SRCDIR=src
BINDIR=bin

MPI_PROGRAMS=heat_stencil_1D_mpi_nb heat_stencil_1D_mpi heat_stencil_2D_mpi_nb heat_stencil_2D_mpi
SEQ_PROGRAMS=heat_stencil_1D_seq heat_stencil_2D_seq

all: $(MPI_PROGRAMS) $(SEQ_PROGRAMS)

$(BINDIR)/%: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BINDIR)/%: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $@ $<

heat_stencil_1D_mpi_nb: $(BINDIR)/heat_stencil_1D_mpi_nb
heat_stencil_1D_mpi: $(BINDIR)/heat_stencil_1D_mpi
heat_stencil_2D_mpi_nb: $(BINDIR)/heat_stencil_2D_mpi_nb
heat_stencil_2D_mpi: $(BINDIR)/heat_stencil_2D_mpi
heat_stencil_1D_seq: $(BINDIR)/heat_stencil_1D_seq
heat_stencil_2D_seq: $(BINDIR)/heat_stencil_2D_seq

clean:
	rm -rf $(BINDIR)
