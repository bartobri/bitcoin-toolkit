#pragma once

#include "cli/options.hpp"
#include "core/json_io.hpp"

class OutputWriter {
public:
    explicit OutputWriter(const Options& opts);
    ~OutputWriter();

    void write(const JsonObject& obj);
    void finish();

private:
    const Options& opts_;
    JsonArray buffered_;
    bool finished_ = false;
};

std::string primary_string(const JsonObject& obj);
JsonObject version_object();
