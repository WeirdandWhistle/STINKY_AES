#ifndef GCM_CORE
#define GCM_CORE
    #include <stdint.h>
    void gcm_gf_multiply_x86(uint8_t* out, uint8_t *X, const uint8_t *H);
#endif