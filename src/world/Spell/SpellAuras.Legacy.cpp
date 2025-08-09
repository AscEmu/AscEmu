/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Item.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Stats.h"
#include "Management/Battleground/Battleground.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Map/Management/MapMgr.hpp"
#include "SpellAura.hpp"
#include "Definitions/SpellModifierType.hpp"
#include "SpellHelpers.h"
#include "SpellMgr.hpp"
#include "Definitions/ProcFlags.hpp"
#include "Definitions/AuraInterruptFlags.hpp"
#include "Definitions/SpellSchoolConversionTable.hpp"
#include "Definitions/SpellTypes.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Definitions/PowerType.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Packets/MsgChannelUpdate.h"
#include "Server/Packets/SmsgPlayerVehicleData.h"
#include "Server/Packets/SmsgSetForceReactions.h"
#include "Server/Packets/SmsgControlVehicle.h"
#include "Server/Packets/SmsgCancelCombat.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Objects/Units/ThreatHandler.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Server/EventMgr.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "CommonTime.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

using AscEmu::World::Spell::Helpers::decimalToMask;

Player* Aura::GetPlayerCaster()
{
    //caster and target are the same
    if (m_casterGuid == m_target->getGuid())
    {
        if (m_target->isPlayer())
            return static_cast<Player*>(m_target);

        return nullptr;
    }

    if (m_target->getWorldMap())
        return m_target->getWorldMap()->getPlayer(WoWGuid::getGuidLowPartFromUInt64(m_casterGuid));

    return nullptr;
}

Unit* Aura::GetUnitTarget()
{
    if (m_target && m_target->getWorldMap())
        return m_target;
    
    return nullptr;
}

Player* Aura::GetPlayerTarget()
{
    if (p_target && p_target->getWorldMap())
        return p_target;

    return nullptr;
}

bool Aura::IsPassive() const { if (!m_spellInfo) return false; return (m_spellInfo->isPassive() && !m_areaAura); }

Unit* Aura::GetUnitCaster()
{
    if (m_casterGuid == m_target->getGuid())
        return m_target;

    if (m_target->getWorldMap())
        return m_target->getWorldMap()->getUnit(m_casterGuid);

    return nullptr;
}

Aura::Aura(SpellInfo const* proto, int32_t duration, Object* caster, Unit* target, bool temporary, Item* i_caster)
{
    m_castInDuel = false;
    m_temporary = temporary; // Aura saving related
    m_deleted = false;
    m_ignoreunapply = false;

    if (i_caster != nullptr)
    {
        m_castedItemId = i_caster->getItemProperties()->ItemId;
        itemCasterGUID = i_caster->getGuid();
    }
    else
    {
        m_castedItemId = 0;
        itemCasterGUID = 0;
    }

    // Modifies current aura duration based on its mechanic type
    if (p_target && getMaxDuration() > 0)
    {
        int32_t DurationModifier = p_target->m_mechanicDurationPctMod[Spell::GetMechanic(proto)];
        if (DurationModifier < -100)
            DurationModifier = -100; // Can't reduce by more than 100%
        setMaxDuration((getMaxDuration() * (100 + DurationModifier)) / 100);
    }

    // SetCasterFaction(caster->getServersideFaction());

    // m_auraSlot = 0;
    m_dynamicValue = 0;
    m_areaAura = false;

    if (caster->isCreatureOrPlayer())
    {
        if (p_target && caster->isPlayer())
        {
            if (p_target->getDuelPlayer() == static_cast<Player*>(caster))
            {
                m_castInDuel = true;
            }
        }
    }

    m_visualSlot = 0xFF;
    pSpellId = 0;
    // sLogger.info("Aura::Constructor {} ({}) from {}.", m_spellProto->getId(), m_spellProto->Name, m_target->getGuidLow());
    m_auraSlot = 0xffff;
    m_interrupted = -1;
    m_flags = 0;

    m_casterfaction = 0;

    // APGL End
    // MIT Start
    //\todo Zyres: We should create Auras in a function to check these pointers. To assert in a constructor after using at least on of the pointers before is bad codestyle ;)
    ASSERT(target != nullptr && proto != nullptr);

    m_spellInfo = proto;
    m_casterGuid = caster->getGuid();
    m_target = target;
    if (m_target->isPlayer())
        p_target = dynamic_cast<Player*>(m_target);

    mPositive = !getSpellInfo()->isNegativeAura();

    // Initialize aura charges
    auto charges = getSpellInfo()->getProcCharges();
    if (caster->isCreatureOrPlayer())
        static_cast<Unit*>(caster)->applySpellModifiers(SPELLMOD_CHARGES, &charges, getSpellInfo(), nullptr, this);

    m_charges = static_cast<uint16_t>(charges);
    m_originalCharges = m_charges;

    m_updatingModifiers = true;

    // Initialize aura effect variables
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        m_auraEffects[i].setAuraEffectType(SPELL_AURA_NONE);
        m_auraEffects[i].setEffectDamage(0.0f);
        m_auraEffects[i].setEffectBaseDamage(0);
        m_auraEffects[i].setEffectFixedDamage(0);
        m_auraEffects[i].setEffectMiscValue(0);
        m_auraEffects[i].setEffectAmplitude(0);
        m_auraEffects[i].setEffectDamageFraction(0.0f);
        m_auraEffects[i].setEffectPercentModifier(1.0f);
        m_auraEffects[i].setEffectDamageStatic(false);
        m_auraEffects[i].setEffectIndex(0);
        m_auraEffects[i].setAura(nullptr);

        // Initialize periodic timer
        m_periodicTimer[i] = 0;
    }

    if (!IsPassive())
        m_originalDuration = duration;

    _calculateSpellPowerBonus();
    _calculateAttackPowerBonus();
    _calculateSpellHaste();
    _calculateCritChance();

    // Set duration after haste calculation
    setMaxDuration(static_cast<int32_t>(m_originalDuration * m_spellHaste));
    setTimeLeft(getMaxDuration());

    // Call aura script hook
    sScriptMgr.callScriptedAuraOnCreate(this);

    // MIT End
    // APGL Start
}

Aura::~Aura()
{
    sEventMgr.RemoveEvents(this);
    m_usedModifiers.clear();
}

void Aura::EventUpdateGroupAA(AuraEffectModifier* /*aurEff*/, float r)
{
    Player* owner = m_target->getPlayerOwnerOrSelf();
    if (owner == nullptr)
    {
        targets.clear();
        return;
    }

    if (!owner->isInGroup())
    {
        if (m_target->getGuid() != owner->getGuid())
        {
            if ((m_target->getDistanceSq(owner) <= r))
            {
                if (!owner->hasAurasWithId(m_spellInfo->getId()))
                    targets.insert(owner->getGuid());
            }
            else
            {
                if (owner->hasAurasWithId(m_spellInfo->getId()))
                {
                    targets.erase(owner->getGuidLow());
                    owner->removeAllAurasById(m_spellInfo->getId());
                }
            }
        }
    }
    else
    {
        owner->getGroup()->Lock();

        SubGroup* sg = owner->getGroup()->GetSubGroup(owner->getSubGroupSlot());
        for (const auto cachedCharacterInfo : sg->getGroupMembers())
        {
            Player* op = sObjectMgr.getPlayer(cachedCharacterInfo->guid);

            if (op == nullptr)
                continue;

            if (m_target->getDistanceSq(op) > r)
                continue;

            if (m_target->GetInstanceID() != op->GetInstanceID())
                continue;

            if ((m_target->GetPhase() & op->GetPhase()) == 0)
                continue;

            if (!op->isAlive())
                continue;

            if (op->hasAurasWithId(m_spellInfo->getId()))
                continue;

            targets.insert(op->getGuid());
        }

        owner->getGroup()->Unlock();
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Player* tp = m_target->getWorldMap()->getPlayer(WoWGuid::getGuidLowPartFromUInt64(*itr2));

        bool removable = false;
        if (tp == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (m_target->getDistanceSq(tp) > r)
            removable = true;

        if ((m_target->GetPhase() & tp->GetPhase()) == 0)
            removable = true;

        if ((tp->getGuid() != owner->getGuid()) && !tp->isInGroup())
            removable = true;
        else
        {
            if (owner->isInGroup())
            {
                if (owner->getGroup()->GetID() != tp->getGroup()->GetID())
                    removable = true;

                if (owner->getSubGroupSlot() != tp->getSubGroupSlot())
                    removable = true;
            }
        }

        if (removable)
        {
            targets.erase(itr2);
            tp->removeAllAurasById(m_spellInfo->getId());
        }
    }
}

void Aura::EventUpdateRaidAA(AuraEffectModifier* /*aurEff*/, float r)
{
    Player* owner = m_target->getPlayerOwnerOrSelf();
    if (owner == nullptr)
    {
        targets.clear();
        return;
    }

    if (!owner->isInGroup())
    {
        if (m_target->getGuid() != owner->getGuid())
        {
            if ((m_target->getDistanceSq(owner) <= r))
            {
                if (!owner->hasAurasWithId(m_spellInfo->getId()))
                    targets.insert(owner->getGuid());
            }
            else
            {
                if (owner->hasAurasWithId(m_spellInfo->getId()))
                {
                    targets.erase(owner->getGuidLow());
                    owner->removeAllAurasById(m_spellInfo->getId());
                }
            }
        }

    }
    else
    {
        const auto group = owner->getGroup();

        group->Lock();

        for (uint32_t i = 0; i < group->GetSubGroupCount(); i++)
        {
            SubGroup* sg = group->GetSubGroup(i);

            for (const auto& cachedCharacterInfo : sg->getGroupMembers())
            {
                Player* op = sObjectMgr.getPlayer(cachedCharacterInfo->guid);

                if (op == nullptr)
                    continue;

                if (op->GetInstanceID() != m_target->GetInstanceID())
                    continue;

                if (m_target->getDistanceSq(op) > r)
                    continue;

                if ((m_target->GetPhase() & op->GetPhase()) == 0)
                    continue;

                if (!op->isAlive())
                    continue;

                if (op->hasAurasWithId(m_spellInfo->getId()))
                    continue;

                targets.insert(op->getGuid());
            }
        }

        group->Unlock();
    }

    // Check for targets that should be no longer affected
    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Player* tp = m_target->getWorldMap()->getPlayer(WoWGuid::getGuidLowPartFromUInt64(*itr2));
        bool removable = false;

        if (tp == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (m_target->getDistanceSq(tp) > r)
            removable = true;

        if ((m_target->GetPhase() & tp->GetPhase()) == 0)
            removable = true;

        if ((tp->getGuid() != owner->getGuid()) && !tp->isInGroup())
            removable = true;

        if (removable)
        {
            targets.erase(itr2);
            tp->removeAllAurasById(m_spellInfo->getId());
        }
    }
}

void Aura::EventUpdatePetAA(AuraEffectModifier* aurEff, float r)
{
    const auto pet = m_target->getPet();
    if (pet == nullptr)
        return;

    if (m_target->getDistanceSq(pet) > r)
    {
        pet->removeAllAurasByIdForGuid(m_spellInfo->getId(), m_target->getGuid());
    }
    else
    {
        if (pet->isAlive() && pet->getAuraWithIdForGuid(m_spellInfo->getId(), m_target->getGuid()) == nullptr)
        {
            auto a = sSpellMgr.newAura(m_spellInfo, getTimeLeft(), m_target, pet, true);
            a->m_areaAura = true;
            a->addAuraEffect(aurEff->getAuraEffectType(), aurEff->getEffectDamage(), aurEff->getEffectMiscValue(), aurEff->getEffectPercentModifier(), true, aurEff->getEffectIndex());
            pet->addAura(std::move(a));
        }
    }
}

void Aura::EventUpdateFriendAA(AuraEffectModifier* /*aurEff*/, float r)
{
    Unit* u = m_target;
    if (u == nullptr)
        return;

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        Object* o = itr;

        if (!o || !o->isCreatureOrPlayer())
            continue;

        Unit* ou = static_cast<Unit*>(o);

        if (u->getDistanceSq(ou) > r)
            continue;

        if ((u->GetPhase() & ou->GetPhase()) == 0)
            continue;

        if (!ou->isAlive())
            continue;

        if (u->isHostileTo(ou))
            continue;

        if (u->isNeutralTo(ou))
            continue;

        if (ou->hasAurasWithId(m_spellInfo->getId()))
            continue;

        targets.insert(ou->getGuid());
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Unit* tu = u->getWorldMap()->getUnit(*itr2);
        bool removable = false;

        if (tu == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (u->getDistanceSq(tu) > r)
            removable = true;

        if (u->isHostileTo(tu))
            removable = true;

        if (u->isNeutralTo(tu))
            removable = true;

        if ((u->GetPhase() & tu->GetPhase()) == 0)
            removable = true;

        if (removable)
        {
            tu->removeAllAurasById(m_spellInfo->getId());
            targets.erase(itr2);
        }
    }
}

void Aura::EventUpdateEnemyAA(AuraEffectModifier* /*aurEff*/, float r)
{
    Unit* u = m_target;
    if (u == nullptr)
        return;

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        Object* o = itr;

        if (!o || !o->isCreatureOrPlayer())
            continue;

        Unit* ou = static_cast<Unit*>(o);

        if (u->getDistanceSq(ou) > r)
            continue;

        if ((u->GetPhase() & ou->GetPhase()) == 0)
            continue;

        if (!ou->isAlive())
            continue;

        if (!u->isHostileTo(ou))
            continue;

        if (ou->hasAurasWithId(m_spellInfo->getId()))
            continue;

        targets.insert(ou->getGuid());
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Unit* tu = u->getWorldMap()->getUnit(*itr2);
        bool removable = false;

        if (tu == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (u->getDistanceSq(tu) > r)
            removable = true;

        if (!u->isHostileTo(tu))
            removable = true;

        if (u->isNeutralTo(tu))
            removable = true;

        if ((u->GetPhase() & tu->GetPhase()) == 0)
            removable = true;

        if (removable)
        {
            tu->removeAllAurasById(m_spellInfo->getId());
            targets.erase(itr2);
        }
    }
}

void Aura::EventUpdateOwnerAA(AuraEffectModifier* aurEff, float r)
{
    Object* o = getOwner();
    if (!o->isCreature())
        return;

    Creature* c = static_cast<Creature*>(o);
    if (!c->isSummon())
        return;

    Unit* ou = nullptr;
    ou = static_cast< Summon* >(c)->getUnitOwner();

    if (ou == nullptr)
        return;

    if (ou->isAlive() &&
        !ou->hasAurasWithId(m_spellInfo->getId()) &&
        (c->getDistanceSq(ou) <= r))
    {

        auto a = sSpellMgr.newAura(m_spellInfo, getTimeLeft(), c, ou, true);
        a->m_areaAura = true;
        a->addAuraEffect(aurEff->getAuraEffectType(), aurEff->getEffectDamage(), aurEff->getEffectMiscValue(), aurEff->getEffectPercentModifier(), true, aurEff->getEffectIndex());
        ou->addAura(std::move(a));
    }


    if (!ou->isAlive() || (c->getDistanceSq(ou) > r))
        ou->removeAllAurasById(m_spellInfo->getId());
}

void Aura::EventUpdateAreaAura(uint8_t effIndex, float r)
{
    /* burlex: cheap hack to get this to execute in the correct context always */
    if (event_GetCurrentInstanceId() == -1)
    {
        event_Relocate();
        return;
    }

    Unit* u_caster = GetUnitCaster();

    // if the caster is no longer valid->remove the aura
    if (u_caster == nullptr)
    {
        removeAura();
        //since we lost the caster we cannot do anything more
        return;
    }

    // Do not update area aura if caster is not in world yet
    if (!u_caster->IsInWorld())
        return;

    uint32_t AreaAuraEffectId = m_spellInfo->getAreaAuraEffect();

    switch (AreaAuraEffectId)
    {

        case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
            EventUpdateGroupAA(&m_auraEffects[effIndex], r);
            break;

        case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
            EventUpdateRaidAA(&m_auraEffects[effIndex], r);
            break;

        case SPELL_EFFECT_APPLY_PET_AREA_AURA:
            EventUpdatePetAA(&m_auraEffects[effIndex], r);
            break;

        case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
            EventUpdateFriendAA(&m_auraEffects[effIndex], r);
            break;

        case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
            EventUpdateEnemyAA(&m_auraEffects[effIndex], r);
            break;

#if VERSION_STRING >= TBC
        case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
            EventUpdateOwnerAA(&m_auraEffects[effIndex], r);
            break;
#endif

        default:
            sLogger.failure("Spell {} ({}) has tried to update Area Aura targets but Spell has no valid Area Aura effect {}.", m_spellInfo->getId(), m_spellInfo->getName(), AreaAuraEffectId);
            return;
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end(); ++itr)
    {
        auto unit = m_target->getWorldMap()->getUnit(*itr);
        if (unit == nullptr)
            return;

        if (unit->hasAurasWithId(m_spellInfo->getId()))
            continue;

        auto a = sSpellMgr.newAura(m_spellInfo, getTimeLeft(), m_target, unit, true);
        a->m_areaAura = true;
        a->addAuraEffect(m_auraEffects[effIndex].getAuraEffectType(), m_auraEffects[effIndex].getEffectDamage(), m_auraEffects[effIndex].getEffectMiscValue(), m_auraEffects[effIndex].getEffectPercentModifier(), true, m_auraEffects[effIndex].getEffectIndex());
        unit->addAura(std::move(a));
    }
}

void Aura::ClearAATargets()
{
    uint32_t spellid = m_spellInfo->getId();

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end(); ++itr)
    {
        Unit* tu = m_target->getWorldMap()->getUnit(*itr);

        if (tu == nullptr)
            continue;

        tu->removeAllAurasById(spellid);
    }
    targets.clear();

    if (m_spellInfo->hasEffect(SPELL_EFFECT_APPLY_PET_AREA_AURA))
    {
        if (auto* const pet = m_target->getPet())
            pet->removeAllAurasByIdForGuid(spellid, m_target->getGuid());
    }

#if VERSION_STRING >= TBC
    if (m_spellInfo->hasEffect(SPELL_EFFECT_APPLY_OWNER_AREA_AURA))
    {
        Unit* u = m_target->getWorldMap()->getUnit(m_target->getCreatedByGuid());

        if (u != nullptr)
            u->removeAllAurasById(spellid);

    }
#endif
}

//------------------------- Aura Effects -----------------------------

void Aura::SpellAuraModBaseResistance(AuraEffectModifier* aurEff, bool apply)
{
    SpellAuraModResistance(aurEff, apply);
    //both add/decrease some resistance difference is unknown
}

