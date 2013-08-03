#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <array.h>
#include <vm.h>
#include <elf.h>
#include <vnode.h>
#include <vfs.h>
#include <uio.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <syscall.h>
#include <coremap.h>
#include "opt-A3.h"
#include "uw-vmstats.h"



/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

#define DUMBVM_STACKPAGES    12

volatile int old_pid;


//used to indicate whether the vm system is running
int vm_running = 0;


void
vm_bootstrap(void)
{
    
	vmstats_init();
//    kprintf("%d thread acquire, ft_init\n", curthread->pid);

    ft_init();
    
//    kprintf("%d thread release, ft_init\n", curthread->pid);

    vm_running = 1;
    
}

static
paddr_t
getppages(unsigned long npages)
{
	int spl;
	paddr_t addr;
    
	spl = splhigh();
    if (vm_running == 0) {
		addr = ram_stealmem(npages);
	}
    else {
        
        
        //	addr = ram_stealmem(npages);
        if (npages == 1) {
//            kprintf("%d thread acquire, frame_alloc\n", curthread->pid);

            addr = frame_alloc();
            
//            kprintf("%d thread release, frame_alloc\n", curthread->pid);

            
        }else {
//            kprintf("%d thread acquire, frames_alloc\n", curthread->pid);

            addr = frames_alloc(npages);
//            kprintf("%d thread release, frames_alloc\n", curthread->pid);

        }
    }
	
	splx(spl);
	return addr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

void
free_kpages(vaddr_t addr)
{
    
    if (vm_running == 0) {
        (void)addr;
    } else {
        
    int spl;
    spl = splhigh();
    
    paddr_t paddr = KVADDR_TO_PADDR(addr);
    
//        kprintf("%d thread acquire, free\n", curthread->pid);

    frames_free(paddr);
//        kprintf("%d thread release, free\n", curthread->pid);

    splx(spl);
    }
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;
    struct array *page_table;
    struct page *curpage;
    
    //segment to indicate text, data or stack
    int segment;
    
	spl = splhigh();
    
	faultaddress &= PAGE_FRAME;
    
    
	switch (faulttype) {
	    case VM_FAULT_READONLY:
            //            panic("dumbvm: got VM_FAULT_READONLY\n");
            sys_exit(faulttype);
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
            break;
	    default:
            splx(spl);
            return EINVAL;
	}
    
	as = curthread->t_vmspace;
    
    page_table = as->page_table;
    
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}
    
	/* Assert that the address space has been set up properly. */
	assert(as->as_vbase1 != 0);
	assert(as->as_npages1 != 0);
	assert(as->as_vbase2 != 0);
	assert(as->as_npages2 != 0);
	assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
    
	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;
    
    
    //the index of the page with bad_address
    int page_index;
    
	if (faultaddress >= vbase1 && faultaddress < vtop1) {
        page_index = (faultaddress - vbase1)/PAGE_SIZE;
        segment = 0;
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
        page_index = as->as_npages1 + (faultaddress - vbase2)/PAGE_SIZE;
        segment = 1;
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop) {
        page_index = as->as_npages1 + as->as_npages2 + (faultaddress - stackbase)/PAGE_SIZE;
        segment = 2;
	}
	else {
		splx(spl);
		return EFAULT;
	}
    
    
    //the page with bad_address
    curpage = array_getguy(page_table, page_index);
    
    
    
    if (curpage->status == 1) {
        //the page has been used
        paddr = curpage->paddr;
        
        vmstats_inc(4);
    } else {
        
        //the page is free
        paddr = getppages(1);
        
        if (paddr == 0) {
            return ENOMEM;
        }
        
        curpage->paddr = paddr;
        
        if (segment != 2) {
            
            //load the file we need
            int result = page_loading(curpage);
            
            if (result) {
                return result;
            }
            
        } else {
            
            //Page Faults (Zeroed)
            vmstats_inc(5);
        }
        
        curpage->status = 1;
    }
    
    /* make sure it's page-aligned */
	assert((paddr & PAGE_FRAME)==paddr);
    
    //the index of the free tlb entry
    int tlb_index = -1;
    
    //update the TLB entries
	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
        
        //index to indicate the free tlb entry
        tlb_index = i;
        
        //TLB Faults with Free
        vmstats_inc(1);
        
        break;
	}
    
    if (tlb_index == -1) {
        tlb_index = tlb_get_rr_victim();
        
        // TLB Faults with Replace
        vmstats_inc(2);
    }
    
    ehi = faultaddress;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
    TLB_Write(ehi, elo, tlb_index);
    
	splx(spl);
	return 0;
}


