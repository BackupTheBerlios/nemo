/*------------------------------------------------------------------------------
/ kernel.h
/
/ Description:
/	function prototypes and data types for the kernel_server code.
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 31/1/2004
/-----------------------------------------------------------------------------*/

#ifndef _KERNEL_H
#define _KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Standard Includes ---------------------------------------------------------*/

/* System Includes -----------------------------------------------------------*/
#include <OS.h>

/* Private Includes ----------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
	/* union semun is defined by including <sys/sem.h> */
#else
	/* according to X/OPEN we have to define it ourselves */
	union semun {
		int val;				/* value for SETVAL */
		struct semid_ds *buf;	/* buffer for IPC_STAT, IPC_SET */
		unsigned short *array;	/* array for GETALL, SETALL */
								/* Linux specific part: */
		struct seminfo *__buf;	/* buffer for IPC_INFO */
	};
#endif

/* Globals -------------------------------------------------------------------*/
#define P_KRNL_FN_TYPE_MASK	0xFFFF0000
#define P_KRNL_FN_CODE_MASK	0x0000FFFF

/* must all be positive */
enum {
    P_KRNL_QUIT		=	0x40000000,
    P_KRNL_AREA		=	0x20000000,
    P_KRNL_PORT		=	0x10000000,
    P_KRNL_SEM		=	0x08000000,
    P_KRNL_TEAM		=	0x04000000,
    P_KRNL_THREAD	=	0x02000000,
    P_KRNL_TIME		=	0x01000000,
    P_KRNL_ALARM	=	0x00800000,
    P_KRNL_DBG		=	0x00400000,
    P_KRNL_IMAGE	=	0x00200000,
    P_KRNL_SYSTEM	=	0x00100000,
    P_KRNL_REPLY	=	0x00080000,
    P_KRNL_RSV1		=	0x00040000,
    P_KRNL_RSV2		=	0x00020000,
    P_KRNL_RSV3		=	0x00010000
};

/* Areas ---------------------------------------------------------------------*/
typedef struct sys_area_info_t {
	area_info		info;
	struct sys_area_info_t *next;
} sys_area_info_t;
extern sys_area_info_t *_area_list;

/* Ports ---------------------------------------------------------------------*/
/* Port function codes */
enum {
    P_KRNL_PORT_CREATE		=	P_KRNL_PORT|0x0000,
    P_KRNL_PORT_FIND		=	P_KRNL_PORT|0x0001,
    P_KRNL_PORT_CLOSE		=	P_KRNL_PORT|0x0002,
    P_KRNL_PORT_DELETE		=	P_KRNL_PORT|0x0004,
    P_KRNL_PORT_OWNER		=	P_KRNL_PORT|0x0008,
    P_KRNL_PORT_INFO		=	P_KRNL_PORT|0x0010,
    P_KRNL_PORT_NEXT_INFO	=	P_KRNL_PORT|0x0020,
    /* not used: */
    P_KRNL_PORT_READ		=	P_KRNL_PORT|0x0040,
    P_KRNL_PORT_WRITE		=	P_KRNL_PORT|0x0080,
    P_KRNL_PORT_BUF_SIZE	=	P_KRNL_PORT|0x0100,
    P_KRNL_PORT_COUNT		=	P_KRNL_PORT|0x0200
};

typedef struct sys_port_info_t {
	port_info		info;
	struct sys_port_info_t	*next;
} sys_port_info_t;
extern sys_port_info_t *_port_list;

typedef struct krnl_port_msg_t {
	int32		cmd;
	port_id		remote;
	status_t	error;
	int32		cookie;

	port_info	info;
} krnl_port_msg_t;

