#include <types.h>
#include <lib.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <synch.h>
#include <addrspace.h>
#include <array.h>
#include <coremap.h>



struct array *frame_table;
struct array *free_frames;
struct lock *cmlock;        //coremap lock
int group = 1;


void ft_init(void){
    int index = 0;
    
    
    frame_table = array_create();
    free_frames = array_create();
    
    cmlock = lock_create("cmlock");
    
//    paddr_t firstpaddr, lastpaddr;
//    ram_getsize(&firstpaddr, &lastpaddr);
    
    
    lock_acquire(cmlock);

    
    while (firstpaddr + PAGE_SIZE <= lastpaddr) {
        struct frame *f = kmalloc(sizeof(struct frame));
        f->paddr = firstpaddr;
        f->index = index;
        f->status = 0;
        f->group = 0;
        array_add(frame_table, f);
        array_add(free_frames, (int *)index);
        
        firstpaddr = firstpaddr + PAGE_SIZE;
        
        index++;
    }
    
    lock_release(cmlock);

}

paddr_t frame_alloc(void) {
    struct frame *f;
    int found = 0;
    int i = 0;
    

    lock_acquire(cmlock);
    
    //scan the frame table to find the free frame
    while (!found) {
        
        //if the frame table is full, use the swap file
        if (i >= array_getnum(frame_table)) {
            index = swap(1);
            f = array_getguy(frame_table, index);
            break;

        }
        
        f = array_getguy(frame_table, i);
        if (f->status == 0) {
            found = 1;
        }
        i++;
    }
    

    
    i = array_getguy(free_frames, 0);
    
    array_remove(free_frames, 0);
    f = array_getguy(frame_table, i);
    
    //modify the frame status
    f->status = 1;
    f->group = group;
    group++;
    
    lock_release(cmlock);
    

    
    return f->paddr;
}

paddr_t frames_alloc(unsigned long npages){
    
    struct frame *f;
    int found = 0;
    int i = 0;
    int j, k;
    int index;
    

    lock_acquire(cmlock);
    
    //scan the frame table to find the contiguous free frames
    while (!found) {
        
        //if the frame table is full, use the swap file
        if (i >= array_getnum(frame_table)) {
            index = swap(npages);
            f = array_getguy(frame_table, index);
            break;
        }
        
        //make sure the free frames are contiguous
        for (j = 0; j < npages; j++) {
            
            index = i + j;
            f = array_getguy(frame_table, index);
            
            if (f->status != 0) {
                break;
            }
            
            if (j == npages - 1) {
                found = 1;
            }
        }
        i++;
    }
    
    //modify the status for the contiguous frames
    
    int positon = f->index;
    
    for (k = positon; k < (positon + npages); k++) {
        f = array_getguy(frame_table, k);
        f->status = 1;
        f->group = group;
        array_remove(free_frames, f->index);
    }
    
    group++;

    
    lock_release(cmlock);
    
    return f->paddr;

}

void frames_free(vaddr_t addr){
    
    
    struct frame *f;
    int i = 0;
    int found = 0;
    

    lock_acquire(cmlock);
    
    //find the frame we need to free
    while (!found) {
        f = array_getguy(frame_table, i);
        
        if (f->paddr == addr) {
            found = 1;
        }
        i++;
    }
    
    //get the group number of the frames
    int curgroup = f->group;
    
    while (f->group == curgroup) {
        
        //set the frame's status to free
        f->status = 0;
        
        array_add(free_frames, (int *)f->index);
        
        f = array_getguy(frame_table, f->index + 1);
    }
    
    lock_release(cmlock);
}
