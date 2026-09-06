"""Извлечь существующий callback без копирования его реализации в тест."""

import pathlib
import re
import sys


def main():
    source = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
    # Границы двух функций намеренно строгие: при рефакторинге обновить стенд,
    # а не незаметно проверить устаревшую копию callback.
    match = re.search(
        r"^static void progress_vfn\([^\n]*\)\n\{\n.*?^\}\n"
        r"(?=\nstatic int process_auth_form\()",
        source,
        re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise SystemExit("Не найдены ожидаемые границы progress_vfn")
    pathlib.Path(sys.argv[2]).write_text(match.group(0), encoding="utf-8")


if __name__ == "__main__":
    main()
