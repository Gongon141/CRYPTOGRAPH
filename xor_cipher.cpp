// Динамическая библиотека шифра гаммирования (XOR).

#include "cipher_api.h"

namespace {

// Общая операция для шифрования и расшифрования — у XOR они совпадают.
CipherStatus apply_xor(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    if (key.size == 0) {
        return CipherStatus::InvalidKeyLength;
    }
    if (output == nullptr || output->data == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }

    for (size_t i = 0; i < input.size; ++i) {
        // Байт ключа берётся циклически: i % key.size.
        output->data[i] = static_cast<uint8_t>(input.data[i] ^ key.data[i % key.size]);
    }

    output->size = input.size;
    return CipherStatus::Ok;
}

} 

extern "C" const AlgorithmInfo* get_algorithm_info() {
    // key_size == 0 — длина ключа не фиксирована, подойдёт ключ любой длины.
    static const AlgorithmInfo info{"xor", 0};
    return &info;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation operation) {
    (void)operation;       // длина данных не меняется
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    return apply_xor(key, input, output);
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    // Для XOR расшифрование полностью совпадает с шифрованием.
    return apply_xor(key, input, output);
}
