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

#include "Objects/Units/Creatures/Pet.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Definitions/SpellInFrontStatus.hpp"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "Definitions/SpellDamageType.hpp"
#include "Definitions/CastInterruptFlags.hpp"
#include "Definitions/SpellTargetType.hpp"
#include "Definitions/SpellIsFlags.hpp"
#include "Definitions/SpellState.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Definitions/SpellEffectTarget.hpp"
#include "Definitions/PowerType.hpp"
#include "Definitions/SpellDidHitResult.hpp"
#include "SpellHelpers.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "Objects/Item.hpp"
#include "Objects/DynamicObject.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.hpp"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "SpellMgr.hpp"
#include "SpellAura.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Management/TaxiMgr.hpp"
#include "Server/World.h"
#include "Server/Packets/SmsgSpellFailure.h"
#include "Server/Packets/SmsgSpellFailedOther.h"
#include "Server/Packets/SmsgResurrectRequest.h"
#include "Server/Packets/SmsgSpellDelayed.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

using AscEmu::World::Spell::Helpers::decimalToMask;

 /// externals for spell system
extern pSpellTarget SpellTargetHandler[EFF_TARGET_LIST_LENGTH_MARKER];

extern const char* SpellEffectNames[TOTAL_SPELL_EFFECTS];

enum SpellTargetSpecification
{
    TARGET_SPECT_NONE = 0,
    TARGET_SPEC_INVISIBLE = 1,
    TARGET_SPEC_DEAD = 2,
};

//i might forget conditions here. Feel free to add them
bool Spell::IsStealthSpell()
{
    //check if aura name is some stealth aura
    if (getSpellInfo()->getEffectApplyAuraName(0) == SPELL_AURA_MOD_STEALTH || getSpellInfo()->getEffectApplyAuraName(1) == SPELL_AURA_MOD_STEALTH || getSpellInfo()->getEffectApplyAuraName(2) == SPELL_AURA_MOD_STEALTH)
        return true;
    return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsInvisibilitySpell()
{
    //check if aura name is some invisibility aura
    if (getSpellInfo()->getEffectApplyAuraName(0) == SPELL_AURA_MOD_INVISIBILITY || getSpellInfo()->getEffectApplyAuraName(1) == SPELL_AURA_MOD_INVISIBILITY || getSpellInfo()->getEffectApplyAuraName(2) == SPELL_AURA_MOD_INVISIBILITY)
        return true;
    return false;
}

void Spell::FillSpecifiedTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind, uint32_t specification)
{
    FillSpecifiedTargetsInArea(ind, srcx, srcy, srcz, getEffectRadius(ind), specification);
}

// for the moment we do invisible targets
void Spell::FillSpecifiedTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range, uint32_t /*specification*/)
{
    std::vector<uint64_t>* tmpMap = &m_effectTargets[i];
    //IsStealth()
    float r = range * range;
    SpellDidHitResult did_hit_result;

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        auto obj = itr;
        // don't add objects that are not units and that are dead
        if (!obj || !obj->isCreatureOrPlayer() || !static_cast<Unit*>(obj)->isAlive())
            continue;

        if (getSpellInfo()->getTargetCreatureType())
        {
            if (!obj->isCreature())
                continue;
            CreatureProperties const* inf = static_cast<Creature*>(obj)->GetCreatureProperties();
            if (!(1 << (inf->Type - 1) & getSpellInfo()->getTargetCreatureType()))
                continue;
        }

        if (obj->isInRange(srcx, srcy, srcz, r))
        {
            if (u_caster != nullptr)
            {
                if (u_caster->isValidTarget(itr, getSpellInfo()))
                {
                    did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                    if (did_hit_result != SPELL_DID_HIT_SUCCESS)
                        safeAddMissedTarget(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS);
                    else
                        SafeAddTarget(tmpMap, itr->getGuid());
                }

            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->getUnitOwner())
                {
                    //trap, check not to attack owner and friendly
                    if (g_caster->getUnitOwner()->isValidTarget(itr, getSpellInfo()))
                        SafeAddTarget(tmpMap, itr->getGuid());
                }
                else
                    SafeAddTarget(tmpMap, itr->getGuid());
            }
            if (getSpellInfo()->getMaxTargets())
            {
                if (getSpellInfo()->getMaxTargets() >= tmpMap->size())
                {
                    return;
                }
            }
        }
    }
}
void Spell::FillAllTargetsInArea(LocationVector & location, uint32_t ind)
{
    FillAllTargetsInArea(ind, location.x, location.y, location.z, getEffectRadius(ind));
}

void Spell::FillAllTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind)
{
    FillAllTargetsInArea(ind, srcx, srcy, srcz, getEffectRadius(ind));
}

// We fill all the targets in the area, including the stealth ed one's
void Spell::FillAllTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range)
{
    std::vector<uint64_t>* tmpMap = &m_effectTargets[i];
    float r = range * range;
    SpellDidHitResult did_hit_result;

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (itr)
        {
            auto obj = itr;
            if (!itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())      //|| (TO< Creature* >(*itr)->isTotem() && !TO< Unit* >(*itr)->isPlayer())) why shouldn't we fill totems?
                continue;

            if (p_caster && (itr)->isPlayer() && p_caster->getGroup() && static_cast<Player*>(itr)->getGroup() && static_cast<Player*>(itr)->getGroup() == p_caster->getGroup())      //Don't attack party members!!
            {
                //Dueling - AoE's should still hit the target party member if you're dueling with him
                if (!p_caster->getDuelPlayer() || p_caster->getDuelPlayer() != static_cast<Player*>(itr))
                    continue;
            }
            if (getSpellInfo()->getTargetCreatureType())
            {
                if (!itr->isCreature())
                    continue;
                CreatureProperties const* inf = static_cast<Creature*>(itr)->GetCreatureProperties();
                if (!(1 << (inf->Type - 1) & getSpellInfo()->getTargetCreatureType()))
                    continue;
            }
            if (obj->isInRange(srcx, srcy, srcz, r))
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    bool isInLOS = m_caster->IsWithinLOSInMap(itr);

                    if (m_caster->GetMapId() == itr->GetMapId() && !isInLOS)
                        continue;
                }

                if (u_caster != nullptr)
                {
                    if (u_caster->isValidTarget(itr, getSpellInfo()))
                    {
                        did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            safeAddMissedTarget(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS);
                    }
                }
                else //cast from GO
                {
                    if (g_caster != nullptr && g_caster->getCreatedByGuid() && g_caster->getUnitOwner() != nullptr)
                    {
                        //trap, check not to attack owner and friendly
                        if (g_caster->getUnitOwner()->isValidTarget(itr, getSpellInfo()))
                            SafeAddTarget(tmpMap, itr->getGuid());
                    }
                    else
                        SafeAddTarget(tmpMap, itr->getGuid());
                }
                if (getSpellInfo()->getMaxTargets())
                    if (getSpellInfo()->getMaxTargets() == tmpMap->size())
                    {
                        return;
                    }
            }
        }
    }
}

// We fill all the targets in the area, including the stealthed ones
void Spell::FillAllFriendlyInArea(uint32_t i, float srcx, float srcy, float srcz, float range)
{
    std::vector<uint64_t>* tmpMap = &m_effectTargets[i];
    float r = range * range;
    SpellDidHitResult did_hit_result;

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (itr)
        {
            auto obj = itr;
            if (!(itr->isCreatureOrPlayer()) || !static_cast<Unit*>(itr)->isAlive())
                continue;

            if (getSpellInfo()->getTargetCreatureType())
            {
                if (!itr->isCreature())
                    continue;
                CreatureProperties const* inf = static_cast<Creature*>(itr)->GetCreatureProperties();
                if (!(1 << (inf->Type - 1) & getSpellInfo()->getTargetCreatureType()))
                    continue;
            }

            if (obj->isInRange(srcx, srcy, srcz, r))
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    bool isInLOS = m_caster->IsWithinLOSInMap(itr);

                    if (m_caster->GetMapId() == itr->GetMapId() && !isInLOS)
                        continue;
                }

                if (u_caster != nullptr)
                {
                    if (u_caster->isFriendlyTo(itr))
                    {
                        did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            safeAddMissedTarget(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS);
                    }
                }
                else //cast from GO
                {
                    if (g_caster != nullptr && g_caster->getCreatedByGuid() && g_caster->getUnitOwner() != nullptr)
                    {
                        //trap, check not to attack owner and friendly
                        if (g_caster->getUnitOwner()->isFriendlyTo(itr))
                            SafeAddTarget(tmpMap, itr->getGuid());
                    }
                    else
                        SafeAddTarget(tmpMap, itr->getGuid());
                }
                if (getSpellInfo()->getMaxTargets())
                    if (getSpellInfo()->getMaxTargets() == tmpMap->size())
                        return;
            }
        }
    }
}

uint64_t Spell::GetSinglePossibleEnemy(uint32_t i, float prange)
{
    float r;
    if (prange)
        r = prange;
    else
    {
        r = getSpellInfo()->custom_base_range_or_radius_sqr;
        if (u_caster != nullptr)
        {
            u_caster->applySpellModifiers(SPELLMOD_RADIUS, &r, getSpellInfo(), this);
        }
    }
    float srcx = m_caster->GetPositionX(), srcy = m_caster->GetPositionY(), srcz = m_caster->GetPositionZ();

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        auto obj = itr;
        if (!obj || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        if (getSpellInfo()->getTargetCreatureType())
        {
            if (!itr->isCreature())
                continue;
            CreatureProperties const* inf = static_cast<Creature*>(itr)->GetCreatureProperties();
            if (!(1 << (inf->Type - 1) & getSpellInfo()->getTargetCreatureType()))
                continue;
        }
        if (obj->isInRange(srcx, srcy, srcz, r))
        {
            if (u_caster != nullptr)
            {
                if (u_caster->isValidTarget(itr, getSpellInfo()) && DidHit(i, static_cast<Unit*>(itr)) == SPELL_DID_HIT_SUCCESS)
                {
                    return itr->getGuid();
                }
            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->getUnitOwner())
                {
                    //trap, check not to attack owner and friendly
                    if (g_caster->getUnitOwner()->isValidTarget(itr, getSpellInfo()))
                    {
                        return itr->getGuid();
                    }
                }
            }
        }
    }
    return 0;
}

