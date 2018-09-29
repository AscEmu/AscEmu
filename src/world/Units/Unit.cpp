/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Unit.h"
#include "Server/Packets/Opcode.h"
#include "Server/WorldSession.h"
#include "Players/Player.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/DiminishingGroup.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Data/WoWUnit.h"
#include "Storage/MySQLDataStore.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

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
uint8_t Unit::getRace() const { return unitData()->field_bytes_0.s.race; }
void Unit::setRace(uint8_t race) { write(unitData()->field_bytes_0.s.race, race); }

uint8_t Unit::getClass() const { return unitData()->field_bytes_0.s.unit_class; }
void Unit::setClass(uint8_t class_) { write(unitData()->field_bytes_0.s.unit_class, class_); }

uint8_t Unit::getGender() const { return unitData()->field_bytes_0.s.gender; }
void Unit::setGender(uint8_t gender) { write(unitData()->field_bytes_0.s.gender, gender); }

uint8_t Unit::getPowerType() const { return unitData()->field_bytes_0.s.power_type; }
void Unit::setPowerType(uint8_t powerType) { write(unitData()->field_bytes_0.s.power_type, powerType); }
//bytes_0 end

uint32_t Unit::getHealth() const { return unitData()->health; }
void Unit::setHealth(uint32_t health) { write(unitData()->health, health); }

uint32_t Unit::getMaxHealth() const { return unitData()->max_health; }
void Unit::setMaxHealth(uint32_t maxHealth) { write(unitData()->max_health, maxHealth); }

void Unit::setMaxMana(uint32_t maxMana) { write(unitData()->max_mana, maxMana); }

