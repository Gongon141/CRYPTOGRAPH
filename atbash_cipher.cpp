#include "cipher_api.h"

namespace {

// Общая операция для шифрования и расшифрования
// Ключ не используется , поэтому его длина не проверяется.
CipherStatus apply_atbash(ConstBuffer input, MutBuffer* output) {
    if (output == nullptr || output->data == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }

    for (size_t i = 0; i < input.size; ++i) {
        // Зеркальное отражение значения байта: 0 <-> 255, 1 <-> 254, ...
        output->data[i] = static_cast<uint8_t>(255 - input.data[i]);
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} 

extern "C" const AlgorithmInfo* get_algorithm_info() {
    // key_size == 0 — ключ не требуется
    static const AlgorithmInfo info{"atbash", 0};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // длина данных не меняется
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    (void)key;             // Атбаш не использует ключ
    return apply_atbash(input, output);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    (void)key; // для шифрования и дешифрования операции одинаковы
    return apply_atbash(input, output);
}
