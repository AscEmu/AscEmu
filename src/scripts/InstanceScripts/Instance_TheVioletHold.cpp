/*
 * AscEmu Framework based on Arcemu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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
#include "Instance_TheVioletHold.h"

struct Location EventPreGuardSP[] = //PortalGuard spwns
{
    { 1888.046265f, 761.654053f, 47.667f, 2.2332f }, // [0] left
    { 1928.545532f, 803.849731f, 52.411f, 3.1223f }, // [1] center
    { 1878.080933f, 844.850281f, 43.334f, 4.2376f }  // [2] right
};

struct Location EventPreGuardWP[] = //PortalGuard WPs
{
    { 1858.386353f, 812.804993f, 42.9995f, 4.2376f }, // [0] left
    { 1861.916382f, 803.873230f, 43.6728f, 3.1223f }, // [1] center
    { 1858.678101f, 796.081970f, 43.1944f, 2.2332f }  // [2] right
};

enum DataIndex
{
    TVH_PHASE_1 = 0, // main event
    TVH_PHASE_2 = 1, // 1. portal
    TVH_PHASE_3 = 2, // 2. portal
    TVH_PHASE_4 = 3, // 3. portal
    TVH_PHASE_5 = 4, // 4. portal
    TVH_PHASE_6 = 5, // 5. portal
    TVH_PHASE_DONE = 6, // 6. portal

    TVH_END = 7
};

enum TVH_ENTRIES
{
    //Map stuff
    AREA_VIOLET_HOLD = 4415,

    //Main event
    CN_LIEUTNANT_SINCLARI = 30658,
    CN_VIOLET_HOLD_GUARD = 30659,
    CN_PORTAL_GUARDIAN = 30660, //enemies
    CN_PORTAL_INTRO = 31011, //portals, not a go its a creature ;)
    CN_CRYSTAL_SYSTEM = 30837, // NPC with spell arcane spher

    //Portal Guardians (Normal)
    CN_AZURE_INVADER = 30661,
    CN_AZURE_SPELLBREAKER = 30662,
    CN_AZURE_BINDER = 30663,
    CN_AZURE_MAGE_SLAYER = 30664,
    CN_AZURE_CAPTAIN = 30666,
    CN_AZURE_SORCEROR = 30667,
    CN_AZURE_RAIDER = 30668,
    CN_AZURE_STALKER = 32191,

    //Bosses
    CN_EREKEM = 29315,
    CN_MORAGG = 29316,
    CN_ICHORON = 29313,
    CN_XEVOZZ = 29266,
    CN_LAVANTHOR = 29312,
    CN_TURAMAT_THE_OBLITERATOR = 29314,
    CN_CYANIGOSA = 31134,

    //Spell Crytals
    SPELL_ARCANE_LIGHTNING = 57930,

    //Crystals
    GO_INTRO_ACTIVATION_CRYSTAL = 193615,
    GO_ACTIVATION_CRYSTAL = 193611,

    //Door
    GO_PRISON_SEAL = 191723,
    GO_XEVOZZ_DOOR = 191556,
    GO_LAVANTHOR_DOOR = 191566,
    GO_ICHORON_DOOR = 191722,
    GO_ZURAMAT_THE_OBLITERATOR_DOOR = 191565,
    GO_EREKEM_DOOR = 191564,
    GO_MORAGG_DOOR = 191606
};

///////////////////////////////////////////////////////
//TheVioletHold Instance
class TheVioletHoldScript : public MoonInstanceScript
{
    friend class SinclariGossip; // Friendship forever ;-)

private:
    int32 m_numBarrel;
    uint32 m_phaseData[TVH_END];

public:
    MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(TheVioletHoldScript, MoonInstanceScript);
    TheVioletHoldScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
    {
        m_numBarrel = 0;

        for (int i = 0; i < TVH_END; ++i)
            m_phaseData[i] = State_NotStarted;
    };

    void SetData(uint32 pIndex, uint32 pData)
    {
        if (pIndex >= TVH_END)
            return;

        // If Data = MainEvent, set state "PreProgress". Gossip Sinclar 1 + 2
        if (pIndex == TVH_PHASE_1)
            mInstance->GetWorldStatesHandler().SetWorldStateForZone(0, AREA_VIOLET_HOLD, WORLDSTATE_VH, State_PreProgress);

        // If Data = second event, set state "InProgress". Gossip Sinclari Case 3
        if (pIndex == TVH_PHASE_2)
            mInstance->GetWorldStatesHandler().SetWorldStateForZone(0, AREA_VIOLET_HOLD, WORLDSTATE_VH, State_InProgress);

        m_phaseData[pIndex] = pData;
    };

    uint32 GetData(uint32 pIndex)
    {
        // If Phase = End/finishes, reset the Phases to 0
        if (pIndex >= TVH_END)
            return 0;

        return m_phaseData[pIndex];
    };

    void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
    {
        if (pType != Data_EncounterState || pIndex == 0)
            return;

        EncounterMap::iterator Iter = mEncounters.find(pIndex);
        if (Iter == mEncounters.end())
            return;

        (*Iter).second.mState = (EncounterState)pData;
    };

    uint32 GetInstanceData(uint32 pType, uint32 pIndex)
    {
        if (pType != Data_EncounterState || pIndex == 0)
            return 0;

        EncounterMap::iterator Iter = mEncounters.find(pIndex);
        if (Iter == mEncounters.end())
            return 0;

        return (*Iter).second.mState;
    };

    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
    {
    };

    void OnPlayerEnter(Player* pPlayer)
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_NotStarted)
        {
            mEncounters.insert(EncounterMap::value_type(MAP_VIOLET_HOLD, State_NotStarted));
        }

    }
};

#define SINCLARI_SAY_1 "Prison guards, we are oleaving! These adventurers are taking over! Go go go!"
#define SINCLARY_SAY_2 "I'm locking the door. Good luck, and thank you for doing this."

#define GO_TVH_PRISON_SEAL 191723

#define SINCLARI_MAX_WP 4

///////////////////////////////////////////////////////
//Lieutnant Sinclari StartEvent
class SinclariAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(SinclariAI, MoonScriptCreatureAI);
    SinclariAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        switch (iWaypointId)
        {
            case 2:
            {
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARI_SAY_1);
                _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_FORWARDTHENSTOP);
            }
            break;

            case 4:
            {
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARY_SAY_2);
                _unit->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                TheVioletHoldScript* pInstance = (TheVioletHoldScript*)_unit->GetMapMgr()->GetScript();
                pInstance->SetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD, State_InProgress);

                GameObject* pVioletHoldDoor = pInstance->FindClosestGameObjectOnMap(GO_TVH_PRISON_SEAL, 1822.59f, 803.93f, 44.36f);
                if (pVioletHoldDoor != NULL)
                    pVioletHoldDoor->SetState(GAMEOBJECT_STATE_CLOSED);
            }
            break;
        }
    }
};

enum eGossipTexts
{
    SINCLARI_ON_HELLO = 13853,
    SINCLARI_ON_FINISH = 13854,
    SINCLARI_OUTSIDE = 14271
};

enum GossipItem
{
    SINCLARI_ACTIVATE = 600,
    SINCLARI_GET_SAFETY = 601,
    SINCLARI_SEND_ME_IN = 602
};


///////////////////////////////////////////////////////
//Lieutnant Sinclari Gossip and init events
//Sinclari Gossip
class SinclariGossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* pPlayer)
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu* menu;

        //Page 1: Textid and first menu item
        if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_NotStarted)
        {
            objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_ON_HELLO, pPlayer);
            menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_ACTIVATE), 1);

            menu->SendTo(pPlayer);
        }

        //If VioletHold is started, Sinclari has this item for people who aould join.
        if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_InProgress)
        {
            objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_OUTSIDE, pPlayer);
            menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_SEND_ME_IN), 3);

            menu->SendTo(pPlayer);
        }
    };

    void GossipSelectOption(Object* pObject, Player*  pPlayer, uint32 Id, uint32 IntId, const char* Code)
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();

        if (!pInstance)
            return;

        if(!pObject->IsCreature())
            return;

        Creature* pCreature = TO_CREATURE(pObject);

        switch (IntId)
        {
            case 0:
                GossipHello(pObject, pPlayer);
                break;

            case 1:
            {
                  GossipMenu* menu;
                  objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_ON_FINISH, pPlayer);
                  menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_GET_SAFETY), 2);
                  menu->SendTo(pPlayer);

                  // New Encounter State included
                  pInstance->SetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD, State_PreProgress);
            }break;

            case 2:
            {
                      TO_CREATURE(pObject)->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
                      pCreature->GetAIInterface()->setMoveType(MOVEMENTTYPE_NONE);
                      //pCreature->MoveToWaypoint(1);
                      pCreature->GetAIInterface()->StopMovement(10);

            }break;

            case 3:
            {
                  Arcemu::Gossip::Menu::Complete(pPlayer);
                  pPlayer->SafeTeleport(pPlayer->GetInstanceID(), MAP_VIOLET_HOLD, 1830.531006f, 803.939758f, 44.340508f, 6.281611f);
            }break;
        }
    }
};

///////////////////////////////////////////////////////
//VH Guards
class VHGuardsAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(VHGuardsAI, MoonScriptCreatureAI);
    VHGuardsAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
    }

    //WPs inserted in db.

};

///////////////////////////////////////////////////////
//Boss: Erekem
//class ErekemAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Moragg
//class MoraggAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Ichoron
//class IchoronAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Xevozz
//class XevozzAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Lavanthos
//class LavanthosAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Zuramat the Obliterator
//class ZuramatTheObliteratorAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Final Boss: Cyanigosa
//class CyanigosaAI : public CreatureAIScript


void SetupTheVioletHold(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldScript::Create);

    //Sinclari and Guards
    mgr->register_creature_script(CN_LIEUTNANT_SINCLARI, &SinclariAI::Create);
    mgr->register_creature_script(CN_VIOLET_HOLD_GUARD, &VHGuardsAI::Create);

    //Bosses
    //mgr->register_creature_script(CN_EREKEM, &ErekemAI::Create);
    //mgr->register_creature_script(CN_MORAGG, &MoraggAI::Create);
    //mgr->register_creature_script(CN_ICHORON, &IchoronAI::Create);
    //mgr->register_creature_script(CN_XEVOZZ, &XevozzAI::Create);
    //mgr->register_creature_script(CN_LAVANTHOR, &LavanthorAI::Create);
    //mgr->register_creature_script(CN_TURAMAT_THE_OBLITERATOR, &ZuramatTheObliteratorAI::Create);
    //mgr->register_creature_script(CN_CYANIGOSA, &CyanigosaAI::Create);

    GossipScript* GSinclari = new SinclariGossip;
    mgr->register_gossip_script(CN_LIEUTNANT_SINCLARI, GSinclari);


}