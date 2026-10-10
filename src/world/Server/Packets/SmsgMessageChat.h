/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Chat/ChatDefines.hpp"

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

        SmsgMessageChat() : SmsgMessageChat(0, 0, 0, "", 0, "", 0, "", 0)
        {
        }

        SmsgMessageChat(uint8_t type, uint32_t language, uint8_t flag, std::string message, uint64_t senderGuid = 0, std::string senderName = "", uint64_t receiverGuid = 0, std::string receiverName = "", uint32_t achievementId = 0, uint64_t groupGuid = 0, uint64_t guildGuid = 0) :
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
            guildGuid(guildGuid)
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

        // the chat types of 7.3.5: the types after the raid warning moved
        static uint8_t legionChatType(uint8_t chatType)
        {
            switch (chatType)
            {
                case CHAT_MSG_RAID_WARNING_WIDESCREEN:  return 40;      // raid warning
                case CHAT_MSG_RAID_BOSS_EMOTE:          return 41;
                case CHAT_MSG_FILTERED:                 return 43;
                case CHAT_MSG_BATTLEGROUND:             return 62;      // instance chat
                case CHAT_MSG_BATTLEGROUND_LEADER:      return 63;      // instance chat leader
                case CHAT_MSG_RESTRICTED:               return 44;
                case CHAT_MSG_ACHIEVEMENT:              return 46;
                case CHAT_MSG_GUILD_ACHIEVEMENT:        return 47;
                case CHAT_MSG_PARTY_LEADER:             return 49;
                default:                                return chatType;
            }
        }

        // the chat types of 6.2.4: as 7.3.5, the instance chat types have other values
        static uint8_t wodChatType(uint8_t chatType)
        {
            switch (chatType)
            {
                case CHAT_MSG_BATTLEGROUND:             return 66;      // instance chat
                case CHAT_MSG_BATTLEGROUND_LEADER:      return 67;      // instance chat leader
                default:                                return legionChatType(chatType);
            }
        }

        bool serialiseLegion(WorldPacket& packet)
        {
            // player senders are resolved by their guid, creatures and battleground events carry their name
            std::string legionSenderName;
            std::string targetName;
            std::string channelName;

            switch (type)
            {
                case CHAT_MSG_MONSTER_SAY:
                case CHAT_MSG_MONSTER_PARTY:
                case CHAT_MSG_MONSTER_YELL:
                case CHAT_MSG_MONSTER_WHISPER:
                case CHAT_MSG_MONSTER_EMOTE:
                case CHAT_MSG_RAID_BOSS_EMOTE:
                case CHAT_MSG_WHISPER_MOB:
                    legionSenderName = senderName;
                    if (receiverGuid && !receiverGuid.isPlayer() && !receiverGuid.isPet() && type != CHAT_MSG_WHISPER_MOB)
                        targetName = receiverName;
                    break;
                case CHAT_MSG_BG_EVENT_NEUTRAL:
                case CHAT_MSG_BG_EVENT_ALLIANCE:
                case CHAT_MSG_BG_EVENT_HORDE:
                    if (receiverGuid && !receiverGuid.isPlayer())
                        targetName = receiverName;
                    break;
                case CHAT_MSG_CHANNEL:
                    channelName = receiverName;
                    break;
                default:
                    break;
            }

            const bool hasGroupGuid = type == CHAT_MSG_PARTY || type == CHAT_MSG_PARTY_LEADER || type == CHAT_MSG_RAID ||
                type == CHAT_MSG_RAID_LEADER || type == CHAT_MSG_RAID_WARNING;
            const bool hasGuildGuid = type == CHAT_MSG_GUILD || type == CHAT_MSG_OFFICER || type == CHAT_MSG_GUILD_ACHIEVEMENT;
            const bool hasAchievement = (type == CHAT_MSG_ACHIEVEMENT || type == CHAT_MSG_GUILD_ACHIEVEMENT) && achievementId;
            const uint32_t virtualRealm = m_protocol.getVirtualRealmAddress();

            // 8.x: the instance chat types of 6.x, the language as uint32
            packet << uint8_t(m_protocol.isLegion() ? legionChatType(type) : wodChatType(type));
            if (m_protocol.isBfA() || m_protocol.isShadowlands())
                packet << uint32_t(language);
            else
                packet << uint8_t(language);
            packet << senderGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
            packet << (hasGuildGuid ? guildGuid.toGuid128(m_protocol.realmId, 0) : WoWGuid128());
            packet << WoWGuid128();                                     // account of the sender
            packet << (type == CHAT_MSG_CHANNEL ? WoWGuid128() : receiverGuid.toGuid128(m_protocol.realmId, m_receiverMapId));
            packet << uint32_t(virtualRealm);                           // realm of the target
            packet << uint32_t(virtualRealm);                           // realm of the sender
            packet << (hasGroupGuid ? groupGuid.toGuid128(m_protocol.realmId, 0) : WoWGuid128());
            packet << uint32_t(hasAchievement ? achievementId : 0);
            packet << float(0.0f);                                      // display time

            packet.writeBits(static_cast<uint32_t>(legionSenderName.length()), 11);
            packet.writeBits(static_cast<uint32_t>(targetName.length()), 11);
            packet.writeBits(0, 5);                                     // addon prefix
            packet.writeBits(static_cast<uint32_t>(channelName.length()), 7);
            packet.writeBits(static_cast<uint32_t>(message.length()), 12);
            packet.writeBits(flag, m_protocol.isShadowlands() ? 14 : 11);
            packet.writeBit(false);                                     // hide in the chat log
            packet.writeBit(false);                                     // fake sender name
            if (m_protocol.isBfA() || m_protocol.isShadowlands())
                packet.writeBit(false);                                 // unused value
            if (m_protocol.isShadowlands())
                packet.writeBit(false);                                 // channel guid
            packet.flushBits();

            packet.writeString(legionSenderName);
            packet.writeString(targetName);
            packet.writeString(channelName);
            packet.writeString(message);
            return true;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands())
                return serialiseLegion(packet);

            if (m_protocol.expansion == WoW::Expansion::_Classic)
            {
                packet << mapInternalToClassicType(type);
                packet << static_cast<uint32_t>(language);

                switch (type)
                {
                    case CHAT_MSG_SYSTEM:
                        {
                            packet << static_cast<uint64_t>(0);
                            packet << static_cast<uint32_t>(message.length() + 1) << message << flag;
                        } break;
                    case CHAT_MSG_CHANNEL:
                        {
                            packet << receiverName; // Channel name (null-terminated string)
                            packet << static_cast<uint32_t>(0);
                            packet << senderGuid.getRawGuid();
                            packet << static_cast<uint32_t>(message.length() + 1) << message << flag;
                        } break;

                    case CHAT_MSG_MONSTER_SAY:
                    case CHAT_MSG_MONSTER_PARTY:
                    case CHAT_MSG_MONSTER_YELL:
                    case CHAT_MSG_MONSTER_WHISPER:
                    case CHAT_MSG_MONSTER_EMOTE:
                    case CHAT_MSG_RAID_BOSS_EMOTE:
                    case CHAT_MSG_WHISPER_MOB:
                        {
                            packet << senderGuid.getRawGuid();
                            packet << static_cast<uint32_t>(senderName.length() + 1) << senderName;
                            packet << receiverGuid.getRawGuid();
                            if (receiverGuid && !receiverGuid.isPlayer() && !receiverGuid.isPet() && type != CHAT_MSG_WHISPER_MOB)
                            {
                                packet << static_cast<uint32_t>(receiverName.length() + 1) << receiverName;
                            }
                            packet << static_cast<uint32_t>(message.length() + 1) << message << flag;
                        } break;
                    case CHAT_MSG_SAY:
                    case CHAT_MSG_YELL:
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_PARTY_LEADER:
                    case CHAT_MSG_RAID:
                    case CHAT_MSG_RAID_LEADER:
                    case CHAT_MSG_RAID_WARNING:
                    case CHAT_MSG_GUILD:
                    case CHAT_MSG_OFFICER:
                    case CHAT_MSG_EMOTE:
                    case CHAT_MSG_TEXT_EMOTE:
                    case CHAT_MSG_WHISPER:
                    case CHAT_MSG_WHISPER_INFORM:
                    case CHAT_MSG_AFK:
                    case CHAT_MSG_DND:
                    default:
                        {
                            packet << receiverGuid.getRawGuid(); // 0
                            packet << senderGuid.getRawGuid(); // Player GUID
                            packet << static_cast<uint32_t>(message.length() + 1) << message << flag;
                        } break;
                }
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
