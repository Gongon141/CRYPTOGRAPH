#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

// Эталонный английский алфавит (5 строк × 5 столбцов, I=J)
extern const string ALPHABET_MATRIX_EN[5][5];

// Проверка: является ли символ английской буквой
bool isEnglishLetter(char c);

// Приведение буквы к заглавной
char normalizeLetterEn(char c);

// Возвращает координаты {строка, столбец} буквы в матрице
pair<int, int> findPositionEn(char letter, const string matrix[5][5]);

// Шифрование одной биграммы
string encryptBigramEn(char a, char b, const string matrix[5][5]);

// Дешифрование одной биграммы
string decryptBigramEn(char a, char b, const string matrix[5][5]);

// Подготовка текста для шифрования (очистка, нормализация, вставка 'X')
vector<char> prepareForEncryptionEn(const string& raw);

// Подготовка текста для дешифрования (только очистка и нормализация)
vector<char> prepareForDecryptionEn(const string& raw);

// Заполнение ключевой матрицы 5×5 по введённому ключу
void buildMatrixEn(const string& keyInput, string matrix[5][5]);