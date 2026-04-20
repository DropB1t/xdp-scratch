# xdp-scratch

eBPF / XDP example programs in C and C++.

---

## Structure

```
xdp-scratch/
├── Makefile
├── vmlinux.h               # generated — run `make vmlinux`
├── scripts/
│   └── gen_vmlinux.sh
├── bin/                    # compiled outputs (.o, binaries)
├── ip_cnt_kern.c
└── pkt_counter.cpp
```

---

## Prerequisites

```bash
sudo apt install -y clang llvm libelf-dev libbpf-dev libxdp-dev build-essential
```

> **WSL2:** the distro `bpftool` is version-mismatched. Build from source:
> ```bash
> git clone --recurse-submodules https://github.com/libbpf/bpftool.git
> cd bpftool/src && make -j$(nproc) && sudo make install
> ```

---

## Usage

```bash
make              # build everything → bin/
make clean        # remove bin/
make vmlinux      # regenerate vmlinux.h from the running kernel
```

---

## Examples

### pkt_counter

Counts IPv4 vs non-IPv4 packets via a BPF array map. The C++ loader attaches the program and polls the map every 2 seconds.

```bash
make
sudo ./bin/pkt_counter <interface> bin/ip_cnt_kern.o
# => ipv4 count: 42
```

---

## Adding a program

1. Drop `*_kern.c` / `.cpp` sources in the root.
2. Add corresponding targets in `Makefile` (outputs go to `bin/`).
3. Use `#include "vmlinux.h"` in BPF programs — `-I.` is already set.