int
tlb_get_rr_victim() {
    int victim;
    static unsigned int next_victim = 0;
    victim = next_victim;
    next_victim = (next_victim + 1) % NUM_TLB;
    return victim;
}



int
page_loading(struct page *p) {
    struct vnode *vnode;
    struct addrspace *as = curthread->t_vmspace;
    char *progname = kstrdup(as->elf_file);
    
    Elf_Phdr ph;
    struct uio ku;
	size_t file_size;
    
    int result = vfs_open(progname, O_RDONLY, &vnode);
    if (result) {
        return result;
    }
    
    int spl = splhigh();
    
    vaddr_t page_vaddr = p->vaddr;
    
    
    mk_kuio(&ku, &ph, sizeof(ph), p->offset, UIO_READ);
    
	result = VOP_READ(vnode, &ku);
	if (result) {
		splx(spl);
		return result;
	}
    if (ku.uio_resid != 0) {
        /* short read; problem with executable? */
        kprintf("ELF: short read on phdr - file truncated?\n");
        return ENOEXEC;
    }
    
    vaddr_t ph_vaddr = ph.p_vaddr & PAGE_FRAME;
    vaddr_t diff1 = ph.p_vaddr - ph_vaddr;
    vaddr_t diff2 = page_vaddr - ph_vaddr;
    
    size_t num_of_pages = diff2 / PAGE_SIZE;
    
    size_t last_page_index = (ph.p_filesz + diff1) / PAGE_SIZE;
    
    if (last_page_index < num_of_pages) {
        file_size = 0;
    } else if (last_page_index == num_of_pages) {
        if (num_of_pages != 0) {
            file_size = ph.p_filesz - diff2;
        } else {
            page_vaddr = page_vaddr + diff1;
            file_size = ph.p_filesz;
        }
    } else {
        if (num_of_pages != 0) {
            file_size = PAGE_SIZE;
        } else {
            file_size = PAGE_SIZE - diff1;
            page_vaddr = page_vaddr + diff1;
        }
    }
    
    if (file_size <= 0) {
        vmstats_inc(5);
    } else {
        vmstats_inc(6);
        vmstats_inc(7);
    }
    
    off_t offset = ph.p_offset + num_of_pages * PAGE_SIZE;
    
    result = load_segment(vnode, offset, page_vaddr, PAGE_SIZE, file_size, ph.p_flags & PF_X);
    
    if (result) {
        splx(spl);
        return result;
    }
    
    vfs_close(vnode);
    
    return 0;
    
}

int
load_segment(struct vnode *v, off_t offset, vaddr_t vaddr,
             size_t memsize, size_t filesize,
             int is_executable)
{
	struct uio u;
	int result;
	size_t fillamt;
    
	if (filesize > memsize) {
		kprintf("ELF: warning: segment filesize > segment memsize\n");
		filesize = memsize;
	}
    
    
	u.uio_iovec.iov_ubase = (userptr_t)vaddr;
	u.uio_iovec.iov_len = memsize;   // length of the memory space
	u.uio_resid = filesize;          // amount to actually read
	u.uio_offset = offset;
	u.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = curthread->t_vmspace;
    
	result = VOP_READ(v, &u);
	if (result) {
		return result;
	}
    
	if (u.uio_resid != 0) {
		/* short read; problem with executable? */
		kprintf("ELF: short read on segment - file truncated?\n");
		return ENOEXEC;
	}
    
	/* Fill the rest of the memory space (if any) with zeros */
	fillamt = memsize - filesize;
	if (fillamt > 0) {
		u.uio_resid += fillamt;
		result = uiomovezeros(fillamt, &u);
	}
	
	return result;
}




/************************************************/

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}
    
#if OPT_A3
    as->as_vbase1 = 0;
	as->as_pbase1 = 0;
	as->as_npages1 = 0;
    as->as_offset1 = 0;
    as->as_permission1 = 0;
    
	as->as_vbase2 = 0;
	as->as_pbase2 = 0;
	as->as_npages2 = 0;
    as->as_offset2 = 0;
    as->as_permission2 = 0;
    
    as->as_stackpbase = 0;
    
    as->page_table = array_create();
    
#else
    /*
	 * Initialize as needed.
	 */
    
#endif
	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;
    
	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}
    
