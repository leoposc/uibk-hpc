MPICC = mpicc
CFLAGS = -O3 -std=gnu11 -Wall -Wextra -pedantic -lm -ffast-math -g

SRCDIR = src
BINDIR = bin

SOURCES := $(wildcard $(SRCDIR)/*.c)
EXECS := $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(SOURCES))

all: $(EXECS)

$(BINDIR)/%: $(SRCDIR)/%.c
	$(MPICC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECS)

.PHONY: all clean
