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

const char kWtxidPre[] =
    "02000000000101000000000000000000000000000000000000000000000000000000000000"
    "00000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3"
    "a323f1433bd602483000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b"
    "16f8179800000000";

std::string tmpdir() {
    char tmpl[] = "/tmp/btk-bal-XXXXXX";
    char* p = mkdtemp(tmpl);
    CHECK(p != nullptr);
    return p;
}

}  // namespace

int main() {
#ifndef BTK_NO_LEVELDB
    const std::string dir = tmpdir();
    CHECK(inspect_index(dir) == IndexState::Empty);

    auto db = BalanceDb::open(dir, true);
    const auto full = hex_decode(
        "0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000");
    CHECK(full.size() == 192);
    const Transaction tx = parse_tx(full);
    Block block;
    block.version = 1;
    block.txs.push_back(tx);
    const BlockEffects fx = effects_from_block(block, 0);
    CHECK(fx.credits.size() == 1);
    CHECK(fx.credits[0].address == "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
    CHECK(fx.credits[0].amount == 100000);
    CHECK(fx.spends.empty());
    db->apply(fx);
    CHECK(db->get_sats("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4") == 100000);
    CHECK(db->get_sats("1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH") == 0);
    CHECK(db->has_metadata());
    CHECK(db->height() == 0);
    db.reset();
    CHECK(inspect_index(dir) == IndexState::Valid);

    destroy_index(dir);
    CHECK(inspect_index(dir) == IndexState::Missing);
#else
    CHECK(!kHaveLeveldb);
#endif
    (void)kWtxidPre;
    return 0;
}
