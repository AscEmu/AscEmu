/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ValuesUpdate.hpp"
#include "Definitions/ValuesUpdate.hpp"
#include "../ProtocolLayout.hpp"
#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    uint32_t buildChangedObjectTypeMask(ValuesUpdatePresence const& presence, bool ownerVisible)
    {
        uint32_t mask = 0;
        for (auto const& descriptor : Definitions::ValuesPresenceBits)
            if ((!descriptor.ownerOnly || ownerVisible) && presence.*(descriptor.member))
                mask |= uint32_t(1) << descriptor.objectTypeBit;
        return mask;
    }

    uint8_t buildContentsChangedMask(ValuesUpdatePresence const& presence)
    {
        if (presence.isPlayerObject) return ProtocolLayout::Values::PlayerContentsChangedMask;
        if (presence.isUnitObject) return ProtocolLayout::Values::UnitContentsChangedMask;
        if (presence.itemData && !presence.containerData) return ProtocolLayout::Values::ItemContentsChangedMask;
        return ProtocolLayout::Values::ObjectContentsChangedMask;
    }

    void writeEntityFragmentsForValuesUpdate(ByteBuffer& data, bool ownerVisible, ValuesUpdatePresence const& presence)
    {
        data << uint8_t(ownerVisible ? 1 : 0) << ProtocolLayout::Values::FragmentIdsChanged << buildContentsChangedMask(presence);
    }

    std::vector<uint8_t> buildValuesUpdateEnvelope(std::span<const uint8_t> packedGuid, bool ownerVisible, ValuesUpdatePresence const& presence, std::span<const uint8_t> fieldsPayload)
    {
        if (packedGuid.empty() || fieldsPayload.empty()) return {};
        const uint32_t changedObjectTypeMask = buildChangedObjectTypeMask(presence, ownerVisible);
        if (changedObjectTypeMask == 0) return {};
        ByteBuffer payload;
        writeEntityFragmentsForValuesUpdate(payload, ownerVisible, presence);
        payload << changedObjectTypeMask;
        payload.append(fieldsPayload.data(), fieldsPayload.size());
        ByteBuffer block;
        block << ProtocolLayout::Values::UpdateType;
        block.append(packedGuid.data(), packedGuid.size());
        block << uint32_t(payload.size());
        block.append(payload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }
}
