/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2005-2012 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "VMapFactory.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "SpellTarget.h"
#include "Spell.h"
#include "Objects/GameObject.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Definitions/SpellDidHitResult.h"
#include "Definitions/SpellEffectTarget.h"
#include "SpellHelpers.h"
#include "Units/Creatures/Pet.h"

uint32_t g_spellImplicitTargetFlags[MAX_IMPLICIT_TARGET_VALUE];

void InitImplicitTargetFlags()
{
    memset(g_spellImplicitTargetFlags, 0, sizeof(uint32) * MAX_IMPLICIT_TARGET_VALUE);

#define SET_TARGET_TYPE(ind, val) g_spellImplicitTargetFlags[ind] = val;

    SET_TARGET_TYPE(0, SPELL_TARGET_REQUIRE_ITEM | SPELL_TARGET_REQUIRE_GAMEOBJECT);
    SET_TARGET_TYPE(1, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(3, SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(4, SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(5, SPELL_TARGET_OBJECT_CURPET);
    SET_TARGET_TYPE(6, SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(7, SPELL_TARGET_OBJECT_SCRIPTED);
    SET_TARGET_TYPE(8, SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(15, SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(16, SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    //SET_TARGET_TYPE(17, SPELL_TARGET_AREA);
    SET_TARGET_TYPE(18, SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT);
    SET_TARGET_TYPE(20, SPELL_TARGET_AREA_PARTY);
    SET_TARGET_TYPE(21, SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(22, SPELL_TARGET_AREA_SELF);
    SET_TARGET_TYPE(23, SPELL_TARGET_REQUIRE_GAMEOBJECT);
    SET_TARGET_TYPE(24, SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(25, SPELL_TARGET_ANY_OBJECT);
    SET_TARGET_TYPE(26, SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ITEM);
    SET_TARGET_TYPE(27, SPELL_TARGET_OBJECT_PETOWNER);
    SET_TARGET_TYPE(28, SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(29, SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_SELF);
    SET_TARGET_TYPE(30, SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(31, SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA);
    //SET_TARGET_TYPE(32, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(33, SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY);
    SET_TARGET_TYPE(35, SPELL_TARGET_AREA_PARTY);
    SET_TARGET_TYPE(36, SPELL_TARGET_OBJECT_SCRIPTED);
    SET_TARGET_TYPE(37, SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID);
    SET_TARGET_TYPE(39, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(40, SPELL_TARGET_OBJECT_SCRIPTED);
    SET_TARGET_TYPE(41, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(42, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(43, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(44, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(45, SPELL_TARGET_AREA_CHAIN | SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(46, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(47, SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT); //dont fill target map for this (fucks up some spell visuals)
    SET_TARGET_TYPE(48, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(49, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(50, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(52, SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(34, SPELL_TARGET_NOT_IMPLEMENTED); //seige stuff
    SET_TARGET_TYPE(53, SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(54, SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(56, SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID); //used by commanding shout, targets raid now
    SET_TARGET_TYPE(57, SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA_PARTY);
    SET_TARGET_TYPE(61, SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID | SPELL_TARGET_OBJECT_TARCLASS | SPELL_TARGET_REQUIRE_FRIENDLY);
    SET_TARGET_TYPE(63, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(64, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(65, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(66, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(67, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(69, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(72, SPELL_TARGET_AREA_RANDOM);
    SET_TARGET_TYPE(73, SPELL_TARGET_OBJECT_SELF);
    SET_TARGET_TYPE(76, SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(77, SPELL_TARGET_REQUIRE_ATTACKABLE);
    SET_TARGET_TYPE(86, SPELL_TARGET_AREA_RANDOM);
    SET_TARGET_TYPE(87, SPELL_TARGET_AREA);
    SET_TARGET_TYPE(90, SPELL_TARGET_OBJECT_CURCRITTER);
    SET_TARGET_TYPE(104, SPELL_TARGET_REQUIRE_ATTACKABLE | SPELL_TARGET_AREA_CONE);

#undef SET_TARGET_TYPE
}

void Spell::FillTargetMap(uint32 i)
{
    //Spell::prepare() has already a m_caster->IsInWorld() check so if now the caster is no more in world something bad happened.
    ARCEMU_ASSERT(m_caster->IsInWorld());

    uint32 TargetType = 0;
    TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetA(static_cast<uint8_t>(i)), i);

    //never get info from B if it is 0 :P
    if (m_spellInfo->getEffectImplicitTargetB(static_cast<uint8_t>(i)) != EFF_TARGET_NONE)
        TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetB(static_cast<uint8_t>(i)), i);

    if (TargetType & SPELL_TARGET_NOT_IMPLEMENTED)
        return;
    if (TargetType & SPELL_TARGET_NO_OBJECT)  //summon spells that appear infront of caster
    {
        HandleTargetNoObject();
        return;
    }

    //always add this guy :P
    if (!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_OBJECT_PETOWNER)))
    {
        Object* target = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
        AddTarget(i, TargetType, target);
    }

    if (TargetType & SPELL_TARGET_OBJECT_SELF)
        AddTarget(i, TargetType, m_caster);
    if (TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF))  //targetted aoe
        AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets());
    ///\todo arcemu, doesn't support summon slots?
    /*if (TargetType & SPELL_TARGET_OBJECT_CURTOTEMS && u_caster != NULL)
        for (uint32 i=1; i<5; ++i) //totem slots are 1, 2, 3, 4
        AddTarget(i, TargetType, u_caster->m_summonslot[i]);*/
    if (TargetType & SPELL_TARGET_OBJECT_CURPET && p_caster != nullptr)
        AddTarget(i, TargetType, p_caster->GetSummon());
    if (TargetType & SPELL_TARGET_OBJECT_PETOWNER)
    {
        uint64 guid = m_targets.m_unitTarget;
        if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_PET)
        {
            Pet* p = m_caster->GetMapMgr()->GetPet(GET_LOWGUID_PART(guid));
            if (p != nullptr)
                AddTarget(i, TargetType, p->GetPetOwner());
        }
    }
    //targets party, not raid
    if ((TargetType & SPELL_TARGET_AREA_PARTY) && !(TargetType & SPELL_TARGET_AREA_RAID))
    {
        if (p_caster == nullptr && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets()); //npcs
        else
            AddPartyTargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets()); //players/pets/totems
    }
    if (TargetType & SPELL_TARGET_AREA_RAID)
    {
        if (p_caster == nullptr && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets()); //npcs
        else
            AddRaidTargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets(), (TargetType & SPELL_TARGET_AREA_PARTY) ? true : false); //players/pets/totems
    }
    if (TargetType & SPELL_TARGET_AREA_CHAIN)
        AddChainTargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets());
    //target cone
    if (TargetType & SPELL_TARGET_AREA_CONE)
        AddConeTargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets());

    if (TargetType & SPELL_TARGET_OBJECT_SCRIPTED)
        AddScriptedOrSpellFocusTargets(i, TargetType, GetRadius(i), m_spellInfo->getMaxTargets());
}

void Spell::AddScriptedOrSpellFocusTargets(uint32 i, uint32 targetType, float r, uint32 /*maxtargets*/)
{
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        Object* o = itr;
        if (!o || !o->IsGameObject())
            continue;

        GameObject* go = static_cast<GameObject*>(o);
        if (go->GetGameObjectProperties()->raw.parameter_0 == m_spellInfo->getRequiresSpellFocus())
        {
            if (!m_caster->isInRange(go, r))
                continue;

            bool success = AddTarget(i, targetType, go);

            if (success)
                return;
        }
    }
}

void Spell::AddConeTargets(uint32 i, uint32 targetType, float /*r*/, uint32 maxtargets)
{
    std::vector<uint64_t>* list = &m_targetUnits[i];
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr || !itr->IsUnit() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //is Creature in range
        if (m_caster->isInRange(itr, GetRadius(i)))
        {
            if (m_spellInfo->cone_width ? m_caster->isInArc(itr, m_spellInfo->cone_width) : m_caster->isInFront(itr))  // !!! is the target within our cone ?
            {
                AddTarget(i, targetType, itr);
            }
        }
        if (maxtargets != 0 && list->size() >= maxtargets)
            return;
    }
}

void Spell::AddChainTargets(uint32 i, uint32 targetType, float /*r*/, uint32 /*maxtargets*/)
{
    if (!m_caster->IsInWorld())
        return;

    Object* targ = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

    if (targ == nullptr)
        return;

    std::vector<uint64_t>* list = &m_targetUnits[i];

    //if selected target is party member, then jumps on party
    Unit* firstTarget = nullptr;

    if (targ->IsUnit())
        firstTarget = static_cast<Unit*>(targ);
    else
        firstTarget = u_caster;

    bool RaidOnly = false;
    float range = GetMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->getRangeIndex()));//this is probably wrong,
    //this is cast distance, not searching distance
    range *= range;

    //is this party only?
    Player* casterFrom = static_cast< Player* >(u_caster->GetPlayerOwner());
    Player* pfirstTargetFrom = static_cast< Player* >(firstTarget->GetPlayerOwner());
    if (casterFrom != nullptr && pfirstTargetFrom != nullptr && casterFrom->GetGroup() == pfirstTargetFrom->GetGroup())
        RaidOnly = true;

    uint32 jumps = m_spellInfo->getEffectChainTarget(static_cast<uint8_t>(i));

    //range
    range /= jumps; //hacky, needs better implementation!

    ascemu::World::Spell::Helpers::spellModFlatIntValue(u_caster->SM_FAdditionalTargets, (int32*)&jumps, m_spellInfo->getSpellGroupType());

    AddTarget(i, targetType, firstTarget);

    if (jumps <= 1 || list->size() == 0) //1 because we've added the first target, 0 size if spell is resisted
        return;

    for (const auto& itr : firstTarget->getInRangeObjectsSet())
    {
        auto obj = itr;
        if (!obj || !itr->IsUnit() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        if (RaidOnly && !pfirstTargetFrom->InRaid(static_cast<Unit*>(itr)))
            continue;

        //healing spell, full health target = NONO
        if (m_spellInfo->isHealingSpell() && static_cast<Unit*>(itr)->GetHealthPct() == 100)
            continue;

        size_t oldsize;

        if (obj->isInRange(firstTarget->GetPositionX(), firstTarget->GetPositionY(), firstTarget->GetPositionZ(), range))
        {
            oldsize = list->size();
            AddTarget(i, targetType, itr);
            if (list->size() == oldsize || list->size() >= jumps) //either out of jumps or a resist
                return;
        }
    }
}

void Spell::AddPartyTargets(uint32 i, uint32 targetType, float r, uint32 /*maxtargets*/)
{
    Object* u = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
    if (u == nullptr)
        u = m_caster;
    Player* p = static_cast< Player* >(u->GetPlayerOwner());

    if (p == nullptr || u_caster == nullptr)
        return;

    AddTarget(i, targetType, p);

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        if (!itr || !itr->IsUnit() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //only affect players and pets
        if (!itr->IsPlayer() && !itr->IsPet())
            continue;

        if (!p->InParty(static_cast<Unit*>(itr)))
            continue;

        if (u->CalcDistance(itr) > r)
            continue;

        AddTarget(i, targetType, itr);
    }
}

void Spell::AddRaidTargets(uint32 i, uint32 targetType, float r, uint32 /*maxtargets*/, bool /*partylimit*/)
{
    Object* u = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
    if (u == nullptr)
        u = m_caster;
    Player* p = static_cast< Player* >(u->GetPlayerOwner());

    if (p == nullptr || u_caster == nullptr)
        return;

    AddTarget(i, targetType, p);

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        if (!itr || !itr->IsUnit() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //only affect players and pets
        if (!itr->IsPlayer() && !itr->IsPet())
            continue;

        if (!p->InRaid(static_cast<Unit*>(itr)))
            continue;

        if (u->CalcDistance(itr) > r)
            continue;

        AddTarget(i, targetType, itr);
    }
}

void Spell::AddAOETargets(uint32 i, uint32 targetType, float r, uint32 maxtargets)
{
    LocationVector source;

    //cant do raid/party stuff here, seperate functions for it
    if (targetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && !(p_caster == nullptr && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem())))
        return;

    Object* tarobj = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

    if (targetType & SPELL_TARGET_AREA_SELF)
        source = m_caster->GetPosition();
    else if (targetType & SPELL_TARGET_AREA_CURTARGET && tarobj != nullptr)
        source = tarobj->GetPosition();
    else
    {
        m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
        source = m_targets.destination();
    }

    //caster might be in the aoe LOL
    if (m_caster->CalcDistance(source) <= r)
        AddTarget(i, targetType, m_caster);

    std::vector<uint64_t>* t = &m_targetUnits[i];

    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr)
            continue;

        if (maxtargets != 0 && t->size() >= maxtargets)
            break;

        float dist = itr->CalcDistance(source);
        if (dist <= r)
            AddTarget(i, targetType, itr);
    }
}

bool Spell::AddTarget(uint32 i, uint32 TargetType, Object* obj)
{
    std::vector<uint64_t>* t = &m_targetUnits[i];

    if (obj == nullptr || !obj->IsInWorld())
        return false;

    //GO target, not item
    if ((TargetType & SPELL_TARGET_REQUIRE_GAMEOBJECT) && !(TargetType & SPELL_TARGET_REQUIRE_ITEM) && !obj->IsGameObject())
        return false;

    //target go, not able to target go
    if (obj->IsGameObject() && !(TargetType & SPELL_TARGET_OBJECT_SCRIPTED) && !(TargetType & SPELL_TARGET_REQUIRE_GAMEOBJECT) && !m_triggeredSpell)
        return false;
    //target item, not able to target item
    if (obj->IsItem() && !(TargetType & SPELL_TARGET_REQUIRE_ITEM) && !m_triggeredSpell)
        return false;

    if (u_caster != nullptr && u_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT) && ((obj->IsPlayer() || obj->IsPet()) || (p_caster != nullptr || m_caster->IsPet())))
        return false;

    if (TargetType & SPELL_TARGET_REQUIRE_FRIENDLY && !isFriendly(m_caster, obj))
        return false;
    if (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !isAttackable(m_caster, obj, false))
        return false;
    if (TargetType & SPELL_TARGET_OBJECT_TARCLASS)
    {
        Object* originaltarget = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

        if (originaltarget == nullptr || (originaltarget->IsPlayer() && obj->IsPlayer() && static_cast<Player*>(originaltarget)->getClass() != static_cast<Player*>(obj)->getClass()) || (originaltarget->IsPlayer() && !obj->IsPlayer()) || (!originaltarget->IsPlayer() && obj->IsPlayer()))
            return false;
    }
    if (TargetType & SPELL_TARGET_OBJECT_CURPET && !obj->IsPet())
        return false;
    if (TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && ((obj->IsUnit() && !static_cast<Unit*>(obj)->isAlive()) || (obj->IsCreature() && obj->IsTotem())))
        return false;

    uint8 hitresult = (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && obj->IsUnit()) ? DidHit(i, static_cast<Unit*>(obj)) : SPELL_DID_HIT_SUCCESS;
    if (hitresult != SPELL_DID_HIT_SUCCESS)
    {
        uint8 extended = 0;
        if (hitresult == SPELL_DID_HIT_REFLECT && u_caster != nullptr)
        {
            //for checks
            Unit* tmp = u_caster;
            u_caster = static_cast<Unit*>(obj);
            extended = DidHit(i, tmp);
            u_caster = tmp;
        }
        ModeratedTargets.push_back(SpellTargetMod(obj->GetGUID(), hitresult));
        return false;
    }
    else
    {
        //check target isnt already in
        for (std::vector<uint64_t>::iterator itr = m_targetUnits[i].begin(); itr != m_targetUnits[i].end(); ++itr)
        {
            if (obj->GetGUID() == *itr)
                return false;
        }
        t->push_back(obj->GetGUID());
    }

    //final checks, require line of sight unless range/radius is 50000 yards
    auto spell_range = sSpellRangeStore.LookupEntry(m_spellInfo->getRangeIndex());
    if (spell_range != nullptr)
    {
        if (worldConfig.terrainCollision.isCollisionEnabled && spell_range->maxRange < 50000 && GetRadius(i) < 50000 && !obj->IsItem())
        {
            float x = m_caster->GetPositionX(), y = m_caster->GetPositionY(), z = m_caster->GetPositionZ() + 0.5f;

            //are we using a different location?
            if (TargetType & SPELL_TARGET_AREA)
            {
                auto destination = m_targets.destination();
                x = destination.x;
                y = destination.y;
                z = destination.z;
            }
            else if (TargetType & SPELL_TARGET_AREA_CHAIN)
            {
                ///\todo Add support for this in arcemu
                /*Object* lasttarget = NULL;
                if (m_orderedObjects.size() > 0)
                {
                lasttarget = m_caster->GetMapMgr()->_GetObject(m_orderedObjects[m_orderedObjects.size() - 1]);
                if (lasttarget != NULL)
                {
                x = lasttarget->GetPositionX();
                y = lasttarget->GetPositionY();
                z = lasttarget->GetPositionZ();
                }
                }*/
            }

            VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
            bool isInLOS = mgr->isInLineOfSight(m_caster->GetMapId(), x, y, z + 2.0f, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + 2.0f);

            if (!isInLOS)
                return false;
        }
    }

    return true;
}

bool Spell::GenerateTargets(SpellCastTargets* t)
{
    if (u_caster == nullptr || u_caster->GetAIInterface() == nullptr || !u_caster->IsInWorld())
        return false;

    bool result = false;

    for (uint8 i = 0; i < 3; ++i)
    {
        if (m_spellInfo->getEffect(i) == 0)
            continue;
        uint32 TargetType = 0;
        TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetA(i), i);

        //never get info from B if it is 0 :P
        if (m_spellInfo->getEffectImplicitTargetB(i) != EFF_TARGET_NONE)
            TargetType |= GetTargetType(m_spellInfo->getEffectImplicitTargetB(i), i);

        if (TargetType & (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        {
            t->m_targetMask |= TARGET_FLAG_UNIT;
            t->m_unitTarget = u_caster->GetGUID();
            result = true;
        }

        if (TargetType & SPELL_TARGET_NO_OBJECT)
        {
            t->m_targetMask = TARGET_FLAG_SELF;
            result = true;
        }

        if (!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE)))
        {

            if (TargetType & SPELL_TARGET_ANY_OBJECT)
            {
                if (u_caster->getUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->GetMapMgr()->_GetObject(u_caster->getUInt64Value(UNIT_FIELD_TARGET));
                    if (target != nullptr)
                    {
                        if (target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if (target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
            }

            if (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if (u_caster->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->GetMapMgr()->_GetObject(u_caster->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
                    if (target != nullptr)
                    {
                        if (target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if (target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                }
                else if (u_caster->getUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->GetMapMgr()->_GetObject(u_caster->getUInt64Value(UNIT_FIELD_TARGET));
                    if (target != nullptr)
                    {
                        if (target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if (target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
                else if (u_caster->IsCreature() && u_caster->IsTotem())
                {
                    Unit* target = u_caster->GetMapMgr()->GetUnit(GetSinglePossibleEnemy(i));
                    if (target != nullptr)
                    {
                        t->m_targetMask |= TARGET_FLAG_UNIT;
                        t->m_unitTarget = target->GetGUID();
                    }
                }
            }

            if (TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                Unit* target = u_caster->GetMapMgr()->GetUnit(GetSinglePossibleFriend(i));
                if (target != nullptr)
                {
                    t->m_targetMask |= TARGET_FLAG_UNIT;
                    t->m_unitTarget = target->GetGUID();
                    result = true;
                }
                else
                {
                    t->m_targetMask |= TARGET_FLAG_UNIT;
                    t->m_unitTarget = u_caster->GetGUID();
                    result = true;
                }
            }
        }

        if (TargetType & SPELL_TARGET_AREA_RANDOM)
        {
            //we always use radius(0) for some reason
            bool isInLOS = false;
            uint8 attempts = 0;
            do
            {
                //prevent deadlock
                ++attempts;
                if (attempts > 10)
                    return false;

                float r = Util::getRandomFloat(GetRadius(0));
                float ang = Util::getRandomFloat(M_PI_FLOAT * 2);
                auto lv = LocationVector();
                lv.x = m_caster->GetPositionX() + (cosf(ang) * r);
                lv.y = m_caster->GetPositionY() + (sinf(ang) * r);
                lv.z = m_caster->GetMapMgr()->GetLandHeight(lv.x, lv.y, m_caster->GetPositionZ() + 2.0f);
                t->setDestination(lv);
                t->m_targetMask = TARGET_FLAG_DEST_LOCATION;

                VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                isInLOS = mgr->isInLineOfSight(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), lv.x, lv.y, lv.z);
            }
            while (worldConfig.terrainCollision.isCollisionEnabled && !isInLOS);
            result = true;
        }
        else if (TargetType & SPELL_TARGET_AREA)  //targetted aoe
        {
            //spells like blizzard, rain of fire
            if (u_caster->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
            {
                Object* target = u_caster->GetMapMgr()->_GetObject(u_caster->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
                if (target)
                {
                    t->m_targetMask |= TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
                    t->setDestination(target->GetPosition());
                }
                result = true;
            }
            else
            {
                if (u_caster->GetAIInterface()->getNextTarget() != nullptr && TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
                {
                    t->m_targetMask |= TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
                    t->setDestination(u_caster->GetAIInterface()->getNextTarget()->GetPosition());
                    result = true;
                }
                else if (TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
                {
                    t->m_targetMask |= TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
                    t->setDestination(u_caster->GetPosition());
                    result = true;
                }
            }
        }
        else if (TargetType & SPELL_TARGET_AREA_SELF)
        {
            t->m_targetMask |= TARGET_FLAG_SOURCE_LOCATION | TARGET_FLAG_UNIT;
            t->m_unitTarget = u_caster->GetGUID();
            t->setSource(u_caster->GetPosition());
            t->setDestination(u_caster->GetPosition());
            result = true;
        }

        if (TargetType & SPELL_TARGET_AREA_CHAIN)
        {
            if (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if (u_caster->GetAIInterface()->getNextTarget() != nullptr)
                {
                    t->m_targetMask |= TARGET_FLAG_UNIT;
                    t->m_unitTarget = u_caster->GetAIInterface()->getNextTarget()->GetGUID();
                    result = true;
                }
            }
            else
            {
                t->m_targetMask |= TARGET_FLAG_UNIT;
                t->m_unitTarget = u_caster->GetGUID();
                result = true;
            }
        }
        //target cone
        if (TargetType & SPELL_TARGET_AREA_CONE)
        {
            if (u_caster->GetAIInterface()->getNextTarget() != nullptr)
            {
                t->m_targetMask |= TARGET_FLAG_DEST_LOCATION;
                t->setDestination(u_caster->GetAIInterface()->getNextTarget()->GetPosition());
                result = true;
            }
        }
    }
    return result;
}
