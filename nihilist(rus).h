#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

// Эталонный русский алфавит 4×8 (без буквы Ё)
extern const string NIH_ALPHABET_MATRIX_RU[4][8];

// Проверка: является ли строка русской буквой (UTF-8)
bool nih_isRussianLetter(const string& s);

// Приведение буквы к заглавной, Ё/ё → Е
string nih_normalizeLetterRu(const string& s);

// Поиск координат {строка, столбец} буквы в матрице
pair<int, int> nih_findPositionRu(const string& letter, const string matrix[4][8]);

// Шифрование одной буквы: координаты складываются с координатами символа ключа
string nih_encryptCharRu(const string& ch, const string& keyCh,
                         const string matrix[4][8]);

// Дешифрование одной буквы: координаты вычитаются
string nih_decryptCharRu(const string& ch, const string& keyCh,
                         const string matrix[4][8]);

// Подготовка текста: оставить только русские буквы, привести к верхнему регистру
vector<string> nih_prepareTextRu(const string& raw);

// Подготовка ключа: то же самое, что и подготовка текста
vector<string> nih_prepareKeyRu(const string& raw);

// Построение ключевой матрицы 4×8 по паролю (как в Плейфере)
void nih_buildMatrixRu(const string& keyInput, string matrix[4][8]);