/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Unit.h"

#include "Data/WoWUnit.h"
#include "Map/MapMgr.h"
#include "Players/Player.h"
#include "Server/Packets/SmsgAuraUpdate.h"
#include "Server/Packets/SmsgAuraUpdateAll.h"
#include "Server/Packets/SmsgClearExtraAuraInfo.h"
#include "Server/Packets/SmsgSetExtraAuraInfo.h"
#include "Server/Packets/SmsgSpellEnergizeLog.h"
#include "Server/Packets/SmsgEnvironmentalDamageLog.h"
#include "Server/Packets/SmsgMonsterMoveTransport.h"
#include "Server/Packets/SmsgPeriodicAuraLog.h"
#include "Server/Packets/SmsgPlaySpellVisual.h"
#include "Server/Packets/SmsgPowerUpdate.h"
#include "Server/Packets/SmsgSpellHealLog.h"
#include "Server/Packets/SmsgSpellOrDamageImmune.h"
#include "Server/Packets/SmsgUpdateAuraDuration.h"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Spell/Definitions/DiminishingGroup.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/SpellTypes.h"
#include "Spell/SpellAuras.h"
#include "Spell/SpellHelpers.h"
#include "Spell/SpellMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Vehicle.h"

using namespace AscEmu::Packets;
using AscEmu::World::Spell::Helpers::spellModFlatFloatValue;
using AscEmu::World::Spell::Helpers::spellModFlatIntValue;
using AscEmu::World::Spell::Helpers::spellModPercentageFloatValue;
using AscEmu::World::Spell::Helpers::spellModPercentageIntValue;

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

MovementAI & Unit::getMovementAI()
{
    return m_movementAI;
}

void Unit::setLocationWithoutUpdate(LocationVector & location)
{
    m_position.ChangeCoords({ location.x, location.y, location.z });
}

uint64_t Unit::getCharmGuid() const { return unitData()->charm_guid.guid; };
void Unit::setCharmGuid(uint64_t guid) { write(unitData()->charm_guid.guid, guid); }

uint64_t Unit::getSummonGuid() const { return unitData()->summon_guid.guid; };
void Unit::setSummonGuid(uint64_t guid) { write(unitData()->summon_guid.guid, guid); }

#if VERSION_STRING > TBC
uint64_t Unit::getCritterGuid() const { return unitData()->critter_guid.guid; };
void Unit::setCritterGuid(uint64_t guid) { write(unitData()->critter_guid.guid, guid); }
#endif

uint64_t Unit::getCharmedByGuid() const { return unitData()->charmed_by_guid.guid; };
void Unit::setCharmedByGuid(uint64_t guid) { write(unitData()->charmed_by_guid.guid, guid); }

uint64_t Unit::getSummonedByGuid() const { return unitData()->summoned_by_guid.guid; };
void Unit::setSummonedByGuid(uint64_t guid) { write(unitData()->summoned_by_guid.guid, guid); }

uint64_t Unit::getCreatedByGuid() const { return unitData()->created_by_guid.guid; };
void Unit::setCreatedByGuid(uint64_t guid) { write(unitData()->created_by_guid.guid, guid); }

uint64_t Unit::getTargetGuid() const { return unitData()->target_guid.guid; };
void Unit::setTargetGuid(uint64_t guid) { write(unitData()->target_guid.guid, guid); }

uint64_t Unit::getChannelObjectGuid() const { return unitData()->channel_object_guid.guid; };
void Unit::setChannelObjectGuid(uint64_t guid) { write(unitData()->channel_object_guid.guid, guid); }

uint32_t Unit::getChannelSpellId() const { return unitData()->channel_spell; };
void Unit::setChannelSpellId(uint32_t spell_id) { write(unitData()->channel_spell, spell_id); }

//bytes_0 begin
uint32_t Unit::getBytes0() const { return unitData()->field_bytes_0.raw; }
void Unit::setBytes0(uint32_t bytes) { write(unitData()->field_bytes_0.raw, bytes); }

uint8_t Unit::getBytes0ByOffset(uint32_t offset) const
{
    switch (offset)
    {
        case 0:
            return getRace();
        case 1:
            return getClass();
        case 2:
            return getGender();
        case 3:
            return static_cast<uint8_t>(getPowerType());
        default:
            LogError("Offset %u is not a valid offset value for byte_0 data (max 3). Returning 0", offset);
            return 0;
    }
}

void Unit::setBytes0ForOffset(uint32_t offset, uint8_t value)
{
    switch (offset)
    {
        case 0:
            setRace(value);
            break;
        case 1:
            setClass(value);
            break;
        case 2:
            setGender(value);
            break;
        case 3:
            setPowerType(value);
            break;
        default:
            LogError("Offset %u is not a valid offset value for byte_0 data (max 3)", offset);
            break;
    }
}

uint8_t Unit::getRace() const { return unitData()->field_bytes_0.s.race; }
void Unit::setRace(uint8_t race) { write(unitData()->field_bytes_0.s.race, race); }

uint8_t Unit::getClass() const { return unitData()->field_bytes_0.s.unit_class; }
void Unit::setClass(uint8_t class_) { write(unitData()->field_bytes_0.s.unit_class, class_); }

uint8_t Unit::getGender() const { return unitData()->field_bytes_0.s.gender; }
void Unit::setGender(uint8_t gender) { write(unitData()->field_bytes_0.s.gender, gender); }

PowerType Unit::getPowerType() const { return static_cast<PowerType>(unitData()->field_bytes_0.s.power_type); }
void Unit::setPowerType(uint8_t powerType)
{
    write(unitData()->field_bytes_0.s.power_type, powerType);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update power type also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_POWER_TYPE : GROUP_UPDATE_FLAG_PET_POWER_TYPE);
}
//bytes_0 end

uint32_t Unit::getHealth() const { return unitData()->health; }
void Unit::setHealth(uint32_t health)
{
    const auto maxHealth = getMaxHealth();
    if (health > maxHealth)
        health = maxHealth;

    write(unitData()->health, health);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update health also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_CUR_HP : GROUP_UPDATE_FLAG_PET_CUR_HP);
}
void Unit::modHealth(int32_t health)
{
    int32_t newHealth = getHealth();
    newHealth += health;

    if (newHealth < 0)
        newHealth = 0;

    setHealth(newHealth);
}

uint32_t Unit::getPower(PowerType type) const
{
    if (type == POWER_TYPE_HEALTH)
        return getHealth();

    // Since cata power fields work differently
    // Get matching power index by power type
    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
            return unitData()->power_1;
        case POWER_FIELD_INDEX_2:
            return unitData()->power_2;
        case POWER_FIELD_INDEX_3:
            return unitData()->power_3;
        case POWER_FIELD_INDEX_4:
            return unitData()->power_4;
        case POWER_FIELD_INDEX_5:
            return unitData()->power_5;
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
            return unitData()->power_6;
        case POWER_FIELD_INDEX_7:
            return unitData()->power_7;
#endif
        default:
            return 0;
    }
}

void Unit::setPower(PowerType type, uint32_t value, bool sendPacket/* = true*/)
{
    if (type == POWER_TYPE_HEALTH)
    {
        setHealth(value);
        return;
    }

    const auto maxPower = getMaxPower(type);
    if (value > maxPower)
        value = maxPower;

    if (getPower(type) == value)
        return;

    // Since cata power fields work differently
    // Get matching power index by power type
    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
            write(unitData()->power_1, value);
            break;
        case POWER_FIELD_INDEX_2:
            write(unitData()->power_2, value);
            break;
        case POWER_FIELD_INDEX_3:
            write(unitData()->power_3, value);
            break;
        case POWER_FIELD_INDEX_4:
            write(unitData()->power_4, value);
            break;
        case POWER_FIELD_INDEX_5:
            write(unitData()->power_5, value);
            break;
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
            write(unitData()->power_6, value);
            break;
        case POWER_FIELD_INDEX_7:
            write(unitData()->power_7, value);
            break;
#endif
        default:
            return;
    }

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Send power update to client
    if (sendPacket)
        sendPowerUpdate(isPlayer());

    // Update power also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_CUR_POWER : GROUP_UPDATE_FLAG_PET_CUR_POWER);
}

void Unit::modPower(PowerType type, int32_t value)
{
    int32_t newPower = getPower(type);
    newPower += value;

    if (newPower < 0)
        newPower = 0;

    setPower(type, newPower);
}

uint32_t Unit::getMaxHealth() const { return unitData()->max_health; }
void Unit::setMaxHealth(uint32_t maxHealth)
{
    write(unitData()->max_health, maxHealth);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update health also to group
    const auto plr = getPlayerOwner();
    if (plr != nullptr && plr->IsInWorld() && plr->GetGroup() != nullptr)
        plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_MAX_HP : GROUP_UPDATE_FLAG_PET_MAX_HP);

    if (maxHealth < getHealth())
        setHealth(maxHealth);
}
void Unit::modMaxHealth(int32_t maxHealth)
{
    int32_t newMaxHealth = getMaxHealth();
    newMaxHealth += maxHealth;

    if (newMaxHealth < 0)
        newMaxHealth = 0;

    setMaxHealth(newMaxHealth);
}

uint32_t Unit::getMaxPower(PowerType type) const
{
    if (type == POWER_TYPE_HEALTH)
        return getMaxHealth();

    // Since cata power fields work differently
    // Get matching power index by power type
    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
            return unitData()->max_power_1;
        case POWER_FIELD_INDEX_2:
            return unitData()->max_power_2;
        case POWER_FIELD_INDEX_3:
            return unitData()->max_power_3;
        case POWER_FIELD_INDEX_4:
            return unitData()->max_power_4;
        case POWER_FIELD_INDEX_5:
            return unitData()->max_power_5;
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
            return unitData()->max_power_6;
        case POWER_FIELD_INDEX_7:
            return unitData()->max_power_7;
#endif
        default:
            return 0;
    }
}

void Unit::setMaxPower(PowerType type, uint32_t value)
{
    if (type == POWER_TYPE_HEALTH)
    {
        setMaxHealth(value);
        return;
    }

    // Since cata power fields work differently
    // Get matching power index by power type
    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
            write(unitData()->max_power_1, value);
            break;
        case POWER_FIELD_INDEX_2:
            write(unitData()->max_power_2, value);
            break;
        case POWER_FIELD_INDEX_3:
            write(unitData()->max_power_3, value);
            break;
        case POWER_FIELD_INDEX_4:
            write(unitData()->max_power_4, value);
            break;
        case POWER_FIELD_INDEX_5:
            write(unitData()->max_power_5, value);
            break;
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
            write(unitData()->max_power_6, value);
            break;
        case POWER_FIELD_INDEX_7:
            write(unitData()->max_power_7, value);
            break;
#endif
        default:
            return;
    }

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update power also to group
    const auto plr = getPlayerOwner();
    if (plr != nullptr && plr->IsInWorld() && plr->GetGroup() != nullptr)
        plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_MAX_POWER : GROUP_UPDATE_FLAG_PET_MAX_POWER);

    if (value < getPower(type))
        setPower(type, value);
}

void Unit::modMaxPower(PowerType type, int32_t value)
{
    int32_t newValue = getMaxPower(type);
    newValue += value;

    if (newValue < 0)
        newValue = 0;

    setMaxPower(type, newValue);
}

#if VERSION_STRING >= WotLK
float Unit::getPowerRegeneration(PowerType type) const
{
    if (type == POWER_TYPE_HEALTH)
        return 0.0f;

    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
        case POWER_FIELD_INDEX_2:
        case POWER_FIELD_INDEX_3:
        case POWER_FIELD_INDEX_4:
        case POWER_FIELD_INDEX_5:
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
        case POWER_FIELD_INDEX_7:
#endif
            return unitData()->power_regen_flat_modifier[powerIndex - 1];
        default:
            return 0.0f;
    }
}

void Unit::setPowerRegeneration(PowerType type, float value)
{
    if (type == POWER_TYPE_HEALTH)
        return;

    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
        case POWER_FIELD_INDEX_2:
        case POWER_FIELD_INDEX_3:
        case POWER_FIELD_INDEX_4:
        case POWER_FIELD_INDEX_5:
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
        case POWER_FIELD_INDEX_7:
#endif
            write(unitData()->power_regen_flat_modifier[powerIndex - 1], value);
            break;
        default:
            break;
    }
}

float Unit::getManaRegeneration() const { return getPowerRegeneration(POWER_TYPE_MANA); }
void Unit::setManaRegeneration(float value) { setPowerRegeneration(POWER_TYPE_MANA, value); }

float Unit::getPowerRegenerationWhileCasting(PowerType type) const
{
    if (type == POWER_TYPE_HEALTH)
        return 0.0f;

    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
        case POWER_FIELD_INDEX_2:
        case POWER_FIELD_INDEX_3:
        case POWER_FIELD_INDEX_4:
        case POWER_FIELD_INDEX_5:
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
        case POWER_FIELD_INDEX_7:
#endif
            return unitData()->power_regen_interrupted_flat_modifier[powerIndex - 1];
        default:
            return 0.0f;
    }
}

void Unit::setPowerRegenerationWhileCasting(PowerType type, float value)
{
    if (type == POWER_TYPE_HEALTH)
        return;

    const auto powerIndex = getPowerIndexFromDBC(type);
    switch (powerIndex)
    {
        case POWER_FIELD_INDEX_1:
        case POWER_FIELD_INDEX_2:
        case POWER_FIELD_INDEX_3:
        case POWER_FIELD_INDEX_4:
        case POWER_FIELD_INDEX_5:
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
        case POWER_FIELD_INDEX_7:
#endif
            write(unitData()->power_regen_interrupted_flat_modifier[powerIndex - 1], value);
            break;
        default:
            break;
    }
}

float Unit::getManaRegenerationWhileCasting() const { return getPowerRegenerationWhileCasting(POWER_TYPE_MANA); }
void Unit::setManaRegenerationWhileCasting(float value) { setPowerRegenerationWhileCasting(POWER_TYPE_MANA, value); }
#endif

