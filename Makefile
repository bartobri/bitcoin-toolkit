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
CTRL=ctrl_mods

CC ?= gcc
CFLAGS ?= -Wextra -Wall -iquote$(SRC)
CLIBS ?= -lleveldb

CTRL_OBJS = $(OBJ)/$(CTRL)/btk_help.o $(OBJ)/$(CTRL)/btk_privkey.o $(OBJ)/$(CTRL)/btk_pubkey.o $(OBJ)/$(CTRL)/btk_address.o $(OBJ)/$(CTRL)/btk_node.o $(OBJ)/$(CTRL)/btk_utxodb.o $(OBJ)/$(CTRL)/btk_addressdb.o $(OBJ)/$(CTRL)/btk_version.o
MOD_OBJS = $(OBJ)/$(MODS)/network.o $(OBJ)/$(MODS)/database.o $(OBJ)/$(MODS)/utxodb.o $(OBJ)/$(MODS)/addressdb.o $(OBJ)/$(MODS)/node.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/base32.o $(OBJ)/$(MODS)/bech32.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/camount.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/$(MODS)/message.o $(OBJ)/$(MODS)/serialize.o $(OBJ)/$(MODS)/json.o $(OBJ)/$(MODS)/qrcode.o $(OBJ)/$(MODS)/input.o $(OBJ)/$(MODS)/output.o $(OBJ)/$(MODS)/opts.o $(OBJ)/$(MODS)/error.o
COM_OBJS = $(OBJ)/$(MODS)/commands/verack.o $(OBJ)/$(MODS)/commands/version.o
JSON_OBJS = $(OBJ)/$(MODS)/cJSON/cJSON.o
QRCODE_OBJS = $(OBJ)/$(MODS)/QRCodeGen/qrcodegen.o
GMP_OBJS = $(OBJ)/$(MODS)/GMP/mini-gmp.o
CRYPTO_OBJS = $(OBJ)/$(MODS)/crypto/rmd160.o $(OBJ)/$(MODS)/crypto/sha256.o

ifeq ($(USE_GMPLIB), 1)
CFLAGS += -D_USE_GMPLIB
CLIBS += -lgmp
undefine GMP_OBJS
endif

.PHONY: all test install uninstall clean

EXES = btk

all: $(EXES)

btk: $(CTRL_OBJS) $(MOD_OBJS) $(COM_OBJS) $(JSON_OBJS) $(QRCODE_OBJS) $(GMP_OBJS) $(CRYPTO_OBJS) $(OBJ)/btk.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

$(OBJ)/$(CTRL)/%.o: $(SRC)/$(CTRL)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/commands/%.o: $(SRC)/$(MODS)/commands/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/cJSON/%.o: $(SRC)/$(MODS)/cJSON/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/QRCodeGen/%.o: $(SRC)/$(MODS)/QRCodeGen/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/GMP/%.o: $(SRC)/$(MODS)/GMP/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/crypto/%.o: $(SRC)/$(MODS)/crypto/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/%.o: $(SRC)/$(MODS)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)
	mkdir -p $(OBJ)/$(MODS)
	mkdir -p $(OBJ)/$(MODS)/commands
	mkdir -p $(OBJ)/$(MODS)/cJSON
	mkdir -p $(OBJ)/$(MODS)/QRCodeGen
	mkdir -p $(OBJ)/$(MODS)/GMP
	mkdir -p $(OBJ)/$(MODS)/crypto
	mkdir -p $(OBJ)/$(CTRL)

test:
	perl test/test_template.pl

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

install:
	install -d $(DESTDIR)$(bindir)
	cd $(BIN) && install $(EXES) $(DESTDIR)$(bindir)

uninstall:
	for exe in $(EXES); do rm $(DESTDIR)$(bindir)/$$exe; done
