CC := gcc
MPICC := mpicc
CC_FLAGS := -O3 -g -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

SRCDIR=src
BINDIR=bin

cfiles=$(wildcard $(SRCDIR)/*.c)
executables=$(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(cfiles))

.PHONY: all clean

all: $(executables) 

$(BINDIR)/%: $(SRCDIR)/*.c
	mkdir -p $(BINDIR)
	$(CC) -c $< -o $@ $(CC_FLAGS)

clean:
	$(RM) -r -f $(BINDIR)

