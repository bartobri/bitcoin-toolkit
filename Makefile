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
LIBS=libs

CC ?= gcc
CFLAGS ?= -Wextra -Wall -iquote$(SRC)
CLIBS ?= -lgmp -lgcrypt

LIB_OBJS = $(OBJ)/$(LIBS)/btk_help.o $(OBJ)/$(LIBS)/btk_privkey.o $(OBJ)/$(LIBS)/btk_pubkey.o $(OBJ)/$(LIBS)/btk_keypair.o
MOD_OBJS = $(OBJ)/$(MODS)/network.o $(OBJ)/$(MODS)/node.o $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/base32.o $(OBJ)/$(MODS)/bech32.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/$(MODS)/message.o $(OBJ)/$(MODS)/serialize.o
COM_OBJS = $(OBJ)/$(MODS)/commands/verack.o $(OBJ)/$(MODS)/commands/version.o

.PHONY: all install uninstall clean

#EXES = test asciiaddress btcaddress
EXES = btk

all: $(EXES)

btk: $(LIB_OBJS) $(MOD_OBJS) $(COM_OBJS) $(OBJ)/btk.o | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)


# test: $(OBJ)/$(MODS)/node.o $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/test.o | $(BIN)
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

# nodetest: $(OBJ)/$(MODS)/commands/verack.o $(OBJ)/$(MODS)/commands/version.o $(OBJ)/$(MODS)/message.o $(OBJ)/$(MODS)/serialize.o $(OBJ)/$(MODS)/node.o $(OBJ)/$(MODS)/hex.o  $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/nodetest.o | $(BIN)
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

# asciiaddress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/asciiaddress.o | $(BIN)
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

# btcaddress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/btcaddress.o | $(BIN)
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)

# uladdress: $(OBJ)/$(MODS)/mem.o $(OBJ)/$(MODS)/assert.o $(OBJ)/$(MODS)/except.o $(OBJ)/$(MODS)/privkey.o $(OBJ)/$(MODS)/pubkey.o $(OBJ)/$(MODS)/base58check.o $(OBJ)/$(MODS)/crypto.o $(OBJ)/$(MODS)/random.o $(OBJ)/$(MODS)/point.o $(OBJ)/$(MODS)/base58.o $(OBJ)/$(MODS)/hex.o $(OBJ)/$(MODS)/compactuint.o $(OBJ)/$(MODS)/txinput.o $(OBJ)/$(MODS)/txoutput.o $(OBJ)/$(MODS)/transaction.o $(OBJ)/$(MODS)/script.o $(OBJ)/uladdress.o | $(BIN)
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(CLIBS)


$(OBJ)/$(LIBS)/%.o: $(SRC)/$(LIBS)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/$(MODS)/commands/%.o: $(SRC)/$(MODS)/commands/%.c | $(OBJ)
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
	mkdir -p $(OBJ)/$(LIBS)

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

install:
#	install -d $(DESTDIR)$(bindir)
#	cd $(BIN) && install $(EXES) $(DESTDIR)$(bindir)
#	install -d $(DESTDIR)$(mandir)/man1
#	install -m644 man/btk-* $(DESTDIR)$(mandir)/man1

uninstall:
#	for exe in $(EXES); do rm $(DESTDIR)$(bindir)/$$exe; done
#	rm -f $(DESTDIR)$(mandir)/man1/btk-*
