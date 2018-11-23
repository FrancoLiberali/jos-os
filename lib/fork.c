// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	if (perm & PTE_W) {
		int r;

		if ((r = sys_page_alloc(dstenv, va, perm)) < 0)
			panic("sys_page_alloc: %e", r);
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, perm)) < 0)
			panic("sys_page_map: %e", r);
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0)
			panic("sys_page_unmap: %e", r);
	} else {
		int r;

		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0)
			panic("sys_page_map: %e", r);
	}
}

envid_t
fork_v0(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;

	// Allocate a new child environment.
	// The kernel will initialize it with a copy of our register state,
	// so that the child will appear to have called sys_exofork() too -
	// except that in the child, this "fake" call to sys_exofork()
	// will return 0 instead of the envid of the child.
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	// Eagerly copy our entire address space into the child.
	for (addr = (uint8_t *) UTEXT; addr < (uint8_t *) UTOP; addr += PGSIZE) {
		// more info:
		// https://pdos.csail.mit.edu/6.828/2017/labs/lab4/uvpt.html
		// uvpt = UVPT = EF400000
		// uvpd = (UVPT+(UVPT>>12)*4) = EF7BD000
		// uvpd let us enter to the page dir with two levels of
		// indirection,
		// because PDX(uvpt) is index of
		// the recursively inserted PD in itself
		// and PTX(uvpt) is index of
		// the recursively inserted PD in itself too
		// So it let us in the physical PD
		// PDX(addr) * 4 in the offset to go to the pde of the pt of
		// addr(* 4 because of the size of the pde's)
		pde_t *pde = (pde_t *) (PGADDR(
		        PDX(uvpd), PTX(uvpd), (PDX(addr) * sizeof(pde_t))));
		// if the pt of addr was present
		if ((*pde) & PTE_P) {
			// uvpt let us enter to the page dir, because PDX(uvpt)
			// is index of
			// the recursively inserted PD in itself
			// PDX(addr) as PTX to index in the PD with the PDX, so
			// it let us in the physical PT where addr is
			// PTX(addr) * the size of the pte's)
			pte_t *pte =
			        (pte_t *) (PGADDR(PDX(uvpt),
			                          PDX(addr),
			                          (PTX(addr) * sizeof(pte_t))));
			// if the page of addr was present
			if ((*pte) & PTE_P)
				dup_or_share(envid, addr, (*pte) & PTE_SYSCALL);
		}
	}

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	addr = ROUNDDOWN(addr, PGSIZE);

	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	pte_t *pte = 0;
	pde_t *pde = (pde_t *) (PGADDR(
	        PDX(uvpd), PTX(uvpd), (PDX(addr) * sizeof(pde_t))));
	// if the pt of addr was present
	if ((*pde) & (PTE_P | PTE_COW))
		pte = (pte_t *) (PGADDR(
		        PDX(uvpt), PDX(addr), (PTX(addr) * sizeof(pte_t))));

	if (!(err & FEC_WR) ||  //(err & FEC_PR) ||
	    !((*pte) & (PTE_P | PTE_COW)))
		panic("pgfault: not copy-on-write region!");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(0, PFTEMP, PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	memmove(PFTEMP, addr, PGSIZE);
	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_W)) < 0)
		panic("sys_page_map: %e", r);

	if ((r = sys_page_unmap(0, PFTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
}

//
// Map our virtual address addr (page-aligned) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, void *addr, int perm)
{
	int r;

	// LAB 4: Your code here.
	bool remap;
	if ((perm & PTE_W) | (perm & PTE_COW)) {
		perm = (perm & ~PTE_W) | PTE_COW;
		remap = true;
	}

	if ((r = sys_page_map(0, (void *) addr, envid, (void *) addr, perm)) < 0)
		panic("sys_page_map in duppage: %e", r);

	if (remap) {
		if ((r = sys_page_map(0, (void *) addr, 0, (void *) addr, perm)) < 0)
			panic("sys_page_map in duppage: %e", r);
	}

	return 0;
}

extern void _pgfault_upcall(void);

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// return fork_v0(); inefficient way
	// LAB 4: Your code here.

	envid_t envid;
	uint8_t *addr;
	int r;

	set_pgfault_handler(&pgfault);

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	// set_pgfault_handler(&pgfault); dont let us choise
	// for what env we want to set the handler, so we do it
	// manually.
	if ((r = sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE), PTE_W)) < 0)
		panic("set_pgfault_handler: allocation failed!");
	
	sys_env_set_pgfault_upcall(envid, &(_pgfault_upcall));

	for (addr = (uint8_t *) 0x0; addr < (uint8_t *) UTOP; addr += PGSIZE) {
		if (((uint8_t *) USTACKTOP < addr) &&
		    (addr < (uint8_t *) UXSTACKTOP))
			continue;
		// more info:
		// https://pdos.csail.mit.edu/6.828/2017/labs/lab4/uvpt.html
		// uvpt = UVPT = EF400000
		// uvpd = (UVPT+(UVPT>>12)*4) = EF7BD000
		// uvpd let us enter to the page dir with two levels of
		// indirection,
		// because PDX(uvpt) is index of
		// the recursively inserted PD in itself
		// and PTX(uvpt) is index of
		// the recursively inserted PD in itself too
		// So it let us in the physical PD
		// PDX(addr) * 4 in the offset to go to the pde of the pt of
		// addr(* 4 because of the size of the pde's)
		pde_t *pde = (pde_t *) (PGADDR(
		        PDX(uvpd), PTX(uvpd), (PDX(addr) * sizeof(pde_t))));
		// if the pt of addr was present
		if ((*pde) & PTE_P) {
			// uvpt let us enter to the page dir, because PDX(uvpt)
			// is index of
			// the recursively inserted PD in itself
			// PDX(addr) as PTX to index in the PD with the PDX, so
			// it let us in the physical PT where addr is
			// PTX(addr) * the size of the pte's)
			pte_t *pte =
			        (pte_t *) (PGADDR(PDX(uvpt),
			                          PDX(addr),
			                          (PTX(addr) * sizeof(pte_t))));
			// if the page of addr was present
			if ((*pte) & PTE_P) {
				duppage(envid, (void *) addr, (*pte) & PTE_SYSCALL);
			}
		}
	}

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
