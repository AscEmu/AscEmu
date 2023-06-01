/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_GruulsLair.hpp"
#include "GruulTheDragonKiller.hpp"
#include "HighKingMaulgar.hpp"

GruulsLairInstanceScript::GruulsLairInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{
    Instance = (GruulsLairInstanceScript*)pMapMgr->getScript();
    mDoorMaulgar = nullptr;
    mDoorGruul = nullptr;
    mKrosh = nullptr;
    mOlm = nullptr;
    mKiggler = nullptr;
    mBlindeye = nullptr;
    mMaulgar = nullptr;
    mGruul = nullptr;
}

InstanceScript* GruulsLairInstanceScript::Create(WorldMap* pMapMgr) { return new GruulsLairInstanceScript(pMapMgr); }

void GruulsLairInstanceScript::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    switch (pGameObject->getEntry())
    {
        case GO_MAULGAR_DOOR:
            mDoorMaulgar = pGameObject;
            break;
        case GO_GRUUL_DOOR:
            mDoorGruul = pGameObject;
            break;
        default:
            break;
    }

    SetGameobjectStates();
}

void GruulsLairInstanceScript::OnCreaturePushToWorld(Creature* pCreature)
{
    switch (pCreature->getEntry())
    {
        case NPC_MAULGAR:
            mMaulgar = pCreature;
            break;
        case NPC_KROSH_FIREHAND:
            mKrosh = pCreature;
            break;
        case NPC_OLM_THE_SUMMONER:
            mOlm = pCreature;
            break;
        case NPC_KIGGLER_THE_CRAZED:
            mKiggler = pCreature;
            break;
        case NPC_BLINDEYE_THE_SEER:
            mBlindeye = pCreature;
            break;
        case NPC_GRUUL_THE_DRAGONKILLER:
            mGruul = pCreature;
            break;
        default:
            break;
    }

    if (mMaulgar && mMaulgar->GetScript())
    {
        if (mKrosh && mKrosh->GetScript() && !mKrosh->GetScript()->getLinkedCreatureAIScript())
            mKrosh->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

        if (mOlm && mOlm->GetScript() && !mOlm->GetScript()->getLinkedCreatureAIScript())
            mOlm->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

        if (mKiggler && mKiggler->GetScript() && !mKiggler->GetScript()->getLinkedCreatureAIScript())
            mKiggler->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

        if (mBlindeye && mBlindeye->GetScript() && !mBlindeye->GetScript()->getLinkedCreatureAIScript())
            mBlindeye->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());
    }
}

void GruulsLairInstanceScript::OnEncounterStateChange(uint32_t entry, uint32_t state)
{
    switch (entry)
    {
        case NPC_MAULGAR:
        {
            switch (state)
            {
            case NotStarted:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                break;
            case InProgress:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                break;
            case Performed:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_ENABLE);
                break;
            default:
                break;
            }
        }
            break;
        case NPC_GRUUL_THE_DRAGONKILLER:
        {
            switch (state)
            {
            case NotStarted:
                setLocalData(DATA_DOOR_GRUUL, ACTION_ENABLE);
                break;
            case InProgress:
                setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
                break;
            case Performed:
                setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
                break;
            default:
                break;
            }
        }
            break;
        default:
            break;
    }
}

void GruulsLairInstanceScript::SetGameobjectStates()
{
    switch (getBossState(DATA_MAULGAR))
    {
        case NotStarted:
            setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
            break;
        case InProgress:
            setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
            break;
        case Performed:
            setLocalData(DATA_DOOR_MAULGAR, ACTION_ENABLE);
            break;
        default:
            break;
    }

    switch (getBossState(DATA_GRUUL))
    {
        case NotStarted:
            setLocalData(DATA_DOOR_GRUUL, ACTION_ENABLE);
            break;
        case InProgress:
            setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
            break;
        case Performed:
            setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
            break;
        default:
            break;
    }
}

