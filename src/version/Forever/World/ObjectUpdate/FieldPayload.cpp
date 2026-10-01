/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../ObjectUpdate.hpp"
#include "CreateFragments.hpp"

#include "version/Forever/Fields/ForeverUpdateFields.hpp"
#include "version/Forever/World/UpdateFields/ObjectData.hpp"
#include "version/Forever/World/UpdateFields/UnitData.hpp"
#include "version/Forever/World/UpdateFields/PlayerData.hpp"
#include "version/Forever/World/UpdateFields/ActivePlayerData.hpp"
#include "Network/ByteBuffer.hpp"

#include <algorithm>
#include <array>

namespace AscEmu::Version::Forever::ObjectUpdate
{
    std::vector<uint8_t> buildPlayerFieldPayload(Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, Fields::PlayerData const& playerFields, Fields::ActivePlayerData const* activePlayerFields, bool ownerVisible, bool partyMemberVisible)
    {
        if (ownerVisible && activePlayerFields == nullptr)
            return {};

        ByteBuffer payload;
        Detail::writePlayerCreateFragments(payload, ownerVisible);

        UpdateFields::writeObjectDataCreate(payload, objectFields);
        UpdateFields::writeUnitDataCreate(payload, unitFields, ownerVisible);
        if (!UpdateFields::writePlayerDataCreate(payload, playerFields, partyMemberVisible))
            return {};

        if (!ownerVisible)
        {
            payload << uint8_t(1);
            Detail::writeEmptyPlayerHouseInfoComponentCreate(payload);
            payload << uint8_t(1);
            Detail::writeEmptyPlayerInitiativeComponentCreate(payload);
            return std::vector<uint8_t>(payload.contents(), payload.contents() + payload.size());
        }

        const Fields::ActivePlayerData activePlayer = UpdateFields::makeConservativeActivePlayerData(*activePlayerFields);

        ByteBuffer activePlayerPayload;
        if (!UpdateFields::writeActivePlayerDataCreate(activePlayerPayload, activePlayer))
            return {};

        payload.append(activePlayerPayload.contents(), activePlayerPayload.size());

        // Remaining 69913 ActivePlayer fields that are structurally required
        // but not semantically identified yet. Keep them isolated from the
        // typed portion so they can be replaced field-by-field later.
        static constexpr std::size_t TypedReferenceEnd = 8422;
        static constexpr std::size_t ZeroDefaultsEnd = 23573;
        static constexpr std::size_t WriterOwnedTailBytes = Fields::ActivePlayerData::UnknownAfterTransmogSize;
        static constexpr std::size_t ZeroDefaultsSize = (ZeroDefaultsEnd - TypedReferenceEnd) - WriterOwnedTailBytes;
        static const std::array<uint8_t, ZeroDefaultsSize> ZeroDefaults{};
        payload.append(ZeroDefaults.data(), ZeroDefaults.size());

        static constexpr std::size_t SparseDefaultsSize = 38749 - 23573;
        static const std::array<uint8_t, SparseDefaultsSize> SparseDefaults = []
        {
            std::array<uint8_t, SparseDefaultsSize> data{};
            data[0] = 0x80;
            data[1] = 0x03;
            data[733] = 0x0C;
            data[860] = 0x80;
            data[2124] = 0x08;
            data[3408] = 0x1C;
            data[9837] = 0x80;
            data[9838] = 0x38;
            data[9929] = 0x40;

            constexpr std::array<uint8_t, 15> finalDefaults = { 0x01,0x00,0x01,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x10 };
            std::copy(finalDefaults.begin(), finalDefaults.end(), data.begin() + 15159);
            return data;
        }();
        payload.append(SparseDefaults.data(), SparseDefaults.size());

        payload << uint8_t(1);
        Detail::writeEmptyPlayerHouseInfoComponentCreate(payload);
        payload << uint8_t(1);
        Detail::writeEmptyPlayerInitiativeComponentCreate(payload);

        return std::vector<uint8_t>(payload.contents(), payload.contents() + payload.size());
    }

    std::vector<uint8_t> buildSelfFieldPayload(Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, Fields::PlayerData const& playerFields, Fields::ActivePlayerData const& activePlayerFields)
    {
        return buildPlayerFieldPayload(objectFields, unitFields, playerFields, &activePlayerFields, true, true);
    }
}
