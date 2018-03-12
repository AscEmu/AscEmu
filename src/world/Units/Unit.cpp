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

void Unit::setHealth(uint32_t health) { write(unitData()->health, health); }
void Unit::setMaxHealth(uint32_t maxHealth) { write(unitData()->max_health, maxHealth); }

void Unit::setBaseHealth(uint32_t baseHealth) { write(unitData()->base_health, baseHealth); }

void Unit::setBaseMana(uint32_t baseMana) { write(unitData()->base_mana, baseMana); }
void Unit::setMaxMana(uint32_t maxMana) { write(unitData()->max_mana, maxMana); }

uint32_t Unit::getLevel() const { return unitData()->level; }
void Unit::setLevel(uint32_t level)
{
    write(unitData()->level, level);
    if (IsPlayer())
        static_cast<Player*>(this)->setNextLevelXp(sMySQLStore.getPlayerXPForLevel(level));
}

uint32_t Unit::getVirtualItemSlotId(uint8_t slot) const { return unitData()->virtual_item_slot_display[slot]; }
void Unit::setVirtualItemSlotId(uint8_t slot, uint32_t item_id) { write(unitData()->virtual_item_slot_display[slot], item_id); }

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

float_t Unit::getBoundingRadius() const { return unitData()->bounding_radius; }
void Unit::setBoundingRadius(float_t radius) { write(unitData()->bounding_radius, radius); }

float_t Unit::getCombatReach() const { return unitData()->combat_reach; }
void Unit::setCombatReach(float_t radius) { write(unitData()->combat_reach, radius); }

uint32_t Unit::getDisplayId() const { return unitData()->display_id; }
void Unit::setDisplayId(uint32_t id) { write(unitData()->display_id, id); }

uint32_t Unit::getNativeDisplayId() const { return unitData()->native_display_id; }
void Unit::setNativeDisplayId(uint32_t id) { write(unitData()->native_display_id, id); }

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

uint32_t Unit::getNpcFlags() const { return unitData()->npc_flags; }
void Unit::setNpcFlags(uint32_t npcFlags) { write(unitData()->npc_flags, npcFlags); }
void Unit::addNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() | npcFlags); }
void Unit::removeNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() & ~npcFlags); }

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

    if (IsPlayer())
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

    if (IsCreature())
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

    if (IsPlayer())
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

    if (IsCreature())
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

    if (IsPlayer())
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

    if (IsCreature())
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

    if (IsPlayer())
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

    if (IsCreature())
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
    if (IsPlayer())
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
    if (IsCreature())
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
    if (IsPlayer())
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

    if (IsCreature())
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
    if (IsPlayer())
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

    if (IsCreature())
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
    if (IsCreature())
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
    if (IsPlayer())
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

    if (IsCreature())
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
    if (IsCreature())
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
        const auto speed_type = static_cast<UnitSpeedType>(i + 1);

        switch(speed_type)
        {
        case TYPE_TURN_RATE:
        case TYPE_PITCH_RATE:
            continue;
            default:
            break;
        }

        const auto speed = getSpeedForType(speed_type);

        fastest_speed = speed > fastest_speed ? speed : fastest_speed;
        fastest_speed_type = speed == fastest_speed ? speed_type : fastest_speed_type;
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
        if (IsPlayer())
            player_mover = static_cast<Player*>(this);
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

void Unit::sendMoveSplinePaket(UnitSpeedType speed_type)
{
    WorldPacket data(12);

    switch (speed_type)
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
    data << float(getSpeedForType(speed_type));

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

    if (IsPlayer())
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
    if (!PvE && !IsPlayer() && !IsPet())
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
    if ((IsPlayer() || IsPet()) && localDuration > uint32_t(10000 >> count))
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
    if (!pve && !IsPlayer() && !IsPet())
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

Aura* Unit::getAuraWithAuraEffect(uint32_t aura_effect)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr && aura->GetSpellInfo()->HasEffectApplyAuraName(aura_effect))
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
