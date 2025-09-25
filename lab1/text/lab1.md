# Лабораторная работа №1

## Задание 1

### Необходимые знания

* `&` в `shell`
* Как работает команда `cat`
* Как работает команда `clear`
* Как работает команда `wc`

### Задание

* Запустите скрипт `background.sh` в фоновом режиме.
* Создайте текстовый файл и выведите его содержимое на экран терминала.
* Очистите окно терминала.
* Посчитайте количество символов в файле.


### Ресурсы

* [Небольшой справочник по основным утилитам командной строки, которые можно найти почти в любом дистрибутиве Linux.](https://searchdatacenter.techtarget.com/tutorial/77-Linux-commands-and-utilities-youll-actually-use)
* [Про `&` в `shell`.](https://stackoverflow.com/questions/13338870/what-does-at-the-end-of-a-linux-command-mean)

# Задание 2

### Необходимые знания

* Как работает команда `grep`
* Как работает редирект в `bash`
* Что такое `pipe` в `bash`
* Специальные девайсы в *nix системах (`/dev/null`, `/dev/full` и т.д.)

### Задание

* С помощью команды `grep`, используя `pipe` и редирект (`>`), запишите в файл `with_cake.txt` все строчки из файла `cake_rhymes.txt`, в которых есть слово `cake`.

* Сделайте команду `rm` "тихой", используя редирект в `/dev/null`.

### Ресурсы

* [Небольшой справочник по основным утилитам командной строки, которые можно найти почти в любом дистрибутиве Linux.](https://searchdatacenter.techtarget.com/tutorial/77-Linux-commands-and-utilities-youll-actually-use)

* [Про `pipe`](https://www.geeksforgeeks.org/piping-in-unix-or-linux/)

* [Про то, как работают редиректы.](http://wiki.bash-hackers.org/howto/redirection_tutorial)

* [Про `/dev/null`, `/dev/full` и другие "special device files" в *nix системах](https://en.wikipedia.org/wiki/Device_file#Pseudo-devices)

# Задание 3

### Необходимые знания

* Права в linux
* Команда `chmod`
* Переменные окружения
* Команда `date` в bash
* Что такое `shebang`

### Задание

* Сделайте файл `hello.sh` исполняемым, выполните его.

* Напишите `bash` скрипт, который выводит текущий путь, текущую дату и время, а также содержимое переменной окружения `PATH`.

### Ресурсы

* [Вводная статья про то, как вообще работать с правами в Linux](https://www.linux.com/learn/understanding-linux-file-permissions)

* [Как напечатать дату и время.](https://unix.stackexchange.com/questions/85982/date-time-in-linux-bash)

* [Описание того, как работают переменные окружения.](https://www.digitalocean.com/community/tutorials/how-to-read-and-set-environmental-and-shell-variables-on-a-linux-vps)

# Задание 4\* (повышенной сложности)

### Необходимые знания

* Как работать с аргументами в `bash` скриптах
* Как работает команда `od`
* Специальные устройства в *nix системах
* Как работает редирект

### Задание

* Напишите скрипт `average.sh`, который выводит количество и среднее арифмитическое его входных аргументов.

* С помощью `bash` и `dev/random` создайте файл `numbers.txt` из 150 рандомных чисел.

* "Скормите" скрипту `average.sh` значения из файла `numbers.txt`.

### Ресурсы

* [Google](https://www.google.com)


@Lans240 ➜ /workspaces/OC_Lab (master) $ cd lab1
@Lans240 ➜ /workspaces/OC_Lab/lab1 (master) $ cd src
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ ./background.sh &
[1] 34132
1 sec
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ 2 sec
3 sec
4 sec
5 sec
6 sec
7 sec
8 sec
9 sec
10 sec
Done!

@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ echo "Пример текста для тестирования" > test.txt
[1]+  Done                    ./background.sh
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ echo "Дополнительная строка" >> test.txt
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ cat test.txt
Пример текста для тестирования
Дополнительная строка

@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ wc -c test.txt
100 test.txt

Задание 2:
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ cat cake_rhymes.txt | grep "cake" > with_cake.txt

cat читает файл cake_rhymes.txt и передаёт его содержимое через pipe на вход grep.

grep фильтрует строки, содержащие "cake", и результат перенаправляется (>) в файл with_cake.txt.


@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ rm filename 2> /dev/null

2> /dev/null перенаправляет поток ошибок (stderr) в /dev/null, подавляя сообщения об ошибках (например, если файл не существует).


Задание 3:
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ nano hello.sh
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ chmod +x hello.sh
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ ./hello.sh
Hello, world!


@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ nano script1
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ chmod +x script1
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ ./script1
=== Системная информация ===
Текущий путь: /workspaces/OC_Lab/lab1/src
Текущая дата и время: Thu Sep 25 13:06:49 UTC 2025
Содержимое PATH: /usr/local/rvm/gems/ruby-3.4.1/bin:/usr/local/rvm/gems/ruby-3.4.1@global/bin:/usr/local/rvm/rubies/ruby-3.4.1/bin:/vscode/bin/linux-x64/0f0d87fa9e96c856c5212fc86db137ac0d783365/bin/remote-cli:/home/codespace/.local/bin:/home/codespace/.dotnet:/home/codespace/nvm/current/bin:/home/codespace/.php/current/bin:/home/codespace/.python/current/bin:/home/codespace/java/current/bin:/home/codespace/.ruby/current/bin:/home/codespace/.local/bin:/usr/local/python/current/bin:/usr/local/py-utils/bin:/usr/local/jupyter:/usr/local/oryx:/usr/local/go/bin:/go/bin:/usr/local/sdkman/bin:/usr/local/sdkman/candidates/java/current/bin:/usr/local/sdkman/candidates/gradle/current/bin:/usr/local/sdkman/candidates/maven/current/bin:/usr/local/sdkman/candidates/ant/current/bin:/usr/local/rvm/gems/default/bin:/usr/local/rvm/gems/default@global/bin:/usr/local/rvm/rubies/default/bin:/usr/local/share/rbenv/bin:/usr/local/php/current/bin:/opt/conda/bin:/usr/local/nvs:/usr/local/share/nvm/versions/node/v22.17.0/bin:/usr/local/hugo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/share/dotnet:/home/codespace/.dotnet/tools:/usr/local/rvm/bin
============================


или bash script1


Задание 4:

@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ nano average.sh
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ chmod +x numbers
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ ./numbers
@Lans240 ➜ /workspaces/OC_Lab/lab1/src (master) $ ./average.sh $(cat numbers.txt)
Количество чисел: 150
Сумма чисел: 79459
Среднее арифметическое: 529.72