# Benchmark Results

This benchmark provides a performance comparison of different Claude Code status line implementations against [mini-ccstatus](https://github.com/meeghele/mini-ccstatus).


**Last Updated:** 2025-10-15 17:30:33 CEST

## Implementations

- [**mini-ccstatus**](https://github.com/meeghele/mini-ccstatus) (@meeghele) - C, cJSON
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
| mini-ccstatus | meeghele | C, cJSON | 446.5 µs | 272.4 µs | 900.9 µs | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 6.3 ms | 5.3 ms | 7.4 ms | **14.1x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 6.4 ms | 5.6 ms | 7.0 ms | **14.3x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 9.3 ms | 8.2 ms | 11.2 ms | **20.8x** |
| python-example.py | Anthropic | Python | 12.8 ms | 11.1 ms | 17.5 ms | **28.7x** |
| nodejs-example.js | Anthropic | Node.js | 17.0 ms | 14.7 ms | 23.2 ms | **38.1x** |
| claude-statusline-powerline | spences10 | Node.js | 31.5 ms | 28.5 ms | 37.4 ms | **70.5x** |
| claude-powerline | Owloops | Node.js | 41.3 ms | 36.8 ms | 46.5 ms | **92.5x** |
| ccstatusline | sirmalloc | Node.js | 155.7 ms | 145.2 ms | 172.0 ms | **348.7x** |
| ccusage | ryoppippi | Node.js | 429.6 ms | 402.9 ms | 684.1 ms | **962.2x** |

## Memory Usage

| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |
|----------------|--------|-------|--------|--------|-------------|
| mini-ccstatus | meeghele | C, cJSON | 3565 | 3 | **baseline** |
| simple-statusline.sh | Anthropic | Bash, jq | 3614 | 3 | **1.0x** |
| helper-function-statusline.sh | Anthropic | Bash, jq | 3623 | 3 | **1.0x** |
| git-aware-statusline.sh | Anthropic | Bash, jq, Git | 4629 | 4 | **1.3x** |
| python-example.py | Anthropic | Python | 12846 | 12 | **3.6x** |
| nodejs-example.js | Anthropic | Node.js | 45745 | 44 | **12.8x** |
| claude-statusline-powerline | spences10 | Node.js | 55055 | 53 | **15.4x** |
| claude-powerline | Owloops | Node.js | 59151 | 57 | **16.6x** |
| ccstatusline | sirmalloc | Node.js | 84330 | 82 | **23.7x** |
| ccusage | ryoppippi | Node.js | 123533 | 120 | **34.7x** |

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