uint32_t Unit::getLevel() const { return unitData()->level; }
void Unit::setLevel(uint32_t level)
{
    write(unitData()->level, level);
    if (isPlayer())
        static_cast<Player*>(this)->setNextLevelXp(sMySQLStore.getPlayerXPForLevel(level));

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update level also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    //\ todo: missing update flag for pet level
    plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_LEVEL : 0);
}

uint32_t Unit::getFactionTemplate() const { return unitData()->faction_template; }
void Unit::setFactionTemplate(uint32_t id) { write(unitData()->faction_template, id); }

uint32_t Unit::getVirtualItemSlotId(uint8_t slot) const { return unitData()->virtual_item_slot_display[slot]; }
void Unit::setVirtualItemSlotId(uint8_t slot, uint32_t item_id) { write(unitData()->virtual_item_slot_display[slot], item_id); }

#if VERSION_STRING < WotLK
uint32_t Unit::getVirtualItemInfo(uint8_t offset) const { return unitData()->virtual_item_info[offset]; }
void Unit::setVirtualItemInfo(uint8_t offset, uint32_t item_info) { write(unitData()->virtual_item_info[offset], item_info); }
#endif

uint32_t Unit::getUnitFlags() const { return unitData()->unit_flags; }
void Unit::setUnitFlags(uint32_t unitFlags) { write(unitData()->unit_flags, unitFlags); }
void Unit::addUnitFlags(uint32_t unitFlags) { setUnitFlags(getUnitFlags() | unitFlags); }
void Unit::removeUnitFlags(uint32_t unitFlags) { setUnitFlags(getUnitFlags() & ~unitFlags); }
bool Unit::hasUnitFlags(uint32_t unitFlags) const { return (getUnitFlags() & unitFlags) != 0; }

#if VERSION_STRING > Classic
uint32_t Unit::getUnitFlags2() const { return unitData()->unit_flags_2; }
void Unit::setUnitFlags2(uint32_t unitFlags2) { write(unitData()->unit_flags_2, unitFlags2); }
void Unit::addUnitFlags2(uint32_t unitFlags2) { setUnitFlags2(getUnitFlags2() | unitFlags2); }
void Unit::removeUnitFlags2(uint32_t unitFlags2) { setUnitFlags2(getUnitFlags2() & ~unitFlags2); }
bool Unit::hasUnitFlags2(uint32_t unitFlags2) const { return (getUnitFlags2() & unitFlags2) != 0; }
#endif

#if VERSION_STRING < WotLK
uint32_t Unit::getAura(uint8_t slot) const { return unitData()->aura[slot]; }
void Unit::setAura(uint8_t slot, uint32_t spellId) { write(unitData()->aura[slot], spellId); }

uint32_t Unit::getAuraFlags(uint8_t slot) const { return unitData()->aura_flags[slot]; }
void Unit::setAuraFlags(uint8_t slot, uint32_t flags)
{
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    auto val = getAuraFlags(index);
    val &= ~(static_cast<uint32_t>(AFLAG_MASK_ALL) << byte);

    if (flags != 0)
        val |= (flags << byte);

    write(unitData()->aura_flags[index], val);
}

uint32_t Unit::getAuraLevel(uint8_t slot) const { return unitData()->aura_levels[slot]; }
void Unit::setAuraLevel(uint8_t slot, uint32_t level)
{
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    auto val = getAuraLevel(index);
    val &= ~(0xFF << byte);
    val |= (level << byte);

    write(unitData()->aura_levels[index], val);
}

uint32_t Unit::getAuraApplication(uint8_t slot) const { return unitData()->aura_applications[slot]; }
void Unit::setAuraApplication(uint8_t slot, uint32_t count)
{
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    // Client expects count - 1
    count = count <= 255 ? count - 1 : 255 - 1;

    auto val = getAuraApplication(index);
    val &= ~(0xFF << byte);
    val |= (static_cast<uint8_t>(count) << byte);

    write(unitData()->aura_applications[index], val);
}
#endif

uint32_t Unit::getAuraState() const { return unitData()->aura_state; }
void Unit::setAuraState(uint32_t state) { write(unitData()->aura_state, state); }
void Unit::addAuraState(uint32_t state) { setAuraState(getAuraState() | state); }
void Unit::removeAuraState(uint32_t state) { setAuraState(getAuraState() & ~state); }

uint32_t Unit::getBaseAttackTime(uint8_t slot) const { return unitData()->base_attack_time[slot]; }
void Unit::setBaseAttackTime(uint8_t slot, uint32_t time) { write(unitData()->base_attack_time[slot], time); }
void Unit::modBaseAttackTime(uint8_t slot, int32_t modTime)
{
    int32_t newAttackTime = getBaseAttackTime(slot);
    newAttackTime += modTime;

    if (newAttackTime < 0)
        newAttackTime = 0;

    setBaseAttackTime(slot, newAttackTime);
}

float Unit::getBoundingRadius() const { return unitData()->bounding_radius; }
void Unit::setBoundingRadius(float radius) { write(unitData()->bounding_radius, radius); }

float Unit::getCombatReach() const { return unitData()->combat_reach; }
void Unit::setCombatReach(float radius) { write(unitData()->combat_reach, radius); }

uint32_t Unit::getDisplayId() const { return unitData()->display_id; }
void Unit::setDisplayId(uint32_t id)
{
    write(unitData()->display_id, id);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update display id also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    //\ todo: missing update flag for player display id
    plr->AddGroupUpdateFlag(isPlayer() ? 0 : GROUP_UPDATE_FLAG_PET_MODEL_ID);
}

uint32_t Unit::getNativeDisplayId() const { return unitData()->native_display_id; }
void Unit::setNativeDisplayId(uint32_t id) { write(unitData()->native_display_id, id); }

uint32_t Unit::getMountDisplayId() const { return unitData()->mount_display_id; }
void Unit::setMountDisplayId(uint32_t id) { write(unitData()->mount_display_id, id); }

float Unit::getMinDamage() const { return unitData()->minimum_damage; }
void Unit::setMinDamage(float damage) { write(unitData()->minimum_damage, damage); }

float Unit::getMaxDamage() const { return unitData()->maximum_damage; }
void Unit::setMaxDamage(float damage) { write(unitData()->maximum_damage, damage); }

float Unit::getMinOffhandDamage() const { return unitData()->minimum_offhand_damage; }
void Unit::setMinOffhandDamage(float damage) { write(unitData()->minimum_offhand_damage, damage); }

float Unit::getMaxOffhandDamage() const { return unitData()->maximum_offhand_damage; }
void Unit::setMaxOffhandDamage(float damage) { write(unitData()->maximum_offhand_damage, damage); }

//bytes_1 begin
uint32_t Unit::getBytes1() const { return unitData()->field_bytes_1.raw; }
void Unit::setBytes1(uint32_t bytes) { write(unitData()->field_bytes_1.raw, bytes); }

uint8_t Unit::getBytes1ByOffset(uint32_t offset) const
{
    switch (offset)
    {
        case 0:
            return getStandState();
        case 1:
            return getPetTalentPoints();
        case 2:
            return getStandStateFlags();
        case 3:
            return getAnimationFlags();
        default:
            LogError("Offset %u is not a valid offset value for byte_1 data (max 3). Returning 0", offset);
            return 0;
    }
}

void Unit::setBytes1ForOffset(uint32_t offset, uint8_t value)
{
    switch (offset)
    {
        case 0:
            setStandState(value);
            break;
        case 1:
            setPetTalentPoints(value);
            break;
        case 2:
            setStandStateFlags(value);
            break;
        case 3:
            setAnimationFlags(value);
            break;
        default:
            LogError("Offset %u is not a valid offset value for byte_1 data (max 3)", offset);
            break;
    }
}

uint8_t Unit::getStandState() const { return unitData()->field_bytes_1.s.stand_state; }
void Unit::setStandState(uint8_t standState) { write(unitData()->field_bytes_1.s.stand_state, standState); }

uint8_t Unit::getPetTalentPoints() const { return unitData()->field_bytes_1.s.pet_talent_points; }
void Unit::setPetTalentPoints(uint8_t talentPoints) { write(unitData()->field_bytes_1.s.pet_talent_points, talentPoints); }

uint8_t Unit::getStandStateFlags() const { return unitData()->field_bytes_1.s.stand_state_flag; }
void Unit::setStandStateFlags(uint8_t standStateFlags) { write(unitData()->field_bytes_1.s.stand_state_flag, standStateFlags); }

uint8_t Unit::getAnimationFlags() const { return unitData()->field_bytes_1.s.animation_flag; }
void Unit::setAnimationFlags(uint8_t animationFlags) { write(unitData()->field_bytes_1.s.animation_flag, animationFlags); }
//bytes_1 end

uint32_t Unit::getPetNumber() const { return unitData()->pet_number; }
void Unit::setPetNumber(uint32_t number) { write(unitData()->pet_number, number); }

uint32_t Unit::getPetNameTimestamp() const { return unitData()->pet_name_timestamp; }
void Unit::setPetNameTimestamp(uint32_t timestamp) { write(unitData()->pet_name_timestamp, timestamp); }

uint32_t Unit::getPetExperience() const { return unitData()->pet_experience; }
void Unit::setPetExperience(uint32_t experience) { write(unitData()->pet_experience, experience); }

uint32_t Unit::getPetNextLevelExperience() const { return unitData()->pet_next_level_experience; }
void Unit::setPetNextLevelExperience(uint32_t experience) { write(unitData()->pet_next_level_experience, experience); }

uint32_t Unit::getDynamicFlags() const { return unitData()->dynamic_flags; }
void Unit::setDynamicFlags(uint32_t dynamicFlags) { write(unitData()->dynamic_flags, dynamicFlags); }
void Unit::addDynamicFlags(uint32_t dynamicFlags) { setDynamicFlags(getDynamicFlags() | dynamicFlags); }
void Unit::removeDynamicFlags(uint32_t dynamicFlags) { setDynamicFlags(getDynamicFlags() & ~dynamicFlags); }

float Unit::getModCastSpeed() const { return unitData()->mod_cast_speed; }
void Unit::setModCastSpeed(float modifier) { write(unitData()->mod_cast_speed, modifier); }
void Unit::modModCastSpeed(float modifier)
{
    float currentMod = getModCastSpeed();
    currentMod += modifier;
    setModCastSpeed(currentMod);
}

uint32_t Unit::getCreatedBySpellId() const { return unitData()->created_by_spell_id; }
void Unit::setCreatedBySpellId(uint32_t id) { write(unitData()->created_by_spell_id, id); }

uint32_t Unit::getNpcFlags() const { return unitData()->npc_flags; }
void Unit::setNpcFlags(uint32_t npcFlags) { write(unitData()->npc_flags, npcFlags); }
void Unit::addNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() | npcFlags); }
void Unit::removeNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() & ~npcFlags); }

uint32_t Unit::getEmoteState() const { return unitData()->npc_emote_state; }
void Unit::setEmoteState(uint32_t id) { write(unitData()->npc_emote_state, id); }

uint32_t Unit::getStat(uint8_t stat) const { return unitData()->stat[stat]; }
void Unit::setStat(uint8_t stat, uint32_t value) { write(unitData()->stat[stat], value); }

#if VERSION_STRING > Classic
uint32_t Unit::getPosStat(uint8_t stat) const { return unitData()->positive_stat[stat]; }
void Unit::setPosStat(uint8_t stat, uint32_t value) { write(unitData()->positive_stat[stat], value); }

uint32_t Unit::getNegStat(uint8_t stat) const { return unitData()->negative_stat[stat]; }
void Unit::setNegStat(uint8_t stat, uint32_t value) { write(unitData()->negative_stat[stat], value); }
#endif

uint32_t Unit::getResistance(uint8_t type) const { return unitData()->resistance[type]; }
void Unit::setResistance(uint8_t type, uint32_t value) { write(unitData()->resistance[type], value); }

#if VERSION_STRING > Classic
uint32_t Unit::getResistanceBuffModPositive(uint8_t type) const { return unitData()->resistance_buff_mod_positive[type]; }
void Unit::setResistanceBuffModPositive(uint8_t type, uint32_t value) { write(unitData()->resistance_buff_mod_positive[type], value); }

uint32_t Unit::getResistanceBuffModNegative(uint8_t type) const { return unitData()->resistance_buff_mod_negative[type]; }
void Unit::setResistanceBuffModNegative(uint8_t type, uint32_t value) { write(unitData()->resistance_buff_mod_negative[type], value); }
#endif

uint32_t Unit::getBaseMana() const { return unitData()->base_mana; }
void Unit::setBaseMana(uint32_t baseMana) { write(unitData()->base_mana, baseMana); }

uint32_t Unit::getBaseHealth() const { return unitData()->base_health; }
void Unit::setBaseHealth(uint32_t baseHealth) { write(unitData()->base_health, baseHealth); }

//byte_2 begin
uint32_t Unit::getBytes2() const { return unitData()->field_bytes_2.raw; }
void Unit::setBytes2(uint32_t bytes) { write(unitData()->field_bytes_2.raw, bytes); }

uint8_t Unit::getBytes2ByOffset(uint32_t offset) const
{
    switch (offset)
    {
        case 0:
            return getSheathType();
        case 1:
            return getPvpFlags();
        case 2:
            return getPetFlags();
        case 3:
            return getShapeShiftForm();
        default:
            LogError("Offset %u is not a valid offset value for byte_2 data (max 3). Returning 0", offset);
            return 0;
    }
}

void Unit::setBytes2ForOffset(uint32_t offset, uint8_t value)
{
    switch (offset)
    {
        case 0:
            setSheathType(value);
            break;
        case 1:
            setPvpFlags(value);
            break;
        case 2:
            setPetFlags(value);
            break;
        case 3:
            setShapeShiftForm(value);
            break;
        default:
            LogError("Offset %u is not a valid offset value for byte_2 data (max 3)", offset);
            break;
    }
}

uint8_t Unit::getSheathType() const { return unitData()->field_bytes_2.s.sheath_type; }
void Unit::setSheathType(uint8_t sheathType) { write(unitData()->field_bytes_2.s.sheath_type, sheathType); }

