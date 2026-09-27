/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Chat/ChatDefines.hpp"
#include "Server/World.h"

#include <cstdint>
#include <string>
#include <utility>

namespace AscEmu::Packets
{
    struct SystemMessagePacket
    {
        SystemMessagePacket(std::string msg) : message(std::move(msg)) {}

        uint8_t type = CHAT_MSG_SYSTEM;
        uint32_t language = LANG_UNIVERSAL;
        uint64_t guid = 0;
        uint32_t unk = 0;
        uint64_t guid2 = 0;
        std::string message;
        uint8_t flag = 0;
    };

    class SmsgMessageChat : public ManagedPacket
    {
    public:
        uint8_t type;
        uint32_t language;
        uint8_t flag;
        std::string message;

        WoWGuid senderGuid;
        std::string senderName;
        WoWGuid receiverGuid;
        std::string receiverName;
        uint32_t achievementId = 0;
        WoWGuid groupGuid;
        WoWGuid guildGuid;
        uint16_t senderMapId = 0;
        uint16_t receiverMapId = 0;

        SmsgMessageChat() : SmsgMessageChat(0, 0, 0, "", 0, "", 0, "", 0)
        {
        }

        SmsgMessageChat(uint8_t type, uint32_t language, uint8_t flag, std::string message, uint64_t senderGuid = 0, std::string senderName = "", uint64_t receiverGuid = 0, std::string receiverName = "", uint32_t achievementId = 0, uint64_t groupGuid = 0, uint64_t guildGuid = 0, uint16_t senderMapId = 0, uint16_t receiverMapId = 0) :
            ManagedPacket(SMSG_MESSAGECHAT, 1 + 4 + 8 + 4 + 8 + (message.length() + 1) + 1),
            type(type),
            language(language),
            flag(flag),
            message(message),
            senderGuid(senderGuid),
            senderName(senderName),
            receiverGuid(receiverGuid),
            receiverName(receiverName),
            achievementId(achievementId),
            groupGuid(groupGuid),
            guildGuid(guildGuid),
            senderMapId(senderMapId),
            receiverMapId(receiverMapId)
        {
        }

