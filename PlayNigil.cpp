#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <sstream>
#include <algorithm>

using namespace std;

// ---------- Общие вспомогательные функции ----------
char normalize(char c) {
    c = toupper(c);
    if (c == 'J') c = 'I';
    return c;
}

string cleanText(const string& input) {
    string res;
    for (char c : input)
        if (isalpha(c)) res += normalize(c);
    return res;
}

// ================== НИГИЛИСТОВ ==================
void nihilist_buildTable(const string& keyword, char table[5][5],
                         map<char, pair<int,int>>& charToPos) {
    string used = "", alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ";
    for (char ch : keyword) {
        ch = normalize(ch);
        if (ch >= 'A' && ch <= 'Z' && used.find(ch) == string::npos)
            used += ch;
    }
    int idx = 0;
    for (char ch : used) {
        int r = idx/5, c = idx%5;
        table[r][c] = ch;
        charToPos[ch] = {r+1, c+1};
        idx++;
    }
    for (char ch : alphabet) {
        if (used.find(ch) == string::npos && idx < 25) {
            int r = idx/5, c = idx%5;
            table[r][c] = ch;
            charToPos[ch] = {r+1, c+1};
            idx++;
        }
    }
}

int nihilist_getCoord(char c, const map<char, pair<int,int>>& charToPos) {
    c = normalize(c);
    auto it = charToPos.find(c);
    if (it == charToPos.end()) throw runtime_error("Символ отсутствует в таблице");
    auto [r, col] = it->second;
    return r*10 + col;
}

char nihilist_coordToChar(int coord, const char table[5][5]) {
    int r = coord/10 - 1, c = coord%10 - 1;
    if (r<0 || r>4 || c<0 || c>4) throw runtime_error("Неверная координата");
    return table[r][c];
}

vector<int> nihilist_encrypt(const string& text, const string& gammaKey,
                             const char table[5][5], const map<char,pair<int,int>>& pos) {
    string gamma;
    while (gamma.size() < text.size()) gamma += gammaKey;
    gamma.resize(text.size());
    vector<int> res;
    for (size_t i=0; i<text.size(); ++i) {
        int p = nihilist_getCoord(text[i], pos);
        int g = nihilist_getCoord(gamma[i], pos);
        res.push_back(p + g);
    }
    return res;
}

string nihilist_decrypt(const vector<int>& enc, const string& gammaKey,
                        const char table[5][5], const map<char,pair<int,int>>& pos) {
    string gamma;
    while (gamma.size() < enc.size()) gamma += gammaKey;
    gamma.resize(enc.size());
    string res;
    for (size_t i=0; i<enc.size(); ++i) {
        int g = nihilist_getCoord(gamma[i], pos);
        int p = enc[i] - g;
        if (p<11 || p>55) throw runtime_error("Ошибка расшифрования");
        res += nihilist_coordToChar(p, table);
    }
    return res;
}

// ================== ПЛЕЙФЕРА ==================
void playfair_buildMatrix(const string& keyword, char m[5][5],
                          map<char, pair<int,int>>& pos) {
    string used = "", alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ";
    for (char c : keyword) {
        c = normalize(c);
        if (c>='A' && c<='Z' && used.find(c)==string::npos) used += c;
    }
    for (char c : alphabet)
        if (used.find(c)==string::npos) used += c;
    for (int i=0; i<5; ++i)
        for (int j=0; j<5; ++j) {
            char c = used[i*5+j];
            m[i][j] = c;
            pos[c] = {i, j};
        }
}

string playfair_prepare(const string& raw) {
    string t = cleanText(raw);
    if (t.empty()) return "";
    string prep;
    for (size_t i=0; i<t.length(); ++i) {
        prep += t[i];
        if (i+1 < t.length() && t[i]==t[i+1]) prep += 'X';
    }
    if (prep.length()%2) prep += 'X';
    return prep;
}

string playfair_encryptDigraph(char a, char b, const char m[5][5],
                               const map<char,pair<int,int>>& pos) {
    auto [r1,c1] = pos.at(a);
    auto [r2,c2] = pos.at(b);
    if (r1 == r2)      return { m[r1][(c1+1)%5], m[r2][(c2+1)%5] };
    else if (c1 == c2) return { m[(r1+1)%5][c1], m[(r2+1)%5][c2] };
    else               return { m[r1][c2], m[r2][c1] };
}

