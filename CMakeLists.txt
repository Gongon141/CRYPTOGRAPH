cmake_minimum_required(VERSION 3.10)
project(cryptum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -fPIC)
endif()

add_library(caesar SHARED caesar.cpp)
add_library(vigenere SHARED vigenere.cpp)
add_executable(cryptum main.cpp)

if (NOT MSVC)
    target_link_libraries(cryptum dl)
endif()