uint64_t Spell::GetSinglePossibleFriend(uint32_t i, float prange)
{
    float r;
    if (prange)
        r = prange;
    else
    {
        r = getSpellInfo()->custom_base_range_or_radius_sqr;
        if (u_caster != nullptr)
        {
            u_caster->applySpellModifiers(SPELLMOD_RADIUS, &r, getSpellInfo(), this);
        }
    }
    float srcx = m_caster->GetPositionX(), srcy = m_caster->GetPositionY(), srcz = m_caster->GetPositionZ();

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        auto obj = itr;
        if (!obj || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;
        if (getSpellInfo()->getTargetCreatureType())
        {
            if (!itr->isCreature())
                continue;
            CreatureProperties const* inf = static_cast<Creature*>(itr)->GetCreatureProperties();
            if (!(1 << (inf->Type - 1) & getSpellInfo()->getTargetCreatureType()))
                continue;
        }
        if (obj->isInRange(srcx, srcy, srcz, r))
        {
            if (u_caster != nullptr)
            {
                if (u_caster->isFriendlyTo(itr) && DidHit(i, static_cast<Unit*>(itr)) == SPELL_DID_HIT_SUCCESS)
                {
                    return itr->getGuid();
                }
            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->getUnitOwner())
                {
                    //trap, check not to attack owner and friendly
                    if (g_caster->getUnitOwner()->isFriendlyTo(itr))
                    {
                        return itr->getGuid();
                    }
                }
            }
        }
    }
    return 0;
}

uint8_t Spell::DidHit(uint32_t effindex, Unit* target)
{
    //note resistchance is vise versa, is full hit chance
    Unit* u_victim = target;
    if (u_victim == nullptr)
        return SPELL_DID_HIT_MISS;

    Player* p_victim = target->isPlayer() ? static_cast<Player*>(target) : NULL;

    float baseresist[3] = { 4.0f, 5.0f, 6.0f };
    int32_t lvldiff;
    float resistchance;


    /************************************************************************/
    /* Can't resist non-unit                                                */
    /************************************************************************/
    if (u_caster == nullptr)
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Can't reduce your own spells                                         */
    /************************************************************************/
    if (u_caster == u_victim)
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Check if the player target is able to deflect spells                 */
    /* Currently (3.3.5a) there is only spell doing that: Deterrence        */
    /************************************************************************/
#if VERSION_STRING >= WotLK
    if (p_victim && p_victim->hasAuraWithAuraEffect(SPELL_AURA_DEFLECT_SPELLS))
    {
        return SPELL_DID_HIT_DEFLECT;
    }
#endif

    // APGL End
    // MIT Start

    // Check if creature target is in evade mode
    if (target->isCreature() && target->isInEvadeMode())
        return SPELL_DID_HIT_EVADE;

    // Check if unit target is immune to this spell effect
    if (target->getSpellImmunity() != SPELL_IMMUNITY_NONE)
    {
        if (target->hasSpellImmunity(SPELL_IMMUNITY_CHARM))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_CHARMED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_CHARMED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_CONFUSE))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_DISORIENTED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_DISORIENTED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_FEAR))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_FLEEING ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_FLEEING)
                return SPELL_DID_HIT_IMMUNE;

            if (getSpellInfo()->getMechanicsType() == MECHANIC_HORRIFIED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_HORRIFIED)
                return SPELL_DID_HIT_IMMUNE;

            if (getSpellInfo()->getMechanicsType() == MECHANIC_TURNED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_TURNED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_ROOT))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_ROOTED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_ROOTED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_SILENCE))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_SILENCED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_SILENCED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_STUN))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_STUNNED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_STUNNED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_POLYMORPH))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_POLYMORPHED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_POLYMORPHED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_BANISH))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_BANISHED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_BANISHED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_SAP))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_SAPPED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_SAPPED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_FROZEN))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_FROZEN ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_FROZEN)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_SLOW))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_ENSNARED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_SLEEP))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_ASLEEP ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_ASLEEP)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_TAUNT))
        {
            if (getSpellInfo()->getEffect(effindex) == SPELL_EFFECT_ATTACK_ME ||
                getSpellInfo()->getEffectApplyAuraName(effindex) == SPELL_AURA_MOD_TAUNT)
                return SPELL_DID_HIT_IMMUNE;
        }

#if VERSION_STRING >= TBC
        if (target->hasSpellImmunity(SPELL_IMMUNITY_SPELL_HASTE))
        {
            if (getSpellInfo()->getEffectApplyAuraName(effindex) == SPELL_AURA_INCREASE_CASTING_TIME_PCT)
                return SPELL_DID_HIT_IMMUNE;
        }
#endif

        if (target->hasSpellImmunity(SPELL_IMMUNITY_INTERRUPT_CAST))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_INTERRUPTED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_INTERRUPTED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_MOD_HEALING))
        {
            if (getSpellInfo()->getEffectApplyAuraName(effindex) == SPELL_AURA_MOD_HEALING_DONE_PERCENT)
            {
                // Prevent only effects with negative value
                const auto val = getSpellInfo()->calculateEffectValue(effindex);
                if (val < 0)
                    return SPELL_DID_HIT_IMMUNE;
            }
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_TOTAL_STATS))
        {
            if (getSpellInfo()->getEffectApplyAuraName(effindex) == SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE)
            {
                // Prevent only effects with negative value
                const auto val = getSpellInfo()->calculateEffectValue(effindex);
                if (val < 0)
                    return SPELL_DID_HIT_IMMUNE;
            }
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_KNOCKBACK))
        {
            if (getSpellInfo()->getEffect(effindex) == SPELL_EFFECT_KNOCK_BACK
#if VERSION_STRING >= TBC
                || getSpellInfo()->getEffect(effindex) == SPELL_EFFECT_KNOCK_BACK_DEST
#endif
                )
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_DISARM))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_DISARMED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_DISARMED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_INCAPACITATE))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_INCAPACIPATED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_INCAPACIPATED)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_BLEED))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_BLEEDING)
                return SPELL_DID_HIT_IMMUNE;
        }

        if (target->hasSpellImmunity(SPELL_IMMUNITY_SHACKLE))
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_SHACKLED ||
                getSpellInfo()->getEffectMechanic(effindex) == MECHANIC_SHACKLED)
                return SPELL_DID_HIT_IMMUNE;
        }
    }

    // Check if target can reflect this spell
    if (m_canBeReflected)
    {
        auto isReflected = false;
        for (const auto& reflectAura : u_victim->m_reflectSpellSchool)
        {
            if (reflectAura->school != -1 && reflectAura->school != getSpellInfo()->getFirstSchoolFromSchoolMask())
                continue;

            if (Util::checkChance(static_cast<float_t>(reflectAura->chance)))
            {
                //the god blessed special case : mage - Frost Warding = is an augmentation to frost warding
                if (reflectAura->spellId != 0 && !u_victim->hasAurasWithId(reflectAura->spellId))
                    continue;

                if (reflectAura->infront)
                {
                    if (m_caster->isInFront(u_victim))
                        isReflected = true;
                }
                else
                {
                    isReflected = true;
                }

                if (reflectAura->charges > 0)
                {
                    reflectAura->charges--;
                    if (reflectAura->charges <= 0)
                    {
                        // should delete + erase RSS too, if unit hasn't such an aura...
                        if (!u_victim->hasAurasWithId(reflectAura->spellId))
                        {
                            // ...do it manually
                            u_victim->m_reflectSpellSchool.remove(reflectAura);
                        }
                        else
                        {
                            u_victim->removeAllAurasById(reflectAura->spellId);
                        }
                    }
                }

                break;
            }
        }

        if (isReflected)
            return SPELL_DID_HIT_REFLECT;
    }

    // MIT End
    // APGL Start

    /************************************************************************/
    /* Check if the target is immune to this spell school                   */
    /* Unless the spell would actually dispel invulnerabilities             */
    /************************************************************************/
    int dispelMechanic = getSpellInfo()->getEffect(0) == SPELL_EFFECT_DISPEL_MECHANIC && getSpellInfo()->getEffectMiscValue(0) == MECHANIC_INVULNERABLE;
    if (u_victim->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] && !dispelMechanic)
        return SPELL_DID_HIT_IMMUNE;

    /* Check if player target has god mode */
    if (p_victim && p_victim->m_cheats.hasGodModeCheat)
    {
        return SPELL_DID_HIT_IMMUNE;
    }

    /*************************************************************************/
    /* Check if the target is immune to this mechanic                        */
    /*************************************************************************/
    if (getSpellInfo()->getMechanicsType() < TOTAL_SPELL_MECHANICS && u_victim->m_mechanicsDispels[getSpellInfo()->getMechanicsType()])

    {
        // Immune - IF, and ONLY IF, there is no damage component!
        bool no_damage_component = true;
        for (uint8_t x = 0; x <= 2; x++)
        {
            if (getSpellInfo()->getEffect(x) == SPELL_EFFECT_SCHOOL_DAMAGE
                || getSpellInfo()->getEffect(x) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
                || getSpellInfo()->getEffect(x) == SPELL_EFFECT_WEAPON_DAMAGE
                || getSpellInfo()->getEffect(x) == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
                || getSpellInfo()->getEffect(x) == SPELL_EFFECT_DUMMY
                || (getSpellInfo()->getEffect(x) == SPELL_EFFECT_APPLY_AURA &&
                (getSpellInfo()->getEffectApplyAuraName(x) == SPELL_AURA_PERIODIC_DAMAGE
                ))
                )
            {
                no_damage_component = false;
                break;
            }
        }
        if (no_damage_component)
            return SPELL_DID_HIT_IMMUNE; // Moved here from Spell::CanCast
    }

    /************************************************************************/
    /* Check if the target has a % resistance to this mechanic              */
    /************************************************************************/
    if (getSpellInfo()->getMechanicsType() < TOTAL_SPELL_MECHANICS)
    {
        float res = u_victim->m_mechanicsResistancesPct[getSpellInfo()->getMechanicsType()];
        if (Util::checkChance(res))
            return SPELL_DID_HIT_RESIST;
    }

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    uint32_t melee_test_result;
    if (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_MELEE || getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED)
    {
        uint32_t _type;
        if (GetType() == SPELL_DMG_TYPE_RANGED)
            _type = RANGED;
        else
        {
            if (hasAttributeExC(ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON))
                _type = OFFHAND;
            else
                _type = MELEE;
        }

        melee_test_result = u_caster->getSpellDidHitResult(u_victim, _type, this);
        if (melee_test_result != SPELL_DID_HIT_SUCCESS)
            return (uint8_t)melee_test_result;
    }

    /************************************************************************/
    /* Check if the spell is resisted.                                      */
    /************************************************************************/
    if (getSpellInfo()->getFirstSchoolFromSchoolMask() == SCHOOL_NORMAL  && getSpellInfo()->getMechanicsType() == MECHANIC_NONE)
        return SPELL_DID_HIT_SUCCESS;

    bool pvp = (p_caster && p_victim);

    if (pvp)
        lvldiff = p_victim->getLevel() - p_caster->getLevel();
    else
        lvldiff = u_victim->getLevel() - u_caster->getLevel();
    if (lvldiff < 0)
    {
        resistchance = baseresist[0] + lvldiff;
    }
    else
    {
        if (lvldiff < 3)
        {
            resistchance = baseresist[lvldiff];
        }
        else
        {
            if (pvp)
                resistchance = baseresist[2] + (((float)lvldiff - 2.0f) * 7.0f);
            else
                resistchance = baseresist[2] + (((float)lvldiff - 2.0f) * 11.0f);
        }
    }
    ///\todo SB@L - This mechanic resist chance is handled twice, once several lines above, then as part of resistchance here check mechanical resistance i have no idea what is the best pace for this code
    if (getSpellInfo()->getMechanicsType() < TOTAL_SPELL_MECHANICS)
    {
        resistchance += u_victim->m_mechanicsResistancesPct[getSpellInfo()->getMechanicsType()];
    }
    //rating bonus
    if (p_caster != nullptr)
    {
        resistchance -= p_caster->calcRating(CR_HIT_SPELL);
        resistchance -= p_caster->getHitFromSpell();
    }

    // school hit resistance: check all schools and take the minimal
    if (p_victim != nullptr && getSpellInfo()->getSchoolMask() > 0)
    {
        int32_t min = 100;
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
        {
            if (getSpellInfo()->getSchoolMask() & (1 << i) && min > p_victim->m_resistHitSpell[i])
                min = p_victim->m_resistHitSpell[i];
        }
        resistchance += min;
    }

    if (getSpellInfo()->getEffect(static_cast<uint8_t>(effindex)) == SPELL_EFFECT_DISPEL)
    {
        u_caster->applySpellModifiers(SPELLMOD_RESIST_DISPEL, &resistchance, getSpellInfo(), this);
    }

    float hitchance = 0;
    u_caster->applySpellModifiers(SPELLMOD_HITCHANCE, &hitchance, getSpellInfo(), this);
    resistchance -= hitchance;

    if (hasAttribute(ATTRIBUTES_IGNORE_INVULNERABILITY))
        resistchance = 0.0f;

    if (resistchance >= 100.0f)
        return SPELL_DID_HIT_RESIST;
    else
    {
        uint8_t res;
        if (resistchance <= 1.0) //resist chance >=1
            res = (Util::checkChance(1.0f) ? uint8_t(SPELL_DID_HIT_RESIST) : uint8_t(SPELL_DID_HIT_SUCCESS));
        else
            res = (Util::checkChance(resistchance) ? uint8_t(SPELL_DID_HIT_RESIST) : uint8_t(SPELL_DID_HIT_SUCCESS));

        if (res == SPELL_DID_HIT_SUCCESS)  // proc handling. mb should be moved outside this function
        {
            // u_caster->HandleProc(PROC_ON_SPELL_LAND,target,GetProto());
        }

        return res;
    }
}

