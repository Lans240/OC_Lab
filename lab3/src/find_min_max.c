//gcc -Wall -Wextra -std=c11 -O2 sequential_min_max.c find_min_max.c utils.c -o sequential
/*# seed = 123, размер массива = 1000
./sequential 123 1000*/

#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  // Проверяем корректность диапазона: если begin >= end, возвращаем
  // min = INT_MAX, max = INT_MIN (т.е. пустой диапазон)
  if (begin >= end) {
    return min_max;
  }

  for (unsigned int i = begin; i < end; ++i) {
    if (array[i] < min_max.min) min_max.min = array[i];
    if (array[i] > min_max.max) min_max.max = array[i];
  }

  return min_max;
}

/* 2. Пояснение (что делает sequential_min_max.c и как работает GetMinMax)

sequential_min_max.c (файл проекта) читает 2 аргумента командной строки:

seed — целое положительное число для srand(seed), чтобы генерация массива была воспроизводимой;

arraysize — размер генерируемого массива.
Пример вызова: ./sequential 123 1000.

Программа выделяет массив array размера array_size, затем вызывает GenerateArray(array, array_size, seed) (функция в utils.c), которая заполняет массив псевдослучайными числами rand().

Затем вызывается GetMinMax(array, 0, array_size) — эта функция должна найти минимальный и максимальный элементы в диапазоне индексов [begin, end) (end — не включается). sequential_min_max.c передаёт весь массив (начиная с 0 до array_size), т.е. ищется глобальный минимум и максимум массива.

Результат (min и max) выводится в консоль.

Реализация GetMinMax в find_min_max.c:

Инициализирует min как INT_MAX и max как INT_MIN.

Если begin >= end — диапазон пустой, возвращаются начальные значения (INT_MAX/INT_MIN) — это корректное поведение для пустого диапазона (можно дополнительно обрабатывать как ошибку, но в этой задаче такое значение сигнализирует о пустом диапазоне).

Иначе проходит циклом for по индексам i = begin ... end-1, сравнивая array[i] с min и max, обновляя их при необходимости.*/