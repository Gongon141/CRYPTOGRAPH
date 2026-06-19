#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <dlfcn.h>

#include "crypto_api.h"

using namespace std;

const string RESET = "\033[0m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string RED = "\033[31m";
const string BOLD = "\033[1m";

vector<uint8_t> readAll(string path) {
    vector<uint8_t> buf;
    if (path.empty() || path == "-") {
        char c;
        while (cin.get(c)) buf.push_back(c);
    } else {
        ifstream f(path, ios::binary);
        if (!f) { 
            cerr << RED << "Ошибка: не удалось открыть файл '" << path << "'" << RESET << endl; 
            exit(1); 
        }
        char c;
        while (f.get(c)) buf.push_back(c);
    }
    return buf;
}

void writeAll(string path, vector<uint8_t>& buf) {
    if (path.empty() || path == "-") {
        cout.write((char*)buf.data(), buf.size());
        cout.flush();
    } else {
        ofstream f(path, ios::binary);
        if (!f) { 
            cerr << RED << "Ошибка: не удалось создать файл '" << path << "'" << RESET << endl; 
            exit(1); 
        }
        f.write((char*)buf.data(), buf.size());
    }
}

void genKey(size_t size, string path) {
    vector<uint8_t> key(size);
    ifstream r("/dev/urandom", ios::binary);
    if (!r) {
        cerr << RED << "Ошибка: не удалось открыть /dev/urandom" << RESET << endl;
        exit(1);
    }
    r.read((char*)key.data(), size);
    writeAll(path, key);
}

void* loadLib(string algo) {
    string lib = "./lib" + algo + ".so";
    void* h = dlopen(lib.c_str(), RTLD_LAZY);
    if (!h) { 
        cerr << RED << "Ошибка: не удалось загрузить библиотеку " << lib << RESET << endl; 
        exit(1); 
    }
    return h;
}

const AlgorithmInfo* getMeta(void* h) {
    auto fnInfo = (const AlgorithmInfo*(*)())dlsym(h, "get_algorithm_info");
    if (!fnInfo) { 
        cerr << RED << "Ошибка: библиотека не экспортирует get_algorithm_info" << RESET << endl; 
        exit(1); 
    }
    return fnInfo();
}

string ask(const string& prompt) {
    cerr << prompt;
    string line;
    getline(cin, line);
    return line;
}

int main() {

    cerr << BOLD << "Выберите алгоритм шифрования:" << RESET << endl;
    cerr << "  1. Цезарь (быстрый, ключ 1 байт)" << endl;
    cerr << "  2. Виженер (надежнее, ключ 16 байт)" << endl;
    string algoChoice = ask("Ваш выбор [1-2]: ");
    
    string algo;
    if (algoChoice == "1") algo = "Cesar";
    else if (algoChoice == "2") algo = "Vigenere";
    else {
        cerr << YELLOW << "Неверный выбор, используется Цезарь" << RESET << endl;
        algo = "Cesar";
    }

    cerr << endl;
    cerr << BOLD << "Выберите режим работы:" << RESET << endl;
    cerr << "  1. Шифрование" << endl;
    cerr << "  2. Расшифрование" << endl;
    cerr << "  3. Генерация ключа" << endl;
    string modeChoice = ask("Ваш выбор [1-3]: ");
    
    string mode;
    if (modeChoice == "1") mode = "encrypt";
    else if (modeChoice == "2") mode = "decrypt";
    else if (modeChoice == "3") mode = "generate-key";
    else {
        cerr << RED << "Неверный выбор" << RESET << endl;
        return 1;
    }

    void* h = loadLib(algo);
    const AlgorithmInfo* meta = getMeta(h);

    if (mode == "generate-key") {
        cerr << endl;
        string keyPath = ask("Введите путь для сохранения ключа: ");
        if (keyPath.empty()) keyPath = "key.bin";
        
        genKey(meta->key_size, keyPath);
        
        cerr << BOLD << GREEN << "✓" << RESET << " Ключ сгенерирован и сохранен в: " << keyPath << endl;
        cerr << "  Размер ключа: " << meta->key_size << " байт" << endl;
        cerr << endl;
        
        dlclose(h);
        return 0;
    }

    cerr << endl;
    cerr << BOLD << "Выберите источник данных:" << RESET << endl;
    cerr << "  1. Ввод с консоли" << endl;
    cerr << "  2. Текстовый файл" << endl;
    cerr << "  3. Медиафайл" << endl;
    string sourceChoice = ask("Ваш выбор [1-3]: ");
    
    string inPath;
    vector<uint8_t> consoleData;
    bool fromConsole = false;
    
    if (sourceChoice == "1") {
        fromConsole = true;
        string text = ask("Введите текст для " + string(mode == "encrypt" ? "шифрования" : "расшифрования") + ": ");
        consoleData.assign(text.begin(), text.end());
    } else if (sourceChoice == "2") {
        inPath = ask("Введите путь к текстовому файлу: ");
    } else if (sourceChoice == "3") {
        inPath = ask("Введите путь к медиафайлу: ");
    } else {
        cerr << RED << "Неверный выбор" << RESET << endl;
        dlclose(h);
        return 1;
    }

    cerr << endl;
    cerr << BOLD << "Выберите действие с ключом:" << RESET << endl;
    cerr << "  1. Использовать существующий ключ" << endl;
    cerr << "  2. Сгенерировать новый ключ" << endl;
    string keyAction = ask("Ваш выбор [1-2]: ");
    
    vector<uint8_t> keyData;
    string keyPath;
    
    if (keyAction == "1") {
        keyPath = ask("Введите путь к файлу ключа: ");
        keyData = readAll(keyPath);
        if (keyData.size() < meta->key_size) {
            cerr << RED << "Ошибка: ключ слишком короткий" << RESET << endl;
            cerr << "  Ожидается: " << meta->key_size << " байт, получено: " << keyData.size() << endl;
            dlclose(h);
            return 1;
        }
        keyData.resize(meta->key_size);
    } else if (keyAction == "2") {
        keyPath = ask("Введите путь для сохранения нового ключа: ");
        if (keyPath.empty()) keyPath = "key.bin";
        
        genKey(meta->key_size, keyPath);
        keyData = readAll(keyPath);
        
        cerr << BOLD << GREEN << "✓" << RESET << " Новый ключ сгенерирован: " << keyPath << endl;
    } else {
        cerr << RED << "Неверный выбор" << RESET << endl;
        dlclose(h);
        return 1;
    }

    cerr << endl;
    string outPath = ask("Введите путь для сохранения результата: ");
    if (outPath.empty()) outPath = "output.bin";

    vector<uint8_t> input;
    if (fromConsole) {
        input = consoleData;
    } else {
        input = readAll(inPath);
    }
    
    if (input.empty()) {
        cerr << YELLOW << "Предупреждение: входные данные пусты" << RESET << endl;
    }

    vector<uint8_t> output(input.size());
    ConstBuffer kb{keyData.data(), keyData.size()};
    ConstBuffer ib{input.data(), input.size()};
    MutBuffer ob{output.data(), output.size()};

    auto fnEnc = (int(*)(ConstBuffer, ConstBuffer, MutBuffer*))dlsym(h, "encrypt");
    auto fnDec = (int(*)(ConstBuffer, ConstBuffer, MutBuffer*))dlsym(h, "decrypt");

    int rc = 0;
    if (mode == "encrypt") {
        cerr << endl << BOLD << BLUE << "ℹ" << RESET << " Шифрование данных..." << endl;
        rc = fnEnc(kb, ib, &ob);
    } else if (mode == "decrypt") {
        cerr << endl << BOLD << BLUE << "ℹ" << RESET << " Расшифрование данных..." << endl;
        rc = fnDec(kb, ib, &ob);
    }

    if (rc != 0) { 
        cerr << RED << "Ошибка криптографической операции (код " << rc << ")" << RESET << endl; 
        dlclose(h);
        return 1; 
    }

    output.resize(ob.size);
    writeAll(outPath, output);
    
    cerr << BOLD << GREEN << "✓" << RESET << " Операция завершена успешно" << endl;
    cerr << "  Результат сохранен в: " << outPath << endl;
    cerr << "  Ключ: " << keyPath << endl;
    cerr << endl;
    
    dlclose(h);
    return 0;
}