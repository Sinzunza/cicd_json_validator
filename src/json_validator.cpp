#include "json_validator.hpp"

#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

namespace json_validator {
namespace {

class Parser {
public:
    explicit Parser(std::string_view text) : text_(text) {}

    ValidationResult parse() {
        skip_whitespace();
        if (!parse_value()) {
            return failure();
        }

        skip_whitespace();
        if (position_ != text_.size()) {
            set_error("Unexpected characters after the JSON value");
            return failure();
        }

        return {true, ""};
    }

private:
    bool parse_value() {
        if (position_ >= text_.size()) {
            set_error("Expected a JSON value");
            return false;
        }

        switch (text_[position_]) {
            case '{':
                return parse_object();
            case '[':
                return parse_array();
            case '"':
                return parse_string();
            case 't':
                return parse_literal("true");
            case 'f':
                return parse_literal("false");
            case 'n':
                return parse_literal("null");
            default:
                if (text_[position_] == '-' ||
                    std::isdigit(static_cast<unsigned char>(text_[position_]))) {
                    return parse_number();
                }
                set_error("Expected a JSON value");
                return false;
        }
    }

    bool parse_object() {
        ++position_;
        skip_whitespace();

        if (consume('}')) {
            return true;
        }

        while (true) {
            if (!parse_string()) {
                if (error_.empty()) {
                    set_error("Expected a string property name");
                }
                return false;
            }

            skip_whitespace();
            if (!consume(':')) {
                set_error("Expected ':' after property name");
                return false;
            }

            skip_whitespace();
            if (!parse_value()) {
                return false;
            }

            skip_whitespace();
            if (consume('}')) {
                return true;
            }
            if (!consume(',')) {
                set_error("Expected ',' or '}' in object");
                return false;
            }
            skip_whitespace();
        }
    }

    bool parse_array() {
        ++position_;
        skip_whitespace();

        if (consume(']')) {
            return true;
        }

        while (true) {
            if (!parse_value()) {
                return false;
            }

            skip_whitespace();
            if (consume(']')) {
                return true;
            }
            if (!consume(',')) {
                set_error("Expected ',' or ']' in array");
                return false;
            }
            skip_whitespace();
        }
    }

    bool parse_string() {
        if (!consume('"')) {
            return false;
        }

        while (position_ < text_.size()) {
            const unsigned char character =
                static_cast<unsigned char>(text_[position_++]);

            if (character == '"') {
                return true;
            }
            if (character < 0x20) {
                set_error("Unescaped control character in string");
                return false;
            }
            if (character != '\\') {
                continue;
            }

            if (position_ >= text_.size()) {
                set_error("Incomplete escape sequence in string");
                return false;
            }

            const char escape = text_[position_++];
            if (escape == 'u') {
                if (!parse_unicode_escape()) {
                    return false;
                }
            } else if (escape != '"' && escape != '\\' && escape != '/' &&
                       escape != 'b' && escape != 'f' && escape != 'n' &&
                       escape != 'r' && escape != 't') {
                set_error("Invalid escape sequence in string");
                return false;
            }
        }

        set_error("Unterminated string");
        return false;
    }

    bool parse_unicode_escape() {
        for (int index = 0; index < 4; ++index) {
            if (position_ >= text_.size() ||
                !std::isxdigit(static_cast<unsigned char>(text_[position_]))) {
                set_error("Invalid Unicode escape sequence");
                return false;
            }
            ++position_;
        }
        return true;
    }

    bool parse_number() {
        consume('-');

        if (consume('0')) {
            if (position_ < text_.size() &&
                std::isdigit(static_cast<unsigned char>(text_[position_]))) {
                set_error("Leading zeros are not allowed in numbers");
                return false;
            }
        } else if (!consume_digits()) {
            set_error("Expected a digit");
            return false;
        }

        if (consume('.')) {
            if (!consume_digits()) {
                set_error("Expected digits after decimal point");
                return false;
            }
        }

        if (position_ < text_.size() &&
            (text_[position_] == 'e' || text_[position_] == 'E')) {
            ++position_;
            if (position_ < text_.size() &&
                (text_[position_] == '+' || text_[position_] == '-')) {
                ++position_;
            }
            if (!consume_digits()) {
                set_error("Expected digits in exponent");
                return false;
            }
        }

        return true;
    }

    bool consume_digits() {
        const std::size_t start = position_;
        while (position_ < text_.size() &&
               std::isdigit(static_cast<unsigned char>(text_[position_]))) {
            ++position_;
        }
        return position_ > start;
    }

    bool parse_literal(std::string_view literal) {
        if (text_.substr(position_, literal.size()) != literal) {
            set_error("Invalid JSON literal");
            return false;
        }
        position_ += literal.size();
        return true;
    }

    bool consume(char expected) {
        if (position_ < text_.size() && text_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void skip_whitespace() {
        while (position_ < text_.size() &&
               (text_[position_] == ' ' || text_[position_] == '\t' ||
                text_[position_] == '\n' || text_[position_] == '\r')) {
            ++position_;
        }
    }

    void set_error(std::string message) {
        if (error_.empty()) {
            error_ = std::move(message);
            error_position_ = position_;
        }
    }

    ValidationResult failure() const {
        std::size_t line = 1;
        std::size_t column = 1;

        for (std::size_t index = 0; index < error_position_; ++index) {
            if (text_[index] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }

        std::ostringstream message;
        message << error_ << " at line " << line << ", column " << column;
        return {false, message.str()};
    }

    std::string_view text_;
    std::size_t position_ = 0;
    std::size_t error_position_ = 0;
    std::string error_;
};

}  // namespace

ValidationResult validate_json(std::string_view text) {
    return Parser(text).parse();
}

ValidationResult validate_json_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {false, "Could not open file: " + path.string()};
    }

    const std::string text(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    if (input.bad()) {
        return {false, "Could not read file: " + path.string()};
    }

    return validate_json(text);
}

}  // namespace json_validator
