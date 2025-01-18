/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_IceCrownCitadel.hpp"
#include "LordMarrowgar.hpp"

#include "Movement/MovementManager.h"
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lord Marrowgar
LordMarrowgarAI::LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    boneStormDuration = getRaidModeValue(20000, 30000, 20000, 30000);
    baseSpeed = pCreature->getSpeedRate(TYPE_RUN, false);
    introDone = false;
    boneSlice = false;
    boneStormtarget = nullptr;
    coldflameLastPos = getCreature()->GetPosition();
    coldflameTarget = 0;

    // Scripted Spells not autocastet
    boneSliceSpell = addAISpell(SPELL_BONE_SLICE, 0.0f, TARGET_ATTACKING);
    boneStormSpell = addAISpell(SPELL_BONE_STORM, 0.0f, TARGET_SELF);
    boneStormSpell->addDBEmote(SAY_MARR_BONE_STORM_EMOTE);

    boneSpikeGraveyardSpell = addAISpell(SPELL_BONE_SPIKE_GRAVEYARD, 0.0f, TARGET_SELF);
    coldflameNormalSpell = addAISpell(SPELL_COLDFLAME_NORMAL, 0.0f, TARGET_SELF);
    coldflameBoneStormSpell = addAISpell(SPELL_COLDFLAME_BONE_STORM, 0.0f, TARGET_SELF);

    berserkSpell = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
    berserkSpell->addDBEmote(SAY_MARR_BERSERK);                  // THE MASTER'S RAGE COURSES THROUGH ME!
    berserkSpell->mIsTriggered = true;

    _setRangedDisabled(true);

    // Messages
    addEmoteForEvent(Event_OnCombatStart, SAY_MARR_AGGRO);     // The Scourge will wash over this world as a swarm of death and destruction!
    addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_1);      // More bones for the offering!
    addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_2);      // Languish in damnation!
    addEmoteForEvent(Event_OnDied, SAY_MARR_DEATH);            // I see... Only darkness.
}

CreatureAIScript* LordMarrowgarAI::Create(Creature* pCreature) { return new LordMarrowgarAI(pCreature); }

void LordMarrowgarAI::IntroStart()
{
    sendDBChatMessage(SAY_MARR_ENTER_ZONE);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!
    introDone = true;
}

void LordMarrowgarAI::OnCombatStart(Unit* /*pTarget*/)
{
    // common events
    scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);
    scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(10000, 15000));
    scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
    scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(45000, 50000));
    scriptEvents.addEvent(EVENT_ENRAGE, 600000);
}

void LordMarrowgarAI::OnCombatStop(Unit* /*_target*/)
{
    Reset();
}

void LordMarrowgarAI::Reset()
{
    scriptEvents.resetEvents();

    getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
    getCreature()->removeAllAurasById(SPELL_BONE_STORM);
    getCreature()->removeAllAurasById(SPELL_BERSERK);

    boneSlice = false;
    boneSpikeImmune.clear();
}