void Spell::castMeOld()
{
    if (p_caster)
    {
        switch (getSpellInfo()->getId())
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
            {
                p_caster->setAttackTimer(OFFHAND, p_caster->getBaseAttackTime(OFFHAND));
                p_caster->setAttackTimer(MELEE, p_caster->getBaseAttackTime(MELEE));
            } break;

            //SPELL_HASH_VICTORY_RUSH
            case 34428:
            {
                p_caster->removeAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);
            } break;

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
            {
                p_caster->removeAllAurasById(53672);
                p_caster->removeAllAurasById(54149);
            } break;
        }

        if (getSpellInfo()->custom_c_is_flags == SPELL_FLAG_IS_DAMAGING)
        {
            uint32_t arcanePotency[] =
            {
                //SPELL_HASH_ARCANE_POTENCY
                24544,
                31571,
                31572,
                33421,
                33713,
                57529,
                57531,
                0
            };
            if (p_caster->hasAurasWithId(arcanePotency))
            {
                p_caster->removeAllAurasById(57529);
                p_caster->removeAllAurasById(57531);
            }
        }

        if (p_caster->isStealthed() && !hasAttributeEx(ATTRIBUTESEX_NOT_BREAK_STEALTH)
            && getSpellInfo()->getId() != 1)  //check spells that get trigger spell 1 after spell loading
        {
            /* talents procing - don't remove stealth either */
            if (!hasAttribute(ATTRIBUTES_PASSIVE) && !(pSpellId && sSpellMgr.getSpellInfo(pSpellId)->isPassive()))
            {
                p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
            }
        }

        // special case battleground additional actions
        if (p_caster->getBattleground())
        {

            // warsong gulch & eye of the storm flag pickup check
            // also includes check for trying to cast stealth/etc while you have the flag
            switch (getSpellInfo()->getId())
            {
                case 21651:
                {
                    // Arathi Basin opening spell, remove stealth, invisibility, etc.
                    p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
                    p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_INVISIBILITY);

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
                    p_caster->removeAllAurasById(divineShield);

                    uint32_t divineProtection[] =
                    {
                        //SPELL_HASH_DIVINE_PROTECTION
                        498,
                        13007,
                        27778,
                        27779,
                        0
                    };
                    p_caster->removeAllAurasById(divineProtection);
                    //SPELL_HASH_BLESSING_OF_PROTECTION
                    p_caster->removeAllAurasById(41450);
                } break;
                case 23333:
                case 23335:
                case 34976:
                {
                    // if we're picking up the flag remove the buffs
                    p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
                    p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_INVISIBILITY);

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
                    p_caster->removeAllAurasById(divineShield);

                    uint32_t divineProtection[] =
                    {
                        //SPELL_HASH_DIVINE_PROTECTION
                        498,
                        13007,
                        27778,
                        27779,
                        0
                    };
                    p_caster->removeAllAurasById(divineProtection);
                    //SPELL_HASH_BLESSING_OF_PROTECTION
                    p_caster->removeAllAurasById(41450);
                } break;
                // cases for stealth - etc
                // we can cast the spell, but we drop the flag (if we have it)
                case 1784:      // Stealth rank 1
                case 1785:      // Stealth rank 2
                case 1786:      // Stealth rank 3
                case 1787:      // Stealth rank 4
                case 5215:      // Prowl rank 1
                case 6783:      // Prowl rank 2
                case 9913:      // Prowl rank 3
                case 498:       // Divine protection
                case 5573:      // Unknown spell
                case 642:       // Divine shield
                case 1020:      // Unknown spell
                case 1022:      // Hand of Protection rank 1 (ex blessing of protection)
                case 5599:      // Hand of Protection rank 2 (ex blessing of protection)
                case 10278:     // Hand of Protection rank 3 (ex blessing of protection)
                case 1856:      // Vanish rank 1
                case 1857:      // Vanish rank 2
                case 26889:     // Vanish rank 3
                case 45438:     // Ice block
                case 20580:     // Unknown spell
                case 58984:     // Shadowmeld
                case 17624:     // Petrification-> http://www.wowhead.com/?spell=17624
                case 66:        // Invisibility
                    if (p_caster->getBattleground()->getType() == BattlegroundDef::TYPE_WARSONG_GULCH)
                    {
                        if (p_caster->getTeam() == 0)
                            p_caster->removeAllAurasById(23333);    // ally player drop horde flag if they have it
                        else
                            p_caster->removeAllAurasById(23335);    // horde player drop ally flag if they have it
                    }
                    if (p_caster->getBattleground()->getType() == BattlegroundDef::TYPE_EYE_OF_THE_STORM)

                        p_caster->removeAllAurasById(34976);        // drop the flag
                    break;
            }
        }
    }
}

void Spell::AddTime(uint32_t type)
{
    if (u_caster != nullptr)
    {
        if (getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_DAMAGE_TAKEN)
        {
            u_caster->interruptSpell(getSpellInfo()->getId());
            return;
        }

        float ch = 0;
        u_caster->applySpellModifiers(SPELLMOD_NONINTERRUPT, &ch, getSpellInfo(), this);
        if (Util::checkChance(ch))
            return;

        if (p_caster != nullptr)
        {
            if (Util::checkChance(p_caster->m_spellDelayResist[type]))
                return;
        }
        if (m_DelayStep == 2)
            return; //spells can only be delayed twice as of 3.0.2
        if (m_spellState == SPELL_STATE_CASTING)
        {
            // no pushback for some spells
            if ((getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_PUSHBACK) == 0)
                return;
            int32_t delay = 500; //0.5 second pushback
            ++m_DelayStep;
            m_timer += delay;
            if (m_timer > m_castTime)
            {
                delay -= (m_timer - m_castTime);
                m_timer = m_castTime;
                if (delay < 0)
                    delay = 1;
            }

            u_caster->sendMessageToSet(SmsgSpellDelayed(u_caster->GetNewGUID(), delay).serialise().get(), true);

            if (p_caster == nullptr)
            {
                //then it's a Creature
                u_caster->pauseMovement(delay);
            }
            //in case cast is delayed, make sure we do not exit combat
            else
            {
                // sEventMgr.ModifyEventTimeLeft(p_caster,EVENT_ATTACK_TIMEOUT,attackTimeoutInterval,true);
                // also add a new delay to offhand and main hand attacks to avoid cutting the cast short

                // TODO: should spell cast time pushback reset swing timers again? -Appled
                //p_caster->delayMeleeAttackTimer(delay);
            }
        }
        else if (getSpellInfo()->getChannelInterruptFlags() != 48140)
        {
            int32_t delay = getDuration() / 4; //0.5 second push back
            ++m_DelayStep;
            m_timer -= delay;
            if (m_timer < 0)
                m_timer = 0;
            //else if (p_caster != nullptr)
                // TODO: should spell cast time pushback reset swing timers again? -Appled
                //p_caster->delayMeleeAttackTimer(-delay);

            m_Delayed = true;
            if (m_timer > 0)
                sendChannelUpdate(m_timer);

        }
    }
}

