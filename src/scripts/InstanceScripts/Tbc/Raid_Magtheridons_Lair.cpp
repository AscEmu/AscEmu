/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Magtheridons_Lair.h"
#include "Objects/Faction.h"
#include "Server/Script/CreatureAIScript.h"

class MagtheridonsLairInstanceScript : public InstanceScript
{
public:
    explicit MagtheridonsLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        Instance = (MagtheridonsLairInstanceScript*)pMapMgr->GetScript();
        door = nullptr;
        hall = nullptr;
        cubes.clear();
        columns.clear();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagtheridonsLairInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance needed to Link all Generators and Bridge
        GetInstance()->updateAllCells(true);

        // activate boss ( testing )
        setLocalData(DATA_MANTICRON_CUBE, ACTION_ENABLE);
        magtheridon->GetAIInterface()->setAllowedToEnterCombat(true);
        magtheridon->GetAIInterface()->setImmuneToPC(false);
        magtheridon->GetAIInterface()->setImmuneToNPC(false);
        magtheridon->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }

    void OnSpawnGroupKilled(uint32_t groupId) override
    {
        if (groupId == 108)
        {
            // Start Battle
            setLocalData(DATA_MANTICRON_CUBE, ACTION_ENABLE);
            magtheridon->GetAIInterface()->setAllowedToEnterCombat(true);
            magtheridon->GetAIInterface()->setImmuneToPC(false);
            magtheridon->GetAIInterface()->setImmuneToNPC(false);
            magtheridon->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
        case GO_MANTICRON_CUBE:
            cubes.push_back(pGameObject);
            break;
        case GO_MAGTHERIDON_DOOR:
            door = pGameObject;
            break;
        case GO_MAGTHERIDON_HALL:
            hall = pGameObject;
            break;
        case GO_MAGTHERIDON_COLUMN_0:
        case GO_MAGTHERIDON_COLUMN_1:
        case GO_MAGTHERIDON_COLUMN_2:
        case GO_MAGTHERIDON_COLUMN_3:
        case GO_MAGTHERIDON_COLUMN_4:
        case GO_MAGTHERIDON_COLUMN_5:
            columns.push_back(pGameObject);
            break;
        default:
            break;
        }
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
        case NPC_MAGTHERIDON:
            magtheridon = pCreature;
            break;
        case NPC_WORLD_TRIGGER:
            worldTrigger = pCreature;
            break;
        default:
            break;
        }
    }

    void setLocalData(uint32_t type, uint32_t data) override
    {
        switch (type)
        {
        case DATA_MANTICRON_CUBE:
            for (auto cube : cubes)
            {
                if (cube)
                {
                    if (data == ACTION_ENABLE)
                        cube->removeFlags(GO_FLAG_NOT_SELECTABLE);
                    else
                        cube->setFlags(GO_FLAG_NOT_SELECTABLE);
                }
            }
            break;
        case DATA_COLLAPSE:
            if (hall)
            {
                if (data == ACTION_ENABLE)
                    hall->setState(GO_STATE_OPEN);
                else
                    hall->setState(GO_STATE_CLOSED);
            }
            break;
        case DATA_COLLAPSE_2:
            for (auto column : columns)
            {
                if (column)
                {
                    if (data == ACTION_ENABLE)
                        column->setState(GO_STATE_OPEN);
                    else
                        column->setState(GO_STATE_CLOSED);
                }
            }
            break;
        default:
            break;
        }
    }

public:
    MagtheridonsLairInstanceScript* Instance;
    GameObject* door;
    GameObject* hall;
    std::list<GameObject*> columns;
    std::list<GameObject*> cubes;
    Creature* magtheridon;
    Creature* worldTrigger;
};

class ManticronCubeGO : public GameObjectAIScript
{
public:
    static GameObjectAIScript* Create(GameObject* GO) { return new ManticronCubeGO(GO); }
    explicit ManticronCubeGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject) 
    {
        Instance = nullptr;
    }

    void OnActivate(Player* pPlayer) override
    {
        Instance = (MagtheridonsLairInstanceScript*)_gameobject->GetMapMgr()->GetScript();

        // We check if player has aura that prevents anyone from using this GO
        if (pPlayer->getAuraWithId(SPELL_MIND_EXHAUSTION) || pPlayer->getAuraWithId(SPELL_SHADOW_GRASP))
            return;

        if (!Instance && !Instance->magtheridon)
            return;

        if (Creature* trigger = _gameobject->GetMapMgr()->GetInterface()->findNearestCreature(pPlayer, NPC_HELFIRE_RAID_TRIGGER, 10.0f))
        {
            trigger->castSpell(Instance->magtheridon, SPELL_SHADOW_GRASP_VISUAL, true);
        }

        pPlayer->setTargetGuid(Instance->magtheridon->getGuid());
        pPlayer->castSpell(pPlayer, SPELL_SHADOW_GRASP, true);
    }

protected:
    MagtheridonsLairInstanceScript* Instance;
};

class CubeTriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CubeTriggerAI(c); }
    explicit CubeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->setAllowedToEnterCombat(false);
    }
};

class RoomTrigger : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RoomTrigger(c); }
    explicit RoomTrigger(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Scripted Spells not autocastet
        debrisVisual = addAISpell(SPELL_DEBRIS_VISUAL, 0.0f, TARGET_SELF);
        debrisDamage = addAISpell(SPELL_DEBRIS_DAMAGE, 0.0f, TARGET_SOURCE);
    }

    void OnLoad() override
    {
        _castAISpell(debrisVisual);
        scriptEvents.addEvent(SPELL_DEBRIS_DAMAGE, 5000);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if(scriptEvents.getFinishedEvent() == SPELL_DEBRIS_DAMAGE)
            _castAISpell(debrisDamage);

    }

protected:
    CreatureAISpells* debrisVisual;
    CreatureAISpells* debrisDamage;
};

class HellfireWarderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireWarderAI(c); }
    explicit HellfireWarderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }
};

class HellfireChannelerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireChannelerAI(c); }
    explicit HellfireChannelerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->setAllowedToEnterCombat(false);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        if (!getCreature()->GetAIInterface()->getAllowedToEnterCombat())
            getCreature()->GetAIInterface()->setAllowedToEnterCombat(true);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
    }
};

class BurningAbyssalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BurningAbyssalAI(c); }
    explicit BurningAbyssalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->m_noRespawn = true;
    }
};

class MagtheridonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagtheridonAI(c); }
    explicit MagtheridonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
    }

    void AIUpdate() override
    {
    }
};

void SetupMagtheridonsLair(ScriptMgr* mgr)
{
    // Instance
    mgr->register_instance_script(MAP_MAGTHERIDONS_LAIR, &MagtheridonsLairInstanceScript::Create);

    // GameObjects
    mgr->register_gameobject_script(GO_MANTICRON_CUBE, &ManticronCubeGO::Create);

    // Creatures
    mgr->register_creature_script(NPC_HELFIRE_RAID_TRIGGER, &CubeTriggerAI::Create);
    mgr->register_creature_script(NPC_MAGTHERIDON_ROOM, &RoomTrigger::Create);
    mgr->register_creature_script(NPC_HELLFIRE_WARDER, &HellfireWarderAI::Create);
    mgr->register_creature_script(NPC_HELLFIRE_CHANNELLER, &HellfireChannelerAI::Create);
    mgr->register_creature_script(NPC_ABYSSAL, &BurningAbyssalAI::Create);

    // Boss
    mgr->register_creature_script(NPC_MAGTHERIDON, &MagtheridonAI::Create);
}
