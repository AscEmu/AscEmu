/*
Copyright (c) 2014-2026 AscEmu Team <http://ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GameObjectData.hpp"
#include "Definitions/GameObjectData.hpp"
#include "WireHelpers.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeGameObjectDataCreate(ByteBuffer& data, Fields::GameObjectData const& fields)
    {
        // Capture-verified Forever CREATE_OBJECT order. A full-mask
        // GameObject VALUES sample serializes the same 104-byte field body.
        data << fields.displayId
             << fields.spellVisualId
             << fields.unknownU32Create3
             << fields.spawnTrackingStateAnimId
             << fields.spawnTrackingStateAnimKitId;

        data << uint32_t(fields.unknownU32Vector0.size())
             << fields.unknownU32Create7;
        for (uint32_t value : fields.unknownU32Vector0)
            data << value;

        writeModernGuid(data, fields.createdBy);
        writeModernGuid(data, fields.guildGuid);

        data << fields.flags << fields.flagsB;
        for (float value : fields.parentRotation)
            data << value;

        data << fields.factionTemplate
             << fields.level
             << fields.state
             << fields.typeId
             << fields.percentHealth
             << fields.artKit;

        data << uint32_t(fields.enableDoodadSets.size())
             << fields.customParam;
        for (int32_t value : fields.enableDoodadSets)
            data << value;

        data << uint32_t(fields.worldEffects.size());
        for (int32_t value : fields.worldEffects)
            data << value;

        data << fields.animGroupInstance
             << fields.uiWidgetItemId
             << fields.uiWidgetItemQuality
             << fields.uiWidgetItemCount
             << fields.unknownU32_26
             << fields.unknownU32_27;
    }

    void writeGameObjectDataUpdate(ByteBuffer& data, Fields::GameObjectData const& fields) { Definitions::GameObjectDataUpdate::writeUpdate(data, fields); }
}
