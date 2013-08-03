#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <machine/trapframe.h>
/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

void sys_exit(int exitcode);
void sys_getpid(int *retval);
int sys_waitpid(int pid, int *status, int options, int *retval);
int sys_fork(struct trapframe *tf, int *retval);
int sys_open(const char *name, int flag, int32_t *err);
int sys_close (int filehandle, int32_t *err);
int sys_read(int filehandle, void *buf, size_t size, int32_t *err);
int sys_write(int filehandle, const void *buf, size_t size, int32_t *err);
int sys_execv(const char *progname, char**args);

#endif /* _SYSCALL_H_ */

