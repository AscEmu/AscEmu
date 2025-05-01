/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>

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

#include "VMapManager2.h"
#include "Map/Management/MapMgr.hpp"
#include "SpellTarget.h"
#include "Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Objects/GameObject.h"
#include "Server/World.h"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "Definitions/SpellDidHitResult.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

void Spell::FillTargetMap(uint32_t i)
{
    //Spell::prepare() has already a m_caster->IsInWorld() check so if now the caster is no more in world something bad happened.
    if (!m_caster->IsInWorld())
        return;

    uint32_t TargetType = 0;
    TargetType |= getSpellInfo()->getRequiredTargetMaskForEffect(static_cast<uint8_t>(i));

    if (TargetType & SPELL_TARGET_NOT_IMPLEMENTED)
        return;
    if (TargetType == SPELL_TARGET_NO_OBJECT)  //summon spells that appear infront of caster
    {
        HandleTargetNoObject();
        return;
    }

    //always add this guy :P
    if (!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_OBJECT_PETOWNER)))
    {
        Object* target = nullptr;
        if (TargetType & SPELL_TARGET_REQUIRE_GAMEOBJECT)
            target = m_caster->getWorldMapObject(m_targets.getGameObjectTargetGuid());
        else if (TargetType & SPELL_TARGET_REQUIRE_ITEM)
            target = m_caster->getWorldMapObject(m_targets.getItemTargetGuid());

        // If target was not found, try unit
        if (target == nullptr)
            target = m_caster->getWorldMapObject(m_targets.getUnitTargetGuid());

        AddTarget(i, TargetType, target);
    }

    if (TargetType & SPELL_TARGET_OBJECT_SELF)
        AddTarget(i, TargetType, m_caster);
    if (TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF))  //targetted aoe
        AddAOETargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets());
    ///\todo arcemu, doesn't support summon slots?
    /*if (TargetType & SPELL_TARGET_OBJECT_CURTOTEMS && u_caster != NULL)
        for (uint32_t i=1; i<5; ++i) //totem slots are 1, 2, 3, 4
        AddTarget(i, TargetType, u_caster->m_summonslot[i]);*/
    if (TargetType & SPELL_TARGET_OBJECT_CURPET && u_caster != nullptr)
        AddTarget(i, TargetType, u_caster->getPet());
    if (TargetType & SPELL_TARGET_OBJECT_PETOWNER)
    {
        WoWGuid wowGuid;
        wowGuid.Init(m_targets.getUnitTargetGuid());
        if (wowGuid.isPet())
        {
            Pet* p = m_caster->getWorldMap()->getPet(wowGuid.getGuidLowPart());
            if (p != nullptr)
                AddTarget(i, TargetType, p->getUnitOwner());
        }
    }
    //targets party, not raid
    if ((TargetType & SPELL_TARGET_AREA_PARTY) && !(TargetType & SPELL_TARGET_AREA_RAID))
    {
        if (p_caster == nullptr && !m_caster->isPet() && (!m_caster->isCreature() || !m_caster->isTotem()))
            AddAOETargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets()); //npcs
        else
            AddPartyTargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets()); //players/pets/totems
    }
    if (TargetType & SPELL_TARGET_AREA_RAID)
    {
        if (p_caster == nullptr && !m_caster->isPet() && (!m_caster->isCreature() || !m_caster->isTotem()))
            AddAOETargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets()); //npcs
        else
            AddRaidTargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets(), (TargetType & SPELL_TARGET_AREA_PARTY) ? true : false); //players/pets/totems
    }
    if (TargetType & SPELL_TARGET_AREA_CHAIN)
        AddChainTargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets());
    //target cone
    if (TargetType & SPELL_TARGET_AREA_CONE)
        AddConeTargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets());

    if (TargetType & SPELL_TARGET_OBJECT_SCRIPTED)
        AddScriptedOrSpellFocusTargets(i, TargetType, getEffectRadius(i), m_spellInfo->getMaxTargets());
}

