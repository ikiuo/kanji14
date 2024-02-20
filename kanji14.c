/*
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kanji14.h"

/* ------------------------------------------------------------------------- */

/*
 * encode
 */

static int k14put_width = 0;
static int k14put_count = 0;

static int k14put(FILE *ofp, unsigned short ucs2)
{
    assert(ucs2 >= 0x0800);

    if (fputc(0xe0 | (ucs2 >> 12), ofp) < 0) return -1;
    if (fputc(0x80 | ((ucs2 >> 6) & 0x3f), ofp) < 0) return -1;
    if (fputc(0x80 | ((ucs2 >> 0) & 0x3f), ofp) < 0) return -1;

    if (k14put_width > 0 && ++k14put_count >= k14put_width)
    {
        k14put_count = 0;
        if (fputc('\n', ofp) < 0)
            return -1;
    }
    return 0;
}

static int kanji14_encode(FILE *ifp, FILE *ofp)
{
    Kanji14Encode k14i;
    Kanji14Encode *k14 = &k14i;
    int c;

    Kanji14EncodeInit(k14);
    for (;;)
    {
        if (ferror(ifp))
            return -1;
        if ((c = fgetc(ifp)) == EOF)
            break;
        if (Kanji14EncodePut(k14, (unsigned char) c) &&
            k14put(ofp, Kanji14EncodeGet(k14)) < 0)
            return -1;
    }
    if (Kanji14EncodeHasLast(k14)) {
        if (k14put(ofp, Kanji14EncodeLast1(k14)) < 0) return -1;
        if (k14put(ofp, Kanji14EncodeLast2(k14)) < 0) return -1;
    }

    if (k14put_width > 0 && fputc('\n', ofp) < 0)
        return -1;

    return  0;
}

/* ------------------------------------------------------------------------- */

/*
 * decode
 */

static int k14get(FILE *ifp)
{
    static int pending = -1;
    const int ce = EOF - 1;
    const int cu = 0xfffd;
    int c1, c2;
    int d = 0;

    if ((c1 = pending) >= 0)
        pending = -1;
    else
    {
        if (ferror(ifp))
            return -1;
        if ((c1 = fgetc(ifp)) == EOF)
            return EOF;
    }
    if (c1 < 0x80)
        return c1;
    if (c1 < 0xc0)
        return cu;
    d = (d << 6) | (c1 & 0x3f);

#define k14get_n(check, mask)                   \
    if ((c2 = fgetc(ifp)) == EOF)               \
        return ce;                              \
    if (c2 < 0x80 || 0xc0 <= c2)                \
    {                                           \
        pending = c2;                           \
        return cu;                              \
    }                                           \
    d = (d << 6) | (c2 & 0x3f);                 \
    if (!(c1 & check))                          \
        return (d & mask)

    k14get_n(0x20, 0x0007ff);
    k14get_n(0x10, 0x00ffff);
    k14get_n(0x08, 0x1fffff);

#undef k14get_n

    return cu;
}

static int kanji14_decode(FILE *ifp, FILE *ofp)
{
    Kanji14Decode k14i;
    Kanji14Decode *k14 = &k14i;
    int c;

    Kanji14DecodeInit(k14);
    for (;;)
    {
        c = k14get(ifp);
        if (c == EOF)
            break;
        if (c < 0)
            return -1;
        if (!Kanji14DecodeCheckValid(c))
            continue;

        Kanji14DecodePut(k14, (unsigned short) c);
        while (Kanji14DecodeHasData(k14))
            fputc(Kanji14DecodeGet(k14), ofp);
    }
    return  0;
}

/* ------------------------------------------------------------------------- */

static const char *program = NULL;

static int usage()
{
    printf("Usage: %s [options] [INPUT [OUTPUT]]\n"
           "\n"
           "options are:\n"
           "    -d    decode mode.\n"
           "    -e    encode mode.\n"
           "    -w N  set width.\n"
           "\n"
           , program);
    return 1;
}

static char *shift_arg(int *argc, char ***argv)
{
    char *arg = NULL;

    if ((*argc) > 0)
    {
        arg = *(*argv);
        --(*argc);
        ++(*argv);
    }
    return arg;
}

#define shift()  (shift_arg(&argc, &argv))

int main (int argc, char **argv)
{
    int new_argc = 0;
    char **new_argv = argv;
    char *arg, *oarg;
    int s_opt;
    int flag_decode = 0;
    int flag_encode = 0;
    FILE *ifp = stdin;
    FILE *ofp = stdout;

    program = shift();
    while (argc > 0)
    {
        arg = shift();
        if ((arg[0] != '-') || (arg[1] == 0))
        {
            new_argv[new_argc++] = arg;
            continue;
        }

        while ((s_opt = *(++arg)) != 0)
        {
            switch (s_opt)
            {
            case 'd':
                flag_decode = !0;
                continue;
            case 'e':
                flag_encode = !0;
                continue;
            case 'w':
                k14put_width = strtoul(shift(), NULL, 10);
                continue;
            default:
                return usage ();
            }
        }
    }

    if (flag_decode == flag_encode)
        return usage();

    argc = new_argc;
    argv = new_argv;

    switch (argc) {
    case 2:
        if (strcmp(argv[1], "-"))
            if (!(ofp = fopen(argv[1], "wb")))
            {
                fprintf(stderr, "open error: %s\n", argv[1]);
                return 2;
            }
    case 1:
        if (strcmp(argv[0], "-"))
            if (!(ifp = fopen(argv[0], "rb")))
            {
                fprintf(stderr, "open error: %s\n", argv[0]);
                return 2;
            }
        break;
    default:
        return usage();
    }

    if (flag_encode)
        return (kanji14_encode(ifp, ofp) ? 2 : 0);
    if (flag_decode)
        return (kanji14_decode(ifp, ofp) ? 2 : 0);

    return usage();
}
