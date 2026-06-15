#ifndef GCM_CORE
#define GCM_CORE
    #include <mmintrin.h>
    #include <stdint.h>
    void gcm_gf_multiply(uint8_t* out, uint8_t *X, const uint8_t *H);
#endif