void LordMarrowgarAI::AIUpdate(unsigned long time_passed)
{
    if (!_isInCombat())
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (_isCasting())
        return;

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_BONE_SPIKE_GRAVEYARD:
            {
                if (isHeroic() || !getCreature()->hasAurasWithId(SPELL_BONE_STORM))
                    _castAISpell(boneSpikeGraveyardSpell);

                scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            }
            case EVENT_COLDFLAME:
            {
                coldflameLastPos = getCreature()->GetPosition();

                if (!getCreature()->hasAurasWithId(SPELL_BONE_STORM))
                    _castAISpell(coldflameNormalSpell);
                else
                    _castAISpell(coldflameBoneStormSpell);

                scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
                break;
            }
            case EVENT_WARN_BONE_STORM:
            {
                boneSlice = false;
                _castAISpell(boneStormSpell);

                scriptEvents.delayEvent(EVENT_BONE_SPIKE_GRAVEYARD, 3000);
                scriptEvents.delayEvent(EVENT_COLDFLAME, 3000);

                scriptEvents.addEvent(EVENT_BONE_STORM_BEGIN, 3050);
                scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(90000, 95000));
            }
            case EVENT_BONE_STORM_BEGIN:
            {
                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed * 3.0f, true);
                sendDBChatMessage(SAY_MARR_BONE_STORM); // BONE STORM!

                scriptEvents.addEvent(EVENT_BONE_STORM_END, boneStormDuration + 1);
            }
                [[fallthrough]];
            case EVENT_BONE_STORM_MOVE:
            {
                scriptEvents.addEvent(EVENT_BONE_STORM_MOVE, boneStormDuration / 3);

                boneStormtarget = getBestPlayerTarget(TargetFilter_NotCurrent);
                if (!boneStormtarget)
                    boneStormtarget = getBestPlayerTarget(TargetFilter_Current);
            
                if (boneStormtarget)
                    getCreature()->getMovementManager()->movePoint(POINT_TARGET_BONESTORM_PLAYER, boneStormtarget->GetPosition());

                break;
            }
            case EVENT_BONE_STORM_END:
            {
                if (MovementGenerator* movement = getCreature()->getMovementManager()->getMovementGenerator([](MovementGenerator const* a) -> bool
                    {
                        if (a->getMovementGeneratorType() == POINT_MOTION_TYPE)
                        {
                            PointMovementGenerator<Creature> const* pointMovement = dynamic_cast<PointMovementGenerator<Creature> const*>(a);
                            return pointMovement && pointMovement->getId() == POINT_TARGET_BONESTORM_PLAYER;
                        }
                        return false;
                    }))

                getCreature()->getMovementManager()->remove(movement);

                getCreature()->getMovementManager()->moveChase(getCreature()->getAIInterface()->getCurrentTarget());

                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
                scriptEvents.removeEvent(EVENT_BONE_STORM_MOVE);
                scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);

                if (!isHeroic())
                    scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            }
            case EVENT_ENABLE_BONE_SLICE:
            {
                boneSlice = true;
                break;
            }
            case EVENT_ENRAGE:
            {
                _castAISpell(berserkSpell);
                break;
            }
            default:
                break;
        }
    }
 
    // We should not melee attack when storming
    if (getCreature()->hasAurasWithId(SPELL_BONE_STORM))
    {
        _setMeleeDisabled(true);
        return;
    }

    _setMeleeDisabled(false);

    // 10 seconds since encounter start Bone Slice replaces melee attacks
    if (boneSlice)
    {
        _castAISpell(boneSliceSpell);
    }

}

void LordMarrowgarAI::OnReachWP(uint32_t type, uint32_t iWaypointId)
{
    if (type != POINT_MOTION_TYPE || iWaypointId != POINT_TARGET_BONESTORM_PLAYER)
        return;

    // lock movement
    getCreature()->getMovementManager()->moveIdle();
}

LocationVector const* LordMarrowgarAI::GetLastColdflamePosition() const
{
    return &coldflameLastPos;
}

void LordMarrowgarAI::SetLastColdflamePosition(LocationVector pos)
{
    coldflameLastPos = pos;
}

void LordMarrowgarAI::SetCreatureData64(uint32_t Type, uint64_t Data)
{
    switch (Type)
    {
        case DATA_COLDFLAME_GUID:
        {
            coldflameTarget = Data;
            break;
        }
        case DATA_SPIKE_IMMUNE:
        {
            boneSpikeImmune.push_back(Data);
            break;
        }
        default:
            break;
    }
}

uint64_t LordMarrowgarAI::GetCreatureData64(uint32_t Type) const
{ 
    switch (Type)
    {
        case DATA_COLDFLAME_GUID:
            return coldflameTarget;
        case DATA_SPIKE_IMMUNE + 0:
        case DATA_SPIKE_IMMUNE + 1:
        case DATA_SPIKE_IMMUNE + 2:
        {
            uint32_t index = Type - DATA_SPIKE_IMMUNE;
            if (index < boneSpikeImmune.size())
                return boneSpikeImmune[index];

            break;
        }
        default:
            return 0;
    }

    return 0;
}

