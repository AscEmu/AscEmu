/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    struct SystemMessagePacket
    {
        SystemMessagePacket(std::string msg) : message(msg) {}

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
        WoWGuid guid;
        std::string message;
        std::string destination;
        uint8_t flag;

        SmsgMessageChat() : SmsgMessageChat(0, 0, 0, "", 0, "")
        {
        }

        SmsgMessageChat(uint8_t type, uint32_t language, uint64_t guid, std::string message,
                    uint8_t flag, std::string destination = "") :
            ManagedPacket(SMSG_MESSAGECHAT, 1 + 4 + 8 + 4 + 8 + (message.length() + 1) + 1),
            type(type),
            language(language),
            guid(guid),
            message(message),
            flag(flag),
            destination(destination)
        {
        }

        SmsgMessageChat(SystemMessagePacket sysMsg) :
            ManagedPacket(SMSG_MESSAGECHAT, 1 + 4 + 8 + 4 + 8 + (sysMsg.message.length() + 1) + 1),
            type(sysMsg.type),
            language(sysMsg.language),
            guid(sysMsg.guid),
            message(sysMsg.message),
            flag(sysMsg.flag)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type << language << guid.getRawGuid() << uint32_t(0) << guid.getRawGuid() << uint32_t(message.length() + 1) << message;

            packet << flag;

            // TODO Check this through message type instead
            if (destination != "")
                packet << destination;

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            uint32_t unk;
            uint32_t message_length;
            uint8_t flag;
            packet >> type >> language >> unpacked_guid >> unk >> unpacked_guid >> message_length >> message >> flag;
            guid = WoWGuid(unpacked_guid);
            return false;
        }
    };
}
