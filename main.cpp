#include <iostream>
#include <string>
#include <vector>
#include "playfair(rus).h"
#include "playfair(eng).h"
using namespace std;

// Пункты общего меню (выбор языка)
enum class Language {
    Exit = 0,
    Russian = 1,
    English = 2
};

// Меню для русского шифра
void runRussianMenu() {
    string matrix[4][8];
    bool keySet = false;

    while (true) {
        cout << "\n===== Русский шифр Плейфера =====\n";
        cout << "1. Задать ключ\n";
        cout << "2. Зашифровать сообщение\n";
        cout << "3. Расшифровать сообщение\n";
        cout << "0. Назад (к выбору языка)\n";
        cout << "Выберите действие: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                cout << "Введите ключ (слово или фразу): ";
                string keyInput;
                getline(cin, keyInput);
                buildMatrixRu(keyInput, matrix);
                keySet = true;

                cout << "\nКлючевая матрица 4x8:\n";
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 8; c++) {
                        cout << matrix[r][c] << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 2:
            case 3: {
                if (!keySet) {
                    cout << "Ошибка: сначала задайте ключ (пункт 1).\n";
                    break;
                }

                cout << "Введите сообщение: ";
                string message;
                getline(cin, message);

                vector<string> prepared;
                if (choice == 2)
                    prepared = prepareForEncryptionRu(message);
                else
                    prepared = prepareForDecryptionRu(message);

                if (prepared.empty()) {
                    cout << "Сообщение не содержит русских букв.\n";
                    break;
                }

                string result;
                size_t total = prepared.size();
                size_t i = 0;
                for (; i + 1 < total; i += 2) {
                    if (choice == 2)
                        result += encryptBigramRu(prepared[i], prepared[i + 1], matrix);
                    else
                        result += decryptBigramRu(prepared[i], prepared[i + 1], matrix);
                }
                if (i < total)
                    result += prepared[i];

                cout << (choice == 2 ? "Зашифрованное" : "Расшифрованное")
                     << " сообщение:\n" << result << endl;
                break;
            }
            default:
                cout << "Неверный пункт меню.\n";
        }
    }
}

// Меню для английского шифра
void runEnglishMenu() {
    string matrix[5][5];
    bool keySet = false;

    while (true) {
        cout << "\n===== English Playfair Cipher =====\n";
        cout << "1. Set key\n";
        cout << "2. Encrypt message\n";
        cout << "3. Decrypt message\n";
        cout << "0. Back (to language selection)\n";
        cout << "Choose an action: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                cout << "Enter the key (word or phrase): ";
                string keyInput;
                getline(cin, keyInput);
                buildMatrixEn(keyInput, matrix);
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
            case 2:
            case 3: {
                if (!keySet) {
                    cout << "Error: set the key first (option 1).\n";
                    break;
                }

                cout << "Enter the message: ";
                string message;
                getline(cin, message);

                vector<char> prepared;
                if (choice == 2)
                    prepared = prepareForEncryptionEn(message);
                else
                    prepared = prepareForDecryptionEn(message);

                if (prepared.empty()) {
                    cout << "Message contains no English letters.\n";
                    break;
                }

                string result;
                size_t total = prepared.size();
                size_t i = 0;
                for (; i + 1 < total; i += 2) {
                    if (choice == 2)
                        result += encryptBigramEn(prepared[i], prepared[i + 1], matrix);
                    else
                        result += decryptBigramEn(prepared[i], prepared[i + 1], matrix);
                }
                if (i < total)
                    result += prepared[i];

                cout << (choice == 2 ? "Encrypted" : "Decrypted")
                     << " message:\n" << result << endl;
                break;
            }
            default:
                cout << "Invalid menu option.\n";
        }
    }
}

int main() {
    while (true) {
        cout << "\n===== Главное меню =====\n";
        cout << "1. Русский шифр Плейфера\n";
        cout << "2. English Playfair Cipher\n";
        cout << "0. Выход\n";
        cout << "Выберите язык (0,1,2): ";

        int langInput;
        cin >> langInput;
        cin.ignore();

        Language lang = static_cast<Language>(langInput);

        switch (lang) {
            case Language::Russian:
                runRussianMenu();
                break;
            case Language::English:
                runEnglishMenu();
                break;
            case Language::Exit:
                return 0;
            default:
                cout << "Неверный выбор. Попробуйте снова.\n";
        }
    }
    return 0;
}