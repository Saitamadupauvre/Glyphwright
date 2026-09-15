#include "JsonValue.hpp"
#include <cctype>
#include <charconv>

namespace gw::detail {

namespace {

class JsonParser {
public:
    explicit JsonParser(std::string_view text) : _text(text) {}

    bool parse(JsonValue& out) {
        skipWs();
        if (!parseValue(out)) return false;
        skipWs();
        return _pos == _text.size();
    }

private:
    std::string_view _text;
    size_t _pos = 0;

    bool atEnd() const { return _pos >= _text.size(); }
    char peek() const { return _text[_pos]; }

    void skipWs() {
        while (!atEnd() && std::isspace(static_cast<unsigned char>(peek()))) ++_pos;
    }

    bool consume(char c) {
        if (atEnd() || peek() != c) return false;
        ++_pos;
        return true;
    }

    bool parseValue(JsonValue& out) {
        skipWs();
        if (atEnd()) return false;
        char c = peek();
        if (c == '{') return parseObject(out);
        if (c == '[') return parseArray(out);
        if (c == '"') {
            std::string s;
            if (!parseString(s)) return false;
            out.data = std::move(s);
            return true;
        }
        if (c == 't' || c == 'f') return parseBool(out);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(out);
        return false;
    }

    bool parseObject(JsonValue& out) {
        if (!consume('{')) return false;
        JsonObject obj;
        skipWs();
        if (consume('}')) {
            out.data = std::move(obj);
            return true;
        }
        while (true) {
            skipWs();
            std::string key;
            if (!parseString(key)) return false;
            skipWs();
            if (!consume(':')) return false;
            JsonValue val;
            if (!parseValue(val)) return false;
            obj.emplace_back(std::move(key), std::move(val));
            skipWs();
            if (consume(',')) continue;
            if (consume('}')) break;
            return false;
        }
        out.data = std::move(obj);
        return true;
    }

    bool parseArray(JsonValue& out) {
        if (!consume('[')) return false;
        JsonArray arr;
        skipWs();
        if (consume(']')) {
            out.data = std::move(arr);
            return true;
        }
        while (true) {
            JsonValue val;
            if (!parseValue(val)) return false;
            arr.push_back(std::move(val));
            skipWs();
            if (consume(',')) continue;
            if (consume(']')) break;
            return false;
        }
        out.data = std::move(arr);
        return true;
    }

    bool parseString(std::string& out) {
        if (!consume('"')) return false;
        out.clear();
        while (!atEnd() && peek() != '"') {
            char c = peek();
            if (c == '\\') {
                ++_pos;
                if (atEnd()) return false;
                char esc = peek();
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case 'n': out += '\n'; break;
                    default: out += esc; break;
                }
                ++_pos;
            } else {
                out += c;
                ++_pos;
            }
        }
        return consume('"');
    }

    bool parseBool(JsonValue& out) {
        if (_text.compare(_pos, 4, "true") == 0) {
            _pos += 4;
            out.data = true;
            return true;
        }
        if (_text.compare(_pos, 5, "false") == 0) {
            _pos += 5;
            out.data = false;
            return true;
        }
        return false;
    }

    bool parseNumber(JsonValue& out) {
        size_t start = _pos;
        if (!atEnd() && peek() == '-') ++_pos;
        while (!atEnd() && (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.' ||
                             peek() == 'e' || peek() == 'E' || peek() == '+' || peek() == '-')) {
            ++_pos;
        }
        if (_pos == start) return false;
        std::string_view sub = _text.substr(start, _pos - start);
        double v = 0.0;
        auto res = std::from_chars(sub.data(), sub.data() + sub.size(), v);
        if (res.ec != std::errc()) return false;
        out.data = v;
        return true;
    }
};

} // namespace

const JsonObject* asObject(const JsonValue& v) { return std::get_if<JsonObject>(&v.data); }
const JsonArray* asArray(const JsonValue& v) { return std::get_if<JsonArray>(&v.data); }
const std::string* asString(const JsonValue& v) { return std::get_if<std::string>(&v.data); }
const double* asNumber(const JsonValue& v) { return std::get_if<double>(&v.data); }
const bool* asBool(const JsonValue& v) { return std::get_if<bool>(&v.data); }

const JsonValue* findMember(const JsonObject& obj, std::string_view key) {
    for (const auto& [k, v] : obj) {
        if (k == key) return &v;
    }
    return nullptr;
}

bool parseJson(std::string_view text, JsonValue& out) {
    JsonParser parser(text);
    return parser.parse(out);
}

} // namespace gw::detail
