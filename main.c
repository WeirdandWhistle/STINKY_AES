#include <assert.h>
#include <tmmintrin.h>
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



void print_hex(unsigned char* a, int aSize){
    for(int i = 0; i<aSize;i++){
        printf("%02X",a[i]);
    }
    printf("\n");
}
void print_hex_reg(__m128i reg){
    uint8_t temp[16] = {0};
    _mm_storeu_si128((__m128i*)temp, reg);
    print_hex(temp,16);
}
void print128_num(__m128i var){
    uint16_t val[8];
    memcpy(val, &var, sizeof(val));
    printf("Numerical: %02x %02x %02x %02x %02x %02x %02x %02x \n",
           val[0], val[1], val[2], val[3], val[4], val[5], 
           val[6], val[7]);
}
/* Hardware acceration part written by AI:
Encrypts a single 16-byte block using pre-expanded round keys */
void aes128_encrypt_block_hw(uint8_t* plaintext, __m128i* round_keys, uint8_t* ciphertext) {
    // #if defined(HARDWARE_SPEED)
    // Load the 16-byte plaintext into a 128-bit register (unaligned load)
    __m128i state = _mm_loadu_si128((const __m128i*)plaintext);

    // Round 0: Whiten
    state = _mm_xor_si128(state, round_keys[0]);
    // print128_num(state);
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
    // #else



    // #endif
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
uint8_t get_cr(uint8_t* in, uint8_t col, uint8_t row){
    return in[col * 4 + row];
}
void set_cr(uint8_t* in, uint8_t col, uint8_t row, uint8_t byte){
    in[col * 4 + row] = byte;
}
void ShiftRow(uint8_t* mat){
    // uint8_t temp = get_cr(mat, 0,1);
    // set_cr(mat, 0, 1, get_cr(mat, 1, 1));
    // set_cr(mat, 1, 1, get_cr(mat, 2, 1));
    // set_cr(mat, 2, 1, get_cr(mat, 3, 1));
    // set_cr(mat, 3, 1, temp);

    // temp = get_cr(mat, 0, 2);
    // uint8_t temp2 = get_cr(mat, 2,1);
    // set_cr(mat, 0, 2, get_cr(mat, 2, 2));
    // set_cr(mat, 1, 2, get_cr(mat, 3, 2));
    // set_cr(mat, 2, 2, temp);
    // set_cr(mat, 3, 2, temp2);

    // temp = get_cr(mat, 3, 3);
    // set_cr(mat, 3, 3, get_cr(mat, 2,3));
    // set_cr(mat, 2, 3, get_cr(mat, 1, 3));
    // set_cr(mat, 1, 3, get_cr(mat, 0, 3));
    // set_cr(mat, 0, 3, temp);

    // if it works it works. WRITTEN BY AI:

    // Row 0: Unchanged

    // Row 1: Shift left by 1 position
    uint8_t temp = get_cr(mat, 0, 1);
    set_cr(mat, 0, 1, get_cr(mat, 1, 1));
    set_cr(mat, 1, 1, get_cr(mat, 2, 1));
    set_cr(mat, 2, 1, get_cr(mat, 3, 1));
    set_cr(mat, 3, 1, temp);

    // Row 2: Shift left by 2 positions
    temp = get_cr(mat, 0, 2);
    uint8_t temp2 = get_cr(mat, 1, 2);
    set_cr(mat, 0, 2, get_cr(mat, 2, 2));
    set_cr(mat, 1, 2, get_cr(mat, 3, 2));
    set_cr(mat, 2, 2, temp);
    set_cr(mat, 3, 2, temp2);

    // Row 3: Shift left by 3 positions (or right by 1)
    temp = get_cr(mat, 3, 3);
    set_cr(mat, 3, 3, get_cr(mat, 2, 3));
    set_cr(mat, 2, 3, get_cr(mat, 1, 3));
    set_cr(mat, 1, 3, get_cr(mat, 0, 3));
    set_cr(mat, 0, 3, temp);
}
void AddRoundKey(uint8_t* mat, uint32_t* w, uint8_t round){
    int l = round * aes_128_Nb;
    for(int col = 0; col < 4; col++){
        uint32_t word = w[l + col];
        // printf("%08"PRIx32"", word);
        for(int row = 0; row < 4; row++){
            // printf("row spec %02x\n",(uint8_t)(word >> ((3-row)*8)));
            mat[col*4 + row] ^= (uint8_t)(word >> ((3-row)*8));
            // printf("during mat "); print_hex(mat, 16);
        }
    }
    // printf("out mat "); print_hex(mat, 16);
    /*
    for(int col = 0; col < 4; col++){
            
    }
    for(int row = 0; row < 4; row++){
        
    }
    
    mat[row*4 + col] ^= (uint8_t)(w[l + col] >> ((3-row)*8));
    */
}
void MixColumns(uint8_t* mat){
    uint8_t orgMat[16] = {0};
    memcpy(orgMat, mat, 16);

    for(int col = 0; col < 4; col++){
        uint8_t s0 = get_cr(orgMat, col, 0);
        uint8_t s1 = get_cr(orgMat, col, 1);
        uint8_t s2 = get_cr(orgMat, col, 2);
        uint8_t s3 = get_cr(orgMat, col, 3);

        set_cr(mat, col, 0,gcm_gf28_mult(2, s0) ^ gcm_gf28_mult(3, s1) ^ s2 ^ s3);
        set_cr(mat, col, 1, s0 ^ gcm_gf28_mult(2, s1) ^ gcm_gf28_mult(3, s2) ^ s3);
        set_cr(mat, col, 2, s0 ^  s1 ^ gcm_gf28_mult(2, s2) ^ gcm_gf28_mult(3, s3));
        set_cr(mat, col, 3, gcm_gf28_mult(3, s0) ^ s1 ^ s2 ^ gcm_gf28_mult(2, s3));
    }
    // 6353e08c0960e104cd70b751bacad0e7
    // 6353E08C0960E104CD70B751BACAD0E7
}
void SubBytes(uint8_t* mat){
    for(int i = 0; i<16; i++){
        mat[i] = sbox[mat[i]];
    }
}
void aes128_engine_no_hw(uint8_t* plaintext, uint32_t* w, uint8_t* ciphertext){
    uint8_t state[16] = {0};
    memcpy(state, plaintext, 16);

    int round = 0;

    // printf("beforestate "); print_hex(state, 16);
    AddRoundKey(state, w, round);
    round++;
    // return;

    while(round<=aes_128_Nr-1){
        // printf("%d start state ",round); print_hex(state, 16);
        SubBytes(state);
        // printf("s_box state "); print_hex(state, 16);
        ShiftRow(state);
        // printf("s_row state "); print_hex(state, 16);
        MixColumns(state);
        // printf("m_col state "); print_hex(state, 16);
        AddRoundKey(state, w, round);
        round++;
        // return;
    }

    SubBytes(state);
    ShiftRow(state);
    AddRoundKey(state, w, round);

    memcpy(ciphertext, state, 16);
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
    while(i < aes_128_Nb * (aes_128_Nr+1)){
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
 void get_uint32_bytes(uint8_t* a, uint32_t b){
    uint32_t temp = htonl(b);
    memcpy(a, &temp, 4);
}

void get_Jn_bytes(uint8_t* out, uint8_t iv[12], uint32_t counter){
    uint8_t counter_array[4];
    get_uint32_bytes(counter_array, counter);
    combine_array(out, iv, 12, counter_array, 4);
}
void generate_round_keys(__m128i round_keys[11], uint8_t key[16]){
    uint32_t w[44] = {0};
    key_expansion(key, w, aes_128_Nk);
    for (int i = 0; i < 44; i++) {
        w[i] = ntohl(w[i]);
    }
    for(int round = 0; round < 11; round++){
        round_keys[round] = _mm_load_si128((const __m128i*)&w[round*4]);
    }
}
void xor_16_hardware(uint8_t out[16], uint8_t a[16], uint8_t b[16]){
    __m128i aReg = _mm_load_si128((__m128i*)a);
    __m128i bReg = _mm_load_si128((__m128i*)b);

    __m128i xor = _mm_xor_si128(aReg, bReg);

    _mm_storeu_si128((__m128i*)out, xor);
}
void s_aes_128_gcm_encypt(uint8_t* ciphertext, uint8_t tag[16], uint8_t* plaintext, int plaintext_length, uint8_t* ad, int ad_len, uint8_t key[16], uint8_t iv[12]){
    // hardware specific varibles are denoted by a _hw at the end.
    #if defined(HARDWARE_SPEED)
        __m128i round_keys[11] __attribute__((aligned(16)));
        generate_round_keys(round_keys, key);
    #else
        uint32_t w[aes_128_W_len] = {0};
        key_expansion(key, w,aes_128_Nk); 
    #endif
    uint32_t counter = 1;
    uint8_t J0[16] = {0};
    get_Jn_bytes(J0, iv, counter);
    counter++;
    uint8_t S[16] = {0};    
    uint8_t H[16] = {0};
    #if defined(HARDWARE_SPEED)
        aes128_encrypt_block_hw(H, round_keys, H);

        __m128i H_hw __attribute__((aligned(16)));
        H_hw = _mm_loadu_si128((__m128i*)H);
        __m128i reverse_mask = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        H_hw = _mm_shuffle_epi8(H_hw, reverse_mask);

        __m128i S_hw = _mm_loadu_si128((__m128i*)S);
    #else
        aes128_engine_no_hw(H, w, H);
    #endif

    

    int n = plaintext_length / 16;
    uint8_t Jn[16] = {0};
    uint8_t cipherblock[16] = {0};

    if(ad_len){
        int adBlocks = ad_len / 16;
        uint8_t extraAD = ad_len - adBlocks * 16;
        for(int i = 0; i<adBlocks;i++){
            #if defined(HARDWARE_SPEED)
                __m128i block = _mm_loadu_si128((__m128i*)ad+i*16);
                gcm_ghash(block, &S_hw, H_hw);
            #else
                for(int j = 0; j<16;j++){
                    S[j] ^= ad[i*16 + j];
                }
                gcm_gf_multiply(S, S, H);
            #endif
        }
        if(extraAD){
            uint8_t extrablock[16] = {0};
            memcpy(extrablock, ad+(adBlocks*16), extraAD);
            #if defined(HARDWARE_SPEED)
                __m128i block = _mm_loadu_si128((__m128i*)extrablock);
                gcm_ghash(block, &S_hw, H_hw);
            #else
                for(int i = 0; i<sizeof S; i++){            
                    S[i] ^= extrablock[i];
                }
                gcm_gf_multiply(S, S, H);
            #endif        
        }
    }
    //temp
    if(0){
        #if defined(HARDWARE_SPEED)
            S_hw = _mm_shuffle_epi8(S_hw, reverse_mask);
            printf("after ad s:"); print_hex_reg(S_hw);
        #else
            printf("after ad s:"); print_hex(S,16);
        #endif
    }

    for(int i = 0; i<n; i++){
        get_Jn_bytes(Jn, iv, counter);
        #if defined(HARDWARE_SPEED)
            aes128_encrypt_block_hw(Jn, round_keys, cipherblock);
            xor_16_hardware(ciphertext+i*16, plaintext+i*16, cipherblock);
            __m128i block = _mm_loadu_si128((__m128i*)ciphertext+16*i);
            gcm_ghash(block, &S_hw, H_hw);
        #else
            aes128_engine_no_hw(Jn, w, cipherblock);
            for(int j = 0; j <sizeof cipherblock; j++){
                ciphertext[(i*16)+j] = plaintext[(i*16)+j] ^ cipherblock[j];
            }
            for(int j = 0; j<sizeof S;j++){
                S[j] ^= ciphertext[(i*16) + j];
            }
            gcm_gf_multiply(S, S, H);
        #endif    
        counter++;
    }
    uint8_t extrablock_len = plaintext_length - (n * 16);
    if(extrablock_len){
        int Nx16 = n*16;
        get_Jn_bytes(Jn, iv, counter);
        #if defined(HARDWARE_SPEED)
            aes128_encrypt_block_hw(Jn, round_keys, cipherblock);
        #else
            aes128_engine_no_hw(Jn, w, cipherblock);
        #endif
        counter++;

        for(int i = 0; i<extrablock_len; i++){
            ciphertext[Nx16 + i] = cipherblock[i] ^ plaintext[Nx16 + i];
        }
        uint8_t extrablock[16] = {0};
        memcpy(extrablock, ciphertext+Nx16, extrablock_len);
        #if defined(HARDWARE_SPEED)
            __m128i block = _mm_loadu_si128((__m128i*)extrablock);
            gcm_ghash(block, &S_hw, H_hw);
        #else
            for(int i = 0; i<sizeof extrablock;i++){
                S[i] ^= extrablock[i];
            }
            gcm_gf_multiply(S, S, H);
        #endif
        
    }

    // int u = 128 * ceil((float)(16 * 8)/128) - (16 * 8);

    uint8_t lenA[8] = {0};
    get_uint32_bytes(lenA+4, ad_len*8);

    uint8_t lenC[8] = {0};
    get_uint32_bytes(lenC+4, plaintext_length*8);

    uint8_t lenBlock[16];
    combine_array(lenBlock, lenA, 8, lenC, 8);

    
    #if defined(HARDWARE_SPEED)
        __m128i block = _mm_loadu_si128((__m128i*)lenBlock);
        gcm_ghash(block, &S_hw, H_hw);
        S_hw = _mm_shuffle_epi8(S_hw, reverse_mask);
        _mm_storeu_si128((__m128i*)S, S_hw);
        aes128_encrypt_block_hw(J0, round_keys, tag);
        xor_16_hardware(tag, tag, S);
    #else
        for(int i = 0; i<sizeof S;i++){
            S[i] ^= lenBlock[i];
        }
        gcm_gf_multiply(S, S, H);
        aes128_engine_no_hw(J0, w, tag);

        for(int i = 0; i< sizeof J0; i++){
            tag[i] ^= S[i];
        }
    #endif
}
int s_aes_128_gcm_decrypt(uint8_t* plaintext, uint8_t* ad, int ad_len, uint8_t* ciphertext, int ciphertext_length, uint8_t tag[16], uint8_t key[16], uint8_t iv[12]){
    #if defined(HARDWARE_SPEED)
        __m128i round_keys[11] __attribute__((aligned(16)));
        generate_round_keys(round_keys, key);
    #else
        uint32_t w[aes_128_W_len] = {0};
        key_expansion(key, w, aes_128_Nk);
    #endif

    uint8_t H[16] = {0};
    #if defined(HARDWARE_SPEED)
         aes128_encrypt_block_hw(H, round_keys, H);
    #else
        aes128_engine_no_hw(H, w, H);
    #endif

    uint32_t counter = 1;
    uint8_t J0[16] = {0};
    get_Jn_bytes(J0, iv, counter);
    counter++;
    uint8_t S[16] = {0};

    int n = ciphertext_length / 16;
    uint8_t extra_ciphertext = ciphertext_length - (n*16);
    uint8_t cipherblock[16];
    uint8_t Jn[16] = {0};

    int addtionalBlocks = ad_len / 16;
    uint8_t extraAD = ad_len - addtionalBlocks;
    for(int i = 0; i<addtionalBlocks; i++){
        #if defined(HARDWARE_SPEED)
            xor_16_hardware(S, S, ad+(i*16));
        #else
            for(int j = 0; j<16; j++){
                S[j] ^= ad[(i*16)+j];
            }
        #endif
        gcm_gf_multiply(S, S, H);
    }
    if(extraAD){
        uint8_t extrablock[16] = {0};
        memcpy(extrablock, ad+(addtionalBlocks*16), 16);

        #if defined(HARDWARE_SPEED)
            xor_16_hardware(S, S, extrablock);
        #else
            for(int i = 0; i<16;i++){
                S[i] ^= extrablock[i];
            }
        #endif
        gcm_gf_multiply(S, S, H);
    }

    for(int i = 0; i<n;i++){
        get_Jn_bytes(Jn, iv, counter);

        #if defined(HARDWARE_SPEED)
            aes128_encrypt_block_hw(Jn, round_keys, cipherblock);

            xor_16_hardware(plaintext+(i*16), ciphertext+(i*16), cipherblock);
            xor_16_hardware(S, S, ciphertext+(i*16));
        #else
            aes128_engine_no_hw(Jn, w, cipherblock);
            for(int j = 0; j<16; j++){
                plaintext[i+j] = cipherblock[j] ^ ciphertext[i+j];
                S[j] ^= ciphertext[(i*16)+j];
            }
        #endif
        counter++;
        gcm_gf_multiply(S, S, H);
    }
    if(extra_ciphertext){
        uint8_t extrablock[16] = {0};
        memcpy(extrablock, ciphertext+(n*16), extra_ciphertext);
        
        get_Jn_bytes(Jn, iv, counter);
        #if defined(HARDWARE_SPEED)
            aes128_encrypt_block_hw(Jn, round_keys, cipherblock);
        #else
            aes128_engine_no_hw(Jn, w, cipherblock);
        #endif
        counter++;

        for(int i = 0; i<extra_ciphertext;i++){
            plaintext[(n*16)+i] = extrablock[i] ^ cipherblock[i];
        }
        #if defined(HARDWARE_SPEED)
            xor_16_hardware(S, S, extrablock);
        #else
            for(int i = 0; i<16;i++){
                S[i] ^= extrablock[i];
            }
        #endif
        gcm_gf_multiply(S, S, H);
    }
    // printf("S: "); print_hex(S, 16);

    uint8_t lenA[8] = {0};
    get_uint32_bytes(lenA+4, ad_len*8);

    uint8_t lenC[8] = {0};
    get_uint32_bytes(lenC+4, ciphertext_length*8);

    uint8_t lenBlock[16];
    combine_array(lenBlock, lenA, 8, lenC, 8);

    #if defined(HARDWARE_SPEED)
        xor_16_hardware(S, S, lenBlock);
    #else
        for(int i = 0; i<sizeof S;i++){
            S[i] ^= lenBlock[i];
        }
    #endif

    gcm_gf_multiply(S, S, H);

    uint8_t T[16] = {0};

    #if defined(HARDWARE_SPEED)
        aes128_encrypt_block_hw(J0, round_keys, T);
        xor_16_hardware(T, S, T);
    #else
        aes128_engine_no_hw(J0, w, T);
        for(int i = 0; i<sizeof S;i++){
            T[i] ^= S[i];
        }
    #endif

    // constant time compare
    uint8_t notEqual = 0;
    for(int i = 0; i<16;i++){
        notEqual |= (tag[i] ^ T[i]);
    }

    // print_hex(T, 16);
    // print_hex(tag, 16);

    return notEqual;
}
void from_hex(uint8_t* out, char* in, int len){
    size_t outlen;
    sodium_hex2bin(out, len/2, in, len, NULL, &outlen, NULL);
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

    // printf("ciphertext: "); print_hex(ciphertext, sizeof ciphertext);
    // printf("tag:        "); print_hex(tag, sizeof tag);

    // uint8_t temp[sizeof plaintext] = {0};
    // assert(s_aes_128_gcm_decrypt(temp, ad, sizeof ad, ciphertext, sizeof ciphertext, tag, key, iv)==0);

    // printf("plaintext\nabcd1028743610923784610275861307580834765028374506\n"); print_hex(plaintext, sizeof plaintext);

    // bench marking
    if(0){
        int len = 1 * 1000 * 1000 * 1000;
        uint8_t* p = malloc(len); 
        uint8_t* c = malloc(len);
        s_aes_128_gcm_encypt(c, tag, p, len, NULL, 0, key, iv); 
    }

    return 0;
}