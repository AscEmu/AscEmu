/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Management/Group.h"

const uint8_t iconCount = 8;

namespace AscEmu { namespace Packets
{
    class MsgRaidTargetUpdate : public ManagedPacket
    {
    public:
        uint8_t option;
        uint64_t playerGuid;
        uint8_t icon;
        uint64_t guid;
        Group* group;

        MsgRaidTargetUpdate() : MsgRaidTargetUpdate(0, 0, 0, 0, nullptr)
        {
        }

        MsgRaidTargetUpdate(uint8_t option, uint64_t playerGuid, uint8_t icon, uint64_t guid, Group* group) :
            ManagedPacket(MSG_RAID_TARGET_UPDATE, 0),
            option(option),
            playerGuid(playerGuid),
            icon(icon),
            guid(guid),
            group(group)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << option;
            if (option == 0)
            {
                packet << playerGuid << icon << guid;
            }
            else
            {
                if (group)
                {
                    for (uint8_t i = 0; i < iconCount; ++i)
                        packet << i << group->m_targetIcons[i];
                }
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> icon;
            if (icon == 0xFF)
                packet >> guid;
            return true;
        }
    };
}}
