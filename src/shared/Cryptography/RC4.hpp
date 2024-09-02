/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#define ASC_RC4_KEY_LENGTH 256

class ASC_RC4
{
public:
    typedef struct {
        uint8_t state[ASC_RC4_KEY_LENGTH];
        uint8_t x, y;
    } RC4_KEY;

    static void RC4_set_key(RC4_KEY* key, int len, const uint8_t* data)
    {
        uint8_t* state = key->state;
        uint8_t j = 0;

        // Initialize the state with the identity permutation
        for (int i = 0; i < ASC_RC4_KEY_LENGTH; i++) {
            state[i] = i;
        }

        // Key-scheduling algorithm (KSA)
        for (int i = 0; i < ASC_RC4_KEY_LENGTH; i++) {
            j = (j + state[i] + data[i % len]) % ASC_RC4_KEY_LENGTH;
            std::swap(state[i], state[j]);
        }

        key->x = 0;
        key->y = 0;
    }

    static void RC4(RC4_KEY* key, size_t len, const uint8_t* indata, uint8_t* outdata)
    {
        uint8_t* state = key->state;
        uint8_t x = key->x;
        uint8_t y = key->y;
        uint8_t xor_index;

        for (size_t i = 0; i < len; i++) {
            x = (x + 1) % ASC_RC4_KEY_LENGTH;
            y = (y + state[x]) % ASC_RC4_KEY_LENGTH;

            std::swap(state[x], state[y]);

            xor_index = (state[x] + state[y]) % ASC_RC4_KEY_LENGTH;
            outdata[i] = indata[i] ^ state[xor_index];
        }

        key->x = x;
        key->y = y;
    }
};
