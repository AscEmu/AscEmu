/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgMessageChat : public ManagedPacket
    {
    public:
        uint8_t type;
        uint32_t language;
        WoWGuid guid;
        std::string message;
        std::string destination;
        bool show_gm_flag;

        SmsgMessageChat() : SmsgMessageChat(0, 0, 0, "", false, "")
        {
        }

        SmsgMessageChat(uint8_t type, uint32_t language, uint64_t guid, std::string message,
                        bool showGMFlag, std::string destination = "") :
            ManagedPacket(SMSG_MESSAGECHAT, 0),
            type(type),
            language(language),
            guid(guid),
            message(message),
            show_gm_flag(showGMFlag),
            destination(destination)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type << language << guid.GetOldGuid() << uint32_t(0) << guid.GetOldGuid() << uint32_t(message.length() + 1) << message;

            packet << (show_gm_flag ? uint8_t(4) : uint8_t(0));

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
}}
