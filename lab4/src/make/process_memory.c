/* Программа для отображения адресной информации о процессе */
/* Адаптировано из Gray, J., программа 1.4 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Ниже определение макроса */
#define SHW_ADR(ID, I) (printf("ID %s \t находится по виртуальному адресу: %8X\n", ID, &I))

extern int etext, edata, end; /* Глобальные переменные для памяти процесса */

char *cptr = "Это сообщение выводится функцией showit()\n"; /* Статическая строка */
char buffer1[25];
int showit(); /* Прототип функции */

main() {
  int i = 0; /* Автоматическая переменная */

  /* Печать адресной информации */
  printf("\nАдрес etext: %8X \n", &etext);
  printf("Адрес edata: %8X \n", &edata);
  printf("Адрес end  : %8X \n", &end);

  SHW_ADR("main", main);
  SHW_ADR("showit", showit);
  SHW_ADR("cptr", cptr);
  SHW_ADR("buffer1", buffer1);
  SHW_ADR("i", i);

  strcpy(buffer1, "Демонстрация\n");   /* Функция из библиотеки */
  write(1, buffer1, strlen(buffer1) + 1); /* Системный вызов */

  showit(cptr);
} /* конец функции main */

/* Следует функция */
int showit(p)
char *p;
{
  char *buffer2;
  SHW_ADR("buffer2", buffer2);

  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) {
    printf("Выделена память по адресу %X\n", buffer2);
    strcpy(buffer2, p);    /* копирование строки */
    printf("%s", buffer2); /* вывод строки */
    free(buffer2);         /* освобождение памяти */
  } else {
    printf("Ошибка выделения памяти\n");
    exit(1);
  }
}
