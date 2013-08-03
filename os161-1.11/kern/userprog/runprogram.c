/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include "opt-A2.h"
#include "opt-A3.h"

#if OPT_A2
// calculate the length of the arguments string.
size_t getlen(int argc, char** args) {
    size_t len = 0;
    int i;
    for (i=0; i<argc; i++){
        len += strlen(args[i]);
        len++;
    }
    return len;
}
#endif

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
#if OPT_A2
int
runprogram(char *progname, unsigned int argc, char**args)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
    
	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}
    
	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);
    
	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}
    
	/* Activate it. */
	as_activate(curthread->t_vmspace);
    
	/* Load the executable. */
#if OPT_A3
    curthread->t_vmspace->elf_file = kstrdup(progname);
#endif
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}
    
	/* Done with the file now. */
	vfs_close(v);
    
	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
    
    /*********************** stuff for A2 *******************/
    char **args_u; /* user space arguments.(array of ptrs
                   to string arguments.*/
    /* string length of the arguments */
    size_t argstrlen = getlen(argc, args);
    /* address space for the string arguments value. */
    char *arg_str = (char *)stackptr - argstrlen;
    /* address space for the pointers to string arguments. */
    args_u = (char **)arg_str - (argc+1)*sizeof(char**);
    /* adjust the address so that its divisable by 4 */
    args_u = (int)args_u - (int)args_u%4;
    
    /* copy the arguments to the user address space */
    int len;
    int i;
    for (i=0; i<argc; i++){
        /* copy a single argument  to the user address space */
        copyoutstr(args[i], (userptr_t)arg_str, strlen(args[i])+1, &len);
        /* set the user argument to the current argument string pointer */
        args_u[i] = arg_str;
        /* increment the argument pointer to the next argument */
        arg_str += (strlen(args[i])+1)*sizeof(char);
    }
    /* set the n+1th argument to be NULL */
    args_u[argc] = NULL;
    /* set the stackptr to the starting point of args_u and adjust the stack pointer */
    stackptr = args_u - sizeof(char**) - ((int)args_u%8);
    
    /* Warp to user mode. */
    md_usermode(argc, (userptr_t) args_u, stackptr, entrypoint);
    /*********************** end of A2 stuff *****************/
    
    /* md_usermode does not return */
    panic("md_usermode returned\n");
    return EINVAL;
}
#else
int
runprogram(char *progname)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}

	/* Warp to user mode. */
	md_usermode(0 /*argc*/, NULL /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}
#endif
