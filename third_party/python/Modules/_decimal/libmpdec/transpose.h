#ifndef TRANSPOSE_H
#define TRANSPOSE_H
#include "third_party/python/Modules/_decimal/libmpdec/mpdecimal.h"
/* clang-format off */

/* Internal header file: all symbols have local scope in the DSO */
MPD_PRAGMA(MPD_HIDE_SYMBOLS_START)

enum {FORWARD_CYCLE, BACKWARD_CYCLE};

void std_trans(mpd_uint_t dest[], mpd_uint_t src[], mpd_size_t rows, mpd_size_t cols);
int transpose_pow2(mpd_uint_t *matrix, mpd_size_t rows, mpd_size_t cols);
void transpose_3xpow2(mpd_uint_t *matrix, mpd_size_t rows, mpd_size_t cols);

static inline void pointerswap(mpd_uint_t **a, mpd_uint_t **b)
{
    mpd_uint_t *tmp;
    tmp = *b;
    *b = *a;
    *a = tmp;
}

MPD_PRAGMA(MPD_HIDE_SYMBOLS_END) /* restore previous scope rules */

#endif
