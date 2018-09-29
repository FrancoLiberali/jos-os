TP1: Memoria virtual en JOS
===========================

page2pa
-------

	A partir de un puntero a un PageInfo (pp), se le resta el puntero del comienzo del arreglo pages (pp - pages) para hallar su distancia. Luego, realizando un shift de 12 bits hacia la izquierda (multiplicando por 4096) se setean los 12 bits mas bajos en cero y se obtiene la direccion de memoria fisica del comienzo de la pagina alineada correspondientemente.


boot_alloc_pos
--------------

Calculo manual:

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
(gdb) b 123
Breakpoint 2 at 0xf0100b8b: file kern/pmap.c, line 123.
(gdb) p (void*) end
$1 = (void *) 0x10012
(gdb) x (void*) end
   0x10012:	add    (%eax),%eax
(gdb) p nextfree
$2 = 0x0
(gdb) x nextfree
   0x0:	push   %ebx
(gdb) c
Continuing.
=> 0xf0100b8b <boot_alloc+100>:	jmp    0xf0100b3f <boot_alloc+24>

Breakpoint 2, boot_alloc (n=281) at kern/pmap.c:123
123		return result;
(gdb) p (void*) end
$3 = (void *) 0x10012
(gdb) x (void*) end
   0x10012:	add    (%eax),%eax
(gdb) p nextfree
$4 = 0xf0119000 ""
(gdb) x nextfree
   0xf0119000:	add    %al,(%eax)
(gdb) 



page_alloc
----------

	page2pa() devuelve, dado un puntero a una pagina, la direccion fisica en la cual comienza.
	En cambio, page2kva() primero obtiene el mismo resultado llamando a page2pa() pero luego le suma KERNBASE. Con esto, se obtiene la direccion de memoria virtual del kernel que corresponde con la pagina pasada por parametro.

map_region_large
----------




