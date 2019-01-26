/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

namespace VMAP
{
    enum class ModelIgnoreFlags : uint32_t
    {
        Nothing = 0x00,
        M2      = 0x01
    };

    inline ModelIgnoreFlags operator&(ModelIgnoreFlags left, ModelIgnoreFlags right)
    {
        return ModelIgnoreFlags(uint32_t(left) & uint32_t(right));
    }
}
