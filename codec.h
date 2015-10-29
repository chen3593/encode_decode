#ifndef __CODEC_H__
#define __CODEC_H__

#include <stdint.h>
#include <stddef.h>

/* Returns 1 if 'val' is a valid char under the encoding scheme */
int is_valid_char(uint8_t val);

/* encode 3x 8-bit binary bytes as 4x '6-bit' characters */
size_t encode_block(uint8_t *inp, uint8_t *out, int len);

/* decode 4x '6-bit' characters into 3x 8-bit binary bytes */
size_t decode_block(uint8_t *inp, uint8_t *out);

#endif /* __CODEC_H__ */
