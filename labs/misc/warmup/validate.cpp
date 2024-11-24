
#include "solution.h"
#include <iostream>

template <typename Solver>
static bool validate(Solver solver, int *arr, int N)
{
  int result = solver.solution(arr, N);

  if (result != (N * (N + 1)) / 2) {
    std::cerr << "Validation Failed. Result = " << result
              << ". Expected = " << (N * (N + 1)) / 2 << std::endl;
    return false;
  }

  return true;
}

int main() {
  constexpr int N = 1000;
  int arr[N];
  for (int i = 0; i < N; i++) {
    arr[i] = i + 1;
  }

  int failedValidations = !validate(Baseline{}, arr, N);
  failedValidations += !validate(Baseline128{}, arr, N);
  failedValidations += !validate(Baseline256{}, arr, N);
  failedValidations += !validate(Baseline512{}, arr, N);
  failedValidations += !validate(Formula{}, arr, N);

  std::cout << "Validation Complete" << std::endl;
  return failedValidations;
}
