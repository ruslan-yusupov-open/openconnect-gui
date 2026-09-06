# Регрессия пустого сообщения OpenConnect

Стенд компилирует **саму функцию `progress_vfn`, извлечённую из `src/vpninfo.cpp`**, с минимальными заменами VpnInfo, PRG_TRACE и Logger. Логика callback не переписана в тесте. Logger проверяется только на границе передачи строки: реальные Qt, FileLogger и VPN здесь не запускаются.

Нужны CMake 3.16+, Python 3 и C++17-компилятор с AddressSanitizer. Для Windows проверяется MSVC из Developer Command Prompt с установленным C++ AddressSanitizer; этот компилятор используется только для изолированного теста, не как новый способ сборки приложения.

Из корня репозитория:

```text
cmake -S tests/progress -B build-release-progress -DPROGRESS_ASAN=ON
cmake --build build-release-progress --config RelWithDebInfo
ctest --test-dir build-release-progress -C RelWithDebInfo --output-on-failure
```

Сценарии: пустая строка, один перевод строки, обычный текст, удаление ровно одного завершающего перевода строки, усечение до 511 байт, пропуск TRACE, форматирование аргументов и пустой результат форматирования. Проверки возвращают ненулевой код при ошибке и не зависят от NDEBUG.

Для проверки исходного дефекта сохрани `src/vpninfo.cpp` из базового коммита в отдельный временный файл и сконфигурируй другой каталог с `-DPROGRESS_SOURCE=<абсолютный путь к этому файлу>`. Критерий успешного отрицательного контроля — сообщение stack-buffer-overflow на пустых сообщениях в исходной версии и восемь успешных сценариев в исправленной. Не меняй рабочий исходник для отрицательного контроля.

В этой работе MSVC 19.41 с ASan не обнаружил исходный дефект ни с исходной оптимизацией RelWithDebInfo, ни с `/Od`. Причина не установлена. Поэтому его восемь успешных тестов не считаются доказательством устранения доступа за границы. Отрицательный контроль подтверждён GCC 14.2 / ASan в WSL. Ограничения ASan и расположение runtime для Windows описаны в [документации Microsoft](https://learn.microsoft.com/en-us/cpp/sanitizers/asan-known-issues?view=msvc-170).

## Запуск без CMake в Linux / WSL

Из корня репозитория, при установленных Python 3 и GCC с ASan:

```sh
mkdir -p build-release-progress-gcc/fixed build-release-progress-gcc/baseline
git show 1b9bc0a:src/vpninfo.cpp > build-release-progress-gcc/baseline/vpninfo.cpp
python3 tests/progress/extract_callback.py src/vpninfo.cpp build-release-progress-gcc/fixed/progress_callback.inc
python3 tests/progress/extract_callback.py build-release-progress-gcc/baseline/vpninfo.cpp build-release-progress-gcc/baseline/progress_callback.inc
g++ -std=c++17 -fsanitize=address -fno-omit-frame-pointer -g -O0 -I build-release-progress-gcc/fixed tests/progress/progress_callback_test.cpp -o build-release-progress-gcc/fixed/test
g++ -std=c++17 -fsanitize=address -fno-omit-frame-pointer -g -O0 -I build-release-progress-gcc/baseline tests/progress/progress_callback_test.cpp -o build-release-progress-gcc/baseline/test
for scenario in empty newline plain trim_one long trace formatted formatted_empty; do
    build-release-progress-gcc/fixed/test "$scenario" || exit 1
done
build-release-progress-gcc/baseline/test empty
build-release-progress-gcc/baseline/test formatted_empty
```

Последние две команды должны завершиться с кодом 1 и диагностикой ASan. Это ожидаемые ошибки отрицательного контроля, а не успешные обычные тесты. Остальные шесть сценариев на baseline должны возвращать 0. Каталоги `build-release*` исключены существующим `.gitignore`.

`-DPROGRESS_ASAN=OFF` позволяет проверить строки без санитайзера, но **не доказывает отсутствие обращения за границы**. Этот вариант не заменяет регрессионную проверку R6. Неуспешная загрузка ASan runtime также не считается обнаружением исходного бага.
