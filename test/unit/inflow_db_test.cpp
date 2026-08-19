#include "chain/inflow_db.hpp"
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
    char tmpl[] = "/tmp/btk-inflow-XXXXXX";
    char* p = mkdtemp(tmpl);
    CHECK(p != nullptr);
    return p;
}

Transaction spend_to(const Hash256& prev, const std::vector<std::uint8_t>& spk,
                     std::int64_t value = 100000) {
    Transaction t;
    t.version = 1;
    TxIn in;
    in.prev_txid = prev;
    in.prev_vout = 0;
    in.sequence = 0xffffffff;
    t.vin.push_back(std::move(in));
    TxOut out;
    out.value = value;
    out.script_pubkey = spk;
    t.vout.push_back(std::move(out));
    return t;
}

Transaction two_outs(const std::vector<std::uint8_t>& spk, std::int64_t a, std::int64_t b) {
    Transaction t;
    t.version = 1;
    TxIn in;
    in.sequence = 0xffffffff;
    t.vin.push_back(std::move(in));
    TxOut o1;
    o1.value = a;
    o1.script_pubkey = spk;
    TxOut o2;
    o2.value = b;
    o2.script_pubkey = spk;
    t.vout.push_back(std::move(o1));
    t.vout.push_back(std::move(o2));
    return t;
}

Block block_of(std::vector<Transaction> txs, std::uint32_t time = 0) {
    Block b;
    b.version = 1;
    b.time = time;
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

        auto db = InflowDb::open(dir, true);
        const BlockEffects fx = effects_from_block(block_of({a10}, 1700000000), 0);
        CHECK(fx.time == 1700000000);
        CHECK(fx.txs.size() == 1);
        CHECK(fx.txs[0].credits.size() == 1);
        db->apply(fx);
        const InflowRow row = db->get_row(kP2wpkhG);
        CHECK(row.sats == 100000);
        CHECK(row.count == 1);
        CHECK(row.last == 1700000000);
        const InflowRow missing = db->get_row(kP2pkhG);
        CHECK(missing.sats == 0);
        CHECK(missing.count == 0);
        CHECK(missing.last == 0);
        CHECK(db->has_metadata());
        CHECK(db->height() == 0);
        db.reset();
        CHECK(inspect_index(dir) == IndexState::Valid);

        destroy_index(dir, "inflow");
        CHECK(inspect_index(dir) == IndexState::Missing);
    }

    {
        const std::string dir = tmpdir();
        auto db = InflowDb::open(dir, true);
        db->apply(effects_from_block(block_of({a10}, 100), 0));
        db->apply(effects_from_block(block_of({spend_to(a10_id, p2pkh_spk)}, 200), 1));
        const InflowRow a = db->get_row(kP2wpkhG);
        CHECK(a.sats == 100000);
        CHECK(a.count == 1);
        CHECK(a.last == 100);
        const InflowRow b = db->get_row(kP2pkhG);
        CHECK(b.sats == 100000);
        CHECK(b.count == 1);
        CHECK(b.last == 200);
        db.reset();
        destroy_index(dir, "inflow");
    }

    {
        const std::string dir = tmpdir();
        auto db = InflowDb::open(dir, true);
        db->apply(effects_from_block(block_of({two_outs(p2wpkh_spk, 100000, 200000)}, 50), 0));
        const InflowRow row = db->get_row(kP2wpkhG);
        CHECK(row.sats == 300000);
        CHECK(row.count == 1);
        CHECK(row.last == 50);
        db.reset();
        destroy_index(dir, "inflow");
    }

    {
        const std::string dir = tmpdir();
        auto db = InflowDb::open(dir, true);
        db->apply(effects_from_block(block_of({a10, spend_to(a10_id, p2wpkh_spk, 50000)}, 9), 0));
        const InflowRow row = db->get_row(kP2wpkhG);
        CHECK(row.sats == 150000);
        CHECK(row.count == 2);
        CHECK(row.last == 9);
        db.reset();
        destroy_index(dir, "inflow");
    }

    {
        const std::string dir = tmpdir();
        auto db = InflowDb::open(dir, true);
        db->apply(effects_from_block(block_of({a10}, 300), 0));
        db->apply(effects_from_block(block_of({spend_to(Hash256{}, p2wpkh_spk, 1)}, 200), 1));
        const InflowRow row = db->get_row(kP2wpkhG);
        CHECK(row.sats == 100001);
        CHECK(row.count == 2);
        CHECK(row.last == 300);
        db.reset();
        destroy_index(dir, "inflow");
    }
#else
    CHECK(!kHaveLeveldb);
#endif
    return 0;
}
