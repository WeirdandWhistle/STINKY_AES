// DISCLAIMER:
// WRITTEN ENTIRELY BY STUIPID HOOMAN


#include <stdint.h>
#include <stdio.h>
#include <string.h>
// #include <wmmintrin.h> // PCLMUL
// #include <emmintrin.h>
// #include <immintrin.h>
// #include <tmmintrin.h> // _mm_shuffle_epi8
#include <math.h>

#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

void gfmul (__m128i a, __m128i b, __m128i *res){
__m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
tmp4 = _mm_clmulepi64_si128(a, b, 0x10);
tmp5 = _mm_clmulepi64_si128(a, b, 0x01);
tmp6 = _mm_clmulepi64_si128(a, b, 0x11);
tmp4 = _mm_xor_si128(tmp4, tmp5);
tmp5 = _mm_slli_si128(tmp4, 8);
tmp4 = _mm_srli_si128(tmp4, 8);
tmp3 = _mm_xor_si128(tmp3, tmp5);
tmp6 = _mm_xor_si128(tmp6, tmp4);
tmp7 = _mm_srli_epi32(tmp3, 31);
tmp8 = _mm_srli_epi32(tmp6, 31);
tmp3 = _mm_slli_epi32(tmp3, 1);
tmp6 = _mm_slli_epi32(tmp6, 1);
tmp9 = _mm_srli_si128(tmp7, 12);
tmp8 = _mm_slli_si128(tmp8, 4);
tmp7 = _mm_slli_si128(tmp7, 4);
tmp3 = _mm_or_si128(tmp3, tmp7);
tmp6 = _mm_or_si128(tmp6, tmp8);
tmp6 = _mm_or_si128(tmp6, tmp9);
tmp7 = _mm_slli_epi32(tmp3, 31);
tmp8 = _mm_slli_epi32(tmp3, 30);
tmp9 = _mm_slli_epi32(tmp3, 25);
tmp7 = _mm_xor_si128(tmp7, tmp8);
tmp7 = _mm_xor_si128(tmp7, tmp9);
tmp8 = _mm_srli_si128(tmp7, 4);
tmp7 = _mm_slli_si128(tmp7, 12);
tmp3 = _mm_xor_si128(tmp3, tmp7);
tmp2 = _mm_srli_epi32(tmp3, 1);
tmp4 = _mm_srli_epi32(tmp3, 2);
tmp5 = _mm_srli_epi32(tmp3, 7);
tmp2 = _mm_xor_si128(tmp2, tmp4);
tmp2 = _mm_xor_si128(tmp2, tmp5);
tmp2 = _mm_xor_si128(tmp2, tmp8);
tmp3 = _mm_xor_si128(tmp3, tmp2);
tmp6 = _mm_xor_si128(tmp6, tmp3);
*res = tmp6;
}
void print_hexs(unsigned char* a, int aSize){
    for(int i = 0; i<aSize;i++){
        printf("%02X",a[i]);
    }
    printf("\n");
}
void gcm_gf_multiply_x86(uint8_t* out, uint8_t *X, const uint8_t *Y){

    // __m128i Xreg = _mm_loadu_si128((__m128i*)X);
    // __m128i Yreg = _mm_loadu_si128((__m128i*)Y);
    // __m128i outReg;

    // gfmul(Xreg, Yreg, &outReg);

    // _mm_storeu_si128((__m128i*)out, outReg);

    uint8_t V[16] = {0};
    memcpy(V, Y, 16);

    uint8_t Z[16] = {0};

    for(int i = 0; i<128; i++){
        int xBit = (X[i/8] >> (7 - (i%8))) & 0x1; // 11001101 & 00000001
        if(xBit){
            for(int j = 0; j <sizeof Z; j++){
                Z[j] ^= V[j];
            }
        }
        uint8_t carry = 0; 
        for(int j = 0; j<16; j++){
            uint8_t next_carry = V[j] & 0x01;
            V[j] = (V[j] >> 1) | (carry << 7);
            carry = next_carry;
        }
        if(carry){
            V[0] ^= 0xE1;
        }        
    }
    memcpy(out, Z, 16);
    // int same = memcmp(out, Z, 16);
    // printf("same %d\n", same);
    // printf("cool: "); print_hexs(out, 16);
    // printf("slow: "); print_hexs(Z, 16);
}
