#pragma once

#include <secp256k1.h>

class SecpContext {
public:
    SecpContext();
    ~SecpContext();

    SecpContext(const SecpContext&) = delete;
    SecpContext& operator=(const SecpContext&) = delete;

    secp256k1_context* get() const { return ctx_; }

    static SecpContext& instance();

private:
    secp256k1_context* ctx_;
};
