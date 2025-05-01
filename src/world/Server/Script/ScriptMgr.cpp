/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ScriptMgr.hpp"

#include "AEVersion.hpp"
#include "WorldConf.h"
#include "CreatureAIScript.hpp"
#include "Management/GameEvent.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Item.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <fstream>

#include <git_version.hpp>

#include "AchievementScript.hpp"
#include "DynLib.hpp"
#include "QuestScript.hpp"
#include "Logging/Logger.hpp"
#include "Management/GameEventMgr.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/GameObject.h"
#include "Server/ServerState.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"

#ifdef WIN32
    #define LIBMASK ".dll";
#else
    #ifndef __APPLE__
        #define LIBMASK ".so";
    #else
        #define LIBMASK ".dylib";
    #endif
#endif

ScriptMgr::ScriptMgr() = default;
ScriptMgr::~ScriptMgr() = default;

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
        if (itr.first == criteriaId)
            return itr.second;

    return nullptr;
}

void ScriptMgr::register_achievement_criteria_script(uint32_t criteriaId, AchievementCriteriaScript* acs)
{
    const auto criteriaEntry = sAchievementCriteriaStore.lookupEntry(criteriaId);
    if (criteriaEntry == nullptr)
    {
        sLogger.failure("ScriptMgr tried to register a script for achievement criteria id {} but criteria does not exist!", criteriaId);
        return;
    }

    if (_achievementCriteriaScripts.contains(criteriaId))
    {
        sLogger.debug("ScriptMgr tried to register a script for achievement criteria id {} but this criteria has already one.", criteriaId);
        return;
    }

    _achievementCriteriaScripts[criteriaId] = acs;
}

void ScriptMgr::register_achievement_criteria_script(uint32_t* criteriaIds, AchievementCriteriaScript* acs)
{
    for (uint32_t i = 0; criteriaIds[i] != 0; ++i)
        register_achievement_criteria_script(criteriaIds[i], acs);
}
#endif

SpellScript* ScriptMgr::getSpellScript(uint32_t spellId) const
{
    for (const auto& itr : _spellScripts)
        if (itr.first == spellId)
            return itr.second;

    return nullptr;
}

void ScriptMgr::register_spell_script(uint32_t spellId, SpellScript* ss, bool registerAllDifficulties/* = true*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        sLogger.failure("ScriptMgr tried to register a script for spell id {} but spell does not exist!", spellId);
        return;
    }

    const auto addScriptToAllDifficultiesReturnCount = [this, ss](SpellInfo const* spInfo) -> uint8_t
    {
        uint8_t registeredSpells = 0;
        if (spInfo->getSpellDifficultyID() != 0)
        {
            for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
            {
                const auto spellDifficultyInfo = sSpellMgr.getSpellInfoByDifficulty(spInfo->getSpellDifficultyID(), i);
                if (spellDifficultyInfo == nullptr)
                    continue;

                _register_spell_script(spellDifficultyInfo->getId(), ss);
                ++registeredSpells;
            }
        }
        return registeredSpells;
    };

    if (spellInfo->hasSpellRanks())
    {
        const auto* rankedSpell = spellInfo->getRankInfo()->getFirstSpell();
        do
        {
            if (registerAllDifficulties)
            {
                const auto registeredSpells = addScriptToAllDifficultiesReturnCount(rankedSpell);
                if (registeredSpells > 0)
                {
                    // Script was certainly added to this rank, safe to continue to next rank
                    rankedSpell = rankedSpell->getRankInfo()->getNextSpell();
                    continue;
                }
            }
            _register_spell_script(rankedSpell->getId(), ss);
            rankedSpell = rankedSpell->getRankInfo()->getNextSpell();
        } while (rankedSpell != nullptr);

        // Script is added to all ranks and difficulties, safe to exit
        return;
    }

    if (registerAllDifficulties)
    {
        const auto registeredSpells = addScriptToAllDifficultiesReturnCount(spellInfo);
        // Make sure to register at least the original spell
        if (registeredSpells > 0)
            return;
    }

    _register_spell_script(spellId, ss);
}

void ScriptMgr::register_spell_script(uint32_t* spellIds, SpellScript* ss, bool registerAllDifficulties/* = true*/)
{
    for (uint32_t i = 0; spellIds[i] != 0; ++i)
        register_spell_script(spellIds[i], ss, registerAllDifficulties);
}

