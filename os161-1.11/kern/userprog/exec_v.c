#include <kern/errno.h>
#include <kern/unistd.h>
#include <types.h>
#include <uio.h>
#include <syscall.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <vfs.h>
#include <vnode.h>
#include <array.h>
#include <lib.h>
#include <machine/trapframe.h>
#include <addrspace.h>
#include "opt-A2.h"



// calculate the length of the arguments string.


// calculate the number of arguments.

int getargc(char** args) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }
    return i;
}

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */

int
sys_execv(const char *progname, char**args) {
    kprintf("hello world");
    struct vnode *v;
    vaddr_t entrypoint, stackptr;
    int result;
    int argc = getargc(args);

    /* Copy the arguments into the kernel buffer */
    char** kbuffer = kmalloc(argc*sizeof(char*));
    int i;
    for (i=0; i<argc; i++) {
        kbuffer[i] = kmalloc(strlen(args[i])*sizeof(char*)+1);
        copyinstr(args[i], kbuffer[i], strlen(args[i])*sizeof(char*)+1, NULL);
    }
    
    args = kbuffer;
    
    /* Open the file. */
    result = vfs_open(progname, O_RDONLY, &v);
    if (result) {
        return result;
    }

    /* This is not the first process, we have to make sure
     the previous process address space is not NULL. */
    assert(curthread->t_vmspace != NULL);

    /* Destroy the old process address space */
    as_destroy(curthread->t_vmspace);

    /* Create a new address space. */
    curthread->t_vmspace = as_create();
    if (curthread->t_vmspace == NULL) {
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
    /* Free the current process user stack */
    kfree(curthread->t_stack);

    /* Define the user stack in the address space */
    result = as_define_stack(curthread->t_vmspace, &stackptr);
    if (result) {
        /* thread_exit destroys curthread->t_vmspace */
        return result;
    }
    
    /* copy arguments to user space */
    char **args_u; /* user space arguments.(array of ptrs
                   to string arguments.*/
    /* string length of the arguments */
    size_t argstrlen = getlen(argc, args);
    /* address space for the string arguments value. */
    char* arg_str = (char *) stackptr - argstrlen;
    /* address space for the pointers to string arguments. */
    args_u = (char **) arg_str - (argc + 1) * sizeof (char**);
    /* adjust the address so that its divisable by 4 */
    args_u = (int) args_u - (int) args_u % 4;

    /* copy the arguments to the user address space */
    int len;
    for (i = 0; i < argc; i++) {
        /* copy a single argument  to the user address space */
        copyoutstr(args[i], (userptr_t) arg_str, strlen(args[i]) + 1, &len);
        /* set the user argument to the current argument string pointer */
        args_u[i] = arg_str;
        /* increment the argument pointer to the next argument */
        arg_str += (strlen(args[i]) + 1) * sizeof (char);
    }
    /* set the n+1th argument to be NULL */
    args_u[argc] = NULL;
    /* set the stackptr to the starting point of args_u and adjust the stack pointer */
    stackptr = args_u - sizeof (char**) - ((int)args_u%8);

    /* Warp to user mode. */
    md_usermode(argc, (userptr_t) args_u, stackptr, entrypoint);
    /*********************** end of A2 stuff *****************/

    /* md_usermode does not return */
    panic("md_usermode returned\n");
    return EINVAL;
}