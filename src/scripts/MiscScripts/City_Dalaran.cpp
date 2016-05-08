/*
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

// Alliance guard
// Cast spell 54028 on horde player if he is in the alliance area
class SilverCovenantMageGuard : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SilverCovenantMageGuard, MoonScriptCreatureAI);
        SilverCovenantMageGuard(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            RegisterAIUpdateEvent(1500);
        }

        void StartDefense()
        {
            Player* player = GetNearestPlayer();

            if (player == nullptr)
                return;

            //Don't do anything with alliance players
            if (player->IsTeamAlliance())
                return;

            float player_x = player->GetPositionX();
            float player_y = player->GetPositionY();
            float player_z = player->GetPositionZ();

            //hardcoded values...
            // the guards should cast the spell if someone is behind them...
            if (player_x < 5761.9f && player_x >5738.68f && player_y < 732.12f && player_y >712.09f && player_z > 635.0f)
            {
                _unit->SetTargetGUID(player->GetGUID());
                _unit->EventCastSpell(player, dbcSpell.LookupEntry(54028));
            }
            else
            {
                _unit->SetTargetGUID(0); //Reset target... ugly
            }
        }

        void AIUpdate()
        {
            if (_unit->GetInRangePlayersCount() > 0)
                StartDefense();
        }
};

// Horde guard
// Cast spell 54029 on alliance player if he is in the horde area
class SunreaversMageGuard : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SunreaversMageGuard, MoonScriptCreatureAI);
        SunreaversMageGuard(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            RegisterAIUpdateEvent(1500);
        }

        void StartDefense()
        {
            Player* player = GetNearestPlayer();

            if (player == nullptr)
                return;

            //Don't do anything with horde players
            if (player->IsTeamHorde())
                return;

            float player_x = player->GetPositionX();
            float player_y = player->GetPositionY();
            float player_z = player->GetPositionZ();

            //hardcoded values...
            // the guards should cast the spell if someone is behind them...
            if (player_x < 5891.88f && player_x >5858.89f && player_y < 594.99f && player_y >565.51f && player_z > 635.0f)
            {
                _unit->SetTargetGUID(player->GetGUID());
                _unit->EventCastSpell(player, dbcSpell.LookupEntry(54029));
            }
            else
            {
                _unit->SetTargetGUID(0); //Reset target... ugly
            }
        }

        void AIUpdate()
        {
            if (_unit->GetInRangePlayersCount() > 0)
                StartDefense();
        }
};

class FactionInvisible : public MoonScriptCreatureAI
{
public:

    MOONSCRIPT_FACTORY_FUNCTION(FactionInvisible, MoonScriptCreatureAI);
    FactionInvisible(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // 0 = all (default), 1 = horde, 2 = alliance
        switch (_unit->spawnid)
        {
            case 128959:
            case 129859:
            case 129346:
                _unit->GetAIInterface()->faction_visibility = 2;
                break;
            case 128960:
            case 129860:
            case 129347:
                _unit->GetAIInterface()->faction_visibility = 1;
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
