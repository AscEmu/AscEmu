/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Unit.h"

#include "Data/WoWUnit.hpp"
#include "Management/Battleground/Battleground.h"
#include "Management/HonorHandler.h"
#include "Map/MapMgr.h"
#include "Objects/GameObject.h"
#include "Server/Packets/SmsgAuraUpdate.h"
#include "Server/Packets/SmsgAuraUpdateAll.h"
#include "Server/Packets/SmsgClearExtraAuraInfo.h"
#include "Server/Packets/SmsgEmote.h"
#include "Server/Packets/SmsgSetExtraAuraInfo.h"
#include "Server/Packets/SmsgSpellEnergizeLog.h"
#include "Server/Packets/SmsgEnvironmentalDamageLog.h"
#include "Server/Packets/SmsgMonsterMoveTransport.h"
#include "Server/Packets/SmsgPeriodicAuraLog.h"
#include "Server/Packets/SmsgPlaySpellVisual.h"
#include "Server/Packets/SmsgPowerUpdate.h"
#include "Server/Packets/SmsgSpellHealLog.h"
#include "Server/Packets/SmsgSpellOrDamageImmune.h"
#include "Server/Packets/SmsgStandstateUpdate.h"
#include "Server/Packets/SmsgUpdateAuraDuration.h"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Spell/Definitions/DiminishingGroup.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/SpellSchoolConversionTable.h"
#include "Spell/Definitions/SpellTypes.h"
#include "Spell/SpellAuras.h"
#include "Spell/SpellMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Players/Player.h"
#include "Movement/Spline/New/MoveSpline.h"
#include "Movement/Spline/New/MoveSplineInit.h"

using namespace AscEmu::Packets;

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
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
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
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
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

uint32_t Unit::getPower(PowerType type, [[maybe_unused]]bool inRealTime/* = true*/) const
{
    if (type == POWER_TYPE_HEALTH)
        return getHealth();

#if VERSION_STRING >= WotLK
    if (inRealTime)
    {
        // Following power types update in real time since wotlk
        // We cannot update WoWData values in real time, otherwise SMSG_UPDATE_OBJECT is sent every 100ms per player
        // Therefore private variables are updated in real time and WoWData values are updated every 2 sec (blizzlike)
        switch (type)
        {
            case POWER_TYPE_MANA:
                return m_manaAmount;
            case POWER_TYPE_RAGE:
                return m_rageAmount;
            case POWER_TYPE_FOCUS:
                return m_focusAmount;
            case POWER_TYPE_ENERGY:
                return m_energyAmount;
            case POWER_TYPE_RUNIC_POWER:
                return m_runicPowerAmount;
            default:
                break;
        }
    }
#endif

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

    if (getPower(type, false) == value)
        return;

#if VERSION_STRING >= WotLK
    // Sync realtime values with WoWData values
    switch (type)
    {
        case POWER_TYPE_MANA:
            m_manaAmount = value;
            break;
        case POWER_TYPE_RAGE:
            m_rageAmount = value;
            break;
        case POWER_TYPE_FOCUS:
            m_focusAmount = value;
            break;
        case POWER_TYPE_ENERGY:
            m_energyAmount = value;
            break;
        case POWER_TYPE_RUNIC_POWER:
            m_runicPowerAmount = value;
            break;
        default:
            break;
    }

    // Reset update timer
    m_powerUpdatePacketTime = REGENERATION_PACKET_UPDATE_INTERVAL;
#endif

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
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
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
    if (plr != nullptr && plr->IsInWorld() && plr->getGroup() != nullptr)
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
    if (plr != nullptr && plr->IsInWorld() && plr->getGroup() != nullptr)
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
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
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
void Unit::setAura(Aura const* aur, bool apply)
{
    if (aur == nullptr)
        return;

    const auto slot = aur->m_visualSlot;
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const auto spellId = apply ? aur->getSpellId() : 0;
    write(unitData()->aura[slot], spellId);
}

uint32_t Unit::getAuraFlags(uint8_t slot) const { return unitData()->aura_flags[slot]; }
void Unit::setAuraFlags(Aura const* aur, bool apply)
{
    if (aur == nullptr)
        return;

    const auto slot = aur->m_visualSlot;
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    const uint32_t flags = apply ? aur->getAuraFlags() : 0;

    auto val = getAuraFlags(index);
    val &= ~(static_cast<uint32_t>(AFLAG_MASK_ALL) << byte);

    if (flags != 0)
        val |= (flags << byte);

    write(unitData()->aura_flags[index], val);
}

uint32_t Unit::getAuraLevel(uint8_t slot) const { return unitData()->aura_levels[slot]; }
void Unit::setAuraLevel(Aura* aur)
{
    if (aur == nullptr)
        return;

    const auto slot = aur->m_visualSlot;
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    const uint32_t level = aur->GetUnitCaster() != nullptr ? aur->GetUnitCaster()->getLevel() : worldConfig.player.playerLevelCap;

    auto val = getAuraLevel(index);
    val &= ~(0xFF << byte);
    val |= (level << byte);

    write(unitData()->aura_levels[index], val);
}

uint32_t Unit::getAuraApplication(uint8_t slot) const { return unitData()->aura_applications[slot]; }
void Unit::setAuraApplication(Aura const* aur)
{
    if (aur == nullptr)
        return;

    const auto slot = aur->m_visualSlot;
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return;

    const uint8_t index = slot / 4;
    const uint32_t byte = (slot % 4) * 8;

    const uint32_t stackAmount = aur->getSpellInfo()->getMaxstack() > 0 ? aur->getStackCount() : aur->getCharges();
    // Client expects count - 1
    const uint8_t count = stackAmount <= 255 ? stackAmount - 1 : 255 - 1;

    auto val = getAuraApplication(index);
    val &= ~(0xFF << byte);
    val |= (count << byte);

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
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
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
void Unit::setStandState(uint8_t standState)
{
    write(unitData()->field_bytes_1.s.stand_state, standState);

    if (isPlayer())
        static_cast<Player*>(this)->SendPacket(SmsgStandstateUpdate(standState).serialise().get());

    if (standState != STANDSTATE_SIT)
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_STAND_UP);
}

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
    if (plr == nullptr || !plr->IsInWorld() || !plr->getGroup())
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
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_WATER_WALK);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_WATER_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_WATER_WALK);
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
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_LAND_WALK);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_LAND_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_LAND_WALK);
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
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_FEATHER_FALL);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_FEATHER_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_FEATHER_FALL);
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
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_NORMAL_FALL);
#endif
        SendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_NORMAL_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_NORMAL_FALL);
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
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_SET_HOVER);
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
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_HOVER);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_HOVER);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNSET_HOVER);
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
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_SET_CAN_FLY);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            // Remove all fly related moveflags
            removeUnitMovementFlag(MOVEFLAG_CAN_FLY);
#if VERSION_STRING > TBC
            removeUnitMovementFlag(MOVEFLAG_DESCENDING);
#endif
            removeUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32(5);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_CAN_FLY);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_FLYING);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            // Remove all fly related moveflags
            removeUnitMovementFlag(MOVEFLAG_CAN_FLY);
#if VERSION_STRING > TBC
            removeUnitMovementFlag(MOVEFLAG_DESCENDING);
#endif
            removeUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNSET_FLYING);
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
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_ROOT);
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
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_UNROOT);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_ROOT);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNROOT);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_START_SWIM);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_STOP_SWIM);
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
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_DISABLE);
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
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_ENABLE);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_DISABLE);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_ENABLE);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_WALK_MODE);
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
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_RUN_MODE);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::handleFall(MovementInfo const& movementInfo)
{
    if (!z_axisposition)
        z_axisposition = movementInfo.getPosition()->z;

    uint32 falldistance = float2int32(z_axisposition - movementInfo.getPosition()->z);
    if (z_axisposition <= movementInfo.getPosition()->z)
        falldistance = 1;

    if (static_cast<int>(falldistance) > m_safeFall)
        falldistance -= m_safeFall;
    else
        falldistance = 1;

    bool disabledUntil = false;
    if (isPlayer())
        disabledUntil = !dynamic_cast<Player*>(this)->m_cheats.hasGodModeCheat && UNIXTIME >= dynamic_cast<Player*>(this)->m_fallDisabledUntil;

    if (isAlive() && !bInvincible && (falldistance > 12) && !m_noFallDamage && disabledUntil)
    {
        auto health_loss = static_cast<uint32_t>(getHealth() * (falldistance - 12) * 0.017f);
        if (health_loss >= getHealth())
        {
            health_loss = getHealth();
        }
#if VERSION_STRING > TBC
        else if ((falldistance >= 65))
        {
            if (isPlayer())
            {
                dynamic_cast<Player*>(this)->GetAchievementMgr().UpdateAchievementCriteria(
                    ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING,
                    falldistance,
                    dynamic_cast<Player*>(this)->GetDrunkenstateByValue(dynamic_cast<Player*>(this)->GetDrunkValue()),
                    0);
            }
        }
#endif

        sendEnvironmentalDamageLogPacket(getGuid(), DAMAGE_FALL, health_loss);
        addSimpleEnvironmentalDamageBatchEvent(DAMAGE_FALL, health_loss);
    }

    z_axisposition = 0.0f;
}

