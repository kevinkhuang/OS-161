//
//  filedescriptor.c
//  
//
//  Created by Xiaoyu Kou on 12-11-08.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include <kern/unistd.h>
#include <kern/errno.h>
#include <filedescriptor.h>
#include <thread.h>
#include <curthread.h>
#include <vnode.h>

// DBU
int add_one(struct filedescriptor *fd, struct descriptorTable *dt);
int get_new_ID(struct descriptorTable *dt);
int search(int fid, struct descriptorTable *dt);


// default to have 3 file descriptors for every process
int constructor(struct thread *thread){
    char *filename1 = NULL;
    char *filename2 = NULL;   
    char *filename3 = NULL;

    int i = 0;
   
    // initialization arrays
    thread->dt = kmalloc(sizeof(struct descriptorTable));
    if(thread->dt == NULL) {
        return ENOMEM;
    }
    thread->dt->table = array_create();
    thread->dt->IDs = array_create();
    thread->dt->totalnum = 0;
    
    // adding stdin into table 
    struct filedescriptor *stdin = NULL;
    struct vnode *vnode1;

    stdin = kmalloc(sizeof(struct filedescriptor));
    if(stdin == NULL) {
        return ENOMEM;
    }
    stdin->file_ID = i;
    stdin->flag = O_RDONLY;
    stdin->offset = 0;
    filename1 = kstrdup("con:");
    vfs_open(filename1, O_RDONLY, &vnode1);
    stdin->vnode = vnode1;
    add_one(stdin, thread->dt);
    i = i + 1;
    
    // adding stdout into table
    struct filedescriptor *stdout = NULL;
    struct vnode *vnode2;
    
    stdout = kmalloc(sizeof(struct filedescriptor));
    if(stdout == NULL) {
        return ENOMEM;
    }
    stdout->file_ID = i;
    stdout->flag = O_WRONLY;
    stdout->offset = 0;
    filename2 = kstrdup("con:");
    vfs_open(filename2, O_WRONLY, &vnode2);
    stdout->vnode = vnode2;
    add_one(stdout, thread->dt);
    i = i + 1;
    
    // adding stderr into table
    struct filedescriptor *stderr = NULL;
    struct vnode *vnode3;
    
    stderr = kmalloc(sizeof(struct filedescriptor));
    if(stderr == NULL) {
        return ENOMEM;
    }
    stderr->file_ID = i;
    stderr->flag = O_WRONLY;
    stderr->offset = 0;
    filename3 = kstrdup("con:");
    vfs_open(filename3, O_WRONLY, &vnode3);
    stderr->vnode = vnode3;
    add_one(stderr, thread->dt);
    
    thread->dt->next = 3;
    kfree(filename1);
    kfree(filename2);
    kfree(filename3);

    return 0;
}

// destructor to deallocate memory that allocated for dt
int destructor(struct descriptorTable *dt) {
    int size = array_getnum(dt->table);
    int i;
    struct filedescriptor *temp;
    for (i = 0; i < size; i++) {
        temp = array_getguy(dt->table, i);
        vfs_close(temp->vnode);
        kfree(temp->vnode);
        kfree(temp);
    }
    array_destroy(dt->table);
    array_destroy(dt->IDs);
    kfree(dt);
    return 0;
}

// add one file descriptor into the descriptor table
int add_one(struct filedescriptor *fd, struct descriptorTable *dt) {
    int ans;
    ans = array_add(dt->table, fd);
    if(ans != 0) {
        return ENOMEM;
    }
    dt->totalnum++;
    return 0;
}

// delete one file descriptor from the descriptor table with given file id
int delete_one(int fid, struct descriptorTable *dt) {
    int i;
    int ans;
    i = search(fid, dt);
    if(i < 0) {
        return -1;      // fail
    }
    else {
        struct filedescriptor *temp;
        temp = array_getguy(dt->table, i);
        vfs_close(temp->vnode);
        kfree(temp->vnode);
        kfree(temp);
        array_remove(dt->table, i);
        ans = array_add(dt->IDs, fid);
        if(ans != 0) {
            return ENOMEM;
        }
        return 0;
    }
}

// search for a file descriptor within the descriptor table with given file id
int search(int fid, struct descriptorTable *dt) {
    int i;
    int total;
    total = dt->totalnum;
    for (i = 0; i <  total; i++) {
        struct filedescriptor *temp = array_getguy(dt->table, i);
        if(temp->file_ID == fid) {
            return i;
        }
    }
    return -1;      // fail
}

// obtain an avaliable file id from either next or IDs
int get_new_ID(struct descriptorTable *dt) {
    if(array_getnum(dt->IDs) == 0) {
        int ans = dt->next;
        dt->next++;
        return ans;
    }
    else {
        int *ans;
        ans = (int *)array_getguy(dt->IDs, 0);
        array_remove(dt->IDs, 0);
        return *ans;
    }
}

//  make a copy of descriptorTable from source thread to dest thread
int make_copy(struct thread *source, struct thread *dest) {
    struct filedescriptor *source_man;
    struct filedescriptor *dest_man;
    int size;
    dest->dt = kmalloc(sizeof(struct descriptorTable));
    if(dest->dt == NULL) {
        return ENOMEM;
    }
    dest->dt->table = array_create();
    if (dest->dt->table == NULL) {
        return ENOMEM;
    }
    dest->dt->IDs = array_create();
    if (dest->dt->IDs == NULL) {
        return ENOMEM;
    }
    size = array_getnum(source->dt->table);
    // copy filedescriptors from source
    int i;
    for (i = 0; i < size; i++) {
        source_man = array_getguy(source->dt->table, i);
        dest_man = kmalloc(sizeof(struct filedescriptor));
        if(dest_man == NULL) {
            return ENOMEM;
        }
        memcpy(dest_man, source_man, sizeof(struct filedescriptor));
        array_add(dest->dt->table, dest_man);
    }
    // copy avaliable IDs from source
    int *source_ids;
    int *dest_ids;
    size = array_getnum(source->dt->IDs);
    int j;
    for (j = 0; j < size; j++) {
        source_ids = array_getguy(source->dt->IDs, j);
        dest_ids = kmalloc(sizeof (int));
        if(dest_ids == NULL) {
            return ENOMEM;
        }
        memcpy(dest_ids, source_ids, sizeof(int));
        array_add(dest->dt->IDs, dest_ids);
    }
    //  copy rest infos.
    dest->dt->totalnum = source->dt->totalnum;
    dest->dt->next = source->dt->next;
    return 0;
}



