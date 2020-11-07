/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2010 ArcEmu Team <http://www.arcemu.org/>
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

/* Script Comment:
 * Everything before escort event is made to work as blizzlike as possible!
 * This is still in development and is tagged[WIP].
 * It is suggested to apply SQL updates to see this script fully working!
 * Have fun ~Azolex
 */

 // \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_OldHillsbradFoothills.h"
#include "Management/TaxiMgr.h"
#include "Management/WorldStates.h"

class OldHilsbradInstance : public InstanceScript
{
private:

    int32_t m_numBarrel;
    uint32_t m_phaseData[OHF_END];

public:

    explicit OldHilsbradInstance(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        m_numBarrel = 0;

        for (uint8_t i = 0; i < OHF_END; ++i)
            m_phaseData[i] = OHF_DATA_NOT_STARTED;
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new OldHilsbradInstance(pMapMgr); }

    void OnPlayerEnter(Player* pPlayer)
    {
        if (pPlayer->getGender() == 0)
            pPlayer->castSpell(pPlayer, 35482, true);   // Human Male illusion
        else
            pPlayer->castSpell(pPlayer, 35483, true);   // Human Female illusion
    }

    void SetData(uint32_t pIndex, uint32_t pData)
    {
        if (pIndex >= OHF_END)
            return;

        if (pIndex == OHF_PHASE_2)
            mInstance->GetWorldStatesHandler().SetWorldStateForZone(2367, 0, WORLDSTATE_OLD_HILLSBRAD_BARRELS, 0);

        m_phaseData[pIndex] = pData;
    }

