#ifndef AES_128_GCM
#define AES_128_GCM

    #include <stdint.h>

    #define HARDWARE_SPEED // make sure to compile with flags like: -O3 -maes -msse4 -mpclmul -march=native

    // for aes 128
    #define aes_128_key_bits 128
    #define aes_128_key_bytes 16
    #define aes_128_W_len 44
    #define aes_128_Nb 4
    #define aes_128_Nr 10
    #define aes_128_Nk 4

    void print_hex(unsigned char* a, int b);

    // used for detached decryption
    int s_aes_128_gcm_decrypt(uint8_t* plaintext, uint8_t* ad, int ad_len, uint8_t* ciphertext, int ciphertext_length, uint8_t tag[16], uint8_t key[aes_128_key_bytes], uint8_t iv[12]);
    // used for detached encryption
    void s_aes_128_gcm_encypt(uint8_t* ciphertext, uint8_t tag[16], uint8_t* plaintext, int plaintext_length, uint8_t* ad, int ad_len, uint8_t key[aes_128_key_bytes], uint8_t iv[12]);

    // helper functions for combined encryption
    void s_aes128_gcm_encrypt_combind(uint8_t* ciphertext, uint8_t* plaintext, int plaintext_length, uint8_t* ad, int ad_len, uint8_t key[aes_128_key_bytes], uint8_t iv[12]);
    int  s_aes_128_gcm_decrypt_combind(uint8_t* plaintext, uint8_t* ad, int ad_len, uint8_t* ciphertext, int ciphertext_length, uint8_t key[aes_128_key_bytes], uint8_t iv[12]);

#endif