#pragma once
#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

struct ConstBuffer {
    const uint8_t* data;
    size_t size;
};

struct MutBuffer {
    uint8_t* data;
    size_t size;
};

struct AlgorithmInfo {
    const char* algorithm_name;
    size_t key_size;
};

extern "C" {
    EXPORT const AlgorithmInfo* get_algorithm_info();
    EXPORT size_t get_output_size(size_t input_size, int operation_type);
    EXPORT int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output);
    EXPORT int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output);
}