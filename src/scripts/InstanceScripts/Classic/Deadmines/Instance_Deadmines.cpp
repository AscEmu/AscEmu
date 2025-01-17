/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_Deadmines.h"

#include "Setup.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Objects/Units/Players/Player.hpp"

class DeadminesInstanceScript : public InstanceScript
{
public:
    explicit DeadminesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        mFactoryDoor_GUID = 0;
        mDefiasCannon_GUID = 0;
        mDoorLever_GUID = 0;
        mMrSmiteChest_GUID = 0;
        mIronCladDoor_GUID = 0;
        InstanceEncounter = 0;

        if (getBossState(Deadmines::BOSS_RHAHKZOR) == Performed)
        {
            setGameObjectStateForEntry(Deadmines::GO_FACTORY_DOOR, GO_STATE_OPEN);
        }

        if (getBossState(Deadmines::BOSS_MR_SMITE) == Performed)
        {
            setGameObjectStateForEntry(Deadmines::GO_IRONCLAD_DOOR, GO_STATE_OPEN);
        }
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DeadminesInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
        case Deadmines::GO_FACTORY_DOOR:
            mFactoryDoor_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        case Deadmines::GO_FACTORY_DOOR_LEVER:
            mDoorLever_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        case Deadmines::GO_IRONCLAD_DOOR:
            mIronCladDoor_GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        }
    }

    void OnCreatureDeath(Creature* pCreature, Unit* /*pUnit*/) override
    {
        switch (pCreature->getEntry())
        {
            case Deadmines::NPC_RHAHK_ZOR:
            {
                GameObject* pDoor1 = GetGameObjectByGuid(mFactoryDoor_GUID);
                if (pDoor1 != nullptr)
                    pDoor1->setState(GO_STATE_OPEN);
            }
            break;
            case Deadmines::NPC_SNEEDS_SHREDDER:
                spawnCreature(Deadmines::NPC_SNEED, pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), pCreature->GetOrientation());
                break;
            case Deadmines::NPC_GILNID:
            {
                GameObject* pDoor2 = getClosestGameObjectForPosition(Deadmines::GO_HEAVY_DOOR, Deadmines::Doors[0].x, Deadmines::Doors[0].y, Deadmines::Doors[0].z);
                if (pDoor2 != nullptr)
                    pDoor2->setState(GO_STATE_OPEN);
            }
            break;
            case Deadmines::NPC_SNEED:
            {
                GameObject* pDoor3 = getClosestGameObjectForPosition(Deadmines::GO_HEAVY_DOOR, Deadmines::Doors[1].x, Deadmines::Doors[1].y, Deadmines::Doors[1].z);
                if (pDoor3 != nullptr)
                    pDoor3->setState(GO_STATE_OPEN);
            }
            break;
        }
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
    {
        switch (pGameObject->getEntry())
        {
            case Deadmines::GO_DEFIAS_CANNON:
            {
                GameObject* pDoor4 = GetGameObjectByGuid(mIronCladDoor_GUID);
                if (pDoor4 != nullptr && pDoor4->getState() != GO_STATE_ALTERNATIVE_OPEN)
                    pDoor4->setState(GO_STATE_ALTERNATIVE_OPEN);
            }
            break;
            case Deadmines::GO_FACTORY_DOOR_LEVER:
            {
                GameObject* pDoor5 = GetGameObjectByGuid(mFactoryDoor_GUID);
                if (pDoor5 != nullptr)
                    pDoor5->setState(pDoor5->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }
            break;
            case Deadmines::GO_IRONCLAD_LEVER:
            {
                GameObject* pDoor6 = GetGameObjectByGuid(mFactoryDoor_GUID);
                //Door can be opened by lever if state isn't 2
                if (pDoor6 != nullptr && pDoor6->getState() != GO_STATE_ALTERNATIVE_OPEN)
                    pDoor6->setState(pDoor6->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }
            break;
            case Deadmines::GO_SNEED_DOOR_LEVER:
            {
                GameObject* pDoor7 = getClosestGameObjectForPosition(Deadmines::GO_HEAVY_DOOR, Deadmines::Doors[1].x, Deadmines::Doors[1].y, Deadmines::Doors[1].z);
                if (pDoor7 != nullptr)
                    pDoor7->setState(pDoor7->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }
            break;
            case Deadmines::GO_GILNID_DOOR_LEVER:
            {
                GameObject* pDoor8 = getClosestGameObjectForPosition(Deadmines::GO_HEAVY_DOOR, Deadmines::Doors[0].x, Deadmines::Doors[0].y, Deadmines::Doors[0].z);
                if (pDoor8 != nullptr)
                    pDoor8->setState(pDoor8->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN  : GO_STATE_CLOSED);
            }
            break;
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

class MrSmiteAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MrSmiteAI(c); }
    explicit MrSmiteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto smiteSlam = addAISpell(Deadmines::SMITE_SLAM, 25.0f, TARGET_ATTACKING, 0, 15, false, true);
        smiteSlam->setMinMaxDistance(0.0f, 8.0f);

        mStomp = addAISpell(Deadmines::SMITE_STOMP, 0.0f, TARGET_SELF);
        mWaitAtChest = 0;
        _setWieldWeapon(true);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isScriptPhase(4))
            _removeAura(Deadmines::SMITES_HAMMER);

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
            else
                MoveToChest();
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
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);

        stopMovement();
        moveTo(1.100060f, -780.026367f, 9.811194f);
    }

    void MoveToPlayer()
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
    }

    bool NearChest()
    {
        if (getCreature()->GetPositionX() == 1.100060f && getCreature()->GetPositionY() == -780.026367f)
            return true;

        float XDiff = getCreature()->GetPositionX() - 1.100060f;
        float YDiff = getCreature()->GetPositionY() + 780.026367f;
        float Distance = std::sqrt(XDiff * XDiff + YDiff * YDiff);
        if (Distance <= 5.0f)
            return true;

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
                _applyAura(Deadmines::SMITES_HAMMER);
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
public:
    static CreatureAIScript* Create(Creature* c) { return new VanCleefAI(c); }
    explicit VanCleefAI(Creature* pCreature) : CreatureAIScript(pCreature){}

    void OnTargetDied(Unit* pTarget) override
    {
        char msg[200];
        if (pTarget->isPlayer())
            sprintf(msg, "And stay down, %s.", dynamic_cast<Player*>(pTarget)->getName().c_str());
        else if (pTarget->GetTypeFromGUID() == HIGHGUID_TYPE_PET)
            sprintf(msg, "And stay down, %s.", dynamic_cast<Pet*>(pTarget)->getName().c_str());

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
    mgr->register_instance_script(MAP_DEADMINES, &DeadminesInstanceScript::Create);
    mgr->register_creature_script(Deadmines::NPC_MR_SMITE, &MrSmiteAI::Create);
    mgr->register_creature_script(Deadmines::NPC_EDWIN_VANCLEEF, &VanCleefAI::Create);
}
