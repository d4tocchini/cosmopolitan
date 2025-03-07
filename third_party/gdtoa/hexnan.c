/*-*- mode:c;indent-tabs-mode:t;c-basic-offset:8;tab-width:8;coding:utf-8   -*-│
│vi: set et ft=c ts=8 tw=8 fenc=utf-8                                       :vi│
╚──────────────────────────────────────────────────────────────────────────────╝
│                                                                              │
│  The author of this software is David M. Gay.                                │
│  Please send bug reports to David M. Gay <dmg@acm.org>                       │
│                          or Justine Tunney <jtunney@gmail.com>               │
│                                                                              │
│  Copyright (C) 1998, 1999 by Lucent Technologies                             │
│  All Rights Reserved                                                         │
│                                                                              │
│  Permission to use, copy, modify, and distribute this software and           │
│  its documentation for any purpose and without fee is hereby                 │
│  granted, provided that the above copyright notice appear in all             │
│  copies and that both that the copyright notice and this                     │
│  permission notice and warranty disclaimer appear in supporting              │
│  documentation, and that the name of Lucent or any of its entities           │
│  not be used in advertising or publicity pertaining to                       │
│  distribution of the software without specific, written prior                │
│  permission.                                                                 │
│                                                                              │
│  LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,               │
│  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.            │
│  IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY           │
│  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES                   │
│  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER             │
│  IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,              │
│  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF              │
│  THIS SOFTWARE.                                                              │
│                                                                              │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "third_party/gdtoa/gdtoa.internal.h"
/* clang-format off */

static void
L_shift(ULong *x, ULong *x1, int i)
{
	int j;
	i = 8 - i;
	i <<= 2;
	j = ULbits - i;
	do {
		*x |= x[1] << j;
		x[1] >>= i;
	} while(++x < x1);
}

int
hexnan( const char **sp, const FPI *fpi, ULong *x0)
{
	ULong c, h, *x, *x1, *xe;
	const char *s;
	int havedig, hd0, i, nbits;
	/**** if (!hexdig['0']) __gdtoa_hexdig_init(); ****/
	nbits = fpi->nbits;
	x = x0 + (nbits >> kshift);
	if (nbits & kmask)
		x++;
	*--x = 0;
	x1 = xe = x;
	havedig = hd0 = i = 0;
	s = *sp;
	/* allow optional initial 0x or 0X */
	while((c = *(const unsigned char*)(s+1)) && c <= ' ') {
		if (!c)
			goto retnan;
		++s;
	}
	if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X')
	    && *(const unsigned char*)(s+3) > ' ')
		s += 2;
	while((c = *(const unsigned char*)++s)) {
		if (!(h = hexdig[c])) {
			if (c <= ' ') {
				if (hd0 < havedig) {
					if (x < x1 && i < 8)
						L_shift(x, x1, i);
					if (x <= x0) {
						i = 8;
						continue;
					}
					hd0 = havedig;
					*--x = 0;
					x1 = x;
					i = 0;
				}
				while((c = *(const unsigned char*)(s+1)) <= ' ') {
					if (!c)
						goto retnan;
					++s;
				}
				if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X')
				    && *(const unsigned char*)(s+3) > ' ')
					s += 2;
				continue;
			}
			if (/*(*/ c == ')' && havedig) {
				*sp = s + 1;
				break;
			}
			do {
				if (/*(*/ c == ')') {
					*sp = s + 1;
					goto break2;
				}
			} while((c = *++s));
		retnan:
			return STRTOG_NaN;
		}
		havedig++;
		if (++i > 8) {
			if (x <= x0)
				continue;
			i = 1;
			*--x = 0;
		}
		*x = (*x << 4) | (h & 0xf);
	}
break2:
	if (!havedig)
		return STRTOG_NaN;
	if (x < x1 && i < 8)
		L_shift(x, x1, i);
	if (x > x0) {
		x1 = x0;
		do *x1++ = *x++;
		while(x <= xe);
		do *x1++ = 0;
		while(x1 <= xe);
	}
	else {
		/* truncate high-order word if necessary */
		if ( (i = nbits & (ULbits-1)) !=0)
			*xe &= ((ULong)0xffffffff) >> (ULbits - i);
	}
	for(x1 = xe;; --x1) {
		if (*x1 != 0)
			break;
		if (x1 == x0) {
			*x1 = 1;
			break;
		}
	}
	return STRTOG_NaNbits;
}