status_t sys_create_port		(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_find_port			(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_close_port			(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_delete_port		(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_set_port_owner		(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_get_port_info		(krnl_port_msg_t *msg, krnl_port_msg_t *reply);
status_t sys_get_next_port_info	(krnl_port_msg_t *msg, krnl_port_msg_t *reply);

/* Semaphores ----------------------------------------------------------------*/
/* Semaphore function codes */
enum {
    P_KRNL_SEM_CREATE		=	P_KRNL_SEM|0x0000,
    P_KRNL_SEM_DELETE		=	P_KRNL_SEM|0x0001,
    P_KRNL_SEM_OWNER		=	P_KRNL_SEM|0x0002,
    P_KRNL_SEM_INFO			=	P_KRNL_SEM|0x0004,
    P_KRNL_SEM_NEXT_INFO	=	P_KRNL_SEM|0x0008,
    /* not used: */
    P_KRNL_SEM_ACQUIRE		=	P_KRNL_SEM|0x0010,
    P_KRNL_SEM_RELEASE		=	P_KRNL_SEM|0x0020,
    P_KRNL_SEM_COUNT		=	P_KRNL_SEM|0x0040
};

typedef struct sys_sem_info_t {
	sem_info		info;
	struct sys_sem_info_t	*next;
} sys_sem_info_t;
extern sys_sem_info_t *_sem_list;

typedef struct krnl_sem_msg_t {
	int32		cmd;
	sem_id		remote;
	status_t	error;
	int32		cookie;

	sem_info	info;
} krnl_sem_msg_t;

status_t sys_create_sem			(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_delete_sem			(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_acquire_sem		(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_release_sem		(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_get_sem_count		(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_set_sem_owner		(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_get_sem_info		(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);
status_t sys_get_next_sem_info	(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply);

/* Teams ---------------------------------------------------------------------*/
/* Team function codes */
enum {
    P_KRNL_TEAM_ADD			=	P_KRNL_TEAM|0x0000,
    P_KRNL_TEAM_DELETE		=	P_KRNL_TEAM|0x0001,
    P_KRNL_TEAM_KILL		=	P_KRNL_TEAM|0x0002,
    P_KRNL_TEAM_INFO		=	P_KRNL_TEAM|0x0004,
    P_KRNL_TEAM_NEXT_INFO	=	P_KRNL_TEAM|0x0008,
    P_KRNL_TEAM_USAGE		=	P_KRNL_TEAM|0x0010,
};

typedef struct sys_team_info_t {
	team_info		info;
	thread_id		_main_thread;
	port_id			_libroot_port;
	sem_id			_libroot_thread_sem;
	struct sys_team_info_t	*next;
} sys_team_info_t;
extern sys_team_info_t *_team_list;

typedef struct krnl_team_msg_t {
	int32		cmd;
	port_id		remote;
	status_t	error;
	int32		cookie;

	int32		who;
	team_info	info;
	team_usage_info
				usage_info;

	port_id		_libroot_port;
	sem_id		_libroot_thread_sem;
} krnl_team_msg_t;

status_t sys_add_team			(krnl_team_msg_t *msg, krnl_team_msg_t *reply);
status_t sys_delete_team		(krnl_team_msg_t *msg, krnl_team_msg_t *reply);
status_t sys_kill_team			(krnl_team_msg_t *msg, krnl_team_msg_t *reply);
status_t sys_get_team_info		(krnl_team_msg_t *msg, krnl_team_msg_t *reply);
status_t sys_get_next_team_info	(krnl_team_msg_t *msg, krnl_team_msg_t *reply);
status_t sys_get_team_usage_info(krnl_team_msg_t *msg, krnl_team_msg_t *reply);

/* Threads -------------------------------------------------------------------*/

/* Time ----------------------------------------------------------------------*/

/* Alarm ---------------------------------------------------------------------*/

/* Debugger ------------------------------------------------------------------*/

/* Images --------------------------------------------------------------------*/

/* System Information --------------------------------------------------------*/

/* Misc Functions ------------------------------------------------------------*/

/* Private Functions & Types -------------------------------------------------*/
#define P_KRNL_PORT_KEY		((key_t)'_KSP')

typedef union krnl_msg_t {
	int32	cmd;

	/*krnl_area_msg_t	area_msg;*/
	struct krnl_port_msg_t	port_msg;
	struct krnl_sem_msg_t	sem_msg;
	struct krnl_team_msg_t	team_msg;
} krnl_msg_t;

extern int _area_list_sem;
extern int _port_list_sem;
extern int _sem_list_sem;
extern int _team_list_sem;

status_t	init(void);
void		cleanup(void);
status_t	listen(void);
void		sig_catcher(int);

status_t	acquire_sem_priv(int semid);
status_t	release_sem_priv(int semid);

	void	handle_area_msg		(krnl_msg_t *msg);
	void	handle_port_msg		(krnl_msg_t *msg);
	void	handle_sem_msg		(krnl_msg_t *msg);
	void	handle_team_msg		(krnl_msg_t *msg);
	void	handle_thread_msg	(krnl_msg_t *msg);
	void	handle_time_msg		(krnl_msg_t *msg);
	void	handle_alarm_msg	(krnl_msg_t *msg);
	void	handle_debug_msg	(krnl_msg_t *msg);
	void	handle_image_msg	(krnl_msg_t *msg);
	void	handle_system_msg	(krnl_msg_t *msg);
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_H */
