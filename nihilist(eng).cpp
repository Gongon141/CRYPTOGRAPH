#include "nihilist(eng).h"
#include <algorithm>
#include <cctype>

using namespace std;

// Эталонная матрица английского алфавита (I=J, без J)
const string NIH_ALPHABET_MATRIX_EN[5][5] = {
    {"A","B","C","D","E"},
    {"F","G","H","I","K"},  // Буква I заменяет J
    {"L","M","N","O","P"},
    {"Q","R","S","T","U"},
    {"V","W","X","Y","Z"}
};

// Проверка на принадлежность к английским буквам
bool nih_isEnglishLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Приведение к верхнему регистру
char nih_normalizeLetterEn(char c) {
    return toupper(static_cast<unsigned char>(c));
}

// Поиск координат: J отображается на I
pair<int, int> nih_findPositionEn(char letter, const string matrix[5][5]) {
    if (letter == 'J') letter = 'I';   // I и J – одна ячейка
    for (int r = 0; r < 5; r++)
        for (int c = 0; c < 5; c++)
            if (matrix[r][c][0] == letter) return {r, c};
    return {-1, -1};
}

// Шифрование: (r1+r2) % 5, (c1+c2) % 5
char nih_encryptCharEn(char ch, char keyCh, const string matrix[5][5]) {
    auto [r1, c1] = nih_findPositionEn(ch, matrix);
    auto [r2, c2] = nih_findPositionEn(keyCh, matrix);
    return matrix[(r1 + r2) % 5][(c1 + c2) % 5][0];
}

// Дешифрование: (r1 - r2 + 5) % 5, (c1 - c2 + 5) % 5
char nih_decryptCharEn(char ch, char keyCh, const string matrix[5][5]) {
    auto [r1, c1] = nih_findPositionEn(ch, matrix);
    auto [r2, c2] = nih_findPositionEn(keyCh, matrix);
    int nr = (r1 - r2 + 5) % 5;
    int nc = (c1 - c2 + 5) % 5;
    return matrix[nr][nc][0];
}

// Извлечение всех английских букв, перевод в заглавные
vector<char> nih_prepareTextEn(const string& raw) {
    vector<char> letters;
    for (char c : raw)
        if (nih_isEnglishLetter(c))
            letters.push_back(nih_normalizeLetterEn(c));
    return letters;
}

// Подготовка ключа – идентична подготовке текста
vector<char> nih_prepareKeyEn(const string& raw) {
    return nih_prepareTextEn(raw);
}

// Построение матрицы 5×5 по ключу
void nih_buildMatrixEn(const string& keyInput, string matrix[5][5]) {
    // Собираем уникальные буквы ключа (J → I)
    vector<char> keyLetters;
    for (char c : keyInput) {
        if (nih_isEnglishLetter(c)) {
            char norm = nih_normalizeLetterEn(c);
            if (norm == 'J') norm = 'I';
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
    // Затем оставшиеся буквы алфавита (без повторов)
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            char letter = NIH_ALPHABET_MATRIX_EN[r][c][0];
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