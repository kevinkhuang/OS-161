#ifndef _coremap_H_
#define _coremap_H_

#include <lib.h>
#include <vm.h>
#include <addrspace.h>


struct frame {
	paddr_t paddr;   //physical address of the frame
	int index;       //the index of the frame
    int status;      //-1 for fixed, 0 for free, 1 for used
    int group;       //group index for the consecutive frames
};


//initialize the frame table, should be called in vm_bootstrap
void ft_init(void);  

//allocate one frame on physical memory
paddr_t frame_alloc(void);

//allocate contiguous frames on physical memory
paddr_t frames_alloc(unsigned long npages);

//free some frames
void frames_free(vaddr_t addr);

#endif
