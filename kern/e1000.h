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
#define E1000_STATUS 0x00008 /* Device Status - RO */

#define E1000_TDBAL 0x03800 /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH 0x03804 /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN 0x03808 /* TX Descriptor Length - RW */
#define E1000_TDH 0x03810   /* TX Descriptor Head - RW */
#define E1000_TDT 0x03818   /* TX Descripotr Tail - RW */

#define E1000_TCTL 0x00400         /* TX Control - RW */
#define E1000_TCTL_EN 0x00000002   /* enable tx */
#define E1000_TCTL_PSP 0x00000008  /* pad short packets */
#define E1000_TCTL_COLD 0x003ff000 /* collision distance */
#define E1000_TCTL_COLD_DUPLEX                                                 \
	0x40                   /* collision distance value for full duplex*/
#define E1000_TCTL_COLD_BIT 12 /* initial bit of the TCTL.COLD */

#define E1000_TIPG 0x00410    /* TX Inter-packet gap -RW */
#define E1000_TIPG_IPGT_BIT 0 /* initial bit of the TIPG.IPGT */
#define E1000_TIPG_IPGT_IEEE802p3                                              \
	10 /* expected value for TIPG.IPGT in IEEE 802.3*/
/* IPGR1 and IPGR2 are not needed in full duplex, but are easier to always
program to the values
shown. */
#define E1000_TIPG_IPGR1_BIT 10 /* initial bit of the TIPG.IPGR1 */
#define E1000_TIPG_IPGR1_IEEE802p3                                             \
	(2 * E1000_TIPG_IPGR2_IEEE802p3 /                                      \
	 3)                     /* expected value for TIPG.IPGR1 in IEEE 802.3*/
#define E1000_TIPG_IPGR2_BIT 20 /* initial bit of the TIPG.IPGR2 */
#define E1000_TIPG_IPGR2_IEEE802p3                                             \
	6 /* expected value for TIPG.IPGR2 in IEEE 802.3*/

#define E1000_RDBAL 0x02800 /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH 0x02804 /* RX Descriptor Base Address High - RW */
/* Receive Address */
#define E1000_RAH_AV 0x80000000 /* Receive descriptor valid */
#define E1000_RDLEN 0x02808     /* RX Descriptor Length - RW */
#define E1000_RDH 0x02810       /* RX Descriptor Head - RW */
#define E1000_RDT 0x02818       /* RX Descriptor Tail - RW */

#define E1000_MTA 0x05200      /* Multicast Table Array - RW Array */
#define E1000_MTA_LEN 4096 / 8 /* Multicast Table Array len in bytes */
#define E1000_RAL 0x05400      /* Receive Address Low 0 - RW*/
#define E1000_RAH 0x05404      /* Receive Address High 0 - RW*/
#define QEMU_DEFAULT_MAC_L                                                     \
	0x12005452 /* the low-order 32 bits of the QEMU default MAC address */
#define QEMU_DEFAULT_MAC_H                                                     \
	0x5634 /* the high-order 16 bits of the QEMU default MAC address */
#define E1000_IMS 0x000D0   /* Interrupt Mask Set - RW */
#define E1000_IMS_DISABLE 0 /* Value in the IMS to disable interumps */

