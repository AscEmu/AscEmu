/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#include "SpellTarget.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/SmsgClearExtraAuraInfo.h"
#include "Spell.Legacy.h"
#include "Definitions/SpellInFrontStatus.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Definitions/SpellDamageType.h"
#include "Definitions/ProcFlags.h"
#include "Definitions/CastInterruptFlags.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/SpellTargetType.h"
#include "Definitions/SpellRanged.h"
#include "Definitions/SpellIsFlags.h"
#include "Definitions/DiminishingGroup.h"
#include "Definitions/SpellState.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/SpellEffectTarget.h"
#include "Definitions/PowerType.h"
#include "Definitions/SpellDidHitResult.h"
#include "SpellHelpers.h"
#include "StdAfx.h"
#include "VMapFactory.h"
#include "Management/Item.h"
#include "Objects/DynamicObject.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Management/Battleground/Battleground.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Objects/Faction.h"
#include "SpellMgr.h"
#include "SpellAuras.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Server/Packets/SmsgSpellFailure.h"
#include "Server/Packets/SmsgSpellFailedOther.h"
#include "Server/Packets/SmsgSpellHealLog.h"
#include "Server/Packets/SmsgResurrectRequest.h"
#include "Server/Packets/SmsgSpellDelayed.h"
#include "Server/Packets/SmsgCancelCombat.h"

using namespace AscEmu::Packets;

using AscEmu::World::Spell::Helpers::decimalToMask;
using AscEmu::World::Spell::Helpers::spellModFlatFloatValue;
using AscEmu::World::Spell::Helpers::spellModPercentageFloatValue;
using AscEmu::World::Spell::Helpers::spellModFlatIntValue;
using AscEmu::World::Spell::Helpers::spellModPercentageIntValue;

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

 /// externals for spell system
extern pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS];
extern pSpellTarget SpellTargetHandler[EFF_TARGET_LIST_LENGTH_MARKER];

extern const char* SpellEffectNames[TOTAL_SPELL_EFFECTS];

enum SpellTargetSpecification
{
    TARGET_SPECT_NONE = 0,
    TARGET_SPEC_INVISIBLE = 1,
    TARGET_SPEC_DEAD = 2,
};

Spell::Spell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur)
{
    ARCEMU_ASSERT(Caster != NULL && info != NULL);

    Caster->m_pendingSpells.insert(this);
    m_overrideBasePoints = false;
    m_overridenBasePoints[0] = 0xFFFFFFFF;
    m_overridenBasePoints[1] = 0xFFFFFFFF;
    m_overridenBasePoints[2] = 0xFFFFFFFF;
    chaindamage = 0;
    bDurSet = 0;
    damage = 0;
    m_spellInfo_override = nullptr;
    bRadSet[0] = 0;
    bRadSet[1] = 0;
    bRadSet[2] = 0;

    if ((info->getSpellDifficultyID() != 0) && (Caster->getObjectTypeId() != TYPEID_PLAYER) && (Caster->GetMapMgr() != nullptr) && (Caster->GetMapMgr()->pInstance != nullptr))
    {
        SpellInfo const* SpellDiffEntry = sSpellMgr.getSpellInfoByDifficulty(info->getSpellDifficultyID(), Caster->GetMapMgr()->iInstanceMode);
        if (SpellDiffEntry != nullptr)
            m_spellInfo = SpellDiffEntry;
        else
            m_spellInfo = info;
    }
    else
        m_spellInfo = info;

    m_spellInfo_override = nullptr;
    m_caster = Caster;
    duelSpell = false;
    m_DelayStep = 0;

    switch (Caster->getObjectTypeId())
    {
        case TYPEID_PLAYER:
        {
            g_caster = nullptr;
            i_caster = nullptr;
            u_caster = static_cast<Unit*>(Caster);
            p_caster = static_cast<Player*>(Caster);
            if (p_caster->GetDuelState() == DUEL_STATE_STARTED)
                duelSpell = true;

#ifdef GM_Z_DEBUG_DIRECTLY
            // cebernic added it
            if (p_caster->GetSession() && p_caster->GetSession()->CanUseCommand('z') && p_caster->IsInWorld())
                sChatHandler.BlueSystemMessage(p_caster->GetSession(), "[%sSystem%s] |rSpell::Spell: %s ID:%u,Category%u,CD:%u,DisType%u,Field4:%u,etA0=%u,etA1=%u,etA2=%u,etB0=%u,etB1=%u,etB2=%u", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE,
                info->getId(), info->Category, info->RecoveryTime, info->DispelType, info->castUI, info->EffectImplicitTargetA[0], info->EffectImplicitTargetA[1], info->EffectImplicitTargetA[2], info->EffectImplicitTargetB[0], info->EffectImplicitTargetB[1], info->EffectImplicitTargetB[2]);
#endif

        }
        break;

        case TYPEID_UNIT:
        {
            g_caster = nullptr;
            i_caster = nullptr;
            p_caster = nullptr;
            u_caster = static_cast<Unit*>(Caster);
            if (u_caster->isPet() && static_cast<Pet*>(u_caster)->getPlayerOwner() != nullptr && static_cast<Pet*>(u_caster)->getPlayerOwner()->GetDuelState() == DUEL_STATE_STARTED)
                duelSpell = true;
        }
        break;

        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
        {
            g_caster = nullptr;
            u_caster = nullptr;
            p_caster = nullptr;
            i_caster = static_cast<Item*>(Caster);
            if (i_caster->getOwner() && i_caster->getOwner()->GetDuelState() == DUEL_STATE_STARTED)
                duelSpell = true;
        }
        break;

        case TYPEID_GAMEOBJECT:
        {
            u_caster = nullptr;
            p_caster = nullptr;
            i_caster = nullptr;
            g_caster = static_cast<GameObject*>(Caster);
        }
        break;

        default:
            LogDebugFlag(LF_SPELL, "[DEBUG][SPELL] Incompatible object type, please report this to the dev's");
            break;
    }
    if (u_caster && m_spellInfo->getAttributesExF() & ATTRIBUTESEXF_CAST_BY_CHARMER)
    {
        Unit* u = u_caster->GetMapMgrUnit(u_caster->getCharmedByGuid());
        if (u)
        {
            u_caster = u;
            if (u->isPlayer())
                p_caster = static_cast<Player*>(u);
        }
    }

    m_spellState = SPELL_STATE_NULL;

    //TriggerSpellId = 0;
    //TriggerSpellTarget = 0;
    if (m_spellInfo->getAttributesExD() & ATTRIBUTESEXD_TRIGGERED)
        triggered = true;
    m_triggeredSpell = triggered;
    m_AreaAura = false;

    m_triggeredByAura = aur;

    damageToHit = 0;
    castedItemId = 0;

    m_usesMana = false;
    m_Spell_Failed = false;
    hadEffect = false;
    bDurSet = false;
    bRadSet[0] = false;
    bRadSet[1] = false;
    bRadSet[2] = false;

    cancastresult = SPELL_CAST_SUCCESS;

    m_requiresCP = false;
    unitTarget = nullptr;
    itemTarget = nullptr;
    gameObjTarget = nullptr;
    playerTarget = nullptr;
    corpseTarget = nullptr;
    targetConstraintCreature = nullptr;
    targetConstraintGameObject = nullptr;
    add_damage = 0;
    m_Delayed = false;
    pSpellId = 0;
    m_cancelled = false;
    ProcedOnSpell = nullptr;
    forced_basepoints[0] = forced_basepoints[1] = forced_basepoints[2] = 0;
    extra_cast_number = 0;
    m_isCasting = false;
    m_glyphslot = 0;
    m_charges = info->getProcCharges();

    uniqueHittedTargets.clear();
    missedTargets.clear();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        m_effectTargets[i].clear();
    }

    //create rune avail snapshot
    if (p_caster && p_caster->isClassDeathKnight())
        m_rune_avail_before = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();
    else
        m_rune_avail_before = 0;

    m_target_constraint = sObjectMgr.GetSpellTargetConstraintForSpell(info->getId());

    m_missilePitch = 0;
    m_missileTravelTime = 0;
    m_IsCastedOnSelf = false;
    m_magnetTarget = 0;
    Dur = 0;

    // APGL End
    // MIT Start

    // Check if spell is reflectable
    if (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_MAGIC && !getSpellInfo()->isPassive() &&
        !(getSpellInfo()->getAttributes() & ATTRIBUTES_ABILITY) && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_BE_REFLECTED) &&
        !(getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
    {
        //\ todo: this is not correct but it works for now
        //\ need to check for effect rather than target type
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            switch (getSpellInfo()->getEffectImplicitTargetA(i))
            {
                case EFF_TARGET_SINGLE_ENEMY:
                case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
                case EFF_TARGET_IN_FRONT_OF_CASTER:
                case EFF_TARGET_DUEL:
                    m_canBeReflected = true;
                    break;
                default:
                    break;
            }

            if (m_canBeReflected)
                break;
        }
    }

    // MIT End
    // APGL Start
}

Spell::~Spell()
{
#if VERSION_STRING >= WotLK
    // If this spell deals with rune power, send spell_go to update client
    // For instance, when Dk cast Empower Rune Weapon, if we don't send spell_go, the client won't update
    if (getSpellInfo()->getFirstSchoolFromSchoolMask() && getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
        sendSpellGo();
#endif

    m_caster->m_pendingSpells.erase(this);

    ///////////////////////////// This is from the virtual_destructor shit ///////////////
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_caster->getCurrentSpell(CurrentSpellType(i)) == this)
            m_caster->interruptSpellWithSpellType(CurrentSpellType(i));
    }

    if (m_spellInfo_override != nullptr)
        delete[] m_spellInfo_override;
    ////////////////////////////////////////////////////////////////////////////////////////


    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        m_effectTargets[i].clear();
    }

    uniqueHittedTargets.clear();
    missedTargets.clear();

    std::map<uint64, Aura*>::iterator itr;
    for (itr = m_pendingAuras.begin(); itr != m_pendingAuras.end(); ++itr)
    {
        if (itr->second != nullptr)
            delete itr->second;
    }
}

int32 Spell::event_GetInstanceID()
{
    return m_caster->GetInstanceID();
}

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

void Spell::FillSpecifiedTargetsInArea(float srcx, float srcy, float srcz, uint32 ind, uint32 specification)
{
    FillSpecifiedTargetsInArea(ind, srcx, srcy, srcz, GetRadius(ind), specification);
}

