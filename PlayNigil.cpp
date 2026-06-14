#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cwctype>
#include <sstream>
#include <algorithm>
#include <locale>

using namespace std;

// ---------- Шифр Нигилистов (русский, 6×6) ----------
const wstring ALPHABET_6X6 = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ .,";
const int SIZE6 = 6;

wchar_t normalize6(wchar_t c) {
    return towupper(c);
}

wstring clean6(const wstring& s) {
    wstring r;
    for (wchar_t c : s) {
        c = normalize6(c);
        if (ALPHABET_6X6.find(c) != wstring::npos) r += c;
    }
    return r;
}

void nihilist_buildTable(const wstring& key, wchar_t table[SIZE6][SIZE6],
                         map<wchar_t, pair<int,int>>& pos) {
    wstring used;
    for (wchar_t c : key) {
        c = normalize6(c);
        if (ALPHABET_6X6.find(c) != wstring::npos && used.find(c) == wstring::npos)
            used += c;
    }
    for (wchar_t c : ALPHABET_6X6)
        if (used.find(c) == wstring::npos) used += c;
    int idx = 0;
    for (int i = 0; i < SIZE6; i++)
        for (int j = 0; j < SIZE6; j++) {
            wchar_t c = used[idx++];
            table[i][j] = c;
            pos[c] = {i+1, j+1};
        }
}

int nihilist_getCoord(wchar_t c, const map<wchar_t, pair<int,int>>& pos) {
    c = normalize6(c);
    auto it = pos.find(c);
    if (it == pos.end()) throw runtime_error("Символ не найден");
    auto [r, col] = it->second;
    return r*10 + col;
}

wchar_t nihilist_coordToChar(int coord, const wchar_t table[SIZE6][SIZE6]) {
    int r = coord/10 - 1, c = coord%10 - 1;
    if (r<0 || r>=SIZE6 || c<0 || c>=SIZE6) throw runtime_error("Неверная координата");
    return table[r][c];
}

vector<int> nihilist_encrypt(const wstring& text, const wstring& gamma,
                             const wchar_t table[SIZE6][SIZE6],
                             const map<wchar_t, pair<int,int>>& pos) {
    wstring g = gamma;
    while (g.size() < text.size()) g += gamma;
    g.resize(text.size());
    vector<int> res;
    for (size_t i = 0; i < text.size(); ++i) {
        int p = nihilist_getCoord(text[i], pos);
        int k = nihilist_getCoord(g[i], pos);
        res.push_back(p + k);
    }
    return res;
}

wstring nihilist_decrypt(const vector<int>& enc, const wstring& gamma,
                         const wchar_t table[SIZE6][SIZE6],
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring g = gamma;
    while (g.size() < enc.size()) g += gamma;
    g.resize(enc.size());
    wstring res;
    for (size_t i = 0; i < enc.size(); ++i) {
        int k = nihilist_getCoord(g[i], pos);
        int p = enc[i] - k;
        if (p < 11 || p > 66) throw runtime_error("Ошибка расшифрования");
        res += nihilist_coordToChar(p, table);
    }
    return res;
}

// ---------- Шифр Плейфера (русский 8×4 / английский 5×5) ----------
wstring playfair_clean(const wstring& s, bool russian) {
    wstring alphabet = russian ? L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                               : L"ABCDEFGHIKLMNOPQRSTUVWXYZ";
    wstring res;
    for (wchar_t c : s) {
        c = towupper(c);
        if (russian && c == L'Ё') c = L'Е';      // Ё -> Е
        if (!russian && c == L'J') c = L'I';    // J -> I
        if (alphabet.find(c) != wstring::npos) res += c;
    }
    return res;
}

void playfair_buildMatrix(const wstring& key, bool russian,
                          vector<vector<wchar_t>>& matrix,
                          map<wchar_t, pair<int,int>>& pos) {
    wstring alphabet = russian ? L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                               : L"ABCDEFGHIKLMNOPQRSTUVWXYZ";
    int rows = russian ? 8 : 5;
    int cols = russian ? 4 : 5;
    wstring used;
    wstring cleanKey;
    for (wchar_t c : key) {
        c = towupper(c);
        if (russian && c == L'Ё') c = L'Е';
        if (!russian && c == L'J') c = L'I';
        if (alphabet.find(c) != wstring::npos && used.find(c) == wstring::npos)
            used += c;
    }
    for (wchar_t c : alphabet)
        if (used.find(c) == wstring::npos) used += c;

    matrix.assign(rows, vector<wchar_t>(cols));
    pos.clear();
    for (int i = 0, idx = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            wchar_t c = used[idx++];
            matrix[i][j] = c;
            pos[c] = {i, j};
        }
}

wstring playfair_prepare(const wstring& raw, bool russian) {
    wstring t = playfair_clean(raw, russian);
    if (t.empty()) return L"";
    wchar_t filler = russian ? L'Ъ' : L'X';
    wstring prep;
    for (size_t i = 0; i < t.length(); i++) {
        prep += t[i];
        if (i+1 < t.length() && t[i] == t[i+1])
            prep += filler;
    }
    if (prep.length() % 2) prep += filler;
    return prep;
}