bool Unit::IsFalling() const
{
    return obj_movement_info.hasMovementFlag(MOVEFLAG_FALLING_MASK) || movespline->isFalling();
}

bool Unit::CanSwim() const
{
    // Mirror client behavior, if this method returns false then client will not use swimming animation and for players will apply gravity as if there was no water
    if (hasUnitFlags(UNIT_FLAG_DEAD))
        return false;
    if (hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE)) // is player
        return true;
#if VERSION_STRING >= TBC
    if (hasUnitFlags2(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE))
        return false;
#endif
    if (isPet() && hasUnitFlags(UNIT_FLAG_PET_IN_COMBAT))
        return true;
    return hasUnitFlags(UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_SWIMMING);
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

void Unit::castSpellLoc(const LocationVector location, uint32_t spellId, bool triggered)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    castSpellLoc(location, spellInfo, triggered);
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

SpellProc* Unit::addProcTriggerSpell(uint32_t spellId, uint32_t originalSpellId, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask/* = nullptr*/, Aura* createdByAura/* = nullptr*/, Object* obj/* = nullptr*/)
{
    return addProcTriggerSpell(sSpellMgr.getSpellInfo(spellId), sSpellMgr.getSpellInfo(originalSpellId), casterGuid, procChance, procFlags, exProcFlags, spellFamilyMask, procClassMask, createdByAura, obj);
}

SpellProc* Unit::addProcTriggerSpell(SpellInfo const* spellInfo, uint64_t casterGuid, Aura* createdByAura/* = nullptr*/, uint32_t const* procClassMask/* = nullptr*/, Object* obj/* = nullptr*/)
{
    return addProcTriggerSpell(spellInfo, spellInfo, casterGuid, spellInfo->getProcChance(), static_cast<SpellProcFlags>(spellInfo->getProcFlags()), EXTRA_PROC_NULL, nullptr, procClassMask, createdByAura, obj);
}

SpellProc* Unit::addProcTriggerSpell(SpellInfo const* spellInfo, Aura* createdByAura, uint64_t casterGuid, uint32_t const* procClassMask/* = nullptr*/, Object* obj/* = nullptr*/)
{
    if (createdByAura == nullptr)
        return nullptr;

    const auto aurSpellInfo = createdByAura->getSpellInfo();
    return addProcTriggerSpell(spellInfo, aurSpellInfo, casterGuid, aurSpellInfo->getProcChance(), static_cast<SpellProcFlags>(aurSpellInfo->getProcFlags()), EXTRA_PROC_NULL, nullptr, procClassMask, createdByAura, obj);
}

SpellProc* Unit::addProcTriggerSpell(SpellInfo const* spellInfo, SpellInfo const* originalSpellInfo, uint64_t casterGuid, uint32_t procChance, uint32_t procFlags, uint32_t const* procClassMask/* = nullptr*/, Aura* createdByAura/* = nullptr*/, Object* obj/* = nullptr*/)
{
    return addProcTriggerSpell(spellInfo, originalSpellInfo, casterGuid, procChance, static_cast<SpellProcFlags>(procFlags), EXTRA_PROC_NULL, nullptr, procClassMask, createdByAura, obj);
}

SpellProc* Unit::addProcTriggerSpell(SpellInfo const* spellInfo, SpellInfo const* originalSpellInfo, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask/* = nullptr*/, Aura* createdByAura/* = nullptr*/, Object* obj/* = nullptr*/)
{
    SpellProc* spellProc = nullptr;
    if (spellInfo != nullptr)
        spellProc = getProcTriggerSpell(spellInfo->getId(), casterGuid);

    if (spellProc != nullptr && !spellProc->isDeleted())
        return spellProc;

    // Create new proc since one did not exist
    spellProc = sSpellProcMgr.newSpellProc(this, spellInfo, originalSpellInfo, casterGuid, procChance, procFlags, exProcFlags, spellFamilyMask, procClassMask, createdByAura, obj);
    if (spellProc == nullptr)
    {
        if (originalSpellInfo != nullptr)
            LogError("Unit::addProcTriggerSpell : Spell id %u tried to add a non-existent spell to Unit %p as SpellProc", originalSpellInfo->getId(), this);
        else
            LogError("Unit::addProcTriggerSpell : Something tried to add a non-existent spell to Unit %p as SpellProc", this);
        return nullptr;
    }

    m_procSpells.push_back(spellProc);
    return spellProc;
}

SpellProc* Unit::getProcTriggerSpell(uint32_t spellId, uint64_t casterGuid) const
{
    for (const auto& spellProc : m_procSpells)
    {
        if (spellProc->getSpell()->getId() == spellId && (casterGuid == 0 || spellProc->getCasterGuid() == casterGuid))
            return spellProc;
    }

    return nullptr;
}

void Unit::removeProcTriggerSpell(uint32_t spellId, uint64_t casterGuid/* = 0*/, uint64_t misc/* = 0*/)
{
    for (auto& spellProc : m_procSpells)
    {
        if (sScriptMgr.callScriptedSpellProcCanDelete(spellProc, spellId, casterGuid, misc))
        {
            spellProc->deleteProc();
            return;
        }

        if (spellProc->canDeleteProc(spellId, casterGuid, misc))
        {
            spellProc->deleteProc();
            return;
        }
    }
}

void Unit::clearProcCooldowns()
{
    for (auto& proc : m_procSpells)
    {
        proc->setLastTriggerTime(0);
    }
}