// for the moment we do invisible targets
void Spell::FillSpecifiedTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float range, uint32 /*specification*/)
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
                if (isAttackable(u_caster, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                {
                    did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                    if (did_hit_result != SPELL_DID_HIT_SUCCESS)
                        missedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS));
                    else
                        SafeAddTarget(tmpMap, itr->getGuid());
                }

            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->m_summoner)
                {
                    //trap, check not to attack owner and friendly
                    if (isAttackable(g_caster->m_summoner, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
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
void Spell::FillAllTargetsInArea(LocationVector & location, uint32 ind)
{
    FillAllTargetsInArea(ind, location.x, location.y, location.z, GetRadius(ind));
}

void Spell::FillAllTargetsInArea(float srcx, float srcy, float srcz, uint32 ind)
{
    FillAllTargetsInArea(ind, srcx, srcy, srcz, GetRadius(ind));
}

// We fill all the targets in the area, including the stealth ed one's
void Spell::FillAllTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float range)
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

            if (p_caster && (itr)->isPlayer() && p_caster->GetGroup() && static_cast<Player*>(itr)->GetGroup() && static_cast<Player*>(itr)->GetGroup() == p_caster->GetGroup())      //Don't attack party members!!
            {
                //Dueling - AoE's should still hit the target party member if you're dueling with him
                if (!p_caster->DuelingWith || p_caster->DuelingWith != static_cast<Player*>(itr))
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
                    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                    bool isInLOS = mgr->isInLineOfSight(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), itr->GetPositionX(), itr->GetPositionY(), itr->GetPositionZ());

                    if (m_caster->GetMapId() == itr->GetMapId() && !isInLOS)
                        continue;
                }

                if (u_caster != nullptr)
                {
                    if (isAttackable(u_caster, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                    {
                        did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            missedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS));
                    }
                }
                else //cast from GO
                {
                    if (g_caster != nullptr && g_caster->getCreatedByGuid() && g_caster->m_summoner != nullptr)
                    {
                        //trap, check not to attack owner and friendly
                        if (isAttackable(g_caster->m_summoner, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
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
void Spell::FillAllFriendlyInArea(uint32 i, float srcx, float srcy, float srcz, float range)
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
                    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                    bool isInLOS = mgr->isInLineOfSight(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), itr->GetPositionX(), itr->GetPositionY(), itr->GetPositionZ());

                    if (m_caster->GetMapId() == itr->GetMapId() && !isInLOS)
                        continue;
                }

                if (u_caster != nullptr)
                {
                    if (isFriendly(u_caster, static_cast<Unit*>(itr)))
                    {
                        did_hit_result = static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(itr)));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            missedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result, SPELL_DID_HIT_SUCCESS));
                    }
                }
                else //cast from GO
                {
                    if (g_caster != nullptr && g_caster->getCreatedByGuid() && g_caster->m_summoner != nullptr)
                    {
                        //trap, check not to attack owner and friendly
                        if (isFriendly(g_caster->m_summoner, static_cast<Unit*>(itr)))
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

uint64 Spell::GetSinglePossibleEnemy(uint32 i, float prange)
{
    float r;
    if (prange)
        r = prange;
    else
    {
        r = getSpellInfo()->custom_base_range_or_radius_sqr;
        if (u_caster != nullptr)
        {
            spellModFlatFloatValue(u_caster->SM_FRadius, &r, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageFloatValue(u_caster->SM_PRadius, &r, getSpellInfo()->getSpellFamilyFlags());
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
                if (isAttackable(u_caster, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)) && DidHit(i, static_cast<Unit*>(itr)) == SPELL_DID_HIT_SUCCESS)
                {
                    return itr->getGuid();
                }
            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->m_summoner)
                {
                    //trap, check not to attack owner and friendly
                    if (isAttackable(g_caster->m_summoner, itr, !(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                    {
                        return itr->getGuid();
                    }
                }
            }
        }
    }
    return 0;
}

uint64 Spell::GetSinglePossibleFriend(uint32 i, float prange)
{
    float r;
    if (prange)
        r = prange;
    else
    {
        r = getSpellInfo()->custom_base_range_or_radius_sqr;
        if (u_caster != nullptr)
        {
            spellModFlatFloatValue(u_caster->SM_FRadius, &r, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageFloatValue(u_caster->SM_PRadius, &r, getSpellInfo()->getSpellFamilyFlags());
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
                if (isFriendly(u_caster, static_cast<Unit*>(itr)) && DidHit(i, static_cast<Unit*>(itr)) == SPELL_DID_HIT_SUCCESS)
                {
                    return itr->getGuid();
                }
            }
            else //cast from GO
            {
                if (g_caster && g_caster->getCreatedByGuid() && g_caster->m_summoner)
                {
                    //trap, check not to attack owner and friendly
                    if (isFriendly(g_caster->m_summoner, static_cast<Unit*>(itr)))
                    {
                        return itr->getGuid();
                    }
                }
            }
        }
    }
    return 0;
}

uint8 Spell::DidHit(uint32 effindex, Unit* target)
{
    //note resistchance is vise versa, is full hit chance
    Unit* u_victim = target;
    if (u_victim == nullptr)
        return SPELL_DID_HIT_MISS;

    Player* p_victim = target->isPlayer() ? static_cast<Player*>(target) : NULL;

    float baseresist[3] = { 4.0f, 5.0f, 6.0f };
    int32 lvldiff;
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
    /* Check if the unit is evading                                         */
    /************************************************************************/
    if (u_victim->isCreature() && u_victim->GetAIInterface()->isAiState(AI_STATE_EVADE))
        return SPELL_DID_HIT_EVADE;

    /************************************************************************/
    /* Check if the player target is able to deflect spells                 */
    /* Currently (3.3.5a) there is only spell doing that: Deterrence        */
    /************************************************************************/
    if (p_victim && p_victim->hasAuraWithAuraEffect(SPELL_AURA_DEFLECT_SPELLS))
    {
        return SPELL_DID_HIT_DEFLECT;
    }

    // APGL End
    // MIT Start

    // Check if target can reflect this spell
    if (m_canBeReflected)
    {
        auto isReflected = false;
        for (const auto& reflectAura : u_victim->m_reflectSpellSchool)
        {
            if (reflectAura->school != -1 && reflectAura->school != getSpellInfo()->getFirstSchoolFromSchoolMask())
                continue;

            if (Rand(static_cast<float_t>(reflectAura->chance)))
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
                        if (!u_victim->RemoveAura(reflectAura->spellId))
                        {
                            // ...do it manually
                            delete reflectAura;
                            u_victim->m_reflectSpellSchool.remove(reflectAura);
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
    if (u_victim->SchoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] && !dispelMechanic)
        return SPELL_DID_HIT_IMMUNE;

    /* Check if player target has god mode */
    if (p_victim && p_victim->m_cheats.GodModeCheat)
    {
        return SPELL_DID_HIT_IMMUNE;
    }

    /*************************************************************************/
    /* Check if the target is immune to this mechanic                        */
    /*************************************************************************/
    if (m_spellInfo->getMechanicsType() < TOTAL_SPELL_MECHANICS && u_victim->MechanicsDispels[m_spellInfo->getMechanicsType()])

    {
        // Immune - IF, and ONLY IF, there is no damage component!
        bool no_damage_component = true;
        for (uint8 x = 0; x <= 2; x++)
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
    if (m_spellInfo->getMechanicsType() < TOTAL_SPELL_MECHANICS)
    {
        float res = u_victim->MechanicsResistancesPCT[m_spellInfo->getMechanicsType()];
        if (Rand(res))
            return SPELL_DID_HIT_RESIST;
    }

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    uint32 melee_test_result;
    if (getSpellInfo()->custom_is_melee_spell || getSpellInfo()->custom_is_ranged_spell)
    {
        uint32 _type;
        if (GetType() == SPELL_DMG_TYPE_RANGED)
            _type = RANGED;
        else
        {
            if (hasAttributeExC(ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON))
                _type = OFFHAND;
            else
                _type = MELEE;
        }

        melee_test_result = u_caster->GetSpellDidHitResult(u_victim, _type, getSpellInfo());
        if (melee_test_result != SPELL_DID_HIT_SUCCESS)
            return (uint8)melee_test_result;
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
        resistchance += u_victim->MechanicsResistancesPCT[getSpellInfo()->getMechanicsType()];
    }
    //rating bonus
    if (p_caster != nullptr)
    {
        resistchance -= p_caster->CalcRating(PCR_SPELL_HIT);
        resistchance -= p_caster->GetHitFromSpell();
    }

    // school hit resistance: check all schools and take the minimal
    if (p_victim != nullptr && getSpellInfo()->getSchoolMask() > 0)
    {
        int32 min = 100;
        for (uint8 i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
        {
            if (getSpellInfo()->getSchoolMask() & (1 << i) && min > p_victim->m_resist_hit_spell[i])
                min = p_victim->m_resist_hit_spell[i];
        }
        resistchance += min;
    }

    if (getSpellInfo()->getEffect(static_cast<uint8_t>(effindex)) == SPELL_EFFECT_DISPEL)
    {
        spellModFlatFloatValue(u_victim->SM_FRezist_dispell, &resistchance, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageFloatValue(u_victim->SM_PRezist_dispell, &resistchance, getSpellInfo()->getSpellFamilyFlags());
    }

    float hitchance = 0;
    spellModFlatFloatValue(u_caster->SM_FHitchance, &hitchance, getSpellInfo()->getSpellFamilyFlags());
    resistchance -= hitchance;

    if (hasAttribute(ATTRIBUTES_IGNORE_INVULNERABILITY))
        resistchance = 0.0f;

    if (resistchance >= 100.0f)
        return SPELL_DID_HIT_RESIST;
    else
    {
        uint8 res;
        if (resistchance <= 1.0) //resist chance >=1
            res = (Rand(1.0f) ? uint8(SPELL_DID_HIT_RESIST) : uint8(SPELL_DID_HIT_SUCCESS));
        else
            res = (Rand(resistchance) ? uint8(SPELL_DID_HIT_RESIST) : uint8(SPELL_DID_HIT_SUCCESS));

        if (res == SPELL_DID_HIT_SUCCESS)  // proc handling. mb should be moved outside this function
        {
            // u_caster->HandleProc(PROC_ON_SPELL_LAND,target,GetProto());
        }

        return res;
    }
}

void Spell::cancel()
{
    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    SendInterrupted(0);
    sendCastResult(SPELL_FAILED_INTERRUPTED);

    if (m_spellState == SPELL_STATE_CASTING)
    {
        ///\ todo: aura should be removed only after dynamic object has been removed - Appled
        if (u_caster != nullptr)
            u_caster->RemoveAura(getSpellInfo()->getId());

        if (m_timer > 0 || m_Delayed)
        {
            if (p_caster && p_caster->IsInWorld())
            {
                Unit* pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->getChannelObjectGuid());
                if (!pTarget)
                    pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());

                if (pTarget)
                {
                    pTarget->RemoveAura(getSpellInfo()->getId(), m_caster->getGuid());
                }
                if (m_AreaAura)//remove of blizz and shit like this
                {
                    uint64 guid = p_caster->getChannelObjectGuid();

                    DynamicObject* dynObj = m_caster->GetMapMgr()->GetDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (dynObj)
                        dynObj->Remove();
                }

                if (p_caster->GetSummonedObject())
                {
                    if (p_caster->GetSummonedObject()->IsInWorld())
                        p_caster->GetSummonedObject()->RemoveFromWorld(true);
                    // for now..
                    ARCEMU_ASSERT(p_caster->GetSummonedObject()->isGameObject());
                    delete p_caster->GetSummonedObject();
                    p_caster->SetSummonedObject(nullptr);
                }

                if (m_timer > 0)
                {
                    RemoveItems();
                }
                // p_caster->setAttackTimer(1000, false);
            }
        }
    }

    sendChannelUpdate(0);

    //m_spellState = SPELL_STATE_FINISHED;

    // prevent memory corruption. free it up later.
    // if this is true it means we are currently in the cast() function somewhere else down the stack
    // (recursive spells) and we don't wanna have this class deleted when we return to it.
    // at the end of cast() it will get freed anyway.
    if (!m_isCasting)
        finish(false);
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
                p_caster->RemoveAura(53672);
                p_caster->RemoveAura(54149);
            } break;
        }

        if (getSpellInfo()->custom_c_is_flags == SPELL_FLAG_IS_DAMAGING)
        {
            uint32 arcanePotency[] =
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
                p_caster->RemoveAura(57529);
                p_caster->RemoveAura(57531);
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
        if (p_caster->m_bg)
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

                    uint32 divineShield[] =
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

                    uint32 divineProtection[] =
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

                    uint32 divineShield[] =
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

                    uint32 divineProtection[] =
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
                    if (p_caster->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
                    {
                        if (p_caster->getTeam() == 0)
                            p_caster->RemoveAura(23333);    // ally player drop horde flag if they have it
                        else
                            p_caster->RemoveAura(23335);    // horde player drop ally flag if they have it
                    }
                    if (p_caster->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)

                        p_caster->RemoveAura(34976);        // drop the flag
                    break;
            }
        }
    }
}

void Spell::AddTime(uint32 type)
{
    if (u_caster != nullptr)
    {
        if (getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_DAMAGE_TAKEN)
        {
            u_caster->interruptSpell(getSpellInfo()->getId());
            return;
        }

        float ch = 0;
        spellModFlatFloatValue(u_caster->SM_PNonInterrupt, &ch, getSpellInfo()->getSpellFamilyFlags());
        if (Rand(ch))
            return;

        if (p_caster != nullptr)
        {
            if (Rand(p_caster->SpellDelayResist[type]))
                return;
        }
        if (m_DelayStep == 2)
            return; //spells can only be delayed twice as of 3.0.2
        if (m_spellState == SPELL_STATE_PREPARING)
        {
            // no pushback for some spells
            if ((getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_PUSHBACK) == 0)
                return;
            int32 delay = 500; //0.5 second pushback
            ++m_DelayStep;
            m_timer += delay;
            if (m_timer > m_castTime)
            {
                delay -= (m_timer - m_castTime);
                m_timer = m_castTime;
                if (delay < 0)
                    delay = 1;
            }

            u_caster->SendMessageToSet(SmsgSpellDelayed(u_caster->GetNewGUID(), delay).serialise().get(), true);

            if (p_caster == nullptr)
            {
                //then it's a Creature
                u_caster->GetAIInterface()->AddStopTime(delay);
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
            int32 delay = GetDuration() / 4; //0.5 second push back
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

void Spell::finish(bool successful)
{
    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;

    if (u_caster != nullptr)
    {
        CALL_SCRIPT_EVENT(u_caster, OnCastSpell)(getSpellInfo()->getId());

        if (!sEventMgr.HasEvent(u_caster, EVENT_CREATURE_RESPAWN))
        {
            // call script
            Unit* target = u_caster->GetMapMgrUnit(u_caster->getTargetGuid());
            if (target != nullptr)
            {
                if (target->isCreature())
                {
                    auto creature = static_cast<Creature*>(target);
                    if (creature->GetScript())
                    {
                        CALL_SCRIPT_EVENT(creature, OnHitBySpell)(getSpellInfo()->getId(), u_caster);
                    }
                }
            }
        }

        u_caster->m_canMove = true;
    }
    /* Mana Regenerates while in combat but not for 5 seconds after each spell */
    /* Only if the spell uses mana, will it cause a regen delay.
       is this correct? is there any spell that doesn't use mana that does cause a delay?
       this is for creatures as effects like chill (when they have frost armor on) prevents regening of mana */

       //moved to spellhandler.cpp -> remove item when click on it! not when it finishes

       //enable pvp when attacking another player with spells
    if (p_caster != nullptr)
    {
        if (hasAttribute(ATTRIBUTES_STOP_ATTACK) && p_caster->IsAttacking())
        {
            p_caster->EventAttackStop();
            p_caster->smsg_AttackStop(p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection()));
            p_caster->SendPacket(SmsgCancelCombat().serialise().get());
        }

        if (m_requiresCP && !GetSpellFailed())
        {
            if (p_caster->m_spellcomboPoints)
            {
                p_caster->m_comboPoints = p_caster->m_spellcomboPoints;
                p_caster->UpdateComboPoints(); //this will make sure we do not use any wrong values here
            }
            else
            {
                p_caster->NullComboPoints();
            }
        }

        if (m_Delayed)
        {
            Unit* pTarget = nullptr;
            if (p_caster->IsInWorld())
            {
                pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->getChannelObjectGuid());
                if (!pTarget)
                    pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());
            }

            if (pTarget)
            {
                pTarget->RemoveAura(getSpellInfo()->getId(), m_caster->getGuid());
            }
        }

        switch (getSpellInfo()->getId())
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
                //Maelstrom Weapon
                if (u_caster != nullptr)
                    p_caster->removeAllAurasByIdForGuid(53817, u_caster->getGuid());
            } break;
        }
    }

    if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_SUMMON_OBJECT ||
        getSpellInfo()->getEffect(1) == SPELL_EFFECT_SUMMON_OBJECT ||
        getSpellInfo()->getEffect(2) == SPELL_EFFECT_SUMMON_OBJECT)
        if (p_caster != nullptr)
            p_caster->SetSummonedObject(nullptr);
    /*
    Set cooldown on item
    */
    if (i_caster && i_caster->getOwner() && cancastresult == SPELL_CAST_SUCCESS && !GetSpellFailed())
    {
        uint8 x;
        for (x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        {
            if (i_caster->getItemProperties()->Spells[x].Trigger == USE)
            {
                if (i_caster->getItemProperties()->Spells[x].Id)
                    break;
            }
        }
        // cooldown starts after leaving combat
        if (i_caster->getItemProperties()->Class == ITEM_CLASS_CONSUMABLE && i_caster->getItemProperties()->SubClass == 1)
        {
            i_caster->getOwner()->SetLastPotion(i_caster->getItemProperties()->ItemId);
            if (!i_caster->getOwner()->CombatStatus.IsInCombat())
                i_caster->getOwner()->UpdatePotionCooldown();
        }
        else
        {
            if (x < MAX_ITEM_PROTO_SPELLS)
                i_caster->getOwner()->Cooldown_AddItem(i_caster->getItemProperties(), x);
        }
    }

    // cebernic added it
    // moved this from ::prepare()
    // With preparing got ClearCooldownForspell, it makes too early for player client.
    // Now .cheat cooldown works perfectly.
    if (!m_triggeredSpell && p_caster != nullptr && p_caster->m_cheats.CooldownCheat)
        p_caster->ClearCooldownForSpell(getSpellInfo()->getId());

    // Send Spell cast info to QuestMgr
    if (successful && p_caster != nullptr && p_caster->IsInWorld())
    {
        // Taming quest spells are handled in SpellAuras.cpp, in SpellAuraDummy
        // OnPlayerCast shouldn't be called here for taming-quest spells, in case the tame fails (which is handled in SpellAuras)
        bool isTamingQuestSpell = false;
        uint32 tamingQuestSpellIds[] = { 19688, 19694, 19693, 19674, 19697, 19696, 19687, 19548, 19689, 19692, 19699, 19700, 30099, 30105, 30102, 30646, 30653, 30654, 0 };
        uint32* spellidPtr = tamingQuestSpellIds;
        while (*spellidPtr)   // array ends with 0, so this works
        {
            if (*spellidPtr == m_spellInfo->getId())   // it is a spell for taming beast quest
            {
                isTamingQuestSpell = true;
                break;
            }
            ++spellidPtr;
        }
        // Don't call QuestMgr::OnPlayerCast for next-attack spells, either.  It will be called during the actual spell cast.
        if (!(hasAttribute(ATTRIBUTES_ON_NEXT_ATTACK) && !m_triggeredSpell) && !isTamingQuestSpell)
        {
            uint32 numTargets = 0;
            std::vector<uint64_t>::iterator itr = uniqueHittedTargets.begin();
            for (; itr != uniqueHittedTargets.end(); ++itr)
            {
                WoWGuid wowGuid;
                wowGuid.Init(*itr);
                if (wowGuid.isUnit())
                {
                    ++numTargets;
                    sQuestMgr.OnPlayerCast(p_caster, getSpellInfo()->getId(), *itr);
                }
            }
            if (numTargets == 0)
            {
                uint64 guid = p_caster->getTargetGuid();
                sQuestMgr.OnPlayerCast(p_caster, getSpellInfo()->getId(), guid);
            }
        }
    }

    if (p_caster != nullptr)
    {
        if (hadEffect || (cancastresult == SPELL_CAST_SUCCESS && !GetSpellFailed()))
            RemoveItems();
    }

    DecRef();
}

void Spell::writeSpellGoTargets(WorldPacket* data)
{
    std::vector<uint64_t>::iterator i;
    for (i = uniqueHittedTargets.begin(); i != uniqueHittedTargets.end(); ++i)
    {

#if VERSION_STRING <= TBC
        auto plr = p_caster;
        if (!plr && u_caster)
            plr = u_caster->m_redirectSpellPackets;

        if (plr && plr->isPlayer())
        {
            AscEmu::Packets::SmsgClearExtraAuraInfo aura_packet;
            aura_packet.guid = *i;
            aura_packet.spell_id = getSpellInfo()->getId();
            plr->SendPacket(aura_packet.serialise().get());
        }
#endif

        *data << *i;
    }
}

//\todo: Not called, should be send after targetting
void Spell::SendLogExecute(uint32 spellDamage, uint64 & targetGuid)
{
    WorldPacket data(SMSG_SPELLLOGEXECUTE, 37);
    data << m_caster->GetNewGUID();
    data << getSpellInfo()->getId();
    data << uint32(1);
    data << getSpellInfo()->getSpellVisual(0);
    data << uint32(1);
    if (m_caster->getGuid() != targetGuid)
        data << targetGuid;
    if (spellDamage)
        data << spellDamage;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendInterrupted(uint8 result)
{
    SetSpellFailed();

    if (m_caster == nullptr || !m_caster->IsInWorld())
        return;

    // send the failure to pet owner if we're a pet
    Player* plr = p_caster;
    if (plr == nullptr && m_caster->isPet())
    {
        static_cast<Pet*>(m_caster)->SendCastFailed(m_spellInfo->getId(), result);
    }
    else
    {
        if (plr == nullptr && u_caster != nullptr && u_caster->m_redirectSpellPackets != nullptr)
            plr = u_caster->m_redirectSpellPackets;

        if (plr != nullptr && plr->isPlayer())
            plr->GetSession()->SendPacket(SmsgSpellFailure(m_caster->GetNewGUID(), extra_cast_number, m_spellInfo->getId(), result).serialise().get());
    }

    m_caster->SendMessageToSet(SmsgSpellFailedOther(m_caster->GetNewGUID(), extra_cast_number, m_spellInfo->getId(), result).serialise().get(), false);
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
    if (m_spellInfo->getAttributesExC() & ATTRIBUTESEXC_IGNORE_RESURRECTION_TIMER)
        overrideTimer = true;

    target->GetSession()->SendPacket(SmsgResurrectRequest(m_caster->getGuid(), casterName, resurrectionSickness, overrideTimer, m_spellInfo->getId()).serialise().get());
    target->m_resurrecter = m_caster->getGuid();
}

void Spell::SendTameFailure(uint8 result)
{
    if (p_caster != nullptr)
    {
        WorldPacket data(SMSG_PET_TAME_FAILURE, 1);
        data << uint8(result);
        p_caster->GetSession()->SendPacket(&data);
    }
}

void Spell::HandleEffects(uint64 guid, uint32 i)
{
    if (event_GetInstanceID() == WORLD_INSTANCE ||
        DuelSpellNoMoreValid())
    {
        DecRef();
        return;
    }

    if (guid == 0)
    {
        unitTarget = nullptr;
        gameObjTarget = nullptr;
        playerTarget = nullptr;
        itemTarget = nullptr;

        if (p_caster != nullptr)
        {
            if (m_targets.getTargetMask() & TARGET_FLAG_ITEM)
                itemTarget = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
            if (m_targets.isTradeItem())
            {
                Player* p_trader = p_caster->getTradeTarget();
                if (p_trader != nullptr)
                    itemTarget = p_trader->getTradeData()->getTradeItem((TradeSlots)m_targets.getItemTarget());
            }
        }
    }
    else if (guid == m_caster->getGuid())
    {
        unitTarget = u_caster;
        gameObjTarget = g_caster;
        playerTarget = p_caster;
        itemTarget = i_caster;
    }
    else
    {
        if (!m_caster->IsInWorld())
        {
            unitTarget = nullptr;
            playerTarget = nullptr;
            itemTarget = nullptr;
            gameObjTarget = nullptr;
            corpseTarget = nullptr;
        }
        else if (m_targets.isTradeItem())
        {
            if (p_caster != nullptr)
            {
                Player* plr = p_caster->getTradeTarget();
                if (plr != nullptr)
                    itemTarget = plr->getTradeData()->getTradeItem((TradeSlots)guid);
            }
        }
        else
        {
            unitTarget = nullptr;
            gameObjTarget = nullptr;
            playerTarget = nullptr;
            itemTarget = nullptr;

            WoWGuid wowGuid;
            wowGuid.Init(guid);

            switch (wowGuid.getHigh())
            {
                case HighGuid::Unit:
                case HighGuid::Vehicle:
                    unitTarget = m_caster->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Pet:
                    unitTarget = m_caster->GetMapMgr()->GetPet(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Player:
                {
                    unitTarget = m_caster->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart());
                    playerTarget = static_cast<Player*>(unitTarget);
                }
                break;
                case HighGuid::Item:
                    if (p_caster != nullptr)
                        itemTarget = p_caster->getItemInterface()->GetItemByGUID(guid);

                    break;
                case HighGuid::GameObject:
                    gameObjTarget = m_caster->GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Corpse:
                    corpseTarget = sObjectMgr.GetCorpse(wowGuid.getGuidLowPart());
                    break;
                default:
                    LOG_ERROR("unitTarget not set");
                    DecRef();
                    return;
            }
        }
    }

    uint32 id = getSpellInfo()->getEffect(static_cast<uint8_t>(i));

    damage = CalculateEffect(i, unitTarget);

#ifdef GM_Z_DEBUG_DIRECTLY
    if (playerTarget && playerTarget->isPlayer() && playerTarget->IsInWorld())
    {
        if (playerTarget->GetSession() && playerTarget->GetSession()->CanUseCommand('z'))
            sChatHandler.BlueSystemMessage(playerTarget->GetSession(), "[%sSystem%s] |rSpellEffect::Handler: %s Target = %u, Effect id = %u, id = %u, Self: %u.", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE,
            playerTarget->getGuidLow(), m_spellInfo->Effect[i], i, guid);
    }
#endif

    uint32 TargetType = 0;
    TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetA(static_cast<uint8_t>(i)), i);

    //never get info from B if it is 0 :P
    if (m_spellInfo->getEffectImplicitTargetB(static_cast<uint8_t>(i)) != EFF_TARGET_NONE)
        TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetB(static_cast<uint8_t>(i)), i);

    if (u_caster != nullptr && unitTarget != nullptr && unitTarget->isCreature() && TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !(m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
    {
        unitTarget->GetAIInterface()->AttackReaction(u_caster, 1, 0);
        unitTarget->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, u_caster, 0);
    }


    if (id < TOTAL_SPELL_EFFECTS)
    {
        LogDebugFlag(LF_SPELL, "WORLD: Spell effect id = %u (%s), damage = %d", id, SpellEffectNames[id], damage);
        SpellScriptExecuteState scriptExecuteState = sScriptMgr.callScriptedSpellBeforeSpellEffect(this, id, static_cast<uint8_t>(i));

        if (scriptExecuteState != SpellScriptExecuteState::EXECUTE_PREVENT)
            (*this.*SpellEffectsHandler[id])(static_cast<uint8_t>(i));

        sScriptMgr.callScriptedSpellAfterSpellEffect(this, id, static_cast<uint8_t>(i));
    }
    else
        LOG_ERROR("SPELL: unknown effect %u spellid %u", id, getSpellInfo()->getId());

    DoAfterHandleEffect(unitTarget, i);
    DecRef();
}

void Spell::HandleAddAura(uint64 guid)
{
    Unit* Target = nullptr;

    std::map<uint64, Aura*>::iterator itr = m_pendingAuras.find(guid);

    if (itr == m_pendingAuras.end() || itr->second == nullptr)
    {
        DecRef();
        return;
    }

    //If this aura isn't added correctly it MUST be deleted
    Aura* aur = itr->second;
    itr->second = nullptr;

    if (event_GetInstanceID() == WORLD_INSTANCE)
    {
        DecRef();
        return;
    }

    if (u_caster && u_caster->getGuid() == guid)
        Target = u_caster;
    else if (m_caster->IsInWorld())
        Target = m_caster->GetMapMgr()->GetUnit(guid);

    if (Target == nullptr)
    {
        delete aur;
        DecRef();
        return;
    }

    // call script
    if (Target->isCreature())
    {
        auto creature = static_cast<Creature*>(Target);
        if (creature->GetScript())
        {
            if (m_caster->isCreatureOrPlayer())
                CALL_SCRIPT_EVENT(creature, OnHitBySpell)(getSpellInfo()->getId(), static_cast<Unit*>(m_caster));
        }
    }

    // Applying an aura to a flagged target will cause you to get flagged.
    // self casting doesn't flag himself.
    if (Target->isPlayer() && p_caster && p_caster != static_cast<Player*>(Target))
    {
        if (static_cast<Player*>(Target)->isPvpFlagSet())
        {
            if (p_caster->isPlayer() && !p_caster->isPvpFlagSet())
                static_cast<Player*>(p_caster)->PvPToggle();
            else
                p_caster->setPvpFlag();
        }
    }

    // remove any auras with same type
    if (getSpellInfo()->custom_BGR_one_buff_on_target > 0)
    {
        Target->RemoveAurasByBuffType(getSpellInfo()->custom_BGR_one_buff_on_target, m_caster->getGuid(), getSpellInfo()->getId());
    }

    uint32 spellid = 0;

    if ((getSpellInfo()->getMechanicsType() == MECHANIC_INVULNARABLE && getSpellInfo()->getId() != 25771) || getSpellInfo()->getId() == 31884)     // Cast spell Forbearance
    {
        if (getSpellInfo()->getId() != 31884)
            spellid = 25771;

        if (Target->isPlayer())
        {
            sEventMgr.AddEvent(static_cast<Player*>(Target), &Player::AvengingWrath, EVENT_PLAYER_AVENGING_WRATH, 30000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            static_cast<Player*>(Target)->mAvengingWrath = false;
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
                uint32 masterOfSubtlety[] =
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
                uint32 vindication[] =
                {
                    //SPELL_HASH_VINDICATION
                    67,
                    9452,
                    26016,
                    26017,
                    36002,
                    0
                };

                if (u_caster && u_caster->hasAurasWithId(vindication))
                    spellid = u_caster->getAuraWithId(vindication)->m_spellInfo->custom_RankNumber == 2 ? 26017 : 67;
            } break;
            case 5229:
            {
                uint32 kingOfTheJungle[] =
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
                        delete aur;
                        DecRef();
                        return;
                    }

                    Spell* spell = sSpellMgr.newSpell(p_caster, spellInfo, true, nullptr);


                    spell->forced_basepoints[0] = p_caster->getAuraWithId(kingOfTheJungle)->m_spellInfo->custom_RankNumber * 5;
                    SpellCastTargets targets(p_caster->getGuid());
                    spell->prepare(&targets);
                }
            } break;
            case 19574:
            {
                if (u_caster != nullptr)
                {
                    uint32 theBeastWithin[] =
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
                    uint32 rapidRecuperation[] =
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
            uint32 arcanePotency[] =
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
            if (Target->getAuraWithId(arcanePotency))
                spellid = Target->getAuraWithId(arcanePotency)->m_spellInfo->custom_RankNumber == 1 ? 57529 : 57531;
        }
        break;
    }

    if (spellid && Target)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(spellid);
        if (!spellInfo)
        {
            delete aur;
            DecRef();
            return;
        }

        Spell* spell = sSpellMgr.newSpell(u_caster, spellInfo, true, nullptr);

        uint32 masterOfSubtlety[] =
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
            spell->forced_basepoints[0] = Target->getAuraWithId(masterOfSubtlety)->m_spellInfo->getEffectBasePoints(0);

        SpellCastTargets targets(Target->getGuid());
        spell->prepare(&targets);
    }

    // avoid map corruption (this is impossible, btw)
    if (Target->GetInstanceID() != m_caster->GetInstanceID())
    {
        delete aur;
        DecRef();
        return;
    }

    int32 charges = m_charges;
    if (charges > 0)
    {
        if (u_caster != nullptr)
        {
            spellModFlatIntValue(u_caster->SM_FCharges, &charges, aur->GetSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(u_caster->SM_PCharges, &charges, aur->GetSpellInfo()->getSpellFamilyFlags());
        }
        for (int i = 0; i < (charges - 1); ++i)
        {
            Aura* staur = sSpellMgr.newAura(aur->GetSpellInfo(), aur->GetDuration(), aur->GetCaster(), aur->GetTarget(), m_triggeredSpell, i_caster);
            Target->AddAura(staur);
        }
        if (!(aur->GetSpellInfo()->getProcFlags() & PROC_REMOVEONUSE))
        {
            SpellCharge charge;
            charge.count = charges;
            charge.spellId = aur->GetSpellId();
            charge.ProcFlag = aur->GetSpellInfo()->getProcFlags();
            charge.lastproc = 0;
            Target->m_chargeSpells.insert(std::make_pair(aur->GetSpellId(), charge));
        }
    }

    Target->AddAura(aur); // the real spell is added last so the modifier is removed last

    DecRef();
}


/*
void Spell::TriggerSpell()
{
if (TriggerSpellId != 0)
{
// check for spell id
SpellEntry *spellInfo = sSpellStore.LookupEntry(TriggerSpellId);

if (!spellInfo)
{
LOG_ERROR("WORLD: unknown spell id %i\n", TriggerSpellId);
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

    auto skill_line_ability = sObjectMgr.GetSpellSkill(getSpellInfo()->getId());
    if (skill_line_ability == nullptr)
        return;

    float chance = 0.0f;

    if (p_caster->_HasSkillLine(skill_line_ability->skilline))
    {
        uint32 amt = p_caster->_GetSkillLineCurrent(skill_line_ability->skilline, false);
        uint32 max = p_caster->_GetSkillLineMax(skill_line_ability->skilline);
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
    if (Rand(chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
        p_caster->_AdvanceSkillLine(skill_line_ability->skilline, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
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
    m_spellInfo_override = sSpellMgr.getSpellInfo(m_spellInfo->getId());
}

uint32 Spell::GetDuration()
{
    if (bDurSet)
        return Dur;
    bDurSet = true;
    int32 c_dur = 0;

    if (getSpellInfo()->getDurationIndex())
    {
        auto spell_duration = sSpellDurationStore.LookupEntry(getSpellInfo()->getDurationIndex());
        if (spell_duration)
        {
            //check for negative and 0 durations.
            //duration affected by level
            if ((int32)spell_duration->Duration1 < 0 && spell_duration->Duration2 && u_caster)
            {
                this->Dur = uint32(((int32)spell_duration->Duration1 + (spell_duration->Duration2 * u_caster->getLevel())));
                if ((int32)this->Dur > 0 && spell_duration->Duration3 > 0 && (int32)this->Dur > (int32)spell_duration->Duration3)
                {
                    this->Dur = spell_duration->Duration3;
                }

                if ((int32)this->Dur < 0)
                    this->Dur = 0;
                c_dur = this->Dur;
            }
            if (!c_dur)
            {
                this->Dur = spell_duration->Duration1;
            }
            //combo point lolerCopter? ;P
            if (p_caster)
            {
                uint32 cp = p_caster->m_comboPoints;
                if (cp)
                {
                    uint32 bonus = (cp * (spell_duration->Duration3 - spell_duration->Duration1)) / 5;
                    if (bonus)
                    {
                        this->Dur += bonus;
                        m_requiresCP = true;
                    }
                }
            }

            if (u_caster != nullptr)
            {
                AscEmu::World::Spell::Helpers::spellModFlatIntValue(u_caster->SM_FDur, (int32*)&this->Dur, getSpellInfo()->getSpellFamilyFlags());
                AscEmu::World::Spell::Helpers::spellModPercentageIntValue(u_caster->SM_PDur, (int32*)&this->Dur, getSpellInfo()->getSpellFamilyFlags());
            }
        }
        else
        {
            this->Dur = (uint32)-1;
        }
    }
    else
    {
        this->Dur = (uint32)-1;
    }

    return this->Dur;
}

float Spell::GetRadius(uint32 i)
{
    if (bRadSet[i])
        return Rad[i];
    bRadSet[i] = true;
    Rad[i] = ::GetRadius(sSpellRadiusStore.LookupEntry(getSpellInfo()->getEffectRadiusIndex(static_cast<uint8_t>(i))));
    if (u_caster != nullptr)
    {
        AscEmu::World::Spell::Helpers::spellModFlatFloatValue(u_caster->SM_FRadius, &Rad[i], getSpellInfo()->getSpellFamilyFlags());
        AscEmu::World::Spell::Helpers::spellModPercentageFloatValue(u_caster->SM_PRadius, &Rad[i], getSpellInfo()->getSpellFamilyFlags());
    }

    return Rad[i];
}

uint32 Spell::GetBaseThreat(uint32 dmg)
{
    //there should be a formula to determine what spell cause threat and which don't
    return dmg;
}

uint32 Spell::GetMechanic(SpellInfo* sp)
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

uint8 Spell::CanCast(bool /*tolerate*/)
{
    /**
     * Object cast checks
     */
    if (m_caster && m_caster->IsInWorld())
    {
        Unit* target = m_caster->GetMapMgr()->GetUnit(m_targets.getUnitTarget());

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
            Creature* corpse = m_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 18240);
            if (corpse != nullptr)
                if (m_caster->CalcDistance(m_caster, corpse) > 5)
                    return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 39246)
        {
            Creature* cleft = m_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 22105);
            if (cleft == nullptr || cleft->isAlive())
                return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 30988)
        {
            Creature* corpse = m_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 17701);
            if (corpse != nullptr)
                if (m_caster->CalcDistance(m_caster, corpse) > 5 || corpse->isAlive())
                    return SPELL_FAILED_NOT_HERE;
        }
        else if (getSpellInfo()->getId() == 43723)
        {
            Creature* abysal = p_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), 19973);
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
            Creature* kilsorrow = p_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ());
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
        if (!p_caster->m_onTaxi)
        {
            if (m_spellInfo->getId() == 33836 || m_spellInfo->getId() == 45072 || m_spellInfo->getId() == 45115 || m_spellInfo->getId() == 31958)
                return SPELL_FAILED_NOT_HERE;
        }

        /**
         * Is mounted check
         */
        if (!p_caster->IsMounted())
        {
            if (getSpellInfo()->getId() == 25860) // Reindeer Transformation
                return SPELL_FAILED_ONLY_MOUNTED;
        }

         /**
          * check if spell is allowed while we have a battleground flag
          */
        if (p_caster->m_bgHasFlag)
        {
            switch (m_spellInfo->getId())
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
                    // thank Cruders for this :P
                    if (p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
                        p_caster->m_bg->HookOnFlagDrop(p_caster);
                    else if (p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
                        p_caster->m_bg->HookOnFlagDrop(p_caster);
                    break;
                }
            }
        }
    }

    /**
     * Targeted Unit Checks
     */
    if (m_targets.getUnitTarget())
    {
        Unit* target = (m_caster->IsInWorld()) ? m_caster->GetMapMgr()->GetUnit(m_targets.getUnitTarget()) : NULL;

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
                if (!target->HasAura(17743))
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

                uint32 facing_flags = getSpellInfo()->getFacingCasterFlags();

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
                        if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_DUMMY && !isFriendly(u_caster, target))
                            facing_flags = SPELL_INFRONT_STATUS_REQUIRE_INFRONT;
                    } break;
                }
            }

            // fishing spells
            if (getSpellInfo()->getEffectImplicitTargetA(0) == EFF_TARGET_SELF_FISHING)  //||
                //GetProto()->EffectImplicitTargetA[1] == EFF_TARGET_SELF_FISHING ||
                //GetProto()->EffectImplicitTargetA[2] == EFF_TARGET_SELF_FISHING)
            {
                uint32 entry = getSpellInfo()->getEffectMiscValue(0);
                if (entry == GO_FISHING_BOBBER)
                {
                    //uint32 mapid = p_caster->GetMapId();
                    float px = u_caster->GetPositionX();
                    float py = u_caster->GetPositionY();
                    float pz = u_caster->GetPositionZ();
                    float orient = m_caster->GetOrientation();
                    float posx = 0, posy = 0, posz = 0;
                    float co = cos(orient);
                    float si = sin(orient);
                    MapMgr* map = m_caster->GetMapMgr();

                    float r;
                    for (r = 20; r > 10; r--)
                    {
                        posx = px + r * co;
                        posy = py + r * si;
                        uint32 liquidtype;
                        map->GetLiquidInfo(posx, posy, pz + 2, posz, liquidtype);
                        if (!(liquidtype & 1))//water
                            continue;
                        if (!map->isInLineOfSight(px, py, pz + 0.5f, posx, posy, posz))
                            continue;
                        if (posz > map->GetLandHeight(posx, posy, pz + 2))
                            break;
                    }
                    if (r <= 10)
                        return SPELL_FAILED_NOT_FISHABLE;

                    // if we are already fishing, don't cast it again
                    if (p_caster->GetSummonedObject())
                        if (p_caster->GetSummonedObject()->getEntry() == GO_FISHING_BOBBER)
                            return SPELL_FAILED_SPELL_IN_PROGRESS;
                }
            }

            //check if we are trying to stealth or turn invisible but it is not allowed right now
            if (IsStealthSpell() || IsInvisibilitySpell())
            {
                uint32 faerieFireFeral[] =
                {
                    //SPELL_HASH_FAERIE_FIRE__FERAL_
                    16857,
                    60089,
                    0
                };

                //if we have Faerie Fire, we cannot stealth or turn invisible
                uint32 faerieFire[] =
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

bool Spell::hasAttribute(SpellAttributes attribute)
{
    return (getSpellInfo()->getAttributes() & attribute) != 0;
}

bool Spell::hasAttributeEx(SpellAttributesEx attribute)
{
    return (getSpellInfo()->getAttributesEx() & attribute) != 0;
}

bool Spell::hasAttributeExB(SpellAttributesExB attribute)
{
    return (getSpellInfo()->getAttributesExB() & attribute) != 0;
}

bool Spell::hasAttributeExC(SpellAttributesExC attribute)
{
    return (getSpellInfo()->getAttributesExC() & attribute) != 0;
}

bool Spell::hasAttributeExD(SpellAttributesExD attribute)
{
    return (getSpellInfo()->getAttributesExD() & attribute) != 0;
}

bool Spell::hasAttributeExE(SpellAttributesExE attribute)
{
    return (getSpellInfo()->getAttributesExE() & attribute) != 0;
}

bool Spell::hasAttributeExF(SpellAttributesExF attribute)
{
    return (getSpellInfo()->getAttributesExF() & attribute) != 0;
}

bool Spell::hasAttributeExG(SpellAttributesExG attribute)
{
    return (getSpellInfo()->getAttributesExG() & attribute) != 0;
}

void Spell::RemoveItems()
{
    // Item Charges & Used Item Removal
    if (i_caster)
    {
        // Stackable Item -> remove 1 from stack
        if (i_caster->getStackCount() > 1)
        {
            i_caster->modStackCount(-1);
            i_caster->m_isDirty = true;
            i_caster = nullptr;
        }
        else
        {
            for (uint8_t x = 0; x < 5; x++)
            {
                int32 charges = i_caster->getSpellCharges(x);

                if (charges == 0)
                    continue;

                bool Removable = false;

                // Items with negative charges are items that disappear when they reach 0 charge.
                if (charges < 0)
                    Removable = true;

                i_caster->m_isDirty = true;

                if (Removable)
                {

                    // If we have only 1 charge left, it's pointless to decrease the charge, we will have to remove the item anyways, so who cares ^^
                    if (charges == -1)
                    {
                        i_caster->getOwner()->getItemInterface()->SafeFullRemoveItemByGuid(i_caster->getGuid());
                    }
                    else
                    {
                        i_caster->modSpellCharges(x, 1);
                    }

                }
                else
                {
                    i_caster->modSpellCharges(x, -1);
                }

                i_caster = nullptr;
                break;
            }
        }
    }
}

int32 Spell::CalculateEffect(uint32 i, Unit* target)
{
    ///\todo Add ARMOR CHECKS; Add npc that have ranged weapons use them;

    // Range checks
    /*   if (GetProto()->getId() == SPELL_RANGED_GUN) // this includes bow and gun
    {
    if (!u_caster || !unitTarget)
    return 0;

    return ::CalculateDamage(u_caster, unitTarget, RANGED, GetProto()->SpellGroupType);
    }
    */
    int32 value = 0;

    /* Random suffix value calculation */
    if (i_caster && (int32(i_caster->getRandomPropertiesId()) < 0))
    {
        auto item_random_suffix = sItemRandomSuffixStore.LookupEntry(abs(int(i_caster->getRandomPropertiesId())));

        for (uint8 j = 0; j < 3; ++j)
        {
            if (item_random_suffix == nullptr)
                continue;

            if (item_random_suffix->enchantments[j] != 0)
            {
                auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(item_random_suffix->enchantments[j]);
                if (spell_item_enchant == nullptr)
                    continue;

                for (uint8 k = 0; k < 3; ++k)
                {
                    if (spell_item_enchant->spell[k] == getSpellInfo()->getId())
                    {
                        if (item_random_suffix->prefixes[k] == 0)
                            goto exit;

                        value = RANDOM_SUFFIX_MAGIC_CALCULATION(item_random_suffix->prefixes[j], i_caster->getPropertySeed());

                        if (value == 0)
                            goto exit;

                        return value;
                    }
                }
            }
        }
    }
exit:

    float basePointsPerLevel = getSpellInfo()->getEffectRealPointsPerLevel(static_cast<uint8_t>(i));
    int32 basePoints;
    if (m_overrideBasePoints)
        basePoints = m_overridenBasePoints[i];
    else
    {
#if VERSION_STRING >= Cata
        // In cata blizzard has corrected the base points in DBC files (no longer need to add 1)
        basePoints = getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i));
#else
        basePoints = getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i)) + 1;
