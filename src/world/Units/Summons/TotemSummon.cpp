/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TotemSummon.h"

#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/SpellMgr.h"
#include "Units/Players/PlayerDefines.hpp"

TotemSummon::TotemSummon(uint64_t guid, uint32_t duration) : Summon(guid, duration) {}

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

    setLevel(unitOwner->getLevel());
    setRace(0);
    setClass(1); // Creature class warrior
    setGender(GENDER_NONE);
    setPowerType(POWER_TYPE_MANA);
    setBaseAttackTime(MELEE, 2000);
    setBaseAttackTime(OFFHAND, 2000);
    setBoundingRadius(1.0f);
    setCombatReach(1.0f);
    setDisplayId(displayId);
    setNativeDisplayId(displayId);
    setModCastSpeed(1.0f);
    setDynamicFlags(0);

    for (uint8_t school = 0; school < TOTAL_SPELL_SCHOOLS; school++)
    {
        ModDamageDone[school] = unitOwner->GetDamageDoneMod(school);
        HealDoneMod[school] = unitOwner->HealDoneMod[school];
    }

    m_aiInterface->Init(this, AI_SCRIPT_TOTEM, Movement::WP_MOVEMENT_SCRIPT_NONE, unitOwner);
    DisableAI();

    if (getPlayerOwner() != nullptr)
        getPlayerOwner()->sendTotemCreatedPacket(static_cast<uint8_t>(m_summonSlot), getGuid(), getTimeLeft(), getCreatedBySpellId());
}

void TotemSummon::unSummon()
{
    ///\ todo: death animation for totems, should happen on unsummon
    /*if (isAlive())
        setDeathState(DEAD);*/

    interruptSpell();
    RemoveAllAuras();

    Summon::unSummon();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void TotemSummon::OnPushToWorld()
{
    getUnitOwner()->getSummonInterface()->addTotem(this, TotemSlots(m_summonSlot));
    Summon::OnPushToWorld();

    SetupSpells();
}

void TotemSummon::OnPreRemoveFromWorld()
{
    getUnitOwner()->getSummonInterface()->removeTotem(this, false);
    Summon::OnPreRemoveFromWorld();
}

bool TotemSummon::isTotem() const { return true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void TotemSummon::Die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/)
{
    // Clear health batch on death
    clearHealthBatch();

    unSummon();
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
        sLogger.debug("Totem %u does not have any spells to cast", creature_properties->Id);
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
            targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
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
