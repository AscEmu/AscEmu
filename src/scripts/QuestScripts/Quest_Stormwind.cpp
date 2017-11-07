/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
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

class DashelStonefist : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(DashelStonefist);
    DashelStonefist(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->SetFaction(12);
        getCreature()->SetStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
    {
        if (getCreature()->getUInt32Value(UNIT_FIELD_HEALTH) - fAmount <= getCreature()->getUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.2f)
        {
            if (mAttacker->IsPlayer())
            {
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(1447);
                if (!qle)
                    return;
                qle->SendQuestComplete();
            }
        }
    }

    void AIUpdate()
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay, okay! Enough fighting. No one else needs to get hurt.");
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(12);
        getCreature()->SetHealthPct(100);
        getCreature()->GetAIInterface()->WipeTargetList();
        getCreature()->GetAIInterface()->WipeHateList();
        getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
        RemoveAIUpdateEvent();
    }
};

class TheMissingDiplomat : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Dashel = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 4961);

        if (Dashel == NULL)
            return;

        Dashel->SetFaction(72);
        Dashel->GetAIInterface()->setMeleeDisabled(false);
        Dashel->GetAIInterface()->SetAllowedToEnterCombat(true);

        uint32 chance = RandomUInt(100);
        if (chance < 15)
        {
            std::string say = "Now you're gonna get it good, ";
            say += (static_cast<Player*>(mTarget))->GetName();
            say += "!";
            Dashel->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
        }
        Creature* ct1 = mTarget->GetMapMgr()->CreateAndSpawnCreature(4969, -8686.803711f, 445.267792f, 99.789223f, 5.461184f);
        if (ct1 != nullptr)
            ct1->Despawn(300000, 0);

        Creature* ct2 = mTarget->GetMapMgr()->CreateAndSpawnCreature(4969, -8675.571289f, 444.162262f, 99.644737f, 3.834103f);
        if (ct2 != nullptr)
            ct2->Despawn(300000, 0);
    }
};


void SetupStormwind(ScriptMgr* mgr)
{
    mgr->register_creature_script(4961, &DashelStonefist::Create);
    mgr->register_quest_script(1447, new TheMissingDiplomat());
}
