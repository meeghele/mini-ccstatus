# mini-ccstatus Benchmark Results

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

**Last Updated:** 2025-11-10 16:43:34 CET

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJSON
- [**mini-ccstatus --all**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJSON
- [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) (@Anthropic) - Bash, jq
- [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) (@Anthropic) - Bash, jq
- [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) (@Anthropic) - Bash, jq, Git
- [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) (@Anthropic) - Python
- [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) (@Anthropic) - Node.js
- [**ccstatusline**](https://github.com/sirmalloc/ccstatusline) (@sirmalloc) - Node.js
- [**ccusage statusline --offline**](https://github.com/ryoppippi/ccusage) (@ryoppippi) - Node.js
- [**ccusage statusline**](https://github.com/ryoppippi/ccusage) (@ryoppippi) - Node.js

**Note:** Unlike the reference implementations `mini-ccstatus` that only process JSON from stdin, `mini-ccstatus --all`, `ccusage` and `ccstatusline` also read configuration files from `$HOME/.claude`.

## Execution Time

| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |
|----------------|--------|-------|-----|-----|-----|-------------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 436.1 µs | 299.1 µs | 650.7 µs | **baseline** |
| [mini-ccstatus --all](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 469.8 µs | 317.7 µs | 611.2 µs | **1.1x** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 5.9 ms | 4.6 ms | 6.3 ms | **13.4x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 5.9 ms | 5.0 ms | 6.7 ms | **13.6x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 8.6 ms | 7.5 ms | 9.2 ms | **19.6x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 12.4 ms | 10.3 ms | 17.2 ms | **28.4x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 15.6 ms | 13.7 ms | 23.0 ms | **35.7x** |
| [ccstatusline](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 146.7 ms | 131.8 ms | 206.9 ms | **336.5x** |
| [ccusage statusline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 277.6 ms | 41.0 ms | 5.4 s | **636.7x** |
| [ccusage statusline --offline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 301.0 ms | 40.7 ms | 5.2 s | **690.3x** |

## CPU Cycles

| Implementation | Author | Stack | Cycles | Instructions | IPC | Cache Miss Rate | vs Baseline (Cycles) |
|----------------|--------|-------|--------|--------------|-----|-----------------|----------------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 1,615,445 | 2,202,493 | 1.36 | 1.04% | **baseline** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 14,196,691 | 25,580,397 | 1.80 | 5.04% | **8.8x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 13,938,171 | 24,568,587 | 1.76 | 6.03% | **8.6x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 18,581,036 | 30,674,742 | 1.65 | 8.42% | **11.5x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 55,389,291 | 78,440,499 | 1.42 | 2.35% | **34.3x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 61,048,530 | 116,941,013 | 1.92 | 24.62% | **37.8x** |
| [ccstatusline](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 575,648,217 | 1,281,271,018 | 2.23 | 19.29% | **356.3x** |
| [ccusage statusline --offline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 1,169,956,764 | 3,224,752,909 | 2.76 | 30.36% | **724.2x** |
| [ccusage statusline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 1,252,381,524 | 3,459,293,292 | 2.76 | 30.73% | **775.3x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| [mini-ccstatus](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 2175 | 2 | **baseline** |
| [mini-ccstatus --all](https://github.com/meeghele/mini-ccstatus) | meeghele | C, cJSON | 969 | 0 | **0.4x** |
| [simple-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) | Anthropic | Bash, jq | 3726 | 3 | **1.7x** |
| [helper-function-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) | Anthropic | Bash, jq | 3742 | 3 | **1.7x** |
| [git-aware-statusline.sh](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) | Anthropic | Bash, jq, Git | 4601 | 4 | **2.1x** |
| [python-example.py](https://docs.claude.com/en/docs/claude-code/statusline#python-example) | Anthropic | Python | 12785 | 12 | **5.9x** |
| [nodejs-example.js](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) | Anthropic | Node.js | 51184 | 49 | **23.5x** |
| [ccstatusline](https://github.com/sirmalloc/ccstatusline) | sirmalloc | Node.js | 92291 | 90 | **42.4x** |
| [ccusage statusline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 98151 | 95 | **45.1x** |
| [ccusage statusline --offline](https://github.com/ryoppippi/ccusage) | ryoppippi | Node.js | 98437 | 96 | **45.3x** |

## Configuration

- **Time Benchmark:** warmup=10, runs=250
- **CPU Cycles Benchmark:** runs=250
- **Memory Benchmark:** runs=250
- **System:** Linux 6.17.7-300.fc43.x86_64
- **CPU:** 13th Gen Intel(R) Core(TM) i7-1360P

### Runtime Versions

- **C Compiler:** cc (GCC) 15.2.1 20251022 (Red Hat 15.2.1-3)
- **Python:** 3.12.12
- **Node.js:** 24.11.0
- **Bash:** 5.3.0
- **jq:** 1.8.1

## Notes

### Prerequisites

Required tools (Fedora):

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

All tests use the same input data from `docs/actual_stdin.json`

### No Shell Overhead

mini-ccstatus benefits from being executed without shell wrapper overhead:

- Time benchmarks use `hyperfine -N` (`--shell=none`) to execute commands directly
- CPU cycles benchmarks use `perf stat` to measure each implementation directly
- Memory benchmarks invoke each implementation directly via `/usr/bin/time`

## Contribute

Feel free to contribute adding more implementations or improving the benchmark methodology, tested tools and configurations.