uint32_t Unit::getLevel() const { return unitData()->level; }
void Unit::setLevel(uint32_t level)
{
    write(unitData()->level, level);
    if (isPlayer())
        static_cast<Player*>(this)->setNextLevelXp(sMySQLStore.getPlayerXPForLevel(level));
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
void Unit::setDisplayId(uint32_t id) { write(unitData()->display_id, id); }

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
uint8_t Unit::getStandState() const { return unitData()->field_bytes_1.s.stand_state; }
void Unit::setStandState(uint8_t standState) { write(unitData()->field_bytes_1.s.stand_state, standState); }

uint8_t Unit::getPetTalentPoints() const { return unitData()->field_bytes_1.s.pet_talent_points; }
void Unit::setPetTalentPoints(uint8_t talentPoints) { write(unitData()->field_bytes_1.s.pet_talent_points, talentPoints); }

uint8_t Unit::getStandStateFlags() const { return unitData()->field_bytes_1.s.stand_state_flag; }
void Unit::setStandStateFlags(uint8_t standStateFlags) { write(unitData()->field_bytes_1.s.stand_state_flag, standStateFlags); }

uint8_t Unit::getAnimationFlags() const { return unitData()->field_bytes_1.s.animation_flag; }
void Unit::setAnimationFlags(uint8_t animationFlags) { write(unitData()->field_bytes_1.s.animation_flag, animationFlags); }
//bytes_1 end

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

uint32_t Unit::getBaseMana() const { return unitData()->base_mana; }
void Unit::setBaseMana(uint32_t baseMana) { write(unitData()->base_mana, baseMana); }

uint32_t Unit::getBaseHealth() const { return unitData()->base_health; }
void Unit::setBaseHealth(uint32_t baseHealth) { write(unitData()->base_health, baseHealth); }

//byte_2 begin
uint8_t Unit::getSheathType() const { return unitData()->field_bytes_2.s.sheath_type; }
void Unit::setSheathType(uint8_t sheathType) { write(unitData()->field_bytes_2.s.sheath_type, sheathType); }

uint8_t Unit::getPvpFlags() const { return unitData()->field_bytes_2.s.pvp_flag; }
void Unit::setPvpFlags(uint8_t pvpFlags) { write(unitData()->field_bytes_2.s.pvp_flag, pvpFlags); }

uint8_t Unit::getPetFlags() const { return unitData()->field_bytes_2.s.pet_flag; }
void Unit::setPetFlags(uint8_t petFlags) { write(unitData()->field_bytes_2.s.pet_flag, petFlags); }

uint8_t Unit::getShapeShiftForm() const { return unitData()->field_bytes_2.s.shape_shift_form; }
void Unit::setShapeShiftForm(uint8_t shapeShiftForm) { write(unitData()->field_bytes_2.s.shape_shift_form, shapeShiftForm); }
//bytes_2 end

uint32_t Unit::getAttackPower() const { return unitData()->attack_power; }
void Unit::setAttackPower(uint32_t value) { write(unitData()->attack_power, value); }

float Unit::getMinRangedDamage() const { return unitData()->minimum_ranged_damage; }
void Unit::setMinRangedDamage(float damage) { write(unitData()->minimum_ranged_damage, damage); }

float Unit::getMaxRangedDamage() const { return unitData()->maximum_ranged_ddamage; }
void Unit::setMaxRangedDamage(float damage) { write(unitData()->maximum_ranged_ddamage, damage); }

float Unit::getPowerCostMultiplier(uint16_t school) const { return unitData()->power_cost_multiplier[school]; }
void Unit::setPowerCostMultiplier(uint16_t school, float multiplier) { write(unitData()->power_cost_multiplier[school], multiplier); }
void Unit::modPowerCostMultiplier(uint16_t school, float multiplier)
{
    float currentMultiplier = getPowerCostMultiplier(school);
    currentMultiplier += multiplier;
    setPowerCostMultiplier(school, currentMultiplier);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#ifdef AE_TBC
uint32_t Unit::addAuraVisual(uint32_t spell_id, uint32_t count, bool positive)
{
    bool out;
    return addAuraVisual(spell_id, count, positive, out);
}

uint32_t Unit::addAuraVisual(uint32_t spell_id, uint32_t count, bool positive,
    bool& skip_client_update)
{
    auto free = -1;
    uint32_t start = positive ? MAX_POSITIVE_VISUAL_AURAS_START : MAX_POSITIVE_VISUAL_AURAS_END;
    uint32_t end = positive ? MAX_NEGATIVE_VISUAL_AURAS_START : MAX_NEGATIVE_VISUAL_AURAS_END;

    for (auto x = start; x < end; ++x)
    {
        if (free == -1 && m_uint32Values[UNIT_FIELD_AURA + x] == 0)
            free = x;

        if (m_uint32Values[UNIT_FIELD_AURA + x] == spell_id)
        {
            const auto aura = m_auras[x];
            ModVisualAuraStackCount(aura, count);
            skip_client_update = true;
            return free;
        }
    }

    skip_client_update = false;

    if (free == -1)
        return 0xff;

    const auto flag_slot = static_cast<uint8_t>((free / 4));
    const uint16_t val_slot = UNIT_FIELD_AURAFLAGS + flag_slot;
    auto value = m_uint32Values[val_slot];
    const auto aura_pos = free % 4 * 8;
    value &= ~(0xff << aura_pos);
    if (positive)
        value |= 0x1f << aura_pos;
    else
        value |= 0x9 << aura_pos;

    m_uint32Values[val_slot] = value;
    m_uint32Values[UNIT_FIELD_AURA + free] = spell_id;
    const auto aura = m_auras[free];
    ModVisualAuraStackCount(aura, 1);
    setAuraSlotLevel(free, positive);

    return free;
}

void Unit::setAuraSlotLevel(uint32_t slot, bool positive)
{
    const auto index = slot / 4;
    auto value = m_uint32Values[UNIT_FIELD_AURALEVELS + index];
    const auto bit = slot % 4 * 8;
    value &= ~(0xff << bit);
    if (positive)
        value |= 0x46 << bit;
    else
        value |= 0x19 << bit;

    m_uint32Values[UNIT_FIELD_AURALEVELS + index] = value;
}
#endif

void Unit::setMoveWaterWalk()
{
    AddUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_WATER_WALK, 12);
#if VERSION_STRING != Cata
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
#if VERSION_STRING != Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_WATER_WALK);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveLandWalk()
{
    RemoveUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_LAND_WALK, 12);
#if VERSION_STRING != Cata
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
#if VERSION_STRING != Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_LAND_WALK);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveFeatherFall()
{
    AddUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_FEATHER_FALL, 12);
#if VERSION_STRING != Cata
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
#if VERSION_STRING != Cata
        data << GetNewGUID();
#else
        movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_FEATHER_FALL);
#endif
        SendMessageToSet(&data, false);
    }
}

void Unit::setMoveNormalFall()
{
    RemoveUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_NORMAL_FALL, 12);
#if VERSION_STRING != Cata
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
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
#if VERSION_STRING != Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_SET_HOVER);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_HOVER);

            setAnimationFlags(UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_HOVER, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_HOVER);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_HOVER);

            setAnimationFlags(getAnimationFlags() &~UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_HOVER, 10);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            RemoveUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 13);
#if VERSION_STRING != Cata
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
            RemoveUnitMovementFlag(MOVEFLAG_CAN_FLY);
            RemoveUnitMovementFlag(MOVEFLAG_DESCENDING);
            RemoveUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            RemoveUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_FLYING);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            // Remove all fly related moveflags
            RemoveUnitMovementFlag(MOVEFLAG_CAN_FLY);
            RemoveUnitMovementFlag(MOVEFLAG_DESCENDING);
            RemoveUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 10);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 12);
