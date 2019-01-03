/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Units/Players/Player.h"

namespace AscEmu { namespace Packets
{
    class MsgBattlegroundPlayerPosition : public ManagedPacket
    {
    public:
        uint32_t unknown1;
        uint32_t flagHolderCount;

        Player* alliancePlayer;
        Player* hordePlayer;

        MsgBattlegroundPlayerPosition() : MsgBattlegroundPlayerPosition(0, 0, nullptr, nullptr)
        {
        }

        MsgBattlegroundPlayerPosition(uint32_t unknown1, uint32_t flagHolderCount, Player* alliancePlayer, Player* hordePlayer) :
            ManagedPacket(MSG_BATTLEGROUND_PLAYER_POSITIONS, 4 + 4 + 16 * flagHolderCount),
            unknown1(unknown1),
            flagHolderCount(flagHolderCount),
            alliancePlayer(alliancePlayer),
            hordePlayer(hordePlayer)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown1 << flagHolderCount;
            if (alliancePlayer != nullptr)
                packet << alliancePlayer->getGuid() << alliancePlayer->GetPositionX() << alliancePlayer->GetPositionY();

            if (hordePlayer != nullptr)
                packet << hordePlayer->getGuid() << hordePlayer->GetPositionX() << hordePlayer->GetPositionY();

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
