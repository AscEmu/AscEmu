/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlegroundPacketCommon.h"
#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class MsgPvpLogData : public ManagedPacket
    {
    public:
        PvpLogDataInput input;

        MsgPvpLogData() : MsgPvpLogData(PvpLogDataInput{})
        {
        }

        explicit MsgPvpLogData(PvpLogDataInput input) :
            ManagedPacket(MSG_PVP_LOG_DATA, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 10 * input.players.size() + 50;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            // Team ids are fixed protocol constants (0 = Alliance, 1 = Horde), not something AscEmu
            // needs a header dependency for.
            constexpr uint8_t hordeTeamId = 1;

            if (m_protocol.isMop())
            {
                // Cata split the old merged MSG_PVP_LOG_DATA opcode into a dedicated client/server pair;
                // the wire shape changed at the same time.
                packet.setOpcode(SMSG_PVP_LOG_DATA);

                uint32_t allianceCount = 0;
                uint32_t hordeCount = 0;
                for (const auto& p : input.players)
                {
                    if (p.bgTeam == hordeTeamId)
                        ++hordeCount;
                    else
                        ++allianceCount;
                }

                packet << uint8_t(allianceCount);
                packet << uint8_t(hordeCount);

                packet.writeBit(input.hasEnded);
                packet.writeBit(0); // Unk - some player stuff
                packet.writeBit(0); // isRated - arena rating display is not sent on Mop

                packet.writeBits(input.players.size(), 19);

                ByteBuffer buff;
                for (const auto& p : input.players)
                {
                    const WoWGuid guid(p.guid);

                    packet.writeBit(guid[6]);
                    packet.writeBit(p.bgTeam == hordeTeamId ? 0 : 1);
                    packet.writeBit(0); // Rating Change
                    packet.writeBit(guid[0]);
                    packet.writeBit(0); // MMR Change
                    packet.writeBit(guid[7]);
                    packet.writeBit(0); // PreMatch MMR
                    packet.writeBit(guid[3]);
                    packet.writeBit(0); // Prematch Rating
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);

                    buff << p.healingDone;
                    buff.writeByteSeq(guid[4]);
                    buff.writeByteSeq(guid[5]);
                    buff.writeByteSeq(guid[2]);

                    if (!input.isArena)
                    {
                        buff << p.deaths;
                        buff << p.honorableKills;
                        buff << uint32_t(p.bonusHonor / 100);
                    }

                    buff.writeByteSeq(guid[3]);
                    buff << p.damageDone;
                    buff << p.killingBlows;
                    buff.writeByteSeq(guid[1]);
                    buff.writeByteSeq(guid[6]);
                    buff.writeByteSeq(guid[7]);
                    buff.writeByteSeq(guid[0]);
                    buff << int32_t(0); // active talent spec - not modelled by AscEmu

                    packet.writeBits(input.fieldCount, 22);
                    for (uint32_t x = 0; x < input.fieldCount && x < 5; ++x)
                        buff << p.miscData[x];

                    packet.writeBit(1); // player is in world
                    packet.writeBit(!input.isArena);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                }

                packet.flushBits();
                packet.append(buff);

                if (input.hasEnded)
                    packet << uint8_t(input.winningTeam);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet.setOpcode(SMSG_PVP_LOG_DATA);

                packet.writeBit(input.rated);
                packet.writeBit(input.isArena);

                if (input.isArena)
                {
                    for (uint8_t i = 0; i < 2; ++i)
                        packet.writeBits(0, 8); // arena team name length - not modelled by AscEmu
                }

                const size_t countPos = packet.bitwpos();
                packet.writeBits(0, 21); // player count, patched below

                ByteBuffer buff;
                for (const auto& p : input.players)
                {
                    const WoWGuid guid(p.guid);

                    packet.writeBit(0); // Unk 1
                    packet.writeBit(0); // Unk 2
                    packet.writeBit(guid[2]);
                    packet.writeBit(input.isArena ? 0 : 1);
                    packet.writeBit(0); // Unk 4
                    packet.writeBit(0); // Unk 5
                    packet.writeBit(0); // Unk 6
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(p.bgTeam);
                    packet.writeBit(guid[7]);

                    buff << p.healingDone;
                    buff << p.damageDone;

                    if (!input.isArena)
                    {
                        buff << uint32_t(p.bonusHonor / 100);
                        buff << p.deaths;
                        buff << p.honorableKills;
                    }

                    buff.writeByteSeq(guid[4]);
                    buff << p.killingBlows;
                    buff.writeByteSeq(guid[5]);
                    buff.writeByteSeq(guid[1]);
                    buff.writeByteSeq(guid[6]);
                    buff << int32_t(0); // primary talent tree - not modelled by AscEmu

                    if (input.isArena)
                    {
                        packet.writeBits(0, 24); // arena scores carry no objectives block
                    }
                    else
                    {
                        packet.writeBits(input.fieldCount, 24);
                        for (uint32_t x = 0; x < input.fieldCount && x < 5; ++x)
                            buff << p.miscData[x];
                    }

                    packet.writeBit(guid[4]);

                    buff.writeByteSeq(guid[0]);
                    buff.writeByteSeq(guid[3]);
                    buff.writeByteSeq(guid[7]);
                    buff.writeByteSeq(guid[2]);
                }

                packet.putBits(countPos, input.players.size(), 21);
                packet.writeBit(input.hasEnded);

                if (input.rated)
                {
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        packet << uint32_t(0); // matchmaker rating - not modelled by AscEmu
                        packet << uint32_t(0); // rating lost - not modelled by AscEmu
                        packet << uint32_t(0); // rating won - not modelled by AscEmu
                    }
                }

                packet.flushBits();
                packet.append(buff);

                if (input.isArena)
                {
                    for (uint8_t i = 0; i < 2; ++i)
                        packet.writeString(""); // arena team name - not modelled by AscEmu
                }

                uint32_t allianceCount = 0;
                uint32_t hordeCount = 0;
                for (const auto& p : input.players)
                {
                    if (p.bgTeam == hordeTeamId)
                        ++hordeCount;
                    else
                        ++allianceCount;
                }

                packet << uint8_t(hordeCount);

                if (input.hasEnded)
                    packet << uint8_t(input.winningTeam);

                packet << uint8_t(allianceCount);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                if (input.isArena)
                {
                    if (!input.hasEnded)
                        return true; // matches original: packet is still sent with no payload past the opcode

                    packet << uint8_t(1);

                    if (!input.rated)
                    {
                        packet << uint32_t(0); //uint32_t(negative rating)
                        packet << uint32_t(0); //uint32_t(positive rating)
                        packet << uint32_t(0); //uint32_t(0)[<-this is the new field in 3.1]
                        packet << uint8_t(0);  //name if available / which is a null-terminated string, and we send an uint8_t(0), so we provide a zero length name string
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint8_t(0);
                    }
                    else
                    {
                        if (input.arenaTeamExists[0])
                        {
                            packet << uint32_t(0);
                            packet << uint32_t(3000 + input.deltaRating[0]);
                            packet << uint32_t(0);
                            packet << uint8_t(0);
                        }
                        else
                        {
                            packet << uint32_t(0);
                            packet << uint32_t(0);
                            packet << uint32_t(0);
                            packet << uint8_t(0);
                        }

                        if (input.arenaTeamExists[1])
                        {
                            packet << uint32_t(0);
                            packet << uint32_t(3000 + input.deltaRating[1]);
                            packet << uint32_t(0);
                            packet << uint8_t(0);
                        }
                        else
                        {
                            packet << uint32_t(0);
                            packet << uint32_t(0);
                            packet << uint32_t(0);
                            packet << uint8_t(0);
                        }
                    }

                    packet << uint8_t(1);
                    packet << uint8_t(input.winningTeam);

                    packet << uint32_t(input.players.size());

                    for (const auto& p : input.players)
                    {
                        packet << uint64_t(p.guid);
                        packet << p.killingBlows;
                        packet << uint8_t(p.bgTeam);
                        packet << p.damageDone;
                        packet << p.healingDone;
                        packet << uint32_t(0);
                    }
                }
                else
                {
                    packet << uint8_t(0);
                    if (input.hasEnded)
                    {
                        packet << uint8_t(1);
                        packet << uint8_t(input.winningTeam ? 0 : 1);
                    }
                    else
                    {
                        packet << uint8_t(0);      // If the game has ended - this will be 1
                    }

                    packet << uint32_t(input.players.size());

                    for (const auto& p : input.players)
                    {
                        packet << uint64_t(p.guid);

                        packet << p.killingBlows;
                        packet << p.honorableKills;
                        packet << p.deaths;
                        packet << p.bonusHonor;
                        packet << p.damageDone;
                        packet << p.healingDone;

                        packet << input.fieldCount;
                        for (uint32_t x = 0; x < input.fieldCount; ++x)
                            packet << p.miscData[x];
                    }
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
