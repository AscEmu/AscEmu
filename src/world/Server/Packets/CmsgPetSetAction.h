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
    class CmsgPetSetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t srcSlot;
        PetActionButtonData srcButton;
        uint32_t dstSlot;
        PetActionButtonData dstButton;

        CmsgPetSetAction() : CmsgPetSetAction(0, 0, PetActionButtonData{ .raw = 0 })
        {
        }

        CmsgPetSetAction(uint64_t guid, uint32_t slot, PetActionButtonData buttonData) :
            ManagedPacket(CMSG_PET_SET_ACTION, 8),
            guid(guid),
            srcSlot(slot),
            srcButton(buttonData),
            dstSlot(0),
            dstButton{ .raw = 0 }
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;
                guid.init(unpacked_guid);

                // Swapping slots
                if (packet.size() == 24)
                {
                    packet >> dstSlot;
                    packet >> dstButton.raw;
                }

                packet >> srcSlot;
                packet >> srcButton.raw;

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> srcSlot;
                packet >> srcButton.raw;

                guid[1] = packet.readBit();
                guid[0] = packet.readBit();
                guid[5] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                guid[7] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                return true;
            }

            return false;
        }
    };
}