void Aura::SpellAuraModBaseResistancePerc(AuraEffectModifier* aurEff, bool apply)
{
    uint32_t Flag = aurEff->getEffectMiscValue();
    int32_t amt;
    if (apply)
    {
        amt = aurEff->getEffectDamage();
        if (amt > 0)
            mPositive = true;
        else
            mPositive = false;
    }
    else
        amt = -aurEff->getEffectDamage();

    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
    {
        if (Flag & (((uint32_t)1) << x))
        {
            if (m_target->isPlayer())
            {
                if (aurEff->getEffectDamage() > 0)
                {
                    static_cast< Player* >(m_target)->m_baseResistanceModPctPos[x] += amt;
                }
                else
                {
                    static_cast< Player* >(m_target)->m_baseResistanceModPctNeg[x] -= amt;
                }
                static_cast< Player* >(m_target)->calcResistance(x);

            }
            else if (m_target->isCreature())
            {
                static_cast< Creature* >(m_target)->BaseResistanceModPct[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraBindSight(AuraEffectModifier* /*aurEff*/, bool apply)
{
    mPositive = true;
    // MindVision
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;

    if (apply)
        caster->setFarsightGuid(m_target->getGuid());
    else
        caster->setFarsightGuid(0);
}

void Aura::SpellAuraModPossess(AuraEffectModifier* /*aurEff*/, bool apply)
{
    Unit* caster = GetPlayerCaster();

    if (apply)
    {
        if (caster != nullptr && caster->IsInWorld())
            caster->possess(m_target);
    }
    else
    {
        if (caster != nullptr && caster->IsInWorld())
        {
            caster->unPossess();
            m_target->removeAllAurasById(getSpellId());
        }

        // make sure Player::UnPossess() didn't fail, if it did we will just free the target here
        if (m_target->getCharmedByGuid() != 0)
        {
            if (m_target->isCreature())
            {
                m_target->setAItoUse(true);
                m_target->m_redirectSpellPackets = nullptr;
            }

            m_target->setCharmedByGuid(0);
            m_target->removeUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);
            m_target->setFaction(m_target->getCharmTempVal());
            m_target->updateInRangeOppositeFactionSet();
        }
        else
        {
            //mob woke up and realized he was controlled. He will turn to controller and also notify the other mobs he is fighting that they should attack the caster
            //sadly i got only 3 test cases about this so i might be wrong :(
            //zack : disabled until tested
            m_target->getAIInterface()->eventChangeFaction(caster);
        }
    }
}

void Aura::SpellAuraModConfuse(AuraEffectModifier* aurEff, bool apply)
{
    Unit* u_caster = GetUnitCaster();

    if (m_target->isTotem())
        return;

    if (apply)
    {
        if (u_caster == nullptr) return;

        // Check Mechanic Immunity
        if (m_target->m_mechanicsDispels[MECHANIC_DISORIENTED]
            || (m_spellInfo->getMechanicsType() == MECHANIC_POLYMORPHED && m_target->m_mechanicsDispels[MECHANIC_POLYMORPHED])
            )
        {
            m_flags |= 1 << aurEff->getEffectIndex();
            return;
        }
        mPositive = false;
        m_target->addUnitFlags(UNIT_FLAG_CONFUSED);
        m_target->interruptSpell();

        m_target->setAItoUse(true);
        m_target->setControlled(true, UNIT_STATE_CONFUSED);
        m_target->getThreatManager().evaluateSuppressed();

        if (p_target)
        {
            // this is a hackfix to stop player from moving -> see AIInterface::_UpdateMovement() Wander AI for more info
            p_target->sendClientControlPacket(m_target, 0);

            p_target->speedCheatDelay(getTimeLeft());
        }
    }
    else if ((m_flags & (1 << aurEff->getEffectIndex())) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        m_target->setControlled(false, UNIT_STATE_CONFUSED);
        m_target->removeUnitFlags(UNIT_FLAG_CONFUSED);
        if (p_target)
            p_target->speedCheatReset();

        if (p_target)
        {
            // re-enable movement
            p_target->sendClientControlPacket(m_target, 1);

            m_target->setAItoUse(false);
        }
        else
        {
            m_target->getAIInterface()->onHostileAction(u_caster);
        }
    }
}

void Aura::SpellAuraModCharm(AuraEffectModifier* aurEff, bool apply)
{
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;
    if (!m_target->isCreature())
        return;

    Creature* target = static_cast< Creature* >(m_target);

    if (target->isTotem())
        return;

    mPositive = true; //we ignore the other 2 effect of this spell and force it to be a positive spell

    if (apply)
    {
        if ((int32_t)m_target->getLevel() > aurEff->getEffectDamage() || m_target->isPet())
            return;

        // this should be done properly
        if (target->GetEnslaveCount() >= 10)
            return;

        if (caster->getCharmGuid() != 0)
            return;

        m_target->addUnitStateFlag(UNIT_STATE_CHARMED);
        m_target->setCharmTempVal(m_target->getFactionTemplate());
        m_target->setFaction(caster->getFactionTemplate());
        m_target->updateInRangeOppositeFactionSet();
        m_target->getAIInterface()->Init(m_target, caster);
        m_target->setCharmedByGuid(caster->getGuid());
        caster->setCharmGuid(target->getGuid());
        //damn it, the other effects of enslave demon will agro him on us anyway :S
        m_target->getThreatManager().clearAllThreat();

        target->SetEnslaveCount(target->GetEnslaveCount() + 1);

        if (caster->getSession())   // crashfix
        {
            WorldPacket data(SMSG_PET_SPELLS, 500);
            data << target->getGuid();
            data << uint16_t(0);
            data << uint32_t(0x1000);
            data << uint32_t(0x100);
            data << uint32_t(PET_SPELL_ATTACK);
            data << uint32_t(PET_SPELL_FOLLOW);
            data << uint32_t(PET_SPELL_STAY);
            for (uint8_t i = 0; i < 4; i++)
                data << uint32_t(0);
            data << uint32_t(PET_SPELL_AGRESSIVE);
            data << uint32_t(PET_SPELL_DEFENSIVE);
            data << uint32_t(PET_SPELL_PASSIVE);
            caster->getSession()->SendPacket(&data);
            target->SetEnslaveSpell(m_spellInfo->getId());
        }
    }
    else
    {
        m_target->removeUnitStateFlag(UNIT_STATE_CHARMED);
        m_target->setFaction(m_target->getCharmTempVal());
        m_target->getThreatManager().clearAllThreat();
        m_target->updateInRangeOppositeFactionSet();
        m_target->getAIInterface()->Init(m_target);
        m_target->setCharmedByGuid(0);

        if (caster->getSession() != nullptr)   // crashfix
        {
            caster->setCharmGuid(0);
            WorldPacket data(SMSG_PET_SPELLS, 8);
            data << uint64_t(0);
            caster->getSession()->SendPacket(&data);
            target->SetEnslaveSpell(0);
        }
    }
}

void Aura::SpellAuraModFear(AuraEffectModifier* aurEff, bool apply)
{
    Unit* u_caster = GetUnitCaster();

    if (m_target->isCreature() &&
        (m_target->isTotem() || m_target->isRooted()))
        return;

    if (apply)
    {
        if (u_caster == nullptr) return;
        // Check Mechanic Immunity
        if (m_target->m_mechanicsDispels[MECHANIC_FLEEING])
        {
            m_flags |= 1 << aurEff->getEffectIndex();
            return;
        }

        mPositive = false;
        m_target->addUnitFlags(UNIT_FLAG_FLEEING);
        m_target->setAItoUse(true);
        m_target->getAIInterface()->handleEvent(EVENT_FEAR, u_caster, 0);
        m_target->m_fearModifiers++;
        if (p_target)
        {
            // this is a hackfix to stop player from moving -> see AIInterface::_UpdateMovement() Fear AI for more info
            p_target->sendClientControlPacket(m_target, 0);

            p_target->speedCheatDelay(getTimeLeft());
        }
    }
    else if ((m_flags & (1 << aurEff->getEffectIndex())) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        m_target->m_fearModifiers--;

        if (m_target->m_fearModifiers <= 0)
        {
            m_target->removeUnitFlags(UNIT_FLAG_FLEEING);
            m_target->getAIInterface()->handleEvent(EVENT_UNFEAR, nullptr, 0);

            if (p_target)
            {
                // re-enable movement
                p_target->sendClientControlPacket(m_target, 1);

                m_target->setAItoUse(false);
                p_target->speedCheatReset();
            }
            else
            {
                m_target->getAIInterface()->onHostileAction(u_caster);
            }
        }
    }
}

void Aura::SpellAuraModAttackSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectDamage() < 0)
        mPositive = false;
    else
        mPositive = true;

    if (apply)
    {
        m_target->modAttackSpeedModifier(MELEE, aurEff->getEffectDamage());
        //\ todo: confirm this for other versions!
#if VERSION_STRING == Classic
        m_target->modAttackSpeedModifier(OFFHAND, aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(RANGED, aurEff->getEffectDamage());
#endif
    }
    else
    {
        m_target->modAttackSpeedModifier(MELEE, -aurEff->getEffectDamage());
        //\ todo: confirm this for other versions!
#if VERSION_STRING == Classic
        m_target->modAttackSpeedModifier(OFFHAND, -aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(RANGED, -aurEff->getEffectDamage());
#endif
    }

    if (m_target->isPlayer())
        static_cast<Player*>(m_target)->updateStats();
}

void Aura::SpellAuraModThreatGenerated(AuraEffectModifier* aurEff, bool apply)
{
    aurEff->getEffectDamage() < 0 ? mPositive = true : mPositive = false;
    for (uint32_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            if (apply)
                m_target->modGeneratedThreatModifyer(x, aurEff->getEffectDamage());
            else
                m_target->modGeneratedThreatModifyer(x, -(aurEff->getEffectDamage()));
        }
    }
}

void Aura::SpellAuraModTaunt(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    Unit* m_caster = GetUnitCaster();

    if (!m_caster || !m_caster->isAlive() || !m_target->isAlive() || !m_target->getThreatManager().canHaveThreatList())
        return;

    mPositive = false;

    m_target->getThreatManager().tauntUpdate();
}

void Aura::SpellAuraModDetaunt(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    Unit* caster = GetUnitCaster();

    if (!caster || !caster->isAlive() || !m_target->isAlive() || !caster->getThreatManager().canHaveThreatList())
        return;

    caster->getThreatManager().tauntUpdate();
}

void Aura::SpellAuraModStun(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Check Mechanic Immunity
        // Stun is a tricky one... it's used for all different kinds of mechanics as a base Aura

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_ICE_BLOCK
            case 27619:
            case 36911:
            case 41590:
            case 45438:
            case 45776:
            case 46604:
            case 46882:
            case 56124:
            case 56644:
            case 62766:
            case 65802:
            case 69924:
                break;
            default:
            {
                if (isNegative())    // ice block stuns you, don't want our own spells to ignore stun effects
                {
                    if ((m_spellInfo->getMechanicsType() == MECHANIC_CHARMED &&  m_target->m_mechanicsDispels[MECHANIC_CHARMED])
                        || (m_spellInfo->getMechanicsType() == MECHANIC_INCAPACIPATED && m_target->m_mechanicsDispels[MECHANIC_INCAPACIPATED])

                        || (m_spellInfo->getMechanicsType() == MECHANIC_SAPPED && m_target->m_mechanicsDispels[MECHANIC_SAPPED])
                        || (m_target->m_mechanicsDispels[MECHANIC_STUNNED])
                        )
                    {
                        m_flags |= 1 << aurEff->getEffectIndex();
                        return;
                    }
                }
            } break;
        }
        mPositive = false;

        //\todo Zyres: is tis really the way this should work?
        m_target->m_rootCounter++;

        if (m_target->m_rootCounter == 1)
            m_target->setMoveRoot(true);

        m_target->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        m_target->m_stunned++;
        m_target->setControlled(true, UNIT_STATE_STUNNED);
        m_target->addUnitFlags(UNIT_FLAG_STUNNED);
        m_target->getThreatManager().evaluateSuppressed();

        // remove the current spell
        if (m_target->isCastingSpell())
        {
            m_target->interruptSpell();
        }

        //warrior talent - second wind triggers on stun and immobilize. This is not used as proc to be triggered always !
        Unit* caster = GetUnitCaster();
        if (caster != nullptr)
        {
            caster->eventStunOrImmobilize(m_target);
            m_target->eventStunOrImmobilize(caster, true);
        }
    }
    else if ((m_flags & (1 << aurEff->getEffectIndex())) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter--;

        if (m_target->m_rootCounter == 0)
            m_target->setMoveRoot(false);

        m_target->m_stunned--;

        if (m_target->m_stunned == 0)
        {
            m_target->setControlled(false, UNIT_STATE_STUNNED);
            m_target->removeUnitFlags(UNIT_FLAG_STUNNED);
        }

        // attack them back.. we seem to lose this sometimes for some reason
        if (m_target->isCreature())
        {
            Unit* target = GetUnitCaster();
            if (m_target->getAIInterface()->getCurrentTarget() != nullptr)
                target = m_target->getAIInterface()->getCurrentTarget();

            if (target == nullptr)
                return;

            m_target->getAIInterface()->onHostileAction(target, nullptr);
        }
    }

    /*
        if (apply)
        {
        switch(this->m_spellProto->getId())
        {
        case 652:
        case 2070:
        case 6770:
        case 6771:
        case 11297:
        case 11298:
        {
        // sap
        Unit* c = GetUnitCaster();
        if (c)
        c->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);  // remove stealth
        }break;
        case 1776:
        case 1777:
        case 1780:
        case 1781:
        case 8629:
        case 8630:
        case 11285:
        case 11286:
        case 11287:
        case 11288:
        case 12540:
        case 13579:
        case 24698:
        case 28456:
        {
        // gouge
        Unit* c = GetUnitCaster();
        if (c && c->getObjectTypeId() == TYPEID_PLAYER)
        {
        //TO< Player* >(c)->CombatModeDelay = 10;
        TO< Player* >(c)->EventAttackStop();
        c->smsg_AttackStop(m_target);
        c->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);  // remove stealth
        }
        }
        }
        }*/
}

void Aura::SpellAuraModDamageDone(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;

    if (m_target->isPlayer())
    {
        if (aurEff->getEffectDamage() > 0)
        {
            if (apply)
            {
                mPositive = true;
                val = aurEff->getEffectDamage();
            }
            else
            {
                val = -aurEff->getEffectDamage();
            }

            for (uint16_t x = 0; x < 7; ++x)
            {
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    dynamic_cast<Player*>(m_target)->modModDamageDonePositive(x, val);
            }

        }
        else
        {
            if (apply)
            {
                mPositive = false;
                val = -aurEff->getEffectDamage();
            }
            else
            {
                val = aurEff->getEffectDamage();
            }

            for (uint16_t x = 0; x < 7; ++x)
            {
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    dynamic_cast<Player*>(m_target)->modModDamageDoneNegative(x, val);
            }
        }
    }
    else if (m_target->isCreature())
    {
        if (aurEff->getEffectDamage() > 0)
        {
            if (apply)
            {
                mPositive = true;
                val = aurEff->getEffectDamage();
            }
            else
            {
                val = -aurEff->getEffectDamage();
            }

        }
        else
        {
            if (apply)
            {
                mPositive = false;
                val = aurEff->getEffectDamage();
            }
            else
            {
                val = -aurEff->getEffectDamage();
            }
        }

        for (uint32_t x = 0; x < 7; ++x)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                static_cast< Creature* >(m_target)->ModDamageDone[x] += val;
        }
    }

    if (aurEff->getEffectMiscValue() & 1)
        m_target->calculateDamage();
}

void Aura::SpellAuraModDamageTaken(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    for (uint32_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            m_target->m_damageTakenMod[x] += val;
        }
    }
}

void Aura::SpellAuraDamageShield(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        mPositive = true;
        DamageProc ds;// = new DamageShield();
        ds.m_damage = aurEff->getEffectDamage();
        ds.m_spellId = getSpellInfo()->getId();
        ds.m_school = getSpellInfo()->getFirstSchoolFromSchoolMask();
        ds.m_flags = PROC_ON_TAKEN_MELEE_HIT | PROC_ON_TAKEN_MELEE_SPELL_HIT; //maybe later we might want to add other flags too here
        ds.owner = (void*)this;
        m_target->m_damageShields.push_back(ds);
    }
    else
    {
        for (std::list<struct DamageProc>::iterator i = m_target->m_damageShields.begin(); i != m_target->m_damageShields.end(); ++i)
        {
            if (i->owner == this)
            {
                m_target->m_damageShields.erase(i);
                return;
            }
        }
    }
}

void Aura::SpellAuraModStealth(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        //Overkill must proc only if we aren't already stealthed, also refreshing duration.
        if (!m_target->isStealthed() && m_target->hasAurasWithId(58426))
        {
            Aura *buff = m_target->getAuraWithId(58427);
            if (buff)
            {
                // Spell Overkill - in stealth and 20 seconds after stealth +30% energy regeneration - -1 duration => hacky infinity
                buff->setMaxDuration(-1);
                buff->refreshOrModifyStack();
            }
            else
                m_target->castSpell(m_target, 58427, true);
        }

        if (p_target && p_target->hasBgFlag())
        {
            if (const auto battleground = p_target->getBattleground())
            {
                if (battleground->getType() == BattlegroundDef::TYPE_WARSONG_GULCH)
                    battleground->HookOnFlagDrop(p_target);

                if (battleground->getType() == BattlegroundDef::TYPE_EYE_OF_THE_STORM)
                    battleground->HookOnFlagDrop(p_target);
            }
        }

        mPositive = true;
        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
                m_target->setStandStateFlags(m_target->getStandStateFlags() | UNIT_STAND_FLAGS_CREEP);
                break;
        }

        m_target->addStandStateFlags(UNIT_STAND_FLAGS_CREEP);
#if VERSION_STRING != Mop
        if (m_target->isPlayer())
            if (const auto player = dynamic_cast<Player*>(m_target))
                player->addAuraVision(AURA_VISION_STEALTH);
#endif

        m_target->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_STEALTH | AURA_INTERRUPT_ON_INVINCIBLE);
        m_target->modStealthLevel(StealthFlag(aurEff->getEffectMiscValue()), aurEff->getEffectDamage());

        // hack fix for vanish stuff
        if (m_target->isPlayer())   // Vanish
        {
            switch (m_spellInfo->getId())
            {
                //SPELL_HASH_VANISH
                case 1856:
                case 1857:
                case 11327:
                case 11329:
                case 24223:
                case 24228:
                case 24229:
                case 24230:
                case 24231:
                case 24232:
                case 24233:
                case 24699:
                case 26888:
                case 26889:
                case 27617:
                case 29448:
                case 31619:
                case 35205:
                case 39667:
                case 41476:
                case 41479:
                case 44290:
                case 55964:
                case 71400:
                {
                    for (const auto& iter : m_target->getInRangeObjectsSet())
                    {
                        if (iter == nullptr || !iter->isCreatureOrPlayer())
                            continue;

                        Unit* _unit = static_cast<Unit*>(iter);
                        if (!_unit->isAlive())
                            continue;

                        if (_unit->isCastingSpell())
                        {
                            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
                            {
                                Spell* curSpell = _unit->getCurrentSpell(CurrentSpellType(i));
                                if (curSpell != nullptr && curSpell->getUnitTarget() == m_target)
                                {
                                    _unit->interruptSpellWithSpellType(CurrentSpellType(i));
                                }
                            }
                        }
                        if(_unit->getThreatManager().canHaveThreatList())
                            _unit->getThreatManager().clearThreat(m_target);

                    }

                    m_target->getCombatHandler().clearCombat();

                    SpellMechanic mechanicList[] =
                    {
                        MECHANIC_ROOTED,
                        MECHANIC_ENSNARED,
                        MECHANIC_NONE,
                    };
                    m_target->removeAllAurasBySpellMechanic(mechanicList, true);

                    // Cast stealth spell/dismount/drop BG flag
                    if (p_target != nullptr)
                    {
                        p_target->castSpell(p_target, 1784, true);

                        p_target->dismount();

                        if (p_target->getBattleground() && p_target->hasBgFlag())
                        {
                            if (p_target->getBattleground()->getType() == BattlegroundDef::TYPE_WARSONG_GULCH || p_target->getBattleground()->getType() == BattlegroundDef::TYPE_EYE_OF_THE_STORM)
                            {
                                p_target->getBattleground()->HookOnFlagDrop(p_target);
                            }
                        }
                    }
                } break;
            }
        }
    }
    else
    {
        m_target->modStealthLevel(StealthFlag(aurEff->getEffectMiscValue()), -aurEff->getEffectDamage());

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_VANISH
            case 1856:
            case 1857:
            case 11327:
            case 11329:
            case 24223:
            case 24228:
            case 24229:
            case 24230:
            case 24231:
            case 24232:
            case 24233:
            case 24699:
            case 26888:
            case 26889:
            case 27617:
            case 29448:
            case 31619:
            case 35205:
            case 39667:
            case 41476:
            case 41479:
            case 44290:
            case 55964:
            case 71400:
                break;
            default:
            {
                m_target->removeStandStateFlags(UNIT_STAND_FLAGS_CREEP);

                if (p_target != nullptr)
                {
#if VERSION_STRING != Mop
                    p_target->removeAuraVision(AURA_VISION_STEALTH);
#endif
                    p_target->sendSpellCooldownEventPacket(m_spellInfo->getId());

                    if (p_target->m_outStealthDamageBonusPeriod && p_target->m_outStealthDamageBonusPct)
                        p_target->m_outStealthDamageBonusTimer = (uint32_t)UNIXTIME + p_target->m_outStealthDamageBonusPeriod;
                }
            } break;
        }

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
            {
                for (uint16_t x = AuraSlots::POSITIVE_SLOT_START; x < AuraSlots::POSITIVE_SLOT_END; ++x)
                {
                    auto* const aur = m_target->getAuraWithAuraSlot(x);
                    if (aur == nullptr)
                        continue;

                    if (aur->getSpellInfo()->getEffectApplyAuraName(0) != SPELL_AURA_DUMMY)
                    {
                        uint32_t tmp_duration = 0;

                        switch (aur->getSpellInfo()->getId())
                        {
                            //SPELL_HASH_MASTER_OF_SUBTLETY
                            case 31221:
                            case 31222:
                            case 31223:
                            case 31665:
                            case 31666:
                            {
                                tmp_duration = TimeVarsMs::Second * 6;
                            } break;

                            //SPELL_HASH_OVERKILL
                            case 58426:
                            case 58427:
                            {
                                tmp_duration = TimeVarsMs::Second * 20;
                            } break;
                        }

                        if (tmp_duration != 0)
                        {
                            aur->setTimeLeft(tmp_duration);
                            aur->refreshOrModifyStack();

                            sEventMgr.ModifyEventTimeLeft(aur, EVENT_AURA_REMOVE, tmp_duration);
                            sEventMgr.AddEvent(aur, &Aura::removeAura, AURA_REMOVE_ON_EXPIRE, EVENT_AURA_REMOVE, tmp_duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
                        }
                    }
                }
            } break;
        }
    }

    m_target->updateVisibility();
}

