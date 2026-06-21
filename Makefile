# Сборка проекта через GNU Make (все файлы лежат в одной папке).
# Цели:
#   make            — собрать всё (библиотеки, главный модуль, тесты);
#   make test       — собрать и запустить тесты;
#   make clean      — удалить скомпилированные файлы.

CXX      := g++
# Строгие флаги качества кода: стандарт C++17, максимум предупреждений,
# предупреждения как ошибки (требование ТЗ). -I. — заголовки в текущей папке.
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -I. -fPIC -O2
LDFLAGS  := -ldl

BUILD_DIR := build
LIB_DIR   := $(BUILD_DIR)/lib

# Исходники главного модуля (без main.cpp — он добавляется отдельно).
MAIN_COMMON := cli_options.cpp cipher_library.cpp file_io.cpp key_generator.cpp

.PHONY: all test clean

all: $(LIB_DIR)/libxor.so $(LIB_DIR)/libatbash.so $(LIB_DIR)/libcaesar.so \
     $(LIB_DIR)/libvigenere.so $(LIB_DIR)/libplayfair.so $(LIB_DIR)/libnihilist.so \
     $(BUILD_DIR)/cryptum $(BUILD_DIR)/cryptum_tests

# --- Динамические библиотеки шифров ---
$(LIB_DIR)/libxor.so: xor_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

$(LIB_DIR)/libatbash.so: atbash_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

$(LIB_DIR)/libcaesar.so: caesar_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

$(LIB_DIR)/libvigenere.so: vigenere_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

$(LIB_DIR)/libplayfair.so: playfair_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

$(LIB_DIR)/libnihilist.so: nihilist_cipher.cpp | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -o $@

# --- Главный модуль ---
$(BUILD_DIR)/cryptum: main.cpp $(MAIN_COMMON) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# --- Тестовая утилита ---
$(BUILD_DIR)/cryptum_tests: test_ciphers.cpp cipher_library.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR): | $(BUILD_DIR)
	mkdir -p $(LIB_DIR)

test: all
	./$(BUILD_DIR)/cryptum_tests

clean:
	rm -rf $(BUILD_DIR)