void LordMarrowgarAI::DoAction(int32_t const action)
{
    if (action == ACTION_CLEAR_SPIKE_IMMUNITIES)
        boneSpikeImmune.clear();

    if (action == ACTION_MARROWGAR_INTRO_START)
        IntroStart();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cold Flame
ColdflameAI::ColdflameAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
    getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
    coldflameTriggerSpell = addAISpell(SPELL_COLDFLAME_SUMMON, 0.0f, TARGET_SOURCE);
    coldflameTriggerSpell->mIsTriggered = true;
}

CreatureAIScript* ColdflameAI::Create(Creature* pCreature) { return new ColdflameAI(pCreature); }

void ColdflameAI::OnLoad()
{
    getCreature()->setVisible(false);
}

void ColdflameAI::OnSummon(Unit* summoner)
{
    if (!mInstance || !summoner->isCreature())
        return;

    if (summoner->hasAurasWithId(SPELL_BONE_STORM))
    {
        // Bonestorm X Pattern
        if (LordMarrowgarAI* marrowgarAI = static_cast<LordMarrowgarAI*>(static_cast<Creature*>(summoner)->GetScript()))
        {
            LocationVector const* ownerPos = marrowgarAI->GetLastColdflamePosition();
            float angle = ownerPos->o / M_PI_FLOAT * 180.0f;
            MoveTeleport(ownerPos->x , ownerPos->y, ownerPos->z, ownerPos->o);
            // Store last Coldflame Position and add 90 degree to create x pattern
            LocationVector nextPos;
            nextPos.x = ownerPos->x;
            nextPos.y = ownerPos->y;
            nextPos.z = ownerPos->z;
            nextPos.o = angle + 90 * M_PI_FLOAT / 180.0f;

            marrowgarAI->SetLastColdflamePosition(nextPos);
        }
    }
    // Random target Case
    else
    {
        Unit* target = mInstance->getInstance()->getUnit(static_cast<Creature*>(summoner)->GetScript()->GetCreatureData64(DATA_COLDFLAME_GUID));
        if (!target)
        {
            getCreature()->Despawn(100, 0);
            return;
        }        
        MoveTeleport(summoner->GetPosition());

        getCreature()->SetOrientation(getCreature()->calcAngle(summoner->GetPositionX(), summoner->GetPositionY(), target->GetPositionX(), target->GetPositionY()) * M_PI_FLOAT / 180.0f);
    }        
    MoveTeleport(summoner->GetPositionX(), summoner->GetPositionY(),summoner->GetPositionZ(), getCreature()->GetOrientation());
    scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
}

void ColdflameAI::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (scriptEvents.getFinishedEvent() == EVENT_COLDFLAME_TRIGGER)
    {
        LocationVector newPos;
        newPos = getCreature()->GetPosition();

        float angle = newPos.o;
        float destx, desty, destz;
        destx = newPos.x + 5.0f * std::cos(angle);
        desty = newPos.y + 5.0f * std::sin(angle);
        destz = newPos.z;

        newPos.x = destx;
        newPos.y = desty;
        newPos.z = destz;

        MoveTeleport(newPos.x, newPos.y, newPos.z, newPos.o);
        _castAISpell(coldflameTriggerSpell);         
        scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
BoneSpikeAI::BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Common
    hasTrappedUnit = false;
    summon = nullptr;

    getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
}

CreatureAIScript* BoneSpikeAI::Create(Creature* pCreature) { return new BoneSpikeAI(pCreature); }

void BoneSpikeAI::OnSummon(Unit* summoner)
{
    summon = summoner;

    getCreature()->castSpell(summoner, SPELL_IMPALED);
    summoner->castSpell(getCreature(), SPELL_RIDE_VEHICLE_SE, true);
    scriptEvents.addEvent(EVENT_FAIL_BONED, 8000);
    hasTrappedUnit = true;
}

