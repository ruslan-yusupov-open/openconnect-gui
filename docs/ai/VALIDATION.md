# Свидетельства практической проверки

Дата: 6 сентября 2026 года. Ниже — сообщения отдельного запуска Codex CLI,
извлечённые из JSONL-событий. Единственная редактура сообщений — замена
локальной абсолютной ссылки на относительную ссылку репозитория.
Сырые события с путями машины и служебным контекстом в PR не включены.

## Сообщения агента

### Сообщение 1

> Прочитаю правила и текущую реализацию R6, затем внесу исправление только в `src/vpninfo.cpp` и выполню доступные проверки.

### Сообщение 2

> Контекст проекта: Qt 5 / C++17
>
> Причина R6 подтверждена: при `len == 0` выражение `buf[len - 1]` читает за границей буфера. В `src/vpninfo.cpp` добавлю проверку длины перед индексированием. Форматирование, усечение, удаление одного завершающего `\n` и фильтр TRACE сохранятся. Проверю diff; поведенческие тесты остаются основному исполнителю согласно вашему запросу.

### Сообщение 3

> Исправлен только [src/vpninfo.cpp](../../src/vpninfo.cpp) (строка 68): добавлено условие `len > 0` перед обращением к `buf[len - 1]`. Причина дефекта — чтение за границей буфера при пустом сообщении.
>
> Обычные сообщения, удаление одного завершающего `\n`, усечение и фильтр TRACE сохранены.
>
> Проверки:
>
> - **PASS** — `git diff --check`.
> - **PASS** — `git diff -- src/vpninfo.cpp`, `git diff --stat`, `git status --short`: изменена одна строка; секретов в diff нет, чужие файлы сохранены.
> - **NOT RUN** — сборка и поведенческие тесты; тестирование оставлено основному исполнителю.
>
> Документы не изменены. Коммит, push и подключение к VPN не выполнялись.

## Запуск тестов основным исполнителем

Windows: CMake 4.2.1, Python 3.14.2, MSVC 19.41.34120, конфигурация RelWithDebInfo.
Команды из корня репозитория:

```text
cmake -S tests/progress -B build-release-progress -DPROGRESS_ASAN=ON
cmake --build build-release-progress --config RelWithDebInfo
ctest --test-dir build-release-progress -C RelWithDebInfo --output-on-failure
```

Результат исправленного callback: 100% tests passed, 0 tests failed out of 8.
В PATH был добавлен каталог runtime установленного MSVC, как в Developer Command Prompt.
Первоначальный запуск MSBuild из ограниченной среды не имел доступа к Windows SDK;
после разрешённого повторного запуска сборка прошла.

Отрицательный контроль MSVC с исходным файлом `git show 1b9bc0a:src/vpninfo.cpp`
также дал 8/8 PASS, в том числе после добавления `/Od`. Поэтому для доказательства
регрессии выполнен второй независимый запуск GCC 14.2.0 через локальную WSL.

Фактические команды WSL запускались с абсолютными путями рабочего каталога;
ниже пути заменены обозначениями ROOT (корень репозитория) и VALIDATION
(локальный каталог результатов вне Git). Это нормализованная запись команд,
не готовая команда для копирования. Готовые команды находятся в README теста.

```text
wsl -d Ubuntu-24.04 --exec g++ --version
wsl -d Ubuntu-24.04 --exec g++ -std=c++17 -fsanitize=address -fno-omit-frame-pointer -g -O0 -I ROOT/build-release-progress-baseline ROOT/tests/progress/progress_callback_test.cpp -o VALIDATION/progress-baseline
wsl -d Ubuntu-24.04 --exec g++ -std=c++17 -fsanitize=address -fno-omit-frame-pointer -g -O0 -I ROOT/build-release-progress ROOT/tests/progress/progress_callback_test.cpp -o VALIDATION/progress-fixed
wsl -d Ubuntu-24.04 --exec VALIDATION/progress-fixed SCENARIO
wsl -d Ubuntu-24.04 --exec VALIDATION/progress-baseline SCENARIO
```

SCENARIO последовательно принимал восемь значений:

| Сценарий | GCC, исходный код | GCC, исправленный код |
| --- | --- | --- |
| empty | exit 1, ASan | exit 0, PASS |
| newline | exit 0, PASS | exit 0, PASS |
| plain | exit 0, PASS | exit 0, PASS |
| trim_one | exit 0, PASS | exit 0, PASS |
| long | exit 0, PASS | exit 0, PASS |
| trace | exit 0, PASS | exit 0, PASS |
| formatted | exit 0, PASS | exit 0, PASS |
| formatted_empty | exit 1, ASan | exit 0, PASS |

Сокращённая диагностика исходной версии (числовые адреса и пути опущены):

```text
ERROR: AddressSanitizer: stack-buffer-overflow
READ of size 1
in progress_vfn ... progress_callback.inc:18
[96, 608) 'buf' ... Memory access at offset 95 underflows this variable
```

Полная настройка приложения:

```text
cmake -S . -B build-release-app-check -G "MinGW Makefiles"
CMake Error: CMAKE_MAKE_PROGRAM is not set.
CMake Error: CMAKE_CXX_COMPILER not set, after EnableLanguage
```

Это FAIL конфигурации среды; полная сборка, реальные Qt log sinks и VPN — NOT RUN.
Две ожидаемые ошибки baseline подтверждают, что регрессия обнаруживает исходный
дефект, а не только сравнивает строки на исправленной версии.
