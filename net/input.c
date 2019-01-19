#include "ns.h"
#include <inc/lib.h>

extern union Nsipc nsipcbuf;
union Nsipc nsipcbuf2 __attribute__((aligned(PGSIZE)));
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
		//cprintf("INTENTO RECIBIR\n");
		//cprintf("%p\n", &nsipcbuf);
		//cprintf("%d\n", nsipcbuf.pkt.jp_len);
		//cprintf("%p\n", nsipcbuf.pkt.jp_data);
		//cprintf("%d\n", nsipcbuf.pkt.jp_data[0]);
		//cprintf("%d\n", nsipcbuf.pkt.jp_data[1]);
		//cprintf("%d\n", *nsipcbuf.pkt.jp_data);
		//nsipcbuf.pkt.jp_data[1] = 'a';
		int received = sys_e1000_try_receive(nsipcbuf2.pkt.jp_data);
		//cprintf("VUEVO DEL INTENTO\n");
		// if receive queue empty
		if (received == -E_TRY_AGAIN){
			//cprintf("NO RECIBI\n");
			sys_yield();
			continue;
		} else {
			cprintf("RECIBI\n");
			cprintf ("received: %d\n", received);
			cprintf("%p\n", &nsipcbuf2);
			nsipcbuf2.pkt.jp_len = received;
			ipc_send(ns_envid, NSREQ_INPUT, (void *)&nsipcbuf2, (PTE_U | PTE_W));
			sys_yield();
		}
	}
}