wstring playfair_transform(const wstring& text, bool russian,
                           const vector<vector<wchar_t>>& matrix,
                           const map<wchar_t, pair<int,int>>& pos,
                           bool decrypt) {
    int rows = russian ? 8 : 5;
    int cols = russian ? 4 : 5;
    wstring result;
    for (size_t i = 0; i < text.length(); i += 2) {
        wchar_t a = text[i], b = text[i+1];
        auto [r1, c1] = pos.at(a);
        auto [r2, c2] = pos.at(b);
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

wstring playfair_encrypt(const wstring& plain, bool russian,
                         const vector<vector<wchar_t>>& matrix,
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring prep = playfair_prepare(plain, russian);
    return playfair_transform(prep, russian, matrix, pos, false);
}

wstring playfair_decrypt(const wstring& cipher, bool russian,
                         const vector<vector<wchar_t>>& matrix,
                         const map<wchar_t, pair<int,int>>& pos) {
    wstring clean = playfair_clean(cipher, russian);
    if (clean.length() % 2) throw runtime_error("Длина шифротекста должна быть чётной");
    return playfair_transform(clean, russian, matrix, pos, true);
}

// ========== Главное меню ==========
int main() {
    locale::global(locale("ru_RU.UTF-8"));
    wcin.imbue(locale());
    wcout.imbue(locale());

    int cipher = 0;
    while (cipher != 3) {
        wcout << L"\n=== ВЫБОР ШИФРА ===\n1 - Нигилистов\n2 - Плейфера\n3 - Выход\nВаш выбор: ";
        cin >> cipher; cin.ignore();
        if (cipher == 3) { wcout << L"До свидания!\n"; break; }
        if (cipher != 1 && cipher != 2) { wcout << L"Неверный выбор\n"; continue; }

        if (cipher == 1) {
            // Нигилистов (русский 6×6)
            wstring tableKey, gammaKey;
            wcout << L"Ключ таблицы: "; getline(wcin, tableKey);
            wcout << L"Гамма-ключ: ";   getline(wcin, gammaKey);
            tableKey = clean6(tableKey);
            gammaKey = clean6(gammaKey);
            if (tableKey.empty() || gammaKey.empty()) {
                wcout << L"Ключи должны содержать символы алфавита\n"; continue;
            }
            wchar_t table[SIZE6][SIZE6];
            map<wchar_t, pair<int,int>> pos;
            nihilist_buildTable(tableKey, table, pos);

            int op = 0;
            while (op != 3) {
                wcout << L"\n[Нигилистов] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                cin >> op; cin.ignore();
                try {
                    if (op == 1) {
                        wcout << L"Текст: "; wstring s; getline(wcin, s);
                        s = clean6(s);
                        if (s.empty()) { wcout << L"Нет допустимых символов\n"; continue; }
                        auto enc = nihilist_encrypt(s, gammaKey, table, pos);
                        wcout << L"Шифротекст: ";
                        for (int x : enc) wcout << x << L' ';
                        wcout << L'\n';
                    } else if (op == 2) {
                        wcout << L"Числа через пробел: ";
                        string line; getline(cin, line);
                        istringstream iss(line);
                        vector<int> v; int x;
                        while (iss >> x) v.push_back(x);
                        if (v.empty()) { wcout << L"Нет чисел\n"; continue; }
                        wcout << L"Расшифровка: " << nihilist_decrypt(v, gammaKey, table, pos) << L'\n';
                    }
                } catch (const exception& e) {
                    wcout << L"Ошибка: " << e.what() << L'\n';
                }
            }
        } else {
            // Плейфера – выбор языка
            int lang = 0;
            while (lang != 1 && lang != 2) {
                wcout << L"\nВыберите язык:\n1 - Русский (8×4)\n2 - Английский (5×5)\nВаш выбор: ";
                cin >> lang; cin.ignore();
            }
            bool russian = (lang == 1);

            wstring key;
            wcout << L"Ключ: "; getline(wcin, key);
            key = playfair_clean(key, russian);
            if (key.empty()) { wcout << L"Ключ не содержит допустимых символов\n"; continue; }

            vector<vector<wchar_t>> matrix;
            map<wchar_t, pair<int,int>> pos;
            playfair_buildMatrix(key, russian, matrix, pos);

            // Вывод матрицы
            wcout << L"Матрица:\n";
            for (const auto& row : matrix) {
                for (wchar_t c : row) wcout << c << L' ';
                wcout << L'\n';
            }

            int op = 0;
            while (op != 3) {
                wcout << L"\n[Плейфера] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                cin >> op; cin.ignore();
                try {
                    if (op == 1) {
                        wcout << L"Текст: "; wstring s; getline(wcin, s);
                        wcout << L"Шифротекст: " << playfair_encrypt(s, russian, matrix, pos) << L'\n';
                    } else if (op == 2) {
                        wcout << L"Шифротекст: "; wstring s; getline(wcin, s);
                        wcout << L"Расшифровка: " << playfair_decrypt(s, russian, matrix, pos) << L'\n';
                    }
                } catch (const exception& e) {
                    wcout << L"Ошибка: " << e.what() << L'\n';
                }
            }
        }
    }
    return 0;
}