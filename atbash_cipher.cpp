// Шифр Атбаш: зеркальная замена букв латинского алфавита
// (A<->Z, B<->Y, ... a<->z, b<->y, ...). Остальные байты не меняются,
// чтобы можно было шифровать произвольные файлы, а не только текст
// из букв алфавита.
//
// Ключ алгоритму не нужен по смыслу преобразования, но интерфейс
// encrypt()/decrypt() сохраняет параметр ключа для совместимости
// с общим C ABI — его содержимое здесь просто не используется.

#include "cipher_api.h"

namespace {

// Отражает один байт относительно середины латинского алфавита,
// если это буква, иначе возвращает байт без изменений.
uint8_t mirror_byte(uint8_t byte) {
    if (byte >= 'A' && byte <= 'Z') {
        return static_cast<uint8_t>('A' + ('Z' - byte));
    }
    if (byte >= 'a' && byte <= 'z') {
        return static_cast<uint8_t>('a' + ('z' - byte));
    }
    return byte;
}

void apply_atbash(ConstBuffer input, MutBuffer* output) {
    for (size_t i = 0; i < input.size; ++i) {
        output->data[i] = mirror_byte(input.data[i]);
    }
    output->size = input.size;
}

// key_size == 0: Атбаш не использует ключ, но main всё равно может
// запросить генерацию "ключа" нулевой длины ради единообразия команд.
const AlgorithmInfo kAtbashInfo{"atbash", 0};

} // namespace

extern "C" const AlgorithmInfo* get_algorithm_info() {
    return &kAtbashInfo;
}

extern "C" size_t get_output_size(size_t input_size, CipherOperation /*operation*/) {
    return input_size;
}

extern "C" CipherStatus encrypt(ConstBuffer /*key*/, ConstBuffer input, MutBuffer* output) {
    if (output == nullptr || output->size < input.size) {
        return CipherStatus::InvalidOutputBuffer;
    }
    if (input.size > 0 && output->data == nullptr) {
        return CipherStatus::InvalidOutputBuffer;
    }
    apply_atbash(input, output);
    return CipherStatus::Ok;
}

extern "C" CipherStatus decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
    // Отражение симметрично самому себе: повторное применение
    // восстанавливает исходные данные, поэтому decrypt == encrypt.
    return encrypt(key, input, output);
}
