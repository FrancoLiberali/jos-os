/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(divide_zero_handler, T_DIVIDE);
/*
from user-mode with not error code
                     +--------------------+ KSTACKTOP             
                     | 0x00000 | old SS   |     " - 4
                     |      old ESP       |     " - 8
                     |     old EFLAGS     |     " - 12
                     | 0x00000 | old CS   |     " - 16
                     |      old EIP       |     " - 20 <---- ESP 
                     +--------------------+   HANDLER
					 		  0                 " - 24
					        trapno              " - 28 <---- ESP

from user-mode with error code
					 +--------------------+ KSTACKTOP             
                     | 0x00000 | old SS   |     " - 4
                     |      old ESP       |     " - 8
                     |     old EFLAGS     |     " - 12
                     | 0x00000 | old CS   |     " - 16
                     |      old EIP       |     " - 20
                     |     error code     |     " - 24 <---- ESP
                     +--------------------+  HANDLER
					 		trapno              " - 28 <---- ESP

from kern-mode with no error code:
 					 +--------------------+ <---- old ESP
                     |     old EFLAGS     |     " - 4
                     | 0x00000 | old CS   |     " - 8
                     |      old EIP       |     " - 12
                     +--------------------+  HANDLER
					 		   0				" - 16
							trapno              " - 20 <---- ESP

from kern-mode with error code:
 					 +--------------------+ <---- old ESP
                     |     old EFLAGS     |     " - 4
                     | 0x00000 | old CS   |     " - 8
                     |      old EIP       |     " - 12
					 |     error code     |     " - 16
                     +--------------------+  HANDLER
					 		trapno              " - 20 <---- ESP
*/
/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	# push values to make the stack look like a struct Trapframe
	# inverse order because the stack grows to smaller positions
	/*pushl %eax
	pushl %ebx
	sub KSTACKTOP, %esp, %eax
	movl %ebx, 28
	cmp %eax, %ebx
	pop %ebx
	pop %eax
	je user_mode
kern_mode:
	jump common_mode
user_mode:
	
common_mode:
no es necesario, misma estructura de trapframe*/
/*	pushl %ss
	pushl %esp
	pushl %eflags
	pushl %cs
	pushl %eip
	pushl %esp
	ya pusheados*/
	pushl %ds
	pushl %es
	pushal
	# load GD_KD into %ds and %es
	mov $GD_KD, %ax
	mov %ax, %ds
	mov %ax, %es # creo que hay que hacer esto?

    pushl %esp # to pass a pointer to the Trapframe as an argument to trap()
    call trap # (can trap ever return?)

/*Consider using the pushal instruction;
it fits nicely with the layout of the struct Trapframe. 
Ayuda: cargar GD_KD en %ds y %es mediante un registro intermedio de 
16 bits (por ejemplo, %ax). Considerar, además, 
que GD_KD es una constante numérica,
 no una dirección de memoria (‘mov $GD_KD’ vs ‘mov GD_KD’).*/

