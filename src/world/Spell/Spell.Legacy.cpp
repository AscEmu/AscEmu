/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Definitions/SpellGoFlags.h"
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


using ascemu::World::Spell::Helpers::decimalToMask;
using ascemu::World::Spell::Helpers::spellModFlatFloatValue;
using ascemu::World::Spell::Helpers::spellModPercentageFloatValue;
using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;

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
            if (u_caster->isPet() && static_cast<Pet*>(u_caster)->getPlayerOwner() != nullptr && dynamic_cast<Player*>(static_cast<Pet*>(u_caster)->getPlayerOwner())->GetDuelState() == DUEL_STATE_STARTED)
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

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
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
    m_CanRelect = false;
    m_IsReflected = false;
    hadEffect = false;
    bDurSet = false;
    bRadSet[0] = false;
    bRadSet[1] = false;
    bRadSet[2] = false;

    cancastresult = SPELL_CANCAST_OK;

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
    m_reflectedParent = nullptr;
    m_isCasting = false;
    m_glyphslot = 0;
    m_charges = info->getProcCharges();

    UniqueTargets.clear();
    ModeratedTargets.clear();
    for (uint8 i = 0; i < 3; ++i)
    {
        m_targetUnits[i].clear();
    }

    //create rune avail snapshot
    if (p_caster && p_caster->isClassDeathKnight())
        m_rune_avail_before = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();
    else
        m_rune_avail_before = 0;

    m_target_constraint = objmgr.GetSpellTargetConstraintForSpell(info->getId());

    m_missilePitch = 0;
    m_missileTravelTime = 0;
    m_IsCastedOnSelf = false;
    m_castTime = 0;
    m_timer = 0;
    m_magnetTarget = 0;
    Dur = 0;
}

Spell::~Spell()
{
#if VERSION_STRING == WotLK
    // If this spell deals with rune power, send spell_go to update client
    // For instance, when Dk cast Empower Rune Weapon, if we don't send spell_go, the client won't update
    if (getSpellInfo()->getSchool() && getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
        SendSpellGo();
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


    for (uint8 i = 0; i < 3; ++i)
    {
        m_targetUnits[i].clear();
    }

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
    std::vector<uint64_t>* tmpMap = &m_targetUnits[i];
    //IsStealth()
    float r = range * range;
    uint8 did_hit_result;

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
                    did_hit_result = DidHit(i, static_cast<Unit*>(itr));
                    if (did_hit_result != SPELL_DID_HIT_SUCCESS)
                        ModeratedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result));
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
    std::vector<uint64_t>* tmpMap = &m_targetUnits[i];
    float r = range * range;
    uint8 did_hit_result;

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
                        did_hit_result = DidHit(i, static_cast<Unit*>(itr));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            ModeratedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result));
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
    std::vector<uint64_t>* tmpMap = &m_targetUnits[i];
    float r = range * range;
    uint8 did_hit_result;

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
                        did_hit_result = DidHit(i, static_cast<Unit*>(itr));
                        if (did_hit_result == SPELL_DID_HIT_SUCCESS)
                            SafeAddTarget(tmpMap, itr->getGuid());
                        else
                            ModeratedTargets.push_back(SpellTargetMod(itr->getGuid(), did_hit_result));
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
    /* Check if the player target is able to deflect spells					*/
    /* Currently (3.3.5a) there is only spell doing that: Deterrence		*/
    /************************************************************************/
    if (p_victim && p_victim->HasAuraWithName(SPELL_AURA_DEFLECT_SPELLS))
    {
        return SPELL_DID_HIT_DEFLECT;
    }

    /************************************************************************/
    /* Check if the target is immune to this spell school                   */
    /* Unless the spell would actually dispel invulnerabilities             */
    /************************************************************************/
    int dispelMechanic = getSpellInfo()->getEffect(0) == SPELL_EFFECT_DISPEL_MECHANIC && getSpellInfo()->getEffectMiscValue(0) == MECHANIC_INVULNERABLE;
    if (u_victim->SchoolImmunityList[getSpellInfo()->getSchool()] && !dispelMechanic)
        return SPELL_DID_HIT_IMMUNE;

    /* Check if player target has god mode */
    if (p_victim && p_victim->m_cheats.GodModeCheat)
    {
        return SPELL_DID_HIT_IMMUNE;
    }

    /*************************************************************************/
    /* Check if the target is immune to this mechanic                        */
    /*************************************************************************/
    if (m_spellInfo->getMechanicsType() < MECHANIC_END && u_victim->MechanicsDispels[m_spellInfo->getMechanicsType()])

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
    if (m_spellInfo->getMechanicsType() < MECHANIC_END)
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
    if (getSpellInfo()->getSchool() == SCHOOL_NORMAL  && getSpellInfo()->getMechanicsType() == MECHANIC_NONE)
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
    if (getSpellInfo()->getMechanicsType() < MECHANIC_END)
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
    if (p_victim != nullptr && getSpellInfo()->custom_SchoolMask > 0)
    {
        int32 min = 100;
        for (uint8 i = 0; i < SCHOOL_COUNT; i++)
        {
            if (getSpellInfo()->custom_SchoolMask & (1 << i) && min > p_victim->m_resist_hit_spell[i])
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
            //			u_caster->HandleProc(PROC_ON_SPELL_LAND,target,GetProto());
        }

        return res;
    }
}
uint8 Spell::prepare(SpellCastTargets* targets)
{
    if (!m_caster->IsInWorld())
    {
        LogDebugFlag(LF_SPELL, "Object " I64FMT " is casting Spell ID %u while not in World", m_caster->getGuid(), getSpellInfo()->getId());
        DecRef();
        return SPELL_FAILED_DONT_REPORT;
    }

    uint8 ccr;

    // In case spell got cast from a script check fear/wander states
    if (!p_caster && u_caster && u_caster->GetAIInterface())
    {
        AIInterface* ai = u_caster->GetAIInterface();
        if (ai->isAiState(AI_STATE_FEAR) || ai->isAiState(AI_STATE_WANDER))
        {
            DecRef();
            return SPELL_FAILED_NOT_READY;
        }
    }

    chaindamage = 0;
    m_targets = *targets;

    if (!m_triggeredSpell && p_caster != nullptr && p_caster->m_cheats.CastTimeCheat)
        m_castTime = 0;
    else
    {
        m_castTime = GetCastTime(sSpellCastTimesStore.LookupEntry(getSpellInfo()->getCastingTimeIndex()));

        if (m_castTime && u_caster != nullptr)
        {
            spellModFlatIntValue(u_caster->SM_FCastTime, (int32*)&m_castTime, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(u_caster->SM_PCastTime, (int32*)&m_castTime, getSpellInfo()->getSpellFamilyFlags());
        }

        // handle MOD_CAST_TIME
        if (u_caster != nullptr && m_castTime)
        {
            m_castTime = float2int32(m_castTime * u_caster->getModCastSpeed());
        }
    }

    if (p_caster != nullptr)
    {
        // HookInterface events
        if (!sHookInterface.OnCastSpell(p_caster, getSpellInfo(), this))
        {
            DecRef();
            return SPELL_FAILED_UNKNOWN;
        }

        if (p_caster->cannibalize)
        {
            sEventMgr.RemoveEvents(p_caster, EVENT_CANNIBALIZE);
            p_caster->setEmoteState(EMOTE_ONESHOT_NONE);
            p_caster->cannibalize = false;
        }
    }

    //let us make sure cast_time is within decent range
    //this is a hax but there is no spell that has more then 10 minutes cast time

    if (m_castTime < 0)
        m_castTime = 0;
    else if (m_castTime > 60 * 10 * 1000)
        m_castTime = 60 * 10 * 1000; //we should limit cast time to 10 minutes right ?

    m_timer = m_castTime;

    m_magnetTarget = 0;

    //if (p_caster != NULL)
    //   m_castTime -= 100;	  // session update time


    m_spellState = SPELL_STATE_PREPARING;

    uint32_t parameter1 = 0, parameter2 = 0;
    if (objmgr.IsSpellDisabled(getSpellInfo()->getId()))//if it's disabled it will not be casted, even if it's triggered.
        cancastresult = m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_SPELL_UNAVAILABLE;
    else
        cancastresult = canCast(false, &parameter1, &parameter2);

    if (cancastresult != 0)
        LogDebugFlag(LF_SPELL, "CanCast result: %u. Refer to SpellFailure.h to work out why." , cancastresult);

    ccr = cancastresult;
    if (cancastresult != SPELL_CANCAST_OK)
    {
        // Triggered spells also need to go through cancast check but they do not pop a error message
        sendCastResult(m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : cancastresult, parameter1, parameter2);

        if (m_triggeredByAura)
        {
            SendChannelUpdate(0);
            if (u_caster != nullptr)
                u_caster->RemoveAura(m_triggeredByAura);
        }
        else
        {
            // HACK, real problem is the way spells are handled
            // when a spell is channeling and a new spell is cast
            // that is a channeling spell, but not triggered by a aura
            // the channel bar/spell is bugged
            if (u_caster && u_caster->getChannelObjectGuid() != 0 && u_caster->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
            {
                u_caster->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
                SendChannelUpdate(0);
                return ccr;
            }
        }
        finish(false);
        return ccr;
    }
    else
    {
        if (p_caster != nullptr && p_caster->isStealthed() && !hasAttributeEx(ATTRIBUTESEX_NOT_BREAK_STEALTH) && m_spellInfo->getId() != 1)   // <-- baaaad, baaad hackfix - for some reason some spells were triggering Spell ID #1 and stuffing up the spell system.
        {
            /* talents procing - don't remove stealth either */
            if (!hasAttribute(ATTRIBUTES_PASSIVE) &&
                !(pSpellId && sSpellMgr.getSpellInfo(pSpellId)->isPassive()))
            {
                p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
            }
        }

        SendSpellStart();

        // start cooldown handler
        if (p_caster != nullptr && !p_caster->m_cheats.CastTimeCheat && !m_triggeredSpell)
        {
            AddStartCooldown();
        }

        if (i_caster == nullptr)
        {
            if (p_caster != nullptr && m_timer > 0 && !m_triggeredSpell && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS))
                p_caster->resetAttackTimers();
            //p_caster->setAttackTimer(m_timer + 1000, false);
        }

        // TODO: for future reference, I removed aurastate removal here if spell had any aurastate set in SpellInfo::CasterAuraState
        // this is not handled anywhere yet -Appled
    }

    // If spell is triggered, skip straight to ::castMe
    if (m_triggeredSpell)
    {
        castMe(false);
    }
    else
    {
        // Spells with this attributes are considered as triggered spells, so don't set as current spell
        if (!(getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_TRIGGERED))
        {
            m_caster->setCurrentSpell(this);
        }

        // Set casting position
        m_castPositionX = m_caster->GetPositionX();
        m_castPositionY = m_caster->GetPositionY();
        m_castPositionZ = m_caster->GetPositionZ();

        // Handle instant and non-channeled spells instantly. Other spells will be handled in ::update on next tick.
        // First autorepeat casts are actually never casted, only set as current spell. Player::updateAutoRepeatSpell handles the shooting.
        if (m_castTime == 0 && getSpellInfo()->getChannelInterruptFlags() == 0 && !getSpellInfo()->isRangedAutoRepeat())
        {
            castMe(false);
        }
    }

    return ccr;
}

void Spell::cancel()
{
    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    SendInterrupted(0);
    sendCastResult(SPELL_FAILED_INTERRUPTED);

    if (m_spellState == SPELL_STATE_CASTING)
    {
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
                //				p_caster->setAttackTimer(1000, false);
            }
        }
    }

    SendChannelUpdate(0);

    //m_spellState = SPELL_STATE_FINISHED;

    // prevent memory corruption. free it up later.
    // if this is true it means we are currently in the cast() function somewhere else down the stack
    // (recursive spells) and we don't wanna have this class deleted when we return to it.
    // at the end of cast() it will get freed anyway.
    if (!m_isCasting)
        finish(false);
}