//\todo: Not called, should be send after targetting
void Spell::SendLogExecute(uint32_t spellDamage, uint64_t & targetGuid)
{
    WorldPacket data(SMSG_SPELLLOGEXECUTE, 37);
    data << m_caster->GetNewGUID();
    data << getSpellInfo()->getId();
    data << uint32_t(1);
    data << getSpellInfo()->getSpellVisual(0);
    data << uint32_t(1);
    if (m_caster->getGuid() != targetGuid)
        data << targetGuid;
    if (spellDamage)
        data << spellDamage;
    m_caster->sendMessageToSet(&data, true);
}

void Spell::SendInterrupted(uint8_t result)
{
    SetSpellFailed();

    if (m_caster == nullptr || !m_caster->IsInWorld())
        return;

    // send the failure to pet owner if we're a pet
    Player* plr = p_caster;
    if (plr == nullptr && m_caster->isPet())
    {
        static_cast<Pet*>(m_caster)->sendPetCastFailed(getSpellInfo()->getId(), result);
    }
    else
    {
        if (plr == nullptr && u_caster != nullptr && u_caster->m_redirectSpellPackets != nullptr)
            plr = u_caster->m_redirectSpellPackets;

        if (plr != nullptr && plr->isPlayer())
            plr->getSession()->SendPacket(SmsgSpellFailure(m_caster->GetNewGUID(), extra_cast_number, getSpellInfo()->getId(), result).serialise().get());
    }

    m_caster->sendMessageToSet(SmsgSpellFailedOther(m_caster->GetNewGUID(), extra_cast_number, getSpellInfo()->getId(), result).serialise().get(), false);
}

void Spell::SendResurrectRequest(Player* target)
{
    bool resurrectionSickness = false;
    std::string casterName;
    if (!m_caster->isPlayer() && m_caster->isCreature())
    {
        casterName = dynamic_cast<Creature*>(m_caster)->GetCreatureProperties()->Name;

        if (dynamic_cast<Creature*>(m_caster)->isSpiritHealer())
            resurrectionSickness = true;
    }

    bool overrideTimer = false;
    if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_IGNORE_RESURRECTION_TIMER)
        overrideTimer = true;

    target->getSession()->SendPacket(SmsgResurrectRequest(m_caster->getGuid(), casterName, resurrectionSickness, overrideTimer, getSpellInfo()->getId()).serialise().get());
    target->setResurrecterGuid(m_caster->getGuid());
}

void Spell::SendTameFailure(uint8_t result)
{
    if (p_caster != nullptr)
        p_caster->sendPetTameFailure(result);
}

void Spell::HandleAddAura(uint64_t guid)
{
    Unit* Target = nullptr;

    auto itr = m_pendingAuras.find(guid);

    if (itr == m_pendingAuras.end() || itr->second.aur == nullptr)
        return;

    //If this aura isn't added correctly it MUST be deleted
    auto&& aur = std::move(itr->second.aur);

    if (u_caster && u_caster->getGuid() == guid)
        Target = u_caster;
    else if (m_caster->IsInWorld())
        Target = m_caster->getWorldMap()->getUnit(guid);

    if (Target == nullptr)
    {
        return;
    }

    if (getUnitCaster() != nullptr)
    {
        if (getUnitCaster()->isFriendlyTo(Target))
        {
            Target->getCombatHandler().takeCombatAction(getUnitCaster(), true);
        }
        else if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
        {
            // Send initial threat
            if (Target->isCreature())
                Target->getAIInterface()->onHostileAction(getUnitCaster());

            // Target should enter combat when aura is added on target
            Target->getCombatHandler().takeCombatAction(getUnitCaster());

            // Add real threat
            if (Target->getThreatManager().canHaveThreatList())
                Target->getThreatManager().addThreat(getUnitCaster(), 1.f, getSpellInfo(), false, false, this);
        }
    }

    // Applying an aura to a flagged target will cause you to get flagged.
    // self casting doesn't flag himself.
    if (Target->isPlayer() && p_caster && p_caster != static_cast<Player*>(Target))
    {
        if (static_cast<Player*>(Target)->isPvpFlagSet())
        {
            if (p_caster->isPlayer() && !p_caster->isPvpFlagSet())
                p_caster->togglePvP();
            else
                p_caster->setPvpFlag();
        }
    }

    // remove any auras with same type
    if (getSpellInfo()->custom_BGR_one_buff_on_target > 0)
    {
        Target->removeAllAurasBySpellType(static_cast<SpellTypes>(getSpellInfo()->custom_BGR_one_buff_on_target), m_caster->getGuid(), getSpellInfo()->getId());
    }

    uint32_t spellid = 0;

    if ((getSpellInfo()->getMechanicsType() == MECHANIC_INVULNARABLE && getSpellInfo()->getId() != 25771) || getSpellInfo()->getId() == 31884)     // Cast spell Forbearance
    {
        if (getSpellInfo()->getId() != 31884)
            spellid = 25771;

        if (Target->isPlayer())
        {
            sEventMgr.AddEvent(static_cast<Player*>(Target), &Player::avengingWrath, EVENT_PLAYER_AVENGING_WRATH, 30000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            static_cast<Player*>(Target)->m_avengingWrath = false;
        }
    }
    else if (getSpellInfo()->getMechanicsType() == MECHANIC_HEALING && getSpellInfo()->getId() != 11196)  // Cast spell Recently Bandaged
        spellid = 11196;
    else if (getSpellInfo()->getMechanicsType() == MECHANIC_SHIELDED && getSpellInfo()->getId() != 6788)  // Cast spell Weakened Soul
        spellid = 6788;
    else if (getSpellInfo()->getId() == 45438)  // Cast spell Hypothermia
        spellid = 41425;
    else
    {
        switch (getSpellInfo()->getId())
        {
            //SPELL_HASH_HEROISM
            case 23682:
            case 23689:
            case 32182:
            case 32927:
            case 32955:
            case 37471:
            case 39200:
            case 65983:
                spellid = 57723;
                break;
            //SPELL_HASH_BLOODLUST
            case 2825:
            case 6742:
            case 16170:
            case 21049:
            case 23951:
            case 24185:
            case 27689:
            case 28902:
            case 33555:
            case 37067:
            case 37309:
            case 37310:
            case 37472:
            case 37599:
            case 41185:
            case 43578:
            case 45584:
            case 50730:
            case 54516:
            case 65980:
                spellid = 57724;
                break;
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
                uint32_t masterOfSubtlety[] =
                {
                    //SPELL_HASH_MASTER_OF_SUBTLETY
                    31221,
                    31222,
                    31223,
                    31665,
                    31666,
                    0
                };

                if (Target->hasAurasWithId(masterOfSubtlety))
                    spellid = 31665;
            } break;
            case 62124:
            {
                uint32_t vindication[] =
                {
                    //SPELL_HASH_VINDICATION
                    67,
                    9452,
                    26016,
                    26017,
                    36002,
                    0
                };

                if (u_caster)
                {
                    if (const auto* vindicationAur = u_caster->getAuraWithId(vindication))
                    {
                        const uint8_t rank = vindicationAur->getSpellInfo()->hasSpellRanks()
                            ? vindicationAur->getSpellInfo()->getRankInfo()->getRank()
                            : 1;
                        spellid = rank == 2 ? 26017 : 67;
                    }
                }
            } break;
            case 5229:
            {
                uint32_t kingOfTheJungle[] =
                {
                    //SPELL_HASH_KING_OF_THE_JUNGLE
                    48492,
                    48494,
                    48495,
                    51178,
                    51185,
                    0
                };

                if (p_caster && (p_caster->getShapeShiftForm() == FORM_BEAR || p_caster->getShapeShiftForm() == FORM_DIREBEAR) &&
                    p_caster->hasAurasWithId(kingOfTheJungle))
                {
                    const auto spellInfo = sSpellMgr.getSpellInfo(51185);
                    if (!spellInfo)
                    {
                        return;
                    }

                    Spell* spell = sSpellMgr.newSpell(p_caster, spellInfo, true, nullptr);

                    if (const auto* kotjAur = p_caster->getAuraWithId(kingOfTheJungle))
                    {
                        const uint8_t rank = kotjAur->getSpellInfo()->hasSpellRanks()
                            ? kotjAur->getSpellInfo()->getRankInfo()->getRank()
                            : 1;
                        spell->forced_basepoints->set(0, rank * 5);
                    }
                    SpellCastTargets targets(p_caster->getGuid());
                    spell->prepare(&targets);
                }
            } break;
            case 19574:
            {
                if (u_caster != nullptr)
                {
                    uint32_t theBeastWithin[] =
                    {
                        //SPELL_HASH_THE_BEAST_WITHIN
                        34471,
                        34692,
                        38373,
                        50098,
                        70029,
                        0
                    };

                    if (u_caster->hasAurasWithId(theBeastWithin))
                        u_caster->castSpell(u_caster, 34471, true);
                }
            } break;
            // SPELL_HASH_RAPID_KILLING
            case 34948:
            case 34949:
            case 35098:
            case 35099:
            {
                if (u_caster != nullptr)
                {
                    uint32_t rapidRecuperation[] =
                    {
                        //SPELL_HASH_RAPID_RECUPERATION
                        53228,
                        53232,
                        56654,
                        58882,
                        58883,
                        64180,
                        64181,
                        0
                    };

                    if (u_caster->hasAurasWithId(rapidRecuperation))
                        spellid = 56654;
                }
            } break;
        }
    }

    switch (getSpellInfo()->getId())
    {
        // SPELL_HASH_CLEARCASTING:
        case 12536:
        case 16246:
        case 16870:
        case 67210:
        // SPELL_HASH_PRESENCE_OF_MIND:
        case 12043:
        case 29976:
        {
            uint32_t arcanePotency[] =
            {
                //SPELL_HASH_ARCANE_POTENCY
                24544,
                31571,
                31572,
                33421,
                33713,
                57529,
                57531,
                0
            };
            if (const auto* arcPotencyAur = Target->getAuraWithId(arcanePotency))
            {
                const uint8_t rank = arcPotencyAur->getSpellInfo()->hasSpellRanks()
                    ? arcPotencyAur->getSpellInfo()->getRankInfo()->getRank()
                    : 1;
                spellid = rank == 1 ? 57529 : 57531;
            }
        }
        break;
    }

    if (spellid)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(spellid);
        if (!spellInfo)
        {
            return;
        }

        Spell* spell = sSpellMgr.newSpell(u_caster, spellInfo, true, nullptr);

        uint32_t masterOfSubtlety[] =
        {
            //SPELL_HASH_MASTER_OF_SUBTLETY
            31221,
            31222,
            31223,
            31665,
            31666,
            0
        };

        if (spellid == 31665 && Target->hasAurasWithId(masterOfSubtlety))
            spell->forced_basepoints->set(0, Target->getAuraWithId(masterOfSubtlety)->getSpellInfo()->getEffectBasePoints(0));

        SpellCastTargets targets(Target->getGuid());
        spell->prepare(&targets);
    }

    // avoid map corruption (this is impossible, btw)
    if (Target->GetInstanceID() != m_caster->GetInstanceID())
    {
        return;
    }

    Target->addAura(std::move(aur)); // the real spell is added last so the modifier is removed last
}


