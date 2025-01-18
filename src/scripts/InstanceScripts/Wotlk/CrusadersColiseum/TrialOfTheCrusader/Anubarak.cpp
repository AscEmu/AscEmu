/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Anubarak.hpp"
#include "Raid_TrialOfTheCrusader.hpp"
#include "Movement/MovementManager.h"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Utilities/Random.hpp"

// ToDo
// Cannot Get Submerge Phase to Work
// For whatever Reason There is no Creaturemodell
// Also The Frozen Orbs dont Cast theyre ground Effect

//////////////////////////////////////////////////////////////////////////////////////////
///  Anubarak
AnubarakAI::AnubarakAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    introDone = false;
    reachedPhase3 = false;
    sphereGuids.resize(6);

    // Events
    addEmoteForEventByIndex(CreatureAIScript::Event_OnCombatStart, anubarak::SAY_AGGRO);
    addEmoteForEventByIndex(Event_OnTargetDied, anubarak::SAY_KILL_PLAYER);
    addEmoteForEventByIndex(Event_OnDied, anubarak::SAY_DEATH);
}

CreatureAIScript* AnubarakAI::Create(Creature* pCreature) { return new AnubarakAI(pCreature); }

void AnubarakAI::InitOrReset()
{
    introDone = false;
    reachedPhase3 = false;

    setScriptPhase(anubarak::PHASE_MELEE);
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    // clean up spawned Frost Spheres
    std::list<Creature*> FrostSphereList;

    GetCreatureListWithEntryInGrid(FrostSphereList, anubarak::NPC_FROST_SPHERE, 150.0f);

    if (!FrostSphereList.empty())
        for (std::list<Creature*>::iterator itr = FrostSphereList.begin(); itr != FrostSphereList.end(); ++itr)
            (*itr)->Despawn(2000, 0);

    burrowGuids.clear();
}

void AnubarakAI::OnCombatStart(Unit* target)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
    getInstanceScript()->setBossState(DATA_ANUBARAK, EncounterStates::InProgress);

    addAIFunction([this](CreatureAIFunc pThis)
    {
        if (_getHealthPercent() <= 30 && getScriptPhase() == anubarak::PHASE_MELEE)
        {
            reachedPhase3 = true;
            castSpellAOE(anubarak::SPELL_LEECHING_SWARM);
            addMessage(Message(anubarak::EMOTE_LEECHING_SWARM), DoOnceScheduler());
            addMessage(Message(anubarak::SAY_LEECHING_SWARM), DoOnceScheduler());
            return;
        }
        repeatFunctionFromScheduler(pThis);
    }, DoOnceScheduler(1s));

    addAISpell(SpellDesc(anubarak::SPELL_FREEZE_SLASH, FilterArgs(TargetFilter_Current), false), DoLoopScheduler(15s, 33.0f, { anubarak::PHASE_MELEE }));
    addAISpell(SpellDesc(anubarak::SPELL_PENETRATING_COLD, FilterArgs(TargetFilter_AOE), false), DoLoopScheduler(20s, 33.0f, { anubarak::PHASE_MELEE }));

    /*addAIFunction([this](CreatureAIFunc pThis)
        {
            if (_isHeroic() || !reachedPhase3)
                castSpellAOE(anubarak::SPELL_SUMMON_BURROWER);
            repeatFunctionFromScheduler(pThis, 45s);
        }, DoOnceScheduler(10s, 33.0f, { anubarak::PHASE_MELEE }));*/

    addAIFunction(&AnubarakAI::submerge, DoOnceScheduler(80s, 100.0f, { anubarak::PHASE_MELEE }));
    addAISpell(SpellDesc(anubarak::SPELL_BERSERK, FilterArgs(TargetFilter_Self), false), DoLoopScheduler(10min, 100.0f));

    if (isHeroic())
        addAIFunction([this](CreatureAIFunc pThis)
            {
                summons.DoActionForEntry(anubarak::ACTION_SHADOW_STRIKE, anubarak::NPC_BURROWER);
                repeatFunctionFromScheduler(pThis, 30s);
            }, DoOnceScheduler(10s, 33.0f, { anubarak::PHASE_MELEE }));

    if (!isHeroic())
        addAIFunction(&AnubarakAI::summonFrostSphere, DoOnceScheduler(20s, 33.0f));

    removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

    // Despawn Scarab Swarms neutral
    summons.DoActionForEntry(anubarak::ACTION_SCARAB_SUBMERGE, anubarak::NPC_SCARAB);

    // Spawn Burrow
    for (int i = 0; i < 4; i++)
        summonCreature(anubarak::NPC_BURROW, AnubarakLoc[i + 2]);

    // Spawn 6 Frost Spheres at start
    for (int i = 0; i < 6; i++)
        if (Unit* summoned = summonCreature(anubarak::NPC_FROST_SPHERE, anubarak::SphereSpawn[i]))
            sphereGuids[i] = summoned->getGuid();
}

