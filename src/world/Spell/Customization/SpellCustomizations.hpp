/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Singleton.h"
#include "Storage/DBC/DBCStores.h"
#include "Spell/SpellInfo.hpp"

class SERVER_DECL SpellCustomizations : public Singleton<SpellCustomizations>
{
public:
    SpellCustomizations();
    ~SpellCustomizations();

    typedef std::unordered_map<uint32, SpellInfo> SpellInfoContainer;

    void LoadSpellInfoData();
    SpellInfo* GetSpellInfo(uint32 spell_id);
    SpellInfoContainer* GetSpellInfoStore();

    void StartSpellCustomization();

    // function for setting up custom vars from db
    void LoadSpellCustomOverride();

    // Function for loading spell coefficient overrides from database
    void loadSpellCoefficientOverride();

    // Calculate spell coefficients
    void setSpellCoefficient(SpellInfo* sp);

    void SetEffectAmplitude(SpellInfo* spell_entry);
    void SetAuraFactoryFunc(SpellInfo* spell_entry);

    void SetMeleeSpellBool(SpellInfo* spell_entry);
    void SetRangedSpellBool(SpellInfo* spell_entry);

    void SetMissingCIsFlags(SpellInfo* spell_entry);
    void SetOnShapeshiftChange(SpellInfo* spell_entry);

    bool isAlwaysApply(SpellInfo* spell_entry);
    uint32_t getDiminishingGroup(uint32_t id);

    SpellInfoContainer _spellInfoContainerStore;
};

#define sSpellCustomizations SpellCustomizations::getSingleton()
