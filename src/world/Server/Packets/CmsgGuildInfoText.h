/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildInfoText : public ManagedPacket
    {
    public:
        std::string text;

        CmsgGuildInfoText() : CmsgGuildInfoText("")
        {
        }

        CmsgGuildInfoText(std::string text) :
            ManagedPacket(CMSG_GUILD_INFO_TEXT, 1),
            text(text)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> text;
            return true;
        }
    };
}}
