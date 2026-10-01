/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../ObjectUpdate.hpp"
#include "CreateFragments.hpp"
#include "CreateMovement.hpp"

#include "version/Forever/Fields/ForeverUpdateFields.hpp"
#include "version/Forever/World/UpdateFields/ObjectData.hpp"
#include "version/Forever/World/UpdateFields/ItemData.hpp"
#include "version/Forever/World/UpdateFields/GameObjectData.hpp"
#include "version/Forever/World/UpdateFields/UnitData.hpp"
#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::ObjectUpdate
{
    namespace
    {
        constexpr uint8_t UPDATE_TYPE_CREATE_OBJECT_2 = 2;
        constexpr uint8_t OBJECT_TYPE_ITEM = 1;
        constexpr uint8_t OBJECT_TYPE_UNIT = 5;
        constexpr uint8_t OBJECT_TYPE_PLAYER = 6;
        constexpr uint8_t OBJECT_TYPE_ACTIVE_PLAYER = 7;
        constexpr uint8_t OBJECT_TYPE_GAMEOBJECT = 8;
    }

    std::vector<uint8_t> buildCreatureCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs, Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, uint32_t vendorDataFlags)
    {
        if (packedGuid.empty())
            return {};

        ByteBuffer fieldPayload;
        Detail::writeCreatureCreateFragments(fieldPayload, vendorDataFlags != 0);
        UpdateFields::writeObjectDataCreate(fieldPayload, objectFields);
        UpdateFields::writeUnitDataCreate(fieldPayload, unitFields, false);

        if (vendorDataFlags != 0)
            fieldPayload << int32_t(vendorDataFlags);

        ByteBuffer block;
        block << uint8_t(1); // CREATE_OBJECT (ordinary world unit)
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_UNIT);
        Detail::writeStationaryUnitMovement(block, packedGuid, x, y, z, orientation, movementTimeMs);
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildGameObjectCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, int64_t packedLocalRotation, Fields::ObjectData const& objectFields, Fields::GameObjectData const& gameObjectFields)
    {
        if (packedGuid.empty())
            return {};

        ByteBuffer fieldPayload;
        Detail::writeGameObjectCreateFragments(fieldPayload);
        UpdateFields::writeObjectDataCreate(fieldPayload, objectFields);
        UpdateFields::writeGameObjectDataCreate(fieldPayload, gameObjectFields);

        ByteBuffer block;
        block << uint8_t(1); // CREATE_OBJECT
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_GAMEOBJECT);
        Detail::writeStationaryGameObjectMovement(block, x, y, z, orientation, packedLocalRotation);
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildItemCreateBlock(std::span<const uint8_t> packedGuid, Fields::ObjectData const& objectFields, Fields::ItemData const& itemFields)
    {
        if (packedGuid.empty())
            return {};

        ByteBuffer fieldPayload;
        Detail::writeItemCreateFragments(fieldPayload);
        UpdateFields::writeObjectDataCreate(fieldPayload, objectFields);
        UpdateFields::writeItemDataCreate(fieldPayload, itemFields, objectFields.entryId);

        ByteBuffer block;
        block << uint8_t(1); // CREATE_OBJECT
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_ITEM);
        Detail::writeItemMovement(block);
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildPlayerCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, std::span<const uint8_t> fieldPayload, bool ownerVisible)
    {
        if (packedGuid.empty() || fieldPayload.empty())
            return {};

        ByteBuffer block;
        block << uint8_t(ownerVisible ? UPDATE_TYPE_CREATE_OBJECT_2 : 1);
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(ownerVisible ? OBJECT_TYPE_ACTIVE_PLAYER : OBJECT_TYPE_PLAYER);
        Detail::writePlayerMovement(block, packedGuid, x, y, z, orientation);
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload.data(), fieldPayload.size());
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildSelfCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, std::span<const uint8_t> fieldPayload)
    {
        return buildPlayerCreateBlock(packedGuid, x, y, z, orientation, fieldPayload, true);
    }
}
