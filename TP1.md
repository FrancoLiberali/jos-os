TP1: Memoria virtual en JOS
===========================

page2pa
-------

A partir de un puntero a un PageInfo (pp), se le resta el  puntero del comienzo del arreglo pages (pp - pages) para hallar su distancia y por lo tanto numero de pagina. Luego, realizando un shift de 12 bits hacia la izquierda (multiplicando por 4096) se setean los 12 bits mas bajos en cero y se obtiene la direccion de memoria fisica del comienzo de la pagina alineada correspondientemente, ya que el tamaño de cada pagina es justamente 4096 bytes.


boot_alloc_pos
--------------

Calculo manual:

Al hacer "nm kernel" se obtiene que end = 0xf0117950, la cual es la direccion a partir de la cual boot_alloc() alocara memoria.

Existen unicamente dos llamados a boot_alloc(). En el primero, se llama con PGSIZE = 2^12 = 4096. En el segundo, se llama con npages\*sizeof(struct PageInfo) = 2^8 * 2^10 = 2^18 = 262144. En total: 2^12 + 2^18 = 266240 = 0x41000.

Si se le suma este valor a end: 0xf0117950 + 0x41000 = 0xf0158950
Pero como boot_alloc() devolvera una direccion alineada a 12 bits, sera 0xf0159000

A continuacion, se muestra una sesión de GDB en la que, poniendo un breakpoint en la función boot_alloc(), se muestra el valor de end y nextfree al comienzo y fin de esa primera llamada a boot_alloc().

```
$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...done.
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100b27: file kern/pmap.c, line 89.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0100b27 <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:89
89	{
(gdb) b 119
Breakpoint 2 at 0xf0100b8b: file kern/pmap.c, line 119.
(gdb) p (void*) end
$1 = (void *) 0x10012
(gdb) x/x (void*) end
   0x10012:	0xf000ff53
(gdb) p nextfree
$2 = 0x0
(gdb) x nextfree
   0x0:	0xf000ff53
(gdb) c
Continuing.
=> 0xf0100b8b <boot_alloc+100>:	jmp    0xf0100b3f <boot_alloc+24>

Breakpoint 2, boot_alloc (n=281) at kern/pmap.c:120
120		}
(gdb) p (void*) end
$3 = (void *) 0x10012
(gdb) x (void*) end
   0x10012:	0xf000ff53
(gdb) p/x nextfree
$4 = 0xf0119000
```



page_alloc
----------

page2pa() devuelve, dado un puntero a una pagina, la direccion física en la cual comienza.
En cambio, page2kva() primero obtiene el mismo resultado llamando a page2pa() pero luego le suma KERNBASE. Con esto, se obtiene la direccion de memoria virtual del kernel que corresponde con la pagina pasada por parametro.

map_region_large
----------

De los tres llamados a boot_map_region, unicamente el ultimo hace uso de large pages ya que en los otros casos la direccion fisica no se encuentra alineada a 22 bits.

En ese llamado, el tamaño que se especifica es: size = 268435456 = 2^28.

Sin utilizar large pages:
La cantidad de page table entries necesarias para direccionar esa size es: 2^28 / 2^12 = 2^16 = 65536
En total, ocupa: 65536pte = 2^16 \* 2^2 = 2^18 = 262144 = 256k bytes
La cantidad de page tables necesarias para direccionar eso es: 2^16/2^10 = 2^6 = 64
Al necesitar 64 page tables, se necesitan 64 page directory entries, en total: 64pde = 2^6 * 2^2 = 2^8 = 256 bytes
Sumando todo, la cantidad de memoria total necesaria es: 2^18 + 2^8 = 262400 bytes

Utilizando large pages:
La cantidad de page directory entries necesarias para direccionar esa size es: 2^28 / 2^22 = 2^6 = 64
En total, ocupa: 64pde = 2^6 * 2^2 = 2^8 = 256 bytes

Haciendo la diferencia, se obtiene que 2^18 + 2^8 - 2^8 = 2^18 = 2^8 * 2^10 = 256k bytes es la cantidad de memoria que se ahorra utilizando large pages. Este valor corresponde con el tamanio de las page tables que no se utilizan en este modo.

Ademas, al quitar un nivel de indireccion, tambien se liberan cantidad de entradas en la TLB lo cual mejora la performance de las busquedas a memoria.

Es una cantidad fija que no depende de la memoria fisica total de la computadora ya que esta implementacion esta relacionada con la manera de traducir direcciones virtuales a fisicas, no con la cantidad que se quiere alocar.
