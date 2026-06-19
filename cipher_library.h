// Загрузка динамической библиотеки шифра через dlopen() и поиск
// в ней нужных функций через dlsym(). Сама библиотека ничего не
// знает про файлы и аргументы — только про байты.

#ifndef CRYPTUM_CIPHER_LIBRARY_H
#define CRYPTUM_CIPHER_LIBRARY_H

#include "cipher_api.h"

#include <string>

// Держит открытый дескриптор библиотеки (dlopen) и указатели на её
// экспортируемые функции. Закрывает библиотеку автоматически
// в деструкторе (RAII), чтобы не забыть вызвать dlclose().
class CipherLibrary {
public:
    // Загружает библиотеку по указанному пути и находит в ней все
    // обязательные функции. Бросает std::runtime_error при любой ошибке.
    explicit CipherLibrary(const std::string& library_path);

    // Библиотеку нельзя копировать (дескриптор dlopen уникален),
    // но можно перемещать.
    CipherLibrary(const CipherLibrary&) = delete;
    CipherLibrary& operator=(const CipherLibrary&) = delete;
    CipherLibrary(CipherLibrary&& other) noexcept;
    CipherLibrary& operator=(CipherLibrary&& other) noexcept;

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

    void release();
};

#endif // CRYPTUM_CIPHER_LIBRARY_H
