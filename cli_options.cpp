#include "cli_options.h"

#include <iostream>
#include <stdexcept>

namespace {

// Один флаг может иметь короткий и длинный вид; description идёт в справку.
struct FlagNames {
    std::string short_name;
    std::string long_name;
    std::string description;
};

// Список всех флагов программы.
std::vector<FlagNames> known_flags() {
    return {
        {"-h", "--help", "вывести справку по программе и завершить работу"},
        {"-a", "--algorithm", "алгоритм: xor, atbash, caesar, vigenere, playfair, nihilist"},
        {"-m", "--mode", "режим работы: encrypt, decrypt или generate-key"},
        {"-k", "--key", "файл с ключом (или '-' для чтения из stdin)"},
        {"-g", "--generate-key", "сгенерировать ключ на лету при шифровании"},
        {"-s", "--save-key", "файл для сохранения ключа (или '-' для stdout)"},
        {"-i", "--input", "входной файл (или '-' для чтения из stdin)"},
        {"-o", "--output", "выходной файл (или '-' для записи в stdout)"},
    };
}

// Совпадает ли аргумент с коротким или длинным именем флага.
bool matches_flag(const std::string& argument, const FlagNames& flag) {
    return argument == flag.short_name || argument == flag.long_name;
}

// "-" в значении пути означает "использовать стандартный поток".
bool is_stdin_marker(const std::string& value) {
    return value == "-";
}

} // namespace

std::vector<AlgorithmDescription> all_algorithms() {
    return {
        {Algorithm::Xor, "xor"},
        {Algorithm::Atbash, "atbash"},
        {Algorithm::Caesar, "caesar"},
        {Algorithm::Vigenere, "vigenere"},
        {Algorithm::Playfair, "playfair"},
        {Algorithm::Nihilist, "nihilist"},
    };
}

std::vector<ModeDescription> all_modes() {
    return {
        {Mode::Encrypt, "encrypt"},
        {Mode::Decrypt, "decrypt"},
        {Mode::GenerateKey, "generate-key"},
    };
}

std::string library_file_name(Algorithm algorithm) {
    std::string base_name;
    for (const AlgorithmDescription& description : all_algorithms()) {
        if (description.value == algorithm) {
            base_name = description.name;
        }
    }
    if (base_name.empty()) {
        throw std::runtime_error("неизвестный алгоритм шифрования");
    }

    // Имя файла библиотеки зависит от операционной системы (п. 4.4.7.7).
#ifdef _WIN32
    return base_name + ".dll";
#else
    return "lib" + base_name + ".so";
#endif
}

CliOptions parse_cli_options(std::vector<std::string> arguments) {
    CliOptions options;

    for (size_t index = 0; index < arguments.size(); ++index) {
        const std::string argument = arguments[index];

        // Справка.
        if (matches_flag(argument, known_flags()[0])) {
            options.show_help = true;
            continue;
        }

        // Флаг без значения: генерация ключа на лету.
        if (matches_flag(argument, known_flags()[4])) {
            options.generate_key = true;
            continue;
        }

        // Все остальные флаги принимают следующее слово как значение.
        // Сначала убеждаемся, что значение вообще передано.
        bool is_known_value_flag = false;
        for (size_t flag_index = 1; flag_index < known_flags().size(); ++flag_index) {
            if (flag_index == 4) {
                continue;  // -g уже обработан выше, у него нет значения
            }
            if (matches_flag(argument, known_flags()[flag_index])) {
                is_known_value_flag = true;
            }
        }
        if (is_known_value_flag && index + 1 >= arguments.size()) {
            throw std::runtime_error("после флага " + argument + " ожидается значение");
        }

        // Алгоритм.
        if (matches_flag(argument, known_flags()[1])) {
            const std::string value = arguments[++index];
            bool found = false;
            for (const AlgorithmDescription& description : all_algorithms()) {
                if (description.name == value) {
                    options.algorithm = description.value;
                    options.has_algorithm = true;
                    found = true;
                }
            }
            if (!found) {
                throw std::runtime_error("неизвестный алгоритм: " + value);
            }
            continue;
        }

        // Режим работы.
        if (matches_flag(argument, known_flags()[2])) {
            const std::string value = arguments[++index];
            bool found = false;
            for (const ModeDescription& description : all_modes()) {
                if (description.name == value) {
                    options.mode = description.value;
                    options.has_mode = true;
                    found = true;
                }
            }
            if (!found) {
                throw std::runtime_error("неизвестный режим работы: " + value);
            }
            continue;
        }

        // Ключ.
        if (matches_flag(argument, known_flags()[3])) {
            const std::string value = arguments[++index];
            if (is_stdin_marker(value)) {
                options.key_from_stdin = true;
            } else {
                options.key_file = value;
                options.key_from_file = true;
            }
            continue;
        }

        // Сохранение ключа.
        if (matches_flag(argument, known_flags()[5])) {
            const std::string value = arguments[++index];
            if (is_stdin_marker(value)) {
                options.save_key_to_stdout = true;
            } else {
                options.save_key_file = value;
                options.save_key_to_file = true;
            }
            continue;
        }

        // Входные данные.
        if (matches_flag(argument, known_flags()[6])) {
            const std::string value = arguments[++index];
            if (is_stdin_marker(value)) {
                options.input_from_stdin = true;
            } else {
                options.input_file = value;
                options.input_from_file = true;
            }
            continue;
        }

        // Выходные данные.
        if (matches_flag(argument, known_flags()[7])) {
            const std::string value = arguments[++index];
            if (is_stdin_marker(value)) {
                options.output_to_stdout = true;
            } else {
                options.output_file = value;
                options.output_to_file = true;
            }
            continue;
        }

        throw std::runtime_error("неизвестный аргумент: " + argument);
    }

    return options;
}

void print_help() {
    std::cout << "Использование: cryptum [флаги]\n\n";

    std::cout << "Доступные флаги:\n";
    for (const FlagNames& flag : known_flags()) {
        std::cout << "  " << flag.short_name << ", " << flag.long_name << "\n";
        std::cout << "      " << flag.description << "\n";
    }

    std::cout << "\nДоступные алгоритмы:\n";
    for (const AlgorithmDescription& description : all_algorithms()) {
        std::cout << "  " << description.name << "\n";
    }

    std::cout << "\nДоступные режимы:\n";
    for (const ModeDescription& description : all_modes()) {
        std::cout << "  " << description.name << "\n";
    }

    std::cout << "\nПримеры:\n";
    std::cout << "  cryptum -a caesar -m generate-key -s key.bin\n";
    std::cout << "  cryptum -a caesar -m encrypt -k key.bin -i photo.jpg -o photo.enc\n";
    std::cout << "  cryptum -a vigenere -m encrypt -g -s key.bin -i data.txt -o data.enc\n";
    std::cout << "  cryptum -a playfair -m decrypt -k key.bin -i data.enc -o data.txt\n";
}
