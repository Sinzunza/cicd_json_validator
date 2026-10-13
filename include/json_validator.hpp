#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace json_validator {

struct ValidationResult {
    bool is_valid;
    std::string error;
};

ValidationResult validate_json(std::string_view text);
ValidationResult validate_json_file(const std::filesystem::path& path);

}  // namespace json_validator

#endif
