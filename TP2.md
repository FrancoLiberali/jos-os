TP2: Procesos de usuario
========================

env_alloc
---------

1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)
El código que se sigue para generar estos 5 identificadores es:
```
#define ENVGENSHIFT 12  // >= LOGNENV
#define LOG2NENV		10
#define NENV			(1 << LOG2NENV)
e = env_free_list
// Generate an env_id for this environment.
generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);
if (generation <= 0)  // Don't create a negative env_id.
    generation = 1 << ENVGENSHIFT;
e->env_id = generation | (e - envs);
env_free_list = e->env_link;
```
donde e->env\_id = 0 ya que estos envs no fueron allocados nunca aún.
```
1 << ENVGENSHIFT = 1 << 12 = 0x1000
~(NENV - 1) = ~(1 << LOG2NENV - 1) = ~(1 << 10 - 1) 
= ~(0x400 - 1) = ~(0x3FF) = 0xFFFFFC00
(0 + 1 << ENVGENSHIFT) & ~(NENV - 1)} = 0x1000 & 0xFFFFFC00 = 0x1000
generation = 0x1000
```
Con i = 0...4
```
e->env_id_i = generation | (e - envs) = 0x1000 | i
e->env_id_0 = 0x1000 | 0 = 0x1000
e->env_id_1 = 0x1000 | 1 = 0x1001
e->env_id_2 = 0x1000 | 2 = 0x1002
e->env_id_3 = 0x1000 | 3 = 0x1003
e->env_id_4 = 0x1000 | 4 = 0x1004
```

2. Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar. ¿Qué identificadores tendrá este proceso en sus sus primeras cinco ejecuciones?
Para empezar, el e->env_id_630 = 0x1000 | 0x276 = 0x1276 la primera vez que se lanza.
Siendo el subíndice la vez que se vuelve a allocar el envs[630]:
```
generation_1 = (0x1276 + 0x1000) & 0xFFFFFC00 = 0x2276 & 0xFFFFFC00 = 0x2000
e->env_id_1 = 0x2000 | 0x276 = 0x2276
generation_2 = (0x2276 + 0x1000) & 0xFFFFFC00 = 0x3276 & 0xFFFFFC00 = 0x3000
e->env_id_2 = 0x3000 | 0x276 = 0x3276
generation_3 = (0x3276 + 0x1000) & 0xFFFFFC00 = 0x4276 & 0xFFFFFC00 = 0x4000
e->env_id_3 = 0x4000 | 0x276 = 0x4276
generation_4 = (0x4276 + 0x1000) & 0xFFFFFC00 = 0x5276 & 0xFFFFFC00 = 0x5000
e->env_id_4 = 0x5000 | 0x276 = 0x5276
generation_5 = (0x5276 + 0x1000) & 0xFFFFFC00 = 0x6276 & 0xFFFFFC00 = 0x6000
e->env_id_5 = 0x6000 | 0x276 = 0x6276
```

env_init_percpu
---------------
La funcion lgdt escribe 3 bytes en el registro correspondiente a la global descriptor table GDTR. Los dos bytes mas bajos representan el tamanio de la gdt y el tercer, cuarto y quinto byte representan la base de la gdt y son lo que se escriben en el registro. El byte mas alto no se utiliza, y su correspondiente en el GDTR se setea en 0.


env_pop_tf
----------

1. ¿Qué hay en (%esp) tras el primer movl de la función?
Tras el primer movl se encuentran los valores de todos los registros del procesador: edi, esi, ebp, oesp, ebx, edx, ecx y eax.

2. ¿Qué hay en (%esp) justo antes de la instrucción iret? ¿Y en 8(%esp)?
Antes de iret, %esp apunta a eip, el entry point del proceso que se va a ejecutar. En 8(%esp) se encuentran los eflags: los flags del environment que se va a ejecutar.

3. ¿Cómo puede determinar la CPU si hay un cambio de ring (nivel de privilegio)?
El cambio de nivel de privilegio se detecta mirando los ultimos bits de la entrada a la gdt. Un cero corresponde a modo kernel, un tres, a modo usuario.


gdb_hello
---------
1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.
```
$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Leyendo símbolos desde obj/kern/kernel...hecho.
Remote debugging using 127.0.0.1:26000
0x0000fff0 in ?? ()
(gdb) b env_pop_tf
Punto de interrupción 1 at 0xf0102cf4: file kern/env.c, line 478.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0102cf4 <env_pop_tf>:	push   %ebp
Breakpoint 1, env_pop_tf (tf=0xf01d8000) at kern/env.c:478
478	{
```

