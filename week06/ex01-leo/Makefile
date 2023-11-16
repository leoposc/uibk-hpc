CC := gcc
CC_FLAGS := -O0 -g3 -std=gnu11 -Wall -Wextra -pedantic -lm -lmpi

SRCDIR=src
BINDIR=bin

cfiles=$(wildcard $(SRCDIR)/*.c)
executables=$(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(cfiles))

.PHONY: all clean

all: $(executables) 

$(BINDIR)/%: $(SRCDIR)/%.c
	mkdir -p $(BINDIR)
	$(CC) $< -o $@ $(CC_FLAGS)
	

clean:
	$(RM) -r -f $(BINDIR)