/*
void Spell::TriggerSpell()
{
if (TriggerSpellId != 0)
{
// check for spell id
SpellEntry *spellInfo = sSpellStore.lookupEntry(TriggerSpellId);

if (!spellInfo)
{
sLogger.failure("WORLD: unknown spell id {}\n", TriggerSpellId);
return;
}

Spell* spell = sSpellMgr.newSpell(m_caster, spellInfo,false, NULL);
WPARCEMU_ASSERT(  spell);

SpellCastTargets targets;
if (TriggerSpellTarget)
targets.getUnitTarget() = TriggerSpellTarget;
else
targets.getUnitTarget() = m_targets.getUnitTarget();

spell->prepare(&targets);
}
}*/

void Spell::DetermineSkillUp()
{
    if (p_caster == nullptr)
        return;

    auto skill_line_ability = sSpellMgr.getFirstSkillEntryForSpell(getSpellInfo()->getId());
    if (skill_line_ability == nullptr)
        return;

    float chance = 0.0f;

    const auto skillLine = static_cast<uint16_t>(skill_line_ability->skilline);
    if (p_caster->hasSkillLine(skillLine))
    {
        uint32_t amt = p_caster->getSkillLineCurrent(skillLine, false);
        uint32_t max = p_caster->getSkillLineMax(skillLine);
        if (amt >= max)
            return;
        if (amt >= skill_line_ability->grey)   //grey
            chance = 0.0f;
        else if ((amt >= (((skill_line_ability->grey - skill_line_ability->green) / 2) + skill_line_ability->green)))          //green
            chance = 33.0f;
        else if (amt >= skill_line_ability->green)   //yellow
            chance = 66.0f;
        else //brown
            chance = 100.0f;
    }
    if (Util::checkChance(chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
        p_caster->advanceSkillLine(skillLine, static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));
}

bool Spell::IsAspect()
{
    switch (getSpellInfo()->getId())
    {
        case 2596:
        case 5118:
        case 14320:
        case 13159:
        case 13161:
        case 20190:
        case 20043:
        case 14322:
        case 14321:
        case 13163:
        case 14319:
        case 14318:
        case 13165:
            return true;
        default:
            return false;
    }
}

bool Spell::IsSeal()
{
    switch (getSpellInfo()->getId())
    {
        case 13903:
        case 17177:
        case 20154:
        case 20164:
        case 20165:
        case 20166:
        case 20375:
        case 21084:
        case 31801:
        case 31892:
        case 53720:
        case 53736:
            return true;
        default:
            return false;
    }
}

void Spell::InitProtoOverride()
{
    if (m_spellInfo_override != nullptr)
        return;
    m_spellInfo_override = sSpellMgr.getSpellInfo(getSpellInfo()->getId());
}

uint32_t Spell::GetBaseThreat(uint32_t dmg)
{
    //there should be a formula to determine what spell cause threat and which don't
    return dmg;
}

uint32_t Spell::GetMechanic(SpellInfo const* sp)
{
    if (sp->getMechanicsType())
        return sp->getMechanicsType();
    if (sp->getEffectMechanic(2))
        return sp->getEffectMechanic(2);
    if (sp->getEffectMechanic(1))
        return sp->getEffectMechanic(1);
    if (sp->getEffectMechanic(0))
        return sp->getEffectMechanic(0);

    return 0;
}

uint8_t Spell::CanCast(bool /*tolerate*/)
{
    /**
     * Object cast checks
     */
    if (m_caster && m_caster->IsInWorld())
    {
        Unit* target = m_caster->getWorldMap()->getUnit(m_targets.getUnitTargetGuid());

        /**
         * Check for valid targets
         */
        if (target)
        {
            switch (getSpellInfo()->getId())
            {
                // SPELL_HASH_DEATH_PACT
                case 17471:
                case 17698:
                case 48743:
                case 51956:
                {
                    if (target->getSummonedByGuid() != m_caster->getGuid())
                        return SPELL_FAILED_BAD_TARGETS;
                } break;
            }
        }

        /**
         * Check for valid location
         */
        if (getSpellInfo()->getId() == 32146)
        {
            Creature* corpse = m_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 18240);
            if (corpse != nullptr)
                if (m_caster->CalcDistance(m_caster, corpse) > 5)
                    return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 39246)
        {
            Creature* cleft = m_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 22105);
            if (cleft == nullptr || cleft->isAlive())
                return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 30988)
        {
            Creature* corpse = m_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 17701);
            if (corpse != nullptr)
                if (m_caster->CalcDistance(m_caster, corpse) > 5 || corpse->isAlive())
                    return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 43723)
        {
            Creature* abysal = p_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), 19973);
            if (abysal != nullptr)
            {
                if (!abysal->isAlive())
                    if (!(p_caster->getItemInterface()->GetItemCount(31672) > 1 && p_caster->getItemInterface()->GetItemCount(31673) > 0 && p_caster->CalcDistance(p_caster, abysal) < 10))
                        return SPELL_FAILED_NOT_HERE;
            }
            else
                return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 32307)
        {
            Creature* kilsorrow = p_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ());
            if (kilsorrow == nullptr || kilsorrow->isAlive() || p_caster->CalcDistance(p_caster, kilsorrow) > 1)
                return SPELL_FAILED_NOT_HERE;
            if (kilsorrow->getEntry() != 17147 && kilsorrow->getEntry() != 17148 && kilsorrow->getEntry() != 18397 && kilsorrow->getEntry() != 18658 && kilsorrow->getEntry() != 17146)
                return SPELL_FAILED_NOT_HERE;
        }
    }

    /**
     * Player caster checks
     */
    if (p_caster)
    {
        /**
         * On taxi check
         */
        if (!p_caster->isOnTaxi())
        {
            if (getSpellInfo()->getId() == 33836 || getSpellInfo()->getId() == 45072 || getSpellInfo()->getId() == 45115 || getSpellInfo()->getId() == 31958)
                return SPELL_FAILED_NOT_HERE;
        }

        /**
         * Is mounted check
         */
        if (!p_caster->isMounted())
        {
            if (getSpellInfo()->getId() == 25860) // Reindeer Transformation
                return SPELL_FAILED_ONLY_MOUNTED;
        }

         /**
          * check if spell is allowed while we have a battleground flag
          */
        if (p_caster->hasBgFlag())
        {
            switch (getSpellInfo()->getId())
            {
                // stealth spells
                case 1784:
                case 1785:
                case 1786:
                case 1787:
                case 5215:
                case 6783:
                case 9913:
                case 1856:
                case 1857:
                case 26889:
                {
                    if (const auto battleground = p_caster->getBattleground())
                    {
                        if (battleground->getType() == BattlegroundDef::TYPE_WARSONG_GULCH)
                            battleground->HookOnFlagDrop(p_caster);
                        else if (battleground->getType() == BattlegroundDef::TYPE_EYE_OF_THE_STORM)
                            battleground->HookOnFlagDrop(p_caster);
                    }
                    break;
                }
            }
        }
    }

    /**
     * Targeted Unit Checks
     */
    if (m_targets.getUnitTargetGuid())
    {
        Unit* target = (m_caster->IsInWorld()) ? m_caster->getWorldMap()->getUnit(m_targets.getUnitTargetGuid()) : NULL;

        if (target)
        {
            // \todo Replace this awful hacks with a better solution
            // Nestlewood Owlkin - Quest 9303
            if (getSpellInfo()->getId() == 29528 && target->isCreature() && target->getEntry() == 16518)
            {
                if (target->isRooted())
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }

                target->setTargetGuid(p_caster->getGuid());
                return SPELL_CAST_SUCCESS;
            }

            // Lazy Peons - Quest 5441
            if (getSpellInfo()->getId() == 19938 && target->isCreature() && target->getEntry() == 10556)
            {
                if (!target->hasAurasWithId(17743))
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }

                return SPELL_CAST_SUCCESS;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // scripted spell stuff
            switch (getSpellInfo()->getId())
            {
                case 603: //curse of doom, can't be cast on players
                case 30910:
                case 47867: // Curse of doom rank 4
                {
                    if (target->isPlayer())
                        return SPELL_FAILED_TARGET_IS_PLAYER;
                }
                break;
                case 13907: // Smite Demon
                {
                    if (target->isPlayer() || target->getClass() != TARGET_TYPE_DEMON)
                        return SPELL_FAILED_SPELL_UNAVAILABLE;
                }
                break;

                default:
                    break;
            }

            // if the target is not the unit caster and not the masters pet
            if (target != u_caster && !m_caster->isPet())
            {

                /***********************************************************
                * Inface checks, these are checked in 2 ways
                * 1e way is check for damage type, as 3 is always ranged
                * 2e way is trough the data in the extraspell db
                *
                **********************************************************/

                uint32_t facing_flags = getSpellInfo()->getFacingCasterFlags();

                // Holy shock need enemies be in front of caster
                switch (getSpellInfo()->getId())
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
                    {
                        if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_DUMMY && !u_caster->isFriendlyTo(target))
                            facing_flags = SPELL_INFRONT_STATUS_REQUIRE_INFRONT;
                    } break;
                }
            }

            // fishing spells
            if (getSpellInfo()->getEffectImplicitTargetA(0) == EFF_TARGET_SELF_FISHING)  //||
                //GetProto()->EffectImplicitTargetA[1] == EFF_TARGET_SELF_FISHING ||
                //GetProto()->EffectImplicitTargetA[2] == EFF_TARGET_SELF_FISHING)
            {
                uint32_t entry = getSpellInfo()->getEffectMiscValue(0);
                if (entry == GO_FISHING_BOBBER)
                {
                    WorldMap* map = m_caster->getWorldMap();
                    float minDist = m_spellInfo->getMinRange(true);
                    float maxDist = m_spellInfo->getMaxRange(true);
                    float posx = 0, posy = 0, posz = 0;
                    float dist = Util::getRandomFloat(minDist, maxDist);

                    float angle = Util::getRandomFloat(0.0f, 1.0f) * static_cast<float>(M_PI * 35.0f / 180.0f) - static_cast<float>(M_PI * 17.5f / 180.0f);
                    m_caster->getClosePoint(posx, posy, posz, 0.388999998569489f, dist, angle);

                    float ground = m_caster->getMapHeight(LocationVector(posx, posy, posz));
                    float liquidLevel = VMAP_INVALID_HEIGHT_VALUE;

                    LiquidData liquidData;
                    if (map->getLiquidStatus(m_caster->GetPhase(), LocationVector(posx, posy, posz), MAP_ALL_LIQUIDS, &liquidData, m_caster->getCollisionHeight()))
                        liquidLevel = liquidData.level;

                    if (liquidLevel <= ground)
                        return SPELL_FAILED_NOT_FISHABLE;

                    if (ground + 0.75 > liquidLevel)
#if VERSION_STRING > Classic
                        return SPELL_FAILED_TOO_SHALLOW;
#else
                        return SPELL_FAILED_NOT_FISHABLE;
#endif

                    // if we are already fishing, don't cast it again
                    if (p_caster->getSummonedObject())
                        if (p_caster->getSummonedObject()->getEntry() == GO_FISHING_BOBBER)
                            return SPELL_FAILED_SPELL_IN_PROGRESS;

                    m_targets.setDestination(LocationVector(posx, posy, liquidLevel));
                }
            }

            //check if we are trying to stealth or turn invisible but it is not allowed right now
            if (IsStealthSpell() || IsInvisibilitySpell())
            {
                uint32_t faerieFireFeral[] =
                {
                    //SPELL_HASH_FAERIE_FIRE__FERAL_
                    16857,
                    60089,
                    0
                };

                //if we have Faerie Fire, we cannot stealth or turn invisible
                uint32_t faerieFire[] =
                {
                    //SPELL_HASH_FAERIE_FIRE
                    770,
                    6950,
                    13424,
                    13752,
                    16498,
                    20656,
                    21670,
                    25602,
                    32129,
                    65863,
                    0
                };

                if (u_caster->getAuraWithId(faerieFire) || u_caster->hasAurasWithId(faerieFireFeral))
                    return SPELL_FAILED_SPELL_UNAVAILABLE;
            }
        }
    }

    // no problems found, so we must be ok
    return SPELL_CAST_SUCCESS;
}

