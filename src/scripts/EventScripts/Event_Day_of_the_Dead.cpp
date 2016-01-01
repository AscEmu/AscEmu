/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Day of the Dead</b>\n
/// event_names entry: 51 \n
/// event_names holiday: 409 \n

/// Catrina
#define SPELL_HONOR_THE_DEAD 65386
#define NPC_CATRINA 34383
#define ACHIEVEMRNT_DEAD_MANS_PARTY 3456

void Catrina(Player* pPlayer, Unit* pUnit)
{
    if (pPlayer == NULL || pUnit == NULL)
        return;

    if (pPlayer->HasAura(SPELL_HONOR_THE_DEAD) == false)
    {
        if (pPlayer->IsDead() == false)
            pUnit->CastSpell(pPlayer, SPELL_HONOR_THE_DEAD, true);
    }
    if (pPlayer->GetAchievementMgr().HasCompleted(ACHIEVEMRNT_DEAD_MANS_PARTY) == false)
        pPlayer->GetAchievementMgr().GMCompleteAchievement(NULL, ACHIEVEMRNT_DEAD_MANS_PARTY);
}

void OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit)
{
    switch (Emote)
    {
        case EMOTE_STATE_DANCE: /// <- Its EMOTE name.
            Catrina(pPlayer, pUnit); /// <- Its Link.
            break;
    }
}


void SetupDayOfTheDead(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)&OnEmote);
}