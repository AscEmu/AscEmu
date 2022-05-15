/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LordMarrowgar.h"
#include "Management/Faction.h"
#include "Objects/Units/Creatures/Summons/Summon.h"
#include <Management/ObjectMgr.h>
#include <Management/TransporterHandler.h>
#include <Objects/Transporter.h>
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.h"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lord Marrowgar
class LordMarrowgarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LordMarrowgarAI(c); }
    explicit LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        boneStormDuration = RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);
        baseSpeed = pCreature->getSpeedRate(TYPE_RUN, false);
        introDone = false;
        boneSlice = false;
        boneStormtarget = nullptr;
        coldflameLastPos = getCreature()->GetPosition();

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

        // Messages
        addEmoteForEvent(Event_OnCombatStart, SAY_MARR_AGGRO);     // The Scourge will wash over this world as a swarm of death and destruction!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_1);      // More bones for the offering!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_2);      // Languish in damnation!
        addEmoteForEvent(Event_OnDied, SAY_MARR_DEATH);            // I see... Only darkness.
    }

    void IntroStart()
    {
        sendDBChatMessage(SAY_MARR_ENTER_ZONE);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!
        introDone = true;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // common events
        scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);
        scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(10000, 15000));
        scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
        scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(45000, 50000));
        scriptEvents.addEvent(EVENT_ENRAGE, 600000);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    void Reset()
    {
        scriptEvents.resetEvents();

        getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
        getCreature()->RemoveAura(SPELL_BONE_STORM);
        getCreature()->RemoveAura(SPELL_BERSERK);

        boneSlice = false;
        boneSpikeImmune.clear();
    }

    void AIUpdate() override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (_isCasting())
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_BONE_SPIKE_GRAVEYARD:
                {
                    if (_isHeroic() || !getCreature()->hasAurasWithId(SPELL_BONE_STORM))
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

                    if (!_isHeroic())
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

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != POINT_MOTION_TYPE || iWaypointId != POINT_TARGET_BONESTORM_PLAYER)
            return;

        // lock movement
        getCreature()->getMovementManager()->moveIdle();
    }

    LocationVector const* GetLastColdflamePosition() const
    {
        return &coldflameLastPos;
    }

    void SetLastColdflamePosition(LocationVector pos)
    {
        coldflameLastPos = pos;
    }

    void SetCreatureData64(uint32_t Type, uint64_t Data) override
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

    uint64_t GetCreatureData64(uint32_t Type) const
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

    void DoAction(int32_t const action) override
    {
        if (action == ACTION_CLEAR_SPIKE_IMMUNITIES)
            boneSpikeImmune.clear();

        if (action == ACTION_MARROWGAR_INTRO_START)
            IntroStart();
    }

protected:
    // Common
    InstanceScript* mInstance;
    float baseSpeed;
    bool introDone;
    bool boneSlice;

    Unit* boneStormtarget;
    LocationVector coldflameLastPos;
    uint64_t coldflameTarget;
    std::vector<uint64_t> boneSpikeImmune;

    // Spells
    CreatureAISpells* boneSliceSpell;
    CreatureAISpells* boneStormSpell;
    CreatureAISpells* boneSpikeGraveyardSpell;
    CreatureAISpells* coldflameNormalSpell;
    CreatureAISpells* coldflameBoneStormSpell;
    CreatureAISpells* berserkSpell;

    uint32_t boneStormDuration;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cold Flame
class ColdflameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ColdflameAI(c); }
    explicit ColdflameAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();     
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
        coldflameTriggerSpell = addAISpell(SPELL_COLDFLAME_SUMMON, 0.0f, TARGET_SOURCE);
        coldflameTriggerSpell->mIsTriggered = true;
    }

    void OnLoad() override
    {
        getCreature()->setVisible(false);
    }

    void OnSummon(Unit* summoner) override
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

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

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

protected:
    // Common
    InstanceScript* mInstance;

    //Spells
    CreatureAISpells* coldflameTriggerSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
class BoneSpikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BoneSpikeAI(c); }
    explicit BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Common
        hasTrappedUnit = false;
        summon = nullptr;

        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
    }

    void OnSummon(Unit* summoner) override
    {
        summon = summoner;
        // Make our Creature in Combat otherwise on Died Script wont trigger
        getCreature()->getAIInterface()->setAiScriptType(AI_SCRIPT_AGRO);

        getCreature()->castSpell(summoner, SPELL_IMPALED);
        summoner->castSpell(getCreature(), SPELL_RIDE_VEHICLE_SE, true);
        scriptEvents.addEvent(EVENT_FAIL_BONED, 8000);
        hasTrappedUnit = true;
    }

    void OnTargetDied(Unit* pTarget) override
    {
        getCreature()->Despawn(100, 0);
        pTarget->RemoveAura(SPELL_IMPALED);
    }

    void OnDied(Unit* /*pTarget*/) override
    {       
        if (summon)
            summon->RemoveAura(SPELL_IMPALED);
         
        getCreature()->Despawn(100, 0);
    }

    void AIUpdate() override
    {
        if (!hasTrappedUnit)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (scriptEvents.getFinishedEvent() == EVENT_FAIL_BONED)
            if (mInstance)
                mInstance->setLocalData(DATA_BONED_ACHIEVEMENT, uint32_t(false));
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Summon
    Unit* summon;

    bool hasTrappedUnit;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm
class BoneStorm : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // set duration here
        int32_t duration = 20000;
        if (aur->GetUnitCaster()->isCreature())
            duration = static_cast<Creature*>(aur->GetUnitCaster())->GetScript()->RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);

        aur->setOriginalDuration(duration);
        aur->setMaxDuration(duration);
        aur->setTimeLeft(duration);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm Damage
class BoneStormDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        auto distance = spell->GetUnitTarget()->GetDistance2dSq(spell->getCaster());
        // If target is closer than 5 yards, do full damage
        if (distance <= 5.0f)
            distance = 1.0f;

        *dmg = float2int32(*dmg / distance);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Spike Graveyard
class BoneSpikeGraveyard : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override
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

    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {          
        if (Creature* marrowgar = static_cast<Creature*>(spell->getUnitCaster()))

            if (GetRandomTargetNotMainTank(marrowgar))
                return SpellCastResult::SPELL_CAST_SUCCESS;

        return SpellCastResult::SPELL_FAILED_DONT_REPORT;
    }

    Unit* GetRandomTargetNotMainTank(Creature* caster)
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

    bool CheckTarget(Unit* target, CreatureAIScript* creatureAI)
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
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame
class Coldflame : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
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

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), spell->damage, true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Bone Storm
class ColdflameBonestorm : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        for (uint8_t i = 0; i < 4; ++i)
            spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), (spell->damage + i), true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Damage
class ColdflameDamage: public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        effectTargets->clear();

        std::vector<Player*> players;
        for (const auto& itr : spell->getUnitCaster()->getInRangePlayersSet())
        {
            auto target = static_cast<Player*>(itr);

            if (CanBeAppliedOn(target, spell))
                effectTargets->push_back(target->getGuid());
        }
    }

    bool CanBeAppliedOn(Unit* target, Spell* spell)
    {
        if (target->hasAurasWithId(SPELL_IMPALED))
            return false;

        if (target->GetDistance2dSq(spell->getUnitCaster()) > static_cast<float>(spell->getSpellInfo()->getEffectRadiusIndex(EFF_INDEX_0)) )
            return false;

        return true;
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* /*spell*/, uint8_t effectIndex) override
    {
        if (effectIndex == EFF_INDEX_2)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Slice
class BoneSlice : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        if (!targetCount)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;
        
        *dmg = float2int32(*dmg / (float)targetCount);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }

    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {
        targetCount = 0;
        static_cast<Creature*>(spell->getUnitCaster())->GetScript()->DoAction(ACTION_CLEAR_SPIKE_IMMUNITIES);

        return SpellCastResult::SPELL_CAST_SUCCESS;
    }

    void filterEffectTargets(Spell* /*spell*/, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        targetCount = static_cast<uint32_t>(effectTargets->size());
    }

    void afterSpellEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        if (spell->GetUnitTarget())
        {
            static_cast<Creature*>(spell->getUnitCaster())->GetScript()->SetCreatureData64(DATA_SPIKE_IMMUNE, spell->GetUnitTarget()->getGuid());
        }
    }

    uint32_t targetCount;
};