void ScriptMgr::_register_spell_script(uint32_t spellId, SpellScript* ss)
{
    if (_spellScripts.contains(spellId))
    {
        sLogger.debug("ScriptMgr tried to register a script for spell id {} but this spell has already one.", spellId);
        return;
    }

    _spellScripts[spellId] = ss;
}

struct ScriptingEngine_dl
{
    std::unique_ptr<Arcemu::DynLib> dl = nullptr;
    exp_script_register InitializeCall = nullptr;
    uint32_t Type = 0;
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
        auto dynLib = std::make_unique<Arcemu::DynLib>(fileName.c_str());

        loadMessageStream << dynLib->GetName() << " : ";

        if (!dynLib->Load())
        {
            loadMessageStream << "ERROR: Cannot open library.";
            sLogger.failure(loadMessageStream.str());
            continue;
        }

        auto serverStateCall = reinterpret_cast<exp_set_serverstate_singleton>(dynLib->GetAddressForSymbol("_exp_set_serverstate_singleton"));
        if (!serverStateCall)
        {
            loadMessageStream << "ERROR: Cannot find set_serverstate_call function.";
            sLogger.failure(loadMessageStream.str());
            continue;
        }

        serverStateCall(ServerState::instance());

        auto versionCall = reinterpret_cast<exp_get_version>(dynLib->GetAddressForSymbol("_exp_get_version"));
        auto registerCall = reinterpret_cast<exp_script_register>(dynLib->GetAddressForSymbol("_exp_script_register"));
        auto typeCall = reinterpret_cast<exp_get_script_type>(dynLib->GetAddressForSymbol("_exp_get_script_type"));
        if (!versionCall || !registerCall || !typeCall)
        {
            loadMessageStream << "ERROR: Cannot find version functions.";
            sLogger.failure(loadMessageStream.str());
            continue;
        }

        std::string dllVersion = versionCall();
        uint32_t scriptType = typeCall();

        if (dllVersion != AE_BUILD_HASH)
        {
            loadMessageStream << "ERROR: Version mismatch.";
            sLogger.failure(loadMessageStream.str());
            continue;
        }

        loadMessageStream << std::string(AE_BUILD_HASH) << " : ";

        if ((scriptType & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            ScriptingEngine_dl scriptingEngineDl;
            scriptingEngineDl.dl = std::move(dynLib);
            scriptingEngineDl.InitializeCall = registerCall;
            scriptingEngineDl.Type = scriptType;

            scriptingEngineDls.push_back(std::move(scriptingEngineDl));

            loadMessageStream << "delayed load";
        }
        else
        {
            registerCall(this);
            dynamiclibs.push_back(std::move(dynLib));

            loadMessageStream << "loaded";
        }
        sLogger.info(loadMessageStream.str());

        dllCount++;
    }

