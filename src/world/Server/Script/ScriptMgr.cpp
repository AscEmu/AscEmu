/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

#include "WorldConf.h"
#include "Management/GameEvent.h"
#include "Management/Item.h"
#include "Storage/MySQLDataStore.hpp"
#include <git_version.h>

#include <fstream>
#include <mutex>
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Objects/ObjectMgr.h"
#include "ScriptMgr.h"
#include "Map/MapScriptInterface.h"
#include "Objects/Faction.h"

initialiseSingleton(ScriptMgr);
initialiseSingleton(HookInterface);

ScriptMgr::ScriptMgr()
{}

ScriptMgr::~ScriptMgr()
{}

struct ScriptingEngine_dl
{
    Arcemu::DynLib* dl;
    exp_script_register InitializeCall;
    uint32 Type;

    ScriptingEngine_dl()
    {
        dl = NULL;
        InitializeCall = NULL;
        Type = 0;
    }
};

void ScriptMgr::LoadScripts()
{
    if (HookInterface::getSingletonPtr() == NULL)
        new HookInterface;

    LogNotice("ScriptMgr : Loading External Script Libraries...");

    std::string Path;
    std::string FileMask;
    Path = PREFIX;
    Path += '/';
#ifdef WIN32
    FileMask = ".dll";
#else
#ifndef __APPLE__
    FileMask = ".so";
#else
    FileMask = ".dylib";
#endif
#endif

    Arcemu::FindFilesResult findres;
    std::vector< ScriptingEngine_dl > Engines;

    Arcemu::FindFiles(Path.c_str(), FileMask.c_str(), findres);
    uint32 count = 0;

    while (findres.HasNext())
    {
        std::stringstream loadmessage;
        std::string fname = Path + findres.GetNext();
        Arcemu::DynLib* dl = new Arcemu::DynLib(fname.c_str());

        loadmessage << dl->GetName() << " : ";

        if (!dl->Load())
        {
            loadmessage << "ERROR: Cannot open library.";
            LOG_ERROR(loadmessage.str().c_str());
            delete dl;
            continue;

        }
        else
        {
            exp_get_version vcall = reinterpret_cast<exp_get_version>(dl->GetAddressForSymbol("_exp_get_version"));
            exp_script_register rcall = reinterpret_cast<exp_script_register>(dl->GetAddressForSymbol("_exp_script_register"));
            exp_get_script_type scall = reinterpret_cast<exp_get_script_type>(dl->GetAddressForSymbol("_exp_get_script_type"));
            exp_set_serverstate_singleton set_serverstate_call = reinterpret_cast<exp_set_serverstate_singleton>(dl->GetAddressForSymbol("_exp_set_serverstate_singleton"));

            if (!set_serverstate_call)
            {
                loadmessage << "ERROR: Cannot find set_serverstate_call function.";
                LOG_ERROR(loadmessage.str().c_str());
                delete dl;
                continue;
            }

            // Make sure we use the same ServerState singleton
            set_serverstate_call(ServerState::instance());

            if (!vcall || !rcall || !scall)
            {
                loadmessage << "ERROR: Cannot find version functions.";
                LOG_ERROR(loadmessage.str().c_str());
                delete dl;
                continue;
            }
            else
            {
                const char *version = vcall();
                uint32 stype = scall();

                if (strcmp(version, BUILD_HASH_STR) != 0)
                {
                    loadmessage << "ERROR: Version mismatch.";
                    LOG_ERROR(loadmessage.str().c_str());
                    delete dl;
                    continue;

                }
                else
                {
                    loadmessage << std::string(BUILD_HASH_STR) << " : ";

                    if ((stype & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
                    {
                        ScriptingEngine_dl se;

                        se.dl = dl;
                        se.InitializeCall = rcall;
                        se.Type = stype;

                        Engines.push_back(se);

                        loadmessage << "delayed load";

                    }
                    else
                    {
                        rcall(this);
                        dynamiclibs.push_back(dl);

                        loadmessage << "loaded";
                    }
                    LogDetail(loadmessage.str().c_str());
                    count++;
                }
            }
        }
    }

    if (count == 0)
    {
        LOG_ERROR("No external scripts found! Server will continue to function with limited functionality.");
    }
    else
    {
        LogDetail("ScriptMgr : Loaded %u external libraries.", count);
        LogNotice("ScriptMgr : Loading optional scripting engine(s)...");

        for (std::vector< ScriptingEngine_dl >::iterator itr = Engines.begin(); itr != Engines.end(); ++itr)
        {
            itr->InitializeCall(this);
            dynamiclibs.push_back(itr->dl);
        }

        LogDetail("ScriptMgr : Done loading scripting engine(s)...");
    }
}

void ScriptMgr::UnloadScripts()
{
    if (HookInterface::getSingletonPtr())
        delete HookInterface::getSingletonPtr();

    for (CustomGossipScripts::iterator itr = _customgossipscripts.begin(); itr != _customgossipscripts.end(); ++itr)
        (*itr)->Destroy();
    _customgossipscripts.clear();

    for (QuestScripts::iterator itr = _questscripts.begin(); itr != _questscripts.end(); ++itr)
        delete *itr;
    _questscripts.clear();

    UnloadScriptEngines();

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
        delete *itr;

    dynamiclibs.clear();
}

void ScriptMgr::DumpUnimplementedSpells()
{
    std::ofstream of;

    LOG_BASIC("Dumping IDs for spells with unimplemented dummy/script effect(s)");
    uint32 count = 0;

    of.open("unimplemented1.txt");

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        SpellInfo* sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (!sp)
            continue;

        if (!sp->HasEffect(SPELL_EFFECT_DUMMY) && !sp->HasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->HasEffect(SPELL_EFFECT_SEND_EVENT))
            continue;

        HandleDummySpellMap::iterator sitr = _spells.find(sp->getId());
        if (sitr != _spells.end())
            continue;

        HandleScriptEffectMap::iterator seitr = SpellScriptEffects.find(sp->getId());
        if (seitr != SpellScriptEffects.end())
            continue;

        std::stringstream ss;
        ss << sp->getId();
        ss << std::endl;

        of.write(ss.str().c_str(), ss.str().length());

        count++;
    }

    of.close();

    LOG_BASIC("Dumped %u IDs.", count);

    LOG_BASIC("Dumping IDs for spells with unimplemented dummy aura effect.");

    std::ofstream of2;
    of2.open("unimplemented2.txt");

    count = 0;

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        SpellInfo* sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (!sp)
            continue;

        if (!sp->appliesAreaAura(SPELL_AURA_DUMMY))
            continue;

        HandleDummyAuraMap::iterator ditr = _auras.find(sp->getId());
        if (ditr != _auras.end())
            continue;

        std::stringstream ss;
        ss << sp->getId();
        ss << std::endl;

        of2.write(ss.str().c_str(), ss.str().length());

        count++;
    }

    of2.close();

    LOG_BASIC("Dumped %u IDs.", count);
}

void ScriptMgr::register_creature_script(uint32 entry, exp_create_creature_ai callback)
{
	m_creaturesMutex.Acquire();

    if (_creatures.find(entry) != _creatures.end())
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Creature ID: %u but this creature has already one!", entry);

    _creatures.insert(CreatureCreateMap::value_type(entry, callback));

	m_creaturesMutex.Release();
}

void ScriptMgr::register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback)
{
    if (_gameobjects.find(entry) != _gameobjects.end())
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for GameObject ID: %u but this go has already one.", entry);

    _gameobjects.insert(GameObjectCreateMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback)
{
    if (_auras.find(entry) != _auras.end())
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Aura ID: %u but this aura has already one.", entry);
    }

    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(entry);
    if (sp == NULL)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a dummy aura handler for invalid Spell ID: %u.", entry);
        return;
    }

    if (!sp->appliesAreaAura(SPELL_AURA_DUMMY) && !sp->appliesAreaAura(SPELL_AURA_PERIODIC_TRIGGER_DUMMY))
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr registered a dummy aura handler for Spell ID: %u (%s), but spell has no dummy aura!", entry, sp->getName().c_str());

    _auras.insert(HandleDummyAuraMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback)
{
    if (_spells.find(entry) != _spells.end())
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Spell ID: %u but this spell has already one", entry);
        return;
    }

    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(entry);
    if (sp == NULL)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a dummy handler for invalid Spell ID: %u.", entry);
        return;
    }

    if (!sp->HasEffect(SPELL_EFFECT_DUMMY) && !sp->HasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->HasEffect(SPELL_EFFECT_SEND_EVENT))
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr registered a dummy handler for Spell ID: %u (%s), but spell has no dummy/script/send event effect!", entry, sp->getName().c_str());

    _spells.insert(HandleDummySpellMap::value_type(entry, callback));
}

void ScriptMgr::register_quest_script(uint32 entry, QuestScript* qs)
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(entry);
    if (q != nullptr)
    {
        if (q->pQuestScript != NULL)
            LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Quest ID: %u but this quest has already one.", entry);

        const_cast<QuestProperties*>(q)->pQuestScript = qs;
    }

    _questscripts.insert(qs);
}

void ScriptMgr::register_event_script(uint32 entry, EventScript* es)
{
    auto gameEvent = sGameEventMgr.GetEventById(entry);
    if (gameEvent != nullptr)
    {
        if (gameEvent->mEventScript != nullptr)
        {
            LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Event ID: %u but this event has already one.", entry);
            return;
        }

        gameEvent->mEventScript = es;
        _eventscripts.insert(es);
    }
}

void ScriptMgr::register_instance_script(uint32 pMapId, exp_create_instance_ai pCallback)
{
    if (mInstances.find(pMapId) != mInstances.end())
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script for Instance ID: %u but this instance already has one.", pMapId);

    mInstances.insert(InstanceCreateMap::value_type(pMapId, pCallback));
};

void ScriptMgr::register_creature_script(uint32* entries, exp_create_creature_ai callback)
{
    for (uint32 y = 0; entries[y] != 0; y++)
    {
        register_creature_script(entries[y], callback);
    }
};

void ScriptMgr::register_gameobject_script(uint32* entries, exp_create_gameobject_ai callback)
{
    for (uint32 y = 0; entries[y] != 0; y++)
    {
        register_gameobject_script(entries[y], callback);
    }
};

void ScriptMgr::register_dummy_aura(uint32* entries, exp_handle_dummy_aura callback)
{
    for (uint32 y = 0; entries[y] != 0; y++)
    {
        register_dummy_aura(entries[y], callback);
    }
};

void ScriptMgr::register_dummy_spell(uint32* entries, exp_handle_dummy_spell callback)
{
    for (uint32 y = 0; entries[y] != 0; y++)
    {
        register_dummy_spell(entries[y], callback);
    }
};

void ScriptMgr::register_script_effect(uint32* entries, exp_handle_script_effect callback)
{
    for (uint32 y = 0; entries[y] != 0; y++)
    {
        register_script_effect(entries[y], callback);
    }
};

void ScriptMgr::register_script_effect(uint32 entry, exp_handle_script_effect callback)
{

    HandleScriptEffectMap::iterator itr = SpellScriptEffects.find(entry);

    if (itr != SpellScriptEffects.end())
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register more than 1 script effect handlers for Spell %u", entry);
        return;
    }

    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(entry);
    if (sp == NULL)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr tried to register a script effect handler for invalid Spell %u.", entry);
        return;
    }

    if (!sp->HasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->HasEffect(SPELL_EFFECT_SEND_EVENT))
        LogDebugFlag(LF_SCRIPT_MGR, "ScriptMgr registered a script effect handler for Spell ID: %u (%s), but spell has no scripted effect!", entry, sp->getName().c_str());

    SpellScriptEffects.insert(std::pair< uint32, exp_handle_script_effect >(entry, callback));
}

CreatureAIScript* ScriptMgr::CreateAIScriptClassForEntry(Creature* pCreature)
{
	uint32 entry = pCreature->GetEntry();

	m_creaturesMutex.Acquire();
    CreatureCreateMap::iterator itr = _creatures.find(entry);
	m_creaturesMutex.Release();

    if (itr == _creatures.end())
        return NULL;

    exp_create_creature_ai function_ptr = itr->second;
    return (function_ptr)(pCreature);
}

