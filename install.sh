#!/usr/bin/env bash
# По умолчанию ставит в /usr/local — этот каталог уже есть в PATH у всех
# пользователей, поэтому команда cryptum работает сразу, без правки PATH.
# Запись в /usr/local требует прав администратора, поэтому при необходимости скрипт использует sudo. 
set -euo pipefail

PREFIX="${PREFIX:-/usr/local}"
APP_DIR="$PREFIX/lib/cryptum"   # сюда кладём cryptum и подкаталог lib
BIN_DIR="$PREFIX/bin"           # сюда — ссылку для запуска

# Если программа ещё не собрана — собираем.
if [ ! -x build/cryptum ]; then
    echo "Сборка проекта..."
    make
fi

# Права администратора нужны, только если в каталог нельзя писать напрямую.
SUDO=""
if [ "$(id -u)" -ne 0 ] && [ ! -w "$PREFIX" ]; then
    echo "Для установки в $PREFIX нужны права администратора — будет запрошен пароль sudo."
    SUDO="sudo"
fi

echo "Установка в $APP_DIR ..."
$SUDO mkdir -p "$APP_DIR/lib" "$BIN_DIR"
$SUDO cp build/cryptum "$APP_DIR/cryptum"
$SUDO cp build/lib/*.so "$APP_DIR/lib/"
$SUDO ln -sf "$APP_DIR/cryptum" "$BIN_DIR/cryptum"

echo "Готово. Теперь можно запускать командой: cryptum"