    if (dllCount == 0)
    {
        sLogger.failure("No external scripts found! Server will continue to function with limited functionality.");
    }
    else
    {
        sLogger.info("ScriptMgr : Loaded {} external libraries.", dllCount);
        sLogger.info("ScriptMgr : Loading optional scripting engine(s)...");

        for (auto& engineDl : scriptingEngineDls)
        {
            engineDl.InitializeCall(this);
            dynamiclibs.push_back(std::move(engineDl.dl));
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

    dynamiclibs.clear();
}

void ScriptMgr::DumpUnimplementedSpells()
{
    std::ofstream of;

    sLogger.info("Dumping IDs for spells with unimplemented dummy/script effect(s)");
    uint32_t count = 0;

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

    sLogger.info("Dumped {} IDs.", count);

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

    sLogger.info("Dumped {} IDs.", count);
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

void ScriptMgr::register_creature_script(uint32_t entry, exp_create_creature_ai callback)
{
    std::lock_guard lock(m_creaturesMutex);

    if (_creatures.contains(entry))
        sLogger.debug("ScriptMgr tried to register a script for Creature ID: {} but this creature has already one!", entry);

    _creatures.insert(CreatureCreateMap::value_type(entry, callback));
}

void ScriptMgr::register_gameobject_script(uint32_t entry, exp_create_gameobject_ai callback)
{
    if (_gameobjects.contains(entry))
        sLogger.debug("ScriptMgr tried to register a script for GameObject ID: {} but this go has already one.", entry);

    _gameobjects.insert(GameObjectCreateMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_aura(uint32_t entry, exp_handle_dummy_aura callback)
{
    if (_auras.contains(entry))
        sLogger.debug("ScriptMgr tried to register a script for Aura ID: {} but this aura has already one.", entry);

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == nullptr)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a dummy aura handler for invalid Spell ID: {}.", entry);
        return;
    }

#if VERSION_STRING >= TBC
    if (!sp->hasEffectApplyAuraName(SPELL_AURA_DUMMY) && !sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_DUMMY))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr registered a dummy aura handler for Spell ID: {} ({}), but spell has no dummy aura!", entry, sp->getName());
#endif

    _auras.insert(HandleDummyAuraMap::value_type(entry, callback));
}

void ScriptMgr::register_dummy_spell(uint32_t entry, exp_handle_dummy_spell callback)
{
    if (_spells.contains(entry))
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a script for Spell ID: {} but this spell has already one", entry);
        return;
    }

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == nullptr)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "ScriptMgr tried to register a dummy handler for invalid Spell ID: {}.", entry);
        return;
    }

    if (!sp->hasEffect(SPELL_EFFECT_DUMMY) && !sp->hasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->hasEffect(SPELL_EFFECT_SEND_EVENT))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr registered a dummy handler for Spell ID: {} ({}), but spell has no dummy/script/send event effect!", entry, sp->getName());

    _spells.insert(HandleDummySpellMap::value_type(entry, callback));
}

void ScriptMgr::register_quest_script(uint32_t entry, QuestScript* qs)
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(entry);
    if (q != nullptr)
    {
        if (q->pQuestScript != nullptr)
            sLogger.debug("ScriptMgr tried to register a script for Quest ID: {} but this quest has already one.", entry);

        const_cast<QuestProperties*>(q)->pQuestScript = qs;
    }

    _questscripts.insert(qs);
}

void ScriptMgr::register_event_script(uint32_t entry, EventScript* es)
{
    auto gameEvent = sGameEventMgr.GetEventById(entry);
    if (gameEvent != nullptr)
    {
        if (gameEvent->mEventScript != nullptr)
        {
            sLogger.debug("ScriptMgr tried to register a script for Event ID: {} but this event has already one.", entry);
            return;
        }

        gameEvent->mEventScript = es;
        _eventscripts.insert(es);
    }
}

void ScriptMgr::register_instance_script(uint32_t pMapId, exp_create_instance_ai pCallback)
{
    if (mInstances.contains(pMapId))
        sLogger.debug("ScriptMgr tried to register a script for Instance ID: {} but this instance already has one.", pMapId);

    mInstances.insert(InstanceCreateMap::value_type(pMapId, pCallback));
}

void ScriptMgr::register_creature_script(uint32_t* entries, exp_create_creature_ai callback)
{
    for (uint32_t y = 0; entries[y] != 0; ++y)
        register_creature_script(entries[y], callback);
}

void ScriptMgr::register_gameobject_script(uint32_t* entries, exp_create_gameobject_ai callback)
{
    for (uint32_t y = 0; entries[y] != 0; ++y)
        register_gameobject_script(entries[y], callback);
}

void ScriptMgr::register_dummy_aura(uint32_t* entries, exp_handle_dummy_aura callback)
{
    for (uint32_t y = 0; entries[y] != 0; ++y)
        register_dummy_aura(entries[y], callback);
}

void ScriptMgr::register_dummy_spell(uint32_t* entries, exp_handle_dummy_spell callback)
{
    for (uint32_t y = 0; entries[y] != 0; ++y)
        register_dummy_spell(entries[y], callback);
}

void ScriptMgr::register_script_effect(uint32_t* entries, exp_handle_script_effect callback)
{
    for (uint32_t y = 0; entries[y] != 0; ++y)
        register_script_effect(entries[y], callback);
}