GameObjectAIScript* ScriptMgr::CreateAIScriptClassForGameObject(uint32 /*uEntryId*/, GameObject* pGameObject)
{
    GameObjectCreateMap::iterator itr = _gameobjects.find(pGameObject->GetEntry());
    if (itr == _gameobjects.end())
        return NULL;

    exp_create_gameobject_ai function_ptr = itr->second;
    return (function_ptr)(pGameObject);
}

InstanceScript* ScriptMgr::CreateScriptClassForInstance(uint32 /*pMapId*/, MapMgr* pMapMgr)
{
    InstanceCreateMap::iterator Iter = mInstances.find(pMapMgr->GetMapId());
    if (Iter == mInstances.end())
        return NULL;
    exp_create_instance_ai function_ptr = Iter->second;
    return (function_ptr)(pMapMgr);
};

bool ScriptMgr::CallScriptedDummySpell(uint32 uSpellId, uint32 i, Spell* pSpell)
{
    HandleDummySpellMap::iterator itr = _spells.find(uSpellId);
    if (itr == _spells.end())
        return false;

    exp_handle_dummy_spell function_ptr = itr->second;
    return (function_ptr)(i, pSpell);
}

bool ScriptMgr::HandleScriptedSpellEffect(uint32 SpellId, uint32 i, Spell* s)
{
    HandleScriptEffectMap::iterator itr = SpellScriptEffects.find(SpellId);
    if (itr == SpellScriptEffects.end())
        return false;

    exp_handle_script_effect ptr = itr->second;
    return (ptr)(i, s);
}

bool ScriptMgr::CallScriptedDummyAura(uint32 uSpellId, uint32 i, Aura* pAura, bool apply)
{
    HandleDummyAuraMap::iterator itr = _auras.find(uSpellId);
    if (itr == _auras.end())
        return false;

    exp_handle_dummy_aura function_ptr = itr->second;
    return (function_ptr)(i, pAura, apply);
}

bool ScriptMgr::CallScriptedItem(Item* pItem, Player* pPlayer)
{
    Arcemu::Gossip::Script* script = this->get_item_gossip(pItem->GetEntry());
    if (script != NULL)
    {
        script->OnHello(pItem, pPlayer);
        return true;
    }
    return false;
}

/* CreatureAI Stuff */
CreatureAIScript::CreatureAIScript(Creature* creature) : _creature(creature), linkedCreatureAI(nullptr), mDespawnWhenInactive(false), mScriptPhase(0), mAIUpdateFrequency(defaultUpdateFrequency),
    mCreatureTimerCount(0), isIdleEmoteEnabled(false), idleEmoteTimerId(0), idleEmoteTimeMin(0), idleEmoteTimeMax(0)
{
    mCreatureTimerIds.clear();
    mCreatureTimer.clear();

    mEnrageSpell = nullptr;
    mEnrageTimerDuration = -1;
    mEnrageTimer = 0;

    mRunToTargetCache = nullptr;
    mRunToTargetSpellCache = nullptr;

    mBaseAttackTime = getCreature()->GetBaseAttackTime(MELEE);

    mCustomAIUpdateDelayTimerId = 0;
    mCustomAIUpdateDelay = 0;
    registerAiUpdateFrequency();

    //new CreatureAISpell handling
    enableCreatureAISpellSystem = false;
    mSpellWaitTimerId = _addTimer(defaultUpdateFrequency);
    mCurrentSpellTarget = nullptr;
    mLastCastedSpell = nullptr;
}

CreatureAIScript::~CreatureAIScript()
{
    //notify our linked creature that we are being deleted.
    if (linkedCreatureAI != nullptr)
        linkedCreatureAI->LinkedCreatureDeleted();

    mPhaseSpells.clear();

    DeleteArray(mSpells);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Event default management
void CreatureAIScript::_internalOnDied()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnDied() called");

    enableOnIdleEmote(false);

    _cancelAllTimers();
    _removeAllAuras();

    removeAiUpdateFrequency();

    CancelAllSpells();
    RemoveAIUpdateEvent();
    sendRandomDBChatMessage(mEmotesOnDied);

    if (_isDespawnWhenInactiveSet())
        despawn(DEFAULT_DESPAWN_TIMER);

    resetScriptPhase();
}

void CreatureAIScript::_internalOnTargetDied()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnTargetDied() called");

    sendRandomDBChatMessage(mEmotesOnTargetDied);
}

void CreatureAIScript::_internalOnCombatStart()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnEnterCombat() called");
    
    enableOnIdleEmote(false);

    setAIAgent(AGENT_MELEE);

    sendRandomDBChatMessage(mEmotesOnCombatStart);

    setScriptPhase(1);
    if (mEnrageSpell && mEnrageTimerDuration > 0)
    {
        mEnrageTimer = _addTimer(mEnrageTimerDuration);
    }

    TriggerCooldownOnAllSpells();

    RegisterAIUpdateEvent(mAIUpdateFrequency);
}

void CreatureAIScript::_internalOnCombatStop()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnCombatStop() called");

    _cancelAllTimers();
    _removeAllAuras();
    setAIAgent(AGENT_NULL);
    RemoveAIUpdateEvent();

    if (_isDespawnWhenInactiveSet())
        despawn(DEFAULT_DESPAWN_TIMER);

    resetScriptPhase();
    enableOnIdleEmote(true);

    _removeTimer(mEnrageTimer);

    CancelAllSpells();
}

void CreatureAIScript::_internalAIUpdate()
{
    //LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalAIUpdate() called");

    updateAITimers();

    // old AIUpdate stuff is now handled by customAIUpdateTimer. Keep this until all scripts are updated to new logic.
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        if (!_isTimerFinished(mCustomAIUpdateDelayTimerId))
            return;
        else
            _resetTimer(mCustomAIUpdateDelayTimerId, mCustomAIUpdateDelay);
    }

    // idleemotes
    if (!_isInCombat() && isIdleEmoteEnabled)
    {
        if (_isTimerFinished(getIdleEmoteTimerId()))
        {
            sendRandomDBChatMessage(mEmotesOnIdle);
            generateNextRandomIdleEmoteTime();
        }
    }

    AIUpdate();

    if (!_isInCombat())
        return;

    if (enableCreatureAISpellSystem)
    {
        newAIUpdateSpellSystem();
    }
    else
    {
        // SP_AI_Spell here
        oldAIUpdateSpellSystem();
    }
}

void CreatureAIScript::_internalOnScriptPhaseChange()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnScriptPhaseChange() called");

    getCreature()->GetScript()->OnScriptPhaseChange(getScriptPhase());
}

//////////////////////////////////////////////////////////////////////////////////////////
// player
Player* CreatureAIScript::getNearestPlayer()
{
    return _creature->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ());
}

//////////////////////////////////////////////////////////////////////////////////////////
// creature
CreatureAIScript* CreatureAIScript::getNearestCreatureAI(uint32_t entry)
{
    Creature* creature = getNearestCreature(entry);
    return (creature ? creature->GetScript() : nullptr);
}

Creature* CreatureAIScript::getNearestCreature(uint32_t entry)
{
    return getNearestCreature(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ(), entry);
}

Creature* CreatureAIScript::getNearestCreature(float posX, float posY, float posZ, uint32_t entry)
{
    return _creature->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(posX, posY, posZ, entry);
}

float CreatureAIScript::getRangeToObject(Object* object)
{
    return _creature->CalcDistance(object);
}

CreatureAIScript* CreatureAIScript::spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    Creature* creature = spawnCreature(entry, posX, posY, posZ, posO, factionId);
    return (creature ? creature->GetScript() : nullptr);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId /*= 0*/)
{
    return spawnCreature(entry, pos.x, pos.y, pos.z, pos.o, factionId);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry);
    if (creatureProperties == nullptr)
    {
        LOG_ERROR("tried to create an invalid creature with entry %u!", entry);
        return nullptr;
    }

    Creature* creature = _creature->GetMapMgr()->GetInterface()->SpawnCreature(entry, posX, posY, posZ, posO, true, true, 0, 0);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->SetFaction(factionId);
    else
        creature->SetFaction(creatureProperties->Faction);

    return creature;
}

void CreatureAIScript::despawn(uint32_t delay /*= 2000*/, uint32_t respawnTime /*= 0*/)
{
    _creature->Despawn(delay, respawnTime);
}

bool CreatureAIScript::isAlive()
{
    return _creature->isAlive();
}

void CreatureAIScript::setAIAgent(AI_Agent agent)
{
    if (agent <= AGENT_CALLFORHELP)
        _creature->GetAIInterface()->setCurrentAgent(agent);
}

uint8_t CreatureAIScript::getAIAgent()
{
    return _creature->GetAIInterface()->getCurrentAgent();
}

void CreatureAIScript::setRooted(bool set)
{
    _creature->setMoveRoot(set);
}

void CreatureAIScript::setFlyMode(bool fly)
{
    if (fly && !_creature->GetAIInterface()->isFlying())
    {
        _creature->setMoveCanFly(true);
        _creature->GetAIInterface()->setSplineFlying();
    }
    else if (!fly && _creature->GetAIInterface()->isFlying())
    {
        _creature->setMoveCanFly(false);
        _creature->GetAIInterface()->unsetSplineFlying();
    }
}

bool CreatureAIScript::isRooted()
{
    return _creature->GetAIInterface()->m_canMove;
}

void CreatureAIScript::moveTo(float posX, float posY, float posZ, bool setRun /*= true*/)
{
    if (setRun)
        _creature->GetAIInterface()->setWalkMode(WALKMODE_RUN);

    _creature->GetAIInterface()->MoveTo(posX, posY, posZ);
}

