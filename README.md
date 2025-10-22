[![CI](https://github.com/meeghele/mini-ccstatus/actions/workflows/ci.yml/badge.svg)](https://github.com/meeghele/mini-ccstatus/actions)
[![C](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Single Binary](https://img.shields.io/badge/Single%20Binary-0C7BDC)](#)
[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

# mini-ccstatus

A fast/minimal C implementation of the statusline for Claude Code CLI.

## Build & Test

### Prerequisites

Install build tools:

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential valgrind
```

**Fedora:**
```bash
sudo dnf install gcc make valgrind
```

### Building

```bash
make                       # Build binary and run demo
make all                   # Build, test, and run valgrind checks
make test                  # Run regression tests
make valgrind              # Run memory checks
make clean                 # Clean bin/ and obj/
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

## Benchmarks

See [`benchmark/`](benchmark/) for performance comparison against other Claude Code statusline implementations, including:
- Anthropic's reference examples (Bash, Python, Node.js)
- Community implementations 

**Quick start:**

```bash
cd benchmark
make time                  # Run time benchmarks
make cycles                # Run CPU cycles benchmarks
make memory                # Run memory benchmarks
make results               # Generate full benchmark report
```

For detailed documentation, see [`benchmark/README.md`](benchmark/README.md).

## Dependencies

This project includes [cJSON](https://github.com/DaveGamble/cJSON) (MIT License) vendored in `lib/cjson/`.

## License

Licensed under the MIT License. See `LICENSE` for details.

## Author

**Michele Tavella** – [meeghele@proton.me](mailto:meeghele@proton.me)