void Aura::SpellAuraModStealthDetection(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->modStealthDetection(StealthFlag(aurEff->getEffectMiscValue()), aurEff->getEffectDamage());
    else
        m_target->modStealthDetection(StealthFlag(aurEff->getEffectMiscValue()), -aurEff->getEffectDamage());
}

void Aura::SpellAuraModInvisibility(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = true;
    if (m_spellInfo->getEffect(aurEff->getEffectIndex()) == SPELL_EFFECT_APPLY_FRIEND_AREA_AURA)  ///\todo WTF is this crap? TODO clean this
        return;

    if (apply)
    {
        m_target->modInvisibilityLevel(InvisibilityFlag(aurEff->getEffectMiscValue()), aurEff->getEffectDamage());
        if (m_target->isPlayer())
        {
#if VERSION_STRING != Mop
            if (getSpellId() == 32612)
                if (const auto player = dynamic_cast<Player*>(m_target))
                    player->addAuraVision(AURA_VISION_INVISIBILITY);   //Mage Invis self visual
#endif
        }

        m_target->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_INVINCIBLE);
    }
    else
    {
        m_target->modInvisibilityLevel(InvisibilityFlag(aurEff->getEffectMiscValue()), -aurEff->getEffectDamage());
        if (m_target->isPlayer())
        {
#if VERSION_STRING != Mop
            if (getSpellId() == 32612)
                if (const auto player = dynamic_cast<Player*>(m_target))
                    player->removeAuraVision(AURA_VISION_INVISIBILITY);
#endif
        }
    }

    m_target->updateVisibility();
}

void Aura::SpellAuraModInvisibilityDetection(AuraEffectModifier* aurEff, bool apply)
{
    //Always Positive

    if (aurEff->getEffectMiscValue() < INVIS_FLAG_TOTAL)
    {
        if (apply)
        {
            m_target->modInvisibilityDetection(InvisibilityFlag(aurEff->getEffectMiscValue()), aurEff->getEffectDamage());
            mPositive = true;
        }
        else
            m_target->modInvisibilityDetection(InvisibilityFlag(aurEff->getEffectMiscValue()), -aurEff->getEffectDamage());

        if (m_target->isPlayer())
            m_target->updateVisibility();
    }
}

void Aura::SpellAuraModResistance(AuraEffectModifier* aurEff, bool apply)
{
    uint32_t Flag = aurEff->getEffectMiscValue();
    int32_t amt;
    if (apply)
    {
        amt = aurEff->getEffectDamage();
        if (amt < 0)//don't change it
            mPositive = false;
        else
            mPositive = true;
    }
    else
        amt = -aurEff->getEffectDamage();
    Unit* caster = GetUnitCaster();
    if (isNegative() && caster != nullptr && m_target->isCreature())
        m_target->getAIInterface()->onHostileAction(caster);

    Player* plr = GetPlayerCaster();
    if (plr != nullptr)
    {
        switch (getSpellInfo()->getId())
        {
            //SPELL_HASH_DEVOTION_AURA
            case 465:
            case 643:
            case 1032:
            case 8258:
            case 10290:
            case 10291:
            case 10292:
            case 10293:
            case 17232:
            case 27149:
            case 41452:
            case 48941:
            case 48942:
            case 52442:
            case 57740:
            case 58944:
            {
                // Increases the armor bonus of your Devotion Aura by %u - HACKY
                if (plr->hasSpell(20140))     // Improved Devotion Aura Rank 3
                    amt = (int32_t)(amt * 1.5);
                else if (plr->hasSpell(20139))     // Improved Devotion Aura Rank 2
                    amt = (int32_t)(amt * 1.34);
                else if (plr->hasSpell(20138))     // Improved Devotion Aura Rank 1
                    amt = (int32_t)(amt * 1.17);
            } break;
        }
    }

    if (m_target->isPlayer())
    {
        for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
        {
            if (Flag & (((uint32_t)1) << x))
            {
                if (aurEff->getEffectDamage() > 0)
                    static_cast< Player* >(m_target)->m_flatResistanceModifierPos[x] += amt;
                else
                    static_cast< Player* >(m_target)->m_flatResistanceModifierNeg[x] -= amt;
                static_cast< Player* >(m_target)->calcResistance(x);
            }
        }
    }
    else if (m_target->isCreature())
    {
        for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
        {
            if (Flag & (((uint32_t)1) << (uint32_t)x))
            {
                static_cast< Creature* >(m_target)->FlatResistanceMod[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraModPacify(AuraEffectModifier* /*aurEff*/, bool apply)
{
    // Can't Attack
    if (apply)
    {
        if (m_spellInfo->getId() == 24937 || m_spellInfo->getId() == 41450) //SPELL_HASH_BLESSING_OF_PROTECTION
            mPositive = true;
        else
            mPositive = false;

        m_target->m_pacified++;
        m_target->addUnitFlags(UNIT_FLAG_PACIFIED);
    }
    else
    {
        m_target->m_pacified--;

        if (m_target->m_pacified == 0)
        {
            m_target->removeUnitFlags(UNIT_FLAG_PACIFIED);
        }
    }
}

void Aura::SpellAuraModRoot(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Check Mechanic Immunity
        if (m_target->m_mechanicsDispels[MECHANIC_ROOTED])
        {
            m_flags |= 1 << aurEff->getEffectIndex();
            return;
        }

        mPositive = false;

        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter++;

        if (m_target->m_rootCounter == 1)
            m_target->setControlled(true, UNIT_STATE_ROOTED);

        //warrior talent - second wind triggers on stun and immobilize. This is not used as proc to be triggered always !
        Unit* caster = GetUnitCaster();
        if (caster != nullptr)
        {
            caster->eventStunOrImmobilize(m_target);
            m_target->eventStunOrImmobilize(caster, true);
        }

        if (getSpellInfo()->getSchoolMask() & SCHOOL_MASK_FROST && !m_target->m_ascFrozen++)
            m_target->addAuraStateAndAuras(AURASTATE_FLAG_FROZEN);

        ///\todo -Supalosa- TODO: Mobs will attack nearest enemy in range on aggro list when rooted. */
    }
    else if ((m_flags & (1 << aurEff->getEffectIndex())) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter--;

        if (m_target->m_rootCounter == 0)
            m_target->setControlled(false, UNIT_STATE_ROOTED);

        if (m_target->isCreature())
            m_target->getAIInterface()->onHostileAction(GetUnitCaster());

        if (getSpellInfo()->getSchoolMask() & SCHOOL_MASK_FROST && !--m_target->m_ascFrozen)
            m_target->removeAuraStateAndAuras(AURASTATE_FLAG_FROZEN);
    }
}

void Aura::SpellAuraModSilence(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        m_target->m_silenced++;
        m_target->addUnitFlags(UNIT_FLAG_SILENCED);

        // Interrupt target's current casted spell (either channeled or generic spell with cast time)
        if (m_target->isCastingSpell(false, true))
        {
            if (m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
            {
                m_target->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
            }
            // No need to check cast time for generic spells, checked already in Object::isCastingSpell()
            else if (m_target->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
            {
                m_target->interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
            }
        }
    }
    else
    {
        m_target->m_silenced--;

        if (m_target->m_silenced == 0)
        {
            m_target->removeUnitFlags(UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraReflectSpells(AuraEffectModifier* aurEff, bool apply)
{
    m_target->removeReflect(getSpellId(), apply);

    if (apply)
    {
        m_target->m_reflectSpellSchool.emplace_back(std::make_unique<ReflectSpellSchool>(getSpellId(), m_spellInfo->getProcCharges(), -1, aurEff->getEffectDamage(), false));
    }
}

void Aura::SpellAuraModStat(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > TBC // support classic
    int32_t stat = aurEff->getEffectMiscValue();
    int32_t val;

    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
    {
        val = -aurEff->getEffectDamage();
    }

    if (stat == -1)   //all stats
    {
        if (m_target->isPlayer())
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                if (aurEff->getEffectDamage() > 0)
                    dynamic_cast< Player* >(m_target)->m_flatStatModPos[x] += val;
                else
                    dynamic_cast< Player* >(m_target)->m_flatStatModNeg[x] -= val;

                dynamic_cast< Player* >(m_target)->calcStat(x);
            }

            dynamic_cast< Player* >(m_target)->updateStats();
            dynamic_cast< Player* >(m_target)->updateChances();
        }
        else if (m_target->isCreature())
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                dynamic_cast< Creature* >(m_target)->FlatStatMod[x] += val;
                dynamic_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else if (stat >= 0)
    {
        if (aurEff->getEffectMiscValue() < 5)
        {
            uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());
            if (m_target->isPlayer())
            {
                if (aurEff->getEffectDamage() > 0)
                    dynamic_cast<Player*>(m_target)->m_flatStatModPos[modValue] += val;
                else
                    dynamic_cast<Player*>(m_target)->m_flatStatModNeg[modValue] -= val;

                dynamic_cast<Player*>(m_target)->calcStat(modValue);

                dynamic_cast<Player*>(m_target)->updateStats();
                dynamic_cast<Player*>(m_target)->updateChances();
            }
            else if (m_target->isCreature())
            {
                dynamic_cast<Creature*>(m_target)->FlatStatMod[modValue] += val;
                dynamic_cast<Creature*>(m_target)->CalcStat(modValue);
            }
        }
    }
#endif
}

void Aura::SpellAuraModSkill(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer())
    {
        const auto skillLine = static_cast<uint16_t>(aurEff->getEffectMiscValue());
        const auto amount = static_cast<int16_t>(aurEff->getEffectDamage());
        if (apply)
        {
            if (!p_target->hasSkillLine(skillLine))
            {
                aurEff->setEffectActive(false);
                return;
            }

            mPositive = true;
            static_cast< Player* >(m_target)->modifySkillBonus(skillLine, amount, false);
        }
        else
            static_cast< Player* >(m_target)->modifySkillBonus(skillLine, -amount, false);

        static_cast< Player* >(m_target)->updateStats();
    }
}

void Aura::SpellAuraModIncreaseSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_speedModifier += aurEff->getEffectDamage();
    else
        m_target->m_speedModifier -= aurEff->getEffectDamage();

    m_target->updateSpeed();
}

void Aura::SpellAuraModIncreaseMountedSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if ((getSpellId() == 68768 || getSpellId() == 68769) && p_target != nullptr)
    {
        int32_t newspeed = 0;

        if (p_target->getSkillLineCurrent(SKILL_RIDING, true) >= 150)
            newspeed = 100;
        else if (p_target->getSkillLineCurrent(SKILL_RIDING, true) >= 75)
            newspeed = 60;

        aurEff->setEffectDamage(newspeed); // EffectBasePoints + 1 (59+1 and 99+1)
    }

    if (apply)
    {
        mPositive = true;
        m_target->m_mountedspeedModifier += aurEff->getEffectDamage();
    }
    else
        m_target->m_mountedspeedModifier -= aurEff->getEffectDamage();
    m_target->updateSpeed();
}

void Aura::SpellAuraModCreatureRangedAttackPower(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        for (uint32_t x = 0; x < 11; x++)
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->m_creatureRangedAttackPowerMod[x + 1] += aurEff->getEffectDamage();
        if (aurEff->getEffectDamage() < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
    {
        for (uint32_t x = 0; x < 11; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                m_target->m_creatureRangedAttackPowerMod[x + 1] -= aurEff->getEffectDamage();
            }
        }
    }
    m_target->calculateDamage();
}

void Aura::SpellAuraModDecreaseSpeed(AuraEffectModifier* aurEff, bool apply)
{
    //there can not be 2 slow downs only most powerful is applied
    if (apply)
    {
        // Check Mechanic Immunity
        if (m_target->m_mechanicsDispels[MECHANIC_ENSNARED])
        {
            m_flags |= 1 << aurEff->getEffectIndex();
            return;
        }
        switch (m_spellInfo->getId())
        {
            // SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
                mPositive = true;
                break;

            // SPELL_HASH_DAZED
            case 1604:
            case 5101:
            case 13496:
            case 15571:
            case 29703:
            case 35955:
            case 50259:
            case 50411:
            case 51372:
                mPositive = false;
                break;

            default:
                /* burlex: this would be better as a if (caster is hostile to target) then effect = negative) */
                if (m_casterGuid != m_target->getGuid())
                    mPositive = false;
                break;
        }

        //let's check Mage talents if we proc anything
        if (m_spellInfo->getSchoolMask() & SCHOOL_MASK_FROST)
        {
            //yes we are freezing the bastard, so can we proc anything on this ?
            Unit* caster = GetUnitCaster();
            if (caster != nullptr && caster->isPlayer())
                caster->eventChill(m_target);
            if (m_target->isPlayer() && caster)
                m_target->eventChill(caster, true);
        }
        m_target->speedReductionMap.insert(std::make_pair(m_spellInfo->getId(), aurEff->getEffectDamage()));
        //m_target->m_slowdown=this;
        //m_target->m_speedModifier += aurEff->getEffectDamage();
    }
    else if ((m_flags & (1 << aurEff->getEffectIndex())) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        std::map< uint32_t, int32_t >::iterator itr = m_target->speedReductionMap.find(m_spellInfo->getId());
        if (itr != m_target->speedReductionMap.end())
            m_target->speedReductionMap.erase(itr);
        //m_target->m_speedModifier -= aurEff->getEffectDamage();
        //m_target->m_slowdown= NULL;
    }
    if (m_target->getSpeedDecrease())
        m_target->updateSpeed();
}

void Aura::UpdateAuraModDecreaseSpeed(AuraEffectModifier* aurEff)
{
    if (m_target->m_mechanicsDispels[MECHANIC_ENSNARED])
    {
        m_flags |= 1 << aurEff->getEffectIndex();
        return;
    }

    //let's check Mage talents if we proc anything
    if (m_spellInfo->getSchoolMask() & SCHOOL_MASK_FROST)
    {
        //yes we are freezing the bastard, so can we proc anything on this ?
        Unit* caster = GetUnitCaster();
        if (caster && caster->isPlayer())
            caster->eventChill(m_target);
        if (m_target->isPlayer() && caster)
            m_target->eventChill(caster, true);
    }
}

void Aura::SpellAuraModIncreaseHealth(AuraEffectModifier* aurEff, bool apply)
{
    int32_t amt;

    if (apply)
    {
        //threat special cases. We should move these to scripted spells maybe
        switch (m_spellInfo->getId())
        {
            case 23782:// Gift of Life
                aurEff->setEffectDamage(1500);
                break;
            case 12976:// Last Stand
                aurEff->setEffectDamage((int32_t)(m_target->getMaxHealth() * 0.3));
                break;
        }
        mPositive = true;
        amt = aurEff->getEffectDamage();
    }
    else
        amt = -aurEff->getEffectDamage();

    if (m_target->isPlayer())
    {
        //maybe we should not adjust hitpoints too but only maximum health
        static_cast< Player* >(m_target)->setHealthFromSpell(static_cast< Player* >(m_target)->getHealthFromSpell() + amt);
        static_cast< Player* >(m_target)->updateStats();
        if (apply)
            m_target->modHealth(amt);
        else
        {
            if ((int32_t)m_target->getHealth() > -amt) //watch it on remove value is negative
                m_target->modHealth(amt);
            else m_target->setHealth(1); //do not kill player but do strip him good
        }
    }
    else
        m_target->modMaxHealth(amt);
}

void Aura::SpellAuraModIncreaseEnergy(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = true;
    //uint32_t powerField,maxField ;
    //uint8_t powerType = m_target->GetPowerType();

    /*if (powerType == POWER_TYPE_MANA) // Mana
    {
    powerField = UNIT_FIELD_POWER1;
    maxField = UNIT_FIELD_MAXPOWER1;
    }
    else if (powerType == POWER_TYPE_RAGE) // Rage
    {
    powerField = UNIT_FIELD_POWER2;
    maxField = UNIT_FIELD_MAXPOWER2;
    }
    else if (powerType == POWER_TYPE_ENERGY) // Energy
    {
    powerField = UNIT_FIELD_POWER4;
    maxField = UNIT_FIELD_MAXPOWER4;
    }
    else // Capt: if we can not use identify the type: do nothing
    return; */

    int32_t amount = apply ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    auto modValue = static_cast<PowerType>(aurEff->getEffectMiscValue());
    m_target->modMaxPower(modValue, amount);
    m_target->modPower(modValue, amount);

    if (modValue == 0 && m_target->isPlayer())
    {
        static_cast< Player* >(m_target)->setManaFromSpell(static_cast< Player* >(m_target)->getManaFromSpell() + amount);
    }
}

void Aura::SpellAuraModEffectImmunity(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (m_spellInfo->getId() == 24937)
        mPositive = true;

    if (!apply)
    {
        if (m_spellInfo->getId() == 23333 || m_spellInfo->getId() == 23335 || m_spellInfo->getId() == 34976)
        {
            Player* plr = GetPlayerCaster();
            if (plr == nullptr || plr->getBattleground() == nullptr)
                return;

            plr->getBattleground()->HookOnFlagDrop(plr);

        }
    }
}

void Aura::SpellAuraModStateImmunity(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    //%50 chance to dispel 1 magic effect on target
    //23922
}

void Aura::SpellAuraModSchoolImmunity(AuraEffectModifier* aurEff, bool apply)
{
    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_DIVINE_SHIELD
        case 642:
        case 13874:
        case 29382:
        case 33581:
        case 40733:
        case 41367:
        case 54322:
        case 63148:
        case 66010:
        case 67251:
        case 71550:
        //SPELL_HASH_ICE_BLOCK
        case 27619:
        case 36911:
        case 41590:
        case 45438:
        case 45776:
        case 46604:
        case 46882:
        case 56124:
        case 56644:
        case 62766:
        case 65802:
        case 69924:
        {
            if (apply)
            {
                if (!m_target->isAlive())
                    return;

                for (uint16_t i = AuraSlots::NEGATIVE_SLOT_START; i < AuraSlots::NEGATIVE_SLOT_END; ++i)
                {
                    auto* const pAura = m_target->getAuraWithAuraSlot(i);
                    if (pAura != this &&
                        pAura != nullptr &&
                        !pAura->IsPassive() &&
                        pAura->isNegative() &&
                        !(pAura->getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
                    {
                        pAura->removeAura();
                    }
                }
            }
        } break;
    }

    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_DIVINE_SHIELD
        case 642:
        case 13874:
        case 29382:
        case 33581:
        case 40733:
        case 41367:
        case 54322:
        case 63148:
        case 66010:
        case 67251:
        case 71550:
        //SPELL_HASH_BLESSING_OF_PROTECTION
        case 41450:
        //SPELL_HASH_ICE_BLOCK
        case 27619:
        case 36911:
        case 41590:
        case 45438:
        case 45776:
        case 46604:
        case 46882:
        case 56124:
        case 56644:
        case 62766:
        case 65802:
        case 69924:
        {
            if (apply)
                m_target->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_INVINCIBLE);
        } break;
    }

    if (apply)
    {
        //fix me may be negative
        Unit* c = GetUnitCaster();
        if (c)
        {
            if (c->isValidTarget(m_target))
                mPositive = false;
            else mPositive = true;
        }
        else
            mPositive = true;

        sLogger.debug("SpellAuraModSchoolImmunity called with misValue = {:x}", aurEff->getEffectMiscValue());
        m_target->removeAllAurasBySchoolMask(static_cast<SchoolMask>(getSpellInfo()->getSchoolMask()), true, true);
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
        {
            if (aurEff->getEffectMiscValue() & (1 << i))
            {
                m_target->m_schoolImmunityList[i]++;
            }
        }
        m_target->getThreatManager().evaluateSuppressed();
    }
    else
    {
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
        {
            if (aurEff->getEffectMiscValue() & (1 << i) &&
                m_target->m_schoolImmunityList[i] > 0)
            {
                m_target->m_schoolImmunityList[i]--;
            }
        }
    }
}

void Aura::SpellAuraModDmgImmunity(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
        m_target->getThreatManager().evaluateSuppressed();
}

void Aura::SpellAuraModDispelImmunity(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectMiscValue() < 10)
    {
        if (apply)
            m_target->m_dispels[aurEff->getEffectMiscValue()]++;
        else
            m_target->m_dispels[aurEff->getEffectMiscValue()]--;

        if (apply)
        {
            for (uint16_t x = AuraSlots::POSITIVE_SLOT_START; x < AuraSlots::POSITIVE_SLOT_END; ++x)
            {
                auto* const aur = m_target->getAuraWithAuraSlot(x);
                // HACK FIX FOR: 41425 and 25771
                if (aur && aur->getSpellId() != 41425 && aur->getSpellId() != 25771)
                    if (aur->getSpellInfo()->getDispelType() == (uint32_t)aurEff->getEffectMiscValue())
                        aur->removeAura();
            }
        }
    }
}

void Aura::SpellAuraProcTriggerSpell(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        uint32_t spellId;

        // Find spell of effect to be triggered
        spellId = getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex());
        if (spellId == 0)
        {
            sLogger.debug("Warning! trigger spell is null for spell {}", getSpellInfo()->getId());
            return;
        }

        // Initialize proc mask
        // TODO: investigate why blizzard is using random spells in the proc mask
        // for example: druid talent earth and moon should proc from Wrath and Starfire
        // however dbc proc mask has Moonfire and Starfall
        // another example: warrior talent juggernaut should proc from Charge
        // dbc proc mask has Slam

        //groupRelation[0] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 0);
        //groupRelation[1] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 1);
        //groupRelation[2] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 2);

        m_target->addProcTriggerSpell(spellId, getSpellInfo()->getId(), m_casterGuid, getSpellInfo()->getProcChance(), SpellProcFlags(getSpellInfo()->getProcFlags()), EXTRA_PROC_NULL, nullptr, nullptr, this);

        sLogger.debug("{} is registering {} chance {} flags {} charges {}", getSpellInfo()->getId(), spellId, getSpellInfo()->getProcChance(), getSpellInfo()->getProcFlags(), getCharges());
    }
    else
    {
        // Find spell of effect to be triggered
        uint32_t spellId = getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex());
        if (spellId == 0)
        {
            sLogger.debug("Warning! trigger spell is null for spell {}", getSpellInfo()->getId());
            return;
        }

        m_target->removeProcTriggerSpell(spellId, m_casterGuid);
    }
}