float_t Unit::applySpellHealingBonus(SpellInfo const* spellInfo, int32_t baseHeal, float_t effectPctModifier/* = 1.0f*/, bool isPeriodic/* = false*/, Spell* castingSpell/* = nullptr*/, Aura* aur/* = nullptr*/)
{
    const auto floatHeal = static_cast<float_t>(baseHeal);
    if (spellInfo->getAttributesExC() & ATTRIBUTESEXC_NO_HEALING_BONUS)
        return floatHeal;

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
                return floatHeal;
            default:
                break;
        }
    }

    float_t bonusHeal = 0.0f, bonusAp = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    if (aur != nullptr)
    {
        bonusHeal = static_cast<float_t>(aur->getHealPowerBonus());
        bonusAp = static_cast<float_t>(aur->getAttackPowerBonus());
    }
    else
    {
        bonusHeal = static_cast<float_t>(HealDoneMod[school]);
        bonusAp = static_cast<float_t>(getAttackPower());
    }

    // Get spell coefficient value
    if (bonusHeal > 0.0f)
    {
        if (!isPeriodic || spellInfo->isChanneled())
        {
            if (spellInfo->spell_coeff_direct > 0.0f)
                bonusHeal *= spellInfo->spell_coeff_direct * effectPctModifier;
            else
                bonusHeal = 0.0f;
        }
        else
        {
            if (spellInfo->spell_coeff_overtime > 0.0f)
                bonusHeal *= spellInfo->spell_coeff_overtime * effectPctModifier;
            else
                bonusHeal = 0.0f;
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

    if (bonusAp > 0.0f)
    {
        // Get attack power bonus
        if (isPeriodic || aur != nullptr)
            bonusAp *= spellInfo->spell_ap_coeff_overtime * effectPctModifier;
        else
            bonusAp *= spellInfo->spell_ap_coeff_direct * effectPctModifier;
    }

    applySpellModifiers(SPELLMOD_PENALTY, &bonusHeal, spellInfo, castingSpell, aur);
    bonusHeal += bonusAp;

    if (isPeriodic && aur != nullptr)
        bonusHeal *= aur->getStackCount();

    float_t heal = floatHeal + std::max(0.0f, bonusHeal);

    if (isPeriodic && aur != nullptr)
        applySpellModifiers(SPELLMOD_PERIODIC_DAMAGE, &heal, spellInfo, nullptr, aur);
    else if (castingSpell != nullptr)
        applySpellModifiers(SPELLMOD_DAMAGE_DONE, &heal, spellInfo, castingSpell, nullptr);

    // Apply pct healing modifiers
    heal += heal * HealDonePctMod[school];

    return heal;
}

float_t Unit::applySpellDamageBonus(SpellInfo const* spellInfo, int32_t baseDmg, float_t effectPctModifier/* = 1.0f*/, bool isPeriodic/* = false*/, Spell* castingSpell/* = nullptr*/, Aura* aur/* = nullptr*/)
{
    const auto floatDmg = static_cast<float_t>(baseDmg);
    if (spellInfo->getAttributesExC() & ATTRIBUTESEXC_NO_DONE_BONUS)
        return floatDmg;

    if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_NOT_USING_DMG_BONUS)
        return floatDmg;

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
                return floatDmg;
            default:
                break;
        }
    }

    float_t bonusDmg = 0.0f, bonusAp = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    if (aur != nullptr)
    {
        bonusDmg = static_cast<float_t>(aur->getSpellPowerBonus());
        bonusAp = static_cast<float_t>(aur->getAttackPowerBonus());
    }
    else
    {
        bonusDmg = static_cast<float_t>(GetDamageDoneMod(school));
        bonusAp = static_cast<float_t>(getAttackPower());
    }

    // Get spell coefficient value
    if (bonusDmg > 0.0f)
    {
        if (!isPeriodic || spellInfo->isChanneled())
        {
            if (spellInfo->spell_coeff_direct > 0.0f)
                bonusDmg *= spellInfo->spell_coeff_direct * effectPctModifier;
            else
                bonusDmg = 0.0f;
        }
        else
        {
            if (spellInfo->spell_coeff_overtime > 0.0f)
                bonusDmg *= spellInfo->spell_coeff_overtime * effectPctModifier;
            else
                bonusDmg = 0.0f;
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

    if (bonusAp > 0.0f)
    {
        // Get attack power bonus
        if (isPeriodic || aur != nullptr)
            bonusAp *= spellInfo->spell_ap_coeff_overtime * effectPctModifier;
        else
            bonusAp *= spellInfo->spell_ap_coeff_direct * effectPctModifier;
    }


    applySpellModifiers(SPELLMOD_PENALTY, &bonusDmg, spellInfo, castingSpell, aur);
    bonusDmg += bonusAp;

    if (isPeriodic && aur != nullptr)
        bonusDmg *= aur->getStackCount();

    float_t dmg = floatDmg + std::max(0.0f, bonusDmg);

    if (isPeriodic && aur != nullptr)
        applySpellModifiers(SPELLMOD_PERIODIC_DAMAGE, &dmg, spellInfo, nullptr, aur);
    else if (castingSpell != nullptr)
        applySpellModifiers(SPELLMOD_DAMAGE_DONE, &dmg, spellInfo, castingSpell, nullptr);

    // Apply pct damage modifiers
    dmg *= GetDamageDonePctMod(school);

    return dmg;
}

float_t Unit::getCriticalChanceForDamageSpell(Spell* spell, Aura* aura, Unit* target)
{
    if (spell == nullptr && aura == nullptr)
        return 0.0f;

    SpellInfo const* spellInfo = nullptr;
    if (spell != nullptr)
        spellInfo = spell->getSpellInfo();
    else
        spellInfo = aura->getSpellInfo();

    ///\ todo: this is mostly copied from legacy method, needs rewrite later
    float_t critChance = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();
    PlayerCombatRating resilienceType = PCR_RANGED_SKILL;

    if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_RANGED)
    {
        if (isPlayer())
        {
            critChance = static_cast<Player const*>(this)->getRangedCritPercentage();
            if (target->isPlayer())
                critChance += static_cast<Player*>(target)->res_R_crit_get();

            critChance += static_cast<float_t>(target->AttackerCritChanceMod[school]);
        }
        else
        {
            // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)
            critChance = 5.0f;
        }

        if (target->isPlayer())
            resilienceType = PCR_RANGED_CRIT_RESILIENCE;
    }
    else if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_MELEE)
    {
        // Same shit with the melee spells, such as Judgment/Seal of Command
        if (isPlayer())
            critChance = static_cast<Player const*>(this)->getMeleeCritPercentage();

        if (target->isPlayer())
        {
            //this could be ability but in that case we overwrite the value
            critChance += static_cast<Player*>(target)->res_M_crit_get();
            resilienceType = PCR_MELEE_CRIT_RESILIENCE;
        }

        // Victim's (!) crit chance mod for physical attacks?
        critChance += static_cast<float_t>(target->AttackerCritChanceMod[0]);
    }
    else
    {
        critChance = spellcritperc + SpellCritChanceSchool[school];

        critChance += static_cast<float_t>(target->AttackerCritChanceMod[school]);

        //\todo Zyres: is tis relly the way this should work?
        if (isPlayer() && (target->m_rootCounter - target->m_stunned))
            critChance += static_cast<float_t>(static_cast<Player const*>(this)->m_RootedCritChanceBonus);

        if (target->isPlayer())
            resilienceType = PCR_SPELL_CRIT_RESILIENCE;
    }

    applySpellModifiers(SPELLMOD_CRITICAL, &critChance, spellInfo, spell, aura);

    if (resilienceType != PCR_RANGED_SKILL)
        critChance -= static_cast<Player*>(target)->CalcRating(resilienceType);

    if (critChance < 0.0f)
        critChance = 0.0f;
    if (critChance > 95.0f)
        critChance = 95.0f;

    return critChance;
}

float_t Unit::getCriticalChanceForHealSpell(Spell* spell, Aura* aura, Unit* /*target*/)
{
    if (spell == nullptr && aura == nullptr)
        return 0.0f;

    SpellInfo const* spellInfo = nullptr;
    if (spell != nullptr)
        spellInfo = spell->getSpellInfo();
    else
        spellInfo = aura->getSpellInfo();

    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    float_t critChance = spellcritperc + SpellCritChanceSchool[school];
    applySpellModifiers(SPELLMOD_CRITICAL, &critChance, spellInfo, spell, aura);

    if (critChance < 0.0f)
        critChance = 0.0f;
    if (critChance > 95.0f)
        critChance = 95.0f;

    return critChance;
}

bool Unit::isCriticalDamageForSpell(Object* target, Spell* spell)
{
    // Spell cannot crit against gameobjects or items
    if (!target->isCreatureOrPlayer())
        return false;

    return Util::checkChance(getCriticalChanceForDamageSpell(spell, nullptr, static_cast<Unit*>(target)));
}

bool Unit::isCriticalHealForSpell(Object* target, Spell* spell)
{
    // Spell cannot crit against gameobjects or items
    if (!target->isCreatureOrPlayer())
        return false;

    return Util::checkChance(getCriticalChanceForHealSpell(spell, nullptr, static_cast<Unit*>(target)));
}

float_t Unit::getCriticalDamageBonusForSpell(float_t damage, Unit* target, Spell* spell, Aura* aura)
{
    SpellInfo const* spellInfo = nullptr;
    if (spell != nullptr)
        spellInfo = spell->getSpellInfo();
    else if (aura != nullptr)
        spellInfo = aura->getSpellInfo();

    int32_t criticalBonus = 100;
    applySpellModifiers(SPELLMOD_CRITICAL_DAMAGE, &criticalBonus, spellInfo, spell, aura);
    if (criticalBonus > 0)
    {
        // The bonuses are halved by 50%
        // todo: verify
        if (spellInfo != nullptr && (spellInfo->getFirstSchoolFromSchoolMask() == SCHOOL_NORMAL || spellInfo->getDmgClass() == SPELL_DMG_TYPE_MELEE || spellInfo->getDmgClass() == SPELL_DMG_TYPE_RANGED))
            damage *= criticalBonus / 100.0f + 1.0f;
        else
            damage *= criticalBonus / 200.0f + 1.0f;
    }

    // Resilience
    // todo: move this elsewhere and correct it to work on all versions
    if (target != nullptr && target->isPlayer())
    {
        float_t dmgReductionPct = 2.0f * static_cast<Player*>(target)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) / 100.0f;
        if (dmgReductionPct > 1.0f)
            dmgReductionPct = 1.0f;

        damage -= damage * dmgReductionPct;
    }

    return damage;
}

float_t Unit::getCriticalHealBonusForSpell(float_t heal, Spell* spell, Aura* aura)
{
    SpellInfo const* spellInfo = nullptr;
    if (spell != nullptr)
        spellInfo = spell->getSpellInfo();
    else if (aura != nullptr)
        spellInfo = aura->getSpellInfo();

    int32_t criticalBonus = 100;
    applySpellModifiers(SPELLMOD_CRITICAL_DAMAGE, &criticalBonus, spellInfo, spell, aura);

    if (criticalBonus > 0)
    {
        // The bonuses are halved by 50%
        // todo: verify
        heal += heal * (criticalBonus / 200.0f);
    }

    return heal;
}

void Unit::sendSpellNonMeleeDamageLog(Object* caster, Object* target, SpellInfo const* spellInfo, uint32_t damage, uint32_t absorbedDamage, uint32_t resistedDamage, uint32_t blockedDamage, [[maybe_unused]]uint32_t overKill, bool isPeriodicDamage, bool isCriticalHit)
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

