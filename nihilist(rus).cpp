#include "nihilist(rus).h"
#include <algorithm>

using namespace std;

// Эталонный алфавит 4×8 (без Ё)
const string NIH_ALPHABET_MATRIX_RU[4][8] = {
    {"А","Б","В","Г","Д","Е","Ж","З"},
    {"И","Й","К","Л","М","Н","О","П"},
    {"Р","С","Т","У","Ф","Х","Ц","Ч"},
    {"Ш","Щ","Ъ","Ы","Ь","Э","Ю","Я"}
};

// Проверка принадлежности строки к русским буквам (UTF-8, 2 байта)
bool nih_isRussianLetter(const string& s) {
    if (s.size() != 2) return false;
    unsigned char c1 = s[0], c2 = s[1];
    // Диапазоны UTF-8: А-Я (0xD0 0x90-0xAF), а-я (0xD0 0xB0-0xBF), Ё (0xD0 0x81), ё (0xD1 0x91)
    if (c1 == 0xD0) {
        if ((c2 >= 0x90 && c2 <= 0xAF) || (c2 >= 0xB0 && c2 <= 0xBF) || c2 == 0x81)
            return true;
    } else if (c1 == 0xD1 && c2 == 0x91) {
        return true; // строчная ё
    }
    return false;
}

// Нормализация: Ё/ё → Е, строчные → заглавные
string nih_normalizeLetterRu(const string& s) {
    if (s == "Ё" || s == "ё") return "Е";
    unsigned char c1 = s[0], c2 = s[1];
    if (c1 == 0xD0 && c2 >= 0xB0 && c2 <= 0xBF)  // строчные а-я
        return string(1, (char)c1) + string(1, (char)(c2 - 0x20)); // увеличиваем код до заглавной
    return s;
}

// Поиск координат буквы в матрице (перебором)
pair<int, int> nih_findPositionRu(const string& letter, const string matrix[4][8]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 8; c++)
            if (matrix[r][c] == letter) return {r, c};
    return {-1, -1}; // никогда не должно случиться
}

// Шифрование: (r_буквы + r_ключа) % 4, (c_буквы + c_ключа) % 8
string nih_encryptCharRu(const string& ch, const string& keyCh, const string matrix[4][8]) {
    auto [r1, c1] = nih_findPositionRu(ch, matrix);
    auto [r2, c2] = nih_findPositionRu(keyCh, matrix);
    return matrix[(r1 + r2) % 4][(c1 + c2) % 8];
}

// Дешифрование: (r_зашифр - r_ключа + 4) % 4, (c_зашифр - c_ключа + 8) % 8
string nih_decryptCharRu(const string& ch, const string& keyCh, const string matrix[4][8]) {
    auto [r1, c1] = nih_findPositionRu(ch, matrix);
    auto [r2, c2] = nih_findPositionRu(keyCh, matrix);
    int nr = (r1 - r2 + 4) % 4;  // +4 для предотвращения отрицательного остатка
    int nc = (c1 - c2 + 8) % 8;
    return matrix[nr][nc];
}

// Извлечение и нормализация русских букв из сырой строки
vector<string> nih_prepareTextRu(const string& raw) {
    vector<string> letters;
    for (size_t i = 0; i < raw.size(); ) {
        int len = 1;
        if ((unsigned char)raw[i] >= 0xC0) len = 2; // русские символы двухбайтовые
        string sym = raw.substr(i, len);
        if (nih_isRussianLetter(sym))
            letters.push_back(nih_normalizeLetterRu(sym));
        i += len;
    }
    return letters;
}

// Подготовка ключа – аналогична подготовке текста
vector<string> nih_prepareKeyRu(const string& raw) {
    return nih_prepareTextRu(raw);
}

// Построение матрицы 4×8 по ключу (уникальные буквы ключа + алфавит)
void nih_buildMatrixRu(const string& keyInput, string matrix[4][8]) {
    // Собираем уникальные буквы ключа в порядке появления
    vector<string> keyLetters;
    for (size_t i = 0; i < keyInput.size(); ) {
        int len = 1;
        if ((unsigned char)keyInput[i] >= 0xC0) len = 2;
        string sym = keyInput.substr(i, len);
        if (nih_isRussianLetter(sym)) {
            string norm = nih_normalizeLetterRu(sym);
            if (find(keyLetters.begin(), keyLetters.end(), norm) == keyLetters.end())
                keyLetters.push_back(norm);
        }
        i += len;
    }

    // Заполняем матрицу сначала буквами ключа, затем оставшимися из алфавита
    int idx = 0;
    for (const string& letter : keyLetters) {
        matrix[idx / 8][idx % 8] = letter;
        idx++;
    }
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 8; c++) {
            const string& letter = NIH_ALPHABET_MATRIX_RU[r][c];
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