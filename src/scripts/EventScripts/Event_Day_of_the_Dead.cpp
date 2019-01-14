/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum
{
    // Catrina
    SPELL_HONOR_THE_DEAD = 65386,
    //NPC_CATRINA = 34383,
    ACHIEVEMRNT_DEAD_MANS_PARTY = 3456,
};

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Day of the Dead</b>\n
// event_properties entry: 51 \n
// event_properties holiday: 409 \n

void Catrina(Player* pPlayer, Unit* pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
    {
        return;
    }

    if (pPlayer->HasAura(SPELL_HONOR_THE_DEAD) == false)
    {
        if (pPlayer->isDead() == false)
        {
            pUnit->castSpell(pPlayer, SPELL_HONOR_THE_DEAD, true);
        }
    }

#if VERSION_STRING > TBC
    if (pPlayer->GetAchievementMgr().HasCompleted(ACHIEVEMRNT_DEAD_MANS_PARTY) == false)
    {
        pPlayer->GetAchievementMgr().GMCompleteAchievement(NULL, ACHIEVEMRNT_DEAD_MANS_PARTY);
    }
#endif
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
