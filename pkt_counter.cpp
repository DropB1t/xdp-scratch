#include <sys/types.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <xdp/libxdp.h>

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
    if (level < LIBBPF_WARN)   // suppress INFO; keep WARN + ERR
        return 0;
    return vfprintf(stderr, format, args);
}

int main(int argc, char **argv) {

    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <interface> <program>" << std::endl;
        return EXIT_FAILURE;
    }

    //libbpf_set_print(libbpf_print_fn);
    libbpf_set_print(nullptr);

    const char *ifname = argv[1];
    const char *prog_path = argv[2];

    struct xdp_program *prog = xdp_program__open_file(prog_path, "xdp_count_ipv4", nullptr);
    if (libbpf_get_error(prog) != 0) {
        std::cerr << "Failed to open program: " << prog_path << std::endl;
        return EXIT_FAILURE;
    }

    int if_index = if_nametoindex(ifname);
    if (if_index == 0) {
        std::cerr << "Failed to get interface index for " << ifname << std::endl;
        return EXIT_FAILURE;
    }

    auto ret = xdp_program__attach(prog, if_index, XDP_MODE_NATIVE, 0);
    if (ret) {
        std::cerr << "Failed to attach to interface: " << ifname <<  std::endl;
        return EXIT_FAILURE;
    }

    auto bpf_obj = xdp_program__bpf_obj(prog);

    if (libbpf_get_error(bpf_obj) != 0) {
        std::cerr << "Failed to get BPF object" << std::endl;
        return EXIT_FAILURE;
    }

    int map_fd = bpf_object__find_map_fd_by_name(bpf_obj, "ip_count_map");

    if (map_fd < 0) {
        std::cerr << "Failed to find map ip_count_map" << std::endl;
        return EXIT_FAILURE;
    }

    __u32 key = 0;
    __u64 value;
    for (;;) {
        if (bpf_map_lookup_elem(map_fd, &key, &value) < 0) {
            std::cerr << "Failed to lookup map: " << strerror(errno) << std::endl;
            return 1;
        }
        std::cout << "=> ipv4 count: " << value << std::endl;
        sleep(2);
    }

    return EXIT_SUCCESS;
}
