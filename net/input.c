#include "ns.h"
#include <inc/lib.h>

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	static union Nsipc nsipcinbuf[NIPCINBUF] __attribute__((aligned(PGSIZE)));
	while (true) {
		// swap IPC input buffer page
		for (int i = 0; i < NIPCINBUF; i++) {
			int received =
			        sys_e1000_try_receive(nsipcinbuf[i].pkt.jp_data,
			                              PGSIZE - sizeof(int));
			// if receive queue empty
			if (received == -E_TRY_AGAIN) {
				sys_yield();
				continue;
			} else {
				nsipcinbuf[i].pkt.jp_len = received;
				ipc_send(ns_envid,
				         NSREQ_INPUT,
				         (void *) &(nsipcinbuf[i]),
				         (PTE_P | PTE_U | PTE_W));
				sys_yield();
			}
		}
	}
}