void Spell::AddCooldown()
{
    if (p_caster != nullptr)
        p_caster->Cooldown_Add(getSpellInfo(), i_caster);
}

void Spell::AddStartCooldown()
{
    if (p_caster != nullptr)
        p_caster->Cooldown_AddStart(getSpellInfo());
}

void Spell::castMe(bool check)
{
    if (DuelSpellNoMoreValid())
    {
        // Can't cast that!
        SendInterrupted(SPELL_FAILED_TARGET_FRIENDLY);
        finish(false);
        return;
    }

    if (m_caster->isPlayer())
    {
        Player* player = static_cast<Player*>(m_caster);
        LogDebugFlag(LF_SPELL, "Spell::cast Id %u (%s), Players: %s (guid: %u)",
                      getSpellInfo()->getId(), getSpellInfo()->getName().c_str(), player->getName().c_str(), player->getPlayerInfo()->guid);
    }
    else if (m_caster->isCreature())
    {
        Creature* creature = static_cast<Creature*>(m_caster);
        LogDebugFlag(LF_SPELL, "Spell::cast Id %u (%s), Creature: %s (spawn id: %u | entry: %u)",
                      getSpellInfo()->getId(), getSpellInfo()->getName().c_str(), creature->GetCreatureProperties()->Name.c_str(), creature->spawnid, creature->getEntry());
    }
    else
    {
        LogDebugFlag(LF_SPELL, "Spell::cast %u, LowGuid: %u", getSpellInfo()->getId(), m_caster->getGuidLow());
    }

    uint32_t parameter1 = 0, parameter2 = 0;
    if (objmgr.IsSpellDisabled(getSpellInfo()->getId()))//if it's disabled it will not be casted, even if it's triggered.
        cancastresult = m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_SPELL_UNAVAILABLE;
    else if (check)
        cancastresult = canCast(true, &parameter1, &parameter2);
    else
        cancastresult = SPELL_CANCAST_OK;

    if (cancastresult == SPELL_CANCAST_OK)
    {
        if (hasAttribute(ATTRIBUTES_ON_NEXT_ATTACK))
        {
            if (!m_triggeredSpell)
            {
                // on next attack - we don't take the mana till it actually attacks.
                if (!HasPower())
                {
                    SendInterrupted(SPELL_FAILED_NO_POWER);
                    sendCastResult(SPELL_FAILED_NO_POWER);
                    finish(false);
                    return;
                }
            }
            else
            {
                // this is the actual spell cast
                if (!TakePower())   // shouldn't happen
                {
                    SendInterrupted(SPELL_FAILED_NO_POWER);
                    sendCastResult(SPELL_FAILED_NO_POWER);
                    finish(false);
                    return;
                }
            }
        }
        else
        {
            if (!m_triggeredSpell)
            {
                if (!TakePower()) //not enough mana
                {
                    //LOG_DEBUG("Spell::Not Enough Mana");
                    SendInterrupted(SPELL_FAILED_NO_POWER);
                    sendCastResult(SPELL_FAILED_NO_POWER);
                    finish(false);
                    return;
                }
            }
        }

        for (uint8 i = 0; i < 3; i++)
        {
            uint32 TargetType = 0;
            TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetA(i), i);

            //never get info from B if it is 0 :P
            if (m_spellInfo->getEffectImplicitTargetB(i) != EFF_TARGET_NONE)
                TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetB(i), i);

            if (TargetType & SPELL_TARGET_AREA_CURTARGET)
            {
                //this just forces dest as the targets location :P
                Object* target = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

                if (target != nullptr)
                {
                    m_targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
                    m_targets.setDestination(target->GetPosition());
                }
            }

            if (getSpellInfo()->getEffect(i) && getSpellInfo()->getEffect(i) != SPELL_EFFECT_PERSISTENT_AREA_AURA)
                FillTargetMap(i);
        }

        if (m_magnetTarget)
        {
            // Spell was redirected
            // Grounding Totem gets destroyed after redirecting 1 spell
            Unit* MagnetTarget = m_caster->GetMapMgr()->GetUnit(m_magnetTarget);
            m_magnetTarget = 0;
            if (MagnetTarget && MagnetTarget->isCreature())
            {
                Creature* MagnetCreature = static_cast<Creature*>(MagnetTarget);
                if (MagnetCreature->isTotem())
                    MagnetCreature->Despawn(1, 0);
            }
        }

        sendCastResult(cancastresult);
        if (cancastresult != SPELL_CANCAST_OK)
        {
            finish(false);
            return;
        }

        m_isCasting = true;

        LogDebugFlag(LF_SPELL, "CanCastResult: %u" , cancastresult);
        if (!m_triggeredSpell)
            AddCooldown();

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
                    case 1784:		// Stealth rank 1
                    case 1785:		// Stealth rank 2
                    case 1786:		// Stealth rank 3
                    case 1787:		// Stealth rank 4
                    case 5215:		// Prowl rank 1
                    case 6783:		// Prowl rank 2
                    case 9913:		// Prowl rank 3
                    case 498:		// Divine protection
                    case 5573:		// Unknown spell
                    case 642:		// Divine shield
                    case 1020:		// Unknown spell
                    case 1022:		// Hand of Protection rank 1 (ex blessing of protection)
                    case 5599:		// Hand of Protection rank 2 (ex blessing of protection)
                    case 10278:		// Hand of Protection rank 3 (ex blessing of protection)
                    case 1856:		// Vanish rank 1
                    case 1857:		// Vanish rank 2
                    case 26889:		// Vanish rank 3
                    case 45438:		// Ice block
                    case 20580:		// Unknown spell
                    case 58984:		// Shadowmeld
                    case 17624:		// Petrification-> http://www.wowhead.com/?spell=17624
                    case 66:		// Invisibility
                        if (p_caster->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
                        {
                            if (p_caster->getTeam() == 0)
                                p_caster->RemoveAura(23333);	// ally player drop horde flag if they have it
                            else
                                p_caster->RemoveAura(23335); 	// horde player drop ally flag if they have it
                        }
                        if (p_caster->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)

                            p_caster->RemoveAura(34976);	// drop the flag
                        break;
                }
            }
        }

        /*SpellExtraInfo* sp = objmgr.GetSpellExtraData(GetProto()->getId());
        if (sp)
        {
        Unit* Target = objmgr.GetUnit(m_targets.m_unitTarget);
        if (Target)
        Target->RemoveBySpecialType(sp->specialtype, p_caster->getGuid());
        }*/

        if (!(hasAttribute(ATTRIBUTES_ON_NEXT_ATTACK) && !m_triggeredSpell))  //on next attack
        {
            SendSpellGo();

            //******************** SHOOT SPELLS ***********************
            //* Flags are now 1,4,19,22 (4718610) //0x480012

            if (hasAttributeExC(ATTRIBUTESEXC_PLAYER_RANGED_SPELLS) && m_caster->isPlayer() && m_caster->IsInWorld())
            {
                // Part of this function contains a hack fix
                // hack fix for shoot spells, should be some other resource for it
                // p_caster->SendSpellCoolDown(GetProto()->getId(), GetProto()->RecoveryTime ? GetProto()->RecoveryTime : 2300);
                WorldPacket data(SMSG_SPELL_COOLDOWN, 14);
                data << p_caster->GetNewGUID();
                data << uint8(0); //unk, some flags
                data << getSpellInfo()->getId();
                data << uint32(getSpellInfo()->getRecoveryTime() ? getSpellInfo()->getRecoveryTime() : 2300);
                p_caster->GetSession()->SendPacket(&data);
            }
            else
            {
                if (getSpellInfo()->getChannelInterruptFlags() != 0 && !m_triggeredSpell)
                {
                    /*
                    Channeled spells are handled a little differently. The five second rule starts when the spell's channeling starts; i.e. when you pay the mana for it.
                    The rule continues for at least five seconds, and longer if the spell is channeled for more than five seconds. For example,
                    Mind Flay channels for 3 seconds and interrupts your regeneration for 5 seconds, while Tranquility channels for 10 seconds
                    and interrupts your regeneration for the full 10 seconds.
                    */

                    uint32 channelDuration = GetDuration();
                    if (u_caster != nullptr)
                        channelDuration = static_cast<uint32>(channelDuration * u_caster->getModCastSpeed());
                    m_spellState = SPELL_STATE_CASTING;
                    SendChannelStart(channelDuration);
                    if (p_caster != nullptr)
                    {
                        //Use channel interrupt flags here
                        if (m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION || m_targets.m_targetMask == TARGET_FLAG_SOURCE_LOCATION)
                            u_caster->setChannelObjectGuid(p_caster->GetSelection());
                        else if (p_caster->GetSelection() == m_caster->getGuid())
                        {
                            if (p_caster->GetSummon())
                                u_caster->setChannelObjectGuid(p_caster->GetSummon()->getGuid());
                            else if (m_targets.m_unitTarget)
                                u_caster->setChannelObjectGuid(m_targets.m_unitTarget);
                            else
                                u_caster->setChannelObjectGuid(p_caster->GetSelection());
                        }
                        else
                        {
                            if (p_caster->GetSelection())
                                u_caster->setChannelObjectGuid(p_caster->GetSelection());
                            else if (p_caster->GetSummon())
                                u_caster->setChannelObjectGuid(p_caster->GetSummon()->getGuid());
                            else if (m_targets.m_unitTarget)
                                u_caster->setChannelObjectGuid(m_targets.m_unitTarget);
                            else
                            {
                                m_isCasting = false;
                                cancel();
                                return;
                            }
                        }
                    }
                    if (u_caster && u_caster->getPowerType() == POWER_TYPE_MANA)
                    {
                        if (channelDuration <= 5000)
                            u_caster->DelayPowerRegeneration(5000);
                        else
                            u_caster->DelayPowerRegeneration(channelDuration);
                    }
                }
            }

            std::vector<uint64>::iterator i, i2;
            // this is here to avoid double search in the unique list
            // bool canreflect = false, reflected = false;
            for (uint8 j = 0; j < 3; j++)
            {
                switch (getSpellInfo()->getEffectImplicitTargetA(j))
                {
                    case EFF_TARGET_SINGLE_ENEMY:
                    case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
                    case EFF_TARGET_IN_FRONT_OF_CASTER:
                    case EFF_TARGET_DUEL:
                        SetCanReflect();
                        break;
                }
                if (GetCanReflect())
                    continue;
                else
                    break;
            }

            if (!IsReflected() && GetCanReflect() && m_caster->IsInWorld())
            {
                for (i = UniqueTargets.begin(); i != UniqueTargets.end(); ++i)
                {
                    Unit* Target = m_caster->GetMapMgr()->GetUnit(*i);
                    if (Target)
                    {
                        SetReflected(Reflect(Target));
                    }

                    // if the spell is reflected
                    if (IsReflected())
                        break;
                }
            }
            else
                SetReflected(false);

            bool isDuelEffect = false;
            //uint32 spellid = GetProto()->getId();

            // we're much better to remove this here, because otherwise spells that change powers etc,
            // don't get applied.
            if (u_caster && !m_triggeredSpell && !m_triggeredByAura && !(m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
            {
                u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
                u_caster->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_CAST);
            }

            // if the spell is not reflected
            if (!IsReflected())
            {
                for (uint8_t x = 0; x < 3; x++)
                {
                    // check if we actually have a effect
                    if (getSpellInfo()->getEffect(x))
                    {
                        isDuelEffect = isDuelEffect || getSpellInfo()->getEffect(x) == SPELL_EFFECT_DUEL;

                        if (m_targetUnits[x].size() > 0)
                        {
                            for (i = m_targetUnits[x].begin(); i != m_targetUnits[x].end();)
                            {
                                i2 = i++;
                                HandleCastEffects(*i2, x);
                            }
                        }
                        else
                            HandleCastEffects(0, x);
                    }
                }

                for (auto target: ModeratedTargets)
                {
                    HandleModeratedTarget(target.targetGuid);
                }

                // Evairfairy's test code for TBC
#if VERSION_STRING == TBC
                auto apply_aura = false;
                for (auto aura_idx = 0; aura_idx < 3; ++aura_idx)
                    apply_aura = apply_aura || getSpellInfo()->getEffectApplyAuraName(0) != 0;
                if (apply_aura)
                {
                    hadEffect = true;
                    for (auto target : UniqueTargets)
                    {
                        // Appled note: HandleAddAura(target) is already called from Spell::SpellEffectApplyAura(i)
                        // but with damage calculations for the aura effect, cast location, spell particle travel time (this solution applies aura instantly on spell cast, rather than when the spell particle hits target) etc
                        HandleAddAura(target);
                    }
                }
#endif

                // spells that proc on spell cast, some talents
                if (p_caster && p_caster->IsInWorld())
                {
                    for (i = UniqueTargets.begin(); i != UniqueTargets.end(); ++i)
                    {
                        Unit* Target = p_caster->GetMapMgr()->GetUnit(*i);

                        if (!Target)
                            continue; //we already made this check, so why make it again ?

                        p_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, Target, getSpellInfo(), m_triggeredSpell);
                        Target->HandleProc(PROC_ON_SPELL_LAND_VICTIM, u_caster, getSpellInfo(), m_triggeredSpell);
                        p_caster->m_procCounter = 0; //this is required for to be able to count the depth of procs (though i have no idea where/why we use proc on proc)

                        // This is wrong but leaving this here commented out for now -Appled
                        //Target->removeAuraStateAndAuras(AuraState(getSpellInfo()->getTargetAuraState()));
                    }
                }
            }

            m_isCasting = false;

            if (m_spellState != SPELL_STATE_CASTING)
            {
                finish();
                return;
            }
        }
        else //this shit has nothing to do with instant, this only means it will be on NEXT melee hit
        {
            // we're much better to remove this here, because otherwise spells that change powers etc,
            // don't get applied.

            if (u_caster && !m_triggeredSpell && !m_triggeredByAura && !(m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
            {
                u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
                u_caster->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_CAST);
            }

            //not sure if it must be there...
            /*if (p_caster != NULL)
            {
            if (p_caster->m_onAutoShot)
            {
            p_caster->GetSession()->OutPacket(SMSG_CANCEL_AUTO_REPEAT);
            p_caster->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
            p_caster->m_onAutoShot = false;
            }
            }*/

            m_isCasting = false;
            sendCastResult(cancastresult);
            if (u_caster != nullptr)
                u_caster->SetOnMeleeSpell(getSpellInfo()->getId(), extra_cast_number);

            finish();

            return;
        }

        //if (u_caster != NULL)
        //	u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, GetProto()->getId());
    }
    else
    {
        // cancast failed
        sendCastResult(cancastresult, parameter1, parameter2);
        SendInterrupted(cancastresult);
        finish(false);
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

            WorldPacket data(SMSG_SPELL_DELAYED, 13);
            data << u_caster->GetNewGUID();
            data << uint32(delay);
            u_caster->SendMessageToSet(&data, true);

            if (p_caster == nullptr)
            {
                //then it's a Creature
                u_caster->GetAIInterface()->AddStopTime(delay);
            }
            //in case cast is delayed, make sure we do not exit combat
            else
            {
                //				sEventMgr.ModifyEventTimeLeft(p_caster,EVENT_ATTACK_TIMEOUT,attackTimeoutInterval,true);
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
                SendChannelUpdate(m_timer);

        }
    }
}

