/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgBattlefieldStatus : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t status;
        uint32_t type;
        uint32_t instanceId;
        uint32_t time;
        uint32_t mapId;
        uint8_t ratedMatch;
        bool isArena;

        SmsgBattlefieldStatus() : SmsgBattlefieldStatus(WoWGuid(),0, 0, 0, 0, 0, 0, false)
        {
        }

        SmsgBattlefieldStatus(WoWGuid guid, uint32_t status, uint32_t type, uint32_t instanceId, uint32_t time, uint32_t mapId, uint8_t ratedMatch, bool isArena) :
            ManagedPacket(SMSG_BATTLEFIELD_STATUS, !status ? 8 + 4 : 4 + 8 + 1 + 1 + 4 + 1 + 4 + 4 + 4),
            guid(guid), status(status), type(type), instanceId(instanceId), time(time), mapId(mapId), ratedMatch(ratedMatch), isArena(isArena)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            if (!status)
            {
                packet << uint32_t(0) << uint64_t(0);
                return true;
            }

            uint8_t arenaType;
            switch (type)
            {
            case 4:
                arenaType = 2;
                break;
            case 5:
                arenaType = 3;
                break;
            case 6:
                arenaType = 5;
                break;
            default:
                arenaType = 0;
                break;
            }

            packet << uint32_t(0);          // Queueslot
            packet << arenaType;
            packet << uint8_t(isArena ? 0xE : 0x0);
            packet << uint32_t(isArena ? 6 : type);
            packet << uint16_t(0x1F90);
#if VERSION_STRING > TBC
            packet << uint8_t(0);           // 3.3.0 - minLevel
            packet << uint8_t(0);           // 3.3.0 - maxLevel
#endif
            packet << uint32_t(isArena ? 11 : instanceId);
            packet << uint8_t(isArena ? ratedMatch : 0);

            packet << status;
            switch (status)
            {
                case 1:                     // BGSTATUS_INQUEUE
                    packet << uint32_t(60);
                    packet << uint32_t(0);  // Time / Elapsed time
                    break;
                case 2:                     // BGSTATUS_READY
                    packet << mapId;
#if VERSION_STRING > TBC
                    packet << uint64_t(0);
#endif
                    packet << time;
                    break;
                case 3:                     // BGSTATUS_TIME
                    packet << mapId;
#if VERSION_STRING > TBC
                    packet << uint64_t(0);
#endif
                    packet << uint32_t(0);
                    packet << time;
                    if (isArena)
                        packet << uint8_t(0);
                    else
                        packet << uint8_t(1);
                    break;
            }
#endif

