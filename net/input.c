#include "ns.h"
extern union Nsipc nsipcbuf;

extern int sys_receive(char* buf,int* length);

#define TMPNUM 6
union Nsipc recbuftmp0 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp1 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp2 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp3 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp4 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp5 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp6 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp7 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp8 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp9 __attribute__((aligned(PGSIZE)));
union Nsipc recbuftmp10 __attribute__((aligned(PGSIZE)));










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
	

	int r;
	int i = 0;
	union Nsipc *recarea = NULL; 	// point to a area to rec and be read for a while
	while(1){
		switch(i%TMPNUM)
		{
			case 0:
				recarea = &recbuftmp0;
				break;
			case 1:
				recarea = &recbuftmp1;
				break;
			case 2:
				recarea = &recbuftmp2;
				break;
			case 3:
				recarea = &recbuftmp3;
				break;
			case 4:
				recarea = &recbuftmp4;
				break;
			case 5:
				recarea = &recbuftmp5;
				break;
			case 6:
				recarea = &recbuftmp6;
				break;
			case 7:
				recarea = &recbuftmp7;
				break;
			case 8:
				recarea = &recbuftmp8;
				break;
			case 9:
				recarea = &recbuftmp9;
				break;
			case 10:
				recarea = &recbuftmp9;
				break;















		}
		  //recarea = &nsipcbu
		recarea->pkt.jp_len = 0;
		r = sys_receive(recarea->pkt.jp_data,&recarea->pkt.jp_len);
		if( r<0 && r!=-E_IPC_NOT_RECV )
			panic ("net: sys receive failed : %e", r);
		if( r==-E_IPC_NOT_RECV )
			continue;
	    ipc_send(ns_envid, NSREQ_INPUT, recarea, PTE_U|PTE_P);
		//sure to be aligned?how to set perm?
		//as it said "don't immediately...same.."
		i++;
	}
}
