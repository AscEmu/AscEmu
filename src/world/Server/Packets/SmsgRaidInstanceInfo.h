/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Map/Maps/InstanceMgr.hpp"
#include "Utilities/Util.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgRaidInstanceInfo : public ManagedPacket
    {
    public:
        struct PermanentBind
        {
            InstanceSaved* save;
            uint8_t extendState;
        };

        std::vector<PermanentBind> permanentBinds;

        SmsgRaidInstanceInfo() : SmsgRaidInstanceInfo(std::vector<PermanentBind>{})
        {
        }

        explicit SmsgRaidInstanceInfo(std::vector<PermanentBind> permanentBinds) :
            ManagedPacket(SMSG_RAID_INSTANCE_INFO, 4),
            permanentBinds(std::move(permanentBinds))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + permanentBinds.size() * 20; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                uint32_t counter = 0;
                const size_t p_counter = packet.wpos();
                packet << uint32_t(counter);

                const auto now = ::Util::getTimeNow();

                for (const auto& bind : permanentBinds)
                {
                    InstanceSaved* save = bind.save;
                    packet << uint32_t(save->getMapId());

                    if (m_protocol.expansion > WoW::Expansion::_TBC && m_protocol.expansion <= WoW::Expansion::_Mop)
                    {
                        packet << uint32_t(save->getDifficulty());
                        packet << uint64_t(save->getInstanceId());
                        packet << uint8_t(bind.extendState != EXTEND_STATE_EXPIRED);
                        packet << uint8_t(bind.extendState == EXTEND_STATE_EXTENDED);

                        time_t nextReset = save->getResetTime();
                        if (bind.extendState == EXTEND_STATE_EXTENDED)
                            nextReset = sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime());

                        packet << uint32_t(nextReset - now);
                    }
                    else if (m_protocol.expansion <= WoW::Expansion::_TBC)
                    {
                        time_t nextReset = save->getResetTime();
                        if (bind.extendState == EXTEND_STATE_EXTENDED)
                            nextReset = sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime());

                        packet << uint32_t(nextReset - now);

                        packet << uint32_t(save->getInstanceId());
                        packet << uint32_t(counter);
                    }
                    else
                    {
                        return false;
                    }

                    ++counter;
                }

                packet.put<uint32_t>(p_counter, counter);

                return true;
            }
            else // Mop
            {
                uint32_t counter = 0;
                const size_t p_counter = packet.bitwpos();
                packet.writeBits(counter, 20);

                const auto now = ::Util::getTimeNow();

                ByteBuffer buffer;

                for (const auto& bind : permanentBinds)
                {
                    InstanceSaved* save = bind.save;
                    WoWGuid instanceGuid = uint64_t(save->getInstanceId());

                    packet.writeBit(instanceGuid[1]);
                    packet.writeBit(instanceGuid[2]);
                    packet.writeBit(instanceGuid[6]);

                    packet.writeBit(bind.extendState == EXTEND_STATE_EXTENDED);

                    packet.writeBit(instanceGuid[5]);
                    packet.writeBit(instanceGuid[4]);

                    packet.writeBit(bind.extendState != EXTEND_STATE_EXPIRED);

                    packet.writeBit(instanceGuid[7]);
                    packet.writeBit(instanceGuid[3]);

                    buffer.writeByteSeq(instanceGuid[7]);
                    buffer.writeByteSeq(instanceGuid[6]);
                    buffer.writeByteSeq(instanceGuid[4]);
                    buffer.writeByteSeq(instanceGuid[2]);
                    buffer.writeByteSeq(instanceGuid[0]);

                    time_t nextReset = save->getResetTime();
                    if (bind.extendState == EXTEND_STATE_EXTENDED)
                        nextReset = sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime());

                    buffer << uint32_t(nextReset - now);
                    buffer << uint32_t(0); //unk encounter?

                    buffer.writeByteSeq(instanceGuid[1]);

                    buffer << uint32_t(save->getMapId());
                    buffer << uint32_t(save->getDifficulty());

                    buffer.writeByteSeq(instanceGuid[3]);
                    buffer.writeByteSeq(instanceGuid[5]);

                    ++counter;
                }
                packet.flushBits();
                packet.putBits(p_counter, counter, 20);
                packet.append(buffer);

                return true;
            }
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
