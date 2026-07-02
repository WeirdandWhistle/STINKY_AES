#include "aes_128_gcm.h"
#include <sodium.h>
#include <assert.h>

void from_hex(uint8_t* out, char* in, int len){
    size_t outlen;
    sodium_hex2bin(out, len/2, in, len, NULL, &outlen, NULL);
}
void print_hex(unsigned char* a, int aSize){
    for(int i = 0; i<aSize;i++){
        printf("%02X",a[i]);
    }
    printf("\n");
}
int main(){

    uint8_t key[16] = {0};
    from_hex(key, "000102030405060708090a0b0c0d0e0f",32);
    uint8_t iv[12] = {0};
    from_hex(iv, "abcdef12345678901f2f3f4f",24);
    uint8_t ad[25] = {};
    from_hex(ad, "abcd1028743610923784610275861307580834765028374506", sizeof(ad)*2);
    uint8_t plaintext[25] = {0};
    from_hex(plaintext, "abcd1028743610923784610275861307580834765028374506",sizeof(plaintext)*2);

    uint8_t ciphertext[sizeof plaintext] = {0};
    uint8_t tag[16] = {0};

    // s_aes_128_gcm_encypt(ciphertext, tag, plaintext, sizeof plaintext, ad, sizeof ad, key, iv);

    // printf("ciphertext : "); print_hex(ciphertext, sizeof ciphertext);
    // printf("tag:         "); print_hex(tag, sizeof tag);

    // uint8_t temp[sizeof plaintext + 16] = {0};
    // s_aes128_gcm_encrypt_combind(temp, plaintext, sizeof plaintext, ad, sizeof ad, key, iv);

    // printf("combind mode:"); print_hex(temp, sizeof temp);

    // uint8_t temp2[sizeof plaintext] = {0};
    // int rc = s_aes_128_gcm_decrypt_combind(temp2, ad, sizeof ad, temp, sizeof temp, key, iv);
    // print_hex(temp2, sizeof temp2);
    // assert(rc==0);

    // uint8_t temp3[sizeof plaintext] = {0};
    // // int rc = -1;
    // // assert(s_aes_128_gcm_decrypt(temp3, ad, sizeof ad, ciphertext, sizeof ciphertext, tag, key, iv)==0);
    // rc = s_aes_128_gcm_decrypt(temp3, ad, sizeof ad, ciphertext, sizeof ciphertext, tag, key, iv);

    // printf("plaintext, rc=%d\nabcd1028743610923784610275861307580834765028374506\n",rc); print_hex(temp3, sizeof plaintext);

    // bench marking 
    if(1){
        int len = 2 * 1000 * 1000 * 1000;
        // int len = 8448;
        uint8_t* p = malloc(len); 
        uint8_t* c = malloc(len);
        s_aes_128_gcm_encypt(c, tag, p, len, NULL, 0, key, iv); 
    }

    return 0;
}