void Aura::SpellAuraProcTriggerDamage(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        DamageProc ds;
        ds.m_damage = aurEff->getEffectDamage();
        ds.m_spellId = getSpellInfo()->getId();
        ds.m_school = getSpellInfo()->getFirstSchoolFromSchoolMask();
        ds.m_flags = m_spellInfo->getProcFlags();
        ds.owner = (void*)this;
        m_target->m_damageShields.push_back(ds);
        sLogger.debug("registering dmg proc {}, school {}, flags {}, charges at least {}", ds.m_spellId, ds.m_school, ds.m_flags, m_spellInfo->getProcCharges());
    }
    else
    {
        for (std::list<struct DamageProc>::iterator i = m_target->m_damageShields.begin(); i != m_target->m_damageShields.end(); ++i)
        {
            if (i->owner == this)
            {
                m_target->m_damageShields.erase(i);
                break;
            }
        }
    }
}

void Aura::SpellAuraTrackCreatures(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            if (p_target->m_trackingSpell != 0)
                p_target->removeAllAurasById(p_target->m_trackingSpell);

            p_target->setTrackCreature((uint32_t)1 << (aurEff->getEffectMiscValue() - 1));
            p_target->m_trackingSpell = getSpellId();
        }
        else
        {
            p_target->m_trackingSpell = 0;
            p_target->setTrackCreature(0);
        }
    }
}

void Aura::SpellAuraTrackResources(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            if (p_target->m_trackingSpell != 0)
                p_target->removeAllAurasById(p_target->m_trackingSpell);

            p_target->setTrackResource((uint32_t)1 << (aurEff->getEffectMiscValue() - 1));
            p_target->m_trackingSpell = getSpellId();
        }
        else
        {
            p_target->m_trackingSpell = 0;
            p_target->setTrackResource(0);
        }
    }
}

void Aura::SpellAuraModParryPerc(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > TBC // support classic
    //if (m_target->getObjectTypeId() == TYPEID_PLAYER)
    {
        int32_t amt;
        if (apply)
        {
            amt = aurEff->getEffectDamage();
            if (amt < 0)
                mPositive = false;
            else
                mPositive = true;

        }
        else
            amt = -aurEff->getEffectDamage();

        m_target->setParryFromSpell(m_target->getParryFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->updateChances();
        }
    }
#endif
}

void Aura::SpellAuraModDodgePerc(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > TBC // support classic
    // if (m_target->getObjectTypeId() == TYPEID_PLAYER)
    {
        int32_t amt = aurEff->getEffectDamage();
        // spellModFlatIntValue(m_target->SM_FSPELL_VALUE, &amt, GetSpellProto()->SpellGroupType);
        if (apply)
        {
            if (amt < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
            amt = -amt;

        m_target->setDodgeFromSpell(m_target->getDodgeFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->updateChances();
        }
    }
#endif
}

void Aura::SpellAuraModBlockPerc(AuraEffectModifier* aurEff, bool apply)
{
    //if (m_target->getObjectTypeId() == TYPEID_PLAYER)
    {
        int32_t amt;
        if (apply)
        {
            amt = aurEff->getEffectDamage();
            if (amt < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
            amt = -aurEff->getEffectDamage();

        m_target->setBlockFromSpell(m_target->getBlockFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->updateStats();
        }
    }
}

void Aura::SpellAuraModCritPerc(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING >= TBC // support classic
    if (p_target != nullptr)
    {
        if (apply)
        {
            WeaponModifier md;
            md.value = float(aurEff->getEffectDamage());
            md.wclass = getSpellInfo()->getEquippedItemClass();
            md.subclass = getSpellInfo()->getEquippedItemSubClass();
            p_target->m_toCritChance.insert(std::make_pair(getSpellId(), md));
        }
        else
        {
            /*std::list<WeaponModifier>::iterator i = TO< Player* >(m_target)->tocritchance.begin();

            for (;i!=TO< Player* >(m_target)->tocritchance.end();i++)
            {
            if ((*i).spellid==getSpellId())
            {
            TO< Player* >(m_target)->tocritchance.erase(i);
            break;
            }
            }*/
            p_target->m_toCritChance.erase(getSpellId());
        }
        p_target->updateChances();
    }
#endif
}

void Aura::SpellAuraModHitChance(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isCreatureOrPlayer()) return;

    int32_t val = aurEff->getEffectDamage();

    if (apply)
    {
        m_target->setHitFromMeleeSpell(m_target->getHitFromMeleeSpell() + val);
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
    {
        m_target->setHitFromMeleeSpell(m_target->getHitFromMeleeSpell() - val);
        if (m_target->getHitFromMeleeSpell() < 0)
        {
            m_target->setHitFromMeleeSpell(0);
        }
    }
}

void Aura::SpellAuraModSpellHitChance(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->setHitFromSpell(p_target->getHitFromSpell() + aurEff->getEffectDamage());
            if (aurEff->getEffectDamage() < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
        {
            p_target->setHitFromSpell(p_target->getHitFromSpell() - aurEff->getEffectDamage());
            if (p_target->getHitFromSpell() < 0)
            {
                p_target->setHitFromSpell(0);
            }
        }
    }
}

void Aura::SpellAuraModSpellCritChance(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        int32_t amt;
        if (apply)
        {
            amt = aurEff->getEffectDamage();
            if (amt < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
            amt = -aurEff->getEffectDamage();

        p_target->m_spellCritPercentage += amt;
        p_target->setSpellCritFromSpell(p_target->getSpellCritFromSpell() + amt);
        p_target->updateChanceFields();
    }
}

void Aura::SpellAuraIncreaseSwimSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (m_target->isAlive())
            mPositive = true;

        m_target->setSpeedRate(TYPE_SWIM, 0.04722222f * (100 + aurEff->getEffectDamage()), true);
    }
    else
        m_target->setSpeedRate(TYPE_SWIM, m_target->getSpeedRate(TYPE_SWIM, false), false);

    if (p_target != nullptr)
    {
        WorldPacket data(SMSG_FORCE_SWIM_SPEED_CHANGE, 17);
        data << p_target->GetNewGUID();
        data << (uint32_t)2;
        data << m_target->getSpeedRate(TYPE_SWIM, true);
        p_target->getSession()->SendPacket(&data);
    }
}

void Aura::SpellAuraModCratureDmgDone(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            for (uint8_t x = 0; x < 11; x++)
                if (aurEff->getEffectMiscValue() & ((uint32_t)1 << x))
                    p_target->m_increaseDamageByType[x + 1] += aurEff->getEffectDamage();

            aurEff->getEffectDamage() < 0 ? mPositive = false : mPositive = true;
        }
        else
            for (uint8_t x = 0; x < 11; x++)
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    p_target->m_increaseDamageByType[x + 1] -= aurEff->getEffectDamage();
    }
}

void Aura::SpellAuraPacifySilence(AuraEffectModifier* /*aurEff*/, bool apply)
{
    // Can't Attack or Cast Spells
    if (apply)
    {
        if (m_spellInfo->getId() == 24937)
            mPositive = true;
        else
            mPositive = false;

        m_target->m_pacified++;
        m_target->m_silenced++;
        m_target->addUnitFlags(UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);

        if (m_target->isCastingSpell())
        {
            m_target->interruptSpell();
        }
    }
    else
    {
        m_target->m_pacified--;

        if (m_target->m_pacified == 0)
        {
            m_target->removeUnitFlags(UNIT_FLAG_PACIFIED);
        }

        m_target->m_silenced--;

        if (m_target->m_silenced == 0)
        {
            m_target->removeUnitFlags(UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraModScale(AuraEffectModifier* aurEff, bool apply)
{
    float current = m_target->getScale();
    float delta = aurEff->getEffectDamage() / 100.0f;

    m_target->setScale(apply ? (current + current * delta) : current / (1.0f + delta));
}

void Aura::SpellAuraModCastingSpeed(AuraEffectModifier* aurEff, bool apply)
{
    float current = m_target->getModCastSpeed();
    if (apply)
        current -= aurEff->getEffectDamage() / 100.0f;
    else
        current += aurEff->getEffectDamage() / 100.0f;

    m_target->setModCastSpeed(current);
}

bool isFeignDeathResisted(uint32_t playerlevel, uint32_t moblevel)
{
    int fMobRes = 0;
    int diff = 0;

    if (playerlevel < moblevel)
    {
        diff = moblevel - playerlevel;

        if (diff <= 2)
            fMobRes = diff + 4;
        else
            fMobRes = (diff - 2) * 11 + 6;

        if (fMobRes > 100)
            fMobRes = 100;

        if (Util::getRandomUInt(1, 100) < static_cast<uint32_t>(fMobRes))
            return true;
    }

    return false;
}

void Aura::SpellAuraFeignDeath(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->eventAttackStop();
            p_target->setDeathState(ALIVE);

#if VERSION_STRING != Classic
            p_target->addUnitFlags2(UNIT_FLAG2_FEIGN_DEATH);
#endif
            p_target->addUnitFlags(UNIT_FLAG_FEIGN_DEATH);
            p_target->addDynamicFlags(U_DYN_FLAG_DEAD);

            //now get rid of mobs agro. pTarget->m_combatStatusHandler.AttackersForgetHate() - this works only for already attacking mobs
            for (const auto& itr : p_target->getInRangeObjectsSet())
            {
                if (itr && itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->isAlive())
                {
                    Unit* u = static_cast<Unit*>(itr);
                    if (isFeignDeathResisted(p_target->getLevel(), u->getLevel()))
                    {
                        removeAura();
                        return;
                    }
                    if (u->isCreature())
                        u->getThreatManager().clearThreat(p_target);

                    //if this is player and targeting us then we interrupt cast
                    if (u->isPlayer())
                    {
                        Player* plr = static_cast<Player*>(itr);
                        if (plr->isCastingSpell())
                            plr->interruptSpell(); // cancel current casting spell
                    }
                }
            }

            // this looks awkward!
            p_target->sendMirrorTimer(MIRROR_TYPE_FIRE, getTimeLeft(), getTimeLeft(), 0xFFFFFFFF);

            p_target->removeUnitFlags(UNIT_FLAG_COMBAT);

            /*if (p_target->hasUnitStateFlag(UNIT_STATE_ATTACKING))
                p_target->removeUnitStateFlag(UNIT_STATE_ATTACKING);*/

            p_target->sendPacket(SmsgCancelCombat().serialise().get());

            // Send server-side cancel message
            WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 8);
            data << p_target->GetNewGUID();
            p_target->sendMessageToSet(&data, false);
        }
        else
        {
#if VERSION_STRING != Classic
            p_target->removeUnitFlags2(UNIT_FLAG2_FEIGN_DEATH);
#endif
            p_target->removeUnitFlags(UNIT_FLAG_FEIGN_DEATH);
            p_target->removeDynamicFlags(U_DYN_FLAG_DEAD);
            p_target->sendStopMirrorTimerPacket(MIRROR_TYPE_FIRE);
        }
    }
}

void Aura::SpellAuraModDisarm(AuraEffectModifier* aurEff, bool apply)
{
    enum AuraModUnitFlag
    {
        UnitFlag,
        UnitFlag2
    };

    uint32_t flag;
    uint16_t field;

    switch (aurEff->getAuraEffectType())
    {
        case SPELL_AURA_MOD_DISARM:
            field = UnitFlag;
            flag = UNIT_FLAG_DISARMED;
            break;
#if VERSION_STRING > TBC
        // TODO: confirm if this actually exists in tbc -Appled
        case SPELL_AURA_MOD_DISARM_OFFHAND:
            field = UnitFlag2;
            flag = UNIT_FLAG2_DISARM_OFFHAND;
            break;
        case SPELL_AURA_MOD_DISARM_RANGED:
            field = UnitFlag2;
            flag = UNIT_FLAG2_DISARM_RANGED;
            break;
#endif
        default:
            return;
    }

    if (apply)
    {
        if (p_target != nullptr && p_target->isInFeralForm())
            return;

        mPositive = false;

        m_target->m_isDisarmed = true;

        if (field == UnitFlag)
            m_target->addUnitFlags(flag);
#if VERSION_STRING > Classic
        else
            m_target->addUnitFlags2(flag);
#endif
    }
    else
    {
        m_target->m_isDisarmed = false;

        if (field == UnitFlag)
            m_target->removeUnitFlags(flag);
#if VERSION_STRING > Classic
        else
            m_target->removeUnitFlags2(flag);
#endif
    }
}

void Aura::SpellAuraModStalked(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        m_target->m_stalkedByGuid = m_casterGuid;
        mPositive = false;
    }
    else
    {
        m_target->m_stalkedByGuid = 0;
    }
}

void Aura::SpellAuraModSpellCritChanceSchool(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->m_spellCritChanceSchool[x] += aurEff->getEffectDamage();
        if (aurEff->getEffectDamage() < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
    {
        for (uint32_t x = 0; x < 7; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                m_target->m_spellCritChanceSchool[x] -= aurEff->getEffectDamage();
                /*if (m_target->m_spellCritChanceSchool[x] < 0)
                    m_target->m_spellCritChanceSchool[x] = 0;*/
            }
        }
    }
    if (p_target != nullptr)
        p_target->updateChanceFields();
}

void Aura::SpellAuraModPowerCost(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    if (apply)
    {
        if (val > 0)
            mPositive = false;
        else
            mPositive = true;
    }
    for (uint16_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            m_target->modPowerCostMultiplier(x, val / 100.0f);
        }
    }
}

void Aura::SpellAuraModPowerCostSchool(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        for (uint16_t x = 1; x < 7; x++)
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->modPowerCostModifier(x, aurEff->getEffectDamage());
    }
    else
    {
        for (uint16_t x = 1; x < 7; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                m_target->modPowerCostModifier(x, -aurEff->getEffectDamage());
            }
        }
    }
}

void Aura::SpellAuraReflectSpellsSchool(AuraEffectModifier* aurEff, bool apply)
{
    m_target->removeReflect(getSpellId(), apply);

    if (apply)
    {
        int32_t school = 0;
        if (m_spellInfo->getAttributes() == 0x400D0 && m_spellInfo->getAttributesEx() == 0)
            school = (int)(log10((float)aurEff->getEffectMiscValue()) / log10((float)2));
        else
            school = m_spellInfo->getFirstSchoolFromSchoolMask();

        m_target->m_reflectSpellSchool.emplace_back(std::make_unique<ReflectSpellSchool>(getSpellId(), 0, school, aurEff->getEffectDamage(), false));
    }
}

void Aura::SpellAuraModLanguage(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_modlanguage = aurEff->getEffectMiscValue();
    else
        m_target->m_modlanguage = -1;
}

void Aura::SpellAuraAddFarSight(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        if (!m_target->isPlayer())
            return;

        //FIXME:grep aka Nublex will fix this
        //Make update circle bigger here
    }
    else
    {
        //Destroy new updated objects here if they are still out of update range
        //w/e
    }
}

void Aura::SpellAuraMechanicImmunity(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectMiscValue() < TOTAL_SPELL_MECHANICS)
        {
        m_target->m_mechanicsDispels[aurEff->getEffectMiscValue()]++;

        if (aurEff->getEffectMiscValue() != 16 && aurEff->getEffectMiscValue() != 25 && aurEff->getEffectMiscValue() != 19) // don't remove bandages, Power Word and protection effect
        {
            /* Supa's test run of Unit::removeAllAurasBySpellMechanic */
            m_target->removeAllAurasBySpellMechanic(static_cast<SpellMechanic>(aurEff->getEffectMiscValue()), false);

            //Insignia/Medallion of A/H //Every Man for Himself
            if (m_spellInfo->getId() == 42292 || m_spellInfo->getId() == 59752)
            {
                m_target->removeAllAurasBySpellMechanic(sSpellMgr.getCrowdControlMechanicList(true));
            }
        }
        }
        else
            mPositive = false;

        // Demonic Circle hack
        if (m_spellInfo->getId() == 48020 && m_target->isPlayer() && m_target->hasAurasWithId(62388))
        {
            GameObject* obj = m_target->getWorldMap()->getGameObject(m_target->m_objectSlots[0]);

            if (obj != nullptr)
            {
                Player* ptarget = static_cast< Player* >(m_target);

                ptarget->safeTeleport(obj->GetMapId(), obj->GetInstanceID(), LocationVector(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), m_target->GetOrientation()));
            }
        }
    }
    else
        m_target->m_mechanicsDispels[aurEff->getEffectMiscValue()]--;
}

