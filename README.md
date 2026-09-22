# Flight Simulator and Monitoring System

Team 3 — Section 9
- 2520030359 — Mohammad Faizaan
- 2520030267 — Ishaan Kulkarni

## Current implementation scope

This starter implementation is based on the team's submitted project abstract and the OS/System Programming course material.

It demonstrates:
- Linux C systems programming
- process creation with `fork()`
- process identification with `getpid()` / `getppid()`
- anonymous IPC with `pipe()`
- process synchronization/termination with `waitpid()`
- asynchronous control using SIGINT/SIGTERM
- flight telemetry generation
- monitoring and threshold detection
- Linux file I/O using `open()` and `dprintf()`
- telemetry logging to `logs/flight.log`

## Build

```bash
make
```

## Run

```bash
./flight_simulator
```

Press `Ctrl+C` during execution to demonstrate signal-based termination.

## Inspect log

```bash
cat logs/flight.log
```

## Note

The project abstract describes additional mechanisms such as FIFOs, POSIX threads, mutexes/semaphores, and control processes. They should only be added when required by the faculty's assigned implementation scope. This version intentionally focuses on a clean CO1–CO3 demonstration rather than claiming features that have not been implemented.
