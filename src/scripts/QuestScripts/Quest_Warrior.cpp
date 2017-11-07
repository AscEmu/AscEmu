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

class TheSummoning : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* qLogEntry)
    {
        Creature* windwatcher = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 6176);
        if (windwatcher == nullptr)
            return;

        // questgiver will walk to the place where Cyclonian is spawned only walk when we are at home
        if (windwatcher->CalcDistance(250.839996f, -1470.579956f, 55.4491f) > 1) return;
        {
            windwatcher->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me");

            windwatcher->LoadWaypointGroup(18);
            windwatcher->SwitchToCustomWaypoints();

            windwatcher->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
        }
        windwatcher->Despawn(15 * 60 * 1000, 0);

        // spawn cyclonian if not spawned already
        Creature* cyclonian = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(323.947f, -1483.68f, 43.1363f, 6239);
        if (cyclonian == nullptr)
        {
            cyclonian = pPlayer->GetMapMgr()->CreateAndSpawnCreature(6239, 323.947f, -1483.68f, 43.1363f, 0.682991f);

            // if spawning cyclonian failed, we have to return.
            if (cyclonian == nullptr)
                return;
        }

        // queue cyclonian for despawn
        cyclonian->Despawn(15 * 60 * 1000, 0);
    }
};

class Bartleby : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Bartleby);
    Bartleby(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->SetFaction(11);
        getCreature()->setEmoteState(EMOTE_ONESHOT_EAT);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
    {
        if (getCreature()->getUInt32Value(UNIT_FIELD_HEALTH) - fAmount <= getCreature()->getUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.37f)
        {
            if (mAttacker->IsPlayer())
            {
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(1640);
                if (!qle)
                    return;
                qle->SendQuestComplete();
            }
        }
    }

    void AIUpdate()
    {
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(11);
        getCreature()->SetHealthPct(100);
        getCreature()->GetAIInterface()->WipeTargetList();
        getCreature()->GetAIInterface()->WipeHateList();
        getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, 0);
    }

    void OnDied(Unit* mKiller)
    {
        RemoveAIUpdateEvent();
    }
};

class BeatBartleby : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Bartleby = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 6090);

        if (Bartleby == nullptr)
            return;

        Bartleby->SetFaction(168);
        Bartleby->GetAIInterface()->setMeleeDisabled(false);
        Bartleby->GetAIInterface()->SetAllowedToEnterCombat(true);
    }
};


void SetupWarrior(ScriptMgr* mgr)
{
    mgr->register_quest_script(1713, new TheSummoning());
    mgr->register_creature_script(6090, &Bartleby::Create);
    mgr->register_quest_script(1640, new BeatBartleby());
}
