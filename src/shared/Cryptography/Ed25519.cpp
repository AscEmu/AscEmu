/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Ed25519.hpp"

#include <openssl/bn.h>
#include <openssl/evp.h>

#include <array>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>

namespace
{
    struct BigNumDeleter
    {
        void operator()(BIGNUM* _value) const { BN_free(_value); }
    };

    struct BigNumContextDeleter
    {
        void operator()(BN_CTX* _context) const { BN_CTX_free(_context); }
    };

    using BigNum = std::unique_ptr<BIGNUM, BigNumDeleter>;

    constexpr size_t HashLength = 64;
    constexpr size_t CoordinateLength = 32;

    // a point of the curve in extended coordinates: x = X / Z, y = Y / Z, T = X Y / Z
    struct Point
    {
        BigNum x{ BN_new() };
        BigNum y{ BN_new() };
        BigNum z{ BN_new() };
        BigNum t{ BN_new() };

        bool valid() const { return x && y && z && t; }
    };

    // the twisted Edwards curve of RFC 8032: the field prime p = 2^255 - 19, the constant d = -121665 / 121666,
    // the group order L = 2^252 + 27742317777372353535851937790883648493 and the base point B with y = 4 / 5
    // and the even x, all computed from these definitions
    class Curve
    {
    public:
        Curve();

        bool valid() const { return m_valid; }
        BN_CTX* context() const { return m_context.get(); }
        const BIGNUM* order() const { return m_order.get(); }

        // _result = _scalar * B
        bool multiplyBase(const BIGNUM* _scalar, Point& _result) const;

        // the encoding of a point: y little endian with the low bit of x in the top bit
        bool encode(const Point& _point, uint8_t* _output) const;

    private:
        // _result = _left + _right, _result may be one of the operands
        bool add(const Point& _left, const Point& _right, Point& _result) const;

        // the even root x of x^2 = (y^2 - 1) / (d y^2 + 1)
        bool recoverX(const BIGNUM* _y, BIGNUM* _x) const;

        std::unique_ptr<BN_CTX, BigNumContextDeleter> m_context{ BN_CTX_new() };
        BigNum m_prime{ BN_new() };
        BigNum m_d{ BN_new() };
        BigNum m_twoD{ BN_new() };
        BigNum m_order{ BN_new() };
        Point m_base;
        bool m_valid = false;
    };

    Curve::Curve()
    {
        BN_CTX* const ctx = m_context.get();
        BigNum u(BN_new());
        BigNum v(BN_new());
        BigNum inverse(BN_new());
        if (!ctx || !m_prime || !m_d || !m_twoD || !m_order || !m_base.valid() || !u || !v || !inverse)
            return;

        // p = 2^255 - 19
        if (!BN_set_bit(m_prime.get(), 255) || !BN_sub_word(m_prime.get(), 19))
            return;

        // L = 2^252 + 27742317777372353535851937790883648493
        BIGNUM* order = m_order.get();
        if (!BN_dec2bn(&order, "27742317777372353535851937790883648493") || !BN_set_bit(order, 252))
            return;

        // d = -121665 / 121666 and 2 d
        if (!BN_set_word(u.get(), 121665) || !BN_set_word(v.get(), 121666)
            || !BN_mod_inverse(inverse.get(), v.get(), m_prime.get(), ctx)
            || !BN_mod_mul(m_d.get(), u.get(), inverse.get(), m_prime.get(), ctx)
            || !BN_mod_sub(m_d.get(), m_prime.get(), m_d.get(), m_prime.get(), ctx)
            || !BN_mod_add(m_twoD.get(), m_d.get(), m_d.get(), m_prime.get(), ctx))
            return;

        // B: y = 4 / 5, Z = 1, T = x y
        if (!BN_set_word(u.get(), 4) || !BN_set_word(v.get(), 5)
            || !BN_mod_inverse(inverse.get(), v.get(), m_prime.get(), ctx)
            || !BN_mod_mul(m_base.y.get(), u.get(), inverse.get(), m_prime.get(), ctx)
            || !recoverX(m_base.y.get(), m_base.x.get())
            || !BN_set_word(m_base.z.get(), 1)
            || !BN_mod_mul(m_base.t.get(), m_base.x.get(), m_base.y.get(), m_prime.get(), ctx))
            return;

        m_valid = true;
    }