#endif
    }
    int32 randomPoints = getSpellInfo()->getEffectDieSides(static_cast<uint8_t>(i));

    //added by Zack : some talents inherit their basepoints from the previously cast spell: see mage - Master of Elements
    if (forced_basepoints[i])
        basePoints = forced_basepoints[i];

    if (u_caster != nullptr)
    {
        int32 diff = -(int32)getSpellInfo()->getBaseLevel();
        if (getSpellInfo()->getMaxLevel() && u_caster->getLevel() > getSpellInfo()->getMaxLevel())
            diff += getSpellInfo()->getMaxLevel();
        else
            diff += u_caster->getLevel();
        //randomPoints += float2int32(diff * randomPointsPerLevel);
        basePoints += float2int32(diff * basePointsPerLevel);
    }

    if (randomPoints <= 1)
        value = basePoints;
    else
        value = basePoints + (int32)Util::getRandomUInt(randomPoints);

    int32 comboDamage = (int32)getSpellInfo()->getEffectPointsPerComboPoint(static_cast<uint8_t>(i));
    if (comboDamage && p_caster != nullptr)
    {
        m_requiresCP = true;
        value += (comboDamage * p_caster->m_comboPoints);
        //this is ugly so i will explain the case maybe someone ha a better idea :
        // while casting a spell talent will trigger upon the spell prepare faze
        // the effect of the talent is to add 1 combo point but when triggering spell finishes it will clear the extra combo point
        p_caster->m_spellcomboPoints = 0;
    }

    value = DoCalculateEffect(i, target, value);

    if (p_caster != nullptr)
    {
        SpellOverrideMap::iterator itr = p_caster->mSpellOverrideMap.find(getSpellInfo()->getId());
        if (itr != p_caster->mSpellOverrideMap.end())
        {
            ScriptOverrideList::iterator itrSO;
            for (itrSO = itr->second->begin(); itrSO != itr->second->end(); ++itrSO)
            {
                value += Util::getRandomUInt((*itrSO)->damage);
            }
        }
    }

    ///\todo INHERIT ITEM MODS FROM REAL ITEM OWNER - BURLEX BUT DO IT PROPERLY

    if (u_caster != nullptr)
    {
        int32 spell_flat_modifers = 0;
        int32 spell_pct_modifers = 100;

        spellModFlatIntValue(u_caster->SM_FMiscEffect, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(u_caster->SM_PMiscEffect, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());

        spellModFlatIntValue(u_caster->SM_FEffectBonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(u_caster->SM_PEffectBonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());

        spellModFlatIntValue(u_caster->SM_FDamageBonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(u_caster->SM_PDamageBonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());

        switch (i)
        {
            case 0:
                spellModFlatIntValue(u_caster->SM_FEffect1_Bonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
                spellModPercentageIntValue(u_caster->SM_PEffect1_Bonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());
                break;
            case 1:
                spellModFlatIntValue(u_caster->SM_FEffect2_Bonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
                spellModPercentageIntValue(u_caster->SM_PEffect2_Bonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());
                break;
            case 2:
                spellModFlatIntValue(u_caster->SM_FEffect3_Bonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
                spellModPercentageIntValue(u_caster->SM_PEffect3_Bonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());
                break;
        }

        value = float2int32(value * (float)(spell_pct_modifers / 100.0f)) + spell_flat_modifers;
    }
    else if (i_caster != nullptr && target != nullptr)
    {
        //we should inherit the modifiers from the conjured food caster
        Unit* item_creator = target->GetMapMgr()->GetUnit(i_caster->getCreatorGuid());

        if (item_creator != nullptr)
        {
            int32 spell_flat_modifers = 0;
            int32 spell_pct_modifers = 100;

            spellModFlatIntValue(item_creator->SM_FMiscEffect, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(item_creator->SM_PMiscEffect, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());

            spellModFlatIntValue(item_creator->SM_FEffectBonus, &spell_flat_modifers, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(item_creator->SM_PEffectBonus, &spell_pct_modifers, getSpellInfo()->getSpellFamilyFlags());

            value = float2int32(value * (float)(spell_pct_modifers / 100.0f)) + spell_flat_modifers;
        }
    }

    return value;
}

bool Spell::HasTarget(const uint64& guid, std::vector<uint64_t>* tmpMap)
{
    for (std::vector<uint64_t>::iterator itr = tmpMap->begin(); itr != tmpMap->end(); ++itr)
    {
        if (*itr == guid)
            return true;
    }

    for (auto target: missedTargets)
    {
        if (target.targetGuid == guid)
        {
            return true;
        }
    }

    return false;
}

int32 Spell::DoCalculateEffect(uint32 i, Unit* target, int32 value)
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
                        value += float2int32(getSpellInfo()->getEffectBasePoints(0) + weapondmg / (it->getItemProperties()->Delay / 1000.0f) * 2.8f);
                    }
                }
                if (target && target->IsDazed())
                    value += getSpellInfo()->getEffectBasePoints(1);
                value += (uint32)(u_caster->GetRAP() * 0.1);
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
                        int32 dmg = float2int32((avgwepdmg)+p_caster->GetAP() / 14 * wepspd);

                        if (target && target->getHealthPct() > 75)
                        {
                            dmg = float2int32(dmg + dmg * 0.35f);
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
                    value += float2int32((getSpellInfo()->getEffectBasePoints(0) + 1) + avgWeaponDmg);
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
                value += (uint32)(p_caster->GetAP() * 0.03f * p_caster->m_comboPoints);
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
                value += (uint32)((p_caster->GetAP() * 0.1526f) + (p_caster->getPower(POWER_TYPE_ENERGY) * getSpellInfo()->getEffectDamageMultiplier(static_cast<uint8_t>(i))));
                p_caster->setPower(POWER_TYPE_ENERGY, 0);
            }
        } break;

        // SPELL_HASH_VICTORY_RUSH:
        case 34428:
        {
            //causing ${$AP*$m1/100} damage
            if (u_caster != nullptr && i == 0)
                value = (value * u_caster->GetAP()) / 100;
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
                float ap = float(u_caster->GetAP());
                if (i == 0)
                    value += float2int32(ceilf(ap * 0.01f)); // / 100
                else if (i == 1)
                    value += float2int32(ap * 0.06f);
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
                value += (uint32)ceilf((u_caster->GetAP() * 0.07f) / 6);
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
                int8 cp = p_caster->m_comboPoints;
                value += (uint32)ceilf((u_caster->GetAP() * 0.04f * cp) / ((6 + (cp << 1)) >> 1));
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
                value += float2int32(p_caster->GetAP() * 0.01f * p_caster->m_comboPoints);
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
                value += u_caster->GetAP() / 5;
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
                value += float2int32(u_caster->GetAP() * 0.06f);
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
                value = float2int32((p_caster->getMinDamage() + p_caster->getMaxDamage()) / (float(p_caster->getBaseAttackTime(MELEE)) / 1000.0f)) << 1;
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

        // SPELL_HASH_VAMPIRIC_EMBRACE:
        case 15286:
        case 15290:
        case 71269:
        {
            value = value * (getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i)) + 1) / 100;
            if (p_caster != nullptr)
            {
                spellModFlatIntValue(p_caster->SM_FMiscEffect, &value, getSpellInfo()->getSpellFamilyFlags());
                spellModPercentageIntValue(p_caster->SM_PMiscEffect, &value, getSpellInfo()->getSpellFamilyFlags());
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
                    value = (p_caster->GetAP() * 22 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 44) * mit->getItemProperties()->Delay / 1000000;
            }
        } break;

        // SPELL_HASH_BLOOD_CORRUPTION:
        case 53742:
        // SPELL_HASH_HOLY_VENGEANCE:
        case 31803:
        {
            if (p_caster != nullptr)
                value = (p_caster->GetAP() * 25 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 13) / 1000;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_LIGHT:
        case 20185:
        case 20267:
        case 20271:
        case 28775:
        case 57774:
        {
            if (u_caster != nullptr)
                value = u_caster->getMaxHealth() * 2 / 100;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_WISDOM:
        case 20186:
        case 20268:
        case 53408:
        {
            if (u_caster != nullptr)
                value = u_caster->getBaseMana() * 2 / 100;
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
                value += (p_caster->GetAP() * 16 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 25) / 100;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS:
        case 20187:
        {
            if (p_caster != nullptr)
                value += (p_caster->GetAP() * 2 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 32) / 100;
        } break;

        // SPELL_HASH_JUDGEMENT_OF_VENGEANCE:
        case 31804:
        // SPELL_HASH_JUDGEMENT_OF_CORRUPTION:
        case 53733:
        {
            if (p_caster != nullptr)
                value += (p_caster->GetAP() * 14 + p_caster->getModDamageDonePositive(SCHOOL_HOLY) * 22) / 100;
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
                value *= p_caster->m_comboPoints;
                value += (uint32)(p_caster->GetAP() * (0.09f * p_caster->m_comboPoints));
                m_requiresCP = true;
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
                value += (uint32)ceilf(u_caster->GetAP() * 0.21f);
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
                    value = float2int32(value * 1.06f);
                break;
            }
            case 57669: //Replenishment
            case 61782:
            {
                if (p_caster != nullptr && i == 0 && target != nullptr)
                    value = int32(0.002 * target->getMaxPower(POWER_TYPE_MANA));
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
                            value += float2int32(u_caster->GetAP() * 0.03f);
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
                            value += float2int32(u_caster->GetAP() * 0.10f);
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
                            value += float2int32(u_caster->GetAP() * 0.04f);
                        break;
                }
            }
        }
    }

    return value;
}

void Spell::DoAfterHandleEffect(Unit* /*target*/, uint32 /*i*/)
{
}

void Spell::HandleTeleport(float x, float y, float z, uint32 mapid, Unit* Target)
{
    if (Target->isPlayer())
    {

        Player* pTarget = static_cast<Player*>(Target);
        pTarget->EventAttackStop();
        pTarget->SetSelection(0);

        // We use a teleport event on this one. Reason being because of UpdateCellActivity,
        // the game object set of the updater thread WILL Get messed up if we teleport from a gameobject
        // caster.

        if (!sEventMgr.HasEvent(pTarget, EVENT_PLAYER_TELEPORT))
        {
            sEventMgr.AddEvent(pTarget, &Player::EventTeleport, mapid, x, y, z, EVENT_PLAYER_TELEPORT, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

    }
    else
    {
        if (mapid != Target->GetMapId())
        {
            LOG_ERROR("Tried to teleport a Creature to another map.");
            return;
        }

        WorldPacket data(SMSG_MONSTER_MOVE, 50);

        data << Target->GetNewGUID();
        data << uint8(0);
        data << Target->GetPositionX();
        data << Target->GetPositionY();
        data << Target->GetPositionZ();
        data << Util::getMSTime();
        data << uint8(0x00);
        data << uint32(256);
        data << uint32(1);
        data << uint32(1);
        data << float(x);
        data << float(y);
        data << float(z);

        Target->SendMessageToSet(&data, true);
        Target->SetPosition(x, y, z, 0.5f);   // need correct orentation
    }
}

void Spell::CreateItem(uint32 itemId)
{
    /// Creates number of items equal to a "damage" of the effect
    if (itemId == 0 || p_caster == nullptr)
        return;

    p_caster->getItemInterface()->AddItemById(itemId, damage, 0);
}

void Spell::SendHealSpellOnPlayer(Object* caster, Object* target, uint32 healed, bool critical, uint32 overhealed, uint32 spellid, uint32 absorbed)
{
    if (caster == nullptr || target == nullptr || !target->isPlayer())
        return;

    caster->SendMessageToSet(SmsgSpellHealLog(target->GetNewGUID(), caster->GetNewGUID(), spellid, healed, overhealed, absorbed, critical).serialise().get(), true);
}

void Spell::Heal(int32 amount, bool ForceCrit)
{
    if (unitTarget == nullptr || !unitTarget->isAlive())
        return;

    if (p_caster != nullptr)
        p_caster->last_heal_spell = getSpellInfo();

    //self healing shouldn't flag himself
    if (p_caster != nullptr && playerTarget != nullptr && p_caster != playerTarget)
    {
        // Healing a flagged target will flag you.
        if (playerTarget->isPvpFlagSet())
        {
            if (!p_caster->isPvpFlagSet())
                p_caster->PvPToggle();
            else
                p_caster->setPvpFlag();
        }
    }

    //Make it critical
    bool critical = false;
    int32 critchance = 0;
    int32 bonus = 0;
    uint32 school = getSpellInfo()->getFirstSchoolFromSchoolMask();

    if (u_caster != nullptr && !(getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_NO_HEALING_BONUS))
    {
        //Basic bonus
        if (p_caster == nullptr ||
            !(p_caster->getClass() == ROGUE
            || p_caster->getClass() == WARRIOR
            || p_caster->getClass() == HUNTER
#if VERSION_STRING > TBC
            || p_caster->getClass() == DEATHKNIGHT
#endif
            ))
            bonus += u_caster->HealDoneMod[school];

        bonus += unitTarget->HealTakenMod[school];

        //Bonus from Intellect & Spirit
        if (p_caster != nullptr)
        {
            for (uint8 a = 0; a < STAT_COUNT; a++)
                bonus += float2int32(p_caster->SpellHealDoneByAttribute[a][school] * p_caster->getStat(a));
        }

        //Spell Coefficient, bonus to DH part
        if (getSpellInfo()->spell_coeff_direct > 0)
            bonus = float2int32(bonus * getSpellInfo()->spell_coeff_direct);

        critchance = float2int32(u_caster->spellcritperc + u_caster->SpellCritChanceSchool[school]);

        //Sacred Shield
        uint32 sacredShield[] =
        {
            //SPELL_HASH_SACRED_SHIELD
            53601,
            58597,
            0
        };

        if (unitTarget->hasAurasWithId(sacredShield))
        {
            switch (m_spellInfo->getId())
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
                    critchance += 50;
                    break;
                default:
                    break;
            }
        }


        int penalty_pct = 0;
        int penalty_flt = 0;
        spellModFlatIntValue(u_caster->SM_PPenalty, &penalty_pct, getSpellInfo()->getSpellFamilyFlags());
        bonus += amount * penalty_pct / 100;
        spellModFlatIntValue(u_caster->SM_FPenalty, &penalty_flt, getSpellInfo()->getSpellFamilyFlags());
        bonus += penalty_flt;
        spellModFlatIntValue(u_caster->SM_CriticalChance, &critchance, getSpellInfo()->getSpellFamilyFlags());


        if (p_caster != nullptr)
        {
            switch (m_spellInfo->getId())
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
                {
                    //Tidal Waves
                    p_caster->RemoveAura(53390, p_caster->getGuid());
                }
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
                {
                    //Maelstrom Weapon
                    p_caster->removeAllAurasByIdForGuid(53817, p_caster->getGuid());
                } break;
            }
        }

        switch (m_spellInfo->getId())
        {
            case 54172: //Paladin - Divine Storm heal effect
            {
                int dmg = (int)CalculateDamage(u_caster, unitTarget, MELEE, nullptr, sSpellMgr.getSpellInfo(53385));    //1 hit
                int target = 0;
                uint8 did_hit_result;

                for (const auto& itr : u_caster->getInRangeObjectsSet())
                {
                    if (itr)
                    {
                        auto obj = itr;
                        if (itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->isAlive() && obj->isInRange(u_caster, 8) && (u_caster->GetPhase() & itr->GetPhase()))
                        {
                            did_hit_result = DidHit(sSpellMgr.getSpellInfo(53385)->getEffect(0), static_cast<Unit*>(itr));
                            if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                                target++;
                        }
                    }
                }
                if (target > 4)
                    target = 4;

                amount = (dmg * target) >> 2;   // 25%
            }
            break;
        }

        amount += bonus;
        amount += amount * (int32)(u_caster->HealDonePctMod[school]);
        amount += float2int32(amount * unitTarget->HealTakenPctMod[school]);

        spellModPercentageIntValue(u_caster->SM_PDamageBonus, &amount, getSpellInfo()->getSpellFamilyFlags());

        if (ForceCrit || ((critical = Rand(critchance)) != 0))
        {
            int32 critical_bonus = 100;
            spellModFlatIntValue(u_caster->SM_PCriticalDamage, &critical_bonus, getSpellInfo()->getSpellFamilyFlags());

            if (critical_bonus > 0)
            {
                // the bonuses are halved by 50% (funky blizzard math :S)
                float b = (critical_bonus / 2.0f) / 100.0f;
                amount += float2int32(amount * b);
            }

            unitTarget->HandleProc(PROC_ON_SPELL_CRIT_HIT_VICTIM, u_caster, getSpellInfo(), false, amount);
            u_caster->HandleProc(PROC_ON_SPELL_CRIT_HIT, unitTarget, getSpellInfo(), false, amount);
        }
    }

    if (amount < 0)
        amount = 0;

    uint32 overheal = 0;
    uint32 curHealth = unitTarget->getHealth();
    uint32 maxHealth = unitTarget->getMaxHealth();
    if ((curHealth + amount) >= maxHealth)
    {
        unitTarget->setHealth(maxHealth);
        overheal = curHealth + amount - maxHealth;
    }
    else
        unitTarget->modHealth(amount);

    SendHealSpellOnPlayer(m_caster, unitTarget, amount, critical, overheal, pSpellId ? pSpellId : getSpellInfo()->getId());

    if (p_caster != nullptr)
    {
        p_caster->m_bgScore.HealingDone += amount;
        if (p_caster->m_bg != nullptr)
            p_caster->m_bg->UpdatePvPData();
    }

    if (p_caster != nullptr)
    {
        p_caster->m_casted_amount[school] = amount;
        p_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, unitTarget, getSpellInfo());
    }

    unitTarget->RemoveAurasByHeal();

    // add threat
    if (u_caster != nullptr)
    {
        std::vector<Unit*> target_threat;
        int count = 0;
        Creature* tmp_creature;
        for (const auto& itr : u_caster->getInRangeObjectsSet())
        {
            if (!itr || !itr->isCreature())
                continue;

            tmp_creature = static_cast<Creature*>(itr);

            if (!tmp_creature->CombatStatus.IsInCombat() || (tmp_creature->GetAIInterface()->getThreatByPtr(u_caster) == 0 && tmp_creature->GetAIInterface()->getThreatByPtr(unitTarget) == 0))
                continue;

            if (!(u_caster->GetPhase() & itr->GetPhase()))     //Can't see, can't be a threat
                continue;

            target_threat.push_back(tmp_creature);
            count++;
        }
        if (count == 0)
            return;

        amount = amount / count;

        for (std::vector<Unit*>::iterator itr = target_threat.begin(); itr != target_threat.end(); ++itr)
        {
            (*itr)->GetAIInterface()->HealReaction(u_caster, unitTarget, m_spellInfo, amount);
        }

        // remember that we healed (for combat status)
        if (unitTarget->IsInWorld() && u_caster->IsInWorld())
            u_caster->CombatStatus.WeHealed(unitTarget);
    }
}

uint32 Spell::GetType() { return (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_NONE ? SPELL_DMG_TYPE_MAGIC : getSpellInfo()->getDmgClass()); }

Item* Spell::GetItemTarget() const
{
    return itemTarget;
}

Unit* Spell::GetUnitTarget() const
{
    return unitTarget;
}

Player* Spell::GetPlayerTarget() const
{
    return playerTarget;
}

GameObject* Spell::GetGameObjectTarget() const
{
    return gameObjTarget;
}

Corpse* Spell::GetCorpseTarget() const
{
    return corpseTarget;
}

void Spell::DetermineSkillUp(uint32 skillid, uint32 targetlevel, uint32 multiplicator)
{
    if (p_caster == nullptr)
        return;

    if (p_caster->GetSkillUpChance(skillid) < 0.01)
        return;//to prevent getting higher skill than max

    int32 diff = p_caster->_GetSkillLineCurrent(skillid, false) / 5 - targetlevel;

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

    if (Rand((chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)) * multiplicator))
    {
        p_caster->_AdvanceSkillLine(skillid, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));

        uint32 value = p_caster->_GetSkillLineCurrent(skillid, true);
        uint32 spellid = 0;

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
            p_caster->addSpell(spellid);
    }
}

