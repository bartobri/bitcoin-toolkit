# Installation directories following GNU conventions
prefix ?= /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
sbindir = $(exec_prefix)/sbin
datarootdir = $(prefix)/share
datadir = $(datarootdir)
includedir = $(prefix)/include
mandir = $(datarootdir)/man

BIN=bin
OBJ=obj
SRC=src
MODS=modules

CC ?= gcc
CFLAGS ?= -Wextra -Wall -iquote$(SRC)
CLIBS ?= -lgmp -lgcrypt

.PHONY: all install uninstall clean

EXES = bitcoin btcaddress asciiaddress

all: $(EXES)

bitcoin: $(OBJ)/$(MODS)/keypair.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/random.o $(OBJ)/main.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)
	
btcaddress: $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/random.o $(OBJ)/btcaddress.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

asciiaddress: $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/random.o $(OBJ)/asciiaddress.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/%.o: $(SRC)/$(MODS)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)/$(MODS)

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

install:
#	install -d $(DESTDIR)$(bindir)
#	cd $(BIN) && install $(EXES) $(DESTDIR)$(bindir)

uninstall:
#	for exe in $(EXES); do rm $(DESTDIR)$(bindir)/$$exe; done
#	rm -rf $(DESTDIR)$(datadir)/wordv
