/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ScriptMgrDefines.hpp"
#include "CommonTypes.hpp"
#include "HookInterfaceDefines.hpp"
#include "Spell/SpellScriptDefines.hpp"

#include <mutex>

struct DamageInfo;
struct AuraEffectModifier;

class Item;
class SpellInfo;
class SpellProc;
class Object;
class Player;

enum SpellCastResult : uint8_t;
enum AuraRemoveMode : uint8_t;
enum SpellProcFlags : uint32_t;
enum ServerHookEvents;

class SERVER_DECL ScriptMgr
{
    friend class HookInterface;

    ScriptMgr();
    ~ScriptMgr();

public:
    static ScriptMgr& getInstance();

    ScriptMgr(ScriptMgr&&) = delete;
    ScriptMgr(ScriptMgr const&) = delete;
    ScriptMgr& operator=(ScriptMgr&&) = delete;
    ScriptMgr& operator=(ScriptMgr const&) = delete;

#ifdef FT_ACHIEVEMENTS
    // Achievement criteria script hooks
    bool callScriptedAchievementCriteriaCanComplete(uint32_t criteriaId, Player* player, Object* target) const;
#endif

    // Spell script hooks
    SpellCastResult callScriptedSpellCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2) const;
    void callScriptedSpellAtStartCasting(Spell* spell);
    void callScriptedSpellFilterTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets);
    void callScriptedSpellBeforeHit(Spell* spell, uint8_t effectIndex);
    void callScriptedSpellAfterMiss(Spell* spell, Unit* unitTarget);
    SpellScriptEffectDamage callScriptedSpellDoCalculateEffect(Spell* spell, uint8_t effectIndex, int32_t* damage) const;
    SpellScriptExecuteState callScriptedSpellBeforeSpellEffect(Spell* spell, uint8_t effectIndex) const;
    SpellScriptCheckDummy callScriptedSpellOnDummyOrScriptedEffect(Spell* spell, uint8_t effectIndex) const;
    void callScriptedSpellAfterSpellEffect(Spell* spell, uint8_t effectIndex);

    // Aura script hooks
    void callScriptedAuraOnCreate(Aura* aur);
    void callScriptedAuraOnApply(Aura* aur);
    void callScriptedAuraOnRemove(Aura* aur, AuraRemoveMode mode);
    void callScriptedAuraOnRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount);
    SpellScriptExecuteState callScriptedAuraBeforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const;
    SpellScriptCheckDummy callScriptedAuraOnDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const;
    SpellScriptExecuteState callScriptedAuraOnPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float* damage) const;

    // Spell proc script hooks
    void callScriptedSpellProcCreate(SpellProc* spellProc, Object* obj);
    bool callScriptedSpellCanProc(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const;
    bool callScriptedSpellCheckProcFlags(SpellProc* spellProc, SpellProcFlags procFlags) const;
    bool callScriptedSpellProcCanDelete(SpellProc* spellProc, uint32_t spellId, uint64_t casterGuid, uint64_t misc) const;
    SpellScriptExecuteState callScriptedSpellProcDoEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const;
    uint32_t callScriptedSpellCalcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell) const;
    bool callScriptedSpellCanProcOnTriggered(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, Aura* triggeredFromAura) const;
    SpellScriptExecuteState callScriptedSpellProcCastSpell(SpellProc* spellProc, Unit* caster, Unit* victim, Spell* spellToProc);

#ifdef FT_ACHIEVEMENTS
    AchievementCriteriaScript* getAchievementCriteriaScript(uint32_t criteriaId) const;
    void register_achievement_criteria_script(uint32_t criteriaId, AchievementCriteriaScript* acs);
    void register_achievement_criteria_script(uint32_t* criteriaIds, AchievementCriteriaScript* acs);
#endif

    SpellScript* getSpellScript(uint32_t spellId) const;
    // By default this will register spell script to spell's all different ranks and difficulties (if they exist)
    void register_spell_script(uint32_t spellId, SpellScript* ss, bool registerAllDifficulties = true);
    // By default this will register spell script to spell's all different ranks and difficulties (if they exist)
    void register_spell_script(uint32_t* spellIds, SpellScript* ss, bool registerAllDifficulties = true);

    // Creature AI script hooks
    void DamageTaken(Creature* pCreature, Unit* attacker, uint32_t* damage) const;
    CreatureAIScript* getCreatureAIScript(Creature* pCreature) const;

