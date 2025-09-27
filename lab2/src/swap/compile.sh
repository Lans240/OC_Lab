gcc -c main.c -o main.o
gcc -c swap.c -o swap.o

gcc main.o swap.o -o program -L../revert_string -lrevert

export LD_LIBRARY_PATH=../revert_string:$LD_LIBRARY_PATH

./program