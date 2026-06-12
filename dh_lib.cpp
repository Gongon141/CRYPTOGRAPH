#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ctime>
#include <cmath>

extern "C" {

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int generate_prime() {
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    int p;
    do {
        p = rand() % 401 + 100;
    } while (!is_prime(p));
    return p;
}

int find_primitive_root(int p) {
    for (int g = 2; g < p; g++) {
        bool is_root = true;
        int result = 1;
        for (int i = 1; i < p - 1; i++) {
            result = (result * g) % p;
            if (result == 1) {
                is_root = false;
                break;
            }
        }
        if (is_root) return g;
    }
    return 2;
}

int mod_pow(int base, int exp, int mod) {
    int result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

void dh_generate_keys(int* priv_key, int* pub_key, int* p, int* g) {
    *p = generate_prime();
    *g = find_primitive_root(*p);
    *priv_key = rand() % (*p - 2) + 1;
    *pub_key = mod_pow(*g, *priv_key, *p);
}

int dh_encrypt_file(const char* input_path, const char* output_path, int priv_key, int pub_other, int p) {
    int shared_secret = mod_pow(pub_other, priv_key, p);
    int key_byte = shared_secret % 256;
    
    std::ifstream infile(input_path, std::ios::binary);
    if (!infile.is_open()) return -1;
    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile.is_open()) return -2;
    
    char ch;
    while (infile.get(ch)) {
        char encrypted = ch ^ key_byte;
        outfile.put(encrypted);
    }
    
    return 0;
}

int dh_decrypt_file(const char* input_path, const char* output_path, int priv_key, int pub_other, int p) {
    return dh_encrypt_file(input_path, output_path, priv_key, pub_other, p);
}

char* dh_encrypt_string(const char* plaintext, int priv_key, int pub_other, int p) {
    int shared_secret = mod_pow(pub_other, priv_key, p);
    int key_byte = shared_secret % 256;
    
    int len = strlen(plaintext);
    char* result = (char*)malloc(len + 1);
    
    for (int i = 0; i < len; i++) {
        result[i] = plaintext[i] ^ key_byte;
    }
    result[len] = '\0';
    return result;
}

char* dh_decrypt_string(const char* ciphertext, int priv_key, int pub_other, int p) {
    return dh_encrypt_string(ciphertext, priv_key, pub_other, p);
}

}