void CreatureAIScript::moveToUnit(Unit* unit)
{
    if (unit != nullptr)
        moveTo(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
}

void CreatureAIScript::moveToSpawn()
{
    LocationVector spawnPos = _creature->GetSpawnPosition();
    _creature->GetAIInterface()->sendSplineMoveToPoint(spawnPos);
}

void CreatureAIScript::stopMovement()
{
    _creature->GetAIInterface()->StopMovement(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// wp movement

Movement::WayPoint* CreatureAIScript::CreateWaypoint(int pId, uint32 pWaittime, uint32 pMoveFlag, Movement::Location pCoords)
{
    Movement::WayPoint* wp = _creature->CreateWaypointStruct();
    wp->id = pId;
    wp->x = pCoords.x;
    wp->y = pCoords.y;
    wp->z = pCoords.z;
    wp->o = pCoords.o;
    wp->waittime = pWaittime;
    wp->flags = pMoveFlag;
    wp->forwardemoteoneshot = false;
    wp->forwardemoteid = 0;
    wp->backwardemoteoneshot = false;
    wp->backwardemoteid = 0;
    wp->forwardskinid = 0;
    wp->backwardskinid = 0;
    return wp;
}

void CreatureAIScript::AddWaypoint(Movement::WayPoint* pWayPoint)
{
    _creature->GetAIInterface()->addWayPoint(pWayPoint);
}

void CreatureAIScript::ForceWaypointMove(uint32 pWaypointId)
{
    if (canEnterCombat())
        _creature->GetAIInterface()->SetAllowedToEnterCombat(false);

    if (isRooted())
        setRooted(false);

    stopMovement();
    _creature->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
    SetWaypointToMove(pWaypointId);
}

void CreatureAIScript::SetWaypointToMove(uint32 pWaypointId)
{
    _creature->GetAIInterface()->setWayPointToMove(pWaypointId);
}

void CreatureAIScript::StopWaypointMovement()
{
    setAIAgent(AGENT_NULL);
    _creature->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
    SetWaypointToMove(0);
}

void CreatureAIScript::SetWaypointMoveType(Movement::WaypointMovementScript wp_move_script_type)
{
    _creature->GetAIInterface()->setWaypointScriptType(wp_move_script_type);

}

uint32 CreatureAIScript::GetCurrentWaypoint()
{
    return _creature->GetAIInterface()->getCurrentWayPointId();
}

size_t CreatureAIScript::GetWaypointCount()
{
    return _creature->GetAIInterface()->getWayPointsCount();
}

bool CreatureAIScript::HasWaypoints()
{
    return _creature->GetAIInterface()->hasWayPoints();
}

//////////////////////////////////////////////////////////////////////////////////////////
// combat setup

bool CreatureAIScript::canEnterCombat()
{
    return _creature->GetAIInterface()->GetAllowedToEnterCombat();
}

void CreatureAIScript::setCanEnterCombat(bool enterCombat)
{
    //Zyres 10/21/2017 creatures can be attackable even if they can not enter combat... the following line is not correct.
    _creature->setUInt64Value(UNIT_FIELD_FLAGS, (enterCombat) ? 0 : UNIT_FLAG_NOT_ATTACKABLE_9);
    _creature->GetAIInterface()->SetAllowedToEnterCombat(enterCombat);
}

bool CreatureAIScript::_isInCombat()
{
    return _creature->CombatStatus.IsInCombat();
}

void CreatureAIScript::_delayNextAttack(int32_t milliseconds)
{
    _creature->setAttackTimer(milliseconds, false);
}

void CreatureAIScript::_setDespawnWhenInactive(bool setDespawnWhenInactive)
{
    mDespawnWhenInactive = setDespawnWhenInactive;
}

bool CreatureAIScript::_isDespawnWhenInactiveSet()
{
    return mDespawnWhenInactive;
}

void CreatureAIScript::_setMeleeDisabled(bool disable)
{
    _creature->GetAIInterface()->setMeleeDisabled(disable);
}

bool CreatureAIScript::_isMeleeDisabled()
{
    return _creature->GetAIInterface()->isMeleeDisabled();
}

void CreatureAIScript::_setRangedDisabled(bool disable)
{
    _creature->GetAIInterface()->setRangedDisabled(disable);
}

bool CreatureAIScript::_isRangedDisabled()
{
    return _creature->GetAIInterface()->isRangedDisabled();
}

void CreatureAIScript::_setCastDisabled(bool disable)
{
    _creature->GetAIInterface()->setCastDisabled(disable);
}

bool CreatureAIScript::_isCastDisabled()
{
    return _creature->GetAIInterface()->isCastDisabled();
}

void CreatureAIScript::_setTargetingDisabled(bool disable)
{
    _creature->GetAIInterface()->setTargetingDisabled(disable);
}

bool CreatureAIScript::_isTargetingDisabled()
{
    return _creature->GetAIInterface()->isTargetingDisabled();
}

void CreatureAIScript::_clearHateList()
{
    _creature->GetAIInterface()->ClearHateList();
}

void CreatureAIScript::_wipeHateList()
{
    _creature->GetAIInterface()->WipeHateList();
}

int32_t CreatureAIScript::_getHealthPercent()
{
    return _creature->GetHealthPct();
}

int32_t CreatureAIScript::_getManaPercent()
{
    return _creature->GetManaPct();
}

void CreatureAIScript::_regenerateHealth()
{
    _creature->RegenerateHealth();
    _creature->RegeneratePower(false);
}

bool CreatureAIScript::_isCasting()
{
    return _creature->isCastingNonMeleeSpell();
}

bool CreatureAIScript::_isHeroic()
{
    MapMgr* mapMgr = _creature->GetMapMgr();
    if (mapMgr == nullptr || mapMgr->iInstanceMode != MODE_HEROIC)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// script phase

uint32_t CreatureAIScript::getScriptPhase()
{
    return mScriptPhase;
}

void CreatureAIScript::setScriptPhase(uint32_t scriptPhase)
{
    if (isScriptPhase(scriptPhase) == false)
    {
        CancelAllSpells();
        for (auto SpellIter : mPhaseSpells)
        {
            if (SpellIter.first == scriptPhase)
                SpellIter.second->mEnabled = true;
            else
                SpellIter.second->mEnabled = false;
        }

        mScriptPhase = scriptPhase;

        if (getScriptPhase() != 0)
            _internalOnScriptPhaseChange();
    }
}

void CreatureAIScript::resetScriptPhase()
{
    setScriptPhase(0);
}

bool CreatureAIScript::isScriptPhase(uint32_t scriptPhase)
{
    return (getScriptPhase() == scriptPhase);
}

//////////////////////////////////////////////////////////////////////////////////////////
// timers

uint32 CreatureAIScript::_addTimer(uint32_t durationInMs)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        uint32_t timerId = inScript->addTimer(durationInMs);
        mCreatureTimerIds.push_back(timerId);

        return timerId;
    }
    else
    {
        uint32_t timerId = ++mCreatureTimerCount;
        mCreatureTimer.push_back(std::make_pair(timerId, durationInMs));

        return timerId;
    }
}

uint32_t CreatureAIScript::_getTimeForTimer(uint32_t timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        return inScript->getTimeForTimer(timerId);
    }
    else
    {
        for (const auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                return intTimer.second;
        }
    }

    return 0;
}

void CreatureAIScript::_removeTimer(uint32_t& timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        uint32_t mTimerId = timerId;
        inScript->removeTimer(timerId);
        if (timerId == 0)
            mCreatureTimerIds.remove(mTimerId);
    }
    else
    {
        for (CreatureTimerArray::iterator intTimer = mCreatureTimer.begin(); intTimer != mCreatureTimer.end(); ++intTimer)
        {
            if (intTimer->first == timerId)
            {
                mCreatureTimer.erase(intTimer);
                timerId = 0;
                break;
            }
        }
    }
}

void CreatureAIScript::_resetTimer(uint32_t timerId, uint32_t durationInMs)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        inScript->resetTimer(timerId, durationInMs);
    }
    else
    {
        for (auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                intTimer.second = durationInMs;
        }
    }
}

bool CreatureAIScript::_isTimerFinished(uint32_t timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        return inScript->isTimerFinished(timerId);
    }
    else
    {
        for (const auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                return intTimer.second == 0;
        }
    }

    return false;
}

void CreatureAIScript::_cancelAllTimers()
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        for (auto& timer : mCreatureTimerIds)
            _removeTimer(timer);

        mCreatureTimerIds.clear();
    }
    else
    {
        mCreatureTimer.clear();
    }

    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_cancelAllTimers() - all cleared!");
}

uint32_t CreatureAIScript::_getTimerCount()
{
    if (InstanceScript* inScript = getInstanceScript())
        return static_cast<uint32_t>(mCreatureTimerIds.size());
    
    return static_cast<uint32_t>(mCreatureTimer.size());
}

void CreatureAIScript::updateAITimers()
{
    for (auto& TimerIter : mCreatureTimer)
    {
        if (TimerIter.second > 0)
        {
            int leftTime = TimerIter.second - mAIUpdateFrequency;
            if (leftTime > 0)
                TimerIter.second -= mAIUpdateFrequency;
            else
                TimerIter.second = 0;
        }
    }
}

