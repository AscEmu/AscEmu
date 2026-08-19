/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Objects/Units/ThreatHandler.h"

namespace AscEmu::Packets
{
    class SmsgThreatUpdate : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::list<std::shared_ptr<ThreatReference>> threadList;

        SmsgThreatUpdate(WoWGuid guid) :
            ManagedPacket(SMSG_THREAT_UPDATE, 8 + (2 * 8)),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + ((threadList.size() + 2) * 8); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet.appendPackGuid(guid.getRawGuid());

                size_t countPos = packet.wpos();
                packet << uint32_t(0); // placeholder counter

                uint32_t count = 0;
                for (const auto& ref : threadList)
                {
                    if (!ref->isAvailable())
                        continue;

                    ByteBuffer packedGuidVictim2;
                    packedGuidVictim2.appendPackGuid(ref->getVictim()->getGuid());
                    packet.append(packedGuidVictim2);
                    packet << uint32_t(ref->getThreat() * 100);
                    ++count;
                }

                packet.put<uint32_t>(countPos, count);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[5]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);

                size_t countPos = packet.wpos();
                packet << uint32_t(0); // placeholder counter

                uint32_t count = 0;
                for (const auto& ref : threadList)
                {
                    if (!ref->isAvailable())
                        continue;

                    WoWGuid listGuid = ref->getVictim()->getGuid();
                    packet.writeBit(listGuid[2]);
                    packet.writeBit(listGuid[3]);
                    packet.writeBit(listGuid[6]);
                    packet.writeBit(listGuid[5]);
                    packet.writeBit(listGuid[1]);
                    packet.writeBit(listGuid[4]);
                    packet.writeBit(listGuid[0]);
                    packet.writeBit(listGuid[7]);
                    ++count;
                }

                packet.put<uint32_t>(countPos, count);

                packet.writeBit(guid[2]);
                packet.flushBits();

                for (const auto& ref : threadList)
                {
                    if (!ref->isAvailable())
                        continue;

                    WoWGuid listGuid = ref->getVictim()->getGuid();

                    packet.writeByteSeq(listGuid[6]);
                    packet.writeByteSeq(listGuid[7]);
                    packet.writeByteSeq(listGuid[0]);
                    packet.writeByteSeq(listGuid[1]);
                    packet.writeByteSeq(listGuid[2]);
                    packet.writeByteSeq(listGuid[5]);
                    packet.writeByteSeq(listGuid[3]);
                    packet.writeByteSeq(listGuid[4]);

                    packet << uint32_t(ref->getThreat() * 100);
                }

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[7]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
