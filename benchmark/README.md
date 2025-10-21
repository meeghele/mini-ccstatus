# Benchmark Results

This benchmark provides a performance comparison of different Claude Code status line implementations against [mini-ccstatus](https://github.com/meeghele/mini-ccstatus).

## Rationale

Claude Code's statusline architecture restarts the statusline command on every update (at most every 300ms) rather than maintaining a persistent process. This design means:

- Your statusline script is spawned fresh each time it needs to update
- The process receives JSON via stdin, outputs to stdout, and terminates
- This cycle repeats throughout your entire Claude Code session

Because the statusline command is executed hundreds or thousands of times during a typical session, even small performance differences compound significantly:

- A script taking 500ms but executed over and over consumes significant CPU resources
- Higher memory usage multiplied across constant process spawning increases system load
- Process startup overhead (interpreter initialization, module loading) happens every single time

A better architecture would start the statusline process once and continuously feed it data, eliminating process startup overhead entirely. This is especially critical for interpreted languages like Node.js or Python, where each invocation must:
- Launch the interpreter runtime
- Parse and compile the script
- Load all required modules and dependencies
- Initialize the application state
- Only then execute the actual statusline logic

Until Claude Code adopts a persistent-process approach, this benchmark measures real-world performance impact to help you choose an implementation that minimizes the overhead of the restart-based architecture.

**Last Updated:** 2025-10-22 00:22:26 CEST

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJSON
- [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) (@Anthropic) - Bash, jq
- [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) (@Anthropic) - Bash, jq
- [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) (@Anthropic) - Bash, jq, Git
- [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) (@Anthropic) - Python
- [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) (@Anthropic) - Node.js

## Execution Time

| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |
|----------------|--------|-------|-----|-----|-----|-------------------|
| [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 483.8 µs | 342.3 µs | 949.6 µs | **baseline** |
| [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 7.4 ms | 5.9 ms | 8.7 ms | **15.3x** |
| [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 7.5 ms | 6.3 ms | 9.0 ms | **15.6x** |
| [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 10.6 ms | 9.4 ms | 12.4 ms | **22.0x** |
| [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 16.4 ms | 13.7 ms | 28.8 ms | **34.0x** |
| [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 17.8 ms | 14.8 ms | 26.5 ms | **36.8x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1575 | 1 | **baseline** |
| [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 3379 | 3 | **2.1x** |
| [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 3386 | 3 | **2.1x** |
| [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 4434 | 4 | **2.8x** |
| [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 11262 | 10 | **7.2x** |
| [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 47093 | 45 | **29.9x** |

## Configuration

- **Time Benchmark:** warmup=10, runs=250
- **Memory Benchmark:** runs=250
- **System:** Linux 6.16.12-200.fc42.x86_64
- **CPU:** Intel(R) Core(TM) Ultra 7 155H

### Runtime Versions

- **C Compiler:** cc (GCC) 15.2.1 20250808 (Red Hat 15.2.1-1)
- **Python:** 3.12.12
- **Node.js:** 22.21.0
- **Bash:** 5.2.37
- **jq:** 1.8.1

## Notes

### Prerequisites

Required tools (Fedora):

```bash
sudo dnf install hyperfine time jq python3 nodejs
```

### Details

- **Time measurements** include process startup overhead
- **Memory measurements** show peak RSS (Resident Set Size)
- All tests use the same input data from `docs/actual_stdin.json`
- I might add JSON files parsing functionalities to [mini-ccstatus](https://github.com/meeghele/mini-ccstatus)

### No Shell Overhead

**mini-ccstatus** benefits from being executed **without shell wrapper overhead**:
- Time benchmarks use `hyperfine -N` (`--shell=none`) to execute commands directly
- Memory benchmarks invoke each implementation directly via `/usr/bin/time`

## Contribute

Feel free to contribute adding more implementations or improving the benchmark methodology, tested tools and configurations.