string playfair_decryptDigraph(char a, char b, const char m[5][5],
                               const map<char,pair<int,int>>& pos) {
    auto [r1,c1] = pos.at(a);
    auto [r2,c2] = pos.at(b);
    if (r1 == r2)      return { m[r1][(c1+4)%5], m[r2][(c2+4)%5] };
    else if (c1 == c2) return { m[(r1+4)%5][c1], m[(r2+4)%5][c2] };
    else               return { m[r1][c2], m[r2][c1] };
}

string playfair_encrypt(const string& text, const char m[5][5],
                        const map<char,pair<int,int>>& pos) {
    string prep = playfair_prepare(text);
    string res;
    for (size_t i=0; i<prep.length(); i+=2)
        res += playfair_encryptDigraph(prep[i], prep[i+1], m, pos);
    return res;
}

string playfair_decrypt(const string& text, const char m[5][5],
                        const map<char,pair<int,int>>& pos) {
    string clean = cleanText(text);
    if (clean.length()%2) throw runtime_error("Длина шифротекста должна быть чётной");
    string res;
    for (size_t i=0; i<clean.length(); i+=2)
        res += playfair_decryptDigraph(clean[i], clean[i+1], m, pos);
    return res;
}

// ================== ГЛАВНОЕ МЕНЮ ==================
int main() {
    setlocale(LC_ALL, "Russian");
    int cipher = 0;
    while (cipher != 3) {
        cout << "\n=== ВЫБОР ШИФРА ===\n1 - Нигилистов\n2 - Плейфера\n3 - Выход\nВаш выбор: ";
        cin >> cipher; cin.ignore();
        if (cipher == 3) { cout << "До свидания!\n"; break; }
        if (cipher != 1 && cipher != 2) { cout << "Неверный выбор\n"; continue; }

        if (cipher == 1) { // Нигилистов
            string tableKey, gammaKey;
            cout << "Ключ таблицы: "; getline(cin, tableKey);
            cout << "Гамма-ключ: ";   getline(cin, gammaKey);
            if (tableKey.empty() || gammaKey.empty()) {
                cout << "Ключи не могут быть пустыми\n"; continue;
            }
            char table[5][5];
            map<char,pair<int,int>> pos;
            nihilist_buildTable(tableKey, table, pos);

            int op = 0;
            while (op != 3) {
                cout << "\n[Нигилистов] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                cin >> op; cin.ignore();
                try {
                    if (op == 1) {
                        cout << "Текст: "; string s; getline(cin, s);
                        s = cleanText(s);
                        if (s.empty()) { cout << "Нет букв\n"; continue; }
                        auto enc = nihilist_encrypt(s, gammaKey, table, pos);
                        cout << "Шифротекст: ";
                        for (int x : enc) cout << x << ' ';
                        cout << '\n';
                    } else if (op == 2) {
                        cout << "Числа через пробел: "; string line; getline(cin, line);
                        istringstream iss(line);
                        vector<int> v; int x;
                        while (iss >> x) v.push_back(x);
                        if (v.empty()) { cout << "Нет чисел\n"; continue; }
                        cout << "Расшифровка: " << nihilist_decrypt(v, gammaKey, table, pos) << '\n';
                    }
                } catch (const exception& e) { cout << "Ошибка: " << e.what() << '\n'; }
            }
        } else { // Плейфера
            string key;
            cout << "Ключ: "; getline(cin, key);
            if (key.empty()) { cout << "Ключ пуст\n"; continue; }
            char m[5][5];
            map<char,pair<int,int>> pos;
            playfair_buildMatrix(key, m, pos);
            cout << "Матрица:\n";
            for (auto & row : m) {
                for (char c : row) cout << c << ' ';
                cout << '\n';
            }
            int op = 0;
            while (op != 3) {
                cout << "\n[Плейфера] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                cin >> op; cin.ignore();
                try {
                    if (op == 1) {
                        cout << "Текст: "; string s; getline(cin, s);
                        cout << "Шифротекст: " << playfair_encrypt(s, m, pos) << '\n';
                    } else if (op == 2) {
                        cout << "Шифротекст: "; string s; getline(cin, s);
                        cout << "Расшифровка: " << playfair_decrypt(s, m, pos) << '\n';
                    }
                } catch (const exception& e) { cout << "Ошибка: " << e.what() << '\n'; }
            }
        }
    }
    return 0;
}