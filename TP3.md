TP3: Multitarea con desalojo
========================

static_assert
---------
¿cómo y por qué funciona la macro static_assert que define JOS?
La definición de esta macro es:
```
// static_assert(x) will generate a compile-time error if 'x' is false.
#define static_assert(x)	switch (x) case 0: case (x):
```
De esta manera, cuando x sea false, false es equivalente a cero y por lo tanto tendremos un switch con un case 0 duplicado, y por lo tanto apareciendo el correspondiente error de compilación:
```
./inc/assert.h:18:45: error: duplicate case value
 #define static_assert(x) switch (x) case 0: case (x):
                                             ^
kern/pmap.c:330:2: note: in expansion of macro ‘static_assert’
  static_assert(MPENTRY_PADDR % PGSIZE == 0);
  ^
./inc/assert.h:18:37: error: previously used here
 #define static_assert(x) switch (x) case 0: case (x):
                                     ^
kern/pmap.c:330:2: note: in expansion of macro ‘static_assert’
  static_assert(MPENTRY_PADDR % PGSIZE == 0);
  ^
```
env_return
---------

Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

Al terminar un proceso su función umain() la ejecución del kernel retoma en el "JOS kernel monitor!".
Cuando termina umain() la ejecución del env retorna a libmain, la función que llama a los umain de los entornos de usuario. Luego de este llamado, libmain llama a exit(), la cual hace uso de syscall sys_env_destroy(). La misma realiza una syscall con el codigo SYS_env_destroy y el envid en 0 como parametro. Esta syscall sigué el camino usual de las syscall, es decir indexa en la IDT el codigo T_SYSCALL (ya que este es el codigo de la interrupción que generó la syscall), la cual lo lleva a la función syscall_trap, la que a su vez pushea el T_SYSCALL y salta a _alltraps. Allí, se termina de conformar la estructura trapframe y se la pasa por parametro a trap(). Esta a su vez llama a trap_dispatch() pasandole por parametro ese mismo trapframe, la cual chequeará que el trap number es T_SYSCALL y por lo tanto llamará al manejador de syscalls del kernel syscall(), pasandole por parametro el syscall number y envid en 0 que envió en env. Como el syscall number es SYS_env_destroy, se llama a sys_env_destroy con el envid en 0. Como el envid se encuentra en 0, el enviroment a destruir es el actual por lo que se hace print de que el env esta saliendo correctamente. A continuación, se llama a env_destroy y a su vez esta llama a env_free, la cual imprime que se esta liberando el environment y libera sus recursos. Luego env_destroy llama a sched_yield el cual actualmente solo llama a sched_halt, que busca entre los environments y como no encuentra ningun otro para ser corrido entonces imprime esa información y llama a monitor().

Podemos ver lo explicado anteriormente en gdb:
```
(gdb) b monitor
Punto de interrupción 1 at 0xf0100b11: file kern/monitor.c, line 113.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0100b11 <monitor>:	push   %ebp

Breakpoint 1, monitor (tf=0x0) at kern/monitor.c:113
113	{
(gdb) bt
#0  monitor (tf=0x0) at kern/monitor.c:113
#1  0xf0103fc9 in sched_halt () at kern/sched.c:56
#2  0xf0104040 in sched_yield () at kern/sched.c:34
#3  0xf0103320 in env_destroy (e=0xf02b8000) at kern/env.c:480
#4  0xf0104121 in sys_env_destroy (envid=40) at kern/syscall.c:63
#5  0xf0104173 in syscall (syscallno=3, a1=0, a2=0, a3=0, a4=0, a5=0)
    at kern/syscall.c:283
#6  0xf0103d03 in trap_dispatch (tf=0xf02b8000) at kern/trap.c:228
#7  0xf0103e77 in trap (tf=0xefffffbc) at kern/trap.c:308
#8  0xf0103f2d in _alltraps () at kern/trapentry.S:88
#9  0xefffffbc in ?? ()
```

¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

Cambia pricipalmente en dos cuentiones. La primera es que tiene en cuenta el caso en que se esta intentando destruit un environment que esta siendo ejecutado en otro CPU, al cual lo pone en modo zombie para que sea destruido por ese CPU. Por otro lado, antes llamaba directamente a monitor, mientras que ahora llama a sched_yield de manera que al destruir un env la idea es que se realice la planificación para que continue la ejecución de otro env, pero que actualmente por no estar implimentada resulta en monitor.

sys_yield
---------
```
$ make qemu-nox
***
*** Use Ctrl-a x to exit qemu
***
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log -smp 1  -d guest_errors
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
Hello, I am environment 00001001.
Hello, I am environment 00001002.
Back in environment 00001000, iteration 0.
Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001002, iteration 3.
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K> QEMU: Terminated
```

Como vemos, al principio se crea el env 00001000, entra en el for y hace sys_yield. Esto hace que el siguiente en la lista de envs sea 00001001, quien se crea, entra en el for y hace sys_yield. El env 00001002 hace lo mismo. Cuando este ultimo hace sys_yield, el siguiente en la lista es nuevamente 00001000, haciendo que se corran sucesivamente una vez cada uno, hasta terminar las iteraciones y destruirse en el mismo orden que se crearon.

envid2env
---------
¿Qué ocurre en JOS, si un proceso llama a sys_env_destroy(0)?
Al usar el envid en 0 como parametro se procederá a destruir el proceso actual, es decir el mismo que realizó esta syscall. Esto se interpreta como que el proceso terminó correctamente su ejecución y desea simplemente terminar.