uint8_t Unit::getPvpFlags() const { return unitData()->field_bytes_2.s.pvp_flag; }
void Unit::setPvpFlags(uint8_t pvpFlags)
{
    write(unitData()->field_bytes_2.s.pvp_flag, pvpFlags);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update pvp flags also to group
    const auto plr = getPlayerOwner();
    if (plr == nullptr || !plr->IsInWorld() || plr->GetGroup() == nullptr)
        return;

    plr->AddGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_STATUS : 0);
}

uint8_t Unit::getPetFlags() const { return unitData()->field_bytes_2.s.pet_flag; }
void Unit::setPetFlags(uint8_t petFlags) { write(unitData()->field_bytes_2.s.pet_flag, petFlags); }

uint8_t Unit::getShapeShiftForm() const { return unitData()->field_bytes_2.s.shape_shift_form; }
void Unit::setShapeShiftForm(uint8_t shapeShiftForm) { write(unitData()->field_bytes_2.s.shape_shift_form, shapeShiftForm); }
//bytes_2 end

uint32_t Unit::getAttackPower() const { return unitData()->attack_power; }
void Unit::setAttackPower(uint32_t value) { write(unitData()->attack_power, value); }

int32_t Unit::getRangedAttackPower() const { return unitData()->ranged_attack_power; }
void Unit::setRangedAttackPower(int32_t power) { write(unitData()->ranged_attack_power, power); }

float Unit::getMinRangedDamage() const { return unitData()->minimum_ranged_damage; }
void Unit::setMinRangedDamage(float damage) { write(unitData()->minimum_ranged_damage, damage); }

float Unit::getMaxRangedDamage() const { return unitData()->maximum_ranged_ddamage; }
void Unit::setMaxRangedDamage(float damage) { write(unitData()->maximum_ranged_ddamage, damage); }

uint32_t Unit::getPowerCostModifier(uint16_t school) const { return unitData()->power_cost_modifier[school]; }
void Unit::setPowerCostModifier(uint16_t school, uint32_t modifier) { write(unitData()->power_cost_modifier[school], modifier); }
void Unit::modPowerCostModifier(uint16_t school, int32_t modifier)
{
    int32_t currentModifier = getPowerCostModifier(school);
    currentModifier += modifier;

    if (currentModifier < 0)
        currentModifier = 0;

    setPowerCostModifier(school, currentModifier);
}

float Unit::getPowerCostMultiplier(uint16_t school) const { return unitData()->power_cost_multiplier[school]; }
void Unit::setPowerCostMultiplier(uint16_t school, float multiplier) { write(unitData()->power_cost_multiplier[school], multiplier); }
void Unit::modPowerCostMultiplier(uint16_t school, float multiplier)
{
    float currentMultiplier = getPowerCostMultiplier(school);
    currentMultiplier += multiplier;
    setPowerCostMultiplier(school, currentMultiplier);
}

int32_t Unit::getAttackPowerMods() const
{
#if VERSION_STRING < Cata
    return unitData()->attack_power_mods;
#else
    return unitData()->attack_power_mod_pos - unitData()->attack_power_mod_neg;
#endif
}

void Unit::setAttackPowerMods(int32_t modifier)
{
#if VERSION_STRING < Cata
    write(unitData()->attack_power_mods, modifier);
#else
    write(unitData()->attack_power_mod_neg, static_cast<uint32_t>(modifier < 0 ? modifier : 0));
    write(unitData()->attack_power_mod_pos, static_cast<uint32_t>(modifier > 0 ? modifier : 0));
#endif
}

void Unit::modAttackPowerMods(int32_t modifier)
{
#if VERSION_STRING < Cata
    int32_t currentModifier = getAttackPowerMods();
    currentModifier += modifier;
    setAttackPowerMods(currentModifier);
#else
    if (modifier == 0) { return; }
#endif
}

float Unit::getAttackPowerMultiplier() const { return unitData()->attack_power_multiplier; }
void Unit::setAttackPowerMultiplier(float multiplier) { write(unitData()->attack_power_multiplier, multiplier); }
void Unit::modAttackPowerMultiplier(float multiplier)
{
    float currentMultiplier = getAttackPowerMultiplier();
    currentMultiplier += multiplier;
    setAttackPowerMultiplier(currentMultiplier);
}

int32_t Unit::getRangedAttackPowerMods() const
{
#if VERSION_STRING < Cata
    return unitData()->ranged_attack_power_mods;
#else
    return unitData()->ranged_attack_power_mods_pos - unitData()->ranged_attack_power_mods_neg;
#endif
}

void Unit::setRangedAttackPowerMods(int32_t modifier)
{
#if VERSION_STRING < Cata
    write(unitData()->ranged_attack_power_mods, modifier);
#else
    write(unitData()->ranged_attack_power_mods_neg, static_cast<uint32_t>(modifier < 0 ? modifier : 0));
    write(unitData()->ranged_attack_power_mods_pos, static_cast<uint32_t>(modifier > 0 ? modifier : 0));
#endif
}

void Unit::modRangedAttackPowerMods(int32_t modifier)
{
#if VERSION_STRING < Cata
    int32_t currentModifier = getRangedAttackPowerMods();
    currentModifier += modifier;
    setRangedAttackPowerMods(currentModifier);
#else
    if (modifier == 0) { return; }
#endif
}

float Unit::getRangedAttackPowerMultiplier() const { return unitData()->ranged_attack_power_multiplier; }
void Unit::setRangedAttackPowerMultiplier(float multiplier) { write(unitData()->ranged_attack_power_multiplier, multiplier); }
void Unit::modRangedAttackPowerMultiplier(float multiplier)
{
    float currentMultiplier = getRangedAttackPowerMultiplier();
    currentMultiplier += multiplier;
    setRangedAttackPowerMultiplier(currentMultiplier);
}

#if VERSION_STRING >= WotLK
float Unit::getHoverHeight() const { return unitData()->hover_height; }
void Unit::setHoverHeight(float height) { write(unitData()->hover_height, height); }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

void Unit::setMoveWaterWalk()
{
    addUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_WATER_WALK, 12);
#if VERSION_STRING < Cata
        data << GetNewGUID();
        data << uint32(0);
#else
        movement_info.writeMovementInfo(data, SMSG_MOVE_WATER_WALK);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_WATER_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_WATER_WALK);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveLandWalk()
{
    removeUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_LAND_WALK, 12);
#if VERSION_STRING < Cata
        data << GetNewGUID();
        data << uint32(0);
#else
        movement_info.writeMovementInfo(data, SMSG_MOVE_LAND_WALK);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_LAND_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_LAND_WALK);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveFeatherFall()
{
    addUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_FEATHER_FALL, 12);
#if VERSION_STRING < Cata
        data << GetNewGUID();
        data << uint32(0);
#else
        movement_info.writeMovementInfo(data, SMSG_MOVE_FEATHER_FALL);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_FEATHER_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_FEATHER_FALL);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveNormalFall()
{
    removeUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_NORMAL_FALL, 12);
#if VERSION_STRING < Cata
        data << GetNewGUID();
        data << uint32(0);
#else
        movement_info.writeMovementInfo(data, SMSG_MOVE_NORMAL_FALL);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_NORMAL_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_NORMAL_FALL);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveHover(bool set_hover)
{
    if (isPlayer())
    {
        if (set_hover)
        {
            addUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_SET_HOVER);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_HOVER);
#endif
            SendMessageToSet(&data, true);
        }
    }

    //\todo spline update
    if (isCreature())
    {
        if (set_hover)
        {
            addUnitMovementFlag(MOVEFLAG_HOVER);

            setAnimationFlags(UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_HOVER, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_HOVER);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_HOVER);

            setAnimationFlags(getAnimationFlags() &~UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_HOVER, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNSET_HOVER);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::setMoveCanFly(bool set_fly)
{
    if (isPlayer())
    {
        if (set_fly)
        {
            addUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            removeUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(2);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_SET_CAN_FLY);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            // Remove all fly related moveflags
            removeUnitMovementFlag(MOVEFLAG_CAN_FLY);
            removeUnitMovementFlag(MOVEFLAG_DESCENDING);
            removeUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(5);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_CAN_FLY);
#endif
            SendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (set_fly)
        {
            addUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            removeUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_FLYING);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            // Remove all fly related moveflags
            removeUnitMovementFlag(MOVEFLAG_CAN_FLY);
            removeUnitMovementFlag(MOVEFLAG_DESCENDING);
            removeUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNSET_FLYING);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::setMoveRoot(bool set_root)
{
    if (isPlayer())
    {
        if (set_root)
        {
            addUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 12);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_ROOT);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 12);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_UNROOT);
#endif
            SendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (set_root)
        {
            // AIInterface
            //\todo stop movement based on movement flag instead of m_canMove
            m_aiInterface->m_canMove = false;
            m_aiInterface->StopMovement(100);

            addUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_ROOT, 9);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_ROOT);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            m_aiInterface->m_canMove = true;

            removeUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 9);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNROOT);
#endif
            SendMessageToSet(&data, true);
        }
    }
}

bool Unit::isRooted() const
{
    return hasUnitMovementFlag(MOVEFLAG_ROOTED);
}

void Unit::setMoveSwim(bool set_swim)
{
    if (isCreature())
    {
        if (set_swim)
        {
            addUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_START_SWIM, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_START_SWIM);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_STOP_SWIM, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_STOP_SWIM);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::setMoveDisableGravity(bool disable_gravity)
{
#if VERSION_STRING > TBC
    if (isPlayer())
    {
        if (disable_gravity)
        {
            addUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_DISABLE, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_DISABLE);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_ENABLE, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_ENABLE);
#endif
            SendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (disable_gravity)
        {
            addUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_DISABLE, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_DISABLE);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_ENABLE);
#endif
            SendMessageToSet(&data, false);
        }
    }
#endif
}

