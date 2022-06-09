### Global variables ###
DEBUG = 0
CC ?= gcc

ifeq ($(DEBUG), 1)
    CFLAGS ?= -W -Wall -ansi -pedantic -g
else
    CFLAGS ?= -W -Wall -ansi -pedantic 
endif

CFLAGS += -I/usr/include/libxml2
LDFLAGS += -lxml2 -lz -lm -ldl

EXEC = imsi_dissector
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

### Rules to build target ###
all: $(EXEC)
ifeq ($(DEBUG), 1)
	@echo "===== Compilation in debug mode ====="
else
	@echo "===== Compilation in release mode ====="
endif

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

### Program cleaning ###
.PHONY: clean mrproper

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f $(EXEC)
