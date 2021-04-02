/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
// Fel Orc Scavengers
class FelOrcScavengersQAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FelOrcScavengersQAI)
    explicit FelOrcScavengersQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(10482, 0, 0);
        }
    }
};

class Dreadtusk : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Dreadtusk)
    explicit Dreadtusk(Creature* pCreature) : CreatureAIScript(pCreature) { }
    void OnDied(Unit* mKiller)
    {
        if (!mKiller->isPlayer())
            return;

        static_cast<Player*>(mKiller)->AddQuestKill(10255, 0, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zeth'Gor Must Burn!
class ZethGorMustBurnAlliance : public GameObjectAIScript
{
public:

    explicit ZethGorMustBurnAlliance(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ZethGorMustBurnAlliance(GO); }

    void OnActivate(Player* pPlayer)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(10895))
        {
            LocationVector pos = pPlayer->GetPosition();

            GameObject* pBeacon = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(pos.x, pos.y, pos.z, 184661);
            if (pBeacon != nullptr && pBeacon->getFlags() > 0)
            {
                pBeacon->removeFlags(GO_FLAG_NONSELECTABLE);
            }

            // Northern Zeth'Gor Tower
            if (questLog->getMobCountByIndex(0) < questLog->getQuestProperties()->required_mob_or_go_count[0])
            {
                GameObject* pNorthern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-820.0f, 2029.0f, 55.0f, 300150);
                if (pNorthern != nullptr && pPlayer->CalcDistance(pPlayer, pNorthern) < 40)      // if reduced the server will crash when out of range
                {
                    pPlayer->AddQuestKill(10895, 0, 0);

                    GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -819.77f, 2029.09f, 55.6082f, 0, 4);
                    if (pGameobject != nullptr)
                        pGameobject->Despawn(1 * 60 * 1000, 0);

                    return;
                }
            }

            // Southern Zeth'Gor Tower
            if (questLog->getMobCountByIndex(1) < questLog->getQuestProperties()->required_mob_or_go_count[1])
            {
                GameObject* pSouthern = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-1150.0f, 2110.0f, 84.0f, 300150);
                if (pSouthern != NULL && pPlayer->CalcDistance(pPlayer, pSouthern) < 40)
                {
                    pPlayer->AddQuestKill(10895, 1, 0);

                    GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -1150.53f, 2109.92f, 84.4204f, 0, 4);
                    if (pGameobject != nullptr)
                        pGameobject->Despawn(1 * 60 * 1000, 0);

                    return;
                }
            }

            // Forge Zeth'Gor Tower
            if (questLog->getMobCountByIndex(2) < questLog->getQuestProperties()->required_mob_or_go_count[2])
            {
                GameObject* pForge = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-893.0f, 1919.0f, 82.0f, 300150);
                if (pForge != NULL && pPlayer->CalcDistance(pPlayer, pForge) < 40)
                {
                    pPlayer->AddQuestKill(10895, 2, 0);

                    GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -893.499f, 1919.27f, 81.6449f, 0, 4);
                    if (pGameobject != nullptr)
                        pGameobject->Despawn(1 * 60 * 1000, 0);

                    return;
                }
            }

            // Foothill Zeth'Gor Tower
            if (questLog->getMobCountByIndex(3) < questLog->getQuestProperties()->required_mob_or_go_count[3])
            {
                GameObject* pFoothill = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-978.0f, 1879.0f, 111.0f, 300150);
                if (pFoothill != NULL && pPlayer->CalcDistance(pPlayer, pFoothill) < 40)
                {
                    pPlayer->AddQuestKill(10895, 3, 0);

                    GameObject* pGameobject = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(183816, -977.713f, 1879.500f, 110.892f, 0, 4);
                    if (pGameobject != nullptr)
                        pGameobject->Despawn(1 * 60 * 1000, 0);

                    return;
                }
            }
            else
            {
                pPlayer->BroadcastMessage("You are too far away!");
            }
        }
        else
        {
            pPlayer->BroadcastMessage("Missing required quest : Zeth'Gor Must Burn");
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Dreghood Elders
class PrisonerGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* pPlayer) override
    {
        int32_t i = -1;
        Creature* pPrisoner = static_cast<Creature*>(pObject);
        switch (pPrisoner->getEntry())
        {
            case 20677:
                i = 0;
                break;
            case 20678:
                i = 1;
                break;
            case 20679:
                i = 2;
                break;
        }

        if (i == -1)
            return;

        if (auto* questLog = pPlayer->getQuestLogByQuestId(10368))
        {
            if (questLog->getMobCountByIndex(i) < questLog->getQuestProperties()->required_mob_or_go_count[i])
            {
                if (pPlayer->getItemInterface()->GetItemCount(29501) > 0)
                {
                    GossipMenu menu(pObject->getGuid(), 10104, pPlayer->GetSession()->language);
                    menu.addItem(GOSSIP_ICON_CHAT, 463, 1);     // Walk free, Elder. Bring the spirits back to your tribe.
                    menu.sendGossipPacket(pPlayer);
                }
            }
        }
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {

        uint8_t i = 66;
        Creature* pPrisoner = static_cast<Creature*>(pObject);
        switch (pPrisoner->getEntry())
        {
            case 20677:
                i = 0;
                break;
            case 20678:
                i = 1;
                break;
            case 20679:
                i = 2;
                break;
        }

        if (i == 66)
            return;

        pPlayer->AddQuestKill(10368, i, 0);

        pPrisoner->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You've freed me! The winds speak to my people one again and grant us their strength. I thank you, stranger.");
        pPrisoner->Despawn(5000, 6 * 60 * 1000);
        pPrisoner->setStandState(STANDSTATE_STAND);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
class PrisonersDreghoodElders : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrisonersDreghoodElders)
    explicit PrisonersDreghoodElders(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_SIT);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
class AncestralSpiritWolf : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AncestralSpiritWolf)
    explicit AncestralSpiritWolf(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnLoad() override
    {
        getCreature()->castSpell(getCreature(), 29938, false);
    }
};

