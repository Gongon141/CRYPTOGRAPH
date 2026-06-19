#include "cipher_library.h"

#include <dlfcn.h>

#include <stdexcept>
#include <utility>

namespace {

// Находит символ в библиотеке по имени и приводит его к нужному
// типу функции. Бросает исключение, если символ не найден —
// это значит, что библиотека не реализует требуемый интерфейс.
template <typename FunctionPointer>
FunctionPointer load_symbol(void* handle, const char* symbol_name) {
    void* symbol = dlsym(handle, symbol_name);
    if (symbol == nullptr) {
        throw std::runtime_error(std::string("в библиотеке не найдена функция: ") + symbol_name);
    }
    return reinterpret_cast<FunctionPointer>(symbol);
}

} // namespace

CipherLibrary::CipherLibrary(const std::string& library_path) {
    handle_ = dlopen(library_path.c_str(), RTLD_NOW);
    if (handle_ == nullptr) {
        const char* error_message = dlerror();
        throw std::runtime_error("не удалось загрузить библиотеку " + library_path + ": " +
                                  (error_message != nullptr ? error_message : "неизвестная ошибка"));
    }

    get_algorithm_info_fn_ = load_symbol<GetAlgorithmInfoFn>(handle_, kGetAlgorithmInfoSymbol);
    get_output_size_fn_ = load_symbol<GetOutputSizeFn>(handle_, kGetOutputSizeSymbol);
    encrypt_fn_ = load_symbol<EncryptFn>(handle_, kEncryptSymbol);
    decrypt_fn_ = load_symbol<DecryptFn>(handle_, kDecryptSymbol);
}

CipherLibrary::CipherLibrary(CipherLibrary&& other) noexcept
    : handle_(other.handle_),
      get_algorithm_info_fn_(other.get_algorithm_info_fn_),
      get_output_size_fn_(other.get_output_size_fn_),
      encrypt_fn_(other.encrypt_fn_),
      decrypt_fn_(other.decrypt_fn_) {
    other.handle_ = nullptr;
}

CipherLibrary& CipherLibrary::operator=(CipherLibrary&& other) noexcept {
    if (this != &other) {
        release();

        handle_ = other.handle_;
        get_algorithm_info_fn_ = other.get_algorithm_info_fn_;
        get_output_size_fn_ = other.get_output_size_fn_;
        encrypt_fn_ = other.encrypt_fn_;
        decrypt_fn_ = other.decrypt_fn_;

        other.handle_ = nullptr;
    }
    return *this;
}

CipherLibrary::~CipherLibrary() {
    release();
}

void CipherLibrary::release() {
    if (handle_ != nullptr) {
        dlclose(handle_);
        handle_ = nullptr;
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