void Spell::AddScriptedOrSpellFocusTargets(uint32_t i, uint32_t targetType, float r, uint32_t /*maxtargets*/)
{
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        Object* o = itr;
        if (!o || !o->isGameObject())
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

void Spell::AddConeTargets(uint32_t i, uint32_t targetType, float /*r*/, uint32_t maxtargets)
{
    std::vector<uint64_t>* list = &m_effectTargets[i];
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //is Creature in range
        if (m_caster->isInRange(itr, getEffectRadius(i)))
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

void Spell::AddChainTargets(uint32_t i, uint32_t targetType, float /*r*/, uint32_t /*maxtargets*/)
{
    if (!m_caster->IsInWorld())
        return;

    Object* targ = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());

    if (targ == nullptr)
        return;

    std::vector<uint64_t>* list = &m_effectTargets[i];

    //if selected target is party member, then jumps on party
    Unit* firstTarget = nullptr;

    if (targ->isCreatureOrPlayer())
        firstTarget = static_cast<Unit*>(targ);
    else
        firstTarget = u_caster;

    bool RaidOnly = false;
    float range = m_spellInfo->getMaxRange(false, m_caster, this);//this is probably wrong,
    //this is cast distance, not searching distance
    range *= range;

    //is this party only?
    Player* casterFrom = u_caster->getPlayerOwnerOrSelf();
    Player* pfirstTargetFrom = firstTarget->getPlayerOwnerOrSelf();
    if (casterFrom != nullptr && pfirstTargetFrom != nullptr && casterFrom->getGroup() == pfirstTargetFrom->getGroup())
        RaidOnly = true;

    uint32_t jumps = m_spellInfo->getEffectChainTarget(static_cast<uint8_t>(i));

    //range
    range /= jumps; //hacky, needs better implementation!

    u_caster->applySpellModifiers(SPELLMOD_ADDITIONAL_TARGET, &jumps, getSpellInfo(), this);

    AddTarget(i, targetType, firstTarget);

    if (jumps <= 1 || list->size() == 0) //1 because we've added the first target, 0 size if spell is resisted
        return;

    for (const auto& itr : firstTarget->getInRangeObjectsSet())
    {
        auto obj = itr;
        if (!obj || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        if (RaidOnly && !pfirstTargetFrom->isUnitOwnerInRaid(static_cast<Unit*>(itr)))
            continue;

        //healing spell, full health target = NONO
        if (m_spellInfo->isHealingSpell() && static_cast<Unit*>(itr)->getHealthPct() == 100)
            continue;

        if (obj->isInRange(firstTarget->GetPositionX(), firstTarget->GetPositionY(), firstTarget->GetPositionZ(), range))
        {
            size_t oldsize = list->size();
            AddTarget(i, targetType, itr);
            if (list->size() == oldsize || list->size() >= jumps) //either out of jumps or a resist
                return;
        }
    }
}

void Spell::AddPartyTargets(uint32_t i, uint32_t targetType, float r, uint32_t /*maxtargets*/)
{
    Object* u = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());
    if (u == nullptr)
        u = m_caster;

    // If spell has area aura effect, aura code will handle proper targetting
    // so add just caster
    if (getSpellInfo()->isAreaAuraEffect(i))
    {
        AddTarget(i, targetType, u);
        return;
    }

    Player* p = u->getPlayerOwnerOrSelf();
    if (p == nullptr || u_caster == nullptr)
        return;

    AddTarget(i, targetType, p);

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //only affect players and pets
        if (!itr->isPlayer() && !itr->isPet())
            continue;

        if (!p->isUnitOwnerInParty(static_cast<Unit*>(itr)))
            continue;

        if (u->CalcDistance(itr) > r)
            continue;

        AddTarget(i, targetType, itr);
    }
}

