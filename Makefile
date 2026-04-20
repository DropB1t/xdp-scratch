# =============================================================================
# xdp-scratch — Makefile
#
# All source files live in the repository root.
# All build artefacts are written to bin/.
#
# Usage:
#   make              build everything
#   make clean        remove bin/
#   make vmlinux      regenerate vmlinux.h from the running kernel
# =============================================================================

CC  := clang
CXX := clang++

# ── Userspace flags ───────────────────────────────────────────────────────────
CXX_FLAGS := -std=c++17 -Wall -Wextra

# ── BPF kernel-object flags ───────────────────────────────────────────────────
#   -O2          required by the BPF verifier (unoptimised code is rejected)
#   -g           embed BTF / DWARF debug info so bpftool can introspect the obj
#   -target bpf  cross-compile for the BPF virtual machine
#   -I.          find vmlinux.h in the repository root
BPF_CFLAGS := -O2 -g -target bpf -I.

BIN := bin

# ── Targets ───────────────────────────────────────────────────────────────────
.PHONY: all clean vmlinux

all: $(BIN)/pkt_counter $(BIN)/ip_cnt_kern.o

# Userspace loader
$(BIN)/pkt_counter: pkt_counter.cpp | $(BIN)
	$(CXX) $(CXX_FLAGS) $< -lbpf -lxdp -o $@

# BPF kernel program
$(BIN)/ip_cnt_kern.o: ip_cnt_kern.c vmlinux.h | $(BIN)
	$(CC) $(BPF_CFLAGS) -c $< -o $@

# Create bin/ if it does not exist yet
$(BIN):
	mkdir -p $(BIN)

# ── Housekeeping ──────────────────────────────────────────────────────────────
clean:
	rm -rf $(BIN)

# Regenerate vmlinux.h from the currently running kernel's BTF data.
# Requires bpftool (see README for WSL2 build instructions).
vmlinux:
	@echo "  GEN    vmlinux.h  (kernel: $$(uname -r))"
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
	@echo "  Done — vmlinux.h written to $(CURDIR)/vmlinux.h"
