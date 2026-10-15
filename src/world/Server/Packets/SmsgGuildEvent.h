/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Guild/GuildDefinitions.hpp"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgGuildEvent : public ManagedPacket
    {
    public:
        uint8_t guildEvent;
        std::vector<std::string> text;
        uint8_t textCount = 0;
        uint64_t guid;

        // 5.x and later: the bank money after a bank event
        uint64_t bankMoney = 0;

        SmsgGuildEvent() : SmsgGuildEvent(0, {}, 0)
        {
        }

        SmsgGuildEvent(uint8_t guildEvent, std::vector<std::string> text, uint64_t guid) :
            ManagedPacket(SMSG_GUILD_EVENT, 0),
            guildEvent(guildEvent),
            text(text),
            textCount(static_cast<uint8_t>(text.size())),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 1 + textCount + (guid ? 8 : 0);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // the bank events have own packets, the other events keep the common layout
                const auto nameAt = [&](size_t index) { return index < text.size() ? text[index] : std::string(); };
                switch (guildEvent)
                {
                    case GE_BANK_TAB_PURCHASED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_TAB_ADDED);
                        return true;
                    }
                    case GE_BANK_TAB_UPDATED:
                    {
                        // icon and name lengths, name, tab, icon
                        packet.setOpcode(SMSG_GUILD_EVENT_TAB_MODIFIED);
                        packet.writeBits(static_cast<uint32_t>(nameAt(2).length()), 9);
                        packet.writeBits(static_cast<uint32_t>(nameAt(1).length()), 7);
                        packet.flushBits();
                        packet.writeString(nameAt(1));
                        packet << uint32_t(std::atoi(nameAt(0).c_str()));
                        packet.writeString(nameAt(2));
                        return true;
                    }
                    case GE_BANK_TAB_AND_MONEY_UPDATED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_BANK_MONEY_CHANGED);
                        packet << uint64_t(bankMoney);
                        return true;
                    }
                    default:
                        break;
                }
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 6.x and later: one packet per event; the names come with the realm and the player of the event
                const uint32_t realmAddress = m_protocol.getVirtualRealmAddress();
                const auto nameAt = [&](size_t index) { return index < text.size() ? text[index] : std::string(); };
                const auto guidOfName = [](std::string const& name) -> WoWGuid
                {
                    if (const auto characterInfo = sObjectMgr.getCachedCharacterInfoByName(name))
                        return WoWGuid(characterInfo->guid, 0, HIGHGUID_TYPE_PLAYER);
                    return WoWGuid();
                };
                const auto writeNamedPlayer = [&](WoWGuid const& playerGuid, std::string const& name)
                {
                    packet << playerGuid.toGuid128(m_protocol.realmId, 0);
                    packet << uint32_t(realmAddress);
                    packet.writeString(name);
                };

                switch (guildEvent)
                {
                    case GE_MOTD:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_MOTD);
                        packet.writeBits(static_cast<uint32_t>(nameAt(0).length()), m_protocol.isWoD() || m_protocol.isLegion() ? 10 : 11);
                        packet.flushBits();
                        packet.writeString(nameAt(0));
                        return true;
                    }
                    case GE_JOINED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_PLAYER_JOINED);
                        packet << WoWGuid(guid).toGuid128(m_protocol.realmId, 0);
                        packet << uint32_t(realmAddress);
                        packet.writeBits(static_cast<uint32_t>(nameAt(0).length()), 6);
                        packet.flushBits();
                        packet.writeString(nameAt(0));
                        return true;
                    }
                    case GE_LEFT:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_PLAYER_LEFT);
                        packet.writeBit(false);                         // removed
                        packet.writeBits(static_cast<uint32_t>(nameAt(0).length()), 6);
                        packet.flushBits();
                        writeNamedPlayer(WoWGuid(guid), nameAt(0));
                        return true;
                    }
                    case GE_REMOVED:
                    {
                        // the removed member, then the officer
                        packet.setOpcode(SMSG_GUILD_EVENT_PLAYER_LEFT);
                        packet.writeBit(true);                          // removed
                        packet.writeBits(static_cast<uint32_t>(nameAt(0).length()), 6);
                        packet.flushBits();
                        packet.writeBits(static_cast<uint32_t>(nameAt(1).length()), 6);
                        packet.flushBits();
                        writeNamedPlayer(guidOfName(nameAt(1)), nameAt(1));
                        writeNamedPlayer(guidOfName(nameAt(0)), nameAt(0));
                        return true;
                    }
                    case GE_LEADER_CHANGED:
                    {
                        // the old leader, then the new one; 6.x names the new leader first
                        const std::string oldName = nameAt(0);
                        const std::string newName = nameAt(1);
                        const WoWGuid oldGuid = guidOfName(oldName);
                        const WoWGuid newGuid = guidOfName(newName);

                        packet.setOpcode(SMSG_GUILD_EVENT_NEW_LEADER);
                        packet.writeBit(false);                         // self promoted
                        packet.writeBits(static_cast<uint32_t>((m_protocol.isWoD() ? newName : oldName).length()), 6);
                        packet.writeBits(static_cast<uint32_t>((m_protocol.isWoD() ? oldName : newName).length()), 6);
                        packet.flushBits();
                        packet << oldGuid.toGuid128(m_protocol.realmId, 0);
                        packet << uint32_t(realmAddress);
                        packet << newGuid.toGuid128(m_protocol.realmId, 0);
                        packet << uint32_t(realmAddress);
                        packet.writeString(m_protocol.isWoD() ? newName : oldName);
                        packet.writeString(m_protocol.isWoD() ? oldName : newName);
                        return true;
                    }
                    case GE_DISBANDED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_DISBANDED);
                        return true;
                    }
                    case GE_RANK_UPDATED:
                    case GE_RANK_CREATED:
                    case GE_RANK_DELETED:
                    case GE_RANK_ORDER_CHANGED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_RANKS_UPDATED);
                        return true;
                    }
                    case GE_SIGNED_ON:
                    case GE_SIGNED_OFF:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_PRESENCE_CHANGE);
                        packet << WoWGuid(guid).toGuid128(m_protocol.realmId, 0);
                        packet << uint32_t(realmAddress);
                        packet.writeBits(static_cast<uint32_t>(nameAt(0).length()), 6);
                        packet.writeBit(guildEvent == GE_SIGNED_ON);    // logged on
                        if (!m_protocol.isTheWarWithinOrLater())
                            packet.writeBit(false);                     // mobile
                        packet.flushBits();
                        packet.writeString(nameAt(0));
                        return true;
                    }
                    case GE_BANK_TAB_PURCHASED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_TAB_ADDED);
                        return true;
                    }
                    case GE_BANK_TAB_UPDATED:
                    {
                        // tab, name, icon
                        packet.setOpcode(SMSG_GUILD_EVENT_TAB_MODIFIED);
                        packet << int32_t(std::atoi(nameAt(0).c_str()));
                        packet.writeBits(static_cast<uint32_t>(nameAt(1).length()), 7);
                        packet.writeBits(static_cast<uint32_t>(nameAt(2).length()), 9);
                        packet.flushBits();
                        packet.writeString(nameAt(1));
                        packet.writeString(nameAt(2));
                        return true;
                    }
                    case GE_BANK_TAB_AND_MONEY_UPDATED:
                    {
                        packet.setOpcode(SMSG_GUILD_EVENT_BANK_MONEY_CHANGED);
                        packet << uint64_t(bankMoney);
                        return true;
                    }
                    default:
                        // promotions come with the rank change, the bank contents with the bank list
                        return false;
                }
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet << guildEvent << textCount;

                for (const auto& stringVars : text)
                    packet << stringVars.c_str();

                if (guid)
                    packet << guid;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
