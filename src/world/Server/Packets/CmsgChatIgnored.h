/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgChatIgnored : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t unk1;

        CmsgChatIgnored() : CmsgChatIgnored(0, 0)
        {
        }

        CmsgChatIgnored(uint64_t guid, uint8_t unk1) :
            ManagedPacket(CMSG_CHAT_IGNORED, 0),
            guid(guid),
            unk1(unk1)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> unk1;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
