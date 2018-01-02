/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

bool ScriptMgr::CallScriptedDummySpell(uint32 uSpellId, uint8_t effectIndex, Spell* pSpell)
{
    HandleDummySpellMap::iterator itr = _spells.find(uSpellId);
    if (itr == _spells.end())
        return false;

    exp_handle_dummy_spell function_ptr = itr->second;
    return (function_ptr)(effectIndex, pSpell);
}

bool ScriptMgr::HandleScriptedSpellEffect(uint32 SpellId, uint8_t effectIndex, Spell* s)
{
    HandleScriptEffectMap::iterator itr = SpellScriptEffects.find(SpellId);
    if (itr == SpellScriptEffects.end())
        return false;

    exp_handle_script_effect ptr = itr->second;
    return (ptr)(effectIndex, s);
}

bool ScriptMgr::CallScriptedDummyAura(uint32 uSpellId, uint8_t effectIndex, Aura* pAura, bool apply)
{
    HandleDummyAuraMap::iterator itr = _auras.find(uSpellId);
    if (itr == _auras.end())
        return false;

    exp_handle_dummy_aura function_ptr = itr->second;
    return (function_ptr)(effectIndex, pAura, apply);
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


//////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
TargetType::TargetType(uint32_t pTargetGen, TargetFilter pTargetFilter, uint32_t pMinTargetNumber, uint32_t pMaxTargetNumber)
{
    mTargetGenerator = pTargetGen;
    mTargetFilter = pTargetFilter;
    mTargetNumber[0] = pMinTargetNumber;    // Unused array for now
    mTargetNumber[1] = pMaxTargetNumber;
}

TargetType::~TargetType()
{
}

/* GameObjectAI Stuff */

GameObjectAIScript::GameObjectAIScript(GameObject* goinstance) : _gameobject(goinstance)
{
}

void GameObjectAIScript::ModifyAIUpdateEvent(uint32_t newfrequency)
{
    sEventMgr.ModifyEventTimeAndTimeLeft(_gameobject, EVENT_SCRIPT_UPDATE_EVENT, newfrequency);
}

void GameObjectAIScript::RemoveAIUpdateEvent()
{
    sEventMgr.RemoveEvents(_gameobject, EVENT_SCRIPT_UPDATE_EVENT);
}

void GameObjectAIScript::RegisterAIUpdateEvent(uint32_t frequency)
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
