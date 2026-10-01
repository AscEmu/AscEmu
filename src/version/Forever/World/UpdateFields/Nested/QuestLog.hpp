/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Fields/ForeverUpdateFields.hpp"

class ByteBuffer;

namespace AscEmu::Version::Forever::UpdateFields::Nested
{
    void writeQuestLogCreate(ByteBuffer& data, Fields::QuestLog const& value);
    void writeQuestLogQuestIdUpdate(ByteBuffer& data, Fields::QuestLog const& value);
    void writeQuestLogQuestIdToIndexUpdate(ByteBuffer& data, Fields::PlayerData const& fields);
}