void Aura::SpellAuraMounted(AuraEffectModifier* aurEff, bool apply)
{
    if (!p_target) return;

    /*Shady: Is it necessary? Stealth should be broken since all spells with Mounted SpellEffect don't have "does not break stealth" flag (except internal Video mount spell).
    So commented, cause we don't need useless checks and hackfixes*/
    /* if (m_target->IsStealth())
    {
    uint32_t id = m_target->m_stealth;
    m_target->m_stealth = 0;
    m_target->removeAllAurasById(id);
    }*/

    if (apply)
    {
        uint32_t creatureEntry = aurEff->getEffectMiscValue();
        uint32_t displayId = 0;
        uint32_t vehicleId = 0;

        mPositive = true;

        if (p_target->getBattleground())
            p_target->getBattleground()->HookOnMount(p_target);

        p_target->dismount();

        if (p_target->getShapeShiftForm() && !(p_target->getShapeShiftForm() & (FORM_BATTLESTANCE | FORM_DEFENSIVESTANCE | FORM_BERSERKERSTANCE)))
            p_target->removeAllAurasByAuraEffect(SPELL_AURA_MOD_SHAPESHIFT);

        // Festive Holiday Mount
        if (p_target->hasAurasWithId(62061))
        {
#if VERSION_STRING >= TBC
            if (getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_ENABLE_FLIGHT2))
                creatureEntry = 24906;
            else
#endif
                creatureEntry = 15665;
        }

        if (CreatureProperties const* creatureInfo = sMySQLStore.getCreatureProperties(creatureEntry))
        {
            displayId = creatureInfo->Male_DisplayID;

            vehicleId = creatureInfo->vehicleid;

            //some spell has one aura of mount and one of vehicle
            for (uint32_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_SUMMON
                    && getSpellInfo()->getEffectMiscValue(i) == aurEff->getEffectMiscValue())
                    displayId = 0;
        }
        
        p_target->setMountSpellId(m_spellInfo->getId());
        p_target->m_flyingAura = 0;

        p_target->mount(displayId, vehicleId, creatureEntry);

#if VERSION_STRING > WotLK
        uint32_t amount = 0;
        if (WDB::Structures::MountCapabilityEntry const* mountCapability = m_target->getMountCapability(uint32_t(getSpellInfo()->getEffectMiscValueB(0))))
            amount = mountCapability->id;

        // cast speed aura
        if (WDB::Structures::MountCapabilityEntry const* mountCapability = sMountCapabilityStore.lookupEntry(amount))
            p_target->castSpell(p_target, mountCapability->speedModSpell, true);
#endif
    }
    else
    {
        p_target->setMountVehicleId(0);
        p_target->setMountSpellId(0);
        p_target->m_flyingAura = 0;

        p_target->dismount();

#if VERSION_STRING > WotLK
        uint32_t amount = 0;
        if (WDB::Structures::MountCapabilityEntry const* mountCapability = m_target->getMountCapability(uint32_t(getSpellInfo()->getEffectMiscValueB(0))))
            amount = mountCapability->id;

        // remove speed aura
        if (WDB::Structures::MountCapabilityEntry const* mountCapability = sMountCapabilityStore.lookupEntry(amount))
            p_target->removeAllAurasById(mountCapability->speedModSpell);
#endif
    }
}

void Aura::SpellAuraModDamagePercDone(AuraEffectModifier* aurEff, bool apply)
{
    float val = (apply) ? aurEff->getEffectDamage() / 100.0f : -aurEff->getEffectDamage() / 100.0f;

    switch (getSpellId())  //dirty or mb not fix bug with wand specializations
    {
        case 14524:
        case 14525:
        case 14526:
        case 14527:
        case 14528:
            return;
    }
    if (p_target != nullptr)
    {
        if (getSpellInfo()->getEquippedItemClass() == -1)  //does not depend on weapon
        {
            for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
            {
                if (aurEff->getEffectMiscValue() & ((uint32_t)1 << x))
                {
                    // display to client (things that are weapon dependant don't get displayed)
                    p_target->setModDamageDonePct(p_target->getModDamageDonePct(x) + val, x);
                }
            }
        }
        if (aurEff->getEffectMiscValue() & 1)
        {
            if (apply)
            {
                WeaponModifier md;
                md.value = val;
                md.wclass = getSpellInfo()->getEquippedItemClass();
                md.subclass = getSpellInfo()->getEquippedItemSubClass();
                p_target->m_damageDone.insert(std::make_pair(getSpellId(), md));
            }
            else
            {
                std::map< uint32_t, WeaponModifier >::iterator i = p_target->m_damageDone.begin();

                for (; i != p_target->m_damageDone.end(); ++i)
                {
                    if ((*i).first == getSpellId())
                    {
                        p_target->m_damageDone.erase(i);
                        break;
                    }
                }
                p_target->m_damageDone.erase(getSpellId());
            }
        }
    }
    else
    {
        for (uint8_t x = 0; x < 7; x++)
        {
            if (aurEff->getEffectMiscValue() & ((uint32_t)1 << x))
            {
                static_cast< Creature* >(m_target)->ModDamageDonePct[x] += val;
            }
        }
    }
    m_target->calculateDamage();
}

void Aura::SpellAuraModPercStat(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING >= TBC // support classic
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
        val = -aurEff->getEffectDamage();

    if (aurEff->getEffectMiscValue() == -1) //all stats
    {
        if (p_target != nullptr)
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                if (aurEff->getEffectDamage() > 0)
                    p_target->m_statModPctPos[x] += val;
                else
                    p_target->m_statModPctNeg[x] -= val;

                p_target->calcStat(x);
            }

            p_target->updateStats();
            p_target->updateChances();
        }
        else
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                static_cast< Creature* >(m_target)->StatModPct[x] += val;
                static_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else
    {
        if (aurEff->getEffectMiscValue() < 5)
        {
            uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());
            if (p_target != nullptr)
            {
                if (aurEff->getEffectDamage() > 0)
                    p_target->m_statModPctPos[modValue] += val;
                else
                    p_target->m_statModPctNeg[modValue] -= val;

                p_target->calcStat(modValue);

                p_target->updateStats();
                p_target->updateChances();
            }
            else if (m_target->isCreature())
            {
                static_cast<Creature*>(m_target)->StatModPct[modValue] += val;
                static_cast<Creature*>(m_target)->CalcStat(modValue);
            }
        }
    }
#endif
}

void Aura::SpellAuraSplitDamage(AuraEffectModifier* aurEff, bool apply)
{
    Unit* source = nullptr;         // This is the Unit whose damage we are splitting
    Unit* destination = nullptr;    // This is the Unit that shares the beating
    Object* caster = getCaster();

    // We don't want to split our damage with the owner
    if (
#if VERSION_STRING >= TBC
        (m_spellInfo->getEffect(aurEff->getEffectIndex()) == SPELL_EFFECT_APPLY_OWNER_AREA_AURA) &&
#endif
        (caster != nullptr) &&
        (m_target != nullptr) &&
        caster->isPet() &&
        caster->getGuid() == m_target->getGuid())
        return;

    if (m_areaAura)
    {
        source = getOwner();
        destination = GetUnitCaster();
    }
    else
    {
        source = GetUnitCaster();
        destination = getOwner();
    }

    if (source == nullptr || destination == nullptr)
        return;

    if (source->m_damageSplitTarget != nullptr)
    {
        source->m_damageSplitTarget = nullptr;
    }

    if (apply)
    {
        auto ds = std::make_unique<DamageSplitTarget>();
        ds->m_flatDamageSplit = 0;
        ds->m_spellId = getSpellInfo()->getId();
        ds->m_pctDamageSplit = aurEff->getEffectMiscValue() / 100.0f;
        ds->damage_type = static_cast<uint8_t>(aurEff->getAuraEffectType());
        ds->creator = (void*)this;
        ds->m_target = destination->getGuid();
        source->m_damageSplitTarget = std::move(ds);
    }
    else
    {
        source->m_damageSplitTarget = nullptr;
    }
}

void Aura::EventPeriodicDrink(uint32_t amount)
{
    uint32_t v = m_target->getPower(POWER_TYPE_MANA) + amount;

    if (v > m_target->getMaxPower(POWER_TYPE_MANA))
        v = m_target->getMaxPower(POWER_TYPE_MANA);

    m_target->setPower(POWER_TYPE_MANA, v);
}

void Aura::EventPeriodicHeal1(uint32_t amount)
{
    if (!m_target->isAlive())
        return;

    uint32_t ch = m_target->getHealth();
    ch += amount;
    uint32_t mh = m_target->getMaxHealth();

    if (ch > mh)
        m_target->setHealth(mh);
    else
        m_target->setHealth(ch);

    if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
    {
        m_target->emote(EMOTE_ONESHOT_EAT);
    }
    else
    {
        if (!(m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_ARMOR))
            m_target->sendPeriodicAuraLog(m_casterGuid, m_target->GetNewGUID(), getSpellInfo(), amount, 0, 0, 0, SPELL_AURA_PERIODIC_HEAL_PCT, false);
    }

    m_target->removeAurasByHeal();
}

void Aura::SpellAuraChannelDeathItem(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = false; //this should always be negative as npcs remove negative auras on death

    if (apply)
    {
        //don't need for now
    }
    else
    {
        if (m_target->isCreatureOrPlayer())
        {
            if (m_target->isCreature() && static_cast<Creature*>(m_target)->GetCreatureProperties()->Type == UNIT_TYPE_CRITTER)
                return;

            if (m_target->isDead())
            {
                Player* pCaster = m_target->getWorldMap()->getPlayer((uint32_t)m_casterGuid);
                if (!pCaster)
                    return;
                /*int32_t delta=pCaster->getLevel()-m_target->getLevel();
                if (abs(delta)>5)
                return;*/

                uint32_t itemid = getSpellInfo()->getEffectItemType(aurEff->getEffectIndex());

                //Warlocks only get Soul Shards from enemies that grant XP or Honor
                if (itemid == 6265 && (pCaster->getLevel() > m_target->getLevel()))
                    if ((pCaster->getLevel() - m_target->getLevel()) > 9)
                        return;


                ItemProperties const* proto = sMySQLStore.getItemProperties(itemid);
                if (pCaster->getItemInterface()->CalculateFreeSlots(proto) > 0)
                {
                    auto itemHolder = sObjectMgr.createItem(itemid, pCaster);
                    if (!itemHolder)
                        return;

                    auto* item = itemHolder.get();
                    item->setCreatorGuid(pCaster->getGuid());
                    const auto [addResult, _] = pCaster->getItemInterface()->AddItemToFreeSlot(std::move(itemHolder));
                    if (!addResult)
                    {
                        pCaster->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        return;
                    }
                    SlotResult* lr = pCaster->getItemInterface()->LastSearchResult();

                    pCaster->sendItemPushResultPacket(true, false, true, lr->ContainerSlot, lr->Slot, 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
                }
            }
        }
    }
}

void Aura::SpellAuraModDamagePercTaken(AuraEffectModifier* aurEff, bool apply)
{
    float val;
    if (apply)
    {
        val = aurEff->getEffectDamage() / 100.0f;
        if (val <= 0)
            mPositive = true;
        else
            mPositive = false;
    }
    else
    {
        val = -aurEff->getEffectDamage() / 100.0f;
    }

    switch (m_spellInfo->getId())   // Ardent Defender it only applys on 20% hp :/
    {
        //SPELL_HASH_ARDENT_DEFENDER
        case 31850:
        case 31851:
        case 31852:
        case 66233:
        case 66235:
            m_target->m_damageTakenPctModOnHP35 += val;
            break;
        default:
            break;
    }

    for (uint32_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            m_target->m_damageTakenPctMod[x] += val;
        }
    }
}

void Aura::SpellAuraModResistChance(AuraEffectModifier* aurEff, bool apply)
{
    apply ? m_target->m_resistChance = aurEff->getEffectDamage() : m_target->m_resistChance = 0;
}

void Aura::SpellAuraModDetectRange(AuraEffectModifier* aurEff, bool apply)
{
    Unit* m_caster = GetUnitCaster();
    if (!m_caster)return;
    if (apply)
    {
        mPositive = false;
        m_caster->setDetectRangeMod(m_target->getGuid(), aurEff->getEffectDamage());
    }
    else
    {
        m_caster->unsetDetectRangeMod(m_target->getGuid());
    }
}

void Aura::SpellAuraPreventsFleeing(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    // Curse of Recklessness
}

void Aura::SpellAuraModUnattackable(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    /*
            Also known as Apply Aura: Mod Unintractable
            Used by: Spirit of Redemption, Divine Intervention, Phase Shift, Flask of Petrification
            It uses one of the UNIT_FIELD_FLAGS, either UNIT_FLAG_NOT_SELECTABLE or UNIT_FLAG_NON_ATTACKABLE
            */
}

void Aura::SpellAuraInterruptRegen(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
        m_target->m_interruptRegen++;
    else
    {
        m_target->m_interruptRegen--;
        if (m_target->m_interruptRegen < 0)
            m_target->m_interruptRegen = 0;
    }
}

void Aura::SpellAuraGhost(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            mPositive = false;
            p_target->setMoveWaterWalk();
        }
        else
        {
            p_target->setMoveLandWalk();
        }
    }
}

void Aura::SpellAuraMagnet(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        Unit* caster = GetUnitCaster();
        if (!caster)
            return;
        mPositive = true;
        m_target->m_magnetCasterGuid = caster->getGuid();
    }
    else
    {
        m_target->m_magnetCasterGuid = 0;
    }
}

void Aura::SpellAuraManaShield(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        mPositive = true;
        m_target->m_manashieldAmount = aurEff->getEffectDamage();
        m_target->m_manaShieldId = getSpellId();
    }
    else
    {
        m_target->m_manashieldAmount = 0;
        m_target->m_manaShieldId = 0;
    }
}

void Aura::SpellAuraSkillTalent(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        const auto skillLine = static_cast<uint16_t>(aurEff->getEffectMiscValue());
        const auto amount = static_cast<int16_t>(aurEff->getEffectDamage());

        if (apply)
        {
            if (!p_target->hasSkillLine(skillLine))
            {
                aurEff->setEffectActive(false);
                return;
            }

            mPositive = true;
            p_target->modifySkillBonus(skillLine, amount, true);
        }
        else
            p_target->modifySkillBonus(skillLine, -amount, true);

        p_target->updateStats();
    }
}

void Aura::SpellAuraModAttackPower(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectDamage() < 0)
        mPositive = false;
    else
        mPositive = true;
    m_target->modAttackPowerMods(apply ? aurEff->getEffectDamage() : -aurEff->getEffectDamage());
    m_target->calculateDamage();
}

void Aura::SpellAuraVisible(AuraEffectModifier* /*aurEff*/, bool apply)
{
    //Show positive spells on target
    if (apply)
    {
        mPositive = false;
    }
}

void Aura::SpellAuraModResistancePCT(AuraEffectModifier* aurEff, bool apply)
{
    uint32_t Flag = aurEff->getEffectMiscValue();
    int32_t amt;
    if (apply)
    {
        amt = aurEff->getEffectDamage();
        //   if (amt>0)mPositive = true;
        // else mPositive = false;
    }
    else
        amt = -aurEff->getEffectDamage();

    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
    {
        if (Flag & (((uint32_t)1) << x))
        {
            if (p_target != nullptr)
            {
                if (aurEff->getEffectDamage() > 0)
                {
                    p_target->m_resistanceModPctPos[x] += amt;
                }
                else
                {
                    p_target->m_resistanceModPctNeg[x] -= amt;
                }
                p_target->calcResistance(x);

            }
            else if (m_target->isCreature())
            {
                static_cast< Creature* >(m_target)->ResistanceModPct[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraModCreatureAttackPower(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        for (uint32_t x = 0; x < 11; x++)
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->m_creatureAttackPowerMod[x + 1] += aurEff->getEffectDamage();

        if (aurEff->getEffectDamage() > 0)
            mPositive = true;
        else
            mPositive = false;
    }
    else
    {
        for (uint32_t x = 0; x < 11; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                m_target->m_creatureAttackPowerMod[x + 1] -= aurEff->getEffectDamage();
            }
        }
    }
    m_target->calculateDamage();
}

void Aura::SpellAuraModTotalThreat(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() < 0)
            mPositive = true;
        else
            mPositive = false;

        m_target->modThreatModifyer(aurEff->getEffectDamage());
    }
    else
        m_target->modThreatModifyer(-(aurEff->getEffectDamage()));
}

void Aura::SpellAuraWaterWalk(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            mPositive = true;
            p_target->setMoveWaterWalk();
        }
        else
        {
            p_target->setMoveLandWalk();
        }
    }
}

void Aura::SpellAuraFeatherFall(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        mPositive = true;
        p_target->setMoveFeatherFall();
        p_target->m_noFallDamage = true;
    }
    else
    {
        p_target->setMoveNormalFall();
        p_target->m_noFallDamage = false;
    }
}

void Aura::SpellAuraHover(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > TBC
    mPositive = true;

    if (apply)
    {
        m_target->setMoveHover(true);
        m_target->setHoverHeight(float(aurEff->getEffectDamage()) / 2);
    }
    else
    {
        m_target->setMoveHover(false);
        m_target->setHoverHeight(0.0f);
    }
#endif
}

void Aura::SendDummyModifierLog(std::map< SpellInfo*, uint32_t >* m, SpellInfo* spellInfo, uint32_t i, bool apply, bool pct)
{
    int32_t v = spellInfo->getEffectBasePoints(static_cast<uint8_t>(i)) + 1;
    auto* mask = spellInfo->getEffectSpellClassMask(static_cast<uint8_t>(i));
    uint8_t type = static_cast<uint8_t>(spellInfo->getEffectMiscValue(static_cast<uint8_t>(i)));

    if (apply)
    {
        m->insert(std::make_pair(spellInfo, i));
    }
    else
    {
        v = -v;
        std::map<SpellInfo*, uint32_t>::iterator itr = m->find(spellInfo);
        if (itr != m->end())
            m->erase(itr);
    }

#if VERSION_STRING >= Cata
    std::vector<std::pair<uint8_t, float>> modValues;
#endif
    uint32_t intbit = 0, groupnum = 0;
    for (uint8_t bit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
    {
        if (intbit == 32)
        {
            ++groupnum;
            intbit = 0;
        }
        if ((1 << intbit) & mask[groupnum])
        {
            if (p_target == nullptr)
                continue;

#if VERSION_STRING < Cata
            p_target->sendSpellModifierPacket(bit, type, v, pct);
#else
            modValues.push_back(std::make_pair(bit, static_cast<float>(v)));
#endif
        }
    }

#if VERSION_STRING >= Cata
    if (p_target != nullptr)
        p_target->sendSpellModifierPacket(type, modValues, pct);

    modValues.clear();
#endif
}

void Aura::SpellAuraAddClassTargetTrigger(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        uint32_t groupRelation[3], procClassMask[3];

        // Find spell of effect to be triggered
        SpellInfo const* sp = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex()));
        if (sp == nullptr)
        {
            sLogger.debug("Warning! class trigger spell is null for spell {}", getSpellInfo()->getId());
            return;
        }

        // Initialize proc class mask
        procClassMask[0] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 0);
        procClassMask[1] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 1);
        procClassMask[2] = getSpellInfo()->getEffectSpellClassMask(aurEff->getEffectIndex(), 2);

        // Initialize mask
        groupRelation[0] = sp->getEffectSpellClassMask(aurEff->getEffectIndex(), 0);
        groupRelation[1] = sp->getEffectSpellClassMask(aurEff->getEffectIndex(), 1);
        groupRelation[2] = sp->getEffectSpellClassMask(aurEff->getEffectIndex(), 2);

        m_target->addProcTriggerSpell(sp->getId(), getSpellInfo()->getId(), m_casterGuid, getSpellInfo()->getEffectBasePoints(aurEff->getEffectIndex()) + 1, SpellProcFlags(getSpellInfo()->getProcFlags()), EXTRA_PROC_NULL, groupRelation, procClassMask, this);

        sLogger.debug("{} is registering {} chance {} flags {} charges {}", getSpellInfo()->getId(), sp->getId(), getSpellInfo()->getProcChance(), getSpellInfo()->getProcFlags(), getCharges());
    }
    else
    {
        // Find spell of effect to be triggered
        uint32_t spellId = getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex());
        if (spellId == 0)
        {
            sLogger.debug("Warning! trigger spell is null for spell {}", getSpellInfo()->getId());
            return;
        }

        m_target->removeProcTriggerSpell(spellId, m_casterGuid);
    }
}

