#include "core/secp.hpp"

#include "util/error.hpp"

#ifndef SECP256K1_CONTEXT_NONE
#define BTK_SECP_FLAGS (SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY)
#else
#define BTK_SECP_FLAGS SECP256K1_CONTEXT_NONE
#endif

SecpContext::SecpContext() : ctx_(secp256k1_context_create(BTK_SECP_FLAGS)) {
    if (ctx_ == nullptr) {
        throw BtkError("", "could not create secp256k1 context");
    }
}

SecpContext::~SecpContext() {
    if (ctx_ != nullptr) {
        secp256k1_context_destroy(ctx_);
    }
}

SecpContext& SecpContext::instance() {
    static SecpContext ctx;
    return ctx;
}
