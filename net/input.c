#include "ns.h"
#include <inc/lib.h>

extern union Nsipc nsipcinbuf[NIPCINBUF];

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
	while (true){
		// swap IPC input buffer page
		for (int i = 0; i < NIPCINBUF; i++){
			nsipcinbuf[i].pkt.jp_data[0] = 'a';
			int received = sys_e1000_try_receive(nsipcinbuf[i].pkt.jp_data);
			// if receive queue empty
			if (received == -E_TRY_AGAIN){
				sys_yield();
				continue;
			} else {
				nsipcinbuf[i].pkt.jp_len = received;
				ipc_send(ns_envid, NSREQ_INPUT, (void *)&(nsipcinbuf[i]), (PTE_P | PTE_U | PTE_W));
				sys_yield();
			}
		}
	}
}