¿Qué ocurre en Linux, si un proceso llama a kill(0, 9)?
Ya que el pid es 0, se enviará la señal 9, es decir, la señal de kill, a todos los procesos que esten en el mismo grupo que el proceso que lo llama. Este grupo incluye al proceso actual y cualquier proceso que hizo fork desde o a el proceso actual ("padres e hijos").

¿Qué ocurre en JOS, si un proceso llama a sys_env_destroy(-1)?
Al llamar a sys_env_destroy con este envid, se llamará a envid2env con el envid -1, resultando que chequeará si el ultimo env del arreglo de envs (envs[NENV-1]) tiene el envid -1, lo cual es imposible porque al allocar los envs nunca se genera un envid negativo, resultando en error.

¿Qué ocurre en Linux, si un proceso llama a kill(-1, 9)?
Si el pid es -1, la señal es enviada a todos los procesos a los cuales el proceso acctual tiene permiso de enviar señales, excepto por el proceso 1-init. De este manera se matarian todos los procesos para los que se tiene permisos.

dumbfork
--------

1. Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?.

	Realizar un llamado a sys_page_alloc previo a dumbfork no propagará una copia de la nueva pagina en el proceso hijo, ya que las paginas de memoria que se copian son las que estan entre UTEXT y end, de manera que se busca copiar solo el codigo del proceso. Luego tambien se copia la ultima página del stack, pero las paginas a copiar son fijas y no se copian demas páginas que no sean codigo o la ultima del stack.

2. ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no. (Ayuda: usar las variables globales uvpd y/o uvpt.)

	No se preserva el estado de solo-lectura de las páginas copiadas, ya que las paǵinas se allocan con sys_page_alloc(dstenv, addr, PTE_P|PTE_U|PTE_W) y como vemos se le ponen permisos de escritura a todas las páginas del nuevo proceso.
    Para saber si una dirección de memoria addr es modificable por el proceso o no, podemos utilizar el siguiente codigo.
    ```
    pde_t *pde = (pde_t *) (PGADDR(
		        PDX(uvpd), PTX(uvpd), (PDX(addr) * sizeof(pde_t))));
	// if the pt of addr was present, 
    // and with user and write permitions.
	if ((*pde) & (PTE_P | PTE_U | PTE_W)) {
		// uvpt let us enter to the page dir, because PDX(uvpt)
		// is index of
		// the recursively inserted PD in itself
		// PDX(addr) as PTX to index in the PD with the PDX, so
		// it let us in the physical PT where addr is
		// PTX(addr) * the size of the pte's)
		pte_t *pte =
		        (pte_t *) (PGADDR(PDX(uvpt),
		                          PDX(addr),
		                          (PTX(addr) * sizeof(pte_t))));
		// if the page of addr was present,
   		// and with user and write permitions.
		if ((*pte) & (PTE_P | PTE_U | PTE_W))
			return true;
	return false;
    ```
3. Describir el funcionamiento de la función duppage().
	La función dubppage, conta de tres pasos:
    * Allocar una nueva página en el nuevo proceso comenzando en la misma dirección que la que se quiere copiar del proceso padre(addr) con sys_page_alloc(dstenv, addr, PTE_P|PTE_U|PTE_W), dandole permiso de usuario y escritura.
    * Mapear la página virtual que comienza una dirección temporal (UTEMP) del espacio de direcciones del proceso padre a la misma página página fisica que la que tiene mapeada la página virtual que comienza en la dirección addr del proceso hijo, la cual fue allocada en esa página fisica en el proceso anterior. Esta acción se hace con sys_page_map(dstenv, addr, 0, UTEMP, PTE_P|PTE_U|PTE_W), y como vemos se mapea esta dirección temporal con permisos de escritura. 
    * Copiar el contenido del largo de una página(PGSIZE) de la posición addr del proceso padre en la dirección temporal UTEMP de este mismo proceso y, ya que esta comparte la página fisica con la dirección addr del proceso hijo, al escribir en esta página virtual del proceso padre se escribe también la del hijo. Esto se hace con memmove(UTEMP, addr, PGSIZE);
    * Desmapear la página virtual temporal del proceso padre, para que deje de apuntar a la misma página fisica que el proceso hijo. Esto se hace con sys_page_unmap(0, UTEMP).

4. Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo:

	* Indicar qué llamada adicional se debería hacer si el booleano es true:
	```
	sys_page_map(dstenv, addr, dstenv, addr, PTE_P|PTE_U);
    ```
    Para reinsertar la misma página fisica en la direccion addr con permisos distintos.
    
    * Describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).
    La manera alternativa podria ser la propuesta por la catedra para fork_v0, en los casos que la página tenga permisos de escritura hacer este mismo proceso y si era de solo lectura simplemente mapear la dirección del nuevo proceso a la misma página fisica del proceso padre.
    
5. ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?
	Para copiar el stack nos basamos en &addr ya que addr es una variable local de la función dumfork que existe en el stack, y por lo tanto su posición de memoria corresponde a una del stack. El uso de ROUNDDOWN es porque la dirección de inicio de todas las páginas virtuales donde este contenida una dirección es el ROUNDOWN ya que la página empieza donde el offset es cero. Esto, es independiente a que esta dirección sea del stack, ROUNDUP de una dirección siempre nos da el inicio de la paǵina siguiente, y si estamos en el stack esto nos daría la paǵina anterior del stack. De esta manera, para que el stack crezca hacia abajo, las paǵinas del mismo se deben utilizar de arriba hacia abajo pero el inicio de la paǵina sigue siendo la dirección más baja de la misma. 



