/*------------------------------------------------------------------------------
/ team.c
/
/ DESCRIPTION:
/	User-side team functions. Communicate with the kernel_server to carry
/	the required operation on a team.
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 17/2/2004
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/

/* System Includes -----------------------------------------------------------*/

/* Private Includes ----------------------------------------------------------*/
#include "kernel.h"
#include "libroot.h"

/* Debugging -----------------------------------------------------------------*/
#include "nemo_debug.h"
#if DEBUG_LIBROOT
	#define OUT(x...)	fprintf(LIBROOT_LOG, "libroot: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

/*----------------------------------------------------------------------------*/
status_t
get_team_usage_info(team_id tmid, int32 who, team_usage_info *tui)
{
	// TODO: implement get_team_usage_info
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
kill_team(team_id team)
{
	krnl_msg_t msg;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_TEAM_KILL;
	msg.team_msg.remote = _libroot_port_id;
	msg.team_msg.info.team = team;

	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.team_msg.error != B_OK)
		return msg.team_msg.error;

	/* all fine */
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
get_team_info(team_id team, team_info *info)
{
	krnl_msg_t msg;

	/* check parameters */
	if(info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_TEAM_INFO;
	msg.team_msg.remote = _libroot_port_id;
	msg.team_msg.info.team = team;

	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.team_msg.error != B_OK)
		return msg.team_msg.error;

	/* all fine, return team info */
	memcpy(info, &(msg.team_msg.info), sizeof(team_info));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
get_next_team_info(int32 *cookie, team_info *info)
{
	krnl_msg_t msg;

	/* check parameters */
	if(cookie == NULL || info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_TEAM_NEXT_INFO;
	msg.team_msg.remote = _libroot_port_id;
	msg.team_msg.cookie = *cookie;

	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.team_msg.error != B_OK)
		return msg.team_msg.error;

	/* all fine, return team info */
	*cookie = 1;
	memcpy(info, &(msg.team_msg.info), sizeof(team_info));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
