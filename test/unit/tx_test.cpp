#include "chain/compactsize.hpp"
#include "chain/script.hpp"
#include "chain/transaction.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"

#include <string>

namespace {

// Appendix A.10 — keep as one string so the length is obvious.
const char kTxidPre[] =
    "020000000100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd600000000";

const char kWtxidPre[] =
    "0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000";

}  // namespace

int main() {
    const auto full = hex_decode(kWtxidPre);
    CHECK(full.size() == 192);

    const Transaction tx = parse_tx(full);
    CHECK(tx.version == 2);
    CHECK(tx.has_witness);
    CHECK(tx.vin.size() == 1);
    CHECK(tx.vout.size() == 1);
    CHECK(tx.vin[0].witness.size() == 2);
    CHECK(tx.vin[0].witness[0].size() == 72);
    CHECK(tx.vin[0].witness[1].size() == 33);
    CHECK(tx.vout[0].value == 100000);
    CHECK(tx.locktime == 0);
    CHECK(is_null_prevout(tx.vin[0]));

    const auto nowit = serialize_tx(tx, false);
    CHECK(hex_encode(nowit) == kTxidPre);
    const auto wit = serialize_tx(tx, true);
    CHECK(hex_encode(wit) == kWtxidPre);

    CHECK(hex_encode(txid(tx)) == "3c58a2ad2dfc2f132e6dd137844f6e6bd749e33672f5e24d5109cea990c282a8");
    CHECK(hex_encode(wtxid(tx)) == "dd22da01b8929076ae782656fb7dfd1d3fe12a39c895b38aa533dbaa7d0806a1");
    CHECK(txid(tx) != wtxid(tx));

    const auto pre_hash = hash256(hex_decode(kTxidPre));
    const auto full_hash = hash256(full);
    CHECK(pre_hash == txid(tx));
    CHECK(full_hash == wtxid(tx));

    const auto addr = address_from_script(tx.vout[0].script_pubkey);
    CHECK(addr.has_value());
    CHECK(*addr == "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
    CHECK(is_mainnet_address(*addr));
    CHECK(is_mainnet_address("1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"));
    CHECK(!is_mainnet_address("tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx"));
    CHECK(!is_mainnet_address("not-an-address"));
    CHECK(classify_mainnet_address(*addr) == "p2wpkh");
    CHECK(classify_mainnet_address("1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH") == "p2pkh");
    CHECK(!classify_mainnet_address("tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx"));
    CHECK(!classify_mainnet_address("not-an-address"));

    // P2PKH of G compressed HASH160
    const auto p2pkh = hex_decode("76a914751e76e8199196d454941c45d1b3a323f1433bd688ac");
    CHECK(address_from_script(p2pkh) == "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH");

    // Historical P2PK compressed G → P2PKH of G
    const auto p2pk = hex_decode(
        "210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798ac");
    CHECK(address_from_script(p2pk) == "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH");

    // Untweaked P2TR of G x
    const auto p2tr = hex_decode(
        "512079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    CHECK(address_from_script(p2tr) ==
          "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0");
    CHECK(classify_mainnet_address(*address_from_script(p2tr)) == "p2tr");

    const auto p2sh = hex_decode("a914000000000000000000000000000000000000000087");
    const auto p2sh_addr = address_from_script(p2sh);
    CHECK(p2sh_addr.has_value());
    CHECK(classify_mainnet_address(*p2sh_addr) == "p2sh");

    const auto p2wsh = hex_decode(
        "00200000000000000000000000000000000000000000000000000000000000000000");
    const auto p2wsh_addr = address_from_script(p2wsh);
    CHECK(p2wsh_addr.has_value());
    CHECK(classify_mainnet_address(*p2wsh_addr) == "p2wsh");

    Block blk;
    blk.version = 1;
    std::vector<std::uint8_t> raw = serialize_block_header(blk);
    write_compact_size(raw, 1);
    raw.insert(raw.end(), full.begin(), full.end());
    const Block parsed = parse_block(raw);
    CHECK(parsed.txs.size() == 1);
    CHECK(parsed.txs[0].vin[0].witness.size() == 2);
    CHECK(hex_encode(txid(parsed.txs[0])) == hex_encode(txid(tx)));
    return 0;
}