int32_t Spell::DoCalculateEffect(uint32_t i, Unit* target, int32_t value)
{
    //1 switch: checking spell id. If the spell is not handled in the first block,
    //2nd block of checks is reached. bool handled is initialized as true and set to false in the default: case of each switch.
    bool handled = true;

    switch (getSpellInfo()->getId())
    {
        // SPELL_HASH_STEADY_SHOT:
        case 34120:
        case 49051:
        case 49052:
        case 56641:
        case 65867:
        {
            if (u_caster != nullptr && i == 0)
            {
                if (p_caster != nullptr)
                {
                    Item* it;
                    it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    if (it)
                    {
                        float weapondmg = Util::getRandomFloat(1) * (it->getItemProperties()->Damage[0].Max - it->getItemProperties()->Damage[0].Min) + it->getItemProperties()->Damage[0].Min;
                        value += Util::float2int32(getSpellInfo()->getEffectBasePoints(0) + weapondmg / (it->getItemProperties()->Delay / 1000.0f) * 2.8f);
                    }
                }
                if (target && target->isDazed())
                    value += getSpellInfo()->getEffectBasePoints(1);
                value += (uint32_t)(u_caster->getCalculatedRangedAttackPower() * 0.1);
            }
        } break;

        // SPELL_HASH_REND:
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
        {
            if (p_caster != nullptr)
            {
                Item* it;
                it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (it)
                {
                    if (it->getItemProperties()->Class == 2)
                    {
                        float avgwepdmg = (it->getItemProperties()->Damage[0].Min + it->getItemProperties()->Damage[0].Max) * 0.5f;
                        float wepspd = (it->getItemProperties()->Delay * 0.001f);
                        int32_t dmg = Util::float2int32((avgwepdmg)+p_caster->getCalculatedAttackPower() / 14 * wepspd);

                        if (target && target->getHealthPct() > 75)
                        {
                            dmg = Util::float2int32(dmg + dmg * 0.35f);
                        }

                        value += dmg / 5;
                    }
                }
            }
        } break;

        // SPELL_HASH_SLAM:
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
        {
            if (p_caster != nullptr)
            {
                auto mainHand = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mainHand != nullptr)
                {
                    float avgWeaponDmg = (mainHand->getItemProperties()->Damage[0].Max + mainHand->getItemProperties()->Damage[0].Min) / 2;
                    value += Util::float2int32((getSpellInfo()->calculateEffectValue(0)) + avgWeaponDmg);
                }
            }
        } break;

        // SPELL_HASH_EVISCERATE:
        case 2098:
        case 6760:
        case 6761:
        case 6762:
        case 8623:
        case 8624:
        case 11299:
        case 11300:
        case 15691:
        case 15692:
        case 26865:
        case 27611:
        case 31016:
        case 41177:
        case 46189:
        case 48667:
        case 48668:
        case 57641:
        case 60008:
        case 65957:
        case 67709:
        case 68094:
        case 68095:
        case 68096:
        case 68317:
        case 71933:
        {
            if (p_caster != nullptr)
                value += (uint32_t)(p_caster->getCalculatedAttackPower() * 0.03f * p_caster->getComboPoints());
        } break;

        // SPELL_HASH_FEROCIOUS_BITE:
        case 22568:
        case 22827:
        case 22828:
        case 22829:
        case 24248:
        case 27557:
        case 31018:
        case 48576:
        case 48577:
        {
            if (p_caster != nullptr)
            {
                value += (uint32_t)((p_caster->getCalculatedAttackPower() * 0.1526f) + (p_caster->getPower(POWER_TYPE_ENERGY) * getSpellInfo()->getEffectDamageMultiplier(static_cast<uint8_t>(i))));
                p_caster->setPower(POWER_TYPE_ENERGY, 0);
            }
        } break;

        // SPELL_HASH_VICTORY_RUSH:
        case 34428:
        {
            //causing ${$AP*$m1/100} damage
            if (u_caster != nullptr && i == 0)
                value = (value * u_caster->getCalculatedAttackPower()) / 100;
        } break;

        // SPELL_HASH_RAKE:
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
        {
            //Rake the target for ${$AP/100+$m1} bleed damage and an additional ${$m2*3+$AP*0.06} damage over $d.
            if (u_caster != nullptr)
            {
                float ap = float(u_caster->getCalculatedAttackPower());
                if (i == 0)
                    value += Util::float2int32(ceilf(ap * 0.01f)); // / 100
                else if (i == 1)
                    value += Util::float2int32(ap * 0.06f);
            }
        } break;

        // SPELL_HASH_GARROTE:
        case 703:
        case 8631:
        case 8632:
        case 8633:
        case 8818:
        case 11289:
        case 11290:
        case 26839:
        case 26884:
        case 37066:
        case 48675:
        case 48676:
        {
            // WoWWiki says +(0.18 * attack power / number of ticks)
            // Tooltip gives no specific reading, but says ", increased by your attack power.".
            if (u_caster != nullptr && i == 0)
                value += (uint32_t)ceilf((u_caster->getCalculatedAttackPower() * 0.07f) / 6);
        } break;

        // SPELL_HASH_RUPTURE:
        case 1943:
        case 8639:
        case 8640:
        case 11273:
        case 11274:
        case 11275:
        case 14874:
        case 14903:
        case 15583:
        case 26867:
        case 48671:
        case 48672:
        {
            /*
            1pt = Attack Power * 0.04 + x
            2pt = Attack Power * 0.10 + y
            3pt = Attack Power * 0.18 + z
            4pt = Attack Power * 0.21 + a
            5pt = Attack Power * 0.24 + b
            */
            if (p_caster != nullptr && i == 0)
            {
                int8_t cp = p_caster->getComboPoints();
                value += (uint32_t)ceilf((u_caster->getCalculatedAttackPower() * 0.04f * cp) / ((6 + (cp << 1)) >> 1));
            }
        } break;

        // SPELL_HASH_RIP:
        case 1079:
        case 9492:
        case 9493:
        case 9752:
        case 9894:
        case 9896:
        case 27008:
        case 33912:
        case 36590:
        case 49799:
        case 49800:
        case 57661:
        case 59989:
        case 71926:
        {
            if (p_caster != nullptr)
                value += Util::float2int32(p_caster->getCalculatedAttackPower() * 0.01f * p_caster->getComboPoints());
        } break;

        // SPELL_HASH_MONGOOSE_BITE:
        case 1495:
        case 14269:
        case 14270:
        case 14271:
        case 36916:
        case 53339:
        {
            // ${$AP*0.2+$m1} damage.
            if (u_caster != nullptr)
                value += u_caster->getCalculatedAttackPower() / 5;
        } break;

        // SPELL_HASH_SWIPE:
        case 27554:
        case 31279:
        case 50256:
        case 53498:
        case 53526:
        case 53528:
        case 53529:
        case 53532:
        case 53533:
        {
            // ${$AP*0.06+$m1} damage.
            if (u_caster != nullptr)
                value += Util::float2int32(u_caster->getCalculatedAttackPower() * 0.06f);
        } break;

        // SPELL_HASH_HAMMER_OF_THE_RIGHTEOUS:
        case 53595:
        case 54423:
        case 66867:
        case 66903:
        case 66904:
        case 66905:
        case 67680:
        {
            if (p_caster != nullptr)
                //4x 1h weapon-dps ->  4*(mindmg+maxdmg)/speed/2 = 2*(mindmg+maxdmg)/speed
                value = Util::float2int32((p_caster->getMinDamage() + p_caster->getMaxDamage()) / (float(p_caster->getBaseAttackTime(MELEE)) / 1000.0f)) << 1;
        } break;

        // SPELL_HASH_BACKSTAB:  // Egari: spell 31220 is interfering with combopoints
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
        {
            if (i == 2)
                return getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i)) + 1;
        } break;

        // SPELL_HASH_FAN_OF_KNIVES:  // rogue - fan of knives
        case 51723:
        case 52874:
        case 61739:
        case 61740:
        case 61741:
        case 61742:
        case 61743:
        case 61744:
        case 61745:
        case 61746:
        case 63753:
        case 65955:
        case 67706:
        case 68097:
        case 68098:
        case 68099:
        case 69921:
        case 71128:
        {
            if (p_caster != nullptr)
            {
                Item* mit = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mit != nullptr)
                {
                    if (mit->getItemProperties()->Class == 2 && mit->getItemProperties()->SubClass == 15)   // daggers
                        value = 105;
                }
            }
        } break;

        // SPELL_HASH_SEAL_OF_RIGHTEOUSNESS:
        case 20154:
        case 21084:
        case 25742:
        {
            if (p_caster != nullptr)
            {
                Item* mit = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mit != nullptr)
                    value = (p_caster->getCalculatedAttackPower() * 22 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 44) * mit->getItemProperties()->Delay / 1000000;
            }
        } break;

        // SPELL_HASH_BLOOD_CORRUPTION:
        case 53742:
        // SPELL_HASH_HOLY_VENGEANCE:
        case 31803:
        {
            if (p_caster != nullptr)
                value = (p_caster->getCalculatedAttackPower() * 25 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 13) / 1000;
        } break;

        // SPELL_HASH_JUDGEMENT:
        case 10321:
        case 23590:
        case 23591:
        case 35170:
        case 41467:
        case 43838:
        case 54158:
        {
            if (p_caster != nullptr)
                value += (p_caster->getCalculatedAttackPower() * 16 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 25) / 100;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS:
        case 20187:
        {
            if (p_caster != nullptr)
                value += (p_caster->getCalculatedAttackPower() * 2 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 32) / 100;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_VENGEANCE:
        case 31804:
        // SPELL_HASH_JUDGEMENT_OF_CORRUPTION:
        case 53733:
        {
            if (p_caster != nullptr)
                value += (p_caster->getCalculatedAttackPower() * 14 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 22) / 100;
        } break;

        // SPELL_HASH_ENVENOM:
        case 32645:
        case 32684:
        case 39967:
        case 41487:
        case 41509:
        case 41510:
        case 57992:
        case 57993:
        {
            if (p_caster != nullptr && i == 0)
            {
                value *= p_caster->getComboPoints();
                value += (uint32_t)(p_caster->getCalculatedAttackPower() * (0.09f * p_caster->getComboPoints()));
            }
        } break;

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
        {
            if (u_caster != nullptr && i == 0)
                value += (uint32_t)ceilf(u_caster->getCalculatedAttackPower() * 0.21f);
        } break;
        default:
        {
            handled = false;
        } break;
    }

    if (!handled)
    {
        //it will be set to false in the default case of the switch.
        handled = true;
        switch (getSpellInfo()->getId())
        {
            case 34123:  //Druid - Tree of Life
            {
                if (p_caster != nullptr && i == 0)
                    //Heal is increased by 6%
                    value = Util::float2int32(value * 1.06f);
                break;
            }
            case 57669: //Replenishment
            case 61782:
            {
                if (p_caster != nullptr && i == 0 && target != nullptr)
                    value = int32_t(0.002 * target->getMaxPower(POWER_TYPE_MANA));
                break;
            }
            default:
            {
                //not handled in this switch
                handled = false;
                break;
            }
        }

        if (!handled)
        {
            if (getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_POISON && u_caster != nullptr)   // poison damage modifier
            {
                switch (getSpellInfo()->getId())
                {
                    // SPELL_HASH_DEADLY_POISON_IX:
                    case 57970:
                    case 57973:
                    // SPELL_HASH_DEADLY_POISON_VIII:
                    case 57969:
                    case 57972:
                    // SPELL_HASH_DEADLY_POISON_VII:
                    case 27186:
                    case 27187:
                    // SPELL_HASH_DEADLY_POISON_VI:
                    case 26967:
                    case 26968:
                    // SPELL_HASH_DEADLY_POISON_V:
                    case 25349:
                    case 25351:
                    // SPELL_HASH_DEADLY_POISON_IV:
                    case 11354:
                    case 11356:
                    // SPELL_HASH_DEADLY_POISON_III:
                    case 11353:
                    case 11355:
                    // SPELL_HASH_DEADLY_POISON_II:
                    case 2819:
                    case 2824:
                    // SPELL_HASH_DEADLY_POISON:
                    case 2818:
                    case 2823:
                    case 3583:
                    case 10022:
                    case 13582:
                    case 21787:
                    case 21788:
                    case 32970:
                    case 32971:
                    case 34616:
                    case 34655:
                    case 34657:
                    case 36872:
                    case 38519:
                    case 38520:
                    case 41191:
                    case 41192:
                    case 41485:
                    case 43580:
                    case 43581:
                    case 56145:
                    case 56149:
                    case 59479:
                    case 59482:
                    case 63755:
                    case 63756:
                    case 67710:
                    case 67711:
                    case 68315:
                    case 72329:
                    case 72330:
                        if (getSpellInfo()->getEffectApplyAuraName(static_cast<uint8_t>(i)) == SPELL_AURA_PERIODIC_DAMAGE)
                            value += Util::float2int32(u_caster->getCalculatedAttackPower() * 0.03f);
                        break;
                    // SPELL_HASH_INSTANT_POISON_IX:
                    case 57965:
                    case 57968:
                    // SPELL_HASH_INSTANT_POISON_VIII:
                    case 57964:
                    case 57967:
                    // SPELL_HASH_INSTANT_POISON_VII:
                    case 26890:
                    case 26891:
                    // SPELL_HASH_INSTANT_POISON_VI:
                    case 11337:
                    case 11340:
                    // SPELL_HASH_INSTANT_POISON_V:
                    case 11336:
                    case 11339:
                    // SPELL_HASH_INSTANT_POISON_IV:
                    case 11335:
                    case 11338:
                    // SPELL_HASH_INSTANT_POISON_III:
                    case 8688:
                    case 8689:
                    // SPELL_HASH_INSTANT_POISON_II:
                    case 8685:
                    case 8686:
                    // SPELL_HASH_INSTANT_POISON:
                    case 8679:
                    case 8680:
                    case 28428:
                    case 41189:
                    case 59242:
                        if (getSpellInfo()->getEffect(static_cast<uint8_t>(i)) == SPELL_EFFECT_SCHOOL_DAMAGE)
                            value += Util::float2int32(u_caster->getCalculatedAttackPower() * 0.10f);
                        break;
                    // SPELL_HASH_WOUND_POISON_VII:
                    case 57975:
                    case 57978:
                    // SPELL_HASH_WOUND_POISON_VI:
                    case 57974:
                    case 57977:
                    // SPELL_HASH_WOUND_POISON_V:
                    case 27188:
                    case 27189:
                    // SPELL_HASH_WOUND_POISON_IV:
                    case 13224:
                    case 13227:
                    // SPELL_HASH_WOUND_POISON_III:
                    case 13223:
                    case 13226:
                    // SPELL_HASH_WOUND_POISON_II:
                    case 13222:
                    case 13225:
                    // SPELL_HASH_WOUND_POISON:
                    case 13218:
                    case 13219:
                    case 30984:
                    case 36974:
                    case 39665:
                    case 43461:
                    case 54074:
                    case 65962:
                        if (getSpellInfo()->getEffect(static_cast<uint8_t>(i)) == SPELL_EFFECT_SCHOOL_DAMAGE)
                            value += Util::float2int32(u_caster->getCalculatedAttackPower() * 0.04f);
                        break;
                }
            }
        }
    }

    return value;
}