void Unit::sendAttackerStateUpdate(const WoWGuid& attackerGuid, const WoWGuid& victimGuid, HitStatus hitStatus, uint32_t damage, [[maybe_unused]]uint32_t overKill, DamageInfo damageInfo, uint32_t absorbedDamage, VisualState visualState, uint32_t blockedDamage, [[maybe_unused]]uint32_t rageGain)
{
#if VERSION_STRING < WotLK
    const size_t size = 106;
#else
    const size_t size = 114;
#endif
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, size);

    // School type in classic, school mask in tbc+
    uint32_t school = 0;
#if VERSION_STRING == Classic
    school = damageInfo.getSchoolTypeFromMask();
#else
    school = damageInfo.schoolMask;
#endif

    data << uint32_t(hitStatus);
    data << attackerGuid;
    data << victimGuid;

    data << uint32_t(damage);                                   // real damage
#if VERSION_STRING >= WotLK
    data << uint32_t(overKill);
#endif
    data << uint8_t(1);                                         // damage counter

    data << uint32_t(school);                                   // damage school
    data << float(1.0f);                                        // some sort of damage coefficient
    data << uint32_t(damage);                                   // full damage in int

    if (hitStatus & HITSTATUS_ABSORBED)
        data << uint32_t(absorbedDamage);

    if (hitStatus & HITSTATUS_RESIST)
        data << uint32_t(damageInfo.resistedDamage);

    data << uint8_t(visualState);
    data << uint32_t(0);                                        // unk, can be 0, 1000 or -1
    data << uint32_t(0);                                        // unk, probably GetMeleeSpell

    if (hitStatus & HITSTATUS_BLOCK)
        data << uint32_t(blockedDamage);

#if VERSION_STRING >= WotLK
    if (hitStatus & HITSTATUS_RAGE_GAIN)
        data << uint32_t(rageGain);
#endif

    if (hitStatus & HITSTATUS_UNK_00)                           // debug information
    {
        data << uint32_t(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);

        data << float(0);                                       // Found in loop
        data << float(0);                                       // Found in loop
        data << uint32_t(0);
    }

    SendMessageToSet(&data, true);
}

void Unit::addSpellModifier(AuraEffectModifier const* aurEff, bool apply)
{
    if (aurEff == nullptr)
        return;

    const auto aur = aurEff->getAura();
    if (isPlayer())
    {
        uint8_t groupNum = 0, intBit = 0;
        for (uint8_t bit = 0; bit < SPELL_GROUPS; ++bit, ++intBit)
        {
            if (intBit == 32)
            {
                ++groupNum;
                intBit = 0;
            }

            const uint32_t bitMask = 1 << intBit;
            if (bitMask & aur->getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), groupNum))
            {
                int32_t totalMod = 0;
                for (const auto& mod : m_spellModifiers[aurEff->getEffectMiscValue()])
                {
                    if (mod->getAuraEffectType() != aurEff->getAuraEffectType())
                        continue;
                    if (bitMask & mod->getAura()->getSpellInfo()->getEffectSpellClassMask(mod->getEffectIndex(), groupNum))
                        totalMod += mod->getEffectDamage();
                }
                totalMod += apply ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();

                const auto isPct = aurEff->getAuraEffectType() == SPELL_AURA_ADD_PCT_MODIFIER;
                static_cast<Player*>(this)->sendSpellModifierPacket(bit, static_cast<uint8_t>(aurEff->getEffectMiscValue()), totalMod, isPct);
            }
        }
    }

    if (apply)
        m_spellModifiers[aurEff->getEffectMiscValue()].push_back(aurEff);
    else
        m_spellModifiers[aurEff->getEffectMiscValue()].remove(aurEff);
}

template <typename T> void Unit::applySpellModifiers(SpellModifierType modType, T* value, SpellInfo const* spellInfo, Spell* castingSpell/* = nullptr*/, Aura* castingAura/* = nullptr*/)
{
    if (spellInfo == nullptr)
        return;

    int32_t totalPct = 100, totalFlat = 0;
    getTotalSpellModifiers(modType, value, &totalFlat, &totalPct, spellInfo, castingSpell, castingAura);

    if (totalPct != 100 || totalFlat != 0)
        *value = static_cast<T>((*value + totalFlat) * std::max(0, totalPct) / 100);
}
template void Unit::applySpellModifiers<int32_t>(SpellModifierType modType, int32_t* value, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura);
template void Unit::applySpellModifiers<uint32_t>(SpellModifierType modType, uint32_t* value, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura);
template void Unit::applySpellModifiers<float_t>(SpellModifierType modType, float_t* value, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura);

template <typename T> void Unit::getTotalSpellModifiers(SpellModifierType modType, T baseValue, int32_t* flatMod, int32_t* pctMod, SpellInfo const* spellInfo, Spell* castingSpell/* = nullptr*/, Aura* castingAura/* = nullptr*/, bool checkOnly/* = false*/)
{
    for (const auto& spellMod : m_spellModifiers[modType])
    {
        const auto modSpellInfo = spellMod->getAura()->getSpellInfo();
        if (modSpellInfo->getSpellFamilyName() != spellInfo->getSpellFamilyName())
            continue;
        if (!modSpellInfo->isEffectIndexAffectingSpell(spellMod->getEffectIndex(), spellInfo))
            continue;

        if (spellMod->getAuraEffectType() == SPELL_AURA_ADD_FLAT_MODIFIER)
        {
            *flatMod += spellMod->getEffectDamage();
        }
        else
        {
            // Skip null values for pct mods
            if (baseValue == 0)
                continue;

            *pctMod += spellMod->getEffectDamage();
        }

        // Add the modifier
        if (!checkOnly)
        {
            if (castingSpell != nullptr)
                castingSpell->addUsedSpellModifier(spellMod);
            else if (castingAura != nullptr)
                castingAura->addUsedSpellModifier(spellMod);
        }
    }
}
template void Unit::getTotalSpellModifiers<int32_t>(SpellModifierType modType, int32_t baseValue, int32_t* flatMod, int32_t* pctMod, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura, bool checkOnly);
template void Unit::getTotalSpellModifiers<uint32_t>(SpellModifierType modType, uint32_t baseValue, int32_t* flatMod, int32_t* pctMod, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura, bool checkOnly);
template void Unit::getTotalSpellModifiers<float_t>(SpellModifierType modType, float_t baseValue, int32_t* flatMod, int32_t* pctMod, SpellInfo const* spellInfo, Spell* castingSpell, Aura* castingAura, bool checkOnly);

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
        setAura(aur, true);
        setAuraFlags(aur, true);
        setAuraLevel(aur);
        setAuraApplication(aur);
#endif

        // Send packet
        sendAuraUpdate(aur, false);
        UpdateAuraForGroup(visualSlot);
    }

    aur->applyModifiers(true);
    aur->RelocateEvents();
    aur->takeUsedSpellModifiers();

    // Call scripted aura apply hook
    sScriptMgr.callScriptedAuraOnApply(aur);

    // Sit down if aura is removed on stand up
    //\ todo: move this and aurastates to spellauras.cpp
    if (aur->getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && !isSitting())
        setStandState(STANDSTATE_SIT);

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
            if (caster->m_auras[i]->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_IGNORE_TARGET_AURA_STATE, spellInfo))
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

void Unit::removeAllAurasByAuraEffect(AuraEffect effect, uint32_t skipSpell/* = 0*/, bool removeOnlyEffect/* = false*/)
{
    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] == nullptr)
            continue;

        const auto aur = m_auras[i];
        for (uint8_t x = 0; x < MAX_SPELL_EFFECTS; ++x)
        {
            if (aur->getAuraEffect(x).getAuraEffectType() == SPELL_AURA_NONE)
                continue;

            if (skipSpell == aur->getSpellId())
                continue;

            if (aur->getAuraEffect(x).getAuraEffectType() == effect)
            {
                if (removeOnlyEffect)
                {
                    aur->removeAuraEffect(x);
                }
                else
                {
                    RemoveAura(aur);
                    break;
                }
            }
        }
    }
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

uint32_t Unit::getTransformAura() const
{
    return m_transformAura;
}