void Spell::Update(unsigned long time_passed)
{
    // skip cast if we're more than 2/3 of the way through
    ///\todo determine which spells can be cast while moving.
    // Client knows this, so it should be easy once we find the flag.
    // XD, it's already there!
    if ((getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_MOVEMENT) &&
        ((m_castTime / 1.5f) > m_timer) &&
        //		float(m_castTime)/float(m_timer) >= 2.0f		&&
        (
        m_castPositionX != m_caster->GetPositionX() ||
        m_castPositionY != m_caster->GetPositionY() ||
        m_castPositionZ != m_caster->GetPositionZ()
        )
        )
    {
        if (u_caster != nullptr)
        {
            if (u_caster->HasNoInterrupt() == 0 && getSpellInfo()->getEffectMechanic(1) != MECHANIC_INCAPACIPATED)
            {
                cancel();
                return;
            }
        }
    }

    if (m_cancelled)
    {
        cancel();
        return;
    }

    switch (m_spellState)
    {
        case SPELL_STATE_PREPARING:
        {
            if (m_timer > 0)
            {
                if (static_cast<int32_t>(time_passed) >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= time_passed;
            }

            if (m_timer == 0 && !getSpellInfo()->isOnNextMeleeAttack() && !getSpellInfo()->isRangedAutoRepeat())
            {
                // Skip checks for instant spells
                castMe(m_castTime == 0);
            }
        }
        break;
        case SPELL_STATE_CASTING:
        {
            if (m_timer > 0)
            {
                if (static_cast<int32>(time_passed) >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= time_passed;
            }
            if (m_timer <= 0)
            {
                SendChannelUpdate(0);
                finish();
            }
        }
        break;
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
        // mana           channeled                                                     power type is mana                             if spell wasn't cast successfully, don't delay mana regeneration
        if (m_usesMana && (getSpellInfo()->getChannelInterruptFlags() == 0 && !m_triggeredSpell) && u_caster->getPowerType() == POWER_TYPE_MANA && successful)
        {
            /*
            Five Second Rule
            After a character expends mana in casting a spell, the effective amount of mana gained per tick from spirit-based regeneration becomes a ratio of the normal
            listed above, for a period of 5 seconds. During this period mana regeneration is said to be interrupted. This is commonly referred to as the five second rule.
            By default, your interrupted mana regeneration ratio is 0%, meaning that spirit-based mana regeneration is suspended for 5 seconds after casting.
            Several effects can increase this ratio, including:
            */

            u_caster->DelayPowerRegeneration(5000);
        }
    }
    /* Mana Regenerates while in combat but not for 5 seconds after each spell */
    /* Only if the spell uses mana, will it cause a regen delay.
       is this correct? is there any spell that doesn't use mana that does cause a delay?
       this is for creatures as effects like chill (when they have frost armor on) prevents regening of mana	*/

       //moved to spellhandler.cpp -> remove item when click on it! not when it finishes

       //enable pvp when attacking another player with spells
    if (p_caster != nullptr)
    {
        if (hasAttribute(ATTRIBUTES_STOP_ATTACK) && p_caster->IsAttacking())
        {
            p_caster->EventAttackStop();
            p_caster->smsg_AttackStop(p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection()));
            p_caster->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
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
    if (i_caster && i_caster->getOwner() && cancastresult == SPELL_CANCAST_OK && !GetSpellFailed())
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
            std::vector<uint64_t>::iterator itr = UniqueTargets.begin();
            for (; itr != UniqueTargets.end(); ++itr)
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
        if (hadEffect || (cancastresult == SPELL_CANCAST_OK && !GetSpellFailed()))
            RemoveItems();
    }

    DecRef();
}

// uint16 0xFFFF
enum SpellStartFlags
{
    //0x01
    SPELL_START_FLAG_DEFAULT = 0x02, // atm set as default flag
    //0x04
    //0x08
    //0x10
    SPELL_START_FLAG_RANGED = 0x20,
    //0x40
    //0x80
    //0x100
    //0x200
    //0x400
    //0x800
    //0x1000
    //0x2000
    //0x4000
    //0x8000
};

void Spell::SendSpellStart()
{
    // no need to send this on passive spells
    if (!m_caster->IsInWorld() || hasAttribute(ATTRIBUTES_PASSIVE) || m_triggeredSpell)
        return;

    WorldPacket data(150);

#if VERSION_STRING >= WotLK
    uint32 cast_flags = 2;
#else
    uint16_t cast_flags = 2;
#endif

    if (GetType() == SPELL_DMG_TYPE_RANGED)
        cast_flags |= 0x20;

    // hacky yeaaaa
    if (getSpellInfo()->getId() == 8326)   // death
        cast_flags = 0x0F;

    data.SetOpcode(SMSG_SPELL_START);
    if (i_caster != NULL)
    {
        data << i_caster->GetNewGUID();
        data << u_caster->GetNewGUID();
    }
    else
    {
        data << m_caster->GetNewGUID();
        data << m_caster->GetNewGUID();
    }

#if VERSION_STRING > TBC
    data << extra_cast_number;
    data << getSpellInfo()->getId();
#else
    data << getSpellInfo()->getId();
    data << extra_cast_number;
#endif
    data << cast_flags;
#if VERSION_STRING >= Cata
    data << uint32(m_timer);
#endif
    data << (uint32)m_castTime;

    m_targets.write(data);

    if (GetType() == SPELL_DMG_TYPE_RANGED)
    {
        ItemProperties const* ip = nullptr;
        if (getSpellInfo()->getId() == SPELL_RANGED_THROW)   // throw
        {
            if (p_caster != NULL)
            {
                auto item = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (item != nullptr)
                {
                    ip = item->getItemProperties();
                    /* Throwing Weapon Patch by Supalosa
                    p_caster->getItemInterface()->RemoveItemAmt(it->getEntry(),1);
                    (Supalosa: Instead of removing one from the stack, remove one from durability)
                    We don't need to check if the durability is 0, because you can't cast the Throw spell if the thrown weapon is broken, because it returns "Requires Throwing Weapon" or something.
                    */

                    // burlex - added a check here anyway (wpe suckers :P)
                    if (item->getDurability() > 0)
                    {
                        item->setDurability(item->getDurability() - 1);
                        if (item->getDurability() == 0)
                            p_caster->ApplyItemMods(item, EQUIPMENT_SLOT_RANGED, false, true);
                    }
                }
                else
                {
                    ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
                }
            }
        }
#if VERSION_STRING < Cata
        else if (hasAttributeExC(ATTRIBUTESEXC_PLAYER_RANGED_SPELLS))
        {
            if (p_caster != nullptr)
                ip = sMySQLStore.getItemProperties(p_caster->getUInt32Value(PLAYER_AMMO_ID));
            else
                ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
        }
#endif

        if (ip != nullptr)
        {
            data << ip->DisplayInfoID;
            data << ip->InventoryType;
        }
#if VERSION_STRING >= Cata
        else
        {
            data << uint32(0);
            data << uint32(0);
        }
#endif
    }

#if VERSION_STRING >= WotLK
    data << (uint32)139; //3.0.2 seems to be some small value around 250 for shadow bolt.
#endif
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendSpellGo()
{
    // Fill UniqueTargets
    std::vector<uint64_t>::iterator i, j;
    for (uint8 x = 0; x < 3; x++)
    {
        if (getSpellInfo()->getEffect(x))
        {
            bool add = true;
            for (i = m_targetUnits[x].begin(); i != m_targetUnits[x].end(); ++i)
            {
                add = true;
                for (j = UniqueTargets.begin(); j != UniqueTargets.end(); ++j)
                {
                    if ((*j) == (*i))
                    {
                        add = false;
                        break;
                    }
                }
                if (add && (*i) != 0)
                    UniqueTargets.push_back((*i));
                //TargetsList::iterator itr = std::unique(m_targetUnits[x].begin(), m_targetUnits[x].end());
                //UniqueTargets.insert(UniqueTargets.begin(),));
                //UniqueTargets.insert(UniqueTargets.begin(), itr);
            }
        }
    }

    // no need to send this on passive spells
    if (!m_caster->IsInWorld() || hasAttribute(ATTRIBUTES_PASSIVE))
        return;

    // Start Spell
    WorldPacket data(200);
    data.SetOpcode(SMSG_SPELL_GO);
#if VERSION_STRING >= WotLK
    uint32 flags = 0;
    if (m_missileTravelTime != 0)
        flags |= 0x20000;
#else
    uint16_t flags = 0;
#endif

    if (GetType() == SPELL_DMG_TYPE_RANGED)
        flags |= SPELL_GO_FLAGS_RANGED; // 0x20 RANGED

    if (i_caster != NULL)
        flags |= SPELL_GO_FLAGS_ITEM_CASTER; // 0x100 ITEM CASTER

    if (ModeratedTargets.size() > 0)
        flags |= SPELL_GO_FLAGS_EXTRA_MESSAGE; // 0x400 TARGET MISSES AND OTHER MESSAGES LIKE "Resist"

#if VERSION_STRING == WotLK
    if (p_caster != NULL && getSpellInfo()->getPowerType() != POWER_TYPE_HEALTH)
        flags |= SPELL_GO_FLAGS_POWER_UPDATE;

    //experiments with rune updates
    uint8 cur_have_runes = 0;
    if (p_caster && p_caster->isClassDeathKnight())   //send our rune updates ^^
    {
        if (getSpellInfo()->getRuneCostID() && getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
            flags |= SPELL_GO_FLAGS_ITEM_CASTER | SPELL_GO_FLAGS_RUNE_UPDATE | SPELL_GO_FLAGS_UNK40000;
        //see what we will have after cast
        cur_have_runes = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();
        if (cur_have_runes != m_rune_avail_before)
            flags |= SPELL_GO_FLAGS_RUNE_UPDATE | SPELL_GO_FLAGS_UNK40000;
    }
#endif

    // hacky..
    if (getSpellInfo()->getId() == 8326)   // death
        flags = SPELL_GO_FLAGS_ITEM_CASTER | 0x0D;

    if (i_caster != NULL && u_caster != NULL)   // this is needed for correct cooldown on items
    {
        data << i_caster->GetNewGUID();
        data << u_caster->GetNewGUID();
    }
    else
    {
        data << m_caster->GetNewGUID();
        data << m_caster->GetNewGUID();
    }

#if VERSION_STRING > TBC
    data << extra_cast_number; //3.0.2
#endif
    data << getSpellInfo()->getId();
    data << flags;
    data <<Util::getMSTime();
    data << (uint8)(UniqueTargets.size()); //number of hits
    writeSpellGoTargets(&data);

    if (flags & SPELL_GO_FLAGS_EXTRA_MESSAGE)
    {
        data << (uint8)(ModeratedTargets.size()); //number if misses
        writeSpellMissedTargets(&data);
    }
    else
        data << uint8(0);   //moderated target size is 0 since we did not set the flag

    m_targets.write(data);   // this write is included the target flag

#if VERSION_STRING >= WotLK
    if (flags & SPELL_GO_FLAGS_POWER_UPDATE)
        data << (uint32)p_caster->getPower(static_cast<uint16_t>(getSpellInfo()->getPowerType()));
#endif

    // er why handle it being null inside if if you can't get into if if its null
    if (GetType() == SPELL_DMG_TYPE_RANGED)
    {
        ItemProperties const* ip = nullptr;
        if (getSpellInfo()->getId() == SPELL_RANGED_THROW)
        {
            if (p_caster != NULL)
            {
                Item* it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (it != nullptr)
                    ip = it->getItemProperties();
            }
            else
                ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
        }
        else
        {
#if VERSION_STRING < Cata
            if (p_caster != nullptr)
                ip = sMySQLStore.getItemProperties(p_caster->getUInt32Value(PLAYER_AMMO_ID));
            else // HACK FIX
                ip = sMySQLStore.getItemProperties(2512);	/*rough arrow*/
#endif
        }
        if (ip != nullptr)
        {
            data << ip->DisplayInfoID;
            data << ip->InventoryType;
        }
#if VERSION_STRING >= WotLK
        else
        {
            data << uint32(0);
            data << uint32(0);
        }
#endif
    }

    //data order depending on flags : 0x800, 0x200000, 0x20000, 0x20, 0x80000, 0x40 (this is not spellgoflag but seems to be from spellentry or packet..)
    //.text:00401110                 mov     eax, [ecx+14h] -> them
    //.text:00401115                 cmp     eax, [ecx+10h] -> us
#if VERSION_STRING == WotLK
    if (flags & SPELL_GO_FLAGS_RUNE_UPDATE)
    {
        data << uint8(m_rune_avail_before);
        data << uint8(cur_have_runes);
        for (uint8 k = 0; k < MAX_RUNES; k++)
        {
            uint8 x = (1 << k);
            if ((x & m_rune_avail_before) != (x & cur_have_runes))
                data << uint8(0);   //values of the rune converted into byte. We just think it is 0 but maybe it is not :P
        }
    }
#endif



    /*
            float dx = targets.m_destX - targets.m_srcX;
            float dy = targets.m_destY - targets.m_srcY;
            if (missilepitch != M_PI / 4 && missilepitch != -M_PI / 4) //lets not divide by 0 lul
            traveltime = (sqrtf(dx * dx + dy * dy) / (cosf(missilepitch) * missilespeed)) * 1000;
            */
#if VERSION_STRING >= WotLK
    if (flags & 0x20000)
    {
        data << float(m_missilePitch);
        data << uint32(m_missileTravelTime);
    }


    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        data << uint8(0);   //some spells require this ? not sure if it is last byte or before that.
#endif

    m_caster->SendMessageToSet(&data, true);

    // spell log execute is still send 2.08
    // as I see with this combination, need to test it more
    //if (flags != 0x120 && GetProto()->Attributes & 16) // not ranged and flag 5
    //SendLogExecute(0,m_targets.m_unitTarget);
}

void Spell::writeSpellGoTargets(WorldPacket* data)
{
    std::vector<uint64_t>::iterator i;
    for (i = UniqueTargets.begin(); i != UniqueTargets.end(); ++i)
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

void Spell::writeSpellMissedTargets(WorldPacket* data)
{
    /*
     * The flags at the end known to us so far are.
     * 1 = Miss
     * 2 = Resist
     * 3 = Dodge // melee only
     * 4 = Deflect
     * 5 = Block // melee only
     * 6 = Evade
     * 7 = Immune
     */
    if (u_caster && u_caster->isAlive())
    {
        for (auto moderated_target: ModeratedTargets)
        {
            *data << moderated_target.targetGuid;
            *data << moderated_target.targetModType;
            ///handle proc on resist spell
            Unit* target = u_caster->GetMapMgr()->GetUnit(moderated_target.targetGuid);
            if (target && target->isAlive())
            {
                u_caster->HandleProc(PROC_ON_RESIST_VICTIM, target, getSpellInfo()/*,damage*/);		/** Damage is uninitialized at this point - burlex */
                target->CombatStatusHandler_ResetPvPTimeout(); // aaa
                u_caster->CombatStatusHandler_ResetPvPTimeout(); // bbb
            }
        }
    }
    else
    {
        for (auto target: ModeratedTargets)
        {
            *data << target.targetGuid;
            *data << target.targetModType;
        }
    }
}
// Not called
void Spell::SendLogExecute(uint32 spellDamage, uint64 & targetGuid)
{
    WorldPacket data(SMSG_SPELLLOGEXECUTE, 37);
    data << m_caster->GetNewGUID();
    data << getSpellInfo()->getId();
    data << uint32(1);
    data << getSpellInfo()->getSpellVisual();
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

    WorldPacket data(SMSG_SPELL_FAILURE, 20);

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
        {
            data << m_caster->GetNewGUID();
            data << uint8(extra_cast_number);
            data << uint32(m_spellInfo->getId());
            data << uint8(result);

            plr->GetSession()->SendPacket(&data);
        }
    }

    data.Initialize(SMSG_SPELL_FAILED_OTHER);

    data << m_caster->GetNewGUID();
    data << uint8(extra_cast_number);
    data << uint32(getSpellInfo()->getId());
    data << uint8(result);

    m_caster->SendMessageToSet(&data, false);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if (time == 0)
    {
        if (u_caster && u_caster->IsInWorld())
        {
            uint64 guid = u_caster->getChannelObjectGuid();

            DynamicObject* dynObj = u_caster->GetMapMgr()->GetDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
            if (dynObj)
                dynObj->Remove();

            if (dynObj == nullptr /*&& m_pendingAuras.find(m_caster->getGuid()) == m_pendingAuras.end()*/)  //no persistant aura or aura on caster
            {
                u_caster->setChannelObjectGuid(0);
                u_caster->setChannelSpellId(0);
            }
        }

        if (p_caster != nullptr)
        {
            if (m_spellInfo->hasEffect(SPELL_EFFECT_SUMMON) && (p_caster->getCharmGuid() != 0))
            {
                Unit* u = p_caster->GetMapMgr()->GetUnit(p_caster->getCharmGuid());
                if ((u != nullptr) && (u->getCreatedBySpellId() == m_spellInfo->getId()))
                    p_caster->UnPossess();
            }
        }
    }

    if (!p_caster)
        return;

    WorldPacket data(MSG_CHANNEL_UPDATE, 18);
    data << p_caster->GetNewGUID();
    data << time;

    p_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelStart(uint32 duration)
{
    if (!m_caster->isGameObject())
    {
        // Send Channel Start
        WorldPacket data(MSG_CHANNEL_START, 22);
        data << WoWGuid(m_caster->GetNewGUID());
        data << uint32(m_spellInfo->getId());
        data << uint32(duration);
#if VERSION_STRING >= Cata
        data << uint8(0);
        data << uint8(0);
#endif
        m_caster->SendMessageToSet(&data, true);
    }

    m_castTime = m_timer = duration;

    if (u_caster != NULL)
    {
        u_caster->setChannelSpellId(getSpellInfo()->getId());
        sEventMgr.AddEvent(u_caster, &Unit::EventStopChanneling, false, EVENT_STOP_CHANNELING, duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Spell::SendResurrectRequest(Player* target)
{
    WorldPacket data(SMSG_RESURRECT_REQUEST, 13);
    data << m_caster->getGuid();
    data << uint32(0);
    data << uint8(0);

    target->GetSession()->SendPacket(&data);
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

bool Spell::HasPower()
{
    uint16_t powerField;
    if (u_caster != nullptr)
        if (u_caster->getNpcFlags() & UNIT_NPC_FLAG_TRAINER)
            return true;

    if (p_caster && p_caster->m_cheats.PowerCheat)
        return true;

    //Items do not use owner's power
    if (i_caster != nullptr)
        return true;

    // Free cast for battle preparation
    if (p_caster && p_caster->HasAura(44521))
        return true;
    if (p_caster && p_caster->HasAura(44535))
        return true;
    if (p_caster && p_caster->HasAura(32727))
        return true;

    switch (getSpellInfo()->getPowerType())
    {
        case POWER_TYPE_HEALTH:
        {	powerField = UNIT_FIELD_HEALTH;						}
        break;
        case POWER_TYPE_MANA:
        {	powerField = UNIT_FIELD_POWER1;	m_usesMana = true;	}
        break;
        case POWER_TYPE_RAGE:
        {	powerField = UNIT_FIELD_POWER2;						}
        break;
        case POWER_TYPE_FOCUS:
        {	powerField = UNIT_FIELD_POWER3;						}
        break;
        case POWER_TYPE_ENERGY:
        {	powerField = UNIT_FIELD_POWER4;						}
        break;
        case POWER_TYPE_HAPPINESS:
        {	powerField = UNIT_FIELD_POWER5;						}
        break;
#if VERSION_STRING == WotLK
        case POWER_TYPE_RUNIC_POWER:
        {	powerField = UNIT_FIELD_POWER7;						}
        break;

        case POWER_TYPE_RUNES:
        {
            if (getSpellInfo()->getRuneCostID() && p_caster)
            {
                auto spell_rune_cost = sSpellRuneCostStore.LookupEntry(getSpellInfo()->getRuneCostID());
                if (!spell_rune_cost)
                    return false;

                DeathKnight* dk = static_cast<DeathKnight*>(p_caster);
                uint32 credit = dk->HasRunes(RUNE_BLOOD, spell_rune_cost->bloodRuneCost) +
                    dk->HasRunes(RUNE_FROST, spell_rune_cost->frostRuneCost) +
                    dk->HasRunes(RUNE_UNHOLY, spell_rune_cost->unholyRuneCost);
                if (credit > 0 && dk->HasRunes(RUNE_DEATH, credit) > 0)
                    return false;
            }
            return true;
        }
#endif
        default:
        {
            LogDebugFlag(LF_SPELL, "unknown power type");
            // we shouldn't be here to return
            return false;
        }
        break;
    }


    //FIX ME: add handler for UNIT_FIELD_POWER_COST_MODIFIER
    //UNIT_FIELD_POWER_COST_MULTIPLIER
    if (u_caster != nullptr)
    {
        if (hasAttributeEx(ATTRIBUTESEX_DRAIN_WHOLE_POWER))  // Uses %100 power
        {
            m_caster->setUInt32Value(powerField, 0);
            return true;
        }
    }

    uint32_t currentPower = m_caster->getUInt32Value(powerField);
    int32 cost = 0;

    if (getSpellInfo()->getManaCostPercentage()) //Percentage spells cost % of !!!BASE!!! mana
    {
        if (u_caster != nullptr)
        {
            if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA)
                cost = (u_caster->getBaseMana() * getSpellInfo()->getManaCostPercentage()) / 100;
            else
                cost = (u_caster->getBaseHealth() * getSpellInfo()->getManaCostPercentage()) / 100;
        }
    }
    else
    {
        cost = getSpellInfo()->getManaCost();
    }

    if ((int32)getSpellInfo()->getPowerType() == POWER_TYPE_HEALTH)
        cost -= getSpellInfo()->getBaseLevel();//FIX for life tap
    else if (u_caster != nullptr)
    {
        if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA)
            cost += u_caster->PowerCostMod[getSpellInfo()->getSchool()];//this is not percent!
        else
            cost += u_caster->PowerCostMod[0];
        cost += float2int32(cost * u_caster->getPowerCostMultiplier(static_cast<uint16_t>(getSpellInfo()->getSchool())));
    }

    //hackfix for shiv's energy cost
    if (p_caster != nullptr && (m_spellInfo->getId() == 5938 || m_spellInfo->getId() == 5940))
    {
        Item* it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        if (it != nullptr)
            cost += (uint32)(10 * (it->getItemProperties()->Delay / 1000.0f));
    }

    //apply modifiers
    if (u_caster != nullptr)
    {
        spellModFlatIntValue(u_caster->SM_FCost, &cost, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(u_caster->SM_PCost, &cost, getSpellInfo()->getSpellFamilyFlags());
    }

    if (cost <= 0)
        return true;

    //FIXME:DK:if field value < cost what happens
    if (powerField == UNIT_FIELD_HEALTH)
    {
        return true;
    }
    else
    {
        if (cost <= static_cast<int32_t>(currentPower)) // Unit has enough power (needed for creatures)
        {
            return true;
        }
        else
            return false;
    }
}

bool Spell::TakePower()
{
    uint16_t powerField;
    if (u_caster != nullptr)
        if (u_caster->getNpcFlags() & UNIT_NPC_FLAG_TRAINER)
            return true;

    if (p_caster && p_caster->m_cheats.PowerCheat)
        return true;

    //Items do not use owner's power
    if (i_caster != nullptr)
        return true;

    // Free cast for battle preparation
    if (p_caster && p_caster->HasAura(44521))
        return true;
    if (p_caster && p_caster->HasAura(44535))
        return true;
    if (p_caster && p_caster->HasAura(32727))
        return true;

    switch (getSpellInfo()->getPowerType())
    {
        case POWER_TYPE_HEALTH:
        {	powerField = UNIT_FIELD_HEALTH;						}
        break;
        case POWER_TYPE_MANA:
        {	powerField = UNIT_FIELD_POWER1;	m_usesMana = true;	}
        break;
        case POWER_TYPE_RAGE:
        {	powerField = UNIT_FIELD_POWER2;						}
        break;
        case POWER_TYPE_FOCUS:
        {	powerField = UNIT_FIELD_POWER3;						}
        break;
        case POWER_TYPE_ENERGY:
        {	powerField = UNIT_FIELD_POWER4;						}
        break;
        case POWER_TYPE_HAPPINESS:
        {	powerField = UNIT_FIELD_POWER5;						}
        break;
#if VERSION_STRING == WotLK
        case POWER_TYPE_RUNIC_POWER:
        {	powerField = UNIT_FIELD_POWER7;						}
        break;

        case POWER_TYPE_RUNES:
        {
            if (getSpellInfo()->getRuneCostID() && p_caster)
            {
                auto spell_rune_cost = sSpellRuneCostStore.LookupEntry(getSpellInfo()->getRuneCostID());
                if (!spell_rune_cost)
                    return false;

                DeathKnight* dk = static_cast<DeathKnight*>(p_caster);
                uint32 credit = dk->TakeRunes(RUNE_BLOOD, spell_rune_cost->bloodRuneCost) +
                    dk->TakeRunes(RUNE_FROST, spell_rune_cost->frostRuneCost) +
                    dk->TakeRunes(RUNE_UNHOLY, spell_rune_cost->unholyRuneCost);
                if (credit > 0 && dk->TakeRunes(RUNE_DEATH, credit) > 0)
                    return false;
                if (spell_rune_cost->runePowerGain)
                {
                    if (u_caster != nullptr)
                    {
                        const auto runicPowerAmount = static_cast<uint32_t>((spell_rune_cost->runePowerGain + u_caster->getPower(POWER_TYPE_RUNIC_POWER)) * worldConfig.getFloatRate(RATE_POWER7));
                        u_caster->setPower(POWER_TYPE_RUNIC_POWER, runicPowerAmount);
                    }
                }
            }
            return true;
        }
#endif
        default:
        {
            LogDebugFlag(LF_SPELL, "unknown power type");
            // we shouldn't be here to return
            return false;
        }
        break;
    }

    //FIX ME: add handler for UNIT_FIELD_POWER_COST_MODIFIER
    //UNIT_FIELD_POWER_COST_MULTIPLIER
    if (u_caster != nullptr)
    {
        if (hasAttributeEx(ATTRIBUTESEX_DRAIN_WHOLE_POWER))  // Uses %100 power
        {
            m_caster->setUInt32Value(powerField, 0);
            return true;
        }
    }

    uint32_t currentPower = m_caster->getUInt32Value(powerField);
    int32 cost = 0;

    if (getSpellInfo()->getManaCostPercentage()) //Percentage spells cost % of !!!BASE!!! mana
    {
        if (u_caster != nullptr)
        {
            if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA)
                cost = (u_caster->getBaseMana() * getSpellInfo()->getManaCostPercentage()) / 100;
            else
                cost = (u_caster->getBaseHealth() * getSpellInfo()->getManaCostPercentage()) / 100;
        }
    }
    else
    {
        cost = getSpellInfo()->getManaCost();
    }

    if ((int32)getSpellInfo()->getPowerType() == POWER_TYPE_HEALTH)
        cost -= getSpellInfo()->getBaseLevel();//FIX for life tap
    else if (u_caster != nullptr)
    {
        if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA)
            cost += u_caster->PowerCostMod[getSpellInfo()->getSchool()];//this is not percent!
        else
            cost += u_caster->PowerCostMod[0];
        cost += float2int32(cost * u_caster->getPowerCostMultiplier(static_cast<uint16_t>(getSpellInfo()->getSchool())));
    }

    //hackfix for shiv's energy cost
    if (p_caster != nullptr && (m_spellInfo->getId() == 5938 || m_spellInfo->getId() == 5940))
    {
        Item* it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        if (it != nullptr)
            cost += it->getItemProperties()->Delay / 100;//10 * it->GetProto()->Delay / 1000;
    }

    //apply modifiers
    if (u_caster != nullptr)
    {
        spellModFlatIntValue(u_caster->SM_FCost, &cost, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(u_caster->SM_PCost, &cost, getSpellInfo()->getSpellFamilyFlags());
    }

    if (cost <= 0)
        return true;

    //FIXME:DK:if field value < cost what happens
    if (powerField == UNIT_FIELD_HEALTH)
    {
        m_caster->DealDamage(u_caster, cost, 0, 0, 0, true);
        return true;
    }
    else
    {
        if (cost <= static_cast<int32_t>(currentPower)) // Unit has enough power (needed for creatures)
        {
            m_caster->setUInt32Value(powerField, currentPower - cost);
            return true;
        }
        else
            return false;
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
            if (m_targets.m_targetMask & TARGET_FLAG_ITEM)
                itemTarget = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
            if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
            {
#if VERSION_STRING >= Cata
                Player* p_trader = p_caster->getTradeTarget();
                if (p_trader != nullptr)
                    itemTarget = p_trader->getTradeData()->getTradeItem((TradeSlots)m_targets.m_itemTarget);
#else
                Player* p_trader = p_caster->GetTradeTarget();
                if (p_trader != NULL)
                    itemTarget = p_trader->getTradeItem((uint32)m_targets.m_itemTarget);
#endif
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
        else if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            if (p_caster != nullptr)
            {
#if VERSION_STRING >= Cata
                Player* plr = p_caster->getTradeTarget();
                if (plr != nullptr)
                    itemTarget = plr->getTradeData()->getTradeItem((TradeSlots)guid);
#else
                Player* plr = p_caster->GetTradeTarget();
                if (plr)
                    itemTarget = plr->getTradeItem((uint32)guid);
#endif
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
                    corpseTarget = objmgr.GetCorpse(wowGuid.getGuidLowPart());
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
        (*this.*SpellEffectsHandler[id])(static_cast<uint8_t>(i));
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
targets.m_unitTarget = TriggerSpellTarget;
else
targets.m_unitTarget = m_targets.m_unitTarget;

spell->prepare(&targets);
}
}*/

void Spell::DetermineSkillUp()
{
    if (p_caster == nullptr)
        return;

    auto skill_line_ability = objmgr.GetSpellSkill(getSpellInfo()->getId());
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
                ascemu::World::Spell::Helpers::spellModFlatIntValue(u_caster->SM_FDur, (int32*)&this->Dur, getSpellInfo()->getSpellFamilyFlags());
                ascemu::World::Spell::Helpers::spellModPercentageIntValue(u_caster->SM_PDur, (int32*)&this->Dur, getSpellInfo()->getSpellFamilyFlags());
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
        ascemu::World::Spell::Helpers::spellModFlatFloatValue(u_caster->SM_FRadius, &Rad[i], getSpellInfo()->getSpellFamilyFlags());
        ascemu::World::Spell::Helpers::spellModPercentageFloatValue(u_caster->SM_PRadius, &Rad[i], getSpellInfo()->getSpellFamilyFlags());
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

uint8 Spell::CanCast(bool tolerate)
{
    /**
     *	Object cast checks
     */
    if (m_caster && m_caster->IsInWorld())
    {
        Unit* target = m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);

        /**
         *	Check for valid targets
         */
        if (target)
        {
            //you can't mind control someone already mind controlled
            switch (getSpellInfo()->getId())
            {
                //SPELL_HASH_MIND_CONTROL
                case 605:
                case 11446:
                case 15690:
                case 36797:
                case 36798:
                case 43550:
                case 43871:
                case 43875:
                case 45112:
                case 67229:
                {
                    uint32 mindControl[] =
                    {
                        //SPELL_HASH_MIND_CONTROL
                        605,
                        11446,
                        15690,
                        36797,
                        36798,
                        43550,
                        43871,
                        43875,
                        45112,
                        67229,
                        0
                    };

                    if (target->hasAurasWithId(mindControl))
                        return SPELL_FAILED_BAD_TARGETS;
                } break;
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
         *	Check for valid location
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
     *	Player caster checks
     */
    if (p_caster)
    {
        /**
         *	Indoor/Outdoor check
         */
        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            if (getSpellInfo()->getMechanicsType() == MECHANIC_MOUNTED)
            {
                if (!MapManagement::AreaManagement::AreaStorage::IsOutdoor(p_caster->GetMapId(), p_caster->GetPositionNC().x, p_caster->GetPositionNC().y, p_caster->GetPositionNC().z))
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;
            }
        }

        /**
         *	Battlegrounds/Arena check
         */
        if (p_caster->m_bg)
        {
            if (!p_caster->m_bg->HasStarted() && (m_spellInfo->getId() == 1953 || m_spellInfo->getId() == 36554))  //Don't allow blink or shadowstep  if in a BG and the BG hasn't started.
                return SPELL_FAILED_SPELL_UNAVAILABLE;
        }

        /**
         * Mana check
         */
        if (!HasPower())
            return SPELL_FAILED_NO_POWER;

        /**
         *	Duel request check
         */
        if (p_caster->GetDuelState() == DUEL_STATE_REQUESTED)
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (getSpellInfo()->getEffect(i) && getSpellInfo()->getEffect(i) != SPELL_EFFECT_APPLY_AURA && getSpellInfo()->getEffect(i) != SPELL_EFFECT_APPLY_PET_AREA_AURA
                    && getSpellInfo()->getEffect(i) != SPELL_EFFECT_APPLY_GROUP_AREA_AURA && getSpellInfo()->getEffect(i) != SPELL_EFFECT_APPLY_RAID_AREA_AURA)
                {
                    return SPELL_FAILED_TARGET_DUELING;
                }
            }
        }

        /**
         *	Duel area check
         */
        if (getSpellInfo()->getId() == 7266)
        {
            auto at = p_caster->GetArea();
            if (at->flags & AREA_CITY_AREA)
                return SPELL_FAILED_NO_DUELING;
            // instance & stealth checks
            if (p_caster->GetMapMgr() && p_caster->GetMapMgr()->GetMapInfo() && p_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
                return SPELL_FAILED_NO_DUELING;
            if (p_caster->isStealthed())
                return SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED;
        }

        /**
         *	On taxi check
         */
        if (p_caster->m_onTaxi)
        {
            if (!hasAttribute(ATTRIBUTES_MOUNT_CASTABLE))    //Are mount castable spells allowed on a taxi?
            {
                if (m_spellInfo->getId() != 33836 && m_spellInfo->getId() != 45072 && m_spellInfo->getId() != 45115 && m_spellInfo->getId() != 31958)   // exception for taxi bombs
                    return SPELL_FAILED_NOT_ON_TAXI;
            }
        }
        else
        {
            if (m_spellInfo->getId() == 33836 || m_spellInfo->getId() == 45072 || m_spellInfo->getId() == 45115 || m_spellInfo->getId() == 31958)
                return SPELL_FAILED_NOT_HERE;
        }

        /**
         *	Is mounted check
         */
        if (!p_caster->IsMounted())
        {
            if (getSpellInfo()->getId() == 25860)  // Reindeer Transformation
                return SPELL_FAILED_ONLY_MOUNTED;
        }

        // check if spell is allowed while shapeshifted
        if (p_caster->getShapeShiftForm())
        {
            switch (p_caster->getShapeShiftForm())
            {
                case FORM_TREE:
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                case FORM_SHADOW:
                case FORM_STEALTH:
                case FORM_MOONKIN:
                {
                    break;
                }

                case FORM_SWIFT:
                case FORM_FLIGHT:
                {
                    // check if item is allowed (only special items allowed in flight forms)
                    if (i_caster && !(i_caster->getItemProperties()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
                        return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;

                    break;
                }

                //case FORM_CAT:
                //case FORM_TRAVEL:
                //case FORM_AQUA:
                //case FORM_BEAR:
                //case FORM_AMBIENT:
                //case FORM_GHOUL:
                //case FORM_DIREBEAR:
                //case FORM_CREATUREBEAR:
                //case FORM_GHOSTWOLF:

                case FORM_SPIRITOFREDEMPTION:
                {
                    //Spirit of Redemption (20711) fix
                    if (!(getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_HEALING) && getSpellInfo()->getId() != 7355)
                        return SPELL_FAILED_CASTER_DEAD;
                    break;
                }


                default:
                {
                    // check if item is allowed (only special & equipped items allowed in other forms)
                    if (i_caster && !(i_caster->getItemProperties()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
                        if (i_caster->getItemProperties()->InventoryType == INVTYPE_NON_EQUIP)
                            return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;
                }
            }
        }

        /**
         *	check if spell requires shapeshift
         */
         // I think Spell prototype's RequiredShapeShift is not entirely accurate ....
         //if (GetProto()->RequiredShapeShift && !(GetProto()->RequiredShapeShift == (uint32)1 << (FORM_SHADOW - 1)) && !((uint32)1 << (p_caster->GetShapeShift()-1) & GetProto()->RequiredShapeShift))
         //{
         //	return SPELL_FAILED_ONLY_SHAPESHIFT;
         //}


         /**
          *	check if spell is allowed while we have a battleground flag
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

        /**
         *	check if we have the required gameobject focus
         */
        float focusRange;

        if (getSpellInfo()->getRequiresSpellFocus())
        {
            bool found = false;

            for (const auto& itr : p_caster->getInRangeObjectsSet())
            {
                auto obj = itr;
                if (!obj || !itr->isGameObject())
                    continue;

                if ((static_cast<GameObject*>(itr))->getGoType() != GAMEOBJECT_TYPE_SPELL_FOCUS)
                    continue;

                if (!(p_caster->GetPhase() & itr->GetPhase()))    //We can't see this, can't be the focus, skip further checks
                    continue;

                auto gameobject_info = static_cast<GameObject*>(itr)->GetGameObjectProperties();
                if (!gameobject_info)
                {
                    LogDebugFlag(LF_SPELL, "Warning: could not find info about game object %u", (itr)->getEntry());
                    continue;
                }

                // professions use rangeIndex 1, which is 0yds, so we will use 5yds, which is standard interaction range.
                if (gameobject_info->raw.parameter_1)
                    focusRange = float(gameobject_info->raw.parameter_1);
                else
                    focusRange = GetMaxRange(sSpellRangeStore.LookupEntry(getSpellInfo()->getRangeIndex()));

                // check if focus object is close enough
                if (!obj->isInRange(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), (focusRange * focusRange)))
                    continue;

                if (gameobject_info->raw.parameter_0 == getSpellInfo()->getRequiresSpellFocus())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
        }
    }

    /**
    *	Targeted Item Checks
    */
    if (p_caster && m_targets.m_itemTarget)
    {
        Item* i_target = nullptr;

        // check if the targeted item is in the trade box
        if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            switch (getSpellInfo()->getEffect(0))
            {
                // only lockpicking and enchanting can target items in the trade box
                case SPELL_EFFECT_OPEN_LOCK:
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                {
                    // get the player we are trading with
#if VERSION_STRING >= Cata
                    Player* t_player = p_caster->getTradeTarget();
                    if (t_player != nullptr)
                        i_target = t_player->getTradeData()->getTradeItem((TradeSlots)m_targets.m_itemTarget);
#else
                    Player* t_player = p_caster->GetTradeTarget();
                    if (t_player)
                        i_target = t_player->getTradeItem((uint32)m_targets.m_itemTarget);
#endif
                }
            }
        }
        // targeted item is not in a trade box, so get our own item
        else
        {
            i_target = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
        }

        ItemProperties const* proto = i_target->getItemProperties();

        // check to make sure the targeted item is acceptable
        switch (getSpellInfo()->getEffect(0))
        {
            // Feed Pet Targeted Item Check
            case SPELL_EFFECT_FEED_PET:
            {
                Pet* pPet = p_caster->GetSummon();

                // check if we have a pet
                if (!pPet)
                    return SPELL_FAILED_NO_PET;

                // check if pet lives
                if (!pPet->isAlive())
                    return SPELL_FAILED_TARGETS_DEAD;

                // check if item is food
                if (!proto->FoodType)
                    return SPELL_FAILED_BAD_TARGETS;

                // check if food type matches pets diet
                if (!(pPet->GetPetDiet() & (1 << (proto->FoodType - 1))))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                // check food level: food should be max 30 lvls below pets level
                if (pPet->getLevel() > proto->ItemLevel + 30)
                    return SPELL_FAILED_FOOD_LOWLEVEL;

                break;
            }

        } // end switch

    } // end targeted item

    /**
     *	set up our max range
     *	latency compensation!!
     *	figure out how much extra distance we need to allow for based on our
     *	movespeed and latency.
     */
    float maxRange = 0;

    auto spell_range = sSpellRangeStore.LookupEntry(getSpellInfo()->getRangeIndex());
    if (spell_range != nullptr)
    {
        if (m_caster->IsInWorld())
        {
            Unit* target = m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);
#if VERSION_STRING > TBC
            if (target != nullptr && isFriendly(m_caster, target))
                maxRange = spell_range->maxRangeFriendly;
            else
                maxRange = spell_range->maxRange;
#else
            maxRange = spell_range->maxRange;
#endif
        }
        else
            maxRange = spell_range->maxRange;
    }

    if (u_caster && m_caster->GetMapMgr() && m_targets.m_unitTarget)
    {
        Unit* utarget = m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);

        if (utarget && utarget->isPlayer() && static_cast<Player*>(utarget)->m_isMoving)
        {
            // this only applies to PvP.
            uint32 lat = static_cast<Player*>(utarget)->GetSession() ? static_cast<Player*>(utarget)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            maxRange += u_caster->getSpeedForType(TYPE_RUN) * 0.001f * lat;
        }
    }

    /**
     *	Some Unit caster range check
     */
    if (u_caster != nullptr)
    {
        spellModFlatFloatValue(u_caster->SM_FRange, &maxRange, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageFloatValue(u_caster->SM_PRange, &maxRange, getSpellInfo()->getSpellFamilyFlags());
    }

    // Targeted Location Checks (AoE spells)
    if (m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION)
    {
        if (!m_caster->isInRange(m_targets.destination(), (maxRange * maxRange)))
            return SPELL_FAILED_OUT_OF_RANGE;
    }

    /**
     *	Targeted Unit Checks
     */
    if (m_targets.m_unitTarget)
    {
        Unit* target = (m_caster->IsInWorld()) ? m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget) : NULL;

        if (target)
        {
            // getBoundingRadius()+ 1.5f; seems to match the client range

            if (tolerate)   // add an extra 33% to range on final check (squared = 1.78x)
            {
                float localrange = maxRange + target->getBoundingRadius() + 1.5f;
                if (!target->isInRange(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), (localrange * localrange * 1.78f)))
                    return SPELL_FAILED_OUT_OF_RANGE;
            }
            else
            {
                float localrange = maxRange + target->getBoundingRadius() + 1.5f;
                if (!target->isInRange(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), (localrange * localrange)))
                    return SPELL_FAILED_OUT_OF_RANGE;
            }

            if (p_caster != nullptr)
            {
                if (target->isPlayer())
                {
                    // disallow spell casting in sanctuary zones
                    // allow attacks in duels
                    if (p_caster->DuelingWith != target && !isFriendly(p_caster, target))
                    {
                        auto atCaster = p_caster->GetArea();
                        auto atTarget = target->GetArea();
                        if (atCaster->flags & 0x800 || atTarget->flags & 0x800)
                            return SPELL_FAILED_NOT_HERE;
                    }
                }
                else
                {
                    if (target->GetAIInterface()->GetIsSoulLinked() && u_caster && target->GetAIInterface()->getSoullinkedWith() != u_caster)
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // pet training
                if (getSpellInfo()->getEffectImplicitTargetA(0) == EFF_TARGET_PET &&
                    getSpellInfo()->getEffect(0) == SPELL_EFFECT_LEARN_SPELL)
                {
                    Pet* pPet = p_caster->GetSummon();
                    // check if we have a pet
                    if (pPet == nullptr)
                        return SPELL_FAILED_NO_PET;

                    // other checks
                    SpellInfo const* trig = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(0));
                    if (trig == nullptr)
                        return SPELL_FAILED_SPELL_UNAVAILABLE;

                    uint32 status = pPet->CanLearnSpell(trig);
                    if (status != 0)
                        return static_cast<uint8>(status);
                }

                if (getSpellInfo()->getEffectApplyAuraName(0) == SPELL_AURA_MOD_POSSESS)  //mind control
                {
                    if (getSpellInfo()->getEffectBasePoints(0))  //got level req;
                    {
                        if ((int32)target->getLevel() > getSpellInfo()->getEffectBasePoints(0) + 1 + int32(p_caster->getLevel() - getSpellInfo()->getSpellLevel()))
                            return SPELL_FAILED_HIGHLEVEL;
                        else if (target->isCreature())
                        {
                            Creature* c = static_cast<Creature*>(target);
                            if (c->GetCreatureProperties()->Rank > ELITE_ELITE)
                                return SPELL_FAILED_HIGHLEVEL;
                        }
                    }
                }
            }

            // \todo Replace this awful hacks with a better solution
            // Nestlewood Owlkin - Quest 9303
            if (getSpellInfo()->getId() == 29528 && target->isCreature() && target->getEntry() == 16518)
            {
                if (target->isRooted())
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }

                target->setTargetGuid(p_caster->getGuid());
                return SPELL_FAILED_SUCCESS;
            }

            // Lazy Peons - Quest 5441
            if (getSpellInfo()->getId() == 19938 && target->isCreature() && target->getEntry() == 10556)
            {
                if (!target->HasAura(17743))
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }

                return SPELL_FAILED_SUCCESS;
            }

            ////////////////////////////////////////////////////// Target check spells that are only castable on certain creatures/gameobjects ///////////////

            if (m_target_constraint != nullptr)
            {
                // target is the wrong creature
                if (target->isCreature() && !m_target_constraint->hasCreature(target->getEntry()) && !m_target_constraint->isFocused(target->getEntry()))
                    return SPELL_FAILED_BAD_TARGETS;

                // target is the wrong GO :/
                if (target->isGameObject() && !m_target_constraint->hasGameObject(target->getEntry()) && !m_target_constraint->isFocused(target->getEntry()))
                    return SPELL_FAILED_BAD_TARGETS;

                bool foundTarget = false;
                Creature* pCreature = nullptr;
                size_t creatures = m_target_constraint->getCreatures().size();

                // Spells for Invisibl Creatures and or Gameobjects ( Casting Spells Near them )
                for (size_t j = 0; j < creatures; ++j)
                {
                    if (!m_target_constraint->isFocused(m_target_constraint->getCreatures()[j]))
                    {
                        pCreature = m_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_target_constraint->getCreatures()[j]);

                        if (pCreature)
                        {
                            if (pCreature->getDistanceSq(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()) <= 15)
                            {
                                SetTargetConstraintCreature(pCreature);
                                foundTarget = true;
                            }
                        }
                    }
                }

                GameObject* pGameobject = nullptr;
                size_t gameobjects = m_target_constraint->getGameObjects().size();

                for (size_t j = 0; j < gameobjects; ++j)
                {
                    if (!m_target_constraint->isFocused(m_target_constraint->getGameObjects()[j]))
                    {
                        pGameobject = m_caster->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_target_constraint->getGameObjects()[j]);

                        if (pGameobject)
                        {
                            if (pGameobject->getDistanceSq(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()) <= 15)
                            {
                                SetTargetConstraintGameObject(pGameobject);
                                foundTarget = true;
                            }
                        }
                    }
                }

                if (!foundTarget)
                    return SPELL_FAILED_BAD_TARGETS;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // scripted spell stuff
            switch (getSpellInfo()->getId())
            {
                case 1515: // tame beast
                {
                    uint8 result = 0;
                    Unit* tgt = unitTarget;
                    if (tgt == nullptr)
                    {
                        // we have to pick a target manually as this is a dummy spell which triggers tame effect at end of channeling
                        if (p_caster->GetSelection() != 0)
                            tgt = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());
                        else
                            return SPELL_FAILED_UNKNOWN;
                    }

                    Creature* tame = tgt->isCreature() ? static_cast<Creature*>(tgt) : NULL;

                    if (tame == nullptr)
                        result = PETTAME_INVALIDCREATURE;
                    else if (!tame->isAlive())
                        result = PETTAME_DEAD;
                    else if (tame->isPet())
                        result = PETTAME_CREATUREALREADYOWNED;
                    else if (tame->GetCreatureProperties()->Type != UNIT_TYPE_BEAST || !tame->GetCreatureProperties()->Family || !(tame->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_TAMEABLE))
                        result = PETTAME_NOTTAMEABLE;
                    else if (!p_caster->isAlive() || p_caster->getClass() != HUNTER)
                        result = PETTAME_UNITSCANTTAME;
                    else if (tame->getLevel() > p_caster->getLevel())
                        result = PETTAME_TOOHIGHLEVEL;
                    else if (p_caster->GetSummon() || p_caster->GetUnstabledPetNumber())
                        result = PETTAME_ANOTHERSUMMONACTIVE;
                    else if (p_caster->GetPetCount() >= 5)
                        result = PETTAME_TOOMANY;
                    else if (!p_caster->HasSpell(53270) && tame->IsExotic())
                        result = PETTAME_CANTCONTROLEXOTIC;
                    else
                    {
                        auto cf = sCreatureFamilyStore.LookupEntry(tame->GetCreatureProperties()->Family);
                        if (cf && !cf->tameable)
                            result = PETTAME_NOTTAMEABLE;
                    }
                    if (result != 0)
                    {
                        SendTameFailure(result);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
                break;

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

            if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_SKINNING)  // skinning
            {
                // if target doesn't have skinnable flag, don't let it be skinned
                if (!target->hasUnitFlags(UNIT_FLAG_SKINNABLE))
                    return SPELL_FAILED_TARGET_UNSKINNABLE;
                // if target is already skinned, don't let it be skinned again
                if (target->isCreature() && static_cast<Creature*>(target)->Skinned)
                    return SPELL_FAILED_TARGET_UNSKINNABLE;
            }

            // all spells with target 61 need to be in group or raid
            ///\todo need to research this if its not handled by the client!!!
            if (getSpellInfo()->getEffectImplicitTargetA(0) == EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS ||
                getSpellInfo()->getEffectImplicitTargetA(1) == EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS ||
                getSpellInfo()->getEffectImplicitTargetA(2) == EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS)
            {
                if (target->isPlayer() && !static_cast<Player*>(target)->InGroup())
                    return SPELL_FAILED_TARGET_NOT_IN_PARTY;
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

            if (p_caster != nullptr)
            {
                if (getSpellInfo()->getCategory() == 1131) //Hammer of wrath, requires target to have 20- % of hp
                {
                    if (target->getHealth() == 0)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (target->getMaxHealth() / target->getHealth() < 5)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (getSpellInfo()->getCategory() == 672) //Conflagrate, requires immolation spell on victim
                {
                    if (!target->HasAuraVisual(46))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                if (target->dispels[getSpellInfo()->getDispelType()])
                    return SPELL_FAILED_DAMAGE_IMMUNE;			// hackfix - burlex

                // Removed by Supalosa and moved to 'completed cast'
                //if (target->MechanicsDispels[GetProto()->MechanicsType])
                //	return SPELL_FAILED_PREVENTED_BY_MECHANIC-1; // Why not just use 	SPELL_FAILED_DAMAGE_IMMUNE                                   = 144,
            }

            // if we're replacing a higher rank, deny it
            AuraCheckResponse acr = target->AuraCheck(getSpellInfo(), m_caster);
            if (acr.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                return SPELL_FAILED_AURA_BOUNCED;

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

    // Special State Checks (for creatures & players)
    if (u_caster != nullptr)
    {
        if (u_caster->SchoolCastPrevent[getSpellInfo()->getSchool()])
        {
            uint32 now_ = Util::getMSTime();
            if (now_ > u_caster->SchoolCastPrevent[getSpellInfo()->getSchool()]) //this limit has expired,remove
                u_caster->SchoolCastPrevent[getSpellInfo()->getSchool()] = 0;
            else
            {
                switch (getSpellInfo()->getId())
                {
                    //SPELL_HASH_BERSERKER_RAGE
                    case 18499:
                    {
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR))
                            break;
                    } break;

                    //SPELL_HASH_WILL_OF_THE_FORSAKEN
                    case 7744:
                    {
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR | UNIT_STATE_CHARM))
                            break;
                    } break;

                    // {Insignia|Medallion} of the {Horde|Alliance}
                    //SPELL_HASH_PVP_TRINKET
                    case 42292:
                    case 65547:
                    // SPELL_HASH_EVERY_MAN_FOR_HIMSELF:
                    case 59752:
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
                    {
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR | UNIT_STATE_CHARM | UNIT_STATE_STUN | UNIT_STATE_CONFUSE) || u_caster->hasUnitMovementFlag(MOVEFLAG_ROOTED))
                            break;
                    } break;

                    // SPELL_HASH_DEATH_WISH:
                    case 12292:
                    // SPELL_HASH_FEAR_WARD:
                    case 6346:
                    {
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR))
                            break;
                    } break;

                    // SPELL_HASH_BARKSKIN:
                    case 20655:
                    case 22812:
                    case 63408:
                    case 63409:
                    case 65860:
                    {
                        // This spell is usable while stunned, frozen, incapacitated, feared or asleep.  Lasts 12 sec.
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_STUN | UNIT_STATE_FEAR))     // Uh, what unit_state is Frozen? (freezing trap...)
                            break;
                    } break;

                    // SPELL_HASH_DISPERSION:
                    case 47218:
                    case 47585:
                    case 49766:
                    case 49768:
                    case 60069:
                    case 63230:
                    case 65544:
                    {
                        if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR | UNIT_STATE_STUN | UNIT_STATE_SILENCE))
                            break;
                    } break;

                    default:
                        return SPELL_FAILED_SILENCED;
                }
            }
        }

        // can only silence non-physical
        if (u_caster->m_silenced && getSpellInfo()->getSchool() != SCHOOL_NORMAL)
        {
            switch (getSpellInfo()->getId())
            {
                // SPELL_HASH_ICE_BLOCK: //Ice Block
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
                // SPELL_HASH_DIVINE_SHIELD: //Divine Shield
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
                // SPELL_HASH_DISPERSION:
                case 47218:
                case 47585:
                case 49766:
                case 49768:
                case 60069:
                case 63230:
                case 65544:
                    break;

                default:
                    return SPELL_FAILED_SILENCED;
            }
        }

        Unit* target = (m_caster->IsInWorld()) ? m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget) : NULL;
        if (target)  /* -Supalosa- Shouldn't this be handled on Spell Apply? */
        {
            for (uint8_t i = 0; i < 3; i++)  // if is going to cast a spell that breaks stun remove stun auras, looks a bit hacky but is the best way i can find
            {
                if (getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MECHANIC_IMMUNITY)
                {
                    target->RemoveAllAurasByMechanic(getSpellInfo()->getEffectMiscValue(i), 0, true);
                    // Remove all debuffs of that mechanic type.
                    // This is also done in SpellAuras.cpp - wtf?
                }
                /*
                if (GetProto()->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY && (GetProto()->EffectMiscValue[i] == 12 || GetProto()->EffectMiscValue[i] == 17))
                {
                for (uint32 x=MAX_POSITIVE_AURAS;x<MAX_AURAS;x++)
                if (target->m_auras[x])
                if (target->m_auras[x]->GetSpellProto()->MechanicsType == GetProto()->EffectMiscValue[i])
                target->m_auras[x]->Remove();
                }
                */
            }
        }

        // only affects physical damage
        if (u_caster->IsPacified() && getSpellInfo()->getSchool() == SCHOOL_NORMAL)
        {
            // HACK FIX
            switch (getSpellInfo()->getId())
            {
                // SPELL_HASH_ICE_BLOCK: //Ice Block
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
                // SPELL_HASH_DIVINE_SHIELD: //Divine Shield
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
                // SPELL_HASH_WILL_OF_THE_FORSAKEN: //Will of the Forsaken
                case 7744:
                // SPELL_HASH_EVERY_MAN_FOR_HIMSELF: // Every Man for Himself
                case 59752:
                {
                    if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR | UNIT_STATE_CHARM))
                        break;
                }
                break;

                default:
                    return SPELL_FAILED_PACIFIED;
            }
        }

        /**
         *	Stun check
         */
        if (u_caster->IsStunned() && (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_STUNNED) == 0)
            return SPELL_FAILED_STUNNED;

        /**
         *	Fear check
         */
        if (u_caster->IsFeared() && (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_FEARED) == 0)
            return SPELL_FAILED_FLEEING;

        /**
         *	Confuse check
         */
        if (u_caster->hasUnitFlags(UNIT_FLAG_CONFUSED) && (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_CONFUSED) == 0)
            return SPELL_FAILED_CONFUSED;
    }

    /**
     * Dead pet check
     */
    if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_DEAD_PET && p_caster != nullptr)
    {
        Pet* pPet = p_caster->GetSummon();
        if (pPet != nullptr && !pPet->isDead())
            return SPELL_FAILED_TARGET_NOT_DEAD;
    }

    // no problems found, so we must be ok
    return SPELL_CANCAST_OK;
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
    // Ammo Removal
    if (p_caster != nullptr)
    {
#if VERSION_STRING < Cata
        if (hasAttributeExB(ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS) || hasAttributeExC(ATTRIBUTESEXC_PLAYER_RANGED_SPELLS))
        {
            if (!p_caster->m_requiresNoAmmo)
                p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(p_caster->getUInt32Value(PLAYER_AMMO_ID), 1, &i_caster);
        }
#endif

        // Reagent Removal
        if (!(p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST) && hasAttributeExE(ATTRIBUTESEXE_REAGENT_REMOVAL)))
        {
            for (uint8 i = 0; i < 8; i++)
            {
                if (getSpellInfo()->getReagent(i))
                {
                    p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(getSpellInfo()->getReagent(i), getSpellInfo()->getReagentCount(i), &i_caster);
                }
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
        basePoints = getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i)) + 1;
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

    for (auto target: ModeratedTargets)
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
                    value += float2int32(ceilf(ap * 0.01f));	// / 100
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
        data <<Util::getMSTime();
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

    WorldPacket data(SMSG_SPELLHEALLOG, 33);
    data << target->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellid;
    data << healed;
    data << overhealed;
    data << absorbed;
    data << uint8(critical);

    caster->SendMessageToSet(&data, true);
}

