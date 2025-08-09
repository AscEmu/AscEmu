/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//#include "Spell.hpp"
//#include "SpellAura.hpp"
//#include "SpellInfo.hpp"
#include "SpellTargetConstraint.hpp"
//#include "Storage/WDB/WDBStructures.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"

#include <memory>

namespace WDB::Structures
{
    struct SkillLineAbilityEntry;
}

class Unit;
class Object;
class Spell;

struct SpellArea
{
    uint32_t spellId;
    uint32_t areaId;            // Zone or area id. 0 if not zone dependant
    uint32_t questStart;        // Quest id if casted on accepting quest
    uint32_t questEnd;          // If not 0, spell won't be casted (and removed if applied) if the quest id given here has been completed
    int32_t auraSpell;          // If negative, spell won't be casted if player has this aura id. If positive, spell won't be casted if player does not have this aura id
    uint32_t raceMask;          // Can only be applied to certain races (bitmask)
    Gender gender;              // Can only be applied to either gender
    bool questStartCanActive;   // If true, aura is applied on quest start. If false, aura is applied on quest complete?
    bool autoCast;              // If true, then aura is auto applied at entering area

    bool fitsToRequirements(Player* player, uint32_t newZone, uint32_t newArea) const;
};

typedef std::unordered_map<uint32_t, std::unique_ptr<SpellInfo>> SpellInfoMap;

