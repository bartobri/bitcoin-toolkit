#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

#include "picojson/picojson.h"

using JsonValue = picojson::value;
using JsonObject = picojson::object;
using JsonArray = picojson::array;

constexpr const char* kBareField = "_bare";

JsonObject make_bare(const std::string& text);
bool is_bare(const JsonObject& obj);
std::string bare_text(const JsonObject& obj);

JsonValue parse_json_value(const std::string& text, const std::string& command = "");
JsonObject parse_json_object(const std::string& text, const std::string& command = "");

// Extract one complete JSON value from an input stream (object/array/atom).
// Skips leading whitespace. Returns false on EOF before any value.
bool extract_json_value(std::istream& in, std::string& text);

std::string json_min(const JsonValue& v);
std::string json_pretty(const JsonValue& v);

void set_string(JsonObject& o, const char* key, const std::string& value);
void set_bool(JsonObject& o, const char* key, bool value);
void set_uint64(JsonObject& o, const char* key, std::uint64_t value);
