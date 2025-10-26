Да, это абсолютно нормальный и корректный результат! Программа работает правильно. Давайте разберем, что произошло:

## Проверка вычислений

**Разделение работы между серверами:**
- Сервер 1: `[1-6] = 1×2×3×4×5×6 = 720`
- Сервер 2: `[7-12] = 7×8×9×10×11×12 = 665280`
- Сервер 3: `[13-20] = 13×14×15×16×17×18×19×20 = 5079110400`

**Вычисления по модулю 100000:**
- Сервер 1: `720 mod 100000 = 720`
- Сервер 2: `665280 mod 100000 = 65280`
- Сервер 3: `5079110400 mod 100000 = 10400`

**Объединение результатов:**
```
720 × 65280 = 47001600
47001600 mod 100000 = 1600

1600 × 10400 = 16640000
16640000 mod 100000 = 40000
```

## Проверка правильности

Давайте проверим результат с помощью Python:
```python
import math

# Вычисляем 20! mod 100000
result = math.factorial(20) % 100000
print(f"20! mod 100000 = {result}")

# Проверяем промежуточные вычисления
part1 = math.factorial(6) % 100000
part2 = (math.factorial(12) // math.factorial(6)) % 100000
part3 = (math.factorial(20) // math.factorial(12)) % 100000

print(f"Part 1-6: {part1}")
print(f"Part 7-12: {part2}")
print(f"Part 13-20: {part3}")

total = (part1 * part2 * part3) % 100000
print(f"Combined result: {total}")
```

Вывод будет:
```
20! mod 100000 = 40000
Part 1-6: 720
Part 7-12: 65280
Part 13-20: 10400
Combined result: 40000
```

## Что демонстрирует этот результат:

1. **✅ Корректное распределение работы** - клиент правильно разделил диапазон [1-20] между тремя серверами
2. **✅ Параллельное выполнение** - серверы работали одновременно (порядок вывода "computed" может быть разным)
3. **✅ Правильные вычисления** - каждый сервер верно вычислил свой диапазон
4. **✅ Корректное объединение** - клиент правильно перемножил результаты по модулю

## Почему 20! mod 100000 = 40000?

Это математически верный результат. Если вы ожидали другое значение, возможно, была ошибка в ручных вычислениях.

**Вывод:** ваша распределенная система работает корректно! Все требования задания выполнены - вычисления факториала распараллелены между несколькими серверами, и клиент правильно объединяет результаты.



# Детальный разбор кода клиента и сервера

## Общая архитектура системы

Система состоит из:
- **Клиента** - распределяет вычисления между серверами
- **Серверов** - вычисляют части факториала параллельно
- **Протокола** - TCP-соединение с бинарной передачей данных

## Детальный разбор client.c

### 1. Структуры данных

```c
struct Server {
  char ip[255];  // IP-адрес сервера
  int port;      // Порт сервера
};

struct ThreadData {
  struct Server server;  // Информация о сервере
  uint64_t begin;        // Начало диапазона вычислений
  uint64_t end;          // Конец диапазона вычислений  
  uint64_t mod;          // Модуль для вычислений
  uint64_t result;       // Результат от сервера
  int success;           // Флаг успешного выполнения
};
```

### 2. Функция MultModulo

```c
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;  // Нормализуем a по модулю
  while (b > 0) {
    if (b % 2 == 1)      // Если b нечетное
      result = (result + a) % mod;  // Добавляем a к результату
    a = (a * 2) % mod;   // Удваиваем a
    b /= 2;              // Делим b пополам
  }
  return result % mod;
}
```

**Алгоритм**: Русское крестьянское умножение
- Сложность: O(log b)
- Преимущество: избегает переполнения при больших числах

### 3. Функция ProcessServer (потоковая функция)

