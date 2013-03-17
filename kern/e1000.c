#include <kern/e1000.h>

// LAB 6: Your driver code here
#include <kern/pmap.h>
#include <inc/error.h>

int
pci_E1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	//cprintf("size need:%d\n",pcif->reg_size[0]);//need 128KB
	

	boot_map_region(kern_pgdir,E1000IO,pcif->reg_size[0],pcif->reg_base[0],PTE_PCD|PTE_PWT|PTE_W);
	e1000 = (void*)(E1000IO);	//used like lapic
	cprintf("dsr  %x pa %x : %x\n",e1000,pcif->reg_base[0],*(int*)(&e1000[E1000_STATUS]));  //DSR offset 0008h.Pay attention that the offset is byte but the data is int(4 byte)


	//transmit initialization s14.5	
	*(uint32_t*)(&e1000[E1000_TDBAL]) = TX_DESC_RING_BASE;
	*(uint32_t*)(&e1000[E1000_TDBAH]) = 0;					//32 not needed
	*(uint32_t*)(&e1000[E1000_TDLEN]) = TX_DESC_RING_LENGTH;
	*(uint32_t*)(&e1000[E1000_TDH])	  = 0;
	*(uint32_t*)(&e1000[E1000_TDT])   = 0;

	tx_desc_head = (uint32_t*)(&e1000[E1000_TDH]);
	tx_desc_tail = (uint32_t*)(&e1000[E1000_TDT]);
	
	//initialize TCTL
	uint32_t *tctl = (uint32_t*)(&e1000[E1000_TCTL]);
	*tctl |= E1000_TCTL_EN;		//1b
	*tctl |= E1000_TCTL_PSP;	//1b
	*tctl |= E1000_TCTL_CT  & ((0x10)<<4 );  
	*tctl |= E1000_TCTL_COLD& ((0x40)<<12);

	//receive initialization s14.4
	int n = 0;
	for(n=0; n < RX_DESC_RING_SIZE; n++){
		rx_desc_ring[n].buffer_addr = (uint32_t)PADDR(&(recbuf[n][0]));
	}
	*(uint32_t*)(&e1000[E1000_RA])	  = 0x12005452;			//QEMU's default MAC address
	*(uint32_t*)(&e1000[E1000_RA+4])  = 0X80005634;			//52:54:00:12:34:56 !!no RAL AND RAH
															//remember to set AS and AV
	*(uint32_t*)(&e1000[E1000_MTA])   = 0;					//SET Multicast TA to 0b
	*(uint32_t*)(&e1000[E1000_RDBAL]) = RX_DESC_RING_BASE;
	*(uint32_t*)(&e1000[E1000_RDBAH]) = 0;					//32 not needed
	*(uint32_t*)(&e1000[E1000_RDLEN]) = RX_DESC_RING_LENGTH;
	*(uint32_t*)(&e1000[E1000_RDH])	  = 0;
	*(uint32_t*)(&e1000[E1000_RDT])   = 0;

	rx_desc_head = (uint32_t*)(&e1000[E1000_RDH]);
	rx_desc_tail = (uint32_t*)(&e1000[E1000_RDT]);
	
	//initialize RCTL
	uint32_t *rctl = (uint32_t*)(&e1000[E1000_RCTL]);
										//MTA 0b?
										//IMS enable interruypt
										//RDTR threshold desired delay time 
	*rctl |= E1000_RCTL_EN;				//1b
	*rctl &= ~E1000_RCTL_LPE;			//0b long packet not enable
	*rctl |= E1000_RCTL_LBM_NO;			//no loopback mode 00bits
	*rctl |= E1000_RCTL_MO_0;
	*rctl |= E1000_RCTL_BAM;			//enable the broadcast
	*rctl |= E1000_RCTL_SZ_1024;		//1KB as maximun of transmit tmp is PGSIZE
										//Thus there's no need to set BSEX
	*rctl |= E1000_RCTL_SECRC;			//strip CRC



	return 1;
}

int E1000_transmit(char* buf,int length){
	struct e1000_tx_desc *transmit_desc =  &tx_desc_ring[(*tx_desc_tail)];
	if(	(transmit_desc->lower.flags.cmd & E1000_TXD_CMD_RS) &&
		!(transmit_desc->upper.fields.status & E1000_TXD_STAT_DD)
		 )
	{
		cprintf("tx_desc_ring is full\n");
		return -E_IPC_NOT_RECV;
	}
	
	transmit_desc->buffer_addr = (uint32_t)PADDR(buf);
	transmit_desc->lower.flags.length = (uint16_t)length;
	transmit_desc->lower.data |= E1000_TXD_CMD_RS | E1000_TXD_CMD_IDE | E1000_TXD_CMD_RPS | E1000_TXD_CMD_TCP ;	//don't forget to add TCP to test the tcpdump
	int nexttail = *tx_desc_tail;
	nexttail = (nexttail+1)%TX_DESC_RING_SIZE;
	*tx_desc_tail = nexttail;	//use a variable to count.
	//cprintf("%d	flags:%x\n",nexttail,transmit_desc->lower);

	return 0;
}

int E1000_receive(char** bufa,int *length){
	struct e1000_rx_desc *receive_desc = &rx_desc_ring[*rx_desc_tail];
	if(	!(receive_desc->status & E1000_RXD_STAT_DD) )
	{
		return -E_IPC_NOT_RECV;
	}
	*length = receive_desc->length;
	*bufa = KADDR(receive_desc->buffer_addr);			//this is already in kernel 
	int nexttail = *rx_desc_tail;
	nexttail = (nexttail+1)%RX_DESC_RING_SIZE;
	*rx_desc_tail = nexttail;

	
	return 0;
}


