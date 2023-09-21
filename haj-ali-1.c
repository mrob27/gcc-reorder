/* Example of code that can be optimised by loop-invariant code motion
and by function inlining, but in which these two optimisations are not
commutative: doing LICM followed by inlining gives a better result
than the other way around.

In GCC 10.2.0 this is much faster with -O1 than with -O2:

   : gcc --version
  gcc (MacPorts gcc10 10.2.0_5) 10.2.0
  Copyright (C) 2020 Free Software Foundation, Inc.
  This is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

   : gcc -O1 haj-ali-1.c -o ha && time ./ha
  0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 

  real  0m0.003s
  user  0m0.001s
  sys   0m0.001s
   : gcc -O2 haj-ali-1.c -o ha && time ./ha
  0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 

  real  0m0.113s
  user  0m0.111s
  sys   0m0.002s
   : 


*/

#include <math.h>
#include <stdio.h>

__attribute__((const))
double mag(const double *A, int n)
{
  double sum = 0;
  for(int j=0; j<n; j++) {
    sum += A[j] * A[j];
  }
  return sqrt(sum);
}

void norm(double *restrict out, const double *restrict in, int n)
{
  for(int i=0; i<n; i++) {
    out[i] = in[i] / mag(in, n);
  }
}

#define SIZE 10000
int main(int argc, char **argv)
{
  double vec1[SIZE];
  double vec2[SIZE];
  int k;
  for(k=0; k<SIZE; k++) {
    vec1[k] = 1.0 + k;
  }
  norm(vec2, vec1, SIZE);
  for(k=0; k<10; k++) {
    printf("%5.3f ", vec2[k]);
  }
  printf("\n");
  return 0;
}