```c
void* ProcessServer(void* arg) {
  struct ThreadData* data = (struct ThreadData*)arg;
  
  // DNS-запрос для получения IP-адреса
  struct hostent *hostname = gethostbyname(data->server.ip);
  
  // Настройка структуры адреса сервера
  struct sockaddr_in server;
  server.sin_family = AF_INET;  // IPv4
  server.sin_port = htons(data->server.port);  // Порт в сетевом порядке байт
  server.sin_addr = *((struct in_addr *)hostname->h_addr_list[0]);  // IP-адрес
  
  // Создание TCP-сокета
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  
  // Установка соединения
  connect(sck, (struct sockaddr *)&server, sizeof(server));
  
  // Формирование задачи: 3 числа uint64_t
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &data->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));
  
  // Отправка задачи
  send(sck, task, sizeof(task), 0);
  
  // Получение ответа
  char response[sizeof(uint64_t)];
  recv(sck, response, sizeof(response), 0);
  memcpy(&data->result, response, sizeof(uint64_t));
  
  close(sck);  // Закрытие соединения
  data->success = 1;
  pthread_exit(NULL);
}
```

### 4. Главная функция клиента

```c
int main(int argc, char **argv) {
  // Разбор аргументов командной строки
  while (true) {
    static struct option options[] = {
      {"k", required_argument, 0, 0},        // Факториал k!
      {"mod", required_argument, 0, 0},      // Модуль
      {"servers", required_argument, 0, 0},  // Файл с серверами
      {0, 0, 0, 0}
    };
    // ... разбор аргументов
  }
  
  // Чтение файла с серверами
  FILE* file = fopen(servers_file, "r");
  while (fgets(line, sizeof(line), file)) {
    // Формат: ip:port
    sscanf(line, "%254[^:]:%d", ip, &port);
  }
  
  // Распределение работы между серверами
  uint64_t range_size = k / servers_num;
  uint64_t remainder = k % servers_num;
  
  for (int i = 0; i < servers_num; i++) {
    thread_data[i].begin = i * range_size + 1;
    thread_data[i].end = (i + 1) * range_size;
    if (i == servers_num - 1) {
      thread_data[i].end += remainder;  // Последний сервер получает остаток
    }
    
    // Создание потока для каждого сервера
    pthread_create(&threads[i], NULL, ProcessServer, &thread_data[i]);
  }
  
  // Ожидание завершения всех потоков
  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    if (thread_data[i].success) {
      // Объединение результатов по модулю
      total_result = MultModulo(total_result, thread_data[i].result, mod);
    }
  }
}
```

## Детальный разбор server.c

### 1. Структура данных для потоков

```c
struct FactorialArgs {
  uint64_t begin;  // Начало диапазона
  uint64_t end;    // Конец диапазона  
  uint64_t mod;    // Модуль
};
```

### 2. Функция Factorial

```c
uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  for (uint64_t i = args->begin; i <= args->end; i++) {
    ans = MultModulo(ans, i, args->mod);
  }
  return ans;
}
```

**Вычисление**: `begin × (begin+1) × ... × end mod mod`

### 3. Потоковая функция ThreadFactorial

```c
void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t *result = malloc(sizeof(uint64_t));  // Выделение памяти для результата
  *result = Factorial(fargs);  // Вычисление факториала
  return (void *)result;  // Возврат указателя на результат
}
```

**Важно**: Результат возвращается через динамическую память, так как потоки не могут возвращать значения напрямую.

### 4. Главная функция сервера

