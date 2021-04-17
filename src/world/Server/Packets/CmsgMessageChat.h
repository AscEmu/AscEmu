/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Chat/ChatDefines.hpp"
#include "Server/OpcodeTable.hpp"

namespace AscEmu::Packets
{
    class CmsgMessageChat : public ManagedPacket
    {
    public:
        uint32_t type;
        uint32_t language;
        std::string destination;
        std::string message;

        CmsgMessageChat() : CmsgMessageChat(0, 0)
        {
        }

        CmsgMessageChat(uint32_t type, uint32_t language, std::string message = "", std::string destination = "") :
            ManagedPacket(CMSG_MESSAGECHAT, 0),
            type(type),
            language(language),
            destination(destination),
            message(message)
        {
        }

#if VERSION_STRING == Cata
        struct OpcodeToChatType
        {
            uint16_t opcode;
            uint8_t chatType;
        };

#define MSG_OPCODE_COUNT 13

        OpcodeToChatType opcodeToChatTypeList[MSG_OPCODE_COUNT] =
        {
            { CMSG_MESSAGECHAT_SAY, CHAT_MSG_SAY },
            { CMSG_MESSAGECHAT_YELL, CHAT_MSG_YELL },
            { CMSG_MESSAGECHAT_CHANNEL, CHAT_MSG_CHANNEL },
            { CMSG_MESSAGECHAT_WHISPER, CHAT_MSG_WHISPER },
            { CMSG_MESSAGECHAT_GUILD, CHAT_MSG_GUILD },
            { CMSG_MESSAGECHAT_OFFICER, CHAT_MSG_OFFICER },
            { CMSG_MESSAGECHAT_AFK, CHAT_MSG_AFK },
            { CMSG_MESSAGECHAT_DND, CHAT_MSG_DND },
            { CMSG_MESSAGECHAT_EMOTE, CHAT_MSG_EMOTE },
            { CMSG_MESSAGECHAT_PARTY, CHAT_MSG_PARTY },
            { CMSG_MESSAGECHAT_RAID, CHAT_MSG_RAID },
            { CMSG_MESSAGECHAT_BATTLEGROUND, CHAT_MSG_BATTLEGROUND },
            { CMSG_MESSAGECHAT_RAID_WARNING, CHAT_MSG_RAID_WARNING }
        };

        uint8_t getMessageTypeForOpcode(uint16_t opcode)
        {
            for (auto& i : opcodeToChatTypeList)
            {
                if (i.opcode == opcode)
                    return i.chatType;
            }

            return 0xFF;
        }
#endif

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override { return true; }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> type >> language;

            switch (type)
            {
                case CHAT_MSG_EMOTE:
                case CHAT_MSG_SAY:
                case CHAT_MSG_PARTY:
                case CHAT_MSG_RAID:
                case CHAT_MSG_RAID_LEADER:
                case CHAT_MSG_RAID_WARNING:
                case CHAT_MSG_GUILD:
                case CHAT_MSG_OFFICER:
                case CHAT_MSG_YELL:
                case CHAT_MSG_AFK:
                case CHAT_MSG_DND:
                    packet >> message;
                    break;
                case CHAT_MSG_WHISPER:
                case CHAT_MSG_CHANNEL:
                    packet >> destination >> message;
                    break;
                default: break;
            }
#else
            type = getMessageTypeForOpcode(sOpcodeTables.getInternalIdForHex(packet.GetOpcode()));
            if (type == 0xFF)
                return false;

            //get language
            switch (type)
            {
                case CHAT_MSG_SAY:
                case CHAT_MSG_YELL:
                case CHAT_MSG_CHANNEL:
                case CHAT_MSG_WHISPER:
                case CHAT_MSG_GUILD:
                case CHAT_MSG_OFFICER:
                case CHAT_MSG_PARTY:
                case CHAT_MSG_RAID:
                case CHAT_MSG_BATTLEGROUND:
                case CHAT_MSG_RAID_WARNING:
                    packet >> language;
                    break;
                default: break;
            }

            if (language >= NUM_LANGUAGES)
                return false;

            uint32_t textLength = 0;
            uint32_t receiverLength = 0;

            switch (type)
            {
                case CHAT_MSG_SAY:
                case CHAT_MSG_PARTY:
                case CHAT_MSG_PARTY_LEADER:
                case CHAT_MSG_BATTLEGROUND_LEADER:
                case CHAT_MSG_GUILD:
                case CHAT_MSG_OFFICER:
                case CHAT_MSG_RAID:
                case CHAT_MSG_RAID_LEADER:
                case CHAT_MSG_RAID_WARNING:
                case CHAT_MSG_BATTLEGROUND:
                case CHAT_MSG_AFK:
                case CHAT_MSG_DND:
                    textLength = packet.readBits(9);
                    message = packet.ReadString(textLength);
                    break;
                case CHAT_MSG_WHISPER:
                {
                    receiverLength = packet.readBits(10);
                    textLength = packet.readBits(9);
                    destination = packet.ReadString(receiverLength);
                    message = packet.ReadString(textLength);

                } break;
                case CHAT_MSG_CHANNEL:
                {
                    receiverLength = packet.readBits(10);
                    textLength = packet.readBits(9);
                    message = packet.ReadString(textLength);
                    destination = packet.ReadString(receiverLength);
                } break;
                default: break;
            }

#endif

            return true;
        }
    };
}