void Spell::DoAfterHandleEffect(Unit* /*target*/, uint32_t /*i*/)
{
}

void Spell::HandleTeleport(LocationVector pos, uint32_t mapid, Unit* Target)
{
    if (Target->isPlayer())
    {
        Player* pTarget = static_cast<Player*>(Target);
        pTarget->eventAttackStop();
        pTarget->setTargetGuid(0);

        // We use a teleport event on this one. Reason being because of UpdateCellActivity,
        // the game object set of the updater thread WILL Get messed up if we teleport from a gameobject
        // caster.

        if (!sEventMgr.HasEvent(pTarget, EVENT_PLAYER_TELEPORT))
        {
            sEventMgr.AddEvent(pTarget, &Player::eventTeleport, mapid, pos, uint32_t(0), EVENT_PLAYER_TELEPORT, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
    else
    {
        if (mapid != Target->GetMapId())
        {
            sLogger.failure("Tried to teleport a Creature to another map.");
            return;
        }

        WorldPacket data(SMSG_MONSTER_MOVE, 50);

        data << Target->GetNewGUID();
        data << uint8_t(0);
        data << Target->GetPositionX();
        data << Target->GetPositionY();
        data << Target->GetPositionZ();
        data << Util::getMSTime();
        data << uint8_t(0x00);
        data << uint32_t(256);
        data << uint32_t(1);
        data << uint32_t(1);
        data << float(pos.x);
        data << float(pos.y);
        data << float(pos.z);

        Target->sendMessageToSet(&data, true);
        Target->SetPosition(pos.x, pos.y, pos.z, pos.o);
    }
}

void Spell::CreateItem(uint32_t itemId)
{
    /// Creates number of items equal to a "damage" of the effect
    if (itemId == 0 || p_caster == nullptr)
        return;

    p_caster->getItemInterface()->AddItemById(itemId, damage, 0);
}

uint32_t Spell::GetType() { return (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_NONE ? SPELL_DMG_TYPE_MAGIC : getSpellInfo()->getDmgClass()); }

void Spell::DetermineSkillUp(uint16_t skillid, uint32_t targetlevel, uint32_t multiplicator)
{
    if (p_caster == nullptr)
        return;

    if (p_caster->getSkillUpChance(skillid) < 0.01)
        return;//to prevent getting higher skill than max

    int32_t diff = p_caster->getSkillLineCurrent(skillid, false) / 5 - targetlevel;

    if (diff < 0)
        diff = -diff;

    float chance;
    if (diff <= 5)
        chance = 95.0f;
    else if (diff <= 10)
        chance = 66.0f;
    else if (diff <= 15)
        chance = 33.0f;
    else
        return;

    if (multiplicator == 0)
        multiplicator = 1;

    if (Util::checkChance((chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)) * multiplicator))
    {
        p_caster->advanceSkillLine(skillid, static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));

        uint32_t value = p_caster->getSkillLineCurrent(skillid, true);
        uint32_t spellid = 0;

        // Lifeblood
        if (skillid == SKILL_HERBALISM)
        {
            switch (value)
            {
                case 75:
                {   spellid = 55428; }
                break;// Rank 1
                case 150:
                {   spellid = 55480; }
                break;// Rank 2
                case 225:
                {   spellid = 55500; }
                break;// Rank 3
                case 300:
                {   spellid = 55501; }
                break;// Rank 4
                case 375:
                {   spellid = 55502; }
                break;// Rank 5
                case 450:
                {   spellid = 55503; }
                break;// Rank 6
                case 525:
                {    spellid = 74497; }
                break;// Rank 7
            }
        }

        // Toughness
        else if (skillid == SKILL_MINING)
        {
            switch (value)
            {
                case 75:
                {   spellid = 53120; }
                break;// Rank 1
                case 150:
                {   spellid = 53121; }
                break;// Rank 2
                case 225:
                {   spellid = 53122; }
                break;// Rank 3
                case 300:
                {   spellid = 53123; }
                break;// Rank 4
                case 375:
                {   spellid = 53124; }
                break;// Rank 5
                case 450:
                {   spellid = 53040; }
                break;// Rank 6
                case 525:
                {    spellid = 74496; }
                break;// Rank 7
            }
        }


        // Master of Anatomy
        else if (skillid == SKILL_SKINNING)
        {
            switch (value)
            {
                case 75:
                {   spellid = 53125; }
                break;// Rank 1
                case 150:
                {   spellid = 53662; }
                break;// Rank 2
                case 225:
                {   spellid = 53663; }
                break;// Rank 3
                case 300:
                {   spellid = 53664; }
                break;// Rank 4
                case 375:
                {   spellid = 53665; }
                break;// Rank 5
                case 450:
                {   spellid = 53666; }
                break;// Rank 6
                case 525:
                {   spellid = 74495; }
                break;// Rank 7
            }
        }

        if (spellid != 0)
            p_caster->addSpell(spellid, skillid);
    }
}

void Spell::DetermineSkillUp(uint16_t skillid)
{
    //This code is wrong for creating items and disenchanting.
    if (p_caster == nullptr)
        return;

    auto skill_line_ability = sSpellMgr.getFirstSkillEntryForSpell(getSpellInfo()->getId());
    if (skill_line_ability != nullptr && skillid == skill_line_ability->skilline && p_caster->hasSkillLine(skillid))
    {
        float chance = 0.0f;
        uint32_t amt = p_caster->getSkillLineCurrent(skillid, false);
        uint32_t max = p_caster->getSkillLineMax(skillid);
        if (amt >= max)
            return;
        if (amt >= skill_line_ability->grey)   //grey
            chance = 0.0f;
        else if ((amt >= (((skill_line_ability->grey - skill_line_ability->green) / 2) + skill_line_ability->green)))          //green
            chance = 33.0f;
        else if (amt >= skill_line_ability->green)   //yellow
            chance = 66.0f;
        else //brown
            chance = 100.0f;

        if (Util::checkChance(chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            p_caster->advanceSkillLine(skillid, static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));
    }
}

void Spell::SafeAddTarget(std::vector<uint64_t>* tgt, uint64_t guid)
{
    if (guid == 0)
        return;

    for (std::vector<uint64_t>::iterator i = tgt->begin(); i != tgt->end(); ++i)
    {
        if (*i == guid)
        {
            return;
        }
    }

    tgt->push_back(guid);
}

uint32_t Spell::getState() const
{
    return m_spellState;
}

bool Spell::DuelSpellNoMoreValid() const
{
    if (duelSpell && (
        (p_caster != nullptr && p_caster->getDuelState() != DUEL_STATE_STARTED) ||
        (u_caster != nullptr && u_caster->isPet() && static_cast<Pet*>(u_caster)->getPlayerOwner() && static_cast<Pet*>(u_caster)->getPlayerOwner()->getDuelState() != DUEL_STATE_STARTED)))
        return true;
    else
        return false;
}

bool Spell::GetSpellFailed() const
{
    return m_Spell_Failed;
}

void Spell::SetSpellFailed(bool failed)
{
    m_Spell_Failed = failed;
}

void Spell::SpellEffectJumpTarget(uint8_t effectIndex)
{
    if (u_caster == nullptr)
        return;

#ifdef FT_VEHICLES
    if (u_caster->getVehicleKit() || u_caster->isTrainingDummy())
        return;
#else
    if (u_caster->isTrainingDummy())
        return;
#endif

    float x = 0;
    float y = 0;
    float z = 0;
    float o = 0;

    if (m_targets.getTargetMask() & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());

        if (uobj == nullptr || !uobj->isCreatureOrPlayer())
        {
            return;
        }

        float rad = m_unitTarget->getBoundingRadius() - u_caster->getBoundingRadius();

        float dx = m_caster->GetPositionX() - m_unitTarget->GetPositionX();
        float dy = m_caster->GetPositionY() - m_unitTarget->GetPositionY();

        if (dx == 0.0f || dy == 0.0f)
        {
            return;
        }

        float alpha = atanf(dy / dx);
        if (dx < 0)
        {
            alpha += M_PI_FLOAT;
        }

        x = rad * cosf(alpha) + m_unitTarget->GetPositionX();
        y = rad * sinf(alpha) + m_unitTarget->GetPositionY();
        z = m_unitTarget->GetPositionZ();
    }
    else
    {
        //this can also jump to a point
        if (m_targets.hasSource())
        {
            auto source = m_targets.getSource();
            x = source.x;
            y = source.y;
            z = source.z;
        }
        if (m_targets.hasDestination())
        {
            auto destination = m_targets.getDestination();
            x = destination.x;
            y = destination.y;
            z = destination.z;
        }
    }

    float speedZ = 0.0f;
    float speedXY = 0.0f;

    o = m_unitTarget->calcRadAngle(u_caster->GetPositionX(), u_caster->GetPositionY(), x, y);
    calculateJumpSpeeds(u_caster, getSpellInfo() ,effectIndex, u_caster->getExactDist2d(x, y), speedXY, speedZ);
    u_caster->getMovementManager()->moveJump(x, y, z, o, speedXY, speedZ);
}

void Spell::calculateJumpSpeeds(Unit* unitCaster, SpellInfo const* spellInfo, uint8_t i, float dist, float& speedXY, float& speedZ)
{
    float runSpeed = unitCaster->getSpeedRate(TYPE_RUN, false);

    if (Creature* creature = unitCaster->ToCreature())
        runSpeed *= creature->GetCreatureProperties()->run_speed;

    float multiplier = m_spellInfo->getEffectMultipleValue(i);
    if (multiplier <= 0.0f)
        multiplier = 1.0f;

    speedXY = std::min(runSpeed * 3.0f * multiplier, std::max(28.0f, unitCaster->getSpeedRate(TYPE_RUN, false) * 4.0f));

    float duration = dist / speedXY;
    float durationSqr = duration * duration;
    float minHeight = spellInfo->getEffectMiscValue(i) ? spellInfo->getEffectMiscValue(i) / 10.0f : 0.5f; // Lower bound is blizzlike
    float maxHeight = spellInfo->getEffectMiscValueB(i) ? spellInfo->getEffectMiscValueB(i) / 10.0f : 1000.0f; // Upper bound is unknown
    float height;

    if (durationSqr < minHeight * 8 / MovementMgr::gravity)
        height = minHeight;
    else if (durationSqr > maxHeight * 8 / MovementMgr::gravity)
        height = maxHeight;
    else
        height = MovementMgr::gravity * durationSqr / 8;

    speedZ = std::sqrt(2 * MovementMgr::gravity * height);
}

void Spell::SpellEffectJumpBehindTarget(uint8_t effectIndex)
{
    if (u_caster == nullptr)
        return;

    if (!m_targets.hasDestination())
        return;

    if (m_targets.getTargetMask() & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());

        if (uobj == nullptr || !uobj->isCreatureOrPlayer())
            return;
        Unit* un = static_cast<Unit*>(uobj);
        float rad = un->getBoundingRadius() + u_caster->getBoundingRadius();
        float angle = float(un->GetOrientation() + M_PI); //behind
        float x = un->GetPositionX() + cosf(angle) * rad;
        float y = un->GetPositionY() + sinf(angle) * rad;
        float z = un->GetPositionZ();
        float o = un->calcRadAngle(x, y, un->GetPositionX(), un->GetPositionY());
       
        float speedXY, speedZ;
        calculateJumpSpeeds(u_caster, getSpellInfo() ,effectIndex, u_caster->getExactDist2d(un->GetPositionX(), un->GetPositionY()), speedXY, speedZ);
        u_caster->getMovementManager()->moveJump(x, y, z, o, speedXY, speedZ, EVENT_JUMP, !m_targets.getUnitTargetGuid());
    }
}

void Spell::HandleTargetNoObject()
{
    float dist = 3;
    float newx = m_caster->GetPositionX() + cosf(m_caster->GetOrientation()) * dist;
    float newy = m_caster->GetPositionY() + sinf(m_caster->GetOrientation()) * dist;
    float newz = m_caster->GetPositionZ();

    //clamp Z
    newz = m_caster->getMapHeight(LocationVector(newx, newy, newz));

    bool isInLOS = m_caster->IsWithinLOS(LocationVector(newx, newy, newz));
    //if not in line of sight, or too far away we summon inside caster
    if (fabs(newz - m_caster->GetPositionZ()) > 10 || !isInLOS)
    {
        newx = m_caster->GetPositionX();
        newy = m_caster->GetPositionY();
        newz = m_caster->GetPositionZ();
    }

    m_targets.addTargetMask(TARGET_FLAG_DEST_LOCATION);
    m_targets.setDestination(LocationVector(newx, newy, newz));
}
