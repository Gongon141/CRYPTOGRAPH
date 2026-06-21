// Динамическая библиотека шифра Цезаря.
#include "cipher_api.h"

namespace {

// ровно один байт (величина сдвига).
constexpr size_t kCaesarKeySize = 1;

CipherStatus shift_bytes(ConstBuffer key, ConstBuffer input, MutBuffer* output, bool encrypting) {
    if (key.size != kCaesarKeySize) {
        return CipherStatus::InvalidKeyLength;
    }
    if (output == nullptr || output->data == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }

    const uint8_t shift = key.data[0];
    for (size_t i = 0; i < input.size; ++i) {
        // Сложение/вычитание по модулю 256 обеспечивается типом uint8_t.
        output->data[i] = encrypting ? static_cast<uint8_t>(input.data[i] + shift) : static_cast<uint8_t>(input.data[i] - shift);
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} 

extern "C" const AlgorithmInfo* get_algorithm_info() {
    static const AlgorithmInfo info{"caesar", kCaesarKeySize};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // длина данных не меняется
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return shift_bytes(key, input, output, true);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return shift_bytes(key, input, output, false);
}
