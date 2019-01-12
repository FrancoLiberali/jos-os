#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>
//#include <kern/tx_desc_array.h>

#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */

#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_COLD_DUPLEX   0x40   /* collision distance value for full duplex*/
#define E1000_TCTL_COLD_BIT 12          /* initial bit of the TCTL.COLD */

#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TIPG_IPGT_BIT         0    /* initial bit of the TIPG.IPGT */
#define E1000_TIPG_IPGT_IEEE802p3   10   /* expected value for TIPG.IPGT in IEEE 802.3*/
/* IPGR1 and IPGR2 are not needed in full duplex, but are easier to always program to the values 
shown. */
#define E1000_TIPG_IPGR1_BIT        10   /* initial bit of the TIPG.IPGR1 */
#define E1000_TIPG_IPGR1_IEEE802p3  (2 * E1000_TIPG_IPGR2_IEEE802p3 / 3)   /* expected value for TIPG.IPGR1 in IEEE 802.3*/
#define E1000_TIPG_IPGR2_BIT        20   /* initial bit of the TIPG.IPGR2 */
#define E1000_TIPG_IPGR2_IEEE802p3  6    /* expected value for TIPG.IPGR2 in IEEE 802.3*/


#define NDESC 64 /* Descriptors in the transmit descriptor array */ 
#define PACKET_LEN 1518  /* Maximum size of an Ethernet packet in bytes */
typedef struct packet { char buffer[PACKET_LEN]; } packet_t;

extern struct tx_desc* tx_desc_array;
extern packet_t* buffers;

#define TDESC_STATUS_DD 1 /* DD field in the status word (TDESC.STATUS)*/
#define TDESC_CMD_RS_SET 0x8 /* value in the command word (TDESC.CMD) to set RS bit to advise
the Ethernet controller needs to report the status information */
#define TDESC_CMD_EOP_SET 0x1 /* value in the command word (TDESC.CMD) to set EOP bit to 
indicates the last descriptor making up the packet. */

struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

typedef struct tx_desc_array{
    uint32_t len;
	uint32_t actual;
    struct tx_desc* data;
} tx_desc_array_t;

#define E_QUEUE_FULL -1

int e1000_attachfn (struct pci_func *pcif);
void e1000_regs_init();
void tx_desc_array_init();
int transmit(void* packet, uint32_t len);
int tx_desc_array_add(void* packet, uint32_t len);

#endif  // JOS_KERN_E1000_H