#include <iostream>
#include <string>
#include <vector>
#include "playfair(rus).h"   // русский Плейфер
#include "playfair(eng).h"   // английский Плейфер
#include "nihilist(rus).h"   // русский Нигилист (функции с префиксом nih_)
#include "nihilist(eng).h"   // английский Нигилист
using namespace std;

// ---------- Перечисления для главного меню ----------
enum class CipherType { 
    Exit = 0, 
    Playfair = 1, 
    Nihilist = 2 
};
enum class Language  { 
    Back = 0, 
    Russian = 1, 
    English = 2 
};

// ================================================================
// Меню РУССКОГО шифра Плейфера (матрица 4×8)
// ================================================================
void playfairRussianMenu() {
    string matrix[4][8];   // ключевая матрица
    bool keySet = false;   // флаг, что ключ уже задан

    while (true) {
        cout << "\n===== Русский шифр Плейфера (4x8) =====\n"
             << "1. Задать ключ\n"
             << "2. Зашифровать сообщение\n"
             << "3. Расшифровать сообщение\n"
             << "0. Назад (к выбору языка)\n"
             << "Выберите действие: ";

        int ch;
        cin >> ch;
        cin.ignore();  // убираем '\n' после числа

        if (ch == 0) break;   // возврат в предыдущее меню

        switch (ch) {
            case 1: {   // ---- Задать ключ ----
                cout << "Введите ключ (слово или фразу): ";
                string key;
                getline(cin, key);
                buildMatrixRu(key, matrix);   // строим матрицу
                keySet = true;

                // Выводим матрицу для контроля
                cout << "\nКлючевая матрица 4x8:\n";
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 8; c++) {
                        cout << matrix[r][c] << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 2:   // шифрование
            case 3: { // дешифрование
                if (!keySet) {
                    cout << "Ошибка: сначала задайте ключ (пункт 1).\n";
                    break;
                }

                cout << "Введите сообщение: ";
                string msg;
                getline(cin, msg);

                // Подготовка текста: для шифрования вставляется 'Ъ' между одинаковыми,
                // для дешифрования – только очистка
                vector<string> prep;
                if (ch == 2)
                    prep = prepareForEncryptionRu(msg);
                else
                    prep = prepareForDecryptionRu(msg);

                if (prep.empty()) {
                    cout << "Сообщение не содержит русских букв.\n";
                    break;
                }

                // Обрабатываем биграммы
                string result;
                for (size_t i = 0; i + 1 < prep.size(); i += 2) {
                    if (ch == 2)
                        result += encryptBigramRu(prep[i], prep[i + 1], matrix);
                    else
                        result += decryptBigramRu(prep[i], prep[i + 1], matrix);
                }
                // Непарный последний символ остаётся без изменений
                if (prep.size() % 2 != 0)
                    result += prep.back();

                cout << (ch == 2 ? "Зашифрованное" : "Расшифрованное")
                     << " сообщение:\n" << result << endl;
                break;
            }
            default:
                cout << "Неверный пункт меню.\n";
        }
    }
}

// ================================================================
// Меню АНГЛИЙСКОГО шифра Плейфера (матрица 5×5)
// ================================================================
void playfairEnglishMenu() {
    string matrix[5][5];
    bool keySet = false;

    while (true) {
        cout << "\n===== English Playfair Cipher (5x5) =====\n"
             << "1. Set key\n"
             << "2. Encrypt message\n"
             << "3. Decrypt message\n"
             << "0. Back (to language selection)\n"
             << "Choose an action: ";

        int ch;
        cin >> ch;
        cin.ignore();

        if (ch == 0) break;

        switch (ch) {
            case 1: {   // ---- Set key ----
                cout << "Enter the key (word or phrase): ";
                string key;
                getline(cin, key);
                buildMatrixEn(key, matrix);
                keySet = true;

                cout << "\nKey matrix 5x5:\n";
                for (int r = 0; r < 5; r++) {
                    for (int c = 0; c < 5; c++) {
                        cout << matrix[r][c] << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 2:   // Encrypt
            case 3: { // Decrypt
                if (!keySet) {
                    cout << "Error: set the key first (option 1).\n";
                    break;
                }

                cout << "Enter the message: ";
                string msg;
                getline(cin, msg);

                // Подготовка: при шифровании вставляется 'X' между одинаковыми
                vector<char> prep;
                if (ch == 2)
                    prep = prepareForEncryptionEn(msg);
                else
                    prep = prepareForDecryptionEn(msg);

                if (prep.empty()) {
                    cout << "Message contains no English letters.\n";
                    break;
                }

                string result;
                for (size_t i = 0; i + 1 < prep.size(); i += 2) {
                    if (ch == 2)
                        result += encryptBigramEn(prep[i], prep[i + 1], matrix);
                    else
                        result += decryptBigramEn(prep[i], prep[i + 1], matrix);
                }
                if (prep.size() % 2 != 0)
                    result += prep.back();

                cout << (ch == 2 ? "Encrypted" : "Decrypted")
                     << " message:\n" << result << endl;
                break;
            }
            default:
                cout << "Invalid menu option.\n";
        }
    }
}

// ================================================================
// Меню РУССКОГО шифра Нигилистов (матрица 4×8)
// ================================================================
void nihilistRussianMenu() {
    string matrix[4][8];
    bool keySet = false;
    vector<string> keySeq;   // очищенная последовательность символов ключа

    while (true) {
        cout << "\n===== Русский шифр Нигилистов (4x8) =====\n"
             << "1. Задать ключ\n"
             << "2. Зашифровать сообщение\n"
             << "3. Расшифровать сообщение\n"
             << "0. Назад (к выбору языка)\n"
             << "Выберите действие: ";

        int ch;
        cin >> ch;
        cin.ignore();

        if (ch == 0) break;

        switch (ch) {
            case 1: {   // ---- Задать ключ ----
                cout << "Введите ключ (слово или фразу): ";
                string key;
                getline(cin, key);
                nih_buildMatrixRu(key, matrix);
                keySet = true;

                keySeq = nih_prepareKeyRu(key);   // сохраняем очищенный ключ
                if (keySeq.empty()) {
                    cout << "Ключ не содержит русских букв.\n";
                    keySet = false;
                    break;
                }

                cout << "\nКлючевая матрица 4x8:\n";
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 8; c++) {
                        cout << matrix[r][c] << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 2:   // шифрование
            case 3: { // дешифрование
                if (!keySet) {
                    cout << "Ошибка: сначала задайте ключ (пункт 1).\n";
                    break;
                }

                cout << "Введите сообщение: ";
                string msg;
                getline(cin, msg);

                vector<string> text = nih_prepareTextRu(msg);
                if (text.empty()) {
                    cout << "Сообщение не содержит русских букв.\n";
                    break;
                }

                // Шифруем/дешифруем посимвольно, ключ повторяется циклически
                string result;
                for (size_t i = 0; i < text.size(); i++) {
                    const string& kc = keySeq[i % keySeq.size()];
                    if (ch == 2)
                        result += nih_encryptCharRu(text[i], kc, matrix);
                    else
                        result += nih_decryptCharRu(text[i], kc, matrix);
                }

                cout << (ch == 2 ? "Зашифрованное" : "Расшифрованное")
                     << " сообщение:\n" << result << endl;
                break;
            }
            default:
                cout << "Неверный пункт меню.\n";
        }
    }
}

// ================================================================
// Меню АНГЛИЙСКОГО шифра Нигилистов (матрица 5×5)
// ================================================================
void nihilistEnglishMenu() {
    string matrix[5][5];
    bool keySet = false;
    vector<char> keySeq;

    while (true) {
        cout << "\n===== English Nihilist Cipher (5x5) =====\n"
             << "1. Set key\n"
             << "2. Encrypt message\n"
             << "3. Decrypt message\n"
             << "0. Back (to language selection)\n"
             << "Choose an action: ";

        int ch;
        cin >> ch;
        cin.ignore();

        if (ch == 0) break;

        switch (ch) {
            case 1: {   // ---- Set key ----
                cout << "Enter the key (word or phrase): ";
                string key;
                getline(cin, key);
                nih_buildMatrixEn(key, matrix);
                keySet = true;

                keySeq = nih_prepareKeyEn(key);
                if (keySeq.empty()) {
                    cout << "Key contains no English letters.\n";
                    keySet = false;
                    break;
                }

                cout << "\nKey matrix 5x5:\n";
                for (int r = 0; r < 5; r++) {
                    for (int c = 0; c < 5; c++) {
                        cout << matrix[r][c] << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 2:   // Encrypt
            case 3: { // Decrypt
                if (!keySet) {
                    cout << "Error: set the key first (option 1).\n";
                    break;
                }

                cout << "Enter the message: ";
                string msg;
                getline(cin, msg);

                vector<char> text = nih_prepareTextEn(msg);
                if (text.empty()) {
                    cout << "Message contains no English letters.\n";
                    break;
                }

                string result;
                for (size_t i = 0; i < text.size(); i++) {
                    char kc = keySeq[i % keySeq.size()];
                    if (ch == 2)
                        result += nih_encryptCharEn(text[i], kc, matrix);
                    else
                        result += nih_decryptCharEn(text[i], kc, matrix);
                }

                cout << (ch == 2 ? "Encrypted" : "Decrypted")
                     << " message:\n" << result << endl;
                break;
            }
            default:
                cout << "Invalid menu option.\n";
        }
    }
}

// ================================================================
// ГЛАВНОЕ МЕНЮ – выбор шифра, затем языка
// ================================================================
int main() {
    while (true) {
        cout << "\n===== Главное меню =====\n"
             << "1. Шифр Плейфера\n"
             << "2. Шифр Нигилистов\n"
             << "0. Выход\n"
             << "Выберите шифр: ";

        int c;
        cin >> c;
        cin.ignore();

        if (c == 0) return 0;   // завершение программы

        if (c != 1 && c != 2) {
            cout << "Неверный выбор. Попробуйте снова.\n";
            continue;
        }

        // Внутренний цикл – выбор языка для выбранного шифра
        while (true) {
            cout << "\n--- Выберите язык ---\n"
                 << "1. Русский\n"
                 << "2. English\n"
                 << "0. Назад (к выбору шифра)\n"
                 << "Ваш выбор: ";

            int l;
            cin >> l;
            cin.ignore();

            if (l == 0) break;   // вернуться к выбору шифра

            if (l == 1) {
                if (c == 1) playfairRussianMenu();
                else        nihilistRussianMenu();
            } else if (l == 2) {
                if (c == 1) playfairEnglishMenu();
                else        nihilistEnglishMenu();
            } else {
                cout << "Неверный выбор языка.\n";
            }
        }
    }
    return 0;
}