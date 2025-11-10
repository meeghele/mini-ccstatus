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
