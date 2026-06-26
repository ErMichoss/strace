# ft_strace

A reimplementation of the `strace` command in C, built as part of the 42 curriculum.

---

## What is strace?

`strace` is a Linux diagnostic tool that **intercepts and displays the system calls (syscalls)** made by a process, along with their arguments and return values. It is widely used for debugging, understanding program behaviour, and reverse engineering.

```
your program  ──── syscall (e.g. open, read, write) ────►  kernel
your program  ◄─── return value / error              ────   kernel
                   ft_strace prints all of this
```

---

## Requirements

- Linux x86_64 (also supports 32-bit binaries via `test32`)
- GCC and Make

> No root privileges required — `ptrace` is available to any process for its own children.

---

## Build

```bash
make        # compile
make debug  # compile with -g -DDEBUG=1
make clean  # remove object files
make fclean # remove object files and binary
make re     # full recompile
```

---

## Usage

```bash
./ft_strace <command> [args...]
```

`<command>` is the program you want to trace, followed by its arguments.

---

## Examples

```bash
# Trace a simple command
./ft_strace ls -la

# Trace a program that causes a segfault
./ft_strace ./segfault

# Trace a 32-bit binary
./ft_strace ./test32

# Trace with multiple arguments
./ft_strace cat /etc/hostname
```

---

## Output format

Each line shows one syscall: its name, arguments, and return value.

```
execve("ls", ["ls", "-la"], 0x... /* N vars */) = 0
brk(NULL)                                       = 0x...
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY)  = 3
read(3, "\177ELF"..., 832)                      = 832
write(1, "total 48\n", 9)                       = 9
...
+++ exited with 0 +++
```

If the traced process is killed by a signal:

```
--- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_MAPERR, si_addr=0x0} ---
+++ killed by SIGSEGV (core dumped) +++
```

---

## How it works

1. **Fork** — the tracer calls `fork()` to create a child process
2. **PTRACE_TRACEME** — the child calls `ptrace(PTRACE_TRACEME)` before `execve()`, telling the kernel to stop it at every syscall
3. **waitpid loop** — the parent waits for the child to stop with `waitpid()`
4. **Read registers** — on each stop, the parent reads the child's CPU registers with `ptrace(PTRACE_GETREGS)` to get the syscall number and arguments
5. **Lookup table** — the syscall number is matched against `table_64.c` or `table_32.c` to get its name and argument types
6. **Print** — arguments and return value are formatted and printed to stderr
7. **Continue** — the parent calls `ptrace(PTRACE_SYSCALL)` to resume the child until the next syscall entry or exit
8. **Signals** — any signal received by the child is caught, displayed, and forwarded

### How syscall arguments are read

In x86_64, syscall arguments are passed in registers:

| Register | Role              |
|----------|-------------------|
| `rax`    | syscall number    |
| `rdi`    | argument 1        |
| `rsi`    | argument 2        |
| `rdx`    | argument 3        |
| `r10`    | argument 4        |
| `r8`     | argument 5        |
| `r9`     | argument 6        |

String arguments are read from the child's memory word by word using `ptrace(PTRACE_PEEKDATA)`.

---

## Project structure

```
ft_strace/
├── incl/                          # Header files
├── src/
│   ├── main.c                     # Entry point, argument parsing
│   ├── tracer.c                   # Main ptrace loop (waitpid + PTRACE_SYSCALL)
│   ├── child.c                    # Fork + PTRACE_TRACEME + execve
│   ├── output.c                   # Formats and prints syscall name, args, return value
│   ├── buffer.c                   # Reads strings from child memory via PTRACE_PEEKDATA
│   ├── aux.c                      # Utility functions
│   ├── signal.c                   # Signal handling and display
│   └── syscalls_table/
│       ├── table_64.c             # x86_64 syscall table (name + arg types)
│       ├── table_32.c             # x86_32 syscall table
│       ├── error_table.c          # errno → name (ENOENT, EACCES, ...)
│       └── signal_table.c         # signal number → name (SIGSEGV, SIGTERM, ...)
├── segfault.c                     # Test program: dereferences a NULL pointer
├── test32                         # Precompiled 32-bit test binary
└── Makefile
```

---

## Test programs

### `segfault`

A minimal C program that triggers a segmentation fault by writing to a NULL pointer:

```c
int main() {
    int *p = NULL;
    *p = 42;
    return 0;
}
```

Use it to verify your strace correctly detects and displays a `SIGSEGV`:

```bash
./ft_strace ./segfault
```

Expected output includes:
```
--- SIGSEGV {si_signo=SIGSEGV, ...} ---
+++ killed by SIGSEGV (core dumped) +++
```

### `test32`

A precompiled 32-bit binary for testing the x86_32 syscall table (`table_32.c`). In 32-bit mode, syscall arguments are in different registers (`eax`, `ebx`, `ecx`, `edx`, `esi`, `edi`).

---

## Running with Docker (no VM needed)

You can build and test `ft_strace` inside a Docker container without touching your host system. The container needs `SYS_PTRACE` capability to allow `ptrace` calls.

**From the root of the repository, run:**

```bash
docker run -it --rm --cap-add=SYS_PTRACE -v "$(pwd):/workspace" -w /workspace \
  ubuntu:24.04 bash -c "apt update && apt install -y build-essential strace gdb valgrind && bash"
```

This command will:
- Start an **Ubuntu 24.04** container with `SYS_PTRACE` capability enabled
- Mount your current project directory at `/workspace`
- Install `gcc`, `make`, the real `strace`, `gdb` and `valgrind`
- Drop you into an interactive shell ready to build and test

**Once inside the container:**

```bash
make
./ft_strace ls -la
./ft_strace ./segfault
./ft_strace cat /etc/hostname
```

### Compare against the real strace

Since `strace` is installed in the container, you can diff your output directly:

```bash
# Reference binary
strace ls -la 2>ref.txt

# Your implementation
./ft_strace ls -la 2>mine.txt

diff ref.txt mine.txt
```

### Debug with GDB

```bash
make debug
gdb ./ft_strace
(gdb) run ls -la
```

### Memory check with Valgrind

```bash
valgrind --leak-check=full ./ft_strace ls
```

---

## Notes

- Only the `ptrace` API is used — no reading from `/proc/<pid>/` directly
- The `PTRACE_SYSCALL` stop happens **twice** per syscall: once on entry (to read arguments) and once on exit (to read the return value)
- 32-bit detection relies on checking the `cs` register value from `PTRACE_GETREGS`
- Strings are truncated at 32 characters by default, matching real `strace` behaviour
- Using the system `strace` binary or its source code is strictly forbidden
