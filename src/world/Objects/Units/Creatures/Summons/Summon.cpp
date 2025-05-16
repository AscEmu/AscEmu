/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"

#include "SummonHandler.hpp"
#include "Logging/Logger.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Spell/SpellAura.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include "CommonTime.hpp"

static CreatureSummonDespawnType getDefaultDespawnTypeForSummon(Summon const* summon)
{
    if (summon->getLifeTime() != 0)
    {
        // todo: should scripted summons behave same like summons from spells?
        return summon->isPet() || summon->getSummonProperties() != nullptr
            ? TIMED_OR_CORPSE_DESPAWN
            : TIMED_DESPAWN;
    }
    else
    {
        return DEAD_DESPAWN;
    }
}

Summon::Summon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties) : Creature(guid), m_summonProperties(properties)
{
    // Disable respawn in Creature class
    m_noRespawn = true;
    // Override initialization from Creature class
    getThreatManager().initialize();
}

Summon::~Summon() = default;

void Summon::load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId)
{
    if (!isPet())
        Creature::Load(creatureProperties, position.x, position.y, position.z, position.o);

    setTimeLeft(duration);
    setLifeTime(duration);

    // Despawn Type
    if (m_despawnType == MANUAL_DESPAWN)
        m_despawnType = getDefaultDespawnTypeForSummon(this);

    setCreatedBySpellId(spellId);
    SetSpawnLocation(position);

    m_summonActive = true;

    if (unitOwner == nullptr)
        return;

    setZoneId(unitOwner->getZoneId());

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

    if (unitOwner->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
        addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);

#if VERSION_STRING == TBC
    if (unitOwner->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
        setPositiveAuraLimit(POS_AURA_LIMIT_PVP_ATTACKABLE);
    else
        setPositiveAuraLimit(POS_AURA_LIMIT_CREATURE);
#endif

    m_unitOwner = unitOwner;

    // Non-scripted summons
    if (isPet() || m_summonProperties != nullptr)
    {
        if (m_summonProperties != nullptr && m_summonProperties->FactionID != 0)
            setFaction(m_summonProperties->FactionID);
        else
            setFaction(unitOwner->getFactionTemplate());

        setCreatedByGuid(unitOwner->getGuid());
        if (unitOwner->getSummonedByGuid() != 0)
            setSummonedByGuid(unitOwner->getSummonedByGuid());
        else
            setSummonedByGuid(unitOwner->getGuid());
    }
}

void Summon::Update(unsigned long time_passed)
{
    if (getDeathState() == DEAD)
    {
        unSummon();
        return;
    }

    switch (m_despawnType)
    {
        case DEAD_DESPAWN:
            break;
        case MANUAL_DESPAWN:
        {
            // Set correct despawn type if not set
            m_despawnType = getDefaultDespawnTypeForSummon(this);
        } break;
        case TIMED_DESPAWN:
        {
            if (getTimeLeft() <= time_passed)
            {
                dieAndDisappearOnExpire();
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
                    dieAndDisappearOnExpire();
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
                // There is a small delay before summon should despawn
                m_despawnType = CORPSE_TIMED_DESPAWN;
                if (_hasExtendedDespawnDelayInDeath())
                    setNewLifeTime(SUMMON_CORPSE_DESPAWN_EXTENDED_TIMER);
                else
                    setNewLifeTime(SUMMON_CORPSE_DESPAWN_DEFAULT_TIMER);
                return;
            }

            if (getTimeLeft() <= time_passed)
            {
                dieAndDisappearOnExpire();
                return;
            }

            setTimeLeft(getTimeLeft() - time_passed);
        } break;
        case TIMED_OR_DEAD_DESPAWN:
        {
            if (getDeathState() == DEAD)
            {
                unSummon();
                return;
            }

            if (getTimeLeft() <= time_passed)
            {
                dieAndDisappearOnExpire();
                return;
            }

            setTimeLeft(getTimeLeft() - time_passed);
        } break;
        default:
        {
            unSummon();
        } break;
    }

    // Call superclass method last in case if dead summon is removed from world
    Creature::Update(time_passed);
}

void Summon::unSummon()
{
    _onUnsummon();
    // Remove us
    Despawn(0, 0);
}

void Summon::dieAndDisappearOnExpire()
{
    // Most summons die and disappear shortly after when they expire
    m_despawnType = CORPSE_TIMED_DESPAWN;
    if (_hasExtendedDespawnDelayInDeath())
        setNewLifeTime(SUMMON_CORPSE_DESPAWN_EXTENDED_TIMER);
    else
        setNewLifeTime(SUMMON_CORPSE_DESPAWN_DEFAULT_TIMER);

    if (isAlive())
        die(nullptr, 0, 0);
}

CreatureSummonDespawnType Summon::getDespawnType() const { return m_despawnType; }
void Summon::setDespawnType(CreatureSummonDespawnType type) { m_despawnType = type; }

WDB::Structures::SummonPropertiesEntry const* Summon::getSummonProperties() const { return m_summonProperties; }

bool Summon::isSummonActive() const
{
    return m_summonActive;
}

bool Summon::isPermanentSummon() const
{
    if (m_lifetime == 0)
        return true;

    // Summon may have pre defined corpse remove time so check for despawn type
    return m_despawnType != TIMED_OR_DEAD_DESPAWN && m_despawnType != TIMED_OR_CORPSE_DESPAWN && m_despawnType != TIMED_DESPAWN && m_despawnType != TIMED_DESPAWN_OUT_OF_COMBAT;
}

uint32_t Summon::getTimeLeft() const { return m_duration; }
void Summon::setTimeLeft(uint32_t time) { m_duration = time; }

uint32_t Summon::getLifeTime() const { return m_lifetime; }
void Summon::setLifeTime(uint32_t time) { m_lifetime = time; }

void Summon::setNewLifeTime(uint32_t time)
{
    m_lifetime = time;
    m_duration = time;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void Summon::OnPushToWorld()
{
    // Add summon to owner
    if (m_unitOwner != nullptr && (m_summonProperties != nullptr || isPet()))
        m_unitOwner->getSummonInterface()->addSummonToHandler(this);

    Creature::OnPushToWorld();
}

void Summon::OnPreRemoveFromWorld()
{
    // Make sure unit is unsummoned properly before removing from world
    if (m_summonActive)
        _onUnsummon();

    //if (const auto plrOwner = getPlayerOwner())
        //plrOwner->sendDestroyObjectPacket(getGuid());

    m_unitOwner = nullptr;
    Creature::OnPreRemoveFromWorld();
}

bool Summon::isSummon() const { return true; }

void Summon::onRemoveInRangeObject(Object* object)
{
    if (m_summonProperties != nullptr || isPet())
    {
        // Remove non-scripted summon or pet when it goes out of range from owner
        if (m_unitOwner != nullptr && object->getGuid() == m_unitOwner->getGuid())
        {
            // Delay unsummon a bit to prevent mutex loop in inrange vectors
            sEventMgr.AddEvent(this, &Summon::unSummon, EVENT_PET_DELAYED_REMOVE, 10, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
            return;
        }
    }

    Creature::onRemoveInRangeObject(object);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void Summon::die(Unit* pAttacker, uint32_t damage, uint32_t spellid)
{
    Creature::die(pAttacker, damage, spellid);

    // If this summon is summoned by a totem, unsummon the totem on death
    if (m_unitOwner != nullptr)
    {
        if (auto* const totemOwner = m_unitOwner->isTotem() ? dynamic_cast<TotemSummon*>(m_unitOwner) : nullptr)
        {
            // Prevent unsummon loop if owner is already dead
            // Also delay unsummon a bit to prevent memory corruption
            if (totemOwner->isSummonActive() && totemOwner->isAlive())
                sEventMgr.AddEvent(totemOwner, &TotemSummon::unSummon, EVENT_PET_DELAYED_REMOVE, 10, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }
    }
}

Unit* Summon::getUnitOwner()
{
    return m_unitOwner;
}

Unit const* Summon::getUnitOwner() const
{
    return m_unitOwner;
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

Player const* Summon::getPlayerOwner() const
{
    if (auto* const unitOwner = getUnitOwner())
    {
        if (unitOwner->isPlayer())
            return dynamic_cast<Player const*>(unitOwner);
    }

    return nullptr;
}

void Summon::_onUnsummon()
{
    if (m_unitOwner != nullptr)
    {
        // If this summon is summoned by a totem, unsummon the totem also
        if (auto* const totemOwner = m_unitOwner->isTotem() ? dynamic_cast<TotemSummon*>(m_unitOwner) : nullptr)
        {
            // Prevent unsummon loop if owner is already dead
            if (totemOwner->isSummonActive() && totemOwner->isAlive())
                totemOwner->unSummon();
        }

        // Script Call
        if (m_unitOwner->ToCreature() && m_unitOwner->IsInWorld() && m_unitOwner->ToCreature()->GetScript())
            m_unitOwner->ToCreature()->GetScript()->OnSummonDespawn(this);

        if (getCreatedBySpellId() != 0)
            m_unitOwner->removeAllAurasById(getCreatedBySpellId());

        // Remove summon from owner
        if (m_summonProperties != nullptr || isPet())
            m_unitOwner->getSummonInterface()->removeSummonFromHandler(this);
    }

    // Prevent unsummon loop
    m_summonActive = false;
}

bool Summon::_hasExtendedDespawnDelayInDeath() const
{
    if (m_summonProperties == nullptr)
        return false;

    switch (m_summonProperties->ID)
    {
        // Generic id that is used in many npc summon spells
        case 67:
        // Warlock Inferno uses only this id
        case 711:
        // Warlock Curse of Doom uses only this id
        case 1221:
        // Druid Force of Nature uses only this id
        case 1562:
            return true;
        default:
            break;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

GuardianSummon::GuardianSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
GuardianSummon::~GuardianSummon() = default;

void GuardianSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector const& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

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
}

//////////////////////////////////////////////////////////////////////////////////////////

CompanionSummon::CompanionSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
CompanionSummon::~CompanionSummon() = default;

void CompanionSummon::load(CreatureProperties const* properties_, Unit* companionOwner, LocationVector const& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, companionOwner, position, duration, spellid);

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
PossessedSummon::~PossessedSummon() = default;

void PossessedSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector const& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

    setLevel(pOwner->getLevel());
    setAItoUse(false);
    stopMoving();
}

//////////////////////////////////////////////////////////////////////////////////////////

WildSummon::WildSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(GUID, properties) {}
WildSummon::~WildSummon() = default;

void WildSummon::load(CreatureProperties const* properties_, Unit* pOwner, LocationVector const& position, uint32_t duration, uint32_t spellid)
{
    Summon::load(properties_, pOwner, position, duration, spellid);

    setLevel(pOwner->getLevel());
}

//////////////////////////////////////////////////////////////////////////////////////////

TotemSummon::TotemSummon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(guid, properties)
{
    // Override initialization from Summon class
    getThreatManager().initialize();
}

TotemSummon::~TotemSummon() = default;

void TotemSummon::load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId)
{
    Summon::load(creatureProperties, unitOwner, position, duration, spellId);

    uint32_t displayId = 0;
    if (unitOwner != nullptr)
    {
        if (const auto displayIds = sMySQLStore.getTotemDisplayId(unitOwner->getRace(), creature_properties->Male_DisplayID))
            displayId = displayIds->race_specific_id;
    }

    if (displayId == 0)
        displayId = creature_properties->Male_DisplayID;

    setLevel(unitOwner != nullptr ? unitOwner->getLevel() : 1);
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

    // Totems should always be unsummoned on death or when expired
    m_despawnType = duration == 0 ? CORPSE_DESPAWN : TIMED_OR_CORPSE_DESPAWN;

    if (unitOwner != nullptr)
    {
        for (uint8_t school = 0; school < TOTAL_SPELL_SCHOOLS; school++)
        {
            ModDamageDone[school] = unitOwner->GetDamageDoneMod(school);
            m_healDoneMod[school] = unitOwner->m_healDoneMod[school];
        }

        m_aiInterface->Init(this, unitOwner);

        setAItoUse(false);
    }
}

void TotemSummon::unSummon()
{
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
// Misc
void TotemSummon::setupSpells()
{
    if (getUnitOwner() == nullptr)
        return;

    const auto creatorSpell = sSpellMgr.getSpellInfo(getCreatedBySpellId());
    const auto totemSpell = sSpellMgr.getSpellInfo(creature_properties->AISpells[0]);
    if (totemSpell == nullptr)
    {
        sLogger.debug("Totem {} does not have any spells to cast", creature_properties->Id);
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
