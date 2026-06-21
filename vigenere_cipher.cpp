// Динамическая библиотека шифра Виженера.


#include "cipher_api.h"

namespace {

// Минимальная длина ключа в байтах.
constexpr size_t kVigenereMinKeySize = 16;

CipherStatus shift_by_key(ConstBuffer key, ConstBuffer input, MutBuffer* output, bool encrypting) {
    if (key.size < kVigenereMinKeySize) {
        return CipherStatus::InvalidKeyLength;
    }
    if (output == nullptr || output->data == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }

    for (size_t i = 0; i < input.size; ++i) {
        // Байт ключа берётся циклически: i % key.size.
        const uint8_t key_byte = key.data[i % key.size];
        output->data[i] = encrypting
            ? static_cast<uint8_t>(input.data[i] + key_byte)
            : static_cast<uint8_t>(input.data[i] - key_byte);
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} 

extern "C" const AlgorithmInfo* get_algorithm_info() {
    // key_size здесь — минимально допустимая длина ключа (а не точная).
    static const AlgorithmInfo info{"vigenere", kVigenereMinKeySize};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // длина данных не меняется
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return shift_by_key(key, input, output, true);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return shift_by_key(key, input, output, false);
}
