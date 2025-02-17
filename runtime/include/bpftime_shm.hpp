#ifndef BPFTIME_SHM_CPP_H
#define BPFTIME_SHM_CPP_H

#include "common/bpftime_config.hpp"
#include <boost/interprocess/interprocess_fwd.hpp>
#include <cstddef>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <ebpf-vm.h>
#include <sys/epoll.h>

namespace bpftime
{

struct bpf_map_attr {
	int type = 0;
	uint32_t key_size = 0;
	uint32_t value_size = 0;
	uint32_t max_ents = 0;
	uint64_t flags = 0;
	uint32_t ifindex = 0;
	uint32_t btf_vmlinux_value_type_id = 0;
	uint32_t btf_id = 0;
	uint32_t btf_key_type_id = 0;
	uint32_t btf_value_type_id = 0;
	uint64_t map_extra = 0;
};

enum class shm_open_type {
	SHM_SERVER,
	SHM_CLIENT,
	SHM_NO_CREATE,
};

extern const shm_open_type global_shm_open_type;

bpftime::agent_config &bpftime_get_agent_config();
} // namespace bpftime

extern "C" {
int bpftime_link_create(int prog_fd, int target_fd);
int bpftime_progs_create(const ebpf_inst *insn, size_t insn_cnt,
			 const char *prog_name, int prog_type);

int bpftime_maps_create(const char *name, bpftime::bpf_map_attr attr);

int bpftime_map_get_info(int fd, bpftime::bpf_map_attr *out_attr,
			 const char **out_name, int *type);

uint32_t bpftime_map_value_size_from_syscall(int fd);
int bpftime_map_get_next_key_from_helper(int fd, const void *key,
					 void *next_key);
const void *bpftime_map_lookup_elem_from_helper(int fd, const void *key);
long bpftime_map_update_elem_from_helper(int fd, const void *key,
					 const void *value, uint64_t flags);
long bpftime_map_delete_elem_from_helper(int fd, const void *key);

int bpftime_map_get_next_key_from_syscall(int fd, const void *key,
					 void *next_key);
const void *bpftime_map_lookup_elem_from_syscall(int fd, const void *key);
long bpftime_map_update_elem_from_syscall(int fd, const void *key,
					 const void *value, uint64_t flags);
long bpftime_map_delete_elem_from_syscall(int fd, const void *key);


int bpftime_uprobe_create(int pid, const char *name, uint64_t offset,
			  bool retprobe, size_t ref_ctr_off);
int bpftime_tracepoint_create(int pid, int32_t tp_id);
int bpftime_perf_event_enable(int fd);
int bpftime_perf_event_disable(int fd);
int bpftime_attach_perf_to_bpf(int perf_fd, int bpf_fd);
int bpftime_add_ringbuf_fd_to_epoll(int ringbuf_fd, int epoll_fd,
				    epoll_data_t extra_data);
int bpftime_add_software_perf_event_fd_to_epoll(int swpe_fd, int epoll_fd,
						epoll_data_t extra_data);
int bpftime_epoll_create();
int bpftime_is_ringbuf_map(int fd);
void *bpftime_get_ringbuf_consumer_page(int ringbuf_fd);
void *bpftime_get_ringbuf_producer_page(int ringbuf_fd);
int bpftime_is_array_map(int fd);
int bpftime_is_epoll_handler(int fd);
void *bpftime_get_array_map_raw_data(int fd);
void bpftime_close(int fd);
void *bpftime_ringbuf_reserve(int fd, uint64_t size);
void bpftime_ringbuf_submit(int fd, void *data, int discard);
int bpftime_epoll_wait(int fd, struct epoll_event *out_evts, int max_evt,
		       int timeout);
int bpftime_add_software_perf_event(int cpu, int32_t sample_type,
				    int64_t config);
int bpftime_is_software_perf_event(int fd);
void *bpftime_get_software_perf_event_raw_buffer(int fd, size_t expected_size);
int bpftime_perf_event_output(int fd, const void *buf, size_t sz);
}

#endif // BPFTIME_SHM_CPP_H
