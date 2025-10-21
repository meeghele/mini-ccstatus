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
