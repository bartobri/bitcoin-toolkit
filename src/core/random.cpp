#include "core/random.hpp"

#include "util/error.hpp"

#include <cstdio>

#if defined(__has_include)
#if __has_include(<sys/random.h>)
#include <sys/random.h>
#define BTK_HAVE_GETENTROPY 1
#endif
#endif

#ifndef BTK_HAVE_GETENTROPY
#if defined(__linux__) || defined(__APPLE__)
#include <sys/random.h>
#define BTK_HAVE_GETENTROPY 1
#endif
#endif

namespace {

class File {
public:
    File(const char* path, const char* mode) : f_(std::fopen(path, mode)) {}
    ~File() {
        if (f_ != nullptr) {
            std::fclose(f_);
        }
    }
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    FILE* get() const { return f_; }

private:
    FILE* f_;
};

}  // namespace

void random_bytes(std::uint8_t* buf, std::size_t n) {
#ifdef BTK_HAVE_GETENTROPY
    // getentropy is documented for n <= 256.
    if (n <= 256 && getentropy(buf, n) == 0) {
        return;
    }
#endif
    File f("/dev/urandom", "rb");
    if (f.get() == nullptr) {
        throw BtkError("privkey", "could not read CSPRNG");
    }
    const std::size_t got = std::fread(buf, 1, n, f.get());
    if (got != n) {
        throw BtkError("privkey", "could not read CSPRNG");
    }
}
