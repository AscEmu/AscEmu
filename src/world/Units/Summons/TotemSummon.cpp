/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/TotemSummon.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/SpellMgr.h"

TotemSummon::TotemSummon(uint64_t guid) : Summon(guid) {}

TotemSummon::~TotemSummon() {}

void TotemSummon::Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot)
{
    Summon::Load(creatureProperties, unitOwner, position, spellId, summonSlot);
    uint32_t displayId;

    const auto displayIds = sMySQLStore.getTotemDisplayId(unitOwner->getRace(), creature_properties->Male_DisplayID);
    if (displayIds != nullptr)
        displayId = displayIds->race_specific_id;
    else
        displayId = creature_properties->Male_DisplayID;

    setMaxPower(POWER_TYPE_FOCUS, unitOwner->getLevel() * 30);
    setPower(POWER_TYPE_FOCUS, unitOwner->getLevel() * 30);
    setLevel(unitOwner->getLevel());
    setRace(0);
    setClass(1);
    setGender(2);
    setPowerType(1);
    setBaseAttackTime(MELEE, 2000);
    setBaseAttackTime(OFFHAND, 2000);
    setBoundingRadius(1.0f);
    setCombatReach(1.0f);
    setDisplayId(displayId);
    setNativeDisplayId(creature_properties->Male_DisplayID);
    setModCastSpeed(1.0f);
    setDynamicFlags(0);

    InheritSMMods(unitOwner);

    for (uint8_t school = 0; school < SCHOOL_COUNT; school++)
    {
        ModDamageDone[school] = unitOwner->GetDamageDoneMod(school);
        HealDoneMod[school] = unitOwner->HealDoneMod[school];
    }

    m_aiInterface->Init(this, AI_SCRIPT_TOTEM, Movement::WP_MOVEMENT_SCRIPT_NONE, unitOwner);
    DisableAI();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void TotemSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();

    SetupSpells();
}

void TotemSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}

bool TotemSummon::isTotem() const { return true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void TotemSummon::Die(Unit* /*pAttacker*/, uint32 /*damage*/, uint32 /*spellid*/)
{
    Despawn(1, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void TotemSummon::SetupSpells()
{
    if (getUnitOwner() == nullptr)
        return;

    const auto creatorSpell = sSpellMgr.getSpellInfo(getCreatedBySpellId());
    const auto totemSpell = sSpellMgr.getSpellInfo(creature_properties->AISpells[0]);
    if (totemSpell == nullptr)
    {
        LOG_DEBUG("Totem %u does not have any spells to cast", creature_properties->Id);
        return;
    }

    // Set up AI, depending on our spells.
    bool isCastingTotem = true;

    if (totemSpell->hasEffect(SPELL_EFFECT_SUMMON) ||
        totemSpell->hasEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA) ||
        totemSpell->hasEffect(SPELL_EFFECT_APPLY_RAID_AREA_AURA) ||
        totemSpell->hasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA) ||
        totemSpell->hasEffect(SPELL_EFFECT_APPLY_AURA) && totemSpell->appliesAreaAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL))
        isCastingTotem = false;

    if (!isCastingTotem)
    {
        // We're an area aura. Simply cast the spell.
        m_aiInterface->totemspell = creatorSpell;

        auto spell = sSpellMgr.newSpell(this, totemSpell, true, nullptr);
        SpellCastTargets targets;

        if (!totemSpell->hasEffect(SPELL_AURA_PERIODIC_TRIGGER_SPELL))
        {
            targets.setDestination(GetPosition());
            targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        }
        spell->prepare(&targets);

    }
    else
    {
        // We're a casting totem. Switch AI on, and tell it to cast this spell.
        EnableAI();
        m_aiInterface->totemspell = totemSpell;
        m_aiInterface->m_totemspelltimer = 0;
        m_aiInterface->m_totemspelltime = 3 * TimeVarsMs::Second;
    }
}
