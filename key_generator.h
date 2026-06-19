// Генерация ключа с помощью криптостойкого генератора случайных
// чисел операционной системы.

#ifndef CRYPTUM_KEY_GENERATOR_H
#define CRYPTUM_KEY_GENERATOR_H

#include <cstdint>
#include <vector>

// Заполняет вектор случайными байтами, используя std::random_device
// (на Linux он читает данные из /dev/urandom) — этот источник
// подходит для криптографических целей в отличие от std::rand().
std::vector<uint8_t> generate_random_key(size_t key_size_in_bytes);

#endif // CRYPTUM_KEY_GENERATOR_H
