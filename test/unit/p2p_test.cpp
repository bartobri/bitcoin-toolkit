#include "core/hex.hpp"
#include "net/p2p.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

#include <cstdint>
#include <string>
#include <vector>

// Frozen version payload: timestamp 1700000000, nonce 0. Tests must
// not call time(NULL).
const char* kPayloadHex =
    "7f110100000000000000000000f1536500000000000000000000000000000000000000000000"
    "ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d00000000"
    "00000000172f426974636f696e2d546f6f6c6b69743a342e302e322f0000000000";

const char* kFullHex =
    "f9beb4d976657273696f6e00000000006d0000004c54afe3"
    "7f110100000000000000000000f1536500000000000000000000000000000000000000000000"
    "ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d00000000"
    "00000000172f426974636f696e2d546f6f6c6b69743a342e302e322f0000000000";

int main() {
    const std::vector<std::uint8_t> payload = hex_decode(kPayloadHex);
    CHECK(payload.size() == 109);

    const VersionPayload parsed = parse_version_payload(payload);
    CHECK(parsed.version == 70015);
    CHECK(parsed.services == 0);
    CHECK(parsed.timestamp == 1700000000);
    CHECK(parsed.addr_recv.services == 0);
    CHECK(parsed.addr_recv.port == 8333);
    CHECK(format_net_ip(parsed.addr_recv.ip) == "127.0.0.1");
    CHECK(parsed.addr_trans.port == 8333);
    CHECK(format_net_ip(parsed.addr_trans.ip) == "127.0.0.1");
    CHECK(parsed.nonce == 0);
    CHECK(parsed.user_agent == "/Bitcoin-Toolkit:4.0.2/");
    CHECK(parsed.start_height == 0);
    CHECK(parsed.relay == false);

    const std::vector<std::uint8_t> rebuilt = serialize_version_payload(parsed);
    CHECK(hex_encode(rebuilt) == std::string(kPayloadHex));

    const VersionPayload outbound = make_outbound_version(1700000000);
    CHECK(hex_encode(serialize_version_payload(outbound)) == std::string(kPayloadHex));

    const std::vector<std::uint8_t> full = hex_decode(kFullHex);
    CHECK(full.size() == 24 + 109);
    CHECK(full[16] == 0x6d && full[17] == 0x00 && full[18] == 0x00 && full[19] == 0x00);
    CHECK(full[20] == 0x4c && full[21] == 0x54 && full[22] == 0xaf && full[23] == 0xe3);
    CHECK(payload[44] == 0x20 && payload[45] == 0x8d);  // addr_recv.port BE 8333

    const P2pMessage msg = parse_p2p_message(full);
    CHECK(msg.command == "version");
    CHECK(hex_encode(msg.payload) == std::string(kPayloadHex));
    CHECK(hex_encode(serialize_p2p_message("version", payload)) == std::string(kFullHex));

    const auto names = service_names((1ull << 0) | (1ull << 3) | (1ull << 5) | (1ull << 10));
    CHECK(names.size() == 4);
    CHECK(names[0] == "NODE_NETWORK");
    CHECK(names[1] == "NODE_WITNESS");
    CHECK(names[2] == "BIT_5");
    CHECK(names[3] == "NODE_NETWORK_LIMITED");

    bool threw = false;
    try {
        parse_version_payload(payload.data(), 20);
    } catch (const BtkError&) {
        threw = true;
    }
    CHECK(threw);

    return 0;
}
