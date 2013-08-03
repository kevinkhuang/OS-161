#include <types.h>
#include <lib.h>
#include <kern/errno.h>
#include <array.h>
#include <machine/spl.h>
#include <machine/pcb.h>
#include <thread.h>
#include <curthread.h>
#include <scheduler.h>
#include <addrspace.h>
#include <process.h>
#include <vnode.h>


// create and initialize the table array includes all processes
void create_process_table(void) {
    
    struct process *first_process;
    processTable = array_create();
    if (processTable == NULL) {
        kprintf("Cannot create process table!\n");
    }
    first_process = create_process(1);
    curthread->pid = 1;
    first_process->myThread = curthread;
    
    
    int result = array_preallocate(processTable, 1);
    if (result) {
        kfree(first_process);
        kfree(processTable);
        
    }
    array_add(processTable, first_process);
    
    
}

// delete the process table and free the memory allocated
void delete_process_table(void) {
    
//    kprintf("shut\n");
    int num = array_getnum(processTable);
    int i;
    for (i = 1; i < num; i++) {
        delete_process(array_getguy(processTable, i));
    }
    
    array_destroy(processTable);
    processTable = NULL;
}

// create a process
struct process *create_process(int pid) {
    struct process *new_proc;
    new_proc = kmalloc(sizeof (struct process));
    if (new_proc == NULL) {
        return NULL;
    }
    
    new_proc->pid = pid;
    new_proc->parent_pid = -1;
    new_proc->exitStatus = 0;
    new_proc->exitCode = -1;
    new_proc->myThread = NULL;
    
    new_proc->p_lock = lock_create("process_lock");
    
    if (new_proc->p_lock == NULL) {
        kfree(new_proc);
        return -1;
    }
    
    new_proc->p_cv = cv_create("process_cv");
    
    if (new_proc->p_cv == NULL) {
        kfree(new_proc);
        return -1;
    }
    
    return new_proc;
}

// set the process status to idle
void drop_process(int pid) {
    struct process *drop_proc;
    struct process *temp_proc;
    drop_proc = array_getguy(processTable, pid);

    int num = array_getnum(processTable);
    int j;
    
    if (drop_proc != NULL) {
        for (j = 1; j < num; j++) {
            temp_proc = array_getguy(processTable, j);

            if (temp_proc->parent_pid == pid && temp_proc->exitStatus == 0) {
                temp_proc->parent_pid == -1;
            }else if(temp_proc->parent_pid == pid && temp_proc->exitStatus == 1) {
                drop_process(temp_proc->pid);
            }
        }
        
        drop_proc->exitStatus = 1;
        drop_proc->exitCode = -1;
        drop_proc->parent_pid = -1;
        drop_proc->myThread = NULL;
    }
}

// delete and free the memory for a process
void delete_process(struct process *delete_proc){
    if (delete_proc != NULL) {
        kfree(delete_proc);
    }
}

// generate a new pid for a process
int new_pid(void) {
    int i;
    int pt_size = array_getnum(processTable);
    struct process *temp_proc;
    struct process *parent_proc;
    int parent_exit;
    
    
    for (i = 1; i < pt_size ; i++) {
        temp_proc = array_getguy(processTable, i);
        
        if(temp_proc->parent_pid == -1){
            parent_exit = 1;
        }else {
            parent_proc = array_getguy(processTable, temp_proc->parent_pid);
            parent_exit = parent_proc->exitStatus;
        }
        
        if (temp_proc->exitStatus == 1 && parent_exit == 1) {
            temp_proc->pid = i;
            temp_proc->exitStatus = 0;
            temp_proc->myThread = curthread;
        }
    }
    
    struct process *new_proc;
    new_proc = create_process(i);
    
    if(new_proc == NULL) {
        return -1;
    }
    
    
    
    int result = array_preallocate(processTable, i);
    if (result) {
        kfree(new_proc);
        return -1;
    }
    
    result = array_add(processTable, new_proc);
    if (result != 0) {
        return -1;
    }
    
    return i;
}

// called when a process exit
void process_exit(int pid, int exitcode) {
    
    
    struct process *exit_proc;
    struct process *child;
    struct lock *mylock;
    struct cv *mycv;
    int size, i;
    
    exit_proc = array_getguy(processTable, pid);

    
    mylock = exit_proc->p_lock;
    mycv = exit_proc->p_cv;
    
    size = array_getnum(processTable);
    if (exit_proc->exitStatus == 0) {
        lock_acquire(mylock);

        
        
        if (exit_proc->parent_pid != -1) {
            
            
            for (i = 1; i < size; i++) {
                child = array_getguy(processTable, i);

                if (child->parent_pid == pid) {
                    if (child->exitStatus == 0) {
                        child->parent_pid = -1;
                    }else {
                        drop_process(child->pid);
                    }
                }
            }
            exit_proc->exitStatus = 1;
//            kprintf("exit, %d\n", pid);

            cv_broadcast(mycv, mylock);


        }else {
            drop_process(pid);
        }
        exit_proc->exitCode = exitcode;
        lock_release(mylock);
        


        thread_exit();

    }
}

// find the process with the pid
int process_find(int pid) {
    
    int num = array_getnum(processTable);
    struct process *temp_proc = array_getguy(processTable, pid);
    
    if(pid > num || temp_proc->exitStatus == 1) {
        return 0;
    }else {
        return 1;
    }
}
