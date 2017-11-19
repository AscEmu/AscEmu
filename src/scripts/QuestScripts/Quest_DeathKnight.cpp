/*
* AscEmu Framework based on Arcemu MMORPG Server
* Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
#include "Management/TaxiMgr.h"

class GossipScourgeGryphon : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(12670) || plr->HasFinishedQuest(12670))
        {
            if (TaxiPath* path = sTaxiMgr.GetTaxiPath(pObject->GetEntry() == 29488 ? 1053 : 1054))
                plr->TaxiStart(path, 26308, 0);
        }
    }
};

const uint32 CN_INITIATE_1 = 29519;
const uint32 CN_INITIATE_2 = 29565;
const uint32 CN_INITIATE_3 = 29567;
const uint32 CN_INITIATE_4 = 29520;

class AcherusSoulPrison : GameObjectAIScript
{
public:

    AcherusSoulPrison(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO)
    {
        return new AcherusSoulPrison(GO);
    }

    void OnActivate(Player* pPlayer) override
    {
        QuestLogEntry* en = pPlayer->GetQuestLogForEntry(12848);
        if (!en)
            return;

        float SSX = pPlayer->GetPositionX();
        float SSY = pPlayer->GetPositionY();
        float SSZ = pPlayer->GetPositionZ();

        Creature* pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ);

        if (!pCreature || !pCreature->isAlive())
            return;

        if (pCreature->GetEntry() == CN_INITIATE_1 || pCreature->GetEntry() == CN_INITIATE_2 || pCreature->GetEntry() == CN_INITIATE_3 || pCreature->GetEntry() == CN_INITIATE_4)
        {
            pPlayer->SendChatMessage(CHAT_MSG_SAY, LANG_UNIVERSAL, "I give you the key to your salvation");
            pCreature->setUInt64Value(UNIT_FIELD_FLAGS, 0);
            pCreature->GetAIInterface()->setNextTarget(pPlayer);
            pCreature->GetAIInterface()->AttackReaction(pPlayer, 1, 0);
            pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have committed a big mistake, demon");

            if (en->GetMobCount(0) != 0)
                return;

            en->SetMobCount(0, 1);
            en->SendUpdateAddKill(0);
            en->UpdatePlayerFields();
        }

    }
};

class QuestInServiceOfLichKing : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/)
    {
        // Play first sound
        mTarget->PlaySound(14734);

        // Play second sound after 22.5 seconds
        sEventMgr.AddEvent(mTarget, &Player::PlaySound, (uint32)14735, EVENT_UNK, 22500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        // Play third sound after 48.5 seconds
        sEventMgr.AddEvent(mTarget, &Player::PlaySound, (uint32)14736, EVENT_UNK, 48500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
};

// QuestID for Praparation for the Battle
enum QUEST_12842_ENUM
{
    QUEST_PREPARATION = 12842,

    SPELL_RUNE_I = 53341, // Spell Rune of Cinderglacier
    SPELL_RUNE_II = 53343, // Spell Rune of Razorice
    SPELL_PREPERATION_FOR_BATTLE_CREDIT = 54586
};

bool PreparationForBattleEffect(uint32 /*effectIndex*/, Spell* pSpell)
{
    Player* pCaster = pSpell->p_caster;
    if (pCaster == nullptr)
        return false;

    // Apply spell if caster has quest and still heven't completed it yet
    if (pCaster->HasQuest(QUEST_PREPARATION) && !pCaster->HasFinishedQuest(QUEST_PREPARATION))
        pCaster->CastSpell(pCaster, SPELL_PREPERATION_FOR_BATTLE_CREDIT, true);

    return true;
}

//Quest Death Comes From On High
class EyeofAcherusControl : public GameObjectAIScript
{
public:
    EyeofAcherusControl(GameObject* gameobject) : GameObjectAIScript(gameobject) {}
    static GameObjectAIScript* Create(GameObject* gameobject_ai) { return new EyeofAcherusControl(gameobject_ai); }

    void OnActivate(Player* player) override
    {
        if (!player->HasQuest(12641))
            return;

        if (player->HasAura(51852))
            return;

        player->CastSpell(player, 51888, true);

        _gameobject->SetState(GO_STATE_CLOSED);
    }
};


void SetupDeathKnight(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* scourgeGryphon = new GossipScourgeGryphon();
    mgr->register_creature_gossip(29488, scourgeGryphon);
    mgr->register_creature_gossip(29501, scourgeGryphon);

    mgr->register_dummy_spell(SPELL_RUNE_I, &PreparationForBattleEffect);
    mgr->register_dummy_spell(SPELL_RUNE_II, &PreparationForBattleEffect);
    mgr->register_quest_script(12593, new QuestInServiceOfLichKing);

    mgr->register_gameobject_script(191588, &AcherusSoulPrison::Create);
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
    mgr->register_gameobject_script(191590, &AcherusSoulPrison::Create);

    mgr->register_gameobject_script(191609, &EyeofAcherusControl::Create);
}