class HellfireDeadNPC : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HellfireDeadNPC)
    explicit HellfireDeadNPC(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};

class DarkTidingsAlliance : public QuestScript
{
public:

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 17479);
        if (pCreature == nullptr)
            return;

        char msg[100];
        sprintf(msg, "Psst, %s, get over here.", pPlayer->getName().c_str());
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg);    // Changed Player to Creature. I wonder if it was blizzlike
    }
};

class DarkTidingsHorde : public QuestScript
{
public:

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 17558);
        if (pCreature == nullptr)
            return;

        char msg[100];
        sprintf(msg, "Psst, %s, get over here.", pPlayer->getName().c_str());
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg);
    }
};

void SetupHellfirePeninsula(ScriptMgr* mgr)
{
    // Finished
    mgr->register_creature_script(16772, &FelOrcScavengersQAI::Create);
    mgr->register_creature_script(19701, &FelOrcScavengersQAI::Create);
    mgr->register_creature_script(16876, &FelOrcScavengersQAI::Create);
    mgr->register_creature_script(16925, &FelOrcScavengersQAI::Create);
    mgr->register_creature_script(18952, &FelOrcScavengersQAI::Create);

    mgr->register_creature_script(16992, &Dreadtusk::Create);

    mgr->register_gameobject_script(184661, &ZethGorMustBurnAlliance::Create);

    mgr->register_creature_gossip(20677, new PrisonerGossip());
    mgr->register_creature_gossip(20678, new PrisonerGossip());
    mgr->register_creature_gossip(20679, new PrisonerGossip());

    // NPC States
    mgr->register_creature_script(20677, &PrisonersDreghoodElders::Create);
    mgr->register_creature_script(20678, &PrisonersDreghoodElders::Create);
    mgr->register_creature_script(20679, &PrisonersDreghoodElders::Create);
    mgr->register_creature_script(17405, &HellfireDeadNPC::Create);
    mgr->register_creature_script(16852, &HellfireDeadNPC::Create);
    mgr->register_creature_script(20158, &HellfireDeadNPC::Create);

    mgr->register_quest_script(9587, new DarkTidingsAlliance());
    mgr->register_quest_script(9588, new DarkTidingsHorde());

    mgr->register_creature_script(17077, &AncestralSpiritWolf::Create);


    //\todo mgr->register_dummy_spell(35460, &FuryOfTheDreghoodElders);
}
