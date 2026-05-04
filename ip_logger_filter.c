#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

struct ip_pair {
 __u32 src; // network byte order
 __u32 dst; // network byte order
};

struct {
 __uint(type, BPF_MAP_TYPE_RINGBUF);
 __uint(max_entries, 1 << 22); // 4 MiB ringbuf (tune as needed)
} ring_buf SEC(".maps");

SEC("xdp")
int xdp_ip_pairs(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

    // Ethernet
    struct ethhdr *eth_header = data;
    if ((void *)(eth_header+1) > data_end)
        return XDP_PASS;

    if (eth_header->h_proto != bpf_ntohs(ETH_P_IP))
        return XDP_PASS;

    // IPv4
    struct iphdr *ip = (void *)(eth_header + 1);
    if ((void *)(ip + 1) > data_end)
        return XDP_PASS;

    // Validate IHL (header length)
    __u32 header_len_bytes = ip->ihl * 4;

    if (header_len_bytes < sizeof(*ip))
     return XDP_PASS;

    if ((void *)ip + header_len_bytes > data_end)
        return XDP_PASS;

    // Reserve event
    struct ip_pair *e = bpf_ringbuf_reserve(&ring_buf, sizeof(*e), 0);
    if (!e) return XDP_PASS; // drop event if buffer full

    e->src = ip->saddr;
    e->dst = ip->daddr;
    bpf_ringbuf_submit(e, 0);

    return XDP_PASS;
}

char LICENSE[] SEC("license") = "GPL";
