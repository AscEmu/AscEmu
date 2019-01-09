/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Singleton.h"

#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.hpp"

#include "Units/Players/PlayerDefines.hpp"

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

typedef std::unordered_map<uint32_t, SpellInfo> SpellInfoMap;

typedef Spell* (*SpellScriptLinker)(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
typedef Aura* (*AuraScriptLinker)(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary, Item* i_caster);

typedef std::multimap<uint32_t, SpellArea> SpellAreaMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForQuestMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForAuraMap;
typedef std::multimap<uint32_t, SpellArea const*> SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForQuestMap::const_iterator, SpellAreaForQuestMap::const_iterator> SpellAreaForQuestMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator> SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator> SpellAreaForAreaMapBounds;

// This class loads spells from Spell.dbc and stores them as SpellInfo objects
// Also, this class registers spell scripts to spells and aura scripts to auras
class SERVER_DECL SpellMgr : public Singleton<SpellMgr>
{
public:
    SpellMgr();
    ~SpellMgr();

    void startSpellMgr();
    void loadSpellDataFromDatabase();
    void loadSpellScripts();

    Spell* newSpell(Object* caster, SpellInfo const* info, bool triggered, Aura* aur);
    Aura* newAura(SpellInfo const* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr);

    // Registering spell scripts
    void addSpellById(uint32_t spellId, SpellScriptLinker spellScript);
    // Registering aura scripts
    void addAuraById(uint32_t spellId, AuraScriptLinker auraScript);
    
    // Spell area maps
    SpellAreaMapBounds getSpellAreaMapBounds(uint32_t spellId) const;
    SpellAreaForAreaMapBounds getSpellAreaForAreaMapBounds(uint32_t areaId) const;
    SpellAreaForAuraMapBounds getSpellAreaForAuraMapBounds(uint32_t spellId) const;
    SpellAreaForQuestMapBounds getSpellAreaForQuestMapBounds(uint32_t questId, bool active) const;
    SpellAreaForQuestMapBounds getSpellAreaForQuestEndMapBounds(uint32_t questId) const;
    bool checkLocation(SpellInfo const* spellInfo, uint32_t zone_id, uint32_t area_id, Player* player) const;

    // Custom methods (defined in SpellCustomizations.cpp)
    bool isAlwaysApply(SpellInfo const* spellInfo) const;
    uint32_t getDiminishingGroup(uint32_t id) const;

    SpellInfo const* getSpellInfo(const uint32_t spellId) const;
    SpellInfo const* getSpellInfoByDifficulty(const uint32_t spellId, const uint8_t difficulty) const;
    SpellInfoMap const* getSpellInfoMap() const { return &mSpellInfoMapStore; }

private:
    // DBC files
    void loadSpellInfoData();

    // Database tables
    void loadSpellCoefficientOverride();
    void loadSpellCustomOverride();
    void loadSpellAIThreat();
    void loadSpellEffectOverride();
    void loadSpellAreas();

    // Calculates spell power coefficient
    void setSpellCoefficient(SpellInfo* sp);

    // Custom setters (defined in SpellCustomizations.cpp)
    void setSpellEffectAmplitude(SpellInfo* sp);
    void setSpellMeleeSpellBool(SpellInfo* sp);
    void setSpellRangedSpellBool(SpellInfo* sp);
    void setSpellMissingCIsFlags(SpellInfo* sp);
    void setSpellOnShapeshiftChange(SpellInfo* sp);
    // Hacky methods (defined in Hackfixes.cpp)
    void createDummySpell(uint32_t id);
    void modifyEffectBasePoints(SpellInfo* sp);
    void setMissingSpellLevel(SpellInfo* sp);
    void modifyAuraInterruptFlags(SpellInfo* sp);
    void modifyRecoveryTime(SpellInfo* sp);
    void applyHackFixes();

    SpellAreaMap mSpellAreaMap;
    SpellAreaForAreaMap mSpellAreaForAreaMap;
    SpellAreaForAuraMap mSpellAreaForAuraMap;
    SpellAreaForQuestMap mSpellAreaForQuestMap;
    SpellAreaForQuestMap mSpellAreaForActiveQuestMap;
    SpellAreaForQuestMap mSpellAreaForQuestEndMap;

    SpellInfoMap mSpellInfoMapStore;

    SpellInfo* getMutableSpellInfo(const uint32_t spellId);

    // Script registerers
    void addSpellBySpellInfo(SpellInfo* info, SpellScriptLinker spellScript);
    void addAuraBySpellInfo(SpellInfo* info, AuraScriptLinker auraScript);
    void setupSpellScripts();
    void setupSpellClassScripts();
};

#define sSpellMgr SpellMgr::getSingleton()