void Unit::setTransformAura(uint32_t auraId)
{
    m_transformAura = auraId;
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

    const uint32_t stackAmount = aur->getSpellInfo()->getMaxstack() > 0 ? aur->getStackCount() : aur->getCharges();
    auraUpdate.stackCount = static_cast<uint8_t>(stackAmount <= 255 ? stackAmount : 255);

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
            if (aur->getAuraEffect(i).getAuraEffectType() != SPELL_AURA_NONE)
                auraUpdate.effAmount[i] = aur->getAuraEffect(i).getEffectDamage();
            else
                auraUpdate.effAmount[i] = 0;
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

        const uint32_t stackAmount = aur->getSpellInfo()->getMaxstack() > 0 ? aur->getStackCount() : aur->getCharges();
        auraUpdate.stackCount = static_cast<uint8_t>(stackAmount <= 255 ? stackAmount : 255);

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
                if (aur->getAuraEffect(x).getAuraEffectType() != SPELL_AURA_NONE)
                    auraUpdate.effAmount[x] = aur->getAuraEffect(x).getEffectDamage();
                else
                    auraUpdate.effAmount[x] = 0;
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
    if (spellInfo == nullptr)
        return false;

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

    if (!IsInWorld() || !obj->IsInWorld() || GetMapId() != obj->GetMapId())
        return false;

    // Unit cannot see objects from different phases
    if ((GetPhase() & obj->GetPhase()) == 0)
        return false;

    // Get map view distance (WIP: usually 100 yards for open world and 500 yards for instanced maps)
    //\ todo: there are some objects which should be visible even further and some objects which should always be visible
    const auto viewDistance = GetMapMgr()->m_UpdateDistance;
    if (obj->isGameObject())
    {
        // TODO: for now, all maps have 500 yard view distance
        // problem is that objects on active map cells are updated only if player can see it, iirc

        // Transports should always be visible
        const auto gobj = static_cast<GameObject*>(obj);
        if (gobj->getGoType() == GAMEOBJECT_TYPE_TRANSPORT || gobj->getGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            return true;
        }
        // Gameobjects on transport should always be visible
        else if (gobj->GetTransport() != nullptr)
        {
            return true;
        }
        else
        {
            if (!isInRange(gobj->GetPosition(), viewDistance))
                return false;
        }
    }
    else
    {
        // Creatures on transports should always be visible
        if (obj->isCreature() && dynamic_cast<Creature*>(obj)->hasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            return true;
        else if (!isInRange(obj->GetPosition(), viewDistance))
            return false;
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
            if (playerMe->getCorpseInstanceId() == playerMe->GetInstanceID() &&
                playerObj->isInRange(playerMe->getCorpseLocation(), corpseViewDistance))
                return true;

            // Otherwise player can only see other players who have released their spirits as well
            return playerObj->getDeathState() == CORPSE;
        }

        // Dead player can also see all objects in arena regardless of range
        if (playerMe->m_deathVision)
            return true;

        if (playerMe->getCorpseInstanceId() == GetInstanceID())
        {
            // Player can see his/her own corpse
            if (obj->isCorpse() && static_cast<Corpse*>(obj)->getOwnerGuid() == getGuid())
                return true;

            // Player can see all objects within 40 yards from his/her own corpse
            if (obj->isInRange(playerMe->getCorpseLocation(), corpseViewDistance))
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
                    if (playerMe->getGroup() && playerMe->getGroup() == playerObj->getGroup())
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
                    if (objectOwner->getGroup() && objectOwner->getGroup()->HasMember(static_cast<Player*>(this)))
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
                    if (objectOwner->getGroup() && objectOwner->getGroup()->HasMember(static_cast<Player*>(this)))
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
void Unit::regenerateHealthAndPowers(uint16_t timePassed)
{
    if (!isAlive())
        return;

    if (isCreature() && getNpcFlags() & UNIT_NPC_FLAG_DISABLE_REGEN)
        return;

    // Health
    m_healthRegenerateTimer += timePassed;
    if ((hasUnitStateFlag(UNIT_STATE_POLYMORPHED) && m_healthRegenerateTimer >= REGENERATION_INTERVAL_HEALTH_POLYMORPHED) ||
        (!hasUnitStateFlag(UNIT_STATE_POLYMORPHED) && m_healthRegenerateTimer >= REGENERATION_INTERVAL_HEALTH))
    {
        if (isPlayer())
            static_cast<Player*>(this)->RegenerateHealth(CombatStatus.IsInCombat());
        else
            static_cast<Creature*>(this)->RegenerateHealth();

        m_healthRegenerateTimer = 0;
    }

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
    // This fixes regen values like 0.98
    auto powerResult = currentPower;
    const auto integerAmount = static_cast<uint32_t>(std::fabs(amount));

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

#if VERSION_STRING < WotLK
    setPower(type, powerResult, sendUpdatePacket);
#else
    // In wotlk+ most of the powers regen in real time but we cannot update WoWData values in realtime,
    // so we use private member variables to store power in real time
    switch (type)
    {
        case POWER_TYPE_MANA:
            m_manaAmount = powerResult;
            break;
        case POWER_TYPE_RAGE:
            m_rageAmount = powerResult;
            break;
        case POWER_TYPE_FOCUS:
            m_focusAmount = powerResult;
            break;
        case POWER_TYPE_ENERGY:
            m_energyAmount = powerResult;
            break;
        case POWER_TYPE_RUNIC_POWER:
            m_runicPowerAmount = powerResult;
            break;
        default:
            setPower(type, powerResult, sendUpdatePacket);
            break;
    }
#endif
}

void Unit::interruptHealthRegeneration(uint32_t timeInMS)
{
    m_healthRegenerationInterruptTime = timeInMS;
}

bool Unit::isHealthRegenerationInterrupted() const
{
    return m_healthRegenerateTimer != 0;
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

uint8_t Unit::getHealthPct() const
{
    if (getHealth() <= 0 || getMaxHealth() <= 0)
        return 0;

    if (getHealth() >= getMaxHealth())
        return 100;

    return static_cast<uint8_t>(getHealth() * 100 / getMaxHealth());
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

void Unit::restoreDisplayId()
{
    // Standard transform aura
    Aura* transform = nullptr;
    // Mostly a negative transform
    Aura* forcedTransform = nullptr;

    for (auto i = MAX_TOTAL_AURAS_END - 1; i >= MAX_TOTAL_AURAS_START; --i)
    {
        const auto aur = m_auras[i];
        if (aur == nullptr)
            continue;

        if (!aur->hasAuraEffect(SPELL_AURA_TRANSFORM))
            continue;

        if (transform == nullptr)
            transform = aur;

        // Forced transform takes priority over other transforms
        const auto isForcedTransform = (aur->getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY && aur->getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO) || aur->getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_HIGH_PRIORITY;
        if (isForcedTransform && forcedTransform == nullptr)
            forcedTransform = aur;

#if VERSION_STRING == Classic
        // In Classic skeleton transforms (i.e. Noggenfogger Elixir) are considered forced transforms and they take priority over shapeshifting
        if (aur->getSpellInfo()->getAttributes() == 0x28000000)
            forcedTransform = aur;
#endif

        // Negative aura has highest priority
        if (aur->isNegative())
        {
            forcedTransform = aur;
            break;
        }
    }

    // Priority:
    // 1. negative transform
    // 2. forced transform (and skeleton transforms in Classic)
    // 3. shapeshift
    // 4. other transform
    // 5. native display id
    if (forcedTransform != nullptr)
    {
        // Get display id from aura
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (forcedTransform->getAuraEffect(i).getAuraEffectType() != SPELL_AURA_TRANSFORM)
                continue;

            const auto displayId = forcedTransform->getAuraEffect(i).getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                EventModelChange();
                setTransformAura(forcedTransform->getSpellId());
                return;
            }
        }
    }

    // There can be only one shapeshift aura
    const auto shapeshift = getAuraWithAuraEffect(SPELL_AURA_MOD_SHAPESHIFT);
    if (shapeshift != nullptr)
    {
        // Get display id from aura
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (shapeshift->getAuraEffect(i).getAuraEffectType() != SPELL_AURA_MOD_SHAPESHIFT)
                continue;

            const auto displayId = shapeshift->getAuraEffect(i).getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                EventModelChange();
                return;
            }
        }
    }

    if (transform != nullptr)
    {
        // Get display id from aura
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (transform->getAuraEffect(i).getAuraEffectType() != SPELL_AURA_TRANSFORM)
                continue;

            const auto displayId = transform->getAuraEffect(i).getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                EventModelChange();
                setTransformAura(transform->getSpellId());
                return;
            }
        }
    }

    // No transform aura, no shapeshift aura => use native display id
    setDisplayId(getNativeDisplayId());
    EventModelChange();
}

bool Unit::isSitting() const
{
    const auto standState = getStandState();
    return
        standState == STANDSTATE_SIT_CHAIR || standState == STANDSTATE_SIT_LOW_CHAIR ||
        standState == STANDSTATE_SIT_MEDIUM_CHAIR || standState == STANDSTATE_SIT_HIGH_CHAIR ||
        standState == STANDSTATE_SIT;
}

void Unit::emote(EmoteType emote)
{

#if VERSION_STRING < Cata
    if (emote == 0)
        return;
#endif

    SendMessageToSet(SmsgEmote(emote, this->getGuid()).serialise().get(), true);
}