void Spell::SendHealManaSpellOnPlayer(Object* caster, Object* target, uint32 dmg, uint32 powertype, uint32 spellid)
{
    if (caster == nullptr || target == nullptr || !target->isPlayer())
        return;

    WorldPacket data(SMSG_SPELLENERGIZELOG, 30);

    data << target->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellid;
    data << powertype;
    data << dmg;

    caster->SendMessageToSet(&data, true);
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
    uint32 school = getSpellInfo()->getSchool();

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
                {	spellid = 55428; }
                break;// Rank 1
                case 150:
                {	spellid = 55480; }
                break;// Rank 2
                case 225:
                {	spellid = 55500; }
                break;// Rank 3
                case 300:
                {	spellid = 55501; }
                break;// Rank 4
                case 375:
                {	spellid = 55502; }
                break;// Rank 5
                case 450:
                {	spellid = 55503; }
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
                {	spellid = 53120; }
                break;// Rank 1
                case 150:
                {	spellid = 53121; }
                break;// Rank 2
                case 225:
                {	spellid = 53122; }
                break;// Rank 3
                case 300:
                {	spellid = 53123; }
                break;// Rank 4
                case 375:
                {	spellid = 53124; }
                break;// Rank 5
                case 450:
                {	spellid = 53040; }
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
                {	spellid = 53125; }
                break;// Rank 1
                case 150:
                {	spellid = 53662; }
                break;// Rank 2
                case 225:
                {	spellid = 53663; }
                break;// Rank 3
                case 300:
                {	spellid = 53664; }
                break;// Rank 4
                case 375:
                {	spellid = 53665; }
                break;// Rank 5
                case 450:
                {	spellid = 53666; }
                break;// Rank 6
                case 525:
                {    spellid = 74495; }
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

    auto skill_line_ability = objmgr.GetSpellSkill(getSpellInfo()->getId());
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
    for (auto target: ModeratedTargets)
    {
        if (target.targetGuid == guid)
        {
            //LOG_DEBUG("[SPELL] Something goes wrong in spell target system");
            // this isn't actually wrong, since we only have one missed target map,
            // whereas hit targets have multiple maps per effect.
            return;
        }
    }

    ModeratedTargets.push_back(SpellTargetMod(guid, 2));
}

