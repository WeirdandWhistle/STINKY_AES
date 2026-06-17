// DISCLAIMER:
// WRITTEN ENTIRELY BY SUPER SMART PEOPLE MAINLY FROM INTEL


#include <stdint.h>
#include <string.h>
#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "gcm_core.h"

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
void hardware_gcm_mult(uint8_t* out, uint8_t *X, const uint8_t *Y){
    __m128i Xreg = _mm_loadu_si128((__m128i*)X);
    __m128i Yreg = _mm_loadu_si128((__m128i*)Y);
    __m128i outReg;
    __m128i reverse_mask = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    Xreg = _mm_shuffle_epi8(Xreg, reverse_mask);
    Yreg = _mm_shuffle_epi8(Yreg, reverse_mask);
    gfmul(Xreg, Yreg, &outReg);
    outReg = _mm_shuffle_epi8(outReg, reverse_mask);
    _mm_storeu_si128((__m128i*)out, outReg);
}
#if defined(HARDWARE_SPEED)
    void gcm_ghash(__m128i X, __m128i S, __m128i H){
        __m128i reverse_mask = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        X = _mm_shuffle_epi8(X, reverse_mask);
        S = _mm_xor_si128(X, S);
        gfmul(S, H, &S);
    }
#endif
void gcm_gf_multiply(uint8_t* out, uint8_t *X, const uint8_t *Y){

    #if defined(HARDWARE_SPEED)
        hardware_gcm_mult(out, X, Y);
        return;
    #endif

    uint8_t V[16] = {0};
    memcpy(V, Y, 16);

    uint8_t Z[16] = {0};

    for(int i = 0; i<128; i++){
        int xBit = (X[i/8] >> (7 - (i%8))) & 0x1; // 11001101 & 00000001
        // if(xBit){
        //     for(int j = 0; j <sizeof Z; j++){
        //         Z[j] ^= V[j];
        //     }
        // }
        for(int j = 0; j <sizeof Z; j++){
            Z[j] ^= V[j] & (-xBit);
        }

        uint8_t carry = 0; 
        for(int j = 0; j<16; j++){
            uint8_t next_carry = V[j] & 0x01;
            V[j] = (V[j] >> 1) | (carry << 7);
            carry = next_carry;
        }
        // if(carry){
        //     V[0] ^= 0xE1;
        // }
        V[0] ^= 0xE1 & (-carry);  
    }
    memcpy(out, Z, 16);
}
// algorithm defined here: https://en.wikipedia.org/wiki/Finite_field_arithmetic#Multiplication
uint8_t gcm_gf28_mult(uint8_t a, uint8_t b){
    uint8_t p = 0;
    for(int i = 0; i<8; i++){
        // if(b & 0x01)// b & 00000001
        //     p ^= a; // polynomial addtion
        p = p ^ (a & ( -(b & 0x01)));

        // if(a & 0x80)
        //     a = (a << 1) ^ 0x1b;
        // else
        //     a <<= 1;
        a = (a << 1) ^ (0x1b & ( -((a & 0x80) >> 7)));

        b >>= 1;
    }
    return p;
}
