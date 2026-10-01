/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgUiMapQuestLinesRequest : public ManagedPacket
    {
    public:
        int32_t uiMapId = 0;

        CmsgUiMapQuestLinesRequest() : ManagedPacket(CMSG_UI_MAP_QUEST_LINES_REQUEST, 4) { }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet >> uiMapId;
            return true;
        }
    };
}
