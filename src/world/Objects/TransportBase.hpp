/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

class SERVER_DECL TransportBase
{
protected:
    TransportBase() = default;
    virtual ~TransportBase() = default;

public:
    // This method transforms supplied transport offsets into global coordinates
    virtual void calculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) = 0;

    // This method transforms supplied global coordinates into local offsets
    virtual void calculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) = 0;

protected:
    void CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO);

    void CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO);
};
