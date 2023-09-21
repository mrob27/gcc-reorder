/* */

#include <stdio.h>

__attribute__((const)) long avg(const long *A, int n)
{
  long sum = 0;
  for(int j=0; j<n; j++) {
    sum += A[j] * A[j];
  }
  return sum/((long)n);
}

void norm(long *restrict out, const long *restrict in, int n)
{
  for(int i=0; i<n; i++) {
    out[i] = in[i] * n * n / avg(in, n);
  }
}

#define SIZE 10000
int main(int argc, char **argv)
{
  long vec1[SIZE];
  long vec2[SIZE];
  int k;
  for(k=0; k<SIZE; k++) {
    vec1[k] = 1 + k;
  }
  norm(vec2, vec1, SIZE);
  for(k=0; k<SIZE; k+=SIZE/10) {
    printf("%5ld ", vec2[k]);
  }
  printf("\n");
  return 0;
}
