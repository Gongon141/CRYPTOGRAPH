// Шифр XOR с повторяющимся ключом произвольной длины.
// C[i] = P[i] XOR K[i mod key.size]
// Операция самообратима: encrypt() и decrypt() делают одно и то же.

#include "cipher_api.h"

namespace {

// Применяет XOR с ключом, циклически растягивая его на весь буфер.
void apply_xor(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    for (size_t i = 0; i < input.size; ++i) {
        const uint8_t key_byte = key.data[i % key.size];
        output->data[i] = input.data[i] ^ key_byte;
    }
    output->size = input.size;
}

// key_size == 0 означает "длина ключа не фиксирована" — для XOR
// подходит ключ любой ненулевой длины.
const AlgorithmInfo kXorInfo{"xor", 0};

} // namespace

extern "C" const AlgorithmInfo* get_algorithm_info() {
    return &kXorInfo;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation /*operation*/) {
    // XOR — потоковый шифр без выравнивания по блокам:
    // выходные данные всегда равны по размеру входным.
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    if (key.data == nullptr || key.size == 0) {
        return CipherStatus::InvalidKeyLength;
    }
    if (output == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }
    if (input.size > 0 && output->data == nullptr) {
        return CipherStatus::InvalidOutputBuffer;
    }
    apply_xor(key, input, output);
    return CipherStatus::Ok;
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    // XOR самообратим, поэтому расшифрование реализуется той же операцией.
    return encrypt(key, input, output);
}
