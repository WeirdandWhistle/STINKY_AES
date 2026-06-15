#ifndef GCM_CORE
#define GCM_CORE
    #include <mmintrin.h>
    #include <stdint.h>
    void gcm_gf_multiply_x86(uint8_t* out, uint8_t *X, const uint8_t *H);
    void gfmul (__m128i a, __m128i b, __m128i *res);
#endif