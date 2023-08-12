/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"

#include "SummonHandler.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"

Summon::Summon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties) : Creature(guid), m_Properties(properties)
{
    // Override initialization from Creature class
    getThreatManager().initialize();
}

Summon::~Summon() {}

void Summon::load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellId)
{
    Creature::Load(creatureProperties, position.x, position.y, position.z, position.o);

    setTimeLeft(duration);
    setLifeTime(duration);

    // Despawn Type
    if (m_despawnType == MANUAL_DESPAWN)
        m_despawnType = (duration == 0) ? DEAD_DESPAWN : TIMED_DESPAWN;

    setCreatedBySpellId(spellId);
    SetSpawnLocation(position);

    if (unitOwner)
    {
        m_summonerGuid = unitOwner->getGuid();

        if (!m_Properties)
            return;

        if (uint32_t slot = m_Properties->Slot)
        {
            WoWGuid guid = getGuid();

            if (SummonHandler* summonHandler = unitOwner->getSummonInterface())
            {
                // Unsummon Summon in old Slot
                if (summonHandler->m_SummonSlot[slot] && summonHandler->m_SummonSlot[slot] != guid.getGuidLowPart())
                {
                    Creature* oldSummon = unitOwner->getWorldMap()->getCreature(summonHandler->m_SummonSlot[slot]);
                    if (oldSummon && oldSummon->isSummon())
                        static_cast<Summon*>(oldSummon)->unSummon();
                }
                summonHandler->m_SummonSlot[slot] = guid.getGuidLowPart();
            }
        }

        if (m_Properties->FactionID)
            setFaction(m_Properties->FactionID);
        else if (isVehicle() && unitOwner) // properties should be vehicle
            setFaction(unitOwner->getFactionTemplate());
    }
}

void Summon::Update(unsigned long time_passed)
{
    if (getDeathState() == DEAD)
    {
        unSummon();
        return;
    }

    switch (getDespawnType())
    {
        case MANUAL_DESPAWN:
        case DEAD_DESPAWN:
            break;
        case TIMED_DESPAWN:
        {
            if (getTimeLeft() <= time_passed)
            {
                unSummon();
                return;
            }

            setTimeLeft(getTimeLeft() - time_passed);
        } break;
        case TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!isInCombat())
            {
                if (getTimeLeft() <= time_passed)
                {
                    unSummon();
                    return;
                }

                setTimeLeft(getTimeLeft() - time_passed);
            }
            else if (getTimeLeft() != getLifeTime())
            {
                setTimeLeft(getLifeTime());
            }
        } break;
        case CORPSE_TIMED_DESPAWN:
        {
            if (getDeathState() == CORPSE)
            {
                if (getTimeLeft() <= time_passed)
                {
                    unSummon();
                    return;
                }

                setTimeLeft(getTimeLeft() - time_passed);
            }
        } break;
        case CORPSE_DESPAWN:
        {
            if (getDeathState() == CORPSE)
            {
                unSummon();
                return;
            }
        } break;
        case TIMED_OR_CORPSE_DESPAWN:
        {
            if (getDeathState() == CORPSE)
            {
                unSummon();
                return;
            }

            if (!isInCombat())
            {
                if (getTimeLeft() <= time_passed)
                {
                    unSummon();
                    return;
                }
                else
                {
                    setTimeLeft(getTimeLeft() - time_passed);
                }
            }
            else if (getTimeLeft() != getLifeTime())
            {
                setTimeLeft(getLifeTime());
            }
        } break;
        case TIMED_OR_DEAD_DESPAWN:
        {
            if (!isInCombat() && isAlive())
            {
                if (getTimeLeft() <= time_passed)
                {
                    unSummon();
                    return;
                }
                else
                {
                    setTimeLeft(getTimeLeft() - time_passed);
                }
            }
            else if (getTimeLeft() != getLifeTime())
            {
                setTimeLeft(getLifeTime());
            }

        } break;
        default:
            unSummon();
    }
}