void AnubarakAI::OnDied(Unit* /*_killer*/)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    getInstanceScript()->setBossState(DATA_ANUBARAK, EncounterStates::Performed);

    // despawn frostspheres and Burrowers on death
    std::list<Creature*> AddList;
    GetCreatureListWithEntryInGrid(AddList, anubarak::NPC_FROST_SPHERE, 150.0f);
    GetCreatureListWithEntryInGrid(AddList, anubarak::NPC_BURROWER, 150.0f);

    if (!AddList.empty())
        for (std::list<Creature*>::iterator itr = AddList.begin(); itr != AddList.end(); ++itr)
            (*itr)->Despawn(2000, 0);
}

void AnubarakAI::justReachedSpawn()
{
    getInstanceScript()->setBossState(DATA_ANUBARAK, EncounterStates::Failed);
    //Summon Scarab Swarms neutral at random places
    for (int i = 0; i < 10; i++)
    {
        if (Creature* scarab = summonCreature(anubarak::NPC_SCARAB, AnubarakLoc[1].getPositionX() + Util::getRandomUInt(0, 50) - 25, AnubarakLoc[1].getPositionY() + Util::getRandomUInt(0, 50) - 25, AnubarakLoc[1].getPositionZ(), AnubarakLoc[1].getOrientation()))
        {
            scarab->setFaction(31);
            scarab->getMovementManager()->moveRandom(10);
        }
    }
}

void AnubarakAI::DoAction(int32_t action)
{
    if (!introDone && action == 5475)
    {
        addMessage(Message(anubarak::SAY_INTRO), DoOnceScheduler());
        introDone = true;
    }
}

void AnubarakAI::onSummonedCreature(Creature* summon)
{
    switch (summon->getEntry())
    {
        case anubarak::NPC_BURROW:
        {
            burrowGuids.push_back(summon->getGuid());
            summon->getAIInterface()->setReactState(REACT_PASSIVE);
            summon->castSpell(summon, anubarak::SPELL_CHURNING_GROUND, false);
            summon->setDisplayId(summon->GetCreatureProperties()->Female_DisplayID);
        } break;
        case anubarak::NPC_SPIKE:
        {
            summon->setDisplayId(summon->GetCreatureProperties()->Male_DisplayID);
            if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player)))
            {
                summon->getAIInterface()->attackStart(target);
                addMessage(Message(anubarak::EMOTE_SPIKE, target), DoOnceScheduler());
            }
        } break;
        default:
            break;
    }
    summons.summon(summon);
}

void AnubarakAI::submerge(CreatureAIFunc pThis)
{
    if (!reachedPhase3 && !hasAura(anubarak::SPELL_BERSERK))
    {
        castSpellOnSelf(anubarak::SPELL_SUBMERGE_ANUBARAK);
        castSpellOnSelf(anubarak::SPELL_CLEAR_ALL_DEBUFFS);
        setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        sendDBChatMessageByIndex(anubarak::EMOTE_BURROWER);
        setScriptPhase(anubarak::PHASE_SUBMERGED);

        cancelFunctionFromScheduler(pThis);
        resetAllFunctionsFromScheduler();
        addAISpell(SpellDesc(anubarak::SPELL_SPIKE_CALL, FilterArgs(TargetFilter_Self), false), DoOnceScheduler(2s, 100.0f, { anubarak::PHASE_SUBMERGED }));
        addAIFunction(&AnubarakAI::summonScarabs, DoOnceScheduler(4s, 100.0f, { anubarak::PHASE_SUBMERGED }));
        addAIFunction(&AnubarakAI::emerge, DoOnceScheduler(60s, 100.0f, { anubarak::PHASE_SUBMERGED }));
    }
}

void AnubarakAI::emerge(CreatureAIFunc pThis)
{
    castSpellOnSelf(anubarak::SPELL_SPIKE_TELE);
    summons.despawnEntry(anubarak::NPC_SPIKE);
    _removeAura(anubarak::SPELL_SUBMERGE_ANUBARAK);
    removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    castSpellOnSelf(anubarak::SPELL_EMERGE_ANUBARAK);
    sendDBChatMessageByIndex(anubarak::EMOTE_EMERGE);
    setScriptPhase(anubarak::PHASE_MELEE);

    cancelFunctionFromScheduler(pThis);
    resetAllFunctionsFromScheduler();
    addAIFunction(&AnubarakAI::submerge, DoOnceScheduler(80s, 100.0f, { anubarak::PHASE_MELEE }));
}