#if VERSION_STRING != Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_ROOT);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 12);
#if VERSION_STRING != Cata
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

            AddUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_ROOT, 9);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_ROOT);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            m_aiInterface->m_canMove = true;

            RemoveUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 9);
#if VERSION_STRING != Cata
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
    return HasUnitMovementFlag(MOVEFLAG_ROOTED);
}

void Unit::setMoveSwim(bool set_swim)
{
    if (isCreature())
    {
        if (set_swim)
        {
            AddUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_START_SWIM, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_START_SWIM);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_STOP_SWIM, 10);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_DISABLE, 13);
#if VERSION_STRING != Cata
            data << GetNewGUID();
            data << uint32(0);
#else
            movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_DISABLE);
#endif
            SendMessageToSet(&data, true);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_ENABLE, 13);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_DISABLE, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_DISABLE);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 10);
#if VERSION_STRING != Cata
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
            AddUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_WALK_MODE, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_WALK_MODE);
#endif
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_RUN_MODE, 10);
#if VERSION_STRING != Cata
            data << GetNewGUID();
#else
            movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_RUN_MODE);
#endif
            SendMessageToSet(&data, false);
        }
    }
}

float Unit::getSpeedForType(UnitSpeedType speed_type, bool get_basic) const
{
    switch (speed_type)
    {
        case TYPE_WALK:
            return (get_basic ? m_basicSpeedWalk : m_currentSpeedWalk);
        case TYPE_RUN:
            return (get_basic ? m_basicSpeedRun : m_currentSpeedRun);
        case TYPE_RUN_BACK:
            return (get_basic ? m_basicSpeedRunBack : m_currentSpeedRunBack);
        case TYPE_SWIM:
            return (get_basic ? m_basicSpeedSwim : m_currentSpeedSwim);
        case TYPE_SWIM_BACK:
            return (get_basic ? m_basicSpeedSwimBack : m_currentSpeedSwimBack);
        case TYPE_TURN_RATE:
            return (get_basic ? m_basicTurnRate : m_currentTurnRate);
        case TYPE_FLY:
            return (get_basic ? m_basicSpeedFly : m_currentSpeedFly);
        case TYPE_FLY_BACK:
            return (get_basic ? m_basicSpeedFlyBack : m_currentSpeedFlyBack);
        case TYPE_PITCH_RATE:
            return (get_basic ? m_basicPitchRate : m_currentPitchRate);
        default:
            return m_basicSpeedWalk;
    }
}

float Unit::getFlySpeed() const
{
    return getSpeedForType(TYPE_FLY);
}

float Unit::getSwimSpeed() const
{
    return getSpeedForType(TYPE_SWIM);
}

float Unit::getRunSpeed() const
{
    return getSpeedForType(TYPE_RUN);
}

UnitSpeedType Unit::getFastestSpeedType() const
{
    auto fastest_speed = 0.f;
    auto fastest_speed_type = TYPE_WALK;
    for (uint32_t i = TYPE_WALK; i <= TYPE_PITCH_RATE; ++i)
    {
        const auto speedType = static_cast<UnitSpeedType>(i + 1);

        switch(speedType)
        {
        case TYPE_TURN_RATE:
        case TYPE_PITCH_RATE:
            continue;
            default:
            break;
        }

        const auto speed = getSpeedForType(speedType);

        fastest_speed = speed > fastest_speed ? speed : fastest_speed;
        fastest_speed_type = speed == fastest_speed ? speedType : fastest_speed_type;
    }
    return fastest_speed_type;
}


float Unit::getFastestSpeed() const
{
    return getSpeedForType(getFastestSpeedType());
}


