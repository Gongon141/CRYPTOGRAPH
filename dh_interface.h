#ifndef DH_INTERFACE_H
#define DH_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

void dh_generate_keys(int* priv_key, int* pub_key, int* p, int* g);
int dh_encrypt_file(const char* input_path, const char* output_path, int priv_key, int pub_other, int p);
int dh_decrypt_file(const char* input_path, const char* output_path, int priv_key, int pub_other, int p);
char* dh_encrypt_string(const char* plaintext, int priv_key, int pub_other, int p);
char* dh_decrypt_string(const char* ciphertext, int priv_key, int pub_other, int p);

#ifdef __cplusplus
}
#endif

#endif