void Spell::AddRaidTargets(uint32_t i, uint32_t targetType, float r, uint32_t /*maxtargets*/, bool /*partylimit*/)
{
    Object* u = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());
    if (u == nullptr)
        u = m_caster;

    // If spell has area aura effect, aura code will handle proper targetting
    // so add just caster
    if (getSpellInfo()->isAreaAuraEffect(i))
    {
        AddTarget(i, targetType, u);
        return;
    }

    Player* p = u->getPlayerOwnerOrSelf();
    if (p == nullptr || u_caster == nullptr)
        return;

    AddTarget(i, targetType, p);

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        //only affect players and pets
        if (!itr->isPlayer() && !itr->isPet())
            continue;

        if (!p->isUnitOwnerInRaid(static_cast<Unit*>(itr)))
            continue;

        if (u->CalcDistance(itr) > r)
            continue;

        AddTarget(i, targetType, itr);
    }
}

void Spell::AddAOETargets(uint32_t i, uint32_t targetType, float r, uint32_t maxtargets)
{
    LocationVector source;

    //cant do raid/party stuff here, seperate functions for it
    if (targetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && !(p_caster == nullptr && !m_caster->isPet() && (!m_caster->isCreature() || !m_caster->isTotem())))
        return;

    Object* tarobj = m_caster->getWorldMap()->getObject(m_targets.getUnitTargetGuid());

    if (targetType & SPELL_TARGET_AREA_SELF)
        source = m_caster->GetPosition();
    else if (targetType & SPELL_TARGET_AREA_CURTARGET && tarobj != nullptr)
        source = tarobj->GetPosition();
    else
    {
        if (!m_targets.getDestination().isSet())
        {
            // If position is not set, try unit target's position
            if (m_targets.getUnitTargetGuid() != 0)
            {
                const auto targetUnit = m_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());
                if (targetUnit != nullptr)
                    m_targets.setDestination(targetUnit->GetPosition());
            }

            // Check again if position was set
            if (!m_targets.getDestination().isSet())
            {
                // No unit target => use caster's position
                m_targets.setDestination(m_caster->GetPosition());
            }
        }

        source = m_targets.getDestination();
    }

    //caster might be in the aoe LOL
    if (!(targetType & SPELL_TARGET_REQUIRE_ATTACKABLE))
    {
        if (m_caster->CalcDistance(source) <= r)
            AddTarget(i, targetType, m_caster);
    }

    std::vector<uint64_t>* t = &m_effectTargets[i];

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

bool Spell::AddTarget(uint32_t i, uint32_t TargetType, Object* obj)
{
    const auto targetCheck = checkExplicitTarget(obj, TargetType);
    if (targetCheck != SPELL_CAST_SUCCESS)
        return false;

    // If checked in checkExplicitTarget, initial aoe spell cast check can fail
    if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_PLAYERS && !obj->isPlayer())
        return false;

    if (u_caster != nullptr && u_caster->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT) && ((obj->isPlayer() || obj->isPet()) || (p_caster != nullptr || m_caster->isPet())))
        return false;

    std::vector<uint64_t>* t = &m_effectTargets[i];

    SpellDidHitResult hitresult = (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && obj->isCreatureOrPlayer()) ? static_cast<SpellDidHitResult>(DidHit(i, static_cast<Unit*>(obj))) : SPELL_DID_HIT_SUCCESS;
    if (hitresult != SPELL_DID_HIT_SUCCESS)
    {
        SpellDidHitResult extended = SPELL_DID_HIT_SUCCESS;
        if (hitresult == SPELL_DID_HIT_REFLECT && u_caster != nullptr)
        {
            //for checks
            Unit* tmp = u_caster;
            u_caster = static_cast<Unit*>(obj);
            extended = static_cast<SpellDidHitResult>(DidHit(i, tmp));
            u_caster = tmp;
        }
        safeAddMissedTarget(obj->getGuid(), hitresult, extended);
        return false;
    }
    else
    {
        //check target isnt already in
        for (std::vector<uint64_t>::iterator itr = m_effectTargets[i].begin(); itr != m_effectTargets[i].end(); ++itr)
        {
            if (obj->getGuid() == *itr)
                return false;
        }
        t->push_back(obj->getGuid());
    }

    //final checks, require line of sight unless range/radius is 50000 yards
    auto spell_range = sSpellRangeStore.lookupEntry(m_spellInfo->getRangeIndex());
    if (spell_range != nullptr)
    {
        if (worldConfig.terrainCollision.isCollisionEnabled && spell_range->maxRange < 50000 && getEffectRadius(i) < 50000 && !obj->isItem())
        {
            float x = m_caster->GetPositionX(), y = m_caster->GetPositionY(), z = m_caster->GetPositionZ() + 0.5f;

            //are we using a different location?
            if (TargetType & SPELL_TARGET_AREA)
            {
                auto destination = m_targets.getDestination();
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
                lasttarget = m_caster->getWorldMap()->_GetObject(m_orderedObjects[m_orderedObjects.size() - 1]);
                if (lasttarget != NULL)
                {
                x = lasttarget->GetPositionX();
                y = lasttarget->GetPositionY();
                z = lasttarget->GetPositionZ();
                }
                }*/
            }

            bool isInLOS = m_caster->getWorldMap()->isInLineOfSight(LocationVector(x, y, z), obj->GetPosition(), m_caster->GetPhase(), LineOfSightChecks::LINEOFSIGHT_ALL_CHECKS);

            if (!isInLOS)
                return false;
        }
    }

    return true;
}

