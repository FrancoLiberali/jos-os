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