#if VERSION_STRING == Cata
            WoWGuid bgGuid(type, 0, HIGHGUID_TYPE_BATTLEGROUND);

            switch (status)
            {
                case 0:
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);

                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(type ? type : 1);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet << uint32_t(0);                    // queue slot
                    packet << uint32_t(0);                    // join time
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    break;
                }
                case 1:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_QUEUED);

                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(1);
                    packet.writeBit(0);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(bgGuid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(bgGuid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(ratedMatch);
                    packet.writeBit(0);
                    packet.writeBit(bgGuid[1]);

                    packet.flushBits();

                    packet.WriteByteSeq(guid[0]);
                    packet << uint32_t(isArena ? type : 1);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet << uint32_t(60);                   // wait time
                    packet.WriteByteSeq(bgGuid[7]);
                    packet.WriteByteSeq(bgGuid[1]);
                    packet.WriteByteSeq(bgGuid[2]);
                    packet << uint8_t(0);
                    packet.WriteByteSeq(bgGuid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint8_t(0);
                    packet.WriteByteSeq(bgGuid[6]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(bgGuid[0]);
                    packet << uint32_t(time);                 // join time
                    packet << uint32_t(0);                    // queue slot
                    packet << uint8_t(0);                     // minlevel
                    packet << uint32_t(time);                 // time in queue
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet << uint32_t(instanceId);
                    packet.WriteByteSeq(guid[4]);
                    break;
                }
                case 2:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION);

                    packet << uint32_t(instanceId);
                    packet << uint32_t(0);                    // time until closed
                    packet << uint8_t(0);
                    packet << uint32_t(0);                    // queue slot
                    packet << uint32_t(time);                 // join time
                    packet << uint8_t(0);                     // minlevel
                    packet << uint32_t(isArena ? type : 1);
                    packet << uint32_t(mapId);
                    packet << uint8_t(0);

                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(ratedMatch);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(bgGuid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(bgGuid[1]);
                    packet.writeBit(bgGuid[5]);
                    packet.writeBit(guid[0]);

                    packet.WriteByteSeq(bgGuid[6]);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet.WriteByteSeq(bgGuid[7]);
                    packet.WriteByteSeq(bgGuid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(bgGuid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(bgGuid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(bgGuid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    break;
                }
                case 3:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_ACTIVE);

                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(bgGuid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(isArena ? 1 : 0);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(bgGuid[5]);
                    packet.writeBit(ratedMatch);
                    packet.writeBit(guid[4]);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(bgGuid[4]);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);

                    packet.flushBits();

                    packet.WriteByteSeq(bgGuid[4]);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(bgGuid[1]);
                    packet.WriteByteSeq(bgGuid[6]);
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(bgGuid[7]);
                    packet.WriteByteSeq(guid[6]);

                    packet << uint32_t(0);                    // join time
                    packet << uint8_t(0);

                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);

                    packet << uint32_t(0);                    // queue slot
                    packet << uint8_t(0);
                    packet << uint32_t(isArena ? type : 1);
                    packet << uint32_t(mapId);
                    packet << uint8_t(0);                     // minlevel
                    packet << uint32_t(time);                 // elapsed time

                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(0);                    // remaining time

                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(bgGuid[2]);

                    packet << uint32_t(instanceId);

                    packet.WriteByteSeq(bgGuid[0]);
                    packet.WriteByteSeq(guid[7]);
                    break;
                }
            }
#endif

#if VERSION_STRING == Mop
            WoWGuid bgGuid(type, 0, HIGHGUID_TYPE_BATTLEGROUND);
            switch (status)
            {
                case 0:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS);

                    packet << uint32_t(time);                 // join time
                    packet << uint32_t(0);
                    packet << uint32_t(0);                    // queue slot

                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);

                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    break;
                }
                case 1:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_QUEUED);

                    packet.writeBit(bgGuid[1]);
                    packet.writeBit(bgGuid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(ratedMatch);
                    packet.writeBit(0);
                    packet.writeBit(1);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(0);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(bgGuid[4]);

                    packet.flushBits();

                    packet.WriteByteSeq(bgGuid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet << uint32_t(time);                 // wait time
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[0]);
                    packet << uint8_t(0);                     // minlevel
                    packet.WriteByteSeq(bgGuid[0]);
                    packet << uint32_t(0);                    // join time
                    packet << uint8_t(isArena ? type : 1);
                    packet.WriteByteSeq(bgGuid[1]);
                    packet << uint32_t(0);                    // time in queue
                    packet.WriteByteSeq(bgGuid[7]);
                    packet << uint32_t(0);                    // queueslot
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(bgGuid[6]);
                    packet << uint8_t(0);                     // maxlevel
                    packet.WriteByteSeq(bgGuid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[1]);
                    packet << uint32_t(instanceId);
                    packet.WriteByteSeq(guid[7]);
                    break;
                }
                case 2:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION);

                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(1);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(bgGuid[4]);
                    packet.writeBit(bgGuid[1]);
                    packet.writeBit(ratedMatch);
                    packet.writeBit(guid[2]);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(bgGuid[5]);

                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(bgGuid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint8_t(isArena ? type : 1);
                    packet << uint32_t(0);                    // queueslot
                    packet << uint32_t(instanceId);
                    packet.WriteByteSeq(bgGuid[6]);
                    packet.WriteByteSeq(bgGuid[7]);
                    packet << uint32_t(time);                 // join time
                    packet.WriteByteSeq(guid[7]);
                    packet << uint8_t(0);                     // maxlevel
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(bgGuid[2]);
                    packet.WriteByteSeq(bgGuid[4]);
                    packet << uint32_t(0);                    // time until close
                    packet << uint8_t(0);                     // minlevel
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(bgGuid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet << uint32_t(mapId);
                    break;
                }
                case 3:
                {
                    packet.Initialize(SMSG_BATTLEFIELD_STATUS_ACTIVE);

                    packet.writeBit(guid[0]);
                    packet.writeBit(bgGuid[3]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(bgGuid[2]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(bgGuid[5]);
                    packet.writeBit(bgGuid[1]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(0);                     // left early
                    packet.writeBit(guid[6]);
                    packet.writeBit(bgGuid[0]);
                    packet.writeBit(isArena ? 1 : 0);
                    packet.writeBit(bgGuid[6]);
                    packet.writeBit(bgGuid[7]);
                    packet.writeBit(bgGuid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(ratedMatch);

                    packet.flushBits();

                    packet.WriteByteSeq(guid[3]);
                    packet << uint32_t(0);                    // join time
                    packet << uint32_t(60);                   // remaining time
                    packet.WriteByteSeq(bgGuid[7]);
                    packet.WriteByteSeq(bgGuid[5]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(bgGuid[6]);
                    packet << uint32_t(time);                 // elapsed time
                    packet << uint8_t(0);                     // maxlevel
                    packet.WriteByteSeq(bgGuid[1]);
                    packet.WriteByteSeq(bgGuid[2]);
                    packet << uint32_t(0);                    // queueslot
                    packet.WriteByteSeq(guid[4]);
                    packet << uint8_t(0);                     // minlevel
                    packet.WriteByteSeq(guid[6]);
                    packet << uint32_t(mapId);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(bgGuid[4]);
                    packet << uint32_t(instanceId);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint8_t(isArena ? type : 1);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(bgGuid[3]);
                    packet.WriteByteSeq(bgGuid[0]);
                    break;
                }
            }
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