void Unit::eventAddEmote(EmoteType emote, uint32 time)
{
    m_oldEmote = getEmoteState();
    setEmoteState(emote);
    sEventMgr.AddEvent(this, &Creature::emoteExpire, EVENT_UNIT_EMOTE, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Unit::emoteExpire()
{
    setEmoteState(m_oldEmote);
    sEventMgr.RemoveEvents(this, EVENT_UNIT_EMOTE);
}

uint32_t Unit::getOldEmote() const { return m_oldEmote; }

void Unit::dealDamage(Unit* victim, uint32_t damage, uint32_t spellId, bool removeAuras/* = true*/)
{
    // Not accepted cases
    if (victim == nullptr || !victim->isAlive() || !victim->IsInWorld() || !IsInWorld())
        return;
    if (victim->isPlayer() && static_cast<Player*>(victim)->m_cheats.hasGodModeCheat)
        return;
    if (victim->bInvincible)
        return;
    if (victim->isCreature() && static_cast<Creature*>(victim)->isSpiritHealer())
        return;

    if (this != victim)
    {
        if (isPlayer())
        {
            const auto plr = static_cast<Player*>(this);
            if (!plr->GetSession()->HasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
                damage = plr->CheckDamageLimits(damage, spellId);

            if (plr->CombatStatus.IsInCombat())
                sHookInterface.OnEnterCombat(plr, victim);
        }

        CombatStatus.OnDamageDealt(victim);

        const auto plrOwner = getPlayerOwner();
        if (plrOwner != nullptr)
        {
            if (victim->isCreature() && victim->IsTaggable())
            {
                victim->Tag(plrOwner->getGuid());
                plrOwner->TagUnit(victim);
            }

            // Battleground damage score
            if (plrOwner->m_bg != nullptr && GetMapMgr() == victim->GetMapMgr())
            {
                plrOwner->m_bgScore.DamageDone += damage;
                plrOwner->m_bg->UpdatePvPData();
            }
        }

        if (victim->isPlayer())
        {
            // Make victim's pet react to attacker
            ///\ todo: what about other summons?
            const auto summons = static_cast<Player*>(victim)->GetSummons();
            for (const auto& pet : summons)
            {
                if (pet->GetPetState() != PET_STATE_PASSIVE)
                {
                    pet->GetAIInterface()->AttackReaction(this, 1, 0);
                    pet->HandleAutoCastEvent(AUTOCAST_EVENT_OWNER_ATTACKED);
                }
            }
        }
        else
        {
            // Generate threat
            victim->GetAIInterface()->AttackReaction(this, damage, spellId);
            sScriptMgr.DamageTaken(static_cast<Creature*>(victim), this, &damage);
        }
    }

    victim->setStandState(STANDSTATE_STAND);

    if (victim->isPvpFlagSet())
    {
        const auto plrOwner = getPlayerOwner();
        if (isPet())
        {
            if (!isPvpFlagSet())
                plrOwner->PvPToggle();

            plrOwner->AggroPvPGuards();
        }
        else if (plrOwner != nullptr)
        {
            if (!plrOwner->isPvpFlagSet())
                plrOwner->PvPToggle();

            plrOwner->AggroPvPGuards();
        }
    }

    // Hackfix - Ardent Defender
    if (victim->DamageTakenPctModOnHP35 && victim->hasAuraState(AURASTATE_FLAG_HEALTH35))
        damage = damage - float2int32(damage * victim->DamageTakenPctModOnHP35) / 100;

    if (removeAuras)
    {
        // Check for auras which are interrupted on damage taken
        // But do not remove the aura created by this spell
        if (spellId != 0)
        {
            victim->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellId);
            ///\ todo: fix this, currently used for root and fear auras
            if (Util::checkChance(35.0f))
                victim->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_UNUSED2, spellId);
        }
        else
        {
            victim->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
            ///\ todo: fix this, currently used for root and fear auras
            if (Util::checkChance(35.0f))
                victim->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
        }
    }

    victim->takeDamage(this, damage, spellId);
}

void Unit::takeDamage(Unit* attacker, uint32_t damage, uint32_t spellId)
{
    if (damage >= getHealth())
    {
        if (isTrainingDummy())
        {
            setHealth(1);
            return;
        }

        // Duel check
        if (isPlayer() && static_cast<Player*>(this)->DuelingWith != nullptr)
        {
            setHealth(5);
            static_cast<Player*>(this)->DuelingWith->EndDuel(DUEL_WINNER_KNOCKOUT);
            emote(EMOTE_ONESHOT_BEG);
            return;
        }

        // The attacker must exist here and if it doesn't exist, victim won't die
        if (attacker == nullptr)
            return;

        if (attacker->getPlayerOwner() != nullptr)
        {
            if (attacker->getPlayerOwner()->m_bg != nullptr)
            {
                attacker->getPlayerOwner()->m_bg->HookOnUnitKill(attacker->getPlayerOwner(), this);

                if (isPlayer())
                    attacker->getPlayerOwner()->m_bg->HookOnPlayerKill(attacker->getPlayerOwner(), dynamic_cast<Player*>(this));
            }

            if (isPlayer())
            {
                sHookInterface.OnKillPlayer(attacker->getPlayerOwner(), dynamic_cast<Player*>(this));
            }
            else if (isCreature())
            {
                attacker->getPlayerOwner()->Reputation_OnKilledUnit(this, false);
#ifdef FT_ACHIEVEMENTS
                attacker->getPlayerOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, attacker->GetMapId(), 0, 0);
#endif
            }

            // Check is the unit gray level for attacker
            if (!isGrayLevel(attacker->getPlayerOwner()->getLevel(), getLevel()) && (getGuid() != attacker->getGuid()))
            {
                if (isPlayer())
                {
#ifdef FT_ACHIEVEMENTS
                    attacker->getPlayerOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, attacker->getPlayerOwner()->getAreaId(), 1, 0);
                    attacker->getPlayerOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL, 1, 0, 0);
#endif
                    HonorHandler::OnPlayerKilled(attacker->getPlayerOwner(), dynamic_cast<Player*>(this));
                }

                attacker->getPlayerOwner()->addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);

                if (!sEventMgr.HasEvent(attacker->getPlayerOwner(), EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                    sEventMgr.AddEvent(dynamic_cast<Unit*>(attacker->getPlayerOwner()), &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(attacker->getPlayerOwner(), EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

                attacker->getPlayerOwner()->HandleProc(PROC_ON_KILL, this, nullptr, DamageInfo(), false);
            }

            // Send zone under attack message
            if (isPvpFlagSet())
            {
                auto team = attacker->getPlayerOwner()->getTeam();
                if (team == TEAM_ALLIANCE)
                    team = TEAM_HORDE;
                else
                    team = TEAM_ALLIANCE;

                const auto area = GetArea();
                sWorld.sendZoneUnderAttackMessage(area != nullptr ? area->id : attacker->GetZoneId(), team);
            }
        }

        if (attacker->isPlayer())
        {
            const auto plr = static_cast<Player*>(attacker);

            plr->EventAttackStop();

            plr->sendPartyKillLogPacket(getGuid());
        }

        Die(attacker, damage, spellId);

        // Loot
        if (isLootable())
        {
            const auto tagger = GetMapMgrPlayer(GetTaggerGUID());
            if (tagger != nullptr)
            {
                if (tagger->isInGroup())
                    tagger->getGroup()->SendLootUpdates(this);
                else
                    tagger->SendLootUpdate(this);
            }
        }

        if (!isPet() && getCreatedByGuid() == 0)
        {
            // Experience points
            if (IsTagged())
            {
                const auto taggerUnit = GetMapMgrUnit(GetTaggerGUID());
                const auto tagger = taggerUnit != nullptr ? taggerUnit->getPlayerOwner() : nullptr;
                if (tagger != nullptr)
                {
                    if (tagger->isInGroup())
                    {
                        tagger->GiveGroupXP(this, tagger);
                    }
                    else
                    {
                        auto xp = CalculateXpToGive(this, tagger);
                        if (xp > 0)
                        {
                            tagger->GiveXP(xp, getGuid(), true);

                            // Give XP to pets also
                            if (tagger->GetSummon() != nullptr && tagger->GetSummon()->CanGainXP())
                            {
                                xp = CalculateXpToGive(this, tagger->GetSummon());
                                if (xp > 0)
                                    tagger->GetSummon()->giveXp(xp);
                            }
                        }
                    }

                    if (isCreature())
                    {
                        sQuestMgr.OnPlayerKill(tagger, dynamic_cast<Creature*>(this), true);

#ifdef FT_ACHIEVEMENTS
                        if (tagger->isInGroup())
                        {
                            tagger->getGroup()->UpdateAchievementCriteriaForInrange(this, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, getEntry(), 1, 0);
                            tagger->getGroup()->UpdateAchievementCriteriaForInrange(this, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                        }
                        else
                        {
                            tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, getEntry(), 1, 0);
                            tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                        }
#endif
                    }
                }
            }
#ifdef FT_ACHIEVEMENTS
            else if (attacker->getPlayerOwner() != nullptr && isCritter())
            {
                attacker->getPlayerOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, getEntry(), 1, 0);
                attacker->getPlayerOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
            }
#endif
        }

        // The unit has died so no need to proceed any further
        return;
    }

    if (isPlayer())
    {
        const auto plr = static_cast<Player*>(this);

        // todo: this should be moved to combat handler...
        // atm its called every time player takes damage -Appled
        if (CombatStatus.IsInCombat())
            sHookInterface.OnEnterCombat(dynamic_cast<Player*>(this), attacker);

        // todo: remove this hackfix...
        if (plr->cannibalize)
        {
            sEventMgr.RemoveEvents(plr, EVENT_CANNIBALIZE);
            setEmoteState(EMOTE_ONESHOT_NONE);
            plr->cannibalize = false;
        }
    }

    // Modify health
    modHealth(-1 * static_cast<int32_t>(damage));
}

