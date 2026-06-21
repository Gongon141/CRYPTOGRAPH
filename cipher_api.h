#ifndef CRYPTUM_CIPHER_API_H
#define CRYPTUM_CIPHER_API_H

#include <cstddef>
#include <cstdint>

// Буфер только для чтения
struct ConstBuffer {
    const uint8_t* data;
    size_t size;
};

// Буфер для записи результата
struct MutBuffer {
    uint8_t* data;
    size_t size;
};

// Какую операцию нужно выполнить — используется в get_output_size(),
enum class CipherOperation : int {
    Encrypt = 0,
    Decrypt = 1,
};

// Код результата работы encrypt()/decrypt().
enum class CipherStatus : int {
    Ok = 0,
    InvalidKeyLength = 1,
    InvalidOutputBuffer = 2,
    InvalidInput = 3,
};

// Метаданные алгоритма: имя для вывода пользователю и требуемая длина ключа в байтах. 
struct AlgorithmInfo {
    const char* algorithm_name;
    size_t key_size;
};

// Сигнатуры функций, которые обязана экспортировать каждая библиотека.

extern "C" {
    using GetAlgorithmInfoFn = const AlgorithmInfo* (*)();
    using GetOutputSizeFn   = size_t (*)(size_t input_size, CipherOperation operation);
    using EncryptFn         = CipherStatus (*)(ConstBuffer key, ConstBuffer input, MutBuffer* output);
    using DecryptFn         = CipherStatus (*)(ConstBuffer key, ConstBuffer input, MutBuffer* output);
}

// Имена символов, которые main ищет в библиотеке через dlsym().
inline constexpr const char* kGetAlgorithmInfoSymbol = "get_algorithm_info";
inline constexpr const char* kGetOutputSizeSymbol    = "get_output_size";
inline constexpr const char* kEncryptSymbol          = "encrypt";
inline constexpr const char* kDecryptSymbol          = "decrypt";

#endif 
