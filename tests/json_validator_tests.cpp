#include "json_validator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect_valid(std::string_view json, const std::string& test_name) {
    const auto result = json_validator::validate_json(json);
    if (!result.is_valid) {
        std::cerr << "FAILED: " << test_name << ": " << result.error << '\n';
        ++failures;
    }
}

void expect_invalid(std::string_view json, const std::string& test_name) {
    const auto result = json_validator::validate_json(json);
    if (result.is_valid) {
        std::cerr << "FAILED: " << test_name << ": expected invalid JSON\n";
        ++failures;
    }
}

void test_file_validation() {
    const auto path =
        std::filesystem::temp_directory_path() / "json_validator_test.json";

    {
        std::ofstream output(path);
        output << R"({"from_file": true})";
    }

    const auto result = json_validator::validate_json_file(path);
    std::filesystem::remove(path);

    if (!result.is_valid) {
        std::cerr << "FAILED: valid file: " << result.error << '\n';
        ++failures;
    }
}

}  // namespace

int main() {
    expect_valid(R"({"name":"example","items":[1,2,3]})", "object");
    expect_valid(R"([true,false,null,"text",-12.5e+2])", "array");
    expect_valid(R"({"escaped":"line\n\u0041"})", "string escapes");
    expect_valid("  42  ", "surrounding whitespace");
    expect_invalid("", "empty input");
    expect_invalid(R"({"name":"example",})", "trailing object comma");
    expect_invalid("[1,]", "trailing array comma");
    expect_invalid("01", "leading zero");
    expect_invalid(R"({"value":"\q"})", "invalid escape");
    expect_invalid("true false", "multiple values");
    test_file_validation();

    if (failures == 0) {
        std::cout << "All JSON validator tests passed.\n";
    }

    return failures == 0 ? 0 : 1;
}
