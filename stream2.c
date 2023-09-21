/*  ~/z-gcc/build/gcc/xgcc -O3 -B ~/z-gcc/build/gcc stream2 -o ss  */
# include <stdio.h>
# include <limits.h>
# include <time.h>

#define OUTER_LOOP_ITERS 200000
#define MAXSIZE 5000 /* inner loop, size of memory arrays */

static float in[MAXSIZE], out[MAXSIZE];  /* 4 MB each */

/* A two-dimensional array of iv values, for generating extra memory
   reads during the inner loop (without increasing FLOPs) */
#define IV_ROWS 7
int iv[IV_ROWS][MAXSIZE]; /* 28 MiB */

double interval(struct timespec start, struct timespec end)
{
  struct timespec temp;
  temp.tv_sec = end.tv_sec - start.tv_sec;
  temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  if (temp.tv_nsec < 0) {
    temp.tv_sec = temp.tv_sec - 1;
    temp.tv_nsec = temp.tv_nsec + 1000000000;
  }
  return (((double)temp.tv_sec) + ((double)temp.tv_nsec)*1.0e-9);
}

long int test_function(void)
{
  float quasi_random = 0;
  float final_answer = 0;
  long int i, j;
  for (i=0; i<OUTER_LOOP_ITERS; i++) {
    quasi_random = quasi_random*quasi_random - 1.923432;
    final_answer += quasi_random + 1.0e-6 * out[(i%MAXSIZE)];
    for (j=0; j<MAXSIZE; j++) {
      out[j] = quasi_random * in[j] + (float)(iv[0][j] + iv[1][j] + iv[2][j]
                                 + iv[3][j] + iv[4][j] + iv[5][j] + iv[6][j]);
    }
  }
  return ((long int) final_answer);
}

int main()
{
  float quasi_random = 0;
  float final_answer = 0;
  long int i, j;
  struct timespec time_start, time_stop;
  double total_time;

  printf("in[] %08x  out[] %08x  int[] %08x\n",
                ((int)((long)in)), ((int)((long)out)), ((int)((long)iv)));

  /* All arrays should be initialized */
  for(i=0; i<MAXSIZE; i++) {
    /* Chaotic dynamics iteration, related to the Julia and Mandelbrot sets. */
    quasi_random = quasi_random*quasi_random - 1.923432;
    in[i] = quasi_random;
    for(j=0; j<IV_ROWS; j++) {
      iv[j][i] = j*1234 + i;
    }
  }

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    final_answer += (float) test_function();
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
  total_time = interval(time_start, time_stop);

  /* To prevent over-optimization on some machines and compilers, we
     quasi-randomly select an element of the output array out[] and
     print it. */
  j = (*((int *)(& final_answer))) & 0xFFFFFFF;
  final_answer = out[j % MAXSIZE];
  printf("We spent all this time calculating %g\n", final_answer);

  printf("test_function_time: %e\n", total_time);

  return 0;
}
