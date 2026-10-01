/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Fields/ForeverUpdateFields.hpp"

class ByteBuffer;

namespace AscEmu::Version::Forever::UpdateFields
{
    Fields::ActivePlayerData makeConservativeActivePlayerData(Fields::ActivePlayerData const& source);
    bool writeActivePlayerDataCreate(ByteBuffer& data, Fields::ActivePlayerData const& fields);
    void writeActivePlayerDataUpdate(ByteBuffer& data, Fields::ActivePlayerData const& fields);
}
