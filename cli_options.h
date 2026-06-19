// Разбор аргументов командной строки. Поддерживаются короткие
// (-a) и длинные (--algorithm) псевдонимы флагов.

#ifndef CRYPTUM_CLI_OPTIONS_H
#define CRYPTUM_CLI_OPTIONS_H

#include <optional>
#include <string>
#include <vector>

// Какой алгоритм шифрования выбран. Список можно расширять —
// весь остальной код работает с этим enum через перебор, а не
// через switch/case на конкретные значения.
enum class Algorithm {
    Xor,
    Atbash,
};

// Что должна сделать программа в этом запуске.
enum class Mode {
    Encrypt,
    Decrypt,
    GenerateKey,
};

// Описание одного значения Algorithm: само значение и его текстовое
// имя в командной строке. Используется для перебора при разборе
// аргументов и при выводе справки, вместо switch/case.
struct AlgorithmDescription {
    Algorithm value;
    const char* name;
};

// Аналогичное описание для Mode.
struct ModeDescription {
    Mode value;
    const char* name;
};

// Полные таблицы соответствий "текст в командной строке <-> enum".
// Если в программу добавится новый алгоритм или режим — достаточно
// дописать одну строку в соответствующую таблицу.
const std::vector<AlgorithmDescription>& all_algorithms();
const std::vector<ModeDescription>& all_modes();

// Возвращает имя библиотеки (без пути), которую нужно загрузить
// для данного алгоритма, например "libxor.so".
std::string library_file_name(Algorithm algorithm);

// Результат разбора аргументов командной строки.
struct CliOptions {
    bool show_help = false;

    std::optional<Algorithm> algorithm;
    std::optional<Mode> mode;

    // Ключ: либо путь к файлу, либо чтение из стандартного ввода.
    std::optional<std::string> key_file;
    bool key_from_stdin = false;

    // Куда сохранить ключ при генерации (необязательно).
    std::optional<std::string> save_key_file;
    bool save_key_to_stdout = false;

    // Входные данные: файл или стандартный ввод.
    std::optional<std::string> input_file;
    bool input_from_stdin = false;

    // Выходные данные: файл или стандартный вывод.
    std::optional<std::string> output_file;
    bool output_to_stdout = false;
};

// Разбирает массив аргументов командной строки (без argv[0]).
// Бросает std::runtime_error с понятным сообщением при ошибке разбора.
CliOptions parse_cli_options(const std::vector<std::string>& arguments);

// Печатает в стандартный вывод справку по флагам программы.
void print_help();

#endif // CRYPTUM_CLI_OPTIONS_H