//\todo Zyres: call it if creature has MoveFlag in its movement info (set in Object::_BuildMovementUpdate)
//             Unfortunately Movement and object update is a mess.
void Unit::setMoveWalk(bool set_walk)
{
    if (isCreature())
    {
        if (set_walk)
        {
            addUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_WALK_MODE, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_WALK_MODE);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_RUN_MODE, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_RUN_MODE);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

float Unit::getSpeedRate(UnitSpeedType type, bool current) const
{
    if (current)
        return m_UnitSpeedInfo.m_currentSpeedRate[type];
    else
        return m_UnitSpeedInfo.m_basicSpeedRate[type];
}

void Unit::setSpeedRate(UnitSpeedType type, float value, bool current)
{
    if (current)
        m_UnitSpeedInfo.m_currentSpeedRate[type] = value;
    else
        m_UnitSpeedInfo.m_basicSpeedRate[type] = value;

    Player* player_mover = GetMapMgrPlayer(getCharmedByGuid());
    if (player_mover == nullptr)
    {
        if (isPlayer())
            player_mover = dynamic_cast<Player*>(this);
    }

    if (player_mover != nullptr)
    {
#if VERSION_STRING < Cata
        player_mover->sendForceMovePacket(type, value);
#endif
        player_mover->sendMoveSetSpeedPaket(type, value);
    }
    else
        sendMoveSplinePaket(type);
}

void Unit::resetCurrentSpeeds()
{
    for (uint8_t i = 0; i < MAX_SPEED_TYPE; ++i)
        m_UnitSpeedInfo.m_currentSpeedRate[i] = m_UnitSpeedInfo.m_basicSpeedRate[i];
}

UnitSpeedType Unit::getFastestSpeedType() const
{
    float fastest_speed = 0.f;
    UnitSpeedType fastest_speed_type = TYPE_WALK;
    for (uint8_t i = TYPE_WALK; i < MAX_SPEED_TYPE; ++i)
    {
        UnitSpeedType const speedType = static_cast<UnitSpeedType>(i + 1);

        switch (speedType)
        {
        case TYPE_TURN_RATE:
        case TYPE_PITCH_RATE:
            continue;
        default:
            break;
        }

        float const speed = getSpeedRate(speedType, true);

        fastest_speed = speed > fastest_speed ? speed : fastest_speed;
        fastest_speed_type = speed == fastest_speed ? speedType : fastest_speed_type;
    }

    return fastest_speed_type;
}

void Unit::sendMoveSplinePaket(UnitSpeedType speedType)
{
    WorldPacket data(12);

    switch (speedType)
    {
        case TYPE_WALK:
            data.Initialize(SMSG_SPLINE_SET_WALK_SPEED);
            break;
        case TYPE_RUN:
            data.Initialize(SMSG_SPLINE_SET_RUN_SPEED);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(SMSG_SPLINE_SET_RUN_BACK_SPEED);
            break;
        case TYPE_SWIM:
            data.Initialize(SMSG_SPLINE_SET_SWIM_SPEED);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(SMSG_SPLINE_SET_SWIM_BACK_SPEED);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(SMSG_SPLINE_SET_TURN_RATE);
            break;
        case TYPE_FLY:
            data.Initialize(SMSG_SPLINE_SET_FLIGHT_SPEED);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(SMSG_SPLINE_SET_FLIGHT_BACK_SPEED);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(SMSG_SPLINE_SET_PITCH_RATE);
            break;
#endif
    }

    data << GetNewGUID();
    data << float(getSpeedRate(speedType, true));

    SendMessageToSet(&data, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

void Unit::playSpellVisual(uint32_t visual_id, uint32_t type)
{
    SendMessageToSet(SmsgPlaySpellVisual(getGuid(), visual_id, type).serialise().get(), true);
}

void Unit::applyDiminishingReturnTimer(uint32_t* duration, SpellInfo const* spell)
{
    uint32_t status = sSpellMgr.getDiminishingGroup(spell->getId());
    uint32_t group  = status & 0xFFFF;
    uint32_t PvE    = (status >> 16) & 0xFFFF;

    // Make sure we have a group
    if (group == 0xFFFF)
    {
        return;
    }

    // Check if we don't apply to pve
    if (!PvE && !isPlayer() && !isPet())
    {
        return;
    }

    uint32_t localDuration = *duration;
    uint32_t count = m_diminishCount[group];

    // Target immune to spell
    if (count > 2)
    {
        *duration = 0;
        return;
    }

    //100%, 50%, 25% bitwise
    localDuration >>= count;
    if ((isPlayer() || isPet()) && localDuration > uint32_t(10000 >> count))
    {
        localDuration = 10000 >> count;
        if (status == DIMINISHING_GROUP_NOT_DIMINISHED)
        {
            *duration = localDuration;
            return;
        }
    }

    *duration = localDuration;

    // Reset the diminishing return counter, and add to the aura count (we don't decrease the timer till we
    // have no auras of this type left.
    ++m_diminishCount[group];
}

void Unit::removeDiminishingReturnTimer(SpellInfo const* spell)
{
    uint32_t status = sSpellMgr.getDiminishingGroup(spell->getId());
    uint32_t group  = status & 0xFFFF;
    uint32_t pve    = (status >> 16) & 0xFFFF;
    uint32_t aura_group;

    // Make sure we have a group
    if (group == 0xFFFF)
    {
        return;
    }

    // Check if we don't apply to pve
    if (!pve && !isPlayer() && !isPet())
    {
        return;
    }

    /*There are cases in which you just refresh an aura duration instead of the whole aura,
    causing corruption on the diminishAura counter and locking the entire diminishing group.
    So it's better to check the active auras one by one*/
    m_diminishAuraCount[group] = 0;
    for (uint32_t x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
    {
        if (m_auras[x])
        {
            aura_group = sSpellMgr.getDiminishingGroup(m_auras[x]->getSpellInfo()->getId());
            if (aura_group == status)
            {
                m_diminishAuraCount[group]++;
            }
        }
    }

    // Start timer decrease
    if (!m_diminishAuraCount[group])
    {
        m_diminishActive = true;
        m_diminishTimer[group] = 15000;
    }
}

bool Unit::canDualWield() const
{
    return m_canDualWield;
}

void Unit::setDualWield(bool enable)
{
    m_canDualWield = enable;

    if (!isPlayer())
        return;

    auto plrUnit = static_cast<Player*>(this);
    if (enable)
    {
        if (!plrUnit->_HasSkillLine(SKILL_DUAL_WIELD))
            plrUnit->_AddSkillLine(SKILL_DUAL_WIELD, 1, 1);
    }
    else
    {
        if (plrUnit->canDualWield2H())
            plrUnit->setDualWield2H(false);

        plrUnit->_RemoveSkillLine(SKILL_DUAL_WIELD);
    }
}

void Unit::castSpell(uint64_t targetGuid, uint32_t spellId, bool triggered)
{
    castSpell(targetGuid, spellId, 0, triggered);
}

void Unit::castSpell(Unit* target, uint32_t spellId, bool triggered)
{
    castSpell(target, spellId, 0, triggered);
}

void Unit::castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, bool triggered)
{
    if (spellInfo == nullptr)
        return;

    castSpell(targetGuid, spellInfo, 0, triggered);
}

void Unit::castSpell(Unit* target, SpellInfo const* spellInfo, bool triggered)
{
    if (spellInfo == nullptr)
        return;
    
    castSpell(target, spellInfo, 0, triggered);
}

void Unit::castSpell(uint64_t targetGuid, uint32_t spellId, uint32_t forcedBasepoints, bool triggered)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    castSpell(targetGuid, spellInfo, forcedBasepoints, triggered);
}

void Unit::castSpell(Unit* target, uint32_t spellId, uint32_t forcedBasepoints, bool triggered)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    castSpell(target, spellInfo, forcedBasepoints, triggered);
}

void Unit::castSpell(Unit* target, SpellInfo const* spellInfo, uint32_t forcedBasePoints, int32_t spellCharges, bool triggered)
{
    if (spellInfo == nullptr)
        return;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    newSpell->forced_basepoints[0] = forcedBasePoints;
    newSpell->m_charges = spellCharges;

    SpellCastTargets targets(0);
    if (target != nullptr)
    {
        targets.addTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(target->getGuid());
    }
    else
        newSpell->GenerateTargets(&targets);

    // Prepare the spell
    newSpell->prepare(&targets);
}

void Unit::castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered)
{
    if (spellInfo == nullptr)
        return;

    SpellCastTargets targets;
    targets.setDestination(location);
    targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);

    // Prepare the spell
    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    newSpell->prepare(&targets);
}

void Unit::eventCastSpell(Unit* target, SpellInfo const* spellInfo)
{
    ARCEMU_ASSERT(spellInfo != nullptr);

    castSpell(target, spellInfo, 0, true);
}

void Unit::castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered)
{
    if (spellInfo == nullptr)
        return;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        newSpell->forced_basepoints[i] = forcedBasepoints;
    }

    SpellCastTargets targets(targetGuid);

    // Prepare the spell
    newSpell->prepare(&targets);
}

void Unit::castSpell(Unit* target, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered)
{
    if (spellInfo == nullptr)
        return;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        newSpell->forced_basepoints[i] = forcedBasepoints;
    }

    SpellCastTargets targets(0);
    if (target != nullptr)
    {
        targets.addTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(target->getGuid());
    }
    else
        newSpell->GenerateTargets(&targets);

    // Prepare the spell
    newSpell->prepare(&targets);
}

float_t Unit::getSpellHealingBonus(SpellInfo const* spellInfo, int32_t baseHeal, bool isPeriodic, Aura* aur/* = nullptr*/)
{
    if (spellInfo->getAttributesExC() & ATTRIBUTESEXC_NO_HEALING_BONUS)
        return 0.0f;

    // Check for correct class
    if (isPlayer())
    {
        switch (static_cast<Player*>(this)->getClass())
        {
            case WARRIOR:
#if VERSION_STRING != Classic
            // Hunters in classic benefit from spell power
            case HUNTER:
#endif
            case ROGUE:
#if VERSION_STRING >= WotLK
            case DEATHKNIGHT:
#endif
                return 0.0f;
            default:
                break;
        }
    }

    float_t bonusHeal = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    if (aur != nullptr)
        bonusHeal = static_cast<float_t>(aur->getHealPowerBonus());
    else
        bonusHeal = static_cast<float_t>(HealDoneMod[school]);

    bonusHeal += static_cast<float_t>(baseHeal * HealDonePctMod[school]);

    // Get spell coefficient value
    if (bonusHeal > 0.0f)
    {
        if (!isPeriodic || spellInfo->isChanneled())
        {
            if (spellInfo->spell_coeff_direct > 0.0f)
                bonusHeal *= spellInfo->spell_coeff_direct;
            else
                return 0.0f;
        }
        else
        {
            if (spellInfo->spell_coeff_overtime > 0.0f)
                bonusHeal *= spellInfo->spell_coeff_overtime;
            else
                return 0.0f;
        }

        // Apply general downranking penalty
        // Level 20 penalty is already applied in SpellMgr::setSpellCoefficient
        // This method only existed in late TBC and WotLK, in Cata spells lost their ranks
#if VERSION_STRING >= TBC
#if VERSION_STRING <= WotLK
        /*
        If caster level is less than max caster level, then the penalty = 1.0.
        If caster level is at or greater than max caster level, then the penalty = (22 + max level - caster level) / 20.
        The penalty is capped at 0.
        */
        const auto maxLevel = spellInfo->getMaxLevel();
        if (maxLevel != 0 && isPlayer())
        {
            float_t penalty = 1.0f;
            if (maxLevel <= getLevel())
                penalty = (22.0f + maxLevel - getLevel()) / 20.0f;

            if (penalty > 1.0f)
                penalty = 1.0f;
            if (penalty < 0.0f)
                penalty = 0.0f;

            bonusHeal *= penalty;
        }
#endif
#endif
    }

    int32_t bonus_penalty = 0;
    spellModFlatIntValue(SM_FPenalty, &bonus_penalty, spellInfo->getSpellFamilyFlags());

    int32_t bonus_penalty_pct = 0;
    spellModFlatIntValue(SM_PPenalty, &bonus_penalty_pct, spellInfo->getSpellFamilyFlags());

    bonusHeal += static_cast<float_t>((bonus_penalty + baseHeal) * bonus_penalty_pct / 100.0f);
    return bonusHeal;
}

float_t Unit::getSpellDamageBonus(SpellInfo const* spellInfo, int32_t baseDmg, bool isPeriodic, Aura* aur/* = nullptr*/)
{
    if (spellInfo->getAttributesExC() & ATTRIBUTESEXC_NO_DONE_BONUS)
        return 0.0f;

    if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_NOT_USING_DMG_BONUS)
        return 0.0f;

    // Check for correct class
    if (isPlayer())
    {
        switch (static_cast<Player*>(this)->getClass())
        {
            case WARRIOR:
#if VERSION_STRING != Classic
            // Hunters in classic benefit from spell power
            case HUNTER:
#endif
            case ROGUE:
#if VERSION_STRING >= WotLK
            case DEATHKNIGHT:
#endif
                return 0.0f;
            default:
                break;
        }
    }

    float_t bonusDmg = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    if (aur != nullptr)
        bonusDmg = static_cast<float_t>(aur->getSpellPowerBonus());
    else
        bonusDmg = static_cast<float_t>(GetDamageDoneMod(school));

    // Subtract 1 because value is initialized with 1
    bonusDmg += static_cast<float_t>(baseDmg * (GetDamageDonePctMod(school) - 1));

    // Get spell coefficient value
    if (bonusDmg > 0.0f)
    {
        if (!isPeriodic || spellInfo->isChanneled())
        {
            if (spellInfo->spell_coeff_direct > 0.0f)
                bonusDmg *= spellInfo->spell_coeff_direct;
            else
                return 0.0f;
        }
        else
        {
            if (spellInfo->spell_coeff_overtime > 0.0f)
                bonusDmg *= spellInfo->spell_coeff_overtime;
            else
                return 0.0f;
        }

        // Apply general downranking penalty
        // Level 20 penalty is already applied in SpellMgr::setSpellCoefficient
        // This method only existed in late TBC and WotLK, in Cata spells lost their ranks
#if VERSION_STRING >= TBC
#if VERSION_STRING <= WotLK
        /*
        If caster level is less than max caster level, then the penalty = 1.0.
        If caster level is at or greater than max caster level, then the penalty = (22 + max level - caster level) / 20.
        The penalty is capped at 0.
        */
        const auto maxLevel = spellInfo->getMaxLevel();
        if (maxLevel != 0 && isPlayer())
        {
            float_t penalty = 1.0f;
            if (maxLevel <= getLevel())
                penalty = (22.0f + maxLevel - getLevel()) / 20.0f;

            if (penalty > 1.0f)
                penalty = 1.0f;
            if (penalty < 0.0f)
                penalty = 0.0f;

            bonusDmg *= penalty;
        }
#endif
#endif
    }

    int32_t bonus_penalty = 0;
    spellModFlatIntValue(SM_FPenalty, &bonus_penalty, spellInfo->getSpellFamilyFlags());

    int32_t bonus_penalty_pct = 0;
    spellModFlatIntValue(SM_PPenalty, &bonus_penalty_pct, spellInfo->getSpellFamilyFlags());

    bonusDmg += static_cast<float_t>((bonus_penalty + baseDmg) * bonus_penalty_pct / 100.0f);
    return bonusDmg;
}

float_t Unit::getCriticalChanceForDamageSpell(SpellInfo const* spellInfo, Unit* victim) const
{
    ///\ todo: this is mostly copied from legacy method, needs rewrite later
    float_t critChance = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();
    PlayerCombatRating resilienceType = PCR_RANGED_SKILL;

    if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_RANGED)
    {
        if (isPlayer())
        {
            critChance = static_cast<Player const*>(this)->getRangedCritPercentage();
            if (victim->isPlayer())
                critChance += static_cast<Player*>(victim)->res_R_crit_get();

            critChance += static_cast<float_t>(victim->AttackerCritChanceMod[school]);
        }
        else
        {
            // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)
            critChance = 5.0f;
        }

        if (victim->isPlayer())
            resilienceType = PCR_RANGED_CRIT_RESILIENCE;
    }
    else if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_MELEE)
    {
        // Same shit with the melee spells, such as Judgment/Seal of Command
        if (isPlayer())
            critChance = static_cast<Player const*>(this)->getMeleeCritPercentage();

        if (victim->isPlayer())
        {
            //this could be ability but in that case we overwrite the value
            critChance += static_cast<Player*>(victim)->res_R_crit_get();
            resilienceType = PCR_MELEE_CRIT_RESILIENCE;
        }

        // Victim's (!) crit chance mod for physical attacks?
        critChance += static_cast<float_t>(victim->AttackerCritChanceMod[0]);
    }
    else
    {
        critChance = spellcritperc + SpellCritChanceSchool[school];

        critChance += static_cast<float_t>(victim->AttackerCritChanceMod[school]);

        //\todo Zyres: is tis relly the way this should work?
        if (isPlayer() && (victim->m_rootCounter - victim->m_stunned))
            critChance += static_cast<float_t>(static_cast<Player const*>(this)->m_RootedCritChanceBonus);

        if (victim->isPlayer())
            resilienceType = PCR_SPELL_CRIT_RESILIENCE;
    }

    spellModFlatFloatValue(SM_CriticalChance, &critChance, spellInfo->getSpellFamilyFlags());

    if (resilienceType != PCR_RANGED_SKILL)
        critChance -= static_cast<Player*>(victim)->CalcRating(resilienceType);

    if (critChance < 0.0f)
        critChance = 0.0f;
    if (critChance > 95.0f)
        critChance = 95.0f;

    return critChance;
}

float_t Unit::getCriticalChanceForHealSpell(SpellInfo const* spellInfo) const
{
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();
    float_t critChance = spellcritperc + SpellCritChanceSchool[school];
    spellModFlatFloatValue(SM_CriticalChance, &critChance, spellInfo->getSpellFamilyFlags());

    if (critChance < 0.0f)
        critChance = 0.0f;
    if (critChance > 95.0f)
        critChance = 95.0f;

    return critChance;
}

