// Загрузка динамической библиотеки шифра через dlopen() и поиск в ней нужных функций через dlsym(). 
#ifndef CRYPTUM_CIPHER_LIBRARY_H
#define CRYPTUM_CIPHER_LIBRARY_H

#include "cipher_api.h"

#include <string>

class CipherLibrary {
public:
    // Загружает библиотеку по пути и находит в ней все обязательные функции.
    
    explicit CipherLibrary(const std::string& library_path);

    // Дескриптор dlopen уникален, поэтому копировать объект нельзя.
    CipherLibrary(const CipherLibrary&) = delete;
    CipherLibrary& operator=(const CipherLibrary&) = delete;

    ~CipherLibrary();

    const AlgorithmInfo* algorithm_info() const;
    size_t output_size(size_t input_size, CipherOperation operation) const;
    CipherStatus encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) const;
    CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) const;

private:
    void* handle_ = nullptr;

    GetAlgorithmInfoFn get_algorithm_info_fn_ = nullptr;
    GetOutputSizeFn get_output_size_fn_ = nullptr;
    EncryptFn encrypt_fn_ = nullptr;
    DecryptFn decrypt_fn_ = nullptr;
};

#endif 
