#include <wmmintrin.h> // Header for Intel/AMD AES-NI intrinsics
#include <emmintrin.h> // Header for SSE2 (__m128i data type)
#include "main.h"
#include <string.h>

#include <stdio.h>
#include <netinet/in.h>
#include <inttypes.h>
#include "gcm_core.h"
#include <sodium.h>
#include <time.h>
#include <math.h>
// #include <stdlib.h>

static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30,
    0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82,
    0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2,
    0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26,
    0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71,
    0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96,
    0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2,
    0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53,
    0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb,
    0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa,
    0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
    0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92,
    0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff,
    0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44,
    0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46,
    0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32,
    0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac,
    0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d,
    0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65,
    0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6,
    0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b,
    0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1,
    0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e,
    0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89,
    0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16};

static const uint8_t invsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf,
    0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, 0x7c, 0xe3,
    0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43,
    0x44, 0xc4, 0xde, 0xe9, 0xcb, 0x54, 0x7b, 0x94, 0x32,
    0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42,
    0xfa, 0xc3, 0x4e, 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9,
    0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1,
    0x25, 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, 0x6c,
    0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15,
    0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, 0x90, 0xd8, 0xab,
    0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05,
    0xb8, 0xb3, 0x45, 0x06, 0xd0, 0x2c, 0x1e, 0x8f, 0xca,
    0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13,
    0x8a, 0x6b, 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc,
    0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2,
    0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, 0x47, 0xf1,
    0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62,
    0x0e, 0xaa, 0x18, 0xbe, 0x1b, 0xfc, 0x56, 0x3e, 0x4b,
    0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78,
    0xcd, 0x5a, 0xf4, 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07,
    0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec,
    0x5f, 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, 0xa0,
    0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb,
    0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2b, 0x04,
    0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63,
    0x55, 0x21, 0x0c, 0x7d};

// Encrypts a single 16-byte block using pre-expanded round keys
__m128i aes128_encrypt_block(uint8_t* plaintext, __m128i* round_keys, uint8_t* ciphertext) {
    // Load the 16-byte plaintext into a 128-bit register (unaligned load)
    __m128i state = _mm_loadu_si128((const __m128i*)plaintext);

    // Round 0: Whiten
    state = _mm_xor_si128(state, round_keys[0]);

    // Rounds 1 through 9
    state = _mm_aesenc_si128(state, round_keys[1]);
    state = _mm_aesenc_si128(state, round_keys[2]);
    state = _mm_aesenc_si128(state, round_keys[3]);
    state = _mm_aesenc_si128(state, round_keys[4]);
    state = _mm_aesenc_si128(state, round_keys[5]);
    state = _mm_aesenc_si128(state, round_keys[6]);
    state = _mm_aesenc_si128(state, round_keys[7]);
    state = _mm_aesenc_si128(state, round_keys[8]);
    state = _mm_aesenc_si128(state, round_keys[9]);

    // Round 10: Last round instruction (skips MixColumns)
    state = _mm_aesenclast_si128(state, round_keys[10]);

    // Store the 128-bit result back into the ciphertext byte array
    _mm_storeu_si128((__m128i*)ciphertext, state);
}

