#include "swap.h"

void Swap(char *left, char *right)
{
	    char temp = *left; // Сохраняем значение из a во временную переменную
    	*left = *right;        // Записываем значение из b в a
    	*right = temp;      // Восстанавливаем значение a в b из временной переменной
}

