// Главный исполняемый модуль cryptum.
//
// Связывает вместе разбор аргументов, ввод-вывод, генерацию ключа и загрузку
// динамической библиотеки выбранного алгоритма. Сам модуль не содержит ни
// одного алгоритма шифрования — он только организует работу программы и
// обменивается с библиотекой байтами через интерфейс из cipher_api.h.
//
// Программа работает в одном из трёх взаимоисключающих режимов (п. 4.1.3 ТЗ):
//   1) справка                — нет аргументов или флаг --help;
//   2) генерация ключа         — --mode generate-key;
//   3) шифрование/расшифрование — --mode encrypt | decrypt.
//
// Примечание по переносимости. Программа ориентирована на Linux (п. 4.4.1 ТЗ).
// Для Windows понадобились бы: загрузка библиотек через LoadLibrary вместо
// dlopen и принудительная установка кодовой страницы консоли в UTF-8
// (SetConsoleOutputCP(CP_UTF8), п. 4.1.6.2). В Linux консоль работает в UTF-8
// по умолчанию, поэтому дополнительных действий не требуется.

#include "cipher_api.h"
#include "cipher_library.h"
#include "cli_options.h"
#include "file_io.h"
#include "key_generator.h"

#include <climits>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <unistd.h>

namespace {

// Длина случайного ключа по умолчанию
const size_t DEFAULT_GENERATED_KEY_SIZE = 16;

// Перезаписывает буфер нулями так, чтобы компилятор не выбросил эту запись при оптимизации 
void secure_zero(std::vector<uint8_t>& buffer) {
    volatile uint8_t* pointer = buffer.data();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pointer[i] = 0;
    }
}

// Каталог, в котором лежит сам исполняемый файл. Рядом с ним программа ищет подкаталог lib с библиотеками. 
std::string executable_directory() {
    char buffer[PATH_MAX];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return ".";
    }
    buffer[length] = '\0';

    const std::string full_path(buffer);
    const size_t last_slash = full_path.find_last_of('/');
    if (last_slash == std::string::npos) {
        return ".";
    }
    return full_path.substr(0, last_slash);
}

// Полный путь к библиотеке выбранного алгоритма.
std::string library_path(Algorithm algorithm) {
    return executable_directory() + "/lib/" + library_file_name(algorithm);
}

// Читает входные данные согласно опциям.
std::vector<uint8_t> read_input(const CliOptions& options) {
    if (options.input_from_stdin) {
        return read_stdin_binary();
    }
    if (options.input_from_file) {
        return read_file_binary(options.input_file);
    }
    throw std::runtime_error("не указан источник входных данных (-i ФАЙЛ или -i -)");
}

// Читает ключ согласно опциям. Если источник ключа не задан, возвращает пустой ключ — бесключевым шифрам (Атбаш) этого достаточно, а шифрам с ключом библиотека сама вернёт ошибку.
std::vector<uint8_t> read_key(const CliOptions& options) {
    if (options.key_from_stdin) {
        return read_stdin_binary();
    }
    if (options.key_from_file) {
        return read_file_binary(options.key_file);
    }
    return std::vector<uint8_t>();
}

// Записывает результат согласно опциям.
void write_output(const CliOptions& options, const std::vector<uint8_t>& data) {
    if (options.output_to_stdout) {
        write_stdout_binary(data);
    } else if (options.output_to_file) {
        write_file_binary(options.output_file, data);
    } else {
        throw std::runtime_error("не указано место вывода результата (-o ФАЙЛ или -o -)");
    }
}

// Сохраняет ключ согласно опциям. Если место сохранения не задано, ключ выводится в стандартный поток вывода.
void save_key(const CliOptions& options, const std::vector<uint8_t>& key) {
    if (options.save_key_to_file) {
        write_file_binary(options.save_key_file, key);
    } else {
        write_stdout_binary(key);
    }
}

// Длина ключа для генерации: её подсказывает сама библиотека алгоритма.
size_t key_size_for(const CipherLibrary& library) {
    const size_t required = library.algorithm_info()->key_size;
    return (required != 0) ? required : DEFAULT_GENERATED_KEY_SIZE;
}

