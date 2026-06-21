#include "file_io.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

std::vector<uint8_t> read_file_binary(const std::string& path) {
    // Открываем файл в бинарном режиме; ios::ate — сразу в конец, чтобы узнать размер.
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("не удалось открыть файл для чтения: " + path);
    }

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0) {
        file.read(reinterpret_cast<char*>(data.data()), size);
        if (!file) {
            throw std::runtime_error("ошибка чтения файла: " + path);
        }
    }
    return data;
}

std::vector<uint8_t> read_stdin_binary() {
    std::vector<uint8_t> data;

    // Читаем стандартный ввод
    char buffer[4096];
    while (std::cin.read(buffer, sizeof(buffer)) || std::cin.gcount() > 0) {
        const std::streamsize read_count = std::cin.gcount();
        for (std::streamsize i = 0; i < read_count; ++i) {
            data.push_back(static_cast<uint8_t>(buffer[i]));
        }
    }
    return data;
}

void write_file_binary(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("не удалось открыть файл для записи: " + path);
    }

    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!file.good()) {
        throw std::runtime_error("ошибка записи в файл: " + path);
    }
}

void write_stdout_binary(const std::vector<uint8_t>& data) {
    std::cout.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    std::cout.flush();
}