void AnubarakAI::summonFrostSphere(CreatureAIFunc pThis)
{
    // Get a random starting index within the valid range
    uint8_t startAt = Util::getRandomUInt(0, static_cast<uint32_t>(sphereGuids.size() - 1));
    uint8_t attempts = 0;

    do {
        // Get the current sphere index to check
        uint8_t currentIndex = (startAt + attempts) % 6;

        Creature* pSphere = getCreature()->getWorldMapCreature(sphereGuids[currentIndex]);

        if (pSphere && !pSphere->hasAurasWithId(anubarak::SPELL_FROST_SPHERE))
        {
            // Create and summon a new frost sphere
            Creature* summon = summonCreature(anubarak::NPC_FROST_SPHERE, anubarak::SphereSpawn[currentIndex]);

            if (summon) 
            {
                // If summoning successful, update the sphereGuids with the new creature's GUID
                sphereGuids[currentIndex] = summon->getGuid();
            }

            // Exit the loop, we found a suitable sphere or summoned a new one
            break;
        }

        // Increment attempts counter to move to the next index in the next iteration
        attempts++;

    } while (attempts < sphereGuids.size());

    repeatFunctionFromScheduler(pThis, 20s);
}

void AnubarakAI::summonScarabs(CreatureAIFunc pThis)
{
    // Check if the burrowGuids vector is not empty
    if (!burrowGuids.empty()) 
    {
        // Get a random index within the valid range
        uint32_t at = Util::getRandomUInt(0, static_cast<uint32_t>(burrowGuids.size() - 1));

        // Get the iterator pointing to the selected element in the vector
        auto i = burrowGuids.begin() + at;

        // Check if the element at the selected index is a valid creature
        if (Creature* pBurrow = getCreature()->getWorldMapCreature(*i)) 
        {
            // Cast the spell on the valid creature
            pBurrow->castSpell(pBurrow, 66340, false);
        }
    }

    repeatFunctionFromScheduler(pThis, 4s);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  SwarmScrab
SwarmScrabAI::SwarmScrabAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* SwarmScrabAI::Create(Creature* pCreature) { return new SwarmScrabAI(pCreature); }

void SwarmScrabAI::InitOrReset()
{
    determinationTimer = Util::getRandomUInt(5 * TimeVarsMs::Second, 60 * TimeVarsMs::Second);
    castSpellOnSelf(anubarak::SPELL_ACID_MANDIBLE);
    setZoneWideCombat();

    if (_isInCombat())
        if (Creature* anubarak = getInstanceScript()->getCreatureFromData(DATA_ANUBARAK))
            anubarak->GetScript()->onSummonedCreature(getCreature());
}

void SwarmScrabAI::OnDied(Unit* _killer)
{
    if (_killer)
        castSpell(_killer, SPELL_TRAITOR_KING);
}

void SwarmScrabAI::DoAction(int32_t action)
{
    switch (action)
    {
        case anubarak::ACTION_SCARAB_SUBMERGE:
            castSpellOnSelf(anubarak::SPELL_SUBMERGE_EFFECT);
            despawn();
            break;
        default:
            break;
    }
}

void SwarmScrabAI::AIUpdate(unsigned long time_passed)
{
    if (getInstanceScript()->getBossState(DATA_ANUBARAK) != InProgress)
        despawn();

    if (!_isInCombat() && !hasAura(anubarak::SPELL_SUBMERGE_EFFECT))
        return;

    if (_isCasting())
        return;

    if (determinationTimer <= static_cast<int32_t>(time_passed))
    {
        castSpellOnSelf(anubarak::SPELL_DETERMINATION);
        determinationTimer = Util::getRandomUInt(10 * TimeVarsMs::Second, 60 * TimeVarsMs::Second);
    }
    else
    {
        determinationTimer -= time_passed;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Burrower
BurrowerAI::BurrowerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    setUnitFlags(UNIT_FLAG_SWIMMING);
    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
}

CreatureAIScript* BurrowerAI::Create(Creature* pCreature) { return new BurrowerAI(pCreature); }

void BurrowerAI::InitOrReset()
{
    submergeTimer = 30 * TimeVarsMs::Second;
    castSpellOnSelf(anubarak::SPELL_EXPOSE_WEAKNESS);
    castSpellOnSelf(anubarak::SPELL_SPIDER_FRENZY);
    castSpellOnSelf(anubarak::SPELL_AWAKENED);
    setZoneWideCombat();

    if (_isInCombat())
        if (Creature* anubarak = getInstanceScript()->getCreatureFromData(DATA_ANUBARAK))
            anubarak->GetScript()->onSummonedCreature(getCreature());
}

void BurrowerAI::DoAction(int32_t action)
{
    switch (action)
    {
        case anubarak::ACTION_SHADOW_STRIKE:
            if (!hasAura(anubarak::SPELL_AWAKENED))
                if (Unit* target = selectUnitTarget())
                    castSpell(target, anubarak::SPELL_SHADOW_STRIKE);
            break;
        default:
            break;
    }
}

void BurrowerAI::AIUpdate(unsigned long time_passed)
{
    if (getInstanceScript()->getBossState(DATA_ANUBARAK) != InProgress)
        despawn();

    if (!_isInCombat() && !hasAura(anubarak::SPELL_SUBMERGE_EFFECT))
        return;

    if (_isCasting())
        return;

    if ((submergeTimer <= static_cast<int32_t>(time_passed)) && _getHealthPercent() <= 80)
    {
        if (hasAura(anubarak::SPELL_SUBMERGE_EFFECT))
        {
            _removeAura(anubarak::SPELL_SUBMERGE_EFFECT);
            castSpellOnSelf(anubarak::SPELL_EMERGE_EFFECT);
            castSpellOnSelf(anubarak::SPELL_AWAKENED);
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
        else
        {
            if (!hasAura(SPELL_PERMAFROST_HELPER))
            {
                castSpellOnSelf(anubarak::SPELL_SUBMERGE_EFFECT);
                setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                castSpellOnSelf(anubarak::SPELL_PERSISTENT_DIRT, true);
            }
        }
        submergeTimer = 20 * TimeVarsMs::Second;
    }
    else
    {
        submergeTimer -= time_passed;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  FrostSphere
FrostSphereAI::FrostSphereAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    setReactState(REACT_PASSIVE);
    setUnitFlags(UNIT_FLAG_SWIMMING);
    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
}

CreatureAIScript* FrostSphereAI::Create(Creature* pCreature) { return new FrostSphereAI(pCreature); }

void FrostSphereAI::InitOrReset()
{
    setReactState(REACT_PASSIVE);
    setUnitFlags(UNIT_FLAG_SWIMMING);
    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
    addAISpell(SpellDesc(anubarak::SPELL_FROST_SPHERE, FilterArgs(TargetFilter_Self)), DoOnceScheduler(100ms));
    _setDisplayId(getCreature()->GetCreatureProperties()->Female_DisplayID);
    getMovementManager()->moveRandom(20.0f);
}

void FrostSphereAI::DamageTaken(Unit* /*_attacker*/, uint32_t* damage)
{
    if (getCreature()->getHealth() <= *damage)
    {
        *damage = 0;
        getCreature()->setHealth(1);

        float floorZ = getCreature()->GetPositionZ();
        getCreature()->updateGroundPositionZ(getCreature()->GetPositionX(), getCreature()->GetPositionY(), floorZ);

        if (fabs(getCreature()->GetPositionZ() - floorZ) < 0.1f)
        {
            // we are close to the ground
            getMovementManager()->moveIdle();
            setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            _removeAura(anubarak::SPELL_FROST_SPHERE);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST_VISUAL);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST_MODEL);
            _setScale(2.0f);
        }
        else
        {
            // we are in air
            getMovementManager()->moveIdle();
            setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            //At hit the ground
            getCreature()->emote(EMOTE_ONESHOT_FLYDEATH);
            getMovementManager()->moveFall(anubarak::POINT_FALL_GROUND);
        }
    }
}

void FrostSphereAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type != EFFECT_MOTION_TYPE)
        return;

    switch (id)
    {
        case anubarak::POINT_FALL_GROUND:
            _removeAura(anubarak::SPELL_FROST_SPHERE);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST_VISUAL);
            castSpellOnSelf(anubarak::SPELL_PERMAFROST_MODEL);
            _setScale(2.0f);
            break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Spike
SpikeAI::SpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_SWIMMING);
    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
}