void CreatureAIScript::displayCreatureTimerList(Player* player)
{
    player->BroadcastMessage("=== Timers for creature %s ===", getCreature()->GetCreatureProperties()->Name.c_str());

    if (mCreatureTimerIds.empty() && mCreatureTimer.empty())
    {
        player->BroadcastMessage("  No Timers available!");
    }
    else
    {
        if (InstanceScript* inScript = getInstanceScript())
        {
            for (const auto& intTimer : mCreatureTimerIds)
                player->BroadcastMessage("  TimerId (%u)  %u ms left", intTimer, _getTimeForTimer(intTimer));
        }
        else
        {
            for (const auto& intTimer : mCreatureTimer)
                player->BroadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// ai upodate frequency
void CreatureAIScript::registerAiUpdateFrequency()
{
    sEventMgr.AddEvent(_creature, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, mAIUpdateFrequency, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void CreatureAIScript::removeAiUpdateFrequency()
{
    sEventMgr.RemoveEvents(_creature, EVENT_SCRIPT_UPDATE_EVENT);
}

// old stuff
void CreatureAIScript::SetAIUpdateFreq(uint32 pUpdateFreq)
{
    if (mCustomAIUpdateDelay != pUpdateFreq)
    {
        mCustomAIUpdateDelay = pUpdateFreq;
        _resetTimer(mCustomAIUpdateDelayTimerId, mCustomAIUpdateDelay);
    }
}

uint32 CreatureAIScript::GetAIUpdateFreq()
{
    return mAIUpdateFrequency;
}

void CreatureAIScript::RegisterAIUpdateEvent(uint32 frequency)
{
    if (mCustomAIUpdateDelayTimerId == 0)
    {
        mCustomAIUpdateDelayTimerId = _addTimer(frequency);
        mCustomAIUpdateDelay = frequency;
    }
    else
    {
        ModifyAIUpdateEvent(frequency);
    }
}

void CreatureAIScript::ModifyAIUpdateEvent(uint32 newfrequency)
{
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        _resetTimer(mCustomAIUpdateDelayTimerId, newfrequency);
        mCustomAIUpdateDelay = newfrequency;
    }
}

void CreatureAIScript::RemoveAIUpdateEvent()
{
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        _removeTimer(mCustomAIUpdateDelayTimerId);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// appearance

void CreatureAIScript::_setScale(float scale)
{
    _creature->setFloatValue(OBJECT_FIELD_SCALE_X, scale);
}

float CreatureAIScript::_getScale()
{
    return _creature->getFloatValue(OBJECT_FIELD_SCALE_X);
}

void CreatureAIScript::_setDisplayId(uint32_t displayId)
{
    _creature->SetDisplayId(displayId);
}

void CreatureAIScript::_setWieldWeapon(bool setWieldWeapon)
{
    if (setWieldWeapon && _creature->getUInt32Value(UNIT_FIELD_BYTES_2) != 1)
    {
        _creature->setUInt32Value(UNIT_FIELD_BYTES_2, 1);
    }
    else if (!setWieldWeapon && _creature->getUInt32Value(UNIT_FIELD_BYTES_2) != 0)
    {
        _creature->setUInt32Value(UNIT_FIELD_BYTES_2, 0);
    }
}

void CreatureAIScript::_setDisplayWeapon(bool setMainHand, bool setOffHand)
{
    _setDisplayWeaponIds(setMainHand ? _creature->GetEquippedItem(MELEE) : 0, setOffHand ? _creature->GetEquippedItem(OFFHAND) : 0);
}

void CreatureAIScript::_setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2)
{
    _creature->SetEquippedItem(MELEE, itemId1);
    _creature->SetEquippedItem(OFFHAND, itemId2);
}

//////////////////////////////////////////////////////////////////////////////////////////
// spell

void CreatureAISpells::addDBEmote(uint32_t textId)
{
    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
    if (npcScriptText != nullptr)
        addEmote(npcScriptText->text, npcScriptText->type, npcScriptText->sound);
    else
        LogDebugFlag(LF_SCRIPT_MGR, "A script tried to add a spell emote with %u! Id is not available in table npc_script_text.", textId);
}

void CreatureAISpells::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mAISpellEmote.push_back(AISpellEmotes(pText, pType, pSoundId));
}

void CreatureAISpells::sendRandomEmote(CreatureAIScript* creatureAI)
{
    if (!mAISpellEmote.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendRandomEmote() : called");

        uint32_t randomUInt = (mAISpellEmote.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mAISpellEmote.size() - 1)) : 0;
        creatureAI->getCreature()->SendChatMessage(mAISpellEmote[randomUInt].mType, LANG_UNIVERSAL, mAISpellEmote[randomUInt].mText.c_str());

        if (mAISpellEmote[randomUInt].mSoundId != 0)
            creatureAI->getCreature()->PlaySoundToSet(mAISpellEmote[randomUInt].mSoundId);
    }
}

void CreatureAISpells::sendAnnouncement(CreatureAIScript* creatureAI)
{
    if (!mAnnouncement.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendAnnouncement() : called");

        creatureAI->getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, mAnnouncement.c_str());
    }
}

void CreatureAIScript::newAIUpdateSpellSystem()
{
    if (mLastCastedSpell)
    {
        if (!_isTimerFinished(mSpellWaitTimerId))
        {
            // spell has a min/max range
            if (!getCreature()->isCastingNonMeleeSpell() && (mLastCastedSpell->mMaxPositionRangeToCast > 0.0f || mLastCastedSpell->mMinPositionRangeToCast > 0.0f))
            {
                // if we have a current target and spell is not triggered
                if (mCurrentSpellTarget != nullptr && !mLastCastedSpell->mIsTriggered)
                {
                    // interrupt spell if we are not in  required range
                    const float targetDistance = getCreature()->GetPosition().Distance2DSq(mCurrentSpellTarget->GetPositionX(), mCurrentSpellTarget->GetPositionY());
                    if (!mLastCastedSpell->isDistanceInRange(targetDistance))
                    {
                        LogDebugFlag(LF_SCRIPT_MGR, "Target outside of spell range (%u)! Min: %f Max: %f, distance to Target: %f", mLastCastedSpell->mSpellInfo->getId(), mLastCastedSpell->mMinPositionRangeToCast, mLastCastedSpell->mMaxPositionRangeToCast, targetDistance);
                        getCreature()->interruptSpell();
                        mLastCastedSpell = nullptr;
                    }
                }
            }
        }
        else
        {
            // spell gets not interupted after casttime(duration) so we can send the emote.
            mLastCastedSpell->sendRandomEmote(this);

            // override attack stop timer if needed
            if (mLastCastedSpell->getAttackStopTimer() != 0)
                getCreature()->setAttackTimer(mLastCastedSpell->getAttackStopTimer(), false);

            mLastCastedSpell = nullptr;
        }
    }

    // cleanup exeeded spells
    for (const auto& AISpell : mCreatureAISpells)
    {
        if (AISpell != nullptr)
        {
            // stop spells and remove aura in case of duration
            if (_isTimerFinished(AISpell->mDurationTimerId) && AISpell->mForceRemoveAura)
            {
                getCreature()->interruptSpell();
                _removeAura(AISpell->mSpellInfo->getId());
            }
        }
    }

    // cast one spell and check if spell is done (duration)
    if (_isTimerFinished(mSpellWaitTimerId))
    {
        CreatureAISpells* usedSpell = nullptr;

        float randomChance = Util::getRandomFloat(100.0f);
        std::random_shuffle(mCreatureAISpells.begin(), mCreatureAISpells.end());
        for (const auto& AISpell : mCreatureAISpells)
        {
            if (AISpell != nullptr)
            {
                // spell was casted before, check if the wait time is done
                if (!_isTimerFinished(AISpell->mCooldownTimerId))
                    continue;

                // is bound to a specific phase (all greater than 0)
                if (!AISpell->isAvailableForScriptPhase(getScriptPhase()))
                    continue;

                // aura stacking
                if (getCreature()->getAuraCountForId(AISpell->mSpellInfo->getId()) >= AISpell->getMaxStackCount())
                    continue;

                // hp range
                if (!AISpell->isHpInPercentRange(getCreature()->GetHealthPct()))
                    continue;

                // no random chance (cast in script)
                if (AISpell->mCastChance == 0.0f)
                    continue;

                // do not cast any spell while stunned/feared/silenced/charmed/confused
                if (getCreature()->hasUnitStateFlag(UNIT_STATE_STUN | UNIT_STATE_FEAR | UNIT_STATE_SILENCE | UNIT_STATE_CHARM | UNIT_STATE_CONFUSE))
                    break;

                // random chance for shuffeld array should do the job
                if (randomChance < AISpell->mCastChance)
                {
                    usedSpell = AISpell;
                    break;
                }
            }
        }

        if (usedSpell != nullptr)
        {
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();
            switch (usedSpell->mTargetType)
            {
                case TARGET_SELF:
                case TARGET_VARIOUS:
                {
                    getCreature()->CastSpell(getCreature(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_ATTACKING:
                {
                    getCreature()->CastSpell(target, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_DESTINATION:
                {
                    getCreature()->CastSpellAoF(target->GetPosition(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_RANDOM_FRIEND:
                case TARGET_RANDOM_SINGLE:
                case TARGET_RANDOM_DESTINATION:
                {
                    castSpellOnRandomTarget(usedSpell);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_CUSTOM:
                {
                    // nos custom target set, no spell cast.
                    if (usedSpell->getCustomTarget() != nullptr)
                        getCreature()->CastSpell(usedSpell->getCustomTarget(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                } break;
            }

            // send announcements on casttime beginn
            usedSpell->sendAnnouncement(this);

            // reset cast wait timer for CreatureAIScript - Important for _internalAIUpdate
            _resetTimer(mSpellWaitTimerId, usedSpell->mDuration);

            // reset spell timers to cleanup exceeded spells
            _resetTimer(usedSpell->mDurationTimerId, usedSpell->mDuration);
            _resetTimer(usedSpell->mCooldownTimerId, usedSpell->mCooldown);

        }
    }
}

void CreatureAIScript::castSpellOnRandomTarget(CreatureAISpells* AiSpell)
{
    if (AiSpell == nullptr)
        return;

    // helper for following code
    bool isTargetRandFriend = (AiSpell->mTargetType == TARGET_RANDOM_FRIEND ? true : false);

    // if we already cast a spell, do not set/cast another one!
    if (!getCreature()->isCastingNonMeleeSpell()
        && getCreature()->GetAIInterface()->getNextTarget())
    {
        // set up targets in range by position, relation and hp range
        std::vector<Unit*> possibleUnitTargets;

        for (const auto& inRangeObject : getCreature()->GetInRangeSet())
        {
            if (((isTargetRandFriend && isFriendly(getCreature(), inRangeObject))
                || (!isTargetRandFriend && isHostile(getCreature(), inRangeObject) && inRangeObject != getCreature())) && inRangeObject->IsUnit())
            {
                Unit* inRangeTarget = static_cast<Unit*>(inRangeObject);

                if (
                    inRangeTarget->isAlive() && AiSpell->isDistanceInRange(getCreature()->GetDistance2dSq(inRangeTarget)) 
                    && ((AiSpell->isHpInPercentRange(inRangeTarget->GetHealthPct()) && isTargetRandFriend)
                    || (getCreature()->GetAIInterface()->getThreatByPtr(inRangeTarget) > 0 && isHostile(getCreature(), inRangeTarget))))
                {
                    possibleUnitTargets.push_back(inRangeTarget);
                }
            }
        }

        // add us as a friendly target.
        if (AiSpell->isHpInPercentRange(getCreature()->GetHealthPct()) && isTargetRandFriend)
            possibleUnitTargets.push_back(getCreature());

        // no targets in our range for hp range and firendly targets
        if (possibleUnitTargets.empty())
            return;

        // get a random target
        uint32_t randomIndex = Util::getRandomUInt(0, static_cast<uint32_t>(possibleUnitTargets.size() - 1));
        Unit* randomTarget = possibleUnitTargets[randomIndex];

        if (randomTarget == nullptr)
            return;

        switch (AiSpell->mTargetType)
        {
            case TARGET_RANDOM_FRIEND:
            case TARGET_RANDOM_SINGLE:
            {
                getCreature()->CastSpell(randomTarget, AiSpell->mSpellInfo, AiSpell->mIsTriggered);
                mCurrentSpellTarget = randomTarget;
            } break;
            case TARGET_RANDOM_DESTINATION:
                getCreature()->CastSpellAoF(randomTarget->GetPosition(), AiSpell->mSpellInfo, AiSpell->mIsTriggered);
                break;
        }

        possibleUnitTargets.clear();
    }
}

void CreatureAIScript::oldAIUpdateSpellSystem()
{
    // MoonScript spell handling...
    if (mSpells.size() > 0)
    {
        // enrage automation not implemented correct yet
        /*if (mEnrageSpell && mEnrageTimerDuration > 0 && _isTimerFinished(mEnrageTimer))
        {
        CastSpell(mEnrageSpell);
        _removeTimer(mEnrageTimer);
        }*/

        SpellDesc* Spell;
        uint32 CurrentTime = (uint32)time(nullptr);

        //Check if we have a spell scheduled to be cast
        if (!_isCasting())
        {
            for (SpellDescList::iterator SpellIter = mScheduledSpells.begin(); SpellIter != mScheduledSpells.end();)
            {
                Spell = (*SpellIter);
                if (CastSpellInternal(Spell, CurrentTime))    //Can fail if we are already casting a spell, or if the spell is on cooldown
                {
                    if (!mScheduledSpells.empty())            // \todo temporary crashfix, we should use mutax here, but it needs more investigation
                        mScheduledSpells.erase(SpellIter);

                    break;
                }
                else
                    ++SpellIter;
            }
        }

        //Do not schedule spell if we are *currently* casting a non-instant cast spell
        if (!_isCasting() && !mRunToTargetCache)
        {
            //Check if have queued spells that needs to be scheduled before we go back to random casting
            if (!mQueuedSpells.empty())
            {
                Spell = mQueuedSpells.front();
                mScheduledSpells.push_back(Spell);
                mQueuedSpells.pop_front();

                //Stop melee attack for a short while for scheduled spell cast
                if (Spell->mCastTime >= 0)
                {
                    _delayNextAttack(mAIUpdateFrequency);
                    if (Spell->mCastTime > 0)
                    {
                        setRooted(false);
                        setAIAgent(AGENT_SPELL);
                    }
                }
                return;    //Scheduling one spell at a time, exit now
            }

            //Try our chance at casting a spell (Will actually be cast on next ai update, so we just
            //schedule it. This is needed to avoid next dealt melee damage while we cast the spell.)
            float ChanceRoll = RandomFloat(100), ChanceTotal = 0;
            for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
            {
                Spell = (*SpellIter);
                if (Spell->mEnabled == false)
                    continue;
                if (Spell->mChance == 0)
                    continue;

                //Check if spell won the roll
                if ((Spell->mChance == 100 || (ChanceRoll >= ChanceTotal && ChanceRoll < ChanceTotal + Spell->mChance)) &&
                    (Spell->mLastCastTime + Spell->mCooldown <= CurrentTime) &&
                    !IsSpellScheduled(Spell))
                {
                    mScheduledSpells.push_back(Spell);

                    //Stop melee attack for a short while for scheduled spell cast
                    if (Spell->mCastTime >= 0)
                    {
                        _delayNextAttack(mAIUpdateFrequency + CalcSpellAttackTime(Spell));
                        if (Spell->mCastTime > 0)
                        {
                            setRooted(true);
                            setAIAgent(AGENT_SPELL);
                        }
                    }
                    return;    //Scheduling one spell at a time, exit now
                }
                else if (Spell->mChance != 100)
                    ChanceTotal += Spell->mChance;    //Only add spells that aren't 100% chance of casting
            }

            //Go back to default behavior since we didn't decide anything
            setRooted(false);
            setAIAgent(AGENT_MELEE);

            //Random taunts
            if (ChanceRoll >= 95)
                sendRandomDBChatMessage(mEmotesOnTaunt);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// spell deprecated

void CreatureAIScript::_applyAura(uint32_t spellId)
{
    _creature->CastSpell(_creature, sSpellCustomizations.GetSpellInfo(spellId), true);
}

void CreatureAIScript::_removeAura(uint32_t spellId)
{
    _creature->RemoveAura(spellId);
}

void CreatureAIScript::_removeAllAuras()
{
    _creature->RemoveAllAuras();
}

void CreatureAIScript::_removeAuraOnPlayers(uint32_t spellId)
{
    for (auto object : *_creature->GetInRangePlayerSet())
    {
        if (object != nullptr)
            static_cast<Player*>(object)->RemoveAura(spellId);
    }
}

void CreatureAIScript::_castOnInrangePlayers(uint32_t spellId, bool triggered)
{
    for (auto object : *_creature->GetInRangePlayerSet())
    {
        if (object != nullptr)
            _creature->CastSpell(static_cast<Player*>(object), spellId, triggered);
    }
}

void CreatureAIScript::_castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered)
{
    for (auto object : *_creature->GetInRangePlayerSet())
    {
        if (object != nullptr)
        {
            float distanceToPlayer = object->GetDistance2dSq(this->getCreature());
            if (distanceToPlayer >= minDistance && distanceToPlayer <= maxDistance)
                _creature->CastSpell(static_cast<Player*>(object), spellId, triggered);
        }
    }
}

void CreatureAIScript::_setTargetToChannel(Unit* target, uint32_t spellId)
{
    if (target != nullptr)
    {
        _creature->SetChannelSpellTargetGUID(target->GetGUID());
        _creature->SetChannelSpellId(spellId);
    }
    else
    {
        _unsetTargetToChannel();
    }
}

void CreatureAIScript::_unsetTargetToChannel()
{
    _creature->SetChannelSpellTargetGUID(0);
    _creature->SetChannelSpellId(0);
}

Unit* CreatureAIScript::_getTargetToChannel()
{
    return _creature->GetMapMgr()->GetUnit(_creature->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
}

//////////////////////////////////////////////////////////////////////////////////////////
// gameobject

GameObject* CreatureAIScript::getNearestGameObject(uint32_t entry)
{
    return getNearestGameObject(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ(), entry);
}

GameObject* CreatureAIScript::getNearestGameObject(float posX, float posY, float posZ, uint32_t entry)
{
    return _creature->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(posX, posY, posZ, entry);
}

//////////////////////////////////////////////////////////////////////////////////////////
// chat message

void CreatureAIScript::sendChatMessage(uint8_t type, uint32_t soundId, std::string text)
{
    if (text.empty() == false)
        _creature->SendChatMessage(type, LANG_UNIVERSAL, text.c_str());

    if (soundId > 0)
        _creature->PlaySoundToSet(soundId);
}

void CreatureAIScript::sendDBChatMessage(uint32_t textId)
{
    _creature->SendScriptTextChatMessage(textId);
}

void CreatureAIScript::sendRandomDBChatMessage(std::vector<uint32_t> emoteVector)
{
    if (!emoteVector.empty())
    {
        uint32_t randomUInt = (emoteVector.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(emoteVector.size() - 1)) : 0;

        sendDBChatMessage(emoteVector[randomUInt]);
    }
}

void CreatureAIScript::addEmoteForEvent(uint32_t eventType, uint32_t scriptTextId)
{
    MySQLStructure::NpcScriptText const* ct = sMySQLStore.getNpcScriptText(scriptTextId);
    if (ct != nullptr)
    {
        switch (eventType)
        {
            case Event_OnCombatStart:
                mEmotesOnCombatStart.push_back(scriptTextId);
                break;
            case Event_OnTargetDied:
                mEmotesOnTargetDied.push_back(scriptTextId);
                break;
            case Event_OnDied:
                mEmotesOnDied.push_back(scriptTextId);
                break;
            case Event_OnTaunt:
                mEmotesOnTaunt.push_back(scriptTextId);
                break;
            case Event_OnIdle:
                mEmotesOnIdle.push_back(scriptTextId);
                break;
            default:
                LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::addEmoteForEvent : Invalid event type: %u !", eventType);
                break;
        }
    }
    else
    {
        LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::addEmoteForEvent : id: %u is not available in table npc_script_text!", scriptTextId);
    }
}

void CreatureAIScript::sendAnnouncement(std::string stringAnnounce)
{
    if (!stringAnnounce.empty())
        _creature->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, stringAnnounce.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// idle emote timer

void CreatureAIScript::enableOnIdleEmote(bool enable, uint32_t durationInMs /*= 0*/)
{
    if (enable && mEmotesOnIdle.empty())
    {
        LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::enableOnIdleEmote : no IdleEvents available!");
        return;
    }

    isIdleEmoteEnabled = enable;

    if (isIdleEmoteEnabled)
    {
        setIdleEmoteTimerId(_addTimer(durationInMs));
    }
    else
    {
        uint32_t idleTimerId = getIdleEmoteTimerId();
        _removeTimer(idleTimerId);
    }
}

void CreatureAIScript::setIdleEmoteTimerId(uint32_t timerId)
{
    idleEmoteTimerId = timerId;
}

uint32_t CreatureAIScript::getIdleEmoteTimerId()
{
    return idleEmoteTimerId;
}

void CreatureAIScript::resetIdleEmoteTime(uint32_t durationInMs)
{
    _resetTimer(idleEmoteTimerId, durationInMs);
}

void CreatureAIScript::setRandomIdleEmoteTime(uint32_t minTime, uint32_t maxTime)
{
    idleEmoteTimeMin = minTime;
    idleEmoteTimeMax = maxTime;
}

void CreatureAIScript::generateNextRandomIdleEmoteTime()
{
    resetIdleEmoteTime(Util::getRandomUInt(idleEmoteTimeMin, idleEmoteTimeMax));
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance

InstanceScript* CreatureAIScript::getInstanceScript()
{
    MapMgr* mapMgr = _creature->GetMapMgr();
    return (mapMgr) ? mapMgr->GetScript() : nullptr;
}

void CreatureAIScript::LinkedCreatureDeleted()
{
    linkedCreatureAI = NULL;
}

void CreatureAIScript::SetLinkedCreature(CreatureAIScript* creatureAI)
{
    //notify our linked creature that we are not more linked
    if (linkedCreatureAI != NULL)
        linkedCreatureAI->LinkedCreatureDeleted();

    //link to the new creature
    linkedCreatureAI = creatureAI;
}


//////////////////////////////////////////////////////////////////////////////////////////
// MoonScriptCreatureAI functions
void CreatureAIScript::MoveTo(Unit* pUnit, RangeStatusPair pRangeStatus)
{
    if (pRangeStatus.first == RangeStatus_TooClose)
        getCreature()->GetAIInterface()->_CalcDestinationAndMove(pUnit, pRangeStatus.second);
    else if (pRangeStatus.first == RangeStatus_TooFar)
        moveTo(pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ());
};

void CreatureAIScript::AggroNearestUnit(uint32_t pInitialThreat)
{
    //Pay attention: if this is called before pushing the Creature to world, OnCombatStart will NOT be called.
    Unit* NearestRandomTarget = GetBestUnitTarget(TargetFilter_Closest);
    if (NearestRandomTarget)
        getCreature()->GetAIInterface()->AttackReaction(NearestRandomTarget, pInitialThreat);
}

void CreatureAIScript::AggroRandomUnit(uint32_t pInitialThreat)
{
    Unit* RandomTarget = GetBestUnitTarget();
    if (RandomTarget)
    {
        getCreature()->GetAIInterface()->AttackReaction(RandomTarget, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(RandomTarget);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

void CreatureAIScript::AggroNearestPlayer(uint32_t pInitialThreat)
{
    Unit* NearestRandomPlayer = GetBestPlayerTarget(TargetFilter_Closest);
    if (NearestRandomPlayer)
    {
        getCreature()->GetAIInterface()->AttackReaction(NearestRandomPlayer, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(NearestRandomPlayer);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

void CreatureAIScript::AggroRandomPlayer(uint32_t pInitialThreat)
{
    Unit* RandomPlayer = GetBestPlayerTarget();
    if (RandomPlayer)
    {
        getCreature()->GetAIInterface()->AttackReaction(RandomPlayer, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(RandomPlayer);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

SpellDesc* CreatureAIScript::AddSpell(uint32_t pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange, float pMaxRange,
    bool pStrictRange, std::string pText, uint8_t pTextType, uint32_t pSoundId, std::string pAnnouncement)
{
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(pSpellId);
    if (spellInfo != nullptr)
    {
        float CastTime = pCastTime;
        int32 Cooldown = pCooldown;
        float MinRange = pMinRange;
        float MaxRange = pMaxRange;

        SpellDesc* newSpellDesc = new SpellDesc(spellInfo, 0, pTargetType, pChance, CastTime, Cooldown, MinRange, MaxRange, pStrictRange, pText, pTextType, pSoundId, pAnnouncement);
        mSpells.push_back(newSpellDesc);
        return newSpellDesc;
    }
    else
    {
        LOG_ERROR("tried to create spell with invalid id %u", pSpellId);
        return nullptr;
    }
}

SpellDesc* CreatureAIScript::AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange, float pMaxRange,
    bool pStrictRange, std::string pText, uint8_t pTextType, uint32_t pSoundId, std::string pAnnouncement)
{
    if (!pFnc)
    {
        LOG_ERROR("tried to create spell with invalid SpellFunc");
        return nullptr;
    }
    else
    {
        SpellDesc* newSpellDesc = new SpellDesc(nullptr, pFnc, pTargetType, pChance, pCastTime, pCooldown, pMinRange, pMaxRange, pStrictRange, pText, pTextType, pSoundId, pAnnouncement);
        mSpells.push_back(newSpellDesc);
        return newSpellDesc;
    }
}

void CreatureAIScript::CastSpell(SpellDesc* pSpell)
{
    if (!IsSpellScheduled(pSpell))
        mQueuedSpells.push_back(pSpell);
}

void CreatureAIScript::CastSpellNowNoScheduling(SpellDesc* pSpell)
{
    if (CastSpellInternal(pSpell))
        _delayNextAttack(CalcSpellAttackTime(pSpell));
}

SpellDesc* CreatureAIScript::FindSpellById(uint32_t pSpellId)
{
    for (auto& creatureSpell : mSpells)
    {
        if (creatureSpell)
        {
            if (creatureSpell->mInfo && creatureSpell->mInfo->getId() == pSpellId)
            {
                return creatureSpell;
            }
        }
    }
    return nullptr;
}

SpellDesc* CreatureAIScript::FindSpellByFunc(SpellFunc pFnc)
{
    for (auto& creatureSpell : mSpells)
    {
        if (creatureSpell)
        {
            if (creatureSpell->mSpellFunc == pFnc)
            {
                return creatureSpell;
            }
        }
    }
    return nullptr;
}

void CreatureAIScript::TriggerCooldownOnAllSpells()
{
    uint32_t currentTime = static_cast<uint32_t>(time(nullptr));
    for (auto& creatureSpell : mSpells)
    {
        if (creatureSpell)
        {
            creatureSpell->setTriggerCooldown(currentTime);
        }
    }
}

void CreatureAIScript::CancelAllCooldowns()
{
    for (auto& creatureSpell : mSpells)
    {
        if (creatureSpell)
        {
            creatureSpell->mLastCastTime = 0;
        }
    }
}

bool CreatureAIScript::IsSpellScheduled(SpellDesc* pSpell)
{
    return (std::find(mScheduledSpells.begin(), mScheduledSpells.end(), pSpell) == mScheduledSpells.end()) ? false : true;
}

void CreatureAIScript::CancelAllSpells()
{
    mQueuedSpells.clear();
    mScheduledSpells.clear();
    PopRunToTargetCache();
}

bool CreatureAIScript::CastSpellInternal(SpellDesc* pSpell, uint32_t pCurrentTime)
{
    if (pSpell == nullptr)
        return false;

    //Do not cast if we are already casting
    if (_isCasting())
        return false;

    //We do not cast in special states such as : stunned, feared, silenced, charmed, asleep, confused and if they are not ignored
    if ((~pSpell->mTargetType.mTargetFilter & TargetFilter_IgnoreSpecialStates) && getCreature()->hasUnitStateFlag(
        (UNIT_STATE_STUN | UNIT_STATE_FEAR | UNIT_STATE_SILENCE | UNIT_STATE_CHARM | UNIT_STATE_CONFUSE)))
        return false;

    //Do not cast if we are in cooldown
    uint32_t CurrentTime = (pCurrentTime == 0) ? static_cast<uint32_t>(time(nullptr)) : pCurrentTime;
    if (pSpell->mLastCastTime + pSpell->mCooldown > CurrentTime)
        return false;

    //Check range before casting
    Unit* Target = GetTargetForSpell(pSpell);
    if (Target)
    {
        RangeStatusPair Status;
        if (pSpell->mTargetType.mTargetFilter & TargetFilter_InRangeOnly || (Status = GetSpellRangeStatusToUnit(Target, pSpell)).first == RangeStatus_Ok)
        {
            //Safe-delay if we got special state flag before
            _delayNextAttack(CalcSpellAttackTime(pSpell));

            //If we were running to a target, stop because we're in range now
            PopRunToTargetCache();

            //Do emote associated with this spell
            pSpell->sendRandomEmote(this);
            sendAnnouncement(pSpell->mAnnouncement);

            //Cast spell now
            if (pSpell->mInfo)
                CastSpellOnTarget(Target, pSpell->mTargetType, pSpell->mInfo, (pSpell->mCastTime == 0) ? true : false);
            else if
                (pSpell->mSpellFunc) pSpell->mSpellFunc(pSpell, this, Target, pSpell->mTargetType);
            else
                LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::CastSpellInternal() : Invalid spell!");

            //Store cast time for cooldown
            pSpell->mLastCastTime = CurrentTime;
            return true;
        }
        else if (!pSpell->mStrictRange)   //Target is out of range, run to it
        {
            PushRunToTargetCache(Target, pSpell, Status);
            return false;
        }
    }

    //If we get here, its because the RunToTarget changed type, so its no longer valid, clear it
    PopRunToTargetCache();
    _delayNextAttack(0);        //Cancel attack delay
    return true;            //No targets possible? Consider spell casted nonetheless
}

void CreatureAIScript::CastSpellOnTarget(Unit* pTarget, TargetType pType, SpellInfo* pEntry, bool pInstant)
{
    switch (pType.mTargetGenerator)
    {
        case TargetGen_Self:
        case TargetGen_Current:
        case TargetGen_Predefined:
        case TargetGen_RandomUnit:
        case TargetGen_RandomPlayer:
            getCreature()->CastSpell(pTarget, pEntry, pInstant);
            break;

        case TargetGen_RandomUnitApplyAura:
        case TargetGen_RandomPlayerApplyAura:
            pTarget->CastSpell(pTarget, pEntry, pInstant);
            break;

        case TargetGen_Destination:
        case TargetGen_RandomUnitDestination:
        case TargetGen_RandomPlayerDestination:
            getCreature()->CastSpellAoF(pTarget->GetPosition(), pEntry, pInstant);
            break;

        default:
            LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::CastSpellOnTarget() : Invalid target type!");
            break;
    };
};

int32 CreatureAIScript::CalcSpellAttackTime(SpellDesc* pSpell)
{
    return mBaseAttackTime + int32(pSpell->mCastTime * 1000);
}

RangeStatusPair CreatureAIScript::GetSpellRangeStatusToUnit(Unit* pTarget, SpellDesc* pSpell)
{
    if (pSpell->mTargetType.mTargetGenerator != TargetGen_Self && pTarget != getCreature() && (pSpell->mMinRange > 0 || pSpell->mMaxRange > 0))
    {
        float Range = getRangeToObject(pTarget);
        if (pSpell->mMinRange > 0 && (Range < pSpell->mMinRange))
            return std::make_pair(RangeStatus_TooClose, pSpell->mMinRange);
        if (pSpell->mMaxRange > 0 && (Range > pSpell->mMaxRange))
            return std::make_pair(RangeStatus_TooFar, pSpell->mMaxRange);
    }

    return std::make_pair(RangeStatus_Ok, 0.0f);
};

Unit* CreatureAIScript::GetTargetForSpell(SpellDesc* pSpell)
{
    //Check if run-to-target cache and return it if its valid
    if (mRunToTargetCache && mRunToTargetSpellCache == pSpell && IsValidUnitTarget(mRunToTargetCache, TargetFilter_None))
        return mRunToTargetCache;

    //Find a suitable target for the described situation :)
    switch (pSpell->mTargetType.mTargetGenerator)
    {
        case TargetGen_Self:
            if (!isAlive())
                return nullptr;
            if ((pSpell->mTargetType.mTargetFilter & TargetFilter_Wounded) && getCreature()->GetHealthPct() >= 99)
                return nullptr;

            return getCreature();
        case TargetGen_SecondMostHated:
            return getCreature()->GetAIInterface()->GetSecondHated();
        case TargetGen_Current:
        case TargetGen_Destination:
            return getCreature()->GetAIInterface()->getNextTarget();
        case TargetGen_Predefined:
            return pSpell->mPredefinedTarget;
        case TargetGen_RandomPlayer:
        case TargetGen_RandomPlayerApplyAura:
        case TargetGen_RandomPlayerDestination:
            return GetBestPlayerTarget(pSpell->mTargetType.mTargetFilter, pSpell->mMinRange, pSpell->mMaxRange);
        case TargetGen_RandomUnit:
        case TargetGen_RandomUnitApplyAura:
        case TargetGen_RandomUnitDestination:
            return GetBestUnitTarget(pSpell->mTargetType.mTargetFilter, pSpell->mMinRange, pSpell->mMaxRange);
        default:
            LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::GetTargetForSpell() : Invalid target type!");
            return nullptr;
    }
};

Unit* CreatureAIScript::GetBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    for (std::set< Object* >::iterator PlayerIter = getCreature()->GetInRangePlayerSetBegin(); PlayerIter != getCreature()->GetInRangePlayerSetEnd(); ++PlayerIter)
    {
        if (IsValidUnitTarget(*PlayerIter, pTargetFilter, pMinRange, pMaxRange))
            TargetArray.push_back(static_cast<Unit*>(*PlayerIter));
    }

    return ChooseBestTargetInArray(TargetArray, pTargetFilter);
};

Unit* CreatureAIScript::GetBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (std::set< Object* >::iterator ObjectIter = getCreature()->GetInRangeSetBegin(); ObjectIter != getCreature()->GetInRangeSetEnd(); ++ObjectIter)
        {
            if (IsValidUnitTarget(*ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(*ObjectIter));
        }

        if (IsValidUnitTarget(getCreature(), pTargetFilter))
            TargetArray.push_back(getCreature());    //Also add self as possible friendly target
    }
    else
    {
        for (std::set< Object* >::iterator ObjectIter = getCreature()->GetInRangeOppFactsSetBegin(); ObjectIter != getCreature()->GetInRangeOppFactsSetEnd(); ++ObjectIter)
        {
            if (IsValidUnitTarget(*ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(*ObjectIter));
        }
    }

    return ChooseBestTargetInArray(TargetArray, pTargetFilter);
};

Unit* CreatureAIScript::ChooseBestTargetInArray(UnitArray & pTargetArray, TargetFilter pTargetFilter)
{
    //If only one possible target, return it
    if (pTargetArray.size() == 1)
        return pTargetArray[0];

    //Find closest unit if requested
    if (pTargetFilter & TargetFilter_Closest)
        return GetNearestTargetInArray(pTargetArray);

    //Find second most hated if requested
    if (pTargetFilter & TargetFilter_SecondMostHated)
        return GetSecondMostHatedTargetInArray(pTargetArray);

    //Choose random unit in array
    return (pTargetArray.size() > 1) ? pTargetArray[RandomUInt((uint32)pTargetArray.size() - 1)] : nullptr;
};

Unit* CreatureAIScript::GetNearestTargetInArray(UnitArray& pTargetArray)
{
    Unit* NearestUnit = nullptr;
    float Distance, NearestDistance = 99999;
    for (UnitArray::iterator UnitIter = pTargetArray.begin(); UnitIter != pTargetArray.end(); ++UnitIter)
    {
        Distance = getRangeToObject(static_cast<Unit*>(*UnitIter));
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestUnit = (*UnitIter);
        }
    }

    return NearestUnit;
};

Unit* CreatureAIScript::GetSecondMostHatedTargetInArray(UnitArray & pTargetArray)
{
    Unit* TargetUnit = nullptr;
    Unit* MostHatedUnit = nullptr;
    Unit* CurrentTarget = static_cast<Unit*>(getCreature()->GetAIInterface()->getNextTarget());
    uint32_t Threat = 0;
    uint32_t HighestThreat = 0;
    for (UnitArray::iterator UnitIter = pTargetArray.begin(); UnitIter != pTargetArray.end(); ++UnitIter)
    {
        TargetUnit = static_cast<Unit*>(*UnitIter);
        if (TargetUnit != CurrentTarget)
        {
            Threat = getCreature()->GetAIInterface()->getThreatByPtr(TargetUnit);
            if (Threat > HighestThreat)
            {
                MostHatedUnit = TargetUnit;
                HighestThreat = Threat;
            }
        }
    }

    return MostHatedUnit;
};

bool CreatureAIScript::IsValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange)
{
    //Make sure its a valid unit
    if (!pObject->IsUnit())
        return false;
    if (pObject->GetInstanceID() != getCreature()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->IsCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->IsPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;
    if (UnitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
        return false;

    //Check if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        //Skip units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) == 0)
            return false;

        //Skip current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCreature()->GetAIInterface()->getNextTarget())
            return false;

        //Keep only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->GetHealthPct() >= 99)
            return false;

        //Skip targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && getRangeToObject(UnitTarget) > getCreature()->GetAIInterface()->_CalcCombatRange(UnitTarget, false))
            return false;

        //Skip targets not in strict range if requested
        if ((pFilter & TargetFilter_InRangeOnly) && (pMinRange > 0 || pMaxRange > 0))
        {
            float Range = getRangeToObject(UnitTarget);
            if (pMinRange > 0 && Range < pMinRange)
                return false;
            if (pMaxRange > 0 && Range > pMaxRange)
                return false;
        }

        //Skip targets not in Line Of Sight if requested
        if ((~pFilter & TargetFilter_IgnoreLineOfSight) && !getCreature()->IsWithinLOSInMap(UnitTarget))
            return false;

        //Handle hostile/friendly
        if ((~pFilter & TargetFilter_Corpse) && (pFilter & TargetFilter_Friendly))
        {
            if (!UnitTarget->CombatStatus.IsInCombat())
                return false; //Skip not-in-combat targets if friendly
            if (isHostile(getCreature(), UnitTarget) || getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) > 0)
                return false;
        }
    }

    return true; //This is a valid unit target
};

void CreatureAIScript::PushRunToTargetCache(Unit* pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus)
{
    if (mRunToTargetCache != pTarget)
    {
        mRunToTargetCache = pTarget;
        mRunToTargetSpellCache = pSpell;
        setRooted(false);
        _setMeleeDisabled(true);
        _setRangedDisabled(true);
        _setCastDisabled(true);
    }

    if (mRunToTargetCache)
        MoveTo(mRunToTargetCache, pRangeStatus);
};

void CreatureAIScript::PopRunToTargetCache()
{
    if (mRunToTargetCache)
    {
        mRunToTargetCache = nullptr;
        mRunToTargetSpellCache = nullptr;
        _setMeleeDisabled(false);
        _setRangedDisabled(false);
        _setCastDisabled(false);
        stopMovement();
    }
};


//////////////////////////////////////////////////////////////////////////////////////////
//MoonScriptBossAI

SpellDesc* CreatureAIScript::AddPhaseSpell(uint32_t pPhase, SpellDesc* pSpell)
{
    mPhaseSpells.push_back(std::make_pair(pPhase, pSpell));
    return pSpell;
}

void CreatureAIScript::SetEnrageInfo(SpellDesc* pSpell, uint32_t pTriggerMilliseconds)
{
    mEnrageSpell = pSpell;
    mEnrageTimerDuration = pTriggerMilliseconds;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
TargetType::TargetType(uint32_t pTargetGen, TargetFilter pTargetFilter, uint32_t pMinTargetNumber, uint32_t pMaxTargetNumber)
{
    mTargetGenerator = pTargetGen;
    mTargetFilter = pTargetFilter;
    mTargetNumber[0] = pMinTargetNumber;    // Unused array for now
    mTargetNumber[1] = pMaxTargetNumber;
};

TargetType::~TargetType()
{
};

//////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
SpellDesc::SpellDesc(SpellInfo* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange, float pMaxRange,
    bool pStrictRange, std::string pText, uint8_t pTextType, uint32_t pSoundId, std::string pAnnouncement)
{
    mInfo = pInfo;
    mSpellFunc = pFnc;
    mTargetType = pTargetType;
    mChance = std::max(std::min(pChance, 100.0f), 0.0f);
    mCastTime = pCastTime;
    mCooldown = pCooldown;
    mMinRange = pMinRange;
    mMaxRange = pMaxRange;
    mStrictRange = pStrictRange;
    mEnabled = true;
    mPredefinedTarget = nullptr;
    mLastCastTime = 0;
    addAnnouncement(pAnnouncement);
    addEmote(pText, pTextType, pSoundId);
}

SpellDesc::~SpellDesc()
{
    mEmotes.clear();
}

void SpellDesc::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mEmotes.push_back(EmoteDesc(pText, pType, pSoundId));
}

void SpellDesc::sendRandomEmote(CreatureAIScript* creatureAI)
{
    if (!mEmotes.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "SpellDesc::SendRandomEmote() : called");

        uint32_t randomUInt = (mEmotes.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mEmotes.size() - 1)) : 0;
        creatureAI->getCreature()->SendChatMessage(mEmotes[randomUInt].mType, LANG_UNIVERSAL, mEmotes[randomUInt].mText.c_str());

        if (mEmotes[randomUInt].mSoundId != 0)
            creatureAI->getCreature()->PlaySoundToSet(mEmotes[randomUInt].mSoundId);
    }
}

void SpellDesc::setTriggerCooldown(uint32_t pCurrentTime)
{
    uint32_t CurrentTime = (pCurrentTime == 0) ? static_cast<uint32_t>(time(nullptr)) : pCurrentTime;
    mLastCastTime = CurrentTime;
}

void SpellDesc::addAnnouncement(std::string pText)
{
    mAnnouncement = (!pText.empty() ? pText : "");
}

//////////////////////////////////////////////////////////////////////////////////////////
//Premade Spell Functions
const uint32_t SPELLFUNC_VANISH = 24699;

void SpellFunc_ClearHateList(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    pCreatureAI->_clearHateList();
}

void SpellFunc_Disappear(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    pCreatureAI->_clearHateList();
    pCreatureAI->setRooted(true);
    pCreatureAI->setCanEnterCombat(false);
    pCreatureAI->_applyAura(SPELLFUNC_VANISH);
}

void SpellFunc_Reappear(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    pCreatureAI->setRooted(false);
    pCreatureAI->setCanEnterCombat(true);
    pCreatureAI->_removeAura(SPELLFUNC_VANISH);
}

void EventFunc_ApplyAura(CreatureAIScript* pCreatureAI, int32_t pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    pCreatureAI->_applyAura(uint32(pMiscVal));
    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}

void EventFunc_ChangeGoState(CreatureAIScript* pCreatureAI, int32_t pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    MapMgr* pInstance = pCreatureAI->getCreature()->GetMapMgr();
    if (!pInstance)
        return;

    GameObject* pSelectedGO = nullptr;
    uint32_t pGOEntry = static_cast<uint32_t>(pMiscVal);
    for (std::vector< GameObject* >::iterator GOIter = pInstance->GOStorage.begin(); GOIter != pInstance->GOStorage.end(); ++GOIter)
    {
        pSelectedGO = (*GOIter);
        if (pSelectedGO->GetEntry() == pGOEntry)
        {
            pSelectedGO->SetState(pSelectedGO->GetState() == 1 ? 0 : 1);
        }
    }

    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}

void EventFunc_RemoveUnitFieldFlags(CreatureAIScript* pCreatureAI, int32_t pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    pCreatureAI->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}


/* GameObjectAI Stuff */

GameObjectAIScript::GameObjectAIScript(GameObject* goinstance) : _gameobject(goinstance)
{

}

void GameObjectAIScript::ModifyAIUpdateEvent(uint32 newfrequency)
{
    sEventMgr.ModifyEventTimeAndTimeLeft(_gameobject, EVENT_SCRIPT_UPDATE_EVENT, newfrequency);
}

void GameObjectAIScript::RemoveAIUpdateEvent()
{
    sEventMgr.RemoveEvents(_gameobject, EVENT_SCRIPT_UPDATE_EVENT);
}

void GameObjectAIScript::RegisterAIUpdateEvent(uint32 frequency)
{
    sEventMgr.AddEvent(_gameobject, &GameObject::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, frequency, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

/* InstanceAI Stuff */

InstanceScript::InstanceScript(MapMgr* pMapMgr) : mInstance(pMapMgr), mSpawnsCreated(false), mTimerCount(0), mUpdateFrequency(defaultUpdateFrequency)
{
    generateBossDataState();
    registerUpdateEvent();
}

// MIT start
//////////////////////////////////////////////////////////////////////////////////////////
// data

void InstanceScript::addData(uint32_t data, uint32_t state /*= NotStarted*/)
{
    auto Iter = mInstanceData.find(data);
    if (Iter == mInstanceData.end())
        mInstanceData.insert(std::pair<uint32_t, uint32_t>(data, state));
    else
        LogDebugFlag(LF_SCRIPT_MGR, "InstanceScript::addData - tried to set state for entry %u. The entry is already available with a state!", data);
}

void InstanceScript::setData(uint32_t data, uint32_t state)
{
    auto Iter = mInstanceData.find(data);
    if (Iter != mInstanceData.end())
        Iter->second = state;
    else
        LogDebugFlag(LF_SCRIPT_MGR, "InstanceScript::setData - tried to set state for entry %u on map %u. The entry is not defined in table instance_bosses or manually to handle states!", data, mInstance->GetMapId());
}

uint32_t InstanceScript::getData(uint32_t data)
{
    auto Iter = mInstanceData.find(data);
    if (Iter != mInstanceData.end())
        return Iter->second;

    return InvalidState;
}

bool InstanceScript::isDataStateFinished(uint32_t data)
{
    return getData(data) == Finished;
}

//used for debug
std::string InstanceScript::getDataStateString(uint32_t bossEntry)
{
    uint32_t eState = NotStarted;

    auto it = mInstanceData.find(bossEntry);
    if (it != mInstanceData.end())
        eState = it->second;

    switch (eState)
    {
        case NotStarted:
            return "Not started";
        case InProgress:
            return "In Progress";
        case Finished:
            return "Finished";
        case Performed:
            return "Performed";
        case PreProgress:
            return "PreProgress";
        default:
            return "Invalid";
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// encounters
void InstanceScript::generateBossDataState()
{
    InstanceBossInfoMap* bossInfoMap = objmgr.m_InstanceBossInfoMap[mInstance->GetMapId()];
    if (bossInfoMap != nullptr)
    {
        for (const auto& encounter : *bossInfoMap)
        {
            CreatureProperties const* creature = sMySQLStore.getCreatureProperties(encounter.second->creatureid);
            if (creature == nullptr)
                LOG_ERROR("Your instance_boss table includes invalid data for boss entry %u!", encounter.second->creatureid);
            else
                mInstanceData.insert(std::pair<uint32_t, uint32_t>(encounter.second->creatureid, NotStarted));
        }

        for (const auto& killedNpc : mInstance->pInstance->m_killedNpcs)
        {
            InstanceBossInfoMap::const_iterator bossInfo = bossInfoMap->find((killedNpc));
            if (bossInfo != bossInfoMap->end())
                setData(bossInfo->first, Finished);
        }
    }

    LogDebugFlag(LF_SCRIPT_MGR, "InstanceScript::generateBossDataState() - Boss State generated for map %u.", mInstance->GetMapId());
}

void InstanceScript::sendUnitEncounter(uint32_t type, Unit* unit, uint8_t value_a, uint8_t value_b)
{
    WorldPacket data(SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT, 16);
    data << uint32(type);

    if (type == 0 || type == 1 || type == 2)        // engage, disengage, priority upd.
    {
        if (unit)
        {
            data << unit->GetNewGUID();
            data << uint8(value_a);
        }
    }
    else if (type == 3 || type == 4 || type == 6)   // add timer, objectives on, objectives off
    {
        data << uint8(value_a);
    }
    else if (type == 5)                             // objectives upd.
    {
        data << uint8(value_a);
        data << uint8(value_b);
    }

    // 7 sort encounters

    MapMgr* instance = GetInstance();
    instance->SendPacketToAllPlayers(&data);
}

void InstanceScript::displayDataStateList(Player* player)
{
    player->BroadcastMessage("=== DataState for instance %s ===", mInstance->GetMapInfo()->name.c_str());

    for (const auto& encounter : mInstanceData)
    {
        CreatureProperties const* creature = sMySQLStore.getCreatureProperties(encounter.first);
        if (creature != nullptr)
        {
            player->BroadcastMessage("  Boss '%s' (%u) - %s", creature->Name.c_str(), encounter.first, getDataStateString(encounter.first).c_str());
        }
        else
        {
            GameObjectProperties const* gameobject = sMySQLStore.getGameObjectProperties(encounter.first);
            if (gameobject != nullptr)
                player->BroadcastMessage("  Object '%s' (%u) - %s", gameobject->name.c_str(), encounter.first, getDataStateString(encounter.first).c_str());
            else
                player->BroadcastMessage("  MiscData %u - %s", encounter.first, getDataStateString(encounter.first).c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// timers

uint32_t InstanceScript::addTimer(uint32_t durationInMs)
{
    uint32_t timerId = ++mTimerCount;
    mTimers.push_back(std::make_pair(timerId, durationInMs));

    return timerId;
}

uint32_t InstanceScript::getTimeForTimer(uint32_t timerId)
{
    for (const auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            return intTimer.second;
    }

    return 0;
}

void InstanceScript::removeTimer(uint32_t& timerId)
{
    for (InstanceTimerArray::iterator intTimer = mTimers.begin(); intTimer != mTimers.end(); ++intTimer)
    {
        if (intTimer->first == timerId)
        {
            mTimers.erase(intTimer);
            timerId = 0;
            break;
        }
    }
}

void InstanceScript::resetTimer(uint32_t timerId, uint32_t durationInMs)
{
    for (auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            intTimer.second = durationInMs;
    }
}

bool InstanceScript::isTimerFinished(uint32_t timerId)
{
    for (const auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            return intTimer.second == 0;
    }

    return false;
}

void InstanceScript::cancelAllTimers()
{
    mTimers.clear();
    mTimerCount = 0;
}

void InstanceScript::updateTimers()
{
    for (auto& TimerIter : mTimers)
    {
        if (TimerIter.second > 0)
        {
            int leftTime = TimerIter.second - getUpdateFrequency();
            if (leftTime > 0)
                TimerIter.second -= getUpdateFrequency();
            else
                TimerIter.second = 0;
        }
    }
}

void InstanceScript::displayTimerList(Player* player)
{
    player->BroadcastMessage("=== Timers for instance %s ===", mInstance->GetMapInfo()->name.c_str());

    if (mTimers.empty())
    {
        player->BroadcastMessage("  No Timers available!");
    }
    else
    {
        for (const auto& intTimer : mTimers)
            player->BroadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance update

void InstanceScript::registerUpdateEvent()
{
    sEventMgr.AddEvent(mInstance, &MapMgr::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, getUpdateFrequency(), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void InstanceScript::modifyUpdateEvent(uint32_t frequencyInMs)
{
    if (getUpdateFrequency() != frequencyInMs)
    {
        setUpdateFrequency(frequencyInMs);
        sEventMgr.ModifyEventTimeAndTimeLeft(mInstance, EVENT_SCRIPT_UPDATE_EVENT, getUpdateFrequency());
    }
}

void InstanceScript::removeUpdateEvent()
{
    sEventMgr.RemoveEvents(mInstance, EVENT_SCRIPT_UPDATE_EVENT);
}

//////////////////////////////////////////////////////////////////////////////////////////
// misc

void InstanceScript::setCellForcedStates(float xMin, float xMax, float yMin, float yMax, bool forceActive /*true*/)
{
    if (xMin == xMax || yMin == yMax)
        return;

    float Y = yMin;
    while (xMin < xMax)
    {
        while (yMin < yMax)
        {
            MapCell* CurrentCell = mInstance->GetCellByCoords(xMin, yMin);
            if (forceActive && CurrentCell == nullptr)
            {
                CurrentCell = mInstance->CreateByCoords(xMin, yMin);
                if (CurrentCell != nullptr)
                    CurrentCell->Init(mInstance->GetPosX(xMin), mInstance->GetPosY(yMin), mInstance);
            }

            if (CurrentCell != nullptr)
            {
                if (forceActive)
                    mInstance->AddForcedCell(CurrentCell);
                else
                    mInstance->RemoveForcedCell(CurrentCell);
            }

            yMin += 40.0f;
        }

        yMin = Y;
        xMin += 40.0f;
    }
}

Creature* InstanceScript::spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry);
    if (creatureProperties == nullptr)
    {
        LOG_ERROR("tried to create a invalid creature with entry %u!", entry);
        return nullptr;
    }

    Creature* creature = mInstance->GetInterface()->SpawnCreature(entry, posX, posY, posZ, posO, true, true, 0, 0);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->SetFaction(factionId);
    else
        creature->SetFaction(creatureProperties->Faction);

    return creature;
}

Creature* InstanceScript::getCreatureBySpawnId(uint32_t entry)
{
    return mInstance->GetSqlIdCreature(entry);
}

Creature* InstanceScript::GetCreatureByGuid(uint32_t guid)
{
    return mInstance->GetCreature(guid);
}

CreatureSet InstanceScript::getCreatureSetForEntry(uint32_t entry, bool debug /*= false*/, Player* player /*= nullptr*/)
{
    CreatureSet creatureSet;
    uint32_t countCreatures = 0;
    for (auto creature : mInstance->CreatureStorage)
    {
        if (creature != nullptr)
        {

            if (creature->GetEntry() == entry)
            {
                creatureSet.insert(creature);
                ++countCreatures;
            }
        }
    }

    if (debug == true)
    {
        if (player != nullptr)
            player->BroadcastMessage("%u Creatures with entry %u found.", countCreatures, entry);
    }

    return creatureSet;
}

CreatureSet InstanceScript::getCreatureSetForEntries(std::vector<uint32_t> entryVector)
{
    CreatureSet creatureSet;
    for (auto creature : mInstance->CreatureStorage)
    {
        if (creature != nullptr)
        {
            for (auto entry : entryVector)
            {
                if (creature->GetEntry() == entry)
                    creatureSet.insert(creature);
            }
        }
    }

    return creatureSet;
}

GameObject* InstanceScript::spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld /*= true*/, uint32_t misc1 /*= 0*/, uint32_t phase /*= 0*/)
{
    GameObject* spawnedGameObject = mInstance->GetInterface()->SpawnGameObject(entry, posX, posY, posZ, posO, addToWorld, misc1, phase);
    return spawnedGameObject;
}

GameObject* InstanceScript::getGameObjectBySpawnId(uint32_t entry)
{
    return mInstance->GetSqlIdGameObject(entry);
}

GameObject* InstanceScript::GetGameObjectByGuid(uint32_t guid)
{
    return mInstance->GetGameObject(guid);
}

GameObject* InstanceScript::getClosestGameObjectForPosition(uint32_t entry, float posX, float posY, float posZ)
{
    GameObjectSet gameObjectSet = getGameObjectsSetForEntry(entry);

    if (gameObjectSet.size() == 0)
        return nullptr;

    if (gameObjectSet.size() == 1)
        return *(gameObjectSet.begin());

    float distance = 99999;
    float nearestDistance = 99999;

    for (auto gameobject : gameObjectSet)
    {
        if (gameobject != nullptr)
        {
            distance = getRangeToObjectForPosition(gameobject, posX, posY, posZ);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                return gameobject;
            }
        }
    }

    return nullptr;
}

GameObjectSet InstanceScript::getGameObjectsSetForEntry(uint32_t entry)
{
    GameObjectSet gameobjectSet;
    for (auto gameobject : mInstance->GOStorage)
    {
        if (gameobject != nullptr)
        {
            if (gameobject->GetEntry() == entry)
                gameobjectSet.insert(gameobject);
        }
    }

    return gameobjectSet;
}

float InstanceScript::getRangeToObjectForPosition(Object* object, float posX, float posY, float posZ)
{
    if (object == nullptr)
        return 0.0f;

    LocationVector pos = object->GetPosition();
    float dX = pos.x - posX;
    float dY = pos.y - posY;
    float dZ = pos.z - posZ;

    return sqrtf(dX * dX + dY * dY + dZ * dZ);
}

void InstanceScript::setGameObjectStateForEntry(uint32_t entry, uint8_t state)
{
    if (entry == 0)
        return;

    GameObjectSet gameObjectSet = getGameObjectsSetForEntry(entry);

    if (gameObjectSet.size() == 0)
        return;

    for (auto gameobject : gameObjectSet)
    {
        if (gameobject != nullptr)
            gameobject->SetState(state);
    }
}

//MIT end

/* Hook Stuff */
void ScriptMgr::register_hook(ServerHookEvents event, void* function_pointer)
{
    ARCEMU_ASSERT(event < NUM_SERVER_HOOKS);
    _hooks[event].insert(function_pointer);
}

bool ScriptMgr::has_creature_script(uint32 entry) const
{
    return (_creatures.find(entry) != _creatures.end());
}

bool ScriptMgr::has_gameobject_script(uint32 entry) const
{
    return (_gameobjects.find(entry) != _gameobjects.end());
}

bool ScriptMgr::has_dummy_aura_script(uint32 entry) const
{
    return (_auras.find(entry) != _auras.end());
}

bool ScriptMgr::has_dummy_spell_script(uint32 entry) const
{
    return (_spells.find(entry) != _spells.end());
}

bool ScriptMgr::has_script_effect(uint32 entry) const
{
    return (SpellScriptEffects.find(entry) != SpellScriptEffects.end());
}

bool ScriptMgr::has_instance_script(uint32 id) const
{
    return (mInstances.find(id) != mInstances.end());
}

bool ScriptMgr::has_hook(ServerHookEvents evt, void* ptr) const
{
    return (_hooks[evt].size() != 0 && _hooks[evt].find(ptr) != _hooks[evt].end());
}

bool ScriptMgr::has_quest_script(uint32 entry) const
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(entry);
    return (q == NULL || q->pQuestScript != NULL);
}

void ScriptMgr::register_creature_gossip(uint32 entry, Arcemu::Gossip::Script* script)
{
    GossipMap::iterator itr = creaturegossip_.find(entry);
    if (itr == creaturegossip_.end())
        creaturegossip_.insert(std::make_pair(entry, script));
    //keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

bool ScriptMgr::has_creature_gossip(uint32 entry) const
{
    return creaturegossip_.find(entry) != creaturegossip_.end();
}

Arcemu::Gossip::Script* ScriptMgr::get_creature_gossip(uint32 entry) const
{
    GossipMap::const_iterator itr = creaturegossip_.find(entry);
    if (itr != creaturegossip_.end())
        return itr->second;
    return NULL;
}

void ScriptMgr::register_item_gossip(uint32 entry, Arcemu::Gossip::Script* script)
{
    GossipMap::iterator itr = itemgossip_.find(entry);
    if (itr == itemgossip_.end())
        itemgossip_.insert(std::make_pair(entry, script));
    //keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

void ScriptMgr::register_go_gossip(uint32 entry, Arcemu::Gossip::Script* script)
{
    GossipMap::iterator itr = gogossip_.find(entry);
    if (itr == gogossip_.end())
        gogossip_.insert(std::make_pair(entry, script));
    //keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

bool ScriptMgr::has_item_gossip(uint32 entry) const
{
    return itemgossip_.find(entry) != itemgossip_.end();
}

bool ScriptMgr::has_go_gossip(uint32 entry) const
{
    return gogossip_.find(entry) != gogossip_.end();
}

Arcemu::Gossip::Script* ScriptMgr::get_go_gossip(uint32 entry) const
{
    GossipMap::const_iterator itr = gogossip_.find(entry);
    if (itr != gogossip_.end())
        return itr->second;
    return NULL;
}

Arcemu::Gossip::Script* ScriptMgr::get_item_gossip(uint32 entry) const
{
    GossipMap::const_iterator itr = itemgossip_.find(entry);
    if (itr != itemgossip_.end())
        return itr->second;
    return NULL;
}

void ScriptMgr::ReloadScriptEngines()
{
    //for all scripting engines that allow reloading, assuming there will be new scripting engines.
    exp_get_script_type version_function;
    exp_engine_reload engine_reloadfunc;

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
    {
        Arcemu::DynLib* dl = *itr;

        version_function = reinterpret_cast<exp_get_script_type>(dl->GetAddressForSymbol("_exp_get_script_type"));
        if (version_function == NULL)
            continue;

        if ((version_function() & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            engine_reloadfunc = reinterpret_cast<exp_engine_reload>(dl->GetAddressForSymbol("_export_engine_reload"));
            if (engine_reloadfunc != NULL)
                engine_reloadfunc();
        }
    }
}

void ScriptMgr::UnloadScriptEngines()
{
    //for all scripting engines that allow unloading, assuming there will be new scripting engines.
    exp_get_script_type version_function;
    exp_engine_unload engine_unloadfunc;

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
    {
        Arcemu::DynLib* dl = *itr;

        version_function = reinterpret_cast<exp_get_script_type>(dl->GetAddressForSymbol("_exp_get_script_type"));
        if (version_function == NULL)
            continue;

        if ((version_function() & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            engine_unloadfunc = reinterpret_cast<exp_engine_unload>(dl->GetAddressForSymbol("_exp_engine_unload"));
            if (engine_unloadfunc != NULL)
                engine_unloadfunc();
        }
    }
}

/* Hook Implementations */
bool HookInterface::OnNewCharacter(uint32 Race, uint32 Class, WorldSession* Session, const char* Name)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_NEW_CHARACTER];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnNewCharacter)* itr)(Race, Class, Session, Name);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnKillPlayer(Player* pPlayer, Player* pVictim)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_KILL_PLAYER];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnKillPlayer)*itr)(pPlayer, pVictim);
}

void HookInterface::OnFirstEnterWorld(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnFirstEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnCharacterCreate(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CHARACTER_CREATE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOCharacterCreate)*itr)(pPlayer);
}

void HookInterface::OnEnterWorld(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ENTER_WORLD];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnGuildCreate(Player* pLeader, Guild* pGuild)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_GUILD_CREATE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnGuildCreate)*itr)(pLeader, pGuild);
}

void HookInterface::OnGuildJoin(Player* pPlayer, Guild* pGuild)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_GUILD_JOIN];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnGuildJoin)*itr)(pPlayer, pGuild);
}

void HookInterface::OnDeath(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_DEATH];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnDeath)*itr)(pPlayer);
}

bool HookInterface::OnRepop(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_REPOP];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnRepop)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_EMOTE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEmote)*itr)(pPlayer, Emote, pUnit);
}

void HookInterface::OnEnterCombat(Player* pPlayer, Unit* pTarget)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ENTER_COMBAT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterCombat)*itr)(pPlayer, pTarget);
}

bool HookInterface::OnCastSpell(Player* pPlayer, SpellInfo* pSpell, Spell* spell)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CAST_SPELL];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnCastSpell)* itr)(pPlayer, pSpell, spell);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

bool HookInterface::OnLogoutRequest(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnLogoutRequest)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnLogout(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOGOUT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnLogout)*itr)(pPlayer);
}

void HookInterface::OnQuestAccept(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_ACCEPT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestAccept)*itr)(pPlayer, pQuest, pQuestGiver);
}

