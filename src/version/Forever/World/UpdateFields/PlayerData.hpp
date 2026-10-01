/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Fields/ForeverUpdateFields.hpp"

class ByteBuffer;

namespace AscEmu::Version::Forever::UpdateFields
{
    bool writePlayerDataCreate(ByteBuffer& data, Fields::PlayerData const& fields, bool partyMemberVisible);
    void writePlayerDataUpdate(ByteBuffer& data, Fields::PlayerData const& fields);
}
