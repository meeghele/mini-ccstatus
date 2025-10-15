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
