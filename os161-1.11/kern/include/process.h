#ifndef _process_H_
#define _process_H_

#include <lib.h>
#include <types.h>
#include <synch.h>
#include <array.h>


struct process {
    int pid;
    int exitCode;
    int exitStatus;
    int parent_pid;
    struct thread *myThread;
    struct lock *p_lock;
    struct cv *p_cv;
};



void create_process_table(void);

void delete_process_table(void);

struct process *create_process(int pid);

void drop_process(int pid);

void delete_process(struct process *delete_proc);

int new_pid(void);

void process_exit(int pid, int exitcode);

int process_find(int pid);


#endif
