/* Private Includes ----------------------------------------------------------*/
#include "nemo_debug.h"

/*----------------------------------------------------------------------------*/
const char* errno2string(int no) {
	
	switch(no) {
		case E2BIG: return "E22BIG";
		case EACCES: return "EACCES";
		case EAGAIN: return "EAGAIN";
		case EBADF: return "EBADF";
		case EBADMSG: return "EBADMSG";
		case EBUSY: return "EBUSY";
		case ECANCELED: return "ECANCELED";
		case ECHILD: return "ECHILD";
		case EDEADLK: return "EDEADLK";
		case EDOM: return "EDOM";
		case EEXIST: return "EEXIST";
		case EFAULT: return "EFAULT";
		case EFBIG: return "EFBIG";
		case EINPROGRESS: return "EINPROGRESS";
		case EINTR: return "EINTR";
		case EINVAL: return "EINVAL";
		case EIO: return "EIO";
		case EISDIR: return "EISDIR";
		case EMFILE: return "EMFILE";
		case EMLINK: return "EMLINK";
		case EMSGSIZE: return "EMSGSIZE";
		case ENAMETOOLONG: return "ENAMETOOLONG";
		case ENFILE: return "ENFILE";
		case ENODEV: return "ENODEV";
		case ENOENT: return "ENOENT";
		case ENOEXEC: return "ENOEXEC";
		case ENOLCK: return "ENOLCK";
		case ENOMEM: return "ENOMEM";
		case ENOMSG: return "ENOMSG";
		case ENOSPC: return "ENOSPC";
		case ENOSYS: return "ENOSYS";
		case ENOTDIR: return "ENOTDIR";
		case ENOTEMPTY: return "ENOTEMPTY";
		case ENOTSUP: return "ENOTSUP";
		case ENOTTY: return "ENOTTY";
		case ENXIO: return "ENXIO";
		case EPERM: return "EPERM";
		case EPIPE: return "EPIPE";
		case ERANGE: return "ERANGE";
		case EROFS: return "EROFS";
		case ESPIPE: return "ESPIPE";
		case ESRCH: return "ESRCH";
		case ETIMEDOUT: return "ETIMEDOUT";
		case EXDEV: return "EXDEV";
		default: return "unknown error code";
	}
}
/*----------------------------------------------------------------------------*/
void not_implemented(FILE* log, const char *func) {
	
	fprintf( log, "%s is not implemented\n", func );
}
/*----------------------------------------------------------------------------*/
