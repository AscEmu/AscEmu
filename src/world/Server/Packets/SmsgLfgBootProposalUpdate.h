/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgLfgBootProposalUpdate : public ManagedPacket
    {
    public:
        uint8_t inProgress;
        uint8_t didVote;
        uint8_t agree;
        uint64_t victim;
        uint32_t votesNum;
        uint32_t agreeNum;
        uint32_t secsLeft;
        uint32_t votedNeeded;
        std::string reason;

        SmsgLfgBootProposalUpdate() : SmsgLfgBootProposalUpdate(0, 0, 0, 0, 0, 0, 0, 0, "")
        {
        }

        SmsgLfgBootProposalUpdate(uint8_t inProgress, uint8_t didVote, uint8_t agree, uint64_t victim, uint32_t votesNum,
            uint32_t agreeNum, uint32_t secsLeft, uint32_t votedNeeded, std::string reason) :
            ManagedPacket(SMSG_LFG_BOOT_PROPOSAL_UPDATE, 0),
            inProgress(inProgress),
            didVote(didVote),
            agree(agree),
            victim(victim),
            votesNum(votesNum),
            agreeNum(agreeNum),
            secsLeft(secsLeft),
            votedNeeded(votedNeeded),
            reason(std::move(reason))
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + 1 + 1 + 8 + 4 + 4 + 4 + 4 + reason.length(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid targetGuid = victim;
                const bool hasReason = !reason.empty();

                packet.writeBit(hasReason);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(didVote != 0);                             // MyVoteCompleted
                packet.writeBit(agreeNum >= votedNeeded);                  // VotePassed
                packet.writeBit(agree != 0);                               // MyVote
                packet.writeBit(targetGuid[6]);
                if (hasReason)
                    packet.writeBits(reason.length(), 8);
                packet.writeBit(inProgress != 0);                          // VoteInProgress
                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[5]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[4]);

                packet.flushBits();

                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[3]);
                packet.writeByteSeq(targetGuid[6]);
                packet << uint32_t(votedNeeded);                           // VotesNeeded
                packet << uint32_t(secsLeft);                              // TimeLeft
                if (hasReason)
                    packet.writeString(reason);
                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(targetGuid[0]);
                packet << uint32_t(agreeNum);                              // BootVotes
                packet.writeByteSeq(targetGuid[7]);
                packet << uint32_t(votesNum);                              // TotalVotes
                packet.writeByteSeq(targetGuid[1]);

                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << uint8_t(inProgress);                                 // Vote in progress
                packet << uint8_t(didVote);                                    // Did Vote
                packet << uint8_t(agree);                                      // Agree
                packet << uint64_t(victim);                                    // Victim GUID
                packet << uint32_t(votesNum);                                  // Total Votes
                packet << uint32_t(agreeNum);                                  // Agree Count
                packet << uint32_t(secsLeft);                                  // Time Left
                packet << uint32_t(votedNeeded);                               // Needed Votes
                packet << reason.c_str();                                      // Kick reason

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
