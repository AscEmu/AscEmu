/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>

namespace AscEmu
{
    constexpr static int keyLength = 256;

    class RC4StreamCipher
    {
    public:
        typedef struct
        {
            uint8_t state[keyLength];
            uint8_t x, y;
        } KeyStruct;

        static void setKey(KeyStruct* _key, const size_t _len, const uint8_t* _data)
        {
            uint8_t* state = _key->state;
            uint8_t j = 0;

            // Initialize the state with the identity permutation
            for (int i = 0; i < keyLength; i++)
            {
                state[i] = i;
            }

            // Key-scheduling algorithm (KSA)
            for (int i = 0; i < keyLength; i++)
            {
                j = (j + state[i] + _data[i % _len]) % keyLength;
                std::swap(state[i], state[j]);
            }

            _key->x = 0;
            _key->y = 0;
        }

        static void process(KeyStruct* _key, const size_t _len, const uint8_t* _inData, uint8_t* _outData)
        {
            uint8_t* state = _key->state;
            uint8_t x = _key->x;
            uint8_t y = _key->y;

            for (size_t i = 0; i < _len; i++)
            {
                x = (x + 1) % keyLength;
                y = (y + state[x]) % keyLength;

                std::swap(state[x], state[y]);

                const uint8_t xor_index = (state[x] + state[y]) % keyLength;
                _outData[i] = _inData[i] ^ state[xor_index];
            }

            _key->x = x;
            _key->y = y;
        }
    };

    class RC4Engine
    {
    public:
        RC4Engine() = default;
        ~RC4Engine() = default;

        void setup(const uint8_t* _data, const size_t _len)
        {
            RC4StreamCipher::setKey(&m_key, _len, _data);
            m_isInitialized = true;
        }

        void process(const uint8_t* _inData, uint8_t* _outData, const size_t _len)
        {
            RC4StreamCipher::process(&m_key, _len, _inData, _outData);
        }

    private:
        RC4StreamCipher::KeyStruct m_key = {{0}, 0, 0};
        bool m_isInitialized = false;
    };
}
