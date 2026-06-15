#include <iostream>
#include <locale>
#include <sstream>
#include <limits>
#include "ciphers.h"

using namespace std;

// Очистка текста для Нигилистов (только символы его алфавита)
wstring cleanNihilText(const wstring& s) {
    const wstring NIHIL_ALPH = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ .,";
    wstring r;
    for (wchar_t c : s) {
        c = towupper(c);
        if (NIHIL_ALPH.find(c) != wstring::npos) r += c;
    }
    return r;
}

int main() {
    // Настройка локали для русского языка
    locale::global(locale("ru_RU.UTF-8"));
    wcin.imbue(locale());
    wcout.imbue(locale());

    int cipher = 0;
    while (cipher != 3) {
        wcout << L"\n=== ВЫБОР ШИФРА ===\n"
              << L"1 — Нигилистов (русский, 6×6)\n"
              << L"2 — Плейфера\n"
              << L"3 — Выход\n"
              << L"Ваш выбор: ";
        wcin >> cipher;
        wcin.ignore(numeric_limits<streamsize>::max(), L'\n');

        if (cipher == 3) { wcout << L"До свидания!\n"; break; }
        if (cipher != 1 && cipher != 2) { wcout << L"Неверный выбор\n"; continue; }

        if (cipher == 1) {
            // ---------- Нигилистов ----------
            wstring tableKey, gammaKey;
            wcout << L"Ключ таблицы: "; getline(wcin, tableKey);
            wcout << L"Гамма-ключ: ";   getline(wcin, gammaKey);
            tableKey = cleanNihilText(tableKey);
            gammaKey = cleanNihilText(gammaKey);
            if (tableKey.empty() || gammaKey.empty()) {
                wcout << L"Ключи должны содержать символы русского алфавита (включая пробел,.,)\n";
                continue;
            }

            wchar_t table[6][6];
            map<wchar_t, pair<int,int>> pos;
            nihilist_buildTable(tableKey, table, pos);

            int op = 0;
            while (op != 3) {
                wcout << L"\n[Нигилистов] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                wcin >> op;
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
                try {
                    if (op == 1) {
                        wcout << L"Текст: "; wstring s; getline(wcin, s);
                        s = cleanNihilText(s);
                        if (s.empty()) { wcout << L"Нет допустимых символов\n"; continue; }
                        auto enc = nihilist_encrypt(s, gammaKey, table, pos);
                        wcout << L"Шифротекст (числа): ";
                        for (int x : enc) wcout << x << L' ';
                        wcout << L'\n';
                    } else if (op == 2) {
                        wcout << L"Числа через пробел: ";
                        wstring line; getline(wcin, line);
                        wistringstream wiss(line);
                        vector<int> v; int x;
                        while (wiss >> x) v.push_back(x);
                        if (v.empty()) { wcout << L"Нет чисел\n"; continue; }
                        wstring dec = nihilist_decrypt(v, gammaKey, table, pos);
                        wcout << L"Расшифровка: " << dec << L'\n';
                    }
                } catch (const exception& e) {
                    wcout << L"Ошибка: " << e.what() << L'\n';
                }
            }
        } else {
            // ---------- Плейфера ----------
            int lang = 0;
            while (lang != 1 && lang != 2) {
                wcout << L"\nВыберите язык:\n1 — Русский (8×4)\n2 — Английский (5×5)\nВаш выбор: ";
                wcin >> lang;
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
            }
            bool russian = (lang == 1);

            wcout << L"Ключ: ";
            wstring keyRaw; getline(wcin, keyRaw);
            // Очистка ключа по алфавиту
            const wstring& alphabet = russian ? L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                                             : L"ABCDEFGHIKLMNOPQRSTUVWXYZ";
            wstring key;
            for (wchar_t c : keyRaw) {
                c = towupper(c);
                if (russian && c == L'Ё') c = L'Е';
                if (!russian && c == L'J') c = L'I';
                if (alphabet.find(c) != wstring::npos) key += c;
            }
            if (key.empty()) { wcout << L"Ключ не содержит допустимых символов\n"; continue; }

            vector<vector<wchar_t>> matrix;
            map<wchar_t, pair<int,int>> pos;
            playfair_buildMatrix(key, russian, matrix, pos);

            wcout << L"Матрица:\n";
            for (const auto& row : matrix) {
                for (wchar_t c : row) wcout << c << L' ';
                wcout << L'\n';
            }

            int op = 0;
            while (op != 3) {
                wcout << L"\n[Плейфера] 1-Зашифровать 2-Расшифровать 3-Назад: ";
                wcin >> op;
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
                try {
                    if (op == 1) {
                        wcout << L"Текст: "; wstring s; getline(wcin, s);
                        if (s.empty()) { wcout << L"Пустой ввод\n"; continue; }
                        wstring enc = playfair_encrypt(s, russian, matrix, pos);
                        if (enc.empty()) wcout << L"Нет допустимых символов\n";
                        else wcout << L"Шифротекст: " << enc << L'\n';
                    } else if (op == 2) {
                        wcout << L"Шифротекст: "; wstring s; getline(wcin, s);
                        if (s.empty()) { wcout << L"Пустой ввод\n"; continue; }
                        wstring dec = playfair_decrypt(s, russian, matrix, pos);
                        if (dec.empty()) wcout << L"Некорректный шифротекст\n";
                        else wcout << L"Расшифровка: " << dec << L'\n';
                    }
                } catch (const exception& e) {
                    wcout << L"Ошибка: " << e.what() << L'\n';
                }
            }
        }
    }
    return 0;
}