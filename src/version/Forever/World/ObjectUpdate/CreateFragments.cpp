/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreateFragments.hpp"

#include "../ProtocolLayout.hpp"

#include "Data/WoWObject.hpp"
#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::ObjectUpdate::Detail
{
    namespace
    {
        void writeModernGuid(ByteBuffer& data, WoWGuid const& guid)
        {
            const std::vector<uint8_t> packed = guid.packModern();
            data.append(packed.data(), packed.size());
        }
    }

    void writeCreatureCreateFragments(ByteBuffer& data, bool hasVendorFragment)
    {
        // Ordinary stationary creature:
        //   04 03 CC FF 01
        //
        // Capture/runtime-verified Forever vendor creature fragment list:
        //   04 03 12 CC FF 01
        data << uint8_t(ProtocolLayout::Create::CreatureFieldFlags) << uint8_t(ProtocolLayout::Create::Fragment::CGObject);
        if (hasVendorFragment)
            data << uint8_t(ProtocolLayout::Create::Fragment::Vendor);
        data << uint8_t(ProtocolLayout::Create::Fragment::TagUnit) << uint8_t(ProtocolLayout::Create::Fragment::End) << uint8_t(ProtocolLayout::Create::IndirectFragmentActivation);
    }

    void writeGameObjectCreateFragments(ByteBuffer& data)
    {
        // Capture-verified ordinary GameObject fragment list:
        //   00 03 CE FF 01
        data << uint8_t(ProtocolLayout::Create::GameObjectFieldFlags) << uint8_t(ProtocolLayout::Create::Fragment::CGObject) << uint8_t(ProtocolLayout::Create::Fragment::TagGameObject) << uint8_t(ProtocolLayout::Create::Fragment::End) << uint8_t(ProtocolLayout::Create::IndirectFragmentActivation);
    }

    void writeItemCreateFragments(ByteBuffer& data)
    {
        // Capture-verified Forever ordinary Item fragment list:
        //   01 03 C8 FF 01
        data << uint8_t(ProtocolLayout::Create::IndirectFragmentActivation) << uint8_t(ProtocolLayout::Create::Fragment::CGObject) << uint8_t(ProtocolLayout::Create::Fragment::TagItem) << uint8_t(ProtocolLayout::Create::Fragment::End) << uint8_t(ProtocolLayout::Create::IndirectFragmentActivation);
    }

    void writePlayerCreateFragments(ByteBuffer& data, bool ownerVisible)
    {
        data << uint8_t(ownerVisible ? ProtocolLayout::Create::SelfFieldFlags : ProtocolLayout::Create::PlayerFieldFlags)
             << uint8_t(ProtocolLayout::Create::Fragment::CGObject)
             << uint8_t(ProtocolLayout::Create::Fragment::PlayerHouseInfo)
             << uint8_t(ProtocolLayout::Create::Fragment::PlayerInitiative)
             << uint8_t(ProtocolLayout::Create::Fragment::TagUnit)
             << uint8_t(ProtocolLayout::Create::Fragment::TagPlayer)
             << uint8_t(ProtocolLayout::Create::Fragment::End)
             << uint8_t(ProtocolLayout::Create::IndirectFragmentActivation); // CGObject indirect fragment activation
    }

    void writeEmptyPlayerHouseInfoComponentCreate(ByteBuffer& data)
    {
        // Forever empty 0x21 component layout verified for the shared wire layout.
        data << uint32_t(0); // Field_8 count (owner)
        data << uint32_t(0); // Houses count
        data << uint32_t(0); // Field_88 count (owner)
        data << uint32_t(0); // Field_C0 count (owner)
        data << uint32_t(0); // Field_F8 count (owner)
        data << uint32_t(0); // Field_130 count (owner)

        // Verified Forever empty house defaults.
        // These two scalar fields are 1 and -1 even with no house data.
        data << int32_t(1) << int32_t(-1) << uint32_t(0);
        data << uint8_t(0);
        data << uint8_t(0); // EditorMode

        // Empty NeighborhoodOwnershipTransfer.
        writeModernGuid(data, WoWGuid());
        writeModernGuid(data, WoWGuid());
        data << uint8_t(0); // empty ownership name
        writeModernGuid(data, WoWGuid()); // CurrentHouse
    }

    void writeEmptyPlayerInitiativeComponentCreate(ByteBuffer& data)
    {
        writeModernGuid(data, WoWGuid()); // NeighborhoodGUID

        // Empty PlayerInitiativeInfo.
        data << int64_t(0);
        data << int32_t(0) << int32_t(0) << int32_t(0);
        data << float(0.0f) << float(0.0f) << float(0.0f);
        data << uint32_t(0); // CompletedTasks count
        data << uint32_t(0); // CompletedInitiatives count
        data << uint32_t(0); // Houses set count (owner)
    }
}