// Генерация ключа
int run_generate_key(const CliOptions& options) {
    if (!options.has_algorithm) {
        throw std::runtime_error("для генерации ключа укажите алгоритм (-a ...)");
    }

    CipherLibrary library(library_path(options.algorithm));
    std::vector<uint8_t> key = generate_random_key(key_size_for(library));
    save_key(options, key);
    secure_zero(key);
    return EXIT_SUCCESS;
}

// Шифрование или расшифрование
int run_transformation(const CliOptions& options) {
    if (!options.has_algorithm) {
        throw std::runtime_error("укажите алгоритм (-a ...), список — в справке (--help)");
    }
    // И ключ, и данные нельзя читать из stdin одновременно
    if (options.key_from_stdin && options.input_from_stdin) {
        throw std::runtime_error("ключ и входные данные не могут одновременно читаться из stdin");
    }

    const bool encrypting = (options.mode == Mode::Encrypt);
    CipherLibrary library(library_path(options.algorithm));

    // Получаем ключ
    std::vector<uint8_t> key;
    if (options.generate_key) {
        if (!encrypting) {
            throw std::runtime_error("генерация ключа (-g) возможна только при шифровании");
        }
        if (options.key_from_file || options.key_from_stdin) {
            throw std::runtime_error("нельзя одновременно задавать ключ (-k) и генерировать его (-g)");
        }
        if (!options.save_key_to_file && !options.save_key_to_stdout) {
            throw std::runtime_error("при -g укажите, куда сохранить ключ (-s ФАЙЛ или -s -)");
        }
        key = generate_random_key(key_size_for(library));
    } else {
        key = read_key(options);
    }

    std::vector<uint8_t> input = read_input(options);

    const CipherOperation operation = encrypting ? CipherOperation::Encrypt : CipherOperation::Decrypt;

    // Память под результат всегда выделяет главный модуль
    std::vector<uint8_t> output(library.output_size(input.size(), operation));

    ConstBuffer key_buffer;
    key_buffer.data = key.data();
    key_buffer.size = key.size();

    ConstBuffer input_buffer;
    input_buffer.data = input.data();
    input_buffer.size = input.size();

    MutBuffer output_buffer;
    output_buffer.data = output.data();
    output_buffer.size = output.size();

    CipherStatus status;
    if (encrypting) {
        status = library.encrypt(key_buffer, input_buffer, &output_buffer);
    } else {
        status = library.decrypt(key_buffer, input_buffer, &output_buffer);
    }

    if (status != CipherStatus::Ok) {
        secure_zero(key);
        secure_zero(input);
        throw std::runtime_error("библиотека шифра вернула ошибку (код " + std::to_string(static_cast<int>(status)) + ")");
    }

    // Реальный размер результата сообщает библиотека через output_buffer.size.
    output.resize(output_buffer.size);
    write_output(options, output);

    // Если ключ был сгенерирован на лету — сохраняем его, чтобы потом расшифровать.
    if (options.generate_key) {
        save_key(options, key);
    }

    // Затираем всё, что содержит ключ или открытый текст.
    secure_zero(key);
    secure_zero(input);
    secure_zero(output);
    return EXIT_SUCCESS;
}

int run(const CliOptions& options) {
    // Справка. Имеет приоритет над остальными флагам, также показывается, если режим работы вообще не задан.
    if (options.show_help || !options.has_mode) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (options.mode == Mode::GenerateKey) {
        return run_generate_key(options);
    }
    return run_transformation(options);
}

} 

int main(int argc, char* argv[]) {
    // argv[0] (имя программы) пропускаем — разбираем только сами флаги.
    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    try {
        const CliOptions options = parse_cli_options(arguments);
        return run(options);
    } catch (const std::exception& error) {
        // Любая ошибка (неверные аргументы, нет файла, ошибка библиотеки) приводит к понятному сообщению и ненулевому коду возврата, а не к аварийному завершению программы 
        std::cerr << "Ошибка: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
}
