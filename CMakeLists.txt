cmake_minimum_required(VERSION 3.16)

project(json_validator VERSION 0.1.0 LANGUAGES CXX)

add_library(json_validator_lib src/json_validator.cpp)
target_include_directories(json_validator_lib PUBLIC include)
target_compile_features(json_validator_lib PUBLIC cxx_std_17)

add_executable(json_validator src/main.cpp)
target_link_libraries(json_validator PRIVATE json_validator_lib)

include(CTest)

if(BUILD_TESTING)
    add_executable(json_validator_tests tests/json_validator_tests.cpp)
    target_link_libraries(json_validator_tests PRIVATE json_validator_lib)
    add_test(NAME json_validator_tests COMMAND json_validator_tests)
endif()