void Summon::unSummon()
{   
    // If this summon is summoned by a totem, unsummon the totem also
    if (getSummonerUnit() && getSummonerUnit()->isTotem())
        dynamic_cast<TotemSummon*>(getSummonerUnit())->unSummon();

    // Script Call
    if (Unit* owner = getSummonerUnit())
    {
        if (owner->ToCreature() && owner->IsInWorld() && owner->ToCreature()->GetScript())
            owner->ToCreature()->GetScript()->OnSummonDespawn(this);
    }

    if (getSummonerUnit())
    {
        if (getCreatedBySpellId() != 0)
            getSummonerUnit()->removeAllAurasById(getCreatedBySpellId());

        // Clear Our Summon Slot
        if (m_Properties)
        {
            if (uint32_t slot = m_Properties->Slot)
            {
                WoWGuid guid = getGuid();
                if (Unit* owner = getSummonerUnit())
                {
                    if (SummonHandler* summonHandler = owner->getSummonInterface())
                    {
                        if (summonHandler->m_SummonSlot[slot] == guid.getGuidLowPart())
                            summonHandler->m_SummonSlot[slot] = 0;
                    }
                }
            }
        }
    }

    // Clear Owner
    m_summonerGuid = 0;

    // Remove us
    Despawn(10, 0);
}

CreatureSummonDespawnType Summon::getDespawnType() const { return m_despawnType; }
void Summon::setDespawnType(CreatureSummonDespawnType type) { m_despawnType = type; }

uint32_t Summon::getTimeLeft() const { return m_duration; }
void Summon::setTimeLeft(uint32_t time) { m_duration = time; }

uint32_t Summon::getLifeTime() const { return m_lifetime; }
void Summon::setLifeTime(uint32_t time) { m_lifetime = time; }

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void Summon::OnPushToWorld()
{
    Creature::OnPushToWorld();
}

void Summon::OnPreRemoveFromWorld()
{
    if (getPlayerOwner() != nullptr)
        getPlayerOwner()->sendDestroyObjectPacket(getGuid());

    // Make sure unit is unsummoned properly before removing from world
    if (m_summonerGuid)
        unSummon();
}

bool Summon::isSummon() const { return true; }

