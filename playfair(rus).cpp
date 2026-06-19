#include "playfair(rus).h"
#include <algorithm>
using namespace std;

// Эталонная матрица алфавита (без Ё, 32 буквы)
const string ALPHABET_MATRIX_RU[4][8] = {
    {"А","Б","В","Г","Д","Е","Ж","З"},
    {"И","Й","К","Л","М","Н","О","П"},
    {"Р","С","Т","У","Ф","Х","Ц","Ч"},
    {"Ш","Щ","Ъ","Ы","Ь","Э","Ю","Я"}
};

// Проверка по диапазонам UTF-8 (русские буквы занимают 2 байта)
bool isRussianLetter(const string& s) {
    if (s.size() != 2) return false;
    unsigned char c1 = s[0], c2 = s[1];
    if (c1 == 0xD0) {
        if ((c2 >= 0x90 && c2 <= 0xAF) || (c2 >= 0xB0 && c2 <= 0xBF) || c2 == 0x81)
            return true;
    } else if (c1 == 0xD1 && c2 == 0x91) { // ё
        return true;
    }
    return false;
}

// Ё и ё → Е, строчные → заглавные
string normalizeLetterRu(const string& s) {
    if (s == "Ё" || s == "ё") return "Е";
    unsigned char c1 = s[0], c2 = s[1];
    if (c1 == 0xD0 && c2 >= 0xB0 && c2 <= 0xBF) { // а-я → А-Я
        return string(1, (char)c1) + string(1, (char)(c2 - 0x20));
    }
    return s;
}

// Линейный поиск координат буквы в матрице
pair<int, int> findPositionRu(const string& letter, const string matrix[4][8]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 8; c++)
            if (matrix[r][c] == letter) return {r, c};
    return {-1, -1}; // такого быть не должно
}

string encryptBigramRu(const string& a, const string& b, const string matrix[4][8]) {
    auto [r1, c1] = findPositionRu(a, matrix);
    auto [r2, c2] = findPositionRu(b, matrix);
    if (r1 == r2)      // одна строка → сдвиг вправо
        return matrix[r1][(c1 + 1) % 8] + matrix[r2][(c2 + 1) % 8];
    if (c1 == c2)      // один столбец → сдвиг вниз
        return matrix[(r1 + 1) % 4][c1] + matrix[(r2 + 1) % 4][c2];
    // прямоугольник → обмен столбцами
    return matrix[r1][c2] + matrix[r2][c1];
}

string decryptBigramRu(const string& a, const string& b, const string matrix[4][8]) {
    auto [r1, c1] = findPositionRu(a, matrix);
    auto [r2, c2] = findPositionRu(b, matrix);
    if (r1 == r2)      // одна строка → сдвиг влево
        return matrix[r1][(c1 - 1 + 8) % 8] + matrix[r2][(c2 - 1 + 8) % 8];
    if (c1 == c2)      // один столбец → сдвиг вверх
        return matrix[(r1 - 1 + 4) % 4][c1] + matrix[(r2 - 1 + 4) % 4][c2];
    // прямоугольник
    return matrix[r1][c2] + matrix[r2][c1];
}

// Вспомогательная: извлечь все русские буквы, привести к нормальной форме
static vector<string> extractLettersRu(const string& raw) {
    vector<string> letters;
    for (size_t i = 0; i < raw.size(); ) {
        int len = 1;
        if ((unsigned char)raw[i] >= 0xC0) len = 2; // русские буквы двухбайтовые
        string sym = raw.substr(i, len);
        if (isRussianLetter(sym))
            letters.push_back(normalizeLetterRu(sym));
        i += len;
    }
    return letters;
}

vector<string> prepareForEncryptionRu(const string& raw) {
    vector<string> letters = extractLettersRu(raw);
    vector<string> prepared;
    // Вставка Ъ между одинаковыми буквами в биграммах
    for (size_t i = 0; i < letters.size(); ) {
        if (i + 1 < letters.size() && letters[i] == letters[i + 1]) {
            prepared.push_back(letters[i]);
            prepared.push_back("Ъ");
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

vector<string> prepareForDecryptionRu(const string& raw) {
    // Для расшифровки не вставляем Ъ, только очищаем
    return extractLettersRu(raw);
}

void buildMatrixRu(const string& keyInput, string matrix[4][8]) {
    // Собираем уникальные буквы ключа в порядке появления
    vector<string> keyLetters;
    for (size_t i = 0; i < keyInput.size(); ) {
        int len = 1;
        if ((unsigned char)keyInput[i] >= 0xC0) len = 2;
        string sym = keyInput.substr(i, len);
        if (isRussianLetter(sym)) {
            string norm = normalizeLetterRu(sym);
            if (find(keyLetters.begin(), keyLetters.end(), norm) == keyLetters.end())
                keyLetters.push_back(norm);
        }
        i += len;
    }

    // Заполняем матрицу: сначала буквы ключа, затем алфавит
    int idx = 0;
    for (const string& letter : keyLetters) {
        matrix[idx / 8][idx % 8] = letter;
        idx++;
    }
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 8; c++) {
            const string& letter = ALPHABET_MATRIX_RU[r][c];
            bool used = false;
            for (int i = 0; i < idx; i++) {
                if (matrix[i / 8][i % 8] == letter) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                matrix[idx / 8][idx % 8] = letter;
                idx++;
            }
        }
    }
}