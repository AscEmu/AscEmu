/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Chat/ChatDefines.hpp"

namespace AscEmu { namespace Packets
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

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type << language;
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
                    packet << message;
                    break;
                case CHAT_MSG_WHISPER:
                case CHAT_MSG_CHANNEL:
                    packet << destination << message;
                    break;
                default: break;
            }

            if (language >= NUM_LANGUAGES)
                return false;

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
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

            return true;
        }
    };
}}
