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

    *((uint32_t*)(e1000 + E1000_TDBAL)) = tx_desc_array;
    *((uint32_t*)(e1000 + E1000_TDBAH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDLEN)) = NDESC * sizeof(struct tx_desc);
    *((uint32_t*)(e1000 + E1000_TDBAH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDH)) = 0;
    *((uint32_t*)(e1000 + E1000_TDT)) = 0;

    *((uint32_t*)(e1000 + E1000_TCTL)) = E1000_TCTL_EN | E1000_TCTL_PSP |.....;




    return 0;
}