void Spell::SafeAddModeratedTarget(uint64 guid, uint16 type)
{
    for (auto target: ModeratedTargets)
    {
        if (target.targetGuid == guid)
        {
            //LOG_DEBUG("[SPELL] Something goes wrong in spell target system");
            // this isn't actually wrong, since we only have one missed target map,
            // whereas hit targets have multiple maps per effect.
            return;
        }
    }

    ModeratedTargets.push_back(SpellTargetMod(guid, (uint8)type));
}

bool Spell::Reflect(Unit* refunit)
{
    SpellInfo const* refspell = nullptr;
    bool canreflect = false;

    if (m_reflectedParent != nullptr || refunit == nullptr || m_caster == refunit)
        return false;

    // if the spell to reflect is a reflect spell, do nothing.
    for (uint8 i = 0; i < 3; i++)
    {
        if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_APPLY_AURA && (getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_REFLECT_SPELLS_SCHOOL ||
            getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_REFLECT_SPELLS))
            return false;
    }

    for (std::list<struct ReflectSpellSchool*>::iterator i = refunit->m_reflectSpellSchool.begin(); i != refunit->m_reflectSpellSchool.end(); ++i)
    {
        if ((*i)->school == -1 || (*i)->school == (int32)getSpellInfo()->getSchool())
        {
            if (Rand((float)(*i)->chance))
            {
                //the god blessed special case : mage - Frost Warding = is an augmentation to frost warding
                if ((*i)->spellId && !refunit->hasAurasWithId((*i)->spellId))
                    continue;

                if ((*i)->infront == true)
                {
                    if (m_caster->isInFront(refunit))
                    {
                        canreflect = true;
                    }
                }
                else
                    canreflect = true;

                if ((*i)->charges > 0)
                {
                    (*i)->charges--;
                    if ((*i)->charges <= 0)
                    {
                        if (!refunit->RemoveAura((*i)->spellId))	// should delete + erase RSS too, if unit hasn't such an aura...
                        {
                            delete *i;								// ...do it manually
                            refunit->m_reflectSpellSchool.erase(i);
                        }
                    }
                }

                refspell = getSpellInfo();
                break;
            }
        }
    }

    if (!refspell || !canreflect)
        return false;

    Spell* spell = sSpellMgr.newSpell(refunit, refspell, true, nullptr);
    spell->SetReflected();
    SpellCastTargets targets;
    targets.m_unitTarget = m_caster->getGuid();
    spell->prepare(&targets);

    return true;
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
        (u_caster != nullptr && u_caster->isPet() && static_cast<Pet*>(u_caster)->getPlayerOwner() && dynamic_cast<Player*>(static_cast<Pet*>(u_caster)->getPlayerOwner())->GetDuelState() != DUEL_STATE_STARTED)))
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

