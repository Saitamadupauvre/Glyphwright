#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace gw::detail {

struct JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::vector<std::pair<std::string, JsonValue>>;

struct JsonValue {
    std::variant<std::monostate, double, bool, std::string, JsonArray, JsonObject> data;
};

const JsonObject* asObject(const JsonValue& v);
const JsonArray* asArray(const JsonValue& v);
const std::string* asString(const JsonValue& v);
const double* asNumber(const JsonValue& v);
const bool* asBool(const JsonValue& v);

const JsonValue* findMember(const JsonObject& obj, std::string_view key);

bool parseJson(std::string_view text, JsonValue& out);

} // namespace gw::detail