    bool Curve::recoverX(const BIGNUM* _y, BIGNUM* _x) const
    {
        BN_CTX* const ctx = m_context.get();
        const BIGNUM* const p = m_prime.get();
        BigNum square(BN_new());
        BigNum u(BN_new());
        BigNum v(BN_new());
        BigNum exponent(BN_new());
        BigNum check(BN_new());
        if (!square || !u || !v || !exponent || !check)
            return false;

        // x^2 = (y^2 - 1) / (d y^2 + 1)
        if (!BN_mod_sqr(square.get(), _y, p, ctx)
            || !BN_mod_sub(u.get(), square.get(), BN_value_one(), p, ctx)
            || !BN_mod_mul(v.get(), m_d.get(), square.get(), p, ctx)
            || !BN_mod_add(v.get(), v.get(), BN_value_one(), p, ctx)
            || !BN_mod_inverse(check.get(), v.get(), p, ctx)
            || !BN_mod_mul(square.get(), u.get(), check.get(), p, ctx))
            return false;

        // the candidate root (x^2)^((p + 3) / 8)
        if (!BN_copy(exponent.get(), p) || !BN_add_word(exponent.get(), 3) || !BN_rshift(exponent.get(), exponent.get(), 3)
            || !BN_mod_exp(_x, square.get(), exponent.get(), p, ctx)
            || !BN_mod_sqr(check.get(), _x, p, ctx))
            return false;

        // otherwise the root is the candidate times 2^((p - 1) / 4)
        if (BN_cmp(check.get(), square.get()) != 0)
        {
            if (!BN_copy(exponent.get(), p) || !BN_sub_word(exponent.get(), 1) || !BN_rshift(exponent.get(), exponent.get(), 2)
                || !BN_set_word(u.get(), 2) || !BN_mod_exp(u.get(), u.get(), exponent.get(), p, ctx)
                || !BN_mod_mul(_x, _x, u.get(), p, ctx)
                || !BN_mod_sqr(check.get(), _x, p, ctx) || BN_cmp(check.get(), square.get()) != 0)
                return false;
        }

        // the even one of the two roots
        return !BN_is_odd(_x) || BN_sub(_x, p, _x);
    }

    bool Curve::add(const Point& _left, const Point& _right, Point& _result) const
    {
        BN_CTX* const ctx = m_context.get();
        const BIGNUM* const p = m_prime.get();
        BigNum a(BN_new());
        BigNum b(BN_new());
        BigNum c(BN_new());
        BigNum d(BN_new());
        BigNum e(BN_new());
        BigNum f(BN_new());
        BigNum g(BN_new());
        BigNum h(BN_new());
        if (!a || !b || !c || !d || !e || !f || !g || !h)
            return false;

        // A = (Y1 - X1)(Y2 - X2), B = (Y1 + X1)(Y2 + X2), C = 2 d T1 T2, D = 2 Z1 Z2
        // E = B - A, F = D - C, G = D + C, H = B + A
        // X3 = E F, Y3 = G H, Z3 = F G, T3 = E H
        return BN_mod_sub(e.get(), _left.y.get(), _left.x.get(), p, ctx) && BN_mod_sub(f.get(), _right.y.get(), _right.x.get(), p, ctx)
            && BN_mod_mul(a.get(), e.get(), f.get(), p, ctx)
            && BN_mod_add(e.get(), _left.y.get(), _left.x.get(), p, ctx) && BN_mod_add(f.get(), _right.y.get(), _right.x.get(), p, ctx)
            && BN_mod_mul(b.get(), e.get(), f.get(), p, ctx)
            && BN_mod_mul(c.get(), _left.t.get(), _right.t.get(), p, ctx) && BN_mod_mul(c.get(), c.get(), m_twoD.get(), p, ctx)
            && BN_mod_mul(d.get(), _left.z.get(), _right.z.get(), p, ctx) && BN_mod_add(d.get(), d.get(), d.get(), p, ctx)
            && BN_mod_sub(e.get(), b.get(), a.get(), p, ctx) && BN_mod_sub(f.get(), d.get(), c.get(), p, ctx)
            && BN_mod_add(g.get(), d.get(), c.get(), p, ctx) && BN_mod_add(h.get(), b.get(), a.get(), p, ctx)
            && BN_mod_mul(_result.x.get(), e.get(), f.get(), p, ctx) && BN_mod_mul(_result.y.get(), g.get(), h.get(), p, ctx)
            && BN_mod_mul(_result.z.get(), f.get(), g.get(), p, ctx) && BN_mod_mul(_result.t.get(), e.get(), h.get(), p, ctx);
    }

    bool Curve::multiplyBase(const BIGNUM* _scalar, Point& _result) const
    {
        Point current;
        if (!_result.valid() || !current.valid()
            || !BN_copy(current.x.get(), m_base.x.get()) || !BN_copy(current.y.get(), m_base.y.get())
            || !BN_copy(current.z.get(), m_base.z.get()) || !BN_copy(current.t.get(), m_base.t.get()))
            return false;

        // the neutral element (0, 1, 1, 0), then double and add from the lowest bit on
        if (!BN_set_word(_result.x.get(), 0) || !BN_set_word(_result.y.get(), 1) || !BN_set_word(_result.z.get(), 1) || !BN_set_word(_result.t.get(), 0))
            return false;

        const int bits = BN_num_bits(_scalar);
        for (int bit = 0; bit < bits; ++bit)
        {
            if (BN_is_bit_set(_scalar, bit) && !add(_result, current, _result))
                return false;
            if (!add(current, current, current))
                return false;
        }
        return true;
    }

