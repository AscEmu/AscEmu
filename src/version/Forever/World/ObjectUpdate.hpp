/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <span>
#include <vector>

class ByteBuffer;

namespace AscEmu::Version::Forever::Fields
{
    struct ObjectData;
    struct ItemData;
    struct ContainerData;
    struct UnitData;
    struct PlayerData;
    struct ActivePlayerData;
    struct GameObjectData;
    struct DynamicObjectData;
    struct CorpseData;
}

namespace AscEmu::Version::Forever::ObjectUpdate
{
    void writeObjectDataCreate(ByteBuffer& data, Fields::ObjectData const& fields);
    void writeUnitDataCreate(ByteBuffer& data, Fields::UnitData const& fields, bool ownerVisible);
    bool writePlayerDataCreate(ByteBuffer& data, Fields::PlayerData const& fields, bool partyMemberVisible);
    bool writeActivePlayerDataCreate(ByteBuffer& data, Fields::ActivePlayerData const& fields);

    std::vector<uint8_t> buildSelfFieldPayload(Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, Fields::PlayerData const& playerFields, Fields::ActivePlayerData const& activePlayerFields);


    // 69913 create-only local updater path for ordinary world units.
    // This intentionally covers the stationary/minimal creature grammar first;
    // spline/transport movement remains separate work.
    std::vector<uint8_t> buildCreatureCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs, Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, uint32_t vendorDataFlags69913 = 0);

    // Capture-verified stationary GameObject CREATE_OBJECT grammar for 69913.
    std::vector<uint8_t> buildGameObjectCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, int64_t packedLocalRotation, Fields::ObjectData const& objectFields, Fields::GameObjectData const& gameObjectFields);


    std::vector<uint8_t> buildValuesUpdateBlock(std::span<const uint8_t> packedGuid, bool ownerVisible, Fields::ObjectData const& objectFields, Fields::ItemData const* itemFields = nullptr, Fields::ContainerData const* containerFields = nullptr, Fields::UnitData const* unitFields = nullptr, Fields::PlayerData const* playerFields = nullptr, Fields::ActivePlayerData const* activePlayerFields = nullptr, Fields::GameObjectData const* gameObjectFields = nullptr, Fields::DynamicObjectData const* dynamicObjectFields = nullptr, Fields::CorpseData const* corpseFields = nullptr);

    std::vector<uint8_t> buildUpdateObjectPacket(uint16_t mapId, uint32_t updateCount, std::span<const uint8_t> updateBlocks, uint32_t destroyCount = 0, std::span<const uint8_t> destroyGuids = {}, uint32_t outOfRangeCount = 0, std::span<const uint8_t> outOfRangeGuids = {});

    std::vector<uint8_t> buildSelfCreatePacket(uint16_t mapId, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, std::span<const uint8_t> fieldPayload);
}
