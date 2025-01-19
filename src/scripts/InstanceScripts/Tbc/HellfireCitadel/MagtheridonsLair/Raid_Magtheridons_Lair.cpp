/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Magtheridons_Lair.hpp"
#include "Magtheridon.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//Magtheridons Lair Instance
MagtheridonsLairInstanceScript::MagtheridonsLairInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{
    Instance = (MagtheridonsLairInstanceScript*)pMapMgr->getScript();
    door = nullptr;
    hall = nullptr;
    cubes.clear();
    columns.clear();

    magtheridon = nullptr;
    worldTrigger = nullptr;
}

InstanceScript* MagtheridonsLairInstanceScript::Create(WorldMap* pMapMgr) { return new MagtheridonsLairInstanceScript(pMapMgr); }

void MagtheridonsLairInstanceScript::OnGameObjectPushToWorld(GameObject* pGameObject)
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

void MagtheridonsLairInstanceScript::OnCreaturePushToWorld(Creature* pCreature)
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

void MagtheridonsLairInstanceScript::setLocalData(uint32_t type, uint32_t data)
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
    case DATA_DOOR:
        if (door)
        {
            if (data == ACTION_ENABLE)
                door->setState(GO_STATE_OPEN);
            else
                door->setState(GO_STATE_CLOSED);
        }
        break;
    default:
        break;
    }
}

