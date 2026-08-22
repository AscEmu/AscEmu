/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetLfgComment : public ManagedPacket
    {
    public:
        std::string comment;

        CmsgSetLfgComment() : CmsgSetLfgComment("")
        {
        }

        CmsgSetLfgComment(std::string comment) :
            ManagedPacket(CMSG_SET_LFG_COMMENT, 0),
            comment(comment)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_Cata)
                return false;

            packet >> comment;
            return true;
        }
    };
}