void Unit::sendSpellNonMeleeDamageLog(Object* caster, Object* target, SpellInfo const* spellInfo, uint32_t damage, uint32_t absorbedDamage, uint32_t resistedDamage, uint32_t blockedDamage, bool isPeriodicDamage, bool isCriticalHit)
{
    if (caster == nullptr || target == nullptr || spellInfo == nullptr)
        return;

    // Classic does not use school mask
    uint32_t school = 0;
#if VERSION_STRING == Classic
    school = spellInfo->getFirstSchoolFromSchoolMask();
#else
    school = spellInfo->getSchoolMask();
#endif

    uint32_t overKill = 0;
    if (target->isCreatureOrPlayer() && damage > static_cast<Unit*>(target)->getHealth())
        overKill = damage - static_cast<Unit*>(target)->getHealth();

    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, 48);

    data << target->GetNewGUID();
    data << caster->GetNewGUID();
    data << uint32_t(spellInfo->getId());
    data << uint32_t(damage);
#if VERSION_STRING >= WotLK
    data << uint32_t(overKill);
#endif
    data << uint8_t(school);
    data << uint32_t(absorbedDamage);
    data << uint32_t(resistedDamage);
    data << uint8_t(isPeriodicDamage);
    data << uint8_t(0); // unk
    data << uint32_t(blockedDamage);

    // Some sort of hit info, other values need more research
    if (isCriticalHit)
        data << uint32_t(0x2);
    else
        data << uint32_t(0);

    data << uint8_t(0); // debug mode boolean

    target->SendMessageToSet(&data, true);
}

void Unit::sendSpellHealLog(Object* caster, Object* target, uint32_t spellId, uint32_t healAmount, bool isCritical, uint32_t overHeal, uint32_t absorbedHeal)
{
    if (caster == nullptr || target == nullptr)
        return;

    target->SendMessageToSet(SmsgSpellHealLog(target->GetNewGUID(), caster->GetNewGUID(), spellId, healAmount, overHeal, absorbedHeal, isCritical).serialise().get(), true);
}

void Unit::sendSpellOrDamageImmune(uint64_t casterGuid, Unit* target, uint32_t spellId)
{
    target->SendMessageToSet(SmsgSpellOrDamageImmune(casterGuid, target->getGuid(), spellId).serialise().get(), true);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Aura

void Unit::addAura(Aura* aur)
{
    if (aur == nullptr)
        return;

    if (!isAlive() && !aur->getSpellInfo()->isDeathPersistent())
    {
        delete aur;
        return;
    }

    // Check school immunity
    const auto school = aur->getSpellInfo()->getFirstSchoolFromSchoolMask();
    if (school != SCHOOL_NORMAL && SchoolImmunityList[school] && aur->getCasterGuid() != getGuid())
    {
        ///\ todo: notify client that aura did not apply
        delete aur;
        return;
    }

    // Check if aura has effects
    if (aur->getAppliedEffectCount() == 0)
    {
        ///\ todo: notify client that aura did not apply
        delete aur;
        return;
    }

    // Check for flying mount
    // This is already checked in Spell::canCast but this could happen on teleport or login
    if (isPlayer() && aur->getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_ONLY_IN_OUTLANDS)
    {
        if (!static_cast<Player*>(this)->canUseFlyingMountHere())
        {
            if (GetMapId() != 571 || !(aur->getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING))
            {
                delete aur;
                return;
            }
        }
    }

    // Check for single target aura
    ///\ todo: this supports only single auras. Missing paladin seals, warlock curses etc
    if (aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA)
    {
        uint64_t previousTargetGuid = 0;

        const auto caster = aur->GetUnitCaster();
        if (caster != nullptr)
        {
            previousTargetGuid = caster->getSingleTargetGuidForAura(aur->getSpellId());

            // Check if aura is applied on different unit
            if (previousTargetGuid != 0 && previousTargetGuid != aur->getOwner()->getGuid())
            {
                const auto previousTarget = GetMapMgrUnit(previousTargetGuid);
                if (previousTarget != nullptr)
                    previousTarget->removeAllAurasByIdForGuid(aur->getSpellId(), caster->getGuid());
            }
        }
    }

    const auto spellInfo = aur->getSpellInfo();

    uint16_t auraSlot = 0xFFFF;
    if (!aur->IsPassive())
    {
        uint16_t CheckLimit, StartCheck;
        if (!aur->isNegative())
        {
            StartCheck = MAX_POSITIVE_AURAS_EXTEDED_START;
            CheckLimit = MAX_POSITIVE_AURAS_EXTEDED_END;
        }
        else
        {
            StartCheck = MAX_NEGATIVE_AURAS_EXTEDED_START;
            CheckLimit = MAX_NEGATIVE_AURAS_EXTEDED_END;
        }

        auto deleteAur = false;

        // Loop through auras
        for (auto i = StartCheck; i < CheckLimit; ++i)
        {
            Aura* _aura = m_auras[i];
            if (_aura == nullptr)
            {
                // Found an empty slot
                if (auraSlot == 0xFFFF)
                    auraSlot = i;

                // Do not break here, check if unit has any similiar aura
                continue;
            }

            // Check if unit has same aura by same caster
            if (_aura->getSpellId() == aur->getSpellId())
            {
                if (_aura->getCasterGuid() != aur->getCasterGuid())
                    continue;

                // The auras are casted by same unit, refresh duration and apply new stack if stackable
                _aura->refresh(false, 1);

                deleteAur = true;
                break;
            }
            // If this is a proc spell, it should not remove its mother aura
            else if (aur->pSpellId != _aura->getSpellId())
            {
                // Check for auras by specific type
                if (aur->getSpellInfo()->getMaxstack() == 0 && spellInfo->custom_BGR_one_buff_on_target > 0 && aur->getSpellInfo()->custom_BGR_one_buff_on_target & spellInfo->custom_BGR_one_buff_on_target)
                {
                    deleteAur = HasAurasOfBuffType(spellInfo->getCustom_BGR_one_buff_on_target(), aur->getCasterGuid(), 0);
                }
                // Check for auras with the same name and a different rank
                else
                {
                    AuraCheckResponse checkResponse = AuraCheck(spellInfo, _aura, aur->getCaster());
                    if (checkResponse.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                    {
                        deleteAur = true;
                    }
                    else if (checkResponse.Error == AURA_CHECK_RESULT_LOWER_BUFF_PRESENT)
                    {
                        _aura->removeAura();
                        continue;
                    }
                }
            }
        }

        if (deleteAur)
        {
            delete aur;
            return;
        }
    }
    else
    {
        // Passive spells always apply
        ///\ todo: probably should add check for passive aura stacking
        for (uint16_t i = MAX_PASSIVE_AURAS_START; i < MAX_PASSIVE_AURAS_END; ++i)
        {
            if (m_auras[i] == nullptr)
            {
                auraSlot = i;
                break;
            }
        }
    }

    // Could not find an empty slot, remove aura
    if (auraSlot == 0xFFFF)
    {
        delete aur;
        return;
    }

    // Find a visual slot for aura
    uint8_t visualSlot = 0xFF;
    if (!aur->IsPassive() || aur->getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO)
        visualSlot = findVisualSlotForAura(!aur->isNegative());

    aur->m_visualSlot = visualSlot;
    aur->m_auraSlot = auraSlot;

    m_auras[auraSlot] = aur;

    if (visualSlot < MAX_NEGATIVE_VISUAL_AURAS_END)
    {
        m_auravisuals[visualSlot] = aur->getSpellId();

#if VERSION_STRING < WotLK
        setAura(visualSlot, aur->getSpellId());
        setAuraFlags(visualSlot, aur->getAuraFlags());
        setAuraLevel(visualSlot, aur->GetUnitCaster() != nullptr ? aur->GetUnitCaster()->getLevel() : worldConfig.player.playerLevelCap);
        setAuraApplication(visualSlot, 1);
#endif

        // Send packet
        sendAuraUpdate(aur, false);
        UpdateAuraForGroup(visualSlot);
    }

    aur->applyModifiers(true);
    aur->RelocateEvents();

    // Possibly a hackfix from legacy method
    // Reaction from enemy AI
    if (aur->isNegative() && aur->IsCombatStateAffecting()) // Creature
    {
        const auto pCaster = aur->GetUnitCaster();
        if (pCaster && pCaster->isAlive() && isAlive())
        {
            pCaster->CombatStatus.OnDamageDealt(this);

            if (isCreature())
                m_aiInterface->AttackReaction(pCaster, 1, aur->getSpellId());
        }
    }

    // Hackfix from legacy method
    if (aur->getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_INVINCIBLE)
    {
        const auto pCaster = aur->GetUnitCaster();
        if (pCaster != nullptr)
        {
            pCaster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
            pCaster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_INVISIBILITY);

            uint32_t iceBlock[] =
            {
                //SPELL_HASH_ICE_BLOCK
                27619,
                36911,
                41590,
                45438,
                45776,
                46604,
                46882,
                56124,
                56644,
                62766,
                65802,
                69924,
                0
            };
            pCaster->removeAllAurasById(iceBlock);

            uint32_t divineShield[] =
            {
                //SPELL_HASH_DIVINE_SHIELD
                642,
                13874,
                29382,
                33581,
                40733,
                41367,
                54322,
                63148,
                66010,
                67251,
                71550,
                0
            };
            pCaster->removeAllAurasById(divineShield);
            //SPELL_HASH_BLESSING_OF_PROTECTION
            pCaster->removeAllAurasById(41450);
        }
    }

    // Store target's guid for single target auras
    if (aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA)
    {
        const auto caster = aur->GetUnitCaster();
        if (caster != nullptr)
            caster->setSingleTargetGuidForAura(aur->getSpellId(), getGuid());
    }

    // Hackfix from legacy method
    if (aur->getSpellInfo()->getMechanicsType() == MECHANIC_ENRAGED && !asc_enraged++)
        addAuraStateAndAuras(AURASTATE_FLAG_ENRAGED);
    else if (aur->getSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING && !asc_bleed++)
        addAuraStateAndAuras(AURASTATE_FLAG_BLEED);
    if (aur->getSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !asc_seal++)
        addAuraStateAndAuras(AURASTATE_FLAG_JUDGEMENT);

}

uint8_t Unit::findVisualSlotForAura(bool isPositive) const
{
    uint8_t start, end;
    if (isPositive)
    {
        start = 0;
        end = MAX_POSITIVE_VISUAL_AURAS_END;
    }
    else
    {
        start = MAX_NEGATIVE_VISUAL_AURAS_START;
        end = MAX_NEGATIVE_VISUAL_AURAS_END;
    }

    uint8_t visualSlot = 0xFF;

    // Find an empty slot
    for (auto i = start; i < end; ++i)
    {
        if (m_auravisuals[i] == 0)
        {
            visualSlot = i;
            break;
        }
    }

    return visualSlot;
}

Aura* Unit::getAuraWithId(uint32_t spell_id)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr)
        {
            if (aura->getSpellId() == spell_id)
                return aura;
        }
    }

    return nullptr;
}

bool Unit::hasAurasWithId(uint32_t* auraId)
{
    for (int i = 0; auraId[i] != 0; ++i)
    {
        for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        {
            if (m_auras[x] && m_auras[x]->getSpellInfo()->getId() == auraId[i])
                return true;
        }
    }

    return false;
}

bool Unit::hasAuraWithAuraEffect(AuraEffect type) const
{
    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] == nullptr)
            continue;
        if (m_auras[i]->getSpellInfo()->hasEffectApplyAuraName(type))
            return true;
    }
    return false;
}

bool Unit::hasAuraState(AuraState state, SpellInfo const* spellInfo, Unit const* caster) const
{
    if (caster != nullptr && spellInfo != nullptr && caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
    {
        for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        {
            if (caster->m_auras[i] == nullptr)
                continue;
            if (!caster->m_auras[i]->getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
                continue;
            if (caster->m_auras[i]->getSpellInfo()->isAffectingSpell(spellInfo))
                return true;
        }
    }
    return getAuraState() & (1 << (state - 1));
}

void Unit::addAuraStateAndAuras(AuraState state)
{
    if (!(getAuraState() & (1 << (state - 1))))
    {
        addAuraState(static_cast<uint32_t>(1 << (state - 1)));
        if (isPlayer())
        {
            // Activate passive spells which require this aurastate
            const auto playerSpellMap = static_cast<Player*>(this)->mSpells;
            for (auto spellId : playerSpellMap)
            {
                // Skip deleted spells, i.e. spells with lower rank than the current rank
                auto deletedSpell = static_cast<Player*>(this)->mDeletedSpells.find(spellId);
                if ((deletedSpell != static_cast<Player*>(this)->mDeletedSpells.end()))
                    continue;
                SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(spellId);
                if (spellInfo == nullptr || !spellInfo->isPassive())
                    continue;
                if (spellInfo->getCasterAuraState() == static_cast<uint32_t>(state))
                    castSpell(this, spellId, true);
            }
        }
    }
}

void Unit::removeAuraStateAndAuras(AuraState state)
{
    if (getAuraState() & (1 << (state - 1)))
    {
        removeAuraState(static_cast<uint32_t>(1 << (state - 1)));
        // Remove self-applied passive auras requiring this aurastate
        // Skip removing enrage effects
        for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        {
            if (m_auras[i] == nullptr)
                continue;
            if (m_auras[i]->getCasterGuid() != getGuid())
                continue;
            if (m_auras[i]->getSpellInfo()->getCasterAuraState() != static_cast<uint32_t>(state))
                continue;
            if (m_auras[i]->getSpellInfo()->isPassive() || state != AURASTATE_FLAG_ENRAGED)
                RemoveAura(m_auras[i]);
        }
    }
}

Aura* Unit::getAuraWithIdForGuid(uint32_t spell_id, uint64_t target_guid)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr)
        {
            if (aura->getSpellId() == spell_id && aura->getCasterGuid() == target_guid)
                return aura;
        }
    }

    return nullptr;
}