        SmsgMessageChat(SystemMessagePacket sysMsg) :
            ManagedPacket(SMSG_MESSAGECHAT, 1 + 4 + 8 + 4 + 8 + (sysMsg.message.length() + 1) + 1),
            type(sysMsg.type),
            language(sysMsg.language),
            flag(sysMsg.flag),
            message(sysMsg.message),
            senderGuid(sysMsg.guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::Forever)
            {
                const auto toModernGuid = [this](WoWGuid const& guid, uint16_t mapId)
                {
                    if (!guid)
                        return WoWGuid::createModernEmpty();

                    // Player/item/guild GUIDs are realm-scoped. World objects must
                    // use exactly the same realm + map identity as ObjectUpdate.
                    switch (guid.getHighType())
                    {
                        case HighGuid::Unit:
                        case HighGuid::Pet:
                        case HighGuid::Vehicle:
                        case HighGuid::GameObject:
                        case HighGuid::DynamicObject:
                        case HighGuid::AreaTrigger:
                        case HighGuid::Corpse:
                            return WoWGuid::createModernFromLegacy(
                                guid.getRawGuid(), worldConfig.battleNetComm.realmId, mapId, 0);
                        default:
                            return WoWGuid::createModernFromLegacy(guid.getRawGuid(), m_protocol.realmId);
                    }
                };

                const auto appendModernGuid = [&packet](WoWGuid const& guid)
                {
                    const auto packedGuid = guid.packModern();
                    packet.append(packedGuid.data(), packedGuid.size());
                };

                const WoWGuid modernSenderGuid = toModernGuid(senderGuid, senderMapId);
                const WoWGuid modernSenderGuildGuid = toModernGuid(guildGuid, 0);
                const WoWGuid modernSenderWowAccount = WoWGuid::createModernEmpty();
                const WoWGuid convertedReceiverGuid = toModernGuid(receiverGuid, receiverMapId);
                const WoWGuid modernReceiverGuid = convertedReceiverGuid ? convertedReceiverGuid : modernSenderGuid;

                const std::string channelName = type == CHAT_MSG_CHANNEL ? receiverName : std::string{};
                const std::string targetName = type == CHAT_MSG_CHANNEL ? std::string{} : receiverName;
                const uint32_t virtualRealmAddress = m_protocol.getVirtualRealmAddress();

                // Forever 1.60.1.70009 SMSG_CHAT (0x004B0001).
                packet << type;
                packet << language;
                appendModernGuid(modernSenderGuid);
                appendModernGuid(modernSenderGuildGuid);
                appendModernGuid(modernSenderWowAccount);
                appendModernGuid(modernReceiverGuid);
                packet << virtualRealmAddress; // target virtual realm address
                packet << virtualRealmAddress; // sender virtual realm address
                packet << static_cast<int32_t>(achievementId);
                packet << static_cast<uint32_t>(flag);
                packet << 0.0f;                    // display time
                packet << static_cast<int32_t>(0); // spell id

                packet.writeBits(senderName.size(), 11);
                packet.writeBits(targetName.size(), 11);
                packet.writeBits(0, 5); // addon prefix
                packet.writeBits(channelName.size(), 7);
                packet.writeBits(message.size(), 12);
                packet.writeBit(false); // hide chat log
                packet.writeBit(false); // fake sender name
                packet.writeBit(false); // BroadcastTextID optional
                packet.writeBit(false); // ChannelGUID optional
                packet.writeBit(false); // EncounterEventID optional
                packet.flushBits();

                if (!senderName.empty())
                    packet.writeString(senderName);
                if (!targetName.empty())
                    packet.writeString(targetName);
                if (!channelName.empty())
                    packet.writeString(channelName);
                if (!message.empty())
                    packet.writeString(message);

                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                // same for all chat types
                packet << type << language << senderGuid.getRawGuid() << uint32_t(0);
                switch (type)
                {
                    case CHAT_MSG_MONSTER_SAY:
                    case CHAT_MSG_MONSTER_PARTY:
                    case CHAT_MSG_MONSTER_YELL:
                    case CHAT_MSG_MONSTER_WHISPER:
                    case CHAT_MSG_MONSTER_EMOTE:
                    case CHAT_MSG_RAID_BOSS_EMOTE:
                    case CHAT_MSG_WHISPER_MOB:
                    {
                        packet << uint32_t(senderName.length() + 1) << senderName;
                        packet << receiverGuid.getRawGuid();
                        if (receiverGuid && !receiverGuid.isPlayer() && !receiverGuid.isPet() && type != CHAT_MSG_WHISPER_MOB)
                        {
                            packet << uint32_t(receiverName.length() + 1);
                            packet << receiverName;
                        }
                        packet << uint32_t(message.length() + 1) << message << flag;
                    } break;
                    case CHAT_MSG_BG_EVENT_NEUTRAL:
                    case CHAT_MSG_BG_EVENT_ALLIANCE:
                    case CHAT_MSG_BG_EVENT_HORDE:
                    {
                        packet << receiverGuid.getRawGuid();
                        if (receiverGuid && !receiverGuid.isPlayer())
                        {
                            packet << uint32_t(receiverName.length() + 1);
                            packet << receiverName;
                        }
                        packet << uint32_t(message.length() + 1) << message << flag;
                    } break;
                    case CHAT_MSG_ACHIEVEMENT:
                    case CHAT_MSG_GUILD_ACHIEVEMENT:
                    {
                        packet << receiverGuid;
                        packet << uint32_t(message.length() + 1) << message << flag;
                        packet << achievementId;
                    } break;
                    default:
                    {
                        if (type == CHAT_MSG_CHANNEL)
                        {
                            packet << receiverName; //channel name
                        }
                        packet << receiverGuid.getRawGuid();
                        packet << uint32_t(message.length() + 1) << message << flag;
                    } break;
                }
            }
            else // Mop
            {
                bool hasSenderName = false;
                bool hasReceiverName = false;
                bool hasChannelName = false;
                bool hasLanguage = language > 0;
                bool hasAchievement = (type == CHAT_MSG_ACHIEVEMENT || type == CHAT_MSG_GUILD_ACHIEVEMENT) && achievementId;
                bool isAddon = false;
                bool hasGroupGuid = false;
                bool hasGuildGuid = false;

                switch (type)
                {
                    case CHAT_MSG_MONSTER_SAY:
                    case CHAT_MSG_MONSTER_PARTY:
                    case CHAT_MSG_MONSTER_YELL:
                    case CHAT_MSG_MONSTER_WHISPER:
                    case CHAT_MSG_MONSTER_EMOTE:
                    case CHAT_MSG_RAID_BOSS_EMOTE:
                    case CHAT_MSG_WHISPER_MOB:
                    {
                        hasSenderName = true;
                        if (receiverGuid && !receiverGuid.isPlayer() && !receiverGuid.isPet() && type != CHAT_MSG_WHISPER_MOB)
                            hasReceiverName = true;
                    } break;
                    case CHAT_MSG_BG_EVENT_NEUTRAL:
                    case CHAT_MSG_BG_EVENT_ALLIANCE:
                    case CHAT_MSG_BG_EVENT_HORDE:
                    {
                        if (receiverGuid && !receiverGuid.isPlayer())
                            hasReceiverName = true;
                    } break;
                    case CHAT_MSG_CHANNEL:
                    {
                        hasChannelName = true;
                        hasSenderName = true;
                    } break;
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_PARTY_LEADER:
                    case CHAT_MSG_RAID:
                    case CHAT_MSG_RAID_LEADER:
                    case CHAT_MSG_RAID_WARNING:
                        hasGroupGuid = true;
                        break;
                    case CHAT_MSG_GUILD:
                    case CHAT_MSG_OFFICER:
                    case CHAT_MSG_GUILD_ACHIEVEMENT:
                        hasGuildGuid = true;
                        break;
                    default:
                        break;
                }

                const WoWGuid effectiveGroupGuid = hasGroupGuid ? groupGuid : WoWGuid(uint64_t(0));
                const WoWGuid effectiveGuildGuid = hasGuildGuid ? guildGuid : WoWGuid(uint64_t(0));

                packet.writeBit(!hasSenderName);
                packet.writeBit(0);     // hide chatlog

                if (hasSenderName)
                    packet.writeBits(senderName.length(), 11);

                packet.writeBit(0);
                packet.writeBit(!hasChannelName);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(!flag);
                packet.writeBit(1);

                packet.writeBit(effectiveGroupGuid[0]);
                packet.writeBit(effectiveGroupGuid[1]);
                packet.writeBit(effectiveGroupGuid[5]);
                packet.writeBit(effectiveGroupGuid[4]);
                packet.writeBit(effectiveGroupGuid[3]);
                packet.writeBit(effectiveGroupGuid[2]);
                packet.writeBit(effectiveGroupGuid[6]);
                packet.writeBit(effectiveGroupGuid[7]);

                if (flag)
                    packet.writeBits(flag, 9);

                packet.writeBit(0);

                packet.writeBit(receiverGuid[7]);
                packet.writeBit(receiverGuid[6]);
                packet.writeBit(receiverGuid[1]);
                packet.writeBit(receiverGuid[4]);
                packet.writeBit(receiverGuid[0]);
                packet.writeBit(receiverGuid[2]);
                packet.writeBit(receiverGuid[3]);
                packet.writeBit(receiverGuid[5]);

                packet.writeBit(0);
                packet.writeBit(!hasLanguage);
                packet.writeBit(!isAddon);

                packet.writeBit(senderGuid[0]);
                packet.writeBit(senderGuid[3]);
                packet.writeBit(senderGuid[7]);
                packet.writeBit(senderGuid[2]);
                packet.writeBit(senderGuid[1]);
                packet.writeBit(senderGuid[5]);
                packet.writeBit(senderGuid[4]);
                packet.writeBit(senderGuid[6]);

                packet.writeBit(!hasAchievement);
                packet.writeBit(!message.length());

                if (hasChannelName)
                    packet.writeBits(receiverName.length(), 7);

                if (message.length())
                    packet.writeBits(message.length(), 12);

                packet.writeBit(!hasReceiverName);

                //writeBits addon name

                packet.writeBit(1);

                if (hasReceiverName)
                    packet.writeBits(receiverName.length(), 11);

                packet.writeBit(0);

                packet.writeBit(effectiveGuildGuid[2]);
                packet.writeBit(effectiveGuildGuid[5]);
                packet.writeBit(effectiveGuildGuid[7]);
                packet.writeBit(effectiveGuildGuid[4]);
                packet.writeBit(effectiveGuildGuid[0]);
                packet.writeBit(effectiveGuildGuid[1]);
                packet.writeBit(effectiveGuildGuid[3]);
                packet.writeBit(effectiveGuildGuid[6]);

                packet.flushBits();

                packet.writeByteSeq(effectiveGuildGuid[4]);
                packet.writeByteSeq(effectiveGuildGuid[5]);
                packet.writeByteSeq(effectiveGuildGuid[7]);
                packet.writeByteSeq(effectiveGuildGuid[3]);
                packet.writeByteSeq(effectiveGuildGuid[2]);
                packet.writeByteSeq(effectiveGuildGuid[6]);
                packet.writeByteSeq(effectiveGuildGuid[0]);
                packet.writeByteSeq(effectiveGuildGuid[1]);

                if (hasChannelName)
                    packet.writeString(receiverName);

                //write addon string

                packet.writeByteSeq(senderGuid[4]);
                packet.writeByteSeq(senderGuid[7]);
                packet.writeByteSeq(senderGuid[1]);
                packet.writeByteSeq(senderGuid[5]);
                packet.writeByteSeq(senderGuid[0]);
                packet.writeByteSeq(senderGuid[6]);
                packet.writeByteSeq(senderGuid[2]);
                packet.writeByteSeq(senderGuid[3]);

                packet << uint8_t(type);

                if (hasAchievement)
                    packet << achievementId;

                packet.writeByteSeq(effectiveGroupGuid[1]);
                packet.writeByteSeq(effectiveGroupGuid[3]);
                packet.writeByteSeq(effectiveGroupGuid[4]);
                packet.writeByteSeq(effectiveGroupGuid[6]);
                packet.writeByteSeq(effectiveGroupGuid[0]);
                packet.writeByteSeq(effectiveGroupGuid[2]);
                packet.writeByteSeq(effectiveGroupGuid[5]);
                packet.writeByteSeq(effectiveGroupGuid[7]);

                packet.writeByteSeq(receiverGuid[2]);
                packet.writeByteSeq(receiverGuid[5]);
                packet.writeByteSeq(receiverGuid[3]);
                packet.writeByteSeq(receiverGuid[6]);
                packet.writeByteSeq(receiverGuid[7]);
                packet.writeByteSeq(receiverGuid[4]);
                packet.writeByteSeq(receiverGuid[1]);
                packet.writeByteSeq(receiverGuid[0]);

                if (hasLanguage)
                    packet << uint8_t(language);

                if (message.length())
                    packet.writeString(message);

                if (hasReceiverName)
                    packet.writeString(receiverName);

                if (hasSenderName)
                    packet.writeString(senderName);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            uint32_t unk;
            uint32_t message_length;
            packet >> type >> language >> unpacked_guid >> unk >> unpacked_guid >> message_length >> message >> flag;
            senderGuid = WoWGuid(unpacked_guid);
            return false;
        }
    };
}
