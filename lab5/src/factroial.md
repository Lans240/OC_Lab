# Параллельное вычисление факториала по модулю: полное руководство

## Математическая основа

### Факториал по модулю
Вычисление `k! mod m` — нахождение остатка от деления факториала числа `k` на `m`.

**Пример**: `10! mod 10 = 3,628,800 mod 10 = 0`

### Свойства модульной арифметики
- `(a * b) mod m = [(a mod m) * (b mod m)] mod m`
- Это позволяет избежать переполнения, беря модуль на промежуточных шагах

### Оптимизации
- Если `k >= m`, то `k! mod m = 0` (в произведении есть множитель `m`)
- Если `m = 1`, результат всегда `0`

## Полная реализация на C с pthreads

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Структура для передачи данных в поток
typedef struct {
    int start;
    int end;
    int mod;
    int partial_result;
} ThreadData;

// Глобальные переменные для синхронизации
pthread_mutex_t mutex;
int final_result = 1;

// Функция потока для вычисления частичного произведения
void* compute_factorial(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->partial_result = 1;
    
    for (int i = data->start; i <= data->end; i++) {
        data->partial_result = (data->partial_result * i) % data->mod;
    }
    
    printf("Поток: %d-%d, результат: %d\n", 
           data->start, data->end, data->partial_result);
    
    // Критическая секция - защищаем мьютексом
    pthread_mutex_lock(&mutex);
    final_result = (final_result * data->partial_result) % data->mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    int k = 0, pnum = 0, mod = 0;
    
    // Разбор аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = atoi(argv[++i]);
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoi(argv[i] + 6);
        }
    }
    
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Использование: %s -k <число> --pnum=<потоки> --mod=<модуль>\n", argv[0]);
        return 1;
    }
    
    // Проверка особых случаев
    if (mod == 1 || k >= mod) {
        printf("Результат: 0\n");
        return 0;
    }
    
    // Инициализация мьютекса
    pthread_mutex_init(&mutex, NULL);
    
    // Создание данных для потоков
    pthread_t threads[pnum];
    ThreadData thread_data[pnum];
    
    int numbers_per_thread = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;
    
    // Распределение работы между потоками
    for (int i = 0; i < pnum; i++) {
        int current_end = current_start + numbers_per_thread - 1;
        if (i < remainder) {
            current_end++;
        }
        
        thread_data[i].start = current_start;
        thread_data[i].end = current_end;
        thread_data[i].mod = mod;
        
        pthread_create(&threads[i], NULL, compute_factorial, &thread_data[i]);
        
        current_start = current_end + 1;
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Финальный результат: %d! mod %d = %d\n", k, mod, final_result);
    
    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
```

## Ключевые концепции

### 1. Структуры и оператор `->`

**Структура** - способ группировки данных:
```c
typedef struct {
    int start;          // начало диапазона
    int end;            // конец диапазона  
    int mod;            // модуль
    int partial_result; // частичный результат
} ThreadData;
```

**Оператор `->`** - доступ к полям структуры через указатель:
```c
ThreadData* data = ...;
data->partial_result = 1;  // эквивалентно (*data).partial_result = 1
```

### 2. Мьютексы (mutex)

**Мьютекс** - механизм синхронизации для защиты общих ресурсов:

```c
pthread_mutex_t mutex;  // объявление мьютекса

// Инициализация
pthread_mutex_init(&mutex, NULL);

// Захват мьютекса (вход в критическую секцию)
pthread_mutex_lock(&mutex);

// Критическая секция - код, который выполняется только одним потоком
final_result = (final_result * data->partial_result) % data->mod;

// Освобождение мьютекса
pthread_mutex_unlock(&mutex);

// Уничтожение мьютекса
pthread_mutex_destroy(&mutex);
```

### 3. Потоки (pthreads)

**Создание потока**:
```c
pthread_t thread;
ThreadData data;
pthread_create(&thread, NULL, compute_factorial, &data);
```

**Ожидание завершения потока**:
```c
pthread_join(thread, NULL);
```

## Алгоритм параллелизации

### Разделение работы
Для `k=10`, `pnum=4`:
```
Поток 1: 1 * 2 * 3 = 6 mod 10 = 6
Поток 2: 4 * 5 * 6 = 120 mod 10 = 0  
Поток 3: 7 * 8 * 9 = 504 mod 10 = 4
Поток 4: 10 = 10 mod 10 = 0
```

### Объединение результатов
```c
// Защищено мьютексом!
final_result = (1 * 6) % 10 = 6
final_result = (6 * 0) % 10 = 0
final_result = (0 * 4) % 10 = 0  
final_result = (0 * 0) % 10 = 0
```

## Компиляция и запуск

```bash
# Компиляция
gcc -o factorial factorial.c -lpthread

# Запуск
./factorial -k 10 --pnum=4 --mod=10
```

## Преимущества параллельного подхода

1. **Ускорение вычислений** для больших `k`
2. **Эффективное использование ресурсов** многопроцессорных систем
3. **Масштабируемость** - можно увеличивать количество потоков
4. **Предотвращение переполнения** за счет модульной арифметики

Этот подход демонстрирует ключевые концепции параллельного программирования: разделение данных, синхронизацию и объединение результатов.