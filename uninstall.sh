#!/usr/bin/env bash
# Удаление программы cryptum (ТЗ, п. 5.1). Каталог должен совпадать с тем,
# что использовался при установке (переменная PREFIX, по умолчанию /usr/local).
set -euo pipefail

PREFIX="${PREFIX:-/usr/local}"
APP_DIR="$PREFIX/lib/cryptum"
BIN_DIR="$PREFIX/bin"

SUDO=""
if [ "$(id -u)" -ne 0 ] && [ ! -w "$PREFIX" ]; then
    SUDO="sudo"
fi

echo "Удаление cryptum ..."
$SUDO rm -f "$BIN_DIR/cryptum"
$SUDO rm -rf "$APP_DIR"
echo "Готово. Программа удалена."
