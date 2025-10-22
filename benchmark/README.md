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
- [**ccusage**](https://github.com/ryoppippi/ccusage) (@ryoppippi) - Node.js
- [**ccstatusline**](https://github.com/sirmalloc/ccstatusline) (@sirmalloc) - Node.js

## ⚠️ Warning
Unlike the reference implementations that only process JSON from stdin, `ccusage` and `ccstatusline` also read configuration files from `$HOME/.claude` and may make API calls. The performance measurements for these tools reflect their full behavior and may vary depending on your system configuration, network latency, and Claude API response times.

Measuring the performance of these tools is out of scope for this benchmark; technically one could point `$CLAUDE_CONFIG_DIR` to a synthetic directory and have a file like `../docs/actual_stdin.json` point to it. This would lead to reproducible results.

## Execution Time

| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |
|----------------|--------|-------|-----|-----|-----|-------------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 688.3 µs | 362.8 µs | 1.2 ms | **baseline** |
| **Examples** | | | | | | | |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 8.7 ms | 6.3 ms | 14.6 ms | **12.7x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 11.3 ms | 6.6 ms | 25.0 ms | **16.4x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 16.6 ms | 9.4 ms | 32.2 ms | **24.2x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 19.3 ms | 13.8 ms | 38.2 ms | **28.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 23.6 ms | 17.3 ms | 40.0 ms | **34.3x** |
| **Third-party tools** | | | | | | |
| [ccstatusline (direct)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 193.4 ms | 169.9 ms | 217.7 ms | **281.0x** |
| [ccusage statusline --offline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 205.5 ms | 51.9 ms | 3.0 s | **298.6x** |
| [ccusage statusline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 252.8 ms | 51.8 ms | 3.1 s | **367.3x** |
| **Third-party tools with `bunx`/`npx`** | | | | | | |
| [ccstatusline (bunx)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Bun, bunx | 669.8 ms | 527.0 ms | 1.6 s | **973.1x** |
| [ccusage statusline --offline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 873.6 ms | 210.9 ms | 3.1 s | **1269.2x** |
| [ccusage statusline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 704.5 ms | 208.9 ms | 3.2 s | **1023.5x** |

## CPU Cycles

| Implementation | Author | Stack | Cycles | Instructions | IPC | Cache Miss Rate | vs Baseline (Cycles) |
|----------------|--------|-------|--------|--------------|-----|-----------------|----------------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1,276,362 | 1,741,398 | 1.36 | 1.39% | **baseline** |
| **Examples** | | | | | | | |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 13,520,602 | 26,933,264 | 1.99 | 7.80% | **10.6x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 13,933,142 | 28,540,923 | 2.05 | 5.63% | **10.9x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 16,830,378 | 32,010,846 | 1.90 | 7.93% | **13.2x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 53,595,859 | 74,758,019 | 1.39 | 6.82% | **42.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 58,988,313 | 122,894,612 | 2.08 | 29.16% | **46.2x** |
| **Third-party tools** | | | | | | | |
| [ccstatusline (direct)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 557,996,963 | 1,263,271,861 | 2.26 | 18.11% | **437.2x** |
| [ccusage statusline --offline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 743,941,587 | 2,192,976,337 | 2.95 | 24.74% | **582.9x** |
| [ccusage statusline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 780,058,704 | 2,311,189,217 | 2.96 | 24.76% | **611.2x** |
| **Third-party tools with `bunx`/`npx`** | | | | | | | |
| [ccstatusline (bunx)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Bun, bunx | 599,324,634 | 1,357,631,857 | 2.27 | 18.25% | **469.6x** |
| [ccusage statusline --offline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 5,607,044,575 | 16,703,413,501 | 2.98 | 24.99% | **4393.0x** |
| [ccusage statusline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 5,607,945,761 | 16,713,890,173 | 2.98 | 24.92% | **4393.7x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1669 | 1 | **baseline** |
| **Examples** | | | | | | | |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 3572 | 3 | **2.1x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 3562 | 3 | **2.1x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 4632 | 4 | **2.8x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 13297 | 12 | **8.0x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 46711 | 45 | **28.0x** |
| **Third-party tools** | | | | | |
| [ccstatusline (direct)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 86376 | 84 | **51.8x** |
| [ccusage statusline --offline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 108568 | 106 | **65.0x** |
| [ccusage statusline (direct)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 116568 | 113 | **69.8x** |
| **Third-party tools with `bunx`/`npx`** | | | | | |
| [ccstatusline (bunx)](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Bun, bunx | 86329 | 84 | **51.7x** |
| [ccusage statusline --offline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 249868 | 244 | **149.7x** |
| [ccusage statusline (npx)](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js, npx | 245819 | 240 | **147.3x** |

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
npm install -g ccusage ccstatusline
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

All tests use the same input data from `../docs/actual_stdin.json`

### No Shell Overhead

mini-ccstatus benefits from being executed without shell wrapper overhead:

- Time benchmarks use `hyperfine -N` (`--shell=none`) to execute commands directly
- CPU cycles benchmarks use `perf stat` to measure each implementation directly
- Memory benchmarks invoke each implementation directly via `/usr/bin/time`

## Contribute

Feel free to contribute adding more implementations or improving the benchmark methodology, tested tools and configurations.