Aura* Unit::getAuraWithAuraEffect(AuraEffect aura_effect)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr && aura->getSpellInfo()->hasEffectApplyAuraName(aura_effect))
            return aura;
    }

    return nullptr;
}

bool Unit::hasAurasWithId(uint32_t auraId)
{
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] && m_auras[x]->getSpellInfo()->getId() == auraId)
            return true;
    }

    return false;
}

Aura* Unit::getAuraWithId(uint32_t* auraId)
{
    for (int i = 0; auraId[i] != 0; ++i)
    {
        for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        {
            if (m_auras[x] && m_auras[x]->getSpellInfo()->getId() == auraId[i])
                return m_auras[x];
        }
    }

    return nullptr;
}

uint32_t Unit::getAuraCountForId(uint32_t auraId)
{
    uint32_t auraCount = 0;

    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] && (m_auras[x]->getSpellInfo()->getId() == auraId))
        {
            ++auraCount;
        }
    }

    return auraCount;
}

Aura* Unit::getAuraWithIdForGuid(uint32_t* auraId, uint64 guid)
{
    for (int i = 0; auraId[i] != 0; ++i)
    {
        for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        {
            Aura* aura = m_auras[x];
            if (aura != nullptr && aura->getSpellInfo()->getId() == auraId[i] && aura->getCasterGuid() == guid)
                return aura;
        }
    }

    return nullptr;
}

void Unit::removeAllAurasById(uint32_t auraId)
{
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellInfo()->getId() == auraId)
            {
                m_auras[x]->removeAura();
            }
        }
    }
}

void Unit::removeAllAurasById(uint32_t* auraId)
{
    for (int i = 0; auraId[i] != 0; ++i)
    {
        for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        {
            if (m_auras[x])
            {
                if (m_auras[x]->getSpellInfo()->getId() == auraId[i])
                {
                    m_auras[x]->removeAura();
                }
            }
        }
    }
}

void Unit::removeAllAurasByIdForGuid(uint32_t spellId, uint64_t guid)
{
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellId() == spellId)
            {
                if (!guid || m_auras[x]->getCasterGuid() == guid)
                {
                    m_auras[x]->removeAura();
                }
            }
        }
    }
}

uint32_t Unit::removeAllAurasByIdReturnCount(uint32_t auraId)
{
    uint32_t res = 0;
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellInfo()->getId() == auraId)
            {
                m_auras[x]->removeAura();
                ++res;
            }
        }
    }
    return res;
}

uint64_t Unit::getSingleTargetGuidForAura(uint32_t spell)
{
    auto itr = m_singleTargetAura.find(spell);

    if (itr != m_singleTargetAura.end())
        return itr->second;
    else
        return 0;
}

uint64_t Unit::getSingleTargetGuidForAura(uint32_t* spellIds, uint32_t* index)
{
    for (uint8 i = 0; ; i++)
    {
        if (!spellIds[i])
            return 0;

        auto itr = m_singleTargetAura.find(spellIds[i]);

        if (itr != m_singleTargetAura.end())
        {
            *index = i;
            return itr->second;
        }
    }
}

void Unit::setSingleTargetGuidForAura(uint32_t spellId, uint64_t guid)
{
    auto itr = m_singleTargetAura.find(spellId);

    if (itr != m_singleTargetAura.end())
        itr->second = guid;
    else
        m_singleTargetAura.insert(std::make_pair(spellId, guid));
}

void Unit::removeSingleTargetGuidForAura(uint32_t spellId)
{
    auto itr = m_singleTargetAura.find(spellId);

    if (itr != m_singleTargetAura.end())
        m_singleTargetAura.erase(itr);
}

void Unit::removeAllAurasByAuraEffect(AuraEffect effect)
{
    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] == nullptr)
            continue;
        if (m_auras[i]->getSpellInfo()->hasEffectApplyAuraName(effect))
            RemoveAura(m_auras[i]);
    }
}

void Unit::sendAuraUpdate(Aura* aur, bool remove)
{
    if (aur->m_visualSlot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    // Check if aura is hidden on self cast
    if (aur->getCasterGuid() == getGuid() && aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_AURA_ON_SELF_CAST)
        return;

    // Check if aura is hidden when not self cast
    if (aur->getCasterGuid() != getGuid() && aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_AURA_ON_NON_SELF_CAST)
        return;

#if VERSION_STRING < WotLK
    if (!remove)
    {
#if VERSION_STRING == Classic
        if (isPlayer() && !aur->IsPassive())
            static_cast<Player*>(this)->SendMessageToSet(SmsgUpdateAuraDuration(aur->m_visualSlot, aur->getTimeLeft()).serialise().get(), true);
#else
        if (isPlayer() && !aur->IsPassive() && !(aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_DURATION))
        {
            static_cast<Player*>(this)->SendMessageToSet(SmsgUpdateAuraDuration(aur->m_visualSlot, aur->getTimeLeft()).serialise().get(), true);

            auto guid = GetNewGUID();
            static_cast<Player*>(this)->SendMessageToSet(SmsgSetExtraAuraInfo(&guid, aur->m_visualSlot, aur->getSpellId(), aur->getMaxDuration(), aur->getTimeLeft()).serialise().get(), true);
        }

        const auto caster = aur->GetUnitCaster();
        if (caster != nullptr && caster->isPlayer() && caster->getGuid() != getGuid())
        {
            WorldPacket data(SMSG_SET_EXTRA_AURA_INFO_NEED_UPDATE, 21);
            data << GetNewGUID();
            data << uint8_t(aur->m_visualSlot);
            data << uint32_t(aur->getSpellId());
            data << uint32_t(aur->getMaxDuration());
            data << uint32_t(aur->getTimeLeft());
            caster->SendMessageToSet(&data, true);
        }
#endif
    }
#if VERSION_STRING == TBC
    else
    {
        const auto caster = aur->GetUnitCaster();
        if (caster != nullptr && caster->isPlayer())
            static_cast<Player*>(caster)->SendMessageToSet(SmsgClearExtraAuraInfo(getGuid(), aur->getSpellId()).serialise().get(), true);
    }
#endif
#else
    SmsgAuraUpdate::AuraUpdate auraUpdate;

    auraUpdate.visualSlot = aur->m_visualSlot;
    auraUpdate.flags = aur->getAuraFlags();
    auraUpdate.spellId = aur->getSpellId();

    const auto casterUnit = aur->GetUnitCaster();
    if (casterUnit != nullptr)
        auraUpdate.level = static_cast<uint8_t>(casterUnit->getLevel());
    else
        auraUpdate.level = static_cast<uint8_t>(worldConfig.player.playerLevelCap);

    ///\ todo: proc charges
    ///\ also proc charges for classic and tbc in Unit::setAuraApplication
    auraUpdate.stackCount = aur->getStackCount() > 255 ? 255 : aur->getStackCount();

    if (!(auraUpdate.flags & AFLAG_IS_CASTER))
        auraUpdate.casterGuid = aur->getCasterGuid();

    if (auraUpdate.flags & AFLAG_DURATION)
    {
        auraUpdate.duration = aur->getMaxDuration();
        auraUpdate.timeLeft = aur->getTimeLeft();
    }

#if VERSION_STRING >= Cata
    if (auraUpdate.flags & AFLAG_SEND_EFFECT_AMOUNT)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aur->getAuraEffect(i).mAuraEffect != SPELL_AURA_NONE)
            {
                ///\ todo: we are sending base amount, should real amount be sent instead?
                auraUpdate.effAmount[i] = aur->getAuraEffect(i).mDamage;
            }
            else
            {
                auraUpdate.effAmount[i] = 0;
            }
        }
    }
#endif

    SendMessageToSet(SmsgAuraUpdate(getGuid(), auraUpdate, remove).serialise().get(), true);
#endif
}

void Unit::sendFullAuraUpdate()
{
#if VERSION_STRING >= WotLK
    auto packetData = SmsgAuraUpdateAll(getGuid(), {});
    auto updates = 0u;

    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aur = m_auras[i];
        if (aur == nullptr)
            continue;

        // Update only auras with a visual slot
        if (aur->m_visualSlot == 0xFF)
            continue;

        SmsgAuraUpdateAll::AuraUpdate auraUpdate;

        auraUpdate.flags = aur->getAuraFlags();
        auraUpdate.visualSlot = aur->m_visualSlot;
        auraUpdate.spellId = aur->getSpellId();

        const auto casterUnit = aur->GetUnitCaster();
        if (casterUnit != nullptr)
            auraUpdate.level = static_cast<uint8_t>(casterUnit->getLevel());
        else
            auraUpdate.level = static_cast<uint8_t>(worldConfig.player.playerLevelCap);

        ///\ todo: proc charges
        auraUpdate.stackCount = aur->getStackCount() > 255 ? 255 : aur->getStackCount();

        if (!(auraUpdate.flags & AFLAG_IS_CASTER))
            auraUpdate.casterGuid = aur->getCasterGuid();

        if (auraUpdate.flags & AFLAG_DURATION)
        {
            auraUpdate.duration = aur->getMaxDuration();
            auraUpdate.timeLeft = aur->getTimeLeft();
        }

#if VERSION_STRING >= Cata
        if (auraUpdate.flags & AFLAG_SEND_EFFECT_AMOUNT)
        {
            for (uint8_t x = 0; x < MAX_SPELL_EFFECTS; ++x)
            {
                if (aur->getAuraEffect(x).mAuraEffect != SPELL_AURA_NONE)
                {
                    ///\ todo: we are sending base amount, should real amount be sent instead?
                    auraUpdate.effAmount[x] = aur->getAuraEffect(x).mDamage;
                }
                else
                {
                    auraUpdate.effAmount[x] = 0;
                }
            }
        }
#endif

        packetData.addAuraUpdate(auraUpdate);
        ++updates;
    }

    SendMessageToSet(packetData.serialise().get(), true);
    LogDebugFlag(LF_AURA, "Unit::sendFullAuraUpdate : Updated %u auras for guid %u", updates, getGuid());
#endif
}

bool Unit::sendPeriodicAuraLog(const WoWGuid& casterGuid, const WoWGuid& targetGuid, SpellInfo const* spellInfo, uint32_t amount, uint32_t overKillOrOverHeal, uint32_t absorbed, uint32_t resisted, AuraEffect auraEffect, bool isCritical, uint32_t miscValue/* = 0*/, float gainMultiplier/* = 0.0f*/)
{
    // Classic does not use school mask
    uint32_t school = 0;
#if VERSION_STRING == Classic
    school = spellInfo->getFirstSchoolFromSchoolMask();
#else
    school = spellInfo->getSchoolMask();
#endif

    switch (auraEffect)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_PERIODIC_HEAL_PCT:
        case SPELL_AURA_PERIODIC_POWER_PCT:
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            break;
        default:
            return false;
    }

    SendMessageToSet(SmsgPeriodicAuraLog(targetGuid, casterGuid, spellInfo->getId(), auraEffect, amount, overKillOrOverHeal, school, absorbed, resisted, isCritical, miscValue, gainMultiplier).serialise().get(), true);
    return true;
}

