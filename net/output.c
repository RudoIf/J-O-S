#include "ns.h"
extern union Nsipc nsipcbuf;

extern int sys_transmit(char* buf,int length);

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	int r;
	envid_t fromwho;

	while(1){
		r = ipc_recv(&fromwho,&nsipcbuf,0);
		if((fromwho != ns_envid) || r != NSREQ_OUTPUT)
			continue;
		while((r = sys_transmit(nsipcbuf.pkt.jp_data,nsipcbuf.pkt.jp_len)) < 0)
					continue;

	}
}
