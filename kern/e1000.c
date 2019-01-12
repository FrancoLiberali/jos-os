#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile void *e1000;
struct tx_desc* tx_desc_array;
void* buffers;

int e1000_attachfn (struct pci_func *pcif){
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("E1000 Service status register %0x\n", *((uint32_t*)(e1000 + E1000_STATUS)));

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




    return 0;
}