bool Spell::IsReflected() const
{
    return m_IsReflected;
}

void Spell::SetReflected(bool reflected)
{
    m_IsReflected = reflected;
}

bool Spell::GetCanReflect() const
{
    return m_CanRelect;
}

void Spell::SetCanReflect(bool reflect)
{
    m_CanRelect = reflect;
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

        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            auto destination = m_targets.destination();
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
            //	time *= 1.25; //reflected projectiles move back 4x faster

            sEventMgr.AddEvent(this, &Spell::HandleEffects, guid, i, EVENT_SPELL_HIT, float2int32(time), 1, 0);
            AddRef();
        }
    }
}

void Spell::HandleModeratedTarget(uint64 guid)
{
    if (m_spellInfo->getSpeed() == 0)  //instant
    {
        AddRef();
        HandleModeratedEffects(guid);
    }
    else
    {
        float destx, desty, destz, dist = 0;

        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            auto destination = m_targets.destination();
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
            HandleModeratedEffects(guid);
        }
        else
        {
            float time = dist * 1000.0f / m_spellInfo->getSpeed();
            //todo: arcemu doesn't support reflected spells
            //if (reflected)
            //	time *= 1.25; //reflected projectiles move back 4x faster
            sEventMgr.AddEvent(this, &Spell::HandleModeratedEffects, guid, EVENT_SPELL_HIT, float2int32(time), 1, 0);
            AddRef();
        }
    }
}

