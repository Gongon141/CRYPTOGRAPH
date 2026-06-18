#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

// Эталонный русский алфавит (4 строки × 8 столбцов, без Ё)
extern const string ALPHABET_MATRIX_RU[4][8];

// Проверка: является ли строка из 2 байт русской буквой в UTF-8
bool isRussianLetter(const string& s);

// Приведение буквы к заглавной, замена Ё/ё → Е
string normalizeLetterRu(const string& s);

// Возвращает координаты {строка, столбец} буквы в матрице
pair<int, int> findPositionRu(const string& letter, const string matrix[4][8]);

// Шифрование одной биграммы (пары букв)
string encryptBigramRu(const string& a, const string& b, const string matrix[4][8]);

// Дешифрование одной биграммы
string decryptBigramRu(const string& a, const string& b, const string matrix[4][8]);

// Подготовка текста для шифрования (очистка, нормализация, вставка Ъ)
vector<string> prepareForEncryptionRu(const string& raw);

// Подготовка текста для дешифрования (только очистка и нормализация)
vector<string> prepareForDecryptionRu(const string& raw);

// Заполнение ключевой матрицы 4×8 по введённому ключу
void buildMatrixRu(const string& keyInput, string matrix[4][8]);