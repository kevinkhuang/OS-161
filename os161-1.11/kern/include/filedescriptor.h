 //
//  filedescriptor.h
//  
//
//  Created by Xiaoyu Kou on 12-11-08.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _filedescriptor_h
#define _filedescriptor_h

#include <types.h>
#include <array.h>
#include <lib.h>
#include <thread.h>

// file descriptors which distinguished by 0, 1, 2 representing stdin, stdout, and stderr
struct filedescriptor {
    int file_ID;
    int flag;
    int offset;
    struct vnode *vnode;
};

// table of file descriptors for every processes having at lease 3 file descrrptors which are 0, 1, and 2 for stdin, stdout, and stderr
struct descriptorTable {
    struct array *table;
    struct array *IDs;      // file ids that have been deallocated before
    int totalnum;
    int next;           // next avaliable file id in IDs
};

// methods for descriptorTable
int constructor(struct thread *thread);  // constructor for descriptor table
int destructor(struct descriptorTable *dt);    // destructor
int add_one(struct filedescriptor *fd, struct descriptorTable *dt);
int delete_one(int fid, struct descriptorTable *dt);
int search(int fid, struct descriptorTable *dt);
int get_new_ID(struct descriptorTable *dt);
int make_copy(struct thread *source, struct thread *dest);
#endif
