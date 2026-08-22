/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgLfgProposalUpdatePlayer
    {
        uint32_t role = 0;
        uint8_t isSelfPlayer = 0;
        uint8_t inDungeon = 0;
        uint8_t sameGroup = 0;
        uint8_t answered = 0;
        uint8_t accepted = 0;
    };

    class SmsgLfgProposalUpdate : public ManagedPacket
    {
    public:
        uint32_t dungeonId;
        uint8_t state;
        uint32_t proposalId;
        uint32_t completedEncounters;
        uint8_t isSameDungeon;
        std::vector<SmsgLfgProposalUpdatePlayer> players;
        uint64_t queueGuid;
        uint64_t playerGuid;

        SmsgLfgProposalUpdate() : SmsgLfgProposalUpdate(0, 0, 0, 0, 0, {}, 0, 0)
        {
        }

        SmsgLfgProposalUpdate(uint32_t dungeonId, uint8_t state, uint32_t proposalId, uint32_t completedEncounters, uint8_t isSameDungeon,
            std::vector<SmsgLfgProposalUpdatePlayer> players, uint64_t queueGuid = 0, uint64_t playerGuid = 0) :
            ManagedPacket(SMSG_LFG_PROPOSAL_UPDATE, 0),
            dungeonId(dungeonId),
            state(state),
            proposalId(proposalId),
            completedEncounters(completedEncounters),
            isSameDungeon(isSameDungeon),
            players(std::move(players)),
            queueGuid(queueGuid),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 1 + 4 + 4 + 1 + 1 + 8 + 8 + players.size() * (4 + 1 + 1 + 1 + 1 + 1);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid queueGuidObject = queueGuid;
                WoWGuid slotGuid = uint64_t(dungeonId) | (uint64_t(0x1F45) << 48);
                ByteBuffer roleData;

                packet.writeBit(slotGuid[6]);
                packet.writeBit(slotGuid[0]);
                packet.writeBit(queueGuidObject[1]);
                packet.writeBit(queueGuidObject[7]);
                packet.writeBit(queueGuidObject[5]);
                packet.writeBit(slotGuid[5]);
                packet.writeBit(queueGuidObject[4]);
                packet.writeBit(isSameDungeon != 0);                                 // ProposalSilent
                packet.writeBit(slotGuid[2]);
                packet.writeBit(queueGuidObject[6]);
                packet.writeBit(slotGuid[3]);
                packet.writeBit(slotGuid[7]);
                packet.writeBit(queueGuidObject[3]);
                packet.writeBits(players.size(), 21);                                // Players

                for (const auto& proposalPlayer : players)
                {
                    packet.writeBit(proposalPlayer.inDungeon);                       // MyParty
                    packet.writeBit(proposalPlayer.isSelfPlayer);                    // Me
                    packet.writeBit(proposalPlayer.answered);                        // Responded
                    packet.writeBit(proposalPlayer.accepted);                        // Accepted
                    packet.writeBit(proposalPlayer.sameGroup);                       // SameParty

                    roleData << uint32_t(proposalPlayer.role);
                }

                packet.writeBit(queueGuidObject[2]);
                packet.writeBit(slotGuid[4]);
                packet.writeBit(false);                                              // Unk
                packet.writeBit(queueGuidObject[0]);
                packet.writeBit(slotGuid[1]);

                packet.flushBits();

                packet.writeByteSeq(slotGuid[1]);
                packet.writeByteSeq(queueGuidObject[4]);
                packet.writeByteSeq(slotGuid[4]);
                packet.writeByteSeq(queueGuidObject[7]);
                packet.writeByteSeq(queueGuidObject[2]);
                packet.writeByteSeq(queueGuidObject[0]);
                packet << uint32_t(dungeonId);                                       // Slot
                packet << uint8_t(state);                                            // State
                packet << uint32_t(0);                                               // Id (LFG queue id - not tracked)
                packet.writeByteSeq(slotGuid[6]);
                packet << uint32_t(proposalId);                                      // ProposalID
                packet.writeByteSeq(queueGuidObject[5]);
                packet.writeByteSeq(queueGuidObject[3]);
                packet << uint32_t(0);                                               // UnixTime (queue join time - not tracked)
                packet.writeByteSeq(slotGuid[5]);
                packet.writeByteSeq(queueGuidObject[6]);
                packet.append(roleData);
                packet << uint32_t(completedEncounters);                             // CompletedMask
                packet.writeByteSeq(queueGuidObject[7]);
                packet.writeByteSeq(queueGuidObject[1]);
                packet.writeByteSeq(slotGuid[0]);
                packet.writeByteSeq(slotGuid[2]);
                packet << uint32_t(3);                                               // Type
                packet.writeByteSeq(slotGuid[3]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                WoWGuid guid1 = playerGuid;
                WoWGuid guid2 = queueGuid;

                packet << uint32_t(0);                                                // Join time - not tracked
                packet << uint32_t(completedEncounters);                              // Encounters done
                packet << uint32_t(0);                                                // Queue Id - not tracked
                packet << uint32_t(3);                                                // Type
                packet << uint32_t(dungeonId);                                        // Dungeon
                packet << uint32_t(proposalId);                                       // Proposal Id
                packet << uint8_t(state);                                             // State

                packet.writeBit(guid2[4]);
                packet.writeBit(guid1[3]);
                packet.writeBit(guid1[7]);
                packet.writeBit(guid1[0]);
                packet.writeBit(guid2[1]);
                packet.writeBit(isSameDungeon != 0);                                  // Silent
                packet.writeBit(guid1[4]);
                packet.writeBit(guid1[5]);
                packet.writeBit(guid2[3]);
                packet.writeBits(players.size(), 23);
                packet.writeBit(guid2[7]);

                for (const auto& proposalPlayer : players)
                {
                    packet.writeBit(proposalPlayer.inDungeon);
                    packet.writeBit(proposalPlayer.sameGroup);
                    packet.writeBit(proposalPlayer.accepted);
                    packet.writeBit(proposalPlayer.answered);
                    packet.writeBit(proposalPlayer.isSelfPlayer);
                }

                packet.writeBit(guid2[5]);
                packet.writeBit(guid1[6]);
                packet.writeBit(guid2[2]);
                packet.writeBit(guid2[6]);
                packet.writeBit(guid1[2]);
                packet.writeBit(guid1[1]);
                packet.writeBit(guid2[0]);
                packet.flushBits();

                packet.writeByteSeq(guid1[5]);
                packet.writeByteSeq(guid2[3]);
                packet.writeByteSeq(guid2[6]);
                packet.writeByteSeq(guid1[6]);
                packet.writeByteSeq(guid1[0]);
                packet.writeByteSeq(guid2[5]);
                packet.writeByteSeq(guid1[1]);

                for (const auto& proposalPlayer : players)
                    packet << uint32_t(proposalPlayer.role);

                packet.writeByteSeq(guid2[7]);
                packet.writeByteSeq(guid1[4]);
                packet.writeByteSeq(guid2[0]);
                packet.writeByteSeq(guid2[1]);
                packet.writeByteSeq(guid1[2]);
                packet.writeByteSeq(guid1[7]);
                packet.writeByteSeq(guid2[2]);
                packet.writeByteSeq(guid1[3]);
                packet.writeByteSeq(guid2[4]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << uint32_t(dungeonId);                                        // Dungeon
                packet << uint8_t(state);                                             // Result state
                packet << uint32_t(proposalId);                                       // Internal Proposal ID
                packet << uint32_t(completedEncounters);                              // Bosses killed
                packet << uint8_t(isSameDungeon);                                     // Silent (show client window)
                packet << uint8_t(players.size());                                    // Group size

                for (const auto& proposalPlayer : players)
                {
                    packet << uint32_t(proposalPlayer.role);                          // Role
                    packet << uint8_t(proposalPlayer.isSelfPlayer);                   // Self player
                    packet << uint8_t(proposalPlayer.inDungeon);                      // In dungeon (silent) / Not in dungeon
                    packet << uint8_t(proposalPlayer.sameGroup);                      // Same Group than player / Not same group
                    packet << uint8_t(proposalPlayer.answered);                       // Answered
                    packet << uint8_t(proposalPlayer.accepted);                       // Accepted
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
