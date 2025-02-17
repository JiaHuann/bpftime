# bpftime: Userspace eBPF runtime for fast Uprobe & Syscall Hook

[![Build and Test VM](https://github.com/eunomia-bpf/bpftime/actions/workflows/test-vm.yml/badge.svg)](https://github.com/eunomia-bpf/bpftime/actions/workflows/test-vm.yml)
[![Build and Test runtime](https://github.com/eunomia-bpf/bpftime/actions/workflows/runtime.yml/badge.svg)](https://github.com/eunomia-bpf/bpftime/actions/workflows/runtime.yml)
[![DOI](https://zenodo.org/badge/676866666.svg)](https://zenodo.org/badge/latestdoi/676866666)


`bpftime`, a full-featured, high-performance eBPF runtime designed to operate in userspace. It offers fast Uprobe and Syscall hook capabilities: Userspace uprobe can be **10x faster than kernel uprobe!** and can programmatically **hook all syscalls of a process** safely and efficiently.

> ⚠️ **Note**: `bpftime` is actively under development. It's at a very early stage and may contain bugs.
> The API or design might change in upcoming releases, and it's not yet recommended for production use. See our [roadmap](#roadmap) for details. We'd love to hear your feedback and suggestions! Please feel free to open an issue or [Contact us](#contact).

## Key Features

- **Uprobe and Syscall hooks based on binary rewriting**: Run eBPF programs in userspace, attaching them to Uprobes and Syscall tracepoints: **No mannual instrumentation or restart required!**. It can `trace`, `replace` or `patch` the execution of a function, `hook`, `filter` or `redirect` all syscalls of a process safely, and efficiently with an eBPF userspace runtime.
- **Performance**: Experience up to a 10x speedup in Uprobe overhead compared to kernel uprobe and uretprobe.
- **Interprocess eBPF Maps**: Implement userspace eBPF maps in shared userspace memory for summary aggregation or control plane communication.
- **Compatibility**: use existing eBPF toolchains like clang and libbpf to develop userspace eBPF without any modifications. Supporting CO-RE via BTF, and offering userspace host function access.
- **JIT Support**: Benefit from a cross-platform eBPF interpreter and a high-speed JIT compiler powered by LLVM. It also includes a handcrafted x86 JIT in C for limited resources.
- **No instrumentation**: Can inject eBPF runtime into any running process without the need for a restart or manual recompilation.

## Components

- [`vm`](vm): The eBPF VM and JIT for eBPF, you can choose from LLVM JIT and a simple JIT/interpreter based on ubpf. It can be built as a standalone library and integrated into other projects.
- [`runtime`](runtime): The userspace runtime for eBPF, including the syscall server and agent, attaching eBPF programs to Uprobes and Syscall tracepoints, and eBPF maps in shared memory.

## Quick Start

With `bpftime`, you can build eBPF applications using familiar tools like clang and libbpf, and execute them in userspace. For instance, the `malloc` eBPF program traces malloc calls using uprobe and aggregates the counts using a hash map.

You can refer to [documents/build-and-test.md](documents/build-and-test.md) for how to build the project.

To get started, you can build and run a libbpf based eBPF program starts with `bpftime` cli:

```console
make -C example/malloc # Build the eBPF program example
bpftime load ./example/malloc/malloc
```

In another shell, Run the target program with eBPF inside:

```console
$ bpftime start ./example/malloc/victim
Hello malloc!
malloc called from pid 250215
continue malloc...
malloc called from pid 250215
```

You can also dynamically attach the eBPF program with a running process:

```console
$ ./example/malloc/victim & echo $! # The pid is 101771
[1] 101771
101771
continue malloc...
continue malloc...
```

And attach to it:

```console
$ sudo bpftime attach 101771 # You may need to run make install in root
Inject: "/root/.bpftime/libbpftime-agent.so"
Successfully injected. ID: 1
```

You can see the output from original program:

```console
$ bpftime load ./example/malloc/malloc
...
12:44:35 
        pid=247299      malloc calls: 10
        pid=247322      malloc calls: 10
```

Alternatively, you can also run our sample eBPF program directly in the kernel eBPF, to see the similar output:

```console
$ sudo example/malloc/malloc
15:38:05
        pid=30415       malloc calls: 1079
        pid=30393       malloc calls: 203
        pid=29882       malloc calls: 1076
        pid=34809       malloc calls: 8
```

See [documents/usage.md](documents/usage.md) for more details.

## In-Depth

### **Examples & Use Cases**

We can use the bpftime userspace runtime for:

- `tracing userspace functions with uprobe`: Attach uprobe, uretprobe or all syscall tracepoints(currently x86 only) eBPF programs to a process or a group of processes:
  - [`malloc`](example/malloc): count the malloc calls in libc by pid. demonstrate how to use the userspace `uprobe` with basic `hashmap`.
- `tracing all syscalls with tracepoints`
  - [`opensnoop`](example/opensnoop): trace file open or close syscalls in a process. demonstrate how to use the userspace `syscall tracepoint` with `ring buffer` output.

More examples can be found in [example](example) dir.

### **How it Works**

Left: kernel eBPF | Right: userspace bpftime

![How it works](documents/bpftime.png)

The hook implementation is based on binary rewriting and the underly technique is inspired by:

- Userspace function hook: [frida-gum](https://github.com/frida/frida-gum)
- Syscall hooks: [zpoline: a system call hook mechanism based on binary rewriting](https://www.usenix.org/conference/atc23/presentation/yasukata)

see [documents/how-it-works.md](documents/how-it-works.md) for details.

### **Performance Benchmarks**

How is the performance of `userspace uprobe` compared to `kernel uprobes`?

| Probe/Tracepoint Types | Kernel (ns)  | Userspace (ns) | Insn Count |
|------------------------|-------------:|---------------:|---------------:|
| Uprobe                 | 3224.172760  | 314.569110     | 4    |
| Uretprobe              | 3996.799580  | 381.270270     | 2    |
| Syscall Tracepoint     | 151.82801    | 232.57691      | 4    |
| Embedding runtime      | Not avaliable |  110.008430   | 4    |

It can be attached to functions in running process just like the kernel uprobe does.

How is the performance of LLVM JIT/AOT compared to other eBPF userspace runtimes, native code or wasm runtimes?

![LLVM jit benchmark](https://github.com/eunomia-bpf/bpf-benchmark/raw/main/example-output/merged_execution_times.png?raw=true)

Across all tests, the LLVM JIT for bpftime consistently showcased superior performance. Both demonstrated high efficiency in integer computations (as seen in log2_int), complex mathematical operations (as observed in prime), and memory operations (evident in memcpy and strcmp). While they lead in performance across the board, each runtime exhibits unique strengths and weaknesses. These insights can be invaluable for users when choosing the most appropriate runtime for their specific use-cases.

see [github.com/eunomia-bpf/bpf-benchmark](https://github.com/eunomia-bpf/bpf-benchmark) for how we evaluate and details.

Hash map or ring buffer compared to kernel(TODO)

See [benchmark](benchmark) dir for detail performance benchmarks.

### Comparing with Kernel eBPF Runtime

- `bpftime` allows you to use `clang` and `libbpf` to build eBPF programs, and run them directly in this runtime. We have tested it with a libbpf version in [third_party/libbpf](third_party/libbpf).
- Some kernel helpers and kfuncs may not be available in userspace.
- It does not support direct access to kernel data structures or functions like `task_struct`.

Refer to [documents/available-features.md](documents/avaliable-features.md) for more details.

## Build and test

see [documents/build-and-test.md](documents/build-and-test.md) for details.

## Roadmap

`bpftime` is continuously evolving with more features in the pipeline:

- [X] ring buffer output support.
- [X] perf event output support.
- [ ] Figure out how to run transparently with kernel probe
- [ ] An AOT compiler for eBPF can be easily added based on the LLVM IR.
- [ ] more examples and usecases.
- [ ] More map types and distribution maps support.
- [ ] More program types support.

Stay tuned for more developments from this promising project! You can find `bpftime` on [GitHub](https://github.com/eunomia-bpf/bpftime).

## License

This project is licensed under the MIT License.

## Contact

<yunwei356@gmail.com>
