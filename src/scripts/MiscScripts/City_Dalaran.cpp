/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

// Alliance guard
// Cast spell 54028 on horde player if he is in the alliance area
class SilverCovenantMageGuard : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SilverCovenantMageGuard);
    explicit SilverCovenantMageGuard(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        RegisterAIUpdateEvent(1500);
    }

    void StartDefense()
    {
        Player* player = getNearestPlayer();
        if (player == nullptr)
        return;

        //Don't do anything with alliance players
        if (player->isTeamAlliance())
            return;

        float player_x = player->GetPositionX();
        float player_y = player->GetPositionY();
        float player_z = player->GetPositionZ();

        //hardcoded values...
        // the guards should cast the spell if someone is behind them...
        if (player_x < 5761.9f && player_x >5738.68f && player_y < 732.12f && player_y >712.09f && player_z > 635.0f)
        {
            getCreature()->setTargetGuid(player->getGuid());
            getCreature()->eventCastSpell(player, sSpellMgr.getSpellInfo(54028));
        }
        else
        {
            getCreature()->setTargetGuid(0); //Reset target... ugly
        }
    }

    void AIUpdate()
    {
        if (getCreature()->getInRangePlayersCount() > 0)
            StartDefense();
    }
};

// Horde guard
// Cast spell 54029 on alliance player if he is in the horde area
class SunreaversMageGuard : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunreaversMageGuard);
    explicit SunreaversMageGuard(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        RegisterAIUpdateEvent(1500);
    }

    void StartDefense()
    {
        Player* player = getNearestPlayer();
        if (player == nullptr)
            return;

        //Don't do anything with horde players
        if (player->isTeamHorde())
            return;

        float player_x = player->GetPositionX();
        float player_y = player->GetPositionY();
        float player_z = player->GetPositionZ();

        //hardcoded values...
        // the guards should cast the spell if someone is behind them...
        if (player_x < 5891.88f && player_x >5858.89f && player_y < 594.99f && player_y >565.51f && player_z > 635.0f)
        {
            getCreature()->setTargetGuid(player->getGuid());
            getCreature()->eventCastSpell(player, sSpellMgr.getSpellInfo(54029));
        }
        else
        {
        getCreature()->setTargetGuid(0); //Reset target... ugly
        }
    }

    void AIUpdate()
    {
        if (getCreature()->getInRangePlayersCount() > 0)
            StartDefense();
    }
};

class FactionInvisible : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FactionInvisible);
    explicit FactionInvisible(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // 0 = all (default), 1 = horde, 2 = alliance
        switch (getCreature()->spawnid)
        {
            case 128959:
            case 129859:
            case 129346:
                getCreature()->GetAIInterface()->faction_visibility = 2;
                break;
            case 128960:
            case 129860:
            case 129347:
                getCreature()->GetAIInterface()->faction_visibility = 1;
                break;
            default:
                break;
        }
    }
};

void SetupCityDalaran(ScriptMgr* mgr)
{
    mgr->register_creature_script(29254, &SilverCovenantMageGuard::Create);
    mgr->register_creature_script(29255, &SunreaversMageGuard::Create);

    // Neutral Masters
    uint32 FactionVisibleIds[] = { 31852, 31851, 32335, 32336, 32206, 32207,  0 };
    mgr->register_creature_script(FactionVisibleIds, &FactionInvisible::Create);
}
