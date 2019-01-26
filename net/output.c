#include "ns.h"
#include <inc/lib.h>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while (true) {
		int32_t value = ipc_recv(NULL, (void *) &nsipcbuf, NULL);
		if (value < 0) {
			panic("ipc_send: failed to send");
		}
		// recv the supported IPC message
		if (value == NSREQ_OUTPUT) {
			int error = 1;
			while (error) {
				error = sys_e1000_try_transmit(
				        nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
				// if transmit queue full
				if (error) {
					sys_yield();
				}
			}
		}
	}
}
