/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef KERNEL_ARCH_THREAD_H
#define KERNEL_ARCH_THREAD_H

#include <thread.h>

int arch_team_init_team_struct(struct team *t, bool kernel);
int arch_thread_init_thread_struct(struct thread *t);
void arch_thread_init_tls(struct thread *thread);
void arch_thread_context_switch(struct thread *t_from, struct thread *t_to);
int arch_thread_init_kthread_stack(struct thread *t, int (*start_func)(void), void (*entry_func)(void), void (*exit_func)(void));
void arch_thread_dump_info(void *info);
void arch_thread_enter_uspace(struct thread *t, addr entry, void *args1, void *args2);
void arch_thread_switch_kstack_and_call(struct thread *t, addr new_kstack, void (*func)(void *), void *arg);

// ToDo: doing this this way is an ugly hack - please fix me!
//		(those functions are "static inline" for x86 - since
//		"extern inline" doesn't work for "gcc -g"...)
#ifndef ARCH_x86
struct thread *arch_thread_get_current_thread(void);
void arch_thread_set_current_thread(struct thread *t);
#endif

void arch_setup_signal_frame(struct thread *t, struct sigaction *sa, int sig, int sig_mask);
int64 arch_restore_signal_frame(void);
void arch_check_syscall_restart(struct thread *t);

// for any inline overrides
#include <arch_thread.h>

#endif	/* KERNEL_ARCH_THREAD_H */