void GruulsLairInstanceScript::setLocalData(uint32_t type, uint32_t data)
{
    switch (type)
    {
        case DATA_DOOR_MAULGAR:
        {
            if (mDoorMaulgar)
            {
                if (data == ACTION_ENABLE)
                    mDoorMaulgar->setState(GO_STATE_OPEN);
                else
                    mDoorMaulgar->setState(GO_STATE_CLOSED);
            }
        }
            break;
        case DATA_DOOR_GRUUL:
        {
            if (mDoorGruul)
            {
                if (data == ACTION_ENABLE)
                    mDoorGruul->setState(GO_STATE_OPEN);
                else
                    mDoorGruul->setState(GO_STATE_CLOSED);
            }
        }
            break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Lair Brute
LairBruteAI::LairBruteAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_CLEAVE, 20.0f, TARGET_ATTACKING, 0, 15);
    addAISpell(SPELL_MORTALSTRIKE, 8.0f, TARGET_ATTACKING, 0, 20);
    addAISpell(SPELL_CHARGE, 7.0f, TARGET_ATTACKING, 0, 35);
}

CreatureAIScript* LairBruteAI::Create(Creature* pCreature) { return new LairBruteAI(pCreature); }

void LairBruteAI::OnCastSpell(uint32_t spellId)
{
    if (spellId == SPELL_CHARGE)
    {
        Unit* pCurrentTarget = getCreature()->getThreatManager().getCurrentVictim();
        if (pCurrentTarget != nullptr)
        {
            getCreature()->getAIInterface()->onHostileAction(pCurrentTarget);
            getCreature()->getThreatManager().addThreat(pCurrentTarget, 500.f);
            getCreature()->getAIInterface()->setCurrentTarget(pCurrentTarget);
            getCreature()->getThreatManager().clearThreat(pCurrentTarget);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Gronn Priest
GronnPriestAI::GronnPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_PSYCHICSCREAM, 8.0f, TARGET_SELF, 0, 20);

    auto renew = addAISpell(SPELL_RENEW, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
    renew->setMinMaxDistance(0.0f, 100.0f);

    auto heal = addAISpell(SPELL_HEAL_GP, 8.0f, TARGET_RANDOM_FRIEND, 2, 30);
    heal->setMinMaxDistance(0.0f, 100.0f);
}

CreatureAIScript* GronnPriestAI::Create(Creature* pCreature) { return new GronnPriestAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Wild Fell Stalker
WildFelStalkerAI::WildFelStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_WILD_BITE, 10.0f, TARGET_ATTACKING, 0, 10);
}

CreatureAIScript* WildFelStalkerAI::Create(Creature* pCreature) { return new WildFelStalkerAI(pCreature); }

void SetupGruulsLair(ScriptMgr* mgr)
{
    // Instance
    mgr->register_instance_script(MAP_GRUULS_LAIR, &GruulsLairInstanceScript::Create);

    // Spells
    mgr->register_script_effect(SPELL_GROUND_SLAM, &GroundSlamEffect);
    mgr->register_script_effect(SPELL_SHATTER, &ShatterEffect);
    mgr->register_spell_script(SPELL_SHATTER_EFFECT, new ShatterDamage);

    // Creatures
    mgr->register_creature_script(NPC_LAIR_BRUTE, &LairBruteAI::Create);
    mgr->register_creature_script(NPC_GRONN_PRIEST, &GronnPriestAI::Create);
    mgr->register_creature_script(NPC_WILD_FEL_STALKER, &WildFelStalkerAI::Create);

    // Boss
    mgr->register_creature_script(NPC_KIGGLER_THE_CRAZED, &KigglerTheCrazedAI::Create);
    mgr->register_creature_script(NPC_BLINDEYE_THE_SEER, &BlindeyeTheSeerAI::Create);
    mgr->register_creature_script(NPC_OLM_THE_SUMMONER, &OlmTheSummonerAI::Create);
    mgr->register_creature_script(NPC_KROSH_FIREHAND, &KroshFirehandAI::Create);
    mgr->register_creature_script(NPC_MAULGAR,    &HighKingMaulgarAI::Create);
    mgr->register_creature_script(NPC_GRUUL_THE_DRAGONKILLER, &GruulTheDragonkillerAI::Create);
}
