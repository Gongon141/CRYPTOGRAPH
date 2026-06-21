#include "cipher_library.h"

#include <dlfcn.h>

#include <stdexcept>

CipherLibrary::CipherLibrary(const std::string& library_path) {
    // RTLD_NOW — сразу проверить, что все символы библиотеки разрешимы.
    handle_ = dlopen(library_path.c_str(), RTLD_NOW);
    if (handle_ == nullptr) {
        const char* error_message = dlerror();
        throw std::runtime_error("не удалось загрузить библиотеку " + library_path + ": " +
                                 (error_message != nullptr ? error_message : "неизвестная ошибка"));
    }

    // Находим каждую обязательную функцию по её имени. dlsym возвращает void*, поэтому приводим указатель к нужному типу функции из cipher_api.h.
    void* info_symbol = dlsym(handle_, kGetAlgorithmInfoSymbol);
    void* size_symbol = dlsym(handle_, kGetOutputSizeSymbol);
    void* encrypt_symbol = dlsym(handle_, kEncryptSymbol);
    void* decrypt_symbol = dlsym(handle_, kDecryptSymbol);

    if (info_symbol == nullptr || size_symbol == nullptr ||
        encrypt_symbol == nullptr || decrypt_symbol == nullptr) {
        dlclose(handle_);
        handle_ = nullptr;
        throw std::runtime_error("библиотека " + library_path + " не реализует нужный интерфейс");
    }

    get_algorithm_info_fn_ = reinterpret_cast<GetAlgorithmInfoFn>(info_symbol);
    get_output_size_fn_ = reinterpret_cast<GetOutputSizeFn>(size_symbol);
    encrypt_fn_ = reinterpret_cast<EncryptFn>(encrypt_symbol);
    decrypt_fn_ = reinterpret_cast<DecryptFn>(decrypt_symbol);
}

CipherLibrary::~CipherLibrary() {
    if (handle_ != nullptr) {
        dlclose(handle_);
    }
}

const AlgorithmInfo* CipherLibrary::algorithm_info() const {
    return get_algorithm_info_fn_();
}

size_t CipherLibrary::output_size(size_t input_size, CipherOperation operation) const {
    return get_output_size_fn_(input_size, operation);
}

CipherStatus CipherLibrary::encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) const {
    return encrypt_fn_(key, input, output);
}

CipherStatus CipherLibrary::decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) const {
    return decrypt_fn_(key, input, output);
}