```c
int main(int argc, char **argv) {
  // Настройка серверного сокета
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);  // Принимать соединения на все интерфейсы
  
  // Привязка сокета к адресу и порту
  bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  
  // Переход в режим прослушивания
  listen(server_fd, 128);  // Очередь до 128 соединений
  
  // Основной цикл сервера
  while (true) {
    // Принятие входящего соединения
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);
    
    // Получение задачи от клиента
    char from_client[sizeof(uint64_t) * 3];
    recv(client_fd, from_client, sizeof(from_client), 0);
    
    // Извлечение begin, end, mod из полученных данных
    uint64_t begin, end, mod;
    memcpy(&begin, from_client, sizeof(uint64_t));
    memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
    memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));
    
    // Распределение работы между потоками сервера
    uint64_t range = end - begin + 1;
    uint64_t step = range / tnum;        // Базовый размер диапазона
    uint64_t remainder = range % tnum;   // Остаток
    
    for (uint32_t i = 0; i < tnum; i++) {
      args[i].begin = begin + i * step;
      args[i].end = begin + (i + 1) * step - 1;
      if (i == tnum - 1) {
        args[i].end += remainder;  // Последний поток получает остаток
      }
      args[i].mod = mod;
      
      // Создание потока для вычисления
      pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]);
    }
    
    // Сбор результатов от всех потоков
    uint64_t total = 1;
    for (uint32_t i = 0; i < tnum; i++) {
      uint64_t *result_ptr;
      pthread_join(threads[i], (void **)&result_ptr);  // Ожидание завершения потока
      total = MultModulo(total, *result_ptr, mod);     // Объединение результатов
      free(result_ptr);  // Освобождение памяти
    }
    
    // Отправка результата клиенту
    char buffer[sizeof(total)];
    memcpy(buffer, &total, sizeof(total));
    send(client_fd, buffer, sizeof(total), 0);
    
    close(client_fd);  // Закрытие соединения с клиентом
  }
}
```

## Процессы и потоки в системе

### Клиент:
```
Главный процесс клиента
    │
    ├── Поток 1: Обработка сервера 1 (127.0.0.1:20001)
    ├── Поток 2: Обработка сервера 2 (127.0.0.1:20002) 
    └── Поток 3: Обработка сервера 3 (127.0.0.1:20003)
```

### Каждый сервер:
```
Процесс сервера
    │
    ├── Главный поток: Прием соединений
    │   └── При соединении клиента:
    │       ├── Поток 1: Вычисление диапазона [X-Y]
    │       ├── Поток 2: Вычисление диапазона [Y-Z]
    │       └── ...
```

## Протокол обмена данными

### Запрос от клиента к серверу:
```
[8 байт: begin][8 байт: end][8 байт: mod]
```

### Ответ от сервера к клиенту:
```
[8 байт: результат]
```

## Распределение вычислений

### Пример для k=20, 3 сервера, 2 потока на сервер:

**Клиент распределяет:**
- Сервер 1: [1-6]
- Сервер 2: [7-12] 
- Сервер 3: [13-20]

**Каждый сервер распределяет дальше:**
- Сервер 1: Поток1 [1-3], Поток2 [4-6]
- Сервер 2: Поток1 [7-9], Поток2 [10-12]
- Сервер 3: Поток1 [13-16], Поток2 [17-20]

## Критические секции и синхронизация

### В клиенте:
- Нет глобальных разделяемых данных между потоками
- Каждый поток работает со своей структурой ThreadData
- Главный поток только читает результаты после завершения рабочих потоков

### В сервере:
- Потоки вычисляют независимые диапазоны
- Главный поток собирает результаты после завершения всех рабочих потоков
- Используется `pthread_join` для синхронизации

## Обработка ошибок

### В клиенте:
- Проверка корректности аргументов
- Проверка возможности открытия файла серверов
- Обработка ошибок сетевого соединения
- Проверка успешности выполнения на каждом сервере

### В сервере:
- Проверка корректности аргументов
- Обработка ошибок создания сокета
- Проверка формата полученных данных
- Обработка ошибок создания потоков

## Преимущества архитектуры

1. **Масштабируемость**: Можно добавлять новые серверы без изменения кода
2. **Параллелизм**: Вычисления происходят одновременно на нескольких серверах и внутри каждого сервера
3. **Отказоустойчивость**: Клиент продолжает работу даже если некоторые серверы недоступны
4. **Эффективность**: Используется оптимальный алгоритм умножения по модулю

Эта архитектура демонстрирует классический подход к распределенным вычислениям с использованием модели "мастер-воркер", где клиент (мастер) распределяет задачи между серверами (воркерами).