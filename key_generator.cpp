#include "key_generator.h"

#include <random>

std::vector<uint8_t> generate_random_key(size_t key_size_in_bytes) {
    std::random_device random_source;
    std::uniform_int_distribution<int> byte_distribution(0, 255);

    std::vector<uint8_t> key(key_size_in_bytes);
    for (uint8_t& byte : key) {
        byte = static_cast<uint8_t>(byte_distribution(random_source));
    }
    return key;
}
