#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "revert_string.h"

int main(int argc, char *argv[])
{
	// Проверка количества аргументов
	if (argc != 2)
	{
		printf("Usage: %s string_to_revert\n", argv[0]);
		return -1;
	}

	// Выделение памяти в куче для копии строки
	char *reverted_str = malloc(sizeof(char) * (strlen(argv[1]) + 1));// +1 для терминального нуля
	strcpy(reverted_str, argv[1]);// Копируем строку из аргумента

	RevertString(reverted_str); // Переворачиваем строку

	printf("Reverted: %s\n", reverted_str);// Выводим результат
	free(reverted_str);// Освобождаем память
	return 0;
}

