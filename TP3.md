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