void Aura::SpellAuraOverrideClassScripts(AuraEffectModifier* aurEff, bool apply)
{
    Player* plr = GetPlayerCaster();
    if (plr == nullptr)
        return;

    //misc value is spell to add
    //spell familyname && grouprelation

    //Adding bonus to effect
    switch (aurEff->getEffectMiscValue())
    {
        //----Shatter---
        // Increases crit chance against rooted targets by (Rank * 10)%.
        case 849:
        case 910:
        case 911:
        case 912:
        case 913:
            if (p_target != nullptr)
            {
                int32_t val = (apply) ? (aurEff->getEffectMiscValue() - 908) * 10 : -(aurEff->getEffectMiscValue() - 908) * 10;
                if (aurEff->getEffectMiscValue() == 849)
                    val = (apply) ? 10 : -10;
                p_target->m_rootedCritChanceBonus += val;
            }
            break;
            // ?
        case 3736:
        case 4415:
        case 4418:
        case 4554:
        case 4555:
        case 4953:
        case 5142:
        case 5147:
        case 5148:
        case 6953:
        {
            if (apply)
            {
                MySQLDataStore::SpellOverrideIdMap::iterator itermap = sMySQLStore._spellOverrideIdStore.find(aurEff->getEffectMiscValue());
                if (itermap == sMySQLStore._spellOverrideIdStore.end())
                {
                    sLogger.failure("Unable to find override with overrideid: {}", aurEff->getEffectMiscValue());
                    break;
                }

                std::list<SpellInfo const*>::iterator itrSE = itermap->second->begin();

                SpellOverrideMap::iterator itr = plr->m_spellOverrideMap.find((*itrSE)->getId());

                if (itr != plr->m_spellOverrideMap.end())
                {
                    ScriptOverrideList::iterator itrSO;
                    for (itrSO = itr->second->begin(); itrSO != itr->second->end(); ++itrSO)
                    {
                        if ((*itrSO)->id == (uint32_t)aurEff->getEffectMiscValue())
                        {
                            if ((int32_t)(*itrSO)->damage > aurEff->getEffectDamage())
                            {
                                (*itrSO)->damage = aurEff->getEffectDamage();
                            }
                            return;
                        }
                    }

                    itr->second->emplace_back(std::make_unique<classScriptOverride>(aurEff->getEffectMiscValue(), 0, 0, aurEff->getEffectDamage(), false));
                }
                else
                {
                    auto lst = std::make_shared<ScriptOverrideList>();
                    lst->emplace_back(std::make_unique<classScriptOverride>(aurEff->getEffectMiscValue(), 0, 0, aurEff->getEffectDamage(), false));

                    for (; itrSE != itermap->second->end(); ++itrSE)
                    {
                        plr->m_spellOverrideMap.emplace((*itrSE)->getId(), lst);
                    }
                }
            }
            else
            {
                MySQLDataStore::SpellOverrideIdMap::iterator itermap = sMySQLStore._spellOverrideIdStore.find(aurEff->getEffectMiscValue());
                SpellOverrideMap::iterator itr = plr->m_spellOverrideMap.begin(), itr2;
                while (itr != plr->m_spellOverrideMap.end())
                {
                    std::list<SpellInfo const*>::iterator itrSE = itermap->second->begin();
                    for (; itrSE != itermap->second->end(); ++itrSE)
                    {
                        if (itr->first == (*itrSE)->getId())
                        {
                            itr2 = itr++;
                            plr->m_spellOverrideMap.erase(itr2);
                            break;
                        }
                    }
                    // Check if the loop above got to the end, if so it means the item wasn't found
                    // and the itr wasn't incremented so increment it now.
                    if (itrSE == itermap->second->end())
                        ++itr;
                }
            }
        }
        break;
        /*      case 19421: //hunter : Improved Hunter's Mark
                case 19422:
                case 19423:
                case 19424:
                case 19425:
                {
                //this should actually add a new functionality to the spell and not override it. There is a lot to decode and to be done here
                }break;*/
        case 4992: // Warlock: Soul Siphon
        case 4993:
        {
            if (apply)
                m_target->m_soulSiphon.m_max += aurEff->getEffectDamage();
            else
                m_target->m_soulSiphon.m_max -= aurEff->getEffectDamage();
        }
        break;
        default:
            sLogger.failure("Unknown override report to devs: {}", aurEff->getEffectMiscValue());
    };
}

void Aura::SpellAuraModRangedDamageTaken(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_rangedDamageTaken += aurEff->getEffectDamage();
    else
    {
        m_target->m_rangedDamageTaken -= aurEff->getEffectDamage();
        if (m_target->m_rangedDamageTaken < 0)
            m_target->m_rangedDamageTaken = 0;
    }
}

void Aura::SpellAuraModHealing(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
        /*if (val>0)
         mPositive = true;
         else
         mPositive = false;*/
    }
    else
        val = -aurEff->getEffectDamage();

    for (uint8_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            m_target->m_healTakenMod[x] += val;
        }
    }
}

void Aura::SpellAuraModMechanicResistance(AuraEffectModifier* aurEff, bool apply)
{
    //silence=26 ?
    //mecanics=9 ?
    if (apply)
    {
        if (aurEff->getEffectMiscValue() < TOTAL_SPELL_MECHANICS)
        {
            m_target->m_mechanicsResistancesPct[aurEff->getEffectMiscValue()] += aurEff->getEffectDamage();

            if (aurEff->getEffectMiscValue() != MECHANIC_HEALING && aurEff->getEffectMiscValue() != MECHANIC_INVULNARABLE && aurEff->getEffectMiscValue() != MECHANIC_SHIELDED)  // don't remove bandages, Power Word and protection effect
            {
                mPositive = true;
            }
            else
            {
                mPositive = false;
            }
        }
    }
    else
        m_target->m_mechanicsResistancesPct[aurEff->getEffectMiscValue()] -= aurEff->getEffectDamage();
}

void Aura::SpellAuraModHealingPCT(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
        val = -aurEff->getEffectDamage();

    for (uint8_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            m_target->m_healTakenPctMod[x] += ((float)(val)) / 100;
        }
    }
}

void Aura::SpellAuraUntrackable(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
        m_target->addStandStateFlags(UNIT_STAND_FLAGS_UNTRACKABLE);
    else
        m_target->removeStandStateFlags(UNIT_STAND_FLAGS_UNTRACKABLE);
}

void Aura::SpellAuraModRangedAttackPower(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)
            mPositive = true;
        else
            mPositive = false;
        m_target->modRangedAttackPowerMods(aurEff->getEffectDamage());
    }
    else
        m_target->modRangedAttackPowerMods(-aurEff->getEffectDamage());
    m_target->calculateDamage();
}

void Aura::SpellAuraModMeleeDamageTaken(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)//does not exist but let it be
            mPositive = false;
        else
            mPositive = true;
        m_target->m_damageTakenMod[0] += aurEff->getEffectDamage();
    }
    else
        m_target->m_damageTakenMod[0] -= aurEff->getEffectDamage();
}

void Aura::SpellAuraModMeleeDamageTakenPct(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() > 0) //does not exist but let it be
            mPositive = false;
        else
            mPositive = true;
        m_target->m_damageTakenPctMod[0] += aurEff->getEffectDamage() / 100.0f;
    }
    else
        m_target->m_damageTakenPctMod[0] -= aurEff->getEffectDamage() / 100.0f;
}

void Aura::SpellAuraRAPAttackerBonus(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_rangeAttackPowerModifier += aurEff->getEffectDamage();
    else
        m_target->m_rangeAttackPowerModifier -= aurEff->getEffectDamage();
}

void Aura::SpellAuraModIncreaseSpeedAlways(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        mPositive = true;
        m_target->m_speedModifier += aurEff->getEffectDamage();
    }
    else
        m_target->m_speedModifier -= aurEff->getEffectDamage();

    m_target->updateSpeed();
}

void Aura::SpellAuraModIncreaseEnergyPerc(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = true;

    auto modValue = static_cast<PowerType>(aurEff->getEffectMiscValue());
    if (apply)
    {
        aurEff->setEffectFixedDamage((m_target->getMaxPower(modValue) * aurEff->getEffectDamage()) / 100);
        m_target->modMaxPower(modValue, aurEff->getEffectFixedDamage());
        if (p_target != nullptr && aurEff->getEffectMiscValue() == POWER_TYPE_MANA)
            p_target->setManaFromSpell(p_target->getManaFromSpell() + aurEff->getEffectFixedDamage());
    }
    else
    {
        m_target->modMaxPower(modValue, -aurEff->getEffectFixedDamage());
        if (p_target != nullptr && aurEff->getEffectMiscValue() == POWER_TYPE_MANA)
            p_target->setManaFromSpell(p_target->getManaFromSpell() - aurEff->getEffectFixedDamage());
    }
}

void Aura::SpellAuraModIncreaseHealthPerc(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = true;
    if (apply)
    {
        aurEff->setEffectFixedDamage((m_target->getMaxHealth() * aurEff->getEffectDamage()) / 100);
        m_target->modMaxHealth(aurEff->getEffectFixedDamage());
        if (p_target != nullptr)
            p_target->setHealthFromSpell(p_target->getHealthFromSpell() + aurEff->getEffectFixedDamage());
        //  else if (m_target->isPet())
        //      TO< Pet* >(m_target)->SetHealthFromSpell(((Pet*)m_target)->GetHealthFromSpell() + aurEff->getEffectFixedDamage());
    }
    else
    {
        m_target->modMaxHealth(-aurEff->getEffectFixedDamage());
        if (m_target->getHealth() > m_target->getMaxHealth())
            m_target->setHealth(m_target->getMaxHealth());
        if (p_target != nullptr)
            p_target->setHealthFromSpell(static_cast<Player*>(m_target)->getHealthFromSpell() - aurEff->getEffectFixedDamage());
        //  else if (m_target->isPet())
        //      TO< Pet* >(m_target)->SetHealthFromSpell(((Pet*)m_target)->GetHealthFromSpell() - aurEff->getEffectFixedDamage());
    }
}

void Aura::SpellAuraModManaRegInterrupt(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
            p_target->m_modInterrManaRegenPct += aurEff->getEffectDamage();
        else
            p_target->m_modInterrManaRegenPct -= aurEff->getEffectDamage();

        p_target->updateStats();
    }
}

void Aura::SpellAuraModTotalStatPerc(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING >= TBC // support classic
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
    }
    else
        val = -aurEff->getEffectDamage();

    if (aurEff->getEffectMiscValue() == -1) //all stats
    {
        if (p_target != nullptr)
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                if (aurEff->getEffectDamage() > 0)
                    p_target->m_totalStatModPctPos[x] += val;
                else
                    p_target->m_totalStatModPctNeg[x] -= val;
                p_target->calcStat(x);
            }

            p_target->updateStats();
            p_target->updateChances();
        }
        else if (m_target->isCreature())
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                static_cast< Creature* >(m_target)->TotalStatModPct[x] += val;
                static_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else
    {
        if (aurEff->getEffectMiscValue() < 5)
        {
            if (p_target != nullptr)
            {
                //druid hearth of the wild should add more features based on form
                switch (m_spellInfo->getId())
                {
                    //SPELL_HASH_HEART_OF_THE_WILD
                case 17003:
                case 17004:
                case 17005:
                case 17006:
                case 24894:
                {
                    //we should remove effect first
                    p_target->eventTalentHearthOfWildChange(false);
                    //set new value
                    if (apply)
                        p_target->setTalentHearthOfWildPCT(val);
                    else
                        p_target->setTalentHearthOfWildPCT(0);   //this happens on a talent reset
                                                                 //reapply
                    p_target->eventTalentHearthOfWildChange(true);
                } break;
                }

                uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());

                if (aurEff->getEffectDamage() > 0)
                    p_target->m_totalStatModPctPos[modValue] += val;
                else
                    p_target->m_totalStatModPctNeg[modValue] -= val;

                p_target->calcStat(modValue);
                p_target->updateStats();
                p_target->updateChances();
            }
            else if (m_target->isCreature())
            {
                uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());

                static_cast<Creature*>(m_target)->TotalStatModPct[modValue] += val;
                static_cast<Creature*>(m_target)->CalcStat(modValue);
            }
        }
    }
#endif
}

void Aura::SpellAuraModHaste(AuraEffectModifier* aurEff, bool apply)
{
    //blade flurry - attack a nearby opponent
    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_BLADE_FLURRY
        case 13877:
        case 22482:
        case 33735:
        case 44181:
        case 51211:
        case 65956:
        {
            if (apply)
                m_target->addExtraStrikeTarget(getSpellInfo(), 0);
            else
                m_target->removeExtraStrikeTarget(getSpellInfo());
        } break;
        default:
            break;
    }

    if (aurEff->getEffectDamage() < 0)
        mPositive = false;
    else
        mPositive = true;

    if (apply)
    {
        m_target->modAttackSpeedModifier(MELEE, aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(OFFHAND, aurEff->getEffectDamage());

        aurEff->setEffectFixedDamage(m_target->getBaseAttackTime(MELEE) * aurEff->getEffectDamage() / 100);
        if (m_target->isCreature())
            static_cast<Creature*>(m_target)->m_speedFromHaste += aurEff->getEffectFixedDamage();
    }
    else
    {
        m_target->modAttackSpeedModifier(MELEE, -aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(OFFHAND, -aurEff->getEffectDamage());

        if (m_target->isCreature())
            static_cast<Creature*>(m_target)->m_speedFromHaste -= aurEff->getEffectFixedDamage();
    }

    if (m_target->isPlayer())
        static_cast<Player*>(m_target)->updateAttackSpeed();
}

void Aura::SpellAuraForceReaction(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;

    uint32_t factionId = aurEff->getEffectMiscValue();
    Standing factionRank = Standing(aurEff->getEffectDamage());

    p_target->applyForcedReaction(factionId, factionRank, apply);
    p_target->getSession()->SendPacket(SmsgSetForceReactions(p_target->m_forcedReactions).serialise().get());
}

void Aura::SpellAuraModRangedHaste(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectDamage() < 0)
        mPositive = false;
    else
        mPositive = true;

    if (apply)
        m_target->modAttackSpeedModifier(RANGED, aurEff->getEffectDamage());
    else
        m_target->modAttackSpeedModifier(RANGED, -aurEff->getEffectDamage());

    if (m_target->isPlayer())
        static_cast<Player*>(m_target)->updateAttackSpeed();
}

void Aura::SpellAuraModRangedAmmoHaste(AuraEffectModifier* aurEff, bool apply)
{
    mPositive = true;
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->modAttackSpeedModifier(RANGED, aurEff->getEffectDamage());
    else
        p_target->modAttackSpeedModifier(RANGED, -aurEff->getEffectDamage());

    p_target->updateAttackSpeed();
}

void Aura::SpellAuraModResistanceExclusive(AuraEffectModifier* aurEff, bool apply)
{
    SpellAuraModResistance(aurEff, apply);
}

void Aura::SpellAuraResistPushback(AuraEffectModifier* aurEff, bool apply)
{
    //DK:This is resist for spell casting delay
    //Only use on players for now

    if (p_target != nullptr)
    {
        int32_t val = 0;
        if (apply)
        {
            val = aurEff->getEffectDamage();
            mPositive = true;
        }
        else
            val = -aurEff->getEffectDamage();

        for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                p_target->m_spellDelayResist[x] += val;
            }
        }
    }
}

void Aura::SpellAuraModShieldBlockPCT(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_modBlockAbsorbValue += (uint32_t)aurEff->getEffectDamage();
        }
        else
        {
            p_target->m_modBlockAbsorbValue -= (uint32_t)aurEff->getEffectDamage();
        }
        p_target->updateStats();
    }
}

void Aura::SpellAuraTrackStealthed(AuraEffectModifier* /*aurEff*/, bool apply)
{
    Unit* c = GetUnitCaster();
    if (c == nullptr)
        return;

    c->m_trackStealth = apply;
}

void Aura::SpellAuraModDetectedRange(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
    {
        mPositive = true;
        p_target->m_detectedRange += aurEff->getEffectDamage();
    }
    else
    {
        p_target->m_detectedRange -= aurEff->getEffectDamage();
    }
}

void Aura::SpellAuraSplitDamageFlat(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->m_damageSplitTarget)
    {
        m_target->m_damageSplitTarget = nullptr;
    }

    if (apply)
    {
        auto ds = std::make_unique<DamageSplitTarget>();
        ds->m_flatDamageSplit = aurEff->getEffectMiscValue();
        ds->m_spellId = getSpellInfo()->getId();
        ds->m_pctDamageSplit = 0;
        ds->damage_type = static_cast<uint8_t>(aurEff->getAuraEffectType());
        ds->creator = (void*)this;
        ds->m_target = m_casterGuid;
        m_target->m_damageSplitTarget = std::move(ds);
        //  printf("registering dmg split %u, amount= %u \n",ds->m_spellId, aurEff->getEffectDamage(), aurEff->getEffectMiscValue(), aurEff->getAuraEffect());
    }
}

void Aura::SpellAuraModStealthLevel(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        mPositive = true;
        m_target->modStealthLevel(StealthFlag(aurEff->getEffectMiscValue()), aurEff->getEffectDamage());
    }
    else
        m_target->modStealthLevel(StealthFlag(aurEff->getEffectMiscValue()), -aurEff->getEffectDamage());
}

void Aura::SpellAuraModUnderwaterBreathing(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        uint32_t m_UnderwaterMaxTimeSaved = p_target->m_underwaterMaxTime;
        if (apply)
            p_target->m_underwaterMaxTime *= (1 + aurEff->getEffectDamage() / 100);
        else
            p_target->m_underwaterMaxTime /= (1 + aurEff->getEffectDamage() / 100);
        p_target->m_underwaterTime *= p_target->m_underwaterMaxTime / m_UnderwaterMaxTimeSaved;
    }
}

void Aura::SpellAuraSafeFall(AuraEffectModifier* aurEff, bool apply)
{
    //FIXME:Find true flag
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_safeFall += aurEff->getEffectDamage();
        }
        else
        {
            p_target->m_safeFall -= aurEff->getEffectDamage();
        }
    }
}

void Aura::SpellAuraModReputationAdjust(AuraEffectModifier* aurEff, bool apply)
{
    /*mPositive = true;
    bool updateclient = true;
    if (isPassive())
    updateclient = false; // don't update client on passive

    if (m_target->getObjectTypeId()==TYPEID_PLAYER)
    {
    if (apply)
    TO< Player* >(m_target)->modPercAllReputation(aurEff->getEffectDamage(), updateclient);
    else
    TO< Player* >(m_target)->modPercAllReputation(-aurEff->getEffectDamage(), updateclient);
    }*/

    // This is _actually_ "Reputation gains increased by x%."
    // not increase all rep by x%.

    if (p_target != nullptr)
    {
        mPositive = true;
        if (apply)
            p_target->setPctReputationMod(p_target->getPctReputationMod() + aurEff->getEffectDamage());
        else
            p_target->setPctReputationMod(p_target->getPctReputationMod() - aurEff->getEffectDamage());
    }
}

void Aura::SpellAuraNoPVPCredit(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->incrementHonorless();
    else
        p_target->decrementHonorless();
}

void Aura::SpellAuraModCritDmgPhysical(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_modPhysCritDmgPct += (uint32_t)aurEff->getEffectDamage();
        }
        else
        {
            p_target->m_modPhysCritDmgPct -= (uint32_t)aurEff->getEffectDamage();
        }
    }
}


void Aura::SpellAuraWaterBreathing(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            mPositive = true;
            p_target->sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
            p_target->m_underwaterState = 0;
        }

        p_target->m_isWaterBreathingEnabled = apply;
    }
}

void Aura::SpellAuraAPAttackerBonus(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_attackPowerModifier += aurEff->getEffectDamage();
    else
        m_target->m_attackPowerModifier -= aurEff->getEffectDamage();
}


void Aura::SpellAuraModPAttackPower(AuraEffectModifier* aurEff, bool apply)
{
    //!!probably there is a flag or something that will signal if randeg or melee attack power !!! (still missing)
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->modAttackPowerMultiplier((float)aurEff->getEffectDamage() / 100.0f);
        }
        else
            p_target->modAttackPowerMultiplier(-(float)aurEff->getEffectDamage() / 100.0f);
        p_target->calculateDamage();
    }
}

void Aura::SpellAuraModRangedAttackPowerPct(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer())
    {
        m_target->modRangedAttackPowerMultiplier(((apply) ? 1 : -1) * (float)aurEff->getEffectDamage() / 100);
        m_target->calculateDamage();
    }
}

void Aura::SpellAuraIncreaseDamageTypePCT(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer())
    {
        if (apply)
        {
            for (uint32_t x = 0; x < 11; x++)
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    static_cast< Player* >(m_target)->m_increaseDamageByTypePct[x + 1] += ((float)(aurEff->getEffectDamage())) / 100;
            if (aurEff->getEffectDamage() < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
        {
            for (uint32_t x = 0; x < 11; x++)
            {
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    static_cast< Player* >(m_target)->m_increaseDamageByTypePct[x + 1] -= ((float)(aurEff->getEffectDamage())) / 100;
            }
        }
    }
}

void Aura::SpellAuraIncreaseCricticalTypePCT(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer())
    {
        if (apply)
        {
            for (uint32_t x = 0; x < 11; x++)
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    static_cast< Player* >(m_target)->m_increaseCricticalByTypePct[x + 1] += ((float)(aurEff->getEffectDamage())) / 100;
            if (aurEff->getEffectDamage() < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
        {
            for (uint32_t x = 0; x < 11; x++)
            {
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                    static_cast< Player* >(m_target)->m_increaseCricticalByTypePct[x + 1] -= ((float)(aurEff->getEffectDamage())) / 100;
            }
        }
    }
}

void Aura::SpellAuraIncreasePartySpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer() && m_target->isAlive() && m_target->getMountDisplayId() == 0)
    {
        if (apply)
        {
            m_target->m_speedModifier += aurEff->getEffectDamage();
        }
        else
        {
            m_target->m_speedModifier -= aurEff->getEffectDamage();
        }
        m_target->updateSpeed();
    }
}

