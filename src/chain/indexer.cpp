#include "chain/indexer.hpp"

#include "chain/script.hpp"

BlockEffects effects_from_block(const Block& block, std::uint32_t height) {
    BlockEffects fx;
    fx.height = height;
    fx.time = block.time;
    fx.hash = block_hash(block);
    for (const Transaction& tx : block.txs) {
        TxEffects te;
        const Hash256 id = txid(tx);
        for (const TxIn& in : tx.vin) {
            if (is_null_prevout(in)) {
                continue;
            }
            OutpointRef sp;
            sp.txid = in.prev_txid;
            sp.vout = in.prev_vout;
            te.spends.push_back(sp);
        }
        for (std::uint32_t i = 0; i < tx.vout.size(); ++i) {
            const TxOut& out = tx.vout[i];
            if (out.value <= 0) {
                continue;
            }
            const auto addr = address_from_script(out.script_pubkey);
            if (!addr) {
                continue;
            }
            Credit c;
            c.txid = id;
            c.vout = i;
            c.address = *addr;
            c.amount = static_cast<std::uint64_t>(out.value);
            te.credits.push_back(std::move(c));
        }
        fx.txs.push_back(std::move(te));
    }
    return fx;
}
