#include "json_validator.hpp"

#include <filesystem>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: json_validator <file> [file ...]\n";
        return 2;
    }

    bool all_valid = true;

    for (int index = 1; index < argc; ++index) {
        const std::filesystem::path path(argv[index]);
        const auto result = json_validator::validate_json_file(path);

        if (result.is_valid) {
            std::cout << "VALID: " << path.string() << '\n';
        } else {
            all_valid = false;
            std::cout << "INVALID: " << path.string() << ": "
                      << result.error << '\n';
        }
    }

    return all_valid ? 0 : 1;
}