2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.
```
(qemu) info registers
EAX=f01d8000 EBX=f01d8000 ECX=f0197000 EDX=00000000
ESI=00010074 EDI=00000000 EBP=f010dfd8 ESP=f010dfbc
EIP=f0102cf4 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3. De vuelta a GDB, imprimir el valor del argumento tf:
```
(gdb) p tf
$1 = (struct Trapframe *) 0xf01d8000
```

4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).
```
(gdb) x/17x tf
0xf01d8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01d8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01d8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01d8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01d8040:	0x00000023
```
5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:
```
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102cf4 <+0>:	push   %ebp
   0xf0102cf5 <+1>:	mov    %esp,%ebp
   0xf0102cf7 <+3>:	sub    $0xc,%esp
   0xf0102cfa <+6>:	mov    0x8(%ebp),%esp
   0xf0102cfd <+9>:	popa   
   0xf0102cfe <+10>:	pop    %es
   0xf0102cff <+11>:	pop    %ds
   0xf0102d00 <+12>:	add    $0x8,%esp
   0xf0102d03 <+15>:	iret   
   0xf0102d04 <+16>:	push   $0xf01051e8
   0xf0102d09 <+21>:	push   $0x1e8
   0xf0102d0e <+26>:	push   $0xf0105186
   0xf0102d13 <+31>:	call   0xf01000a9 <_panic>
