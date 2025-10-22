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

**Last Updated:** 2025-10-22 16:04:47 CEST

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
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 688.3 µs | 362.8 µs | 1.2 ms | **baseline** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 8.7 ms | 6.3 ms | 14.6 ms | **12.7x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 11.3 ms | 6.6 ms | 25.0 ms | **16.4x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 16.6 ms | 9.4 ms | 32.2 ms | **24.2x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 19.3 ms | 13.8 ms | 38.2 ms | **28.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 23.6 ms | 17.3 ms | 40.0 ms | **34.3x** |

## CPU Cycles

| Implementation | Author | Stack | Cycles | Instructions | IPC | Cache Miss Rate | vs Baseline (Cycles) |
|----------------|--------|-------|--------|--------------|-----|-----------------|----------------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1,276,362 | 1,741,398 | 1.36 | 1.39% | **baseline** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 13,520,602 | 26,933,264 | 1.99 | 7.80% | **10.6x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 13,933,142 | 28,540,923 | 2.05 | 5.63% | **10.9x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 16,830,378 | 32,010,846 | 1.90 | 7.93% | **13.2x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 53,595,859 | 74,758,019 | 1.39 | 6.82% | **42.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 58,988,313 | 122,894,612 | 2.08 | 29.16% | **46.2x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1669 | 1 | **baseline** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 3572 | 3 | **2.1x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 3562 | 3 | **2.1x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 4632 | 4 | **2.8x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 13297 | 12 | **8.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 46711 | 45 | **28.0x** |

## Configuration

- **Time Benchmark:** warmup=10, runs=250
- **CPU Cycles Benchmark:** runs=250
- **Memory Benchmark:** runs=250
- **System:** Linux 6.16.12-200.fc42.x86_64
- **CPU:** 13th Gen Intel(R) Core(TM) i7-1360P

### Runtime Versions

- **C Compiler:** cc (GCC) 15.2.1 20250808 (Red Hat 15.2.1-1)
- **Python:** 3.12.12
- **Node.js:** 22.20.0
- **Bash:** 5.2.37
- **jq:** 1.8.1

## Notes

### Prerequisites

```bash
sudo dnf install hyperfine time jq python3 nodejs perf
```

#### Note

First run will download packages; subsequent runs use cached versions.

#### Testing tools

- `hyperfine` - Command-line benchmarking tool for execution time measurements
- `perf stat` - Linux performance analysis tool for CPU cycles, instructions, and cache metrics
- `/usr/bin/time` - GNU time utility for memory (RSS) profiling

### Details

#### Time measurements

Include process startup overhead

#### CPU cycles measurements

Show hardware-level resource consumption using `perf stat`, providing deterministic metrics for actual CPU work done (cycles, instructions, IPC, cache efficiency)

#### Memory measurements

Show peak RSS (Resident Set Size)

#### Test data

All tests use the same input data from `docs/actual_stdin.json`

### No Shell Overhead

mini-ccstatus benefits from being executed without shell wrapper overhead:

- Time benchmarks use `hyperfine -N` (`--shell=none`) to execute commands directly
- CPU cycles benchmarks use `perf stat` to measure each implementation directly
- Memory benchmarks invoke each implementation directly via `/usr/bin/time`

## Contribute

Feel free to contribute adding more implementations or improving the benchmark methodology, tested tools and configurations.