void Summon::onRemoveInRangeObject(Object* object)
{
    // Remove us when we are Summoned by the Object which got removed
    if (object->getGuid() == m_summonerGuid && m_Properties != nullptr)
        unSummon();

    Creature::onRemoveInRangeObject(object);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void Summon::die(Unit* pAttacker, uint32 damage, uint32 spellid)
{
    // If this summon is summoned by a totem, unsummon the totem on death
    if (getUnitOwner() && getUnitOwner()->isTotem())
        static_cast<TotemSummon*>(getUnitOwner())->unSummon();

    Creature::die(pAttacker, damage, spellid);
}

Unit* Summon::getSummonerUnit()
{
    return m_summonerGuid ? getWorldMapUnit(m_summonerGuid) : nullptr;
}

Unit* Summon::getUnitOwner()
{
    return getCreatedByGuid() ? getWorldMapUnit(getCreatedByGuid()) : nullptr;
}

Unit* Summon::getUnitOwnerOrSelf()
{
    if (auto* const unitOwner = getUnitOwner())
        return unitOwner;

    return this;
}

Player* Summon::getPlayerOwner()
{
    if (auto* const unitOwner = getUnitOwner())
    {
        if (unitOwner->isPlayer())
            return dynamic_cast<Player*>(unitOwner);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////

GuardianSummon::GuardianSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
GuardianSummon::~GuardianSummon() {}

void GuardianSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

    // Summoner
    if (pOwner)
    {
        setCreatedByGuid(pOwner->getGuid());
        setSummonedByGuid(pOwner->getGuid());
        setFaction(pOwner->getFactionTemplate());
    }

    // Stats
    setPowerType(POWER_TYPE_MANA);
    setMaxPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setPower(POWER_TYPE_MANA, getPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setLevel(pOwner->getLevel());
    setMaxHealth(getMaxHealth() + 28 + 30 * getLevel());
    setHealth(getMaxHealth());
    SetType(CREATURE_TYPE_GUARDIAN);

    m_aiInterface->Init(this, pOwner);
    m_aiInterface->setPetOwner(pOwner);

    m_noRespawn = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

CompanionSummon::CompanionSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
CompanionSummon::~CompanionSummon() {}

void CompanionSummon::load(CreatureProperties const* properties_, Unit* companionOwner, LocationVector& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, companionOwner, position, duration, spellid);

    // Summoner
    if (companionOwner)
    {
        setCreatedByGuid(companionOwner->getGuid());
        setSummonedByGuid(companionOwner->getGuid());
        setFaction(companionOwner->getFactionTemplate());
    }

    setFaction(35);
    setLevel(1);

    m_aiInterface->Init(this, companionOwner);
    m_aiInterface->setPetOwner(companionOwner);
    m_aiInterface->setMeleeDisabled(true);

    m_isInvincible = true;

    removePvpFlag();
    removeFfaPvpFlag();
}

//////////////////////////////////////////////////////////////////////////////////////////

PossessedSummon::PossessedSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
PossessedSummon::~PossessedSummon() {}

void PossessedSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

    // Summoner
    if (pOwner)
    {
        setCreatedByGuid(pOwner->getGuid());
        setSummonedByGuid(pOwner->getGuid());
        setFaction(pOwner->getFactionTemplate());
    }

    setLevel(pOwner->getLevel());
    setAItoUse(false);
    stopMoving();
}

//////////////////////////////////////////////////////////////////////////////////////////

WildSummon::WildSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
WildSummon::~WildSummon() {}

void WildSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

    // Summoner
    if (pOwner)
    {
        setCreatedByGuid(pOwner->getGuid());
        setSummonedByGuid(pOwner->getGuid());
        setFaction(pOwner->getFactionTemplate());
    }

    setLevel(pOwner->getLevel());
}

//////////////////////////////////////////////////////////////////////////////////////////

TotemSummon::TotemSummon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(guid, properties)
{
    // Override initialization from Summon class
    getThreatManager().initialize();
}

TotemSummon::~TotemSummon() {}

void TotemSummon::load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellId)
{
    Summon::load(creatureProperties, unitOwner, position, duration, spellId);

    const MySQLStructure::TotemDisplayIds* displayIds = nullptr;

    // Summoner
    if (unitOwner)
    {
        setCreatedByGuid(unitOwner->getGuid());
        setSummonedByGuid(unitOwner->getGuid());
        setFaction(unitOwner->getFactionTemplate());

        displayIds = sMySQLStore.getTotemDisplayId(unitOwner->getRace(), creature_properties->Male_DisplayID);

        setLevel(unitOwner->getLevel());
    }
    else
    {
        setLevel(1);
    }

    uint32_t displayId;
    if (displayIds != nullptr)
        displayId = displayIds->race_specific_id;
    else
        displayId = creature_properties->Male_DisplayID;

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

    if (unitOwner)
    {
        for (uint8_t school = 0; school < TOTAL_SPELL_SCHOOLS; school++)
        {
            ModDamageDone[school] = unitOwner->GetDamageDoneMod(school);
            m_healDoneMod[school] = unitOwner->m_healDoneMod[school];
        }

        m_aiInterface->Init(this, unitOwner);

        setAItoUse(false);

        if (unitOwner->isPlayer())
        {
            uint32_t slot = m_Properties->Slot;
            if (slot >= SUMMON_SLOT_TOTEM_FIRE && slot < SUMMON_SLOT_MINIPET)
                dynamic_cast<Player*>(unitOwner)->sendTotemCreatedPacket(static_cast<uint8_t>(slot - SUMMON_SLOT_TOTEM_FIRE), getGuid(), getTimeLeft(), getCreatedBySpellId());
        }
    }
}

void TotemSummon::unSummon()
{
    ///\ todo: death animation for totems, should happen on unsummon
    /*if (isAlive())
        setDeathState(DEAD);*/

    interruptSpell();
    removeAllAuras();

    Summon::unSummon();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void TotemSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();

    setupSpells();
}

bool TotemSummon::isTotem() const { return true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void TotemSummon::die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/)
{
    // Clear health batch on death
    clearHealthBatch();

    unSummon();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void TotemSummon::setupSpells()
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
