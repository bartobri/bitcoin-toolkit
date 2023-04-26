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
CFLAGS ?= -Wextra -Wall -iquote$(SRC) -idirafter$(SRC)/missing
CLIBS ?= -lpthread

CTRL_OBJS = $(OBJ)/$(CTRL)/btk_help.o $(OBJ)/$(CTRL)/btk_privkey.o $(OBJ)/$(CTRL)/btk_pubkey.o $(OBJ)/$(CTRL)/btk_address.o $(OBJ)/$(CTRL)/btk_node.o $(OBJ)/$(CTRL)/btk_balance.o $(OBJ)/$(CTRL)/btk_config.o $(OBJ)/$(CTRL)/btk_version.o
MOD_OBJS = $(OBJ)/$(MODS)/network.o $(OBJ)/$(MODS)/database.o $(OBJ)/$(MODS)/chainstate.o $(OBJ)/$(MODS)/balance.o $(OBJ)/$(MODS)/txoa.o $(OBJ)/$(MODS)/node.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/address.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/base32.o $(OBJ)/$(MODS)/bech32.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/camount.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/utxokey.o $(OBJ)/$(MODS)/utxovalue.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/block.o $(OBJ)/$(MODS)/script.o $(OBJ)/$(MODS)/message.o $(OBJ)/$(MODS)/serialize.o $(OBJ)/$(MODS)/json.o $(OBJ)/$(MODS)/jsonrpc.o $(OBJ)/$(MODS)/qrcode.o $(OBJ)/$(MODS)/input.o $(OBJ)/$(MODS)/output.o $(OBJ)/$(MODS)/opts.o $(OBJ)/$(MODS)/config.o $(OBJ)/$(MODS)/error.o
COM_OBJS = $(OBJ)/$(MODS)/commands/verack.o $(OBJ)/$(MODS)/commands/version.o
JSON_OBJS = $(OBJ)/$(MODS)/cJSON/cJSON.o
QRCODE_OBJS = $(OBJ)/$(MODS)/QRCodeGen/qrcodegen.o
GMP_OBJS = $(OBJ)/$(MODS)/GMP/mini-gmp.o
CRYPTO_OBJS = $(OBJ)/$(MODS)/crypto/rmd160.o $(OBJ)/$(MODS)/crypto/sha256.o
LEVELDB_OBJS = $(OBJ)/$(MODS)/leveldb/stub.o

## Install libgmp-dev
ifeq ($(shell ld -lgmp -M -o /dev/null 2>/dev/null | grep -c -m 1 libgmp ), 1)
   CLIBS += -lgmp
   GMP_OBJS = $()
endif

## Install libssl-dev
ifeq ($(shell ld -lcrypto -M -o /dev/null 2>/dev/null | grep -c -m 1 libcrypto ), 1)
   CLIBS += -lcrypto
   CRYPTO_OBJS = $()
endif

## Install libleveldb-dev
ifeq ($(shell ld -lleveldb -M -o /dev/null 2>/dev/null | grep -c -m 1 libleveldb ), 1)
   CLIBS += -lleveldb
   LEVELDB_OBJS = $()
endif

.PHONY: all test install uninstall clean

EXES = btk

all: $(EXES)

btk: $(CTRL_OBJS) $(MOD_OBJS) $(COM_OBJS) $(JSON_OBJS) $(QRCODE_OBJS) $(GMP_OBJS) $(CRYPTO_OBJS) $(LEVELDB_OBJS) $(OBJ)/btk.o | $(BIN)
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

$(OBJ)/$(MODS)/leveldb/%.o: $(SRC)/$(MODS)/leveldb/%.c | $(OBJ)
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
	mkdir -p $(OBJ)/$(MODS)/leveldb
	mkdir -p $(OBJ)/$(CTRL)

test:
	python3 test/test.py

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

install:
	install -d $(DESTDIR)$(mandir)/man1
	install -m644 man/*.1 $(DESTDIR)$(mandir)/man1
	install -d $(DESTDIR)$(bindir)
	cd $(BIN) && install $(EXES) $(DESTDIR)$(bindir)

uninstall:
	rm -f $(DESTDIR)$(mandir)/man1/btk*
	for exe in $(EXES); do rm $(DESTDIR)$(bindir)/$$exe; done
