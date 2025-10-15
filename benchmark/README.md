# Benchmark Results

Performance comparison of different Claude Code statusline implementations.

**Last Updated:** 2025-10-15 12:16:44 CEST

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C
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
| mini-ccstatus | meeghele | C | 569.7 µs | 311.6 µs | 1128.1 µs | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 7.1 ms | 6.1 ms | 9.6 ms | **12.5x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 10.4 ms | 9.0 ms | 12.2 ms | **18.3x** |
| python-example.py | Anthropic | Python | 15.5 ms | 12.6 ms | 20.1 ms | **27.2x** |
| nodejs-example.js | Anthropic | Node.js | 20.4 ms | 17.5 ms | 28.5 ms | **35.8x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 7.4 ms | 6.1 ms | 10.6 ms | **13.0x** |
| claude-powerline | Owloops | Node.js | 53.8 ms | 45.1 ms | 143.2 ms | **94.4x** |
| ccstatusline | sirmalloc | Node.js | 185.2 ms | 155.1 ms | 375.0 ms | **325.1x** |
| ccusage | ryoppippi | Node.js | 464.3 ms | 407.1 ms | 962.1 ms | **815.0x** |
| claude-statusline-powerline | spences10 | Node.js | 34.6 ms | 30.1 ms | 62.8 ms | **60.7x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| mini-ccstatus | meeghele | C | 3607 | 3 | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 3662 | 3 | **1.0x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 4629 | 4 | **1.3x** |
| python-example.py | Anthropic | Python | 13283 | 12 | **3.7x** |
| nodejs-example.js | Anthropic | Node.js | 46229 | 45 | **12.8x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 3664 | 3 | **1.0x** |
| claude-powerline | Owloops | Node.js | 59997 | 58 | **16.6x** |
| ccstatusline | sirmalloc | Node.js | 85170 | 83 | **23.6x** |

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

