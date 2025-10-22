## Notes

### Prerequisites

Required tools (Fedora):

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
