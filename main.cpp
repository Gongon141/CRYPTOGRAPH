// Главный модуль программы Multi-Algo Cryptotool.
//
// Этот файл отвечает только за связывание модулей друг с другом:
// разобрать аргументы, прочитать ключ и данные, загрузить нужную
// библиотеку, выполнить операцию и записать результат. Сама логика
// шифрования находится в динамических библиотеках (xor, atbash).

#include "cipher_api.h"
#include "cipher_library.h"
#include "cli_options.h"
#include "file_io.h"
#include "key_generator.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Длина ключа в байтах, которая используется по умолчанию при его
// генерации для алгоритмов, не требующих фиксированной длины ключа.
constexpr size_t kDefaultGeneratedKeySize = 16;

// Путь к каталогу с библиотеками шифров. Программа ищет библиотеки
// рядом с собой, в подкаталоге lib.
const std::string kLibraryDirectory = "./lib/";

// Загружает динамическую библиотеку, соответствующую выбранному
// алгоритму шифрования.
CipherLibrary load_cipher_library(Algorithm algorithm) {
    const std::string library_path = kLibraryDirectory + library_file_name(algorithm);
    return CipherLibrary(library_path);
}

// Читает ключ либо из файла, либо из стандартного ввода, в зависимости
// от того, что указано в аргументах командной строки.
std::vector<uint8_t> read_key(const CliOptions& options) {
    if (options.key_from_stdin) {
        return read_stdin_binary();
    }
    if (options.key_file.has_value()) {
        return read_file_binary(options.key_file.value());
    }
    throw std::runtime_error("для выбранного режима требуется ключ: укажите -k <файл> или -k -");
}

// Читает входные данные либо из файла, либо из стандартного ввода.
std::vector<uint8_t> read_input(const CliOptions& options) {
    if (options.input_from_stdin) {
        return read_stdin_binary();
    }
    if (options.input_file.has_value()) {
        return read_file_binary(options.input_file.value());
    }
    throw std::runtime_error("не указаны входные данные: укажите -i <файл> или -i -");
}

// Записывает результирующие данные либо в файл, либо в стандартный вывод.
void write_output(const CliOptions& options, const std::vector<uint8_t>& data) {
    if (options.output_to_stdout) {
        write_stdout_binary(data);
        return;
    }
    if (options.output_file.has_value()) {
        write_file_binary(options.output_file.value(), data);
        return;
    }
    throw std::runtime_error("не указано место для результата: укажите -o <файл> или -o -");
}

// Сохраняет сгенерированный ключ туда, куда указали в аргументах.
// Если ни -s, ни stdout не указаны — выводит ключ на stdout по умолчанию,
// чтобы сгенерированный ключ не терялся.
void save_generated_key(const CliOptions& options, const std::vector<uint8_t>& key) {
    if (options.save_key_to_stdout) {
        write_stdout_binary(key);
        return;
    }
    if (options.save_key_file.has_value()) {
        write_file_binary(options.save_key_file.value(), key);
        return;
    }
    write_stdout_binary(key);
}

// Выполняет операцию шифрования или расшифрования над входными данными
// с помощью загруженной библиотеки и возвращает результат.
std::vector<uint8_t> run_cipher_operation(const CipherLibrary& library,
                                           Mode mode,
                                           const std::vector<uint8_t>& key,
                                           const std::vector<uint8_t>& input) {
    const ConstBuffer key_buffer{key.data(), key.size()};
    const ConstBuffer input_buffer{input.data(), input.size()};

    const CipherOperation operation =
        (mode == Mode::Encrypt) ? CipherOperation::Encrypt : CipherOperation::Decrypt;

    const size_t expected_output_size = library.output_size(input.size(), operation);

    std::vector<uint8_t> output(expected_output_size);
    MutBuffer output_buffer{output.data(), output.size()};

    const CipherStatus status = (mode == Mode::Encrypt)
        ? library.encrypt(key_buffer, input_buffer, &output_buffer)
        : library.decrypt(key_buffer, input_buffer, &output_buffer);

    if (status != CipherStatus::Ok) {
        throw std::runtime_error("библиотека шифрования вернула код ошибки: " +
                                  std::to_string(static_cast<int>(status)));
    }

    output.resize(output_buffer.size);
    return output;
}

