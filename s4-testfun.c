/*  ~/z-gcc/build/gcc/xgcc -O3 -B ~/z-gcc/build/gcc stream3.c -o ss  */
# include <stdio.h>
# include <limits.h>
# include <time.h>

#include "s4.h"

float in[MAXSIZE], out[MAXSIZE];

/* A two-dimensional array of integer values, for generating extra memory
   reads during the inner loop (without increasing FLOPs) */
int iv[IV_ROWS][MAXSIZE];

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
