/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../ObjectUpdate.hpp"

#include "version/Forever/Fields/ForeverUpdateFields.hpp"
#include "version/Forever/World/ProtocolLayout.hpp"
#include "version/Forever/World/UpdateFields/ObjectData.hpp"
#include "version/Forever/World/UpdateFields/ItemData.hpp"
#include "version/Forever/World/UpdateFields/ContainerData.hpp"
#include "version/Forever/World/UpdateFields/UnitData.hpp"
#include "version/Forever/World/UpdateFields/PlayerData.hpp"
#include "version/Forever/World/UpdateFields/ActivePlayerData.hpp"
#include "version/Forever/World/UpdateFields/GameObjectData.hpp"
#include "version/Forever/World/UpdateFields/DynamicObjectData.hpp"
#include "version/Forever/World/UpdateFields/CorpseData.hpp"
#include "version/Forever/World/UpdateFields/ValuesUpdate.hpp"
#include "Network/ByteBuffer.hpp"
#include <atomic>

namespace AscEmu::Version::Forever::ObjectUpdate
{
    namespace
    {
        std::atomic_bool deathUpdateWireDumpRequested{false};
    }

    bool consumeDeathUpdateWireDumpRequest()
    {
        return deathUpdateWireDumpRequested.exchange(false, std::memory_order_acq_rel);
    }

    std::vector<uint8_t> buildValuesUpdateBlock(std::span<const uint8_t> packedGuid, bool ownerVisible, Fields::ObjectData const& objectFields, Fields::ItemData const* itemFields, Fields::ContainerData const* containerFields, Fields::UnitData const* unitFields, Fields::PlayerData const* playerFields, Fields::ActivePlayerData const* activePlayerFields, Fields::GameObjectData const* gameObjectFields, Fields::DynamicObjectData const* dynamicObjectFields, Fields::CorpseData const* corpseFields)
    {
        if (packedGuid.empty())
            return {};

        UpdateFields::ValuesUpdatePresence presence{};
        presence.objectData = objectFields.hasChanges();
        presence.itemData = itemFields && itemFields->hasChanges();
        presence.containerData = containerFields && containerFields->hasChanges();
        presence.unitData = unitFields && unitFields->hasChanges();
        presence.playerData = playerFields && playerFields->hasChanges();
        presence.activePlayerData = activePlayerFields && activePlayerFields->hasChanges();
        presence.gameObjectData = gameObjectFields && gameObjectFields->hasChanges();
        presence.dynamicObjectData = dynamicObjectFields && dynamicObjectFields->hasChanges();
        presence.corpseData = corpseFields && corpseFields->hasChanges();
        presence.isUnitObject = unitFields != nullptr;
        presence.isPlayerObject = playerFields != nullptr;

        const uint32_t changedObjectTypeMask = UpdateFields::buildChangedObjectTypeMask(presence, ownerVisible);
        if (changedObjectTypeMask == 0)
            return {};

        ByteBuffer fieldsPayload;
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Object)) UpdateFields::writeObjectDataUpdate(fieldsPayload, objectFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Item)) UpdateFields::writeItemDataUpdate(fieldsPayload, *itemFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Container)) UpdateFields::writeContainerDataUpdate(fieldsPayload, *containerFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Unit)) UpdateFields::writeUnitDataUpdate(fieldsPayload, *unitFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Player)) UpdateFields::writePlayerDataUpdate(fieldsPayload, *playerFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::ActivePlayer)) UpdateFields::writeActivePlayerDataUpdate(fieldsPayload, *activePlayerFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::GameObject)) UpdateFields::writeGameObjectDataUpdate(fieldsPayload, *gameObjectFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::DynamicObject)) UpdateFields::writeDynamicObjectDataUpdate(fieldsPayload, *dynamicObjectFields);
        if (changedObjectTypeMask & (uint32_t(1) << ProtocolLayout::ObjectType::Corpse)) UpdateFields::writeCorpseDataUpdate(fieldsPayload, *corpseFields);

        if (unitFields && unitFields->changes.test(Fields::UnitData::HealthBit) && unitFields->health == 0)
            deathUpdateWireDumpRequested.store(true, std::memory_order_release);

        const bool lootDeathRelevant = objectFields.changes.test(Fields::ObjectData::DynamicFlagsBit) || (unitFields && (unitFields->changes.test(Fields::UnitData::HealthBit) || unitFields->changes.test(Fields::UnitData::FlagsBit) || unitFields->changes.test(Fields::UnitData::Flags2Bit) || unitFields->changes.test(Fields::UnitData::AuraStateBit)));
        std::vector<uint8_t> block = UpdateFields::buildValuesUpdateEnvelope(packedGuid, ownerVisible, presence, std::span<const uint8_t>(fieldsPayload.contents(), fieldsPayload.size()));
        if (lootDeathRelevant)
        {
            std::string payloadHex;
            payloadHex.reserve(fieldsPayload.size() * 2);
            static constexpr char Hex[] = "0123456789ABCDEF";
            for (std::size_t i = 0; i < fieldsPayload.size(); ++i) { const uint8_t value = fieldsPayload.contents()[i]; payloadHex.push_back(Hex[value >> 4]); payloadHex.push_back(Hex[value & 0x0F]); }
            sLogger.info("[ForeverDebug][LootDeath][Values] typeMask=0x{:08X} ownerVisible={} objectChanges=0x{:X} unitChanges={} payloadBytes={} blockBytes={} payloadHex=[{}]", changedObjectTypeMask, ownerVisible, objectFields.changes.to_ulong(), unitFields ? unitFields->changes.count() : 0, fieldsPayload.size(), block.size(), payloadHex);
        }
        return block;
    }
}