#if OPT_A3
    newas->as_vbase1 = old->as_vbase1;
	newas->as_npages1 = old->as_npages1;
    newas->as_offset1 = old->as_offset1;
    newas->as_permission1 = old->as_permission1;
    
	newas->as_vbase2 = old->as_vbase2;
	newas->as_npages2 = old->as_npages2;
    newas->as_offset2 = old->as_offset2;
    newas->as_permission2 = old->as_permission2;
    
    
	if (as_prepare_load(newas)) {
		as_destroy(newas);
		return ENOMEM;
	}
    
	assert(newas->as_pbase1 != 0);
	assert(newas->as_pbase2 != 0);
	assert(newas->as_stackpbase != 0);
    
	memmove((void *)PADDR_TO_KVADDR(newas->as_pbase1),
            (const void *)PADDR_TO_KVADDR(old->as_pbase1),
            old->as_npages1*PAGE_SIZE);
    
	memmove((void *)PADDR_TO_KVADDR(newas->as_pbase2),
            (const void *)PADDR_TO_KVADDR(old->as_pbase2),
            old->as_npages2*PAGE_SIZE);
    
	memmove((void *)PADDR_TO_KVADDR(newas->as_stackpbase),
            (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
            DUMBVM_STACKPAGES*PAGE_SIZE);
    
#else
	/*
	 * Write this.
	 */
    
	(void)old;
#endif
	
	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
#if OPT_A3
    
#else
	/*
	 * Clean up as needed.
	 */
#endif
	
	kfree(as);
}

void
as_activate(struct addrspace *as)
{
#if OPT_A3
    int i, spl;
    
	spl = splhigh();
    
    if (old_pid != curthread->pid) {
        for (i=0; i<NUM_TLB; i++) {
            TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }
        
        vmstats_inc(3);
        
        old_pid = curthread->pid;
    }
    
	splx(spl);
    
    (void)as;
    
#else
	/*
	 * Write this.
	 */
    
	(void)as;  // suppress warning until code gets written
#endif
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
                 int readable, int writeable, int executable)
{
#if OPT_A3
    size_t npages;
    
	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;
    
	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
    
	npages = sz / PAGE_SIZE;
    
    
    
	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
        as->as_permission1 = writeable;
		return 0;
	}
    
	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
        as->as_permission2 = writeable;
		return 0;
	}
    
    
	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
    
#else
	/*
	 * Write this.
	 */
    
	(void)as;
	(void)vaddr;
	(void)sz;
	(void)readable;
	(void)writeable;
	(void)executable;
	return EUNIMP;
#endif
    
}

int
as_prepare_load(struct addrspace *as)
{
#if OPT_A3
    //    assert(as->as_pbase1 == 0);
    //	assert(as->as_pbase2 == 0);
    //	assert(as->as_stackpbase == 0);
    
    struct page *curpage;
    int index;
    
    for (index = 0; index < as->as_npages1; index++) {
        curpage = kmalloc(sizeof(struct page));
        curpage->vaddr = as->as_vbase1 + index * PAGE_SIZE;
        curpage->paddr = 0;
        curpage->status = 0;
        curpage->permission = as->as_permission1;
        curpage->offset = as->as_offset1;
        
        array_add(as->page_table, curpage);
    }
    
    for (index = 0; index < as->as_npages2; index++) {
        curpage = kmalloc(sizeof(struct page));
        curpage->vaddr = as->as_vbase2 + index * PAGE_SIZE;
        curpage->paddr = 0;
        curpage->status = 0;
        curpage->permission = as->as_permission2;
        curpage->offset = as->as_offset2;
        
        array_add(as->page_table, curpage);
    }
    
    for (index = 0; index < DUMBVM_STACKPAGES; index++) {
        
        vaddr_t stackvbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
        
        curpage = kmalloc(sizeof(struct page));
        curpage->vaddr = stackvbase + index * PAGE_SIZE;
        curpage->paddr = 0;
        curpage->status = 0;
        curpage->permission = 1;
        curpage->offset = index * PAGE_SIZE;
        
        array_add(as->page_table, curpage);
    }
    
    
#else
	/*
	 * Write this.
	 */
    
	(void)as;
#endif
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
#if OPT_A3
    (void)as;
    
#else
	/*
	 * Write this.
	 */
    
	(void)as;
#endif
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
#if OPT_A3
    //    assert(as->as_stackpbase != 0);
    
#else
	/*
	 * Write this.
	 */
    
	(void)as;
    
#endif
	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
	
	return 0;
}