uint32_t Word(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    return (a<<24) | (b<<16) | (c<<8) | (d);
}
void UnWord(uint8_t* a, uint32_t b){
    a[0] = (b >> 24) & 0xFF;
    a[1] = (b >> 16) & 0xFF;
    a[2] = (b >> 8)  & 0xFF;
    a[3] = b         & 0xFF;
}
uint32_t RotWord(uint32_t word){
    return (word << 8) | (word >> 24);
}
uint32_t SubWord(uint32_t word){
    uint8_t temp[4];
    UnWord(temp, word);
    return Word(sbox[temp[0]], sbox[temp[1]], sbox[temp[2]], sbox[temp[3]]);
}
void key_expansion(uint8_t* key, uint32_t* w, int Nk){
    uint8_t rcon[] = {0x01, 0x02, 0x04, 0x08, 0x10,
                      0x20, 0x40, 0x80, 0x1b, 0x36};
    int i = 0;
    
    while(i < Nk){
        w[i] = Word(key[4*i], key[4*i+1], key[4*i+2], key[4*i+3]);
        i += 1;
    }

    uint32_t temp;

    i = Nk;
    while(i < Nb * (Nr+1)){
        temp = w[i-1];
        // printf("temp = %" PRIx32 "\n", temp);
        if(i % Nk == 0){
            temp = SubWord(RotWord(temp));
            temp ^= ((uint32_t)rcon[(i/Nk) - 1] << 24);
        } else if(Nk > 6 && i % Nk == 4){
            temp = SubWord(temp);
        }
        w[i] = w[i-Nk] ^ temp;
        i += 1;
    }
}
void combine_array(unsigned char* p, unsigned char* a, int a_len, unsigned char* b, int b_len){
    for(int i = 0; i<a_len+b_len;i++){
        p[i] = i<a_len ? a[i] : b[i-a_len]; 
    }
}
void print_hex(unsigned char* a, int aSize){
    for(int i = 0; i<aSize;i++){
        printf("%02X",a[i]);
    }
    printf("\n");
}
 void get_uint32_bytes(uint8_t* a, uint32_t b){
    uint32_t temp = htonl(b);
    memcpy(a, &temp, 4);
}

void get_Jn_bytes(uint8_t* out, uint8_t iv[12], uint32_t counter){
    uint8_t counter_array[4];
    get_uint32_bytes(counter_array, counter);
    combine_array(out, iv, 12, counter_array, 4);
}

