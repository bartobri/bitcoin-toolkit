CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -I . -I src -I third_party
LIBS     ?= -lsecp256k1 -lpthread
prefix   ?= /usr/local

HAVE_SECP := $(shell printf 'int main(){return 0;}\n' | \
	$(CXX) -x c++ - $(CXXFLAGS) $(LIBS) -o /dev/null 2>/dev/null && echo 1)
ifeq ($(HAVE_SECP),)
$(error libsecp256k1 is required — sudo apt-get install libsecp256k1-dev (Debian/Ubuntu), sudo dnf install libsecp256k1-devel (Fedora), or brew install secp256k1 (macOS))
endif

HAVE_LEVELDB := $(shell printf 'int main(){return 0;}\n' | \
	$(CXX) -x c++ - -lleveldb -o /dev/null 2>/dev/null && echo 1)
ifeq ($(HAVE_LEVELDB),)
CPPFLAGS += -DBTK_NO_LEVELDB
else
LIBS += -lleveldb
endif

SRC := \
	src/main.cpp \
	src/cli/dispatcher.cpp \
	src/cli/options.cpp \
	src/cli/io.cpp \
	src/cli/output.cpp \
	src/core/hash.cpp \
	src/core/hex.cpp \
	src/core/base58.cpp \
	src/core/json_io.cpp \
	src/core/secp.cpp \
	src/core/random.cpp \
	src/core/privkey.cpp \
	src/core/pubkey.cpp \
	src/core/bech32.cpp \
	src/core/address.cpp \
	src/net/p2p.cpp \
	src/net/jsonrpc.cpp \
	src/chain/compactsize.cpp \
	src/chain/transaction.cpp \
	src/chain/script.cpp \
	src/chain/balance_db.cpp \
	src/chain/indexer.cpp \
	src/cmd/privkey.cpp \
	src/cmd/pubkey.cpp \
	src/cmd/address.cpp \
	src/cmd/node.cpp \
	src/cmd/balance.cpp \
	src/cmd/config.cpp \
	src/util/error.cpp \
	src/util/config.cpp \
	src/util/interrupt.cpp

OBJ := $(patsubst src/%.cpp,obj/%.o,$(SRC))

.PHONY: all test test-unit test-cli test-net install uninstall clean

all: bin/btk

bin/btk: $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

bin/test_hash: test/unit/hash_test.cpp src/core/hash.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_hex: test/unit/hex_test.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_base58: test/unit/base58_test.cpp src/core/base58.cpp src/core/hash.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_privkey: test/unit/privkey_test.cpp src/core/privkey.cpp src/core/base58.cpp src/core/hash.cpp src/core/hex.cpp src/core/secp.cpp src/core/random.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

bin/test_pubkey: test/unit/pubkey_test.cpp src/core/pubkey.cpp src/core/privkey.cpp src/core/base58.cpp src/core/hash.cpp src/core/hex.cpp src/core/secp.cpp src/core/random.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

bin/test_bech32: test/unit/bech32_test.cpp src/core/bech32.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_address: test/unit/address_test.cpp src/core/address.cpp src/core/bech32.cpp src/core/pubkey.cpp src/core/privkey.cpp src/core/base58.cpp src/core/hash.cpp src/core/hex.cpp src/core/secp.cpp src/core/random.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

bin/test_p2p: test/unit/p2p_test.cpp src/net/p2p.cpp src/core/hash.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_compactsize: test/unit/compactsize_test.cpp src/chain/compactsize.cpp src/core/hex.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_tx: test/unit/tx_test.cpp src/chain/compactsize.cpp src/chain/transaction.cpp src/chain/script.cpp src/core/hash.cpp src/core/hex.cpp src/core/base58.cpp src/core/bech32.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

bin/test_balance_db: test/unit/balance_db_test.cpp src/chain/balance_db.cpp src/chain/compactsize.cpp src/chain/transaction.cpp src/chain/script.cpp src/chain/indexer.cpp src/core/hash.cpp src/core/hex.cpp src/core/base58.cpp src/core/bech32.cpp src/util/error.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

test-unit: bin/test_hash bin/test_hex bin/test_base58 bin/test_privkey bin/test_pubkey bin/test_bech32 bin/test_address bin/test_p2p bin/test_compactsize bin/test_tx bin/test_balance_db
	bin/test_hash
	bin/test_hex
	bin/test_base58
	bin/test_privkey
	bin/test_pubkey
	bin/test_bech32
	bin/test_address
	bin/test_p2p
	bin/test_compactsize
	bin/test_tx
	bin/test_balance_db

test-cli: bin/btk
	python3 test/runner.py

test-net: bin/btk
	BTK_RUN_NET=1 python3 test/cli/test_node.py

test: test-unit test-cli

install: bin/btk
	install -d $(DESTDIR)$(prefix)/bin
	install -m 755 bin/btk $(DESTDIR)$(prefix)/bin/btk
	@if [ -d man ]; then \
		install -d $(DESTDIR)$(prefix)/share/man/man1; \
		install -m 644 man/btk*.1 $(DESTDIR)$(prefix)/share/man/man1/; \
	fi

uninstall:
	rm -f $(DESTDIR)$(prefix)/bin/btk
	rm -f $(DESTDIR)$(prefix)/share/man/man1/btk*.1

clean:
	rm -rf bin obj