void Aura::SpellAuraIncreaseSpellDamageByAttribute(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;

    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;

        aurEff->setEffectFixedDamage(val); //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -aurEff->getEffectFixedDamage();

    uint8_t stat = 3;
    for (uint8_t i = 0; i < 3; i++)
    {
        //bit hacky but it will work with all currently available spells
        if (m_spellInfo->getEffectApplyAuraName(i) == SPELL_AURA_INCREASE_SPELL_HEALING_PCT)
        {
            if (m_spellInfo->getEffectMiscValue(i) < 5)
                stat = static_cast<uint8_t>(m_spellInfo->getEffectMiscValue(i));
            else
                return;
        }
    }

    if (m_target->isPlayer())
    {
        for (uint8_t x = 1; x < TOTAL_SPELL_SCHOOLS; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            {
                if (apply)
                {
                    aurEff->setEffectFixedDamage(Util::float2int32(((float)val / 100) * m_target->getStat(stat)));
                    p_target->modModDamageDonePositive(x, aurEff->getEffectFixedDamage());
                }
                else
                    p_target->modModDamageDonePositive(x, -aurEff->getEffectFixedDamage());
            }
        }
        p_target->updateChanceFields();
    }
}

void Aura::SpellAuraModSpellDamageByAP(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;

    if (apply)
    {
        //!! caster may log out before spell expires on target !
        Unit* pCaster = GetUnitCaster();
        if (pCaster == nullptr)
            return;

        val = aurEff->getEffectDamage() * pCaster->getCalculatedAttackPower() / 100;
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;

        aurEff->setEffectFixedDamage(val); //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -aurEff->getEffectFixedDamage();

    if (m_target->isPlayer())
    {
        for (uint16_t x = 1; x < 7; x++) //melee damage != spell damage.
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                p_target->modModDamageDonePositive(x, val);

        p_target->updateChanceFields();
    }
}

void Aura::SpellAuraIncreaseHealingByAttribute(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > Classic
    int32_t val = aurEff->getEffectDamage();

    if (apply)
    {
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }

    uint8_t stat;
    if (aurEff->getEffectMiscValue() < 5)
        stat = static_cast<uint8_t>(aurEff->getEffectMiscValue());
    else
    {
        sLogger.failure("Aura::SpellAuraIncreaseHealingByAttribute::Unknown spell attribute type {} in spell {}.\n", aurEff->getEffectMiscValue(), getSpellId());
        return;
    }

    if (p_target != nullptr)
    {
        p_target->updateChanceFields();
        if (apply)
        {
            aurEff->setEffectFixedDamage(Util::float2int32(((float)val / 100.0f) * p_target->getStat(stat)));
            p_target->modModHealingDone(aurEff->getEffectFixedDamage());
        }
        else
            p_target->modModHealingDone(-aurEff->getEffectFixedDamage());

        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            if (apply)
                p_target->m_healDoneMod[i] += aurEff->getEffectFixedDamage();
            else
                p_target->m_healDoneMod[i] -= aurEff->getEffectFixedDamage();
        }
    }
#endif
}

void Aura::SpellAuraModHealingByAP(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > Classic
    int32_t val;

    if (apply)
    {
        //!! caster may log out before spell expires on target !
        Unit* pCaster = GetUnitCaster();
        if (pCaster == nullptr)
            return;

        val = aurEff->getEffectDamage() * pCaster->getCalculatedAttackPower() / 100;
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;

        aurEff->setEffectFixedDamage(val); //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -aurEff->getEffectFixedDamage();



    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
    {
        if (aurEff->getEffectMiscValue()  & (((uint32_t)1) << x))
        {
            m_target->m_healDoneMod[x] += val;
        }
    }

    if (p_target != nullptr)
    {
        p_target->modModHealingDone(val);
        p_target->updateChanceFields();
    }
#endif
}

void Aura::SpellAuraModHealingDone(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING > Classic
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
        val = -aurEff->getEffectDamage();

    uint32_t player_class = m_target->getClass();
    if (player_class == DRUID || player_class == PALADIN || player_class == SHAMAN || player_class == PRIEST)
        val = Util::float2int32(val * 1.88f);

    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
    {
        if (aurEff->getEffectMiscValue()  & (((uint32_t)1) << x))
        {
            m_target->m_healDoneMod[x] += val;
        }
    }
    if (p_target != nullptr)
    {
        p_target->updateChanceFields();
        p_target->modModHealingDone(val);
    }
#endif
}

void Aura::SpellAuraModHealingDonePct(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val;
    if (apply)
    {
        val = aurEff->getEffectDamage();
        if (val < 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
        val = -aurEff->getEffectDamage();

    for (uint32_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue()  & (((uint32_t)1) << x))
        {
            m_target->m_healDonePctMod[x] += ((float)(val)) / 100;
        }
    }
}

void Aura::SpellAuraEmphaty(AuraEffectModifier* /*aurEff*/, bool apply)
{
    mPositive = true;
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;

    // Show extra info about beast
    uint32_t dynflags = m_target->getDynamicFlags();
    if (apply)
        dynflags |= U_DYN_FLAG_PLAYER_INFO;

#if VERSION_STRING < Mop
    m_target->BuildFieldUpdatePacket(caster, getOffsetForStructuredField(WoWUnit, dynamic_flags), dynflags);
#else
    m_target->BuildFieldUpdatePacket(caster, getOffsetForStructuredField(WoWObject, dynamic_field), dynflags);
#endif
}

void Aura::SpellAuraModOffhandDamagePCT(AuraEffectModifier* aurEff, bool apply)
{
    //Used only by talents of rogue and warrior;passive,positive
    if (p_target != nullptr)
    {
        if (apply)
        {
            mPositive = true;
            p_target->m_offhandDmgMod *= (100 + aurEff->getEffectDamage()) / 100.0f;
        }
        else
            p_target->m_offhandDmgMod /= (100 + aurEff->getEffectDamage()) / 100.0f;

        p_target->calculateDamage();
    }
}

void Aura::SpellAuraModPenetration(AuraEffectModifier* aurEff, bool apply) // armor penetration & spell penetration
{
    //SPELL_HASH_SERRATED_BLADES
    if (m_spellInfo->getId() == 14171 || m_spellInfo->getId() == 14172 || m_spellInfo->getId() == 14173)
    {
        if (p_target == nullptr)
            return;

        if (apply)
        {
            if (m_spellInfo->getId() == 14171)
                p_target->m_powerCostPctMod[0] += m_target->getLevel() * 2.67f;
            else if (m_spellInfo->getId() == 14172)
                p_target->m_powerCostPctMod[0] += m_target->getLevel() * 5.43f;
            else if (m_spellInfo->getId() == 14173)
                p_target->m_powerCostPctMod[0] += m_target->getLevel() * 8.0f;
        }
        else
        {
            if (m_spellInfo->getId() == 14171)
                p_target->m_powerCostPctMod[0] -= m_target->getLevel() * 2.67f;
            else if (m_spellInfo->getId() == 14172)
                p_target->m_powerCostPctMod[0] -= m_target->getLevel() * 5.43f;
            else if (m_spellInfo->getId() == 14173)
                p_target->m_powerCostPctMod[0] -= m_target->getLevel() * 8.0f;
        }
        return;
    }

    if (apply)
    {
        if (aurEff->getEffectDamage() < 0)
            mPositive = true;
        else
            mPositive = false;

        for (uint8_t x = 0; x < 7; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->m_powerCostPctMod[x] -= aurEff->getEffectDamage();
        }

        if (p_target != nullptr)
        {
#if VERSION_STRING != Classic
            if (aurEff->getEffectMiscValue() & 124)
                p_target->modModTargetResistance(aurEff->getEffectDamage());
            if (aurEff->getEffectMiscValue() & 1)
                p_target->modModTargetPhysicalResistance(aurEff->getEffectDamage());
#endif
        }
    }
    else
    {
        for (uint8_t x = 0; x < 7; x++)
        {
            if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                m_target->m_powerCostPctMod[x] += aurEff->getEffectDamage();
        }
        if (p_target != nullptr)
        {
#if VERSION_STRING != Classic
            if (aurEff->getEffectMiscValue() & 124)
                p_target->modModTargetResistance(-aurEff->getEffectDamage());
            if (aurEff->getEffectMiscValue() & 1)
                p_target->modModTargetPhysicalResistance(-aurEff->getEffectDamage());
#endif
        }
    }
}

void Aura::SpellAuraIncreaseArmorByPctInt(AuraEffectModifier* aurEff, bool apply)
{
    uint32_t i_Int = m_target->getStat(STAT_INTELLECT);

    int32_t amt = Util::float2int32(i_Int * ((float)aurEff->getEffectDamage() / 100.0f));
    amt *= (!apply) ? -1 : 1;

    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
        {
            if (p_target != nullptr)
            {
                p_target->m_flatResistanceModifierPos[x] += amt;
                p_target->calcResistance(x);
            }
            else if (m_target->isCreature())
            {
                static_cast< Creature* >(m_target)->FlatResistanceMod[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraReduceAttackerMHitChance(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
        p_target->m_resistHit[MOD_MELEE] += aurEff->getEffectDamage();
    else
        p_target->m_resistHit[MOD_MELEE] -= aurEff->getEffectDamage();
}

void Aura::SpellAuraReduceAttackerRHitChance(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
        p_target->m_resistHit[MOD_RANGED] += aurEff->getEffectDamage();
    else
        p_target->m_resistHit[MOD_RANGED] -= aurEff->getEffectDamage();
}

void Aura::SpellAuraReduceAttackerSHitChance(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;
    for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
    {
        if (aurEff->getEffectMiscValue() & (1 << i))     // check school
        {
            // signs reversed intentionally
            if (apply)
                p_target->m_resistHitSpell[i] -= aurEff->getEffectDamage();
            else
                p_target->m_resistHitSpell[i] += aurEff->getEffectDamage();
        }
    }
}

void Aura::SpellAuraReduceEnemyMCritChance(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    if (const auto targetPlayer = dynamic_cast<Player*>(m_target))
    {
        //value is negative percent
        if (apply)
            targetPlayer->setResistMCrit(targetPlayer->getResistMCrit() + static_cast<float>(aurEff->getEffectDamage()));
        else
            targetPlayer->setResistMCrit(targetPlayer->getResistMCrit() - static_cast<float>(aurEff->getEffectDamage()));
    }
}

void Aura::SpellAuraReduceEnemyRCritChance(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    if (const auto targetPlayer = dynamic_cast<Player*>(m_target))
    {
        //value is negative percent
        if (apply)
            targetPlayer->setResistRCrit(targetPlayer->getResistRCrit() + static_cast<float>(aurEff->getEffectDamage()));
        else
            targetPlayer->setResistRCrit(targetPlayer->getResistRCrit() - static_cast<float>(aurEff->getEffectDamage()));
    }
}

void Aura::SpellAuraLimitSpeed(AuraEffectModifier* aurEff, bool apply)
{
    int32_t amount = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    m_target->m_maxSpeed += (float)amount;
    m_target->updateSpeed();
}
void Aura::SpellAuraIncreaseTimeBetweenAttacksPCT(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    float pct_value = -val / 100.0f;
    m_target->modModCastSpeed(pct_value);
}

void Aura::SpellAuraMeleeHaste(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectDamage() < 0)
        mPositive = false;
    else
        mPositive = true;

    if (apply)
    {
        m_target->modAttackSpeedModifier(MELEE, aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(OFFHAND, aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(RANGED, aurEff->getEffectDamage());
    }
    else
    {
        m_target->modAttackSpeedModifier(MELEE, -aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(OFFHAND, -aurEff->getEffectDamage());
        m_target->modAttackSpeedModifier(RANGED, -aurEff->getEffectDamage());
    }

    if (m_target->isPlayer())
        static_cast<Player*>(m_target)->updateAttackSpeed();
}

/*
void Aura::SpellAuraIncreaseSpellDamageByInt(AuraEffectModifier* aurEff, bool apply)
{
float val;
if (apply)
{
val = aurEff->getEffectDamage()/100.0f;
if (aurEff->getEffectDamage()>0)
mPositive = true;
else
mPositive = false;
}
else
val =- aurEff->getEffectDamage()/100.0f;

if (m_target->isPlayer())
{
for (uint32_t x=1;x<7;x++)
{
if (aurEff->getEffectMiscValue() & (((uint32_t)1)<<x))
{
TO< Player* >(m_target)->SpellDmgDoneByInt[x]+=val;
}
}
}
}

void Aura::SpellAuraIncreaseHealingByInt(AuraEffectModifier* aurEff, bool apply)
{
float val;
if (apply)
{
val = aurEff->getEffectDamage()/100.0f;
if (val>0)
mPositive = true;
else
mPositive = false;
}
else
val =- aurEff->getEffectDamage()/100.0f;

if (m_target->isPlayer())
{
for (uint32_t x=1;x<7;x++)
{
//  if (aurEff->getEffectMiscValue() & (((uint32_t)1)<<x))
{
TO< Player* >(m_target)->SpellHealDoneByInt[x]+=val;
}
}
}
}
*/
void Aura::SpellAuraModAttackerCritChance(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
    m_target->m_attackerCritChanceMod[0] += val;
}

void Aura::SpellAuraIncreaseAllWeaponSkill(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target->isPlayer())
    {
        const auto amount = static_cast<int16_t>(aurEff->getEffectDamage());
        if (apply)
        {
            mPositive = true;
            // TO< Player* >(m_target)->ModSkillBonusType(SKILL_TYPE_WEAPON, aurEff->getEffectDamage());
            //since the frikkin above line does not work we have to do it manually
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_SWORDS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_AXES, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_BOWS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_GUNS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_MACES, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_SWORDS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_STAVES, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_MACES, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_AXES, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_DAGGERS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_CROSSBOWS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_WANDS, amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_POLEARMS, amount, true);
        }
        else
        {
            //  TO< Player* >(m_target)->ModSkillBonusType(SKILL_TYPE_WEAPON, -aurEff->getEffectDamage());
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_SWORDS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_AXES, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_BOWS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_GUNS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_MACES, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_SWORDS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_STAVES, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_MACES, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_2H_AXES, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_DAGGERS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_CROSSBOWS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_WANDS, -amount, true);
            static_cast< Player* >(m_target)->modifySkillBonus(SKILL_POLEARMS, -amount, true);
        }

        static_cast< Player* >(m_target)->updateStats();
    }
}

void Aura::SpellAuraIncreaseHitRate(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    static_cast< Player* >(m_target)->modifyBonuses(ITEM_MOD_SPELL_HIT_RATING, aurEff->getEffectDamage(), apply);
    static_cast< Player* >(m_target)->updateStats();
}

void Aura::SpellAuraIncreaseRageFromDamageDealtPCT(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    static_cast< Player* >(m_target)->m_rageFromDamageDealt += (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();
}

int32_t Aura::event_GetInstanceID()
{
    return m_target->event_GetInstanceID();
}

void Aura::RelocateEvents()
{
    event_Relocate();
}

void Aura::SpellAuraReduceCritMeleeAttackDmg(AuraEffectModifier* aurEff, bool apply)
{
    signed int val;
    if (apply)
        val = aurEff->getEffectDamage();
    else
        val = -aurEff->getEffectDamage();

    for (uint32_t x = 1; x < 7; x++)
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            m_target->m_critMeleeDamageTakenPctMod[x] += val / 100.0f;
}

void Aura::SpellAuraReduceCritRangedAttackDmg(AuraEffectModifier* aurEff, bool apply)
{
    signed int val;
    if (apply)
        val = aurEff->getEffectDamage();
    else
        val = -aurEff->getEffectDamage();

    for (uint32_t x = 1; x < 7; x++)
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            m_target->m_critRangedDamageTakenPctMod[x] += val / 100.0f;
}

void Aura::SpellAuraEnableFlight(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        m_target->setMoveCanFly(true);
        m_target->m_flyspeedModifier += aurEff->getEffectDamage();
        m_target->updateSpeed();
        if (m_target->isPlayer())
        {
            static_cast< Player* >(m_target)->m_flyingAura = m_spellInfo->getId();
        }
    }
    else
    {
        m_target->setMoveCanFly(false);
        m_target->m_flyspeedModifier -= aurEff->getEffectDamage();
        m_target->updateSpeed();
        if (m_target->isPlayer())
        {
            static_cast< Player* >(m_target)->m_flyingAura = 0;
        }
    }
}

void Aura::SpellAuraEnableFlightWithUnmountedSpeed(AuraEffectModifier* aurEff, bool apply)
{
    // Used in flight form (only so far)
    if (apply)
    {
        m_target->setMoveCanFly(true);
        m_target->m_flyspeedModifier += aurEff->getEffectDamage();
        m_target->updateSpeed();
        if (m_target->isPlayer())
        {
            static_cast< Player* >(m_target)->m_flyingAura = m_spellInfo->getId();
        }
    }
    else
    {
        m_target->setMoveCanFly(false);
        m_target->m_flyspeedModifier -= aurEff->getEffectDamage();
        m_target->updateSpeed();
        if (m_target->isPlayer())
        {
            static_cast< Player* >(m_target)->m_flyingAura = 0;
        }
    }
}

void Aura::SpellAuraIncreaseMovementAndMountedSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_mountedspeedModifier += aurEff->getEffectDamage();
    else
        m_target->m_mountedspeedModifier -= aurEff->getEffectDamage();
    m_target->updateSpeed();
}

void Aura::SpellAuraIncreaseFlightSpeed(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_flyspeedModifier += aurEff->getEffectDamage();
    else
        m_target->m_flyspeedModifier -= aurEff->getEffectDamage();
    m_target->updateSpeed();
}


void Aura::SpellAuraIncreaseRating(AuraEffectModifier* aurEff, bool apply)
{
    int v = apply ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();

    if (!m_target->isPlayer())
        return;

    Player* plr = static_cast< Player* >(m_target);
    for (uint32_t x = 1; x < 24; x++)  //skip x= 0
        if ((((uint32_t)1) << x) & aurEff->getEffectMiscValue())
            plr->modifyBonuses(11 + x, aurEff->getEffectDamage(), apply);

    //MELEE_CRITICAL_AVOIDANCE_RATING + RANGED_CRITICAL_AVOIDANCE_RATING + SPELL_CRITICAL_AVOIDANCE_RATING
    //comes only as combination of them  - ModifyBonuses() not adding them individually anyhow
    if (aurEff->getEffectMiscValue() & (0x0004000 | 0x0008000 | 0x0010000))
        plr->modifyBonuses(ITEM_MOD_RESILIENCE_RATING, aurEff->getEffectDamage(), apply);

    if (aurEff->getEffectMiscValue() & 1)  //weapon skill
    {
        std::map<uint32_t, uint32_t>::iterator i;
        for (uint32_t y = 0; y < 20; y++)
            if (m_spellInfo->getEquippedItemSubClass() & (((uint32_t)1) << y))
            {
                i = static_cast< Player* >(m_target)->m_wratings.find(y);
                if (i == static_cast< Player* >(m_target)->m_wratings.end())    //no prev
                {
                    static_cast< Player* >(m_target)->m_wratings[y] = v;
                }
                else
                {
                    i->second += v;
                    if (i->second == 0)
                        static_cast< Player* >(m_target)->m_wratings.erase(i);
                }
            }
    }

    plr->updateStats();
}

void Aura::SpellAuraRegenManaStatPCT(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    if (apply)
        static_cast<Player*>(m_target)->m_modManaRegenFromStat[aurEff->getEffectMiscValue()] += aurEff->getEffectDamage();
    else
        static_cast<Player*>(m_target)->m_modManaRegenFromStat[aurEff->getEffectMiscValue()] -= aurEff->getEffectDamage();

    static_cast<Player*>(m_target)->updateStats();
}

void Aura::SpellAuraSpellHealingStatPCT(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING >= TBC // support classic
    if (!m_target->isPlayer())
        return;

    if (apply)
    {
        //mPositive = true;
        /*aurEff->getEffectFixedDamage() = (aurEff->getEffectDamage() * m_target->getStat(aurEff->getEffectMiscValue()) /1 00;

        for (uint32_t x = 1; x < 7; x++)
        m_target->m_healDoneMod[x] += aurEff->getEffectFixedDamage();*/

        aurEff->setEffectFixedDamage(((m_target->getStat(STAT_SPIRIT) * aurEff->getEffectDamage()) / 100));

        static_cast<Player*>(m_target)->modifyBonuses(ITEM_MOD_CRITICAL_STRIKE_RATING, aurEff->getEffectFixedDamage(), true);
        static_cast<Player*>(m_target)->updateChances();
    }
    else
    {
        /*for (uint32_t x = 1; x < 7; x++)
            m_target->m_healDoneMod[x] -= aurEff->getEffectFixedDamage();*/

        static_cast<Player*>(m_target)->modifyBonuses(ITEM_MOD_CRITICAL_STRIKE_RATING, aurEff->getEffectFixedDamage(), false);
        static_cast<Player*>(m_target)->updateChances();
    }
#endif
}

void Aura::SpellAuraAllowFlight(AuraEffectModifier* /*aurEff*/, bool apply)
{
    m_target->setMoveCanFly(apply);
}

void Aura::SpellAuraFinishingMovesCannotBeDodged(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        if (!m_target->isPlayer())
            return;

        static_cast< Player* >(m_target)->m_finishingMovesDodge = true;
    }
    else
    {
        if (!m_target->isPlayer())
            return;

        static_cast< Player* >(m_target)->m_finishingMovesDodge = false;
    }
}

void Aura::SpellAuraReduceAOEDamageTaken(AuraEffectModifier* aurEff, bool apply)
{
    float val = aurEff->getEffectDamage() / 100.0f;
    if (apply)
    {
        aurEff->setEffectFixedDamage((int)(m_target->m_AOEDmgMod * val));
        m_target->m_AOEDmgMod += aurEff->getEffectFixedDamage();
    }
    else
    {
        m_target->m_AOEDmgMod -= aurEff->getEffectFixedDamage();
    }
}

void Aura::SpellAuraIncreaseMaxHealth(AuraEffectModifier* aurEff, bool apply)
{
    //should only be used by a player
    //and only ever target players
    if (!m_target->isPlayer())
        return;

    int32_t amount;
    if (apply)
        amount = aurEff->getEffectDamage();
    else
        amount = -aurEff->getEffectDamage();

    static_cast< Player* >(m_target)->setHealthFromSpell(static_cast< Player* >(m_target)->getHealthFromSpell() + amount);
    static_cast< Player* >(m_target)->updateStats();
}

void Aura::SpellAuraSpiritOfRedemption(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (!m_target->isPlayer())
        return;

    if (apply)
    {
        m_target->setScale(0.5);
        m_target->setHealth(1);
        SpellInfo const* sorInfo = sSpellMgr.getSpellInfo(27792);
        Spell* sor = sSpellMgr.newSpell(m_target, sorInfo, true, nullptr);
        SpellCastTargets spellTargets(m_target->getGuid());
        sor->prepare(&spellTargets);
    }
    else
    {
        m_target->setScale(1);
        m_target->removeAllAurasById(27792);
        m_target->setHealth(0);
    }
}

void Aura::SpellAuraIncreaseAttackerSpellCrit(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = aurEff->getEffectDamage();

    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)
            mPositive = false;
        else
            mPositive = true;
    }
    else
        val = -val;

    for (uint32_t x = 0; x < 7; x++)
    {
        if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
            m_target->m_attackerCritChanceMod[x] += val;
    }
}

void Aura::SpellAuraIncreaseRepGainPct(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target)
    {
        mPositive = true;
        if (apply)
            p_target->setPctReputationMod(p_target->getPctReputationMod() + aurEff->getEffectDamage());
        else
            p_target->setPctReputationMod(p_target->getPctReputationMod() - aurEff->getEffectDamage());
    }
}