#define E1000_RCTL 0x00100 /* RX Control - RW */
/* Receive Control */
#define E1000_RCTL_EN 0x00000002             /* enable */
#define E1000_RCTL_SBP 0x00000004            /* store bad packet */
#define E1000_RCTL_UPE 0x00000008            /* unicast promiscuous enable */
#define E1000_RCTL_MPE 0x00000010            /* multicast promiscuous enab */
#define E1000_RCTL_LPE 0x00000020            /* long packet enable */
#define E1000_RCTL_LBM_NO 0x00000000         /* no loopback mode */
#define E1000_RCTL_LBM_MAC 0x00000040        /* MAC loopback mode */
#define E1000_RCTL_LBM_SLP 0x00000080        /* serial link loopback mode */
#define E1000_RCTL_LBM_TCVR 0x000000C0       /* tcvr loopback mode */
#define E1000_RCTL_DTYP_MASK 0x00000C00      /* Descriptor type mask */
#define E1000_RCTL_DTYP_PS 0x00000400        /* Packet Split descriptor */
#define E1000_RCTL_RDMTS_HALF 0x00000000     /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_QUAT 0x00000100     /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_EIGTH 0x00000200    /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_RESERVED 0x00000300 /* RDMTS reserved */
#define E1000_RCTL_MO_SHIFT 12               /* multicast offset shift */
#define E1000_RCTL_MO_0 0x00000000           /* multicast offset 11:0 */
#define E1000_RCTL_MO_1 0x00001000           /* multicast offset 12:1 */
#define E1000_RCTL_MO_2 0x00002000           /* multicast offset 13:2 */
#define E1000_RCTL_MO_3 0x00003000           /* multicast offset 15:4 */
#define E1000_RCTL_MDR 0x00004000            /* multicast desc ring 0 */
#define E1000_RCTL_BAM 0x00008000            /* broadcast enable */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 0 */
#define E1000_RCTL_SZ_2048 0x00000000 /* rx buffer size 2048 */
#define E1000_RCTL_SZ_1024 0x00010000 /* rx buffer size 1024 */
#define E1000_RCTL_SZ_512 0x00020000  /* rx buffer size 512 */
#define E1000_RCTL_SZ_256 0x00030000  /* rx buffer size 256 */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 1 */
#define E1000_RCTL_SZ_16384 0x00010000    /* rx buffer size 16384 */
#define E1000_RCTL_SZ_8192 0x00020000     /* rx buffer size 8192 */
#define E1000_RCTL_SZ_4096 0x00030000     /* rx buffer size 4096 */
#define E1000_RCTL_VFE 0x00040000         /* vlan filter enable */
#define E1000_RCTL_CFIEN 0x00080000       /* canonical form enable */
#define E1000_RCTL_CFI 0x00100000         /* canonical form indicator */
#define E1000_RCTL_DPF 0x00400000         /* discard pause frames */
#define E1000_RCTL_PMCF 0x00800000        /* pass MAC control frames */
#define E1000_RCTL_BSEX 0x02000000        /* Buffer size extension */
#define E1000_RCTL_SECRC 0x04000000       /* Strip Ethernet CRC */
#define E1000_RCTL_FLXBUF_MASK 0x78000000 /* Flexible buffer size */
#define E1000_RCTL_FLXBUF_SHIFT 27        /* Flexible buffer shift */

#define TX_NDESC 64        /* Descriptors in the transmit descriptor array */
#define RX_NDESC 128       /* Descriptors in the receive descriptor array */
#define TX_PACKET_LEN 1518 /* Maximum size of an Ethernet packet in bytes */
typedef struct tx_packet {
	char buffer[TX_PACKET_LEN];
} tx_packet_t;
#define RX_PACKET_LEN                                                          \
	2048 /* Minimum E1000 allow rx buffer size that is                     \
bigger than the maximum size of an Ethernet packet in bytes */
typedef struct rx_packet {
	char buffer[RX_PACKET_LEN];
} rx_packet_t;

#define TDESC_STATUS_DD 1 /* DD field in the status word (TDESC.STATUS)*/
#define TDESC_CMD_RS_SET                                                       \
	0x8 /* value in the command word (TDESC.CMD) to set RS bit to advise   \
the Ethernet controller needs to report the status information */
#define TDESC_CMD_EOP_SET                                                      \
	0x1 /* value in the command word (TDESC.CMD) to set EOP bit to         \
indicates the last descriptor making up the packet. */

#define RDESC_STATUS_DD 1 /* DD field in the status word (RDESC.STATUS)*/

struct tx_desc {
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

struct rx_desc {
	uint64_t addr;
	uint16_t length;
	uint16_t packet_checksum;
	uint8_t status;
	uint8_t errors;
	uint16_t special;
};

int e1000_attachfn(struct pci_func *pcif);
int e1000_try_transmit(void *packet, uint32_t len);
int e1000_try_receive(void *u_buffer, size_t len);
#endif  // JOS_KERN_E1000_H