/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
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

#include <fstream>

#include <git_version.h>
#include "ScriptMgr.h"
#include "Common.hpp"
#include "CreatureAIScript.h"
#include "WorldConf.h"
#include "Management/GameEvent.h"
#include "Management/ObjectMgr.h"
#include "Management/LFG/LFGMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Item.hpp"
#include "Server/MainServerDefines.h"
#include "Server/Packets/SmsgUpdateInstanceEncounterUnit.h"
#include "Spell/SpellAuras.h"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/DBC/DBCStores.h"

using namespace AscEmu::Packets;

// APGL End
// MIT Start

ScriptMgr& ScriptMgr::getInstance()
{
    static ScriptMgr mInstance;
    return mInstance;
}

#ifdef FT_ACHIEVEMENTS
bool ScriptMgr::callScriptedAchievementCriteriaCanComplete(uint32_t criteriaId, Player* player, Object* target) const
{
    const auto achievementCriteriaScript = getAchievementCriteriaScript(criteriaId);
    if (achievementCriteriaScript == nullptr)
        return true;

    return achievementCriteriaScript->canCompleteCriteria(criteriaId, player, target);
}
#endif

SpellCastResult ScriptMgr::callScriptedSpellCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2) const
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return SPELL_CAST_SUCCESS;

    return spellScript->onCanCast(spell, parameter1, parameter2);
}

void ScriptMgr::callScriptedSpellAtStartCasting(Spell* spell)
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->doAtStartCasting(spell);
}

void ScriptMgr::callScriptedSpellFilterTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->filterEffectTargets(spell, effectIndex, effectTargets);
}

void ScriptMgr::callScriptedSpellBeforeHit(Spell* spell, uint8_t effectIndex)
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->doBeforeEffectHit(spell, effectIndex);
}

void ScriptMgr::callScriptedSpellAfterMiss(Spell* spell, Unit* unitTarget)
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->doAfterSpellMissed(spell, unitTarget);
}

SpellScriptEffectDamage ScriptMgr::callScriptedSpellDoCalculateEffect(Spell* spell, uint8_t effectIndex, int32_t* damage) const
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;

    return spellScript->doCalculateEffect(spell, effectIndex, damage);
}

SpellScriptExecuteState ScriptMgr::callScriptedSpellBeforeSpellEffect(Spell* spell, uint8_t effectIndex) const
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    return spellScript->beforeSpellEffect(spell, effectIndex);
}

SpellScriptCheckDummy ScriptMgr::callScriptedSpellOnDummyOrScriptedEffect(Spell* spell, uint8_t effectIndex) const
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

    return spellScript->onDummyOrScriptedEffect(spell, effectIndex);
}

void ScriptMgr::callScriptedSpellAfterSpellEffect(Spell* spell, uint8_t effectIndex)
{
    const auto spellScript = getSpellScript(spell->getSpellInfo()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->afterSpellEffect(spell, effectIndex);
}

void ScriptMgr::callScriptedAuraOnCreate(Aura* aur)
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return;

    spellScript->onAuraCreate(aur);
}

void ScriptMgr::callScriptedAuraOnApply(Aura* aur)
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return;

    spellScript->onAuraApply(aur);
}

void ScriptMgr::callScriptedAuraOnRemove(Aura* aur, AuraRemoveMode mode)
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return;

    spellScript->onAuraRemove(aur, mode);
}

void ScriptMgr::callScriptedAuraOnRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount)
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return;

    spellScript->onAuraRefreshOrGainNewStack(aur, newStackCount, oldStackCount);
}

SpellScriptExecuteState ScriptMgr::callScriptedAuraBeforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    return spellScript->beforeAuraEffect(aur, aurEff, apply);
}

SpellScriptCheckDummy ScriptMgr::callScriptedAuraOnDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

    return spellScript->onAuraDummyEffect(aur, aurEff, apply);
}

SpellScriptExecuteState ScriptMgr::callScriptedAuraOnPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage) const
{
    const auto spellScript = getSpellScript(aur->getSpellId());
    if (spellScript == nullptr)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    return spellScript->onAuraPeriodicTick(aur, aurEff, damage);
}

void ScriptMgr::callScriptedSpellProcCreate(SpellProc* spellProc, Object* obj)
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return;

    spellScript->onCreateSpellProc(spellProc, obj);
}

bool ScriptMgr::callScriptedSpellCanProc(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return true;

    return spellScript->canProc(spellProc, victim, castingSpell, damageInfo);
}

