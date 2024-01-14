MPICC = mpicc
CFLAGS = -O3 -g -std=gnu11 -Wall -Wextra -pedantic -lm -ffast-math

SRCDIR = src
BINDIR = bin

SOURCES := $(wildcard $(SRCDIR)/*.c)
EXECS := $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(SOURCES))

all: $(EXECS)

$(BINDIR)/%: $(SRCDIR)/%.c
	mkdir -p $(BINDIR)
	$(MPICC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECS)

cleanoutput:
	rm -f /scratch/$(USER)/output*

.PHONY: all clean