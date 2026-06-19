#include "ciphers.h"
#include <stdexcept>
#include <cwctype>

using namespace std;

namespace {
    // Нигилистов
    const wstring NIHIL_ALPHABET = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ .,";
    const int N_SIZE = 6;

    // Плейфера
    const wstring RUS_ALPH = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"; // 32 буквы
    const wstring ENG_ALPH = L"ABCDEFGHIKLMNOPQRSTUVWXYZ";        // 25 букв

    wchar_t normalize(wchar_t c) {
        return towupper(c);
    }

    // Очистка для Плейфера
    wstring cleanPlayfair(const wstring& s, bool russian) {
        const wstring& alphabet = russian ? RUS_ALPH : ENG_ALPH;
        wstring res;
        for (wchar_t c : s) {
            c = normalize(c);
            if (russian && c == L'Ё') c = L'Е';
            if (!russian && c == L'J') c = L'I';
            if (alphabet.find(c) != wstring::npos) res += c;
        }
        return res;
    }

    // Подготовка текста для Плейфера
    wstring preparePlayfair(const wstring& raw, bool russian) {
        wstring t = cleanPlayfair(raw, russian);
        if (t.empty()) return L"";
        wchar_t filler = russian ? L'Ъ' : L'X';
        wstring prep;
        for (size_t i = 0; i < t.length(); ++i) {
            prep += t[i];
            if (i + 1 < t.length() && t[i] == t[i + 1]) prep += filler;
        }
        if (prep.length() % 2) prep += filler;
        return prep;
    }

    // Преобразование биграмм Плейфера
    wstring transformPlayfair(const wstring& text,
                              bool russian,
                              const vector<vector<wchar_t>>& matrix,
                              const map<wchar_t, pair<int,int>>& pos,
                              bool decrypt) {
        if (text.empty()) return L"";
        int rows = russian ? 4 : 5;
        int cols = russian ? 8 : 5;
        wstring result;
        for (size_t i = 0; i < text.length(); i += 2) {
            wchar_t a = text[i], b = text[i+1];
            auto itA = pos.find(a), itB = pos.find(b);
            if (itA == pos.end() || itB == pos.end())
                throw runtime_error("Символ не найден в матрице Плейфера");

            int r1 = itA->second.first, c1 = itA->second.second;
            int r2 = itB->second.first, c2 = itB->second.second;
            wchar_t cA, cB;

            if (r1 == r2) {
                int shift = decrypt ? (cols - 1) : 1;
                cA = matrix[r1][(c1 + shift) % cols];
                cB = matrix[r2][(c2 + shift) % cols];
            } else if (c1 == c2) {
                int shift = decrypt ? (rows - 1) : 1;
                cA = matrix[(r1 + shift) % rows][c1];
                cB = matrix[(r2 + shift) % rows][c2];
            } else {
                cA = matrix[r1][c2];
                cB = matrix[r2][c1];
            }
            result += cA;
            result += cB;
        }
        return result;
    }
}

// ================== Нигилистов ==================
void nihilist_buildTable(const wstring& keyword,
                         wchar_t table[N_SIZE][N_SIZE],
                         map<wchar_t, pair<int,int>>& charToPos) {
    wstring used;
    for (wchar_t ch : keyword) {
        ch = normalize(ch);
        if (NIHIL_ALPHABET.find(ch) != wstring::npos && used.find(ch) == wstring::npos)
            used += ch;
    }
    for (wchar_t ch : NIHIL_ALPHABET)
        if (used.find(ch) == wstring::npos) used += ch;

    int idx = 0;
    for (int i = 0; i < N_SIZE; i++)
        for (int j = 0; j < N_SIZE; j++) {
            wchar_t c = used[idx++];
            table[i][j] = c;
            charToPos[c] = {i+1, j+1}; // координаты 1..6
        }
}

static int getNihilCoord(wchar_t c, const map<wchar_t, pair<int,int>>& pos) {
    c = normalize(c);
    auto it = pos.find(c);
    if (it == pos.end()) throw runtime_error("Символ не найден в таблице Нигилистов");
    return it->second.first * 10 + it->second.second;
}

static wchar_t nihilCoordToChar(int coord, const wchar_t table[N_SIZE][N_SIZE]) {
    int r = coord / 10 - 1;
    int c = coord % 10 - 1;
    if (r < 0 || r >= N_SIZE || c < 0 || c >= N_SIZE)
        throw runtime_error("Неверная координата Нигилистов");
    return table[r][c];
}

vector<int> nihilist_encrypt(const wstring& text,
                             const wstring& gammaKey,
                             const wchar_t table[N_SIZE][N_SIZE],
                             const map<wchar_t, pair<int,int>>& pos) {
    wstring gamma;
    while (gamma.size() < text.size()) gamma += gammaKey;
    gamma.resize(text.size());
    vector<int> res;
    for (size_t i = 0; i < text.size(); ++i) {
        int p = getNihilCoord(text[i], pos);
        int g = getNihilCoord(gamma[i], pos);
        res.push_back(p + g);
    }
    return res;
}

wstring nihilist_decrypt(const vector<int>& ciphertext,
                         const wstring& gammaKey,
                         const wchar_t table[N_SIZE][N_SIZE],
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring gamma;
    while (gamma.size() < ciphertext.size()) gamma += gammaKey;
    gamma.resize(ciphertext.size());
    wstring res;
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        int g = getNihilCoord(gamma[i], pos);
        int p = ciphertext[i] - g;
        if (p < 11 || p > 66) throw runtime_error("Ошибка расшифрования Нигилистов");
        res += nihilCoordToChar(p, table);
    }
    return res;
}

// ================== Плейфера ==================
void playfair_buildMatrix(const wstring& keyword,
                          bool russian,
                          vector<vector<wchar_t>>& matrix,
                          map<wchar_t, pair<int,int>>& charPos) {
    const wstring& alphabet = russian ? RUS_ALPH : ENG_ALPH;
    int rows = russian ? 4 : 5;
    int cols = russian ? 8 : 5;

    wstring used;
    for (wchar_t c : keyword) {
        c = normalize(c);
        if (russian && c == L'Ё') c = L'Е';
        if (!russian && c == L'J') c = L'I';
        if (alphabet.find(c) != wstring::npos && used.find(c) == wstring::npos)
            used += c;
    }
    for (wchar_t c : alphabet)
        if (used.find(c) == wstring::npos) used += c;

    matrix.assign(rows, vector<wchar_t>(cols));
    charPos.clear();
    int idx = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            wchar_t c = used[idx++];
            matrix[i][j] = c;
            charPos[c] = {i, j};
        }
}

wstring playfair_encrypt(const wstring& plaintext,
                         bool russian,
                         const vector<vector<wchar_t>>& matrix,
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring prep = preparePlayfair(plaintext, russian);
    return transformPlayfair(prep, russian, matrix, pos, false);
}

wstring playfair_decrypt(const wstring& ciphertext,
                         bool russian,
                         const vector<vector<wchar_t>>& matrix,
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring clean = cleanPlayfair(ciphertext, russian);
    if (clean.empty()) return L"";
    if (clean.length() % 2) throw runtime_error("Шифротекст Плейфера должен быть чётной длины");
    return transformPlayfair(clean, russian, matrix, pos, true);
}