void Unit::setSpeedForType(UnitSpeedType speed_type, float speed, bool set_basic)
{
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            if (set_basic)
                m_basicSpeedWalk = speed;
            else
                m_currentSpeedWalk = speed;
        } break;
        case TYPE_RUN:
        {
            if (set_basic)
                m_basicSpeedRun = speed;
            else
                m_currentSpeedRun = speed;
        } break;
        case TYPE_RUN_BACK:
        {
            if (set_basic)
                m_basicSpeedRunBack = speed;
            else
                m_currentSpeedRunBack = speed;
        } break;
        case TYPE_SWIM:
        {
            if (set_basic)
                m_basicSpeedSwim = speed;
            else
                m_currentSpeedSwim = speed;
        } break;
        case TYPE_SWIM_BACK:
        {
            if (set_basic)
                m_basicSpeedSwimBack = speed;
            else
                m_currentSpeedSwimBack = speed;
        } break;
        case TYPE_TURN_RATE:
        {
            if (set_basic)
                m_basicTurnRate = speed;
            else
                m_currentTurnRate = speed;
        } break;
        case TYPE_FLY:
        {
            if (set_basic)
                 m_basicSpeedFly = speed;
            else
                m_currentSpeedFly = speed;
        } break;
        case TYPE_FLY_BACK:
        {
            if (set_basic)
                m_basicSpeedFlyBack = speed;
            else
                m_currentSpeedFlyBack = speed;
        } break;
        case TYPE_PITCH_RATE:
        {
            if (set_basic)
                m_basicPitchRate = speed;
            else
                m_currentPitchRate = speed;
        } break;
    }

    Player* player_mover = GetMapMgrPlayer(getCharmedByGuid());
    if (player_mover == nullptr)
    {
        if (isPlayer())
            player_mover = dynamic_cast<Player*>(this);
    }

    if (player_mover != nullptr)
    {
#if VERSION_STRING != Cata
        player_mover->sendForceMovePacket(speed_type, speed);
#endif
        player_mover->sendMoveSetSpeedPaket(speed_type, speed);
    }
    else
    {
        sendMoveSplinePaket(speed_type);
    }

}

void Unit::resetCurrentSpeed()
{
    m_currentSpeedWalk = m_basicSpeedWalk;
    m_currentSpeedRun = m_basicSpeedRun;
    m_currentSpeedRunBack = m_basicSpeedRunBack;
    m_currentSpeedSwim = m_basicSpeedSwim;
    m_currentSpeedSwimBack = m_basicSpeedSwimBack;
    m_currentTurnRate = m_basicTurnRate;
    m_currentSpeedFly = m_basicSpeedFly;
    m_currentSpeedFlyBack = m_basicSpeedFlyBack;
    m_currentPitchRate = m_basicPitchRate;
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
    data << float(getSpeedForType(speedType));

    SendMessageToSet(&data, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

void Unit::playSpellVisual(uint64_t guid, uint32_t spell_id)
{
#if VERSION_STRING != Cata
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << uint64_t(guid);
    data << uint32_t(spell_id);

    if (isPlayer())
        static_cast<Player*>(this)->SendMessageToSet(&data, true);
    else
        SendMessageToSet(&data, false);
#else
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4 + 8);
    data << uint32_t(0);
    data << uint32_t(spell_id);

    data << uint32_t(guid == getGuid() ? 1 : 0);

    ObjectGuid targetGuid = guid;
    data.writeBit(targetGuid[4]);
    data.writeBit(targetGuid[7]);
    data.writeBit(targetGuid[5]);
    data.writeBit(targetGuid[3]);
    data.writeBit(targetGuid[1]);
    data.writeBit(targetGuid[2]);
    data.writeBit(targetGuid[0]);
    data.writeBit(targetGuid[6]);

    data.flushBits();

    data.WriteByteSeq(targetGuid[0]);
    data.WriteByteSeq(targetGuid[4]);
    data.WriteByteSeq(targetGuid[1]);
    data.WriteByteSeq(targetGuid[6]);
    data.WriteByteSeq(targetGuid[7]);
    data.WriteByteSeq(targetGuid[2]);
    data.WriteByteSeq(targetGuid[3]);
    data.WriteByteSeq(targetGuid[5]);

    SendMessageToSet(&data, true);
#endif
}

void Unit::applyDiminishingReturnTimer(uint32_t* duration, SpellInfo* spell)
{
    uint32_t status = sSpellCustomizations.getDiminishingGroup(spell->getId());
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

void Unit::removeDiminishingReturnTimer(SpellInfo* spell)
{
    uint32_t status = sSpellCustomizations.getDiminishingGroup(spell->getId());
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
            aura_group = sSpellCustomizations.getDiminishingGroup(m_auras[x]->GetSpellInfo()->getId());
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

//////////////////////////////////////////////////////////////////////////////////////////
// Aura

Aura* Unit::getAuraWithId(uint32_t spell_id)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr)
        {
            if (aura->GetSpellId() == spell_id)
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
            if (m_auras[x] && m_auras[x]->GetSpellInfo()->getId() == auraId[i])
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
        if (m_auras[i]->GetSpellInfo()->hasEffectApplyAuraName(type))
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
            if (!caster->m_auras[i]->GetSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
                continue;
            if (caster->m_auras[i]->GetSpellInfo()->isAffectingSpell(spellInfo))
                return true;
        }
    }
    return getAuraState() & (1 << (state - 1));
}

void Unit::addAuraStateAndAuras(AuraState state)
{
    if (!(getAuraState() & (1 << (state - 1))))
    {
        addAuraState(uint32_t(1 << (state - 1)));
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
                SpellInfo const* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
                if (spellInfo == nullptr || !spellInfo->isPassive())
                    continue;
                if (spellInfo->getCasterAuraState() == uint32_t(state))
                    CastSpell(this, spellId, true);
            }
        }
    }
}

