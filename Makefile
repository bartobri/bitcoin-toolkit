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
MODS=mods

CC ?= gcc
CFLAGS ?= -Wextra -Wall -iquote$(SRC)
CLIBS ?= -lgmp -lgcrypt

.PHONY: all install uninstall clean

#EXES = test asciiaddress btcaddress
EXES = test

all: $(EXES)

test: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/wif.o $(OBJ)///$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/test.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

asciiaddress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/wif.o $(OBJ)///$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/asciiaddress.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

btcaddress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/wif.o $(OBJ)///$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/btcaddress.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

uladdress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/wif.o $(OBJ)///$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/uladdress.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

$(OBJ)/$(MODS)/%.o: $(SRC)/$(MODS)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)
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
