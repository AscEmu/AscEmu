/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
#include "Management/LocalizationMgr.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"

void WorldSession::HandleLearnTalentOpcode(WorldPacket& recv_data)
{
    uint32_t talent_id;
    uint32_t requested_rank;

    recv_data >> talent_id;
    recv_data >> requested_rank;

    _player->LearnTalent(talent_id, requested_rank);
}

void WorldSession::HandleUnlearnTalents(WorldPacket& recv_data)
{
    uint32_t price = GetPlayer()->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
    if (!GetPlayer()->HasGold(price))
        return;

    GetPlayer()->SetTalentResetTimes(GetPlayer()->GetTalentResetTimes() + 1);
    GetPlayer()->ModGold(-(int32_t)price);
    GetPlayer()->Reset_Talents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    uint32_t skill_line_id;
    uint32_t points_remaining = _player->GetPrimaryProfessionPoints();

    recv_data >> skill_line_id;

    _player->RemoveSpellsFromLine(skill_line_id);
    _player->_RemoveSkillLine(skill_line_id);

    if (points_remaining == _player->GetPrimaryProfessionPoints())
    {
        auto skill_line = sSkillLineStore.LookupEntry(skill_line_id);
        if (!skill_line)
            return;

        if (skill_line->type == SKILL_TYPE_PROFESSION && points_remaining < 2)
            _player->SetPrimaryProfessionPoints(points_remaining + 1);
    }
}
