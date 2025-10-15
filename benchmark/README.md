# Benchmark Results

This benchmark provides a performance comparison of different Claude Code status line implementations against [mini-ccstatus](https://github.com/meeghele/mini-ccstatus).


**Last Updated:** 2025-10-15 17:48:08 CEST

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJSON
- [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) (@Anthropic) - Bash, jq
- [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) (@Anthropic) - Bash, jq
- [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) (@Anthropic) - Bash, jq, Git
- [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) (@Anthropic) - Python
- [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) (@Anthropic) - Node.js
- [**claude-statusline-powerline**](https://github.com/spences10/claude-statusline-powerline) (@spences10) - Node.js
- [**claude-powerline**](https://github.com/Owloops/claude-powerline) (@Owloops) - Node.js
- [**ccstatusline**](https://github.com/sirmalloc/ccstatusline) (@sirmalloc) - Node.js
- [**ccusage**](https://github.com/ryoppippi/ccusage) (@ryoppippi) - Node.js

## Execution Time

| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |
|----------------|--------|-------|-----|-----|-----|-------------------|
| mini-ccstatus | meeghele | C, cJSON | 467.6 µs | 272.0 µs | 849.1 µs | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 6.3 ms | 5.5 ms | 7.2 ms | **13.5x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 6.4 ms | 5.3 ms | 7.2 ms | **13.7x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 9.3 ms | 8.2 ms | 10.2 ms | **19.9x** |
| python-example.py | Anthropic | Python | 12.8 ms | 10.5 ms | 18.0 ms | **27.4x** |
| nodejs-example.js | Anthropic | Node.js | 16.3 ms | 14.2 ms | 22.9 ms | **34.9x** |
| claude-statusline-powerline | spences10 | Node.js | 30.2 ms | 27.0 ms | 35.2 ms | **64.6x** |
| claude-powerline | Owloops | Node.js | 39.4 ms | 35.4 ms | 47.2 ms | **84.3x** |
| ccstatusline | sirmalloc | Node.js | 147.2 ms | 138.6 ms | 158.7 ms | **314.8x** |
| ccusage | ryoppippi | Node.js | 409.7 ms | 388.8 ms | 435.3 ms | **876.2x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| mini-ccstatus | meeghele | C, cJSON | 3560 | 3 | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 3612 | 3 | **1.0x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 3620 | 3 | **1.0x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 4617 | 4 | **1.3x** |
| python-example.py | Anthropic | Python | 12858 | 12 | **3.6x** |
| nodejs-example.js | Anthropic | Node.js | 45733 | 44 | **12.8x** |
| claude-statusline-powerline | spences10 | Node.js | 55087 | 53 | **15.5x** |
| claude-powerline | Owloops | Node.js | 59164 | 57 | **16.6x** |
| ccstatusline | sirmalloc | Node.js | 84314 | 82 | **23.7x** |
| ccusage | ryoppippi | Node.js | 123820 | 120 | **34.8x** |

## Configuration

- **Time Benchmark:** warmup=10, runs=250
- **Memory Benchmark:** runs=250
- **System:** Linux 6.16.10-200.fc42.x86_64
- **CPU:** 13th Gen Intel(R) Core(TM) i7-1360P

### Runtime Versions

- **C Compiler:** cc (GCC) 15.2.1 20250808 (Red Hat 15.2.1-1)
- **Python:** 3.12.12
- **Node.js:** 22.20.0
- **Bash:** 5.2.37
- **jq:** 1.8.1

## Notes

### Measurement Methodology

- **Lower values are better** for both time and memory
- **Time measurements** include process startup overhead
- **Memory measurements** show peak RSS (Resident Set Size)
- All tests use the same input data from `docs/actual_stdin.json`

### No Shell Overhead

**mini-ccstatus** benefits from being executed **without shell wrapper overhead**:
- Time benchmarks use `hyperfine -N` (`--shell=none`) to execute commands directly
- This avoids spawning an intermediate shell process (bash/sh)
- Memory benchmarks follow the same principle for consistency
- Pure stdin → process → stdout pipeline for all tests

### Test Consistency

All implementations are tested using **identical settings**:
- Same input data (stdin from JSON file)
- Same execution environment (no shell wrapper where possible)
- Same measurement tools (hyperfine for time, GNU time for memory)
- Same number of iterations (warmup=10, runs=250)

### External NPM Package Considerations

Some npm packages may access external resources during execution:
- **claude-statusline-powerline**
- **ccstatusline**
- **claude-powerline**

**Note**: Benchmarks measure these packages as-installed so to reflect real-world usage/performance.