void BoneSpikeAI::OnTargetDied(Unit* pTarget)
{
    getCreature()->Despawn(100, 0);
    pTarget->removeAllAurasById(SPELL_IMPALED);
}

void BoneSpikeAI::OnDied(Unit* /*pTarget*/)
{       
    if (summon)
        summon->removeAllAurasById(SPELL_IMPALED);
     
    getCreature()->Despawn(100, 0);
}

void BoneSpikeAI::AIUpdate(unsigned long time_passed)
{
    if (!hasTrappedUnit)
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (scriptEvents.getFinishedEvent() == EVENT_FAIL_BONED)
        if (mInstance)
            mInstance->setLocalData(DATA_BONED_ACHIEVEMENT, uint32_t(false));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm
void BoneStorm::onAuraCreate(Aura* aur)
{
    // set duration here
    int32_t duration = 20000;
    if (aur->GetUnitCaster()->isCreature())
        duration = static_cast<Creature*>(aur->GetUnitCaster())->GetScript()->getRaidModeValue(20000, 30000, 20000, 30000);

    aur->setNewMaxDuration(duration, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm Damage
SpellScriptEffectDamage BoneStormDamage::doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg)
{
    if (effIndex != EFF_INDEX_0 || spell->getUnitTarget() == nullptr)
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;

    auto distance = spell->getUnitTarget()->GetDistance2dSq(spell->getCaster());
    // If target is closer than 5 yards, do full damage
    if (distance <= 5.0f)
        distance = 1.0f;

    *dmg = Util::float2int32(*dmg / distance);
    return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Spike Graveyard
void BoneSpikeGraveyard::onAuraApply(Aura* aur)
{
    aur->removeAuraEffect(EFF_INDEX_1);

    if (Creature* marrowgar = static_cast<Creature*>(aur->GetUnitCaster()))
    {
        CreatureAIScript* marrowgarAI = marrowgar->GetScript();
        if (!marrowgarAI)
            return;

        uint8_t boneSpikeCount = uint8_t(aur->GetUnitCaster()->getWorldMap()->getSpawnMode() & 1 ? 3 : 1);
        std::list<Unit*> targets;

        targets.clear();

        for (uint8_t target = 0; target < boneSpikeCount; ++target)
            targets.push_back(GetRandomTargetNotMainTank(marrowgar));

        if (targets.empty())
            return;

        uint32_t i = 0;
        for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr, ++i)
        {
            Unit* target = *itr;
            target->castSpell(target, BoneSpikeSummonId[i], true);
        }


        std::vector<uint32_t> emoteVector;
        emoteVector.clear();
        emoteVector.push_back(SAY_MARR_BONESPIKE_1);// Stick Around
        emoteVector.push_back(SAY_MARR_BONESPIKE_2);// The only Escape is Darkness 
        emoteVector.push_back(SAY_MARR_BONESPIKE_3);// More Bones for the offering

        marrowgarAI->sendRandomDBChatMessage(emoteVector, nullptr);
    }
}

SpellCastResult BoneSpikeGraveyard::onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/)
{          
    if (Creature* marrowgar = static_cast<Creature*>(spell->getUnitCaster()))

        if (GetRandomTargetNotMainTank(marrowgar))
            return SpellCastResult::SPELL_CAST_SUCCESS;

    return SpellCastResult::SPELL_FAILED_DONT_REPORT;
}

Unit* BoneSpikeGraveyard::GetRandomTargetNotMainTank(Creature* caster)
{
    Unit* target = nullptr;
    std::vector<Player*> players;

    Unit* mt = caster->getAIInterface()->getCurrentTarget();
    if (mt == nullptr || !mt->isPlayer())
        return 0;

    for (const auto& itr : caster->getInRangePlayersSet())
    {
        Player* obj = static_cast<Player*>(itr);
        if (obj != mt && CheckTarget(obj, caster->GetScript()))
            players.push_back(obj);
    }

    if (players.size())
        target = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];

    return target;
}

