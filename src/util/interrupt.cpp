#include "util/interrupt.hpp"

#include <atomic>

namespace {
std::atomic<int> g_stop{0};
}

void request_stop() {
    g_stop.store(1, std::memory_order_relaxed);
}

bool stop_requested() {
    return g_stop.load(std::memory_order_relaxed) != 0;
}
