/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

//////////////////////////////////////////////////////////////////////////////////////////
// On quest accept

void Hanazua(Player* pPlayer, Object* pObject)
{
    std::string say;
    say = "Go swiftly, ";
    say += pPlayer->getName().c_str();
    say += ", my fate is in your hands.";
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
}

void AHumbleTask(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Greatmother Hawkwind gestures to the pitcher of water sitting on the edge of the well.");
}

void Yorus_Barleybrew(Player* pPlayer, Object* pObject)
{
    (static_cast<Creature*>(pObject))->emote(EMOTE_ONESHOT_POINT);
    pPlayer->castSpell(pPlayer, sSpellMgr.getSpellInfo(8554), true);
}

void Menara_Voidrender(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->eventAddEmote(EMOTE_STATE_USESTANDING, 3000);
}

void Hanazua_III(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Killed  Sarkoth !!");
}

void TheFamilyAndTheFishingPole(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I got some extras with me; if you pay me back what i paid for them, you can have one. I got some bait too.");
}

void MillysHarvest(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I hope for you!");
    (static_cast<Creature*>(pObject))->eventAddEmote(EMOTE_ONESHOT_CRY, 2000);
}

void Rest_n_Relaxation(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Think of small pumpkins, my friend!!");
    (static_cast<Creature*>(pObject))->emote(EMOTE_ONESHOT_CHEER);
}

void OntoGoldshireComplete(Player* pPlayer, Object* pObject)
{
    char msg[256];
    snprintf((char*)msg, 256, "You are dismissed %s . ", pPlayer->getName().c_str());
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
    (static_cast<Creature*>(pObject))->emote(EMOTE_ONESHOT_BOW);
}

void ZuluhedtheWhacked(Player* pPlayer, Object* /*pObject*/)
{
    Creature* Zuluhed = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-4206.199219f, 313.5462f, 122.907f, 11980);
    if(Zuluhed == nullptr)
    {
        pPlayer->GetMapMgr()->CreateAndSpawnCreature(11980, -4206.199219f, 313.5462f, 122.907f, 1.2589f);
    }
}

void OnQuestAccept(Player* pPlayer, QuestProperties* pQuest, Object* pObject)
{
    if (pPlayer == nullptr || pQuest == nullptr || pObject == nullptr || !pObject->IsInWorld() || !pPlayer->IsInWorld() || !pObject->isCreature())
        return;

    switch (pQuest->id)
    {
        case 790:
            Hanazua_III(pPlayer, pObject);
            break;
        case 804:
            Hanazua(pPlayer, pObject);
            break;
        case 753:
            AHumbleTask(pPlayer, pObject);
            break;
        case 1699:
            Yorus_Barleybrew(pPlayer, pObject);
            break;
        case 4786:
            Menara_Voidrender(pPlayer, pObject);
            break;
        case 4963:
            Menara_Voidrender(pPlayer, pObject);
            break;
        case 1141:
            TheFamilyAndTheFishingPole(pPlayer, pObject);
            break;
        case 3904:
            MillysHarvest(pPlayer, pObject);
            break;
        case 2158:
            Rest_n_Relaxation(pPlayer, pObject);
            break;
        case 54:
            OntoGoldshireComplete(pPlayer, pObject);
            break;
        case 10872:
            ZuluhedtheWhacked(pPlayer, pObject);
            break;
    }
}

// On quest finished
//////////////////////////////////////////////////////////////////////////////////////////

void Hanazua_II(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->setStandState(STANDSTATE_KNEEL);
}

void Wishock(Player* pPlayer, Object* pObject)
{
    (static_cast<Creature*>(pObject))->setStandState(STANDSTATE_DEAD);
    pPlayer->emote(EMOTE_STATE_LAUGH);
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Arrgh...");
}

void CapturedMountaineer(Player* pPlayer, Object* pObject)
{
    std::string say = "I raise my brew and hope to be rid of the likes of you!  Cheers, you no good scoundrel, ";
    say += pPlayer->getName().c_str();
    say += "!";
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
}

void PlaguedLands(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Be cleansed!");
}

void DeeprunRatRoundup(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Into the box me pretties! Thas it. One by one ye go.");
}

void MaybellComplete(Player* /*pPlayer*/, Object* pObject)
{
    (static_cast<Creature*>(pObject))->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Here goes nothing...");
    (static_cast<Creature*>(pObject))->emote(EMOTE_ONESHOT_CRY);
    (static_cast<Creature*>(pObject))->Despawn(5000, 30000);
}

