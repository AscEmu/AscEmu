/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Unit.hpp"

#include "Creatures/Corpse.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Management/HonorHandler.h"
#include "Management/Loot/LootMgr.hpp"
#include "Movement/Spline/MovementPacketBuilder.h"
#include "Objects/GameObject.h"
#include "Server/Packets/SmsgAuraUpdate.h"
#include "Server/Packets/SmsgAuraUpdateAll.h"
#include "Server/Packets/SmsgClearExtraAuraInfo.h"
#include "Server/Packets/SmsgEmote.h"
#include "Server/Packets/SmsgSpellEnergizeLog.h"
#include "Server/Packets/SmsgEnvironmentalDamageLog.h"
#include "Server/Packets/SmsgPeriodicAuraLog.h"
#include "Server/Packets/SmsgPlaySpellVisual.h"
#include "Server/Packets/SmsgPowerUpdate.h"
#include "Server/Packets/SmsgSpellHealLog.h"
#include "Server/Packets/SmsgSpellOrDamageImmune.h"
#include "Server/Packets/SmsgStandStateUpdate.h"
#include "Server/Packets/SmsgControlVehicle.h"
#include "Server/Packets/SmsgPlayerVehicleData.h"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"
#include "Spell/Definitions/AuraInterruptFlags.hpp"
#include "Spell/Definitions/DiminishingGroup.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellDamageType.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/Definitions/SpellIsFlags.hpp"
#include "Spell/Definitions/SpellMechanics.hpp"
#include "Spell/Definitions/SpellTypes.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellTarget.h"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Server/Packets/SmsgAttackStart.h"
#include "Server/Packets/SmsgAttackStop.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Creatures/CreatureGroups.h"
#include "Management/AchievementMgr.h"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Movement/AbstractFollower.h"
#include "Movement/MovementManager.h"
#include "Objects/DynamicObject.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Item.hpp"
#include "Objects/ItemDefines.hpp"
#include "Server/Packets/SmsgAttackSwingBadFacing.h"
#include "Server/Packets/SmsgSpellDamageShield.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/Definitions/SpellLog.hpp"
#include "Spell/Definitions/SpellSchoolConversionTable.hpp"
#include "Objects/Transporter.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/Script/HookInterface.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

#if VERSION_STRING <= TBC
#include "Server/Packets/SmsgUpdateAuraDuration.h"
#include "Server/Packets/SmsgSetExtraAuraInfo.h"
#endif

using namespace AscEmu::Packets;

Unit::Unit() :
    movespline(std::make_unique<MovementMgr::MoveSpline>()),
    i_movementManager(std::make_unique<MovementManager>(this)),
    m_summonInterface(std::make_unique<SummonHandler>(this)),
    m_combatHandler(this),
    m_aiInterface(std::make_unique<AIInterface>()),
#ifdef FT_VEHICLES
    m_vehicleKit(nullptr),
#endif
    m_damageSplitTarget(nullptr)
{
    m_objectType |= TYPE_UNIT;

#if VERSION_STRING < Cata
    m_updateFlag = (UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION);
#else
    m_updateFlag = UPDATEFLAG_LIVING;
#endif

    m_lastAiInterfaceUpdateTime = Util::getMSTime();

    std::fill(m_auraList.begin(), m_auraList.end(), nullptr);

    m_aiInterface->Init(this);
    getThreatManager().initialize();
}

Unit::~Unit()
{
    removeAllAuras();

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (getCurrentSpell(static_cast<CurrentSpellType>(i)) != nullptr)
            interruptSpellWithSpellType(static_cast<CurrentSpellType>(i));
    }

    for (uint8_t i = 0; i < MAX_SPELLMOD_TYPE; ++i)
        m_spellModifiers[i].clear();

    for (auto extraStrikeTarget = m_extraStrikeTargets.begin(); extraStrikeTarget != m_extraStrikeTargets.end(); ++extraStrikeTarget)
    {
        const auto& extraStrike = *extraStrikeTarget;
        sLogger.failure("ExtraStrike added to Unit {} by Spell ID {} wasn't removed when removing the Aura", getGuid(), extraStrike->spell_info->getId());
    }
    m_extraStrikeTargets.clear();

    // delete auras which did not get added to unit yet
    for (auto tempAura = m_tempAuraMap.begin(); tempAura != m_tempAuraMap.end(); ++tempAura)
        delete tempAura->second;

    m_tempAuraMap.clear();

    m_procSpells.clear();

    m_singleTargetAura.clear();

    m_summonInterface->removeAllSummons();

    clearHealthBatch();

    removeGarbage();

    getThreatManager().clearAllThreat();
    getThreatManager().removeMeFromThreatLists();
}

void Unit::Update(unsigned long time_passed)
{
    const auto msTime = Util::getMSTime();

    auto diff = msTime - m_lastSpellUpdateTime;
    if (m_lastSpellUpdateTime == 0)
    {
        m_lastSpellUpdateTime = msTime;
    }
    else if (diff >= 100)
    {
        // Spells and auras are updated every 100ms
        _UpdateSpells(diff);
        _updateAuras(diff);

        // Update spell school lockout timer
        // TODO: Moved here from Spell::CanCast, figure out a better way to handle this... -Appled
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            if (m_schoolCastPrevent[i] == 0)
                continue;

            if (msTime >= m_schoolCastPrevent[i])
                m_schoolCastPrevent[i] = 0;
        }

        removeGarbage();

        m_lastSpellUpdateTime = msTime;
    }

    if (isAlive())
    {
        // Update health batch
        if (time_passed >= m_healthBatchTime)
        {
            _updateHealth();
            m_healthBatchTime = HEALTH_BATCH_INTERVAL;
        }
        else
        {
            m_healthBatchTime -= static_cast<uint16_t>(time_passed);
        }

        // POWER & HP REGENERATION
        regenerateHealthAndPowers(static_cast<uint16_t>(time_passed));

        if (m_healthRegenerationInterruptTime > 0)
        {
            if (time_passed >= m_healthRegenerationInterruptTime)
                m_healthRegenerationInterruptTime = 0;
            else
                m_healthRegenerationInterruptTime -= time_passed;
        }

#if VERSION_STRING < Cata
        if (m_powerRegenerationInterruptTime > 0)
        {
            if (time_passed >= m_powerRegenerationInterruptTime)
            {
                m_powerRegenerationInterruptTime = 0;

#if VERSION_STRING > TBC
                if (isPlayer())
                    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif
            }
            else
            {
                m_powerRegenerationInterruptTime -= time_passed;
            }
        }
#endif

        if (m_aiInterface != nullptr)
        {
            diff = msTime - m_lastAiInterfaceUpdateTime;
            if (m_lastAiInterfaceUpdateTime == 0)
            {
                m_lastAiInterfaceUpdateTime = msTime;
            }
            else if (diff >= 100)
            {
                m_aiInterface->update(diff);
                m_lastAiInterfaceUpdateTime = msTime;
            }
        }
        getThreatManager().update(time_passed);
        getCombatHandler().updateCombat(msTime);

        if (m_diminishActive)
        {
            uint32_t count = 0;
            for (uint32_t x = 0; x < DIMINISHING_GROUP_COUNT; ++x)
            {
                // diminishing return stuff
                if (m_diminishTimer[x] && !m_diminishAuraCount[x])
                {
                    if (time_passed >= m_diminishTimer[x])
                    {
                        // resetting after 15 sec
                        m_diminishTimer[x] = 0;
                        m_diminishCount[x] = 0;
                    }
                    else
                    {
                        // reducing, still.
                        m_diminishTimer[x] -= static_cast<uint16_t>(time_passed);
                        ++count;
                    }
                }
            }
            if (!count)
                m_diminishActive = false;
        }
    }
    else
    {
        // Small chance that aura states are readded after they have been cleared in ::Die
        // so make sure they are removed when unit is dead
        if (getAuraState() != 0)
            setAuraState(0);

        if (m_aiInterface != nullptr)
            m_lastAiInterfaceUpdateTime = msTime;
    }

    updateSplineMovement(time_passed);
    getMovementManager()->update(time_passed);
}

void Unit::RemoveFromWorld(bool free_guid)
{
#ifdef FT_VEHICLES
    if (isVehicle())
        removeVehicleKit();
#endif
    removeAllFollowers();

    getCombatHandler().onRemoveFromWorld();

#if VERSION_STRING > TBC
    if (getCritterGuid() != 0)
    {
        setCritterGuid(0);

        if (Unit* unit = m_WorldMap->getUnit(getCritterGuid()))
            unit->Delete();
    }
#endif

    m_summonInterface->removeAllSummons();

    if (m_dynamicObject != nullptr)
        m_dynamicObject->remove();

    for (unsigned int& m_ObjectSlot : m_objectSlots)
    {
        if (m_ObjectSlot != 0)
        {
            if (GameObject* game_object = m_WorldMap->getGameObject(m_ObjectSlot))
                game_object->expireAndDelete();

            m_ObjectSlot = 0;
        }
    }

    clearAllAreaAuraTargets();
    removeAllAreaAurasCastedByOther();

    // Attempt to prevent memory corruption
    for (const auto& object : getInRangeObjectsSet())
    {
        if (!object->isCreatureOrPlayer())
            continue;

        dynamic_cast<Unit*>(object)->clearCasterFromHealthBatch(this);
    }

    Object::RemoveFromWorld(free_guid);

    for (uint16_t x = AuraSlots::TOTAL_SLOT_START; x < AuraSlots::TOTAL_SLOT_END; ++x)
    {
        if (auto* const aura = getAuraWithAuraSlot(x))
        {
            if (aura->m_deleted)
            {
                m_auraList[x] = nullptr;
                continue;
            }
            aura->RelocateEvents();
        }
    }
    getThreatManager().removeMeFromThreatLists();
}

void Unit::OnPushToWorld()
{
    for (const auto& aura : getAuraList())
    {
        if (aura != nullptr)
            aura->RelocateEvents();
    }

#if VERSION_STRING >= WotLK
    if (isVehicle() && getVehicleKit())
    {
        getVehicleKit()->initialize();
        getVehicleKit()->loadAllAccessories(false);
    }

    m_zAxisPosition = 0.0f;
#endif

    getMovementManager()->addToWorld();
}

void Unit::die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/)
{}

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
            sLogger.failure("Offset {} is not a valid offset value for byte_0 data (max 3). Returning 0", offset);
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
            sLogger.failure("Offset {} is not a valid offset value for byte_0 data (max 3)", offset);
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
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
        return;

    plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_POWER_TYPE : GROUP_UPDATE_FLAG_PET_POWER_TYPE);
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
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
        return;

    plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_CUR_HP : GROUP_UPDATE_FLAG_PET_CUR_HP);
}
void Unit::modHealth(int32_t health)
{
    int32_t newHealth = getHealth();
    newHealth += health;

    if (newHealth < 0)
        newHealth = 0;

    setHealth(newHealth);
}

void Unit::setFullHealth() { setHealth(getMaxHealth()); }
void Unit::setHealthPct(uint32_t val) { if (val > 0) setHealth(Util::float2int32(val * 0.01f * getMaxHealth())); }

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

void Unit::setPower(PowerType type, uint32_t value, bool sendPacket/* = true*/, bool skipObjectUpdate/* = false*/)
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
            write(unitData()->power_1, value, skipObjectUpdate);
            break;
        case POWER_FIELD_INDEX_2:
            write(unitData()->power_2, value, skipObjectUpdate);
            break;
        case POWER_FIELD_INDEX_3:
            write(unitData()->power_3, value, skipObjectUpdate);
            break;
        case POWER_FIELD_INDEX_4:
            write(unitData()->power_4, value, skipObjectUpdate);
            break;
        case POWER_FIELD_INDEX_5:
            write(unitData()->power_5, value, skipObjectUpdate);
            break;
#if VERSION_STRING == WotLK
        case POWER_FIELD_INDEX_6:
            write(unitData()->power_6, value, skipObjectUpdate);
            break;
        case POWER_FIELD_INDEX_7:
            write(unitData()->power_7, value, skipObjectUpdate);
            break;
#endif
        default:
            return;
    }

    if (skipObjectUpdate)
        return;

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Send power update to client
    if (sendPacket)
        sendPowerUpdate(isPlayer());

    // Update power also to group
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
        return;

    plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_CUR_POWER : GROUP_UPDATE_FLAG_PET_CUR_POWER);
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
    const auto plr = getPlayerOwnerOrSelf();
    if (plr != nullptr && plr->IsInWorld() && plr->getGroup() != nullptr)
        plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_MAX_HP : GROUP_UPDATE_FLAG_PET_MAX_HP);

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
    const auto plr = getPlayerOwnerOrSelf();
    if (plr != nullptr && plr->IsInWorld() && plr->getGroup() != nullptr)
        plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_MAX_POWER : GROUP_UPDATE_FLAG_PET_MAX_POWER);

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

float Unit::getPowerRegeneration(PowerType type) const
{
#if VERSION_STRING < WotLK
    switch (type)
    {
        case POWER_TYPE_MANA:
#if VERSION_STRING == TBC
            if (isPlayer())
                return dynamic_cast<Player const*>(this)->getManaRegeneration();
            else
#endif
                return m_manaRegeneration;
        case POWER_TYPE_RAGE:
            return m_rageRegeneration;
        case POWER_TYPE_FOCUS:
            return m_focusRegeneration;
        case POWER_TYPE_ENERGY:
            return m_energyRegeneration;
        default:
            return 0.0f;
    }
#else
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
#endif
}

void Unit::setPowerRegeneration(PowerType type, float value)
{
#if VERSION_STRING < WotLK
    switch (type)
    {
        case POWER_TYPE_MANA:
#if VERSION_STRING == TBC
            if (isPlayer())
                dynamic_cast<Player*>(this)->setManaRegeneration(value);
            else
#endif
                m_manaRegeneration = value;
            break;
        case POWER_TYPE_RAGE:
            m_rageRegeneration = value;
            break;
        case POWER_TYPE_FOCUS:
            m_focusRegeneration = value;
            break;
        case POWER_TYPE_ENERGY:
            m_energyRegeneration = value;
            break;
        default:
            break;
    }
#else
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
#endif
}

float Unit::getPowerRegenerationWhileInterrupted(PowerType type) const
{
#if VERSION_STRING < WotLK
    switch (type)
    {
        case POWER_TYPE_MANA:
#if VERSION_STRING == TBC
            if (isPlayer())
                return dynamic_cast<Player const*>(this)->getManaRegenerationWhileCasting();
            else
#endif
                return m_manaRegenerationWhileCasting;
        case POWER_TYPE_RAGE:
            return m_rageRegenerationWhileCombat;
        case POWER_TYPE_FOCUS:
            return m_focusRegeneration;
        case POWER_TYPE_ENERGY:
            return m_energyRegeneration;
        default:
            return 0.0f;
    }
#else
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
#endif
}

void Unit::setPowerRegenerationWhileInterrupted(PowerType type, float value)
{
#if VERSION_STRING < WotLK
    switch (type)
    {
        case POWER_TYPE_MANA:
#if VERSION_STRING == TBC
            if (isPlayer())
                dynamic_cast<Player*>(this)->setManaRegenerationWhileCasting(value);
            else
#endif
                m_manaRegenerationWhileCasting = value;
            break;
        case POWER_TYPE_RAGE:
            m_rageRegenerationWhileCombat = value;
            break;
        case POWER_TYPE_FOCUS:
            m_focusRegeneration = value;
            break;
        case POWER_TYPE_ENERGY:
            m_energyRegeneration = value;
            break;
        default:
            break;
    }
#else
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
#endif
}

uint32_t Unit::getLevel() const { return unitData()->level; }
void Unit::setLevel(uint32_t level)
{
    write(unitData()->level, level);
    if (isPlayer())
        dynamic_cast<Player*>(this)->setNextLevelXp(sMySQLStore.getPlayerXPForLevel(level));

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update level also to group
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
        return;

    //\ todo: missing update flag for pet level
    plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_LEVEL : 0);
}

uint32_t Unit::getFactionTemplate() const { return unitData()->faction_template; }
void Unit::setFactionTemplate(uint32_t id) { write(unitData()->faction_template, id); }

#if VERSION_STRING >= WotLK
uint32_t Unit::getVirtualItemSlotId(uint8_t slot) const { return unitData()->virtual_item_slot_display[slot]; }
#else
uint32_t Unit::getVirtualItemDisplayId(uint8_t slot) const { return unitData()->virtual_item_slot_display[slot]; }
#endif
void Unit::setVirtualItemSlotId(uint8_t slot, uint32_t item_id)
{
    const auto isProperOffhandWeapon = [](uint32_t itemClass, uint32_t itemSubClass) -> bool
    {
        if (itemClass != ITEM_CLASS_WEAPON)
            return false;

        switch (itemSubClass)
        {
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_GUN:
            case ITEM_SUBCLASS_WEAPON_THROWN:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            case ITEM_SUBCLASS_WEAPON_WAND:
                return false;
            default:
                break;
        }

        return true;
    };

    if (item_id == 0)
    {
        write(unitData()->virtual_item_slot_display[slot], 0U);
#if VERSION_STRING < WotLK
        setVirtualItemInfo(slot, 0);
#endif

        if (isCreature())
        {
#if VERSION_STRING < WotLK
            dynamic_cast<Creature*>(this)->setVirtualItemEntry(slot, 0);
#endif
            if (slot == OFFHAND)
                dynamic_cast<Creature*>(this)->toggleDualwield(false);
        }
        return;
    }

#if VERSION_STRING >= WotLK
    const auto itemDbc = sItemStore.lookupEntry(item_id);
    if (itemDbc == nullptr
        || !(itemDbc->Class == ITEM_CLASS_WEAPON
        || (itemDbc->Class == ITEM_CLASS_ARMOR && itemDbc->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        || itemDbc->InventoryType == INVTYPE_HOLDABLE))
        return;

    if (isCreature() && slot == OFFHAND)
        dynamic_cast<Creature*>(this)->toggleDualwield(isProperOffhandWeapon(itemDbc->Class, itemDbc->SubClass));

    write(unitData()->virtual_item_slot_display[slot], item_id);
#else
    unit_virtual_item_info virtualItemInfo{};

    uint32_t displayId = 0;
    virtualItemInfo.fields.item_class = 0;
    virtualItemInfo.fields.item_subclass = 0;
    // Seems to be always -1
    virtualItemInfo.fields.unk0 = -1;
    virtualItemInfo.fields.material = 0;
    virtualItemInfo.fields.inventory_type = 0;
    virtualItemInfo.fields.sheath = 0;
    if (const auto itemProperties = sMySQLStore.getItemProperties(item_id))
    {
        displayId = itemProperties->DisplayInfoID;
        virtualItemInfo.fields.item_class = static_cast<uint8_t>(itemProperties->Class);
        virtualItemInfo.fields.item_subclass = static_cast<uint8_t>(itemProperties->SubClass);
        virtualItemInfo.fields.material = static_cast<uint8_t>(itemProperties->LockMaterial);
        virtualItemInfo.fields.inventory_type = static_cast<uint8_t>(itemProperties->InventoryType);
        virtualItemInfo.fields.sheath = static_cast<uint8_t>(itemProperties->SheathID);
    }
    else if (const auto itemDbc = sItemStore.lookupEntry(item_id))
    {
        displayId = itemDbc->DisplayId;
        virtualItemInfo.fields.inventory_type = static_cast<uint8_t>(itemDbc->InventoryType);
        virtualItemInfo.fields.sheath = static_cast<uint8_t>(itemDbc->Sheath);

        // Following values do not exist in dbcs and must be "hackfixed"
        virtualItemInfo.fields.material = ITEM_MATERIAL_METAL;
        switch (virtualItemInfo.fields.inventory_type)
        {
            case INVTYPE_WEAPON:
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_WEAPONOFFHAND:
                virtualItemInfo.fields.item_class = ITEM_CLASS_WEAPON;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_SWORD;
                break;
            case INVTYPE_SHIELD:
                virtualItemInfo.fields.item_class = ITEM_CLASS_ARMOR;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_ARMOR_SHIELD;
                break;
            case INVTYPE_RANGED:
                virtualItemInfo.fields.item_class = ITEM_CLASS_WEAPON;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_BOW;
                break;
            case INVTYPE_RANGEDRIGHT:
                virtualItemInfo.fields.item_class = ITEM_CLASS_WEAPON;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_GUN;
                break;
            case INVTYPE_2HWEAPON:
                virtualItemInfo.fields.item_class = ITEM_CLASS_WEAPON;
                if (virtualItemInfo.fields.sheath == ITEM_SHEATH_STAFF)
                    virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_STAFF;
                else
                    virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD;
                break;
            case INVTYPE_HOLDABLE:
                virtualItemInfo.fields.item_class = ITEM_CLASS_MISCELLANEOUS;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_MISC_JUNK;
                break;
            case INVTYPE_THROWN:
                virtualItemInfo.fields.item_class = ITEM_CLASS_WEAPON;
                virtualItemInfo.fields.item_subclass = ITEM_SUBCLASS_WEAPON_THROWN;
                break;
            default:
                return;
        }
    }
    else
    {
        return;
    }

    if (isCreature())
    {
        dynamic_cast<Creature*>(this)->setVirtualItemEntry(slot, item_id);
        if (slot == OFFHAND)
            dynamic_cast<Creature*>(this)->toggleDualwield(isProperOffhandWeapon(virtualItemInfo.fields.item_class, virtualItemInfo.fields.item_subclass));
    }

    write(unitData()->virtual_item_slot_display[slot], displayId);
    setVirtualItemInfo(slot, virtualItemInfo.raw);
#endif
}

#if VERSION_STRING < WotLK
uint64_t Unit::getVirtualItemInfo(uint8_t slot) const { return unitData()->virtual_item_info[slot].raw; }
unit_virtual_item_info Unit::getVirtualItemInfoFields(uint8_t slot) const { return unitData()->virtual_item_info[slot]; }
void Unit::setVirtualItemInfo(uint8_t slot, uint64_t item_info) { write(unitData()->virtual_item_info[slot].raw, item_info); }
#endif

uint32_t Unit::getUnitFlags() const { return unitData()->unit_flags; }
void Unit::setUnitFlags(uint32_t unitFlags) { write(unitData()->unit_flags, unitFlags); }
void Unit::addUnitFlags(uint32_t unitFlags) { setUnitFlags(getUnitFlags() | unitFlags); }
void Unit::removeUnitFlags(uint32_t unitFlags) { setUnitFlags(getUnitFlags() & ~unitFlags); }
bool Unit::hasUnitFlags(uint32_t unitFlags) const { return (getUnitFlags() & unitFlags) != 0; }

//helpers
bool Unit::canSwim()
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
#if VERSION_STRING == Classic
    return hasUnitFlags(UNIT_FLAG_SWIMMING);
#else
    return hasUnitFlags(UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_SWIMMING);
#endif
}

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
    if (slot >= AuraSlots::NEGATIVE_VISUAL_SLOT_END)
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
    if (slot >= AuraSlots::NEGATIVE_VISUAL_SLOT_END)
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
    if (slot >= AuraSlots::NEGATIVE_VISUAL_SLOT_END)
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
    if (slot >= AuraSlots::NEGATIVE_VISUAL_SLOT_END)
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
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || plr->getGroup() == nullptr)
        return;

    //\ todo: missing update flag for player display id
    plr->addGroupUpdateFlag(isPlayer() ? 0 : GROUP_UPDATE_FLAG_PET_MODEL_ID);
}
void Unit::resetDisplayId()
{
    write(unitData()->display_id, unitData()->native_display_id);
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
#if VERSION_STRING < WotLK
            return getPetLoyalty();
#elif VERSION_STRING < Mop
            return getPetTalentPoints();
#else
            return unitData()->field_bytes_1.s.unk1;
#endif
        case 2:
#if VERSION_STRING == Classic
            return getShapeShiftForm();
#else
            return getStandStateFlags();
#endif
        case 3:
#if VERSION_STRING == Classic
            return getStandStateFlags();
#else
            return getAnimationFlags();
#endif
        default:
            sLogger.failure("Offset {} is not a valid offset value for byte_1 data (max 3). Returning 0", offset);
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
#if VERSION_STRING < WotLK
            setPetLoyalty(value);
#elif VERSION_STRING < Mop
            setPetTalentPoints(value);
#else
            write(unitData()->field_bytes_1.s.unk1, value);
#endif
            break;
        case 2:
#if VERSION_STRING == Classic
            setShapeShiftForm(value);
#else
            setStandStateFlags(value);
#endif
            break;
        case 3:
#if VERSION_STRING == Classic
            setStandStateFlags(value);
#else
            setAnimationFlags(value);
#endif
            break;
        default:
            sLogger.failure("Offset {} is not a valid offset value for byte_1 data (max 3)", offset);
            break;
    }
}

uint8_t Unit::getStandState() const { return unitData()->field_bytes_1.s.stand_state; }
void Unit::setStandState(uint8_t standState)
{
    write(unitData()->field_bytes_1.s.stand_state, standState);

    if (isPlayer())
        dynamic_cast<Player*>(this)->sendPacket(SmsgStandStateUpdate(standState).serialise().get());

    if (standState != STANDSTATE_SIT)
        removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_STAND_UP);
}

#if VERSION_STRING < WotLK
uint8_t Unit::getPetLoyalty() const { return unitData()->field_bytes_1.s.pet_loyalty; }
void Unit::setPetLoyalty(uint8_t loyalty) { write(unitData()->field_bytes_1.s.pet_loyalty, loyalty); }
#elif VERSION_STRING < Mop
uint8_t Unit::getPetTalentPoints() const { return unitData()->field_bytes_1.s.pet_talent_points; }
void Unit::setPetTalentPoints(uint8_t talentPoints) { write(unitData()->field_bytes_1.s.pet_talent_points, talentPoints); }
#endif

#if VERSION_STRING == Classic
uint8_t Unit::getShapeShiftForm() const { return unitData()->field_bytes_1.s.shape_shift_form; }
void Unit::setShapeShiftForm(uint8_t shapeShiftForm) { write(unitData()->field_bytes_1.s.shape_shift_form, shapeShiftForm); }
#endif

uint8_t Unit::getStandStateFlags() const { return unitData()->field_bytes_1.s.stand_state_flag; }
void Unit::setStandStateFlags(uint8_t standStateFlags) { write(unitData()->field_bytes_1.s.stand_state_flag, standStateFlags); }
void Unit::addStandStateFlags(uint8_t standStateFlags) { setStandStateFlags(getStandStateFlags() | standStateFlags); }
void Unit::removeStandStateFlags(uint8_t standStateFlags) { setStandStateFlags(getStandStateFlags() & ~standStateFlags); }

#if VERSION_STRING != Classic
uint8_t Unit::getAnimationFlags() const { return unitData()->field_bytes_1.s.animation_flag; }
void Unit::setAnimationFlags(uint8_t animationFlags) { write(unitData()->field_bytes_1.s.animation_flag, animationFlags); }
#endif
//bytes_1 end

uint32_t Unit::getPetNumber() const { return unitData()->pet_number; }
void Unit::setPetNumber(uint32_t number) { write(unitData()->pet_number, number); }

uint32_t Unit::getPetNameTimestamp() const { return unitData()->pet_name_timestamp; }
void Unit::setPetNameTimestamp(uint32_t timestamp) { write(unitData()->pet_name_timestamp, timestamp); }

uint32_t Unit::getPetExperience() const { return unitData()->pet_experience; }
void Unit::setPetExperience(uint32_t experience) { write(unitData()->pet_experience, experience); }

uint32_t Unit::getPetNextLevelExperience() const { return unitData()->pet_next_level_experience; }
void Unit::setPetNextLevelExperience(uint32_t experience) { write(unitData()->pet_next_level_experience, experience); }

#if VERSION_STRING < Mop
uint32_t Unit::getDynamicFlags() const { return unitData()->dynamic_flags; }
void Unit::setDynamicFlags(uint32_t dynamicFlags) { write(unitData()->dynamic_flags, dynamicFlags); }
void Unit::addDynamicFlags(uint32_t dynamicFlags) { setDynamicFlags(getDynamicFlags() | dynamicFlags); }
void Unit::removeDynamicFlags(uint32_t dynamicFlags) { setDynamicFlags(getDynamicFlags() & ~dynamicFlags); }
bool Unit::hasDynamicFlags(uint32_t dynamicFlags) const { return (getDynamicFlags() & dynamicFlags) != 0; }
#endif

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

#if VERSION_STRING < Mop
uint32_t Unit::getNpcFlags() const { return unitData()->npc_flags; }
void Unit::setNpcFlags(uint32_t npcFlags) { write(unitData()->npc_flags, npcFlags); }
void Unit::addNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() | npcFlags); }
void Unit::removeNpcFlags(uint32_t npcFlags) { setNpcFlags(getNpcFlags() & ~npcFlags); }
#else
uint64_t Unit::getNpcFlags() const { return unitData()->npc_flags; }
void Unit::setNpcFlags(uint64_t npcFlags) { write(unitData()->npc_flags, npcFlags); }
void Unit::addNpcFlags(uint64_t npcFlags) { setNpcFlags(getNpcFlags() | npcFlags); }
void Unit::removeNpcFlags(uint64_t npcFlags) { setNpcFlags(getNpcFlags() & ~npcFlags); }
#endif

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
#if VERSION_STRING == Classic
            return unitData()->field_bytes_2.s.unk1;
#elif VERSION_STRING == TBC
            return getPositiveAuraLimit();
#else
            return getPvpFlags();
#endif
        case 2:
#if VERSION_STRING == Classic
            return unitData()->field_bytes_2.s.unk2;
#else
            return getPetFlags();
#endif
        case 3:
#if VERSION_STRING == Classic
            return unitData()->field_bytes_2.s.unk3;
#else
            return getShapeShiftForm();
#endif
        default:
            sLogger.failure("Offset {} is not a valid offset value for byte_2 data (max 3). Returning 0", offset);
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
#if VERSION_STRING == Classic
            write(unitData()->field_bytes_2.s.unk1, value);
#elif VERSION_STRING == TBC
            setPositiveAuraLimit(value);
#else
            setPvpFlags(value);
#endif
            break;
        case 2:
#if VERSION_STRING == Classic
            write(unitData()->field_bytes_2.s.unk2, value);
#else
            setPetFlags(value);
#endif
            break;
        case 3:
#if VERSION_STRING == Classic
            write(unitData()->field_bytes_2.s.unk3, value);
#else
            setShapeShiftForm(value);
#endif
            break;
        default:
            sLogger.failure("Offset {} is not a valid offset value for byte_2 data (max 3)", offset);
            break;
    }
}

uint8_t Unit::getSheathType() const { return unitData()->field_bytes_2.s.sheath_type; }
void Unit::setSheathType(uint8_t sheathType) { write(unitData()->field_bytes_2.s.sheath_type, sheathType); }

#if VERSION_STRING == TBC
uint8_t Unit::getPositiveAuraLimit() const { return unitData()->field_bytes_2.s.positive_aura_limit; }
void Unit::setPositiveAuraLimit(uint8_t limit) { write(unitData()->field_bytes_2.s.positive_aura_limit, limit); }
#elif VERSION_STRING >= WotLK
uint8_t Unit::getPvpFlags() const { return unitData()->field_bytes_2.s.pvp_flag; }
void Unit::setPvpFlags(uint8_t pvpFlags)
{
    write(unitData()->field_bytes_2.s.pvp_flag, pvpFlags);

    // Update pvp flags also to group
    const auto plr = getPlayerOwnerOrSelf();
    if (plr == nullptr || !plr->IsInWorld() || !plr->getGroup())
        return;

    plr->addGroupUpdateFlag(isPlayer() ? GROUP_UPDATE_FLAG_STATUS : 0);
}
void Unit::addPvpFlags(uint8_t pvpFlags)
{
    auto flags = getPvpFlags();
    setPvpFlags(flags |= pvpFlags);
}
void Unit::removePvpFlags(uint8_t pvpFlags)
{
    auto flags = getPvpFlags();
    setPvpFlags(flags &= ~pvpFlags);
}
#endif

#if VERSION_STRING >= TBC
uint8_t Unit::getPetFlags() const { return unitData()->field_bytes_2.s.pet_flag; }
void Unit::setPetFlags(uint8_t petFlags) { write(unitData()->field_bytes_2.s.pet_flag, petFlags); }
void Unit::addPetFlags(uint8_t petFlags) { setPetFlags(getPetFlags() | petFlags); }
void Unit::removePetFlags(uint8_t petFlags) { setPetFlags(getPetFlags() & ~petFlags); }

uint8_t Unit::getShapeShiftForm() const { return unitData()->field_bytes_2.s.shape_shift_form; }
void Unit::setShapeShiftForm(uint8_t shapeShiftForm) { write(unitData()->field_bytes_2.s.shape_shift_form, shapeShiftForm); }
#endif
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
// Area & Position

void Unit::setLocationWithoutUpdate(LocationVector& location)
{
    m_position.ChangeCoords({ location.x, location.y, location.z });
}

void Unit::setPhase(uint8_t command/* = PHASE_SET*/, uint32_t newPhase/* = 1*/)
{
    Object::Phase(command, newPhase);

    for (const auto& itr : getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer())
            dynamic_cast<Unit*>(itr)->updateVisibility();
    }

    updateVisibility();
}

bool Unit::isWithinCombatRange(Unit* obj, float dist2compare)
{
    if (!obj || !IsInMap(obj) || !(GetPhase() == obj->GetPhase()))
        return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx * dx + dy * dy + dz * dz;

    float sizefactor = getCombatReach() + obj->getCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist* maxdist;
}

bool Unit::isWithinMeleeRangeAt(LocationVector const& pos, Unit* obj)
{
    if (!obj || !IsInMap(obj) || !(GetPhase() == obj->GetPhase()))
        return false;

    float dx = pos.getPositionX() - obj->GetPositionX();
    float dy = pos.getPositionY() - obj->GetPositionY();
    float dz = pos.getPositionZ() - obj->GetPositionZ();
    float distsq = dx * dx + dy * dy + dz * dz;

    float maxdist = getMeleeRange(obj);

    return distsq <= maxdist * maxdist;
}

float Unit::getMeleeRange(Unit* target)
{
    float range = getCombatReach() + target->getCombatReach() + 4.0f / 3.0f;
    return std::max(range, NOMINAL_MELEE_RANGE);
}

bool Unit::isInInstance() const
{
    return IsInWorld() && !getWorldMap()->getBaseMap()->isWorldMap();
}

bool Unit::isInWater() const
{
    if (worldConfig.terrainCollision.isCollisionEnabled && getWorldMap())
    {
        return getWorldMap()->getLiquidStatus(0, GetPosition(), MAP_ALL_LIQUIDS) & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER);
    }

    return false;
}

bool Unit::isUnderWater() const
{
    if (worldConfig.terrainCollision.isCollisionEnabled && getWorldMap())
    {
        return getWorldMap()->getLiquidStatus(0, GetPosition(), MAP_ALL_LIQUIDS) & LIQUID_MAP_UNDER_WATER;
    }

    return false;
}

bool Unit::isInAccessiblePlaceFor(Creature* c) const
{
    if (isInWater())
        return c->canSwim();

    if (IsFlying() && !GetTransport()) // we could be flying antihack!
        return c->canFly();

    return c->canWalk() || c->canFly();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Combat
CombatHandler& Unit::getCombatHandler()
{
    return m_combatHandler;
}

CombatHandler const& Unit::getCombatHandler() const
{
    return m_combatHandler;
}

int32_t Unit::getCalculatedAttackPower() const
{
    int32_t baseap = getAttackPower() + getAttackPowerMods();
    float totalap = baseap * (getAttackPowerMultiplier() + 1);
    if (totalap >= 0)
        return Util::float2int32(totalap);
    return 0;
}

int32_t Unit::getCalculatedRangedAttackPower() const
{
    int32_t baseap = getRangedAttackPower() + getRangedAttackPowerMods();
    float totalap = baseap * (getRangedAttackPowerMultiplier() + 1);
    if (totalap >= 0)
        return Util::float2int32(totalap);
    return 0;
}

bool Unit::canReachWithAttack(Unit* unitTarget)
{
    if (GetMapId() != unitTarget->GetMapId())
        return false;

    float selfreach = getCombatReach();
    if (isPlayer())
        selfreach = 5.0f;

    float targetradius = unitTarget->getModelHalfSize();
    float selfradius = getModelHalfSize();

    const float delta_x = unitTarget->GetPositionX() - GetPositionX();
    const float delta_y = unitTarget->GetPositionY() - GetPositionY();
    const float distance = std::sqrt(delta_x * delta_x + delta_y * delta_y);

    float attackreach = targetradius + selfreach + selfradius;

    if (isPlayer())
    {
        if (unitTarget->isPlayer() && dynamic_cast<Player*>(unitTarget)->isMoving())
        {
            uint32_t latency = dynamic_cast<Player*>(unitTarget)->getSession() ? dynamic_cast<Player*>(unitTarget)->getSession()->GetLatency() : 0;

            latency = latency > 500 ? 500 : latency;

            attackreach += getSpeedRate(TYPE_RUN, true) * 0.001f * static_cast<float>(latency);
        }

        if (dynamic_cast<Player*>(this)->isMoving())
        {
            uint32_t latency = dynamic_cast<Player*>(this)->getSession() ? dynamic_cast<Player*>(this)->getSession()->GetLatency() : 0;

            latency = latency > 500 ? 500 : latency;

            attackreach += getSpeedRate(TYPE_RUN, true) * 0.001f * static_cast<float>(latency);
        }
    }

    return (distance <= attackreach);
}

bool Unit::canBeginCombat(Unit* target)
{
    if (this == target)
        return false;

    // ...the two units need to be in the world
    if (!IsInWorld() || !target->IsInWorld())
        return false;
    // ...the two units need to both be alive
    if (!isAlive() || !target->isAlive())
        return false;
    // ...the two units need to be on the same map
    if (getWorldMap() != target->getWorldMap())
        return false;
    // ...the two units need to be in the same phase
    if (GetPhase() != target->GetPhase())
        return false;
    if (hasUnitStateFlag(UNIT_STATE_EVADING) || target->hasUnitStateFlag(UNIT_STATE_EVADING))
        return false;
    if (hasUnitStateFlag(UNIT_STATE_IN_FLIGHT) || target->hasUnitStateFlag(UNIT_STATE_IN_FLIGHT))
        return false;
    // ... both units must not be ignoring combat
    if (getAIInterface()->isCombatDisabled() || target->getAIInterface()->isCombatDisabled())
        return false;
    if (isFriendlyTo(target) || target->isFriendlyTo(this))
        return false;

    Player* playerA = getUnitOwnerOrSelf() ? getUnitOwnerOrSelf()->ToPlayer() : nullptr;
    Player* playerB = target->getUnitOwnerOrSelf() ? target->getUnitOwnerOrSelf()->ToPlayer() : nullptr;

    // ...neither of the two units must be (owned by) a player with .gm on
    if ((playerA && playerA->isGMFlagSet()) || (playerB && playerB->isGMFlagSet()))
        return false;

    return true;
}

void Unit::calculateDamage()
{
    if (isPet())
        dynamic_cast<Pet*>(this)->UpdateAP();

    const float ap_bonus = static_cast<float>(getCalculatedAttackPower()) / 14000.0f;

    const float bonus = ap_bonus * static_cast<float>(getBaseAttackTime(MELEE) + dynamic_cast<Creature*>(this)->m_speedFromHaste);

    const float delta = static_cast<float>(dynamic_cast<Creature*>(this)->ModDamageDone[0]);
    const float mult = dynamic_cast<Creature*>(this)->ModDamageDonePct[0];
    float r = (m_baseDamage[0] + bonus) * mult + delta;
    setMinDamage(r > 0 ? (isPet() ? r * 0.9f : r) : 0);

    r = (m_baseDamage[1] + bonus) * mult + delta;
    setMaxDamage(r > 0 ? (isPet() ? r * 1.1f : r) : 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// MovementInfo (from class Object)

MovementInfo* Unit::getMovementInfo() { return &obj_movement_info; }

uint32_t Unit::getUnitMovementFlags() const { return obj_movement_info.flags; }
void Unit::setUnitMovementFlags(uint32_t f) { obj_movement_info.flags = f; }
void Unit::addUnitMovementFlag(uint32_t f) { obj_movement_info.flags |= f; }
void Unit::removeUnitMovementFlag(uint32_t f) { obj_movement_info.flags &= ~f; }
bool Unit::hasUnitMovementFlag(uint32_t f) const { return (obj_movement_info.flags & f) != 0; }

//\brief: this is not uint16_t on version < wotlk
uint16_t Unit::getExtraUnitMovementFlags() const { return obj_movement_info.flags2; }
void Unit::addExtraUnitMovementFlag(uint16_t f2) { obj_movement_info.flags2 |= f2; }
bool Unit::hasExtraUnitMovementFlag(uint16_t f2) const { return (obj_movement_info.flags2 & f2) != 0; }

//helpers
bool Unit::isRooted() const
{
    return hasUnitMovementFlag(MOVEFLAG_ROOTED);
}

bool Unit::IsFalling() const
{
    return obj_movement_info.hasMovementFlag(MOVEFLAG_FALLING_MASK) || movespline->isFalling();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Movement
void Unit::setInFront(Object const* target)
{
    if (!hasUnitStateFlag(UNIT_STATE_CANNOT_TURN))
        SetOrientation(getAbsoluteAngle(target));
}

void Unit::setFacingTo(float ori, bool force)
{
    // do not face when already moving
    if (!force && (/*!IsStopped() ||*/ !movespline->Finalized()))
        return;

    MovementMgr::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ(), false);

    if (getTransGuid())
        init.DisableTransportPathTransformations(); // It makes no sense to target global orientation
    init.SetFacing(ori);

    init.Launch();
}

void Unit::setFacingToObject(Object* object, bool force)
{
    // do not face when already moving
    if (!force && (/*!IsStopped() ||*/ !movespline->Finalized()))
        return;

    MovementMgr::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ(), false);
    init.SetFacing(getAbsoluteAngle(object));   // when on transport, GetAbsoluteAngle will still return global coordinates (and angle) that needs transforming

    init.Launch();
}

void Unit::setMoveWaterWalk()
{
    addUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (isPlayer())
    {
        WorldPacket data(SMSG_MOVE_WATER_WALK, 12);
#if VERSION_STRING < Cata
        data << GetNewGUID();
        data << uint32_t(0);
#else
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_WATER_WALK);
#endif
        sendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_WATER_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_WATER_WALK);
#endif
        sendMessageToSet(&data, false);
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
        data << uint32_t(0);
#else
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_LAND_WALK);
#endif
        sendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_LAND_WALK, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_LAND_WALK);
#endif
        sendMessageToSet(&data, false);
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
        data << uint32_t(0);
#else
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_FEATHER_FALL);
#endif
        sendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_FEATHER_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_FEATHER_FALL);
#endif
        sendMessageToSet(&data, false);
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
        data << uint32_t(0);
#else
        obj_movement_info.writeMovementInfo(data, SMSG_MOVE_NORMAL_FALL);
#endif
        sendMessageToSet(&data, true);
    }

    if (isCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_NORMAL_FALL, 9);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_NORMAL_FALL);
#endif
        sendMessageToSet(&data, false);
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
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_SET_HOVER);
#endif
            sendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_HOVER);
#endif
            sendMessageToSet(&data, true);
        }
    }

    //\todo spline update
    if (isCreature())
    {
        if (set_hover)
        {
            addUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_HOVER, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_HOVER);
#endif
            sendMessageToSet(&data, false);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_HOVER, 10);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNSET_HOVER);
#endif
            sendMessageToSet(&data, false);
        }

#if VERSION_STRING >= TBC
        if (hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
            setAnimationFlags(ANIMATION_FLAG_FLY);
        else if (isHovering())
            setAnimationFlags(ANIMATION_FLAG_HOVER);
        else
            setAnimationFlags(ANIMATION_FLAG_GROUND);
#endif
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
            data << uint32_t(2);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_SET_CAN_FLY);
#endif

            sendMessageToSet(&data, true);
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
            data << uint32_t(5);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_UNSET_CAN_FLY);
#endif

            sendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (set_fly == hasUnitMovementFlag(MOVEFLAG_CAN_FLY))
            return;

        if (set_fly)
        {
            addUnitMovementFlag(MOVEFLAG_CAN_FLY);
            removeUnitMovementFlag(MOVEFLAG_FALLING);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_CAN_FLY);
        }

        if (!movespline->Initialized())
            return;

        WorldPacket data(set_fly ? SMSG_SPLINE_MOVE_SET_FLYING : SMSG_SPLINE_MOVE_UNSET_FLYING, 10);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_SET_FLYING);
#endif
        sendMessageToSet(&data, false);
    }
}

void Unit::setMoveRoot(bool set_root)
{
    if (isPlayer())
    {
        if (set_root)
        {
            addUnitMovementFlag(MOVEFLAG_ROOTED);
            stopMoving();

            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 12);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_ROOT);
#endif
            sendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_ROOTED);
            stopMoving();

            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 12);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_MOVE_UNROOT);
#endif
            sendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (set_root)
        {
            stopMoving();
            addUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_ROOT, 9);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_ROOT);
#endif
            sendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_ROOTED);

            WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 9);
#if VERSION_STRING < Cata
            data << GetNewGUID();
#else
            obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_UNROOT);
#endif
            sendMessageToSet(&data, true);
        }
    }
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
            sendMessageToSet(&data, false);
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
            sendMessageToSet(&data, false);
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
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_DISABLE);
#endif
            sendMessageToSet(&data, true);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_ENABLE, 13);
#if VERSION_STRING < Cata
            data << GetNewGUID();
            data << uint32_t(0);
#else
            obj_movement_info.writeMovementInfo(data, SMSG_MOVE_GRAVITY_ENABLE);
#endif
            sendMessageToSet(&data, true);
        }
    }

    if (isCreature())
    {
        if (disable_gravity == hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
            return;

        if (disable_gravity)
        {
            addUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);
            removeUnitMovementFlag(MOVEFLAG_FALLING);
        }
        else
        {
            removeUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);
        }

        if (isAlive() && !hasUnitStateFlag(UNIT_STATE_ROOTED))
        {
            if (hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
                setAnimationFlags(ANIMATION_FLAG_FLY);
            else if (isHovering())
                setAnimationFlags(ANIMATION_FLAG_HOVER);
            else
                setAnimationFlags(ANIMATION_FLAG_GROUND);
        }

        if (!movespline->Initialized())
            return;

        WorldPacket data(disable_gravity ? SMSG_SPLINE_MOVE_GRAVITY_DISABLE : SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 10);
#if VERSION_STRING < Cata
        data << GetNewGUID();
#else
        obj_movement_info.writeMovementInfo(data, SMSG_SPLINE_MOVE_GRAVITY_DISABLE);
#endif
        sendMessageToSet(&data, false);
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
            sendMessageToSet(&data, false);
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
            sendMessageToSet(&data, false);
        }
    }
}

#if VERSION_STRING == TBC
void Unit::setFacing(float newo)
{
    SetOrientation(newo);

    WorldPacket data(SMSG_MONSTER_MOVE, 60);
    data << GetNewGUID();
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();
    data << Util::getMSTime();
    if (newo != 0.0f)
    {
        data << uint8_t(4);
        data << newo;
    }
    else
    {
        data << uint8_t(0);
    }

    data << uint32_t(0x1000);   // move flags: run
    data << uint32_t(0);        // movetime
    data << uint32_t(1);        // 1 point
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();

    sendMessageToSet(&data, true);
}
#else
void Unit::setFacing(float newo)
{
    SetOrientation(newo);

    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << GetNewGUID();
    data << uint8_t(0);         // vehicle seat index
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(4);         // set orientation
    data << newo;
    data << uint32_t(0x1000);   // move flags: run
    data << uint32_t(0);        // movetime
    data << uint32_t(1);        // 1 point
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();

    sendMessageToSet(&data, true);
}
#endif

void Unit::handleFall(MovementInfo const& movementInfo)
{
    if (!m_zAxisPosition)
        m_zAxisPosition = movementInfo.getPosition()->z;

    uint32_t falldistance = Util::float2int32(m_zAxisPosition - movementInfo.getPosition()->z);
    if (m_zAxisPosition <= movementInfo.getPosition()->z)
        falldistance = 1;

    if (static_cast<int>(falldistance) > m_safeFall)
        falldistance -= m_safeFall;
    else
        falldistance = 1;

    bool disabledUntil = false;
    if (isPlayer())
        disabledUntil = !dynamic_cast<Player*>(this)->m_cheats.hasGodModeCheat && UNIXTIME >= dynamic_cast<Player*>(this)->getFallDisabledUntil();

    if (isAlive() && !m_isInvincible && (falldistance > 12) && !m_noFallDamage && disabledUntil)
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
                dynamic_cast<Player*>(this)->updateAchievementCriteria(
                    ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING,
                    falldistance,
                    dynamic_cast<Player*>(this)->getDrunkStateByValue(dynamic_cast<Player*>(this)->getServersideDrunkValue()),
                    0);
            }
        }
#endif

        sendEnvironmentalDamageLogPacket(getGuid(), DAMAGE_FALL, health_loss);
        addSimpleEnvironmentalDamageBatchEvent(DAMAGE_FALL, health_loss);
    }

    m_zAxisPosition = 0.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Speed

float Unit::getSpeedRate(UnitSpeedType type, bool current) const
{
    if (current)
        return m_UnitSpeedInfo.m_currentSpeedRate[type];
    else
        return m_UnitSpeedInfo.m_basicSpeedRate[type];
}

#if VERSION_STRING < Cata
void Unit::setSpeedRate(UnitSpeedType mtype, float rate, bool current)
{
    if (rate < 0)
        rate = 0.0f;

    // Update speed only on change
    if (m_UnitSpeedInfo.m_currentSpeedRate[mtype] == rate)
        return;

    if (current)
        m_UnitSpeedInfo.m_currentSpeedRate[mtype] = rate;
    else
        m_UnitSpeedInfo.m_basicSpeedRate[mtype] = rate;

    // Update Also For Movement Generators
    propagateSpeedChange();

    // Spline packets are for units controlled by AI. "Force speed change" (wrongly named opcodes) and "move set speed" packets are for units controlled by a player.
#if VERSION_STRING == Classic
    static Opcodes const moveTypeToOpcode[MAX_SPEED_TYPE][3] =
    {
        {SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED         },
        {SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED          },
        {SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED     },
        {SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED         },
        {SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED    },
        {SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE          },
    };
#endif

#if VERSION_STRING == TBC
    static Opcodes const moveTypeToOpcode[MAX_SPEED_TYPE][3] =
    {
        {SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED         },
        {SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED          },
        {SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED     },
        {SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED         },
        {SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED    },
        {SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE          },
        {SMSG_SPLINE_SET_FLIGHT_SPEED,      SMSG_FORCE_FLIGHT_SPEED_CHANGE,         MSG_MOVE_SET_FLIGHT_SPEED       },
        {SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE,    MSG_MOVE_SET_FLIGHT_BACK_SPEED  },
    };
#endif

#if VERSION_STRING == WotLK
    static uint16_t const moveTypeToOpcode[MAX_SPEED_TYPE][3] =
    {
        {SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED         },
        {SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED          },
        {SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED     },
        {SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED         },
        {SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED    },
        {SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE          },
        {SMSG_SPLINE_SET_FLIGHT_SPEED,      SMSG_FORCE_FLIGHT_SPEED_CHANGE,         MSG_MOVE_SET_FLIGHT_SPEED       },
        {SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE,    MSG_MOVE_SET_FLIGHT_BACK_SPEED  },
        {SMSG_SPLINE_SET_PITCH_RATE,        SMSG_FORCE_PITCH_RATE_CHANGE,           MSG_MOVE_SET_PITCH_RATE         },
    };
#endif

    if (auto* const plr = isPlayer() ? dynamic_cast<Player*>(this) : nullptr)
    {
        // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
        // and do it only for real sent packets and use run for run/mounted as client expected
        ++plr->m_forced_speed_changes[mtype];

        if (!isInCombat())
            plr->getSummonInterface()->notifyOnOwnerSpeedChange(mtype, m_UnitSpeedInfo.m_currentSpeedRate[mtype], false);
    }

    Player* player_mover = getWorldMapPlayer(getCharmedByGuid());
    if (player_mover == nullptr)
    {
        if (isPlayer())
            player_mover = dynamic_cast<Player*>(this);
    }

    if (player_mover) // unit controlled by a player.
    {
#if VERSION_STRING < Cata
        // Send notification to self. this packet is only sent to one client (the client of the player concerned by the change).
        WorldPacket self;
        self.Initialize(moveTypeToOpcode[mtype][1], mtype != TYPE_RUN ? 8 + 4 + 4 : 8 + 4 + 1 + 4);
        self << GetNewGUID();
        self << (uint32_t)0;                                  // Movement counter.
        if (mtype == TYPE_RUN)
            self << uint8_t(1);                               // unknown byte added in 2.1.0
        self << float(rate);

        player_mover->sendPacket(&self);
#endif
        // Send notification to other players. sent to every clients (if in range) except one: the client of the player concerned by the change.
        WorldPacket data;
        data.Initialize(moveTypeToOpcode[mtype][2], 8 + 30 + 4);
        data << GetNewGUID();
        buildMovementPacket(&data);
        data << float(rate);

        player_mover->sendMessageToSet(&data, false);
    }
    else // unit controlled by AI.
    {
        // send notification to every clients.
        WorldPacket data;
        data.Initialize(moveTypeToOpcode[mtype][0], 8 + 4);
        data << GetNewGUID();
        data << float(rate);
        sendMessageToSet(&data, false);
    }
}
#else
void Unit::setSpeedRate(UnitSpeedType type, float value, bool current)
{
    if (value < 0)
        value = 0.0f;

    // Update speed only on change
    if (m_UnitSpeedInfo.m_currentSpeedRate[type] == value)
        return;

    if (current)
        m_UnitSpeedInfo.m_currentSpeedRate[type] = value;
    else
        m_UnitSpeedInfo.m_basicSpeedRate[type] = value;

    // Update Also For Movement Generators
    propagateSpeedChange();

    if (auto* const plr = isPlayer() ? dynamic_cast<Player*>(this) : nullptr)
    {
        // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
        // and do it only for real sent packets and use run for run/mounted as client expected
        ++plr->m_forced_speed_changes[type];

        if (!isInCombat())
            plr->getSummonInterface()->notifyOnOwnerSpeedChange(type, m_UnitSpeedInfo.m_currentSpeedRate[type], false);
    }

    WorldPacket data;
    ObjectGuid guid = getGuid();

    switch (type)
    {
    case TYPE_WALK:
        data.Initialize(SMSG_SPLINE_SET_WALK_SPEED, 8 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4);
        data.writeBit(guid[0]);
        data.writeBit(guid[6]);
        data.writeBit(guid[7]);
        data.writeBit(guid[3]);
        data.writeBit(guid[5]);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
        data.writeBit(guid[4]);
        data.flushBits();
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[3]);
        data << float(value);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[5]);
        break;
    case TYPE_RUN:
        data.Initialize(SMSG_SPLINE_SET_RUN_SPEED, 1 + 8 + 4);
        data.writeBit(guid[4]);
        data.writeBit(guid[0]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);
        data.writeBit(guid[6]);
        data.writeBit(guid[3]);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
        data.flushBits();
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[4]);
        data << float(value);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[1]);
        break;
    case TYPE_RUN_BACK:
        data.Initialize(SMSG_SPLINE_SET_RUN_BACK_SPEED, 1 + 8 + 4);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
        data.writeBit(guid[6]);
        data.writeBit(guid[0]);
        data.writeBit(guid[3]);
        data.writeBit(guid[7]);
        data.writeBit(guid[5]);
        data.writeBit(guid[4]);
        data.flushBits();
        data.WriteByteSeq(guid[1]);
        data << float(value);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[7]);
        break;
    case TYPE_SWIM:
        data.Initialize(SMSG_SPLINE_SET_SWIM_SPEED, 1 + 8 + 4);
        data.writeBit(guid[4]);
        data.writeBit(guid[2]);
        data.writeBit(guid[5]);
        data.writeBit(guid[0]);
        data.writeBit(guid[7]);
        data.writeBit(guid[6]);
        data.writeBit(guid[3]);
        data.writeBit(guid[1]);
        data.flushBits();
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[4]);
        data << float(value);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[3]);
        break;
    case TYPE_SWIM_BACK:
        data.Initialize(SMSG_SPLINE_SET_SWIM_BACK_SPEED, 1 + 8 + 4);
        data.writeBit(guid[0]);
        data.writeBit(guid[1]);
        data.writeBit(guid[3]);
        data.writeBit(guid[6]);
        data.writeBit(guid[4]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);
        data.writeBit(guid[2]);
        data.flushBits();
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[6]);
        data << float(value);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[2]);
        break;
    case TYPE_TURN_RATE:
        data.Initialize(SMSG_SPLINE_SET_TURN_RATE, 1 + 8 + 4);
        data.writeBit(guid[2]);
        data.writeBit(guid[4]);
        data.writeBit(guid[6]);
        data.writeBit(guid[1]);
        data.writeBit(guid[3]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);
        data.writeBit(guid[0]);
        data.flushBits();
        data << float(value);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[0]);
        break;
    case TYPE_FLY:
        data.Initialize(SMSG_SPLINE_SET_FLIGHT_SPEED, 1 + 8 + 4);
        data.writeBit(guid[7]);
        data.writeBit(guid[4]);
        data.writeBit(guid[0]);
        data.writeBit(guid[1]);
        data.writeBit(guid[3]);
        data.writeBit(guid[6]);
        data.writeBit(guid[5]);
        data.writeBit(guid[2]);
        data.flushBits();
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data << float(value);
        break;
    case TYPE_FLY_BACK:
        data.Initialize(SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4);
        data.writeBit(guid[2]);
        data.writeBit(guid[1]);
        data.writeBit(guid[6]);
        data.writeBit(guid[5]);
        data.writeBit(guid[0]);
        data.writeBit(guid[3]);
        data.writeBit(guid[4]);
        data.writeBit(guid[7]);
        data.flushBits();
        data.WriteByteSeq(guid[5]);
        data << float(value);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[4]);
        break;
    case TYPE_PITCH_RATE:
        data.Initialize(SMSG_SPLINE_SET_PITCH_RATE, 1 + 8 + 4);
        data.writeBit(guid[3]);
        data.writeBit(guid[5]);
        data.writeBit(guid[6]);
        data.writeBit(guid[1]);
        data.writeBit(guid[0]);
        data.writeBit(guid[4]);
        data.writeBit(guid[7]);
        data.writeBit(guid[2]);
        data.flushBits();
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[2]);
        data << float(value);
        data.WriteByteSeq(guid[4]);
        break;
    default:
        sLogger.failure("Unit::setSpeedRate: Unsupported move type ({}), data not sent to client.", type);
        return;
    }

    sendMessageToSet(&data, true);
}
#endif

void Unit::resetCurrentSpeeds()
{
    for (uint8_t i = 0; i < MAX_SPEED_TYPE; ++i)
        m_UnitSpeedInfo.m_currentSpeedRate[i] = m_UnitSpeedInfo.m_basicSpeedRate[i];
}
void Unit::propagateSpeedChange()
{
    getMovementManager()->propagateSpeedChange();
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

bool Unit::getSpeedDecrease()
{
    int32_t before = m_speedModifier;
    m_speedModifier -= m_slowdown;
    m_slowdown = 0;

    for (auto& itr : speedReductionMap)
        m_slowdown = static_cast<int32_t>(std::min(m_slowdown, itr.second));

    if (m_slowdown < -100)
        m_slowdown = 100;

    m_speedModifier += m_slowdown;

    if (m_speedModifier != before)
        return true;

    return false;
}

void Unit::updateSpeed()
{
    if (getMountDisplayId() == 0)
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, false) * (1.0f + static_cast<float>(m_speedModifier) / 100.0f), true);
    }
    else
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, false) * (1.0f + static_cast<float>(m_mountedspeedModifier) / 100.0f), true);
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true) + (m_speedModifier < 0 ? (getSpeedRate(TYPE_RUN, false) * static_cast<float>(m_speedModifier) / 100.0f) : 0), true);
    }

    setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, false) * (1.0f + ((float)m_flyspeedModifier) / 100.0f), true);

    // Limit speed due to effects such as http://www.wowhead.com/?spell=31896 [Judgement of Justice]
    if (m_maxSpeed && getSpeedRate(TYPE_RUN, true) > m_maxSpeed)
    {
        setSpeedRate(TYPE_RUN, m_maxSpeed, true);
    }

    if (isPlayer() && dynamic_cast<Player*>(this)->m_changingMaps)
    {
        dynamic_cast<Player*>(this)->m_resendSpeed = true;
    }
    else
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true), true);
        setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, true), true);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Movement spline
void Unit::removeAllFollowers()
{
    while (!m_followingMe.empty())
        (*m_followingMe.begin())->setTarget(nullptr);
}

bool Unit::canFly()
{
    return false;
}

void Unit::stopMoving()
{
    removeUnitStateFlag(UNIT_STATE_MOVING);

    // not need send any packets if not in world or not moving
    if (!IsInWorld() || movespline->Finalized())
        return;

    // Update position now since Stop does not start a new movement that can be updated later
    if (movespline->HasStarted())
        updateSplinePosition();
    MovementMgr::MoveSplineInit init(this);
    init.Stop();
}

void Unit::pauseMovement(uint32_t timer/* = 0*/, uint8_t slot/* = 0*/, bool forced/* = true*/)
{
    if (isInvalidMovementSlot(slot))
        return;

    if (MovementGenerator* movementGenerator = getMovementManager()->getCurrentMovementGenerator(MovementSlot(slot)))
        movementGenerator->pause(timer);

    if (forced && getMovementManager()->getCurrentSlot() == MovementSlot(slot))
        stopMoving();
}

void Unit::resumeMovement(uint32_t timer/* = 0*/, uint8_t slot/* = 0*/)
{
    if (isInvalidMovementSlot(slot))
        return;

    if (MovementGenerator* movementGenerator = getMovementManager()->getCurrentMovementGenerator(MovementSlot(slot)))
        movementGenerator->resume(timer);
}

void Unit::setFeared(bool apply)
{
    if (apply)
    {
        setTargetGuid(0);

        Unit* caster = nullptr;
        if (const auto fearAura = getAuraWithAuraEffect(SPELL_AURA_MOD_FEAR))
            caster = fearAura->GetUnitCaster();

        if (caster == nullptr)
            caster = getAIInterface()->getCurrentTarget();

        getMovementManager()->moveFleeing(caster);             // caster == NULL processed in MoveFleeing
    }
    else
    {
        if (isAlive())
        {
            getMovementManager()->remove(FLEEING_MOTION_TYPE);
            if (getThreatManager().getCurrentVictim())
                setTargetGuid(getThreatManager().getCurrentVictim()->getGuid());

            if (!isPlayer() && !isInCombat() && getTargetGuid() == 0)
                getMovementManager()->moveTargetedHome();
            else
                getMovementManager()->moveChase(getThreatManager().getCurrentVictim());
        }
    }

    // block / allow control to real player in control (eg charmer)
    if (isPlayer())
    {
        if (auto* const plrOwner = getPlayerOwnerOrSelf())
            plrOwner->sendClientControlPacket(this, !apply);
    }
}

void Unit::setConfused(bool apply)
{
    if (apply)
    {
        setTargetGuid(0);
        getMovementManager()->moveConfused();
    }
    else
    {
        if (isAlive())
        {
            getMovementManager()->remove(CONFUSED_MOTION_TYPE);
            if (getThreatManager().getCurrentVictim())
                setTargetGuid(getThreatManager().getCurrentVictim()->getGuid());
        }
    }

    // block / allow control to real player in control (eg charmer)
    if (isPlayer())
    {
        if (auto* const plrOwner = getPlayerOwnerOrSelf())
            plrOwner->sendClientControlPacket(this, !apply);
    }
}

void Unit::setStunned(bool apply)
{
    if (apply)
    {
        setTargetGuid(0);
        addUnitFlags(UNIT_FLAG_STUNNED);

        // MOVEMENTFLAG_ROOT cannot be used in conjunction with MOVEMENTFLAG_MASK_MOVING (tested 3.3.5a)
        // this will freeze clients. That's why we remove MOVEMENTFLAG_MASK_MOVING before
        // setting MOVEMENTFLAG_ROOT
        removeUnitMovementFlag(MOVEFLAG_MOVING_MASK);
        addUnitMovementFlag(MOVEFLAG_ROOTED);
        stopMoving();

        if (isPlayer())
        {
            setStandState(STANDSTATE_STAND);

            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
            data << GetNewGUID();
            data << 0;
            sendMessageToSet(&data, true);
        }
        else
        {
            WorldPacket data(SMSG_SPLINE_MOVE_ROOT, 8);
            data << 0;
            sendMessageToSet(&data, true);
        }
    }
    else
    {
        if (isAlive() && getThreatManager().getCurrentVictim())
            setTargetGuid(getThreatManager().getCurrentVictim()->getGuid());

        // don't remove UNIT_FLAG_STUNNED for pet when owner is mounted (disabled pet's interface)
        Player* owner = getPlayerOwner();
        if (!owner || !owner->isMounted())
            removeUnitFlags(UNIT_FLAG_STUNNED);

        if (!hasUnitStateFlag(UNIT_STATE_ROOTED))         // prevent moving if it also has root effect
        {
            if (isPlayer())
            {
                WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 10);
                data << GetNewGUID();
                data << 0;
                sendMessageToSet(&data, true);
            }
            else
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 8);
                data << GetNewGUID();
                sendMessageToSet(&data, true);
            }

            removeUnitMovementFlag(MOVEFLAG_ROOTED);
        }
    }
}

void Unit::updateSplineMovement(uint32_t t_diff)
{
    if (movespline->Finalized())
        return;

    movespline->updateState(t_diff);
    bool arrived = movespline->Finalized();

    if (movespline->isCyclic())
    {
        m_splineSyncTimer -= t_diff;
        if (m_splineSyncTimer <= 0)
        {
            m_splineSyncTimer = 5000; // Retail value, do not change

            ByteBuffer packedGuid;
            packedGuid.appendPackGUID(getGuid());

            WorldPacket data(SMSG_FLIGHT_SPLINE_SYNC, 4 + packedGuid.size());
            MovementMgr::PacketBuilder::WriteSplineSync(*movespline, data);
            data.append(packedGuid);
            sendMessageToSet(&data, true);
        }
    }

    if (arrived)
    {
        disableSpline();
#if VERSION_STRING >= WotLK
        if (movespline->HasAnimation())
            setAnimationFlags(movespline->GetAnimationTier());
#endif
    }

    updateSplinePosition();
}

void Unit::updateSplinePosition()
{
    MovementMgr::Location loc = movespline->ComputePosition();

    if (movespline->onTransport)
    {
        LocationVector& pos = getMovementInfo()->transport_position;
        pos.x = loc.x;
        pos.y = loc.y;
        pos.z = loc.z;
        pos.o = normalizeOrientation(loc.orientation);

#ifdef FT_VEHICLES
        if (TransportBase* vehicle = getVehicle())
        {
            vehicle->calculatePassengerPosition(loc.x, loc.y, loc.z, &loc.orientation);
        }
        else if (TransportBase* transport = GetTransport())
        {
            transport->calculatePassengerPosition(loc.x, loc.y, loc.z, &loc.orientation);
        }
        else
        {
            return;
        }
#else
        if (TransportBase* transport = GetTransport())
        {
            transport->calculatePassengerPosition(loc.x, loc.y, loc.z, &loc.orientation);
        }
        else
        {
            return;
        }
#endif
    }

    if (hasUnitStateFlag(UNIT_STATE_CANNOT_TURN))
        loc.orientation = GetOrientation();

    SetPosition(loc.x, loc.y, loc.z, loc.orientation);
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

    sendMessageToSet(&data, false);
}

void Unit::disableSpline()
{
#if VERSION_STRING >= Cata
    getMovementInfo()->removeMovementFlag(MovementFlags(MOVEFLAG_MOVE_FORWARD));
#else
    getMovementInfo()->removeMovementFlag(MovementFlags(MOVEFLAG_SPLINE_FORWARD_ENABLED));
#endif

    movespline->_Interrupt();
}

bool Unit::isSplineEnabled() const
{
    return movespline->Initialized();
}

void Unit::jumpTo(float speedXY, float speedZ, bool forward, Optional<LocationVector> dest)
{
    float angle = forward ? 0 : static_cast<float>(M_PI);
    if (dest)
        angle += getRelativeAngle(*dest);

    if (isCreature())
    {
        getMovementManager()->moveJumpTo(angle, speedXY, speedZ);
    }
    else
    {
        const float vcos = std::cos(angle + GetOrientation());
        const float vsin = std::sin(angle + GetOrientation());

        WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8 + 4 + 4 + 4 + 4 + 4));
        data << GetNewGUID();
        data << uint32_t(0);                                    // Sequence
        data << vcos << vsin;
        data << float(speedXY);                                 // Horizontal speed
        data << float(-speedZ);                                 // Z Movement speed (vertical)

        ToPlayer()->sendPacket(&data);
    }
}

void Unit::jumpTo(Object* obj, float speedZ, bool withOrientation)
{
    float x, y, z;
    obj->getNearPoint(this, x, y, z, 0.5f, getAbsoluteAngle(obj->GetPosition()));
    float speedXY = getExactDist2d(x, y) * 10.0f / speedZ;
    getMovementManager()->moveJump(x, y, z, getAbsoluteAngle(obj), speedXY, speedZ, EVENT_JUMP, withOrientation);
}

void Unit::handleKnockback(Object* object, float horizontal, float vertical)
{
    if (object == nullptr)
        object = this;

    float angle = calcRadAngle(object->GetPositionX(), object->GetPositionY(), GetPositionX(), GetPositionY());
    if (object == this)
        angle = static_cast<float>(M_PI + GetOrientation());

    float destx, desty, destz;
    if (GetPoint(angle, horizontal, destx, desty, destz, true))
        getMovementManager()->moveKnockbackFrom(destx, desty, horizontal, vertical);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Internal States

void Unit::setControlled(bool apply, UnitStates state)
{
    if (apply)
    {
        if (hasUnitStateFlag(state))
            return;

        addUnitStateFlag(state);
        switch (state)
        {
        case UNIT_STATE_STUNNED:
            setStunned(true);
            break;
        case UNIT_STATE_ROOTED:
            if (!hasUnitStateFlag(UNIT_STATE_STUNNED))
                setMoveRoot(true);
            break;
        case UNIT_STATE_CONFUSED:
            if (!hasUnitStateFlag(UNIT_STATE_STUNNED))
            {
                removeUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
                setConfused(true);
            }
            break;
        case UNIT_STATE_FLEEING:
            if (!hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED))
            {
                removeUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
                setFeared(true);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch (state)
        {
        case UNIT_STATE_STUNNED:
            if (getAuraWithAuraEffect(SPELL_AURA_MOD_STUN))
                return;

            removeUnitStateFlag(state);
            setStunned(false);
            break;
        case UNIT_STATE_ROOTED:
            if (getAuraWithAuraEffect(SPELL_AURA_MOD_ROOT) || isVehicle() || (ToCreature() && ToCreature()->getMovementTemplate().isRooted()))
                return;

            removeUnitStateFlag(state);
            setMoveRoot(false);
            break;
        case UNIT_STATE_CONFUSED:
            if (getAuraWithAuraEffect(SPELL_AURA_MOD_CONFUSE))
                return;

            removeUnitStateFlag(state);
            setConfused(false);
            break;
        case UNIT_STATE_FLEEING:
            if (getAuraWithAuraEffect(SPELL_AURA_MOD_FEAR))
                return;

            removeUnitStateFlag(state);
            setFeared(false);
            break;
        default:
            return;
        }

        applyControlStatesIfNeeded();
    }
}

void Unit::applyControlStatesIfNeeded()
{
    // Unit States might have been already cleared but auras still present. I need to check with HasAuraType
    if (hasUnitStateFlag(UNIT_STATE_STUNNED) || getAuraWithAuraEffect(SPELL_AURA_MOD_STUN))
        setStunned(true);

    if (hasUnitStateFlag(UNIT_STATE_ROOTED) || getAuraWithAuraEffect(SPELL_AURA_MOD_ROOT))
        setMoveRoot(true);

    if (hasUnitStateFlag(UNIT_STATE_CONFUSED) || getAuraWithAuraEffect(SPELL_AURA_MOD_CONFUSE))
        setConfused(true);

    if (hasUnitStateFlag(UNIT_STATE_FLEEING) || getAuraWithAuraEffect(SPELL_AURA_MOD_FEAR))
        setFeared(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

void Unit::playSpellVisual(uint32_t visual_id, uint32_t type)
{
    sendMessageToSet(SmsgPlaySpellVisual(getGuid(), visual_id, type).serialise().get(), true);
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
    for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
    {
        if (const auto* aur = getAuraWithAuraSlot(x))
        {
            aura_group = sSpellMgr.getDiminishingGroup(aur->getSpellInfo()->getId());
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

    auto plrUnit = dynamic_cast<Player*>(this);
    if (enable)
    {
        if (!plrUnit->hasSkillLine(SKILL_DUAL_WIELD))
            plrUnit->addSkillLine(SKILL_DUAL_WIELD, 1, 1);
    }
    else
    {
        if (plrUnit->canDualWield2H())
            plrUnit->setDualWield2H(false);

        plrUnit->removeSkillLine(SKILL_DUAL_WIELD);
    }
}

SpellCastResult Unit::castSpell(uint64_t targetGuid, uint32_t spellId, bool triggered/* = false*/)
{
    const SpellForcedBasePoints forcedBasePoints;
    return castSpell(targetGuid, spellId, forcedBasePoints, triggered);
}

SpellCastResult Unit::castSpell(Unit* target, uint32_t spellId, bool triggered/* = false*/)
{
    const SpellForcedBasePoints forcedBasePoints;
    return castSpell(target, spellId, forcedBasePoints, triggered);
}

SpellCastResult Unit::castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, bool triggered/* = false*/)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    const SpellForcedBasePoints forcedBasePoints;
    return castSpell(targetGuid, spellInfo, forcedBasePoints, triggered);
}

SpellCastResult Unit::castSpell(Unit* target, SpellInfo const* spellInfo, bool triggered/* = false*/)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;
    
    const SpellForcedBasePoints forcedBasePoints;
    return castSpell(target, spellInfo, forcedBasePoints, triggered);
}

SpellCastResult Unit::castSpell(uint64_t targetGuid, uint32_t spellId, SpellForcedBasePoints forcedBasepoints, bool triggered/* = false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    return castSpell(targetGuid, spellInfo, forcedBasepoints, triggered);
}

SpellCastResult Unit::castSpell(Unit* target, uint32_t spellId, SpellForcedBasePoints forcedBasepoints, bool triggered/* = false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    return castSpell(target, spellInfo, forcedBasepoints, triggered);
}

SpellCastResult Unit::castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasePoints, int32_t spellCharges, bool triggered/* = false*/)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    newSpell->forced_basepoints = std::make_shared<SpellForcedBasePoints>(forcedBasePoints);
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
    return newSpell->prepare(&targets);
}

SpellCastResult Unit::castSpell(SpellCastTargets targets, uint32_t spellId, bool triggered/* = false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    return castSpell(targets, spellInfo, triggered);
}

SpellCastResult Unit::castSpell(SpellCastTargets targets, SpellInfo const* spellInfo, bool triggered/* = false*/)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    return newSpell->prepare(&targets);
}

SpellCastResult Unit::castSpellLoc(const LocationVector location, uint32_t spellId, bool triggered/* = false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    return castSpellLoc(location, spellInfo, triggered);
}

SpellCastResult Unit::castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered/* = false*/)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    SpellCastTargets targets;
    targets.setDestination(location);
    targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);

    // Prepare the spell
    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    return newSpell->prepare(&targets);
}

void Unit::eventCastSpell(Unit* target, SpellInfo const* spellInfo)
{
    const SpellForcedBasePoints forcedBasePoints;
    if (spellInfo != nullptr)
        castSpell(target, spellInfo, forcedBasePoints, true);
    else
        sLogger.failure("Unit::eventCastSpell tried to cast invalid spell with no spellInfo (nullptr)");
}

SpellCastResult Unit::castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    newSpell->forced_basepoints = std::make_shared<SpellForcedBasePoints>(forcedBasepoints);

    SpellCastTargets targets(targetGuid);

    // Prepare the spell
    return newSpell->prepare(&targets);
}

SpellCastResult Unit::castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered)
{
    if (spellInfo == nullptr)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, triggered, nullptr);
    newSpell->forced_basepoints = std::make_shared<SpellForcedBasePoints>(forcedBasepoints);

    SpellCastTargets targets(0);
    if (target != nullptr)
    {
        targets.addTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(target->getGuid());
    }
    else
        newSpell->GenerateTargets(&targets);

    // Prepare the spell
    return newSpell->prepare(&targets);
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
    auto spellProcHolder = sSpellProcMgr.newSpellProc(this, spellInfo, originalSpellInfo, casterGuid, procChance, procFlags, exProcFlags, spellFamilyMask, procClassMask, createdByAura, obj);
    if (spellProcHolder == nullptr)
    {
        if (originalSpellInfo != nullptr)
            sLogger.failure("Unit::addProcTriggerSpell : Spell id {} tried to add a non-existent spell to Unit {} as SpellProc", originalSpellInfo->getId(), fmt::ptr(this));
        else
            sLogger.failure("Unit::addProcTriggerSpell : Something tried to add a non-existent spell to Unit {} as SpellProc", fmt::ptr(this));
        return nullptr;
    }

    spellProc = spellProcHolder.get();
    m_procSpells.emplace_back(std::move(spellProcHolder));
    return spellProc;
}

SpellProc* Unit::getProcTriggerSpell(uint32_t spellId, uint64_t casterGuid) const
{
    for (const auto& spellProc : m_procSpells)
    {
        if (spellProc->getSpell()->getId() == spellId && (casterGuid == 0 || spellProc->getCasterGuid() == casterGuid))
            return spellProc.get();
    }

    return nullptr;
}

void Unit::removeProcTriggerSpell(uint32_t spellId, uint64_t casterGuid/* = 0*/, uint64_t misc/* = 0*/)
{
    for (auto& spellProc : m_procSpells)
    {
        if (sScriptMgr.callScriptedSpellProcCanDelete(spellProc.get(), spellId, casterGuid, misc))
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
        switch (this->getClass())
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
        bonusHeal = static_cast<float_t>(m_healDoneMod[school]);
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
    heal += heal * m_healDonePctMod[school];

    return heal;
}

float_t Unit::applySpellDamageBonus(SpellInfo const* spellInfo, int32_t baseDmg, float_t effectPctModifier/* = 1.0f*/, bool isPeriodic/* = false*/, Spell* castingSpell/* = nullptr*/, Aura* aur/* = nullptr*/)
{
    const auto floatDmg = static_cast<float_t>(baseDmg);
    if (spellInfo->getAttributesExC() & ATTRIBUTESEXC_NO_DONE_BONUS)
        return floatDmg;

    if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_NOT_USING_DMG_BONUS)
        return floatDmg;

    // Check if class can benefit from spell power
    auto canBenefitFromSpellPower = true;
    if (isPlayer())
    {
        switch (this->getClass())
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
                canBenefitFromSpellPower = false;
                break;
            default:
                break;
        }
    }

    float_t bonusDmg = 0.0f, bonusAp = 0.0f;
    const auto school = spellInfo->getFirstSchoolFromSchoolMask();

    if (canBenefitFromSpellPower)
    {
        if (aur != nullptr)
            bonusDmg = static_cast<float_t>(aur->getSpellPowerBonus());
        else
            bonusDmg = static_cast<float_t>(GetDamageDoneMod(school));

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

        applySpellModifiers(SPELLMOD_PENALTY, &bonusDmg, spellInfo, castingSpell, aur);
    }

    if (aur != nullptr)
        bonusAp = static_cast<float_t>(aur->getAttackPowerBonus());
    else
        bonusAp = static_cast<float_t>(getAttackPower());

    if (bonusAp > 0.0f)
    {
        // Get attack power bonus
        if (isPeriodic || aur != nullptr)
        {
            if (spellInfo->spell_ap_coeff_overtime > 0.0f)
                bonusAp *= spellInfo->spell_ap_coeff_overtime * effectPctModifier;
            else
                bonusAp = 0.0f;
        }
        else
        {
            if (spellInfo->spell_ap_coeff_direct > 0.0f)
                bonusAp *= spellInfo->spell_ap_coeff_direct * effectPctModifier;
            else
                bonusAp = 0.0f;
        }
    }

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
    PlayerCombatRating resilienceType = CR_WEAPON_SKILL;

    if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_RANGED)
    {
        if (isPlayer())
        {
            critChance = dynamic_cast<Player const*>(this)->getRangedCritPercentage();
            if (target->isPlayer())
                critChance += dynamic_cast<Player*>(target)->getResistRCrit();

            critChance += static_cast<float_t>(target->m_attackerCritChanceMod[school]);
        }
        else
        {
            // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)
            critChance = 5.0f;
        }

        if (target->isPlayer())
#if VERSION_STRING >= Cata
            resilienceType = CR_RESILIENCE_PLAYER_DAMAGE_TAKEN;
#else
            resilienceType = CR_CRIT_TAKEN_RANGED;
#endif
    }
    else if (spellInfo->getDmgClass() == SPELL_DMG_TYPE_MELEE)
    {
        // Same shit with the melee spells, such as Judgment/Seal of Command
        if (isPlayer())
            critChance = dynamic_cast<Player const*>(this)->getMeleeCritPercentage();

        if (target->isPlayer())
        {
            //this could be ability but in that case we overwrite the value
            critChance += dynamic_cast<Player*>(target)->getResistMCrit();
#if VERSION_STRING >= Cata
            resilienceType = CR_RESILIENCE_CRIT_TAKEN;
#else
            resilienceType = CR_CRIT_TAKEN_MELEE;
#endif
        }

        // Victim's (!) crit chance mod for physical attacks?
        critChance += static_cast<float_t>(target->m_attackerCritChanceMod[0]);
    }
    else
    {
        critChance = m_spellCritPercentage + m_spellCritChanceSchool[school];

        critChance += static_cast<float_t>(target->m_attackerCritChanceMod[school]);

        //\todo Zyres: is tis relly the way this should work?
        if (isPlayer() && (target->m_rootCounter - target->m_stunned))
            critChance += static_cast<float_t>(dynamic_cast<Player const*>(this)->m_rootedCritChanceBonus);

        if (target->isPlayer())
            resilienceType = CR_CRIT_TAKEN_SPELL;
    }

    applySpellModifiers(SPELLMOD_CRITICAL, &critChance, spellInfo, spell, aura);

    if (resilienceType != CR_WEAPON_SKILL)
        critChance -= dynamic_cast<Player*>(target)->calcRating(resilienceType);

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

    float_t critChance = m_spellCritPercentage + m_spellCritChanceSchool[school];
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

    return Util::checkChance(getCriticalChanceForDamageSpell(spell, nullptr, dynamic_cast<Unit*>(target)));
}

bool Unit::isCriticalHealForSpell(Object* target, Spell* spell)
{
    // Spell cannot crit against gameobjects or items
    if (!target->isCreatureOrPlayer())
        return false;

    return Util::checkChance(getCriticalChanceForHealSpell(spell, nullptr, dynamic_cast<Unit*>(target)));
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
#if VERSION_STRING >= Cata
        float_t dmgReductionPct = 2.0f * dynamic_cast<Player*>(target)->calcRating(CR_RESILIENCE_CRIT_TAKEN) / 100.0f;
#else
        float_t dmgReductionPct = 2.0f * static_cast<Player*>(target)->calcRating(CR_CRIT_TAKEN_MELEE) / 100.0f;
#endif
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

    target->sendMessageToSet(&data, true);
}

void Unit::sendSpellHealLog(Object* caster, Object* target, uint32_t spellId, uint32_t healAmount, bool isCritical, uint32_t overHeal, uint32_t absorbedHeal)
{
    if (caster == nullptr || target == nullptr)
        return;

    target->sendMessageToSet(SmsgSpellHealLog(target->GetNewGUID(), caster->GetNewGUID(), spellId, healAmount, overHeal, absorbedHeal, isCritical).serialise().get(), true);
}

void Unit::sendSpellOrDamageImmune(uint64_t casterGuid, Unit* target, uint32_t spellId)
{
    target->sendMessageToSet(SmsgSpellOrDamageImmune(casterGuid, target->getGuid(), spellId).serialise().get(), true);
}

#if VERSION_STRING > TBC
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

    sendMessageToSet(&data, true);
}
#else
void Unit::sendAttackerStateUpdate(const WoWGuid& attackerGuid, const WoWGuid& victimGuid, HitStatus hitStatus, uint32_t damage, [[maybe_unused]] uint32_t overKill, DamageInfo damageInfo, uint32_t absorbedDamage, VisualState visualState, uint32_t blockedDamage, [[maybe_unused]] uint32_t rageGain)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Status {}, damage {}", uint32_t(hitStatus), damage);

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (4 + 8 + 8 + 4) + 1 + (1 * (4 + 4 + 4 + 4 + 4)) + (4 + 4 + 4 + 4));

    // School type in classic, school mask in tbc+
    uint32_t school;
#if VERSION_STRING == Classic
    school = damageInfo.getSchoolTypeFromMask();
#else
    school = damageInfo.schoolMask;
#endif

    data << uint32_t(hitStatus);
    data << attackerGuid;
    data << victimGuid;
    data << uint32_t(damage);                                   // real damage

    data << uint8_t(1);                                         // damage counter

    data << uint32_t(school);       // damage school
    data << float(damageInfo.fullDamage);
    data << uint32_t(damageInfo.fullDamage);
    data << uint32_t(absorbedDamage);
    data << int32_t(damageInfo.resistedDamage);

    data << uint32_t(visualState);
    data << uint32_t(0);
    data << uint32_t(0);

    data << uint32_t(blockedDamage);

    if (hitStatus & HITSTATUS_UNK_00)
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
        for (uint8_t i = 0; i < 5; ++i)
        {
            data << float(0);
            data << float(0);
        }
        data << uint32_t(0);
    }

    sendMessageToSet(&data, true);
}
#endif

void Unit::addSpellModifier(AuraEffectModifier const* aurEff, bool apply)
{
    if (aurEff == nullptr)
        return;

    const auto aur = aurEff->getAura();
    if (isPlayer())
    {
#if VERSION_STRING >= Cata
        std::vector<std::pair<uint8_t, float>> modValues;
#endif
        const auto isPct = aurEff->getAuraEffectType() == SPELL_AURA_ADD_PCT_MODIFIER;
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

#if VERSION_STRING < Cata
                static_cast<Player*>(this)->sendSpellModifierPacket(bit, static_cast<uint8_t>(aurEff->getEffectMiscValue()), totalMod, isPct);
#else
                modValues.push_back(std::make_pair(bit, static_cast<float>(totalMod)));
#endif
            }
        }

#if VERSION_STRING >= Cata
        dynamic_cast<Player*>(this)->sendSpellModifierPacket(static_cast<uint8_t>(aurEff->getEffectMiscValue()), modValues, isPct);
        modValues.clear();
#endif
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

void Unit::addSpellImmunity(SpellImmunityMask immunityMask, bool apply)
{
    if (apply)
        m_spellImmunityMask |= immunityMask;
    else
        m_spellImmunityMask &= ~immunityMask;
}

uint32_t Unit::getSpellImmunity() const
{
    return m_spellImmunityMask;
}

bool Unit::hasSpellImmunity(SpellImmunityMask immunityMask) const
{
    return m_spellImmunityMask & immunityMask;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Aura

void Unit::addAura(std::unique_ptr<Aura> auraHolder)
{
    if (auraHolder == nullptr)
        return;

    if (!isAlive() && !auraHolder->getSpellInfo()->isDeathPersistent())
        return;

    // Check school immunity
    const auto school = auraHolder->getSpellInfo()->getFirstSchoolFromSchoolMask();
    if (school != SCHOOL_NORMAL && m_schoolImmunityList[school] && auraHolder->getCasterGuid() != getGuid())
    {
        ///\ todo: notify client that aura did not apply
        return;
    }

    // Check if aura has effects
    if (auraHolder->getAppliedEffectCount() == 0)
    {
        ///\ todo: notify client that aura did not apply
        return;
    }

    // Check for flying mount
    // This is already checked in Spell::canCast but this could happen on teleport or login
    if (isPlayer() && auraHolder->getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_ONLY_IN_OUTLANDS)
    {
        if (!dynamic_cast<Player*>(this)->canUseFlyingMountHere())
        {
            if (GetMapId() != 571 || !(auraHolder->getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING))
                return;
        }
    }

    // Check for single target aura
    ///\ todo: this supports only single auras. Missing paladin seals, warlock curses etc
    if (auraHolder->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA)
    {
        uint64_t previousTargetGuid = 0;

        const auto caster = auraHolder->GetUnitCaster();
        if (caster != nullptr)
        {
            previousTargetGuid = caster->getSingleTargetGuidForAura(auraHolder->getSpellId());

            // Check if aura is applied on different unit
            if (previousTargetGuid != 0 && previousTargetGuid != auraHolder->getOwner()->getGuid())
            {
                const auto previousTarget = getWorldMapUnit(previousTargetGuid);
                if (previousTarget != nullptr)
                    previousTarget->removeAllAurasByIdForGuid(auraHolder->getSpellId(), caster->getGuid());
            }
        }
    }

    const auto spellInfo = auraHolder->getSpellInfo();
    uint16_t auraSlot = 0xFFFF;

    if (!auraHolder->IsPassive())
    {
        uint16_t startLimit = 0, endLimit = 0;
        if (auraHolder->isNegative())
        {
            startLimit = AuraSlots::NEGATIVE_SLOT_START;
            endLimit = AuraSlots::NEGATIVE_SLOT_END;
        }
        else
        {
            startLimit = AuraSlots::POSITIVE_SLOT_START;
            endLimit = AuraSlots::POSITIVE_SLOT_END;
        }

        auto deleteAur = false;

        // Find available slot for new aura
        for (auto i = startLimit; i < endLimit; ++i)
        {
            auto* _aura = m_auraList[i].get();
            if (_aura == nullptr)
            {
                // Found an empty slot
                if (auraSlot == 0xFFFF)
                    auraSlot = i;

                // Do not break here, check if unit has any similiar aura
                continue;
            }

            // Check if unit has same aura by same caster or is stackable from multiple casters
            if (_aura->getSpellId() == auraHolder->getSpellId())
            {
                if (_aura->getCasterGuid() != auraHolder->getCasterGuid() &&
                    !auraHolder->getSpellInfo()->isStackableFromMultipleCasters())
                    continue;

                // The auras are casted by same unit or aura is stackable from multiple units, reapply all effects
                // Old aura will never have more effects than new aura and all effects have same indexes
                // but old aura can have less effects if certain effects have been removed by i.e. pvp trinket
                for (uint8_t x = 0; x < MAX_SPELL_EFFECTS; ++x)
                {
                    _aura->removeAuraEffect(x, true);

                    // Do not add empty effects
                    if (auraHolder->getAuraEffect(x)->getAuraEffectType() == SPELL_AURA_NONE)
                        continue;

                    _aura->addAuraEffect(auraHolder->getAuraEffect(x), true);
                }

                // On reapply get duration from new aura
                _aura->setOriginalDuration(auraHolder->getOriginalDuration());

                // Refresh duration and apply new stack if stackable
                _aura->refreshOrModifyStack(false, 1);

                deleteAur = true;
                break;
            }
            // If this is a proc spell, it should not remove its mother aura
            else if (auraHolder->pSpellId != _aura->getSpellId())
            {
                // Check for auras by specific type
                if (auraHolder->getSpellInfo()->getMaxstack() == 0 && spellInfo->custom_BGR_one_buff_on_target > 0 && auraHolder->getSpellInfo()->custom_BGR_one_buff_on_target & spellInfo->custom_BGR_one_buff_on_target)
                {
                    deleteAur = hasAuraWithSpellType(static_cast<SpellTypes>(spellInfo->getCustom_BGR_one_buff_on_target()), auraHolder->getCasterGuid(), 0);
                }
                // Check for auras with the same name and a different rank
                else
                {
                    AuraCheckResponse checkResponse = auraCheck(spellInfo, _aura, auraHolder->getCaster());
                    if (checkResponse.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                    {
                        // Existing aura is stronger, delete new aura
                        deleteAur = true;
                        break;
                    }
                    else if (checkResponse.Error == AURA_CHECK_RESULT_LOWER_BUFF_PRESENT)
                    {
                        // Remove the weaker aura
                        _aura->removeAura();
                        // Restart search
                        i = startLimit;
                        continue;
                    }
                }
            }
        }

        if (deleteAur)
            return;
    }
    else
    {
        // Passive spells always apply
        ///\ todo: probably should add check for passive aura stacking
        for (uint16_t i = AuraSlots::PASSIVE_SLOT_START; i < AuraSlots::PASSIVE_SLOT_END; ++i)
        {
            if (m_auraList[i] == nullptr)
            {
                auraSlot = i;
                break;
            }
        }
    }

    // Could not find an empty slot, remove aura
    if (auraSlot == 0xFFFF)
        return;

    // Find a visual slot for aura
    const auto visualSlot = findVisualSlotForAura(auraHolder.get());

    auraHolder->m_visualSlot = visualSlot;
    auraHolder->setAuraSlot(auraSlot);

    auto* aur = auraHolder.get();
    _addAura(std::move(auraHolder));

    if (visualSlot < AuraSlots::NEGATIVE_VISUAL_SLOT_END)
    {
        m_auraVisualList[visualSlot] = aur->getSpellId();

#if VERSION_STRING < WotLK
        setAura(aur, true);
        setAuraFlags(aur, true);
        setAuraLevel(aur);
        setAuraApplication(aur);
#endif

        // Send packet
        sendAuraUpdate(aur, false);
        updateAuraForGroup(visualSlot);
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
    if (aur->getSpellInfo()->getMechanicsType() == MECHANIC_ENRAGED && !m_ascEnraged++)
        addAuraStateAndAuras(AURASTATE_FLAG_ENRAGED);
    else if (aur->getSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING && !m_ascBleed++)
        addAuraStateAndAuras(AURASTATE_FLAG_BLEED);
    if (aur->getSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !m_ascSeal++)
        addAuraStateAndAuras(AURASTATE_FLAG_JUDGEMENT);

}

uint8_t Unit::findVisualSlotForAura(Aura const* aur) const
{
    uint8_t visualSlot = 0xFF;
#if VERSION_STRING < WotLK
    // Pre wotlk do not send self casted passive area auras
    if (aur->IsPassive())
#else
    // Since wotlk send all passive area auras
    if (aur->IsPassive() && !aur->IsAreaAura() && !aur->hasAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
#endif
        return visualSlot;

    uint8_t start, end;
    if (!aur->isNegative())
    {
        start = AuraSlots::POSITIVE_VISUAL_SLOT_START;
        end = AuraSlots::POSITIVE_VISUAL_SLOT_END;
    }
    else
    {
        start = AuraSlots::NEGATIVE_VISUAL_SLOT_START;
        end = AuraSlots::NEGATIVE_VISUAL_SLOT_END;
    }

    // Find an empty slot
    for (auto i = start; i < end; ++i)
    {
        if (m_auraVisualList[i] == 0)
        {
            visualSlot = i;
            break;
        }
    }

    return visualSlot;
}

Aura* Unit::getAuraWithId(uint32_t spell_id) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur && aur->getSpellId() == spell_id)
            return aur.get();
    }

    return nullptr;
}

Aura* Unit::getAuraWithId(uint32_t const* auraId) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr)
            continue;

        for (int i = 0; auraId[i] != 0; ++i)
        {
            if (aur->getSpellId() == auraId[i])
                return aur.get();
        }
    }

    return nullptr;
}

Aura* Unit::getAuraWithIdForGuid(uint32_t const* auraId, uint64_t guid) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr || aur->getCasterGuid() != guid)
            continue;

        for (int i = 0; auraId[i] != 0; ++i)
        {
            if (aur->getSpellId() == auraId[i])
                return aur.get();
        }
    }

    return nullptr;
}

Aura* Unit::getAuraWithIdForGuid(uint32_t spell_id, uint64_t target_guid) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur && aur->getSpellId() == spell_id && aur->getCasterGuid() == target_guid)
            return aur.get();
    }

    return nullptr;
}

Aura* Unit::getAuraWithAuraEffect(AuraEffect aura_effect) const
{
    if (aura_effect >= TOTAL_SPELL_AURAS)
        return nullptr;

    if (getAuraEffectList(aura_effect).empty())
        return nullptr;

    return getAuraEffectList(aura_effect).front()->getAura();
}

Aura* Unit::getAuraWithAuraEffectForGuid(AuraEffect aura_effect, uint64_t guid) const
{
    if (aura_effect >= TOTAL_SPELL_AURAS)
        return nullptr;

    if (m_auraEffectList[aura_effect].empty())
        return nullptr;

    for (const auto& aurEff : m_auraEffectList[aura_effect])
    {
        if (aurEff->getAura()->getCasterGuid() == guid)
            return aurEff->getAura();
    }

    return nullptr;
}

Aura* Unit::getAuraWithVisualSlot(uint8_t visualSlot) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur && aur->m_visualSlot == visualSlot)
            return aur.get();
    }

    return nullptr;
}

Aura* Unit::getAuraWithAuraSlot(uint16_t auraSlot) const
{
    if (auraSlot >= AuraSlots::TOTAL_SLOT_END)
        return nullptr;

    return m_auraList[auraSlot].get();
}

int32_t Unit::getTotalIntDamageForAuraEffect(AuraEffect aura_effect) const
{
    int32_t totalDamage = 0;
    for (const auto& aurEff : getAuraEffectList(aura_effect))
        totalDamage += aurEff->getEffectDamage();

    return totalDamage;
}

int32_t Unit::getTotalIntDamageForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const
{
    int32_t totalDamage = 0;
    for (const auto& aurEff : getAuraEffectList(aura_effect))
    {
        if (aurEff->getEffectMiscValue() == miscValue)
            totalDamage += aurEff->getEffectDamage();
    }

    return totalDamage;
}

float_t Unit::getTotalFloatDamageForAuraEffect(AuraEffect aura_effect) const
{
    float_t totalDamage = 0.0f;
    for (const auto& aurEff : getAuraEffectList(aura_effect))
        totalDamage += aurEff->getEffectFloatDamage();

    return totalDamage;
}

float_t Unit::getTotalFloatDamageForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const
{
    float_t totalDamage = 0.0f;
    for (const auto& aurEff : getAuraEffectList(aura_effect))
    {
        if (aurEff->getEffectMiscValue() == miscValue)
            totalDamage += aurEff->getEffectFloatDamage();
    }

    return totalDamage;
}

float_t Unit::getTotalPctMultiplierForAuraEffect(AuraEffect aura_effect) const
{
    return 1.0f + (getTotalFloatDamageForAuraEffect(aura_effect) / 100.0f);
}

float_t Unit::getTotalPctMultiplierForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const
{
    return 1.0f + (getTotalFloatDamageForAuraEffectByMiscValue(aura_effect, miscValue) / 100.0f);
}

bool Unit::hasAurasWithId(uint32_t auraId) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur && aur->getSpellId() == auraId)
            return true;
    }

    return false;
}

bool Unit::hasAurasWithId(uint32_t const* auraId) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr)
            continue;

        for (int i = 0; auraId[i] != 0; ++i)
        {
            if (aur->getSpellId() == auraId[i])
                return true;
        }
    }

    return false;
}

bool Unit::hasAurasWithIdForGuid(uint32_t auraId, uint64_t guid) const
{
    for (const auto& aur : m_auraList)
    {
        if (aur && aur->getSpellId() == auraId && aur->getCasterGuid() == guid)
            return true;
    }

    return false;
}

bool Unit::hasAurasWithIdForGuid(uint32_t const* auraId, uint64_t guid) const
{
    for (const auto& aur : m_auraList)
    {
        if (aur == nullptr)
            continue;

        for (int i = 0; auraId[i] != 0; ++i)
        {
            if (aur->getSpellId() == auraId[i] && aur->getCasterGuid() == guid)
                return true;
        }
    }

    return false;
}

bool Unit::hasAuraWithAuraEffect(AuraEffect type) const
{
    if (type >= TOTAL_SPELL_AURAS)
        return false;

    return !getAuraEffectList(type).empty();
}

bool Unit::hasAuraWithAuraEffectForGuid(AuraEffect type, uint64_t guid) const
{
    if (type >= TOTAL_SPELL_AURAS)
        return false;

    for (const auto& aurEff : m_auraEffectList[type])
    {
        if (aurEff->getAura()->getCasterGuid() == guid)
            return true;
    }

    return false;
}

bool Unit::hasAuraWithMechanic(SpellMechanic mechanic) const
{
    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr)
            continue;

        if (aur->getSpellInfo()->getMechanicsType() == mechanic)
        {
            return true;
        }
        else
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (aur->getAuraEffect(i)->getAuraEffectType() == SPELL_AURA_NONE)
                    continue;

                if (aur->getSpellInfo()->getEffectMechanic(i) == mechanic)
                    return true;
            }
        }
    }

    return false;
}

bool Unit::hasAuraWithSpellType(SpellTypes type, uint64_t casterGuid/* = 0*/, uint32_t skipSpellId/* = 0*/) const
{
    const uint64_t sGuid = type == SPELL_TYPE_BLESSING || type == SPELL_TYPE_WARRIOR_SHOUT ? casterGuid : 0;
    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr)
            continue;

        if (skipSpellId != 0 && aur->getSpellId() == skipSpellId)
            continue;

        if (!(aur->getSpellInfo()->custom_BGR_one_buff_on_target & type))
            continue;

        if (sGuid == 0 || aur->getCasterGuid() == sGuid)
            return true;
    }

    return false;
}

bool Unit::hasAuraState(AuraState state, SpellInfo const* spellInfo, Unit const* caster) const
{
#if VERSION_STRING >= WotLK
    if (caster != nullptr && spellInfo != nullptr && caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
    {
        const auto& auraStateList = caster->getAuraEffectList(SPELL_AURA_IGNORE_TARGET_AURA_STATE);
        for (const auto& aurEff : auraStateList)
        {
            if (aurEff->getAura()->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_IGNORE_TARGET_AURA_STATE, spellInfo))
                return true;
        }
    }
#endif

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
            const auto player = dynamic_cast<Player*>(this);
            for (const auto& spellId : player->getSpellSet())
            {
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

        // Remove self-applied auras requiring this aurastate
        uint16_t startLimit = AuraSlots::TOTAL_SLOT_START;
        uint16_t endLimit = AuraSlots::TOTAL_SLOT_END;
        // Do not remove non-passive enrage effects
        if (state == AURASTATE_FLAG_ENRAGED)
        {
            startLimit = AuraSlots::PASSIVE_SLOT_START;
            endLimit = AuraSlots::PASSIVE_SLOT_END;
        }

        for (auto i = startLimit; i < endLimit; ++i)
        {
            auto* const aur = getAuraWithAuraSlot(i);
            if (aur == nullptr)
                continue;
            if (aur->getCasterGuid() != getGuid())
                continue;
            if (aur->getSpellInfo()->getCasterAuraState() != static_cast<uint32_t>(state))
                continue;

            aur->removeAura();
        }
    }
}

uint32_t Unit::getAuraCountForId(uint32_t auraId) const
{
    uint32_t auraCount = 0;

    for (const auto& aur : getAuraList())
    {
        if (aur && aur->getSpellId() == auraId)
            ++auraCount;
    }

    return auraCount;
}

uint32_t Unit::getAuraCountForEffect(AuraEffect aura_effect) const
{
    if (aura_effect >= TOTAL_SPELL_AURAS)
        return 0;

    return static_cast<uint32_t>(getAuraEffectList(aura_effect).size());
}

uint32_t Unit::getAuraCountWithDispelType(DispelType type, uint64_t casterGuid/* = 0*/) const
{
    uint32_t auraCount = 0;

    for (const auto& aur : getAuraList())
    {
        if (aur == nullptr)
            continue;

        if (casterGuid != 0 && aur->getCasterGuid() != casterGuid)
            continue;

        if (aur->getSpellInfo()->getDispelType() == type)
            ++auraCount;
    }

    return auraCount;
}

void Unit::removeAllAuras()
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        if (auto* const aur = getAuraWithAuraSlot(i))
            aur->removeAura();
    }
}

void Unit::removeAllAurasById(uint32_t auraId, AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur && aur->getSpellId() == auraId)
            aur->removeAura(mode);
    }
}

void Unit::removeAllAurasById(uint32_t const* auraId, AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        for (int x = 0; auraId[x] != 0; ++x)
        {
            if (aur->getSpellId() == auraId[x])
                aur->removeAura(mode);
        }
    }
}

void Unit::removeAllAurasByIdForGuid(uint32_t auraId, uint64_t guid, AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        if (guid != 0 && aur->getCasterGuid() != guid)
            continue;

        if (aur->getSpellId() == auraId)
            aur->removeAura(mode);
    }
}

void Unit::removeAllAurasByAuraInterruptFlag(uint32_t auraInterruptFlag, uint32_t skipSpellId/* = 0*/)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        if (skipSpellId != 0 && aur->getSpellId() == skipSpellId)
            continue;

        if (aur->getSpellInfo()->getAuraInterruptFlags() & auraInterruptFlag)
            aur->removeAura();
    }
}

void Unit::removeAllAurasByAuraEffect(AuraEffect effect, uint32_t skipSpell/* = 0*/, bool removeOnlyEffect/* = false*/, uint64_t casterGuid/* = 0*/, AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    if (!hasAuraWithAuraEffect(effect))
        return;

    const auto& aurEffList = getAuraEffectList(effect);
    for (auto itr = aurEffList.cbegin(); itr != aurEffList.cend();)
    {
        const auto aurEff = *itr;
        auto* const aur = aurEff->getAura();
        ++itr;

        if (skipSpell == aur->getSpellId())
            continue;

        if (casterGuid != 0 && aur->getCasterGuid() != casterGuid)
            continue;

        if (removeOnlyEffect)
            aur->removeAuraEffect(aurEff->getEffectIndex());
        else
            aur->removeAura(mode);
    }
}

void Unit::removeAllAurasBySpellMechanic(SpellMechanic mechanic, bool negativeOnly/* = true*/)
{
    const uint16_t start = negativeOnly ? AuraSlots::NEGATIVE_SLOT_START : AuraSlots::TOTAL_SLOT_START;
    const uint16_t end = negativeOnly ? AuraSlots::NEGATIVE_SLOT_END : AuraSlots::TOTAL_SLOT_END;

    for (auto i = start; i < end; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        if (aur->getSpellInfo()->getMechanicsType() == mechanic)
        {
            aur->removeAura();
        }
        else
        {
            for (uint8_t x = 0; x < MAX_SPELL_EFFECTS; ++x)
            {
                if (aur->getAuraEffect(x)->getAuraEffectType() == SPELL_AURA_NONE)
                    continue;

                // Remove only aura effect in this case
                if (aur->getSpellInfo()->getEffectMechanic(x) == mechanic)
                    aur->removeAuraEffect(x);
            }
        }
    }
}

void Unit::removeAllAurasBySpellMechanic(SpellMechanic const* mechanic, bool negativeOnly/* = true*/)
{
    const uint16_t start = negativeOnly ? AuraSlots::NEGATIVE_SLOT_START : AuraSlots::TOTAL_SLOT_START;
    const uint16_t end = negativeOnly ? AuraSlots::NEGATIVE_SLOT_END : AuraSlots::TOTAL_SLOT_END;

    for (auto i = start; i < end; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        for (int x = 0; mechanic[x] != MECHANIC_NONE; ++x)
        {
            if (aur->getSpellInfo()->getMechanicsType() == mechanic[x])
            {
                aur->removeAura();
            }
            else
            {
                for (uint8_t u = 0; u < MAX_SPELL_EFFECTS; ++u)
                {
                    if (aur->getAuraEffect(u)->getAuraEffectType() == SPELL_AURA_NONE)
                        continue;

                    // Remove only aura effect in this case
                    if (aur->getSpellInfo()->getEffectMechanic(u) == mechanic[x])
                        aur->removeAuraEffect(u);
                }
            }
        }
    }
}

void Unit::removeAllAurasBySpellType(SpellTypes type, uint64_t casterGuid/* = 0*/, uint32_t skipSpellId/* = 0*/)
{
    const uint64_t sGuid = type >= SPELL_TYPE_BLESSING ? casterGuid : 0;
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        if (skipSpellId != 0 && aur->getSpellId() == skipSpellId)
            continue;

        if (!(aur->getSpellInfo()->custom_BGR_one_buff_on_target & type))
            continue;

        if (sGuid == 0 || aur->getCasterGuid() == sGuid)
            aur->removeAura();
    }
}

void Unit::removeAllAurasBySchoolMask(SchoolMask schoolMask, bool negativeOnly/* = true*/, bool isImmune/* = false*/)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        if (negativeOnly && !aur->isNegative())
            continue;

        if (!(aur->getSpellInfo()->getSchoolMask() & schoolMask))
            continue;

        if (!isImmune && aur->getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY)
            aur->removeAura();
    }
}

void Unit::removeAllNegativeAuras()
{
    for (uint16_t i = AuraSlots::NEGATIVE_SLOT_START; i < AuraSlots::NEGATIVE_SLOT_END; ++i)
    {
        if (auto* const aur = getAuraWithAuraSlot(i))
            aur->removeAura();
    }
}

void Unit::removeAllPositiveAuras()
{
    for (uint16_t i = AuraSlots::POSITIVE_SLOT_START; i < AuraSlots::POSITIVE_SLOT_END; ++i)
    {
        if (auto* const aur = getAuraWithAuraSlot(i))
            aur->removeAura();
    }
}

void Unit::removeAllNonPersistentAuras()
{
    for (uint16_t i = AuraSlots::REMOVABLE_SLOT_START; i < AuraSlots::REMOVABLE_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur && !aur->getSpellInfo()->isDeathPersistent())
            aur->removeAura();
    }
}

void Unit::removeAuraByItemGuid(uint32_t auraId, uint64_t itemGuid)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur && aur->getSpellId() == auraId && aur->itemCasterGUID == itemGuid)
            aur->removeAura();
    }
}

uint32_t Unit::removeAllAurasByIdReturnCount(uint32_t auraId, AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    uint32_t res = 0;
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur && aur->getSpellInfo()->getId() == auraId)
        {
            aur->removeAura(mode);
            ++res;
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

uint64_t Unit::getSingleTargetGuidForAura(uint32_t const* spellIds, uint32_t* index)
{
    for (uint8_t i = 0; ; i++)
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

void Unit::clearAllAreaAuraTargets()
{
    for (const auto& aur : getAuraList())
    {
        // Aura is area aura but it was not casted by this unit
        if (aur == nullptr || aur->m_areaAura)
            continue;

        if (aur->IsAreaAura())
            aur->ClearAATargets();
    }
}

void Unit::removeAllAreaAurasCastedByOther()
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        auto* const aur = getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        // Not area aura, or is area aura casted by this unit
        if (!aur->m_areaAura)
            continue;

        aur->removeAura();
    }
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
    if (aur->m_visualSlot >= AuraSlots::NEGATIVE_VISUAL_SLOT_END)
        return;

#if VERSION_STRING < WotLK
    if (!remove)
    {
#if VERSION_STRING == Classic
        if (isPlayer() && !aur->IsPassive())
            static_cast<Player*>(this)->sendMessageToSet(SmsgUpdateAuraDuration(aur->m_visualSlot, aur->getTimeLeft()).serialise().get(), true);
#else

        if (isPlayer() && !aur->IsPassive() && !(aur->getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_DURATION))
        {
            static_cast<Player*>(this)->sendMessageToSet(SmsgUpdateAuraDuration(aur->m_visualSlot, aur->getTimeLeft()).serialise().get(), true);

            auto guid = GetNewGUID();
            static_cast<Player*>(this)->sendMessageToSet(SmsgSetExtraAuraInfo(guid, aur->m_visualSlot, aur->getSpellId(), aur->getMaxDuration(), aur->getTimeLeft()).serialise().get(), true);
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
            caster->sendMessageToSet(&data, true);
        }
#endif
    }
#if VERSION_STRING == TBC
    else
    {
        const auto caster = aur->GetUnitCaster();
        if (caster != nullptr && caster->isPlayer())
            static_cast<Player*>(caster)->sendMessageToSet(SmsgClearExtraAuraInfo(getGuid(), aur->getSpellId()).serialise().get(), true);
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
            if (aur->getAuraEffect(i)->getAuraEffectType() != SPELL_AURA_NONE)
                auraUpdate.effAmount[i] = aur->getAuraEffect(i)->getEffectDamage();
            else
                auraUpdate.effAmount[i] = 0;
        }
    }
#endif

    sendMessageToSet(SmsgAuraUpdate(getGuid(), auraUpdate, remove).serialise().get(), true);
#endif
}

void Unit::sendFullAuraUpdate()
{
#if VERSION_STRING >= WotLK
    auto packetData = SmsgAuraUpdateAll(getGuid(), {});
    auto updates = 0u;

    for (const auto& aur : getAuraList())
    {
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
                if (aur->getAuraEffect(x)->getAuraEffectType() != SPELL_AURA_NONE)
                    auraUpdate.effAmount[x] = aur->getAuraEffect(x)->getEffectDamage();
                else
                    auraUpdate.effAmount[x] = 0;
            }
        }
#endif

        packetData.addAuraUpdate(auraUpdate);
        ++updates;
    }

    sendMessageToSet(packetData.serialise().get(), true);
    sLogger.debug("Unit::sendFullAuraUpdate : Updated {} auras for guid {}", updates, getGuid());
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

    sendMessageToSet(SmsgPeriodicAuraLog(targetGuid, casterGuid, spellInfo->getId(), auraEffect, amount, overKillOrOverHeal, school, absorbed, resisted, isCritical, miscValue, gainMultiplier).serialise().get(), true);
    return true;
}

AuraArray const& Unit::getAuraList() const
{
    return m_auraList;
}

AuraEffectList const& Unit::getAuraEffectList(AuraEffect effect) const
{
    return m_auraEffectList[effect];
}

VisualAuraArray const& Unit::getVisualAuraList() const
{
    return m_auraVisualList;
}

bool Unit::isPoisoned()
{
    for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
    {
        const auto* aur = getAuraWithAuraSlot(x);
        if (aur && aur->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_POISON)
            return true;
    }

    return false;
}

bool Unit::isDazed() const
{
    for (const auto& aur : getAuraList())
    {
        if (aur)
        {
            if (aur->getSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED)
                return true;

            for (uint8_t y = 0; y < 3;++y)
                if (aur->getSpellInfo()->getEffectMechanic(y) == MECHANIC_ENSNARED)
                    return true;
        }
    }

    return false;
}

void Unit::_addAura(std::unique_ptr<Aura> aur)
{
    if (aur == nullptr)
        return;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        const auto aurEff = aur->getAuraEffect(i);
        if (aurEff->getAuraEffectType() == SPELL_AURA_NONE)
            continue;

        _addAuraEffect(aurEff);
    }

    m_auraList[aur->getAuraSlot()] = std::move(aur);
}

void Unit::_addAuraEffect(AuraEffectModifier const* aurEff)
{
    if (aurEff == nullptr)
        return;

    m_auraEffectList[aurEff->getAuraEffectType()].push_back(aurEff);
}

std::unique_ptr<Aura> Unit::_removeAura(Aura* aur)
{
    if (aur == nullptr)
        return nullptr;

    auto&& tmp = std::move(m_auraList[aur->getAuraSlot()]);
    return tmp;
}

void Unit::_removeAuraEffect(AuraEffectModifier const* aurEff)
{
    if (aurEff == nullptr)
        return;

    m_auraEffectList[aurEff->getAuraEffectType()].remove(aurEff);
}

void Unit::_updateAuras(unsigned long diff)
{
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        if (auto* const aur = getAuraWithAuraSlot(i))
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
    // should cover all Instances with 5000 * 5000 easyier for far Gameobjects / Creatures to Handle, also Loaded Cells affect the Distance standart 2 Cells equal 500.0f * 500.0f : aaron02
    const auto viewDistance = getWorldMap()->getBaseMap()->isInstanceMap() ? 5000.0f * 5000.0f : getWorldMap()->getVisibilityRange();
    if (obj->isGameObject())
    {
        // TODO: for now, all maps have 500 yard view distance
        // problem is that objects on active map cells are updated only if player can see it, iirc

        // Transports and Destructible Buildings should always be visible
        const auto gobj = dynamic_cast<GameObject*>(obj);
        if (gobj->getGoType() == GAMEOBJECT_TYPE_TRANSPORT || gobj->getGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT || gobj->getGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
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
        if (!isInRange(obj->GetPosition(), viewDistance))
            return false;
    }

    // Unit cannot see invisible Game Masters unless he/she has Game Master flag on
    if (obj->isPlayer() && dynamic_cast<Player*>(obj)->m_isGmInvisible)
        return isPlayer() && dynamic_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM);

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
        // Player should see all default spawned gameobjects when dead
        if (obj->isGameObject() && obj->getPlayerOwner() == nullptr)
            return true;

        const float_t corpseViewDistance = 1600.0f; // 40*40 yards
        const auto playerMe = dynamic_cast<Player*>(this);
        // If object is another player
        if (obj->isPlayer())
        {
            // Dead player can see all players in arena regardless of range
            if (playerMe->m_deathVision)
                return true;

            // Player can see all friendly and unfriendly players within 40 yards from his/her corpse
            const auto playerObj = dynamic_cast<Player*>(obj);
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
            if (obj->isCorpse() && dynamic_cast<Corpse*>(obj)->getOwnerGuid() == getGuid())
                return true;

            // Player can see all objects within 40 yards from his/her own corpse
            if (obj->isInRange(playerMe->getCorpseLocation(), corpseViewDistance))
                return true;
        }

        // Player can see Spirit Healers
        if (obj->isCreature() && dynamic_cast<Creature*>(obj)->isSpiritHealer())
            return true;

        return false;
    }

    // Unit is alive or player hasn't released spirit yet
    // Do checks based on object's type
    switch (obj->getObjectTypeId())
    {
        case TYPEID_PLAYER:
        {
            const auto playerObj = dynamic_cast<Player*>(obj);
            if (playerObj->getDeathState() == CORPSE)
            {
                if (isPlayer())
                {
                    // If players are from same group, they can see each other normally
                    const auto playerMe = dynamic_cast<Player*>(this);
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
            if (obj->isCreature() && dynamic_cast<Creature*>(obj)->isSpiritHealer())
                return isPlayer() && dynamic_cast<Player*>(this)->hasPlayerFlags(PLAYER_FLAG_GM);

            const auto unitObj = dynamic_cast<Unit*>(obj);

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
                const auto objectOwner = getWorldMapPlayer(ownerGuid);
                if (objectOwner != nullptr)
                {
                    if (objectOwner->getGroup() && objectOwner->getGroup()->HasMember(dynamic_cast<Player*>(this)))
                    {
                        if (objectOwner->getDuelPlayer() != dynamic_cast<Player*>(this))
                            return true;
                    }
                }

                // If object is only visible to either faction
                if (unitObj->getAIInterface()->faction_visibility == 1)
                    return dynamic_cast<Player*>(this)->isTeamHorde() ? true : false;
                if (unitObj->getAIInterface()->faction_visibility == 2)
                    return dynamic_cast<Player*>(this)->isTeamHorde() ? false : true;
            }
        } break;
        case TYPEID_GAMEOBJECT:
        {
            const auto gameObjectObj = dynamic_cast<GameObject*>(obj);
            // Stealthed / invisible gameobjects
            if (gameObjectObj->inStealth || gameObjectObj->invisible)
            {
                ownerGuid = gameObjectObj->getCreatedByGuid();
                // Unit can always see their own created gameobjects
                if (getGuid() == ownerGuid)
                    return true;

                // Group members can see each other's created gameobjects
                // unless they are dueling, then it's based on detection
                const auto objectOwner = getWorldMapPlayer(ownerGuid);
                if (objectOwner != nullptr && isPlayer())
                {
                    if (objectOwner->getGroup() && objectOwner->getGroup()->HasMember(dynamic_cast<Player*>(this)))
                    {
                        if (objectOwner->getDuelPlayer() != dynamic_cast<Player*>(this))
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
    if (obj->isCreatureOrPlayer() && dynamic_cast<Unit*>(obj)->m_stalkedByGuid == getGuid())
        return true;

    // Pets and summoned units don't have detection, they rely on their master's detection
    auto meUnit = this;
    if (getCharmedByGuid() != 0)
    {
        const auto summoner = getWorldMapUnit(getCharmedByGuid());
        if (summoner != nullptr)
            meUnit = summoner;
    }
    else if (getSummonedByGuid() != 0)
    {
        const auto summoner = getWorldMapUnit(getSummonedByGuid());
        if (summoner != nullptr)
            meUnit = summoner;
    }

    const auto unitTarget = dynamic_cast<Unit*>(obj);
    const auto gobTarget = dynamic_cast<GameObject*>(obj);

    ////////////////////////////
    // Invisibility detection

    if (obj->isCreatureOrPlayer())
    {
        // Players should never see these types of invisible units
        // Creatures need to be able to see them so invisible triggers can cast spells on visible targets
        if (meUnit->isPlayer() && unitTarget->getInvisibilityLevel(INVIS_FLAG_NEVER_VISIBLE) > 0)
            return false;
    }

    for (uint8_t i = 0; i < INVIS_FLAG_TOTAL; ++i)
    {
        if (i == INVIS_FLAG_NEVER_VISIBLE)
            continue;

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
#if VERSION_STRING >= TBC
            // Shadow Sight buff in arena makes unit detect stealth regardless of distance and facing
            if (meUnit->hasAuraWithAuraEffect(SPELL_AURA_DETECT_STEALTH))
                return true;
#endif

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
                const auto summoner = gobTarget->getWorldMapUnit(gobTarget->getCreatedByGuid());
                if (summoner != nullptr)
                    detectionValue -= summoner->getLevel() * 5;
            }
            else
                // If trap has no owner, subtract trap's level from detection value
                detectionValue -= gobTarget->GetGameObjectProperties()->trap.level * 5;
        }

        auto visibilityRange = detectionValue * 0.3f + combatReach;
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
    updateVisibility();
}

void Unit::updateVisibility()
{
    ByteBuffer buf(3000);
    uint32_t count;
    bool canSee;
    bool isVisible;

    if (isPlayer())
    {
        Player* player = dynamic_cast<Player*>(this);
        for (const auto& inRangeObject : getInRangeObjectsSet())
        {
            if (inRangeObject)
            {
                canSee = player->canSee(inRangeObject);
                isVisible = player->isVisibleObject(inRangeObject->getGuid());
                if (canSee)
                {
                    if (!isVisible)
                    {
                        buf.clear();
                        count = inRangeObject->buildCreateUpdateBlockForPlayer(&buf, player);
                        player->getUpdateMgr().pushCreationData(&buf, count);
                        player->addVisibleObject(inRangeObject->getGuid());
                    }
                }
                else
                {
                    if (isVisible)
                    {
                        player->sendDestroyObjectPacket(inRangeObject->getGuid());
                        player->removeVisibleObject(inRangeObject->getGuid());
                    }
                }

                if (inRangeObject->isPlayer())
                {
                    Player* inRangePlayer = dynamic_cast<Player*>(inRangeObject);
                    canSee = inRangePlayer->canSee(player);
                    isVisible = inRangePlayer->isVisibleObject(player->getGuid());
                    if (canSee)
                    {
                        if (!isVisible)
                        {
                            buf.clear();
                            count = player->buildCreateUpdateBlockForPlayer(&buf, inRangePlayer);
                            inRangePlayer->getUpdateMgr().pushCreationData(&buf, count);
                            inRangePlayer->addVisibleObject(player->getGuid());
                        }
                    }
                    else
                    {
                        if (isVisible)
                        {
                            inRangePlayer->sendDestroyObjectPacket(player->getGuid());
                            inRangePlayer->removeVisibleObject(player->getGuid());
                        }
                    }
                }
                else if (inRangeObject->isCreature() && player->getSession() && player->getSession()->HasGMPermissions())
                {
                    auto* const inRangeCreature = dynamic_cast<Creature*>(inRangeObject);

                    uint32_t fieldIds[] =
                    {
                        // Update unit flags to remove not selectable flag
                        getOffsetForStructuredField(WoWUnit, unit_flags),
                        // Placeholder if creature is a trigger npc
                        0,
                        0
                    };

                    // Update trigger model
                    if (inRangeCreature->GetCreatureProperties()->isTriggerNpc)
                        fieldIds[1] = getOffsetForStructuredField(WoWUnit, display_id);

                    inRangeCreature->forceBuildUpdateValueForFields(fieldIds, player);
                }
            }
        }
    }
    else // For units we can save a lot of work
    {
        for (const auto& inRangeObject : getInRangePlayersSet())
        {
            if (Player* inRangePlayer = dynamic_cast<Player*>(inRangeObject))
            {
                canSee = inRangePlayer->canSee(this);
                isVisible = inRangePlayer->isVisibleObject(this->getGuid());
                if (!canSee)
                {
                    if (isVisible)
                    {
                        inRangePlayer->sendDestroyObjectPacket(getGuid());
                        inRangePlayer->removeVisibleObject(getGuid());
                    }
                }
                else
                {
                    if (!isVisible)
                    {
                        buf.clear();
                        count = buildCreateUpdateBlockForPlayer(&buf, inRangePlayer);
                        inRangePlayer->getUpdateMgr().pushCreationData(&buf, count);
                        inRangePlayer->addVisibleObject(this->getGuid());
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Power related
void Unit::regenerateHealthAndPowers(uint16_t timePassed)
{
    if (!isAlive())
        return;

    // Health
    m_healthRegenerateTimer += timePassed;
    if ((hasUnitStateFlag(UNIT_STATE_POLYMORPHED) && m_healthRegenerateTimer >= REGENERATION_INTERVAL_HEALTH_POLYMORPHED) ||
        (!hasUnitStateFlag(UNIT_STATE_POLYMORPHED) && m_healthRegenerateTimer >= REGENERATION_INTERVAL_HEALTH))
    {
        if (isPlayer())
        {
            dynamic_cast<Player*>(this)->regenerateHealth(getCombatHandler().isInCombat());
            m_healthRegenerateTimer = 0;
        }
        else
        {
            m_healthRegenerateTimer = 0;
            if (isCreature() && getNpcFlags() & UNIT_NPC_FLAG_DISABLE_REGEN)
                return;

            dynamic_cast<Creature*>(this)->RegenerateHealth();
        }
    }

#if VERSION_STRING < WotLK
    // Mana and Energy
    m_manaEnergyRegenerateTimer += timePassed;
    if (isPlayer())
    {
        if (m_manaEnergyRegenerateTimer >= REGENERATION_INTERVAL_MANA_ENERGY)
        {
            regeneratePower(POWER_TYPE_MANA, m_manaEnergyRegenerateTimer);
            regeneratePower(POWER_TYPE_ENERGY, m_manaEnergyRegenerateTimer);
            m_manaEnergyRegenerateTimer = 0;
        }
    }
    else
    {
        if (m_manaEnergyRegenerateTimer >= CREATURE_REGENERATION_INTERVAL_MANA_ENERGY)
        {
            if (!(isCreature() && getNpcFlags() & UNIT_NPC_FLAG_DISABLE_PWREGEN))
            {
                regeneratePower(POWER_TYPE_MANA, m_manaEnergyRegenerateTimer);
                regeneratePower(POWER_TYPE_ENERGY, m_manaEnergyRegenerateTimer);
            }
            m_manaEnergyRegenerateTimer = 0;
        }
    }

    // Focus
    m_focusRegenerateTimer += timePassed;
    if (m_focusRegenerateTimer >= REGENERATION_INTERVAL_FOCUS)
    {
        regeneratePower(POWER_TYPE_FOCUS, m_focusRegenerateTimer);
        m_focusRegenerateTimer = 0;
    }
#else
    m_powerUpdatePacketTime += timePassed;
    m_powerRegenerateTimer += timePassed;

    // Creatures that are not owned by players do not need to be updated in real time
    const auto updateInterval = getPlayerOwnerOrSelf() != nullptr
        ? REGENERATION_INTERVAL_POWER
        : REGENERATION_PACKET_UPDATE_INTERVAL;

    // Since wotlk most powers regenerate in real time
    if (m_powerRegenerateTimer >= updateInterval)
    {
        regeneratePower(POWER_TYPE_MANA, m_powerRegenerateTimer);
        regeneratePower(POWER_TYPE_ENERGY, m_powerRegenerateTimer);
        regeneratePower(POWER_TYPE_FOCUS, m_powerRegenerateTimer);
        if (isPlayer())
        {
            regeneratePower(POWER_TYPE_RAGE, m_powerRegenerateTimer);
            regeneratePower(POWER_TYPE_RUNIC_POWER, m_powerRegenerateTimer);
        }

        m_powerRegenerateTimer = 0;
        if (m_powerUpdatePacketTime >= REGENERATION_PACKET_UPDATE_INTERVAL)
            m_powerUpdatePacketTime = 0;
    }
#endif

    // Update player only resources
    if (isPlayer())
        dynamic_cast<Player*>(this)->regeneratePlayerPowers(timePassed);
}

void Unit::regeneratePower(PowerType type, uint16_t timePassed)
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

        if (this->m_interruptRegen)
            return;
    }

    const auto currentPower = getPower(type);
    const auto maxPower = getMaxPower(type);
    if (maxPower == 0)
        return;

    float_t amount = 0.0f;
    auto sendUpdatePacket = false;
    switch (type)
    {
        case POWER_TYPE_MANA:
        {
            if (isPlayer())
            {
#if VERSION_STRING < Cata
                // Check for 5 second regen interruption
                if (isPowerRegenerationInterrupted())
                    amount = getPowerRegenerationWhileInterrupted(POWER_TYPE_MANA);
                else
                    amount = getPowerRegeneration(POWER_TYPE_MANA);
#else
                // Check for combat (5 second rule was removed in cata)
                if (getCombatHandler().isInCombat())
                    amount = getPowerRegenerationWhileInterrupted(POWER_TYPE_MANA);
                else
                    amount = getPowerRegeneration(POWER_TYPE_MANA);
#endif
#if VERSION_STRING < WotLK
                // Send update packet pre-wotlk
                sendUpdatePacket = true;
#endif
            }
            else
            {
                const auto manaRate = worldConfig.getFloatRate(isVehicle() ? RATE_VEHICLES_POWER_REGEN : RATE_POWER1);
                //\ todo: this creature mana regeneration is not correct, rewrite it
                if (getCombatHandler().isInCombat())
                {
                    amount = (getLevel() + 10) * getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_MANA);
                    // Apply config rate for player owned creatures only
                    if (getPlayerOwner() != nullptr)
                        amount *= manaRate;
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
            amount *= timePassed / 1000.0f;
        } break;
        case POWER_TYPE_RAGE:
#if VERSION_STRING >= WotLK
        case POWER_TYPE_RUNIC_POWER:
#endif
        {
            if (getCombatHandler().isInCombat())
            {
                amount += getPowerRegenerationWhileInterrupted(type);
            }
            else
            {
                // Rage and Runic Power are lost at rate of 1.25 point per 1 second when out of combat
                amount = -12.5f;
                amount += getPowerRegeneration(type);
            }

            // Convert it to correct amount for expansion (wotlk 100ms, pre wotlk 3000ms)
            amount *= timePassed / 1000.0f;
#if VERSION_STRING < WotLK
            sendUpdatePacket = true;
#endif
        } break;
        case POWER_TYPE_FOCUS:
        {
#if VERSION_STRING < WotLK
            // 24 focus per 4 seconds or 6 focus per 1 second according to WoWWiki
            amount = 6.0f;
            sendUpdatePacket = true;
#else
            // Focus regens 1 point per 200ms or 5 point per 1 second as of 3.0.2
            amount = 5.0f;
            // Do not send for players or player owned creatures after wotlk, they regen in real time
            if (getPlayerOwnerOrSelf() == nullptr)
                sendUpdatePacket = true;
#endif
            amount += getPowerRegeneration(type);
            // Convert it to correct amount for expansion (wotlk 100ms, pre wotlk 4000ms)
            amount *= timePassed / 1000.0f;
        } break;
        case POWER_TYPE_ENERGY:
        {
            // 10 energy per 1 second
            amount = 10.0f;
            amount += getPowerRegeneration(type);

            // Send update packet for creatures in all versions
#if VERSION_STRING >= WotLK
            // but not for players or player owned creatures after wotlk, they regen in real time
            if (getPlayerOwnerOrSelf() == nullptr)
#endif
            {
                sendUpdatePacket = true;
            }

            // Convert it to correct amount for expansion / unit (i.e in wotlk 100ms for players and 2000ms for creatures)
            amount *= timePassed / 1000.0f;
        } break;
#if VERSION_STRING >= Cata
        case POWER_TYPE_HOLY_POWER:
        {
            if (getCombatHandler().isInCombat())
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
    const auto updateObject = sendUpdatePacket || m_powerUpdatePacketTime >= REGENERATION_PACKET_UPDATE_INTERVAL || powerResult == maxPower || powerResult == 0;
    setPower(type, powerResult, sendUpdatePacket, !updateObject);
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
#if VERSION_STRING > TBC
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

    // Send either SMSG_SPELLENERGIZELOG or SMSG_POWER_UPDATE packet, not both
    target->setPower(type, target->getPower(type) + amount, !sendPacket);

#if VERSION_STRING >= Cata
    // Reset Holy Power timer back to 10 seconds
    if (isPlayer() && type == POWER_TYPE_HOLY_POWER)
        dynamic_cast<Player*>(this)->resetHolyPowerTimer();
#endif
}

void Unit::sendSpellEnergizeLog(Unit* target, uint32_t spellId, uint32_t amount, PowerType type)
{
    sendMessageToSet(SmsgSpellEnergizeLog(target->GetNewGUID(), GetNewGUID(), spellId, type, amount).serialise().get(), true);
}

uint8_t Unit::getHealthPct() const
{
    if (getHealth() <= 0 || getMaxHealth() <= 0)
        return 0;

    if (getHealth() >= getMaxHealth())
        return 100;

    return static_cast<uint8_t>(getHealth() * 100 / getMaxHealth());
}

uint8_t Unit::getPctFromMaxHealth(uint8_t pct) const
{
    return static_cast<uint8_t>(getMaxHealth() * static_cast<float>(pct) / 100.0f);
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
    sendMessageToSet(SmsgPowerUpdate(GetNewGUID(), static_cast<uint8_t>(getPowerType()), powerAmount).serialise().get(), self);
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

void Unit::_regeneratePowersAtRegenUpdate([[maybe_unused]]PowerType type)
{
    // Before power regeneration is updated there must be a regeneration update with old values
#if VERSION_STRING < WotLK
    switch (type)
    {
        case POWER_TYPE_MANA:
        case POWER_TYPE_ENERGY:
            // Mana and energy are linked together with same timer
            regeneratePower(POWER_TYPE_MANA, m_manaEnergyRegenerateTimer);
            regeneratePower(POWER_TYPE_ENERGY, m_manaEnergyRegenerateTimer);
            m_manaEnergyRegenerateTimer = 0;
            break;
        case POWER_TYPE_RAGE:
            regeneratePower(POWER_TYPE_RAGE, m_rageRegenerateTimer);
            m_rageRegenerateTimer = 0;
            break;
        case POWER_TYPE_FOCUS:
            regeneratePower(POWER_TYPE_FOCUS, m_focusRegenerateTimer);
            m_focusRegenerateTimer = 0;
            break;
        default:
            break;
    }
#else
    // In wotlk all powers are on same timer so must update all
    regeneratePower(POWER_TYPE_MANA, m_powerRegenerateTimer);
    regeneratePower(POWER_TYPE_ENERGY, m_powerRegenerateTimer);
    regeneratePower(POWER_TYPE_FOCUS, m_powerRegenerateTimer);
    if (isPlayer())
    {
        regeneratePower(POWER_TYPE_RAGE, m_powerRegenerateTimer);
        regeneratePower(POWER_TYPE_RUNIC_POWER, m_powerRegenerateTimer);
    }

    m_powerRegenerateTimer = 0;
    if (m_powerUpdatePacketTime >= REGENERATION_PACKET_UPDATE_INTERVAL)
        m_powerUpdatePacketTime = 0;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Chat
std::unique_ptr<WorldPacket> Unit::createChatPacket(uint8_t type, uint32_t language, std::string msg, Unit* target/* = nullptr*/,  uint32_t sessionLanguage/* = 0*/)
{
    // Note: target is not the one who receives the message
    // it is whom the message should be pointed at
    // for example in text $N would get replaced by target's name
    // and $R would get replaced by target's race
    std::string senderName = "", targetName = "";
    uint64_t targetGuid = 0;

    // Get sender's name
    if (isPlayer())
    {
        senderName = dynamic_cast<Player*>(this)->getName();
    }
    else
    {
        const auto creature = dynamic_cast<Creature*>(this);
        const auto localizedName = (sessionLanguage > 0) ? sMySQLStore.getLocalizedCreature(creature->getEntry(), sessionLanguage) : nullptr;
        if (localizedName != nullptr)
            senderName = localizedName->name;
        else
            senderName = creature->GetCreatureProperties()->Name;
    }

    // Get target's name
    if (target != nullptr)
    {
        targetGuid = target->getGuid();

        if (target->isPlayer())
        {
            targetName = dynamic_cast<Player*>(target)->getName();
        }
        else
        {
            const auto creature = dynamic_cast<Creature*>(target);
            auto* const localizedName = (sessionLanguage > 0) ? sMySQLStore.getLocalizedCreature(creature->getEntry(), sessionLanguage) : nullptr;
            if (localizedName != nullptr)
                targetName = localizedName->name;
            else
                targetName = creature->GetCreatureProperties()->Name;
        }
    }

    return SmsgMessageChat(type, language, 0, msg, getGuid(), senderName, targetGuid, targetName).serialise();
}

void Unit::sendChatMessage(uint8_t type, uint32_t language, std::string msg, Unit* target/* = nullptr*/, uint32_t sessionLanguage/* = 0*/)
{
    const auto data = createChatPacket(type, language, msg, target, sessionLanguage);
    sendMessageToSet(data.get(), true);
}

void Unit::sendChatMessage(uint8_t type, uint32_t language, std::string msg, uint32_t delay)
{
    if (delay > 0)
    {
        sEventMgr.AddEvent(this, &Unit::sendChatMessage, type, language, msg, uint32_t(0), EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    sendChatMessage(type, language, msg);
}

void Unit::sendChatMessage(MySQLStructure::NpcScriptText const* text, uint32_t delay, Unit* target/* = nullptr*/)
{
    if (!isCreature() || text == nullptr)
        return;

    if (delay > 0)
    {
        sEventMgr.AddEvent(this, &Unit::sendChatMessage, text, uint32_t(0), target, EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    SendCreatureChatMessageInRange(dynamic_cast<Creature*>(this), text->id, target);
}

void Unit::sendChatMessageToPlayer(uint8_t type, uint32_t language, std::string msg, Player* plr)
{
    if (plr == nullptr)
        return;

    const auto data = createChatPacket(type, language, msg, plr, plr->getSession()->language);
    plr->getSession()->SendPacket(data.get());
}

void Unit::sendChatMessageAlternateEntry(uint32_t entry, uint8_t type, uint32_t lang, std::string  msg)
{
    if (CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry))
    {
        const auto data = SmsgMessageChat(type, lang, 0, msg, getGuid(), creatureProperties->Name).serialise();
        sendMessageToSet(data.get(), true);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void Unit::setAttackTimer(WeaponDamageType type, uint32_t time)
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
    setAttackTimer(type, static_cast<uint32_t>(getBaseAttackTime(type) * m_attackSpeed[type]));
}

void Unit::modAttackSpeedModifier(WeaponDamageType type, int32_t amount)
{
    if (amount > 0)
        m_attackSpeed[type] *= 1.0f + amount / 100.0f;
    else
        m_attackSpeed[type] /= 1.0f + -amount / 100.0f;
}

float Unit::getAttackSpeedModifier(WeaponDamageType type) const
{
    return m_attackSpeed[type];
}

void Unit::sendEnvironmentalDamageLogPacket(uint64_t guid, uint8_t type, uint32_t damage, uint64_t unk /*= 0*/)
{
    sendMessageToSet(SmsgEnvironmentalDamageLog(guid, type, damage, unk).serialise().get(), true, false);
}

bool Unit::isPvpFlagSet() const { return false; }
void Unit::setPvpFlag() {}
void Unit::removePvpFlag() {}

bool Unit::isFfaPvpFlagSet() const { return false; }
void Unit::setFfaPvpFlag() {}
void Unit::removeFfaPvpFlag() {}

bool Unit::isSanctuaryFlagSet() const { return false; }
void Unit::setSanctuaryFlag() {}
void Unit::removeSanctuaryFlag() {}

void Unit::restoreDisplayId()
{
    // Standard transform aura
    Aura* transform = nullptr;
    // Mostly a negative transform
    Aura* forcedTransform = nullptr;

    const auto& transformAuraList = getAuraEffectList(SPELL_AURA_TRANSFORM);
    for (auto itr = transformAuraList.crbegin(); itr != transformAuraList.crend(); ++itr)
    {
        auto* const aur = (*itr)->getAura();
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
            if (forcedTransform->getAuraEffect(i)->getAuraEffectType() != SPELL_AURA_TRANSFORM)
                continue;

            const auto displayId = forcedTransform->getAuraEffect(i)->getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                eventModelChange();
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
            if (shapeshift->getAuraEffect(i)->getAuraEffectType() != SPELL_AURA_MOD_SHAPESHIFT)
                continue;

            const auto displayId = shapeshift->getAuraEffect(i)->getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                eventModelChange();
                return;
            }
        }
    }

    if (transform != nullptr)
    {
        // Get display id from aura
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (transform->getAuraEffect(i)->getAuraEffectType() != SPELL_AURA_TRANSFORM)
                continue;

            const auto displayId = transform->getAuraEffect(i)->getEffectFixedDamage();
            if (displayId != 0)
            {
                setDisplayId(displayId);
                eventModelChange();
                setTransformAura(transform->getSpellId());
                return;
            }
        }
    }

    // No transform aura, no shapeshift aura => use native display id
    setDisplayId(getNativeDisplayId());
    eventModelChange();
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

    sendMessageToSet(SmsgEmote(emote, this->getGuid()).serialise().get(), true);
}

void Unit::eventAddEmote(EmoteType emote, uint32_t time)
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
    if (victim->isPlayer() && dynamic_cast<Player*>(victim)->m_cheats.hasGodModeCheat)
        return;
    if (victim->m_isInvincible)
        return;
    if (victim->isCreature() && dynamic_cast<Creature*>(victim)->isSpiritHealer())
        return;

    if (this != victim)
    {
        if (isPlayer())
        {
            const auto plr = dynamic_cast<Player*>(this);
            if (!plr->getSession()->hasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
                damage = plr->checkDamageLimits(damage, spellId);
        }

        const auto plrOwner = getPlayerOwnerOrSelf();
        if (plrOwner != nullptr)
        {
            // Battleground damage score
            if (plrOwner->getBattleground() && getWorldMap() == victim->getWorldMap())
            {
                plrOwner->m_bgScore.DamageDone += damage;
                plrOwner->getBattleground()->updatePvPData();
            }
        }

        // Make victim's pets react to attacker
        victim->getSummonInterface()->notifyOnOwnerAttacked(this);
    }

    victim->setStandState(STANDSTATE_STAND);

    // Tagging should happen when damage packets are sent
    const auto plrOwner = getPlayerOwnerOrSelf();
    if (plrOwner != nullptr && victim->isCreature() && victim->isTaggableFor(this))
    {
        victim->setTaggerGuid(this);
        plrOwner->tagUnit(victim);
    }

    if (removeAuras)
    {
        // Check for auras which are interrupted on damage taken
        // But do not remove the aura created by this spell
        if (spellId != 0)
        {
            victim->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellId);
            ///\ todo: fix this, currently used for root and fear auras
            if (Util::checkChance(35.0f))
                victim->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_UNUSED2, spellId);
        }
        else
        {
            victim->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
            ///\ todo: fix this, currently used for root and fear auras
            if (Util::checkChance(35.0f))
                victim->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
        }
    }

    victim->takeDamage(this, damage, spellId);
}

void Unit::takeDamage(Unit* attacker, uint32_t damage, uint32_t spellId)
{
    // Call damage taken creature script hook on entire batch
    if (attacker != nullptr && isCreature())
        sScriptMgr.DamageTaken(dynamic_cast<Creature*>(this), attacker, &damage);

    // Hackfix - Ardent Defender
    if (m_damageTakenPctModOnHP35 && hasAuraState(AURASTATE_FLAG_HEALTH35))
        damage = damage - Util::float2int32(damage * m_damageTakenPctModOnHP35) / 100;

    if (damage >= getHealth())
    {
        if (isTrainingDummy())
        {
            setHealth(1);
            return;
        }

        // Duel check
        if (isPlayer() && dynamic_cast<Player*>(this)->getDuelPlayer() != nullptr)
        {
            setHealth(5);
            dynamic_cast<Player*>(this)->getDuelPlayer()->endDuel(DUEL_WINNER_KNOCKOUT);
            emote(EMOTE_ONESHOT_BEG);
            return;
        }

        // The attacker must exist here and if it doesn't exist, victim won't die
        if (attacker == nullptr)
            return;

        if (auto* const plrOwner = attacker->getPlayerOwnerOrSelf())
        {
            if (plrOwner->getBattleground())
            {
                plrOwner->getBattleground()->HookOnUnitKill(plrOwner, this);

                if (isPlayer())
                    plrOwner->getBattleground()->HookOnPlayerKill(plrOwner, dynamic_cast<Player*>(this));
            }

            if (isPlayer())
            {
                sHookInterface.OnKillPlayer(plrOwner, dynamic_cast<Player*>(this));
            }
            else if (isCreature())
            {
                plrOwner->onKillUnitReputation(this, false);
#ifdef FT_ACHIEVEMENTS
                plrOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, attacker->GetMapId(), 0, 0);
#endif
            }

            // Check is the unit gray level for attacker
            if (!isGrayLevel(plrOwner->getLevel(), getLevel()) && (getGuid() != attacker->getGuid()))
            {
                if (isPlayer())
                {
#ifdef FT_ACHIEVEMENTS
                    plrOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, plrOwner->getAreaId(), 1, 0);
                    plrOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL, 1, 0, 0);
#endif
                    HonorHandler::OnPlayerKilled(plrOwner, dynamic_cast<Player*>(this));
                }

                plrOwner->addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);

                if (!sEventMgr.HasEvent(plrOwner, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                    sEventMgr.AddEvent(dynamic_cast<Unit*>(plrOwner), &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(plrOwner, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

                plrOwner->handleProc(PROC_ON_KILL, this, nullptr, DamageInfo(), false);
            }

            // Send zone under attack message
            if (isPvpFlagSet())
            {
                auto team = plrOwner->getTeam();
                if (team == TEAM_ALLIANCE)
                    team = TEAM_HORDE;
                else
                    team = TEAM_ALLIANCE;

                const auto area = GetArea();
                sWorld.sendZoneUnderAttackMessage(area != nullptr ? area->id : attacker->getZoneId(), team);
            }
        }

        if (attacker->isPlayer())
        {
            const auto plr = dynamic_cast<Player*>(attacker);

            plr->eventAttackStop();

            plr->sendPartyKillLogPacket(getGuid());
        }

        die(attacker, damage, spellId);

        // Loot
        if (isLootable())
        {
            const auto tagger = getWorldMapPlayer(getTaggerGuid());
            if (tagger != nullptr)
            {
                if (tagger->isInGroup()) // Group Case
                {
                    for (uint8_t i = 0; i < tagger->getGroup()->GetSubGroupCount(); i++)
                    {
                        if (tagger->getGroup()->GetSubGroup(i) != nullptr)
                        {
                            for (auto itr : tagger->getGroup()->GetSubGroup(i)->getGroupMembers())
                            {
                                if (itr != nullptr)
                                {
                                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
                                    {
                                        if (ToCreature()->HasLootForPlayer(loggedInPlayer))
                                            loggedInPlayer->sendLootUpdate(this);
                                    }
                                }
                            }
                        }
                    }
                }
                else if (ToCreature()->HasLootForPlayer(tagger))    // Player case
                {
                    tagger->sendLootUpdate(this);
                }
            }
        }

        if (!isPet() && getCreatedByGuid() == 0)
        {
            // Experience points
            if (isTagged())
            {
                const auto taggerUnit = getWorldMapUnit(getTaggerGuid());
                const auto tagger = taggerUnit != nullptr ? taggerUnit->getPlayerOwnerOrSelf() : nullptr;
                if (tagger != nullptr)
                {
                    if (tagger->isInGroup())
                    {
                        tagger->giveGroupXP(this, tagger);
                    }
                    else
                    {
                        auto xp = CalculateXpToGive(this, tagger);
                        if (xp > 0)
                        {
                            tagger->giveXp(xp, getGuid(), true);
                            // Give XP to pets also
                            if (tagger->getPet() != nullptr && tagger->getPet()->canGainXp())
                            {
                                xp = CalculateXpToGive(this, tagger->getPet());
                                if (xp > 0)
                                    tagger->getPet()->giveXp(xp);
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
                            tagger->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, getEntry(), 1, 0);
                            tagger->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                        }
#endif
                    }
                }
            }
#ifdef FT_ACHIEVEMENTS
            else if (isCritter())
            {
                if (auto* const plrOwner = attacker->getPlayerOwnerOrSelf())
                {
                    plrOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, getEntry(), 1, 0);
                    plrOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                }
            }
#endif
        }

        // The unit has died so no need to proceed any further
        return;
    }

    if (isPlayer())
    {
        const auto plr = dynamic_cast<Player*>(this);
        // todo: remove this hackfix...
        if (plr->m_cannibalize)
        {
            sEventMgr.RemoveEvents(plr, EVENT_CANNIBALIZE);
            setEmoteState(EMOTE_ONESHOT_NONE);
            plr->m_cannibalize = false;
        }
    }

    // Modify health
    modHealth(-1 * static_cast<int32_t>(damage));
}

void Unit::addSimpleDamageBatchEvent(uint32_t damage, Unit* attacker/* = nullptr*/, SpellInfo const* spellInfo/* = nullptr*/)
{
    auto batch = std::make_unique<HealthBatchEvent>();
    batch->caster = attacker;
    batch->damageInfo.realDamage = damage;
    batch->spellInfo = spellInfo;
    
    addHealthBatchEvent(std::move(batch));
}

void Unit::addSimpleEnvironmentalDamageBatchEvent(EnviromentalDamage type, uint32_t damage, uint32_t absorbedDamage/* = 0*/)
{
    auto batch = std::make_unique<HealthBatchEvent>();
    batch->damageInfo.realDamage = damage;
    batch->isEnvironmentalDamage = true;
    batch->environmentType = type;

    // Only fire and lava environmental damage types can be absorbed
    if (type == DAMAGE_FIRE || type == DAMAGE_LAVA)
        batch->damageInfo.absorbedDamage = absorbedDamage;

    addHealthBatchEvent(std::move(batch));
}

void Unit::addSimpleHealingBatchEvent(uint32_t heal, Unit* healer/* = nullptr*/, SpellInfo const* spellInfo/* = nullptr*/)
{
    auto batch = std::make_unique<HealthBatchEvent>();
    batch->caster = healer;
    batch->damageInfo.realDamage = heal;
    batch->spellInfo = spellInfo;
    batch->isHeal = true;

    addHealthBatchEvent(std::move(batch));
}

void Unit::addHealthBatchEvent(std::unique_ptr<HealthBatchEvent> batch)
{
    if (batch == nullptr)
        return;

    // Do some checks before adding the health event into batch list
    if (!isAlive() || !IsInWorld() || m_isInvincible)
        return;

    if (isPlayer())
    {
        const auto plr = dynamic_cast<Player*>(this);
        if (!batch->isHeal && plr->m_cheats.hasGodModeCheat)
            return;
    }
    else if (isCreature())
    {
        if (dynamic_cast<Creature*>(this)->isSpiritHealer())
            return;
    }

    m_healthBatch.push_back(std::move(batch));
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
    for (auto& aur : getAuraList())
    {
        if (aur == nullptr || !aur->isAbsorbAura())
            continue;

        AbsorbAura* abs = dynamic_cast<AbsorbAura*>(aur.get());
        totalAbsorbedDamage += abs->absorbDamage(schoolMask, dmg, checkOnly);
    }

    if (isPlayer() && dynamic_cast<Player*>(this)->m_cheats.hasGodModeCheat)
    {
        totalAbsorbedDamage += *dmg;
        *dmg = 0;
    }

    return totalAbsorbedDamage;
}

void Unit::smsg_AttackStop(Unit* pVictim)
{
    if (pVictim)
        sendMessageToSet(SmsgAttackStop(GetNewGUID(), pVictim->GetNewGUID()).serialise().get(), true);
    else
        sendMessageToSet(SmsgAttackStop(GetNewGUID(), WoWGuid()).serialise().get(), true);
}

void Unit::smsg_AttackStart(Unit* pVictim)
{
    sendMessageToSet(SmsgAttackStart(getGuid(), pVictim->getGuid()).serialise().get(), false);

    sLogger.debug("WORLD: Sent SMSG_ATTACK_START");

    if (isPlayer())
    {
        Player* player = dynamic_cast<Player*>(this);
        if (player->m_cannibalize)
        {
            sEventMgr.RemoveEvents(player, EVENT_CANNIBALIZE);
            player->setEmoteState(EMOTE_ONESHOT_NONE);
            player->m_cannibalize = false;
        }
    }
}

void Unit::addToInRangeObjects(Object* pObj)
{
    if (pObj->isCreatureOrPlayer())
    {
        if (this->isHostileTo(pObj))
            addInRangeOppositeFaction(pObj);

        if (this->isFriendlyTo(pObj))
            addInRangeSameFaction(pObj);
    }

    Object::addToInRangeObjects(pObj);
}

void Unit::onRemoveInRangeObject(Object* pObj)
{
    removeObjectFromInRangeOppositeFactionSet(pObj);
    removeObjectFromInRangeSameFactionSet(pObj);

    if (pObj->isCreatureOrPlayer())
    {
        if (getCharmGuid() == pObj->getGuid())
            interruptSpell();
    }
}

void Unit::clearInRangeSets()
{
    Object::clearInRangeSets();
}

bool Unit::setDetectRangeMod(uint64_t guid, int32_t amount)
{
    int next_free_slot = -1;
    for (uint8_t i = 0; i < 5; i++)
    {
        if (m_detectRangeGuids[i] == 0 && next_free_slot == -1)
        {
            next_free_slot = i;
        }
        if (m_detectRangeGuids[i] == guid)
        {
            m_detectRangeMods[i] = amount;
            return true;
        }
    }
    if (next_free_slot != -1)
    {
        m_detectRangeGuids[next_free_slot] = guid;
        m_detectRangeMods[next_free_slot] = amount;
        return true;
    }
    return false;
}

void Unit::unsetDetectRangeMod(uint64_t guid)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (m_detectRangeGuids[i] == guid)
        {
            m_detectRangeGuids[i] = 0;
            m_detectRangeMods[i] = 0;
        }
    }
}

int32_t Unit::getDetectRangeMod(uint64_t guid) const
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (m_detectRangeGuids[i] == guid)
            return m_detectRangeMods[i];
    }
    return 0;
}

void Unit::knockbackFrom(float x, float y, float speedXY, float speedZ)
{
    Player* player = ToPlayer();
    if (!player)
    {
        if (getCharmGuid())
        {
            Unit* charmer = getWorldMapPlayer(getCharmGuid());
            player = charmer->ToPlayer();
        }
    }

    if (!player)
    {
        getMovementManager()->moveKnockbackFrom(x, y, speedXY, speedZ);
    }
    else
    {
        player->getSession()->SendPacket(SmsgMoveKnockBack(player->GetNewGUID(), Util::getMSTime(), cosf(player->GetOrientation()), sinf(player->GetOrientation()), speedXY, -speedZ).serialise().get());

#if VERSION_STRING >= TBC
        if (player->hasAuraWithAuraEffect(SPELL_AURA_ENABLE_FLIGHT2) || player->hasAuraWithAuraEffect(SPELL_AURA_FLY))
            player->setMoveCanFly(true);
#endif
    }
}

void Unit::_updateHealth()
{
    const auto curHealth = getHealth();
    int32_t healthVal = 0;
    uint32_t totalAbsorbDamage = 0;
    // Victim's rage generation on damage taken
    int32_t totalRageGenerated = 0;

    Unit* killer = nullptr;
    // Get single damager from entire batch for creature scripts
    Unit* singleDamager = nullptr;

    // Process through health batch
    auto batchItr = m_healthBatch.begin();
    while (batchItr != m_healthBatch.end())
    {
        const auto& batch = *batchItr;

        if (batch->isHeal)
        {
            uint32_t absorbedHeal = 0;
            const auto heal = _handleBatchHealing(batch.get(), &absorbedHeal);
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
            const auto damage = _handleBatchDamage(batch.get(), &rageGenerated);
            healthVal -= damage;

            singleDamager = batch->caster;

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

        batchItr = m_healthBatch.erase(batchItr);
    }

    // Do the real absorb damage
    absorbDamage(SCHOOL_MASK_ALL, &totalAbsorbDamage, false);

    // If the value is negative, then damage in the batch exceeds healing and unit takes damage
    if (healthVal < 0)
        takeDamage(killer != nullptr ? killer : singleDamager, static_cast<uint32_t>(std::abs(healthVal)), 0);
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
    auto damage = batch->damageInfo.realDamage;

    const auto attacker = batch->caster;
    if (attacker != nullptr && attacker != this)
    {
        if (attacker->isPlayer())
        {
            const auto plr = dynamic_cast<Player*>(attacker);
            if (!plr->getSession()->hasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
                damage = plr->checkDamageLimits(damage, spellId);
        }

        // Rage generation for victim
        ///\ todo: this is inaccurate
        if (getPowerType() == POWER_TYPE_RAGE)
        {
            const auto level = static_cast<float_t>(getLevel());
            const float_t c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

            float_t val = 2.5f * damage / c;
            const auto rage = getPower(POWER_TYPE_RAGE);

            if (rage + Util::float2int32(val) > 1000)
                val = 1000.0f - static_cast<float>(getPower(POWER_TYPE_RAGE));

            val *= 10.0;
            *rageGenerated = Util::float2int32(val);
        }

        const auto plrOwner = attacker->getPlayerOwnerOrSelf();
        if (plrOwner != nullptr)
        {
            // Battleground damage score
            if (plrOwner->getBattleground() && getWorldMap() == attacker->getWorldMap())
            {
                plrOwner->m_bgScore.DamageDone += damage;
                plrOwner->getBattleground()->updatePvPData();
            }
        }

        // Make victim's pets react to attacker
        m_summonInterface->notifyOnOwnerAttacked(attacker);
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
        removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellId);
        ///\ todo: fix this, currently used for root and fear auras
        if (Util::checkChance(35.0f))
            removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_UNUSED2, spellId);
    }
    else
    {
        removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
        ///\ todo: fix this, currently used for root and fear auras
        if (Util::checkChance(35.0f))
            removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
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
    if (healer )
    {
        const auto plrOwner = healer->getPlayerOwnerOrSelf();

        // Update battleground score
        if (plrOwner && plrOwner->getBattleground() && plrOwner->getWorldMap() == getWorldMap())
        {
            plrOwner->m_bgScore.HealingDone += healing;
            plrOwner->getBattleground()->updatePvPData();
        }
    }

    removeAurasByHeal();

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

#ifdef FT_VEHICLES
    bool isOnVehicle = getVehicle() != nullptr;
#else
    bool isOnVehicle = false;
#endif

    if (state != ALIVE && state != JUST_RESPAWNED)
    {
#ifdef FT_VEHICLES
        exitVehicle();
#endif
        removeAllNonPersistentAuras();
        
        if (!isPet())
            removeUnitFlags(UNIT_FLAG_PET_IN_COMBAT);
    }

    if (state == JUST_DIED)
    {
        getThreatManager().removeMeFromThreatLists();
        removeAllNonPersistentAuras();

        // Don't clear the movement if the Unit was on a vehicle as we are exiting now
        if (!isOnVehicle)
        {
            if (IsInWorld())
            {
                getMovementManager()->clear();
                getMovementManager()->moveIdle();
            }

            stopMoving();
            disableSpline();
        }

        setHealth(0);
        setPower(getPowerType(), 0);
        setEmoteState(0);
    }
    else if (state == JUST_RESPAWNED)
    {
        removeUnitFlags(UNIT_FLAG_SKINNABLE); // clear skinnable for creature and player
    }
}

DeathState Unit::getDeathState() const { return m_deathState; }

//////////////////////////////////////////////////////////////////////////////////////////
// Summons

Pet* Unit::getPet() const
{
    return m_summonInterface->getPet();
}

TotemSummon* Unit::getTotem(SummonSlot slot) const
{
    auto* const totem = m_summonInterface->getSummonInSlot(slot);
    if (totem == nullptr || !totem->isTotem())
        return nullptr;

    return dynamic_cast<TotemSummon*>(totem);
}

SummonHandler* Unit::getSummonInterface()
{
    return m_summonInterface.get();
}

SummonHandler const* Unit::getSummonInterface() const
{
    return m_summonInterface.get();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Unit Owner
bool Unit::isUnitOwnerInParty(Unit* unit)
{
    if (unit)
    {
        Player* playOwner = getPlayerOwnerOrSelf();
        Player* playerOwnerFromUnit = unit->getPlayerOwnerOrSelf();
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
        Player* playerOwner = getPlayerOwnerOrSelf();
        Player* playerOwnerFromUnit = unit->getPlayerOwnerOrSelf();
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
#ifdef FT_VEHICLES
    if (getVehicle())
        return getVehicleBase()->getGuid();
#endif
    if (GetTransport())
        return GetTransport()->getGuid();

    return 0;
}

MovementGeneratorType Unit::getDefaultMovementType() const
{
    return IDLE_MOTION_TYPE;
}

#if VERSION_STRING >= Cata
WDB::Structures::MountCapabilityEntry const* Unit::getMountCapability(uint32_t mountType)
{
    if (!mountType)
        return nullptr;

    auto const* mountTypeEntry = sMountTypeStore.lookupEntry(mountType);
    if (!mountTypeEntry)
        return nullptr;

    uint32_t zoneId = 0;
    uint32_t areaId = 0;

    if (getZoneId())
        zoneId = getZoneId();

    if (GetArea())
        areaId = GetArea()->id;

    uint32_t ridingSkill = 5000;
    if (GetTypeFromGUID() == TYPEID_PLAYER)
        ridingSkill = ToPlayer()->getSkillLineCurrent(SKILL_RIDING);

    for (uint32_t i = MAX_MOUNT_CAPABILITIES; i > 0; --i)
    {
        auto const* mountCapability = sMountCapabilityStore.lookupEntry(mountTypeEntry->capabilities[i - 1]);
        if (!mountCapability)
            continue;

        if (ridingSkill < mountCapability->reqRidingSkill)
            continue;

        if (hasExtraUnitMovementFlag(MOVEFLAG2_FULLSPEED_PITCHING))
        {
            if (!(mountCapability->flag & MOUNT_FLAG_CAN_PITCH))
                continue;
        }
        else if (hasUnitMovementFlag(MOVEFLAG_SWIMMING))
        {
            if (!(mountCapability->flag & MOUNT_FLAG_CAN_SWIM))
                continue;
        }
        else if (!(mountCapability->flag & 0x1))   // unknown flags, checked in 4.2.2 14545 client
        {
            if (!(mountCapability->flag & 0x2))
                continue;
        }

        if (mountCapability->reqMap != -1 && int32_t(GetMapId()) != mountCapability->reqMap)
            continue;

        if (mountCapability->reqArea && (mountCapability->reqArea != zoneId && mountCapability->reqArea != areaId))
            continue;

        if (mountCapability->reqAura && !hasAurasWithId(mountCapability->reqAura))
            continue;

        if (mountCapability->reqSpell && (GetTypeFromGUID() != TYPEID_PLAYER || !ToPlayer()->hasSpell(mountCapability->reqSpell)))
            continue;

        return mountCapability;
    }

    return nullptr;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Threat Management

void Unit::clearHateList()
{
    getThreatManager().resetAllThreat();
}

void Unit::wipeHateList()
{
    getThreatManager().clearAllThreat();
}

void Unit::wipeTargetList()
{
    getThreatManager().clearAllThreat();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Tagging (helper for dynamic flags)

void Unit::setTaggerGuid(Unit const* tagger)
{
    if (tagger != nullptr)
    {
        this->m_taggerGuid = tagger->getGuid();
        m_taggedBySummon = tagger->isSummon();
        if (!m_taggedBySummon)
            addDynamicFlags(U_DYN_FLAG_TAGGED_BY_OTHER);
    }
    else
    {
        this->m_taggerGuid = 0;
        m_taggedBySummon = false;
        removeDynamicFlags(U_DYN_FLAG_TAGGED_BY_OTHER);
    }
}

uint64_t Unit::getTaggerGuid() const
{
    return m_taggerGuid;
}

bool Unit::isTagged() const
{
    return hasDynamicFlags(U_DYN_FLAG_TAGGED_BY_OTHER) || (m_taggerGuid != 0 && m_taggedBySummon);
}

bool Unit::isTaggableFor(Unit const* unit) const
{
    if (isPet())
        return false;

    if (isTagged())
    {
        if (m_taggedBySummon && unit != nullptr)
        {
            // If tagged by summon, owner can tag it for themself
            for (const auto* summon : unit->getSummonInterface()->getSummons())
            {
                if (summon->getGuid() == m_taggerGuid)
                    return true;
            }
        }

        return false;
    }

    return true;
}

bool Unit::isTaggedByPlayerOrItsGroup(Player* tagger)
{
    if (!isTagged() || tagger == nullptr)
        return false;

    if (getTaggerGuid() == tagger->getGuid())
        return true;

    if (tagger->isInGroup())
    {
        if (const auto playerTagger = getWorldMapPlayer(getTaggerGuid()))
            if (tagger->getGroup()->HasMember(playerTagger))
                return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loot

bool Unit::isLootable()
{
    if (isTagged() && !isPet() && !(isPlayer() && !IsInBg()) && (getCreatedByGuid() == 0) && !isVehicle())
    {
        if (const auto creatureProperties = sMySQLStore.getCreatureProperties(getEntry()))
        {
            if (isCreature() && !sLootMgr.isCreatureLootable(getEntry()) && creatureProperties->money == 0)
                return false;
        }

        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Vehicle
#ifdef FT_VEHICLES
bool Unit::createVehicleKit(uint32_t id, uint32_t creatureEntry)
{
    auto vehInfo = sVehicleStore.lookupEntry(id);
    if (!vehInfo)
        return false;

    m_vehicleKit = std::make_unique<Vehicle>(this, vehInfo, creatureEntry);
    m_updateFlag |= UPDATEFLAG_VEHICLE;
    return true;
}

void Unit::removeVehicleKit()
{
    if (!m_vehicleKit)
        return;

    m_vehicleKit->deactivate();
    m_vehicleKit = nullptr;

    m_updateFlag &= ~UPDATEFLAG_VEHICLE;
    removeNpcFlags(UNIT_NPC_FLAG_SPELLCLICK | UNIT_NPC_FLAG_PLAYER_VEHICLE);
}

bool Unit::isOnVehicle(Unit const* vehicle) const
{
    return m_vehicle && m_vehicle == vehicle->getVehicleKit();
}

Unit* Unit::getVehicleBase() const
{
    return m_vehicle ? m_vehicle->getBase() : nullptr;
}

Unit* Unit::getVehicleRoot() const
{
    Unit* vehicleRoot = getVehicleBase();

    if (!vehicleRoot)
        return nullptr;

    for (;;)
    {
        if (!vehicleRoot->getVehicleBase())
            return vehicleRoot;

        vehicleRoot = vehicleRoot->getVehicleBase();
    }
}

Creature* Unit::getVehicleCreatureBase() const
{
    if (Unit* veh = getVehicleBase())
        if (Creature* c = veh->ToCreature())
            return c;

    return nullptr;
}

void Unit::handleSpellClick(Unit* clicker, int8_t seatId /*= -1*/)
{
    bool spellClickHandled = false;
    uint32_t spellClickEntry = getVehicleKit() ? getVehicleKit()->getEntry() : getEntry();
    
    std::vector<SpellClickInfo> clickBounds = sMySQLStore.getSpellClickInfo(spellClickEntry);
    for (const auto& clickPair : clickBounds)
    {
            // First check simple relations from clicker to clickee
            if (!clickPair.isFitToRequirements(clicker, this))
                continue;

            Unit* caster = (clickPair.castFlags & NPC_CLICK_CAST_CASTER_CLICKER) ? clicker : this;
            Unit* target = (clickPair.castFlags & NPC_CLICK_CAST_TARGET_CLICKER) ? clicker : this;

            SpellInfo const* spellEntry = sSpellMgr.getSpellInfo(clickPair.spellId);

            // Vehicle Handling
            if (seatId > -1)
            {
                uint8_t i = 0;
                bool valid = false;
                while (i < MAX_SPELL_EFFECTS)
                {
                    if (spellEntry->getEffectApplyAuraName(i) == SPELL_AURA_CONTROL_VEHICLE)
                    {
                        valid = true;
                        break;
                    }
                    ++i;
                }

                if (!valid)
                {
                    sLogger.failure("Spell {} specified in npc_spellclick_spells is not a valid vehicle enter aura!", clickPair.spellId);
                    continue;
                }

                if (IsInMap(caster))
                {
                    //   VEHICLE_SPELL_RIDE_HARDCODED gets Casted on the Target
                    //   We pass trough seatId trough EffectBaseDamage for further use
                    //   The Aura Handler "HANDLE_AURA_CONTROL_VEHICLE" takes care of us.
                    //   HANDLE_AURA_CONTROL_VEHICLE will call enterVehicle or exitVehicle

                    SpellForcedBasePoints bp;
                    bp.set(i, seatId + 1);
                    caster->castSpell(target, clickPair.spellId, bp, true);
                }
            }
            else
            {
                // Creatures like Lightwell...
                if (IsInMap(caster))
                    caster->castSpell(target, spellEntry->getId(), true);
            }

            spellClickHandled = true;
        }

        if (isCreature())
        {
            if (CreatureAIScript* ai = ToCreature()->GetScript())
            {
                ai->OnSpellClick(clicker, spellClickHandled);
            }
        }  
}

void Unit::callEnterVehicle(Unit* base, int8_t seatId /*= -1*/)
{
    //   VEHICLE_SPELL_RIDE_HARDCODED gets Casted on the Target
    //   We pass trough seatId trough EffectBaseDamage for further use
    //   The Aura Handler "HANDLE_AURA_CONTROL_VEHICLE" takes care of us.
    //   HANDLE_AURA_CONTROL_VEHICLE will call enterVehicle or exitVehicle

    SpellForcedBasePoints bp;
    bp.set(0, seatId + 1);
    castSpell(base, VEHICLE_SPELL_RIDE_HARDCODED, bp, true);
}

void Unit::enterVehicle(Vehicle* vehicle, int8_t seatId)
{
    if (!isAlive() || getVehicleKit() == vehicle || vehicle->getBase()->isOnVehicle(this))
        return;

    if (m_vehicle)
    {
        if (m_vehicle != vehicle)
        {
            callExitVehicle();
        }
        else if (seatId >= 0 && seatId == GetTransSeat())
        {
            return;
        }
        else
        {
            //Exit the current vehicle because unit will reenter in a new seat.
            m_vehicle->getBase()->removeAllAurasByAuraEffect(SPELL_AURA_CONTROL_VEHICLE, 0, false, getGuid());
        }
    }

    if (Player* player = ToPlayer())
    {
        if (vehicle->getBase()->isPlayer() && player->isInCombat())
        {
            vehicle->getBase()->removeAllAurasByAuraEffect(SPELL_AURA_CONTROL_VEHICLE);
            return;
        }

        if (vehicle->getBase()->isCreature())
        {
            // If a player entered a vehicle that is part of a formation, remove it from the formation
            if (CreatureGroup* creatureGroup = vehicle->getBase()->ToCreature()->getFormation())
                creatureGroup->removeMember(vehicle->getBase()->ToCreature());
        }
    }

    // If vehicle flag for fixed position set (cannons), or if the following hardcoded units, then set state rooted
    //  30236 | Argent Cannon
    //  39759 | Tankbuster Cannon
    if ((vehicle->getVehicleInfo()->flags & VEHICLE_FLAG_POSITION_FIXED) || vehicle->getBase()->getEntry() == 30236 || vehicle->getBase()->getEntry() == 39759)
        setControlled(true, UNIT_STATE_ROOTED);

    if (!vehicle->addPassenger(this, seatId))
    {
        if (isCreature())
            ToCreature()->Despawn(2000, 0);
    }
}

void Unit::callChangeSeat(int8_t seatId, bool next)
{
    if (!m_vehicle)
        return;

    // Don't change if current and new seat are identical
    if (seatId == GetTransSeat())
        return;

    SeatMap::const_iterator seat = (seatId < 0 ? m_vehicle->getNextEmptySeat(GetTransSeat(), next) : m_vehicle->Seats.find(seatId));
    if (seat == m_vehicle->Seats.end() || !seat->second.isEmpty())
        return;

    //   VEHICLE_SPELL_RIDE_HARDCODED gets Casted on the Target
    //   We pass trough seatId trough EffectBaseDamage for further use
    //   The Aura Handler "HANDLE_AURA_CONTROL_VEHICLE" takes care of us.
    //   HANDLE_AURA_CONTROL_VEHICLE will call enterVehicle or exitVehicle

    // Unit riding a vehicle must always have control vehicle aura on target
    for (const auto& aurEff : m_vehicle->getBase()->getAuraEffectList(SPELL_AURA_CONTROL_VEHICLE))
    {
        if (aurEff->getAura()->getCasterGuid() != getGuid())
            continue;

        auto modifiableEff = aurEff->getAura()->getModifiableAuraEffect(aurEff->getEffectIndex());
        modifiableEff->setEffectBaseDamage(seat->first + 1);

        aurEff->getAura()->refreshOrModifyStack();
        break;
    }
}

void Unit::callExitVehicle(LocationVector const* /*exitPosition*/)
{
    //   VEHICLE_SPELL_RIDE_HARDCODED gets Casted on the Target
    //   We pass trough seatId trough EffectBaseDamage for further use
    //   The Aura Handler "HANDLE_AURA_CONTROL_VEHICLE" takes care of us.
    //   HANDLE_AURA_CONTROL_VEHICLE will call enterVehicle or exitVehicle

    if (!m_vehicle)
        return;

    getVehicleBase()->removeAllAurasByAuraEffect(SPELL_AURA_CONTROL_VEHICLE, 0, false, getGuid());
}

void Unit::exitVehicle(LocationVector const* exitPosition)
{
    if (!m_vehicle)
        return;

    VehicleSeatAddon const* seatAddon = m_vehicle->getSeatAddonForSeatOfPassenger(this);
    Vehicle* vehicle = m_vehicle->removePassenger(this);

    Player* player = ToPlayer();

    // Unroot the Passenger
    setControlled(false, UNIT_STATE_ROOTED);

    addUnitStateFlag(UNIT_STATE_MOVE);

    // Unroot the Passenger when the Above code fails
    if (hasUnitMovementFlag(MOVEFLAG_ROOTED))
    {
        WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 8);
        data << GetNewGUID();
        sendMessageToSet(&data, false);
    }

    LocationVector pos;
    // If we ask for a specific exit position, use that one. Otherwise allow scripts to modify it
    if (exitPosition)
    {
        pos = *exitPosition;
    }
    else
    {
        // Set exit position to vehicle position and use the current orientation
        pos = vehicle->getBase()->GetPosition();
        pos.o = GetOrientation();

        // Change exit position based on seat entry addon data
        if (seatAddon)
        {
            if (seatAddon->exitParameter == VehicleExitParameters::Offset)
                pos.ChangeCoordsOffset(seatAddon->exitLocation);
            else if (seatAddon->exitParameter == VehicleExitParameters::Destination)
                pos.ChangeCoords(seatAddon->exitLocation);
        }
    }

    // Send movement Spline
    MovementMgr::MoveSplineInit init(this);
    init.MoveTo(pos.getPositionX(), pos.getPositionY(), pos.getPositionZ(), false);
    init.SetFacing(pos.getOrientation());
    init.SetTransportExit();
    getMovementManager()->launchMoveSpline(std::move(init), EVENT_VEHICLE_EXIT, MOTION_PRIORITY_HIGHEST);

    // Spawn active Pets
    if (player)
        player->summonTemporarilyUnsummonedPet();

    // Despawn Accessories
    if (vehicle->getBase()->hasUnitStateFlag(UNIT_STATE_ACCESSORY) && vehicle->getBase()->isCreature())
        if ((vehicle->getBase())->getVehicleKit()->getBase() == this)
            vehicle->getBase()->ToCreature()->Despawn(2000, 0);

    if (hasUnitStateFlag(UNIT_STATE_ACCESSORY))
    {
        // Vehicle just died, we die too
        if (vehicle->getBase()->getDeathState() == JUST_DIED)
        {
            setDeathState(JUST_DIED);
        }
        else
        {
            // If for other reason we as Accessories are exiting the vehicle 
            // (ejected, master dismounted) despawn.
            ToCreature()->Despawn(2000, 0);
        }
    }
}
#else
void Unit::handleSpellClick(Unit* clicker)
{
    bool spellClickHandled = false;
    uint32_t spellClickEntry = getEntry();

    std::vector<SpellClickInfo> clickBounds = sMySQLStore.getSpellClickInfo(spellClickEntry);
    for (const auto& clickPair : clickBounds)
    {
        //! First check simple relations from clicker to clickee
        if (!clickPair.isFitToRequirements(clicker, this))
            continue;

        Unit* caster = (clickPair.castFlags & NPC_CLICK_CAST_CASTER_CLICKER) ? clicker : this;
        Unit* target = (clickPair.castFlags & NPC_CLICK_CAST_TARGET_CLICKER) ? clicker : this;
        auto* const unitOwner = getUnitOwner();
        uint64_t origCasterGUID = (unitOwner && clickPair.castFlags & NPC_CLICK_CAST_ORIG_CASTER_OWNER) ? unitOwner->getGuid() : clicker->getGuid();

        SpellInfo const* spellEntry = sSpellMgr.getSpellInfo(clickPair.spellId);

        // Creatures like Lightwell...
        if (IsInMap(caster))
            caster->castSpell(target, spellEntry->getId(), true);

        spellClickHandled = true;
    }

    if (isCreature())
    {
        if (CreatureAIScript* ai = ToCreature()->GetScript())
        {
            ai->OnSpellClick(clicker, spellClickHandled);
        }
    }
}
#endif

bool Unit::isMounted() const
{
#if VERSION_STRING == Classic
    // TODO
    return false;
#else
    return hasUnitFlags(UNIT_FLAG_MOUNT);
#endif
}

void Unit::mount(uint32_t mount, uint32_t VehicleId, uint32_t creatureEntry)
{
#if VERSION_STRING == Classic
    // TODO
#else
    if (mount)
        setMountDisplayId(mount);

    addUnitFlags(UNIT_FLAG_MOUNT);

    if (Player* player = ToPlayer())
    {
#if VERSION_STRING > TBC
        // mount as a vehicle
        if (VehicleId)
        {
            if (createVehicleKit(VehicleId, creatureEntry))
            {
                // Send others that we now have a vehicle
                sendMessageToSet(SmsgPlayerVehicleData(WoWGuid(getGuid()), VehicleId).serialise().get(), true);
                sendPacket(SmsgControlVehicle().serialise().get());

                // mounts can also have accessories
                getVehicleKit()->initialize();
                getVehicleKit()->loadAllAccessories(false);
            }
        }
#endif
        // unsummon pet
        if (player->isPetRequiringTemporaryUnsummon())
            player->unSummonPetTemporarily();

        // if we have charmed npc, stun him also (everywhere)
        if (Unit* charm = getWorldMapUnit(getCharmGuid()))
            if (charm->isCreature())
                charm->addUnitFlags(UNIT_FLAG_STUNNED);

        ByteBuffer guidData;
        guidData << GetNewGUID();

        WorldPacket data(SMSG_MOVE_SET_COLLISION_HGT, guidData.size() + 4 + 4);
        data.append(guidData);
        data << uint32_t(Util::getTimeNow());   // Packet counter
        data << getCollisionHeight();
        sendPacket(&data);
    }

    removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_MOUNT);
#endif
}

void Unit::dismount(bool resummonPet/* = true*/)
{
    if (!isMounted())
        return;

#if VERSION_STRING == Classic
    // TODO
#else
    setMountDisplayId(0);
    removeUnitFlags(UNIT_FLAG_MOUNT);

    if (Player* player = ToPlayer())
    {
        ByteBuffer guidData;
        guidData << GetNewGUID();
        WorldPacket data(SMSG_MOVE_SET_COLLISION_HGT, guidData.size() + 4 + 4);
        data.append(guidData);
        data << uint32_t(Util::getTimeNow());   // Packet counter
        data << getCollisionHeight();
        sendPacket(&data);

        if (player->getMountSpellId() != 0)
        {
            removeAllAurasById(player->getMountSpellId());
            player->setMountSpellId(0);
        }

        //if we had pet then respawn
        if (resummonPet)
            player->summonTemporarilyUnsummonedPet();
    }

    WorldPacket data(SMSG_DISMOUNT, 8);
    data << GetNewGUID();
    sendMessageToSet(&data, true);

#if VERSION_STRING >= WotLK
    // dismount as a vehicle
    if (isPlayer() && getVehicleKit())
    {
        // Send other players that we are no longer a vehicle
        sendMessageToSet(SmsgPlayerVehicleData().serialise().get(), true);
        // Remove vehicle from player
        removeVehicleKit();
    }
#endif

    removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_DISMOUNT);

    // if we have charmed npc, remove stun also
    if (Unit* charm = getWorldMapUnit(getCharmGuid()))
        if (charm->isCreature() && charm->hasUnitFlags(UNIT_FLAG_STUNNED) && !charm->hasUnitStateFlag(UNIT_STATE_STUNNED))
            charm->removeUnitFlags(UNIT_FLAG_STUNNED);
#endif
}

// Returns collisionheight of the unit. If it is 0, it returns DEFAULT_COLLISION_HEIGHT.
float Unit::getCollisionHeight() const
{
    float scaleMod = getScale();

    // Mounted
    if (getMountDisplayId())
    {
        if (const auto* mountDisplayInfo = sObjectMgr.getCreatureDisplayInfoData(getMountDisplayId()))
        {
            if (const auto* mountModelData = mountDisplayInfo->modelInfo)
            {
                const auto* displayInfo = sObjectMgr.getCreatureDisplayInfoData(getNativeDisplayId());
                if (!displayInfo)
                    return DEFAULT_COLLISION_HEIGHT;

                const auto* modelData = displayInfo->modelInfo;
                if (!modelData)
                    return DEFAULT_COLLISION_HEIGHT;

#if VERSION_STRING > Classic
                float const collisionHeight = scaleMod * (mountModelData->MountHeight + modelData->CollisionHeight * displayInfo->creatureModelScale * 0.5f);
#else
                // Do the Collision Calc without Mount height since there are not that many Different Mounts
                float const collisionHeight = scaleMod * (modelData->CollisionHeight * displayInfo->creatureModelScale * 0.5f);
#endif
                return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
            }
        }
    }

    // Dismounted case
    const auto* displayInfo = sObjectMgr.getCreatureDisplayInfoData(getNativeDisplayId());
    if (!displayInfo)
        return DEFAULT_COLLISION_HEIGHT;

    const auto* modelData = displayInfo->modelInfo;
    if (!modelData)
        return DEFAULT_COLLISION_HEIGHT;

    float const collisionHeight = scaleMod * modelData->CollisionHeight * displayInfo->creatureModelScale;
    return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
}

GameObject* Unit::getGameObject(uint32_t spellId) const
{
    for (GameObjectList::const_iterator i = m_gameObj.begin(); i != m_gameObj.end(); ++i)
        if ((*i)->getSpellId() == spellId)
            return *i;

    return nullptr;
}

void Unit::addGameObject(GameObject* gameObj)
{
    if (!gameObj || gameObj->getCreatedByGuid())
        return;

    m_gameObj.push_back(gameObj);
    gameObj->setOwnerGuid(getGuid());
}

void Unit::removeGameObject(GameObject* gameObj, bool del)
{
    if (!gameObj || gameObj->getCreatedByGuid() != getGuid())
        return;

    if (isPlayer())
        ToPlayer()->setSummonedObject(nullptr);

    gameObj->setOwnerGuid(0);

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (m_objectSlots[i] == gameObj->GetUIdFromGUID())
        {
            m_objectSlots[i] = 0;
            break;
        }
    }

    // GO created by some spell
    if (uint32_t spellid = gameObj->getSpellId())
        removeAllAurasById(spellid);

    m_gameObj.remove(gameObj);

    if (del)
    {
        gameObj->setRespawnTime(0);
        gameObj->Delete();
    }
}

void Unit::removeGameObject(uint32_t spellId, bool del)
{
    if (m_gameObj.empty())
        return;

    GameObjectList::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        if (spellId == 0 || (*i)->getSpellId() == spellId)
        {
            (*i)->setOwnerGuid(0);
            if (del)
            {
                (*i)->setRespawnTime(0);
                (*i)->Delete();
            }

            next = m_gameObj.erase(i);
        }
        else
        {
            ++next;
        }
    }
}

void Unit::removeAllGameObjects()
{
    // remove references to unit
    while (!m_gameObj.empty())
    {
        GameObjectList::iterator i = m_gameObj.begin();
        (*i)->setOwnerGuid(0);
        (*i)->setRespawnTime(0);
        (*i)->Delete();
        m_gameObj.erase(i);
    }
}

void Unit::deMorph()
{
    uint32_t displayid = this->getNativeDisplayId();
    this->setDisplayId(displayid);
    eventModelChange();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void Unit::buildMovementPacket(ByteBuffer* data)
{
    *data << static_cast<uint32_t>(getUnitMovementFlags());            // movement flags
#if VERSION_STRING == TBC
    * data << static_cast<uint8_t>(getExtraUnitMovementFlags());        // 2.3.0
#elif VERSION_STRING >= WotLK
    * data << uint16_t(getExtraUnitMovementFlags());       // 3.x.x
#endif
    * data << static_cast<uint32_t>(Util::getMSTime());                 // time / counter
    *data << GetPositionX();
    *data << GetPositionY();
    *data << GetPositionZ();
    *data << GetOrientation();

#if VERSION_STRING < Cata
    // 0x00000200
    if (hasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        if (isPlayer())
        {
            const auto plr = dynamic_cast<Player*>(this);
            if (plr->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT))
            {
                obj_movement_info.transport_guid = plr->obj_movement_info.transport_guid;
            }
        }
#ifdef FT_VEHICLES
        if (Unit* u = getVehicleBase())
            obj_movement_info.transport_guid = u->getGuid();
#endif
        * data << obj_movement_info.transport_guid;
        *data << obj_movement_info.transport_guid;
        *data << GetTransOffsetX();
        *data << GetTransOffsetY();
        *data << GetTransOffsetZ();
        *data << GetTransOffsetO();
        *data << GetTransTime();
#ifdef FT_VEHICLES
        * data << GetTransSeat();

        // TODO what is this in BC?
        if (getExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << getMovementInfo()->transport_time2;
#endif
    }

    // 0x02200000
    if ((getUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (getExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << getMovementInfo()->pitch_rate;

    *data << getMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING < Cata
    if (getUnitMovementFlags() & MOVEFLAG_FALLING)
    {
        *data << getMovementInfo()->jump_info.velocity;
        *data << getMovementInfo()->jump_info.sinAngle;
        *data << getMovementInfo()->jump_info.cosAngle;
        *data << getMovementInfo()->jump_info.xyspeed;
    }

    // 0x04000000
    if (getUnitMovementFlags() & MOVEFLAG_SPLINE_ELEVATION)
        *data << getMovementInfo()->spline_elevation;
#endif
}


void Unit::buildMovementPacket(ByteBuffer* data, float x, float y, float z, float o)
{
    *data << getUnitMovementFlags();            // movement flags
#if VERSION_STRING == TBC
    * data << static_cast<uint8_t>(getExtraUnitMovementFlags());        // 2.3.0
#elif VERSION_STRING >= WotLK
    * data << getExtraUnitMovementFlags();      // 3.x.x
#endif
    * data << Util::getMSTime();                // time / counter
    *data << x;
    *data << y;
    *data << z;
    *data << o;

#if VERSION_STRING < Cata
    // 0x00000200
    if (hasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        *data << obj_movement_info.transport_guid;
        *data << GetTransOffsetX();
        *data << GetTransOffsetY();
        *data << GetTransOffsetZ();
        *data << GetTransOffsetO();
        *data << GetTransTime();
#ifdef FT_VEHICLES
        * data << GetTransSeat();

        if (getExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << getMovementInfo()->transport_time2;
#endif
    }

    // 0x02200000
    if ((getUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (getExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << getMovementInfo()->pitch_rate;

    *data << getMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING < Cata
    if (getUnitMovementFlags() & MOVEFLAG_FALLING)
    {
        *data << getMovementInfo()->jump_info.velocity;
        *data << getMovementInfo()->jump_info.sinAngle;
        *data << getMovementInfo()->jump_info.cosAngle;
        *data << getMovementInfo()->jump_info.xyspeed;
    }

    // 0x04000000
    if (getUnitMovementFlags() & MOVEFLAG_SPLINE_ELEVATION)
        *data << getMovementInfo()->spline_elevation;
#endif
}

void Unit::removeGarbage()
{
    for (auto pet : m_GarbagePets)
        delete pet;

    m_GarbageAuras.clear();
    m_GarbagePets.clear();
}

void Unit::addGarbageAura(std::unique_ptr<Aura> aur)
{
    m_GarbageAuras.push_back(std::move(aur));
}

void Unit::addGarbagePet(Pet* pet)
{
    if (pet->getPlayerOwner()->getGuid() == getGuid() && !pet->IsInWorld())
        m_GarbagePets.push_back(pet);
}

void Unit::possess(Unit* unitTarget, uint32_t delay)
{
    Player* playerController;
    if (isPlayer())
        playerController = dynamic_cast<Player*>(this);
    else // do not support creatures just yet
        return;

    if (!playerController)
        return;

    if (getCharmGuid())
        return;

    setMoveRoot(true);

    if (delay != 0)
    {
        sEventMgr.AddEvent(this, &Unit::possess, unitTarget, static_cast<uint32_t>(0), 0, delay, 1, 0);
        return;
    }
    if (unitTarget == nullptr)
    {
        setMoveRoot(false);
        return;
    }

    playerController->setCharmGuid(unitTarget->getGuid());
    if (unitTarget->isCreature())
    {
        unitTarget->setAItoUse(false);
        unitTarget->stopMoving();
        unitTarget->m_redirectSpellPackets = playerController;
        unitTarget->m_playerControler = playerController;
    }

    m_noInterrupt++;

    setCharmGuid(unitTarget->getGuid());
    unitTarget->setCharmedByGuid(getGuid());
    unitTarget->setCharmTempVal(unitTarget->getFactionTemplate());

    playerController->setFarsightGuid(unitTarget->getGuid());
    playerController->m_controledUnit = unitTarget;

    unitTarget->setFaction(getFactionTemplate());
    unitTarget->addUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);

    addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    playerController->sendClientControlPacket(unitTarget, 1);

    unitTarget->updateInRangeOppositeFactionSet();

    if (!(unitTarget->isPet() && dynamic_cast<Pet*>(unitTarget) == playerController->getPet()))
    {
        if (auto* creatureTarget = unitTarget->ToCreature())
            creatureTarget->sendSpellsToController(playerController, 0);
    }
}

void Unit::unPossess()
{
    Player* playerController;
    if (isPlayer())
        playerController = dynamic_cast<Player*>(this);
    else // creatures no support yet
        return;

    if (!playerController)
        return;

    if (!getCharmGuid())
        return;

    Unit* unitTarget = getWorldMap()->getUnit(getCharmGuid());
    if (!unitTarget)
        return;

    playerController->speedCheatReset();

    if (unitTarget->isCreature())
    {
        unitTarget->setAItoUse(true);
        unitTarget->m_redirectSpellPackets = nullptr;
        unitTarget->m_playerControler = nullptr;
    }

    m_noInterrupt--;
    playerController->setFarsightGuid(0);
    playerController->m_controledUnit = this;

    setCharmGuid(0);
    unitTarget->setCharmedByGuid(0);

    removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    unitTarget->removeUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);
    unitTarget->setFaction(unitTarget->getCharmTempVal());
    unitTarget->updateInRangeOppositeFactionSet();

    playerController->sendClientControlPacket(unitTarget, 0);

    if (!(unitTarget->isPet() && dynamic_cast<Pet*>(unitTarget) == playerController->getPet()))
        playerController->sendEmptyPetSpellList();

    setMoveRoot(false);

    if (!unitTarget->isPet() && (unitTarget->getCreatedByGuid() == getGuid()))
        sEventMgr.AddEvent(static_cast<Object*>(unitTarget), &Object::Delete, 0, 1, 1, 0);
}

void Unit::deactivate(WorldMap* mgr)
{
    if (m_useAI)
        getAIInterface()->enterEvadeMode();

    getCombatHandler().clearCombat();
    Object::deactivate(mgr);
}

float Unit::getChanceToDaze(Unit* target)
{
    if (target->getLevel() < CREATURE_DAZE_MIN_LEVEL) // since 3.3.0
        return 0.0f;

    float attack_skill = getLevel() * 5.0f;
    float defense_skill;

    if (target->isPlayer())
        defense_skill = static_cast<float>(dynamic_cast<Player*>(target)->getSkillLineCurrent(SKILL_DEFENSE, false));
    else
        defense_skill = target->getLevel() * 5.0f;

    if (!defense_skill)
        defense_skill = 1;

    float chance_to_daze = attack_skill * 20 / defense_skill;//if level is equal then we get a 20% chance to daze
    chance_to_daze = chance_to_daze * std::min(target->getLevel() / 30.0f, 1.0f); //for targets below level 30 the chance decreases
    if (chance_to_daze > 40)
        return 40.0f;

    return chance_to_daze;
}

void Unit::eventModelChange()
{
    MySQLStructure::DisplayBoundingBoxes const* displayBoundingBox = sMySQLStore.getDisplayBounding(getDisplayId());

    //\todo if has mount, grab mount model and add the z value of attachment 0
    if (displayBoundingBox != nullptr)
        m_modelHalfSize = displayBoundingBox->high[2] / 2;
    else
        m_modelHalfSize = 1.0f;
}

void Unit::aggroPvPGuards()
{
    for (const auto& inRangeObject : getInRangeObjectsSet())
    {
        if (inRangeObject && inRangeObject->isCreature())
        {
            Unit* inRangeUnit = dynamic_cast<Unit*>(inRangeObject);
            if (inRangeUnit->getAIInterface() && inRangeUnit->getAIInterface()->m_isNeutralGuard && CalcDistance(inRangeUnit) <= 50.0f * 50.0f)
                inRangeUnit->getAIInterface()->onHostileAction(this);
        }
    }
}

void Unit::setTriggerStunOrImmobilize(uint32_t newTrigger, uint32_t newChance, bool isVictim/* = false*/)
{
    if (isVictim == false)
    {
        m_triggerOnStun = newTrigger;
        m_triggerOnStunChance = newChance;
    }
    else
    {
        m_triggerOnStunVictim = newTrigger;
        m_triggerOnStunChanceVictim = newChance;
    }
}

void Unit::eventStunOrImmobilize(Unit* unitProcTarget, bool isVictim)
{
    if (this == unitProcTarget)
        return;

    int32_t t_trigger_on_stun;
    int32_t t_trigger_on_stun_chance;

    if (isVictim == false)
    {
        t_trigger_on_stun = static_cast<int32_t>(m_triggerOnStun);
        t_trigger_on_stun_chance = static_cast<int32_t>(m_triggerOnStunChance);
    }
    else
    {
        t_trigger_on_stun = static_cast<int32_t>(m_triggerOnStunVictim);
        t_trigger_on_stun_chance = static_cast<int32_t>(m_triggerOnStunChanceVictim);
    }

    if (t_trigger_on_stun)
    {
        if (t_trigger_on_stun_chance < 100 && !Util::checkChance(t_trigger_on_stun_chance))
            return;

        const auto spellInfo = sSpellMgr.getSpellInfo(t_trigger_on_stun);
        if (!spellInfo)
            return;

        if (Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr))
        {
            SpellCastTargets targets;

            if (unitProcTarget)
                targets.setUnitTarget(unitProcTarget->getGuid());
            else
                targets.setUnitTarget(getGuid());

            spell->prepare(&targets);
        }
    }
}

void Unit::setTriggerChill(uint32_t newTrigger, uint32_t newChance, bool isVictim/*= false*/)
{
    if (isVictim == false)
    {
        m_triggerOnChill = newTrigger;
        m_triggerOnChillChance = newChance;
    }
    else
    {
        m_triggerOnChillVictim = newTrigger;
        m_triggerOnChillChanceVictim = newChance;
    }
}

void Unit::eventChill(Unit* unitProcTarget, bool isVictim)
{
    if (this == unitProcTarget)
        return;

    int32_t t_trigger_on_chill;
    int32_t t_trigger_on_chill_chance;

    if (isVictim == false)
    {
        t_trigger_on_chill = static_cast<int32_t>(m_triggerOnChill);
        t_trigger_on_chill_chance = static_cast<int32_t>(m_triggerOnChillChance);
    }
    else
    {
        t_trigger_on_chill = static_cast<int32_t>(m_triggerOnChillVictim);
        t_trigger_on_chill_chance = static_cast<int32_t>(m_triggerOnChillChanceVictim);
    }

    if (t_trigger_on_chill)
    {
        if (t_trigger_on_chill_chance < 100 && !Util::checkChance(t_trigger_on_chill_chance))
            return;

        const auto spellInfo = sSpellMgr.getSpellInfo(t_trigger_on_chill);
        if (!spellInfo)
            return;

        if (Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr))
        {
            SpellCastTargets targets;

            if (unitProcTarget)
                targets.setUnitTarget(unitProcTarget->getGuid());
            else
                targets.setUnitTarget(getGuid());

            spell->prepare(&targets);
        }
    }
}

void Unit::removeExtraStrikeTarget(SpellInfo const* spellInfo)
{
    for (auto extraStrikeTarget = m_extraStrikeTargets.begin(); extraStrikeTarget != m_extraStrikeTargets.end(); ++extraStrikeTarget)
    {
        if (spellInfo == (*extraStrikeTarget)->spell_info)
        {
            m_extraStrikeTargetC--;
            m_extraStrikeTargets.erase(extraStrikeTarget);
            break;
        }
    }
}

void Unit::addExtraStrikeTarget(SpellInfo const* spellInfo, uint32_t charges)
{
    for (auto extraStrikeTarget = m_extraStrikeTargets.begin(); extraStrikeTarget != m_extraStrikeTargets.end(); ++extraStrikeTarget)
    {
        //a pointer check or id check ...should be the same
        if (spellInfo == (*extraStrikeTarget)->spell_info)
        {
            (*extraStrikeTarget)->charges = charges;
            return;
        }
    }

    m_extraStrikeTargets.emplace_back(std::make_unique<ExtraStrike>(spellInfo, charges));
    m_extraStrikeTargetC++;
}

uint32_t Unit::doDamageSplitTarget(uint32_t res, SchoolMask schoolMask, bool isMeleeDmg)
{
    Unit* splittarget = (getWorldMap() != nullptr) ? getWorldMap()->getUnit(m_damageSplitTarget->m_target) : nullptr;
    if (splittarget != nullptr && res > 0)
    {
        // calculate damage
        uint32_t tmpsplit = m_damageSplitTarget->m_flatDamageSplit;
        if (tmpsplit > res)
            tmpsplit = res;

        uint32_t splitdamage = tmpsplit;
        res -= tmpsplit;
        tmpsplit = Util::float2int32(m_damageSplitTarget->m_pctDamageSplit * res);
        if (tmpsplit > res)
            tmpsplit = res;

        splitdamage += tmpsplit;
        res -= tmpsplit;

        if (splitdamage)
        {
            splittarget->dealDamage(splittarget, splitdamage, 0);

            if (isMeleeDmg)
            {
                DamageInfo damageInfo;
                damageInfo.fullDamage = splitdamage;
                damageInfo.schoolMask = schoolMask;
                sendAttackerStateUpdate(GetNewGUID(), splittarget->GetNewGUID(), HITSTATUS_NORMALSWING, splitdamage, 0, damageInfo, 0, VisualState::ATTACK, 0, 0);
            }
            else
            {
                uint32_t overKill = 0;
                if (splitdamage > splittarget->getHealth())
                    overKill = splitdamage - splittarget->getHealth();

                splittarget->sendSpellNonMeleeDamageLog(this, splittarget, sSpellMgr.getSpellInfo(m_damageSplitTarget->m_spellId), splitdamage, 0, 0, 0, overKill, false, false);
            }
        }
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////
///Removes and deletes reflects from unit by spell id, does not remove aura which created it
///In specific cases reflects can be created by a dummy spelleffect (eg. spell 28332 or 13043), then we need to remove it in ~unit
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::removeReflect(uint32_t spellId, bool apply)
{
    for (auto reflectSpellSchool = m_reflectSpellSchool.begin(); reflectSpellSchool != m_reflectSpellSchool.end();)
    {
        if (spellId == (*reflectSpellSchool)->spellId)
        {
            reflectSpellSchool = m_reflectSpellSchool.erase(reflectSpellSchool);
        }
        else
        {
            ++reflectSpellSchool;
        }
    }

    if (apply && spellId == 23920 && isPlayer())
    {
        const uint32_t improvedSpellReflection[] =
        {
            //SPELL_HASH_IMPROVED_SPELL_REFLECTION
            59088,
            59089,
            0
        };

        if (hasAurasWithId(improvedSpellReflection))
        {
            const auto player = dynamic_cast<Player*>(this);
            if (auto group = player->getGroup())
            {
                int32_t targets = 0;
                if (player->hasAurasWithId(59088))
                    targets = 2;
                else if (player->hasAurasWithId(59089))
                    targets = 4;

                group->Lock();
                for (uint32_t subGroupNumber = 0; subGroupNumber < group->GetSubGroupCount(); ++subGroupNumber)
                {
                    SubGroup* subGroup = group->GetSubGroup(subGroupNumber);
                    for (auto subGroupMember : subGroup->getGroupMembers())
                    {
                        Player* member = sObjectMgr.getPlayer(subGroupMember->guid);
                        if (member == nullptr || member == player || !member->IsInWorld() || !member->isAlive() || member->hasAurasWithId(59725))
                            continue;

                        if (!member->isInRange(player, 20))
                            continue;

                        player->castSpell(member, 59725, true);
                        targets -= 1;
                    }
                }
                group->Unlock();
            }
        }
    }

    if (!apply && spellId == 59725 && isPlayer())
    {
        const auto player = dynamic_cast<Player*>(this);
        if (auto group = player->getGroup())
        {
            group->Lock();
            for (uint32_t subGroupNumber = 0; subGroupNumber < group->GetSubGroupCount(); ++subGroupNumber)
            {
                for (auto subgroupMember : group->GetSubGroup(subGroupNumber)->getGroupMembers())
                {
                    Player* playerMember = sObjectMgr.getPlayer(subgroupMember->guid);
                    if (playerMember == nullptr)
                        continue;

                    playerMember->removeAllAurasById(59725);
                }
            }
            group->Unlock();
        }
    }
}

void Unit::castOnMeleeSpell()
{
    const auto spellInfo = sSpellMgr.getSpellInfo(getOnMeleeSpell());

    if (Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr))
    {
        spell->extra_cast_number = getOnMeleeSpellEcn();
        SpellCastTargets targets(getTargetGuid());
        spell->prepare(&targets);
        setOnMeleeSpell(0);
    }
}

void Unit::updateAuraForGroup(uint8_t slot)
{
    if (slot >= 64)
        return;

    if (isPlayer())
    {
        const auto player = dynamic_cast<Player*>(this);
        if (player->getGroup())
        {
            player->addGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->setAuraUpdateMaskForRaid(slot);
        }
    }
    else if (Player* owner = getPlayerOwner())
    {
        if (owner->getGroup())
        {
            owner->addGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
            setAuraUpdateMaskForRaid(slot);
        }
    }
}

void Unit::giveGroupXP(Unit* unitVictim, Player* playerInGroup)
{
    if (!playerInGroup || !unitVictim || !playerInGroup->isInGroup())
        return;

    auto group = playerInGroup->getGroup();
    if (!group)
        return;

    Player* pHighLvlPlayer = nullptr;
    uint8_t activePlayerCount = 0;
    Player* activePlayerList[MAX_GROUP_SIZE_RAID];
    uint32_t totalLevel = 0;

    group->Lock();
    for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
    {
        for (auto subGroupMember : group->GetSubGroup(i)->getGroupMembers())
        {
            Player* player = sObjectMgr.getPlayer(subGroupMember->guid);
            if (player && player->isAlive() && unitVictim->getWorldMap() == player->getWorldMap() && player->getDistanceSq(unitVictim) < 100 * 100)
            {
                activePlayerList[activePlayerCount] = player;
                activePlayerCount++;
                totalLevel += player->getLevel();

                if (pHighLvlPlayer)
                {
                    if (player->getLevel() > pHighLvlPlayer->getLevel())
                        pHighLvlPlayer = player;
                }
                else
                {
                    pHighLvlPlayer = player;
                }
            }
        }
    }
    group->Unlock();

    uint32_t xp;
    if (activePlayerCount < 1)
    {
        xp = CalculateXpToGive(unitVictim, playerInGroup);
        playerInGroup->giveXp(xp, unitVictim->getGuid(), true);
    }
    else
    {
        float xpMod = 1.0f;
        if (group->getGroupType() == GROUP_TYPE_PARTY)
        {
            if (activePlayerCount == 3)
                xpMod = 1.1666f;
            else if (activePlayerCount == 4)
                xpMod = 1.3f;
            else if (activePlayerCount == 5)
                xpMod = 1.4f;
            else
                xpMod = 1;
        }
        else if (group->getGroupType() == GROUP_TYPE_RAID)
        {
            xpMod = 0.5f;
        }

        xp = CalculateXpToGive(unitVictim, pHighLvlPlayer);

        for (uint8_t i = 0; i < activePlayerCount; i++)
        {
            Player* plr = activePlayerList[i];
            plr->giveXp(Util::float2int32(static_cast<float>(xp) * static_cast<float>(plr->getLevel()) / static_cast<float>(totalLevel) * xpMod), unitVictim->getGuid(), true);

            activePlayerList[i]->addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);
            if (!sEventMgr.HasEvent(activePlayerList[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
            {
                sEventMgr.AddEvent(static_cast<Unit*>(activePlayerList[i]), &Unit::removeAuraStateAndAuras,
                    AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                sEventMgr.ModifyEventTimeLeft(activePlayerList[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);
            }

            if (plr->getPet() && plr->getPet()->canGainXp())
            {
                const auto petXP = static_cast<uint32_t>(static_cast<float>(CalculateXpToGive(unitVictim, plr->getPet())) * xpMod);
                if (petXP > 0)
                    plr->getPet()->giveXp(petXP);
            }
        }
    }
}

void Unit::calculateResistanceReduction(Unit* unitVictim, DamageInfo* damageInfo, SpellInfo const* spellInfoAbility, float armorPctReduce)
{
    float averageResistance = 0.0f;
    
    if ((*damageInfo).schoolMask == SCHOOL_MASK_NORMAL)
    {
        float armorReduction;

#if VERSION_STRING > TBC
        if (this->isPlayer())
            armorReduction = m_powerCostPctMod[0] + ((float)unitVictim->getResistance(0) * (armorPctReduce + static_cast<Player*>(this)->calcRating(CR_ARMOR_PENETRATION)) / 100.0f);
        else
            armorReduction = 0.0f;
#else
        if (this->isPlayer())
            armorReduction = m_powerCostPctMod[0];
        else
            armorReduction = 0.0f;
#endif

        if (armorReduction >= unitVictim->getResistance(0))
            return;

        double levelReduction = 0;
        if (getLevel() < 60)
            levelReduction = static_cast<double>(unitVictim->getResistance(0) - armorReduction) / static_cast<double>(unitVictim->getResistance(0) + 400 + (85 * getLevel()));
        else if (getLevel() > 59 && getLevel() < DBC_PLAYER_LEVEL_CAP)
            levelReduction = static_cast<double>(unitVictim->getResistance(0) - armorReduction) / static_cast<double>(unitVictim->getResistance(0) - 22167.5 + (467.5 * getLevel()));
        else
            levelReduction = static_cast<double>(unitVictim->getResistance(0) - armorReduction) / static_cast<double>(unitVictim->getResistance(0) + 10557.5);

        if (levelReduction > 0.75f)
            levelReduction = 0.75f;
        else if (levelReduction < 0)
            levelReduction = 0;

        if (levelReduction)
            (*damageInfo).fullDamage = static_cast<uint32_t>((*damageInfo).fullDamage * (1 - levelReduction)); // no multiply by 0
    }
    else
    {
        // applying resistance to other type of damage
        int32_t schoolResistance = Util::float2int32((unitVictim->getResistance((*damageInfo).getSchoolTypeFromMask()) + ((unitVictim->getLevel() > getLevel()) ? (unitVictim->getLevel() - this->getLevel()) * 5 : 0)) - m_powerCostPctMod[(*damageInfo).getSchoolTypeFromMask()]);
        if (schoolResistance < 0)
            schoolResistance = 0;

        averageResistance = (static_cast<float>(schoolResistance) / static_cast<float>(getLevel() * 5) * 0.75f);
        if (averageResistance > 0.75f)
            averageResistance = 0.75f;

        // NOT WOWWIKILIKE but i think it's actually to add some fullresist chance from resistances
        if (!spellInfoAbility || !(spellInfoAbility->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
        {
            float resistChance = static_cast<float>(unitVictim->getResistance((*damageInfo).getSchoolTypeFromMask())) / static_cast<float>(unitVictim->getLevel());
            resistChance *= resistChance;
            if (Util::checkChance(resistChance))
                averageResistance = 1.0f;
        }

        if (averageResistance > 0)
            (*damageInfo).resistedDamage = static_cast<uint32_t>(((*damageInfo).fullDamage) * averageResistance);
        else
            (*damageInfo).resistedDamage = 0;
    }
}

bool Unit::removeAurasByHeal()
{
    bool result = false;
    for (uint16_t x = AuraSlots::TOTAL_SLOT_START; x < AuraSlots::TOTAL_SLOT_END; x++)
    {
        if (auto* const aur = getAuraWithAuraSlot(x))
        {
            switch (aur->getSpellId())
            {
                // remove after heal
                case 35321:
                case 38363:
                case 39215:
                {
                    aur->removeAura();
                    result = true;
                }
                break;
                // remove when healed to 100%
                case 31956:
                case 38801:
                case 43093:
                {
                    if (getHealth() == getMaxHealth())
                    {
                        aur->removeAura();
                        result = true;
                    }
                }
                break;
                // remove at p% health
                case 38772:
                {
                    uint32_t p = aur->getSpellInfo()->getEffectBasePoints(1);
                    if (getMaxHealth() * p <= getHealth() * 100)
                    {
                        aur->removeAura();
                        result = true;
                    }
                }
                break;
            }
        }
    }

    return result;
}

bool Unit::auraActionIf(AuraAction* auraAction, AuraCondition* auraCondition)
{
    bool done = false;

    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        Aura* aura = getAuraWithAuraSlot(i);
        if (aura == nullptr)
            continue;

        if ((*auraCondition)(aura))
        {
            (*auraAction)(aura);
            done = true;
        }
    }

    return done;
}

uint32_t Unit::getManaShieldAbsorbedDamage(uint32_t damage)
{
    if (!m_manashieldAmount)
        return 0;

    uint32_t mana = getPower(POWER_TYPE_MANA);

    int32_t potential = (mana * 50) / 100;
    if (potential > m_manashieldAmount)
        potential = m_manashieldAmount;

    if (static_cast<int32_t>(damage) < potential)
        potential = damage;

    uint32_t cost = (potential * 100) / 50;

    setPower(POWER_TYPE_MANA, mana - cost);

    m_manashieldAmount -= potential;
    if (!m_manashieldAmount)
        removeAllAurasById(m_manaShieldId);

    return potential;
}

AuraCheckResponse Unit::auraCheck(SpellInfo const* spellInfo, Object* /*caster*/)
{
    AuraCheckResponse auraCheckResponse;

    // no error for now
    auraCheckResponse.Error = AURA_CHECK_RESULT_NONE;
    auraCheckResponse.Misc = 0;

    if (!spellInfo->hasSpellRanks())
        return auraCheckResponse;

    // look for spells with same namehash
    for (uint16_t x = AuraSlots::TOTAL_SLOT_START; x < AuraSlots::TOTAL_SLOT_END; x++)
    {
        Aura* aura = getAuraWithAuraSlot(x);
        if (aura != nullptr)
        {
            if (!spellInfo->getRankInfo()->isSpellPartOfThisSpellRankChain(aura->getSpellId()))
                continue;

            // we've got an aura with the same name as the one we're trying to apply
            // but first we check if it has the same effects
            SpellInfo const* aura_sp = aura->getSpellInfo();

            if ((aura_sp->getEffect(0) == spellInfo->getEffect(0) && (aura_sp->getEffect(0) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(0) == spellInfo->getEffectApplyAuraName(0))) &&
                (aura_sp->getEffect(1) == spellInfo->getEffect(1) && (aura_sp->getEffect(1) != SPELL_EFFECT_APPLY_AURA ||
                    aura_sp->getEffectApplyAuraName(1) == spellInfo->getEffectApplyAuraName(1))) &&
                (aura_sp->getEffect(2) == spellInfo->getEffect(2) && (aura_sp->getEffect(2) != SPELL_EFFECT_APPLY_AURA ||
                    aura_sp->getEffectApplyAuraName(2) == spellInfo->getEffectApplyAuraName(2))))
            {
                auraCheckResponse.Misc = aura->getSpellInfo()->getId();

                // compare the rank to our applying spell
                if (aura_sp->getRankInfo()->getRank() > spellInfo->getRankInfo()->getRank())
                {
                    if (spellInfo->getEffect(0) == SPELL_EFFECT_TRIGGER_SPELL ||
                        spellInfo->getEffect(1) == SPELL_EFFECT_TRIGGER_SPELL ||
                        spellInfo->getEffect(2) == SPELL_EFFECT_TRIGGER_SPELL)
                    {
                        auraCheckResponse.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
                    }
                    else
                        auraCheckResponse.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
                }
                else
                    auraCheckResponse.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;

                // we found something, save some loops and exit
                break;
            }
        }
    }
    //sLogger.debug("resp = {}", resp.Error);
    // return it back to our caller
    return auraCheckResponse;
}

AuraCheckResponse Unit::auraCheck(SpellInfo const* spellInfo, Aura* aura, Object* /*caster*/)
{
    AuraCheckResponse auraCheckResponse;
    SpellInfo const* auraSpellInfo = aura->getSpellInfo();

    // no error for now
    auraCheckResponse.Error = AURA_CHECK_RESULT_NONE;
    auraCheckResponse.Misc = 0;

    if (!spellInfo->hasSpellRanks() || !aura->getSpellInfo()->hasSpellRanks())
        return auraCheckResponse;

    if (spellInfo->getRankInfo()->isSpellPartOfThisSpellRankChain(aura->getSpellId()))
    {
        // we've got an aura with the same name as the one we're trying to apply
        // but first we check if it has the same effects
        if ((auraSpellInfo->getEffect(0) == spellInfo->getEffect(0) &&
            (auraSpellInfo->getEffect(0) != SPELL_EFFECT_APPLY_AURA || auraSpellInfo->getEffectApplyAuraName(0) == spellInfo->getEffectApplyAuraName(0))) &&
            (auraSpellInfo->getEffect(1) == spellInfo->getEffect(1) &&
                (auraSpellInfo->getEffect(1) != SPELL_EFFECT_APPLY_AURA || auraSpellInfo->getEffectApplyAuraName(1) == spellInfo->getEffectApplyAuraName(1))) &&
            (auraSpellInfo->getEffect(2) == spellInfo->getEffect(2) &&
                (auraSpellInfo->getEffect(2) != SPELL_EFFECT_APPLY_AURA || auraSpellInfo->getEffectApplyAuraName(2) == spellInfo->getEffectApplyAuraName(2))))
        {
            auraCheckResponse.Misc = aura->getSpellInfo()->getId();

            // compare the rank to our applying spell
            if (aura->getSpellInfo()->getRankInfo()->getRank() > spellInfo->getRankInfo()->getRank())
                auraCheckResponse.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
            else
                auraCheckResponse.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
        }
    }

    // return it back to our caller
    return auraCheckResponse;
}

#ifdef AE_CLASSIC
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f   //60
};
#endif
#ifdef AE_TBC
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f    //70
};
#endif
#ifdef AE_WOTLK
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f,   //70
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f     //80
};
#endif
#ifdef AE_CATA
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f,   //70
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,    //80
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f    // 85
};
#endif
#ifdef AE_MOP
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f,   //70
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,    //80
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f    // 85
};
#endif

uint32_t Unit::getSpellDidHitResult(Unit* pVictim, uint32_t weapon_damage_type, Spell* castingSpell)
{
    Item* it = NULL;
    float hitchance = 0.0f;
    float dodge = 0.0f;
    float parry = 0.0f;
    float block = 0.0f;

    float hitmodifier = 0;
    int32_t self_skill;
    int32_t victim_skill;
    uint16_t SubClassSkill = SKILL_UNARMED;
    const auto ability = castingSpell->getSpellInfo();

    bool backAttack = !pVictim->isInFront(this);   // isInBack is bugged!
    uint32_t vskill = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->isPlayer())
    {
        vskill = static_cast<Player*>(pVictim)->getSkillLineCurrent(SKILL_DEFENSE);
        if (weapon_damage_type != RANGED && !backAttack)                // block chance
        {
            block = static_cast<Player*>(pVictim)->getBlockPercentage(); //shield check already done in Update chances

            if (pVictim->m_stunned <= 0)                                // dodge chance
            {
                dodge = static_cast<Player*>(pVictim)->getDodgePercentage();
            }

            if (pVictim->m_canParry && !pVictim->m_isDisarmed)               // parry chance
            {
                if (static_cast<Player*>(pVictim)->hasSpell(3127) || static_cast<Player*>(pVictim)->hasSpell(18848))
                {
                    parry = static_cast<Player*>(pVictim)->getParryPercentage();
                }
            }
        }
        victim_skill = Util::float2int32(vskill + static_cast<Player*>(pVictim)->calcRating(CR_DEFENSE_SKILL));
    }
    else                                                                // mob defensive chances
    {
        if (weapon_damage_type != RANGED && !backAttack)
            dodge = pVictim->getStat(STAT_AGILITY) / 14.5f;             // what is this value?
        victim_skill = pVictim->getLevel() * 5;

        if (pVictim->isCreature())
        {
            Creature* c = static_cast<Creature*>(pVictim);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            {
                victim_skill = std::max(victim_skill, (static_cast<int32_t>(this->getLevel()) + 3) * 5);       //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Attacker Skill Base Calculation
    if (this->isPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->getHitFromMeleeSpell();

        switch (weapon_damage_type)
        {
        case MELEE:   // melee main hand weapon
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            hitmodifier += pr->calcRating(CR_HIT_MELEE);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_MAINHAND));
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            hitmodifier += pr->calcRating(CR_HIT_MELEE);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_OFFHAND));
            break;
        case RANGED:  // ranged weapon
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            hitmodifier += pr->calcRating(CR_HIT_RANGED);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_RANGED));
            break;
        }

        // erm. some spells don't use ranged weapon skill but are still a ranged spell and use melee stats instead
        // i.e. hammer of wrath
        if (ability)
        {
            switch (ability->getId())
            {
                //SPELL_HASH_HAMMER_OF_WRATH
            case 24239:
            case 24274:
            case 24275:
            case 27180:
            case 32772:
            case 37251:
            case 37255:
            case 37259:
            case 48805:
            case 48806:
            case 51384:
            {
                it = pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                hitmodifier += pr->calcRating(CR_HIT_MELEE);
                self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_MAINHAND));
            } break;
            default:
                break;
            }
        }

        if (it)
            SubClassSkill = it->getRequiredSkill();
        else
            SubClassSkill = SKILL_UNARMED;

        if (SubClassSkill == SKILL_FIST_WEAPONS)
            SubClassSkill = SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if (static_cast<Player*>(this)->isInFeralForm())
        {
            uint8_t form = this->getShapeShiftForm();
            if (form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
#if VERSION_STRING <= Cata
                SubClassSkill = SKILL_FERAL_COMBAT;
#endif
                self_skill += pr->getLevel() * 5;           // Adjust skill for Level * 5 for Feral Combat
            }
        }


        self_skill += pr->getSkillLineCurrent(SubClassSkill);
    }
    else
    {
        self_skill = this->getLevel() * 5;
        if (isCreature())
        {
            Creature* c = static_cast<Creature*>(this);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
                self_skill = std::max(self_skill, (static_cast<int32_t>(pVictim->getLevel()) + 3) * 5);        //used max to avoid situation when lowlvl hits boss.
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Special Chances Base Calculation
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = static_cast<float>(self_skill);
    if (static_cast<int32_t>(pVictim->getLevel() * 5) > victim_skill)
        diffVcapped -= static_cast<float>(victim_skill);
    else
        diffVcapped -= static_cast<float>(pVictim->getLevel() * 5);

    float diffAcapped = static_cast<float>(victim_skill);
    if (static_cast<int32_t>(this->getLevel() * 5) > self_skill)
        diffAcapped -= static_cast<float>(self_skill);
    else
        diffAcapped -= static_cast<float>(getLevel() * 5);
    //<SHIT END>

    // by victim state
    if (pVictim->isPlayer() && pVictim->getStandState()) //every not standing state is>0
    {
        hitchance = 100.0f;
    }

    // by damage type and by weapon type
    if (weapon_damage_type == RANGED)
    {
        dodge = 0.0f;
        parry = 0.0f;
    }

    // by skill difference
    float vsk = static_cast<float>(self_skill) - static_cast<float>(victim_skill);
    dodge = std::max(0.0f, dodge - vsk * 0.04f);

    if (parry)
        parry = std::max(0.0f, parry - vsk * 0.04f);

    if (block)
        block = std::max(0.0f, block - vsk * 0.04f);

    if (vsk > 0)
        hitchance = std::max(hitchance, 95.0f + vsk * 0.02f + hitmodifier);
    else
    {
        if (pVictim->isPlayer())
            hitchance = std::max(hitchance, 95.0f + vsk * 0.1f + hitmodifier);      //wowwiki multiplier - 0.04 but i think 0.1 more balanced
        else
            hitchance = std::max(hitchance, 100.0f + vsk * 0.6f + hitmodifier);     //not wowwiki but more balanced
    }

    if (ability != nullptr && castingSpell != nullptr)
    {
        applySpellModifiers(SPELLMOD_HITCHANCE, &hitchance, ability, castingSpell);
    }

    if (ability && ability->getAttributes() & ATTRIBUTES_CANT_BE_DPB)
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //One Roll Processing
    // cumulative chances generation
    float chances[4];
    chances[0] = std::max(0.0f, 100.0f - hitchance);
    chances[1] = chances[0] + dodge;
    chances[2] = chances[1] + parry;
    chances[3] = chances[2] + block;


    // roll
    float Roll = Util::getRandomFloat(100.0f);
    uint32_t r = 0;

    while (r < 4 && Roll > chances[r])
    {
        r++;
    }

    uint32_t roll_results[5] = { SPELL_DID_HIT_MISS, SPELL_DID_HIT_DODGE, SPELL_DID_HIT_PARRY, SPELL_DID_HIT_BLOCK, SPELL_DID_HIT_SUCCESS };
    return roll_results[r];
}

DamageInfo Unit::strike(Unit* pVictim, WeaponDamageType weaponType, SpellInfo const* ability, int32_t add_damage, int32_t pct_dmg_mod, uint32_t exclusive_damage, bool isSpellTriggered, bool skip_hit_check, bool force_crit, Spell* castingSpell)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //Unacceptable Cases Processing
    if (!pVictim || !pVictim->isAlive() || !isAlive() || isStunned() || isPacified() || isFeared())
        return DamageInfo();

    if (!isInFront(pVictim))
    {
        const auto spellTargetMask = ability != nullptr ? ability->getRequiredTargetMask(true) : 0;
        if (!(ability && ability->getAttributesEx() & ATTRIBUTESEX_IGNORE_IN_FRONT) && !(spellTargetMask & SPELL_TARGET_AREA_MASK))
        {
#if VERSION_STRING < Mop
            if (isPlayer())
                dynamic_cast<Player*>(this)->sendPacket(SmsgAttackSwingBadFacing().serialise().get());
#endif

            return DamageInfo();
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Variables Initialization
    DamageInfo dmg = DamageInfo();
    dmg.weaponType = weaponType;

    Item* it = NULL;

    float hitchance = 0.0f;
    float dodge = 0.0f;
    float parry = 0.0f;
    float glanc = 0.0f;
    float block = 0.0f;
    float crit = 0.0f;
    float crush = 0.0f;

    uint32_t targetEvent = 0;
    uint32_t hit_status = HITSTATUS_NORMALSWING;

    VisualState vstate = VisualState::ATTACK;

    float hitmodifier = 0;
    float ArmorPctReduce = m_ignoreArmorPct;
    int32_t self_skill;
    int32_t victim_skill = 0;
    uint16_t SubClassSkill = SKILL_UNARMED;

    bool backAttack = !pVictim->isInFront(this);
    uint32_t vskill = 0;
    bool disable_dR = false;

    if (ability)
        dmg.schoolMask = static_cast<SchoolMask>(ability->getSchoolMask());
    else
    {
        if (isCreature())
            dmg.schoolMask = static_cast<SchoolMask>(g_spellSchoolConversionTable[static_cast<Creature*>(this)->BaseAttackType]);
        else
            dmg.schoolMask = SCHOOL_MASK_NORMAL;
    }

#if VERSION_STRING >= TBC // support classic
    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->isPlayer())
    {
        Player* plr = static_cast<Player*>(pVictim);
        vskill = plr->getSkillLineCurrent(SKILL_DEFENSE);

        if (!backAttack)
        {
            // not an attack from behind so we may dodge/parry/block

            //uint32_t pClass = plr->getClass();
            //uint32_t pLevel = (getLevel()> DBC_PLAYER_LEVEL_CAP) ? DBC_PLAYER_LEVEL_CAP : getLevel();

            if (dmg.weaponType != RANGED)
            {
                // cannot dodge/parry ranged attacks

                if (pVictim->m_stunned <= 0)
                {
                    // can dodge as long as we're not stunned
                    dodge = plr->getDodgeChance();
                }

                if (pVictim->m_canParry && !m_isDisarmed)
                {
                    // can parry as long as we're not disarmed
                    parry = plr->getParryChance();
                }
            }
            // can block ranged attacks

            // Is an offhand equipped and is it a shield?
            Item* it2 = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            if (it2 != nullptr && it2->getItemProperties()->InventoryType == INVTYPE_SHIELD)
            {
                block = plr->getBlockChance();
            }
        }
        victim_skill = Util::float2int32(vskill + floorf(plr->calcRating(CR_DEFENSE_SKILL)));
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //mob defensive chances
    else
    {
        // not a player, must be a creature
        Creature* c = static_cast<Creature*>(pVictim);

        // mobs can dodge attacks from behind
        if (dmg.weaponType != RANGED && pVictim->m_stunned <= 0)
        {
            dodge = pVictim->getStat(STAT_AGILITY) / 14.5f;
            dodge += pVictim->getDodgeFromSpell();
        }

        if (!backAttack)
        {
            // can parry attacks from the front
            ///\todo different bosses have different parry rates (db patch?)
            if (!m_isDisarmed)    ///\todo this is wrong
            {
                parry = c->GetBaseParry();
                parry += pVictim->getParryFromSpell();
            }

            ///\todo add shield check/block chance here how do we check what the creature has equipped?
        }

        victim_skill = pVictim->getLevel() * 5;
        if (pVictim->isCreature())
        {
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            {
                victim_skill = std::max(victim_skill, (static_cast<int32_t>(getLevel()) + 3) * 5);     //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    //Attacker Skill Base Calculation
    if (this->isPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->getHitFromMeleeSpell();

        switch (dmg.weaponType)
        {
        case MELEE:   // melee main hand weapon
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_MAINHAND));
            if (it)
            {
                dmg.schoolMask = static_cast<SchoolMask>(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
                if (it->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                    ArmorPctReduce += m_ignoreArmorPctMaceSpec;
            }
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_OFFHAND));
            hit_status |= HITSTATUS_DUALWIELD;//animation
            if (it)
            {
                dmg.schoolMask = static_cast<SchoolMask>(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
                if (it->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                    ArmorPctReduce += m_ignoreArmorPctMaceSpec;
            }
            break;
        case RANGED:  // ranged weapon
            it = m_isDisarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            self_skill = Util::float2int32(pr->calcRating(CR_WEAPON_SKILL_RANGED));
            if (it)
                dmg.schoolMask = static_cast<SchoolMask>(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
            break;
        }

        if (it)
        {
            SubClassSkill = it->getRequiredSkill();
            if (SubClassSkill == SKILL_FIST_WEAPONS)
                SubClassSkill = SKILL_UNARMED;
        }
        else
            SubClassSkill = SKILL_UNARMED;


        //chances in feral form don't depend on weapon skill
        if (pr->isInFeralForm())
        {
            uint8_t form = pr->getShapeShiftForm();
            if (form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
#if VERSION_STRING <= Cata
                SubClassSkill = SKILL_FERAL_COMBAT;
#endif
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->getSkillLineCurrent(SubClassSkill);
        crit = static_cast<Player*>(this)->getMeleeCritPercentage();
    }
    else
    {
        self_skill = this->getLevel() * 5;
        if (isCreature())
        {
            Creature* c = static_cast<Creature*>(this);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
                self_skill = std::max(self_skill, (static_cast<int32_t>(pVictim->getLevel()) + 3) * 5);    //used max to avoid situation when lowlvl hits boss.
        }
        crit = 5.0f;        //will be modified later

        if (dmg.weaponType == OFFHAND)
            hit_status |= HITSTATUS_DUALWIELD;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Special Chances Base Calculation

    // crushing blow chance
    //http://www.wowwiki.com/Crushing_blow
    if (pVictim->isPlayer() && !this->isPlayer() && !ability && dmg.schoolMask == SCHOOL_MASK_NORMAL)
    {
        int32_t baseDefense = static_cast<Player*>(pVictim)->getSkillLineCurrent(SKILL_DEFENSE, false);
        int32_t skillDiff = self_skill - baseDefense;
        if (skillDiff >= 15)
            crush = -15.0f + 2.0f * skillDiff;
        else
            crush = 0.0f;
    }

    // glancing blow chance
    //http://www.wowwiki.com/Glancing_blow
    // did my own quick research here, seems base glancing against equal level mob is about 5%
    // and goes up 5% each level. Need to check this further.
    float diffAcapped = victim_skill - std::min(static_cast<float>(self_skill), getLevel() * 5.0f);

    if (this->isPlayer() && !pVictim->isPlayer() && !ability)
    {
        glanc = 5.0f + diffAcapped;

        if (glanc < 0)
            glanc = 0.0f;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Advanced Chances Modifications
    // by talents
    if (pVictim->isPlayer())
    {
        if (dmg.weaponType != RANGED)
        {
            crit += static_cast<Player*>(pVictim)->getResistMCrit();
            hitmodifier += static_cast<Player*>(pVictim)->m_resistHit[MOD_MELEE];
        }
        else
        {
            crit += static_cast<Player*>(pVictim)->getResistRCrit();                 //this could be ability but in that case we overwrite the value
            hitmodifier += static_cast<Player*>(pVictim)->m_resistHit[MOD_RANGED];
        }
    }
    crit += static_cast<float>(pVictim->m_attackerCritChanceMod[0]);

    // by skill difference
    float vsk = static_cast<float>(self_skill - victim_skill);
    dodge = std::max(0.0f, dodge - vsk * 0.04f);

    if (parry)
        parry = std::max(0.0f, parry - vsk * 0.04f);

    if (block)
        block = std::max(0.0f, block - vsk * 0.04f);

    crit += pVictim->isPlayer() ? vsk * 0.04f : std::min(vsk * 0.2f, 0.0f);

    // http://www.wowwiki.com/Miss
    float misschance;
    float ask = -vsk;

    if (pVictim->isPlayer())
    {
        if (ask > 0)
            misschance = ask * 0.04f;
        else
            misschance = ask * 0.02f;
    }
    else
    {
        if (ask <= 10)
            misschance = (ask * 0.1f);
        else
            misschance = (2 + (ask - 10) * 0.4f);
    }
    hitchance = 100.0f - misschance;            // base miss chances are worked out further down

    if (ability != nullptr && castingSpell != nullptr)
    {
        applySpellModifiers(SPELLMOD_CRITICAL, &crit, ability, castingSpell);
        if (!skip_hit_check)
            applySpellModifiers(SPELLMOD_HITCHANCE, &hitchance, ability, castingSpell);
    }

    // by ratings
#if VERSION_STRING >= Cata
    crit -= pVictim->isPlayer() ? static_cast<Player*>(pVictim)->calcRating(CR_RESILIENCE_CRIT_TAKEN) : 0.0f;
#else
    crit -= pVictim->isPlayer() ? static_cast<Player*>(pVictim)->calcRating(CR_CRIT_TAKEN_MELEE) : 0.0f;
#endif

    if (crit < 0)
        crit = 0.0f;

    if (this->isPlayer())
    {
        Player* plr = static_cast<Player*>(this);
        hitmodifier += (dmg.weaponType == RANGED) ? plr->calcRating(CR_HIT_RANGED) : plr->calcRating(CR_HIT_MELEE);

        float expertise_bonus = plr->calcRating(CR_EXPERTISE);
#if VERSION_STRING != Classic
        if (dmg.weaponType == MELEE)
            expertise_bonus += plr->getExpertise();
        else if (dmg.weaponType == OFFHAND)
            expertise_bonus += plr->getOffHandExpertise();
#endif

        dodge -= expertise_bonus;
        if (dodge < 0)
            dodge = 0.0f;

        parry -= expertise_bonus;
        if (parry < 0)
            parry = 0.0f;
    }

    //by aura mods
    //Aura 248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
    dodge += m_CombatResult_Dodge;
    if (dodge < 0)
        dodge = 0.0f;

    //by damage type and by weapon type
    if (dmg.weaponType == RANGED)
    {
        dodge = 0.0f;
        parry = 0.0f;
        glanc = 0.0f;
    }

    if (this->isPlayer())
    {
        it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

        if (!ability && it != nullptr && (it->getItemProperties()->InventoryType == INVTYPE_WEAPON || it->getItemProperties()->InventoryType == INVTYPE_WEAPONOFFHAND))
        {
            // offhand weapon can either be a 1 hander weapon or an offhander weapon
            hitmodifier -= 24.0f;   //dualwield miss chance
        }
        else
        {
            hitmodifier -= 5.0f;    // base miss chance
        }
    }
    else
    {
        hitmodifier -= 5.0f;        // mobs base hit chance
    }

    hitchance += hitmodifier;

    //Hackfix for Surprise Attacks
    if (this->isPlayer() && ability && static_cast<Player*>(this)->m_finishingMovesDodge && ability->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE)
        dodge = 0.0f;

    if (skip_hit_check)
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if (ability != NULL && ability->getAttributes() & ATTRIBUTES_CANT_BE_DPB)
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if (ability && ability->getAttributesExB() & ATTRIBUTESEXB_CANT_CRIT)
        crit = 0.0f;

    // by victim state
    if (pVictim->isPlayer() && pVictim->getStandState())    //every not standing state is>0
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
        crush = 0.0f;
        crit = 100.0f;
    }
    if (backAttack)
    {
        if (pVictim->isPlayer())
        {
            dodge = 0.0f;               //However mobs can dodge attacks from behind
        }
        parry = 0.0f;
        block = 0.0f;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //One Roll Processing
    // cumulative chances generation
    float chances[7];
    chances[0] = std::max(0.0f, 100.0f - hitchance);
    chances[1] = chances[0] + dodge;
    chances[2] = chances[1] + parry;
    chances[3] = chances[2] + glanc;
    chances[4] = chances[3] + block;
    chances[5] = chances[4] + crit;
    chances[6] = chances[5] + crush;

    // roll
    float Roll = Util::getRandomFloat(100.0f);
    uint32_t r = 0;
    while (r < 7 && Roll> chances[r])
    {
        r++;
    }
    if (force_crit)
        r = 5;
    // postroll processing

    //trigger hostile action in ai
    pVictim->getAIInterface()->handleEvent(EVENT_HOSTILEACTION, this, 0);

    switch (r)
    {
    case 0:     // miss
        hit_status |= HITSTATUS_MISS;
        vstate = VisualState::MISS;
        break;
    case 1:     //dodge
        if (pVictim->IsInWorld() && pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetScript())
            static_cast<Creature*>(pVictim)->GetScript()->OnTargetDodged(this);

        if (IsInWorld() && isCreature() && static_cast<Creature*>(this)->GetScript())
            static_cast<Creature*>(this)->GetScript()->OnDodged(this);

        targetEvent = 1;
        vstate = VisualState::DODGE;
        pVictim->emote(EMOTE_ONESHOT_PARRYUNARMED); // Animation

        if (this->isPlayer() && this->getClass() == WARRIOR)
        {
            auto* const playerMe = dynamic_cast<Player*>(this);
            playerMe->addComboPoints(pVictim->getGuid(), 1);

            if (!sEventMgr.HasEvent(playerMe, EVENT_COMBO_POINT_CLEAR_FOR_TARGET))
                sEventMgr.AddEvent(playerMe, &Player::clearComboPoints, (uint32_t)EVENT_COMBO_POINT_CLEAR_FOR_TARGET, static_cast<uint32_t>(5000), static_cast<uint32_t>(1), static_cast<uint32_t>(0));
            else
                sEventMgr.ModifyEventTimeLeft(playerMe, EVENT_COMBO_POINT_CLEAR_FOR_TARGET, 5000, 0);
        }

        // Rune strike
#if VERSION_STRING > TBC
        if (pVictim->isPlayer() && pVictim->getClass() == DEATHKNIGHT)   // omg! dirty hack!
            pVictim->castSpell(pVictim, 56817, true);
#endif

        pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY);
        if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
            sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
        else
            sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 0);
        break;
    case 2:     //parry
        if (pVictim->IsInWorld() && pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetScript())
            static_cast<Creature*>(pVictim)->GetScript()->OnTargetParried(this);

        if (IsInWorld() && isCreature() && static_cast<Creature*>(this)->GetScript())
            static_cast<Creature*>(this)->GetScript()->OnParried(this);

        targetEvent = 3;
        vstate = VisualState::PARRY;
        pVictim->emote(EMOTE_ONESHOT_PARRYUNARMED); // Animation

        if (pVictim->isPlayer())
        {
#if VERSION_STRING > TBC
            // Rune strike
            if (pVictim->getClass() == DEATHKNIGHT) // omg! dirty hack!
                pVictim->castSpell(pVictim, 56817, true);
#endif

            pVictim->addAuraStateAndAuras(AURASTATE_FLAG_PARRY); // SB@L: Enables spells requiring parry
            if (!sEventMgr.HasEvent(pVictim, EVENT_PARRY_FLAG_EXPIRE))
                sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_PARRY, EVENT_PARRY_FLAG_EXPIRE, 5000, 1, 0);
            else
                sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_PARRY_FLAG_EXPIRE, 5000);
            if (pVictim->getClass() == 1 || pVictim->getClass() == 4) // warriors for 'revenge' and rogues for 'riposte'
            {
                pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY); // SB@L: Enables spells requiring dodge
                if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                    sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000);
            }
        }
        break;
        //////////////////////////////////////////////////////////////////////////////////////////
        //not miss,dodge or parry
    default:
        hit_status |= HITSTATUS_HITANIMATION;//hit animation on victim
        if (pVictim->m_schoolImmunityList[0])
            vstate = VisualState::IMMUNE;
        else
        {
            //////////////////////////////////////////////////////////////////////////////////////////
            //state proc initialization
            dmg.victimProcFlags |= PROC_ON_TAKEN_ANY_DAMAGE;
            if (dmg.weaponType == RANGED)
            {
                if (ability != nullptr)
                {
                    dmg.attackerProcFlags |= PROC_ON_DONE_RANGED_SPELL_HIT;
                    dmg.victimProcFlags |= PROC_ON_TAKEN_RANGED_SPELL_HIT;
                }
                else
                {
                    dmg.attackerProcFlags |= PROC_ON_DONE_RANGED_HIT;
                    dmg.victimProcFlags |= PROC_ON_TAKEN_RANGED_HIT;
                }
            }
            else
            {
                if (ability != nullptr)
                {
                    dmg.attackerProcFlags |= PROC_ON_DONE_MELEE_SPELL_HIT;
                    dmg.victimProcFlags |= PROC_ON_TAKEN_MELEE_SPELL_HIT;
                }
                else
                {
                    dmg.attackerProcFlags |= PROC_ON_DONE_MELEE_HIT;
                    dmg.victimProcFlags |= PROC_ON_TAKEN_MELEE_HIT;
                }

                if (dmg.weaponType == OFFHAND)
                {
                    dmg.attackerProcFlags |= PROC_ON_DONE_OFFHAND_ATTACK;
                    dmg.victimProcFlags |= PROC_ON_TAKEN_OFFHAND_ATTACK;
                }
            }
            //////////////////////////////////////////////////////////////////////////////////////////
            //base damage calculation
            if (exclusive_damage)
                dmg.fullDamage = exclusive_damage;
            else
            {
                if (dmg.weaponType == MELEE && ability)
                    dmg.fullDamage = CalculateDamage(this, pVictim, MELEE, ability->getSpellFamilyFlags(), ability);
                else
                    dmg.fullDamage = CalculateDamage(this, pVictim, dmg.weaponType, 0, ability);
            }

            if (pct_dmg_mod > 0)
                dmg.fullDamage = dmg.fullDamage * pct_dmg_mod / 100;

            dmg.fullDamage += add_damage;


            dmg.fullDamage += pVictim->m_damageTakenMod[dmg.getSchoolTypeFromMask()];
            if (dmg.weaponType == RANGED)
            {
                dmg.fullDamage += pVictim->m_rangedDamageTaken;
            }

            if (ability && ability->getMechanicsType() == MECHANIC_BLEEDING)
                disable_dR = true;


            dmg.fullDamage += Util::float2int32(dmg.fullDamage * pVictim->m_damageTakenPctMod[dmg.getSchoolTypeFromMask()]);

            if (dmg.schoolMask != SCHOOL_MASK_NORMAL)
                dmg.fullDamage += Util::float2int32(dmg.fullDamage * (GetDamageDonePctMod(dmg.getSchoolTypeFromMask()) - 1));

            if (ability != NULL)
            {
                switch (ability->getId())
                {
                    //SPELL_HASH_SHRED
                case 3252:
                case 5221:
                case 6800:
                case 8992:
                case 9829:
                case 9830:
                case 27001:
                case 27002:
                case 27555:
                case 48571:
                case 48572:
                case 49121:
                case 49165:
                case 61548:
                case 61549:
                    dmg.fullDamage += Util::float2int32(dmg.fullDamage * pVictim->m_modDamageTakenByMechPct[MECHANIC_BLEEDING]);
                    break;
                }
            }

            if (ability != NULL)
            {
                switch (ability->getId())
                {
                    //SPELL_HASH_MAUL
                case 6807:
                case 6808:
                case 6809:
                case 7092:
                case 8972:
                case 9745:
                case 9880:
                case 9881:
                case 12161:
                case 15793:
                case 17156:
                case 20751:
                case 26996:
                case 27553:
                case 34298:
                case 48479:
                case 48480:
                case 51875:
                case 52506:
                case 54459:
                    dmg.fullDamage += Util::float2int32(dmg.fullDamage * pVictim->m_modDamageTakenByMechPct[MECHANIC_BLEEDING]);
                    break;
                }
            }

#if VERSION_STRING < Cata
            //pet happiness state dmg modifier
            if (isPet() && static_cast<Pet*>(this)->isHunterPet())
                dmg.fullDamage = (dmg.fullDamage <= 0) ? 0 : Util::float2int32(dmg.fullDamage * static_cast<Pet*>(this)->getHappinessDamageMod());
#endif

            if (dmg.fullDamage < 0)
                dmg.fullDamage = 0;
            //////////////////////////////////////////////////////////////////////////////////////////
            //check for special hits
            switch (r)
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //glancing blow
            case 3:
            {
                float low_dmg_mod = 1.5f - (0.05f * diffAcapped);
                if (low_dmg_mod < 0.01)
                    low_dmg_mod = 0.01f;
                if (low_dmg_mod > 0.91)
                    low_dmg_mod = 0.91f;
                float high_dmg_mod = 1.2f - (0.03f * diffAcapped);
                if (high_dmg_mod > 0.99)
                    high_dmg_mod = 0.99f;
                if (high_dmg_mod < 0.2)
                    high_dmg_mod = 0.2f;

                float damage_reduction = (high_dmg_mod + low_dmg_mod) / 2.0f;
                if (damage_reduction > 0)
                {
                    dmg.fullDamage = Util::float2int32(damage_reduction * dmg.fullDamage);
                }
                hit_status |= HITSTATUS_GLANCING;
            }
            break;
            //////////////////////////////////////////////////////////////////////////////////////////
            //block
            case 4:
            {
                Item* shield = static_cast<Player*>(pVictim)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                if (shield != nullptr)
                {
                    targetEvent = 2;
                    pVictim->emote(EMOTE_ONESHOT_PARRYSHIELD);// Animation

                    if (shield->getItemProperties()->InventoryType == INVTYPE_SHIELD)
                    {
                        float block_multiplier = (100.0f + static_cast<Player*>(pVictim)->m_modBlockAbsorbValue) / 100.0f;
                        if (block_multiplier < 1.0f)block_multiplier = 1.0f;

                        dmg.blockedDamage = Util::float2int32((shield->getItemProperties()->Block + ((static_cast<Player*>(pVictim)->m_modBlockValueFromSpells + static_cast<Player*>(pVictim)->getCombatRating(CR_BLOCK))) + ((pVictim->getStat(STAT_STRENGTH) / 2.0f) - 1.0f)) * block_multiplier);

                        if (Util::checkChance(m_blockModPct))
                            dmg.blockedDamage *= 2;
                    }
                    else
                    {
                        dmg.blockedDamage = 0;
                    }

                    if (dmg.fullDamage <= static_cast<int32_t>(dmg.blockedDamage))
                        vstate = VisualState::BLOCK;
                    if (dmg.blockedDamage)
                    {
                        if (pVictim->IsInWorld() && pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetScript())
                            static_cast<Creature*>(pVictim)->GetScript()->OnTargetBlocked(this, dmg.blockedDamage);

                        if (IsInWorld() && isCreature() && static_cast<Creature*>(this)->GetScript())
                            static_cast<Creature*>(this)->GetScript()->OnBlocked(pVictim, dmg.blockedDamage);
                    }
                    if (pVictim->isPlayer())  //not necessary now but we'll have blocking mobs in future
                    {
                        pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY); //SB@L: Enables spells requiring dodge
                        if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                            sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
                        else
                            sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000);
                    }
                }
            }
            break;
            //////////////////////////////////////////////////////////////////////////////////////////
            //critical hit
            case 5:
            {
                hit_status |= HITSTATUS_CRICTICAL;
                //LogDebug("DEBUG: Critical Strike! Full_damage: %u" , dmg.full_damage);
                if (ability != nullptr && castingSpell != nullptr)
                {
                    applySpellModifiers(SPELLMOD_CRITICAL_DAMAGE, &dmg.fullDamage, ability, castingSpell);
                }

                //LogDebug("DEBUG: After m_critMeleeDamageTakenPctMod: %u" , dmg.full_damage);
                if (isPlayer())
                {
                    if (dmg.weaponType != RANGED)
                    {
                        dmg.fullDamage += dmg.fullDamage * static_cast<Player*>(this)->m_modPhysCritDmgPct / 100;
                    }
                    if (!pVictim->isPlayer())
                        dmg.fullDamage += Util::float2int32(dmg.fullDamage * static_cast<Player*>(this)->m_increaseCricticalByTypePct[static_cast<Creature*>(pVictim)->GetCreatureProperties()->Type]);
                    //LogDebug("DEBUG: After IncreaseCricticalByTypePCT: %u" , dmg.full_damage);
                }

                if (dmg.weaponType == RANGED)
                    dmg.fullDamage = dmg.fullDamage - Util::float2int32(dmg.fullDamage * m_critRangedDamageTakenPctMod[dmg.getSchoolTypeFromMask()]);
                else
                    dmg.fullDamage = dmg.fullDamage - Util::float2int32(dmg.fullDamage * m_critMeleeDamageTakenPctMod[dmg.getSchoolTypeFromMask()]);

                if (pVictim->isPlayer())
                {
                    //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
#if VERSION_STRING >= Cata
                    float dmg_reduction_pct = 2.0f * static_cast<Player*>(pVictim)->calcRating(CR_RESILIENCE_CRIT_TAKEN) / 100.0f;
#else
                    float dmg_reduction_pct = 2.0f * static_cast<Player*>(pVictim)->calcRating(CR_CRIT_TAKEN_MELEE) / 100.0f;
#endif
                    if (dmg_reduction_pct > 1.0f)
                        dmg_reduction_pct = 1.0f; //we cannot resist more then he is criticalling us, there is no point of the critical then :P
                    dmg.fullDamage = Util::float2int32(dmg.fullDamage - dmg.fullDamage * dmg_reduction_pct);
                    //LogDebug("DEBUG: After Resilience check: %u" , dmg.full_damage);
                }

                if (pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetCreatureProperties()->Rank != ELITE_WORLDBOSS)
                    pVictim->emote(EMOTE_ONESHOT_WOUNDCRITICAL);

                if (pVictim->IsInWorld() && pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetScript())
                    static_cast<Creature*>(pVictim)->GetScript()->OnTargetCritHit(this, dmg.fullDamage);

                if (IsInWorld() && isCreature() && static_cast<Creature*>(this)->GetScript())
                    static_cast<Creature*>(this)->GetScript()->OnCritHit(pVictim, dmg.fullDamage);
            }
            break;
            //////////////////////////////////////////////////////////////////////////////////////////
            //crushing blow
            case 6:
                hit_status |= HITSTATUS_CRUSHINGBLOW;
                dmg.fullDamage = (dmg.fullDamage * 3) >> 1;
                break;
                //////////////////////////////////////////////////////////////////////////////////////////
                //regular hit
            default:
                break;
            }
            //////////////////////////////////////////////////////////////////////////////////////////
            //Post Roll Damage Processing
            //////////////////////////////////////////////////////////////////////////////////////////
            //absorption
            uint32_t dm = dmg.fullDamage;
            dmg.absorbedDamage = pVictim->absorbDamage(dmg.schoolMask, &dm);

            if (dmg.fullDamage > static_cast<int32_t>(dmg.blockedDamage))
            {
                uint32_t sh = pVictim->getManaShieldAbsorbedDamage(dmg.fullDamage);
                //////////////////////////////////////////////////////////////////////////////////////////
                //armor reducing
                if (sh)
                {
                    dmg.fullDamage -= sh;
                    if (dmg.fullDamage && !disable_dR)
                        calculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
                    dmg.fullDamage += sh;
                    dmg.absorbedDamage += sh;
                }
                else if (!disable_dR)
                    calculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
            }

            if (dmg.schoolMask == SCHOOL_MASK_NORMAL)
            {
                dmg.absorbedDamage += dmg.resistedDamage;
                dmg.resistedDamage = 0;
            }

            int32_t realdamage = dmg.fullDamage - dmg.absorbedDamage - dmg.resistedDamage - dmg.blockedDamage;
            if (realdamage < 0)
            {
                realdamage = 0;
                if (!(hit_status & HITSTATUS_BLOCK))
                    hit_status |= HITSTATUS_ABSORBED;
                else
                    hit_status |= HITSTATUS_BLOCK;
            }
            dmg.realDamage = realdamage;
            if (IsInWorld() && isCreature() && static_cast<Creature*>(this)->GetScript())
                static_cast<Creature*>(this)->GetScript()->OnHit(pVictim, static_cast<float>(realdamage));
        }
        break;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Roll Special Cases Processing
    //////////////////////////////////////////////////////////////////////////////////////////
    // Special Effects Processing
    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if (pVictim->m_damageSplitTarget)
    {
        dmg.fullDamage = pVictim->doDamageSplitTarget(dmg.fullDamage, dmg.schoolMask, true);
        dmg.realDamage = dmg.fullDamage;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //special states processing
    if (pVictim->isCreature())
    {
        if (pVictim->isInEvadeMode())
            /*      || (pVictim->getAIInterface()->GetIsSoulLinked() && pVictim->getAIInterface()->getSoullinkedWith() != this))*/
        {
            vstate = VisualState::EVADE;
            dmg.realDamage = 0;
            dmg.fullDamage = 0;
            dmg.resistedDamage = 0;
        }
    }
    if (pVictim->isPlayer() && static_cast<Player*>(pVictim)->m_cheats.hasGodModeCheat == true)
    {
        dmg.resistedDamage = dmg.fullDamage; //godmode
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //dirty fixes
    //vstate=1-wound,2-dodge,3-parry,4-interrupt,5-block,6-evade,7-immune,8-deflect
    // the above code was remade it for reasons : damage shield needs moslty same flags as handleproc + dual wield should proc too ?

    //damage shield must come before handleproc to not loose 1 charge : spell gets removed before last charge
    if (dmg.realDamage > 0 && dmg.weaponType != OFFHAND)
    {
        pVictim->handleProcDmgShield(dmg.victimProcFlags, this);
        handleProcDmgShield(dmg.attackerProcFlags, pVictim);
    }

    dmg.isCritical = hit_status & HITSTATUS_CRICTICAL;

    /*if (resisted_dmg)
    {
        dmg.resistedDamage += resisted_dmg;
        dmg.fullDamage -= resisted_dmg;
        dmg.realDamage = dmg.realDamage - resisted_dmg < 0 ? 0 : dmg.realDamage - resisted_dmg;
    }*/
    //////////////////////////////////////////////////////////////////////////////////////////
    //spells triggering
    if (dmg.realDamage > 0 && ability == 0)
    {
        //ugly hack for shadowfiend restoring mana
        if (getSummonedByGuid() != 0 && getEntry() == 19668)
        {
            Player* owner = getWorldMap()->getPlayer(static_cast<uint32_t>(getSummonedByGuid()));
            if (owner)
            {
                uint32_t amount = static_cast<uint32_t>(owner->getMaxPower(POWER_TYPE_MANA) * 0.05f);
                this->energize(owner, 34650, amount, POWER_TYPE_MANA);
            }
        }
        //ugly hack for Bloodsworm restoring hp
        if (getSummonedByGuid() != 0 && getEntry() == 28017)
        {
            Player* owner = getWorldMap()->getPlayer(static_cast<uint32_t>(getSummonedByGuid()));
            if (owner != NULL)
                owner->addSimpleHealingBatchEvent(Util::float2int32(1.5f * dmg.realDamage), owner, sSpellMgr.getSpellInfo(50452));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Data Sending
    if (!ability)
    {
        if (dmg.fullDamage > 0)
        {
#if VERSION_STRING >= WotLK
            if (dmg.fullDamage == (int32_t)dmg.absorbedDamage)
                hit_status |= HITSTATUS_ABSORB_FULL;
            else if (dmg.absorbedDamage > 0)
                hit_status |= HITSTATUS_ABSORB_PARTIAL;
#else
            if (dmg.absorbedDamage > 0)
                hit_status |= HITSTATUS_ABSORBED;
#endif

            if (dmg.fullDamage <= static_cast<int32_t>(dmg.resistedDamage))
            {
                hit_status |= HITSTATUS_RESIST;
                dmg.resistedDamage = dmg.fullDamage;
            }
        }

        if (dmg.fullDamage < 0)
            dmg.fullDamage = 0;
    }

    if (this != pVictim && vstate != VisualState::EVADE)
    {
        if (castingSpell == nullptr)
        {
            // Send initial threat
            if (pVictim->isCreature())
                pVictim->getAIInterface()->onHostileAction(this);

            // Handle combat for both caster and victim if normal attack
            // For spells this is handled in spell system
            getCombatHandler().onHostileAction(pVictim);
            pVictim->getCombatHandler().takeCombatAction(this);
        }

        // Add real threat
        if (pVictim->getThreatManager().canHaveThreatList())
        {
            const auto threat = dmg.realDamage == 0 ? 1 : dmg.realDamage;
            const auto spellInfo = castingSpell != nullptr ? castingSpell->getSpellInfo() : ability;
            pVictim->getThreatManager().addThreat(this, static_cast<float>(threat), spellInfo, false, false, castingSpell);
        }
    }

    if (ability && dmg.realDamage == 0)
    {
        auto logSent = true;
        switch (vstate)
        {
        case VisualState::MISS:
            SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_MISS);
            break;
        case VisualState::DODGE:
            SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_DODGE);
            break;
        case VisualState::PARRY:
            SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_PARRY);
            break;
        case VisualState::EVADE:
            SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_EVADE);
            break;
        case VisualState::IMMUNE:
            SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_IMMUNE);
            break;
        default:
            logSent = false;
            break;
        }

        if (logSent)
            return dmg;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Damage Dealing

    if (this->isPlayer() && ability)
        static_cast<Player*>(this)->m_castedAmount[dmg.getSchoolTypeFromMask()] = dmg.realDamage + dmg.absorbedDamage;

    // Generate rage on damage done
    ///\ todo: this is inaccurate and almost directly copied here from few lines below
    uint32_t rageGenerated = 0;
    if (dmg.fullDamage > 0 && isPlayer() && getPowerType() == POWER_TYPE_RAGE && ability == nullptr)
    {
        float_t val = 0.0f;
        uint32_t level = getLevel();
        float_t conv = 0.0f;
        if (level <= DBC_PLAYER_LEVEL_CAP)
            conv = AttackToRageConversionTable[level];
        else
            conv = 3.75f / (0.0091107836f * level * level + 3.225598133f * level + 4.2652911f);

        // Hit Factor
        float_t f = (dmg.weaponType == OFFHAND) ? 1.75f : 3.5f;

        if (hit_status & HITSTATUS_CRICTICAL)
            f *= 2.0f;

        float_t s = 1.0f;

        // Weapon speed (normal)
        const auto weapon = (static_cast<Player*>(this)->getItemInterface())->GetInventoryItem(INVENTORY_SLOT_NOT_SET, (dmg.weaponType == OFFHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND));
        if (weapon == nullptr)
        {
            if (dmg.weaponType == OFFHAND)
                s = getBaseAttackTime(OFFHAND) / 1000.0f;
            else
                s = getBaseAttackTime(MELEE) / 1000.0f;
        }
        else
        {
            const auto entry = weapon->getEntry();
            const auto pProto = sMySQLStore.getItemProperties(entry);
            if (pProto != nullptr)
            {
                s = pProto->Delay / 1000.0f;
            }
        }

        val = conv * dmg.fullDamage + f * s / 2.0f;
        val *= (1 + (static_cast<Player*>(this)->m_rageFromDamageDealt / 100.0f));
        const auto ragerate = worldConfig.getFloatRate(RATE_POWER2);
        val *= 10 * ragerate;

        if (val > 0)
        {
            rageGenerated = static_cast<uint32_t>(std::ceil(val));
#if VERSION_STRING > TBC
            hit_status |= HITSTATUS_RAGE_GAIN;
#endif
        }
    }

    // Calculate estimated overkill based on current health and current health events in health batch
    const auto overKill = pVictim->calculateEstimatedOverKillForCombatLog(dmg.realDamage);
    if (ability == nullptr)
        sendAttackerStateUpdate(GetNewGUID(), pVictim->GetNewGUID(), static_cast<HitStatus>(hit_status), dmg.realDamage, overKill, dmg, dmg.absorbedDamage, vstate, dmg.blockedDamage, rageGenerated);
    else if (dmg.fullDamage > 0)
        pVictim->sendSpellNonMeleeDamageLog(this, pVictim, ability, dmg.realDamage, dmg.absorbedDamage, dmg.resistedDamage, dmg.blockedDamage, overKill, false, hit_status & HITSTATUS_CRICTICAL);

    // invincible people don't take damage
    if (pVictim->m_isInvincible == false)
    {
        if (dmg.realDamage)
        {
            auto batch = std::make_unique<HealthBatchEvent>();
            batch->caster = this;
            batch->damageInfo = dmg;
            batch->spellInfo = ability;

            pVictim->addHealthBatchEvent(std::move(batch));
            //pVictim->HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,this);
            // HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,pVictim);

            if (pVictim->isCastingSpell())
            {
                if (pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
                    pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->AddTime(0);
                else if (pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->getCastTimeLeft() > 0)
                    pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->AddTime(0);
            }
        }
    }

    // Tagging should happen when damage packets are sent
    const auto plrOwner = getPlayerOwnerOrSelf();
    if (plrOwner != nullptr && pVictim->isCreature() && pVictim->isTaggableFor(this))
    {
        pVictim->setTaggerGuid(this);
        plrOwner->tagUnit(pVictim);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Damage Dealing Processing
    //////////////////////////////////////////////////////////////////////////////////////////
    // proc handling

    // If called from spell class, handle caster's procs when spell has finished all targets
    if (castingSpell == nullptr)
        handleProc(dmg.attackerProcFlags, pVictim, ability, dmg, isSpellTriggered);

    pVictim->handleProc(dmg.victimProcFlags, this, ability, dmg, isSpellTriggered);

    //durability processing
    if (pVictim->isPlayer())
    {
        static_cast<Player*>(pVictim)->getItemInterface()->ReduceItemDurability();
        if (!this->isPlayer())
        {
            Player* pr = static_cast<Player*>(pVictim);
            if (Util::checkChance(pr->getSkillUpChance(SKILL_DEFENSE) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->advanceSkillLine(SKILL_DEFENSE, static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));
#if VERSION_STRING >= TBC // support classic
                pr->updateChances();
#endif
            }
        }
        else
        {
            static_cast<Player*>(this)->getItemInterface()->ReduceItemDurability();
        }
    }
    else
    {
        if (this->isPlayer())//not pvp
        {
            static_cast<Player*>(this)->getItemInterface()->ReduceItemDurability();
            Player* pr = static_cast<Player*>(this);
            if (Util::checkChance(pr->getSkillUpChance(SubClassSkill) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->advanceSkillLine(SubClassSkill, static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));
                //pr->UpdateChances();
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //rage processing
    //http://www.wowwiki.com/Formulas:Rage_generation

    if (rageGenerated > 0)
    {
        modPower(POWER_TYPE_RAGE, static_cast<int32_t>(rageGenerated));
        if (getPower(POWER_TYPE_RAGE) > 1000)
            modPower(POWER_TYPE_RAGE, 1000 - getPower(POWER_TYPE_RAGE));
    }

    removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);
    //////////////////////////////////////////////////////////////////////////////////////////
    //extra strikes processing
    if (!m_extraAttackCounter)
    {
        int32_t extra_attacks = m_extraAttacks;
        m_extraAttackCounter = true;
        m_extraAttacks = 0;

        while (extra_attacks > 0)
        {
            extra_attacks--;
            strike(pVictim, dmg.weaponType, NULL, 0, 0, 0, false, false);
        }

        m_extraAttackCounter = false;
    }

    if (m_extraStrikeTargetC > 0 && !m_extraStrikeTarget)
    {
        m_extraStrikeTarget = true;

        for (auto itx = m_extraStrikeTargets.begin(); itx != m_extraStrikeTargets.end();)
        {
            auto itx2 = itx++;
            const auto& ex = *itx2;

            for (const auto& itr : getInRangeObjectsSet())
            {
                if (!itr || itr == pVictim || !itr->isCreatureOrPlayer())
                    continue;

                if (CalcDistance(itr) < 5.0f && this->isValidTarget(itr) && itr->isInFront(this) && !static_cast<Unit*>(itr)->isPacified())
                {
                    // Sweeping Strikes hits cannot be dodged, missed or parried (from wowhead)
                    bool skip_hit_check2 = ex->spell_info->getId() == 12328 ? true : false;
                    //zack : should we use the spell id the registered this extra strike when striking ? It would solve a few proc on proc problems if so ;)
                    // Strike(TO<Unit*>(*itr), weapon_damage_type, ability, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check);
                    strike(static_cast<Unit*>(itr), dmg.weaponType, ex->spell_info, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check2);
                    break;
                }
            }

            // Sweeping Strikes charges are used up regardless whether there is a secondary target in range or not. (from wowhead)
            if (ex->charges > 0)
            {
                ex->charges--;
                if (ex->charges <= 0)
                {
                    m_extraStrikeTargetC--;
                    m_extraStrikeTargets.erase(itx2);
                }
            }
        }

        m_extraStrikeTarget = false;
    }

    return dmg;
}

uint32_t Unit::handleProc(uint32_t flag, Unit* victim, SpellInfo const* CastingSpell, DamageInfo damageInfo, bool isSpellTriggered, ProcEvents procEvent/* = PROC_EVENT_DO_ALL*/, Aura* triggeredFromAura/* = nullptr*/)
{
    if (flag == 0)
        return 0;

    uint32_t resisted_dmg = 0;
    bool can_delete = !m_isProcInUse; //if this is a nested proc then we should have this set to TRUE by the father proc
    m_isProcInUse = true; //locking the proc list

    std::list<SpellProc*> happenedProcs;

    for (auto itr = m_procSpells.begin(); itr != m_procSpells.end();)    // Proc Trigger Spells for Victim
    {
        auto itr2 = itr++;
        SpellProc* spell_proc = itr2->get();

        // Check if list item was deleted elsewhere, so here it's removed and freed
        if (spell_proc->isDeleted())
        {
            if (can_delete)
            {
                m_procSpells.erase(itr2);
            }
            continue;
        }

        // APGL End
        // MIT Start

        // Check if spell proc is marked to skip this call
        if (spell_proc->isSkippingHandleProc())
        {
            spell_proc->skipOnNextHandleProc(false);
            continue;
        }

        if (CastingSpell != nullptr)
        {
            // A spell cannot proc itself
            if (CastingSpell->getId() == spell_proc->getSpell()->getId())
                continue;

            // Check proc class mask
            if (!spell_proc->checkClassMask(CastingSpell))
                continue;
        }

        if (procEvent == PROC_EVENT_DO_CASTER_PROCS_ONLY && !spell_proc->isCastedOnProcOwner())
            continue;

        if (procEvent == PROC_EVENT_DO_TARGET_PROCS_ONLY && spell_proc->isCastedOnProcOwner())
            continue;

        // Check proc flags
        if (!sScriptMgr.callScriptedSpellCheckProcFlags(spell_proc, static_cast<SpellProcFlags>(flag)))
            continue;

        // Check extra proc flags
        if (!spell_proc->checkExtraProcFlags(this, damageInfo))
            continue;

        // Check if this proc can happen
        if (!sScriptMgr.callScriptedSpellCanProc(spell_proc, victim, CastingSpell, damageInfo))
            continue;
        if (!spell_proc->canProc(victim, CastingSpell))
            continue;

        if (CastingSpell != nullptr)
        {
            // Check if this proc can trigger on already triggered spell
            // by default procs can't
            if (isSpellTriggered && !sScriptMgr.callScriptedSpellCanProcOnTriggered(spell_proc, victim, CastingSpell, triggeredFromAura))
                continue;
        }

        const auto spe = spell_proc->getSpell();
        // Spell id which is going to proc
        auto spellId = spe->getId();

        // Get spellinfo of the spell that created this proc
        uint32_t origId = 0;
        if (spell_proc->getOriginalSpell() != nullptr)
            origId = spell_proc->getOriginalSpell()->getId();

        // No need to check if exists or not since we were not able to register this trigger if it would not exist :P
        const auto ospinfo = spell_proc->getOriginalSpell();

        auto proc_Chance = sScriptMgr.callScriptedSpellCalcProcChance(spell_proc, victim, CastingSpell);

        // Check if spell proc uses procs-per-minute system
        if (isPlayer())
        {
            // Procs-per-minute, or PPM, amount describes how many procs (on average) can occur in one minute
            // To calculate Proc-chance-Per-Hit, or PPH, formula is:
            // unmodified weapon speed * PPM / 60
            auto ppmAmount = spell_proc->getProcsPerMinute();
            const auto plr = static_cast<Player*>(this);

            // Old hackfixes
            switch (spellId)
            {
                //SPELL_HASH_BLACK_TEMPLE_MELEE_TRINKET
            case 40475:
                ppmAmount = 1.0f;
                break;
                // SPELL_HASH_MAGTHERIDON_MELEE_TRINKET:
            case 34774:
                ppmAmount = 1.5f;
                break;                          // dragonspine trophy
                // SPELL_HASH_ROMULO_S_POISON:
            case 34586:
            case 34587:
                ppmAmount = 1.5f;
                break;                          // romulo's
                // SPELL_HASH_FROSTBRAND_ATTACK:
            case 8034:
            case 8037:
            case 10458:
            case 16352:
            case 16353:
            case 25501:
            case 38617:
            case 54609:
            case 58797:
            case 58798:
            case 58799:
            case 64186:
                ppmAmount = 9.0f;
                break;                          // Frostbrand Weapon
            case 16870:
                ppmAmount = 2.0f;
                break; //druid: clearcasting
            default:
                break;
            }

            // Default value is 0.0
            if (ppmAmount != 0.0f)
            {
                // Unarmed speed is 2 sec
                uint32_t weaponSpeed = 2000;
                if (plr->isInFeralForm())
                {
#if VERSION_STRING > Classic
                    // Get shapeshift form's attack speed
                    const auto form = sSpellShapeshiftFormStore.lookupEntry(plr->getShapeShiftForm());
                    if (form != nullptr && form->AttackSpeed != 0)
                        weaponSpeed = form->AttackSpeed;
#endif
                }
                else
                {
                    switch (damageInfo.weaponType)
                    {
                    case MELEE:
                    {
                        const auto mainHand = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                        if (mainHand != nullptr && mainHand->isWeapon())
                            weaponSpeed = mainHand->getItemProperties()->Delay;
                    } break;
                    case OFFHAND:
                    {
                        const auto offHand = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                        if (offHand != nullptr && offHand->isWeapon())
                            weaponSpeed = offHand->getItemProperties()->Delay;
                    } break;
                    case RANGED:
                    {
                        const auto ranged = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                        if (ranged != nullptr && ranged->isWeapon())
                            weaponSpeed = ranged->getItemProperties()->Delay;
                    } break;
                    default:
                        break;
                    }
                }

                proc_Chance = Util::float2int32((weaponSpeed * 0.001f * ppmAmount / 60.0f) * 100.0f);
            }
        }

        // Apply modifiers to proc chance
        // todo: this will not use spell charges
        applySpellModifiers(SPELLMOD_TRIGGER, &proc_Chance, ospinfo);

        if (!Util::checkChance(proc_Chance))
            continue;

        // Check if proc has interval
        if (spell_proc->getProcInterval() > 0)
        {
            // Check for cooldown cheat
            if (!(spell_proc->getProcOwner()->isPlayer() && static_cast<Player*>(spell_proc->getProcOwner())->m_cheats.hasCooldownCheat))
            {
                const auto timeNow = Util::getMSTime();
                if (spell_proc->getLastTriggerTime() + spell_proc->getProcInterval() > timeNow)
                    continue;

                spell_proc->setLastTriggerTime(timeNow);
            }
        }

        // MIT End
        // APGL Start

#if VERSION_STRING >= TBC
        // SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
        for (uint8_t i = 0; i < 3; ++i)
        {
            if (ospinfo && ospinfo->getEffectApplyAuraName(i) == SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE)
            {
                spell_proc->setOverrideEffectDamage(i, ospinfo->getEffectBasePoints(i) + 1);
                sScriptMgr.callScriptedSpellProcDoEffect(spell_proc, victim, CastingSpell, damageInfo);
                spell_proc->doEffect(victim, CastingSpell, flag, damageInfo.realDamage, damageInfo.absorbedDamage, damageInfo.weaponType);
            }
        }
#endif

        // give spell_proc a chance to handle the effect
        const auto scriptResult = sScriptMgr.callScriptedSpellProcDoEffect(spell_proc, victim, CastingSpell, damageInfo);
        if (scriptResult == SpellScriptExecuteState::EXECUTE_PREVENT)
            continue;
        if (spell_proc->doEffect(victim, CastingSpell, flag, damageInfo.realDamage, damageInfo.absorbedDamage, damageInfo.weaponType))
            continue;

        //these are player talents. Fuckem they pull the emu speed down
        if (isPlayer())
        {
            uint32_t talentlevel = 0;
            switch (origId)
            {
                //mace specialization
            case 12284:
            {talentlevel = 1; }
            break;
            case 12701:
            {talentlevel = 2; }
            break;
            case 12702:
            {talentlevel = 3; }
            break;
            case 12703:
            {talentlevel = 4; }
            break;
            case 12704:
            {talentlevel = 5; }
            break;

            //Unbridled Wrath
            case 12332:
            {talentlevel = 1; }
            break;
            case 12999:
            {talentlevel = 2; }
            break;
            case 13000:
            {talentlevel = 3; }
            break;
            case 13001:
            {talentlevel = 4; }
            break;
            case 13002:
            {talentlevel = 5; }
            break;
            }

            switch (spellId)
            {
            case 32747:     //Deadly Throw Interrupt (rogue arena gloves set)
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                case 26679:
                case 37074:
                case 48673:
                case 48674:
                case 52885:
                case 59180:
                case 64499:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 16959:     //Druid - Primal Fury Proc
            {
                if (!isPlayer())
                    continue;
                Player* p = static_cast<Player*>(this);
                if (p->getShapeShiftForm() != FORM_BEAR && p->getShapeShiftForm() != FORM_DIREBEAR)
                    continue;
            }
            break;
            case 16953:     //Druid - Blood Frenzy Proc
            {
                if (!isPlayer() || !CastingSpell)
                    continue;

                Player* p = static_cast<Player*>(this);
                if (p->getShapeShiftForm() != FORM_CAT)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SHRED
                case 3252:
                case 5221:
                case 6800:
                case 8992:
                case 9829:
                case 9830:
                case 27001:
                case 27002:
                case 27555:
                case 48571:
                case 48572:
                case 49121:
                case 49165:
                case 61548:
                case 61549:
                    //SPELL_HASH_RAVAGE
                case 3242:
                case 3446:
                case 6785:
                case 6787:
                case 8391:
                case 9866:
                case 9867:
                case 24213:
                case 24333:
                case 27005:
                case 29906:
                case 33781:
                case 48578:
                case 48579:
                case 50518:
                case 53558:
                case 53559:
                case 53560:
                case 53561:
                case 53562:
                    //SPELL_HASH_CLAW
                case 1082:
                case 2975:
                case 2976:
                case 2977:
                case 2980:
                case 2981:
                case 2982:
                case 3009:
                case 3010:
                case 3029:
                case 3666:
                case 3667:
                case 5201:
                case 9849:
                case 9850:
                case 16827:
                case 16828:
                case 16829:
                case 16830:
                case 16831:
                case 16832:
                case 24187:
                case 27000:
                case 27049:
                case 27347:
                case 31289:
                case 47468:
                case 48569:
                case 48570:
                case 51772:
                case 52471:
                case 52472:
                case 62225:
                case 67774:
                case 67793:
                case 67879:
                case 67980:
                case 67981:
                case 67982:
                case 75159:
                    //SPELL_HASH_RAKE
                case 1822:
                case 1823:
                case 1824:
                case 9904:
                case 24331:
                case 24332:
                case 27003:
                case 27556:
                case 27638:
                case 36332:
                case 48573:
                case 48574:
                case 53499:
                case 54668:
                case 59881:
                case 59882:
                case 59883:
                case 59884:
                case 59885:
                case 59886:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 14189: //Seal Fate
            {
                if (!this->isPlayer() || !CastingSpell || CastingSpell->getId() == 14189 || CastingSpell->getId() == 16953 || CastingSpell->getId() == 16959)
                    continue;
                if (CastingSpell->getEffect(0) != SPELL_EFFECT_ADD_COMBO_POINTS && CastingSpell->getEffect(1) != SPELL_EFFECT_ADD_COMBO_POINTS &&
                    CastingSpell->getEffect(2) != SPELL_EFFECT_ADD_COMBO_POINTS)
                {
                    switch (CastingSpell->getId())
                    {
                    case 33876:
                    case 33982:
                    case 33983:
                    case 48565:
                    case 48566:
                        break;
                    default:
                        continue;
                    }
                }
            }
            break;
            case 17106: //druid intensity
            {
                if (CastingSpell == NULL)
                    continue;
                if (CastingSpell->getId() != 5229)  //enrage
                    continue;
            }
            break;
            case 31616: //Nature's Guardian
            {
                //yep, another special case: Nature's grace
                if (getHealthPct() > 30)
                    continue;
            }
            break;
            case 37309: //Bloodlust
            {
                if (!this->isPlayer())
                    continue;
                if (this->getShapeShiftForm() != FORM_BEAR &&
                    this->getShapeShiftForm() != FORM_DIREBEAR)
                    continue;
            }
            break;
            case 37310://Bloodlust
            {
                if (!this->isPlayer() || this->getShapeShiftForm() != FORM_CAT)
                    continue;
            }
            break;
            case 16459:
            {
                //sword specialization
                Item* item_mainhand = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                Item* item_offhand = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                uint16_t reqskillMH = 0;
                uint16_t reqskillOH = 0;

                if (item_mainhand != nullptr)
                    reqskillMH = item_mainhand->getRequiredSkill();

                if (item_offhand != nullptr)
                    reqskillOH = item_offhand->getRequiredSkill();

                if (reqskillMH != SKILL_SWORDS && reqskillMH != SKILL_2H_SWORDS && reqskillOH != SKILL_SWORDS && reqskillOH != SKILL_2H_SWORDS)
                    continue;
            }
            break;
            case 12721:
            {
                //deep wound requires a melee weapon
                auto item = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (item)
                {
                    //class 2 means weapons ;)
                    if (item->getItemProperties()->Class != 2)
                        continue;
                }
                else continue; //no weapon no joy
            }
            break;
            //Warrior - Sword and Board
            case 50227:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REVENGE
                case 6572:
                case 6574:
                case 7379:
                case 11600:
                case 11601:
                case 12170:
                case 19130:
                case 25269:
                case 25288:
                case 28844:
                case 30357:
                case 37517:
                case 40392:
                case 57823:
                    //SPELL_HASH_DEVASTATE
                case 20243:
                case 30016:
                case 30017:
                case 30022:
                case 36891:
                case 36894:
                case 38849:
                case 38967:
                case 44452:
                case 47497:
                case 47498:
                case 57795:
                case 60018:
                case 62317:
                case 69902:
                    break;
                default:
                    continue;
                }
            } break;

            //Warrior - Safeguard
            case 46946:
            case 46947:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_INTERVENE
                case 3411:
                case 34784:
                case 41198:
                case 53476:
                case 59667:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Warrior - Taste for Blood
            case 60503:
            {
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REND
                case 772:
                case 6546:
                case 6547:
                case 6548:
                case 11572:
                case 11573:
                case 11574:
                case 11977:
                case 12054:
                case 13318:
                case 13443:
                case 13445:
                case 13738:
                case 14087:
                case 14118:
                case 16393:
                case 16403:
                case 16406:
                case 16509:
                case 17153:
                case 17504:
                case 18075:
                case 18078:
                case 18106:
                case 18200:
                case 18202:
                case 21949:
                case 25208:
                case 29574:
                case 29578:
                case 36965:
                case 36991:
                case 37662:
                case 43246:
                case 43931:
                case 46845:
                case 47465:
                case 48880:
                case 53317:
                case 54703:
                case 54708:
                case 59239:
                case 59343:
                case 59691:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Warrior - Unbridled Wrath
            case 12964:
            {
                //let's recalc chance to cast since we have a full 100 all time on this one
                Item* it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (it == nullptr)
                    continue; //no weapon no joy
                //float chance=float(it->GetProto()->Delay)*float(talentlevel)/600.0f;
                uint32_t chance = it->getItemProperties()->Delay * talentlevel / 300; //zack this had a very low proc rate. Kinda like a wasted talent
                uint32_t myroll = Util::getRandomUInt(100);
                if (myroll > chance)
                    continue;
            }
            break;
            //Warrior - Gag Order
            case 18498:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SHIELD_BASH
                case 72:
                case 1671:
                case 1672:
                case 11972:
                case 29704:
                case 33871:
                case 35178:
                case 36988:
                case 38233:
                case 41180:
                case 41197:
                case 70964:
                case 72194:
                case 72196:
                    //SPELL_HASH_HEROIC_THROW
                case 57755:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Warrior - Bloodsurge
            case 46916:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                case 23881:
                    //SPELL_HASH_WHIRLWIND
                case 1680:
                case 8989:
                case 9633:
                case 13736:
                case 15576:
                case 15577:
                case 15578:
                case 15589:
                case 17207:
                case 24236:
                case 26038:
                case 26083:
                case 26084:
                case 26686:
                case 28334:
                case 28335:
                case 29573:
                case 29851:
                case 29852:
                case 31737:
                case 31738:
                case 31909:
                case 31910:
                case 33238:
                case 33239:
                case 33500:
                case 36132:
                case 36142:
                case 36175:
                case 36981:
                case 36982:
                case 37582:
                case 37583:
                case 37640:
                case 37641:
                case 37704:
                case 38618:
                case 38619:
                case 39232:
                case 40236:
                case 40653:
                case 40654:
                case 41056:
                case 41057:
                case 41058:
                case 41059:
                case 41061:
                case 41097:
                case 41098:
                case 41194:
                case 41195:
                case 41399:
                case 41400:
                case 43442:
                case 44949:
                case 45895:
                case 45896:
                case 46270:
                case 46271:
                case 48280:
                case 48281:
                case 49807:
                case 50228:
                case 50229:
                case 50622:
                case 52027:
                case 52028:
                case 52977:
                case 54797:
                case 55266:
                case 55267:
                case 55463:
                case 55977:
                case 56408:
                case 59322:
                case 59323:
                case 59549:
                case 59550:
                case 61076:
                case 61078:
                case 61136:
                case 61137:
                case 61139:
                case 63805:
                case 63806:
                case 63807:
                case 63808:
                case 65510:
                case 67037:
                case 67716:
                    //SPELL_HASH_HEROIC_STRIKE
                case 78:
                case 284:
                case 285:
                case 1608:
                case 11564:
                case 11565:
                case 11566:
                case 11567:
                case 25286:
                case 25710:
                case 25712:
                case 29426:
                case 29567:
                case 29707:
                case 30324:
                case 31827:
                case 41975:
                case 45026:
                case 47449:
                case 47450:
                case 52221:
                case 53395:
                case 57846:
                case 59035:
                case 59607:
                case 62444:
                case 69566:
                    break;
                default:
                    continue;
                }
            } break;

            ////////////////////////////////////////////////////////////////////////////
            // Mage ignite talent only for fire dmg
            case 12654:
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE))
                    continue;
                const auto spellInfo = sSpellMgr.getSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                auto spell_duration = sSpellDurationStore.lookupEntry(spellInfo->getDurationIndex());
                uint32_t tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);

                if (ospinfo)
                    spell_proc->setOverrideEffectDamage(0, ospinfo->getEffectBasePoints(0) * damageInfo.realDamage / (100 * tickcount));
            }
            break;
            //druid - Primal Fury
            case 37116:
            case 37117:
            {
                if (!this->isPlayer())
                    continue;
                Player* mPlayer = static_cast<Player*>(this);
                if (!mPlayer->isInFeralForm() ||
                    (mPlayer->getShapeShiftForm() != FORM_CAT &&
                        mPlayer->getShapeShiftForm() != FORM_BEAR &&
                        mPlayer->getShapeShiftForm() != FORM_DIREBEAR))
                    continue;
            }
            break;
            //rogue - blade twisting
            case 31125:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SINISTER_STRIKE
                case 1752:
                case 1757:
                case 1758:
                case 1759:
                case 1760:
                case 8621:
                case 11293:
                case 11294:
                case 14873:
                case 15581:
                case 15667:
                case 19472:
                case 26861:
                case 26862:
                case 46558:
                case 48637:
                case 48638:
                case 57640:
                case 59409:
                case 60195:
                case 69920:
                case 71145:
                    //SPELL_HASH_SHIV
                case 5938:
                case 5940:
                    //SPELL_HASH_BACKSTAB
                case 53:
                case 2589:
                case 2590:
                case 2591:
                case 7159:
                case 8721:
                case 11279:
                case 11280:
                case 11281:
                case 15582:
                case 15657:
                case 22416:
                case 25300:
                case 26863:
                case 30992:
                case 34614:
                case 37685:
                case 48656:
                case 48657:
                case 52540:
                case 58471:
                case 63754:
                case 71410:
                case 72427:
                    //SPELL_HASH_GOUGE
                case 1776:
                case 1777:
                case 8629:
                case 11285:
                case 11286:
                case 12540:
                case 13579:
                case 24698:
                case 28456:
                case 29425:
                case 34940:
                case 36862:
                case 38764:
                case 38863:
                    break;
                default:
                    continue;
                }
            }
            break;
            //priest - Grace
            case 47930:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_PENANCE
                case 47540:
                case 47666:
                case 47750:
                case 47757:
                case 47758:
                case 52983:
                case 52984:
                case 52985:
                case 52986:
                case 52987:
                case 52988:
                case 52998:
                case 52999:
                case 53000:
                case 53001:
                case 53002:
                case 53003:
                case 53005:
                case 53006:
                case 53007:
                case 54518:
                case 54520:
                case 66097:
                case 66098:
                case 68029:
                case 68030:
                case 68031:
                case 69905:
                case 69906:
                case 71137:
                case 71138:
                case 71139:
                    //SPELL_HASH_FLASH_HEAL
                case 2061:
                case 9472:
                case 9473:
                case 9474:
                case 10915:
                case 10916:
                case 10917:
                case 17137:
                case 17138:
                case 17843:
                case 25233:
                case 25235:
                case 27608:
                case 38588:
                case 42420:
                case 43431:
                case 43516:
                case 43575:
                case 48070:
                case 48071:
                case 56331:
                case 56919:
                case 66104:
                case 68023:
                case 68024:
                case 68025:
                case 71595:
                case 71782:
                case 71783:
                    //SPELL_HASH_GREATER_HEAL
                case 2060:
                case 10963:
                case 10964:
                case 10965:
                case 22009:
                case 25210:
                case 25213:
                case 25314:
                case 28809:
                case 29564:
                case 34119:
                case 35096:
                case 38580:
                case 41378:
                case 48062:
                case 48063:
                case 49348:
                case 57775:
                case 60003:
                case 61965:
                case 62334:
                case 62442:
                case 63760:
                case 69963:
                case 71131:
                case 71931:
                    break;
                default:
                    continue;
                }
            }
            break;
            //warlock - Improved Shadow Bolt
            case 17794:
            case 17798:
            case 17797:
            case 17799:
            case 17800:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SHADOW_BOLT
                case 686:
                case 695:
                case 705:
                case 1088:
                case 1106:
                case 7617:
                case 7619:
                case 7641:
                case 9613:
                case 11659:
                case 11660:
                case 11661:
                case 12471:
                case 12739:
                case 13440:
                case 13480:
                case 14106:
                case 14122:
                case 15232:
                case 15472:
                case 15537:
                case 16408:
                case 16409:
                case 16410:
                case 16783:
                case 16784:
                case 17393:
                case 17434:
                case 17435:
                case 17483:
                case 17509:
                case 18111:
                case 18138:
                case 18164:
                case 18205:
                case 18211:
                case 18214:
                case 18217:
                case 19728:
                case 19729:
                case 20298:
                case 20791:
                case 20807:
                case 20816:
                case 20825:
                case 21077:
                case 21141:
                case 22336:
                case 22677:
                case 24668:
                case 25307:
                case 26006:
                case 27209:
                case 29317:
                case 29487:
                case 29626:
                case 29640:
                case 29927:
                case 30055:
                case 30505:
                case 30686:
                case 31618:
                case 31627:
                case 32666:
                case 32860:
                case 33335:
                case 34344:
                case 36714:
                case 36868:
                case 36972:
                case 36986:
                case 36987:
                case 38378:
                case 38386:
                case 38628:
                case 38825:
                case 38892:
                case 39025:
                case 39026:
                case 39297:
                case 39309:
                case 40185:
                case 41069:
                case 41280:
                case 41957:
                case 43330:
                case 43649:
                case 43667:
                case 45055:
                case 45679:
                case 45680:
                case 47076:
                case 47248:
                case 47808:
                case 47809:
                case 49084:
                case 50455:
                case 51363:
                case 51432:
                case 51608:
                case 52257:
                case 52534:
                case 53086:
                case 53333:
                case 54113:
                case 55984:
                case 56405:
                case 57374:
                case 57464:
                case 57644:
                case 57725:
                case 58827:
                case 59016:
                case 59246:
                case 59254:
                case 59351:
                case 59357:
                case 59389:
                case 59575:
                case 60015:
                case 61558:
                case 61562:
                case 65821:
                case 68151:
                case 68152:
                case 68153:
                case 69028:
                case 69068:
                case 69211:
                case 69212:
                case 69387:
                case 69577:
                case 69972:
                case 70043:
                case 70080:
                case 70182:
                case 70208:
                case 70270:
                case 70386:
                case 70387:
                case 71143:
                case 71254:
                case 71296:
                case 71297:
                case 71936:
                case 72008:
                case 72503:
                case 72504:
                case 72901:
                case 72960:
                case 72961:
                case 75330:
                case 75331:
                case 75384:
                    break;
                default:
                    continue;
                }
            } break;

            // warlock - Seed of Corruption
            case 27285:
            {
                bool can_proc_now = false;
                //if we proced on spell tick
                if (flag & PROC_ON_DONE_PERIODIC)
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SEED_OF_CORRUPTION
                    case 27243:
                    case 27285:
                    case 32863:
                    case 32865:
                    case 36123:
                    case 37826:
                    case 38252:
                    case 39367:
                    case 43991:
                    case 44141:
                    case 47831:
                    case 47832:
                    case 47833:
                    case 47834:
                    case 47835:
                    case 47836:
                    case 70388:
                        break;
                    default:
                        continue;
                    }

                    //this spell builds up n time
                    if (ospinfo && damageInfo.realDamage < this->getHealth())    //if this is not a killer blow
                        can_proc_now = true;
                }
                else can_proc_now = true; //target died
                if (can_proc_now == false)
                    continue;
                Unit* new_caster = victim;
                if (new_caster && new_caster->isAlive())
                {
                    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                    Spell* spell = sSpellMgr.newSpell(new_caster, spellInfo, true, NULL);
                    SpellCastTargets targets;
                    targets.setDestination(GetPosition());
                    spell->prepare(&targets);
                }
                spell_proc->deleteProc();
                continue;
            }
            break;
            // warlock - Improved Drain Soul
            case 18371:
            {
                if (!CastingSpell)
                    continue;

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_DRAIN_SOUL
                case 1120:
                case 8288:
                case 8289:
                case 11675:
                case 18371:
                case 27217:
                case 32862:
                case 35839:
                case 47855:
                case 60452:
                    break;
                default:
                    continue;
                }

                //null check was made before like 2 times already :P
                if (ospinfo)
                    spell_proc->setOverrideEffectDamage(0, (ospinfo->calculateEffectValue(2)) * getMaxPower(POWER_TYPE_MANA) / 100);
            }
            break;
            // warlock - Unstable Affliction
            case 31117:
            {
                //null check was made before like 2 times already :P
                if (ospinfo)
                    spell_proc->setOverrideEffectDamage(0, (ospinfo->calculateEffectValue(0)) * 9);
            }
            break;

            //warlock - Nighfall
            case 17941:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_CORRUPTION
                case 172:
                case 6222:
                case 6223:
                case 7648:
                case 11671:
                case 11672:
                case 13530:
                case 16402:
                case 16985:
                case 17510:
                case 18088:
                case 18376:
                case 18656:
                case 21068:
                case 23642:
                case 25311:
                case 27216:
                case 28829:
                case 30938:
                case 31405:
                case 32063:
                case 32197:
                case 37113:
                case 37961:
                case 39212:
                case 39621:
                case 41988:
                case 47782:
                case 47812:
                case 47813:
                case 56898:
                case 57645:
                case 58811:
                case 60016:
                case 61563:
                case 65810:
                case 68133:
                case 68134:
                case 68135:
                case 70602:
                case 70904:
                case 71937:
                    //SPELL_HASH_DRAIN_LIFE
                case 689:
                case 699:
                case 709:
                case 7651:
                case 11699:
                case 11700:
                case 12693:
                case 16375:
                case 16414:
                case 16608:
                case 17173:
                case 17238:
                case 17620:
                case 18084:
                case 18557:
                case 18815:
                case 18817:
                case 20743:
                case 21170:
                case 24300:
                case 24435:
                case 24585:
                case 24618:
                case 26693:
                case 27219:
                case 27220:
                case 27994:
                case 29155:
                case 30412:
                case 34107:
                case 34696:
                case 35748:
                case 36224:
                case 36655:
                case 36825:
                case 37992:
                case 38817:
                case 39676:
                case 43417:
                case 44294:
                case 46155:
                case 46291:
                case 46466:
                case 47857:
                case 55646:
                case 64159:
                case 64160:
                case 69066:
                case 70213:
                case 71838:
                case 71839:
                    break;
                default:
                    continue;
                }
            }
            break;
            //warlock - Shadow Embrace
            case 32386:
            case 32388:
            case 32389:
            case 32390:
            case 32391:
            {
                if (CastingSpell == NULL)
                    continue;
                else
                {
                    switch (CastingSpell->getId())
                    {
                    case 184:       //Corruption
                        //SPELL_HASH_CURSE_OF_AGONY
                    case 980:
                    case 1014:
                    case 6217:
                    case 11711:
                    case 11712:
                    case 11713:
                    case 14868:
                    case 14875:
                    case 17771:
                    case 18266:
                    case 18671:
                    case 27218:
                    case 29930:
                    case 32418:
                    case 37334:
                    case 39672:
                    case 46190:
                    case 47863:
                    case 47864:
                    case 65814:
                    case 68136:
                    case 68137:
                    case 68138:
                    case 69404:
                    case 70391:
                    case 71112:
                        //SPELL_HASH_SIPHON_LIFE
                    case 35195:
                    case 41597:
                    case 63106:
                    case 63108:
                        //SPELL_HASH_SEED_OF_CORRUPTION
                    case 27243:
                    case 27285:
                    case 32863:
                    case 32865:
                    case 36123:
                    case 37826:
                    case 38252:
                    case 39367:
                    case 43991:
                    case 44141:
                    case 47831:
                    case 47832:
                    case 47833:
                    case 47834:
                    case 47835:
                    case 47836:
                    case 70388:
                        break;
                    default:
                        continue;
                    }
                }
            }
            break;
#if VERSION_STRING <= WotLK
            //warlock - Aftermath
            case 18118:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                auto _continue = false;
                const auto spellSkillRange = sSpellMgr.getSkillEntryRangeForSpell(CastingSpell->getId());
                for (const auto& [_, skill_line_ability] : spellSkillRange)
                {
                    if (skill_line_ability->skilline != SKILL_DESTRUCTION)
                    {
                        _continue = true;
                        break;
                    }
                }

                if (_continue)
                    continue;
            }
            break;
#endif
            //warlock - Nether Protection
            case 30300:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
                if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE) &&
                    !(CastingSpell->getSchoolMask() & SCHOOL_MASK_SHADOW))
                    continue;
            }
            break;
            //warlock - Soul Leech - this whole spell should get rewritten. Uses bad formulas, bad trigger method, spell is rewritten ...
            case 30294:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere
                //only trigger effect for specified spells
                uint32_t amount;

                switch (CastingSpell->getId())
                {
                    // SPELL_HASH_SHADOW_BOLT: //Shadow Bolt
                case 686:
                case 695:
                case 705:
                case 1088:
                case 1106:
                case 7617:
                case 7619:
                case 7641:
                case 9613:
                case 11659:
                case 11660:
                case 11661:
                case 12471:
                case 12739:
                case 13440:
                case 13480:
                case 14106:
                case 14122:
                case 15232:
                case 15472:
                case 15537:
                case 16408:
                case 16409:
                case 16410:
                case 16783:
                case 16784:
                case 17393:
                case 17434:
                case 17435:
                case 17483:
                case 17509:
                case 18111:
                case 18138:
                case 18164:
                case 18205:
                case 18211:
                case 18214:
                case 18217:
                case 19728:
                case 19729:
                case 20298:
                case 20791:
                case 20807:
                case 20816:
                case 20825:
                case 21077:
                case 21141:
                case 22336:
                case 22677:
                case 24668:
                case 25307:
                case 26006:
                case 27209:
                case 29317:
                case 29487:
                case 29626:
                case 29640:
                case 29927:
                case 30055:
                case 30505:
                case 30686:
                case 31618:
                case 31627:
                case 32666:
                case 32860:
                case 33335:
                case 34344:
                case 36714:
                case 36868:
                case 36972:
                case 36986:
                case 36987:
                case 38378:
                case 38386:
                case 38628:
                case 38825:
                case 38892:
                case 39025:
                case 39026:
                case 39297:
                case 39309:
                case 40185:
                case 41069:
                case 41280:
                case 41957:
                case 43330:
                case 43649:
                case 43667:
                case 45055:
                case 45679:
                case 45680:
                case 47076:
                case 47248:
                case 47808:
                case 47809:
                case 49084:
                case 50455:
                case 51363:
                case 51432:
                case 51608:
                case 52257:
                case 52534:
                case 53086:
                case 53333:
                case 54113:
                case 55984:
                case 56405:
                case 57374:
                case 57464:
                case 57644:
                case 57725:
                case 58827:
                case 59016:
                case 59246:
                case 59254:
                case 59351:
                case 59357:
                case 59389:
                case 59575:
                case 60015:
                case 61558:
                case 61562:
                case 65821:
                case 68151:
                case 68152:
                case 68153:
                case 69028:
                case 69068:
                case 69211:
                case 69212:
                case 69387:
                case 69577:
                case 69972:
                case 70043:
                case 70080:
                case 70182:
                case 70208:
                case 70270:
                case 70386:
                case 70387:
                case 71143:
                case 71254:
                case 71296:
                case 71297:
                case 71936:
                case 72008:
                case 72503:
                case 72504:
                case 72901:
                case 72960:
                case 72961:
                case 75330:
                case 75331:
                case 75384:
                    // SPELL_HASH_SOUL_FIRE: //Soul Fire
                case 6353:
                case 17924:
                case 27211:
                case 30545:
                case 47824:
                case 47825:
                    // SPELL_HASH_INCINERATE: //Incinerate
                case 19397:
                case 23308:
                case 23309:
                case 29722:
                case 32231:
                case 32707:
                case 36832:
                case 38401:
                case 38918:
                case 39083:
                case 40239:
                case 41960:
                case 43971:
                case 44519:
                case 46043:
                case 47837:
                case 47838:
                case 53493:
                case 69973:
                case 71135:
                    // SPELL_HASH_SEARING_PAIN: //Searing Pain
                case 5676:
                case 17919:
                case 17920:
                case 17921:
                case 17922:
                case 17923:
                case 27210:
                case 29492:
                case 30358:
                case 30459:
                case 47814:
                case 47815:
                case 65819:
                case 68148:
                case 68149:
                case 68150:
                    // SPELL_HASH_CONFLAGRATE: //Conflagrate
                case 17962:
                    // SPELL_HASH_CHAOS_BOLT: //Chaos Bolt
                case 50796:
                case 51287:
                case 59170:
                case 59171:
                case 59172:
                case 69576:
                case 71108:
                {
                    amount = CastingSpell->calculateEffectValue(0);
                } break;

                //SPELL_HASH_SHADOWBURN
                case 17877:
                case 18867:
                case 18868:
                case 18869:
                case 18870:
                case 18871:
                case 27263:
                case 29341:
                case 30546:
                case 47826:
                case 47827:
                {
                    amount = CastingSpell->calculateEffectValue(1);
                } break;
                default:
                    amount = 0;

                }

                if (!amount)
                    continue;

                const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
                Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
                spell->setUnitTarget(this);
                if (ospinfo)
                    doSpellHealing(this, spellId, amount * (ospinfo->calculateEffectValue(0)) / 100.0f, true);
                delete spell;
                spell = NULL;
                continue;
            }
            break;
            //warlock - pyroclasm
            case 18093:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_RAIN_OF_FIRE
                case 4629:
                case 5740:
                case 6219:
                case 11677:
                case 11678:
                case 11990:
                case 16005:
                case 19474:
                case 19717:
                case 20754:
                case 24669:
                case 27212:
                case 28794:
                case 31340:
                case 31598:
                case 33508:
                case 33617:
                case 33627:
                case 33972:
                case 34169:
                case 34185:
                case 34360:
                case 34435:
                case 36808:
                case 37279:
                case 37465:
                case 38635:
                case 38741:
                case 39024:
                case 39273:
                case 39363:
                case 39376:
                case 42023:
                case 42218:
                case 42223:
                case 42224:
                case 42225:
                case 42226:
                case 42227:
                case 43440:
                case 47817:
                case 47818:
                case 47819:
                case 47820:
                case 49518:
                case 54099:
                case 54210:
                case 57757:
                case 58936:
                case 59971:
                case 69670:
                    //SPELL_HASH_HELLFIRE_EFFECT
                case 5857:
                case 11681:
                case 11682:
                case 27214:
                case 30860:
                case 47822:
                case 65817:
                case 68142:
                case 68143:
                case 68144:
                case 69585:
                case 70284:
                    //SPELL_HASH_SOUL_FIRE
                case 6353:
                case 17924:
                case 27211:
                case 30545:
                case 47824:
                case 47825:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 54274:
            case 54276:
            case 54277:
            {
                if (CastingSpell == NULL)
                    continue;

                if (CastingSpell->getId() != 17962)
                    continue;
            }
            break;
            //Mage - Missile Barrage
            case 44401:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_ARCANE_BLAST
                case 10833:
                case 16067:
                case 18091:
                case 20883:
                case 22893:
                case 22920:
                case 22940:
                case 24857:
                case 30451:
                case 30661:
                case 31457:
                case 32935:
                case 34793:
                case 35314:
                case 35927:
                case 36032:
                case 37126:
                case 38342:
                case 38344:
                case 38538:
                case 38881:
                case 40837:
                case 40881:
                case 42894:
                case 42896:
                case 42897:
                case 49198:
                case 50545:
                case 51797:
                case 51830:
                case 56969:
                case 58462:
                case 59257:
                case 59909:
                case 65791:
                case 67997:
                case 67998:
                case 67999:
                    //SPELL_HASH_ARCANE_BARRAGE
                case 44425:
                case 44780:
                case 44781:
                case 50273:
                case 50804:
                case 56397:
                case 58456:
                case 59248:
                case 59381:
                case 63934:
                case 64599:
                case 64607:
                case 65799:
                case 67994:
                case 67995:
                case 67996:
                    //SPELL_HASH_FIREBALL
                case 133:
                case 143:
                case 145:
                case 3140:
                case 8400:
                case 8401:
                case 8402:
                case 9053:
                case 9487:
                case 9488:
                case 10148:
                case 10149:
                case 10150:
                case 10151:
                case 10578:
                case 11839:
                case 11921:
                case 11985:
                case 12466:
                case 13140:
                case 13375:
                case 13438:
                case 14034:
                case 15228:
                case 15242:
                case 15536:
                case 15662:
                case 15665:
                case 16101:
                case 16412:
                case 16413:
                case 16415:
                case 16788:
                case 17290:
                case 18082:
                case 18105:
                case 18108:
                case 18199:
                case 18392:
                case 18796:
                case 19391:
                case 19816:
                case 20420:
                case 20678:
                case 20692:
                case 20714:
                case 20793:
                case 20797:
                case 20808:
                case 20811:
                case 20815:
                case 20823:
                case 21072:
                case 21159:
                case 21162:
                case 21402:
                case 21549:
                case 22088:
                case 23024:
                case 23411:
                case 24374:
                case 24611:
                case 25306:
                case 27070:
                case 29456:
                case 29925:
                case 29953:
                case 30218:
                case 30534:
                case 30691:
                case 30943:
                case 30967:
                case 31262:
                case 31620:
                case 32363:
                case 32369:
                case 32414:
                case 32491:
                case 33417:
                case 33793:
                case 33794:
                case 34083:
                case 34348:
                case 34653:
                case 36711:
                case 36805:
                case 36920:
                case 36971:
                case 37111:
                case 37329:
                case 37463:
                case 38641:
                case 38692:
                case 38824:
                case 39267:
                case 40554:
                case 40598:
                case 40877:
                case 41383:
                case 41484:
                case 42802:
                case 42832:
                case 42833:
                case 42834:
                case 42853:
                case 44189:
                case 44202:
                case 44237:
                case 45580:
                case 45595:
                case 45748:
                case 46164:
                case 46988:
                case 47074:
                case 49512:
                case 52282:
                case 54094:
                case 54095:
                case 54096:
                case 57628:
                case 59994:
                case 61567:
                case 61909:
                case 62796:
                case 63789:
                case 63815:
                case 66042:
                case 68310:
                case 68926:
                case 69570:
                case 69583:
                case 69668:
                case 70282:
                case 70409:
                case 70754:
                case 71153:
                case 71500:
                case 71501:
                case 71504:
                case 71748:
                case 71928:
                case 72023:
                case 72024:
                case 72163:
                case 72164:
                    //SPELL_HASH_FROSTBOLT
                case 116:
                case 205:
                case 837:
                case 7322:
                case 8406:
                case 8407:
                case 8408:
                case 9672:
                case 10179:
                case 10180:
                case 10181:
                case 11538:
                case 12675:
                case 12737:
                case 13322:
                case 13439:
                case 15043:
                case 15497:
                case 15530:
                case 16249:
                case 16799:
                case 17503:
                case 20297:
                case 20792:
                case 20806:
                case 20819:
                case 20822:
                case 21369:
                case 23102:
                case 23412:
                case 24942:
                case 25304:
                case 27071:
                case 27072:
                case 28478:
                case 28479:
                case 29457:
                case 29926:
                case 29954:
                case 30942:
                case 31296:
                case 31622:
                case 32364:
                case 32370:
                case 32984:
                case 34347:
                case 35316:
                case 36279:
                case 36710:
                case 36990:
                case 37930:
                case 38238:
                case 38534:
                case 38645:
                case 38697:
                case 38826:
                case 39064:
                case 40429:
                case 40430:
                case 41384:
                case 41486:
                case 42719:
                case 42803:
                case 42841:
                case 42842:
                case 43083:
                case 43428:
                case 44606:
                case 44843:
                case 46035:
                case 46987:
                case 49037:
                case 50378:
                case 50721:
                case 54791:
                case 55802:
                case 55807:
                case 56775:
                case 56837:
                case 57665:
                case 57825:
                case 58457:
                case 58535:
                case 59017:
                case 59251:
                case 59280:
                case 59638:
                case 59855:
                case 61087:
                case 61461:
                case 61590:
                case 61730:
                case 61747:
                case 62583:
                case 62601:
                case 63913:
                case 65807:
                case 68003:
                case 68004:
                case 68005:
                case 69274:
                case 69573:
                case 70277:
                case 70327:
                case 71318:
                case 71420:
                case 72007:
                case 72166:
                case 72167:
                case 72501:
                case 72502:
                    //SPELL_HASH_FROSTFIRE_BOLT
                case 44614:
                case 47610:
                case 51779:
                case 69869:
                case 69984:
                case 70616:
                case 71130:
                    break;
                default:
                    continue;
                }
            }
            break;
            //mage - Improved Scorch
            case 22959:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SCORCH
                case 1811:
                case 2948:
                case 8444:
                case 8445:
                case 8446:
                case 8447:
                case 8448:
                case 8449:
                case 10205:
                case 10206:
                case 10207:
                case 10208:
                case 10209:
                case 10210:
                case 13878:
                case 15241:
                case 17195:
                case 27073:
                case 27074:
                case 27375:
                case 27376:
                case 35377:
                case 36807:
                case 38391:
                case 38636:
                case 42858:
                case 42859:
                case 47723:
                case 50183:
                case 56938:
                case 62546:
                case 62548:
                case 62549:
                case 62551:
                case 62553:
                case 63473:
                case 63474:
                case 63475:
                case 63476:
                case 75412:
                case 75419:
                    break;
                default:
                    continue;
                }
            }
            break;
            //mage - Combustion
            case 28682:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere
                //only trigger effect for specified spells
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING) || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE))
                    continue;
                if (damageInfo.isCritical && spell_proc->getCreatedByAura() != nullptr)
                {
                    auto procAura = spell_proc->getCreatedByAura();
                    procAura->setCharges(procAura->getCharges() + 1);
                    if (procAura->getCharges() >= 3)   //whatch that number cause it depends on original stack count !
                    {
                        uint32_t combastion[] =
                        {
                            //SPELL_HASH_COMBUSTION
                            11129,
                            28682,
                            29977,
                            74630,
                            75882,
                            75883,
                            75884,
                            0
                        };

                        removeAllAurasById(combastion);
                        continue;
                    }
                }
            }
            break;
            //mage - Winter's Chill
            case 12579:
                // Winter's Chill shouldn't proc on self
                if (victim == this || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FROST))
                    continue;
                break;
                //item - Thunderfury
            case 21992:
                if (victim == this)
                    continue;
                break;
                //warrior - Intimidating Shout
            case 5246:
                if (victim == this)
                    continue;
                break;

                //priest - Borrowed time
            case 59887:
            case 59888:
            case 59889:
            case 59890:
            case 59891:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_POWER_WORD__SHIELD
                case 17:
                case 592:
                case 600:
                case 3747:
                case 6065:
                case 6066:
                case 10898:
                case 10899:
                case 10900:
                case 10901:
                case 11647:
                case 11835:
                case 11974:
                case 17139:
                case 20697:
                case 22187:
                case 25217:
                case 25218:
                case 27607:
                case 29408:
                case 32595:
                case 35944:
                case 36052:
                case 41373:
                case 44175:
                case 44291:
                case 46193:
                case 48065:
                case 48066:
                case 66099:
                case 68032:
                case 68033:
                case 68034:
                case 71548:
                case 71780:
                case 71781:
                    break;
                default:
                    continue;
                }
            }
            break;

            //priest - Inspiration
            case 15363:
            case 14893:
            case 15357:
            case 15359:
            {
                if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            break;
            //priest - Blessed Recovery
            case 27813:
            case 27817:
            case 27818:
            {
                if (!isPlayer() || !damageInfo.realDamage)
                    continue;
                SpellInfo const* parentproc = sSpellMgr.getSpellInfo(origId);
                const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
                if (!parentproc || !spellInfo)
                    continue;
                int32_t val = parentproc->calculateEffectValue(0);
                Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
                spell->forced_basepoints->set(0, (val * damageInfo.realDamage) / 300); //per tick
                SpellCastTargets targets(getGuid());
                spell->prepare(&targets);
                continue;
            }
            break;


            //Shaman - Healing Way
            case 29203:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere

                //only trigger effect for specified spells
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_HEALING_WAVE
                case 331:
                case 332:
                case 547:
                case 913:
                case 939:
                case 959:
                case 8005:
                case 10395:
                case 10396:
                case 11986:
                case 12491:
                case 12492:
                case 15982:
                case 25357:
                case 25391:
                case 25396:
                case 26097:
                case 38330:
                case 43548:
                case 48700:
                case 49272:
                case 49273:
                case 51586:
                case 52868:
                case 55597:
                case 57785:
                case 58980:
                case 59083:
                case 60012:
                case 61569:
                case 67528:
                case 68318:
                case 69958:
                case 71133:
                case 75382:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Shaman - Elemental Devastation
            case 29177:
            case 29178:
            case 30165:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere
                //only trigger effect for specified spells
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))  //healing wave
                    continue;
            }
            break;
            //Shaman - Ancestral Fortitude
            case 16177:
            case 16236:
            case 16237:
            {
                if (CastingSpell == NULL)
                    continue;

                //Do not proc on Earth Shield crits
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_EARTH_SHIELD
                case 379:
                case 974:
                case 32593:
                case 32594:
                case 32734:
                case 38590:
                case 49283:
                case 49284:
                case 54479:
                case 54480:
                case 55599:
                case 55600:
                case 56451:
                case 57802:
                case 57803:
                case 58981:
                case 58982:
                case 59471:
                case 59472:
                case 60013:
                case 60014:
                case 66063:
                case 66064:
                case 67530:
                case 67537:
                case 68320:
                case 68592:
                case 68593:
                case 68594:
                case 69568:
                case 69569:
                case 69925:
                case 69926:
                    continue;
                default:
                    break;
                }
            }
            //Shaman - Earthliving Weapon
            case 51940:
            case 51989:
            case 52004:
            case 52005:
            case 52007:
            case 52008:
            {
                if (CastingSpell == NULL)
                    continue;

                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))   //healing spell
                    continue;
            }
            break;
            //Shaman - Tidal Waves
            case 51562:
            case 51563:
            case 51564:
            case 51565:
            case 51566:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_CHAIN_HEAL
                case 1064:
                case 10622:
                case 10623:
                case 14900:
                case 15799:
                case 16367:
                case 25422:
                case 25423:
                case 33642:
                case 41114:
                case 42027:
                case 42477:
                case 43527:
                case 48894:
                case 54481:
                case 55458:
                case 55459:
                case 59473:
                case 69923:
                case 70425:
                case 71120:
                case 75370:
                    //SPELL_HASH_RIPTIDE
                case 22419:
                case 61295:
                case 61299:
                case 61300:
                case 61301:
                case 66053:
                case 68118:
                case 68119:
                case 68120:
                case 75367:
                    break;
                default:
                    continue;
                }
            }
            break;
            // Totem of the Third Wind
            case 42371:
            case 34132:
            case 46099:
            case 43729:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LESSER_HEALING_WAVE
                case 8004:
                case 8008:
                case 8010:
                case 10466:
                case 10467:
                case 10468:
                case 25420:
                case 27624:
                case 28849:
                case 28850:
                case 44256:
                case 46181:
                case 49275:
                case 49276:
                case 49309:
                case 66055:
                case 68115:
                case 68116:
                case 68117:
                case 75366:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Stonebreaker's Totem
            case 43749:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_EARTH_SHOCK
                case 8042:
                case 8044:
                case 8045:
                case 8046:
                case 10412:
                case 10413:
                case 10414:
                case 13281:
                case 13728:
                case 15501:
                case 22885:
                case 23114:
                case 24685:
                case 25025:
                case 25454:
                case 26194:
                case 43305:
                case 47071:
                case 49230:
                case 49231:
                case 54511:
                case 56506:
                case 57783:
                case 60011:
                case 61668:
                case 65973:
                case 68100:
                case 68101:
                case 68102:
                    //SPELL_HASH_FLAME_SHOCK
                case 8050:
                case 8052:
                case 8053:
                case 10447:
                case 10448:
                case 13729:
                case 15039:
                case 15096:
                case 15616:
                case 16804:
                case 22423:
                case 23038:
                case 25457:
                case 29228:
                case 32967:
                case 34354:
                case 39529:
                case 39590:
                case 41115:
                case 43303:
                case 49232:
                case 49233:
                case 51588:
                case 55613:
                case 58940:
                case 58971:
                case 59684:
                    //SPELL_HASH_FROST_SHOCK
                case 8056:
                case 8058:
                case 10472:
                case 10473:
                case 12548:
                case 15089:
                case 15499:
                case 19133:
                case 21030:
                case 21401:
                case 22582:
                case 23115:
                case 25464:
                case 29666:
                case 34353:
                case 37332:
                case 37865:
                case 38234:
                case 39062:
                case 41116:
                case 43524:
                case 46180:
                case 49235:
                case 49236:
                    break;
                default:
                    continue;
                }
            }
            break;
            // Librams of Justice
            case 34135:
            case 42369:
            case 43727:
            case 46093:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_FLASH_OF_LIGHT
                case 19750:
                case 19939:
                case 19940:
                case 19941:
                case 19942:
                case 19943:
                case 25514:
                case 27137:
                case 33641:
                case 37249:
                case 37254:
                case 37257:
                case 48784:
                case 48785:
                case 57766:
                case 59997:
                case 66113:
                case 66922:
                case 68008:
                case 68009:
                case 68010:
                case 71930:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Libram of Divine Judgement
            case 43747:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_JUDGEMENT_OF_COMMAND
                case 20425:
                case 20467:
                case 29386:
                case 32778:
                case 33554:
                case 41368:
                case 41470:
                case 66005:
                case 68017:
                case 68018:
                case 68019:
                case 71551:
                    //SPELL_HASH_JUDGEMENT
                case 10321:
                case 23590:
                case 23591:
                case 35170:
                case 41467:
                case 43838:
                case 54158:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 16246:
            {
                switch (CastingSpell->getId())
                {
                    // Lightning Overload Proc is already free
                case 39805:
                    //SPELL_HASH_LIGHTNING_BOLT
                case 403:
                case 529:
                case 548:
                case 915:
                case 943:
                case 6041:
                case 8246:
                case 9532:
                case 10391:
                case 10392:
                case 12167:
                case 13482:
                case 13527:
                case 14109:
                case 14119:
                case 15207:
                case 15208:
                case 15234:
                case 15801:
                case 16782:
                case 18081:
                case 18089:
                case 19874:
                case 20295:
                case 20802:
                case 20805:
                case 20824:
                case 22414:
                case 23592:
                case 25448:
                case 25449:
                case 26098:
                case 31764:
                case 34345:
                case 35010:
                case 36152:
                case 37273:
                case 37661:
                case 37664:
                case 38465:
                case 39065:
                case 41184:
                case 42024:
                case 43526:
                case 43903:
                case 45075:
                case 45284:
                case 45286:
                case 45287:
                case 45288:
                case 45289:
                case 45290:
                case 45291:
                case 45292:
                case 45293:
                case 45294:
                case 45295:
                case 45296:
                case 48698:
                case 48895:
                case 49237:
                case 49238:
                case 49239:
                case 49240:
                case 49418:
                case 49454:
                case 51587:
                case 51618:
                case 53044:
                case 53314:
                case 54843:
                case 55044:
                case 56326:
                case 56891:
                case 57780:
                case 57781:
                case 59006:
                case 59024:
                case 59081:
                case 59169:
                case 59199:
                case 59683:
                case 59863:
                case 60009:
                case 60032:
                case 61374:
                case 61893:
                case 63809:
                case 64098:
                case 64696:
                case 65987:
                case 68112:
                case 68113:
                case 68114:
                case 69567:
                case 69970:
                case 71136:
                case 71934:
                    //SPELL_HASH_CHAIN_LIGHTNING
                case 421:
                case 930:
                case 2860:
                case 10605:
                case 12058:
                case 15117:
                case 15305:
                case 15659:
                case 16006:
                case 16033:
                case 16921:
                case 20831:
                case 21179:
                case 22355:
                case 23106:
                case 23206:
                case 24680:
                case 25021:
                case 25439:
                case 25442:
                case 27567:
                case 28167:
                case 28293:
                case 28900:
                case 31330:
                case 31717:
                case 32337:
                case 33643:
                case 37448:
                case 39066:
                case 39945:
                case 40536:
                case 41183:
                case 42441:
                case 42804:
                case 43435:
                case 44318:
                case 45297:
                case 45298:
                case 45299:
                case 45300:
                case 45301:
                case 45302:
                case 45868:
                case 46380:
                case 48140:
                case 48699:
                case 49268:
                case 49269:
                case 49270:
                case 49271:
                case 50830:
                case 52383:
                case 54334:
                case 54531:
                case 59082:
                case 59220:
                case 59223:
                case 59273:
                case 59517:
                case 59716:
                case 59844:
                case 61528:
                case 61879:
                case 62131:
                case 63479:
                case 64213:
                case 64215:
                case 64390:
                case 64758:
                case 64759:
                case 67529:
                case 68319:
                case 69696:
                case 75362:
                    //SPELL_HASH_EARTH_SHOCK
                case 8042:
                case 8044:
                case 8045:
                case 8046:
                case 10412:
                case 10413:
                case 10414:
                case 13281:
                case 13728:
                case 15501:
                case 22885:
                case 23114:
                case 24685:
                case 25025:
                case 25454:
                case 26194:
                case 43305:
                case 47071:
                case 49230:
                case 49231:
                case 54511:
                case 56506:
                case 57783:
                case 60011:
                case 61668:
                case 65973:
                case 68100:
                case 68101:
                case 68102:
                    //SPELL_HASH_FLAME_SHOCK
                case 8050:
                case 8052:
                case 8053:
                case 10447:
                case 10448:
                case 13729:
                case 15039:
                case 15096:
                case 15616:
                case 16804:
                case 22423:
                case 23038:
                case 25457:
                case 29228:
                case 32967:
                case 34354:
                case 39529:
                case 39590:
                case 41115:
                case 43303:
                case 49232:
                case 49233:
                case 51588:
                case 55613:
                case 58940:
                case 58971:
                case 59684:
                    //SPELL_HASH_FROST_SHOCK
                case 8056:
                case 8058:
                case 10472:
                case 10473:
                case 12548:
                case 15089:
                case 15499:
                case 19133:
                case 21030:
                case 21401:
                case 22582:
                case 23115:
                case 25464:
                case 29666:
                case 34353:
                case 37332:
                case 37865:
                case 38234:
                case 39062:
                case 41116:
                case 43524:
                case 46180:
                case 49235:
                case 49236:
                    break;
                default:
                    continue;
                }
            }
            break;
            //shaman - windfury weapon
            case 8232:
            case 8235:
            case 10486:
            case 16362:
            case 25505:
            {
                if (!isPlayer())
                    continue;
                //!! The weird thing is that we need the spell that triggered this enchant spell in order to output logs ..we are using oldspell info too
                //we have to recalc the value of this spell
                const auto spellInfo = sSpellMgr.getSpellInfo(origId);
                uint32_t AP_owerride = spellInfo->calculateEffectValue(0);
                uint32_t dmg2 = static_cast<Player*>(this)->getMainMeleeDamage(AP_owerride);
                SpellInfo const* sp_for_the_logs = sSpellMgr.getSpellInfo(spellId);
                strike(victim, MELEE, sp_for_the_logs, dmg2, 0, 0, true, false);
                strike(victim, MELEE, sp_for_the_logs, dmg2, 0, 0, true, false);
                spellId = 33010; // WF animation
            }
            break;
            //rogue - Ruthlessness
            case 14157:
            {
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere
                //we need a finishing move for this
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE) || victim == this)
                    continue;
            }
            break;
            // rogue - T10 4P bonus
            case 70802:
            {
                // The rogue bonus set of T10 requires a finishing move
                if (!(CastingSpell && CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE))
                    continue;
            }
            break;
            //warrior - improved berserker rage
            case 23690:
            case 23691:
            {
                if (!CastingSpell || CastingSpell->getId() != 18499)
                    continue;
            }
            break;
            //mage - Arcane Concentration
            case 12536:
            {
                //requires damageing spell
                if (CastingSpell == NULL)
                    continue;//this should not occur unless we made a fuckup somewhere
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            //mage - Improved Blizzard
            case 12484:
            case 12485:
            case 12486:
            {
                if (CastingSpell == NULL)
                    continue;

                if (victim == this)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_BLIZZARD
                case 10:
                case 1196:
                case 6141:
                case 6142:
                case 8364:
                case 8427:
                case 8428:
                case 10185:
                case 10186:
                case 10187:
                case 10188:
                case 10189:
                case 10190:
                case 15783:
                case 19099:
                case 20680:
                case 21096:
                case 21367:
                case 25019:
                case 26607:
                case 27085:
                case 27384:
                case 27618:
                case 29458:
                case 29951:
                case 30093:
                case 31266:
                case 31581:
                case 33418:
                case 33624:
                case 33634:
                case 34167:
                case 34183:
                case 34356:
                case 37263:
                case 37671:
                case 38646:
                case 39416:
                case 41382:
                case 41482:
                case 42198:
                case 42208:
                case 42209:
                case 42210:
                case 42211:
                case 42212:
                case 42213:
                case 42937:
                case 42938:
                case 42939:
                case 42940:
                case 44178:
                case 46195:
                case 47727:
                case 49034:
                case 50715:
                case 56936:
                case 58693:
                case 59278:
                case 59369:
                case 59854:
                case 61085:
                case 62576:
                case 62577:
                case 62602:
                case 62603:
                case 62706:
                case 64642:
                case 64653:
                case 70362:
                case 70421:
                case 71118:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Hunter - The Beast Within
            case 34471:
            {
                if (CastingSpell == NULL)
                    continue;

                if (CastingSpell->getId() != 19574)
                    continue;
            }
            //Hunter - Thrill of the Hunt
            case 34720:
            {
                if (CastingSpell == NULL)
                    continue;
                spell_proc->setOverrideEffectDamage(0, CastingSpell->getManaCost() * 40 / 100);
            }
            break;
            //priest - Reflective Shield
            case 33619:
            {
                if (!damageInfo.absorbedDamage)
                    continue;

                //requires Power Word: Shield active
                uint32_t powerWordShield[] =
                {
                    //SPELL_HASH_POWER_WORD__SHIELD
                    17,
                    592,
                    600,
                    3747,
                    6065,
                    6066,
                    10898,
                    10899,
                    10900,
                    10901,
                    11647,
                    11835,
                    11974,
                    17139,
                    20697,
                    22187,
                    25217,
                    25218,
                    27607,
                    29408,
                    32595,
                    35944,
                    36052,
                    41373,
                    44175,
                    44291,
                    46193,
                    48065,
                    48066,
                    66099,
                    68032,
                    68033,
                    68034,
                    71548,
                    71780,
                    71781,
                    0
                };

                int power_word_id = hasAurasWithId(powerWordShield);
                if (!power_word_id)
                    power_word_id = 17;
                //make a direct strike then exit rest of handler
                if (ospinfo)
                {
                    auto tdmg = damageInfo.absorbedDamage * (ospinfo->calculateEffectValue(0)) / 100.0f;
                    //somehow we should make this not caused any threat (to be done)
                    doSpellDamage(victim, power_word_id, tdmg, 0, true);
                }
                continue;
            }
            break;
            //rogue - combat potency
            case 35542:
            case 35545:
            case 35546:
            case 35547:
            case 35548:
            {
                if (!isPlayer() || !damageInfo.realDamage)
                    continue;
                //this needs offhand weapon
                Item* it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                if (it == nullptr || it->getItemProperties()->InventoryType != INVTYPE_WEAPON)
                    continue;
            }
            break;
            //paladin - Improved Lay on Hands
            case 20233:
            case 20236:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LAY_ON_HANDS
                case 633:
                case 2800:
                case 9257:
                case 10310:
                case 17233:
                case 20233:
                case 20236:
                case 27154:
                case 48788:
                case 53778:
                    break;
                default:
                    continue;
                }
            }
            break;
            //paladin - Infusion of Light
            case 53672:
            case 54149:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_HOLY_SHOCK
                case 20473:
                case 20929:
                case 20930:
                case 25902:
                case 25903:
                case 25911:
                case 25912:
                case 25913:
                case 25914:
                case 27174:
                case 27175:
                case 27176:
                case 32771:
                case 33072:
                case 33073:
                case 33074:
                case 35160:
                case 36340:
                case 38921:
                case 48820:
                case 48821:
                case 48822:
                case 48823:
                case 48824:
                case 48825:
                case 66114:
                case 68014:
                case 68015:
                case 68016:
                    break;
                default:
                    continue;
                }
            }
            break;
            //paladin - Sacred Cleansing
            case 53659:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_CLEANSE
                case 4987:
                case 28787:
                case 28788:
                case 29380:
                case 32400:
                case 39078:
                case 57767:
                case 66116:
                case 68621:
                case 68622:
                case 68623:
                    break;
                default:
                    continue;
                }
            }
            break;
            //paladin - Judgements of the Pure
            case 53655:
            case 53656:
            case 53657:
            case 54152:
            case 54153:
            {
                if (CastingSpell == NULL)
                    continue;
                if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                    continue;
            }
            break;
            case 21183: //Paladin - Heart of the Crusader
            case 54498:
            case 54499:
            {
                if (CastingSpell == NULL)
                    continue;
                if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                    continue;
            }
            break;
            case 54203: //Paladin - Sheath of Light
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_FLASH_OF_LIGHT
                case 19750:
                case 19939:
                case 19940:
                case 19941:
                case 19942:
                case 19943:
                case 25514:
                case 27137:
                case 33641:
                case 37249:
                case 37254:
                case 37257:
                case 48784:
                case 48785:
                case 57766:
                case 59997:
                case 66113:
                case 66922:
                case 68008:
                case 68009:
                case 68010:
                case 71930:
                    //SPELL_HASH_HOLY_LIGHT
                case 635:
                case 639:
                case 647:
                case 1026:
                case 1042:
                case 3472:
                case 10328:
                case 10329:
                case 13952:
                case 15493:
                case 25263:
                case 25292:
                case 27135:
                case 27136:
                case 29383:
                case 29427:
                case 29562:
                case 31713:
                case 32769:
                case 37979:
                case 43451:
                case 44479:
                case 46029:
                case 48781:
                case 48782:
                case 52444:
                case 56539:
                case 58053:
                case 66112:
                case 68011:
                case 68012:
                case 68013:
                    break;
                default:
                    continue;
                }

                const auto spellInfo = sSpellMgr.getSpellInfo(54203);
                auto spell_duration = sSpellDurationStore.lookupEntry(spellInfo->getDurationIndex());
                uint32_t tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);
                if (ospinfo)
                    spell_proc->setOverrideEffectDamage(0, ospinfo->getEffectBasePoints(0) * damageInfo.realDamage / (100 * tickcount));
            }
            break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // WARRIOR

            // Warrior - Improved Revenge
            case 12798:
            {
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REVENGE
                case 6572:
                case 6574:
                case 7379:
                case 11600:
                case 11601:
                case 12170:
                case 19130:
                case 25269:
                case 25288:
                case 28844:
                case 30357:
                case 37517:
                case 40392:
                case 57823:
                    break;
                default:
                    continue;
                }
            }
            break;
            // Warrior - Unrelenting Assault
            case 64849:
            case 64850:
            {
                if (CastingSpell == nullptr)
                    continue;
                //trigger only on heal spell cast by NOT us
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING) || this == victim)
                    continue;
                //this is not counting the bonus effects on heal
                auto idx = CastingSpell->firstBeneficialEffect();
                if (idx != 1)
                {
                    if (ospinfo)
                        spell_proc->setOverrideEffectDamage(0, ((CastingSpell->getEffectBasePoints(static_cast<uint8_t>(idx)) + 1) * (ospinfo->calculateEffectValue(0)) / 100));
                }
            }
            break;
            //paladin - Light's Grace
            case 31834:
            {
                if (CastingSpell == nullptr)
                    continue;//this should not occur unless we made a fuckup somewhere

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_HOLY_LIGHT
                case 635:
                case 639:
                case 647:
                case 1026:
                case 1042:
                case 3472:
                case 10328:
                case 10329:
                case 13952:
                case 15493:
                case 25263:
                case 25292:
                case 27135:
                case 27136:
                case 29383:
                case 29427:
                case 29562:
                case 31713:
                case 32769:
                case 37979:
                case 43451:
                case 44479:
                case 46029:
                case 48781:
                case 48782:
                case 52444:
                case 56539:
                case 58053:
                case 66112:
                case 68011:
                case 68012:
                case 68013:
                    break;
                default:
                    continue;
                }
            }
            break;
            //paladin - Blessed Life
            case 31828:
            {
                //we should test is damage is from environment or not :S
                resisted_dmg = (damageInfo.realDamage / 2);
                continue; //there is no visual for this ?
            }
            break;
            //paladin - Judgements of the Wise
            case 54180:
            {
                if (CastingSpell == NULL)
                    continue;
                if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                    continue;
                if (!isPlayer())
                    continue;
            }
            break;
            case 54172: //Paladin - Divine Storm heal effect
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_DIVINE_STORM
                case 53385:
                case 54171:
                case 54172:
                case 58127:
                case 66006:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Energized
            case 43751:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LIGHTNING_BOLT
                case 403:
                case 529:
                case 548:
                case 915:
                case 943:
                case 6041:
                case 8246:
                case 9532:
                case 10391:
                case 10392:
                case 12167:
                case 13482:
                case 13527:
                case 14109:
                case 14119:
                case 15207:
                case 15208:
                case 15234:
                case 15801:
                case 16782:
                case 18081:
                case 18089:
                case 19874:
                case 20295:
                case 20802:
                case 20805:
                case 20824:
                case 22414:
                case 23592:
                case 25448:
                case 25449:
                case 26098:
                case 31764:
                case 34345:
                case 35010:
                case 36152:
                case 37273:
                case 37661:
                case 37664:
                case 38465:
                case 39065:
                case 41184:
                case 42024:
                case 43526:
                case 43903:
                case 45075:
                case 45284:
                case 45286:
                case 45287:
                case 45288:
                case 45289:
                case 45290:
                case 45291:
                case 45292:
                case 45293:
                case 45294:
                case 45295:
                case 45296:
                case 48698:
                case 48895:
                case 49237:
                case 49238:
                case 49239:
                case 49240:
                case 49418:
                case 49454:
                case 51587:
                case 51618:
                case 53044:
                case 53314:
                case 54843:
                case 55044:
                case 56326:
                case 56891:
                case 57780:
                case 57781:
                case 59006:
                case 59024:
                case 59081:
                case 59169:
                case 59199:
                case 59683:
                case 59863:
                case 60009:
                case 60032:
                case 61374:
                case 61893:
                case 63809:
                case 64098:
                case 64696:
                case 65987:
                case 68112:
                case 68113:
                case 68114:
                case 69567:
                case 69970:
                case 71136:
                case 71934:
                    break;
                default:
                    continue;
                }

            }
            break;
            //Spell Haste Trinket
            //http://www.wowhead.com/?item=28190 scarab of the infinite circle
            case 33370:
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            case 60487: // Extract of Necromantic Power
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            case 33953: // The Egg of Mortal essence
            {
                if (!CastingSpell)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            break;
            case 60529: // Forethough Talisman
            {
                if (!CastingSpell)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            break;
            case 53390: //Tidal Waves
            {
                if (!CastingSpell)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_CHAIN_HEAL
                case 1064:
                case 10622:
                case 10623:
                case 14900:
                case 15799:
                case 16367:
                case 25422:
                case 25423:
                case 33642:
                case 41114:
                case 42027:
                case 42477:
                case 43527:
                case 48894:
                case 54481:
                case 55458:
                case 55459:
                case 59473:
                case 69923:
                case 70425:
                case 71120:
                case 75370:
                    //SPELL_HASH_RIPTIDE
                case 22419:
                case 61295:
                case 61299:
                case 61300:
                case 61301:
                case 66053:
                case 68118:
                case 68119:
                case 68120:
                case 75367:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Earthliving
            case 51945:
            case 51990:
            case 51997:
            case 51998:
            case 51999:
            case 52000:
            {
                if (!CastingSpell)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            break;
            //shaman - Lightning Overload
            case 39805:
            {
                if (CastingSpell == NULL)
                    continue;                   //this should not occur unless we made a fuckup somewhere

                //trigger on lightning and chain lightning. Spell should be identical , well maybe next time :P
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LIGHTNING_BOLT
                case 403:
                case 529:
                case 548:
                case 915:
                case 943:
                case 6041:
                case 8246:
                case 9532:
                case 10391:
                case 10392:
                case 12167:
                case 13482:
                case 13527:
                case 14109:
                case 14119:
                case 15207:
                case 15208:
                case 15234:
                case 15801:
                case 16782:
                case 18081:
                case 18089:
                case 19874:
                case 20295:
                case 20802:
                case 20805:
                case 20824:
                case 22414:
                case 23592:
                case 25448:
                case 25449:
                case 26098:
                case 31764:
                case 34345:
                case 35010:
                case 36152:
                case 37273:
                case 37661:
                case 37664:
                case 38465:
                case 39065:
                case 41184:
                case 42024:
                case 43526:
                case 43903:
                case 45075:
                case 45284:
                case 45286:
                case 45287:
                case 45288:
                case 45289:
                case 45290:
                case 45291:
                case 45292:
                case 45293:
                case 45294:
                case 45295:
                case 45296:
                case 48698:
                case 48895:
                case 49237:
                case 49238:
                case 49239:
                case 49240:
                case 49418:
                case 49454:
                case 51587:
                case 51618:
                case 53044:
                case 53314:
                case 54843:
                case 55044:
                case 56326:
                case 56891:
                case 57780:
                case 57781:
                case 59006:
                case 59024:
                case 59081:
                case 59169:
                case 59199:
                case 59683:
                case 59863:
                case 60009:
                case 60032:
                case 61374:
                case 61893:
                case 63809:
                case 64098:
                case 64696:
                case 65987:
                case 68112:
                case 68113:
                case 68114:
                case 69567:
                case 69970:
                case 71136:
                case 71934:
                    //SPELL_HASH_CHAIN_LIGHTNING
                case 421:
                case 930:
                case 2860:
                case 10605:
                case 12058:
                case 15117:
                case 15305:
                case 15659:
                case 16006:
                case 16033:
                case 16921:
                case 20831:
                case 21179:
                case 22355:
                case 23106:
                case 23206:
                case 24680:
                case 25021:
                case 25439:
                case 25442:
                case 27567:
                case 28167:
                case 28293:
                case 28900:
                case 31330:
                case 31717:
                case 32337:
                case 33643:
                case 37448:
                case 39066:
                case 39945:
                case 40536:
                case 41183:
                case 42441:
                case 42804:
                case 43435:
                case 44318:
                case 45297:
                case 45298:
                case 45299:
                case 45300:
                case 45301:
                case 45302:
                case 45868:
                case 46380:
                case 48140:
                case 48699:
                case 49268:
                case 49269:
                case 49270:
                case 49271:
                case 50830:
                case 52383:
                case 54334:
                case 54531:
                case 59082:
                case 59220:
                case 59223:
                case 59273:
                case 59517:
                case 59716:
                case 59844:
                case 61528:
                case 61879:
                case 62131:
                case 63479:
                case 64213:
                case 64215:
                case 64390:
                case 64758:
                case 64759:
                case 67529:
                case 68319:
                case 69696:
                case 75362:
                {
                    castSpell(this, 39805, true);
                    spellId = CastingSpell->getId();
                    origId = 39805;
                } break;
                default:
                    continue;
                }
            }
            break;
            //item - Band of the Eternal Sage
            case 35084:
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))     //requires offensive spell. ! might not cover all spells
                    continue;
            }
            break;
            //druid - Earth and Moon
            case 60431:
            case 60432:
            case 60433:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_STARFIRE
                case 2912:
                case 8949:
                case 8950:
                case 8951:
                case 9875:
                case 9876:
                case 21668:
                case 25298:
                case 26986:
                case 35243:
                case 38935:
                case 40344:
                case 48464:
                case 48465:
                case 65854:
                case 67947:
                case 67948:
                case 67949:
                case 75332:
                    //SPELL_HASH_WRATH
                case 5176:
                case 5177:
                case 5178:
                case 5179:
                case 5180:
                case 6780:
                case 8905:
                case 9739:
                case 9912:
                case 17144:
                case 18104:
                case 20698:
                case 21667:
                case 21807:
                case 26984:
                case 26985:
                case 31784:
                case 43619:
                case 48459:
                case 48461:
                case 52501:
                case 57648:
                case 59986:
                case 62793:
                case 63259:
                case 63569:
                case 65862:
                case 67951:
                case 67952:
                case 67953:
                case 69968:
                case 71148:
                case 75327:
                    break;
                default:
                    continue;
                }
            }
            break;
            // druid - Celestial Focus
            case 16922:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_STARFIRE
                case 2912:
                case 8949:
                case 8950:
                case 8951:
                case 9875:
                case 9876:
                case 21668:
                case 25298:
                case 26986:
                case 35243:
                case 38935:
                case 40344:
                case 48464:
                case 48465:
                case 65854:
                case 67947:
                case 67948:
                case 67949:
                case 75332:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 37565: //setbonus
            {
                if (!CastingSpell)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_FLASH_HEAL
                case 2061:
                case 9472:
                case 9473:
                case 9474:
                case 10915:
                case 10916:
                case 10917:
                case 17137:
                case 17138:
                case 17843:
                case 25233:
                case 25235:
                case 27608:
                case 38588:
                case 42420:
                case 43431:
                case 43516:
                case 43575:
                case 48070:
                case 48071:
                case 56331:
                case 56919:
                case 66104:
                case 68023:
                case 68024:
                case 68025:
                case 71595:
                case 71782:
                case 71783:
                    break;
                default:
                    continue;
                }
            }
            break;
            //SETBONUSES
            case 37379:
            {
                if (!CastingSpell || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_SHADOW) || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            case 37378:
            {
                if (!CastingSpell || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE) || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            case 45062: // Vial of the Sunwell
            case 39950: // Wave Trance
            {
                if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            break;
            case 37234:
            case 37214:
            case 37601:
            {
                if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;
            case 37237:
            {
                if (!CastingSpell)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LIGHTNING_BOLT
                case 403:
                case 529:
                case 548:
                case 915:
                case 943:
                case 6041:
                case 8246:
                case 9532:
                case 10391:
                case 10392:
                case 12167:
                case 13482:
                case 13527:
                case 14109:
                case 14119:
                case 15207:
                case 15208:
                case 15234:
                case 15801:
                case 16782:
                case 18081:
                case 18089:
                case 19874:
                case 20295:
                case 20802:
                case 20805:
                case 20824:
                case 22414:
                case 23592:
                case 25448:
                case 25449:
                case 26098:
                case 31764:
                case 34345:
                case 35010:
                case 36152:
                case 37273:
                case 37661:
                case 37664:
                case 38465:
                case 39065:
                case 41184:
                case 42024:
                case 43526:
                case 43903:
                case 45075:
                case 45284:
                case 45286:
                case 45287:
                case 45288:
                case 45289:
                case 45290:
                case 45291:
                case 45292:
                case 45293:
                case 45294:
                case 45295:
                case 45296:
                case 48698:
                case 48895:
                case 49237:
                case 49238:
                case 49239:
                case 49240:
                case 49418:
                case 49454:
                case 51587:
                case 51618:
                case 53044:
                case 53314:
                case 54843:
                case 55044:
                case 56326:
                case 56891:
                case 57780:
                case 57781:
                case 59006:
                case 59024:
                case 59081:
                case 59169:
                case 59199:
                case 59683:
                case 59863:
                case 60009:
                case 60032:
                case 61374:
                case 61893:
                case 63809:
                case 64098:
                case 64696:
                case 65987:
                case 68112:
                case 68113:
                case 68114:
                case 69567:
                case 69970:
                case 71136:
                case 71934:
                    break;
                default:
                    continue;
                }
            } break;
            //Tier 7 Warlock setbonus
            case 61082:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_LIFE_TAP
                case 1454:
                case 1455:
                case 1456:
                case 4090:
                case 11687:
                case 11688:
                case 11689:
                case 27222:
                case 28830:
                case 31818:
                case 32553:
                case 57946:
                case 63321:
                    break;
                default:
                    continue;
                }
            }
            break;
            //Tier 5 Paladin setbonus - Crystalforge Battlegear or Crystalforge Raiment
            case 37196:
            case 43838:
            {
                if (!CastingSpell)
                    continue;

                switch (CastingSpell->getId())
                {
                case 31804:
                    //SPELL_HASH_JUDGEMENT_OF_JUSTICE
                case 20184:
                case 53407:
                    //SPELL_HASH_JUDGEMENT_OF_LIGHT
                case 20185:
                case 20267:
                case 20271:
                case 28775:
                case 57774:
                    //SPELL_HASH_JUDGEMENT_OF_WISDOM
                case 20186:
                case 20268:
                case 53408:
                    //SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS
                case 20187:
                    //SPELL_HASH_JUDGEMENT_OF_BLOOD
                case 31898:
                case 32220:
                case 41461:
                    //SPELL_HASH_JUDGEMENT_OF_COMMAND
                case 20425:
                case 20467:
                case 29386:
                case 32778:
                case 33554:
                case 41368:
                case 41470:
                case 66005:
                case 68017:
                case 68018:
                case 68019:
                case 71551:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 43837:
            {
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_FLASH_OF_LIGHT
                case 19750:
                case 19939:
                case 19940:
                case 19941:
                case 19942:
                case 19943:
                case 25514:
                case 27137:
                case 33641:
                case 37249:
                case 37254:
                case 37257:
                case 48784:
                case 48785:
                case 57766:
                case 59997:
                case 66113:
                case 66922:
                case 68008:
                case 68009:
                case 68010:
                case 71930:
                    //SPELL_HASH_HOLY_LIGHT
                case 635:
                case 639:
                case 647:
                case 1026:
                case 1042:
                case 3472:
                case 10328:
                case 10329:
                case 13952:
                case 15493:
                case 25263:
                case 25292:
                case 27135:
                case 27136:
                case 29383:
                case 29427:
                case 29562:
                case 31713:
                case 32769:
                case 37979:
                case 43451:
                case 44479:
                case 46029:
                case 48781:
                case 48782:
                case 52444:
                case 56539:
                case 58053:
                case 66112:
                case 68011:
                case 68012:
                case 68013:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 37529:
            {
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_OVERPOWER
                case 7384:
                case 7887:
                case 11584:
                case 11585:
                case 14895:
                case 17198:
                case 24407:
                case 32154:
                case 37321:
                case 37529:
                case 43456:
                case 58516:
                case 65924:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 37517:
            {
                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REVENGE
                case 6572:
                case 6574:
                case 7379:
                case 11600:
                case 11601:
                case 12170:
                case 19130:
                case 25269:
                case 25288:
                case 28844:
                case 30357:
                case 40392:
                case 57823:
                    break;
                case 37517:
                default:
                    continue;
                }
            }
            break;
            case 38333: // Ribbon of Sacrifice
            {
                if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                    continue;
            }
            //SETBONUSES END
            //http://www.wowhead.com/?item=32493 Ashtongue Talisman of Shadows
            case 40480:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_CORRUPTION
                case 172:
                case 6222:
                case 6223:
                case 7648:
                case 11671:
                case 11672:
                case 13530:
                case 16402:
                case 16985:
                case 17510:
                case 18088:
                case 18376:
                case 18656:
                case 21068:
                case 23642:
                case 25311:
                case 27216:
                case 28829:
                case 30938:
                case 31405:
                case 32063:
                case 32197:
                case 37113:
                case 37961:
                case 39212:
                case 39621:
                case 41988:
                case 47782:
                case 47812:
                case 47813:
                case 56898:
                case 57645:
                case 58811:
                case 60016:
                case 61563:
                case 65810:
                case 68133:
                case 68134:
                case 68135:
                case 70602:
                case 70904:
                case 71937:
                    break;
                default:
                    continue;
                }
            }
            break;

            //http://www.wowhead.com/?item=32496  Memento of Tyrande
            case 37656: //don't say damaging spell but EACH time spell is cast there is a chance (so can be healing spell)
            {
                if (CastingSpell == NULL)
                    continue;
            }
            break;
            //http://www.wowhead.com/?item=32488 Ashtongue Talisman of Insight
            case 40483:
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                    continue;
            }
            break;

            //http://www.wowhead.com/?item=32487 Ashtongue Talisman of Swiftness
            case 40487:
            {
                switch (CastingSpell->getId())
                {
                    // SPELL_HASH_STEADY_SHOT:
                case 34120:
                case 49051:
                case 49052:
                case 56641:
                case 65867:
                    break;
                default:
                    continue;
                }
            }
            break;

            //http://www.wowhead.com/?item=32485 Ashtongue Talisman of Valor
            case 40459:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getAreaAuraEffect())
                {
                    //SPELL_HASH_SHIELD_SLAM
                case 8242:
                case 15655:
                case 23922:
                case 23923:
                case 23924:
                case 23925:
                case 25258:
                case 29684:
                case 30356:
                case 30688:
                case 46762:
                case 47487:
                case 47488:
                case 49863:
                case 59142:
                case 69903:
                case 72645:
                    //SPELL_HASH_BLOODTHIRST
                case 23880:
                case 23881:
                case 23885:
                case 23892:
                case 23893:
                case 23894:
                case 25251:
                case 30335:
                case 30474:
                case 30475:
                case 30476:
                case 31996:
                case 31997:
                case 31998:
                case 33964:
                case 35123:
                case 35125:
                case 35947:
                case 35948:
                case 35949:
                case 39070:
                case 39071:
                case 39072:
                case 40423:
                case 55968:
                case 55969:
                case 55970:
                case 57790:
                case 57791:
                case 57792:
                case 60017:
                case 71938:
                    //SPELL_HASH_MORTAL_STRIKE
                case 9347:
                case 12294:
                case 13737:
                case 15708:
                case 16856:
                case 17547:
                case 19643:
                case 21551:
                case 21552:
                case 21553:
                case 24573:
                case 25248:
                case 27580:
                case 29572:
                case 30330:
                case 31911:
                case 32736:
                case 35054:
                case 37335:
                case 39171:
                case 40220:
                case 43441:
                case 43529:
                case 44268:
                case 47485:
                case 47486:
                case 57789:
                case 65926:
                case 67542:
                case 68782:
                case 68783:
                case 68784:
                case 71552:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 28804://Epiphany :Each spell you cast can trigger an Epiphany, increasing your mana regeneration by 24 for 30 sec.
            {
                if (!CastingSpell)
                    continue;
            }
            break;
            //SETBONUSES END
            //item - Band of the Eternal Restorer
            case 35087:
            {
                if (CastingSpell == NULL)
                    continue;
                if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING)) //requires healing spell.
                    continue;
            }
            break;

            //http://www.wowhead.com/?item=32486 Ashtongue Talisman of Equilibrium
            case 40452: //Mangle has a 40% chance to grant 140 Strength for 8 sec
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    // SPELL_HASH_MANGLE__BEAR_
                case 33878:
                case 33986:
                case 33987:
                case 48563:
                case 48564:
                    // SPELL_HASH_MANGLE__CAT_
                case 33876:
                case 33982:
                case 33983:
                case 48565:
                case 48566:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 40445: //Starfire has a 25% chance to grant up to 150 spell damage for 8 sec
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_STARFIRE
                case 2912:
                case 8949:
                case 8950:
                case 8951:
                case 9875:
                case 9876:
                case 21668:
                case 25298:
                case 26986:
                case 35243:
                case 38935:
                case 40344:
                case 48464:
                case 48465:
                case 65854:
                case 67947:
                case 67948:
                case 67949:
                case 75332:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 40446: //Rejuvenation has a 25% chance to grant up to 210 healing for 8 sec
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REJUVENATION
                case 774:
                case 1058:
                case 1430:
                case 2090:
                case 2091:
                case 3627:
                case 8070:
                case 8910:
                case 9839:
                case 9840:
                case 9841:
                case 12160:
                case 15981:
                case 20664:
                case 20701:
                case 25299:
                case 26981:
                case 26982:
                case 27532:
                case 28716:
                case 28722:
                case 28723:
                case 28724:
                case 31782:
                case 32131:
                case 38657:
                case 42544:
                case 48440:
                case 48441:
                case 53607:
                case 64801:
                case 66065:
                case 67971:
                case 67972:
                case 67973:
                case 69898:
                case 70691:
                case 71142:
                    break;
                default:
                    continue;
                }
            }
            break;

            //http://www.wowhead.com/?item=32490 Ashtongue Talisman of Acumen
            case 40441: //Each time your Shadow Word: Pain deals damage, it has a 10% chance to grant you 220 spell damage for 10 sec
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_SHADOW_WORD__PAIN
                case 589:
                case 594:
                case 970:
                case 992:
                case 2767:
                case 10892:
                case 10893:
                case 10894:
                case 11639:
                case 14032:
                case 15654:
                case 17146:
                case 19776:
                case 23268:
                case 23952:
                case 24212:
                case 25367:
                case 25368:
                case 27605:
                case 30854:
                case 30898:
                case 34441:
                case 34941:
                case 34942:
                case 37275:
                case 41355:
                case 46560:
                case 48124:
                case 48125:
                case 57778:
                case 59864:
                case 60005:
                case 60446:
                case 65541:
                case 68088:
                case 68089:
                case 68090:
                case 72318:
                case 72319:
                    break;
                default:
                    continue;
                }
            }
            break;

            //http://www.wowhead.com/?item=32490 Ashtongue Talisman of Acumen
            case 40440: //Each time your Renew heals, it has a 10% chance to grant you 220 healing for 5 sec
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_RENEW
                case 139:
                case 6074:
                case 6075:
                case 6076:
                case 6077:
                case 6078:
                case 8362:
                case 10927:
                case 10928:
                case 10929:
                case 11640:
                case 22168:
                case 23895:
                case 25058:
                case 25221:
                case 25222:
                case 25315:
                case 27606:
                case 28807:
                case 31325:
                case 34423:
                case 36679:
                case 36969:
                case 37260:
                case 37978:
                case 38210:
                case 41456:
                case 44174:
                case 45859:
                case 46192:
                case 46563:
                case 47079:
                case 48067:
                case 48068:
                case 49263:
                case 56332:
                case 57777:
                case 60004:
                case 61967:
                case 62333:
                case 62441:
                case 66177:
                case 66537:
                case 67675:
                case 68035:
                case 68036:
                case 68037:
                case 71932:
                    break;
                default:
                    continue;
                }
            }
            break;

            //http://www.wowhead.com/?item=32492 Ashtongue Talisman of Lethality
            case 37445: //using a mana gem grants you 225 spell damage for 15 sec
            {
                if (!CastingSpell)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_REPLENISH_MANA
                case 5405:
                case 10052:
                case 10057:
                case 10058:
                case 18385:
                case 27103:
                case 33394:
                case 42987:
                case 42988:
                case 71565:
                case 71574:
                    break;
                default:
                    continue;
                }
            }
            break;
            case 16886: // druid - Nature's Grace
            {
                // Remove aura if it exists so it gets reapplied
                removeAllAurasById(16886);
            }
            break;
            case 38395:
            {
                if (CastingSpell == NULL)
                    continue;

                switch (CastingSpell->getId())
                {
                    //SPELL_HASH_IMMOLATE
                case 348:
                case 707:
                case 1094:
                case 2941:
                case 8981:
                case 9034:
                case 9275:
                case 9276:
                case 11665:
                case 11667:
                case 11668:
                case 11962:
                case 11984:
                case 12742:
                case 15505:
                case 15506:
                case 15570:
                case 15661:
                case 15732:
                case 15733:
                case 17883:
                case 18542:
                case 20294:
                case 20787:
                case 20800:
                case 20826:
                case 25309:
                case 27215:
                case 29928:
                case 36637:
                case 36638:
                case 37668:
                case 38805:
                case 38806:
                case 41958:
                case 44267:
                case 44518:
                case 46042:
                case 46191:
                case 47810:
                case 47811:
                case 75383:
                    //SPELL_HASH_CORRUPTION
                case 172:
                case 6222:
                case 6223:
                case 7648:
                case 11671:
                case 11672:
                case 13530:
                case 16402:
                case 16985:
                case 17510:
                case 18088:
                case 18376:
                case 18656:
                case 21068:
                case 23642:
                case 25311:
                case 27216:
                case 28829:
                case 30938:
                case 31405:
                case 32063:
                case 32197:
                case 37113:
                case 37961:
                case 39212:
                case 39621:
                case 41988:
                case 47782:
                case 47812:
                case 47813:
                case 56898:
                case 57645:
                case 58811:
                case 60016:
                case 61563:
                case 65810:
                case 68133:
                case 68134:
                case 68135:
                case 70602:
                case 70904:
                case 71937:
                    break;
                default:
                    continue;
                }
            }
            break;
            }
        }

        if (spellId == 17364 || spellId == 32175 || spellId == 32176)   //Stormstrike
            continue;
        if (spellId == 22858 && isInBack(victim))       //retatliation needs target to be not in front. Can be cast by creatures too
            continue;

        spell_proc->castSpell(victim, CastingSpell);

        if (origId == 39805)
        {
            removeAllAurasById(39805);          // Remove lightning overload aura after procing
        }

        if (spell_proc->getCreatedByAura() != nullptr)
            happenedProcs.push_back(spell_proc);
    }

    if (!happenedProcs.empty())
    {
        for (auto procItr = happenedProcs.begin(); procItr != happenedProcs.end();)
        {
            auto proc = *procItr;
            if (proc->getCreatedByAura() != nullptr)
                proc->getCreatedByAura()->removeCharge();

            procItr = happenedProcs.erase(procItr);
        }
    }

    // Leaving old hackfixes commented here -Appled
    /*switch (iter2->second.spellId)
    {
        case 43339: // Shaman - Shamanist Focus
        {
            switch (CastingSpell->getId())
            {
                //SPELL_HASH_EARTH_SHOCK
            case 8042:
            case 8044:
            case 8045:
            case 8046:
            case 10412:
            case 10413:
            case 10414:
            case 13281:
            case 13728:
            case 15501:
            case 22885:
            case 23114:
            case 24685:
            case 25025:
            case 25454:
            case 26194:
            case 43305:
            case 47071:
            case 49230:
            case 49231:
            case 54511:
            case 56506:
            case 57783:
            case 60011:
            case 61668:
            case 65973:
            case 68100:
            case 68101:
            case 68102:
                //SPELL_HASH_FLAME_SHOCK
            case 8050:
            case 8052:
            case 8053:
            case 10447:
            case 10448:
            case 13729:
            case 15039:
            case 15096:
            case 15616:
            case 16804:
            case 22423:
            case 23038:
            case 25457:
            case 29228:
            case 32967:
            case 34354:
            case 39529:
            case 39590:
            case 41115:
            case 43303:
            case 49232:
            case 49233:
            case 51588:
            case 55613:
            case 58940:
            case 58971:
            case 59684:
                //SPELL_HASH_FROST_SHOCK
            case 8056:
            case 8058:
            case 10472:
            case 10473:
            case 12548:
            case 15089:
            case 15499:
            case 19133:
            case 21030:
            case 21401:
            case 22582:
            case 23115:
            case 25464:
            case 29666:
            case 34353:
            case 37332:
            case 37865:
            case 38234:
            case 39062:
            case 41116:
            case 43524:
            case 46180:
            case 49235:
            case 49236:
                break;
            default:
                continue;
            }
        }
        break;
        case 12043: // Mage - Presence of Mind
        {
            //if (!sd->CastTime||sd->CastTime>10000) continue;
            if (spell_cast_time->CastTime == 0)
                continue;
        }
        break;
        case 17116: // Shaman - Nature's Swiftness
        case 16188: // Druid - Nature's Swiftness
        {
            //if (CastingSpell->School!=SCHOOL_NATURE||(!sd->CastTime||sd->CastTime>10000)) continue;
            if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_NATURE) || spell_cast_time->CastTime == 0)
                continue;
        }
        break;
        case 16166:
        {
            if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE || CastingSpell->getSchoolMask() & SCHOOL_MASK_FROST || CastingSpell->getSchoolMask() & SCHOOL_MASK_NATURE))
                continue;
        }
        break;
        case 14177: // Cold blood will get removed on offensive spell
        {
            if (!(CastingSpell->getSpellFamilyFlags(0) & 0x6820206 || CastingSpell->getSpellFamilyFlags(1) & 0x240009))
                continue;
        }
        break;
        case 46916: // Bloodsurge - Slam! effect should dissapear after casting Slam only
        {
            switch (CastingSpell->getId())
            {
                //SPELL_HASH_SLAM
            case 1464:
            case 8820:
            case 11430:
            case 11604:
            case 11605:
            case 25241:
            case 25242:
            case 34620:
            case 47474:
            case 47475:
            case 50782:
            case 50783:
            case 52026:
            case 67028:
                break;
            default:
                continue;
            }

        }
        break;
        case 60503: // Taste for Blood should dissapear after casting Overpower
        {
            switch (CastingSpell->getId())
            {
                //SPELL_HASH_OVERPOWER
            case 7384:
            case 7887:
            case 11584:
            case 11585:
            case 14895:
            case 17198:
            case 24407:
            case 32154:
            case 37321:
            case 37529:
            case 43456:
            case 58516:
            case 65924:
                break;
            default:
                continue;
            }
        }
        break;
        case 23694: // Imp. Hamstring
        {
            switch (CastingSpell->getId())
            {
                //SPELL_HASH_IMPROVED_HAMSTRING
            case 12289:
            case 12668:
            case 23694:
            case 23695:
            case 24428:
                break;
            default:
                continue;
            }
        }
        break;
        case 65156: // Juggernaut
        {
            switch (CastingSpell->getId())
            {
                //SPELL_HASH_MORTAL_STRIKE
            case 9347:
            case 12294:
            case 13737:
            case 15708:
            case 16856:
            case 17547:
            case 19643:
            case 21551:
            case 21552:
            case 21553:
            case 24573:
            case 25248:
            case 27580:
            case 29572:
            case 30330:
            case 31911:
            case 32736:
            case 35054:
            case 37335:
            case 39171:
            case 40220:
            case 43441:
            case 43529:
            case 44268:
            case 47485:
            case 47486:
            case 57789:
            case 65926:
            case 67542:
            case 68782:
            case 68783:
            case 68784:
            case 71552:
                //SPELL_HASH_SLAM
            case 1464:
            case 8820:
            case 11430:
            case 11604:
            case 11605:
            case 25241:
            case 25242:
            case 34620:
            case 47474:
            case 47475:
            case 50782:
            case 50783:
            case 52026:
            case 67028:
                break;
            default:
                continue;
            }
        }
        break;
    }*/

    if (can_delete)   //are we the upper level of nested procs ? If yes then we can remove the lock
        m_isProcInUse = false;

    return resisted_dmg;
}

//damage shield is a triggered spell by owner to atacker
void Unit::handleProcDmgShield(uint32_t flag, Unit* attacker)
{
    //make sure we do not loop dmg procs
    if (this == attacker || !attacker)
        return;
    if (m_damageShieldsInUse)
        return;
    m_damageShieldsInUse = true;

    //charges are already removed in handleproc
    for (std::list<DamageProc>::iterator i = m_damageShields.begin(); i != m_damageShields.end();)    // Deal Damage to Attacker
    {
        std::list<DamageProc>::iterator i2 = i++; //we should not proc on proc.. not get here again.. not needed.Better safe then sorry.
        if ((flag & (*i2).m_flags))
        {
            {
                if (const auto spellInfo = sSpellMgr.getSpellInfo((*i2).m_spellId))
                {
                    sendMessageToSet(SmsgSpellDamageShield(this->getGuid(), attacker->getGuid(), spellInfo->getId(), (*i2).m_damage, spellInfo->getSchoolMask()).serialise().get(), true);
                    addSimpleDamageBatchEvent((*i2).m_damage, this);
                }
            }
        }
    }
    m_damageShieldsInUse = false;
}

