// This file is part of keepcoding_core
// ==================================
//
// base64.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#ifndef KC_BASE64_H
#define KC_BASE64_H

#include "../common.h"

#include <stdio.h>
#include <stdint.h>

/*
 * This header file defines a simple Base64 encoding and decoding library in C.
 * It provides functions to encode binary data into Base64 format and decode
 * Base64-encoded strings into binary data. The library follows the Base64
 * alphabet as defined in RFC 4648.
 *
 *      Base64 alphabet defined in RFC 4648
 *   +-----------------------------------------+
 *   |  0 - A  |  16 - Q  |  32 - g  |  48 - w |
 *   |  1 - B  |  17 - R  |  33 - h  |  49 - x |
 *   |  2 - C  |  18 - S  |  34 - i  |  50 - y |
 *   |  3 - D  |  19 - T  |  35 - j  |  51 - z |
 *   |  4 - E  |  20 - U  |  36 - k  |  52 - 0 |
 *   |  5 - F  |  21 - V  |  37 - l  |  53 - 1 |
 *   |  6 - G  |  22 - W  |  38 - m  |  54 - 2 |
 *   |  7 - H  |  23 - X  |  39 - n  |  55 - 3 |
 *   |  8 - I  |  24 - Y  |  40 - o  |  56 - 4 |
 *   |  9 - J  |  25 - Z  |  41 - p  |  57 - 5 |
 *   | 10 - K  |  26 - a  |  42 - q  |  58 - 6 |
 *   | 11 - L  |  27 - b  |  43 - r  |  59 - 7 |
 *   | 12 - M  |  28 - c  |  44 - s  |  60 - 8 |
 *   | 13 - N  |  29 - d  |  45 - t  |  60 - 9 |
 *   | 14 - O  |  30 - e  |  46 - u  |  60 - + |
 *   | 15 - P  |  31 - f  |  47 - v  |  60 - / |
 *   +-----------------------------------------+
 *   | Pading  |  =                            |
 *   +-----------------------------------------+
 */

#define KC_BASE64_LEN  64

//---------------------------------------------------------------------------//

int kc_base64_encode  (char* src, size_t src_len, char** output);
int kc_base64_decode  (char* src, size_t src_len, char** output);

//---------------------------------------------------------------------------//

#endif /* KC_BASE64_H */
