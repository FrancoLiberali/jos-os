TP2: Procesos de usuario
========================

env_alloc
---------

...


env_init_percpu
---------------
...
  La funcion lgdt escribe 3 bytes en el registro correspondiente a la global descriptor table GDTR. Los dos bytes mas bajos representan el tamanio de la gdt y el tercer, cuarto y quinto byte representan la base de la gdt y son lo que se escriben en el registro. El byte mas alto no se utiliza, y su correspondiente en el GDTR se setea en 0.


env_pop_tf
----------
...
	1. Tras el primer movl se encuentran los valores de todos los registros del procesador: edi, esi, ebp, oesp, ebx, edx, ecx y eax.
	2. Antes de iret, %esp apunta a eip, el entry point del proceso que se va a ejecutar. En 8(%esp) se encuentran los eflags: los flags del environment que se va a ejecutar.
	3. El cambio de nivel de privilegio se detecta mirando los ultimos bits de la entrada a la gdt. Un cero corresponde a modo kernel, un tres, a modo usuario.


gdb_hello
---------

...