void ScriptMgr::register_script_effect(uint32_t entry, exp_handle_script_effect callback)
{
    HandleScriptEffectMap::iterator itr = SpellScriptEffects.find(entry);

    if (itr != SpellScriptEffects.end())
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr tried to register more than 1 script effect handlers for Spell {}", entry);
        return;
    }

    SpellInfo const* sp = sSpellMgr.getSpellInfo(entry);
    if (sp == nullptr)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr tried to register a script effect handler for invalid Spell {}.", entry);
        return;
    }

    if (!sp->hasEffect(SPELL_EFFECT_SCRIPT_EFFECT) && !sp->hasEffect(SPELL_EFFECT_SEND_EVENT))
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "ScriptMgr registered a script effect handler for Spell ID: {} ({}), but spell has no scripted effect!", entry, sp->getName());

    SpellScriptEffects.insert(std::pair< uint32_t, exp_handle_script_effect >(entry, callback));
}

CreatureAIScript* ScriptMgr::CreateAIScriptClassForEntry(Creature* pCreature)
{
    uint32_t entry = pCreature->getEntry();

    CreatureCreateMap::iterator itr = _creatures.find(entry);
    if (itr == _creatures.end())
        return nullptr;

    exp_create_creature_ai function_ptr = itr->second;
    return (function_ptr)(pCreature);
}

GameObjectAIScript* ScriptMgr::CreateAIScriptClassForGameObject(uint32_t /*uEntryId*/, GameObject* pGameObject)
{
    GameObjectCreateMap::iterator itr = _gameobjects.find(pGameObject->getEntry());
    if (itr == _gameobjects.end())
        return nullptr;

    exp_create_gameobject_ai function_ptr = itr->second;
    return (function_ptr)(pGameObject);
}

InstanceScript* ScriptMgr::CreateScriptClassForInstance(uint32_t /*pMapId*/, WorldMap* pMapMgr)
{
    InstanceCreateMap::iterator Iter = mInstances.find(pMapMgr->getBaseMap()->getMapId());
    if (Iter == mInstances.end())
        return nullptr;
    exp_create_instance_ai function_ptr = Iter->second;
    return (function_ptr)(pMapMgr);
}

bool ScriptMgr::CallScriptedDummySpell(uint32_t uSpellId, uint8_t effectIndex, Spell* pSpell)
{
    HandleDummySpellMap::iterator itr = _spells.find(uSpellId);
    if (itr == _spells.end())
        return false;

    exp_handle_dummy_spell function_ptr = itr->second;
    return (function_ptr)(effectIndex, pSpell);
}

bool ScriptMgr::HandleScriptedSpellEffect(uint32_t SpellId, uint8_t effectIndex, Spell* s)
{
    HandleScriptEffectMap::iterator itr = SpellScriptEffects.find(SpellId);
    if (itr == SpellScriptEffects.end())
        return false;

    exp_handle_script_effect ptr = itr->second;
    return (ptr)(effectIndex, s);
}

bool ScriptMgr::CallScriptedDummyAura(uint32_t uSpellId, uint8_t effectIndex, Aura* pAura, bool apply)
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

void ScriptMgr::register_hook(ServerHookEvents event, void* function_pointer)
{
    if (event < NUM_SERVER_HOOKS)
        _hooks[event].insert(function_pointer);
    else
        sLogger.failure("ScriptMgr::register_hook tried to register invalid event {}", event);
}

bool ScriptMgr::has_creature_script(uint32_t entry) const
{
    return _creatures.contains(entry);
}

bool ScriptMgr::has_gameobject_script(uint32_t entry) const
{
    return _gameobjects.contains(entry);
}

bool ScriptMgr::has_dummy_aura_script(uint32_t entry) const
{
    return _auras.contains(entry);
}

bool ScriptMgr::has_dummy_spell_script(uint32_t entry) const
{
    return _spells.contains(entry);
}

bool ScriptMgr::has_script_effect(uint32_t entry) const
{
    return SpellScriptEffects.contains(entry);
}

bool ScriptMgr::has_instance_script(uint32_t id) const
{
    return mInstances.contains(id);
}

