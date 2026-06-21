cmake_minimum_required(VERSION 3.10)
project(cryptum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Предупреждения компилятора считаются ошибками — требование ТЗ.
add_compile_options(-Wall -Wextra -Werror)

# Заголовки лежат в той же папке, что и исходники.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Исполняемые файлы — в bin, динамические библиотеки — в bin/lib рядом с ними.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

# --- Динамические библиотеки шифров ---
add_library(xor SHARED xor_cipher.cpp)
set_target_properties(xor PROPERTIES PREFIX "lib" OUTPUT_NAME "xor")

add_library(atbash SHARED atbash_cipher.cpp)
set_target_properties(atbash PROPERTIES PREFIX "lib" OUTPUT_NAME "atbash")

add_library(caesar SHARED caesar_cipher.cpp)
set_target_properties(caesar PROPERTIES PREFIX "lib" OUTPUT_NAME "caesar")

add_library(vigenere SHARED vigenere_cipher.cpp)
set_target_properties(vigenere PROPERTIES PREFIX "lib" OUTPUT_NAME "vigenere")

add_library(playfair SHARED playfair_cipher.cpp)
set_target_properties(playfair PROPERTIES PREFIX "lib" OUTPUT_NAME "playfair")

add_library(nihilist SHARED nihilist_cipher.cpp)
set_target_properties(nihilist PROPERTIES PREFIX "lib" OUTPUT_NAME "nihilist")

# --- Главный исполняемый модуль ---
add_executable(cryptum
    main.cpp
    cli_options.cpp
    cipher_library.cpp
    file_io.cpp
    key_generator.cpp
)
target_link_libraries(cryptum dl)

# --- Тестовая утилита ---
add_executable(cryptum_tests
    test_ciphers.cpp
    cipher_library.cpp
)
target_link_libraries(cryptum_tests dl)

# Библиотеки должны собираться раньше исполняемых файлов, которые их загружают.
add_dependencies(cryptum xor atbash caesar vigenere playfair nihilist)
add_dependencies(cryptum_tests xor atbash caesar vigenere playfair nihilist)
