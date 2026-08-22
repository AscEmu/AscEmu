/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLfgProposalResult : public ManagedPacket
    {
    public:
        uint32_t lfgGroupId;
        bool accept;

        CmsgLfgProposalResult() : CmsgLfgProposalResult(0, false)
        {
        }

        CmsgLfgProposalResult(uint32_t lfgGroupId, bool accept) :
            ManagedPacket(CMSG_LFG_PROPOSAL_RESULT, 5),
            lfgGroupId(lfgGroupId),
            accept(accept)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid guid1, guid2;

                packet >> lfgGroupId;
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                accept = packet.readBit();

                guid1[6] = packet.readBit();
                guid1[0] = packet.readBit();
                guid1[2] = packet.readBit();
                guid1[4] = packet.readBit();
                guid2[6] = packet.readBit();
                guid2[7] = packet.readBit();
                guid1[3] = packet.readBit();
                guid2[4] = packet.readBit();
                guid1[7] = packet.readBit();
                guid2[1] = packet.readBit();
                guid1[5] = packet.readBit();
                guid2[0] = packet.readBit();
                guid1[1] = packet.readBit();
                guid2[2] = packet.readBit();
                guid2[3] = packet.readBit();
                guid2[5] = packet.readBit();

                packet.readByteSeq(guid1[3]);
                packet.readByteSeq(guid1[6]);
                packet.readByteSeq(guid1[4]);
                packet.readByteSeq(guid1[1]);
                packet.readByteSeq(guid2[7]);
                packet.readByteSeq(guid2[0]);
                packet.readByteSeq(guid1[7]);
                packet.readByteSeq(guid2[6]);
                packet.readByteSeq(guid1[5]);
                packet.readByteSeq(guid2[3]);
                packet.readByteSeq(guid2[1]);
                packet.readByteSeq(guid2[5]);
                packet.readByteSeq(guid2[4]);
                packet.readByteSeq(guid1[0]);
                packet.readByteSeq(guid1[2]);
                packet.readByteSeq(guid2[2]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                WoWGuid guid1, guid2;

                packet >> lfgGroupId;
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                guid2[4] = packet.readBit();
                guid2[5] = packet.readBit();
                guid2[0] = packet.readBit();
                guid2[6] = packet.readBit();
                guid2[2] = packet.readBit();
                guid2[7] = packet.readBit();
                guid2[1] = packet.readBit();
                guid2[3] = packet.readBit();

                packet.readByteSeq(guid2[7]);
                packet.readByteSeq(guid2[4]);
                packet.readByteSeq(guid2[3]);
                packet.readByteSeq(guid2[2]);
                packet.readByteSeq(guid2[6]);
                packet.readByteSeq(guid2[0]);
                packet.readByteSeq(guid2[1]);
                packet.readByteSeq(guid2[5]);

                guid1[7] = packet.readBit();
                accept = packet.readBit();
                guid1[1] = packet.readBit();
                guid1[3] = packet.readBit();
                guid1[0] = packet.readBit();
                guid1[5] = packet.readBit();
                guid1[4] = packet.readBit();
                guid1[6] = packet.readBit();
                guid1[2] = packet.readBit();

                packet.readByteSeq(guid1[7]);
                packet.readByteSeq(guid1[1]);
                packet.readByteSeq(guid1[5]);
                packet.readByteSeq(guid1[6]);
                packet.readByteSeq(guid1[3]);
                packet.readByteSeq(guid1[4]);
                packet.readByteSeq(guid1[0]);
                packet.readByteSeq(guid1[2]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> lfgGroupId >> accept;
                return true;
            }

            return false;
        }
    };
}