void Unit::addSimpleDamageBatchEvent(uint32_t damage, Unit* attacker/* = nullptr*/, SpellInfo const* spellInfo/* = nullptr*/)
{
    auto batch = new HealthBatchEvent;
    batch->caster = attacker;
    batch->damageInfo.realDamage = damage;
    batch->spellInfo = spellInfo;
    
    addHealthBatchEvent(batch);
}

void Unit::addSimpleEnvironmentalDamageBatchEvent(EnviromentalDamage type, uint32_t damage, uint32_t absorbedDamage/* = 0*/)
{
    auto batch = new HealthBatchEvent;
    batch->damageInfo.realDamage = damage;
    batch->isEnvironmentalDamage = true;
    batch->environmentType = type;

    // Only fire and lava environmental damage types can be absorbed
    if (type == DAMAGE_FIRE || type == DAMAGE_LAVA)
        batch->damageInfo.absorbedDamage = absorbedDamage;

    addHealthBatchEvent(batch);
}

void Unit::addSimpleHealingBatchEvent(uint32_t heal, Unit* healer/* = nullptr*/, SpellInfo const* spellInfo/* = nullptr*/)
{
    auto batch = new HealthBatchEvent;
    batch->caster = healer;
    batch->damageInfo.realDamage = heal;
    batch->spellInfo = spellInfo;
    batch->isHeal = true;

    addHealthBatchEvent(batch);
}

void Unit::addHealthBatchEvent(HealthBatchEvent* batch)
{
    ARCEMU_ASSERT(batch != nullptr);

    // Do some checks before adding the health event into batch list
    if (!isAlive() || !IsInWorld() || bInvincible)
    {
        delete batch;
        return;
    }

    if (isPlayer())
    {
        const auto plr = static_cast<Player*>(this);
        if (!batch->isHeal && plr->m_cheats.hasGodModeCheat)
        {
            delete batch;
            return;
        }
    }
    else if (isCreature())
    {
        if (static_cast<Creature*>(this)->isSpiritHealer())
        {
            delete batch;
            return;
        }
    }

    m_healthBatch.push_back(batch);
}

uint32_t Unit::calculateEstimatedOverKillForCombatLog(uint32_t damage) const
{
    if (damage == 0 || !isAlive())
        return 0;

    const auto curHealth = getHealth();
    int32_t totalDamage = 0;

    for (const auto& batch : m_healthBatch)
    {
        if (batch->isHeal)
            totalDamage += batch->damageInfo.realDamage;
        else
            totalDamage -= batch->damageInfo.realDamage;
    }

    const int32_t healthValue = curHealth + totalDamage;
    if (healthValue <= 0)
        return damage;
    
    if (damage > static_cast<uint32_t>(healthValue))
        return damage - healthValue;

    return 0;
}

uint32_t Unit::calculateEstimatedOverHealForCombatLog(uint32_t heal) const
{
    if (heal == 0 || !isAlive())
        return 0;

    const auto curHealth = getHealth();
    const auto maxHealth = getMaxHealth();
    int32_t totalHeal = 0;

    for (const auto& batch : m_healthBatch)
    {
        if (batch->isHeal)
            totalHeal += batch->damageInfo.realDamage;
        else
            totalHeal -= batch->damageInfo.realDamage;
    }

    const int32_t healthValue = curHealth + totalHeal;
    if (healthValue < 0)
        return 0;

    const auto healthVal = static_cast<uint32_t>(healthValue);
    if (healthVal >= maxHealth)
        return heal;

    const auto healthDiff = maxHealth - healthVal;
    if (heal > healthDiff)
        return heal - healthDiff;

    return 0;
}

void Unit::clearHealthBatch()
{
    for (auto& itr : m_healthBatch)
        delete itr;

    m_healthBatch.clear();

    // This function is also called on unit death so make sure to remove health based aurastates
    removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH35);
    removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH20);
#if VERSION_STRING >= WotLK
    removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH75);
#endif
}

void Unit::clearCasterFromHealthBatch(Unit const* caster)
{
    for (auto& itr : m_healthBatch)
    {
        if (itr->caster == caster)
            itr->caster = nullptr;
    }
}

uint32_t Unit::absorbDamage(SchoolMask schoolMask, uint32_t* dmg, bool checkOnly/* = true*/)
{
    if (dmg == nullptr)
        return 0;

    uint32_t totalAbsorbedDamage = 0;
    for (auto& aur : m_auras)
    {
        if (aur == nullptr || !aur->isAbsorbAura())
            continue;

        AbsorbAura* abs = static_cast<AbsorbAura*>(aur);
        totalAbsorbedDamage += abs->absorbDamage(schoolMask, dmg, checkOnly);
    }

    if (isPlayer() && static_cast<Player*>(this)->m_cheats.hasGodModeCheat)
    {
        totalAbsorbedDamage += *dmg;
        *dmg = 0;
    }

    return totalAbsorbedDamage;
}

bool Unit::isTaggedByPlayerOrItsGroup(Player* tagger)
{
    if (!IsTagged() || tagger == nullptr)
        return false;

    if (GetTaggerGUID() == tagger->getGuid())
        return true;

    if (tagger->isInGroup())
    {
        const auto playerTagger = GetMapMgrPlayer(GetTaggerGUID());
        if (playerTagger != nullptr && tagger->getGroup()->HasMember(playerTagger))
            return true;
    }

    return false;
}

void Unit::_updateHealth()
{
    const auto curHealth = getHealth();
    int32_t healthVal = 0;
    uint32_t totalAbsorbDamage = 0;
    // Victim's rage generation on damage taken
    int32_t totalRageGenerated = 0;

    Unit* killer = nullptr;

    // Process through health batch
    auto batchItr = m_healthBatch.begin();
    while (batchItr != m_healthBatch.end())
    {
        auto batch = *batchItr;

        if (batch->isHeal)
        {
            uint32_t absorbedHeal = 0;
            const auto heal = _handleBatchHealing(batch, &absorbedHeal);
            healthVal += heal;

            const int32_t diff = curHealth + healthVal;
            // Check if unit got healed in the same batch where it received a fataling blow
            if (diff > 0)
            {
                // Reset killer
                killer = nullptr;
            }
        }
        else
        {
            uint32_t rageGenerated = 0;
            const auto damage = _handleBatchDamage(batch, &rageGenerated);
            healthVal -= damage;

            totalAbsorbDamage += batch->damageInfo.absorbedDamage;
            totalRageGenerated += rageGenerated;

            const int32_t diff = curHealth + healthVal;
            if (diff <= 0)
            {
                // Set killer if health event has overkill and killer does not exist yet
                if (killer == nullptr)
                    killer = batch->caster;
            }
        }

        delete *batchItr;
        batchItr = m_healthBatch.erase(batchItr);
    }

    // Do the real absorb damage
    absorbDamage(SCHOOL_MASK_ALL, &totalAbsorbDamage, false);

    // If the value is negative, then damage in the batch exceeds healing and unit takes damage
    if (healthVal < 0)
        takeDamage(killer, static_cast<uint32_t>(std::abs(healthVal)), 0);
    else
        setHealth(curHealth + healthVal);

    // Generate rage on damage taken
    if (totalRageGenerated > 0)
        modPower(POWER_TYPE_RAGE, totalRageGenerated);

    // Update health based aurastates
    const auto healthPct = getHealthPct();

    // This is for some reason called after death so make sure unit has health
    // This prevents Execute/Hammer of Wrath/Kill Shot showing on dead units
    if (healthPct == 0)
        return;

    // Health below 35%
    if (healthPct < 35)
        addAuraStateAndAuras(AURASTATE_FLAG_HEALTH35);
    else
        removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH35);

    // Health below 20%
    if (healthPct < 20)
        addAuraStateAndAuras(AURASTATE_FLAG_HEALTH20);
    else
        removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH20);

