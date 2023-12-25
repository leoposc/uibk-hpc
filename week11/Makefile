CHPLC = chpl
CHPL_FLAGS = --fast

SRC_DIR = src
BIN_DIR = bin

CHPL_SOURCES := $(wildcard $(SRC_DIR)/*.chpl)
CHPL_EXECUTABLES := $(patsubst $(SRC_DIR)/%.chpl,$(BIN_DIR)/%,$(CHPL_SOURCES))

all: $(CHPL_EXECUTABLES)

$(BIN_DIR)/%: $(SRC_DIR)/%.chpl
	$(CHPLC) $(CHPL_FLAGS) -o $@ $<

clean:
	rm -f $(CHPL_EXECUTABLES)

.PHONY: all clean
