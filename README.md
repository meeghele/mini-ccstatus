[![CI](https://github.com/meeghele/mini-ccstatus/actions/workflows/ci.yml/badge.svg)](https://github.com/meeghele/mini-ccstatus/actions)
[![C](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Single Binary](https://img.shields.io/badge/Single%20Binary-0C7BDC)](#)
[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

# mini-ccstatus

A fast/minimal C implementation of the statusline for Claude Code CLI.

## Build & Test

GNU Make will build a binary into `./bin/mini-ccstatus`.`

```bash
make

# Prints the statusline
make demo

# Run the shell-based regression tests
make test

# Clean bin/ and obj/
make clean
```

## Usage

### Compact Mode
```json
{
  "statusLine": {
    "type": "command",
    "command": "~/mini-ccstatus/bin/mini-ccstatus",
    "padding": 0
  }
}
```

### Verbose Mode
```json
{
  "statusLine": {
    "type": "command",
    "command": "VERBOSE=yeah ~/mini-ccstatus/bin/mini-ccstatus",
    "padding": 0
  }
}
```

## Screenshots

### Compact Mode
![Compact Mode](docs/mini-ccstatus_compact.png)

### Verbose Mode
![Verbose Mode](docs/mini-ccstatus_verbose.png)

## Dependencies

This project includes [cJSON](https://github.com/DaveGamble/cJSON) (MIT License) vendored in `lib/cjson/`.

## License

Licensed under the MIT License. See `LICENSE` for details.

## Author

**Michele Tavella** – [meeghele@proton.me](mailto:meeghele@proton.me)