#if VERSION_STRING >= WotLK
    // Health above 75%
    if (healthPct > 75)
        addAuraStateAndAuras(AURASTATE_FLAG_HEALTH75);
    else
        removeAuraStateAndAuras(AURASTATE_FLAG_HEALTH75);
#endif
}

uint32_t Unit::_handleBatchDamage(HealthBatchEvent const* batch, uint32_t* rageGenerated)
{
    const auto spellId = batch->spellInfo != nullptr ? batch->spellInfo->getId() : 0;
    const uint8_t absSchool = batch->spellInfo != nullptr ? batch->spellInfo->getFirstSchoolFromSchoolMask() : 0;
    auto damage = batch->damageInfo.realDamage;

    const auto attacker = batch->caster;
    if (attacker != nullptr && attacker != this)
    {
        if (attacker->isPlayer())
        {
            const auto plr = static_cast<Player*>(attacker);
            if (!plr->GetSession()->HasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
                damage = plr->CheckDamageLimits(damage, spellId);

            //\ todo: this hook is called here and in takeDamage... sort this out
            if (plr->CombatStatus.IsInCombat())
                sHookInterface.OnEnterCombat(plr, this);
        }

        // Rage generation for victim
        ///\ todo: this is inaccurate
        if (getPowerType() == POWER_TYPE_RAGE)
        {
            const auto level = static_cast<float_t>(getLevel());
            const float_t c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

            float_t val = 2.5f * damage / c;
            const auto rage = getPower(POWER_TYPE_RAGE);

            if (rage + float2int32(val) > 1000)
                val = 1000.0f - static_cast<float>(getPower(POWER_TYPE_RAGE));

            val *= 10.0;
            *rageGenerated = float2int32(val);
        }

        attacker->CombatStatus.OnDamageDealt(this);

        const auto plrOwner = attacker->getPlayerOwner();
        if (plrOwner != nullptr)
        {
            if (isCreature() && IsTaggable())
            {
                Tag(plrOwner->getGuid());
                plrOwner->TagUnit(this);
            }

            // Battleground damage score
            if (plrOwner->m_bg != nullptr && GetMapMgr() == attacker->GetMapMgr())
            {
                plrOwner->m_bgScore.DamageDone += damage;
                plrOwner->m_bg->UpdatePvPData();
            }
        }

        if (isPvpFlagSet())
        {
            if (attacker->isPet())
            {
                if (!attacker->isPvpFlagSet())
                    plrOwner->PvPToggle();

                plrOwner->AggroPvPGuards();
            }
            else if (plrOwner != nullptr)
            {
                if (!plrOwner->isPvpFlagSet())
                    plrOwner->PvPToggle();

                plrOwner->AggroPvPGuards();
            }
        }

        if (isPlayer())
        {
            // Make victim's pet react to attacker
            ///\ todo: what about other summons?
            const auto summons = static_cast<Player*>(this)->GetSummons();
            for (const auto& pet : summons)
            {
                if (pet->GetPetState() != PET_STATE_PASSIVE)
                {
                    pet->GetAIInterface()->AttackReaction(attacker, 1, 0);
                    pet->HandleAutoCastEvent(AUTOCAST_EVENT_OWNER_ATTACKED);
                }
            }
        }
        else
        {
            // Generate threat
            GetAIInterface()->AttackReaction(attacker, damage, spellId);
            sScriptMgr.DamageTaken(static_cast<Creature*>(this), attacker, &damage);
        }

        // Hackfix - Ardent Defender
        if (DamageTakenPctModOnHP35 && hasAuraState(AURASTATE_FLAG_HEALTH35))
            damage = damage - float2int32(damage * DamageTakenPctModOnHP35) / 100;
    }

    // Create heal effect for leech effects
    if (batch->isLeech && attacker != nullptr && attacker != this)
    {
        // Leech damage can be more than victim's health but the heal should not exceed victim's remaining health
        auto healAmount = std::min(damage, getHealth());
        if (batch->spellInfo != nullptr && batch->spellInfo->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEALTH_FUNNEL))
        {
            // but health funnel periodic effects can heal more than victim's health
            healAmount = damage;
        }

        attacker->doSpellHealing(attacker, spellId, healAmount * batch->leechMultipleValue, false, batch->isPeriodic, true, false);
    }

    setStandState(STANDSTATE_STAND);

    // Check for auras which are interrupted on damage taken
    // But do not remove the aura created by this spell
    if (spellId != 0)
    {
        RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellId);
        ///\ todo: fix this, currently used for root and fear auras
        if (Util::checkChance(35.0f))
            RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_UNUSED2, spellId);
    }
    else
    {
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
        ///\ todo: fix this, currently used for root and fear auras
        if (Util::checkChance(35.0f))
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
    }

    return damage;
}

uint32_t Unit::_handleBatchHealing(HealthBatchEvent const* batch, uint32_t* absorbedHeal)
{
    auto healing = batch->damageInfo.realDamage;

    // Handle heal absorb
    ///\ todo: implement (aura effect 301)
    *absorbedHeal = 0;

    const auto healer = batch->caster;
    if (healer != nullptr)
    {
        const auto plrOwner = healer->getPlayerOwner();

        // Healing a flagged unit will flag the caster
        if (isPvpFlagSet())
        {
            if (healer->isPet())
            {
                if (!healer->isPvpFlagSet())
                    plrOwner->PvPToggle();
            }
            else if (plrOwner != nullptr)
            {
                if (!plrOwner->isPvpFlagSet())
                    plrOwner->PvPToggle();
            }
        }

        // Update battleground score
        if (plrOwner != nullptr && plrOwner->m_bg != nullptr && plrOwner->GetMapMgr() == GetMapMgr())
        {
            plrOwner->m_bgScore.HealingDone += healing;
            plrOwner->m_bg->UpdatePvPData();
        }

        // Handle threat
        std::vector<Unit*> target_threat;
        int count = 0;
        for (const auto& itr : healer->getInRangeObjectsSet())
        {
            if (!itr || !itr->isCreature())
                continue;

            const auto tmp_creature = static_cast<Creature*>(itr);

            if (!tmp_creature->CombatStatus.IsInCombat() || (tmp_creature->GetAIInterface()->getThreatByPtr(healer) == 0 && tmp_creature->GetAIInterface()->getThreatByPtr(this) == 0))
                continue;

            if (!(healer->GetPhase() & itr->GetPhase()))     //Can't see, can't be a threat
                continue;

            target_threat.push_back(tmp_creature);
            count++;
        }
        
        if (count != 0)
        {
            auto heal_threat = healing / count;

            for (const auto& itr : target_threat)
            {
                itr->GetAIInterface()->HealReaction(healer, this, batch->spellInfo, heal_threat);
            }
        }

        if (IsInWorld() && healer->IsInWorld())
            healer->CombatStatus.WeHealed(this);
    }

    RemoveAurasByHeal();

    return healing;
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

MovementInfo* Unit::getMovementInfo() { return &obj_movement_info; }

uint32_t Unit::getUnitMovementFlags() const { return obj_movement_info.flags; }   //checked
void Unit::setUnitMovementFlags(uint32_t f) { obj_movement_info.flags = f; }
void Unit::addUnitMovementFlag(uint32_t f) { obj_movement_info.flags |= f; }
void Unit::removeUnitMovementFlag(uint32_t f) { obj_movement_info.flags &= ~f; }
bool Unit::hasUnitMovementFlag(uint32_t f) const { return (obj_movement_info.flags & f) != 0; }

//\brief: this is not uint16_t on version < wotlk
uint16_t Unit::getExtraUnitMovementFlags() const { return obj_movement_info.flags2; }
void Unit::addExtraUnitMovementFlag(uint16_t f2) { obj_movement_info.flags2 |= f2; }
bool Unit::hasExtraUnitMovementFlag(uint16_t f2) const { return (obj_movement_info.flags2 & f2) != 0; }

//////////////////////////////////////////////////////////////////////////////////////////
// Summons

TotemSummon* Unit::getTotem(TotemSlots slot) const
{
    if (slot >= MAX_TOTEM_SLOT)
        return nullptr;

    return getSummonInterface()->getTotemInSlot(slot);
}

SummonHandler* Unit::getSummonInterface() const
{
    return m_summonInterface;
}

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

        if (playOwner->getGroup()
            && playerOwnerFromUnit->getGroup()
            && playOwner->getGroup() == playerOwnerFromUnit->getGroup()
            && playOwner->getSubGroupSlot() == playerOwnerFromUnit->getSubGroupSlot())
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

        if (playerOwner->getGroup()
            && playerOwnerFromUnit->getGroup()
            && playerOwner->getGroup() == playerOwnerFromUnit->getGroup())
            return true;
    }

    return false;
}

uint64_t Unit::getTransGuid()
{
    if (getCurrentVehicle())
        return getVehicleBase()->getGuid();
    if (GetTransport())
        return GetTransport()->getGuid();

    return 0;
}
