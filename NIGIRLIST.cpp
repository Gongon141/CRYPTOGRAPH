#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <sstream>
#include <algorithm>

using namespace std;

// Приведение буквы: заглавная, J -> I
char normalize(char c) {
    c = toupper(c);
    if (c == 'J') c = 'I';
    return c;
}

// Построение таблицы 5x5 и карты символов
void buildTable(const string& keyword,
                char table[5][5],
                map<char, pair<int,int>>& charToPos) {
    string used = "";
    string alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ"; // без J

    // Уникальные буквы ключа
    for (char ch : keyword) {
        ch = normalize(ch);
        if (ch >= 'A' && ch <= 'Z' && used.find(ch) == string::npos) {
            used += ch;
        }
    }

    int idx = 0;
    for (char ch : used) {
        if (idx >= 25) break;
        int row = idx / 5;
        int col = idx % 5;
        table[row][col] = ch;
        charToPos[ch] = {row + 1, col + 1};
        idx++;
    }

    for (char ch : alphabet) {
        if (used.find(ch) == string::npos) {
            if (idx >= 25) break;
            int row = idx / 5;
            int col = idx % 5;
            table[row][col] = ch;
            charToPos[ch] = {row + 1, col + 1};
            idx++;
        }
    }
}

// Получить двузначное число координат
int getCoord(char c, const map<char, pair<int,int>>& charToPos) {
    c = normalize(c);
    auto it = charToPos.find(c);
    if (it == charToPos.end()) {
        throw runtime_error(string("Символ '") + c + "' отсутствует в таблице");
    }
    auto [row, col] = it->second;
    return row * 10 + col;
}

// Восстановить символ по координатам
char coordToChar(int coord, const char table[5][5]) {
    int row = coord / 10 - 1;
    int col = coord % 10 - 1;
    if (row < 0 || row > 4 || col < 0 || col > 4) {
        throw runtime_error("Некорректная координата: " + to_string(coord));
    }
    return table[row][col];
}

// Очистка открытого текста: только буквы, без пробелов
string cleanText(const string& input) {
    string result;
    for (char c : input) {
        if (isalpha(c)) result += normalize(c);
    }
    return result;
}

// Шифрование
vector<int> encrypt(const string& plaintext,
                   const string& tableKey,
                   const string& gammaKey,
                   const char table[5][5],
                   const map<char, pair<int,int>>& charToPos) {
    string gamma;
    if (gammaKey.empty()) throw runtime_error("Гамма-ключ не может быть пустым");
    while (gamma.length() < plaintext.length()) {
        gamma += gammaKey;
    }
    gamma = gamma.substr(0, plaintext.length());

    vector<int> result;
    for (size_t i = 0; i < plaintext.length(); ++i) {
        int pCoord = getCoord(plaintext[i], charToPos);
        int gCoord = getCoord(gamma[i], charToPos);
        result.push_back(pCoord + gCoord);
    }
    return result;
}

// Дешифрование
string decrypt(const vector<int>& ciphertext,
              const string& tableKey,
              const string& gammaKey,
              const char table[5][5],
              const map<char, pair<int,int>>& charToPos) {
    string gamma;
    while (gamma.length() < ciphertext.size()) {
        gamma += gammaKey;
    }
    gamma = gamma.substr(0, ciphertext.size());

    string plaintext;
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        int sum = ciphertext[i];
        int gCoord = getCoord(gamma[i], charToPos);
        int pCoord = sum - gCoord;
        if (pCoord < 11 || pCoord > 55) {
            throw runtime_error("Ошибка расшифрования: неверная сумма на позиции " + to_string(i));
        }
        plaintext += coordToChar(pCoord, table);
    }
    return plaintext;
}

int main() {
    setlocale(LC_ALL, "Russian"); // для корректного отображения кириллицы в консоли (Windows)
    string tableKey, gammaKey;
    cout << "Введите ключ для таблицы (латинские буквы): ";
    getline(cin, tableKey);
    cout << "Введите гамма-ключ (латинские буквы): ";
    getline(cin, gammaKey);

    // Построим таблицу один раз для обеих операций
    char table[5][5];
    map<char, pair<int,int>> charToPos;
    buildTable(tableKey, table, charToPos);

    int choice = 0;
    while (choice != 3) {
        cout << "\nВыберите действие:\n"
             << "1 - Зашифровать текст\n"
             << "2 - Расшифровать текст\n"
             << "3 - Выход\n"
             << "Ваш выбор: ";
        cin >> choice;
        cin.ignore(); // убрать символ новой строки после числа

        try {
            if (choice == 1) {
                cout << "Введите открытый текст (латиница, пробелы будут удалены): ";
                string input;
                getline(cin, input);
                string clean = cleanText(input);
                if (clean.empty()) {
                    cout << "Текст не содержит букв!" << endl;
                    continue;
                }
                vector<int> encrypted = encrypt(clean, tableKey, gammaKey, table, charToPos);
                cout << "Шифротекст (числа через пробел): ";
                for (int num : encrypted) cout << num << " ";
                cout << endl;
            }
            else if (choice == 2) {
                cout << "Введите шифротекст (числа через пробел): ";
                string line;
                getline(cin, line);
                istringstream iss(line);
                vector<int> ciphertext;
                int num;
                while (iss >> num) ciphertext.push_back(num);
                if (ciphertext.empty()) {
                    cout << "Не введено ни одного числа!" << endl;
                    continue;
                }
                string decrypted = decrypt(ciphertext, tableKey, gammaKey, table, charToPos);
                cout << "Расшифрованный текст: " << decrypted << endl;
            }
            else if (choice == 3) {
                cout << "До свидания!" << endl;
            }
            else {
                cout << "Некорректный выбор. Попробуйте снова." << endl;
            }
        } catch (const exception& ex) {
            cout << "Ошибка: " << ex.what() << endl;
        }
    }
    return 0;
}