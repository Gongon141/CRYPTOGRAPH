#include "cli_options.h"

#include <iostream>
#include <stdexcept>

namespace {

// Один флаг командной строки может иметь короткий и длинный вид.
// Здесь же хранится описание для справки --help.
struct FlagNames {
    std::string short_name;
    std::string long_name;
    std::string description;
};

const std::vector<FlagNames>& known_flags() {
    static const std::vector<FlagNames> flags{
        {"-h", "--help", "вывести справку по программе и завершить работу"},
        {"-a", "--algorithm", "алгоритм шифрования: xor или atbash"},
        {"-m", "--mode", "режим работы: encrypt, decrypt или generate-key"},
        {"-k", "--key", "путь к файлу с ключом (или '-' для чтения из stdin)"},
        {"-s", "--save-key", "путь к файлу для сохранения сгенерированного ключа (или '-' для stdout)"},
        {"-i", "--input", "путь к входному файлу (или '-' для чтения из stdin)"},
        {"-o", "--output", "путь к выходному файлу (или '-' для записи в stdout)"},
    };
    return flags;
}

// Проверяет, совпадает ли аргумент с коротким или длинным именем флага.
bool matches_flag(const std::string& argument, const FlagNames& flag) {
    return argument == flag.short_name || argument == flag.long_name;
}

// Возвращает следующий аргумент-значение для флага или бросает
// исключение, если значение не было передано.
std::string take_value(const std::vector<std::string>& arguments, size_t& index, const std::string& flag_name) {
    if (index + 1 >= arguments.size()) {
        throw std::runtime_error("после флага " + flag_name + " ожидается значение");
    }
    ++index;
    return arguments[index];
}

// "-" в качестве пути к файлу означает "использовать стандартный поток".
bool is_stdin_marker(const std::string& value) {
    return value == "-";
}

} // namespace

const std::vector<AlgorithmDescription>& all_algorithms() {
    static const std::vector<AlgorithmDescription> algorithms{
        {Algorithm::Xor, "xor"},
        {Algorithm::Atbash, "atbash"},
    };
    return algorithms;
}

const std::vector<ModeDescription>& all_modes() {
    static const std::vector<ModeDescription> modes{
        {Mode::Encrypt, "encrypt"},
        {Mode::Decrypt, "decrypt"},
        {Mode::GenerateKey, "generate-key"},
    };
    return modes;
}

std::string library_file_name(Algorithm algorithm) {
    // Перебор вместо switch/case: ищем совпадение значения enum
    // в таблице описаний алгоритмов.
    for (const AlgorithmDescription& description : all_algorithms()) {
        if (description.value == algorithm) {
            return std::string("lib") + description.name + ".so";
        }
    }
    throw std::runtime_error("неизвестный алгоритм шифрования");
}

namespace {

std::optional<Algorithm> find_algorithm_by_name(const std::string& name) {
    for (const AlgorithmDescription& description : all_algorithms()) {
        if (name == description.name) {
            return description.value;
        }
    }
    return std::nullopt;
}

std::optional<Mode> find_mode_by_name(const std::string& name) {
    for (const ModeDescription& description : all_modes()) {
        if (name == description.name) {
            return description.value;
        }
    }
    return std::nullopt;
}

} // namespace

CliOptions parse_cli_options(const std::vector<std::string>& arguments) {
    CliOptions options;

    for (size_t index = 0; index < arguments.size(); ++index) {
        const std::string& argument = arguments[index];

        if (matches_flag(argument, known_flags()[0])) {
            options.show_help = true;
            continue;
        }

        if (matches_flag(argument, known_flags()[1])) {
            const std::string value = take_value(arguments, index, argument);
            const std::optional<Algorithm> algorithm = find_algorithm_by_name(value);
            if (!algorithm.has_value()) {
                throw std::runtime_error("неизвестный алгоритм: " + value);
            }
            options.algorithm = algorithm;
            continue;
        }

        if (matches_flag(argument, known_flags()[2])) {
            const std::string value = take_value(arguments, index, argument);
            const std::optional<Mode> mode = find_mode_by_name(value);
            if (!mode.has_value()) {
                throw std::runtime_error("неизвестный режим работы: " + value);
            }
            options.mode = mode;
            continue;
        }

        if (matches_flag(argument, known_flags()[3])) {
            const std::string value = take_value(arguments, index, argument);
            if (is_stdin_marker(value)) {
                options.key_from_stdin = true;
            } else {
                options.key_file = value;
            }
            continue;
        }

        if (matches_flag(argument, known_flags()[4])) {
            const std::string value = take_value(arguments, index, argument);
            if (is_stdin_marker(value)) {
                options.save_key_to_stdout = true;
            } else {
                options.save_key_file = value;
            }
            continue;
        }

        if (matches_flag(argument, known_flags()[5])) {
            const std::string value = take_value(arguments, index, argument);
            if (is_stdin_marker(value)) {
                options.input_from_stdin = true;
            } else {
                options.input_file = value;
            }
            continue;
        }

        if (matches_flag(argument, known_flags()[6])) {
            const std::string value = take_value(arguments, index, argument);
            if (is_stdin_marker(value)) {
                options.output_to_stdout = true;
            } else {
                options.output_file = value;
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
    std::cout << "  cryptum -a xor -m generate-key -s key.bin\n";
    std::cout << "  cryptum -a xor -m encrypt -k key.bin -i photo.jpg -o photo.jpg.enc\n";
    std::cout << "  cryptum -a atbash -m decrypt -k key.bin -i letter.txt.enc -o letter.txt\n";
}
