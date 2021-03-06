/* kerors.h
 *
 * Kernel ONLY error codes
 */

#ifndef _KERNEL_KERRORS_H
#define _KERNEL_KERRORS_H

/* These are the old newos errors - we should be trying to remove these
 * in favour of the codes in posix/errno.h or even os/support/Errors.h
 */

/* General errors */
#define ERR_GENERAL              -1
#define ERR_IO_ERROR             EIO
#define ERR_TIMED_OUT            ETIMEDOUT
#define ERR_NOT_ALLOWED          EPERM
#define ERR_PERMISSION_DENIED    EACCES
#define ERR_INVALID_BINARY       ERR_GENERAL-7
#define ERR_INVALID_HANDLE       ERR_GENERAL-8
#define ERR_NO_MORE_HANDLES      ERR_GENERAL-9
#define ERR_UNIMPLEMENTED        ENOSYS
#define ERR_TOO_BIG              EDOM
#define ERR_NOT_FOUND            ERR_GENERAL-12
#define ERR_NOT_IMPLEMENTED_YET  ERR_GENERAL-13

/* Semaphore errors */
#define ERR_SEM_NOT_INTERRUPTABLE -1030


/* Tasker errors */
#define ERR_TASK_GENERAL         -2048
#define ERR_TASK_PROC_DELETED    ERR_TASK_GENERAL-1

/* VFS errors */
#define ERR_VFS_GENERAL          -3072
#define ERR_VFS_INVALID_FS       ERR_VFS_GENERAL-1
#define ERR_VFS_NOT_MOUNTPOINT   ERR_VFS_GENERAL-2
#define ERR_VFS_PATH_NOT_FOUND   ERR_VFS_GENERAL-3
#define ERR_VFS_INSUFFICIENT_BUF ENOBUFS
#define ERR_VFS_READONLY_FS      EROFS
#define ERR_VFS_ALREADY_EXISTS   EEXIST
#define ERR_VFS_FS_BUSY          ERR_VFS_GENERAL-7
#define ERR_VFS_FD_TABLE_FULL    ERR_VFS_GENERAL-8
#define ERR_VFS_CROSS_FS_RENAME  ERR_VFS_GENERAL-9
#define ERR_VFS_DIR_NOT_EMPTY    ERR_VFS_GENERAL-10
#define ERR_VFS_NOT_DIR          ENOTDIR
#define ERR_VFS_WRONG_STREAM_TYPE   ERR_VFS_GENERAL-12
#define ERR_VFS_ALREADY_MOUNTPOINT ERR_VFS_GENERAL-13

/* VM errors */
#define ERR_VM_GENERAL           -4096
#define ERR_VM_INVALID_ASPACE    ERR_VM_GENERAL-1
#define ERR_VM_INVALID_REGION    ERR_VM_GENERAL-2
#define ERR_VM_BAD_ADDRESS       ERR_VM_GENERAL-3
#define ERR_VM_PF_FATAL          ERR_VM_GENERAL-4
#define ERR_VM_PF_BAD_ADDRESS    ERR_VM_GENERAL-5
#define ERR_VM_PF_BAD_PERM       ERR_VM_GENERAL-6
#define ERR_VM_PAGE_NOT_PRESENT  ERR_VM_GENERAL-7
#define ERR_VM_NO_REGION_SLOT    ERR_VM_GENERAL-8
#define ERR_VM_WOULD_OVERCOMMIT  ERR_VM_GENERAL-9
#define ERR_VM_BAD_USER_MEMORY   ERR_VM_GENERAL-10

/* Elf errors */
#define ERR_ELF_GENERAL		  -5120
#define ERR_ELF_RESOLVING_SYMBOL  ERR_ELF_GENERAL -1

#endif /* _KERNEL_KERRORS_H */