void OnQuestFinished(Player* pPlayer, QuestProperties* pQuest, Object* pObject)
{
    if (pPlayer == nullptr || pQuest == nullptr || pObject == nullptr || !pObject->isCreature())
        return;

    switch (pQuest->id)
    {
        case 790:
            Hanazua_II(pPlayer, pObject);
            break;
        case 336:
            Wishock(pPlayer, pObject);
            break;
        case 492:
            CapturedMountaineer(pPlayer, pObject);
            break;
        case 2118:
            PlaguedLands(pPlayer, pObject);
            break;
        case 6661:
            DeeprunRatRoundup(pPlayer, pObject);
            break;
        case 114:
            MaybellComplete(pPlayer, pObject);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// On quest cancelled

void ZuluhedtheWhackedCancel(Player* pPlayer)
{
    Creature* Zuluhed = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-4206.199219f, 313.5462f, 122.907f, 11980);
    if(Zuluhed != nullptr)
    {
        Zuluhed->Despawn(0, 0);
    }
}

void OnQuestCancelled(Player* pPlayer, QuestProperties* pQuest)
{
    if (pPlayer == nullptr || pQuest == nullptr)
        return;

    switch (pQuest->id)
    {
        case 10872:
            ZuluhedtheWhackedCancel(pPlayer);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// On emote

void InnkeeperFlex(Player* pPlayer, Unit* pUnit)
{
    if (pUnit->getEntry() == 6740)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8356))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
    else if (pUnit->getEntry() == 6929)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8359))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
}

void InnkeeperDance(Player* pPlayer, Unit* pUnit)
{
    if (pUnit->getEntry() == 6735)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8357))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
    else if (pUnit->getEntry() == 6746)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8360))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
}

void InnkeeperTrain(Player* pPlayer, Unit* pUnit)
{
    if (pUnit->getEntry() == 6826)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8355))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
    else if (pUnit->getEntry() == 11814)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8358))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
}

void InnkeeperChicken(Player* pPlayer, Unit* pUnit)
{
    if (pUnit->getEntry() == 5111)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8353))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
    else if (pUnit->getEntry() == 6741)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(8354))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->SendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
}

void OnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
{
    pUnit = pPlayer->GetMapMgr()->GetUnit(pPlayer->getTargetGuid());
    if(!pUnit || !pUnit->isAlive() || pUnit->GetAIInterface()->getNextTarget())
        return;

    switch(Emote)
    {
        case EMOTE_ONESHOT_FLEX:
            InnkeeperFlex(pPlayer, pUnit);
            break;

        case EMOTE_STATE_DANCE:
            InnkeeperDance(pPlayer, pUnit);
            break;

        case EMOTE_ONESHOT_TRAIN:
            InnkeeperTrain(pPlayer, pUnit);
            break;

        case EMOTE_ONESHOT_CHICKEN:
            InnkeeperChicken(pPlayer, pUnit);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// On areatrigger

void InvasionPointCataclysm(Player* pPlayer, uint32_t /*AreaTrigger*/)
{
    if (pPlayer->hasQuestInQuestLog(10766))
        pPlayer->SafeTeleport(530, 0, -2723.674561f, 1952.664673f, 146.939743f, 3.185559f);
}

void Scratches(Player* pPlayer, uint32_t /*AreaTrigger*/)
{
    if (auto* questLog = pPlayer->getQuestLogByQuestId(10556))
    {
        Creature* Kaliri = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 21468);
        if (Kaliri == nullptr)
            return;

        Kaliri->Despawn(0, 0);
        questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
        questLog->SendUpdateAddKill(0);
        questLog->updatePlayerFields();
    }
}

void OnAreaTrigger(Player* pPlayer, uint32_t AreaTrigger)
{
    switch(AreaTrigger)
    {
        case 4546:
            Scratches(pPlayer, 4546);
            break;
        case 4560:
            InvasionPointCataclysm(pPlayer, 4560);
            break;
    }
}

void SetupQuestHooks(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT, (void*)&OnQuestAccept);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_QUEST_FINISHED, (void*)&OnQuestFinished);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED, (void*)&OnQuestCancelled);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)&OnEmote);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_AREATRIGGER, (void*)&OnAreaTrigger);
}
