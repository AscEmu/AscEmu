/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgAddIgnore : public ManagedPacket
    {
    public:
        std::string name;

        CmsgAddIgnore() : CmsgAddIgnore("")
        {
        }

        CmsgAddIgnore(std::string name) :
            ManagedPacket(CMSG_ADD_IGNORE, 4),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> name;
                return true;
            }

            return false;
        }
    };
}
