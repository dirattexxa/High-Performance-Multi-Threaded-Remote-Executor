# High-Performance Multi-Threaded Remote Executor

A production-ready, highly efficient Linux remote command execution server written in modern C++. This project demonstrates low-level systems programming, secure memory management, and asynchronous network architecture under Linux.

## Features

* **Zero-Copy Memory Management:** Custom implementation of Move Semantics (`CommandPacket&&`) to transfer heavy network buffers between threads without deep copying.
* **Explicit Resource Safety:** Prevention of memory leaks and `Double Free` vulnerabilities by explicitly deleting copy constructors and assignment operators (`= delete`).
* **Asynchronous Multi-Threading:** High-throughput request handling utilizing POSIX threads via `std::thread` in a detached architecture.
* **Process Isolation & Stream Redirection:** Safe command execution via isolated child processes using `fork()` and `execvp()`, with full network stream redirection using `dup2()`.

## Architecture Overview

1.  **Main Thread:** Permanently listens on port `8080`, accepts incoming TCP connections, allocates a packet, and instantly delegates processing to a detached worker thread.
2.  **Worker Thread:** Reads raw data into a `CommandPacket` via Move Semantics, sanitizes the payload, and forks a new process.
3.  **Child Process:** Re-routes `STDOUT` and `STDERR` directly into the client's socket descriptor, then overlays the execution context with `/bin/sh -c` to execute the payload.
4.  **Parent Thread:** Synchronizes via `waitpid()` to prevent premature socket termination and clean up zombie processes, ensuring OS resource stability.

## Requirements

* **OS:** Linux (Tested thoroughly on Arch Linux)
* **Compiler:** GCC / G++ (Supporting C++11 or higher)
* **Tools:** `netcat` (for testing)

## Compilation & Usage

To compile the multi-threaded server, run the following command in your terminal:

```bash
g++ server.cpp -o remote_executor -pthread
