#include "file_io.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

std::vector<uint8_t> read_file_binary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("не удалось открыть файл для чтения: " + path);
    }

    file.unsetf(std::ios::skipws);

    std::vector<uint8_t> data;
    data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return data;
}

std::vector<uint8_t> read_stdin_binary() {
    std::ios::sync_with_stdio(false);

    std::vector<uint8_t> data;
    data.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
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
