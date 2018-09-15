/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "FastQueue.h"
#include "Threading/Mutex.h"
#include "WorldPacket.h"
#include "Management/Item.h"
#include "Exceptions/PlayerExceptions.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Pet.h"

void WorldSession::HandleLearnTalentOpcode(WorldPacket& recv_data)
{
    uint32_t talent_id;
    uint32_t requested_rank;

    recv_data >> talent_id;
    recv_data >> requested_rank;

    _player->learnTalent(talent_id, requested_rank);
    _player->smsg_TalentsInfo(false);
}

void WorldSession::HandleLearnPreviewTalentsOpcode(WorldPacket& recv_data)
{
    int32_t current_tab;
    uint32_t talent_count;
    uint32_t talent_id;
    uint32_t talent_rank;

    //if currentTab -1 player has already the spec.
    recv_data >> current_tab;
    recv_data >> talent_count;

    for (uint32_t i = 0; i < talent_count; ++i)
    {
        recv_data >> talent_id;
        recv_data >> talent_rank;

        _player->learnTalent(talent_id, talent_rank);
    }

    _player->smsg_TalentsInfo(false);
}

void WorldSession::HandleUnlearnTalents(WorldPacket& /*recvData*/)
{
    uint32_t price = GetPlayer()->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
    if (!GetPlayer()->HasGold(price))
        return;

    GetPlayer()->SetTalentResetTimes(GetPlayer()->GetTalentResetTimes() + 1);
    GetPlayer()->ModGold(-(int32_t)price);
    GetPlayer()->resetTalents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    uint32_t skill_line_id;
    uint32_t points_remaining = _player->getFreePrimaryProfessionPoints();

    recv_data >> skill_line_id;

    _player->RemoveSpellsFromLine(skill_line_id);
    _player->_RemoveSkillLine(skill_line_id);

    if (points_remaining == _player->getFreePrimaryProfessionPoints())
    {
        auto skill_line = sSkillLineStore.LookupEntry(skill_line_id);
        if (!skill_line)
            return;

        if (skill_line->type == SKILL_TYPE_PROFESSION && points_remaining < 2)
            _player->setFreePrimaryProfessionPoints(points_remaining + 1);
    }
}
