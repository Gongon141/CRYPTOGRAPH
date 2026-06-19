// Чтение и запись данных в бинарном режиме: либо из/в файл,
// либо из/в стандартные потоки ввода-вывода (stdin/stdout).
// Этот модуль ничего не знает о шифрах — только байты.

#ifndef CRYPTUM_FILE_IO_H
#define CRYPTUM_FILE_IO_H

#include <cstdint>
#include <string>
#include <vector>

// Читает все байты из файла по указанному пути в бинарном режиме.
// Бросает std::runtime_error, если файл не удалось открыть.
std::vector<uint8_t> read_file_binary(const std::string& path);

// Читает все байты из стандартного потока ввода (stdin) до конца потока.
std::vector<uint8_t> read_stdin_binary();

// Записывает байты в файл по указанному пути в бинарном режиме,
// перезаписывая файл, если он уже существует.
void write_file_binary(const std::string& path, const std::vector<uint8_t>& data);

// Записывает байты в стандартный поток вывода (stdout) в бинарном режиме.
void write_stdout_binary(const std::vector<uint8_t>& data);

#endif // CRYPTUM_FILE_IO_H
