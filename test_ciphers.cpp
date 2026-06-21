// Отдельная утилита для тестирования библиотек шифров (п. 4.4.10 ТЗ).
//
// Для каждого алгоритма выполняется несколько тестов на разных входных
// данных, в том числе на случайных байтах и на данных, содержащих все 256
// значений байта. 

#include "cipher_api.h"
#include "cipher_library.h"

#include <climits>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <unistd.h>

namespace {

std::string executable_directory() {
    char buffer[PATH_MAX];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return ".";
    }
    buffer[length] = '\0';
    const std::string full_path(buffer);
    const size_t last_slash = full_path.find_last_of('/');
    return last_slash == std::string::npos ? std::string(".") : full_path.substr(0, last_slash);
}

std::vector<uint8_t> to_bytes(const std::string& text) {
    return std::vector<uint8_t>(text.begin(), text.end());
}

// Один прогон: шифруем данные, расшифровываем и проверяем совпадение с исходником.
bool roundtrip_ok(const CipherLibrary& library, const std::vector<uint8_t>& key, const std::vector<uint8_t>& plaintext) {
    const ConstBuffer key_buffer{key.data(), key.size()};

    std::vector<uint8_t> ciphertext(library.output_size(plaintext.size(), CipherOperation::Encrypt));

    const ConstBuffer plaintext_buffer{plaintext.data(), plaintext.size()};

    MutBuffer ciphertext_buffer{ciphertext.data(), ciphertext.size()};

    if (library.encrypt(key_buffer, plaintext_buffer, &ciphertext_buffer) != CipherStatus::Ok) {
        return false;
    }
    ciphertext.resize(ciphertext_buffer.size);

    std::vector<uint8_t> decrypted(library.output_size(ciphertext.size(), CipherOperation::Decrypt));

    const ConstBuffer ciphertext_const{ciphertext.data(), ciphertext.size()};

    MutBuffer decrypted_buffer{decrypted.data(), decrypted.size()};

    if (library.decrypt(key_buffer, ciphertext_const, &decrypted_buffer) != CipherStatus::Ok) {
        return false;
    }
    decrypted.resize(decrypted_buffer.size);

    return decrypted == plaintext;
}

// Случайный набор байт фиксированной длины 

std::vector<uint8_t> random_bytes(size_t length, uint32_t seed) {

    std::mt19937 generator(seed);

    std::uniform_int_distribution<int> distribution(0, 255);

    std::vector<uint8_t> data(length);

    for (uint8_t& byte : data) {
        byte = static_cast<uint8_t>(distribution(generator));
    }
    return data;
}

// Все 256 значений байта дважды подряд — проверяет работу со всем диапазоном байт и с парами одинаковых значений.
std::vector<uint8_t> all_byte_values_twice() {

    std::vector<uint8_t> data;

    data.reserve(512);

    for (int repeat = 0; repeat < 2; ++repeat) {

        for (int value = 0; value < 256; ++value) {

            data.push_back(static_cast<uint8_t>(value));
        }
    }
    return data;
}

struct NamedData {
    std::string name;
    std::vector<uint8_t> data;
};

// Набор тестовых данных, общий для обоих алгоритмов.
std::vector<NamedData> test_cases() {

    return {
        {"короткий ASCII-текст", to_bytes("Attack at dawn!!")},
        {"повторяющиеся байты", to_bytes("AAAAAAAAAAAAAAA")},
        {"нечётная длина (7 байт)", to_bytes("oddsize")},
        {"случайные 1000 байт", random_bytes(1000, 12345)},
        {"все значения байта x2", all_byte_values_twice()},
    };
}

// Готовит ключ подходящей длины для конкретного алгоритма
std::vector<uint8_t> make_key(const CipherLibrary& library) {

    const size_t required = library.algorithm_info()->key_size;

    const size_t key_size = (required != 0) ? required : 16;

    std::vector<uint8_t> key(key_size);

    for (size_t i = 0; i < key_size; ++i) {

        key[i] = static_cast<uint8_t>(0x41 + (i % 26));  // повторяющиеся буквы A..Z
    }
    return key;
}

// Прогоняет все тестовые случаи для одной библиотеки и печатает результат.
bool run_suite(const std::string& algorithm_name, const std::string& library_file) {
    std::cout << "=== " << algorithm_name << " ===\n";

    CipherLibrary library(executable_directory() + "/lib/" + library_file);

    const std::vector<uint8_t> key = make_key(library);

    bool all_passed = true;

    for (const NamedData& test : test_cases()) {

        const bool ok = roundtrip_ok(library, key, test.data);

        std::cout << "  [" << (ok ? "OK  " : "FAIL") << "] " << test.name << "\n";

        if (!ok) {
            all_passed = false;
        }
    }
    return all_passed;
}

} 

int main() {
    bool all_passed = true;

    try {
        all_passed &= run_suite("Шифр гаммирования (XOR)", "libxor.so");
        all_passed &= run_suite("Шифр Атбаша", "libatbash.so");
        all_passed &= run_suite("Шифр Цезаря", "libcaesar.so");
        all_passed &= run_suite("Шифр Виженера", "libvigenere.so");
        all_passed &= run_suite("Шифр Плейфера", "libplayfair.so");
        all_passed &= run_suite("Шифр нигилистов", "libnihilist.so");
    } catch (const std::exception& error) {
        
        std::cerr << "Ошибка при тестировании: " << error.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "\nИтог: " << (all_passed ? "все тесты пройдены" : "есть непройденные тесты") << "\n";
    return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
