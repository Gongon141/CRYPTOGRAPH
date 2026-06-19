#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

// Эталонный английский алфавит 5×5 (I и J объединены)
extern const string NIH_ALPHABET_MATRIX_EN[5][5];

// Проверка: является ли символ английской буквой
bool nih_isEnglishLetter(char c);

// Приведение буквы к заглавной
char nih_normalizeLetterEn(char c);

// Поиск координат буквы (J заменяется на I)
pair<int, int> nih_findPositionEn(char letter, const string matrix[5][5]);

// Шифрование буквы сложением координат с ключом
char nih_encryptCharEn(char ch, char keyCh, const string matrix[5][5]);

// Дешифрование буквы вычитанием координат ключа
char nih_decryptCharEn(char ch, char keyCh, const string matrix[5][5]);

// Подготовка текста: извлечение и нормализация английских букв
vector<char> nih_prepareTextEn(const string& raw);

// Подготовка ключа (аналогично тексту)
vector<char> nih_prepareKeyEn(const string& raw);

// Построение ключевой матрицы 5×5
void nih_buildMatrixEn(const string& keyInput, string matrix[5][5]);//