void HookInterface::OnZone(Player* pPlayer, uint32 zone, uint32 oldZone)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ZONE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnZone)*itr)(pPlayer, zone, oldZone);
}

bool HookInterface::OnChat(Player* pPlayer, uint32 type, uint32 lang, const char* message, const char* misc)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_CHAT];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnChat)* itr)(pPlayer, type, lang, message, misc);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}

void HookInterface::OnLoot(Player* pPlayer, Unit* pTarget, uint32 money, uint32 itemId)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_LOOT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnLoot)*itr)(pPlayer, pTarget, money, itemId);
}

void HookInterface::OnObjectLoot(Player* pPlayer, Object* pTarget, uint32 money, uint32 itemId)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_OBJECTLOOT];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnObjectLoot)*itr)(pPlayer, pTarget, money, itemId);
}

void HookInterface::OnFullLogin(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_FULL_LOGIN];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnEnterWorld)*itr)(pPlayer);
}

void HookInterface::OnQuestCancelled(Player* pPlayer, QuestProperties const* pQuest)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_CANCELLED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestCancel)*itr)(pPlayer, pQuest);
}

void HookInterface::OnQuestFinished(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_QUEST_FINISHED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnQuestFinished)*itr)(pPlayer, pQuest, pQuestGiver);
}

