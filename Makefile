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

EXES = test

all: $(EXES)

test: $(OBJ)/bitcoin.o $(OBJ)/privkey.o $(OBJ)/pubkey.o $(OBJ)/base58check.o $(OBJ)/crypto.o $(OBJ)/address.o $(OBJ)/wif.o $(OBJ)/random.o $(OBJ)/point.o $(OBJ)/base58.o $(OBJ)/test.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

$(OBJ)/bitcoin.o: $(SRC)/modules/bitcoin.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/privkey.o: $(SRC)/modules/bitcoin/privkey.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/pubkey.o: $(SRC)/modules/bitcoin/pubkey.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/base58check.o: $(SRC)/modules/bitcoin/base58check.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/crypto.o: $(SRC)/modules/bitcoin/crypto.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/address.o: $(SRC)/modules/bitcoin/address.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/wif.o: $(SRC)/modules/bitcoin/wif.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/random.o: $(SRC)/modules/bitcoin/random.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/point.o: $(SRC)/modules/bitcoin/point.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/base58.o: $(SRC)/modules/bitcoin/base58check/base58.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

install:
#	install -d $(DESTDIR)$(bindir)
#	cd $(BIN) && install $(EXES) $(DESTDIR)$(bindir)

uninstall:
#	for exe in $(EXES); do rm $(DESTDIR)$(bindir)/$$exe; done
#	rm -rf $(DESTDIR)$(datadir)/wordv
