#ifndef GCM_CORE
#define GCM_CORE

    #define HARDWARE_SPEED // make sure to compile with flags like: -O3 -maes -msse4 -mpclmul -march=native

    #include <mmintrin.h>
    #include <stdint.h>
    void gcm_gf_multiply(uint8_t* out, uint8_t *X, const uint8_t *H);
    uint8_t gcm_gf28_mult(uint8_t a, uint8_t b);
    
    #if defined(HARDWARE_SPEED)
        void gcm_ghash(__m128i X, __m128i S, __m128i H);
    #endif

    
#endif