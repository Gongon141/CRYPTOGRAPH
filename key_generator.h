// Генерация ключа с помощью криптостойкого генератора случайных чисел операционной системы.

#ifndef CRYPTUM_KEY_GENERATOR_H
#define CRYPTUM_KEY_GENERATOR_H

#include <cstdint>
#include <vector>

// Заполняет вектор случайными байтами, используя std::random_device
std::vector<uint8_t> generate_random_key(size_t key_size_in_bytes);

#endif
