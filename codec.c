#include "codec.h"
#include <string.h>

#define WHITESPACE 64
#define EQUALS 65
#define INVALID 66

/* Encoding translation table */
static const unsigned char enc64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Decoding translation table */
static const unsigned char dec64[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

/*
 * @brief Checks the value passed is a valid encoded character or not
 *        Returns 1 if valid, 0 if invalid
 *
 * @param val value to be tested
 */
int is_valid_char(uint8_t val)
{
    int flag = 0;
    unsigned char c = dec64[val];

    switch(c) {
        case WHITESPACE:
        case INVALID:
            flag = 0;
            break;
        case EQUALS:
        default:
            flag = 1;
    }
    return flag;
}

/* @brief encode 3x 8-bit binary bytes as 4x '6-bit' characters
 *        Returns length of encoded string
 *
 * @param inp input array (size 3)
 * @param out output array (size 4)
 * @param len length of input array
 */
size_t encode_block(uint8_t *inp, uint8_t *out, int len)
{
    size_t output_len = (sizeof(uint8_t) * 4);
    memset(out, 0, output_len);
    out[0] = (uint8_t) enc64[(int)(inp[0] >> 2) ];
    out[1] = (uint8_t) enc64[(int)(((inp[0] & 0x03) << 4) |
            ((inp[1] & 0xf0) >> 4)) ];
    out[2] = (uint8_t) ((len > 1) ? (enc64[(int)(((inp[1] & 0x0f) << 2) |
                    ((inp[2] & 0xc0) >> 6)) ]) : '=');
    out[3] = (uint8_t) ((len > 2) ? (enc64[(int)(inp[2] & 0x3f) ]) : '=');
    return output_len;
}

/* @brief decode 4x '6-bit' characters into 3x 8-bit binary bytes
 *        Returns the length of the decoded block
 *
 * @param inp input array (size 4)
 * @param out output array (size 3)
 */
size_t decode_block(uint8_t *inp, uint8_t *out)
{   
    int j, iter = 0;
    unsigned char val;
    size_t buf = 0;
    size_t output_len = 0;

    memset(out, '\0', sizeof(uint8_t) * 3);
    for (j = 0; j < 4; j++) {
        if (inp[j] == '=')
            continue;
        val = dec64[inp[j]];
        buf = buf << 6 | val;
        ++iter;
    }
    if (iter == 4) {
        out[0] = (unsigned char) (buf >> 16) & 0xff;
        out[1] = (unsigned char) (buf >> 8) & 0xff;
        out[2] = (unsigned char) (buf & 0xff);
    } else if (iter == 3) {
        out[0] = (unsigned char) (buf >> 10) & 0xff;
        out[1] = (unsigned char) (buf >> 2) & 0xff;
    } else if (iter == 2) {
        out[0] = (unsigned char) (buf >> 4) & 0xff;
    }
    for (j = 0; j < (iter - 1); j++) {
        if (out[j] != 0)
            ++output_len;
    }
    return output_len;
}
