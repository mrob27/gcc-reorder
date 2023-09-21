/*  ~/z-gcc/build/gcc/xgcc -O3 -B ~/z-gcc/build/gcc stream3.c -o ss  */
# include <stdio.h>
# include <limits.h>
# include <time.h>

#define CLK_RATE 2.3e9

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

static float in[MAXSIZE], out[MAXSIZE];

/* A two-dimensional array of integer values, for generating extra memory
   reads during the inner loop (without increasing FLOPs) */
#define IV_ROWS 7
int iv[IV_ROWS][MAXSIZE];

/* This union struct is for calling RDTSC and reinterpreting its two
   32-bit integer results as a single 64-bit integer                  */
typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} tsc_counter;

/* We define RDTSC using inline assembly language instruction rdtsc */
#define RDTSC(cpu_c)              \
  __asm__ __volatile__ ("rdtsc" : \
  "=a" ((cpu_c).int32.lo),        \
  "=d"((cpu_c).int32.hi))

long int test_function(void)
{
  float quasi_random = 0;
  float final_answer = 0;
  long int i, j;
  for (i=0; i<OUTER_LOOP_ITERS; i++) {
    quasi_random = quasi_random*quasi_random - 1.923432;
    final_answer += quasi_random + 1.0e-6 * out[(i%MAXSIZE)];
    for (j=0; j<MAXSIZE; j++) {
      out[j] = quasi_random * in[j]
                       + (float)(iv[0][j] + iv[1][j] + iv[2][j] + iv[3][j]
#if (IV_ROWS > 4)
                                          + iv[4][j] + iv[5][j] + iv[6][j]
#endif
      );
    }
  }
  return ((long int) final_answer);
}

int main()
{
  float quasi_random = 0;
  float final_answer = 0;
  long int i, j;
  tsc_counter tsc_start, tsc_stop;
  double total_time;

  printf("mem usage: %9.3e B",
    ((double) MAXSIZE) * ((double)(sizeof(float) * 2))
    + ((double) MAXSIZE) * ((double) IV_ROWS) * ((double)(sizeof(float))) );
  printf(", align in[] %08x  out[] %08x  int[] %08x\n",
                ((int)((long)in)), ((int)((long)out)), ((int)((long)iv)));
#if 0
  printf("int[%d..%d][]", 0, IV_ROWS-1);
  printf(" %08x %08x %08x %08x", ((int)((long)(&(iv[i][0])))),
    ((int)((long)(&(iv[i][1])))), ((int)((long)(&(iv[i][2])))),
    ((int)((long)(&(iv[i][3])))) );
#if (IV_ROWS > 4)
  printf(" %08x %08x %08x", ((int)((long)(&(iv[i][4])))),
    ((int)((long)(&(iv[i][5])))), ((int)((long)(&(iv[i][6])))) );
#endif
  printf("\n");
#endif

  /* All arrays should be initialized */
  for(i=0; i<MAXSIZE; i++) {
    /* Chaotic dynamics iteration, related to the Julia and Mandelbrot sets. */
    quasi_random = quasi_random*quasi_random - 1.923432;
    in[i] = quasi_random;
    for(j=0; j<IV_ROWS; j++) {
      iv[j][i] = j*1234 + i;
    }
  }

  RDTSC(tsc_start);
    final_answer += (float) test_function();
  RDTSC(tsc_stop);
  total_time = ((double)(tsc_stop.int64-tsc_start.int64)) / CLK_RATE;

  /* To prevent over-optimization on some machines and compilers, we
     quasi-randomly select an element of the output array out[] and
     print it. */
  j = (*((int *)(& final_answer))) & 0xFFFFFFF;
  final_answer = out[j % MAXSIZE];
  printf("We spent all this time calculating %g\n", final_answer);

  printf("test_function_time: %e\n", total_time);

  return 0;
}