bool Spell::GenerateTargets(SpellCastTargets* t)
{
    if (u_caster == nullptr || u_caster->getAIInterface() == nullptr || !u_caster->IsInWorld())
        return false;

    bool result = false;

    for (uint8_t i = 0; i < 3; ++i)
    {
        if (m_spellInfo->getEffect(i) == 0)
            continue;
        uint32_t TargetType = 0;
        TargetType |= getSpellInfo()->getRequiredTargetMaskForEffect(i);

        if (TargetType & (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        {
            t->addTargetMask(TARGET_FLAG_UNIT);
            t->setUnitTarget(u_caster->getGuid());
            result = true;
        }

        if (TargetType & SPELL_TARGET_NO_OBJECT)
        {
            t->setTargetMask(TARGET_FLAG_SELF);
            result = true;
        }

        if (!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE)))
        {

            if (TargetType & SPELL_TARGET_ANY_OBJECT)
            {
                if (u_caster->getTargetGuid())
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->getWorldMap()->getObject(u_caster->getTargetGuid());
                    if (target != nullptr)
                    {
                        if (target->isCreatureOrPlayer())
                        {
                            t->addTargetMask(TARGET_FLAG_UNIT);
                            t->setUnitTarget(target->getGuid());
                            result = true;
                        }
                        else if (target->isGameObject())
                        {
                            t->addTargetMask(TARGET_FLAG_OBJECT);
                            t->setGameObjectTarget(target->getGuid());
                            result = true;
                        }
                    }
                    result = true;
                }
            }

            if (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if (u_caster->getChannelObjectGuid())
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->getWorldMap()->getObject(u_caster->getChannelObjectGuid());
                    if (target != nullptr)
                    {
                        if (target->isCreatureOrPlayer())
                        {
                            t->addTargetMask(TARGET_FLAG_UNIT);
                            t->setUnitTarget(target->getGuid());
                            result = true;
                        }
                        else if (target->isGameObject())
                        {
                            t->addTargetMask(TARGET_FLAG_OBJECT);
                            t->setGameObjectTarget(target->getGuid());
                            result = true;
                        }
                    }
                }
                else if (u_caster->getTargetGuid())
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    Object* target = u_caster->getWorldMap()->getObject(u_caster->getTargetGuid());
                    if (target != nullptr)
                    {
                        if (target->isCreatureOrPlayer())
                        {
                            t->addTargetMask(TARGET_FLAG_UNIT);
                            t->setUnitTarget(target->getGuid());
                            result = true;
                        }
                        else if (target->isGameObject())
                        {
                            t->addTargetMask(TARGET_FLAG_OBJECT);
                            t->setGameObjectTarget(target->getGuid());
                            result = true;
                        }
                    }
                    result = true;
                }
                else if (u_caster->isCreature() && u_caster->isTotem())
                {
                    Unit* target = u_caster->getWorldMap()->getUnit(GetSinglePossibleEnemy(i));
                    if (target != nullptr)
                    {
                        t->addTargetMask(TARGET_FLAG_UNIT);
                        t->setUnitTarget(target->getGuid());
                    }
                }
            }

            if (TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                Unit* target = u_caster->getWorldMap()->getUnit(GetSinglePossibleFriend(i));
                if (target != nullptr)
                {
                    t->addTargetMask(TARGET_FLAG_UNIT);
                    t->setUnitTarget(target->getGuid());
                    result = true;
                }
                else
                {
                    t->addTargetMask(TARGET_FLAG_UNIT);
                    t->setUnitTarget(u_caster->getGuid());
                    result = true;
                }
            }
        }

        if (TargetType & SPELL_TARGET_AREA_RANDOM)
        {
            //we always use radius(0) for some reason
            bool isInLOS = false;
            uint8_t attempts = 0;
            do
            {
                //prevent deadlock
                ++attempts;
                if (attempts > 10)
                    return false;

                float r = Util::getRandomFloat(getEffectRadius(0));
                float ang = Util::getRandomFloat(M_PI_FLOAT * 2);
                auto lv = LocationVector();
                lv.x = m_caster->GetPositionX() + (cosf(ang) * r);
                lv.y = m_caster->GetPositionY() + (sinf(ang) * r);
                lv.z = m_caster->getMapHeight(LocationVector(lv.x, lv.y, m_caster->GetPositionZ() + 2.0f));
                t->setDestination(lv);
                t->setTargetMask(TARGET_FLAG_DEST_LOCATION);

                isInLOS = m_caster->IsWithinLOS(lv);
            }
            while (worldConfig.terrainCollision.isCollisionEnabled && !isInLOS);
            result = true;
        }
        else if (TargetType & SPELL_TARGET_AREA)  //targetted aoe
        {
            //spells like blizzard, rain of fire
            if (u_caster->getChannelObjectGuid())
            {
                Object* target = u_caster->getWorldMap()->getObject(u_caster->getChannelObjectGuid());
                if (target)
                {
                    t->addTargetMask(TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT);
                    t->setDestination(target->GetPosition());
                }
                result = true;
            }
            else
            {
                if (u_caster->getAIInterface()->getCurrentTarget() != nullptr && TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
                {
                    t->addTargetMask(TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT);
                    t->setDestination(u_caster->getAIInterface()->getCurrentTarget()->GetPosition());
                    result = true;
                }
                else if (TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
                {
                    t->addTargetMask(TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT);
                    t->setDestination(u_caster->GetPosition());
                    result = true;
                }
            }
        }
        else if (TargetType & SPELL_TARGET_AREA_SELF)
        {
            t->addTargetMask(TARGET_FLAG_SOURCE_LOCATION | TARGET_FLAG_UNIT);
            t->setUnitTarget(u_caster->getGuid());
            t->setSource(u_caster->GetPosition());
            t->setDestination(u_caster->GetPosition());
            result = true;
        }

        if (TargetType & SPELL_TARGET_AREA_CHAIN)
        {
            if (TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if (u_caster->getAIInterface()->getCurrentTarget() != nullptr)
                {
                    t->addTargetMask(TARGET_FLAG_UNIT);
                    t->setUnitTarget(u_caster->getAIInterface()->getCurrentTarget()->getGuid());
                    result = true;
                }
            }
            else
            {
                t->addTargetMask(TARGET_FLAG_UNIT);
                t->setUnitTarget(u_caster->getGuid());
                result = true;
            }
        }
        //target cone
        if (TargetType & SPELL_TARGET_AREA_CONE)
        {
            if (u_caster->getAIInterface()->getCurrentTarget() != nullptr)
            {
                t->addTargetMask(TARGET_FLAG_DEST_LOCATION);
                t->setDestination(u_caster->getAIInterface()->getCurrentTarget()->GetPosition());
                result = true;
            }
        }
    }
    return result;
}
