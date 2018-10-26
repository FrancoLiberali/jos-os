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

kern_idt
---------

...
	Para algunas traps el procesador pushea automaticamente al stack un codigo de error. En esos casos, se debe utilizar
TRAPHANDLER mientras que en los casos en los que no se debe utilizar TRAPHANDLER_NOEC ya que esta funcion pushea un $0 en el
lugar donde deberia ir el codigo de error. Si se utilizara solo la primera, aquellas traps que no tienen codigo de error
fallarian porque el stack no queda configurado de la manera que deberia segun la estructura de un TrapFrame.
	El parametro istrap de la macro SETGATE define el comportamiento del manejo del problema. Si se trata de una interrupcion
(istrap = 0), se resetea el valor de IF (interrupt-enable flag) lo cual previene que otras interrupciones interfieran con el
manejo de la primera. Luego, cuando se ejecuta iret, se restaura el valor de IF al valor que tenia EFLAGS en el stack. Por otra
parte, si se trata de una excepcion (istrap = 1), no se modifica el valor de IF.



user_evilhello
---------

...
	La diferencia entre estas versiones es que la primera utiliza directamente una direccion de memoria invalida como
parametro para sys_cputs mientras que la segunda utiliza una direccion valida (la de la variable first) pero cuyo contenido es la
direccion invalida.
