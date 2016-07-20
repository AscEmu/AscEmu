/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"


uint16 GetItemIDFromLink(const char* itemlink, uint32* itemid)
{
    if (itemlink == NULL)
    {
        *itemid = 0;
        return 0;
    }
    uint16 slen = (uint16)strlen(itemlink);
    const char* ptr = strstr(itemlink, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }
    ptr += 7;                       // item id is just past "|Hitem:" (7 bytes)
    *itemid = atoi(ptr);
    ptr = strstr(itemlink, "|r");   // the end of the item link
    if (ptr == NULL)                // item link was invalid
    {
        *itemid = 0;
        return slen;
    }
    ptr += 2;
    return (ptr - itemlink) & 0x0000ffff;
}

/// DGM: Get skill level command for getting information about a skill
bool ChatHandler::HandleGetSkillLevelCommand(const char* args, WorldSession* m_session)
{
    uint32 skill = 0;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) return false;
    if (skill > SkillNameManager->maxskill)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    char* SkillName = SkillNameManager->SkillNames[skill];
    if (SkillName == 0)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    if (!plr->_HasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }
    uint32 nobonus = plr->_GetSkillLineCurrent(skill, false);
    uint32 bonus = plr->_GetSkillLineCurrent(skill, true) - nobonus;
    uint32 max = plr->_GetSkillLineMax(skill);
    BlueSystemMessage(m_session, "Player's %s skill has level: %u maxlevel: %u. (+ %u bonus)", SkillName, nobonus, max, bonus);
    return true;
}
