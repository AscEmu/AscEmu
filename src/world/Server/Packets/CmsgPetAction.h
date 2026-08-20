/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Objects/Units/Creatures/PetDefines.hpp"

namespace AscEmu::Packets
{
    class CmsgPetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        PetActionButtonData buttonData;
        uint64_t targetguid;

        // Only populated on Mop, for ground-targeted pet abilities
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        CmsgPetAction() : CmsgPetAction(0, PetActionButtonData{ .raw = 0 }, 0)
        {
        }

        CmsgPetAction(uint64_t guid, PetActionButtonData buttonData, uint64_t targetguid) :
            ManagedPacket(CMSG_PET_ACTION, 20),
            guid(guid),
            buttonData(buttonData),
            targetguid(targetguid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid petGuid;
                WoWGuid targetGuid;

                packet >> buttonData.raw;
                packet >> y >> z >> x;

                petGuid[1] = packet.readBit();
                petGuid[0] = packet.readBit();
                petGuid[6] = packet.readBit();
                petGuid[7] = packet.readBit();
                petGuid[5] = packet.readBit();
                targetGuid[7] = packet.readBit();
                petGuid[2] = packet.readBit();
                petGuid[3] = packet.readBit();
                targetGuid[6] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[2] = packet.readBit();
                targetGuid[5] = packet.readBit();
                petGuid[4] = packet.readBit();
                targetGuid[4] = packet.readBit();
                targetGuid[1] = packet.readBit();

                packet.readByteSeq(petGuid[7]);
                packet.readByteSeq(petGuid[6]);
                packet.readByteSeq(petGuid[1]);
                packet.readByteSeq(petGuid[2]);
                packet.readByteSeq(petGuid[5]);
                packet.readByteSeq(petGuid[4]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(petGuid[3]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(petGuid[0]);

                guid = petGuid;
                targetguid = targetGuid.getRawGuid();

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> buttonData.raw >> targetguid;
                guid.init(unpacked_guid);

                return true;
            }

            return false;
        }
    };
}
