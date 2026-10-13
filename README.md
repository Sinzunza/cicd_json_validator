# JSON Validator

A small, dependency-free C++ program that checks whether files contain valid
JSON. GitHub Actions workflows compile the project and run its tests on
GitHub-hosted Linux and Windows runners.

## Requirements

- A C++17-compatible compiler
- CMake 3.16 or newer

## Build on Windows

From PowerShell in the project root:

```powershell
cmake -S . -B build
cmake --build build
```

Run the validator:

```powershell
.\build\Debug\json_validator.exe path\to\file.json
```

Run the tests:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

## Build on Linux

From a terminal in the project root:

```bash
cmake -S . -B build
cmake --build build
```

Run the validator:

```bash
./build/json_validator path/to/file.json
```

Run the tests:

```bash
ctest --test-dir build --output-on-failure
```

## Validate multiple files

Pass multiple paths to either executable:

```text
json_validator first.json second.json
```

The command prints `VALID` or `INVALID` for each file. It exits with code `0`
when every file is valid, code `1` when any file is invalid or cannot be read,
and code `2` when no file arguments are provided. These exit codes will be
useful in a later CI phase.

## Note

This repo is public in order to use GitHub actions for free.