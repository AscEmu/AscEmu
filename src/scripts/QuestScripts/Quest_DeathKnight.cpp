/*
* AscEmu Framework based on Arcemu MMORPG Server
* Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
* Copyright (C) 2009-2012 ArcEmu Team <http://www.arcemu.org>
* Copyright (C) 2008-2009 Sun++ Team <http://www.sunplusplus.info>
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
#include "../Common/Base.h"
#include "../Common/EasyFunctions.h"

class GossipScourgeGryphon : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            if (plr->HasQuest(12670) || plr->HasFinishedQuest(12670))
            {
                if (TaxiPath* path = sTaxiMgr.GetTaxiPath(pObject->GetEntry() == 29488 ? 1053 : 1054))
                    plr->TaxiStart(path, 26308, 0);
            }
        }
};

// QuestID for Praparation for the Battle
#define QUEST_PREPARATION               12842
//Spell Rune of Cinderglacier
#define SPELL_RUNE_I                    53341
//Spell Rune of Razorice
#define SPELL_RUNE_II                   53343

bool PreparationForBattleQuestCast(Player* pPlayer, SpellEntry* pSpell, Spell* spell)
{
    if (pSpell->Id == SPELL_RUNE_I || pSpell->Id == SPELL_RUNE_II)
    {
        QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(QUEST_PREPARATION);

        if (!qle || qle->CanBeFinished())
            return true;

        sEventMgr.AddEvent(TO_UNIT(pPlayer), &Unit::EventCastSpell, TO_UNIT(pPlayer), dbcSpell.LookupEntry(54586), EVENT_CREATURE_UPDATE, 5000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        return true;
    }
    else
        return true; // change this to false blocks all spells!
};

#define CN_INITIATE_1                29519
#define CN_INITIATE_2                29565
#define CN_INITIATE_3                29567
#define CN_INITIATE_4                29520

class AcherusSoulPrison : GameObjectAIScript
{
    public:
        AcherusSoulPrison(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO)
        {
            return new AcherusSoulPrison(GO);
        }

        void OnActivate(Player* pPlayer)
        {
            QuestLogEntry* en = pPlayer->GetQuestLogForEntry(12848);
            if(!en)
                return;

            float SSX = pPlayer->GetPositionX();
            float SSY = pPlayer->GetPositionY();
            float SSZ = pPlayer->GetPositionZ();

            Creature* pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ);

            if(!pCreature || !pCreature->isAlive())
                return;

            if(pCreature->GetEntry() == CN_INITIATE_1 || pCreature->GetEntry() == CN_INITIATE_2 || pCreature->GetEntry() == CN_INITIATE_3 || pCreature->GetEntry() == CN_INITIATE_4)
            {
                pPlayer->SendChatMessage(CHAT_MSG_SAY, LANG_UNIVERSAL, "I give you the key to your salvation");
                pCreature->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
                pCreature->GetAIInterface()->setNextTarget(pPlayer);
                pCreature->GetAIInterface()->AttackReaction(pPlayer, 1, 0);
                pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have committed a big mistake, demon");

                if(en->GetMobCount(0) != 0)
                    return;

                en->SetMobCount(0, 1);
                en->SendUpdateAddKill(0);
                en->UpdatePlayerFields();
            }

        }
};

class RuneforgingPreparationForBattle : QuestScripts
{
    /*If Player casted Spell 53341 or 53343 set quest as finished*/
};

class QuestInServiceOfLichKing : public QuestScript
{
    public:
        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            mTarget->PlaySound(14734);
            sEventMgr.AddEvent(mTarget, &Player::PlaySound, (uint32)14735, EVENT_UNK, 22500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            sEventMgr.AddEvent(mTarget, &Player::PlaySound, (uint32)14736, EVENT_UNK, 48500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
};

void SetupDeathKnight(ScriptMgr* mgr)
{
    mgr->register_gossip_script(29488, new GossipScourgeGryphon);
    mgr->register_gossip_script(29501, new GossipScourgeGryphon);

    mgr->register_hook(SERVER_HOOK_EVENT_ON_CAST_SPELL, (void*)PreparationForBattleQuestCast);
    mgr->register_quest_script(12593, new QuestInServiceOfLichKing);

    // These gobs had already a script by Type (in gameobject_names Type = 1 = Button).
    /*mgr->register_gameobject_script(191588, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191577, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191580, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191581, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191582, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191583, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191584, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191585, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191586, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191587, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191589, &AcherusSoulPrison::Create);
    mgr->register_gameobject_script(191590, &AcherusSoulPrison::Create);*/

}