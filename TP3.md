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

$ make run-hello-nox-gdb
make[1]: se entra en el directorio '/home/franco/Documentos/sisop/tps/jos'
make[1]: se sale del directorio '/home/franco/Documentos/sisop/tps/jos'
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log -smp 1  -d guest_errors -S
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
hello, world
i am environment 00001000
[00001000] exiting gracefully
[00001000] free env 00001000
No runnable environments in the system!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K> QEMU: Terminated


(gdb) b env_free
Punto de interrupción 1 at 0xf010316b: file kern/env.c, line 413.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf010316b <env_free>:	push   %ebp

Breakpoint 1, env_free (e=0xf02b8000) at kern/env.c:413
413	{
(gdb) bt
#0  env_free (e=0xf02b8000) at kern/env.c:413
#1  0xf01032f6 in env_destroy (e=0xf02b8000) at kern/env.c:476
#2  0xf0104121 in sys_env_destroy (envid=0) at kern/syscall.c:63
#3  0xf0104173 in syscall (syscallno=3, a1=0, a2=0, a3=0, a4=0, a5=0)
    at kern/syscall.c:283
#4  0xf0103d03 in trap_dispatch (tf=0xf02b8000) at kern/trap.c:228
#5  0xf0103e77 in trap (tf=0xefffffbc) at kern/trap.c:308
#6  0xf0103f2d in _alltraps () at kern/trapentry.S:88
#7  0xefffffbc in ?? ()
Backtrace stopped: previous frame inner to this frame (corrupt stack?)

(gdb) symbol-file obj/user/hello
¿Cargar una tabla de símbolos nueva desde «obj/user/hello»? (y or n) y
Leyendo símbolos desde obj/user/hello...hecho.
(gdb) b umain
Punto de interrupción 1 at 0x800033: file user/hello.c, line 6.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0x800033 <umain>:	push   %ebp

Breakpoint 1, umain (argc=0, argv=0x0) at user/hello.c:6
6	{
(gdb) disas
Dump of assembler code for function umain:
=> 0x00800033 <+0>:	push   %ebp
   0x00800034 <+1>:	mov    %esp,%ebp
   0x00800036 <+3>:	sub    $0x14,%esp
   0x00800039 <+6>:	push   $0x800ee0
   0x0080003e <+11>:	call   0x80014a <cprintf>
   0x00800043 <+16>:	call   0x800aba <sys_getenvid>
   0x00800048 <+21>:	add    $0x8,%esp
   0x0080004b <+24>:	push   %eax
   0x0080004c <+25>:	push   $0x800eee
   0x00800051 <+30>:	call   0x80014a <cprintf>
   0x00800056 <+35>:	add    $0x10,%esp
   0x00800059 <+38>:	leave  
   0x0080005a <+39>:	ret    
End of assembler dump.
(gdb) bt
#0  umain (argc=0, argv=0x0) at user/hello.c:6
#1  0x00800096 in libmain (argc=0, argv=0x0) at lib/libmain.c:25
#2  0x00800031 in args_exist () at lib/entry.S:33




