#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H


#include <inc/types.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/stdio.h>
#include <inc/x86.h>
#include <kern/e1000_hw.h>
#include <kern/cpu.h>
#include <kern/pci.h>


#define E1000IO KSTACKTOP+PGSIZE		//use gap between KSTACKTOP and KERBASE as the MMIO
#define TX_DESC_RING_SIZE 64			//128byte aligned and desc is 16byte,not more than 64 desc
#define TX_DESC_RING_LENGTH TX_DESC_RING_SIZE*16
#define RX_DESC_RING_SIZE 64
#define RX_DESC_RING_LENGTH RX_DESC_RING_SIZE*16

volatile uint8_t *e1000;				// used as extern in cpu.h like lapic

//transmit desc array/ring in section 3.4
struct e1000_tx_desc tx_desc_ring[TX_DESC_RING_SIZE];
uint32_t *tx_desc_head;				//head and tail are offset.can be up to 64k desc
uint32_t *tx_desc_tail;
#define TX_DESC_RING_BASE  (uint32_t)PADDR(tx_desc_ring)

//transmit function in kernel
int E1000_transmit(char* buf,int length);

//transmit tmp in kernel for pkt
char tmp[PGSIZE];

//receive packets array/ring in section 3.3
struct e1000_rx_desc rx_desc_ring[RX_DESC_RING_SIZE];
uint32_t *rx_desc_head;
uint32_t *rx_desc_tail;
#define RX_DESC_RING_BASE	(uint32_t)PADDR(rx_desc_ring)

char recbuf[RX_DESC_RING_SIZE][PGSIZE];		//receive buffers

//receive function in kernel
int E1000_receive(char** bufa,int*length);

int pci_E1000_attach(struct pci_func *pcif);//pci vendor attach






#endif
