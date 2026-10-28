/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/AchievementMgr.h"
#include "Objects/Units/UnitDefines.hpp"
#include "Objects/Units/Players/Player.hpp"

// Catrina
inline constexpr uint32_t SPELL_HONOR_THE_DEAD = 65386;
// inline constexpr uint32_t NPC_CATRINA = 34383;
inline constexpr uint32_t ACHIEVEMENT_DEAD_MANS_PARTY = 3456;

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

    if (pPlayer->hasAurasWithId(SPELL_HONOR_THE_DEAD) == false)
    {
        if (pPlayer->isDead() == false)
        {
            pUnit->castSpell(pPlayer, SPELL_HONOR_THE_DEAD, true);
        }
    }

    // Achievements since WotLK
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
    {
        auto* achiMgr = pPlayer->getAchievementMgr();
        if (achiMgr != nullptr && !achiMgr->hasCompleted(ACHIEVEMENT_DEAD_MANS_PARTY))
        {
            achiMgr->gmCompleteAchievement(nullptr, ACHIEVEMENT_DEAD_MANS_PARTY);
        }
    }
}

void OnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
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