bool ScriptMgr::has_hook(ServerHookEvents evt, void* ptr) const
{
    return _hooks[evt].size() != 0 && _hooks[evt].contains(ptr);
}

bool ScriptMgr::has_quest_script(uint32_t entry) const
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(entry);
    return q == nullptr || q->pQuestScript != nullptr;
}

void ScriptMgr::register_creature_gossip(uint32_t entry, GossipScript* script)
{
    std::lock_guard lock(m_gossipCreatureMutex);

    const auto itr = creaturegossip_.find(entry);
    if (itr == creaturegossip_.end())
    {
        creaturegossip_.insert(std::make_pair(entry, script));
        // keeping track of all created gossips to delete them all on shutdown
        _customgossipscripts.insert(script);
    }
    else
    {
        delete script;
    }
}

bool ScriptMgr::has_creature_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipCreatureMutex);

    return creaturegossip_.contains(entry);
}

GossipScript* ScriptMgr::get_creature_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipCreatureMutex);

    const auto itr = creaturegossip_.find(entry);
    if (itr != creaturegossip_.end())
        return itr->second;
    return nullptr;
}

void ScriptMgr::register_item_gossip(uint32_t entry, GossipScript* script)
{
    std::lock_guard lock(m_gossipItemMutex);

    const auto itr = itemgossip_.find(entry);
    if (itr == itemgossip_.end())
        itemgossip_.insert(std::make_pair(entry, script));
    // keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

void ScriptMgr::register_go_gossip(uint32_t entry, GossipScript* script)
{
    std::lock_guard lock(m_gossipGoMutex);

    const auto itr = gogossip_.find(entry);
    if (itr == gogossip_.end())
        gogossip_.insert(std::make_pair(entry, script));
    // keeping track of all created gossips to delete them all on shutdown
    _customgossipscripts.insert(script);
}

bool ScriptMgr::has_item_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipItemMutex);

    return itemgossip_.contains(entry);
}

bool ScriptMgr::has_go_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipGoMutex);

    return gogossip_.contains(entry);
}

GossipScript* ScriptMgr::get_go_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipGoMutex);

    const auto itr = gogossip_.find(entry);
    if (itr != gogossip_.end())
        return itr->second;
    return nullptr;
}

GossipScript* ScriptMgr::get_item_gossip(uint32_t entry) const
{
    std::lock_guard lock(m_gossipItemMutex);

    const auto itr = itemgossip_.find(entry);
    if (itr != itemgossip_.end())
        return itr->second;

    return nullptr;
}

void ScriptMgr::ReloadScriptEngines()
{
    // for all scripting engines that allow reloading, assuming there will be new scripting engines.
    exp_get_script_type version_function;
    exp_engine_reload engine_reloadfunc;

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
    {
        const auto& dl = *itr;

        version_function = reinterpret_cast<exp_get_script_type>(dl->GetAddressForSymbol("_exp_get_script_type"));
        if (version_function == nullptr)
            continue;

        if ((version_function() & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            engine_reloadfunc = reinterpret_cast<exp_engine_reload>(dl->GetAddressForSymbol("_export_engine_reload"));
            if (engine_reloadfunc != nullptr)
                engine_reloadfunc();
        }
    }
}

void ScriptMgr::UnloadScriptEngines()
{
    // for all scripting engines that allow unloading, assuming there will be new scripting engines.
    exp_get_script_type version_function;
    exp_engine_unload engine_unloadfunc;

    for (DynamicLibraryMap::iterator itr = dynamiclibs.begin(); itr != dynamiclibs.end(); ++itr)
    {
        const auto& dl = *itr;

        version_function = reinterpret_cast<exp_get_script_type>(dl->GetAddressForSymbol("_exp_get_script_type"));
        if (version_function == nullptr)
            continue;

        if ((version_function() & SCRIPT_TYPE_SCRIPT_ENGINE) != 0)
        {
            engine_unloadfunc = reinterpret_cast<exp_engine_unload>(dl->GetAddressForSymbol("_exp_engine_unload"));
            if (engine_unloadfunc != nullptr)
                engine_unloadfunc();
        }
    }
}