void Unit::_updateAuras(unsigned long diff)
{
    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        const auto aur = m_auras[i];
        if (aur == nullptr)
            continue;

        aur->update(diff);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Visibility system
bool Unit::canSee(Object* const obj)
{
    if (obj == nullptr)
        return false;

    if (this == obj)
        return true;

    if (!obj->IsInWorld() || GetMapId() != obj->GetMapId())
        return false;

    // Unit cannot see objects from different phases
    if ((GetPhase() & obj->GetPhase()) == 0)
        return false;

    // Unit cannot see objects which are further than 100 yards away (visibility range)
    //\ todo: should this be in MapMgr? also there are some objects which should be visibile even further and some objects which should always be visible
    if (!isInRange(obj->GetPosition(), 100.0f * 100.0f))
    {
        // Battlegrounds have an increased visibility range
        if (getPlayerOwner() != nullptr && getPlayerOwner()->m_bg != nullptr)
        {
            if (!isInRange(obj->GetPosition(), 300.0f * 300.0f))
                return false;
        }
        else
        {
            return false;
        }
    }

    // Unit cannot see invisible Game Masters unless he/she has Game Master flag on
    if (obj->isPlayer() && static_cast<Player*>(obj)->m_isGmInvisible)
        return isPlayer() && static_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM);

    uint64_t ownerGuid = 0;
    if (getCharmedByGuid() != 0)
        ownerGuid = getCharmedByGuid();
    else
        ownerGuid = getSummonedByGuid();

    // Unit can always see its master
    if (ownerGuid == obj->getGuid())
        return true;

    // Player is dead and has released spirit
    if (isPlayer() && getDeathState() == CORPSE)
    {
        const float_t corpseViewDistance = 1600.0f; // 40*40 yards
        const auto playerMe = static_cast<Player*>(this);
        // If object is another player
        if (obj->isPlayer())
        {
            // Dead player can see all players in arena regardless of range
            if (playerMe->m_deathVision)
                return true;

            // Player can see all friendly and unfriendly players within 40 yards from his/her corpse
            const auto playerObj = static_cast<Player*>(obj);
            if (playerMe->getMyCorpseInstanceId() == playerMe->GetInstanceID() &&
                playerObj->isInRange(playerMe->getMyCorpseLocation(), corpseViewDistance))
                return true;

            // Otherwise player can only see other players who have released their spirits as well
            return playerObj->getDeathState() == CORPSE;
        }

        // Dead player can also see all objects in arena regardless of range
        if (playerMe->m_deathVision)
            return true;

        if (playerMe->getMyCorpseInstanceId() == GetInstanceID())
        {
            // Player can see his/her own corpse
            if (obj->isCorpse() && static_cast<Corpse*>(obj)->getOwnerGuid() == getGuid())
                return true;

            // Player can see all objects within 40 yards from his/her own corpse
            if (obj->isInRange(playerMe->getMyCorpseLocation(), corpseViewDistance))
                return true;
        }

        // Player can see Spirit Healers
        if (obj->isCreature() && static_cast<Creature*>(obj)->isSpiritHealer())
            return true;

        return false;
    }

    // Unit is alive or player hasn't released spirit yet
    // Do checks based on object's type
    switch (obj->getObjectTypeId())
    {
        case TYPEID_PLAYER:
        {
            const auto playerObj = static_cast<Player*>(obj);
            if (playerObj->getDeathState() == CORPSE)
            {
                if (isPlayer())
                {
                    // If players are from same group, they can see each other normally
                    const auto playerMe = static_cast<Player*>(this);
                    if (playerMe->GetGroup() != nullptr && playerMe->GetGroup() == playerObj->GetGroup())
                        return true;

                    // Game Masters can see all dead players
                    return dynamic_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM);
                }
                else
                {
                    // Non-player units cannot see dead players
                    return false;
                }
            }
        } break;
        case TYPEID_UNIT:
        {
            // Unit cannot see Spirit Healers when unit's alive
            // unless unit is a Game Master
            if (obj->isCreature() && static_cast<Creature*>(obj)->isSpiritHealer())
                return isPlayer() && dynamic_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM);

            const auto unitObj = static_cast<Unit*>(obj);

            if (unitObj->getCharmedByGuid() != 0)
                ownerGuid = unitObj->getCharmedByGuid();
            else
                ownerGuid = unitObj->getSummonedByGuid();

            // Unit can always see their own summoned units
            if (getGuid() == ownerGuid)
                return true;

            if (isPlayer())
            {
                // Group members can see each other's summoned units
                // unless they are dueling, then it's based on detection
                const auto objectOwner = GetMapMgrPlayer(ownerGuid);
                if (objectOwner != nullptr)
                {
                    if (objectOwner->GetGroup() != nullptr && objectOwner->GetGroup()->HasMember(static_cast<Player*>(this)))
                    {
                        if (objectOwner->DuelingWith != static_cast<Player*>(this))
                            return true;
                    }
                }

                // If object is only visible to either faction
                if (unitObj->GetAIInterface()->faction_visibility == 1)
                    return static_cast<Player*>(this)->isTeamHorde() ? true : false;
                if (unitObj->GetAIInterface()->faction_visibility == 2)
                    return static_cast<Player*>(this)->isTeamHorde() ? false : true;
            }
        } break;
        case TYPEID_GAMEOBJECT:
        {
            const auto gameObjectObj = static_cast<GameObject*>(obj);
            // Stealthed / invisible gameobjects
            if (gameObjectObj->inStealth || gameObjectObj->invisible)
            {
                ownerGuid = gameObjectObj->getCreatedByGuid();
                // Unit can always see their own created gameobjects
                if (getGuid() == ownerGuid)
                    return true;

                // Group members can see each other's created gameobjects
                // unless they are dueling, then it's based on detection
                const auto objectOwner = GetMapMgrPlayer(ownerGuid);
                if (objectOwner != nullptr && isPlayer())
                {
                    if (objectOwner->GetGroup() != nullptr && objectOwner->GetGroup()->HasMember(static_cast<Player*>(this)))
                    {
                        if (objectOwner->DuelingWith != static_cast<Player*>(this))
                            return true;
                    }
                }
            }
        } break;
        default:
            break;
    }

    // Game Masters can see invisible and stealthed objects
    if (isPlayer() && dynamic_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM))
        return true;

    // Hunter Marked units are always visible to caster
    if (obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->stalkedby == getGuid())
        return true;

    // Pets and summoned units don't have detection, they rely on their master's detection
    auto meUnit = this;
    if (getCharmedByGuid() != 0)
    {
        const auto summoner = GetMapMgrUnit(getCharmedByGuid());
        if (summoner != nullptr)
            meUnit = summoner;
    }
    else if (getSummonedByGuid() != 0)
    {
        const auto summoner = GetMapMgrUnit(getSummonedByGuid());
        if (summoner != nullptr)
            meUnit = summoner;
    }

    const auto unitTarget = static_cast<Unit*>(obj);
    const auto gobTarget = static_cast<GameObject*>(obj);

    ////////////////////////////
    // Invisibility detection

    for (uint8_t i = 0; i < INVIS_FLAG_TOTAL; ++i)
    {
        auto unitInvisibilityValue = meUnit->getInvisibilityLevel(InvisibilityFlag(i));
        auto unitInvisibilityDetection = meUnit->getInvisibilityDetection(InvisibilityFlag(i));
        auto objectInvisibilityValue = 0;
        auto objectInvisibilityDetection = 0;

        if (obj->isCreatureOrPlayer())
        {
            objectInvisibilityValue = unitTarget->getInvisibilityLevel(InvisibilityFlag(i));
            objectInvisibilityDetection = unitTarget->getInvisibilityDetection(InvisibilityFlag(i));

            // When unit is invisible, unit can only see those objects which have enough detection value
            if ((unitInvisibilityValue > objectInvisibilityDetection) ||
            // When object is invisible, unit can only see it if unit has enough detection value
                (objectInvisibilityValue > unitInvisibilityDetection))
                return false;
        }
        else if (obj->isGameObject() && gobTarget->invisible && i == INVIS_FLAG_TRAP)
        {
            // Base value for invisible traps seems to be 300 according to spell id 2836
            objectInvisibilityValue = 300;
            if (objectInvisibilityValue > unitInvisibilityDetection)
                return false;
        }
    }

    ////////////////////////////
    // Stealth detection

    if ((obj->isCreatureOrPlayer() && unitTarget->isStealthed()) || (obj->isGameObject() && gobTarget->inStealth))
    {
        // Get absolute distance
        const auto distance = meUnit->CalcDistance(obj);
        const auto combatReach = meUnit->getCombatReach();
        if (obj->isCreatureOrPlayer())
        {
            // Shadow Sight buff in arena makes unit detect stealth regardless of distance and facing
            if (meUnit->hasAuraWithAuraEffect(SPELL_AURA_DETECT_STEALTH))
                return true;

            // Normally units not in front cannot be detected
            if (!meUnit->isInFront(obj))
                return false;

            // If object is closer than unit's combat reach
            if (distance < combatReach)
                return true;
        }

        // Objects outside of Line of Sight cannot be detected
        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            if (!meUnit->IsWithinLOSInMap(obj))
                return false;
        }

        // In unit cases base stealth level and base stealth detection increases by 5 points per unit's level
        // Stealth detection base points start from 30ish, exact value unknown
        int detectionValue = 30 + meUnit->getLevel() * 5;

        // Apply modifiers which increases unit's stealth detection
        if (obj->isCreatureOrPlayer())
            detectionValue += meUnit->getStealthDetection(STEALTH_FLAG_NORMAL);
        else if (obj->isGameObject())
            detectionValue += meUnit->getStealthDetection(STEALTH_FLAG_TRAP);

        // Subtract object's stealth level from detection value
        if (obj->isCreatureOrPlayer())
            detectionValue -= unitTarget->getStealthLevel(STEALTH_FLAG_NORMAL);
        else if (obj->isGameObject())
        {
            // Base value for stealthed gameobjects seems to be 70 according to spell id 2836
            detectionValue -= 70;
            if (gobTarget->getCreatedByGuid() != 0)
            {
                // If trap has an owner, subtract owner's stealth level (unit level * 5) from detection value
                const auto summoner = gobTarget->GetMapMgrUnit(gobTarget->getCreatedByGuid());
                if (summoner != nullptr)
                    detectionValue -= summoner->getLevel() * 5;
            }
            else
                // If trap has no owner, subtract trap's level from detection value
                detectionValue -= gobTarget->GetGameObjectProperties()->trap.level * 5;
        }

        auto visibilityRange = static_cast<float_t>(detectionValue * 0.3f + combatReach);
        if (visibilityRange <= 0.0f)
            return false;

        // Players cannot see stealthed objects from further than 30 yards
        if (meUnit->isPlayer() && visibilityRange > 30.0f)
            visibilityRange = 30.0f;

        // Object is further than unit's visibility range
        if (distance > visibilityRange)
            return false;
    }
    return true;
}

int32_t Unit::getStealthLevel(StealthFlag flag) const
{
    return m_stealthLevel[flag];
}

int32_t Unit::getStealthDetection(StealthFlag flag) const
{
    return m_stealthDetection[flag];
}

void Unit::modStealthLevel(StealthFlag flag, const int32_t amount)
{
    m_stealthLevel[flag] += amount;
}

void Unit::modStealthDetection(StealthFlag flag, const int32_t amount)
{
    m_stealthDetection[flag] += amount;
}

bool Unit::isStealthed() const
{
    return hasAuraWithAuraEffect(SPELL_AURA_MOD_STEALTH);
}

int32_t Unit::getInvisibilityLevel(InvisibilityFlag flag) const
{
    return m_invisibilityLevel[flag];
}

int32_t Unit::getInvisibilityDetection(InvisibilityFlag flag) const
{
    return m_invisibilityDetection[flag];
}

void Unit::modInvisibilityLevel(InvisibilityFlag flag, const int32_t amount)
{
    m_invisibilityLevel[flag] += amount;
}

void Unit::modInvisibilityDetection(InvisibilityFlag flag, const int32_t amount)
{
    m_invisibilityDetection[flag] += amount;
}

bool Unit::isInvisible() const
{
    return hasAuraWithAuraEffect(SPELL_AURA_MOD_INVISIBILITY);
}

