#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here
volatile void *e1000;
struct tx_desc* tx_desc_array;
struct rx_desc* rx_desc_array;
tx_packet_t* tx_buffers;
rx_packet_t* rx_buffers;
uint32_t actual_idx;

static void setreg(uint32_t offset, uint32_t value)
{
  *((uint32_t*)(e1000 + offset)) = value;
}

/* Initialize each descriptor of the transmit descriptor array
matching each descriptor to its respective buffer in the tx_buffers array
and setting the DD field in the status word (TDESC.STATUS) to show TDESC is free 
*/
static void tx_desc_array_init() {
    for (int i = 0; i < TX_NDESC; i++){
        tx_desc_array[i].addr = (uint32_t) PADDR(tx_buffers + i);
		/* initialy free */
		tx_desc_array[i].status = TDESC_STATUS_DD;
	}
}

/* Perform the initialization steps described in section 
14.5 of Intel's Software Developer's Manual for the E1000 */
static void e1000_tx_regs_init(){
    setreg(E1000_TDBAL, (uint32_t) PADDR(tx_desc_array));
    setreg(E1000_TDBAH, 0);
    setreg(E1000_TDLEN, TX_NDESC * sizeof(struct tx_desc));
    setreg(E1000_TDH, 0);
    setreg(E1000_TDT, 0);

    setreg(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | 
                       E1000_TCTL_COLD_DUPLEX << E1000_TCTL_COLD_BIT);

    setreg(E1000_TIPG, E1000_TIPG_IPGT_IEEE802p3 << E1000_TIPG_IPGT_BIT |
                       E1000_TIPG_IPGR1_IEEE802p3 << E1000_TIPG_IPGR1_BIT | 
                       E1000_TIPG_IPGR2_IEEE802p3 << E1000_TIPG_IPGR2_BIT);

	tx_desc_array_init();
}

/* Initialize each descriptor of the receive descriptor array
matching each descriptor to its respective buffer in the rx_buffers array
*/
static void rx_desc_array_init() {
    for (int i = 0; i < RX_NDESC; i++){
        rx_desc_array[i].addr = (uint32_t) PADDR(rx_buffers + i);
	}
}

/* Perform the initialization steps described in section 
14.4 of Intel's Software Developer's Manual for the E1000 */
static void e1000_rx_regs_init(void){
    setreg(E1000_RAL, QEMU_DEFAULT_MAC_L);
    setreg(E1000_RAH, QEMU_DEFAULT_MAC_H | E1000_RAH_AV);
    
    for (int i = 1; i < 16; i++){
        setreg(E1000_RAL + 8 * i, 0);
        setreg(E1000_RAH + 8 * i, 0);
    }
    
    memset((void*)(e1000 + E1000_MTA), 0, E1000_MTA_LEN);

    setreg(E1000_IMS, E1000_IMS_DISABLE);

    setreg(E1000_RDBAL, (uint32_t) PADDR(rx_desc_array));
    setreg(E1000_RDBAH, 0);
    setreg(E1000_RDLEN, RX_NDESC * sizeof(struct rx_desc));

    rx_desc_array_init();

    setreg(E1000_RDH, 0);
    setreg(E1000_RDT, RX_NDESC - 1);

    setreg(E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_LBM_NO | //E1000_RCTL_LPE |
                       E1000_RCTL_BAM | E1000_RCTL_SZ_2048 |  
                       E1000_RCTL_SECRC);
}

int e1000_attachfn (struct pci_func *pcif){
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("E1000 Service status register %0x\n", *((uint32_t*)(e1000 + E1000_STATUS)));

    e1000_tx_regs_init();
    e1000_rx_regs_init();
    
    /*char packet1[] = "PRUEBA 1";
    transmit(packet1, sizeof(packet1));
    char packet2[] = "PRUEBA 2";
    transmit(packet2, sizeof(packet2));
    char packet3[] = "PRUEBA 3";
    transmit(packet3, sizeof(packet3));
    char packet4[] = "PRUEBA 4";
    transmit(packet4, sizeof(packet4));
    char packet5[] = "PRUEBA 5 MAS LARGA";
    transmit(packet5, sizeof(packet5));
    char packet6[2000] = "PRUEBA 6 PAQUETE QUE NO ENTRA";
    transmit(packet6, sizeof(packet6));
    for (int i = 0; i < 64; i++){
        transmit(packet5, sizeof(packet5));
    }*/

    return 0;
}

/* Tries to add a packet to the de tx_desc_array by checking that the 
next descriptor is free, copying the packet data into the next descriptor
Returns:
    -E_QUEUE_FULL if the transmit queue is full
    The new tail of the transmit queue that should be set to the TBT   
*/
static int tx_desc_array_add(void* packet, uint32_t len){
    int transmited_len, this_len;
    int new_actual_idx = actual_idx;
    for (transmited_len = 0; transmited_len < len; transmited_len += TX_PACKET_LEN){
        /* if the next one not is free */
        if ((tx_desc_array[new_actual_idx].status & TDESC_STATUS_DD) != TDESC_STATUS_DD){
            return -E_QUEUE_FULL;
        }
        /* no more free */
        tx_desc_array[new_actual_idx].status = 0;//tx_desc_array[new_actual_idx].status & ~(TDESC_STATUS_DD);
        if (len - transmited_len > TX_PACKET_LEN){
            this_len = TX_PACKET_LEN;
            /* set the RS field in the command word (TDESC.CMD) to advise
            the Ethernet controller needs to report the status information */
            tx_desc_array[new_actual_idx].cmd = TDESC_CMD_RS_SET;
        } else {
            this_len = len - transmited_len;
            /* set the RS field and the EOP because it is the last part of the packet */
            tx_desc_array[new_actual_idx].cmd = TDESC_CMD_RS_SET | TDESC_CMD_EOP_SET;
        }
        tx_desc_array[new_actual_idx].length = this_len;
        memmove(KADDR(tx_desc_array[new_actual_idx].addr), packet + transmited_len, this_len);
        new_actual_idx++;
        /* if reach the the end of the circular array */
        if (new_actual_idx == TX_NDESC){
            new_actual_idx = 0;
        }
    }
    actual_idx = new_actual_idx;
    return actual_idx;
}

/* Tries to transmit a packet by adding it to the tx_desc_array
and updating RDT
Returns:
    -E_QUEUE_FULL if the transmit queue is full
    0 otherwise 
*/
int e1000_try_transmit(void* packet, uint32_t len){
    int new_actual_idx = tx_desc_array_add(packet, len);
    if (new_actual_idx == -E_QUEUE_FULL){
        return -E_QUEUE_FULL;
    }
    setreg(E1000_TDT, new_actual_idx);
    return 0;
}

/* Tries to receive a packet by copying it out in u_buffer
and updating RDT 
Returns:
    -E_TRY_AGAIN if the receive queue is empty
    packet len > 0 otherwise 
*/
int e1000_try_receive(void* u_buffer){
    uint32_t rx_tail = *((uint32_t*)(e1000 + E1000_RDT));
    rx_tail++;
    if (rx_tail == RX_NDESC){
        rx_tail = 0;
    }
    // The next one not used by e1000
    if ((rx_desc_array[rx_tail].status & RDESC_STATUS_DD) != RDESC_STATUS_DD){
        return -E_TRY_AGAIN;
    } else {
        memmove(u_buffer, KADDR(rx_desc_array[rx_tail].addr), rx_desc_array[rx_tail].length);
        setreg(E1000_RDT, rx_tail);
        return rx_desc_array[rx_tail].length;
    }
}