bool ScriptMgr::callScriptedSpellCheckProcFlags(SpellProc* spellProc, SpellProcFlags procFlags) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return spellProc->getProcFlags() & procFlags;

    return spellScript->onCheckProcFlags(spellProc, procFlags);
}

bool ScriptMgr::callScriptedSpellProcCanDelete(SpellProc* spellProc, uint32_t spellId, uint64_t casterGuid, uint64_t misc) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
    {
        if (spellProc->getSpell()->getId() == spellId && (casterGuid == 0 || spellProc->getCasterGuid() == casterGuid) && !spellProc->isDeleted())
            return true;

        return false;
    }

    return spellScript->canDeleteProc(spellProc, spellId, casterGuid, misc);
}

SpellScriptExecuteState ScriptMgr::callScriptedSpellProcDoEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    return spellScript->onDoProcEffect(spellProc, victim, castingSpell, damageInfo);
}

uint32_t ScriptMgr::callScriptedSpellCalcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return spellProc->calcProcChance(victim, castingSpell);

    return spellScript->calcProcChance(spellProc, victim, castingSpell);
}

bool ScriptMgr::callScriptedSpellCanProcOnTriggered(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, Aura* triggeredFromAura) const
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
    {
        if (spellProc->getOriginalSpell() != nullptr && spellProc->getOriginalSpell()->getAttributesExC() & ATTRIBUTESEXC_CAN_PROC_ON_TRIGGERED)
            return true;

        return false;
    }

    return spellScript->canProcOnTriggered(spellProc, victim, castingSpell, triggeredFromAura);
}

SpellScriptExecuteState ScriptMgr::callScriptedSpellProcCastSpell(SpellProc* spellProc, Unit* caster, Unit* victim, Spell* spellToProc)
{
    const auto spellScript = getSpellScript(spellProc->getSpell()->getId());
    if (spellScript == nullptr)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    return spellScript->onCastProcSpell(spellProc, caster, victim, spellToProc);
}

#ifdef FT_ACHIEVEMENTS
AchievementCriteriaScript* ScriptMgr::getAchievementCriteriaScript(uint32_t criteriaId) const
{
    for (const auto& itr : _achievementCriteriaScripts)
    {
        if (itr.first == criteriaId)
            return itr.second;
    }

    return nullptr;
}

void ScriptMgr::register_achievement_criteria_script(uint32_t criteriaId, AchievementCriteriaScript* acs)
{
    const auto criteriaEntry = sAchievementCriteriaStore.LookupEntry(criteriaId);
    if (criteriaEntry == nullptr)
    {
        sLogger.failure("ScriptMgr tried to register a script for achievement criteria id %u but criteria does not exist!", criteriaId);
        return;
    }

    if (_achievementCriteriaScripts.find(criteriaId) != _achievementCriteriaScripts.end())
    {
        sLogger.debug("ScriptMgr tried to register a script for achievement criteria id %u but this criteria has already one.", criteriaId);
        return;
    }

    _achievementCriteriaScripts[criteriaId] = acs;
}

void ScriptMgr::register_achievement_criteria_script(uint32_t* criteriaIds, AchievementCriteriaScript* acs)
{
    for (uint32_t i = 0; criteriaIds[i] != 0; ++i)
    {
        register_achievement_criteria_script(criteriaIds[i], acs);
    }
}
#endif

SpellScript* ScriptMgr::getSpellScript(uint32_t spellId) const
{
    for (const auto& itr : _spellScripts)
    {
        if (itr.first == spellId)
            return itr.second;
    }

    return nullptr;
}

