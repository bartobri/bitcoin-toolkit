#include "chain/balance_db.hpp"
#include "chain/compactsize.hpp"
#include "chain/indexer.hpp"
#include "chain/transaction.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

const char kA10[] =
    "0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000";

const char kP2wpkhG[] = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4";
const char kP2pkhG[] = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH";

std::string tmpdir() {
    char tmpl[] = "/tmp/btk-bal-XXXXXX";
    char* p = mkdtemp(tmpl);
    CHECK(p != nullptr);
    return p;
}

Transaction spend_to(const Hash256& prev, const std::vector<std::uint8_t>& spk) {
    Transaction t;
    t.version = 1;
    TxIn in;
    in.prev_txid = prev;
    in.prev_vout = 0;
    in.sequence = 0xffffffff;
    t.vin.push_back(std::move(in));
    TxOut out;
    out.value = 100000;
    out.script_pubkey = spk;
    t.vout.push_back(std::move(out));
    return t;
}

Block block_of(std::vector<Transaction> txs) {
    Block b;
    b.version = 1;
    b.txs = std::move(txs);
    return b;
}

}  // namespace

int main() {
#ifndef BTK_NO_LEVELDB
    const auto full = hex_decode(kA10);
    CHECK(full.size() == 192);
    const Transaction a10 = parse_tx(full);
    const Hash256 a10_id = txid(a10);
    const auto p2pkh_spk = hex_decode("76a914751e76e8199196d454941c45d1b3a323f1433bd688ac");
    const auto p2wpkh_spk = hex_decode("0014751e76e8199196d454941c45d1b3a323f1433bd6");

    {
        const std::string dir = tmpdir();
        CHECK(inspect_index(dir) == IndexState::Empty);

        auto db = BalanceDb::open(dir, true);
        const BlockEffects fx = effects_from_block(block_of({a10}), 0);
        CHECK(fx.txs.size() == 1);
        CHECK(fx.txs[0].credits.size() == 1);
        CHECK(fx.txs[0].credits[0].address == kP2wpkhG);
        CHECK(fx.txs[0].credits[0].amount == 100000);
        CHECK(fx.txs[0].spends.empty());
        db->apply(fx);
        CHECK(db->get_sats(kP2wpkhG) == 100000);
        CHECK(db->get_sats(kP2pkhG) == 0);
        CHECK(db->has_metadata());
        CHECK(db->height() == 0);
        std::string addr;
        std::uint64_t amount = 0;
        CHECK(db->get_outpoint(a10_id, 0, addr, amount));
        CHECK(addr == kP2wpkhG);
        CHECK(amount == 100000);
        db.reset();
        CHECK(inspect_index(dir) == IndexState::Valid);

        destroy_index(dir, "balance");
        CHECK(inspect_index(dir) == IndexState::Missing);
    }

    {
        const std::string dir = tmpdir();
        auto db = BalanceDb::open(dir, true);
        db->apply(effects_from_block(block_of({a10}), 0));
        db->apply(effects_from_block(block_of({spend_to(a10_id, p2pkh_spk)}), 1));
        CHECK(db->get_sats(kP2wpkhG) == 0);
        CHECK(db->get_sats(kP2pkhG) == 100000);
        std::string addr;
        std::uint64_t amount = 0;
        CHECK(!db->get_outpoint(a10_id, 0, addr, amount));
        db.reset();
        destroy_index(dir, "balance");
    }

    {
        const std::string dir = tmpdir();
        auto db = BalanceDb::open(dir, true);
        const BlockEffects fx =
            effects_from_block(block_of({a10, spend_to(a10_id, p2pkh_spk)}), 0);
        CHECK(fx.txs.size() == 2);
        CHECK(fx.txs[0].credits.size() == 1);
        CHECK(fx.txs[1].spends.size() == 1);
        CHECK(fx.txs[1].credits.size() == 1);
        db->apply(fx);
        CHECK(db->get_sats(kP2wpkhG) == 0);
        CHECK(db->get_sats(kP2pkhG) == 100000);
        std::string addr;
        std::uint64_t amount = 0;
        CHECK(!db->get_outpoint(a10_id, 0, addr, amount));
        CHECK(db->get_outpoint(txid(spend_to(a10_id, p2pkh_spk)), 0, addr, amount));
        CHECK(addr == kP2pkhG);
        CHECK(amount == 100000);
        db.reset();
        destroy_index(dir, "balance");
    }

    {
        const std::string dir = tmpdir();
        auto db = BalanceDb::open(dir, true);
        db->apply(effects_from_block(block_of({a10, spend_to(a10_id, p2wpkh_spk)}), 0));
        CHECK(db->get_sats(kP2wpkhG) == 100000);
        CHECK(db->get_sats(kP2pkhG) == 0);
        db.reset();
        destroy_index(dir, "balance");
    }
#else
    CHECK(!kHaveLeveldb);
#endif
    return 0;
}
