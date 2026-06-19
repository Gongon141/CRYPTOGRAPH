#include "crypto_api.h"

static const AlgorithmInfo info = {"caesar", 1};

extern "C" EXPORT const AlgorithmInfo* get_algorithm_info() {
    return &info;
}

extern "C" EXPORT size_t get_output_size(size_t size, int) {
    return size;
}

extern "C" EXPORT int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    if (!key.data || !input.data || !output || !output->data) return 1;
    if (key.size != info.key_size) return 2;
    if (output->size < input.size) return 3;

    uint8_t shift = key.data[0];
    for (size_t i = 0; i < input.size; i++)
        output->data[i] = input.data[i] + shift;

    output->size = input.size;
    return 0;
}

extern "C" EXPORT int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    if (!key.data || !input.data || !output || !output->data) return 1;
    if (key.size != info.key_size) return 2;
    if (output->size < input.size) return 3;

    uint8_t shift = key.data[0];
    for (size_t i = 0; i < input.size; i++)
        output->data[i] = input.data[i] - shift;

    output->size = input.size;
    return 0;
}