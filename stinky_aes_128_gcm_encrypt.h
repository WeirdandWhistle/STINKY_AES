#ifndef AES_128_GCM_STINKY
#define AES_128_GCM_STINKY
    #include <stdint.h>
    // for aes 128
    #define Nb 4
    #define Nr 10
    void temp_AES_GCM(uint8_t* ciphertext, uint8_t* plaintext, int plaintext_length, uint8_t* ad, int ad_len, uint8_t key[16], uint8_t iv[12]);

#endif