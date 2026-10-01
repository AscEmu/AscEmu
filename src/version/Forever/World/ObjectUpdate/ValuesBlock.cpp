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

namespace AscEmu::Version::Forever::ObjectUpdate
{

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
        return UpdateFields::buildValuesUpdateEnvelope(packedGuid, ownerVisible, presence, std::span<const uint8_t>(fieldsPayload.contents(), fieldsPayload.size()));
    }
}
