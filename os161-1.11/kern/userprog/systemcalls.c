#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <filedescriptor.h>
#include <uio.h>
#include <vfs.h>
#include <thread.h>
#include <curthread.h>
#include <process.h>    
#include <kern/unistd.h>
#include <addrspace.h>
#include <vm.h>
#include <synch.h>
#include <vnode.h>
#include "opt-A2.h"

//
//  systemcalls.c
//  
//
//  Created by Xiaoyu Kou on 12-11-10.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


int sys_fork(struct trapframe *tf, int *retval) {
    
//    kprintf("fork\n");

    struct lock *f_lock;
    struct trapframe *dup_tf;
    struct thread *child;
    
    f_lock = lock_create("fork_lock");
    
    if (tf == NULL) return (-1) * EINVAL;
    
    lock_acquire(f_lock);
//    kprintf("%d thread acquire, sysfork\n", curthread->pid);

    
    
    dup_tf = kmalloc(sizeof (struct trapframe));
    if (dup_tf == NULL) {
        lock_release(f_lock);
        return (-1) * ENOMEM;
    }
    
    memcpy(dup_tf, tf, sizeof(struct trapframe));
    


    int result = thread_fork((const char*)curthread->t_name, dup_tf, 0, md_forkentry, &child);
    
    
    if (result) {
        lock_release(f_lock);
        return result;
    }
    
    *retval = child->pid;
    
    
    lock_release(f_lock);
//    kprintf("%d thread release, sysfork\n", curthread->pid);

    
    
    
    return 0;
    
}

void
md_forkentry(struct trapframe *tf)
{
#if OPT_A2
    
    struct trapframe dup_tf;
    
    memcpy(&dup_tf, tf, sizeof(struct trapframe));
    
    kfree(tf);
    
    as_activate(curthread->t_vmspace);
    
    dup_tf.tf_epc = dup_tf.tf_epc + 4;
    dup_tf.tf_v0 = 0;
    dup_tf.tf_a3 = 0;
    
//    kprintf("entry\n");

    
    mips_usermode(&dup_tf);
    

    
#else
	/*
	 * This function is provided as a reminder. You need to write
	 * both it and the code that calls it.
	 *
	 * Thus, you can trash it and do things another way if you prefer.
	 */
    
	(void)tf;
#endif
}


void sys_exit(int exitcode){

    int curpid = curthread->pid;
    process_exit(curpid, exitcode);
    
}

int sys_waitpid(int pid, int *status, int options, int *retval) {
    

    
    if(pid == curthread->pid || options != 0 || pid < 0 || pid > array_getnum(processTable)) {
        return (-1) * EINVAL;
    }
    
    if(status == NULL) {
        return (-1) * EFAULT;
    }
        
    struct process *child_proc;
    struct lock *child_lock;
    struct cv *child_cv;
    
    child_proc = array_getguy(processTable, pid);
    child_lock = child_proc->p_lock;
    child_cv = child_proc->p_cv;
    
    if(child_proc->parent_pid != curthread->pid){
		return EINVAL; 
    }
    
    lock_acquire(child_lock);
//    kprintf("%d thread acquire, waitpid\n", curthread->pid);


    while(child_proc->exitStatus == 0) {
//        kprintf("wait: %d\n", pid);
        cv_wait(child_cv, child_lock);
    }
    
//    kprintf("waitpid\n");

    *status = child_proc->exitCode;
    *retval = pid;
    
    lock_release(child_lock);
    
//    kprintf("%d thread release, waitpid\n", curthread->pid);

    
    return 0;
    
}


void sys_getpid(int *retval){

    *retval = curthread->pid;
    
}

// OPEN
int sys_open(const char *name, int flag, int32_t *err) {
    if(name == NULL) {

        return (-1) * EFAULT;
    }
    struct filedescriptor *fd = kmalloc(sizeof(struct filedescriptor));
    struct vnode *vnode;
    char *filename = NULL;
    filename = kstrdup(name);
    if(filename == NULL) {

        return (-1) * ENOMEM;
    }
    int ans = vfs_open(filename, flag, &vnode);
    kfree(filename);
    fd->file_ID = get_new_ID(curthread->dt);
    fd->flag = flag;
    fd->offset = 0;
    fd->vnode = vnode;
    ans = add_one(fd, curthread->dt);
    if(ans) {

        return (-1) * ENOMEM;
    }
    *err = fd->file_ID;
    return 0;
}

// CLOSE
int sys_close(int filehandle, int32_t *err) {
    if (filehandle < 0) {
        return (-1) * EBADF;
    }
    struct filedescriptor *fd;
    int index = 0;
    index = search(filehandle, curthread->dt);
    if (index < 0) {
        return (-1) * EBADF;
    }
    fd = array_getguy(curthread->dt->table, index); // find correspond fd
    int result = delete_one(filehandle, curthread->dt);
    *err = 0;
    return 0;
}

// READ
int sys_read(int filehandle, void *buf, size_t size, int32_t *err) {
    
    if (buf == NULL) {

        return (-1) * EFAULT;
    }
    struct uio user_read;
    struct filedescriptor *fd = NULL;
    int index = 0;
    index = search(filehandle, curthread->dt);
    if (index < 0) {
        return (-1) * EBADF;
    }
    fd = array_getguy(curthread->dt->table, filehandle); //  find correspond fd
    void *kbuf = kmalloc(size + 1);
    
    mk_kuio(&user_read, kbuf, size, fd->offset, UIO_READ);    // initialize UIO
    
    int read = VOP_READ(fd->vnode, &user_read);

    fd->offset = user_read.uio_offset;  // memorize
    int result = copyout(kbuf, (const_userptr_t)buf, size - user_read.uio_resid + 1);
    //int stuff = user_read.uio_offset - fd->offset;  // things been readed
    kfree(kbuf);
//    kprintf("WHAT I HAVE HERE  %d", stuff);
    *err = size-user_read.uio_resid;
    return 0;
}

// WRITE
int sys_write(int filehandle, const void *buf, size_t size, int32_t *err) {

    if (buf == NULL) {
        return (-1) * EFAULT;
    }
    struct uio user_write;
    struct filedescriptor *fd = NULL;
    int index = 0;
    index = search(filehandle, curthread->dt);
    if (index < 0) {
        return (-1) * EBADF;
    }
    fd = array_getguy(curthread->dt->table, index); // find correspond fd
    void *kbuf = kmalloc(size + 1);
    int result = copyin((const_userptr_t)buf, kbuf, (size + 1));
    mk_kuio(&user_write, kbuf, size, fd->offset, UIO_WRITE);   // initialize UIO 
    
    int write = VOP_WRITE(fd->vnode, &user_write);
//    int stuff = user_write.uio_offset - fd->offset; // things been written
    fd->offset = user_write.uio_offset; //  memorize
    kfree(kbuf);
    *err = size - user_write.uio_resid;
    return 0;
}