void Spell::HandleModeratedEffects(uint64 guid)
{
    //note: because this was a miss etc, we don't need to do attackable target checks
    if (u_caster != nullptr && u_caster->GetMapMgr() != nullptr)
    {
        Object* obj = u_caster->GetMapMgr()->_GetObject(guid);

        if (obj != nullptr && obj->isCreature() && !(m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
        {
            static_cast<Creature*>(obj)->GetAIInterface()->AttackReaction(u_caster, 0, 0);
            static_cast<Creature*>(obj)->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, u_caster, 0);
        }
    }

    DecRef();
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

    if (m_targets.m_targetMask & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

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
            auto source = m_targets.source();
            x = source.x;
            y = source.y;
            z = source.z;
        }
        if (m_targets.hasDestination())
        {
            auto destination = m_targets.destination();
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
    if (m_targets.m_targetMask & TARGET_FLAG_UNIT)
    {
        Object* uobj = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

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
    else if (m_targets.m_targetMask & (TARGET_FLAG_SOURCE_LOCATION | TARGET_FLAG_DEST_LOCATION))
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        //this can also jump to a point
        if (m_targets.m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        {
            auto source = m_targets.source();
            x = source.x;
            y = source.y;
            z = source.z;
        }

        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            auto destination = m_targets.destination();
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

    m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    m_targets.setDestination(LocationVector(newx, newy, newz));
}