typedef Spell* (*SpellScriptLinker)(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
typedef std::unique_ptr<Aura> (*AuraScriptLinker)(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary, Item* i_caster);

typedef std::multimap<uint32_t, uint32_t> SpellRequiredMap;
typedef std::multimap<uint32_t, uint32_t> SpellsRequiringSpellMap;
typedef std::unordered_multimap<uint16_t, WDB::Structures::SkillLineAbilityEntry const*> SkillSkillAbilityMap;
typedef std::unordered_multimap<uint32_t, WDB::Structures::SkillLineAbilityEntry const*> SpellSkillAbilityMap;
typedef std::pair<SpellRequiredMap::const_iterator, SpellRequiredMap::const_iterator> SpellRequiredMapBounds;
typedef std::pair<SpellsRequiringSpellMap::const_iterator, SpellsRequiringSpellMap::const_iterator> SpellsRequiringSpellMapBounds;
typedef std::pair<SkillSkillAbilityMap::const_iterator, SkillSkillAbilityMap::const_iterator> SkillSkillAbilityMapBounds;
typedef std::pair<SpellSkillAbilityMap::const_iterator, SpellSkillAbilityMap::const_iterator> SpellSkillAbilityMapBounds;

typedef std::map<uint32_t, std::unique_ptr<SpellTargetConstraint>> SpellTargetConstraintMap;

typedef std::multimap<uint32_t, SpellArea> SpellAreaMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForQuestMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForAuraMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForQuestMap::const_iterator, SpellAreaForQuestMap::const_iterator> SpellAreaForQuestMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator> SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator> SpellAreaForAreaMapBounds;

// This class loads spells from Spell.dbc and stores them as SpellInfo objects
class SERVER_DECL SpellMgr
{
private:
    SpellMgr();
    ~SpellMgr();

public:
    static SpellMgr& getInstance();

    SpellMgr(SpellMgr&&) = delete;
    SpellMgr(SpellMgr const&) = delete;
    SpellMgr& operator=(SpellMgr&&) = delete;
    SpellMgr& operator=(SpellMgr const&) = delete;

    void initialize();
    void finalize();

    void loadSpellDataFromDatabase();
    void calculateSpellCoefficients();
    // Legacy scripts
    void loadSpellScripts();

    Spell* newSpell(Object* caster, SpellInfo const* info, bool triggered, Aura* aur);
    std::unique_ptr<Aura> newAura(SpellInfo const* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr);

    // Registering legacy spell scripts (DO NOT USE, use ScriptMgr and SpellScript instead!)
    void addSpellById(uint32_t spellId, SpellScriptLinker spellScript);
    // Registering legacy aura scripts (DO NOT USE, use ScriptMgr and SpellScript instead!)
    void addAuraById(uint32_t spellId, AuraScriptLinker auraScript);

    SpellMechanic const* getCrowdControlMechanicList(bool includeSilence) const;

    // Spell required
    SpellRequiredMapBounds getSpellsRequiredForSpellBounds(uint32_t spellId) const;
    SpellsRequiringSpellMap getSpellsRequiringSpell() const;
    SpellsRequiringSpellMapBounds getSpellsRequiringSpellBounds(uint32_t spellId) const;
    bool isSpellRequiringSpell(uint32_t spellId, uint32_t requiredSpellId) const;
    uint32_t getSpellRequired(uint32_t spellId) const;

    bool isSpellDisabled(uint32_t spellId) const;
    void reloadSpellDisabled();

    // Skills
    // Returns skill ability entries by spell id
    SpellSkillAbilityMapBounds getSkillEntryForSpellBounds(uint32_t spellId) const;
    // Returns skill ability entries by skill id
    SkillSkillAbilityMapBounds getSkillEntryForSkillBounds(uint16_t skillId) const;
    // Use forPlayer if you want to see if skill ability entry fits for player
    WDB::Structures::SkillLineAbilityEntry const* getFirstSkillEntryForSpell(uint32_t spellId, Player const* forPlayer = nullptr) const;

    SpellTargetConstraint const* getSpellTargetConstraintForSpell(uint32_t spellId) const;
    
    // Spell area maps
    SpellAreaMapBounds getSpellAreaMapBounds(uint32_t spellId) const;
    SpellAreaForAreaMapBounds getSpellAreaForAreaMapBounds(uint32_t areaId) const;
    SpellAreaForAuraMapBounds getSpellAreaForAuraMapBounds(uint32_t spellId) const;
    SpellAreaForQuestMapBounds getSpellAreaForQuestMapBounds(uint32_t questId, bool active) const;
    SpellAreaForQuestMapBounds getSpellAreaForQuestEndMapBounds(uint32_t questId) const;
    bool checkLocation(SpellInfo const* spellInfo, uint32_t zone_id, uint32_t area_id, Player* player) const;

    // Custom methods (defined in SpellCustomizations.cpp)
    uint32_t getDiminishingGroup(uint32_t id) const;

    SpellInfo const* getSpellInfo(const uint32_t spellId) const;
    SpellInfo const* getSpellInfoByDifficulty(const uint32_t spellDifficultyId, const uint8_t difficulty) const;
    SpellInfoMap const* getSpellInfoMap() const { return &mSpellInfoMapStore; }

private:
    // DBC files
    void loadSpellInfoData();
    void loadSkillLineAbilityMap();
#if VERSION_STRING < Mop
    void loadTalentRanks();
#endif

    // Database tables
    void loadSpellCoefficientOverride();
    void loadSpellCustomOverride();
    void loadSpellAIThreat();
    void loadSpellEffectOverride();
    void loadSpellAreas();
    void loadSpellRequired();
    void loadSpellTargetConstraints();
    void loadSpellDisabled();
    void loadSpellRanks();

    // Calculates spell power coefficient
    void setSpellCoefficient(SpellInfo* sp);

    // Custom setters (defined in SpellCustomizations.cpp)
    void setSpellEffectAmplitude(SpellInfo* sp);
    void setSpellMissingCIsFlags(SpellInfo* sp);
    // Hacky methods (defined in Hackfixes.cpp)
    void createDummySpell(uint32_t id);
    void modifyEffectBasePoints(SpellInfo* sp);
    void setMissingSpellLevel(SpellInfo* sp);
    void modifyAuraInterruptFlags(SpellInfo* sp);
    void modifyRecoveryTime(SpellInfo* sp);
    void applyHackFixes();

    SpellsRequiringSpellMap mSpellsRequiringSpell;
    SpellRequiredMap mSpellRequired;

    SpellSkillAbilityMap mSpellSkillsMap;
    SkillSkillAbilityMap mSkillSpellsMap;

    SpellTargetConstraintMap mSpellTargetConstraintMap;

    std::set<uint32_t> mDisabledSpells;

    SpellAreaMap mSpellAreaMap;
    SpellAreaForAreaMap mSpellAreaForAreaMap;
    SpellAreaForAuraMap mSpellAreaForAuraMap;

public:
    SpellAreaForQuestMap mSpellAreaForQuestMap;

private:
    SpellAreaForQuestMap mSpellAreaForActiveQuestMap;
    SpellAreaForQuestMap mSpellAreaForQuestEndMap;

    SpellInfoMap mSpellInfoMapStore;

    SpellInfo* getMutableSpellInfo(const uint32_t spellId) const;

    // Legacy script registerers
    void addSpellBySpellInfo(SpellInfo* info, SpellScriptLinker spellScript);
    void addAuraBySpellInfo(SpellInfo* info, AuraScriptLinker auraScript);
    void setupSpellScripts();
    void setupSpellClassScripts();
};

#define sSpellMgr SpellMgr::getInstance()
