#include "aes_128_gcm.h"
#include <sodium.h>
#include <assert.h>

void from_hex(uint8_t* out, char* in, int len){
    size_t outlen;
    sodium_hex2bin(out, len/2, in, len, NULL, &outlen, NULL);
}
// void print_hex(unsigned char* a, int aSize){
//     for(int i = 0; i<aSize;i++){
//         printf("%02X",a[i]);
//     }
//     printf("\n");
// }
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

    s_aes_128_gcm_encypt(ciphertext, tag, plaintext, sizeof plaintext, ad, sizeof ad, key, iv);

    printf("ciphertext: "); print_hex(ciphertext, sizeof ciphertext);
    printf("tag:        "); print_hex(tag, sizeof tag);

    uint8_t temp[sizeof plaintext] = {0};
    int rc = -1;
    assert(s_aes_128_gcm_decrypt(temp, ad, sizeof ad, ciphertext, sizeof ciphertext, tag, key, iv)==0);
    // int rc = s_aes_128_gcm_decrypt(temp, ad, sizeof ad, ciphertext, sizeof ciphertext, tag, key, iv);

    printf("plaintext, rc=%d\nabcd1028743610923784610275861307580834765028374506\n",rc); print_hex(temp, sizeof plaintext);

    // bench marking
    if(0){
        int len = 1 * 1000 * 1000 * 1000;
        uint8_t* p = malloc(len); 
        uint8_t* c = malloc(len);
        s_aes_128_gcm_encypt(c, tag, p, len, NULL, 0, key, iv); 
    }

    return 0;
}