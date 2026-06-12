#include <cstdlib>
#include <cstring>
#include <fstream>

extern "C" {

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int affine_check_key(int a, int b) {
    return (gcd(a, 95) == 1) ? 1 : 0;
}

int mod_inverse(int a, int m) {
    a = a % m;
    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) return x;
    }
    return -1;
}

char encrypt_char(char ch, int a, int b) {
    int x = (int)ch;
    if (x >= 32 && x <= 126) {
        x = x - 32;
        int y = (a * x + b) % 95;
        return (char)(y + 32);
    }
    return ch;
}

char decrypt_char(char ch, int a, int b) {
    int y = (int)ch;
    if (y >= 32 && y <= 126) {
        y = y - 32;
        int a_inv = mod_inverse(a, 95);
        if (a_inv == -1) return ch;
        int x = (a_inv * (y - b + 95)) % 95;
        return (char)(x + 32);
    }
    return ch;
}

char* affine_encrypt(const char* plaintext, int a, int b) {
    if (!affine_check_key(a, b)) return nullptr;
    int len = strlen(plaintext);
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = encrypt_char(plaintext[i], a, b);
    }
    result[len] = '\0';
    return result;
}

char* affine_decrypt(const char* ciphertext, int a, int b) {
    if (!affine_check_key(a, b)) return nullptr;
    int len = strlen(ciphertext);
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = decrypt_char(ciphertext[i], a, b);
    }
    result[len] = '\0';
    return result;
}

int affine_encrypt_file(const char* input_path, const char* output_path, int a, int b) {
    if (!affine_check_key(a, b)) return -1;
    std::ifstream infile(input_path);
    if (!infile.is_open()) return -2;
    std::ofstream outfile(output_path);
    if (!outfile.is_open()) return -3;
    char ch;
    while (infile.get(ch)) {
        outfile.put(encrypt_char(ch, a, b));
    }
    return 0;
}

int affine_decrypt_file(const char* input_path, const char* output_path, int a, int b) {
    if (!affine_check_key(a, b)) return -1;
    std::ifstream infile(input_path);
    if (!infile.is_open()) return -2;
    std::ofstream outfile(output_path);
    if (!outfile.is_open()) return -3;
    char ch;
    while (infile.get(ch)) {
        outfile.put(decrypt_char(ch, a, b));
    }
    return 0;
}

}