void Unit::removeAuraStateAndAuras(AuraState state)
{
    if (getAuraState() & (1 << (state - 1)))
    {
        removeAuraState(uint32_t(1 << (state - 1)));
        // Remove self-applied passive auras requiring this aurastate
        // Skip removing enrage effects
        for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        {
            if (m_auras[i] == nullptr)
                continue;
            if (m_auras[i]->GetCasterGUID() != getGuid())
                continue;
            if (m_auras[i]->GetSpellInfo()->getCasterAuraState() != uint32_t(state))
                continue;
            if (m_auras[i]->GetSpellInfo()->isPassive() || state != AURASTATE_FLAG_ENRAGED)
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
            if (getAuraWithId(spell_id) && aura->m_casterGuid == target_guid)
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
        if (aura != nullptr && aura->GetSpellInfo()->hasEffectApplyAuraName(aura_effect))
            return aura;
    }

    return nullptr;
}

bool Unit::hasAurasWithId(uint32_t auraId)
{
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] && m_auras[x]->GetSpellInfo()->getId() == auraId)
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
            if (m_auras[x] && m_auras[x]->GetSpellInfo()->getId() == auraId[i])
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
        if (m_auras[x] && (m_auras[x]->GetSpellInfo()->getId() == auraId))
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
            if (aura != nullptr && aura->GetSpellInfo()->getId() == auraId[i] && aura->m_casterGuid == guid)
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
            if (m_auras[x]->GetSpellInfo()->getId() == auraId)
            {
                m_auras[x]->Remove();
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
                if (m_auras[x]->GetSpellInfo()->getId() == auraId[i])
                {
                    m_auras[x]->Remove();
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
            if (m_auras[x]->GetSpellId() == spellId)
            {
                if (!guid || m_auras[x]->m_casterGuid == guid)
                {
                    m_auras[x]->Remove();
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
            if (m_auras[x]->GetSpellInfo()->getId() == auraId)
            {
                m_auras[x]->Remove();
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
        if (m_auras[i]->GetSpellInfo()->hasEffectApplyAuraName(effect))
            RemoveAura(m_auras[i]);
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

    // Unit cannot see invisible Game Masters unless he/she has Game Master flag on
    if (obj->isPlayer() && static_cast<Player*>(obj)->m_isGmInvisible)
        return isPlayer() && HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);

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
        const auto corpseViewDistance = 1600.f; // 40*40 yards
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
                playerObj->getDistanceSq(playerMe->getMyCorpseLocation()) <= corpseViewDistance)
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
            if (obj->getDistanceSq(playerMe->getMyCorpseLocation()) <= corpseViewDistance)
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
                    return HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
                }
                else
                    // Non-player units cannot see dead players
                    return false;
            }
            break;
        }
        case TYPEID_UNIT:
        {
            // Unit cannot see Spirit Healers when unit's alive
            // unless unit is a Game Master
            if (obj->isCreature() && static_cast<Creature*>(obj)->isSpiritHealer())
                return isPlayer() && HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);

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
                    return static_cast<Player*>(this)->IsTeamHorde() ? true : false;
                if (unitObj->GetAIInterface()->faction_visibility == 2)
                    return static_cast<Player*>(this)->IsTeamHorde() ? false : true;
            }
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            const auto gameObjectObj = static_cast<GameObject*>(obj);
            // Stealthed / invisible gameobjects
            if (gameObjectObj->inStealth || gameObjectObj->invisible)
            {
                const auto ownerGuid = gameObjectObj->getCreatedByGuid();
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
            break;
        }
        default:
            break;
    }

    // Game Masters can see invisible and stealthed objects
    if (isPlayer() && HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
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

    for (auto i = 0; i < INVIS_FLAG_TOTAL; ++i)
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

        auto visibilityRange = float_t(detectionValue * 0.3f + combatReach);
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
// Misc
void Unit::setAttackTimer(WeaponDamageType type, int32_t time)
{
    // TODO: getModCastSpeed() is no longer used here, is it required?
    // it was used in the old function but isnt it about spell casttime only.. - Appled
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
