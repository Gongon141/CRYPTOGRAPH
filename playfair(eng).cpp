#include "playfair(eng).h"
#include <algorithm>
#include <cctype>
using namespace std;

// Эталонная матрица: английский алфавит без J (I=J), 5×5
const string ALPHABET_MATRIX_EN[5][5] = {
    {"A","B","C","D","E"},
    {"F","G","H","I","K"},  // I и J объединены
    {"L","M","N","O","P"},
    {"Q","R","S","T","U"},
    {"V","W","X","Y","Z"}
};

// Проверка на английскую букву (A-Z, a-z)
bool isEnglishLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Приведение к заглавной
char normalizeLetterEn(char c) {
    return toupper(static_cast<unsigned char>(c));
}

// Поиск позиции буквы в матрице (J → I)
pair<int, int> findPositionEn(char letter, const string matrix[5][5]) {
    if (letter == 'J') letter = 'I';
    for (int r = 0; r < 5; r++)
        for (int c = 0; c < 5; c++)
            if (matrix[r][c][0] == letter) return {r, c};
    return {-1, -1};
}

string encryptBigramEn(char a, char b, const string matrix[5][5]) {
    auto [r1, c1] = findPositionEn(a, matrix);
    auto [r2, c2] = findPositionEn(b, matrix);
    if (r1 == r2)      // одна строка → вправо
        return matrix[r1][(c1 + 1) % 5] + matrix[r2][(c2 + 1) % 5];
    if (c1 == c2)      // один столбец → вниз
        return matrix[(r1 + 1) % 5][c1] + matrix[(r2 + 1) % 5][c2];
    // прямоугольник → обмен столбцами
    return matrix[r1][c2] + matrix[r2][c1];
}

string decryptBigramEn(char a, char b, const string matrix[5][5]) {
    auto [r1, c1] = findPositionEn(a, matrix);
    auto [r2, c2] = findPositionEn(b, matrix);
    if (r1 == r2)      // одна строка → влево
        return matrix[r1][(c1 - 1 + 5) % 5] + matrix[r2][(c2 - 1 + 5) % 5];
    if (c1 == c2)      // один столбец → вверх
        return matrix[(r1 - 1 + 5) % 5][c1] + matrix[(r2 - 1 + 5) % 5][c2];
    // прямоугольник
    return matrix[r1][c2] + matrix[r2][c1];
}

// Извлечь все английские буквы, привести к заглавным
static vector<char> extractLettersEn(const string& raw) {
    vector<char> letters;
    for (char c : raw) {
        if (isEnglishLetter(c))
            letters.push_back(normalizeLetterEn(c));
    }
    return letters;
}

vector<char> prepareForEncryptionEn(const string& raw) {
    vector<char> letters = extractLettersEn(raw);
    vector<char> prepared;
    // Вставка 'X' между одинаковыми буквами в биграмме
    for (size_t i = 0; i < letters.size(); ) {
        if (i + 1 < letters.size() && letters[i] == letters[i + 1]) {
            prepared.push_back(letters[i]);
            prepared.push_back('X');
            i += 1; // второй символ станет началом новой пары
        } else {
            prepared.push_back(letters[i]);
            if (i + 1 < letters.size()) {
                prepared.push_back(letters[i + 1]);
                i += 2;
            } else {
                i += 1;
            }
        }
    }
    return prepared;
}

vector<char> prepareForDecryptionEn(const string& raw) {
    // Для расшифровки не вставляем X
    return extractLettersEn(raw);
}

void buildMatrixEn(const string& keyInput, string matrix[5][5]) {
    // Собираем уникальные буквы ключа (J → I)
    vector<char> keyLetters;
    for (char c : keyInput) {
        if (isEnglishLetter(c)) {
            char norm = normalizeLetterEn(c);
            if (norm == 'J') norm = 'I'; // J = I
            if (find(keyLetters.begin(), keyLetters.end(), norm) == keyLetters.end())
                keyLetters.push_back(norm);
        }
    }

    int idx = 0;
    // Сначала буквы ключа
    for (char letter : keyLetters) {
        matrix[idx / 5][idx % 5] = letter;
        idx++;
    }
    // Затем оставшиеся буквы алфавита (кроме J)
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            char letter = ALPHABET_MATRIX_EN[r][c][0];
            bool used = false;
            for (int i = 0; i < idx; i++) {
                if (matrix[i / 5][i % 5][0] == letter) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                matrix[idx / 5][idx % 5] = letter;
                idx++;
            }
        }
    }
}//