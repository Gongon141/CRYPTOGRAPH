// Динамическая библиотека шифра Плейфера.
#include "cipher_api.h"

namespace {

// Сторона квадрата: 16 * 16 = 256 ячеек — по одной на каждое значение байта.
constexpr int kSquareSide = 256 / 16;

// square[index]    — значение байта в ячейке с линейным индексом index;
// position[value]  — линейный индекс ячейки, в которой стоит байт value
//                    (обратный поиск, чтобы не искать координаты перебором).
// Линейный индекс однозначно раскладывается на (строку, столбец):
// строка = index / 16, столбец = index % 16.
void build_square(ConstBuffer key, uint8_t square[256], uint8_t position[256]) {
    bool already_used[256] = {false};
    int next_cell = 0;

    for (size_t i = 0; i < key.size; ++i) {
        const uint8_t value = key.data[i];
        if (!already_used[value]) {
            square[next_cell] = value;
            position[value] = static_cast<uint8_t>(next_cell);
            already_used[value] = true;
            ++next_cell;
        }
    }

    for (int value = 0; value < 256; ++value) {
        if (!already_used[value]) {
            square[next_cell] = static_cast<uint8_t>(value);
            position[static_cast<uint8_t>(value)] = static_cast<uint8_t>(next_cell);
            ++next_cell;
        }
    }
}

// Преобразует одну пару несовпадающих байт по правилам Плейфера.

void transform_pair(uint8_t first, uint8_t second,
                    const uint8_t square[256], const uint8_t position[256],
                    int shift, uint8_t& out_first, uint8_t& out_second) {
    const int first_row = position[first] / kSquareSide;
    const int first_col = position[first] % kSquareSide;
    const int second_row = position[second] / kSquareSide;
    const int second_col = position[second] % kSquareSide;

    if (first_row == second_row) {
        out_first = square[first_row * kSquareSide + (first_col + shift) % kSquareSide];
        out_second = square[second_row * kSquareSide + (second_col + shift) % kSquareSide];
    } else if (first_col == second_col) {
        out_first = square[((first_row + shift) % kSquareSide) * kSquareSide + first_col];
        out_second = square[((second_row + shift) % kSquareSide) * kSquareSide + second_col];
    } else {
        out_first = square[first_row * kSquareSide + second_col];
        out_second = square[second_row * kSquareSide + first_col];
    }
}

// Общая часть шифрования и расшифрования: отличается только направлением.
CipherStatus transform(ConstBuffer key, ConstBuffer input, MutBuffer* output, bool encrypting) {
    if (key.size == 0) {
        return CipherStatus::InvalidKeyLength;
    }
    if (output == nullptr || output->data == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }

    uint8_t square[256];
    uint8_t position[256];
    build_square(key, square, position);

    const int shift = encrypting ? 1 : (kSquareSide - 1);

    size_t index = 0;
    for (; index + 1 < input.size; index += 2) {
        const uint8_t first = input.data[index];
        const uint8_t second = input.data[index + 1];

        // Пара из двух одинаковых байт оставляется без изменений: для неё
        // правила Плейфера не определены. Это безопасно, потому что любая
        // пара из РАЗНЫХ байт всегда переходит в пару из разных байт, значит
        // одинаковая пара в шифротексте однозначно означает такую же пару в
        // открытом тексте, и расшифрование восстановит её точно. Так длина
        // данных не меняется и сохраняется полная обратимость.
        if (first == second) {
            output->data[index] = first;
            output->data[index + 1] = second;
        } else {
            transform_pair(first, second, square, position, shift,
                           output->data[index], output->data[index + 1]);
        }
    }

    // Непарный последний байт (при нечётной длине) остаётся как есть.
    if (index < input.size) {
        output->data[index] = input.data[index];
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} 

extern "C" const AlgorithmInfo* get_algorithm_info() {
    // key_size == 0 — длина ключа не фиксирована: подойдёт ключ любой длины.
    static const AlgorithmInfo info{"playfair", 0};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // шифр не меняет длину данных при любой операции
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return transform(key, input, output, true);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return transform(key, input, output, false);
}
