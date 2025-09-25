#include "revert_string.h"
#include <string.h>
#include <stdlib.h>

void RevertString(char *str)
{
    int length = strlen(str);
    int i, j;
    
    // Меняем симметричные символы местами от краев к центру
    for (i = 0, j = length - 1; i < j; i++, j--)
    {
        // Замена через временную переменную
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}