    bool Curve::encode(const Point& _point, uint8_t* _output) const
    {
        BN_CTX* const ctx = m_context.get();
        const BIGNUM* const p = m_prime.get();
        BigNum inverse(BN_new());
        BigNum x(BN_new());
        BigNum y(BN_new());
        if (!inverse || !x || !y)
            return false;

        if (!BN_mod_inverse(inverse.get(), _point.z.get(), p, ctx)
            || !BN_mod_mul(x.get(), _point.x.get(), inverse.get(), p, ctx)
            || !BN_mod_mul(y.get(), _point.y.get(), inverse.get(), p, ctx)
            || BN_bn2lebinpad(y.get(), _output, CoordinateLength) != static_cast<int>(CoordinateLength))
            return false;

        if (BN_is_odd(x.get()))
            _output[CoordinateLength - 1] |= 0x80;
        return true;
    }

    // SHA-512 of the concatenated parts as a scalar modulo L
    bool hashToScalar(const Curve& _curve, std::initializer_list<std::pair<const uint8_t*, size_t>> _parts, BIGNUM* _scalar)
    {
        std::array<uint8_t, HashLength> hash{};
        unsigned int hashLength = 0;
        std::unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)> digest(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!digest || EVP_DigestInit_ex(digest.get(), EVP_sha512(), nullptr) != 1)
            return false;

        for (const auto& [data, size] : _parts)
        {
            if (EVP_DigestUpdate(digest.get(), data, size) != 1)
                return false;
        }

        return EVP_DigestFinal_ex(digest.get(), hash.data(), &hashLength) == 1 && hashLength == HashLength
            && BN_lebin2bn(hash.data(), static_cast<int>(hash.size()), _scalar)
            && BN_nnmod(_scalar, _scalar, _curve.order(), _curve.context());
    }

    // SHA-512 of the private key: the clamped secret scalar in the first half, the prefix in the second half
    bool expandPrivateKey(const uint8_t* _privateKey, std::array<uint8_t, HashLength>& _hash)
    {
        unsigned int hashLength = 0;
        if (EVP_Digest(_privateKey, Ed25519::KeyLength, _hash.data(), &hashLength, EVP_sha512(), nullptr) != 1 || hashLength != HashLength)
            return false;

        _hash[0] &= 248;
        _hash[31] &= 63;
        _hash[31] |= 64;
        return true;
    }
}

bool Ed25519::publicKey(const uint8_t* _privateKey, uint8_t* _publicKey)
{
    const Curve curve;
    std::array<uint8_t, HashLength> hash{};
    if (!curve.valid() || !expandPrivateKey(_privateKey, hash))
        return false;

    BigNum secret(BN_lebin2bn(hash.data(), CoordinateLength, nullptr));
    Point point;
    return secret && curve.multiplyBase(secret.get(), point) && curve.encode(point, _publicKey);
}

bool Ed25519::signWithContext(const uint8_t* _privateKey, const uint8_t* _context, size_t _contextLength, const uint8_t* _message, size_t _messageLength, uint8_t* _signature)
{
    if (_contextLength > 255)
        return false;

    const Curve curve;
    std::array<uint8_t, HashLength> hash{};
    if (!curve.valid() || !expandPrivateKey(_privateKey, hash))
        return false;

    // the public key A = a B
    BigNum secret(BN_lebin2bn(hash.data(), CoordinateLength, nullptr));
    Point point;
    std::array<uint8_t, CoordinateLength> encodedA{};
    if (!secret || !curve.multiplyBase(secret.get(), point) || !curve.encode(point, encodedA.data()))
        return false;

    // the domain separator: the tag, the flag 0 of the variant without prehash, the context length and the context
    constexpr char tag[] = "SigEd25519 no Ed25519 collisions";
    std::array<uint8_t, sizeof(tag) - 1 + 2 + 255> domain{};
    std::memcpy(domain.data(), tag, sizeof(tag) - 1);
    domain[sizeof(tag) - 1] = 0;
    domain[sizeof(tag)] = static_cast<uint8_t>(_contextLength);
    std::memcpy(domain.data() + sizeof(tag) + 1, _context, _contextLength);
    const size_t domainLength = sizeof(tag) + 1 + _contextLength;

    // r = H(domain || prefix || message), R = r B
    BigNum r(BN_new());
    std::array<uint8_t, CoordinateLength> encodedR{};
    if (!r || !hashToScalar(curve, { { domain.data(), domainLength }, { hash.data() + CoordinateLength, CoordinateLength }, { _message, _messageLength } }, r.get())
        || !curve.multiplyBase(r.get(), point) || !curve.encode(point, encodedR.data()))
        return false;

    // k = H(domain || R || A || message), S = r + k a
    BigNum k(BN_new());
    BigNum s(BN_new());
    if (!k || !s || !hashToScalar(curve, { { domain.data(), domainLength }, { encodedR.data(), encodedR.size() }, { encodedA.data(), encodedA.size() }, { _message, _messageLength } }, k.get())
        || !BN_mod_mul(k.get(), k.get(), secret.get(), curve.order(), curve.context())
        || !BN_mod_add(s.get(), r.get(), k.get(), curve.order(), curve.context()))
        return false;

    // the signature R || S
    std::memcpy(_signature, encodedR.data(), encodedR.size());
    return BN_bn2lebinpad(s.get(), _signature + encodedR.size(), CoordinateLength) == static_cast<int>(CoordinateLength);
}
