#include "ns.h"
#include <inc/lib.h>

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	static union Nsipc nsipcoutbuf __attribute__((aligned(PGSIZE)));
	envid_t from_env_store;
	while (true) {
		int32_t value =
		        ipc_recv(&from_env_store, (void *) &nsipcoutbuf, NULL);
		if (value < 0) {
			panic("ipc_send: failed to send");
		}
		// recv the supported IPC message
		if (value == NSREQ_OUTPUT && from_env_store == ns_envid) {
			int error = 1;
			while (error) {
				error = sys_e1000_try_transmit(
				        nsipcoutbuf.pkt.jp_data,
				        nsipcoutbuf.pkt.jp_len);
				// if transmit queue full
				if (error) {
					sys_yield();
				}
			}
		}
	}
}