void ScriptMgr::register_spell_script(uint32_t spellId, SpellScript* ss, bool registerAllDifficulties/* = true*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        sLogger.failure("ScriptMgr tried to register a script for spell id %u but spell does not exist!", spellId);
        return;
    }

    if (registerAllDifficulties)
    {
        if (spellInfo->getSpellDifficultyID() != 0)
        {
            uint8_t registeredSpells = 0;
            for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
            {
                const auto spellDifficultyInfo = sSpellMgr.getSpellInfoByDifficulty(spellInfo->getSpellDifficultyID(), i);
                if (spellDifficultyInfo == nullptr)
                    continue;

                _register_spell_script(spellDifficultyInfo->getId(), ss);
                ++registeredSpells;
            }

            // Make sure to register at least the original spell
            if (registeredSpells > 0)
                return;
        }
    }

    _register_spell_script(spellId, ss);
}

void ScriptMgr::register_spell_script(uint32_t* spellIds, SpellScript* ss)
{
    for (uint32_t i = 0; spellIds[i] != 0; ++i)
    {
        register_spell_script(spellIds[i], ss);
    }
}

void ScriptMgr::_register_spell_script(uint32_t spellId, SpellScript* ss)
{
    if (_spellScripts.find(spellId) != _spellScripts.end())
    {
        sLogger.debug("ScriptMgr tried to register a script for spell id %u but this spell has already one.", spellId);
        return;
    }

    _spellScripts[spellId] = ss;
}

// MIT End
// APGL Start

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
    sLogger.info("ScriptMgr : Loading External Script Libraries...");

    std::string modulePath = PREFIX;
    modulePath += '/';

    std::string libMask = LIBMASK;

    std::vector<ScriptingEngine_dl> scriptingEngineDls;

    uint32_t dllCount = 0;

    auto directoryContentMap = Util::getDirectoryContent(modulePath, libMask);
    for (const auto content : directoryContentMap)
    {
        std::stringstream loadMessageStream;
        auto fileName = modulePath + content.second;
        auto dynLib = new Arcemu::DynLib(fileName.c_str());

        loadMessageStream << dynLib->GetName() << " : ";

        if (!dynLib->Load())
        {
            loadMessageStream << "ERROR: Cannot open library.";
            sLogger.failure(loadMessageStream.str().c_str());
            delete dynLib;
            continue;
        }

        auto serverStateCall = reinterpret_cast<exp_set_serverstate_singleton>(dynLib->GetAddressForSymbol("_exp_set_serverstate_singleton"));
        if (!serverStateCall)
        {
            loadMessageStream << "ERROR: Cannot find set_serverstate_call function.";
            sLogger.failure(loadMessageStream.str().c_str());
            delete dynLib;
            continue;
        }

        serverStateCall(ServerState::instance());

        auto versionCall = reinterpret_cast<exp_get_version>(dynLib->GetAddressForSymbol("_exp_get_version"));
        auto registerCall = reinterpret_cast<exp_script_register>(dynLib->GetAddressForSymbol("_exp_script_register"));
        auto typeCall = reinterpret_cast<exp_get_script_type>(dynLib->GetAddressForSymbol("_exp_get_script_type"));
        if (!versionCall || !registerCall || !typeCall)
        {
            loadMessageStream << "ERROR: Cannot find version functions.";
            sLogger.failure(loadMessageStream.str().c_str());
            delete dynLib;
            continue;
        }

        std::string dllVersion = versionCall();
        uint32_t scriptType = typeCall();

        if (dllVersion != BUILD_HASH_STR)
        {
            loadMessageStream << "ERROR: Version mismatch.";
            sLogger.failure(loadMessageStream.str().c_str());
            delete dynLib;
            continue;
        }

        loadMessageStream << std::string(BUILD_HASH_STR) << " : ";

        if ((scriptType & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            ScriptingEngine_dl scriptingEngineDl;
            scriptingEngineDl.dl = dynLib;
            scriptingEngineDl.InitializeCall = registerCall;
            scriptingEngineDl.Type = scriptType;

            scriptingEngineDls.push_back(scriptingEngineDl);

            loadMessageStream << "delayed load";
        }
        else
        {
            registerCall(this);
            dynamiclibs.push_back(dynLib);

            loadMessageStream << "loaded";
        }
        sLogger.info(loadMessageStream.str().c_str());

        dllCount++;
    }

    if (dllCount == 0)
    {
        sLogger.failure("No external scripts found! Server will continue to function with limited functionality.");
    }
    else
    {
        sLogger.info("ScriptMgr : Loaded %u external libraries.", dllCount);
        sLogger.info("ScriptMgr : Loading optional scripting engine(s)...");

        for (auto& engineDl : scriptingEngineDls)
        {
            engineDl.InitializeCall(this);
            dynamiclibs.push_back(engineDl.dl);
        }

        sLogger.info("ScriptMgr : Done loading scripting engine(s)...");
    }
}

void ScriptMgr::UnloadScripts()
{
    for (CustomGossipScripts::iterator itr = _customgossipscripts.begin(); itr != _customgossipscripts.end(); ++itr)
        (*itr)->destroy();
    _customgossipscripts.clear();

    for (QuestScripts::iterator itr = _questscripts.begin(); itr != _questscripts.end(); ++itr)
        delete *itr;
    _questscripts.clear();

#ifdef FT_ACHIEVEMENTS
    for (auto itr = _achievementCriteriaScripts.begin(); itr != _achievementCriteriaScripts.end();)
        itr = _achievementCriteriaScripts.erase(itr);
#endif

    for (auto itr = _spellScripts.begin(); itr != _spellScripts.end();)
        itr = _spellScripts.erase(itr);

    UnloadScriptEngines();

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
        delete *itr;

    dynamiclibs.clear();
}

void ScriptMgr::DumpUnimplementedSpells()
{
    std::ofstream of;

    sLogger.info("Dumping IDs for spells with unimplemented dummy/script effect(s)");
    uint32 count = 0;

    of.open("unimplemented1.txt");

    for (auto it = sSpellMgr.getSpellInfoMap()->begin(); it != sSpellMgr.getSpellInfoMap()->end(); ++it)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo(it->first);
        if (!sp)
            continue;

        if (!sp->hasEffect(SPELL_EFFECT_DUMMY) && !sp->hasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->hasEffect(SPELL_EFFECT_SEND_EVENT))
            continue;

        HandleDummySpellMap::iterator sitr = _spells.find(sp->getId());
        if (sitr != _spells.end())
            continue;

        HandleScriptEffectMap::iterator seitr = SpellScriptEffects.find(sp->getId());
        if (seitr != SpellScriptEffects.end())
            continue;

        std::stringstream ss;
        ss << sp->getId();
        ss << "\n";

        of.write(ss.str().c_str(), ss.str().length());

        count++;
    }

    of.close();

    sLogger.info("Dumped %u IDs.", count);

    sLogger.info("Dumping IDs for spells with unimplemented dummy aura effect.");

    std::ofstream of2;
    of2.open("unimplemented2.txt");

    count = 0;

    for (auto it = sSpellMgr.getSpellInfoMap()->begin(); it != sSpellMgr.getSpellInfoMap()->end(); ++it)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo(it->first);
        if (!sp)
            continue;

        if (!sp->appliesAreaAura(SPELL_AURA_DUMMY))
            continue;

        HandleDummyAuraMap::iterator ditr = _auras.find(sp->getId());
        if (ditr != _auras.end())
            continue;

        std::stringstream ss;
        ss << sp->getId();
        ss << "\n";

        of2.write(ss.str().c_str(), ss.str().length());

        count++;
    }

    of2.close();

    sLogger.info("Dumped %u IDs.", count);
}

void ScriptMgr::DamageTaken(Creature* pCreature, Unit* attacker, uint32_t* damage) const
{
    const auto AIScript = pCreature->GetScript();
    if (AIScript == nullptr)
        return;

    return AIScript->DamageTaken(attacker, damage);
}

CreatureAIScript* ScriptMgr::getCreatureAIScript(Creature* pCreature) const
{
    for (const auto& itr : _creatures)
    {
        if (itr.first == pCreature->getEntry())
        {
            exp_create_creature_ai function_ptr = itr.second;
            return (function_ptr)(pCreature);
        }
    }

    return nullptr;
}

void ScriptMgr::register_creature_script(uint32 entry, exp_create_creature_ai callback)
{
    m_creaturesMutex.Acquire();

    if (_creatures.find(entry) != _creatures.end())
        sLogger.debug("ScriptMgr tried to register a script for Creature ID: %u but this creature has already one!", entry);

    _creatures.insert(CreatureCreateMap::value_type(entry, callback));

    m_creaturesMutex.Release();
}

void ScriptMgr::register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback)
{
    if (_gameobjects.find(entry) != _gameobjects.end())
        sLogger.debug("ScriptMgr tried to register a script for GameObject ID: %u but this go has already one.", entry);

    _gameobjects.insert(GameObjectCreateMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback)
{
    if (_auras.find(entry) != _auras.end())
    {
        sLogger.debug("ScriptMgr tried to register a script for Aura ID: %u but this aura has already one.", entry);
    }

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == NULL)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a dummy aura handler for invalid Spell ID: %u.", entry);
        return;
    }

#if VERSION_STRING >= TBC
    if (!sp->hasEffectApplyAuraName(SPELL_AURA_DUMMY) && !sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_DUMMY))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr registered a dummy aura handler for Spell ID: %u (%s), but spell has no dummy aura!", entry, sp->getName().c_str());
#endif

    _auras.insert(HandleDummyAuraMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback)
{
    if (_spells.find(entry) != _spells.end())
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a script for Spell ID: %u but this spell has already one", entry);
        return;
    }

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == NULL)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a dummy handler for invalid Spell ID: %u.", entry);
        return;
    }

    if (!sp->hasEffect(SPELL_EFFECT_DUMMY) && !sp->hasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->hasEffect(SPELL_EFFECT_SEND_EVENT))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr registered a dummy handler for Spell ID: %u (%s), but spell has no dummy/script/send event effect!", entry, sp->getName().c_str());

    _spells.insert(HandleDummySpellMap::value_type(entry, callback));
}

void ScriptMgr::register_quest_script(uint32 entry, QuestScript* qs)
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(entry);
    if (q != nullptr)
    {
        if (q->pQuestScript != NULL)
            sLogger.debug("ScriptMgr tried to register a script for Quest ID: %u but this quest has already one.", entry);

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
            sLogger.debug("ScriptMgr tried to register a script for Event ID: %u but this event has already one.", entry);
            return;
        }

        gameEvent->mEventScript = es;
        _eventscripts.insert(es);
    }
}

void ScriptMgr::register_instance_script(uint32 pMapId, exp_create_instance_ai pCallback)
{
    if (mInstances.find(pMapId) != mInstances.end())
        sLogger.debug("ScriptMgr tried to register a script for Instance ID: %u but this instance already has one.", pMapId);

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
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr tried to register more than 1 script effect handlers for Spell %u", entry);
        return;
    }

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == NULL)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr tried to register a script effect handler for invalid Spell %u.", entry);
        return;
    }

    if (!sp->hasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->hasEffect(SPELL_EFFECT_SEND_EVENT))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr registered a script effect handler for Spell ID: %u (%s), but spell has no scripted effect!", entry, sp->getName().c_str());

    SpellScriptEffects.insert(std::pair< uint32, exp_handle_script_effect >(entry, callback));
}

CreatureAIScript* ScriptMgr::CreateAIScriptClassForEntry(Creature* pCreature)
{
    uint32 entry = pCreature->getEntry();

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
    GameObjectCreateMap::iterator itr = _gameobjects.find(pGameObject->getEntry());
    if (itr == _gameobjects.end())
        return NULL;

    exp_create_gameobject_ai function_ptr = itr->second;
    return (function_ptr)(pGameObject);
}

InstanceScript* ScriptMgr::CreateScriptClassForInstance(uint32 /*pMapId*/, WorldMap* pMapMgr)
{
    InstanceCreateMap::iterator Iter = mInstances.find(pMapMgr->getBaseMap()->getMapId());
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
    auto script = this->get_item_gossip(pItem->getEntry());
    if (script)
    {
        script->onHello(pItem, pPlayer);
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

//////////////////////////////////////////////////////////////////////////////////////////
// instance
InstanceScript* GameObjectAIScript::getInstanceScript()
{
    WorldMap* mapMgr = _gameobject->getWorldMap();
    return (mapMgr) ? mapMgr->getScript() : nullptr;
}

bool GameObjectAIScript::_isHeroic()
{
    WorldMap* mapMgr = _gameobject->getWorldMap();
    if (mapMgr == nullptr || mapMgr->getDifficulty() != InstanceDifficulty::DUNGEON_HEROIC)
        return false;

    return true;
}

/* InstanceAI Stuff */

InstanceScript::InstanceScript(WorldMap* pMapMgr) : mInstance(pMapMgr), mSpawnsCreated(false), mTimerCount(0), mUpdateFrequency(defaultUpdateFrequency)
{
    Difficulty = pMapMgr->getDifficulty();
    registerUpdateEvent();
}

// MIT start
//////////////////////////////////////////////////////////////////////////////////////////
// data

//used for debug
std::string InstanceScript::getDataStateString(uint8_t state)
{
    switch (state)
    {
        case NotStarted:
            return "Not started";
        case InProgress:
            return "In Progress";
        case Failed:
            return "Failed";
        case Performed:
            return "Performed";
        case PreProgress:
            return "PreProgress";
        default:
            return "Invalid";
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance Script Data Fast Access
void InstanceScript::setupInstanceData(ObjectData const* creatureData, ObjectData const* gameObjectData)
{
    if (creatureData)
        setupObjectData(creatureData, _creatureInfo);

    if (gameObjectData)
        setupObjectData(gameObjectData, _gameObjectInfo);
}

void InstanceScript::setupObjectData(ObjectData const* data, ObjectInfoMap& objectInfo)
{
    while (data->entry)
    {
        ASSERT(objectInfo.find(data->entry) == objectInfo.end());
        objectInfo[data->entry] = data->type;
        ++data;
    }
}
void InstanceScript::addObject(Object* obj)
{
    WoWGuid guid = obj->getGuid();

    if (obj->isCreature())
    {
        ObjectInfoMap::const_iterator j = _creatureInfo.find(obj->getEntry());
        if (j != _creatureInfo.end())
            _objectGuids[j->second] = guid.getGuidLowPart();
    }
    else if (obj->isGameObject())
    {
        ObjectInfoMap::const_iterator j = _gameObjectInfo.find(obj->getEntry());
        if (j != _gameObjectInfo.end())
            _objectGuids[j->second] = guid.getGuidLowPart();
    }
}

void InstanceScript::removeObject(Object* obj)
{
    WoWGuid guid = obj->getGuid();

    if (obj->isCreature())
    {
        ObjectInfoMap::const_iterator j = _creatureInfo.find(obj->getEntry());
        if (j != _creatureInfo.end())
        {
            ObjectGuidMap::iterator i = _objectGuids.find(j->second);
            if (i != _objectGuids.end() && i->second == guid.getGuidLowPart())
                _objectGuids.erase(i);
        }
    }
    else if (obj->isGameObject())
    {
        ObjectInfoMap::const_iterator j = _gameObjectInfo.find(obj->getEntry());
        if (j != _gameObjectInfo.end())
        {
            ObjectGuidMap::iterator i = _objectGuids.find(j->second);
            if (i != _objectGuids.end() && i->second == guid.getGuidLowPart())
                _objectGuids.erase(i);
        }
    }
}

uint32_t InstanceScript::getGuidFromData(uint32_t type)
{
    ObjectGuidMap::const_iterator i = _objectGuids.find(type);
    if (i != _objectGuids.end())
        return i->second;

    return 0;
}

Creature* InstanceScript::getCreatureFromData(uint32_t type)
{
    return GetCreatureByGuid(getGuidFromData(type));
}

GameObject* InstanceScript::getGameObjectFromData(uint32_t type)
{
    return GetGameObjectByGuid(getGuidFromData(type));
}


//////////////////////////////////////////////////////////////////////////////////////////
// encounters
void InstanceScript::generateBossDataState()
{
    const auto* encounters = sObjectMgr.GetDungeonEncounterList(getWorldMap()->getBaseMap()->getMapId(), getWorldMap()->getDifficulty());
    uint32_t i = 0;

    for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr, ++i)
    {
        DungeonEncounter const* encounter = *itr;

        BossInfo* bossInfo = &bosses[i];
        bossInfo->entry = encounter->creditEntry;
        bossInfo->state = NotStarted;
    }

    // Set States
    for (i = 0; i < bosses.size(); ++i)
        setBossState(i, NotStarted);
}

bool InstanceScript::setBossState(uint32_t id, EncounterStates state)
{
    if (id < bosses.size())
    {
        BossInfo* bossInfo = &bosses[id];
        if (bossInfo->state == InvalidState) // loading
        {
            bossInfo->state = state;
            return false;
        }
        else
        {
            if (bossInfo->state == state)
                return false;

            if (bossInfo->state == Performed)
            {
                return false;
            }

            bossInfo->state = state;
            saveToDB();
        }

        OnEncounterStateChange(id, state);

        if (state == NotStarted)
            getInstance()->respawnBossLinkedGroups(bossInfo->entry);

        return true;
    }
    return false;
}

void InstanceScript::saveToDB()
{
    std::string data = getSaveData();
    if (data.empty())
        return;

    CharacterDatabase.Execute("UPDATE instance SET completedEncounters=%u, data=\'%s\' WHERE id=%u", getCompletedEncounterMask(), data.c_str(), mInstance->getInstanceId());
}

void InstanceScript::loadSavedInstanceData(char const* data)
{
    if (!data)
    {
        sLogger.failure("Unable to load Saved Instance Data for Instance %s (Map %d, Instance Id: %d).", mInstance->getBaseMap()->getMapName().c_str(), mInstance->getBaseMap()->getMapId(), mInstance->getInstanceId());
        return;
    }

    std::istringstream loadStream(data);

    readSaveDataBossStates(loadStream);
    readSaveDataExtended(loadStream);

    sLogger.debug("Saved Instance Data Loaded for Instance %s (Map %d, Instance Id: %d) is complete.", mInstance->getBaseMap()->getMapName().c_str(), mInstance->getBaseMap()->getMapId(), mInstance->getInstanceId());
}

void InstanceScript::readSaveDataBossStates(std::istringstream& data)
{
    const auto* encounters = sObjectMgr.GetDungeonEncounterList(getWorldMap()->getBaseMap()->getMapId(), getWorldMap()->getDifficulty());
    size_t i = 0;

    for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr, ++i)
    {
        DungeonEncounter const* encounter = *itr;

        BossInfo* bossInfo = &bosses[i];
        bossInfo->entry = encounter->creditEntry;
    }

    uint32_t bossId = 0;
    for (std::vector<BossInfo>::iterator itr = bosses.begin(); itr != bosses.end(); ++itr, ++bossId)
    {
        uint32_t buff;
        data >> buff;
        if (buff == InProgress || buff == Failed || buff == PreProgress)
            buff = NotStarted;

        if (buff < InvalidState)
        {
            setBossState(bossId, EncounterStates(buff));
        }
    }
}

void InstanceScript::writeSaveDataBossStates(std::ostringstream& data)
{
    for (auto const& bossInfo : bosses)
        data << uint32_t(bossInfo.state) << ' ';
}

std::string InstanceScript::getSaveData()
{
    std::ostringstream saveStream;

    writeSaveDataBossStates(saveStream);
    writeSaveDataExtended(saveStream);

    return saveStream.str();
}

#if VERSION_STRING >= WotLK
void InstanceScript::updateEncounterState(EncounterCreditType type, uint32_t creditEntry)
{
    DungeonEncounterList const* encounters = sObjectMgr.GetDungeonEncounterList(mInstance->getBaseMap()->getMapId(), mInstance->getDifficulty());
    if (!encounters)
        return;

    uint32_t dungeonId = 0;

    for (auto const& encounter : *encounters)
    {
        if (encounter->creditType == type && encounter->creditEntry == creditEntry)
        {
            completedEncounters |= 1 << encounter->dbcEntry->encounterIndex;
            if (encounter->lastEncounterDungeon)
            {
                dungeonId = encounter->lastEncounterDungeon;
                break;
            }
        }
    }

    if (dungeonId)
    {
        for (auto const& ref : mInstance->getPlayers())
        {
            if (Player* player = ref.second)
            {
                if (Group* grp = player->getGroup())
                {
                    if (grp->isLFGGroup())
                    {
                        sLfgMgr.RewardDungeonDoneFor(dungeonId, player);
                        return;
                    }
                }
            }
        }
    }
}

void InstanceScript::updateEncountersStateForCreature(uint32_t creditEntry, uint8_t /*difficulty*/)
{
    updateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, creditEntry);
}

void InstanceScript::updateEncountersStateForSpell(uint32_t creditEntry, uint8_t /*difficulty*/)
{
    updateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, creditEntry);
}
#endif

