#!/bin/bash

# Статическая версия
echo "Building static version..."
# Компилируем файл revert_string.c в объектный файл
gcc -c revert_string.c 
# Создаем статическую библиотеку
ar rcs librevert.a revert_string.o
# Создаем программу со статической библиотекой
gcc main.c librevert.a -o program_static

# Динамическая версия  
echo "Building dynamic version..."
# Компилируем для динамической библиотеки
gcc -c -fPIC revert_string.c
# Создаем динамическую библиотеку
gcc -shared revert_string.o -o librevert.so
# Создаем программу с динамической библиотекой
gcc main.c -L. -lrevert -o program_dynamic

echo "Done! Run:"
echo "Static:  ./program_static 'Your text'"
echo "Dynamic: LD_LIBRARY_PATH=. ./program_dynamic 'Your text'"
