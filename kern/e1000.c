#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here

volatile void *e1000;
struct tx_desc* tx_desc_array;
uint32_t actual_idx;
packet_t* buffers;

int e1000_attachfn (struct pci_func *pcif){
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("E1000 Service status register %0x\n", *((uint32_t*)(e1000 + E1000_STATUS)));

    e1000_regs_init();

	tx_desc_array_init();
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
/* Perform the initialization steps described in section 
14.5 of Intel's Software Developer's Manual for the E1000 */
void e1000_regs_init(){
    *((uint32_t*)(e1000 + E1000_TDBAL)) = (uint32_t) PADDR(tx_desc_array);
    *((uint32_t*)(e1000 + E1000_TDBAH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDLEN)) = NDESC * sizeof(struct tx_desc);
    *((uint32_t*)(e1000 + E1000_TDBAH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDT)) = 0;

    *((uint32_t*)(e1000 + E1000_TCTL)) = *((uint32_t*)(e1000 + E1000_TCTL)) |
                                        E1000_TCTL_EN | 
                                        E1000_TCTL_PSP | 
                                        E1000_TCTL_COLD_DUPLEX << E1000_TCTL_COLD_BIT;

    *((uint32_t*)(e1000 + E1000_TIPG)) = *((uint32_t*)(e1000 + E1000_TIPG)) | 
                                        E1000_TIPG_IPGT_IEEE802p3 << E1000_TIPG_IPGT_BIT |
                                        E1000_TIPG_IPGR1_IEEE802p3 << E1000_TIPG_IPGR1_BIT | 
                                        E1000_TIPG_IPGR2_IEEE802p3 << E1000_TIPG_IPGR2_BIT;
    
}

/* Initialize each descriptor of the descriptor array
matching each descriptor to its respective buffer in the buffers array
and setting the DD field in the status word (TDESC.STATUS) to show TDESC is free 
*/
void tx_desc_array_init() {
    for (int i = 0; i < NDESC; i++){
        tx_desc_array[i].addr = (uint32_t) PADDR(buffers + i);
		/* initialy free */
		tx_desc_array[i].status = TDESC_STATUS_DD;
	}
}

/* Tries to transmit a packet by adding it to the tx_desc_array
and updating TDT 
Returns:
    -E_QUEUE_FULL if the transmit queue is full
    0 otherwise 
*/
int e1000_try_transmit(void* packet, uint32_t len){
    int new_actual_idx = tx_desc_array_add(packet, len);
    if (new_actual_idx == -E_QUEUE_FULL){
        return -E_QUEUE_FULL;
    }
    *((uint32_t*)(e1000 + E1000_TDT)) = new_actual_idx;
    return 0;
}

/* Tries to add a packet to the de tx_desc_array by checking that the 
next descriptor is free, copying the packet data into the next descriptor
Returns:
    -E_QUEUE_FULL if the transmit queue is full
    The new tail of the transmit queue that should be set to the TBT   
*/
int tx_desc_array_add(void* packet, uint32_t len){
    int transmited_len, this_len;
    int new_actual_idx = actual_idx;
    for (transmited_len = 0; transmited_len < len; transmited_len += PACKET_LEN){
        /* if the next one not is free */
        if ((tx_desc_array[new_actual_idx].status & TDESC_STATUS_DD) != TDESC_STATUS_DD){
            return -E_QUEUE_FULL;
        }
        /* no more free */
        tx_desc_array[new_actual_idx].status = 0;//tx_desc_array[new_actual_idx].status & ~(TDESC_STATUS_DD);
        if (len - transmited_len > PACKET_LEN){
            this_len = PACKET_LEN;
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
        if (new_actual_idx == NDESC){
            new_actual_idx = 0;
        }
    }
    actual_idx = new_actual_idx;
    return actual_idx;
}
