//gcc -Wall -Wextra -std=c11 -O2 sequential_min_max.c find_min_max.c utils.c -o sequential
/*./sequential 123 1000*/

#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  if (begin >= end) {
    return min_max;
  }

  for (unsigned int i = begin; i < end; ++i) {
    if (array[i] < min_max.min) min_max.min = array[i];
    if (array[i] > min_max.max) min_max.max = array[i];
  }

  return min_max;
}