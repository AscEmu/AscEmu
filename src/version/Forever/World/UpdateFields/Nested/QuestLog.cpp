/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "QuestLog.hpp"

#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Nested
{
    void writeQuestLogCreate(ByteBuffer& data, Fields::QuestLog const& value)
    {
        data << value.questId << value.stateFlags;
        for (int16_t progress : value.objectiveProgress)
            data << progress;
        data << value.endTime << value.objectiveFlags << value.enabledObjectivesMask;
    }

    void writeQuestLogQuestIdUpdate(ByteBuffer& data, Fields::QuestLog const& value)
    {
        // Verified directly from the 70124 retail UPDATE_OBJECT emitted after accepting quest 783:
        // nested QuestLog mask bytes C0 00 00 01 80, followed by int32 QuestID.
        // Keep CREATE serialization untouched; this is only the differential QuestID update.
        data.writeBit(true);
        data.writeBits(uint32_t(0x80000003), 32);
        data.flushBits();
        data << value.questId;
    }

    void writeQuestLogQuestIdToIndexUpdate(ByteBuffer& data, Fields::PlayerData const& fields)
    {
        // Verified 70124 MapUpdateField shape:
        // uint8 ignoreChangesMask(0), uint16 changeCount,
        // then int32 QuestID, uint8 state (1=changed, 2=deleted), and int32 index for changed entries.
        data << uint8_t(0) << uint16_t(fields.questLogQuestIdToIndexChanges.size());
        for (Fields::QuestLogQuestIdToIndexChange const& change : fields.questLogQuestIdToIndexChanges)
        {
            data << change.questId << change.state;
            if (change.state != 2)
                data << change.index;
        }
    }
}
