// Разбор аргументов командной строки. Поддерживаются короткие (-a) и длинные (--algorithm) псевдонимы флагов

#ifndef CRYPTUM_CLI_OPTIONS_H
#define CRYPTUM_CLI_OPTIONS_H

#include <string>
#include <vector>

// Какой алгоритм шифрования выбран
enum class Algorithm {
    Xor,
    Atbash,
    Caesar,
    Vigenere,
    Playfair,
    Nihilist,
};

// Что должна сделать программа в этом запуске.
enum class Mode {
    Encrypt,
    Decrypt,
    GenerateKey,
};

// Описание одного алгоритма: само значение и его имя в командной строке.
struct AlgorithmDescription {
    Algorithm value;
    std::string name;
};

// Аналогичное описание для режима работы.
struct ModeDescription {
    Mode value;
    std::string name;
};

// Таблицы соответствий "текст в командной строке <-> значение перечисления".
std::vector<AlgorithmDescription> all_algorithms();
std::vector<ModeDescription> all_modes();

// Имя файла библиотеки для выбранного алгоритма. Имя зависит от операционной
// системы: в Linux это "libxor.so", в Windows — "xor.dll"
std::string library_file_name(Algorithm algorithm);

// Результат разбора аргументов командной строки.
struct CliOptions {
    bool show_help = false;

    bool has_algorithm = false;
    Algorithm algorithm = Algorithm::Xor;

    bool has_mode = false;
    Mode mode = Mode::Encrypt;

    // Источник ключа: файл, стандартный ввод или генерация ключа на лету.
    bool key_from_file = false;
    std::string key_file;
    bool key_from_stdin = false;
    bool generate_key = false;    

    // Куда сохранить ключ (при генерации).
    bool save_key_to_file = false;
    std::string save_key_file;
    bool save_key_to_stdout = false;

    // Входные данные.
    bool input_from_file = false;
    std::string input_file;
    bool input_from_stdin = false;

    // Выходные данные.
    bool output_to_file = false;
    std::string output_file;
    bool output_to_stdout = false;
};

// Разбирает массив аргументов (без имени программы argv[0]).
CliOptions parse_cli_options(std::vector<std::string> arguments);

// Печатает в стандартный вывод справку по программе.
void print_help();

#endif 