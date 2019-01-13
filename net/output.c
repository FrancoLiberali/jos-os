#include "ns.h"
#include <lib/ipc.c>
#include <lib/syscall.c>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while (true){
		int32_t value = ipc_recv(NULL, (void *)&nsipcbuf, NULL);
		if (value < 0){
			panic("ipc_send: failed to send");
		}
		// recv the supported IPC message
		if (value == NSREQ_OUTPUT){
			int error = 1;
			while(error){
				error = sys_e1000_try_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
				// if transmit queue full
				if (error){
					sys_yield();
				}
				/*
				Be aware of the interaction between the device driver, 
				the output environment and the core network server when 
				there is no more space in the device driver's transmit queue. 
				The core network server sends packets to the output environment 
				using IPC. If the output environment is suspended due to 
				a send packet system call because the driver has no more 
				buffer space for new packets, the core network server 
				will block waiting for the output server to accept the IPC call. */
			}
		}
	}
}