void InstanceScript::sendUnitEncounter(uint32_t type, Unit* unit, uint8_t value_a, uint8_t value_b)
{
    WorldMap* instance = getInstance();
    instance->sendPacketToAllPlayers(SmsgUpdateInstanceEncounterUnit(type, unit ? unit->GetNewGUID() : WoWGuid(), value_a, value_b).serialise().get());
}

void InstanceScript::displayDataStateList(Player* player)
{
    player->broadcastMessage("=== DataState for instance %s ===", mInstance->getBaseMap()->getMapInfo()->name.c_str());

    for (const auto& encounters : bosses)
    {
        CreatureProperties const* creature = sMySQLStore.getCreatureProperties(encounters.entry);
        if (creature != nullptr)
        {
            player->broadcastMessage("  Boss '%s' (%u) - %s", creature->Name.c_str(), encounters.entry, getDataStateString(encounters.state).c_str());
        }
        else
        {
            GameObjectProperties const* gameobject = sMySQLStore.getGameObjectProperties(encounters.entry);
            if (gameobject != nullptr)
                player->broadcastMessage("  Object '%s' (%u) - %s", gameobject->name.c_str(), encounters.entry, getDataStateString(encounters.state).c_str());
            else
                player->broadcastMessage("  MiscData %u - %s", encounters.entry, getDataStateString(encounters.state).c_str());
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
    player->broadcastMessage("=== Timers for instance %s ===", mInstance->getBaseMap()->getMapInfo()->name.c_str());

    if (mTimers.empty())
    {
        player->broadcastMessage("  No Timers available!");
    }
    else
    {
        for (const auto& intTimer : mTimers)
            player->broadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance update

void InstanceScript::registerUpdateEvent()
{
    sEventMgr.AddEvent(mInstance, &WorldMap::callScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, getUpdateFrequency(), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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
            MapCell* CurrentCell = mInstance->getCellByCoords(xMin, yMin);
            if (forceActive && CurrentCell == nullptr)
            {
                CurrentCell = mInstance->createByCoords(xMin, yMin);
                if (CurrentCell != nullptr)
                    CurrentCell->init(mInstance->getPosX(xMin), mInstance->getPosY(yMin), mInstance);
            }

            if (CurrentCell != nullptr)
            {
                if (forceActive)
                    mInstance->addForcedCell(CurrentCell);
                else
                    mInstance->removeForcedCell(CurrentCell);
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
        sLogger.failure("tried to create a invalid creature with entry %u!", entry);
        return nullptr;
    }

    Creature* creature = mInstance->getInterface()->spawnCreature(entry, LocationVector(posX, posY, posZ, posO), true, true, 0, 0);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->setFaction(factionId);
    else
        creature->setFaction(creatureProperties->Faction);

    return creature;
}

Creature* InstanceScript::getCreatureBySpawnId(uint32_t entry)
{
    return mInstance->getSqlIdCreature(entry);
}

Creature* InstanceScript::GetCreatureByGuid(uint32_t guid)
{
    return mInstance->getCreature(guid);
}

CreatureSet InstanceScript::getCreatureSetForEntry(uint32_t entry, bool debug /*= false*/, Player* player /*= nullptr*/)
{
    CreatureSet creatureSet;
    uint32_t countCreatures = 0;
    for (auto creature : mInstance->getCreatures())
    {
        if (creature != nullptr)
        {

            if (creature->getEntry() == entry)
            {
                creatureSet.insert(creature);
                ++countCreatures;
            }
        }
    }

    if (debug == true)
    {
        if (player != nullptr)
            player->broadcastMessage("%u Creatures with entry %u found.", countCreatures, entry);
    }

    return creatureSet;
}

CreatureSet InstanceScript::getCreatureSetForEntries(std::vector<uint32_t> entryVector)
{
    CreatureSet creatureSet;
    for (auto creature : mInstance->getCreatures())
    {
        if (creature != nullptr)
        {
            for (auto entry : entryVector)
            {
                if (creature->getEntry() == entry)
                    creatureSet.insert(creature);
            }
        }
    }

    return creatureSet;
}

Creature* InstanceScript::findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/)
{
    Creature* pCreature = mInstance->getInterface()->findNearestCreature(pObject, entry, maxSearchRange);
    return pCreature;
}

GameObject* InstanceScript::spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld /*= true*/, uint32_t misc1 /*= 0*/, uint32_t phase /*= 0*/)
{
    GameObject* spawnedGameObject = mInstance->getInterface()->spawnGameObject(entry, LocationVector(posX, posY, posZ, posO), addToWorld, misc1, phase);
    return spawnedGameObject;
}

GameObject* InstanceScript::getGameObjectBySpawnId(uint32_t entry)
{
    return mInstance->getSqlIdGameObject(entry);
}

GameObject* InstanceScript::GetGameObjectByGuid(uint32_t guid)
{
    return mInstance->getGameObject(guid);
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
    for (auto gameobject : mInstance->getGameObjects())
    {
        if (gameobject != nullptr)
        {
            if (gameobject->getEntry() == entry)
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
            gameobject->setState(state);
    }
}

//MIT end

/* Hook Stuff */
void ScriptMgr::register_hook(ServerHookEvents event, void* function_pointer)
{
    if (event < NUM_SERVER_HOOKS)
        _hooks[event].insert(function_pointer);
    else
        sLogger.failure("ScriptMgr::register_hook tried to register invalid event %u", event);
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

void ScriptMgr::register_creature_gossip(uint32 entry, GossipScript* script)
{
    const auto itr = creaturegossip_.find(entry);
    if (itr == creaturegossip_.end())
        creaturegossip_.insert(std::make_pair(entry, script));
    //keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

bool ScriptMgr::has_creature_gossip(uint32 entry) const
{
    return creaturegossip_.find(entry) != creaturegossip_.end();
}

GossipScript* ScriptMgr::get_creature_gossip(uint32 entry) const
{
    const auto itr = creaturegossip_.find(entry);
    if (itr != creaturegossip_.end())
        return itr->second;
    return nullptr;
}

void ScriptMgr::register_item_gossip(uint32 entry, GossipScript* script)
{
    const auto itr = itemgossip_.find(entry);
    if (itr == itemgossip_.end())
        itemgossip_.insert(std::make_pair(entry, script));
    //keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

void ScriptMgr::register_go_gossip(uint32 entry, GossipScript* script)
{
    const auto itr = gogossip_.find(entry);
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

GossipScript* ScriptMgr::get_go_gossip(uint32 entry) const
{
    const auto itr = gogossip_.find(entry);
    if (itr != gogossip_.end())
        return itr->second;
    return nullptr;
}

GossipScript* ScriptMgr::get_item_gossip(uint32 entry) const
{
    const auto itr = itemgossip_.find(entry);
    if (itr != itemgossip_.end())
        return itr->second;
    return nullptr;
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
HookInterface& HookInterface::getInstance()
{
    static HookInterface mInstance;
    return mInstance;
}

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

bool HookInterface::OnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell)
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

void HookInterface::OnArenaFinish(Player* pPlayer, std::shared_ptr<ArenaTeam> pTeam, bool victory, bool rated)
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
