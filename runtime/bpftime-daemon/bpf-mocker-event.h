/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __SYSCALL_TRACER_H
#define __SYSCALL_TRACER_H

#define TASK_COMM_LEN 16
#define NAME_MAX 255
#define INVALID_UID ((uid_t)-1)

#define MAX_INSN_SIZE 16384

#define BPF_OBJ_NAME_LEN 16U

enum event_type {
	SYS_OPEN,
	SYS_BPF,
	SYS_PERF_EVENT_OPEN,
	BPF_PROG_LOAD_EVENT,
};

struct event {
	enum event_type type;

	// basic info
	int pid;
	char comm[TASK_COMM_LEN];

	union {
		struct {
			int ret;
			unsigned int bpf_cmd;
			union bpf_attr attr;
			unsigned int attr_size;
		} bpf_data;

		struct {
			int ret;
			int flags;
			char fname[NAME_MAX];
		} open_data;

		struct {
			int ret;
			struct perf_event_attr attr;
			int pid;
			int cpu;
		} perf_event_data;

		struct {
			unsigned int type;
			unsigned int insn_cnt;
			char prog_name[BPF_OBJ_NAME_LEN];
			unsigned int insns[MAX_INSN_SIZE];
		} bpf_loaded_prog;
	};
};

#endif /* __SYSCALL_TRACER_H */
