# Benchmark Results

This benchmark provides a performance comparison of different Claude Code status line implementations against [mini-ccstatus](https://github.com/meeghele/mini-ccstatus).

**Last Updated:** 2025-10-15 13:23:18 CEST

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJson
- [**simple-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#simple-status-line) (@Anthropic) - Bash, jq
- [**git-aware-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#git-aware-status-line) (@Anthropic) - Bash, jq, Git
- [**python-example.py**](https://docs.claude.com/en/docs/claude-code/statusline#python-example) (@Anthropic) - Python
- [**nodejs-example.js**](https://docs.claude.com/en/docs/claude-code/statusline#node-js-example) (@Anthropic) - Node.js
- [**helper-function-statusline.sh**](https://docs.claude.com/en/docs/claude-code/statusline#helper-function-approach) (@Anthropic) - Bash, jq
- [**claude-powerline**](https://github.com/Owloops/claude-powerline) (@Owloops) - Node.js
- [**ccstatusline**](https://github.com/sirmalloc/ccstatusline) (@sirmalloc) - Node.js
- [**ccusage**](https://github.com/ryoppippi/ccusage) (@ryoppippi) - Node.js
- [**claude-statusline-powerline**](https://github.com/spences10/claude-statusline-powerline) (@spences10) - Node.js

## Execution Time

| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |
|----------------|--------|-------|-----|-----|-----|-------------------|
| mini-ccstatus | meeghele | C, cJson | 483.8 µs | 284.9 µs | 821.0 µs | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 6.4 ms | 5.5 ms | 7.1 ms | **13.2x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 9.5 ms | 8.5 ms | 10.8 ms | **19.6x** |
| python-example.py | Anthropic | Python | 13.0 ms | 10.7 ms | 18.5 ms | **26.9x** |
| nodejs-example.js | Anthropic | Node.js | 16.7 ms | 14.4 ms | 24.2 ms | **34.5x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 6.6 ms | 5.7 ms | 7.4 ms | **13.6x** |
| claude-powerline | Owloops | Node.js | 40.0 ms | 35.0 ms | 54.6 ms | **82.7x** |
| ccstatusline | sirmalloc | Node.js | 152.1 ms | 141.0 ms | 206.6 ms | **314.4x** |
| ccusage | ryoppippi | Node.js | 410.8 ms | 381.3 ms | 496.0 ms | **849.1x** |
| claude-statusline-powerline | spences10 | Node.js | 31.3 ms | 28.2 ms | 43.3 ms | **64.7x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| mini-ccstatus | meeghele | C, cJson | 3602 | 3 | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 3663 | 3 | **1.0x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 4606 | 4 | **1.3x** |
| python-example.py | Anthropic | Python | 13314 | 13 | **3.7x** |
| nodejs-example.js | Anthropic | Node.js | 46658 | 45 | **13.0x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 3661 | 3 | **1.0x** |
| claude-powerline | Owloops | Node.js | 60398 | 58 | **16.8x** |
| ccstatusline | sirmalloc | Node.js | 85796 | 83 | **23.8x** |

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