CreatureAIScript* SpikeAI::Create(Creature* pCreature) { return new SpikeAI(pCreature); }

void SpikeAI::InitOrReset()
{
    setZoneWideCombat();
    _setDisplayId(11686);
    setScriptPhase(anubarak::PHASE_NO_MOVEMENT);
    phaseTimer = 1;
}

void SpikeAI::OnCombatStart(Unit* _target)
{
    if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player)))
    {
        startChase(target);
        addEmote(Message(anubarak::EMOTE_SPIKE, target), DoOnceScheduler());
    }
}

void SpikeAI::handlePermafrostHit(Creature* pCreature)
{
    if (pCreature->getEntry() != anubarak::NPC_FROST_SPHERE)
        return;

    if (getScriptPhase() == anubarak::PHASE_NO_MOVEMENT)
        return;

    if (getCreature()->IsWithinDistInMap(pCreature, 7.0f))
    {
        switch (getScriptPhase())
        {
            case anubarak::PHASE_IMPALE_NORMAL:
                _removeAura(anubarak::SPELL_SPIKE_SPEED1);
                break;
            case anubarak::PHASE_IMPALE_MIDDLE:
                _removeAura(anubarak::SPELL_SPIKE_SPEED2);
                break;
            case anubarak::PHASE_IMPALE_FAST:
                _removeAura(anubarak::SPELL_SPIKE_SPEED3);
                break;
            default:
                break;
        }

        castSpellOnSelf(anubarak::SPELL_SPIKE_FAIL, true);

        // After the spikes hit the icy surface they can't move for about ~5 seconds
        setScriptPhase(anubarak::PHASE_NO_MOVEMENT);
        phaseTimer = 5 * TimeVarsMs::Second;
        setRooted(false);
        getMovementManager()->moveIdle();
        getMovementManager()->clear();
    }
}

