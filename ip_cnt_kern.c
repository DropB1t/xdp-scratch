#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

/* Ethernet */
#define ETH_P_IP   0x0800
#define ETH_P_IPV6 0x86DD
#define ETH_P_ARP  0x0806

/* IP protocols */
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_ICMP 1

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, __u32);
    __type(value, __u64);
    __uint(max_entries, 2);
} ip_count_map SEC(".maps");

SEC("xdp_count_ipv4")
int count_ipv4(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth = data;

    if (data + sizeof(struct ethhdr) > data_end) {
        return XDP_PASS;
    }

    __u16 ethertype = eth->h_proto;

    if (ethertype != bpf_htons(ETH_P_IP)) {
        bpf_printk("received ip packet");
        // count ipv4 packets
        __u32 key = 0;
        __u64 *value = bpf_map_lookup_elem(&ip_count_map, &key);
        if (value) {
            __sync_fetch_and_add(value, 1);
        }
    } else {
        __u32 key = 1;
        __u64 *value = bpf_map_lookup_elem(&ip_count_map, &key);
        if (value) {
            __sync_fetch_and_add(value, 1);
        }
    }

    return XDP_PASS;
}

char LICENSE[] SEC("license") = "GPL";
