#ifndef _KANJI14_H_
#define _KANJI14_H_  1

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if 0  /* dummy */
}      /* dummy */
#endif /* dummy */

/*
 * Encode
 */

typedef struct Kanji14Encode {
    unsigned int data;
    unsigned char bits;
} Kanji14Encode;

unsigned int Kanji14EncodeCharLength(unsigned int bytes);

void Kanji14EncodeInit(Kanji14Encode *k14);
int Kanji14EncodePut(Kanji14Encode *k14, unsigned char data);
unsigned short Kanji14EncodeGet(Kanji14Encode *k14);
int Kanji14EncodeHasLast(Kanji14Encode *k14);
unsigned short Kanji14EncodeLast1(Kanji14Encode *k14);
unsigned short Kanji14EncodeLast2(Kanji14Encode *k14);

/*
 * Decode
 */

typedef struct Kanji14Decode {
    unsigned int data;
    unsigned char bits;
} Kanji14Decode;

int Kanji14DecodeCheckValid(int ucs);
int Kanji14DecodeCheckData(int ucs);
int Kanji14DecodeCheckEnd(int ucs);

void Kanji14DecodeInit(Kanji14Decode *k14);
void Kanji14DecodePut(Kanji14Decode *k14, unsigned short ucs2);
int Kanji14DecodeHasData(Kanji14Decode *k14);
unsigned char Kanji14DecodeGet(Kanji14Decode *k14);

/*
 * inline - Encode
 */

inline unsigned int Kanji14EncodeCharLength(unsigned int bytes)
{
    unsigned int d7 = bytes / 7;
    unsigned int r7 = bytes % 7;
    return (d7 << 2) + (!r7 ? 0 : (r7 >> 1) + 2);
}

inline void Kanji14EncodeInit(Kanji14Encode *k14)
{
    k14->data = 0;
    k14->bits = 0;
}

inline int Kanji14EncodePut(Kanji14Encode *k14, unsigned char data)
{
    assert(k14->bits < 14);
    k14->data = (k14->data << 8) | data;
    return (k14->bits += 8) >= 14;
}

inline unsigned short Kanji14EncodeGet(Kanji14Encode *k14)
{
    assert(k14->bits >= 14);
    return 0x4E10 + (0x3FFF & (k14->data >> (k14->bits -= 14)));
}

inline int Kanji14EncodeHasLast(Kanji14Encode *k14)
{
    return k14->bits != 0;
}

inline unsigned short Kanji14EncodeLast1(Kanji14Encode *k14)
{
    assert(0 < k14->bits && k14->bits < 14);
    return 0x4E10 + (0x3FFF & (k14->data << (14 - k14->bits)));
}

inline unsigned short Kanji14EncodeLast2(Kanji14Encode *k14)
{
    assert(0 < k14->bits && k14->bits < 14);
    return 0x4E10 - k14->bits;
}

/*
 * inline - Decode
 */

inline int Kanji14DecodeCheckValid(int ucs)
{
    return 0x4E03 <= ucs && ucs <= 0x8E0F;
}

inline int Kanji14DecodeCheckData(int ucs)
{
    return 0x4E10 <= ucs && ucs <= 0x8E0F;
}

inline int Kanji14DecodeCheckEnd(int ucs)
{
    return 0x4E03 <= ucs && ucs <= 0x4E0F;
}

inline void Kanji14DecodeInit(Kanji14Decode *k14)
{
    k14->data = 0;
    k14->bits = 0;
}

inline void Kanji14DecodePut(Kanji14Decode *k14, unsigned short ucs2)
{
    if (ucs2 >= 0x8E10)
        return;
    if (ucs2 >= 0x4E10)
    {
        assert(k14->bits < 8);
        k14->data = (k14->data << 14) | (ucs2 - 0x4E10);
        k14->bits += 14;
        return;
    }
    if (ucs2 >= 0x4E03)
    {
        k14->data >>= ucs2 - 0x4E02;
        k14->bits -= ucs2 - 0x4E02;
    }
}

inline int Kanji14DecodeHasData(Kanji14Decode *k14)
{
    return k14->bits >= 8;
}

inline unsigned char Kanji14DecodeGet(Kanji14Decode *k14)
{
    assert(k14->bits >= 8);
    return k14->data >> (k14->bits -= 8);
}

#if 0  /* dummy */
{      /* dummy */
#endif /* dummy */
#ifdef __cplusplus
}      /* extern "C" */
#endif /* __cplusplus */
#endif  /* _KANJI14_H_ */