// Обрабатывает режим генерации ключа: создаёт случайный ключ нужной
// длины и сохраняет его в указанное место.
void handle_generate_key_mode(const CliOptions& options, const CipherLibrary& library) {
    const AlgorithmInfo* info = library.algorithm_info();

    // key_size == 0 означает, что библиотека не требует фиксированной
    // длины ключа — в этом случае используем разумное значение по умолчанию.
    const size_t key_size = (info->key_size != 0) ? info->key_size : kDefaultGeneratedKeySize;

    const std::vector<uint8_t> key = generate_random_key(key_size);
    save_generated_key(options, key);

    std::cerr << "сгенерирован ключ длиной " << key_size << " байт для алгоритма "
              << info->algorithm_name << "\n";
}

// Обрабатывает режимы шифрования и расшифрования: читает ключ и
// входные данные, выполняет операцию, записывает результат.
void handle_cipher_mode(const CliOptions& options, const CipherLibrary& library, Mode mode) {
    const std::vector<uint8_t> key = read_key(options);
    const std::vector<uint8_t> input = read_input(options);

    const std::vector<uint8_t> output = run_cipher_operation(library, mode, key, input);

    write_output(options, output);

    // Если вместе с шифрованием попросили сохранить ключ — сохраняем
    // тот же самый ключ, который был прочитан (например, после
    // генерации "на лету" через --generate-key, если такая возможность
    // потребуется в будущем).
    if (options.save_key_file.has_value() || options.save_key_to_stdout) {
        save_generated_key(options, key);
    }
}

// Проверяет, что для выбранного режима присутствуют все обязательные
// аргументы, и сообщает понятную ошибку, если что-то отсутствует.
void validate_options(const CliOptions& options) {
    if (!options.algorithm.has_value()) {
        throw std::runtime_error("не указан алгоритм шифрования: используйте -a xor или -a atbash");
    }
    if (!options.mode.has_value()) {
        throw std::runtime_error("не указан режим работы: используйте -m encrypt, decrypt или generate-key");
    }

    const Mode mode = options.mode.value();
    if (mode == Mode::GenerateKey) {
        return;
    }

    const bool has_key_source = options.key_file.has_value() || options.key_from_stdin;
    const bool has_input_source = options.input_file.has_value() || options.input_from_stdin;
    const bool has_output_target = options.output_file.has_value() || options.output_to_stdout;

    if (!has_key_source) {
        throw std::runtime_error("не указан источник ключа: используйте -k <файл> или -k -");
    }
    if (!has_input_source) {
        throw std::runtime_error("не указан источник входных данных: используйте -i <файл> или -i -");
    }
    if (!has_output_target) {
        throw std::runtime_error("не указано место для результата: используйте -o <файл> или -o -");
    }

    // Если ключ и входные данные оба читаются из stdin — это конфликт,
    // так как стандартный поток ввода можно прочитать только один раз.
    if (options.key_from_stdin && options.input_from_stdin) {
        throw std::runtime_error("ключ и входные данные не могут оба читаться из stdin одновременно");
    }
}

} // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> arguments(argv + 1, argv + argc);

    try {
        const CliOptions options = parse_cli_options(arguments);

        if (options.show_help || arguments.empty()) {
            print_help();
            return 0;
        }

        validate_options(options);

        const CipherLibrary library = load_cipher_library(options.algorithm.value());
        const Mode mode = options.mode.value();

        if (mode == Mode::GenerateKey) {
            handle_generate_key_mode(options, library);
        } else {
            handle_cipher_mode(options, library, mode);
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ошибка: " << error.what() << "\n";
        return 1;
    }
}
