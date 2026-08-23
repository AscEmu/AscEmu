/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgVoidStorageTransfer : public ManagedPacket
    {
    public:
        WoWGuid npcGuid;
        std::vector<WoWGuid> depositItemGuids;
        std::vector<WoWGuid> withdrawItemGuids;

        CmsgVoidStorageTransfer() :
            ManagedPacket(CMSG_VOID_STORAGE_TRANSFER, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                npcGuid[7] = packet.readBit();
                npcGuid[4] = packet.readBit();

                const uint32_t countDeposit = packet.readBits(24);
                if (countDeposit > 9)
                    return false;

                depositItemGuids.resize(countDeposit);
                for (uint32_t i = 0; i < countDeposit; ++i)
                {
                    depositItemGuids[i][0] = packet.readBit();
                    depositItemGuids[i][3] = packet.readBit();
                    depositItemGuids[i][6] = packet.readBit();
                    depositItemGuids[i][5] = packet.readBit();
                    depositItemGuids[i][4] = packet.readBit();
                    depositItemGuids[i][2] = packet.readBit();
                    depositItemGuids[i][1] = packet.readBit();
                    depositItemGuids[i][7] = packet.readBit();
                }

                const uint32_t countWithdraw = packet.readBits(24);
                if (countWithdraw > 9)
                    return false;

                withdrawItemGuids.resize(countWithdraw);
                for (uint32_t i = 0; i < countWithdraw; ++i)
                {
                    withdrawItemGuids[i][4] = packet.readBit();
                    withdrawItemGuids[i][0] = packet.readBit();
                    withdrawItemGuids[i][5] = packet.readBit();
                    withdrawItemGuids[i][7] = packet.readBit();
                    withdrawItemGuids[i][6] = packet.readBit();
                    withdrawItemGuids[i][1] = packet.readBit();
                    withdrawItemGuids[i][2] = packet.readBit();
                    withdrawItemGuids[i][3] = packet.readBit();
                }

                npcGuid[6] = packet.readBit();
                npcGuid[0] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[2] = packet.readBit();
                npcGuid[5] = packet.readBit();

                packet.flushBits();

                for (uint32_t i = 0; i < countDeposit; ++i)
                {
                    packet.readByteSeq(depositItemGuids[i][5]);
                    packet.readByteSeq(depositItemGuids[i][6]);
                    packet.readByteSeq(depositItemGuids[i][3]);
                    packet.readByteSeq(depositItemGuids[i][4]);
                    packet.readByteSeq(depositItemGuids[i][1]);
                    packet.readByteSeq(depositItemGuids[i][7]);
                    packet.readByteSeq(depositItemGuids[i][2]);
                    packet.readByteSeq(depositItemGuids[i][0]);
                }

                packet.readByteSeq(npcGuid[5]);

                for (uint32_t i = 0; i < countWithdraw; ++i)
                {
                    packet.readByteSeq(withdrawItemGuids[i][0]);
                    packet.readByteSeq(withdrawItemGuids[i][4]);
                    packet.readByteSeq(withdrawItemGuids[i][1]);
                    packet.readByteSeq(withdrawItemGuids[i][2]);
                    packet.readByteSeq(withdrawItemGuids[i][6]);
                    packet.readByteSeq(withdrawItemGuids[i][3]);
                    packet.readByteSeq(withdrawItemGuids[i][7]);
                    packet.readByteSeq(withdrawItemGuids[i][5]);
                }

                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(npcGuid[6]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                npcGuid[1] = packet.readBit();

                const uint32_t countDeposit = packet.readBits(26);
                if (countDeposit > 9)
                    return false;

                depositItemGuids.resize(countDeposit);
                for (uint32_t i = 0; i < countDeposit; ++i)
                {
                    depositItemGuids[i][4] = packet.readBit();
                    depositItemGuids[i][6] = packet.readBit();
                    depositItemGuids[i][7] = packet.readBit();
                    depositItemGuids[i][0] = packet.readBit();
                    depositItemGuids[i][1] = packet.readBit();
                    depositItemGuids[i][5] = packet.readBit();
                    depositItemGuids[i][3] = packet.readBit();
                    depositItemGuids[i][2] = packet.readBit();
                }

                npcGuid[2] = packet.readBit();
                npcGuid[0] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[4] = packet.readBit();

                const uint32_t countWithdraw = packet.readBits(26);
                if (countWithdraw > 9)
                    return false;

                withdrawItemGuids.resize(countWithdraw);
                for (uint32_t i = 0; i < countWithdraw; ++i)
                {
                    withdrawItemGuids[i][4] = packet.readBit();
                    withdrawItemGuids[i][7] = packet.readBit();
                    withdrawItemGuids[i][1] = packet.readBit();
                    withdrawItemGuids[i][0] = packet.readBit();
                    withdrawItemGuids[i][2] = packet.readBit();
                    withdrawItemGuids[i][3] = packet.readBit();
                    withdrawItemGuids[i][5] = packet.readBit();
                    withdrawItemGuids[i][6] = packet.readBit();
                }

                npcGuid[7] = packet.readBit();

                packet.flushBits();

                for (uint32_t i = 0; i < countDeposit; ++i)
                {
                    packet.readByteSeq(depositItemGuids[i][6]);
                    packet.readByteSeq(depositItemGuids[i][1]);
                    packet.readByteSeq(depositItemGuids[i][0]);
                    packet.readByteSeq(depositItemGuids[i][2]);
                    packet.readByteSeq(depositItemGuids[i][4]);
                    packet.readByteSeq(depositItemGuids[i][5]);
                    packet.readByteSeq(depositItemGuids[i][3]);
                    packet.readByteSeq(depositItemGuids[i][7]);
                }

                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(npcGuid[6]);

                for (uint32_t i = 0; i < countWithdraw; ++i)
                {
                    packet.readByteSeq(withdrawItemGuids[i][3]);
                    packet.readByteSeq(withdrawItemGuids[i][1]);
                    packet.readByteSeq(withdrawItemGuids[i][0]);
                    packet.readByteSeq(withdrawItemGuids[i][6]);
                    packet.readByteSeq(withdrawItemGuids[i][2]);
                    packet.readByteSeq(withdrawItemGuids[i][7]);
                    packet.readByteSeq(withdrawItemGuids[i][5]);
                    packet.readByteSeq(withdrawItemGuids[i][4]);
                }

                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[0]);

                return true;
            }

            return false;
        }
    };
}