protected:
#ifdef FT_ACHIEVEMENTS
    AchievementCriteriaScripts _achievementCriteriaScripts;
#endif
    SpellScripts _spellScripts;

private:
    void _register_spell_script(uint32_t spellId, SpellScript* ss);

public:
    void LoadScripts();
    void UnloadScripts();

    void DumpUnimplementedSpells();

    CreatureAIScript* CreateAIScriptClassForEntry(Creature* pCreature);
    GameObjectAIScript* CreateAIScriptClassForGameObject(uint32_t uEntryId, GameObject* pGameObject);
    InstanceScript* CreateScriptClassForInstance(uint32_t pMapId, WorldMap* pMapMgr);

    bool CallScriptedDummySpell(uint32_t uSpellId, uint8_t effectIndex, Spell* pSpell);
    bool HandleScriptedSpellEffect(uint32_t SpellId, uint8_t effectIndex, Spell* s);
    bool CallScriptedDummyAura(uint32_t uSpellId, uint8_t effectIndex, Aura* pAura, bool apply);
    bool CallScriptedItem(Item* pItem, Player* pPlayer);

    // Single Entry Registers
    void register_creature_script(uint32_t entry, exp_create_creature_ai callback);
    void register_gameobject_script(uint32_t entry, exp_create_gameobject_ai callback);
    void register_dummy_aura(uint32_t entry, exp_handle_dummy_aura callback);
    void register_dummy_spell(uint32_t entry, exp_handle_dummy_spell callback);
    void register_script_effect(uint32_t entry, exp_handle_script_effect callback);
    void register_instance_script(uint32_t pMapId, exp_create_instance_ai pCallback);
    void register_hook(ServerHookEvents event, void* function_pointer);
    void register_quest_script(uint32_t entry, QuestScript* qs);
    void register_event_script(uint32_t entry, EventScript* es);

    // Gossip interface registration
    void register_creature_gossip(uint32_t, GossipScript*);
    void register_item_gossip(uint32_t, GossipScript*);
    void register_go_gossip(uint32_t, GossipScript*);

    // Mutliple Entry Registers
    void register_creature_script(uint32_t* entries, exp_create_creature_ai callback);
    void register_gameobject_script(uint32_t* entries, exp_create_gameobject_ai callback);
    void register_dummy_aura(uint32_t* entries, exp_handle_dummy_aura callback);
    void register_dummy_spell(uint32_t* entries, exp_handle_dummy_spell callback);
    void register_script_effect(uint32_t* entries, exp_handle_script_effect callback);

    void ReloadScriptEngines();
    void UnloadScriptEngines();

    bool has_creature_script(uint32_t) const;
    bool has_gameobject_script(uint32_t) const;
    bool has_dummy_aura_script(uint32_t) const;
    bool has_dummy_spell_script(uint32_t) const;
    bool has_script_effect(uint32_t) const;
    bool has_instance_script(uint32_t) const;
    bool has_hook(ServerHookEvents, void*) const;
    bool has_quest_script(uint32_t) const;

    bool has_creature_gossip(uint32_t) const;
    bool has_item_gossip(uint32_t) const;
    bool has_go_gossip(uint32_t) const;

    GossipScript* get_creature_gossip(uint32_t) const;
    GossipScript* get_go_gossip(uint32_t) const;
    GossipScript* get_item_gossip(uint32_t) const;

protected:
    InstanceCreateMap mInstances;
    CreatureCreateMap _creatures;
    std::mutex m_creaturesMutex;
    GameObjectCreateMap _gameobjects;
    HandleDummyAuraMap _auras;
    HandleDummySpellMap _spells;
    HandleScriptEffectMap SpellScriptEffects;
    DynamicLibraryMap dynamiclibs;
    ServerHookList _hooks[NUM_SERVER_HOOKS];
    CustomGossipScripts _customgossipscripts;
    EventScripts _eventscripts;
    QuestScripts _questscripts;

    GossipMap creaturegossip_;
    mutable std::mutex m_gossipCreatureMutex;

    GossipMap gogossip_;
    mutable std::mutex m_gossipGoMutex;

    GossipMap itemgossip_;
    mutable std::mutex m_gossipItemMutex;
};

#define sScriptMgr ScriptMgr::getInstance()
