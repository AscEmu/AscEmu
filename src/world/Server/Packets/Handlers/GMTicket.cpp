/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldSession.h"
#include "../../../../scripts/Common/Base.h"

enum GMticketType
{
    GM_TICKET_TYPE_STUCK                = 1,
    GM_TICKET_TYPE_BEHAVIOR_HARASSMENT  = 2,
    GM_TICKET_TYPE_GUILD                = 3,
    GM_TICKET_TYPE_ITEM                 = 4,
    GM_TICKET_TYPE_ENVIRONMENTAL        = 5,
    GM_TICKET_TYPE_NON_QUEST_CREEP      = 6,
    GM_TICKET_TYPE_QUEST_QUEST_NPC      = 7,
    GM_TICKET_TYPE_TECHNICAL            = 8,
    GM_TICKET_TYPE_ACCOUNT_BILLING      = 9,
    GM_TICKET_TYPE_CHARACTER            = 10
};

enum LagReportType
{
    LAG_REPORT_LOOT,
    LAG_REPORT_AH,
    LAG_REPORT_MAIL,
    LAG_REPORT_CHAT,
    LAG_REPORT_MOVEMENT,
    LAG_REPORT_SPELLS
};

void WorldSession::HandleGMTicketToggleSystemStatusOpcode(WorldPacket& recv_data)
{
    if (!HasGMPermissions())
        return;

    sWorld.toggleGmTicketStatus();
}

void WorldSession::HandleReportLag(WorldPacket& recv_data)
{
    uint32 lagType;
    uint32 mapId;
    float position_x;
    float position_y;
    float position_z;

    recv_data >> lagType;
    recv_data >> mapId;
    recv_data >> position_x;
    recv_data >> position_y;
    recv_data >> position_z;

    if (GetPlayer() != nullptr)
    {
        CharacterDatabase.Execute("INSERT INTO lag_reports (player, account, lag_type, map_id, position_x, position_y, position_z) VALUES(%u, %u, %u, %u, %f, %f, %f)", GetPlayer()->GetLowGUID(), _accountId, lagType, mapId, position_x, position_y, position_z);
    }

    LogDebugFlag(LF_OPCODE, "Player %s has reported a lagreport with Type: %u on Map: %u", GetPlayer()->GetName(), lagType, mapId);
}

void WorldSession::HandleGMSurveySubmitOpcode(WorldPacket& recv_data)
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(survey_id) FROM gm_survey");

    uint32 next_survey_id = result->Fetch()[0].GetUInt32() + 1;
    uint32 main_survey;
    recv_data >> main_survey;

    std::unordered_set<uint32> survey_ids;
    for (uint8 i = 0; i < 10; i++)
    {
        uint32 sub_survey_id;
        recv_data >> sub_survey_id;
        if (!sub_survey_id)
            break;

        uint8 answer_id;
        recv_data >> answer_id;

        std::string comment; // unused empty string
        recv_data >> comment;

        if (!survey_ids.insert(sub_survey_id).second)
            continue;

        CharacterDatabase.Execute("INSERT INTO gm_survey_answers VALUES(%u , %u , %u)", next_survey_id, sub_survey_id, answer_id);
    }

    std::string comment; // receive the player comment for this survey
    recv_data >> comment;

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))", next_survey_id, GetPlayer()->GetLowGUID(), main_survey, CharacterDatabase.EscapeString(comment).c_str());

    LogDebugFlag(LF_OPCODE, "Player %s has submitted the gm suvey %u successfully.", GetPlayer()->GetName(), next_survey_id);
}

