/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef KERNEL_ARCH_SMP_H
#define KERNEL_ARCH_SMP_H


#include <kernel.h>

struct kernel_args;


// must match MAX_BOOT_CPUS in platform_kernel_args.h
#define SMP_MAX_CPUS MAX_BOOT_CPUS

#ifdef __cplusplus
extern "C" {
#endif

int arch_smp_init(struct kernel_args *ka);
void arch_smp_send_ici(int target_cpu);
void arch_smp_send_broadcast_ici(void);

#ifdef __cplusplus
}
#endif

#endif	/* KERNEL_ARCH_SMP_H */