void SpikeAI::AIUpdate(unsigned long time_passed)
{
    if (!getCreature()->getAIInterface()->updateTarget())
    {
        despawn();
        return;
    }

    if (phaseTimer)
    {
        if (phaseTimer <= static_cast<int32_t>(time_passed))
        {
            switch (getScriptPhase())
            {
                case anubarak::PHASE_NO_MOVEMENT:
                {
                    castSpellOnSelf(anubarak::SPELL_SPIKE_SPEED1);
                    castSpellOnSelf(anubarak::SPELL_SPIKE_TRAIL);
                    setScriptPhase(anubarak::PHASE_IMPALE_NORMAL);
                    if (Unit* target2 = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    {
                        startChase(target2);
                        sendDBChatMessageByIndex(anubarak::EMOTE_SPIKE, target2);
                    }
                    phaseTimer = 7 * TimeVarsMs::Second;
                } return;
                case anubarak::PHASE_IMPALE_NORMAL:
                {
                    castSpellOnSelf(anubarak::SPELL_SPIKE_SPEED2);
                    setScriptPhase(anubarak::PHASE_IMPALE_MIDDLE);
                    phaseTimer = 7 * TimeVarsMs::Second;
                } return;
                case anubarak::PHASE_IMPALE_MIDDLE:
                {
                    castSpellOnSelf(anubarak::SPELL_SPIKE_SPEED3);
                    setScriptPhase(anubarak::PHASE_IMPALE_FAST);
                    phaseTimer = 0;
                } return;
                default:
                    return;
            }
        }
        else
        {
            phaseTimer -= time_passed;
        }
    }
}

bool SpikeAI::canAttackTarget(Unit* target)
{
    return target->isPlayer();
}

void SpikeAI::DamageTaken(Unit* /*_attacker*/, uint32_t* damage)
{
    *damage = 0;
}

void SpikeAI::startChase(Unit* target)
{
    castSpell(target, anubarak::SPELL_MARK);
    setSpeedRate(TYPE_RUN, 0.5f, true);
    // make sure the Spine will really follow the one he should
    _clearHateList();
    setZoneWideCombat();
    addThreat(target, 1000000.0f);
    getCreature()->getAIInterface()->onHostileAction(target);
    getCreature()->getAIInterface()->attackStart(target);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spikes
bool PermafrostDummySpell(uint8_t effectIndex, Spell* pSpell)
{
    // always check spellid and effectindex
    if (pSpell->getSpellInfo()->getId() == anubarak::SPELL_PERMAFROST_DUMMY && effectIndex == EFF_INDEX_0)
    {
        Unit* target = pSpell->getUnitTarget();
        if (target && target->isCreature())
        {
            if (SpikeAI* pSpikeAI = dynamic_cast<SpikeAI*>(target->ToCreature()->GetScript()))
                pSpikeAI->handlePermafrostHit((Creature*)pSpell->getUnitCaster());
        }
        // always return true when we are handling this spell and effect
        return true;
    }

    return false;
}