#!/bin/bash

# Переходим в папку revert_string (где исходный код библиотеки)
cd revert_string

# Создаем библиотеку
echo "Создаем библиотеку..."
gcc -c -fPIC revert_string.c -o revert_string.o
gcc -shared revert_string.o -o librevert.so

# Возвращаемся назад и компилируем тесты
cd ..
cd tests

echo "Компилируем тесты..."
# Добавляем -lcunit для подключения библиотеки CUnit
gcc tests.c -I../revert_string -L../revert_string -lrevert -lcunit -o test_program

# Запускаем тесты
echo "Запускаем тесты..."
LD_LIBRARY_PATH=../revert_string ./test_program