void Aura::SpellAuraIncreaseRAPbyStatPct(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)
            mPositive = true;
        else
            mPositive = false;

        uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());
        aurEff->setEffectFixedDamage(m_target->getStat(modValue) * aurEff->getEffectDamage() / 100);
        m_target->modRangedAttackPowerMods(aurEff->getEffectFixedDamage());
    }
    else
        m_target->modRangedAttackPowerMods(-aurEff->getEffectFixedDamage());

    m_target->calculateDamage();
}

/* not used
void Aura::SpellAuraModRangedDamageTakenPCT(AuraEffectModifier* aurEff, bool apply)
{
if (apply)
m_target->RangedDamageTakenPct += aurEff->getEffectDamage();
else
m_target->RangedDamageTakenPct -= aurEff->getEffectDamage();
}*/

void Aura::SpellAuraModBlockValue(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target != nullptr)
    {
        int32_t amt;
        if (apply)
        {
            amt = aurEff->getEffectDamage();
            if (amt < 0)
                mPositive = false;
            else
                mPositive = true;
        }
        else
        {
            amt = -aurEff->getEffectDamage();
        }
        p_target->m_modBlockValueFromSpells += amt;
        p_target->updateStats();
    }
}

void Aura::SendChannelUpdate(uint32_t time, Object* m_caster)
{
    m_caster->sendMessageToSet(MsgChannelUpdate(m_caster->GetNewGUID(), time).serialise().get(), true);
}

void Aura::SpellAuraExpertise(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    if (p_target == nullptr)
        return;

    p_target->calcExpertise();
}

void Aura::SpellAuraForceMoveForward(AuraEffectModifier* /*aurEff*/, bool apply)
{
#if VERSION_STRING != Classic
    if (apply)
        m_target->addUnitFlags2(UNIT_FLAG2_FORCE_MOVE);
    else
        m_target->removeUnitFlags2(UNIT_FLAG2_FORCE_MOVE);
#endif
}

void Aura::SpellAuraComprehendLang(AuraEffectModifier* /*aurEff*/, bool apply)
{
#if VERSION_STRING != Classic
    if (apply)
        m_target->addUnitFlags2(UNIT_FLAG2_COMPREHEND_LANG);
    else
        m_target->removeUnitFlags2(UNIT_FLAG2_COMPREHEND_LANG);
#endif
}

void Aura::SpellAuraModPossessPet(AuraEffectModifier* /*aurEff*/, bool apply)
{
    Player* pCaster = GetPlayerCaster();
    if (pCaster == nullptr || !pCaster->IsInWorld())
        return;

    if (!m_target->isPet() || m_target->getPlayerOwner() != pCaster)
        return;

    if (apply)
    {
        pCaster->possess(m_target);
        pCaster->speedCheatDelay(getTimeLeft());
    }
    else
    {
        pCaster->unPossess();
    }
}

void Aura::SpellAuraReduceEffectDuration(AuraEffectModifier* aurEff, bool apply)
{
    if (!m_target->isPlayer())
        return;

    int32_t val;
    if (apply)
    {
        mPositive = true;
        val = aurEff->getEffectDamage(); ///\todo Only maximum effect should be used for Silence or Interrupt effects reduction
    }
    else
    {
        val = -aurEff->getEffectDamage();
    }
    if (aurEff->getEffectMiscValue() > 0 && aurEff->getEffectMiscValue() < 28)
    {
        m_target->m_mechanicDurationPctMod[aurEff->getEffectMiscValue()] += val;
    }
}

// Caster = player
// Target = vehicle
void Aura::HandleAuraControlVehicle(AuraEffectModifier* aurEff, bool apply)
{
#ifdef FT_VEHICLES
    if (!getCaster())
        return;

    if (!getCaster()->isCreatureOrPlayer())
        return;

    if (m_target->isCreature() && !m_target->isVehicle() || !m_target->getVehicleKit())
        return;

    Unit* caster = static_cast<Unit*>(getCaster());
    auto seatId = static_cast<int8_t>(aurEff->getEffectBaseDamage() - 1);

    if (apply)
    {
        caster->enterVehicle(m_target->getVehicleKit(), seatId);
    }
    else
    {
        if (getSpellId() == 53111) // Devour Humanoid
        {
            if (caster->getObjectTypeId() == TYPEID_UNIT)
                caster->ToCreature()->Despawn(0, 0);
        }

        if (seatId == m_target->getVehicleKit()->getSeatForNumberPassenger(caster))
            caster->exitVehicle();
        else if (seatId >= 0)
            m_target->getVehicleKit()->removePassenger(caster);
        else
            caster->exitVehicle();

        // some SPELL_AURA_CONTROL_VEHICLE auras have a dummy effect on the player - remove them
        caster->removeAllAurasById(getSpellId());
    }
#endif
}

void Aura::SpellAuraModCombatResultChance(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        switch (aurEff->getEffectMiscValue())
        {
            case 1:
                //m_target->m_CombatResult_Parry += aurEff->getEffectDamage(); // parry?
                break;
            case 2:
                m_target->m_CombatResult_Dodge += aurEff->getEffectDamage();
                break;
        }
    }
    else
    {
        switch (aurEff->getEffectMiscValue())
        {
            case 1:
                //m_target->m_CombatResult_Parry += -aurEff->getEffectDamage(); // parry?
                break;
            case 2:
                m_target->m_CombatResult_Dodge += -aurEff->getEffectDamage();
                break;
        }
    }
}

void Aura::SpellAuraAddHealth(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        mPositive = true;
        m_target->modMaxHealth(aurEff->getEffectDamage());
        m_target->modHealth(aurEff->getEffectDamage());
    }
    else
    {
        m_target->modMaxHealth(-aurEff->getEffectDamage());
        uint32_t maxHealth = m_target->getMaxHealth();
        if (m_target->getHealth() > maxHealth)
            m_target->setMaxHealth(maxHealth);
    }
}

void Aura::SpellAuraRemoveReagentCost(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target == nullptr)
        return;

#if VERSION_STRING >= TBC
    if (apply)
    {
        p_target->addUnitFlags(UNIT_FLAG_NO_REAGANT_COST);
    }
    else
    {
        p_target->removeUnitFlags(UNIT_FLAG_NO_REAGANT_COST);
    }
#endif
}
void Aura::SpellAuraBlockMultipleDamage(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->m_blockModPct += aurEff->getEffectDamage();
    else
        p_target->m_blockModPct += -aurEff->getEffectDamage();
}

void Aura::SpellAuraModMechanicDmgTakenPct(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
        m_target->m_modDamageTakenByMechPct[aurEff->getEffectMiscValue()] += (float)aurEff->getEffectDamage() / 100;

    else
        m_target->m_modDamageTakenByMechPct[aurEff->getEffectMiscValue()] -= (float)aurEff->getEffectDamage() / 100;
}

void Aura::SpellAuraAllowOnlyAbility(AuraEffectModifier* /*aurEff*/, bool apply)
{
    // cannot perform any abilities, currently only works on players
    if (!p_target)
        return;

#if VERSION_STRING >= WotLK
    // Generic
    if (apply)
    {
        p_target->addPlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST);
    }
    else
    {
        p_target->removePlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST);
    }
#endif
}

void Aura::SpellAuraIncreaseAPbyStatPct(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)
            mPositive = true;
        else
            mPositive = false;

        uint8_t modValue = static_cast<uint8_t>(aurEff->getEffectMiscValue());

        aurEff->setEffectFixedDamage(m_target->getStat(modValue) * aurEff->getEffectDamage() / 100);
        m_target->modAttackPowerMods(aurEff->getEffectFixedDamage());
    }
    else
        m_target->modAttackPowerMods(-aurEff->getEffectFixedDamage());

    m_target->calculateDamage();
}

void Aura::SpellAuraModSpellDamageDOTPct(AuraEffectModifier* aurEff, bool apply)
{
    int32_t val = (apply) ? aurEff->getEffectDamage() : -aurEff->getEffectDamage();

    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_HAUNT
        case 48181:
        case 48184:
        case 48210:
        case 50091:
        case 59161:
        case 59163:
        case 59164:
            m_target->m_DoTPctIncrease[m_spellInfo->getFirstSchoolFromSchoolMask()] += val;
            break;
        default:
        {
            for (uint32_t x = 0; x < 7; x++)
            {
                if (aurEff->getEffectMiscValue() & (((uint32_t)1) << x))
                {
                    m_target->m_DoTPctIncrease[x] += val;
                }
            }
        } break;
    }
}

void Aura::SpellAuraConsumeNoAmmo(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        p_target->m_requiresNoAmmo = true;
    }
    else
    {
        bool other = false;

#if VERSION_STRING >= WotLK
        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_REQUIRES_NO_AMMO
            case 46699:
            {
                // We are unequipping Thori'dal but have an aura with no ammo consumption effect
                if (p_target->hasAuraWithAuraEffect(SPELL_AURA_CONSUMES_NO_AMMO))
                    other = true;
            } break;

            default:
            {
#endif
                // we have Thori'dal too
                if (m_spellInfo->getId() != 46699 && p_target->getAuraWithId(46699))
                    other = true;
#if VERSION_STRING >= WotLK
            }
        }
#endif

#if VERSION_STRING >= WotLK
        // We have more than 1 aura with no ammo consumption effect
        if (p_target->getAuraCountForEffect(SPELL_AURA_CONSUMES_NO_AMMO) >= 2)
            other = true;
#endif

        p_target->m_requiresNoAmmo = other;
    }
}

void Aura::SpellAuraModIgnoreArmorPct(AuraEffectModifier* aurEff, bool apply)
{
    switch (getSpellInfo()->getId())
    {
        case 5530:
        case 12284:
        case 12701:
        case 12702:
        case 12703:
        case 12704:
        case 13709:
        case 13800:
        case 13801:
        case 13802:
        case 13803:
        case 20864:
        case 59224:
        {
            if (apply)
                m_target->m_ignoreArmorPctMaceSpec += (aurEff->getEffectDamage() / 100.0f);
            else
                m_target->m_ignoreArmorPctMaceSpec -= (aurEff->getEffectDamage() / 100.0f);
        } break;
        default:
        {
            if (apply)
                m_target->m_ignoreArmorPct += (aurEff->getEffectDamage() / 100.0f);
            else
                m_target->m_ignoreArmorPct -= (aurEff->getEffectDamage() / 100.0f);
        } break;
    }
}

void Aura::SpellAuraModBaseHealth(AuraEffectModifier* aurEff, bool apply)
{
    if (!p_target)
        return;

    if (apply)
        aurEff->setEffectFixedDamage(p_target->getBaseHealth());

    int32_t amt = aurEff->getEffectFixedDamage() * aurEff->getEffectDamage() / 100;

    if (!apply)
        amt *= -1;

    p_target->setHealthFromSpell(p_target->getHealthFromSpell() + amt);
    p_target->updateStats();
}

void Aura::SpellAuraModAttackPowerOfArmor(AuraEffectModifier* aurEff, bool apply)
{
    /* Need more info about mods, currently it's only for armor
    uint32_t modifier;
    switch(aurEff->getEffectMiscValue()):
    {
    case 1: //Armor
    modifier = UNIT_FIELD_RESISTANCES;
    break;
    }
    */

    if (apply)
    {
        if (aurEff->getEffectDamage() > 0)
            mPositive = true;
        else
            mPositive = false;

        aurEff->setEffectFixedDamage(m_target->getResistance(SCHOOL_NORMAL) / aurEff->getEffectDamage());
        m_target->modAttackPowerMods(aurEff->getEffectFixedDamage());
    }
    else
        m_target->modAttackPowerMods(-aurEff->getEffectFixedDamage());

    m_target->calculateDamage();
}

void Aura::SpellAuraDeflectSpells(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    //Currently used only by Detterence and handled in Spell::DidHit
}

void Aura::SpellAuraPhase(AuraEffectModifier* aurEff, bool apply)
{
#if VERSION_STRING >= TBC
    if (m_target->getAuraCountForId(SPELL_AURA_PHASE) > 1)
    {
        if (m_target->isPlayer())
            static_cast< Player* >(m_target)->getSession()->SystemMessage("You can have only one phase aura!");
        removeAura();
        return;
    }
#endif

    if (apply)
    {
        if (m_target->isPlayer())
            static_cast<Player*>(m_target)->setPhase(PHASE_SET, m_spellInfo->getEffectMiscValue(aurEff->getEffectIndex()));
        else
            m_target->setPhase(PHASE_SET, m_spellInfo->getEffectMiscValue(aurEff->getEffectIndex()));
    }
    else
    {
        if (m_target->isPlayer())
            static_cast<Player*>(m_target)->setPhase(PHASE_RESET);
        else
            m_target->setPhase(PHASE_RESET);
    }
}

void Aura::SpellAuraCallStabledPet(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (apply)
    {
        Player* pcaster = GetPlayerCaster();
        if (pcaster != nullptr && pcaster->getClass() == HUNTER && pcaster->getSession() != nullptr)
            pcaster->getSession()->sendStabledPetList(0);
    }
}

bool Aura::IsCombatStateAffecting()
{
    auto sp = m_spellInfo;

    if (sp->appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_LEECH) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_MANA_LEECH))
        return true;

    return false;
}

bool Aura::IsInrange(float x1, float y1, float z1, Object* o, float square_r)
{
    float t;
    float r;
    t = x1 - o->GetPositionX();
    r = t * t;
    t = y1 - o->GetPositionY();
    r += t * t;
    t = z1 - o->GetPositionZ();
    r += t * t;
    return (r <= square_r);
}

bool Aura::IsAreaAura() const
{
    auto sp = m_spellInfo;

    if (sp->hasEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA) ||
        sp->hasEffect(SPELL_EFFECT_APPLY_RAID_AREA_AURA) ||
        sp->hasEffect(SPELL_EFFECT_APPLY_PET_AREA_AURA) ||
        sp->hasEffect(SPELL_EFFECT_APPLY_FRIEND_AREA_AURA) ||
        sp->hasEffect(SPELL_EFFECT_APPLY_ENEMY_AREA_AURA)
#if VERSION_STRING >= TBC
        || sp->hasEffect(SPELL_EFFECT_APPLY_OWNER_AREA_AURA)
#endif
        )
        return true;

    return false;
}

void Aura::SpellAuraConvertRune(AuraEffectModifier* aurEff, bool apply)
{
    if (p_target == nullptr || !p_target->isClassDeathKnight())
        return;

    DeathKnight* dk = static_cast<DeathKnight*>(p_target);

    if (apply)
    {
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            if (dk->GetRuneType(i) == aurEff->getEffectMiscValue() && !dk->GetRuneIsUsed(i))
            {
                dk->ConvertRune(i, static_cast<uint8_t>(getSpellInfo()->getEffectMiscValueB(aurEff->getEffectIndex())));
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            if (dk->GetRuneType(i) == getSpellInfo()->getEffectMiscValueB(aurEff->getEffectIndex()))
            {
                dk->ConvertRune(i, dk->GetBaseRuneType(i));
            }
        }
    }
}

void Aura::SpellAuraMirrorImage(AuraEffectModifier* aurEff, bool apply)
{
    if (m_target == nullptr || !m_target->isCreature())
        return;

    if (apply && m_target->isSummon() && (m_target->getCreatedByGuid() == getCasterGuid()))
    {
        Summon* s = static_cast< Summon* >(m_target);

        s->setDisplayId(s->getUnitOwner()->getDisplayId());
#if VERSION_STRING != Classic
        s->addUnitFlags2(UNIT_FLAG2_MIRROR_IMAGE);
#endif
    }

    SpellAuraMirrorImage2(aurEff, apply);
}

void Aura::SpellAuraMirrorImage2(AuraEffectModifier* /*aurEff*/, bool apply)
{
    if (m_target == nullptr)
        return;

    if (apply && m_target->isSummon() && (m_target->getCreatedByGuid() == getCasterGuid()))
    {
        if (getCaster()->isPlayer())
        {
            Player* p = static_cast<Player*>(getCaster());

            Item* item;

            item = p->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            if (item != nullptr)
                m_target->setVirtualItemSlotId(0, item->getItemProperties()->ItemId);

            item = p->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            if (item != nullptr)
                m_target->setVirtualItemSlotId(OFFHAND, item->getItemProperties()->ItemId);
        }
#if VERSION_STRING >= WotLK
        else if (getCaster()->isCreatureOrPlayer())
        {
            auto unit = static_cast<Unit*>(getCaster());
            m_target->setVirtualItemSlotId(MELEE, unit->getVirtualItemSlotId(MELEE));
            m_target->setVirtualItemSlotId(OFFHAND, unit->getVirtualItemSlotId(OFFHAND));
            m_target->setVirtualItemSlotId(RANGED, unit->getVirtualItemSlotId(RANGED));
        }
#endif
    }
}