void HookInterface::OnHonorableKill(Player* pPlayer, Player* pKilled)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_HONORABLE_KILL];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnHonorableKill)*itr)(pPlayer, pKilled);
}

void HookInterface::OnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ARENA_FINISH];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnArenaFinish)*itr)(pPlayer, pTeam, victory, rated);
}

void HookInterface::OnAreaTrigger(Player* pPlayer, uint32 areaTrigger)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_AREATRIGGER];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAreaTrigger)*itr)(pPlayer, areaTrigger);
}

void HookInterface::OnPostLevelUp(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_POST_LEVELUP];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnPostLevelUp)*itr)(pPlayer);
}

bool HookInterface::OnPreUnitDie(Unit* killer, Unit* victim)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_PRE_DIE];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnPreUnitDie)* itr)(killer, victim);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}


void HookInterface::OnAdvanceSkillLine(Player* pPlayer, uint32 skillLine, uint32 current)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAdvanceSkillLine)*itr)(pPlayer, skillLine, current);
}

void HookInterface::OnDuelFinished(Player* Winner, Player* Looser)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_DUEL_FINISHED];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnDuelFinished)*itr)(Winner, Looser);
}

void HookInterface::OnAuraRemove(Aura* aura)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_AURA_REMOVE];
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
        ((tOnAuraRemove)*itr)(aura);
}

bool HookInterface::OnResurrect(Player* pPlayer)
{
    ServerHookList hookList = sScriptMgr._hooks[SERVER_HOOK_EVENT_ON_RESURRECT];
    bool ret_val = true;
    for (ServerHookList::iterator itr = hookList.begin(); itr != hookList.end(); ++itr)
    {
        bool rv = ((tOnResurrect)* itr)(pPlayer);
        if (rv == false)  // never set ret_val back to true, once it's false
            ret_val = false;
    }
    return ret_val;
}