bool BoneSpikeGraveyard::CheckTarget(Unit* target, CreatureAIScript* creatureAI)
{
    if (target->getObjectTypeId() != TYPEID_PLAYER)
        return false;

    if (target->hasAurasWithId(SPELL_IMPALED))
        return false;

    // Check if it is one of the tanks soaking Bone Slice
    for (uint32_t i = 0; i < MAX_BONE_SPIKE_IMMUNE; ++i)
        if (target->getGuid() == creatureAI->GetCreatureData64(DATA_SPIKE_IMMUNE + i))
            return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame
void Coldflame::filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    if (effectIndex != EFF_INDEX_0)
        return;

    effectTargets->clear();

    Unit* coldflametarget = nullptr;
    Creature* pCreature = nullptr;

    if (spell->getUnitCaster()->isCreature())
        pCreature = static_cast<Creature*>(spell->getUnitCaster());

    // select any unit but not the tank 
    if (pCreature)
    {
        coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_NotCurrent);
        if (!coldflametarget)
            coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_Current);

        if (coldflametarget)
        {
            pCreature->GetScript()->SetCreatureData64(DATA_COLDFLAME_GUID, coldflametarget->getGuid());
            effectTargets->push_back(coldflametarget->getGuid());
        }
    }
}

SpellScriptExecuteState Coldflame::beforeSpellEffect(Spell* spell, uint8_t effectIndex)
{
    if (effectIndex != EFF_INDEX_0)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    spell->getUnitCaster()->castSpell(spell->getUnitTarget(), spell->damage, true);

    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Bone Storm
SpellScriptExecuteState ColdflameBonestorm::beforeSpellEffect(Spell* spell, uint8_t effectIndex)
{
    if (effectIndex != EFF_INDEX_0)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    for (uint8_t i = 0; i < 4; ++i)
        spell->getUnitCaster()->castSpell(spell->getUnitTarget(), (spell->damage + i), true);

    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Damage
void ColdflameDamage::filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    if (effectIndex != EFF_INDEX_0)
        return;

    effectTargets->clear();

    for (const auto& itr : spell->getUnitCaster()->getInRangePlayersSet())
    {
        auto target = static_cast<Player*>(itr);

        if (CanBeAppliedOn(target, spell))
            effectTargets->push_back(target->getGuid());
    }
}

bool ColdflameDamage::CanBeAppliedOn(Unit* target, Spell* spell)
{
    if (target->hasAurasWithId(SPELL_IMPALED))
        return false;

    if (target->GetDistance2dSq(spell->getUnitCaster()) > static_cast<float>(spell->getSpellInfo()->getEffectRadiusIndex(EFF_INDEX_0)))
        return false;

    return true;
}

SpellScriptExecuteState ColdflameDamage::beforeSpellEffect(Spell* /*spell*/, uint8_t effectIndex)
{
    if (effectIndex == EFF_INDEX_2)
        return SpellScriptExecuteState::EXECUTE_PREVENT;

    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Slice
SpellScriptEffectDamage BoneSlice::doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg)
{
    if (effIndex != EFF_INDEX_0 || spell->getUnitTarget() == nullptr)
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;

    if (!targetCount)
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;
    
    *dmg = Util::float2int32(*dmg / (float)targetCount);
    return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
}

SpellCastResult BoneSlice::onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/)
{
    targetCount = 0;
    static_cast<Creature*>(spell->getUnitCaster())->GetScript()->DoAction(ACTION_CLEAR_SPIKE_IMMUNITIES);

    return SpellCastResult::SPELL_CAST_SUCCESS;
}

void BoneSlice::filterEffectTargets(Spell* /*spell*/, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    if (effectIndex != EFF_INDEX_0)
        return;

    targetCount = static_cast<uint32_t>(effectTargets->size());
}

void BoneSlice::afterSpellEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex != EFF_INDEX_0)
        return;

    if (spell->getUnitTarget())
    {
        static_cast<Creature*>(spell->getUnitCaster())->GetScript()->SetCreatureData64(DATA_SPIKE_IMMUNE, spell->getUnitTarget()->getGuid());
    }
}