    uint32_t GetData(uint32_t pIndex)
    {
        if (pIndex >= OHF_END)
            return 0;

        return m_phaseData[pIndex];
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
    {
        if (pGameObject->getEntry() != GO_LODGE_ABLAZE || GetData(OHF_PHASE_1) == OHF_DATA_DONE)
            return;

        pGameObject->Despawn(1000, 0);
        m_numBarrel++;
        pGameObject->GetMapMgr()->GetWorldStatesHandler().SetWorldStateForZone(2367, 0, WORLDSTATE_OLD_HILLSBRAD_BARRELS, m_numBarrel);
        if (m_numBarrel != 5)
            return;

        SetData(OHF_PHASE_1, OHF_DATA_DONE);

        for (PlayerStorageMap::iterator itr = mInstance->m_PlayerStorage.begin(); itr != mInstance->m_PlayerStorage.end(); ++itr)
        {
            pPlayer = itr->second;

            QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(10283);
            if (!qle)
                continue;

            qle->setMobCountForIndex(0, qle->getMobCountByIndex(0) + 1);
            qle->SendUpdateAddKill(0);
            qle->updatePlayerFields();
        }

        for (uint8_t i = 0; i < 21; ++i)
        {
            GameObject* pGO = spawnGameObject(GO_FIRE, Fires[i].x, Fires[i].y, Fires[i].z, Fires[i].o);
            if (pGO != nullptr)
                pGO->Despawn(10 * 60 * 1000, 0);
        }

        spawnCreature(CN_LIEUTENANT_DRAKE, 2118.310303f, 89.565969f, 52.453037f, 2.027089f);
    }
};

/*
class ErozionGossip : public Script
{
public:

    void OnHello(Object* pObject, Player* Plr)
    {
        OldHilsbradInstance* pInstance = dynamic_cast<OldHilsbradInstance*>(pObject->GetMapMgr()->GetScript());
        if (!pInstance)
            return;

        Menu menu(pObject->getGuid(), EROZION_ON_HELLO, 0);
        if (pInstance->GetData(OHF_PHASE_1) != OHF_DATA_DONE && !Plr->HasItemCount(25853, 1))
            menu.addItem(GOSSIP_ICON_CHAT, EROZION_NEED_PACKET, 1);

        // It should give another menu if instance is done id: 10474, NYI
        menu.sendGossipPacket(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* Code, uint32_t gossipId)
    {
        switch (Id)
        {
            case 1:
                Item* pBombs = sObjectMgr.CreateItem(25853, Plr);
                if (pBombs)
                    if (!Plr->getItemInterface()->AddItemToFreeSlot(pBombs))
                        pBombs->DeleteMe();
                break;
        }
    }
};

class BrazenGossip : public Script
{
public:

    void OnHello(Object* pObject, Player* Plr)
    {
        Menu menu(pObject->getGuid(), BRAZAN_ON_HELLO, 0);
        menu.addItem(GOSSIP_ICON_CHAT, BRAZAN_DURNHOLDE_KEEP, 1);
        menu.sendGossipPacket(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* Code, uint32_t gossipId)
    {
        switch (Id)
        {
            case 1:
            {
                if (!Plr->HasItemCount(25853, 1))
                {
                    Menu menu(pObject->getGuid(), BRAZAN_NEED_ITEM, 0);
                    menu.sendGossipPacket(Plr);
                }
                else
                {
                    Plr->TaxiStart(sTaxiMgr.GetTaxiPath(534), 8317, 0);
                    Plr->removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
                }
            }
            break;
        }
    }

};

class LieutenantDrakeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LieutenantDrakeAI)

    OldHilsbradInstance* pInstance;

    explicit LieutenantDrakeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pInstance = dynamic_cast<OldHilsbradInstance*>(GetInstanceScript());
    }

    void OnCombatStart(Unit* pTarget)
    {
        if (pInstance)
            pInstance->SetData(OHF_PHASE_2, OHF_DATA_IN_PROGRESS); 
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (pInstance)
            pInstance->SetData(OHF_PHASE_2, OHF_DATA_PERFORMED);
    }
};

class ThrallAI : public CreatureAIScript // this will be replaced with escortAI
{
    ADD_CREATURE_FACTORY_FUNCTION(ThrallAI)
    explicit ThrallAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        for (uint8_t i = 1; i < MAX_THRALLWP1; ++i)
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_WALK, ThrallWP1[i]));

        m_currentWp = 0;
    }

    void StartEscort(Player* pPlayer)
    {
        GameObject* pGO = getNearestGameObject();
        if (pGO)
            pGO->setState(pGO->getState() == 1 ? 0 : 1);

        _unit->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
    }

    void OnCombatStop(Unit* pTarget)
    {
        SetWaypointToMove(m_currentWp);
    }

    void OnReachWP(uint32_t iWaypointId, bool bForwards)
    {
        m_currentWp = iWaypointId;
    }

    uint32_t m_currentWp;
};

class ThrallGossip : public Script
{
public:

    void OnHello(Object* pObject, Player* Plr)
    {
        Menu menu(pObject->getGuid(), THRALL_ON_HELLO, 0);
        menu.addItem(GOSSIP_ICON_CHAT, THRALL_START_ESCORT, 1);
        menu.sendGossipPacket(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* Code, uint32_t gossipId)
    {
        ThrallAI* pThrall = static_cast<ThrallAI*>(static_cast<Creature*>(pObject)->GetScript());
        if (pThrall)
            pThrall->StartEscort(Plr);
    }
};*/

void SetupOldHillsbradFoothills(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_OLD_HILSBRAD, &OldHilsbradInstance::Create);
    /*mgr->register_creature_script(CN_LIEUTENANT_DRAKE, &LieutenantDrakeAI::Create);
    mgr->register_creature_script(CN_THRALL, &ThrallAI::Create);

    Script* eGossip = new ErozionGossip();
    mgr->register_creature_gossip(CN_EROZION, eGossip);
    Script* bGossip = new BrazenGossip();
    mgr->register_creature_gossip(CN_BRAZEN, bGossip);
    Script* tGossip = new ThrallGossip();
    mgr->register_creature_gossip(CN_THRALL, tGossip);*/
}