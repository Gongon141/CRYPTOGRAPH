#ifndef AFFINE_INTERFACE_H
#define AFFINE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

int affine_check_key(int a, int b);
char* affine_encrypt(const char* plaintext, int a, int b);
char* affine_decrypt(const char* ciphertext, int a, int b);
int affine_encrypt_file(const char* input_path, const char* output_path, int a, int b);
int affine_decrypt_file(const char* input_path, const char* output_path, int a, int b);

#ifdef __cplusplus
}
#endif
