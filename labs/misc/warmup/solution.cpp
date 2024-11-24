#include "solution.h"

int Formula::solution(int *arr, int N) {
  return (N * (N + 1)) / 2;
}

int Baseline::solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
}

// The project is built with `-march=native`, or equivalent, so fudge with individual method targets
// to see what each level of vectorization does. Also put in a little bit of protection to avoid
// running commands that don't exist, defaulting back to the baseline solution.
//
#ifdef __x86_64__
[[gnu::target("arch=x86-64")]]
int Baseline128::solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
}
#else
int Baseline128::solution(int *arr, int N) { return Baseline::solution(arr, N); }
#endif

#ifdef __AVX2__
[[gnu::target("arch=x86-64"), gnu::target("avx2")]]
int Baseline256::solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
}
#else
int Baseline256::solution(int *arr, int N) { return Baseline::solution(arr, N); }
#endif

#ifdef __AVX512F__
[[gnu::target("arch=x86-64"), gnu::target("avx512f")]]
int Baseline512::solution(int *arr, int N) {
  int res = 0;
  for (int i = 0; i < N; i++) {
    res += arr[i];
  }
  return res;
}
#else
int Baseline512::solution(int *arr, int N) { return Baseline::solution(arr, N); }
#endif