void temp_AES_GCM(uint8_t* ciphertext, uint8_t* plaintext, int plaintext_length, uint8_t* ad, int ad_len, uint8_t key[16], uint8_t iv[12]){
    uint32_t w[44] = {0};
    key_expansion(key, w, 4);
    __m128i round_keys[11] __attribute__((aligned(16)));
    for (int i = 0; i < 44; i++) {
        w[i] = ntohl(w[i]);
    }
    for(int round = 0; round < 11; round++){
        round_keys[round] = _mm_load_si128((const __m128i*)&w[round*4]);
    }

    uint8_t zero_block[16] = {0};
    memset(zero_block, 0, sizeof zero_block);
    uint8_t H[16] = {0};
    aes128_encrypt_block(zero_block, round_keys, H);

    uint32_t counter = 1;
    uint8_t J0[16] = {0};
    get_Jn_bytes(J0, iv, counter);
    counter++;
    uint8_t S[16] = {0};

    int n = plaintext_length / 16;
    uint8_t Jn[16] = {0};
    uint8_t cipherblock[16] = {0};
    uint8_t sStarted = 0;

    if(ad_len){
        int adBlocks = ad_len / 16;
        uint8_t extraAD = ad_len - adBlocks * 16;
        for(int i = 0; i<adBlocks;i++){
            if(sStarted){
                for(int j = 0; j<16;j++){
                    S[j] ^= ad[i*16 + j];
                }
            } else {
                memcpy(S, ad, 16);
                sStarted = 1;
            }
            gcm_gf_multiply_x86(S, S, H);
        }
        if(extraAD){
            uint8_t extrablock[16] = {0};
            memcpy(extrablock, ad+(adBlocks*16), extraAD);
            if(sStarted){
                for(int i = 0; i<sizeof S; i++){            
                    S[i] ^= extrablock[i];
                }
            } else {
                memcpy(S, extrablock, 16);
                sStarted = 1;
            }
            gcm_gf_multiply_x86(S, S, H);            
        }
    }

    for(int i = 0; i<n; i++){
        get_Jn_bytes(Jn, iv, counter);
        aes128_encrypt_block(Jn, round_keys, cipherblock);
        for(int j = 0; j <sizeof cipherblock; j++){
            ciphertext[(i*16)+j] = plaintext[(i*16)+j] ^ cipherblock[j];
        }
        counter++;
        if(sStarted){
            for(int j = 0; j<sizeof S;j++){
                S[j] ^= ciphertext[(i*16) + j];
            }
        } else {            
            memcpy(S, ciphertext, 16);
            sStarted = 1;
        }
        gcm_gf_multiply_x86(S, S, H);
    }
    uint8_t extrablock_len = plaintext_length - (n * 16);
    if(extrablock_len){
        int Nx16 = n*16;

        get_Jn_bytes(Jn, iv, counter);
        aes128_encrypt_block(Jn, round_keys, cipherblock);
        counter++;

        for(int i = 0; i<extrablock_len; i++){
            ciphertext[Nx16 + i] = cipherblock[i] ^ plaintext[Nx16 + i];
        }
        uint8_t extrablock[16] = {0};
        memcpy(extrablock, ciphertext+Nx16, extrablock_len);
        if(sStarted){
            for(int i = 0; i<sizeof extrablock;i++){
                S[i] ^= extrablock[i];
            }
        } else {
            memcpy(S, extrablock, sizeof extrablock);
            sStarted = 1;
        }
        gcm_gf_multiply_x86(S, S, H);
    }

    // int u = 128 * ceil((float)(16 * 8)/128) - (16 * 8);

    uint8_t lenA[8] = {0};
    get_uint32_bytes(lenA+4, ad_len*8);

    uint8_t lenC[8] = {0};
    get_uint32_bytes(lenC+4, plaintext_length*8);

    uint8_t lenBlock[16];
    combine_array(lenBlock, lenA, 8, lenC, 8);

    for(int i = 0; i<sizeof S;i++){
        S[i] ^= lenBlock[i];
    }

    gcm_gf_multiply_x86(S, S, H);

    uint8_t tag[16];

    aes128_encrypt_block(J0, round_keys, tag);

    for(int i = 0; i< sizeof J0; i++){
        tag[i] ^= S[i];
    }

    printf("tag:        "); print_hex(tag, 16);

}
void from_hex(uint8_t* out, char* in, int len){
    size_t outlen;
    sodium_hex2bin(out, len/2, in, len, NULL, &outlen, NULL);
}
void reverse_16array(uint8_t* arr){
    uint8_t temp;
    for(int i = 0; i<8; i++){
        temp = arr[i];
        arr[i] = arr[15-i];
        arr[15-i] = temp;
    }
}
int main(){

    uint8_t key[16] = {0};
    from_hex(key, "00112233445566778899aabbccddeeff",32);
    uint8_t iv[12] = {0};
    from_hex(iv, "abcdef12345678901f2f3f4f",24);
    uint8_t ad[25] = {};
    from_hex(ad, "abcd1028743610923784610275861307580834765028374506", sizeof(ad)*2);
    uint8_t plaintext[25] = {0};
    from_hex(plaintext, "abcd1028743610923784610275861307580834765028374506",sizeof(plaintext)*2);

    uint8_t ciphertext[sizeof plaintext] = {0};

    temp_AES_GCM(ciphertext, plaintext, sizeof plaintext, ad, sizeof ad, key, iv);

    printf("ciphertext: "); print_hex(ciphertext, sizeof ciphertext);



    // benchmark
    if(0){
        long data_len = 1 * 1000 * 1000 * 1000;
        uint8_t* data = malloc(data_len);
        memset(data, 5, data_len);
        uint8_t* des = malloc(data_len);
        
        float start_time = (float)clock()/CLOCKS_PER_SEC;

        temp_AES_GCM(des, data, data_len, NULL, 0, key, iv);

        float dif_time = ((float)clock()/CLOCKS_PER_SEC) - start_time;
        printf("----- %f ------\n",dif_time);
    }
    // gcm testing
    if(1){
        printf("----- GCM MATH TESTING ------\n");
        if(1){
            uint8_t X[16] = {1};
            uint8_t Y[16] = {1};
            uint8_t out1[16] = {0};

            __m128i Xreg = _mm_loadu_si128((__m128i*)X);
            __m128i Yreg = _mm_loadu_si128((__m128i*)Y);
            __m128i outReg;

            gfmul(Xreg, Yreg, &outReg);

            _mm_storeu_si128((__m128i*)out1, outReg);
            reverse_16array(out1);

            printf("out1 "); print_hex(out1, 16);
        }
        if(1){
            uint8_t X[16] = {1};
            uint8_t Y[16] = {1};
            uint8_t out2[16] = {0};

            gcm_gf_multiply_x86(out2, X, Y);

            printf("out2 "); print_hex(out2, 16);
        }
    }

    return 0;
}