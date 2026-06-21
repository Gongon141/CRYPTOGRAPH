// Динамическая библиотека шифра нигилистов.
#include "cipher_api.h"

namespace {

// Сторона квадрата: 16 * 16 = 256 — по ячейке на каждое значение байта.
constexpr int kSquareSide = 256 / 16;

// square[index]   — байт в ячейке с линейным индексом index;
// position[value] — линейный индекс ячейки байта value (обратный поиск).
// Координаты ячейки: строка = index / 16, столбец = index % 16.
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

// Общая часть шифрования и расшифрования: координаты текущего байта текста складываются с координатами байта ключа (при расшифровании — вычитаются).
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

    for (size_t i = 0; i < input.size; ++i) {
        const int text_row = position[input.data[i]] / kSquareSide;
        const int text_col = position[input.data[i]] % kSquareSide;

        // Байт ключа берётся циклически: ключ повторяется, если он короче данных.
        const uint8_t key_byte = key.data[i % key.size];
        const int key_row = position[key_byte] / kSquareSide;
        const int key_col = position[key_byte] % kSquareSide;

        int result_row;
        int result_col;
        if (encrypting) {
            result_row = (text_row + key_row) % kSquareSide;
            result_col = (text_col + key_col) % kSquareSide;
        } else {
            // + kSquareSide перед взятием остатка, чтобы не получить отрицательное число.
            result_row = (text_row - key_row + kSquareSide) % kSquareSide;
            result_col = (text_col - key_col + kSquareSide) % kSquareSide;
        }

        output->data[i] = square[result_row * kSquareSide + result_col];
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} // namespace

extern "C" const AlgorithmInfo* get_algorithm_info() {
    // key_size == 0 — длина ключа не фиксирована.
    static const AlgorithmInfo info{"nihilist", 0};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // шифр не меняет длину данных
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return transform(key, input, output, true);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return transform(key, input, output, false);
}
