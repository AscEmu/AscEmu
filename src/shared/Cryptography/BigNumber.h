/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AUTH_BIGNUMBER_H
#define _AUTH_BIGNUMBER_H

#include "ByteBuffer.h"
#include <vector>

//#include "openssl/bn.h"
struct bignum_st;

class BigNumber
{
public:
    BigNumber();
    BigNumber(const BigNumber & bn);
    BigNumber(uint32_t);
    ~BigNumber();

    void SetDword(uint32_t);
    void SetQword(uint64_t);
    void SetBinary(const uint8_t* bytes, int len);
    void SetHexStr(const char* str);

    void SetRand(int numbits);

    BigNumber& operator=(const BigNumber & bn);

    BigNumber operator+=(const BigNumber & bn);
    BigNumber operator+(const BigNumber & bn)
    {
        BigNumber t(*this);
        return t += bn;
    }
    BigNumber operator-=(const BigNumber & bn);
    BigNumber operator-(const BigNumber & bn)
    {
        BigNumber t(*this);
        return t -= bn;
    }
    BigNumber operator*=(const BigNumber & bn);
    BigNumber operator*(const BigNumber & bn)
    {
        BigNumber t(*this);
        return t *= bn;
    }
    BigNumber operator/=(const BigNumber & bn);
    BigNumber operator/(const BigNumber & bn)
    {
        BigNumber t(*this);
        return t /= bn;
    }
    BigNumber operator%=(const BigNumber & bn);
    BigNumber operator%(const BigNumber & bn)
    {
        BigNumber t(*this);
        return t %= bn;
    }

    BigNumber ModExp(const BigNumber & bn1, const BigNumber & bn2);
    BigNumber Exp(const BigNumber &);

    int GetNumBytes(void);

    struct bignum_st* BN() { return _bn; }

    uint32_t AsDword();
    uint8_t* AsByteArray();
    ByteBuffer AsByteBuffer();
    std::vector<uint8_t> AsByteVector();

    const char* AsHexStr();
    const char* AsDecStr();

private:
    struct bignum_st* _bn;
    std::unique_ptr<uint8_t[]> _array;
};

#endif      //_AUTH_BIGNUMBER_H
