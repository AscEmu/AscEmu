/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "StdAfx.h"

namespace G3D
{
    class Vector3;
    class Vector4;
}

namespace MovementNew {

using G3D::Vector3;
using G3D::Vector4;

inline uint32_t SecToMS(float sec)
{
    return static_cast<uint32_t>(sec * 1000.f);
}

inline float MSToSec(uint32_t ms)
{
    return ms / 1000.f;
}

float computeFallTime(float path_length, bool isSafeFall);
float computeFallElevation(float t_passed, bool isSafeFall, float start_velocity = 0.0f);

template<class T, T limit>
class counter
{
public:
    counter() { init(); }

    void Increase()
    {
        if (m_counter == limit)
            init();
        else
            ++m_counter;
    }

    T NewId() { Increase(); return m_counter; }
    T getCurrent() const { return m_counter; }

private:
    void init() { m_counter = 0; }
    T m_counter;
};

typedef counter<uint32_t, 0xFFFFFFFF> UInt32Counter;

SERVER_DECL extern float gravity;
SERVER_DECL extern UInt32Counter splineIdGen;

} // namespace MovementNew
