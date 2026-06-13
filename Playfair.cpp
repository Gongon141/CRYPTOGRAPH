#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>

using namespace std;

// Приведение буквы: заглавная, J -> I
char normalize(char c) {
    c = toupper(c);
    if (c == 'J') c = 'I';
    return c;
}

// Очистка текста: только буквы (J->I), остальное удаляется
string cleanText(const string& input) {
    string result;
    for (char c : input) {
        if (isalpha(c)) result += normalize(c);
    }
    return result;
}

// Построение матрицы Плейфера 5x5 по ключу
// Возвращает матрицу и карту символ -> (строка, столбец)
void buildPlayfairMatrix(const string& keyword,
                         char matrix[5][5],
                         map<char, pair<int,int>>& charPos) {
    string used = "";
    // Порядок алфавита (без J)
    string alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ";

    // Добавляем уникальные буквы ключа
    for (char c : keyword) {
        c = normalize(c);
        if (c >= 'A' && c <= 'Z' && used.find(c) == string::npos) {
            used += c;
        }
    }

    // Дополняем оставшимися буквами алфавита
    for (char c : alphabet) {
        if (used.find(c) == string::npos) {
            used += c;
        }
    }

    // Заполняем матрицу 5x5
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            char c = used[i * 5 + j];
            matrix[i][j] = c;
            charPos[c] = {i, j};
        }
    }
}

// Подготовка открытого текста: разбивка на пары с обработкой дубликатов и нечётной длины
string preparePlaintext(const string& raw) {
    string text = cleanText(raw);
    if (text.empty()) return "";

    string prepared;
    for (size_t i = 0; i < text.length(); i++) {
        prepared += text[i];
        // Если следующий символ такой же, вставляем 'X'
        if (i + 1 < text.length() && text[i] == text[i + 1]) {
            prepared += 'X';
        }
    }
    // Если длина нечётная, добавляем 'X' в конец
    if (prepared.length() % 2 != 0) {
        prepared += 'X';
    }
    return prepared;
}

// Шифрование биграммы
string encryptDigraph(char a, char b,
                      const char matrix[5][5],
                      const map<char, pair<int,int>>& charPos) {
    auto [rowA, colA] = charPos.at(a);
    auto [rowB, colB] = charPos.at(b);
    char c1, c2;

    if (rowA == rowB) {
        // Одна строка: сдвиг вправо циклически
        c1 = matrix[rowA][(colA + 1) % 5];
        c2 = matrix[rowB][(colB + 1) % 5];
    } else if (colA == colB) {
        // Один столбец: сдвиг вниз
        c1 = matrix[(rowA + 1) % 5][colA];
        c2 = matrix[(rowB + 1) % 5][colB];
    } else {
        // Прямоугольник: углы
        c1 = matrix[rowA][colB];
        c2 = matrix[rowB][colA];
    }
    return {c1, c2};
}

// Дешифрование биграммы (правила обратные)
string decryptDigraph(char a, char b,
                      const char matrix[5][5],
                      const map<char, pair<int,int>>& charPos) {
    auto [rowA, colA] = charPos.at(a);
    auto [rowB, colB] = charPos.at(b);
    char c1, c2;

    if (rowA == rowB) {
        // Одна строка: сдвиг влево циклически
        c1 = matrix[rowA][(colA + 4) % 5]; // +4 ≡ -1 mod 5
        c2 = matrix[rowB][(colB + 4) % 5];
    } else if (colA == colB) {
        // Один столбец: сдвиг вверх
        c1 = matrix[(rowA + 4) % 5][colA];
        c2 = matrix[(rowB + 4) % 5][colB];
    } else {
        // Прямоугольник: углы
        c1 = matrix[rowA][colB];
        c2 = matrix[rowB][colA];
    }
    return {c1, c2};
}

// Полное шифрование текста
string playfairEncrypt(const string& plaintext,
                       const char matrix[5][5],
                       const map<char, pair<int,int>>& charPos) {
    string prepared = preparePlaintext(plaintext);
    string ciphertext;
    for (size_t i = 0; i < prepared.length(); i += 2) {
        ciphertext += encryptDigraph(prepared[i], prepared[i+1], matrix, charPos);
    }
    return ciphertext;
}

// Полное дешифрование
string playfairDecrypt(const string& ciphertext,
                       const char matrix[5][5],
                       const map<char, pair<int,int>>& charPos) {
    string plaintext;
    for (size_t i = 0; i < ciphertext.length(); i += 2) {
        plaintext += decryptDigraph(ciphertext[i], ciphertext[i+1], matrix, charPos);
    }
    return plaintext;
}

int main() {
    setlocale(LC_ALL, "Russian");
    string keyword;
    cout << "Шифр Плейфера\n";
    cout << "Введите ключ (латиница): ";
    getline(cin, keyword);

    char matrix[5][5];
    map<char, pair<int,int>> charPos;
    buildPlayfairMatrix(keyword, matrix, charPos);

    // Показать матрицу (опционально)
    cout << "Матрица 5x5:\n";
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            cout << matrix[i][j] << ' ';
        }
        cout << '\n';
    }

    int choice = 0;
    while (choice != 3) {
        cout << "\nВыберите действие:\n"
             << "1 - Зашифровать\n"
             << "2 - Расшифровать\n"
             << "3 - Выход\n"
             << "Ваш выбор: ";
        cin >> choice;
        cin.ignore();

        try {
            if (choice == 1) {
                cout << "Введите открытый текст: ";
                string input;
                getline(cin, input);
                string encrypted = playfairEncrypt(input, matrix, charPos);
                cout << "Шифротекст: " << encrypted << endl;
            }
            else if (choice == 2) {
                cout << "Введите шифротекст (только буквы): ";
                string input;
                getline(cin, input);
                string clean = cleanText(input);
                if (clean.length() % 2 != 0) {
                    cout << "Ошибка: длина шифротекста должна быть чётной." << endl;
                    continue;
                }
                string decrypted = playfairDecrypt(clean, matrix, charPos);
                cout << "Расшифрованный текст: " << decrypted << endl;
            }
            else if (choice == 3) {
                cout << "До свидания!" << endl;
            }
            else {
                cout << "Некорректный выбор." << endl;
            }
        } catch (const exception& ex) {
            cout << "Ошибка: " << ex.what() << endl;
        }
    }
    return 0;
}