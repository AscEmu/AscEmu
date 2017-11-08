/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Instance_Deadmines.h"
#include "Server/LazyTimer.h"
#include <Units/Creatures/Pet.h>


static Movement::Location Doors[] =
{
    { -168.514f, -579.861f, 19.3159f, 0 },    //Gilnid doors
    { -290.294f, -536.96f, 49.4353f, 0 }        //Sneed doors
};

//it will useful for cannon event
static Movement::Location Guards[] =
{
    { -89.7001f, -691.332f, 8.24514f, 0 }, //Guard
    { -102.521f, -697.942f, 8.84454f, 0 }, //Guard
    { -89.6744f, -694.063f, 8.43202f, 0 }  //Parrot
};

//class DeadminesInstanceScript : public InstanceScript
//{
//    public:
//
//        DeadminesInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
//        {
//            mFactoryDoor_GUID = 0;
//            mDefiasCannon_GUID = 0;
//            mDoorLever_GUID = 0;
//            mMrSmiteChest_GUID = 0;
//            mIronCladDoor_GUID = 0;
//            InstanceEncounter = 0;
//        }
//
//        static InstanceScript* Create(MapMgr* pMapMgr) { return new DeadminesInstanceScript(pMapMgr); }
//
//        void OnGameObjectPushToWorld(GameObject* pGameObject)
//        {
//            switch (pGameObject->GetEntry())
//            {
//                case GO_FACTORY_DOOR:
//                    mFactoryDoor_GUID = static_cast<uint32>(pGameObject->GetGUID());
//                    break;
//                case GO_FACTORY_DOOR_LEVER:
//                    mDoorLever_GUID = static_cast<uint32>(pGameObject->GetGUID());
//                    break;
//                case GO_IRONCLAD_DOOR:
//                    mIronCladDoor_GUID = static_cast<uint32>(pGameObject->GetGUID());
//                    break;
//            }
//        }
//
//        void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
//        {
//            switch (pGameObject->GetEntry())
//            {
//                case GO_DEFIAS_CANNON:
//                {
//                    GameObject* pDoor4 = GetGameObjectByGuid(mIronCladDoor_GUID);
//                    if (pDoor4 != NULL && pDoor4->GetState() != 2)
//                        pDoor4->SetState(2);
//                }break;
//                case GO_FACTORY_DOOR_LEVER:
//                {
//                    GameObject* pDoor5 = GetGameObjectByGuid(mFactoryDoor_GUID);
//                    if (pDoor5 != NULL)
//                        pDoor5->SetState(pDoor5->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
//                }break;
//                case GO_IRONCLAD_LEVER:
//                {
//                    GameObject* pDoor6 = GetGameObjectByGuid(mFactoryDoor_GUID);
//                    //Door can be opened by lever if state isn't 2
//                    if (pDoor6 != NULL && pDoor6->GetState() != 2)
//                        pDoor6->SetState(pDoor6->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
//                }break;
//                case GO_SNEED_DOOR_LEVER:
//                {
//                    GameObject* pDoor7 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[1].x, Doors[1].y, Doors[1].z);
//                    if (pDoor7 != NULL)
//                        pDoor7->SetState(pDoor7->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
//                }break;
//                case GO_GILNID_DOOR_LEVER:
//                {
//                    GameObject* pDoor8 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[0].x, Doors[0].y, Doors[0].z);
//                    if (pDoor8 != NULL)
//                        pDoor8->SetState(pDoor8->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
//                }break;
//            }
//        }
//
//        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
//        {
//            switch (pCreature->GetEntry())
//            {
//                case NPC_RHAHK_ZOR:
//                {
//                    GameObject* pDoor1 = GetGameObjectByGuid(mFactoryDoor_GUID);
//                    if (pDoor1 != NULL)
//                        pDoor1->SetState(GO_STATE_OPEN );
//                }break;
//                case NPC_SNEEDS_SHREDDER:
//                    SpawnCreature(NPC_SNEED, pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), pCreature->GetOrientation());
//                    break;
//                case NPC_GILNID:
//                {
//                    GameObject* pDoor2 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[0].x, Doors[0].y, Doors[0].z);
//                    if (pDoor2 != NULL)
//                        pDoor2->SetState(GO_STATE_OPEN );
//                }break;
//                case NPC_SNEED:
//                {
//                    GameObject* pDoor3 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[1].x, Doors[1].y, Doors[1].z);
//                    if (pDoor3 != NULL)
//                        pDoor3->SetState(GO_STATE_OPEN );
//                }break;
//            }
//        }
//
//    protected:
//
//        uint32 mFactoryDoor_GUID;
//        uint32 mDefiasCannon_GUID;
//        uint32 mDoorLever_GUID;
//        uint32 mMrSmiteChest_GUID;
//        uint32 mIronCladDoor_GUID;
//        uint32 InstanceEncounter;
//};

class RhahkZorAI : public MoonScriptCreatureAI
{
    // Just for testing
    LazyTimer debugTimer;

    MOONSCRIPT_FACTORY_FUNCTION(RhahkZorAI, MoonScriptCreatureAI);
    RhahkZorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature), debugTimer(1500)
    {
        AddSpell(6304, Target_Current, 8, 0, 3);    // Rhahk'Zor Slam
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(5495);     // VanCleef pay big for you heads!

        std::stringstream ss;
        ss << "Timer Init Value: " << debugTimer.getRealDelta();
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, ss.str().c_str());
    }
};


class MrSmiteAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(MrSmiteAI, MoonScriptCreatureAI);
        MrSmiteAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SMITE_SLAM, Target_Current, 25, 0.0f, 15, 0.0f, 8.0f, true);
            mStomp = AddSpell(SMITE_STOMP, Target_Self, 0, 0, 0);
            mWaitAtChest = INVALIDATE_TIMER;
            _setWieldWeapon(true);
        }

        void OnCombatStop(Unit* pTarget)
        {
            if (GetPhase() == 4)
                _removeAura(SMITES_HAMMER);

            if (!isAlive())
                _setWieldWeapon(false);

            SetPhase(1);
            SwitchWeapons();
            _removeTimer(mWaitAtChest);
            ParentClass::OnCombatStop(pTarget);
        }

        void AIUpdate()
        {
            if (_getHealthPercent() <= 66 && GetPhase() == 1)
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 5778, "You landlubbers are tougher than I thought. I'll have to improvise!");
                SetPhase(2, mStomp);
            }
            else if (_getHealthPercent() <= 33 && GetPhase() == 3)
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 5779, "D'ah! Now you're making me angry!");
                SetPhase(4, mStomp);
            }

            if (GetPhase() == 2 || GetPhase() == 4)
            {
                if (NearChest())
                    SwitchWeapons();
                else if (getCreature()->GetAIInterface()->isAiState(AI_STATE_SCRIPTMOVE) == false)
                {
                    MoveToChest();
                }
            }

            if (_isTimerFinished(mWaitAtChest))
                MoveToPlayer();

            ParentClass::AIUpdate();
        }

        void MoveToChest()
        {
            if (canEnterCombat())
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

            stopMovement();
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            moveTo(1.100060f, -780.026367f, 9.811194f);
        }

        void MoveToPlayer()
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
        }

        bool NearChest()
        {
            if (getCreature()->GetPositionX() == 1.100060f && getCreature()->GetPositionY() == -780.026367f)
                return true;
            else if (getCreature()->GetAIInterface()->isAiState(AI_STATE_SCRIPTMOVE) == false)
            {
                // Too small distance - let's prevent from blocking
                float XDiff, YDiff;
                XDiff = getCreature()->GetPositionX() - 1.100060f;
                YDiff = getCreature()->GetPositionY() + 780.026367f;
                float Distance = static_cast<float>(sqrt(XDiff * XDiff + YDiff * YDiff));
                if (Distance <= 5.0f)
                    return true;
            }

            return false;
        }

        void SwitchWeapons()
        {
            // CREDITS to Skyboat on ascentemu.com/forums  he had some of this info on one of his releases
            switch (GetPhase())
            {
                case 1: // Phase 1 (Default)
                    _setDisplayWeaponIds(5192, 0);
                    getCreature()->SetBaseAttackTime(MELEE, getCreature()->GetBaseAttackTime(MELEE));    // 1483 is taken from NCDB creature_proto
                    break;
                case 2: // Phase 2
                    _setDisplayWeaponIds(5196, 5196);
                    getCreature()->SetBaseAttackTime(MELEE, getCreature()->GetBaseAttackTime(MELEE) / 2);
                    break;
                case 4: // Phase 4
                    // Is base attack time change needed if we use aura ?
                    _setDisplayWeaponIds(7230, 0);
                    getCreature()->SetBaseAttackTime(MELEE, getCreature()->GetBaseAttackTime(MELEE) * 2);
                    _applyAura(SMITES_HAMMER);
                    break;
            }

            // Wait at the chest for 4.5seconds -- Still needs work
            getCreature()->setAttackTimer(4500, false);
            mWaitAtChest = _addTimer(4500);
            SetPhase(GetPhase() + 1);
        }

    protected:

        SpellDesc* mStomp;
        uint32 mWaitAtChest;
};


// VanCleef
class VanCleefAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VanCleefAI, MoonScriptCreatureAI);
    VanCleefAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(3391, Target_Self, 25, 0, 0);    //Thrash (Gives the caster 2 extra attacks.)
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(7722);     // None may challenge the Brotherhood!
    }

    void OnTargetDied(Unit* pTarget)
    {
        char msg[200];
        if (pTarget->IsPlayer())
            sprintf(msg, "And stay down, %s.", static_cast<Player*>(pTarget)->GetName());
        else if (pTarget->GetTypeFromGUID() == HIGHGUID_TYPE_PET)
            sprintf(msg, "And stay down, %s.", static_cast<Pet*>(pTarget)->GetName().c_str());

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 5781, msg);
        ParentClass::OnTargetDied(pTarget);
    }

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(7727);     // The Brotherhood shall prevail!
    }

    void AIUpdate()
    {
        if (_getHealthPercent() <= 75 && GetPhase() == 1)
        {
            sendDBChatMessage(7723);     // Lapdogs, all of you!
            SetPhase(2);
        }
        else if (_getHealthPercent() <= 50 && GetPhase() == 2)
        {
            sendDBChatMessage(7725);     // Fools! Our cause is righteous!

            for (uint8 x = 0; x < 2; x++)
            {
                MoonScriptCreatureAI* Guard = SpawnCreature(636, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), false);
                if (Guard != NULL)
                {
                    Guard->_setDespawnWhenInactive(true);
                    Guard->getCreature()->m_noRespawn = true;
                }
            }

            SetPhase(3);

        }
        else if (_getHealthPercent() <= 25 && GetPhase() == 3)
        {
            sendDBChatMessage(7727);     // The Brotherhood shall prevail!
            SetPhase(4);
        }
        ParentClass::AIUpdate();
    }
};

void SetupDeadmines(ScriptMgr* mgr)
{
    //mgr->register_instance_script(MAP_DEADMINES, &DeadminesInstanceScript::Create);
    mgr->register_creature_script(NPC_RHAHK_ZOR, &RhahkZorAI::Create);
    mgr->register_creature_script(NPC_MR_SMITE, &MrSmiteAI::Create);
    mgr->register_creature_script(NPC_EDWIN_VANCLEEF, &VanCleefAI::Create);
}