End of assembler dump.
(gdb) si 4
=> 0xf0102cfd <env_pop_tf+9>:	popa   
0xf0102cfd	479		asm volatile("\tmovl %0,%%esp\n"
```
6. Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).
```
(gdb) x/17x $sp
0xf01d8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01d8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01d8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01d8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01d8040:	0x00000023
```
Si, es el mismo contenido.

7. Explicar con el mayor detalle posible cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.
Los primeros ocho 0x00000000 representan los primeros ocho registros del struct Trapframe, los registros de proposito general: %edi, %esi, %ebp, %oesp, %ebx, %edx, %ecx, %eax en este orden. Todos se encuentran en 0 porque la ejecución de este env aún no comenzó por lo que en ningun momento guardó nada en registros de proposito general.
   En los siguientes dos y en el ultimo encontramos un 0x00000023. Estos valores representan a los registros %es, %ds, %ss respectivamente. Estos son registros de segmento y fueron cargados con este valor en env_alloc con "GD_UD | 3", para que estos segmentos tengan la referencia a el user data segment y ademas se le agrega un tres en los dos bits mas bajos, el cual quiere significa privilegios de user mode.
Los dos siguientes se encuentran en cero, siendo estos el %trapno y el %err. Se encuentran en cero porque no ocurrió ninguna interupción si el env nisiquiera se esta ejecutando.
Luego encontramos un 0x00800020, el cual representa al %eip. Este numero es el entry point del env, la primera instrucción que va a ejecutar cuando inicie. El mismo fue cargado durante load_icode, y el valor se encontraba contenido dentro del ELF.
A continuación encontramos un 0x0000001b, el cual reprensenta al %cs. El code segment fue cargado durante env_alloc con "GD_UT | 3", para que tenga referencia al segmento user text y con privilegios de usuario.
El siguiente es un 0x00000000 de %eflags, nuevamente como aún no hubo ejecucón no tiene sentido que haya un vlaor allí.
Por ultimo tenemos un 0xeebfe000, el cual representa al %esp. El mismo fue cargado durante env_alloc apuntando al user stack top, ya que como se va a empezar a usar el stack debe iniciar desde el tope del mismo.

8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.
```
(gdb) si 4
=> 0xf0102d03 <env_pop_tf+15>:	iret   
0xf0102d03	479		asm volatile("\tmovl %0,%%esp\n"
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01d8030
EIP=f0102d03 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
Los cambios producidos son que en los registros de proposito general ya se pusieron los pertenecientes al env a ejecutar y que se ha cambiado un DPL de 0 (kernel mode) a 3 (user mode). Sin embargo, el %eip sigue apuntando a una instrucción del kernel, ya que aún falta ejecutar iret para el inicio de la ejecución del env se complete. 

9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.
```
(gdb) 
=> 0x800020:	cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
```

	1. Imprimir el valor del contador de programa con p $pc o p $eip
```
(gdb) p $pc
$1 = (void (*)()) 0x800020
```

	2. Cargar los símbolos de hello con symbol-file obj/user/hello
```
(gdb) symbol-file obj/user/hello
¿Cargar una tabla de símbolos nueva desde «obj/user/hello»? (y or n) y
Leyendo símbolos desde obj/user/hello...hecho.
Error in re-setting breakpoint 1: Función «env_pop_tf» no definida.
```

	3. Volver a imprimir el valor del contador de programa
```
(gdb) p $pc
$2 = (void (*)()) 0x800020 <_start>
```

	4. Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.
```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```
Ahora, todos los datos que se encontraban en el Trapframe del env se encuentran en el CPU, el cual se esta ejecutando ya que la siguiente instrucción a ejecutar apuntada por el %eip es el entry point del mismo. El cambio de contexto se ha realizado con exito.

10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.
```
(gdb) tbreak syscall
Punto de interrupción temporal 2 at 0x8009f3: file lib/syscall.c, line 8.
(gdb) c
Continuando.
=> 0x8009f3 <syscall>:	push   %ebp
Temporary breakpoint 2, syscall (num=num@entry=0, check=check@entry=0, 
    a1=a1@entry=4005551752, a2=13, a3=0, a4=0, a5=0) at lib/syscall.c:8
8	{
(gdb) disas
Dump of assembler code for function syscall:
   ...
=> 0x00800a01 <+17>:	mov    0x8(%ebp),%ecx
   0x00800a04 <+20>:	mov    0xc(%ebp),%ebx
   0x00800a07 <+23>:	mov    0x10(%ebp),%edi
   0x00800a0a <+26>:	mov    0x14(%ebp),%esi
   0x00800a0d <+29>:	int    $0x30
```
Como vemos, antes de ejecutar la interupción se cargan los valores recibidos por parametro en los registros correspondientes según la call convention para syscalls definida en JOS.
```
(gdb) si 4
=> 0x800a0d <syscall+29>:	int    $0x30
0x00800a0d	23		asm volatile("int %1\n"
(gdb) si
=> 0xf010364a:	push   $0x30
0xf010364a in ?? ()
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=efffffe8
EIP=f010364a EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
Como vemos, luego de la interrupción se ha efectuado un cambio de contexto y nuevamente la proxima instrucción ejecutar esta dentro del codigo del kernel, que será el codigo necesario para efectuar la syscall.

kern_idt
---------
Para algunas traps el procesador pushea automaticamente al stack un codigo de error. En esos casos, se debe utilizar TRAPHANDLER mientras que en los casos en los que no se debe utilizar TRAPHANDLER_NOEC ya que esta funcion pushea un $0 en el lugar donde deberia ir el codigo de error. Si se utilizara solo la primera, aquellas traps que no tienen codigo de error fallarian porque el stack no queda configurado de la manera que deberia segun la estructura de un TrapFrame.

El parametro istrap de la macro SETGATE define el comportamiento del manejo del problema. Si se trata de una interrupcion (istrap = 0), se resetea el valor de IF (interrupt-enable flag) lo cual previene que otras interrupciones interfieran con el manejo de la primera. Luego, cuando se ejecuta iret, se restaura el valor de IF al valor que tenia EFLAGS en el stack. Por otra parte, si se trata de una excepcion (istrap = 1), no se modifica el valor de IF.

Al correr make run-softint-nox se observa que se genero una excepcion del tipo General Protection en vez de Page Fault (que corresponde a la interrupcion 14). Esto se debe a que lanzar dicha interrupcion es privilegiado, es decir, solo puede lanzarse desde modo kernel. Como el programa se corre en modo usuario, se genera la excepcion de proteccion general por intentar utilizar una instruccin privilegiada.


user_evilhello
---------
La diferencia entre estas versiones es que la primera utiliza directamente una direccion de memoria invalida como parametro para sys_cputs, ya que 0xf010000c es una posición del kernel a la que un proceso de usuario no tiene acceso. La aquí sitada una direccion valida, ya que variable first se encuentra declarada en el stack de este proceso que intenta tener el contenido de la dirección invalida.

Durante la ejecución lo que cambia es que la primera va a lograr su cometido ya que aún no disponemos de protección de memoria durante un syscall (la cual se agrega con user_mem_check) mientras que la aquí sitada no podrá. Esto se debe a que la de evilhello va a intentar acceder a la memoria del kernel durante un syscall, y durante la ejecución del mismo el sistema se encuentra en modo kernel porque lo que si no se hace la debido protección el kernel tendrá acceso a esa posición. Por su parte, la opción aqui sitada intentará leer el contenido de la posición del kernel directamente desreferenciando su puntero, por lo que la lectura de la posición se estará intentando hacer desde user mode, generando una page fault por la MMU.