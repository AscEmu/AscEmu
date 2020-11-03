/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
#include "Instance_Deadmines.h"
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


class DeadminesInstanceScript : public InstanceScript
{
public:

    explicit DeadminesInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        mFactoryDoor_GUID = 0;
        mDefiasCannon_GUID = 0;
        mDoorLever_GUID = 0;
        mMrSmiteChest_GUID = 0;
        mIronCladDoor_GUID = 0;
        InstanceEncounter = 0;

        if (getData(NPC_RHAHK_ZOR) == Finished)
        {
            setGameObjectStateForEntry(GO_FACTORY_DOOR, GO_STATE_OPEN);
        }

        if (getData(NPC_MR_SMITE) == Finished)
        {
            setGameObjectStateForEntry(GO_IRONCLAD_DOOR, GO_STATE_OPEN);
        }

    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new DeadminesInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject)
    {
        switch (pGameObject->getEntry())
        {
        case GO_FACTORY_DOOR:
            mFactoryDoor_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;

        case GO_FACTORY_DOOR_LEVER:
            mDoorLever_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;

        case GO_IRONCLAD_DOOR:
            mIronCladDoor_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        }
    }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        switch (pCreature->getEntry())
        {
            case NPC_RHAHK_ZOR:
            {
                GameObject* pDoor1 = GetGameObjectByGuid(mFactoryDoor_GUID);
                if (pDoor1 != NULL)
                    pDoor1->setState(GO_STATE_OPEN);
            }break;
            case NPC_SNEEDS_SHREDDER:
                spawnCreature(NPC_SNEED, pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), pCreature->GetOrientation());
                break;
            case NPC_GILNID:
            {
                GameObject* pDoor2 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[0].x, Doors[0].y, Doors[0].z);
                if (pDoor2 != NULL)
                    pDoor2->setState(GO_STATE_OPEN);
            }break;
            case NPC_SNEED:
            {
                GameObject* pDoor3 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[1].x, Doors[1].y, Doors[1].z);
                if (pDoor3 != NULL)
                    pDoor3->setState(GO_STATE_OPEN);
            }break;
        }
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
    {
        switch (pGameObject->getEntry())
        {
            case GO_DEFIAS_CANNON:
            {
                GameObject* pDoor4 = GetGameObjectByGuid(mIronCladDoor_GUID);
                if (pDoor4 != NULL && pDoor4->getState() != GO_STATE_ALTERNATIVE_OPEN)
                    pDoor4->setState(GO_STATE_ALTERNATIVE_OPEN);
            }break;

            case GO_FACTORY_DOOR_LEVER:
            {
                GameObject* pDoor5 = GetGameObjectByGuid(mFactoryDoor_GUID);
                if (pDoor5 != NULL)
                    pDoor5->setState(pDoor5->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }break;

            case GO_IRONCLAD_LEVER:
            {
                GameObject* pDoor6 = GetGameObjectByGuid(mFactoryDoor_GUID);
                //Door can be opened by lever if state isn't 2
                if (pDoor6 != NULL && pDoor6->getState() != GO_STATE_ALTERNATIVE_OPEN)
                    pDoor6->setState(pDoor6->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }break;

            case GO_SNEED_DOOR_LEVER:
            {
                GameObject* pDoor7 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[1].x, Doors[1].y, Doors[1].z);
                if (pDoor7 != NULL)
                    pDoor7->setState(pDoor7->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }break;

            case GO_GILNID_DOOR_LEVER:
            {
                GameObject* pDoor8 = getClosestGameObjectForPosition(GO_HEAVY_DOOR, Doors[0].x, Doors[0].y, Doors[0].z);
                if (pDoor8 != NULL)
                    pDoor8->setState(pDoor8->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }break;
        }
    }

protected:

    uint32_t mFactoryDoor_GUID;
    uint32_t mDefiasCannon_GUID;
    uint32_t mDoorLever_GUID;
    uint32_t mMrSmiteChest_GUID;
    uint32_t mIronCladDoor_GUID;
    uint32_t InstanceEncounter;
};

class RhahkZorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RhahkZorAI)
    explicit RhahkZorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(6304, 8.0f, TARGET_ATTACKING, 1500, 3);    // Rhahk'Zor Slam

        addEmoteForEvent(Event_OnCombatStart, 5495);     // VanCleef pay big for you heads!
    }

};


class MrSmiteAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MrSmiteAI)
    explicit MrSmiteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto smiteSlam = addAISpell(SMITE_SLAM, 25.0f, TARGET_ATTACKING, 0, 15, false, true);
        smiteSlam->setMinMaxDistance(0.0f, 8.0f);

        mStomp = addAISpell(SMITE_STOMP, 0.0f, TARGET_SELF);
        mWaitAtChest = 0;
        _setWieldWeapon(true);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isScriptPhase(4))
            _removeAura(SMITES_HAMMER);

        if (!isAlive())
            _setWieldWeapon(false);

        setScriptPhase(1);
        SwitchWeapons();
        _removeTimer(mWaitAtChest);
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 66 && isScriptPhase(1))
            setScriptPhase(2);
        else if (_getHealthPercent() <= 33 && isScriptPhase(3))
            setScriptPhase(4);

        if (isScriptPhase(2) || isScriptPhase(4))
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
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
            case 2:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 5778, "You landlubbers are tougher than I thought. I'll have to improvise!");
                _castAISpell(mStomp);
                break;
            case 4:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 5779, "D'ah! Now you're making me angry!");
                _castAISpell(mStomp);
                break;
            default:
                break;
        }
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

        if (getCreature()->GetAIInterface()->isAiState(AI_STATE_SCRIPTMOVE) == false)
        {
            // Too small distance - let's prevent from blocking
            float XDiff, YDiff;
            XDiff = getCreature()->GetPositionX() - 1.100060f;
            YDiff = getCreature()->GetPositionY() + 780.026367f;
            float Distance = static_cast<float>(std::sqrt(XDiff * XDiff + YDiff * YDiff));
            if (Distance <= 5.0f)
                return true;
        }

        return false;
    }

    void SwitchWeapons()
    {
        // CREDITS to Skyboat on ascentemu.com/forums  he had some of this info on one of his releases
        switch (getScriptPhase())
        {
            case 1: // Phase 1 (Default)
                _setDisplayWeaponIds(5192, 0);
                getCreature()->setBaseAttackTime(MELEE, getCreature()->getBaseAttackTime(MELEE));    // 1483 is taken from NCDB creature_proto
                break;
            case 2: // Phase 2
                _setDisplayWeaponIds(5196, 5196);
                getCreature()->setBaseAttackTime(MELEE, getCreature()->getBaseAttackTime(MELEE) / 2);
                break;
            case 4: // Phase 4
                // Is base attack time change needed if we use aura ?
                _setDisplayWeaponIds(7230, 0);
                getCreature()->setBaseAttackTime(MELEE, getCreature()->getBaseAttackTime(MELEE) * 2);
                _applyAura(SMITES_HAMMER);
                break;
        }

        // Wait at the chest for 4.5seconds -- Still needs work
        getCreature()->setAttackTimer(MELEE, 4500);
        mWaitAtChest = _addTimer(4500);
        setScriptPhase(getScriptPhase() + 1);
    }

protected:

    CreatureAISpells* mStomp;
    uint32_t mWaitAtChest;
};

class VanCleefAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VanCleefAI)
    explicit VanCleefAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(3391, 25.0f, TARGET_SELF); // Thrash (Gives the caster 2 extra attacks.)

        addEmoteForEvent(Event_OnCombatStart, 7722); // None may challenge the Brotherhood!
        addEmoteForEvent(Event_OnDied, 7727); // The Brotherhood shall prevail!
    }

    void OnTargetDied(Unit* pTarget) override
    {
        char msg[200];
        if (pTarget->isPlayer())
            sprintf(msg, "And stay down, %s.", static_cast<Player*>(pTarget)->getName().c_str());
        else if (pTarget->GetTypeFromGUID() == HIGHGUID_TYPE_PET)
            sprintf(msg, "And stay down, %s.", static_cast<Pet*>(pTarget)->GetName().c_str());

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 5781, msg);
    }

    void AIUpdate() override
    {
        // case for scriptPhase
        if (_getHealthPercent() <= 75 && isScriptPhase(1))
        {
            sendDBChatMessage(7723);     // Lapdogs, all of you!
            setScriptPhase(2);
        }
        else if (_getHealthPercent() <= 50 && isScriptPhase(2))
        {
            sendDBChatMessage(7725);     // Fools! Our cause is righteous!

            for (uint8_t x = 0; x < 2; x++)
            {
                Creature* Guard = spawnCreature(636, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                if (Guard != nullptr)
                {
                    Guard->m_noRespawn = true;
                }
            }

            setScriptPhase(3);

        }
        else if (_getHealthPercent() <= 25 && isScriptPhase(3))
        {
            sendDBChatMessage(7727);     // The Brotherhood shall prevail!
            setScriptPhase(4);
        }
    }
};

void SetupDeadmines(ScriptMgr* mgr)
{
    mgr->register_instance_script(36, &DeadminesInstanceScript::Create);
    //mgr->register_creature_script(NPC_RHAHK_ZOR, &RhahkZorAI::Create);
    mgr->register_creature_script(NPC_MR_SMITE, &MrSmiteAI::Create);
    mgr->register_creature_script(NPC_EDWIN_VANCLEEF, &VanCleefAI::Create);
}
