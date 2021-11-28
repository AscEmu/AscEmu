/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"

Summon::Summon(uint64_t guid, uint32_t duration) : Creature(guid), m_duration(duration)
{
    // Override initialization from Creature class
    getThreatManager().initialize();
}

Summon::~Summon() {}

void Summon::Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot)
{
    if (unitOwner != nullptr)
    {
        Creature::Load(creatureProperties, position.x, position.y, position.z, position.o);

        setFaction(unitOwner->getFactionTemplate());
        setPhase(PHASE_SET, unitOwner->GetPhase());
        SetZoneId(unitOwner->GetZoneId());
        setCreatedBySpellId(spellId);
        this->m_summonSlot = summonSlot;

        if (unitOwner->isPvpFlagSet())
            setPvpFlag();
        else
            removePvpFlag();

        if (unitOwner->isFfaPvpFlagSet())
            setFfaPvpFlag();
        else
            removeFfaPvpFlag();

        if (unitOwner->isSanctuaryFlagSet())
            setSanctuaryFlag();
        else
            removeSanctuaryFlag();

        setCreatedByGuid(unitOwner->getGuid());

        if (unitOwner->getSummonedByGuid() == 0)
            setSummonedByGuid(unitOwner->getGuid());
        else
            setSummonedByGuid(unitOwner->getSummonedByGuid());

        this->m_unitOwner = unitOwner;

        if (unitOwner->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
            addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    }
}

void Summon::unSummon()
{
    // If this summon is summoned by a totem, unsummon the totem also
    if (m_unitOwner->isTotem())
        dynamic_cast<TotemSummon*>(m_unitOwner)->unSummon();

    Despawn(10, 0);
}

uint32_t Summon::getTimeLeft() const { return m_duration; }

void Summon::setTimeLeft(uint32_t time) { m_duration = time; }

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void Summon::OnPushToWorld()
{
    if (!isTotem())
        m_unitOwner->getSummonInterface()->addGuardian(this);

    Creature::OnPushToWorld();
}

void Summon::OnPreRemoveFromWorld()
{
    if (m_unitOwner == nullptr)
        return;

    if (getCreatedBySpellId() != 0)
        m_unitOwner->RemoveAura(getCreatedBySpellId());

    if (!isTotem())
        m_unitOwner->getSummonInterface()->removeGuardian(this, false);

    if (getPlayerOwner() != nullptr)
        getPlayerOwner()->sendDestroyObjectPacket(getGuid());

    m_summonSlot = -1;
    m_unitOwner = nullptr;
}

bool Summon::isSummon() const { return true; }

void Summon::onRemoveInRangeObject(Object* object)
{
    if (m_unitOwner != nullptr && object->getGuid() == m_unitOwner->getGuid())
        unSummon();

    Creature::onRemoveInRangeObject(object);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void Summon::Die(Unit* pAttacker, uint32 damage, uint32 spellid)
{
    // If this summon is summoned by a totem, unsummon the totem on death
    if (m_unitOwner->isTotem())
        static_cast<TotemSummon*>(m_unitOwner)->unSummon();

    Creature::Die(pAttacker, damage, spellid);

    m_unitOwner->getSummonInterface()->removeGuardian(this, false);

    m_summonSlot = -1;
    m_unitOwner = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Summon::isSummonedToSlot() const{ return m_summonSlot != -1; }

Player* Summon::getPlayerOwner()
{
    if (m_unitOwner != nullptr && m_unitOwner->isPlayer())
        return dynamic_cast<Player*>(m_unitOwner);

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////

GuardianSummon::GuardianSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration) {}
GuardianSummon::~GuardianSummon() {}

void GuardianSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setPowerType(POWER_TYPE_MANA);
    setMaxPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setPower(POWER_TYPE_MANA, getPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setLevel(pOwner->getLevel());
    setMaxHealth(getMaxHealth() + 28 + 30 * getLevel());
    setHealth(getMaxHealth());
    SetType(CREATURE_TYPE_GUARDIAN);

    m_aiInterface->Init(this, AI_SCRIPT_PET, pOwner);
    m_aiInterface->setPetOwner(pOwner);

    m_noRespawn = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

CompanionSummon::CompanionSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration) {}
CompanionSummon::~CompanionSummon() {}

void CompanionSummon::Load(CreatureProperties const* properties_, Unit* companionOwner, LocationVector& position, uint32_t spellid, int32_t summonSlot)
{
    Summon::Load(properties_, companionOwner, position, spellid, summonSlot);

    setFaction(35);
    setLevel(1);

    m_aiInterface->Init(this, AI_SCRIPT_PET, companionOwner);
    m_aiInterface->setPetOwner(companionOwner);
    m_aiInterface->setMeleeDisabled(true);

    bInvincible = true;

    removePvpFlag();
    removeFfaPvpFlag();
}

//////////////////////////////////////////////////////////////////////////////////////////

PossessedSummon::PossessedSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration) {}
PossessedSummon::~PossessedSummon() {}

void PossessedSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setLevel(pOwner->getLevel());
    setAItoUse(false);
    stopMoving();
}

//////////////////////////////////////////////////////////////////////////////////////////

WildSummon::WildSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration) {}
WildSummon::~WildSummon() {}

void WildSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setLevel(pOwner->getLevel());
}

//////////////////////////////////////////////////////////////////////////////////////////

TotemSummon::TotemSummon(uint64_t guid, uint32_t duration) : Summon(guid, duration)
{
    // Override initialization from Summon class
    getThreatManager().initialize();
}

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

    m_aiInterface->Init(this, AI_SCRIPT_TOTEM, unitOwner);

    setAItoUse(false);

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
        setAItoUse(true);
        m_aiInterface->totemspell = totemSpell;
        m_aiInterface->m_totemspelltimer = 0;
        m_aiInterface->m_totemspelltime = 3 * TimeVarsMs::Second;
    }
}