void Unit::setVisible(const bool visible)
{
    if (!visible)
        modInvisibilityLevel(INVIS_FLAG_NEVER_VISIBLE, 1);
    else
        modInvisibilityLevel(INVIS_FLAG_NEVER_VISIBLE, -getInvisibilityLevel(INVIS_FLAG_NEVER_VISIBLE));
    UpdateVisibility();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Power related
void Unit::regeneratePowers(uint16_t timePassed)
{
    if (!isAlive())
        return;

    // Mana and Energy
    m_manaEnergyRegenerateTimer += timePassed;
    if (isPlayer() || getPlayerOwner() != nullptr)
    {
        // Player and player owned creatures regen in real time since wotlk
        if (m_manaEnergyRegenerateTimer >= REGENERATION_INTERVAL_MANA_ENERGY)
        {
            regeneratePower(POWER_TYPE_MANA);
            regeneratePower(POWER_TYPE_ENERGY);
            m_manaEnergyRegenerateTimer = 0;
        }
    }
    else
    {
        if (m_manaEnergyRegenerateTimer >= CREATURE_REGENERATION_INTERVAL_MANA_ENERGY)
        {
            regeneratePower(POWER_TYPE_MANA);
            regeneratePower(POWER_TYPE_ENERGY);
            m_manaEnergyRegenerateTimer = 0;
        }
    }

    // Focus
    m_focusRegenerateTimer += timePassed;
    if (m_focusRegenerateTimer >= REGENERATION_INTERVAL_FOCUS)
    {
        regeneratePower(POWER_TYPE_FOCUS);
        m_focusRegenerateTimer = 0;
    }

    // Update player only resources
    if (isPlayer())
        static_cast<Player*>(this)->regeneratePlayerPowers(timePassed);
}

void Unit::regeneratePower(PowerType type)
{
#if VERSION_STRING >= Cata
    if (getPowerIndexFromDBC(type) == TOTAL_PLAYER_POWER_TYPES)
        return;
#endif

    if (isCreature())
    {
        // Check for correct power type
        if (getPowerType() != type)
            return;

        if (static_cast<Creature*>(this)->m_interruptRegen)
            return;
    }

    const auto currentPower = getPower(type);
    const auto maxPower = getMaxPower(type);
    if (maxPower == 0)
        return;

    // Helper lambda to get correct rate from config files
    const auto getConfigRate = [&](WorldConfigRates rate) -> float_t
    {
        if (!isPlayer() && isVehicle())
            return worldConfig.getFloatRate(RATE_VEHICLES_POWER_REGEN);
        else
            return worldConfig.getFloatRate(rate);
    };

    float_t amount = 0.0f;
    auto sendUpdatePacket = false;
    switch (type)
    {
        case POWER_TYPE_MANA:
        {
            const auto manaRate = getConfigRate(RATE_POWER1);
            if (isPlayer())
            {
#if VERSION_STRING < Cata
                // Check for 5 second regen interruption
                if (isPowerRegenerationInterrupted())
                    amount = static_cast<Player*>(this)->getManaRegenerationWhileCasting();
                else
                    amount = static_cast<Player*>(this)->getManaRegeneration();
#else
                // Check for combat (5 second rule was removed in cata)
                if (CombatStatus.IsInCombat())
                    amount = getManaRegenerationWhileCasting();
                else
                    amount = getManaRegeneration();
#endif
#if VERSION_STRING < WotLK
                // Send update packet pre-wotlk
                sendUpdatePacket = true;
#endif
            }
            else
            {
                //\ todo: this creature mana regeneration is not correct, rewrite it
                if (CombatStatus.IsInCombat())
                {
                    amount = (getLevel() + 10) * PctPowerRegenModifier[POWER_TYPE_MANA];
                }
                else
                {
                    // 33% of total mana per tick when out of combat
                    amount = maxPower * 0.33f;
                }

                // Send update packet for creatures in all versions
#if VERSION_STRING >= WotLK
                // but not for player owned creatures after wotlk, they regen in real time
                if (getPlayerOwner() == nullptr)
#endif
                {
                    sendUpdatePacket = true;
                }
            }

            // Mana regeneration is calculated per 1 second
            // Convert it to correct amount for expansion / unit (i.e in wotlk 100ms for players and 2000ms for creatures)
            amount *= manaRate * (m_manaEnergyRegenerateTimer * 0.001f);
        } break;
        case POWER_TYPE_RAGE:
#if VERSION_STRING >= WotLK
        case POWER_TYPE_RUNIC_POWER:
#endif
        {
            // Rage and Runic Power do not decay while in combat
            if (CombatStatus.IsInCombat())
                return;

            // TODO: fix this hackfix when aura system supports this
            const auto hasAngerManagement = HasAura(12296);

            // Rage and Runic Power are lost at rate of 1.25 point per 1 second (or 1 point per 800ms)
            // Convert the value first to 5 seconds because regeneration modifiers work like that
            float_t decayValue = 12.5f * 5;

            // Anger Management slows out of combat rage decay
            if (hasAngerManagement)
                decayValue -= 17.0f;

#if VERSION_STRING >= WotLK
            // Divide it first by 5 and then multiply by 0.8 to get correct amount for 800ms
            decayValue = (decayValue / 5) * 0.8f;
#else
            // Convert the value to 3 seconds (Pre-Wotlk rage decays every 3 seconds)
            decayValue *= 0.6f;

            sendUpdatePacket = true;
#endif

            amount = currentPower <= decayValue ? -static_cast<int32_t>(currentPower) : -decayValue;
        } break;
        case POWER_TYPE_FOCUS:
        {
            const auto focusRate = getConfigRate(RATE_POWER3);
#if VERSION_STRING < WotLK
            sendUpdatePacket = true;

            // 24 focus per 4 seconds according to WoWWiki
            amount = 24.0f;
#else
            // Focus regens 1 point per 200ms as of 3.0.2
            amount = 1.0f;
#endif
            amount *= focusRate * PctPowerRegenModifier[POWER_TYPE_FOCUS];
        } break;
        case POWER_TYPE_ENERGY:
        {
            const auto energyRate = getConfigRate(RATE_POWER4);

            // 10 energy per 1 second
            // Convert it to correct amount for expansion / unit (i.e in wotlk 100ms for players and 2000ms for creatures)
            amount = 10.0f * (m_manaEnergyRegenerateTimer * 0.001f);

#if VERSION_STRING >= WotLK
            // Do not send update packet for players or player owned creatures after wotlk
            if (!isPlayer() && getPlayerOwner() == nullptr)
#endif
            {
                sendUpdatePacket = true;
            }

            amount *= energyRate * PctPowerRegenModifier[POWER_TYPE_ENERGY];
        } break;
#if VERSION_STRING >= Cata
        case POWER_TYPE_HOLY_POWER:
        {
            if (CombatStatus.IsInCombat())
                return;

            amount = -1.0f;
        } break;
#endif
        default:
            return;
    }

    if (amount < 0.0f)
    {
        if (currentPower == 0)
            return;
    }
    else if (amount > 0.0f)
    {
        if (currentPower >= maxPower)
            return;
    }
    else
    {
        return;
    }

    amount += m_powerFractions[type];

    // Convert the float amount to integer and save the fraction for next power update
    // This fixes regen like 0.98
    auto powerResult = currentPower;
    auto integerAmount = static_cast<uint32_t>(std::fabs(amount));

    if (amount < 0.0f)
    {
        if (currentPower > integerAmount)
        {
            powerResult -= integerAmount;
            m_powerFractions[type] = amount + integerAmount;
        }
        else
        {
            powerResult = 0;
            m_powerFractions[type] = 0;
        }
    }
    else
    {
        powerResult += integerAmount;
        if (powerResult > maxPower)
        {
            powerResult = maxPower;
            m_powerFractions[type] = 0;
        }
        else
        {
            m_powerFractions[type] = amount - integerAmount;
        }
    }

    setPower(type, powerResult, sendUpdatePacket);
}

#if VERSION_STRING < Cata
void Unit::interruptPowerRegeneration(uint32_t timeInMS)
{
#if VERSION_STRING != Classic
    if (isPlayer() && !isPowerRegenerationInterrupted())
        removeUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

    m_powerRegenerationInterruptTime = timeInMS;
}

bool Unit::isPowerRegenerationInterrupted() const
{
    return m_powerRegenerationInterruptTime != 0;
}
#endif

void Unit::energize(Unit* target, uint32_t spellId, uint32_t amount, PowerType type, bool sendPacket/* = true*/)
{
    if (target == nullptr || spellId == 0 || amount == 0)
        return;

    // Send packet first
    if (sendPacket)
        sendSpellEnergizeLog(target, spellId, amount, type);

    target->setPower(type, target->getPower(type) + amount);

#if VERSION_STRING >= Cata
    // Reset Holy Power timer back to 10 seconds
    if (isPlayer() && type == POWER_TYPE_HOLY_POWER)
        static_cast<Player*>(this)->resetHolyPowerTimer();
#endif
}

void Unit::sendSpellEnergizeLog(Unit* target, uint32_t spellId, uint32_t amount, PowerType type)
{
    SendMessageToSet(SmsgSpellEnergizeLog(target->GetNewGUID(), GetNewGUID(), spellId, type, amount).serialise().get(), true);
}

uint8_t Unit::getPowerPct(PowerType powerType) const
{
    if (powerType == POWER_TYPE_HEALTH)
        return getHealthPct();

    if (getPower(powerType) <= 0 || getMaxPower(powerType) <= 0)
        return 0;

    if (getPower(powerType) >= getMaxPower(powerType))
        return 100;

    return static_cast<uint8_t>(getPower(powerType) * 100 / getMaxPower(powerType));
}

void Unit::sendPowerUpdate([[maybe_unused]]bool self)
{
    // Save current power so the same amount is sent to player and everyone else
    const auto powerAmount = getPower(getPowerType());

#if VERSION_STRING >= WotLK
    SendMessageToSet(SmsgPowerUpdate(GetNewGUID(), static_cast<uint8_t>(getPowerType()), powerAmount).serialise().get(), self);
#else
    //\ todo: is this correct for TBC?
    auto packet = BuildFieldUpdatePacket(getOffsetForStructuredField(WoWUnit, power_1) + (getPowerIndexFromDBC(getPowerType()) - 1), powerAmount);
    SendMessageToSet(packet, false);
    delete packet;
#endif
}

uint8_t Unit::getPowerIndexFromDBC(PowerType type) const
{
#if VERSION_STRING <= WotLK
    // Prior to Cataclysm power type equals index
    return static_cast<uint8_t>(type + 1);
#else
    if (!isPlayer())
    {
        // For creatures return first index
        //\ todo: can creatures use multiple power types?
        return POWER_FIELD_INDEX_1;
    }

    return getPowerIndexByClass(getClass(), static_cast<uint8_t>(type));
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void Unit::setAttackTimer(WeaponDamageType type, int32_t time)
{
    m_attackTimer[type] = Util::getMSTime() + time;
}

uint32_t Unit::getAttackTimer(WeaponDamageType type) const
{
    return m_attackTimer[type];
}

bool Unit::isAttackReady(WeaponDamageType type) const
{
    return Util::getMSTime() >= m_attackTimer[type];
}

void Unit::resetAttackTimer(WeaponDamageType type)
{
    setAttackTimer(type, static_cast<int32_t>(getBaseAttackTime(type) * m_attackSpeed[type]));
}

void Unit::modAttackSpeedModifier(WeaponDamageType type, int32_t amount)
{
    if (amount > 0)
        m_attackSpeed[type] *= 1.0f + static_cast<float>(amount / 100.0f);
    else
        m_attackSpeed[type] /= 1.0f + static_cast<float>((-amount) / 100.0f);
}

float Unit::getAttackSpeedModifier(WeaponDamageType type) const
{
    return m_attackSpeed[type];
}

void Unit::sendEnvironmentalDamageLogPacket(uint64_t guid, uint8_t type, uint32_t damage, uint64_t unk /*= 0*/)
{
    SendMessageToSet(SmsgEnvironmentalDamageLog(guid, type, damage, unk).serialise().get(), true, false);
}

bool Unit::isPvpFlagSet() { return false; }
void Unit::setPvpFlag() {}
void Unit::removePvpFlag() {}

bool Unit::isFfaPvpFlagSet() { return false; }
void Unit::setFfaPvpFlag() {}
void Unit::removeFfaPvpFlag() {}

bool Unit::isSanctuaryFlagSet() { return false; }
void Unit::setSanctuaryFlag() {}
void Unit::removeSanctuaryFlag() {}

bool Unit::isSitting() const
{
    const auto standState = getStandState();
    return
        standState == STANDSTATE_SIT_CHAIR || standState == STANDSTATE_SIT_LOW_CHAIR ||
        standState == STANDSTATE_SIT_MEDIUM_CHAIR || standState == STANDSTATE_SIT_HIGH_CHAIR ||
        standState == STANDSTATE_SIT;
}

uint8_t Unit::getHealthPct() const
{
    if (getHealth() <= 0 || getMaxHealth() <= 0)
        return 0;

    if (getHealth() >= getMaxHealth())
        return 100;

    return static_cast<uint8_t>(getHealth() * 100 / getMaxHealth());
}

bool Unit::isTaggedByPlayerOrItsGroup(Player* tagger)
{
    if (!IsTagged() || tagger == nullptr)
        return false;

    if (GetTaggerGUID() == tagger->getGuid())
        return true;

    if (tagger->InGroup())
    {
        const auto playerTagger = GetMapMgrPlayer(GetTaggerGUID());
        if (playerTagger != nullptr && tagger->GetGroup()->HasMember(playerTagger))
            return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Death
bool Unit::isAlive() const { return m_deathState == ALIVE; }
bool Unit::isDead() const { return  m_deathState != ALIVE; }
bool Unit::justDied() const { return m_deathState == JUST_DIED; }

void Unit::setDeathState(DeathState state)
{
    m_deathState = state;
    if (state == JUST_DIED)
        DropAurasOnDeath();
}

DeathState Unit::getDeathState() const { return m_deathState; }

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

MovementInfo* Unit::getMovementInfo() { return &movement_info; }

uint32_t Unit::getUnitMovementFlags() const { return movement_info.flags; }   //checked
void Unit::setUnitMovementFlags(uint32_t f) { movement_info.flags = f; }
void Unit::addUnitMovementFlag(uint32_t f) { movement_info.flags |= f; }
void Unit::removeUnitMovementFlag(uint32_t f) { movement_info.flags &= ~f; }
bool Unit::hasUnitMovementFlag(uint32_t f) const { return (movement_info.flags & f) != 0; }

//\brief: this is not uint16_t on version < wotlk
uint16_t Unit::getExtraUnitMovementFlags() const { return movement_info.flags2; }
void Unit::addExtraUnitMovementFlag(uint16_t f2) { movement_info.flags2 |= f2; }
bool Unit::hasExtraUnitMovementFlag(uint16_t f2) const { return (movement_info.flags2 & f2) != 0; }

//////////////////////////////////////////////////////////////////////////////////////////
// Vehicle

Vehicle* Unit::getCurrentVehicle() const { return m_currentVehicle; }

void Unit::setCurrentVehicle(Vehicle* vehicle) { m_currentVehicle = vehicle; }

void Unit::addPassengerToVehicle(uint64_t vehicleGuid, uint32_t delay)
{
    if (delay > 0)
    {
        sEventMgr.AddEvent(this, &Unit::addPassengerToVehicle, vehicleGuid, static_cast<uint32_t>(0), 0, delay, 1, 0);
        return;
    }

    if (const auto unit = m_mapMgr->GetUnit(vehicleGuid))
    {
        if (unit->getVehicleComponent() == nullptr)
            return;

        if (m_currentVehicle != nullptr)
            return;

        unit->getVehicleComponent()->AddPassenger(this);
    }
}

Vehicle* Unit::getVehicleComponent() const
{
    return m_vehicle;
}

Unit* Unit::getVehicleBase()
{
    if (m_currentVehicle != nullptr)
        return m_currentVehicle->GetOwner();

    if (m_vehicle != nullptr)
        return this;

    return nullptr;
}

void Unit::sendHopOnVehicle(Unit* vehicleOwner, uint32_t seat)
{
    SendMessageToSet(SmsgMonsterMoveTransport(GetNewGUID(), vehicleOwner->GetNewGUID(), static_cast<uint8_t>(seat), GetPosition()).serialise().get(), true);
}

void Unit::sendHopOffVehicle(Unit* vehicleOwner, LocationVector& /*landPosition*/)
{
    WorldPacket data(SMSG_MONSTER_MOVE, 1 + 12 + 4 + 1 + 4 + 4 + 4 + 12 + 8);
    data << GetNewGUID();

    if (isPlayer())
        data << uint8(1);
    else
        data << uint8(0);

    data << float(GetPositionX());
    data << float(GetPositionY());
    data << float(GetPositionZ());
    data << uint32(Util::getMSTime());
    data << uint8(4);                            // SPLINETYPE_FACING_ANGLE
    data << float(GetOrientation());             // guess
    data << uint32(0x01000000);                  // SPLINEFLAG_EXIT_VEHICLE
    data << uint32(0);                           // Time in between points
    data << uint32(1);                           // 1 single waypoint
    data << float(vehicleOwner->GetPositionX());
    data << float(vehicleOwner->GetPositionY());
    data << float(vehicleOwner->GetPositionZ());

    SendMessageToSet(&data, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Unit Owner
bool Unit::isUnitOwnerInParty(Unit* unit)
{
    if (unit)
    {
        Player* playOwner = getPlayerOwner();
        Player* playerOwnerFromUnit = unit->getPlayerOwner();
        if (playOwner == nullptr || playerOwnerFromUnit == nullptr)
            return false;

        if (playOwner == playerOwnerFromUnit)
            return true;

        if (playOwner->GetGroup() != nullptr
            && playerOwnerFromUnit->GetGroup() != nullptr
            && playOwner->GetGroup() == playerOwnerFromUnit->GetGroup()
            && playOwner->GetSubGroup() == playerOwnerFromUnit->GetSubGroup())
            return true;
    }

    return false;
}

bool Unit::isUnitOwnerInRaid(Unit* unit)
{
    if (unit)
    {
        Player* playerOwner = getPlayerOwner();
        Player* playerOwnerFromUnit = unit->getPlayerOwner();
        if (playerOwner == nullptr || playerOwnerFromUnit == nullptr)
            return false;

        if (playerOwner == playerOwnerFromUnit)
            return true;

        if (playerOwner->GetGroup() != nullptr
            && playerOwnerFromUnit->GetGroup() != nullptr
            && playerOwner->GetGroup() == playerOwnerFromUnit->GetGroup())
            return true;
    }

    return false;
}
