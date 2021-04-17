/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

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
        uint32_t achievementId;

        SmsgMessageChat() : SmsgMessageChat(0, 0, 0, "", 0, "", 0, "", 0)
        {
        }

        SmsgMessageChat(uint8_t type, uint32_t language, uint8_t flag, std::string message, uint64_t senderGuid = 0, std::string senderName = "", uint64_t receiverGuid = 0, std::string receiverName = "", uint32_t achievementId = 0) :
            ManagedPacket(SMSG_MESSAGECHAT, 1 + 4 + 8 + 4 + 8 + (message.length() + 1) + 1),
            type(type),
            language(language),
            flag(flag),
            message(message),
            senderGuid(senderGuid),
            senderName(senderName),
            receiverGuid(receiverGuid),
            receiverName(receiverName),
            achievementId(achievementId)
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
                    packet << uint32_t(senderName.length() + 1) << senderName;
                    packet << receiverGuid.getRawGuid();
                    if (receiverGuid && !receiverGuid.isPlayer() && !receiverGuid.isPet() && type != CHAT_MSG_WHISPER_MOB)
                    {
                        packet << uint32_t(receiverName.length() + 1);
                        packet << receiverName;
                    }
                    packet << uint32_t(message.length() + 1) << message << flag;
                    break;
                case CHAT_MSG_BG_EVENT_NEUTRAL:
                case CHAT_MSG_BG_EVENT_ALLIANCE:
                case CHAT_MSG_BG_EVENT_HORDE:
                    packet << receiverGuid.getRawGuid();
                    if (receiverGuid && !receiverGuid.isPlayer())
                    {
                        packet << uint32_t(receiverName.length() + 1);
                        packet << receiverName;
                    }
                    packet << uint32_t(message.length() + 1) << message << flag;
                    break;
                case CHAT_MSG_ACHIEVEMENT:
                case CHAT_MSG_GUILD_ACHIEVEMENT:
                    packet << receiverGuid;
                    packet << uint32_t(message.length() + 1) << message << flag;
                    packet << achievementId;
                    break;
                default:
                    if (type == CHAT_MSG_CHANNEL)
                    {
                        packet << receiverName; //channel name
                    }
                    packet << receiverGuid.getRawGuid();
                    packet << uint32_t(message.length() + 1) << message << flag;
                    break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            uint32_t unk;
            uint32_t message_length;
            uint8_t flag;
            packet >> type >> language >> unpacked_guid >> unk >> unpacked_guid >> message_length >> message >> flag;
            senderGuid = WoWGuid(unpacked_guid);
            return false;
        }
    };
}