void Spell::DetermineSkillUp(uint32 skillid)
{
    //This code is wrong for creating items and disenchanting.
    if (p_caster == nullptr)
        return;

    float chance = 0.0f;

    auto skill_line_ability = sObjectMgr.GetSpellSkill(getSpellInfo()->getId());
    if (skill_line_ability != nullptr && skillid == skill_line_ability->skilline && p_caster->_HasSkillLine(skillid))
    {
        uint32 amt = p_caster->_GetSkillLineCurrent(skillid, false);
        uint32 max = p_caster->_GetSkillLineMax(skillid);
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
    if (Rand(chance * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
        p_caster->_AdvanceSkillLine(skillid, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
}

void Spell::SafeAddTarget(std::vector<uint64_t>* tgt, uint64 guid)
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

void Spell::SafeAddMissedTarget(uint64 guid)
{
    for (auto target: missedTargets)
    {
        if (target.targetGuid == guid)
        {
            //LOG_DEBUG("[SPELL] Something goes wrong in spell target system");
            // this isn't actually wrong, since we only have one missed target map,
            // whereas hit targets have multiple maps per effect.
            return;
        }
    }

    missedTargets.push_back(SpellTargetMod(guid, SPELL_DID_HIT_RESIST, SPELL_DID_HIT_SUCCESS));
}

void Spell::SafeAddModeratedTarget(uint64 guid, uint16 type)
{
    for (auto target: missedTargets)
    {
        if (target.targetGuid == guid)
        {
            //LOG_DEBUG("[SPELL] Something goes wrong in spell target system");
            // this isn't actually wrong, since we only have one missed target map,
            // whereas hit targets have multiple maps per effect.
            return;
        }
    }

    missedTargets.push_back(SpellTargetMod(guid, (SpellDidHitResult)type, SPELL_DID_HIT_SUCCESS));
}

uint32 Spell::getState() const
{
    return m_spellState;
}

void Spell::SetUnitTarget(Unit* punit)
{
    unitTarget = punit;
}

void Spell::SetTargetConstraintCreature(Creature* pCreature)
{
    targetConstraintCreature = pCreature;
}

void Spell::SetTargetConstraintGameObject(GameObject* pGameobject)
{
    targetConstraintGameObject = pGameobject;
}

Creature* Spell::GetTargetConstraintCreature() const
{
    return targetConstraintCreature;
}

GameObject* Spell::GetTargetConstraintGameObject() const
{
    return targetConstraintGameObject;
}

bool Spell::DuelSpellNoMoreValid() const
{
    if (duelSpell && (
        (p_caster != nullptr && p_caster->GetDuelState() != DUEL_STATE_STARTED) ||
        (u_caster != nullptr && u_caster->isPet() && static_cast<Pet*>(u_caster)->getPlayerOwner() && static_cast<Pet*>(u_caster)->getPlayerOwner()->GetDuelState() != DUEL_STATE_STARTED)))
        return true;
    else
        return false;
}

void Spell::safe_cancel()
{
    m_cancelled = true;
}

bool Spell::GetSpellFailed() const
{
    return m_Spell_Failed;
}

void Spell::SetSpellFailed(bool failed)
{
    m_Spell_Failed = failed;
}

uint32 Spell::GetTargetType(uint32 value, uint32 i)
{
    uint32 type = g_spellImplicitTargetFlags[value];

    //CHAIN SPELLS ALWAYS CHAIN!
    uint32 jumps = m_spellInfo->getEffectChainTarget(static_cast<uint8_t>(i));
    if (u_caster != nullptr)
        spellModFlatIntValue(u_caster->SM_FAdditionalTargets, (int32*)&jumps, m_spellInfo->getSpellFamilyFlags());
    if (jumps != 0)
        type |= SPELL_TARGET_AREA_CHAIN;

    return type;
}

void Spell::HandleCastEffects(uint64 guid, uint32 i)
{
    if (m_spellInfo->getSpeed() == 0)  //instant
    {
        AddRef();
        HandleEffects(guid, i);
    }
    else
    {
        float destx, desty, destz, dist = 0;

        if (m_targets.hasDestination())
        {
            auto destination = m_targets.getDestination();
            destx = destination.x;
            desty = destination.y;
            destz = destination.z;

            dist = m_caster->CalcDistance(destx, desty, destz);
        }
        else if (guid == 0)
        {
            return;
        }
        else
        {
            if (!m_caster->IsInWorld())
                return;

            if (m_caster->getGuid() != guid)
            {
                Object* obj = m_caster->GetMapMgr()->_GetObject(guid);
                if (obj == nullptr)
                    return;

                destx = obj->GetPositionX();
                desty = obj->GetPositionY();
                //\todo this should be destz = obj->GetPositionZ() + (obj->GetModelHighBoundZ() / 2 * obj->getScale())
                if (obj->isCreatureOrPlayer())
                    destz = obj->GetPositionZ() + static_cast<Unit*>(obj)->GetModelHalfSize();
                else
                    destz = obj->GetPositionZ();

                dist = m_caster->CalcDistance(destx, desty, destz);
            }
        }

        if (dist == 0.0f)
        {
            AddRef();
            HandleEffects(guid, i);
        }
        else
        {
            float time;

            if (m_missileTravelTime != 0)
                time = static_cast<float>(m_missileTravelTime);
            else
                time = dist * 1000.0f / m_spellInfo->getSpeed();

            ///\todo arcemu doesn't support reflected spells
            //if (reflected)
            //  time *= 1.25; //reflected projectiles move back 4x faster

            sEventMgr.AddEvent(this, &Spell::HandleEffects, guid, i, EVENT_SPELL_HIT, float2int32(time), 1, 0);
            AddRef();
        }
    }
}

void Spell::SpellEffectJumpTarget(uint8_t effectIndex)
{
    if (u_caster == nullptr)
        return;

    if (u_caster->getCurrentVehicle() || u_caster->isTrainingDummy())
        return;

    float x = 0;
    float y = 0;
    float z = 0;
    float o = 0;

    if (m_targets.getTargetMask() & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->GetMapMgr()->_GetObject(m_targets.getUnitTarget());

        if (uobj == nullptr || !uobj->isCreatureOrPlayer())
        {
            return;
        }

        float rad = unitTarget->getBoundingRadius() - u_caster->getBoundingRadius();

        float dx = m_caster->GetPositionX() - unitTarget->GetPositionX();
        float dy = m_caster->GetPositionY() - unitTarget->GetPositionY();

        if (dx == 0.0f || dy == 0.0f)
        {
            return;
        }

        float alpha = atanf(dy / dx);
        if (dx < 0)
        {
            alpha += M_PI_FLOAT;
        }

        x = rad * cosf(alpha) + unitTarget->GetPositionX();
        y = rad * sinf(alpha) + unitTarget->GetPositionY();
        z = unitTarget->GetPositionZ();
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

    if (m_spellInfo->getEffectMiscValue(effectIndex))
        speedZ = float(m_spellInfo->getEffectMiscValue(effectIndex)) / 10;
    else if (m_spellInfo->getEffectMiscValueB(effectIndex))
        speedZ = float(m_spellInfo->getEffectMiscValueB(effectIndex)) / 10;

    o = unitTarget->calcRadAngle(u_caster->GetPositionX(), u_caster->GetPositionY(), x, y);

    if (speedZ <= 0.0f)
        u_caster->GetAIInterface()->splineMoveJump(x, y, z, o, getSpellInfo()->getEffect(effectIndex) == 145);
    else
        u_caster->GetAIInterface()->splineMoveJump(x, y, z, o, speedZ, getSpellInfo()->getEffect(effectIndex) == 145);
}

void Spell::SpellEffectJumpBehindTarget(uint8_t /*i*/)
{
    if (u_caster == nullptr)
        return;
    if (m_targets.getTargetMask() & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->GetMapMgr()->_GetObject(m_targets.getUnitTarget());

        if (uobj == nullptr || !uobj->isCreatureOrPlayer())
            return;
        Unit* un = static_cast<Unit*>(uobj);
        float rad = un->getBoundingRadius() + u_caster->getBoundingRadius();
        float angle = float(un->GetOrientation() + M_PI); //behind
        float x = un->GetPositionX() + cosf(angle) * rad;
        float y = un->GetPositionY() + sinf(angle) * rad;
        float z = un->GetPositionZ();
        float o = un->calcRadAngle(x, y, un->GetPositionX(), un->GetPositionY());

        if (u_caster->GetAIInterface() != nullptr)
            u_caster->GetAIInterface()->splineMoveJump(x, y, z, o);
    }
    else if (m_targets.getTargetMask() & (TARGET_FLAG_SOURCE_LOCATION | TARGET_FLAG_DEST_LOCATION))
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

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

        if (x != 0.0f && y != 0.0f && z != 0.0f)
        {
            if (u_caster->GetAIInterface() != nullptr)
                u_caster->GetAIInterface()->splineMoveJump(x, y, z);
        }
        else
        {
            LogDebugFlag(LF_SPELL, "Coordinates are empty");
        }
    }
}

void Spell::HandleTargetNoObject()
{
    float dist = 3;
    float newx = m_caster->GetPositionX() + cosf(m_caster->GetOrientation()) * dist;
    float newy = m_caster->GetPositionY() + sinf(m_caster->GetOrientation()) * dist;
    float newz = m_caster->GetPositionZ();

    //clamp Z
    newz = m_caster->GetMapMgr()->GetLandHeight(newx, newy, newz);

    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
    bool isInLOS = mgr->isInLineOfSight(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ() + 2.0f, newx, newy, newz + 2.0f);
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