void MagtheridonsLairInstanceScript::DoAction(int32_t const action)
{
    switch (action)
    {
    case ACTION_START_CHANNELERS_EVENT:
        setLocalData(DATA_DOOR, ACTION_DISABLE);
        magtheridon->GetScript()->DoAction(ACTION_START_CHANNELERS_EVENT);
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Gameobject: manticron Cubes
ManticronCubeGO::ManticronCubeGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
{
    Instance = nullptr;
}

GameObjectAIScript* ManticronCubeGO::Create(GameObject* pGameObject) { return new ManticronCubeGO(pGameObject); }

void ManticronCubeGO::OnActivate(Player* pPlayer)
{
    Instance = (MagtheridonsLairInstanceScript*)_gameobject->getWorldMap()->getScript();

    // We check if player has aura that prevents anyone from using this GO
    if (pPlayer->getAuraWithId(SPELL_MIND_EXHAUSTION) || pPlayer->getAuraWithId(SPELL_SHADOW_GRASP))
        return;

    if (!Instance && !Instance->magtheridon)
        return;

    if (Creature* trigger = _gameobject->getWorldMap()->getInterface()->findNearestCreature(pPlayer, NPC_HELFIRE_RAID_TRIGGER, 10.0f))
        trigger->castSpell(Instance->magtheridon, SPELL_SHADOW_GRASP_VISUAL, true);

    pPlayer->setTargetGuid(Instance->magtheridon->getGuid());
    pPlayer->castSpell(pPlayer, SPELL_SHADOW_GRASP, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Cube Trigger Npc
CubeTriggerAI::CubeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
}

CreatureAIScript* CubeTriggerAI::Create(Creature* pCreature) { return new CubeTriggerAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Room Triger ( Roof falling down )
RoomTrigger::RoomTrigger(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Scripted Spells not autocastet
    debrisVisual = addAISpell(SPELL_DEBRIS_VISUAL, 0.0f, TARGET_SELF);
    debrisDamage = addAISpell(SPELL_DEBRIS_DAMAGE, 0.0f, TARGET_SOURCE);
}

CreatureAIScript* RoomTrigger::Create(Creature* pCreature) { return new RoomTrigger(pCreature); }

void RoomTrigger::OnLoad()
{
    _castAISpell(debrisVisual);
    scriptEvents.addEvent(SPELL_DEBRIS_DAMAGE, 5000);
    despawn(20000, 0);
}

void RoomTrigger::AIUpdate()
{
    scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

    if(scriptEvents.getFinishedEvent() == SPELL_DEBRIS_DAMAGE)
        _castAISpell(debrisDamage);

}

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Hellfire Warder
HellfireWarderAI::HellfireWarderAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    auto shadowBoltVolley = addAISpell(HW_SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 5, false, true);
    shadowBoltVolley->setAttackStopTimer(1000);

    auto wordPain = addAISpell(SHADOW_WORD_PAIN, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
    wordPain->setAttackStopTimer(1000);
    wordPain->setMinMaxDistance(0.0f, 30.0f);

    auto unstableAfflection = addAISpell(UNSTABLE_AFFLICTION, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
    unstableAfflection->setAttackStopTimer(1000);
    unstableAfflection->setMinMaxDistance(0.0f, 30.0f);

    auto deathCoil = addAISpell(DEATH_COIL, 5.0f, TARGET_RANDOM_SINGLE, 0, 8, false, true);
    deathCoil->setAttackStopTimer(1000);
    deathCoil->setMinMaxDistance(0.0f, 30.0f);

    auto rainOfFire = addAISpell(RAIN_OF_FIRE, 5.0f, TARGET_RANDOM_DESTINATION, 0, 6, false, true);
    rainOfFire->setAttackStopTimer(1000);
    rainOfFire->setMinMaxDistance(0.0f, 30.0f);

    auto fear = addAISpell(HW_FEAR, 4.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
    fear->setAttackStopTimer(1000);
    fear->setMinMaxDistance(0.0f, 30.0f);

    auto shadowBurst = addAISpell(SHADOW_BURST, 4.0f, TARGET_VARIOUS, 0, 8);
    shadowBurst->setAttackStopTimer(1000);
    shadowBurst->setMinMaxDistance(0.0f, 30.0f);
}

CreatureAIScript* HellfireWarderAI::Create(Creature* pCreature) { return new HellfireWarderAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Hellfire Channeler
CreatureAIScript* HellfireChannelerAI::Create(Creature* pCreature) { return new HellfireChannelerAI(pCreature); }
HellfireChannelerAI::HellfireChannelerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    shadowGasp = addAISpell(SPELL_SHADOW_GRASP_C, 0.0f, TARGET_SELF);
    shadowBolt = addAISpell(SPELL_SHADOW_BOLT_VOLLEY, 0.0f, TARGET_DESTINATION);
    fear = addAISpell(SPELL_FEAR, 0.0f, TARGET_RANDOM_SINGLE);
    darkMending = addAISpell(SPELL_DARK_MENDING, 8.0f, TARGET_RANDOM_FRIEND, 0, Util::getRandomUInt(10, 20));
    darkMending->setAttackStopTimer(1000);
    darkMending->setMinMaxDistance(0.0f, 30.0f);
    darkMending->setMinMaxPercentHp(0, 70);
    summonAbyssal = addAISpell(SPELL_BURNING_ABYSSAL, 0.0f, TARGET_ATTACKING);
    soulTransfer = addAISpell(SPELL_SOUL_TRANSFER, 0.0f, TARGET_SOURCE);
}

void HellfireChannelerAI::OnLoad()
{
    _castAISpell(shadowGasp);
    getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
    getCreature()->getAIInterface()->setReactState(REACT_DEFENSIVE);
}

void HellfireChannelerAI::OnCombatStart(Unit* /*mTarget*/)
{
    getInstanceScript()->DoAction(ACTION_START_CHANNELERS_EVENT);

    scriptEvents.addEvent(EVENT_SHADOWBOLT, 20000);
    scriptEvents.addEvent(EVENT_ABYSSAL, 30000);
    scriptEvents.addEvent(EVENT_FEAR1, Util::getRandomUInt(15000, 20000));
}

void HellfireChannelerAI::OnCombatStop(Unit* /*mTarget*/)
{
    getInstanceScript()->setLocalData(DATA_DOOR, ACTION_ENABLE);
}

void HellfireChannelerAI::AIUpdate(unsigned long time_passed)
{
    if(!_isInCombat() && !getCreature()->hasAurasWithId(SPELL_SHADOW_GRASP_C))
        _castAISpell(shadowGasp);

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (_isCasting())
        return;

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
        case EVENT_SHADOWBOLT:
            _castAISpell(shadowBolt);
            scriptEvents.addEvent(EVENT_SHADOWBOLT, Util::getRandomUInt(15000, 20000));
            break;
        case EVENT_FEAR1:
            _castAISpell(fear);
            scriptEvents.addEvent(EVENT_FEAR1, Util::getRandomUInt(25000, 40000));
            break;
        case EVENT_ABYSSAL:
            _castAISpell(summonAbyssal);
            scriptEvents.addEvent(EVENT_ABYSSAL, 60000);
            break;
        default:
            break;
        }
    }
}

void HellfireChannelerAI::OnDied(Unit* /*killer*/)
{
     _castAISpell(soulTransfer);
}

void HellfireChannelerAI::OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/)
{
    if (!getCreature()->getAIInterface()->getAllowedToEnterCombat())
        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
}

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

    // Spells
    mgr->register_spell_script(SPELL_SOUL_TRANSFER, new SoulTransfer);

    // Boss
    mgr->register_creature_script(NPC_MAGTHERIDON, &MagtheridonAI::Create);
}
