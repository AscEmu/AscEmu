/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <span>
#include <vector>

class ByteBuffer;

namespace AscEmu::Version::Forever::UpdateFields
{
    struct ValuesUpdatePresence
    {
        bool objectData = false;
        bool itemData = false;
        bool containerData = false;
        bool unitData = false;
        bool playerData = false;
        bool activePlayerData = false;
        bool gameObjectData = false;
        bool dynamicObjectData = false;
        bool corpseData = false;

        // Object family is independent from which field group is dirty.
        // Retail 1.60.x Creature VALUES packets keep the Unit contents mask (0x03)
        // even when only ObjectData changes, e.g. DynamicFlags.
        bool isUnitObject = false;
        bool isPlayerObject = false;
    };

    uint32_t buildChangedObjectTypeMask(ValuesUpdatePresence const& presence, bool ownerVisible);
    uint8_t buildContentsChangedMask(ValuesUpdatePresence const& presence);
    void writeEntityFragmentsForValuesUpdate(ByteBuffer& data, bool ownerVisible, ValuesUpdatePresence const& presence);
    std::vector<uint8_t> buildValuesUpdateEnvelope(std::span<const uint8_t> packedGuid, bool ownerVisible, ValuesUpdatePresence const& presence, std::span<const uint8_t> fieldsPayload);
}
