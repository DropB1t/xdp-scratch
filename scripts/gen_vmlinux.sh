#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT="${1:-${REPO_ROOT}/vmlinux.h}"

if ! command -v bpftool &>/dev/null; then
    echo "[ERROR] bpftool not found in PATH." >&2
    exit 1
fi

if [[ ! -f /sys/kernel/btf/vmlinux ]]; then
    echo "[ERROR] /sys/kernel/btf/vmlinux not found (CONFIG_DEBUG_INFO_BTF=y required)." >&2
    exit 1
fi

bpftool btf dump file /sys/kernel/btf/vmlinux format c > "${OUTPUT}"
echo "[OK] vmlinux.h written to ${OUTPUT} ($(wc -l < "${OUTPUT}") lines)."
