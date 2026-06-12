#include <iostream>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <fstream>

using namespace std;

typedef int (*AffineCheckKeyFunc)(int, int);
typedef char* (*AffineEncryptFunc)(const char*, int, int);
typedef char* (*AffineDecryptFunc)(const char*, int, int);
typedef int (*AffineEncryptFileFunc)(const char*, const char*, int, int);
typedef int (*AffineDecryptFileFunc)(const char*, const char*, int, int);

typedef void (*DHGenerateKeysFunc)(int*, int*, int*, int*);
typedef int (*DHEncryptFileFunc)(const char*, const char*, int, int, int);
typedef int (*DHDecryptFileFunc)(const char*, const char*, int, int, int);
typedef char* (*DHEncryptStringFunc)(const char*, int, int, int);
typedef char* (*DHDecryptStringFunc)(const char*, int, int, int);

int main() {
    void* affine_lib = nullptr;
    void* dh_lib = nullptr;
    
    AffineCheckKeyFunc affine_check_key = nullptr;
    AffineEncryptFunc affine_encrypt = nullptr;
    AffineDecryptFunc affine_decrypt = nullptr;
    AffineEncryptFileFunc affine_encrypt_file = nullptr;
    AffineDecryptFileFunc affine_decrypt_file = nullptr;
    
    DHGenerateKeysFunc dh_generate_keys = nullptr;
    DHEncryptFileFunc dh_encrypt_file = nullptr;
    DHDecryptFileFunc dh_decrypt_file = nullptr;
    DHEncryptStringFunc dh_encrypt_string = nullptr;
    DHDecryptStringFunc dh_decrypt_string = nullptr;
    
    int choice;
    
    cout << "===== CryptoLibTester =====" << endl;
    cout << "1. Аффинный шифр" << endl;
    cout << "2. Шифр Диффи-Хэллмана" << endl;
    cout << "3. Выход" << endl;
    cout << "Выберите алгоритм: ";
    cin >> choice;
    cin.ignore();
    
    if (choice == 1) {
        affine_lib = dlopen("./libaffine.so", RTLD_LAZY);
        if (!affine_lib) {
            cerr << "Ошибка загрузки libaffine.so: " << dlerror() << endl;
            return 1;
        }
        
        affine_check_key = (AffineCheckKeyFunc)dlsym(affine_lib, "affine_check_key");
        affine_encrypt = (AffineEncryptFunc)dlsym(affine_lib, "affine_encrypt");
        affine_decrypt = (AffineDecryptFunc)dlsym(affine_lib, "affine_decrypt");
        affine_encrypt_file = (AffineEncryptFileFunc)dlsym(affine_lib, "affine_encrypt_file");
        affine_decrypt_file = (AffineDecryptFileFunc)dlsym(affine_lib, "affine_decrypt_file");
        
        int mode, a, b;
        string input_file, output_file, text;
        char* result;
        
        cout << "1. Шифровать строку" << endl;
        cout << "2. Дешифровать строку" << endl;
        cout << "3. Шифровать файл" << endl;
        cout << "4. Дешифровать файл" << endl;
        cout << "Выбор: ";
        cin >> mode;
        cin.ignore();
        
        cout << "Введите a (множитель, gcd(a,95)=1): ";
        cin >> a;
        cout << "Введите b (сдвиг): ";
        cin >> b;
        cin.ignore();
        
        if (!affine_check_key(a, b)) {
            cerr << "Ошибка: НОД(a,95) != 1. Шифр необратим." << endl;
            dlclose(affine_lib);
            return 1;
        }
        
        if (mode == 1) {
            cout << "Введите строку: ";
            getline(cin, text);
            result = affine_encrypt(text.c_str(), a, b);
            cout << "Зашифровано: " << result << endl;
            free(result);
        } else if (mode == 2) {
            cout << "Введите строку: ";
            getline(cin, text);
            result = affine_decrypt(text.c_str(), a, b);
            cout << "Расшифровано: " << result << endl;
            free(result);
        } else if (mode == 3) {
            cout << "Входной файл: ";
            getline(cin, input_file);
            cout << "Выходной файл: ";
            getline(cin, output_file);
            int ret = affine_encrypt_file(input_file.c_str(), output_file.c_str(), a, b);
            if (ret == 0) cout << "Файл успешно зашифрован" << endl;
            else if (ret == -2) cerr << "Ошибка: не удалось открыть входной файл" << endl;
            else if (ret == -3) cerr << "Ошибка: не удалось создать выходной файл" << endl;
            else cerr << "Ошибка шифрования" << endl;
        } else if (mode == 4) {
            cout << "Входной файл: ";
            getline(cin, input_file);
            cout << "Выходной файл: ";
            getline(cin, output_file);
            int ret = affine_decrypt_file(input_file.c_str(), output_file.c_str(), a, b);
            if (ret == 0) cout << "Файл успешно расшифрован" << endl;
            else if (ret == -2) cerr << "Ошибка: не удалось открыть входной файл" << endl;
            else if (ret == -3) cerr << "Ошибка: не удалось создать выходной файл" << endl;
            else cerr << "Ошибка дешифрования" << endl;
        }
        
        dlclose(affine_lib);
    }
    else if (choice == 2) {
        dh_lib = dlopen("./libdh.so", RTLD_LAZY);
        if (!dh_lib) {
            cerr << "Ошибка загрузки libdh.so: " << dlerror() << endl;
            return 1;
        }
        
        dh_generate_keys = (DHGenerateKeysFunc)dlsym(dh_lib, "dh_generate_keys");
        dh_encrypt_file = (DHEncryptFileFunc)dlsym(dh_lib, "dh_encrypt_file");
        dh_decrypt_file = (DHDecryptFileFunc)dlsym(dh_lib, "dh_decrypt_file");
        dh_encrypt_string = (DHEncryptStringFunc)dlsym(dh_lib, "dh_encrypt_string");
        dh_decrypt_string = (DHDecryptStringFunc)dlsym(dh_lib, "dh_decrypt_string");
        
        int mode, priv1, pub1, priv2, pub2, p, g;
        string input_file, output_file, text;
        char* result;
        
        cout << "1. Шифровать строку" << endl;
        cout << "2. Дешифровать строку" << endl;
        cout << "3. Шифровать файл" << endl;
        cout << "4. Дешифровать файл" << endl;
        cout << "Выбор: ";
        cin >> mode;
        cin.ignore();
        
        dh_generate_keys(&priv1, &pub1, &p, &g);
        dh_generate_keys(&priv2, &pub2, &p, &g);
        
        cout << "\n=== Параметры Диффи-Хэллмана ===" << endl;
        cout << "p (простое число) = " << p << endl;
        cout << "g (первообразный корень) = " << g << endl;
        cout << "Сторона A: закрытый ключ = " << priv1 << ", открытый = " << pub1 << endl;
        cout << "Сторона B: закрытый ключ = " << priv2 << ", открытый = " << pub2 << endl;
        cout << "Общий секрет используется как ключ XOR" << endl;
        cout << "===================================\n" << endl;
        
        if (mode == 1) {
            cout << "Введите строку: ";
            getline(cin, text);
            result = dh_encrypt_string(text.c_str(), priv1, pub2, p);
            cout << "Зашифровано (HEX): ";
            for (int i = 0; i < text.length(); i++) {
                printf("%02X ", (unsigned char)result[i]);
            }
            cout << endl;
            free(result);
        } else if (mode == 2) {
            cout << "Введите зашифрованную строку: ";
            getline(cin, text);
            result = dh_decrypt_string(text.c_str(), priv1, pub2, p);
            cout << "Расшифровано: " << result << endl;
            free(result);
        } else if (mode == 3) {
            cout << "Входной файл: ";
            getline(cin, input_file);
            cout << "Выходной файл: ";
            getline(cin, output_file);
            int ret = dh_encrypt_file(input_file.c_str(), output_file.c_str(), priv1, pub2, p);
            if (ret == 0) cout << "Файл успешно зашифрован" << endl;
            else if (ret == -1) cerr << "Ошибка: не удалось открыть входной файл" << endl;
            else if (ret == -2) cerr << "Ошибка: не удалось создать выходной файл" << endl;
        } else if (mode == 4) {
            cout << "Входной файл: ";
            getline(cin, input_file);
            cout << "Выходной файл: ";
            getline(cin, output_file);
            int ret = dh_decrypt_file(input_file.c_str(), output_file.c_str(), priv1, pub2, p);
            if (ret == 0) cout << "Файл успешно расшифрован" << endl;
            else if (ret == -1) cerr << "Ошибка: не удалось открыть входной файл" << endl;
            else if (ret == -2) cerr << "Ошибка: не удалось создать выходной файл" << endl;
        }
        
        dlclose(dh_lib);
    }
    else {
        cout << "Выход." << endl;
    }
    
    return 0;
}
