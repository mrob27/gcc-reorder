#define OUTER_LOOP_ITERS 200000
#define MAXSIZE 5000 /* inner loop, size of memory arrays */

/* Values of MAXSIZE and OUTER_LOOP_ITERS are adjusted together to
change the amount of memory that is accessed repeatedly, and thus the cache
hit/miss rates. Combinations I have used:
   MAXSIZE   OUT_L_I   memory
   1000000   1000      32 MiB
   100000    10000     3.2 MiB
   10000     100000    320 kiB
   5000      200000    160 kiB
   500       2000000   16 kiB
   416       10000000  15 kiB, deliberately longer runtime
 */

#define IV_ROWS 7
