/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Northrend_Beasts.hpp"
#include "Raid_TrialOfTheCrusader.hpp"
#include "Map/AreaBoundary.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/AIUtils.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
///  Combat Stalker
CombatStalkerAI::CombatStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* CombatStalkerAI::Create(Creature* pCreature) { return new CombatStalkerAI(pCreature); }

void CombatStalkerAI::OnLoad()
{
    // Dont get us un Combat we are a Spectator
    setReactState(REACT_PASSIVE);

    // The Encounter we are Currently Facing Should go Berserk
    addAIFunction(&CombatStalkerAI::Berserk, DoOnceScheduler(isHeroic() ? 9min : 15min));
}

void CombatStalkerAI::DoAction(int32_t action)
{
    switch (action)
    {
        case ACTION_START_JORMUNGARS:
            startJormungars = addAIFunction(&CombatStalkerAI::StartJormungars, DoOnceScheduler(150s));
            break;
        case ACTION_START_ICEHOWL:
            startIcehowl = addAIFunction(&CombatStalkerAI::StartIcehowl, DoOnceScheduler(150s));
            break;
        case Beasts::ACTION_GORMOK_DEAD:
            cancelFunctionFromScheduler(startJormungars);
            break;
        case Beasts::ACTION_JORMUNGARS_DEAD:
            cancelFunctionFromScheduler(startIcehowl);
            break;
        default:
            break;
    }
}

void CombatStalkerAI::Berserk(CreatureAIFunc pThis)
{
    CombatStalkerAI* script = static_cast<CombatStalkerAI*>(this);
    if (script && script->getInstanceScript())
    {
        if (Creature* gormok = script->getInstanceScript()->getCreatureFromData(DATA_GORMOK_THE_IMPALER))
            gormok->castSpell(gormok, Beasts::Icehowl::SPELL_BERSERK, true);

        if (Creature* dreadscale = script->getInstanceScript()->getCreatureFromData(DATA_DREADSCALE))
            dreadscale->castSpell(dreadscale, Beasts::Icehowl::SPELL_BERSERK, true);

        if (Creature* acidmaw = script->getInstanceScript()->getCreatureFromData(DATA_ACIDMAW))
            acidmaw->castSpell(acidmaw, Beasts::Icehowl::SPELL_BERSERK, true);

        if (Creature* icehowl = script->getInstanceScript()->getCreatureFromData(DATA_ICEHOWL))
            icehowl->castSpell(icehowl, Beasts::Icehowl::SPELL_BERSERK, true);
    }
}

void CombatStalkerAI::StartJormungars(CreatureAIFunc pThis)
{
    CombatStalkerAI* script = static_cast<CombatStalkerAI*>(this);
    if (script && script->getInstanceScript())
    {
        if (Creature* tirion = script->getInstanceScript()->getCreatureFromData(DATA_FORDRING))
            tirion->GetScript()->DoAction(ACTION_START_JORMUNGARS);
    }
}

void CombatStalkerAI::StartIcehowl(CreatureAIFunc pThis)
{
    CombatStalkerAI* script = static_cast<CombatStalkerAI*>(this);
    if (script && script->getInstanceScript())
    {
        if (Creature* tirion = script->getInstanceScript()->getCreatureFromData(DATA_FORDRING))
            tirion->GetScript()->DoAction(ACTION_START_ICEHOWL);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Northrend Beasts
NorthrendBeastsAI::NorthrendBeastsAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Add Boundary
    pCreature->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(LocationVector(563.26f, 139.6f), 75.0));
}

CreatureAIScript* NorthrendBeastsAI::Create(Creature* pCreature) { return new NorthrendBeastsAI(pCreature); }

void NorthrendBeastsAI::InitOrReset()
{
    removeAllFunctionsFromScheduler();
    summons.despawnAll();
    setScriptPhase(Beasts::PHASE_EVENT);
    setReactState(REACT_PASSIVE);

    addAIFunction(&NorthrendBeastsAI::initialMove, DoOnceScheduler(1s));
}

void NorthrendBeastsAI::OnCombatStart(Unit* _target)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
    handleEncounterProgress();

    if (isHeroic())
        handleWithHeroicEvents();
}

void NorthrendBeastsAI::OnCombatStop(Unit* _target)
{
    getInstanceScript()->setLocalData(DATA_DESPAWN_SNOBOLDS, 0);
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

    // prevent losing 2 attempts at once on heroics
    if (getInstanceScript()->getLocalData(TYPE_NORTHREND_BEASTS) != EncounterStates::Failed)
        getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, EncounterStates::Failed);

    // Despawn Combat Stalker
    if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
        combatStalker->Despawn(0, 0);

    summons.despawnAll();
    despawn();
}

void NorthrendBeastsAI::OnDied(Unit* /*_killer*/)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

    if (getCreature()->getEntry() == NPC_GORMOK)
    {
        getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, GORMOK_DONE);
        if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
            combatStalker->GetScript()->DoAction(Beasts::ACTION_GORMOK_DEAD);
    }
    else
        getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, ICEHOWL_DONE);
}

void NorthrendBeastsAI::handleInitialMovement()
{
    switch (getCreature()->getEntry())
    {
        case NPC_GORMOK:
        case NPC_DREADSCALE:
            moveAlongSplineChain(Beasts::POINT_INITIAL_MOVEMENT, SPLINE_INITIAL_MOVEMENT, false);
            break;
        case NPC_ICEHOWL:
            moveAlongSplineChain(Beasts::POINT_INITIAL_MOVEMENT, SPLINE_INITIAL_MOVEMENT, true);
            break;
        default:
            break;
    }
}

void NorthrendBeastsAI::handleEncounterProgress()
{
    switch (getCreature()->getEntry())
    {
        case NPC_GORMOK:
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, GORMOK_IN_PROGRESS);
            break;
        case NPC_ACIDMAW:
        case NPC_DREADSCALE:
            if (getInstanceScript()->getLocalData(TYPE_NORTHREND_BEASTS) != SNAKES_IN_PROGRESS)
                getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_IN_PROGRESS);
            break;
        case NPC_ICEHOWL:
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, ICEHOWL_IN_PROGRESS);
            break;
        default:
            break;
    }
}

void NorthrendBeastsAI::handleWithHeroicEvents()
{
    if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
    {
        if (getCreature()->getEntry() == NPC_GORMOK)
            combatStalker->GetScript()->DoAction(ACTION_START_JORMUNGARS);
        else if (getCreature()->getEntry() == NPC_DREADSCALE)
            combatStalker->GetScript()->DoAction(ACTION_START_ICEHOWL);
    }
}

void NorthrendBeastsAI::initialMove(CreatureAIFunc pThis)
{
    NorthrendBeastsAI* script = static_cast<NorthrendBeastsAI*>(this);
    if (script && script->getInstanceScript())
    {
        handleInitialMovement();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Gormok
GormokAI::GormokAI(Creature* pCreature) : NorthrendBeastsAI(pCreature)
{
    setUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT | UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_PLUS_MOB);
    getCreature()->setAItoUse(true);
    _setWieldWeapon(true);
}

CreatureAIScript* GormokAI::Create(Creature* pCreature) { return new GormokAI(pCreature); }

void GormokAI::OnCombatStart(Unit* _target)
{
    NorthrendBeastsAI::OnCombatStart(_target);

    addAISpell(SpellDesc(Beasts::Gormok::SPELL_IMPALE, FilterArgs(TargetFilter_Current)), DoLoopScheduler(10s, 75.0f));
    addAISpell(SpellDesc(Beasts::Gormok::SPELL_STAGGERING_STOMP, FilterArgs(TargetFilter_Current)), DoLoopScheduler(15s, 75.0f));
    
    addAIFunction(&GormokAI::Throw, DoOnceScheduler(20s, 33.0f));
}

void GormokAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == SPLINE_CHAIN_MOTION_TYPE && id == Beasts::POINT_INITIAL_MOVEMENT)
        addAIFunction(&GormokAI::Engage, DoOnceScheduler(7s));
}

void GormokAI::OnAddPassenger(Unit* _passenger, int8_t _seatId)
{
    if (_seatId == Beasts::GORMOK_HAND_SEAT)
    {
        _passenger->castSpell(getCreature(), Beasts::Gormok::SPELL_RISING_ANGER, true);

        // Find A Target for our Snobol Friend
        if (CreatureAIScript* snoboldAI = _passenger->ToCreature()->GetScript())
        {
            if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player, 0.0f, 0.0f, -Beasts::Gormok::SPELL_SNOBOLLED)))
            {
                sendDBChatMessageByIndex(Beasts::Gormok::EMOTE_SNOBOLLED);
                snoboldAI->DoAction(Beasts::ACTION_ACTIVE_SNOBOLD);
                _passenger->castSpell(target, Beasts::Gormok::SPELL_RIDE_PLAYER, true);
            }
        }
    }
}

void GormokAI::Engage(CreatureAIFunc)
{
    GormokAI* script = static_cast<GormokAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_CLOSE_GATE);

        setImmuneToPC(false);
        setReactState(REACT_AGGRESSIVE);

        // Npc that should keep raid in combat while boss change
        if (Creature* combatStalker = script->summonCreature(NPC_BEASTS_COMBAT_STALKER, CombatStalkerPosition))
        {
            setZoneWideCombat(combatStalker);
            combatStalker->getAIInterface()->instanceCombatProgress(true);
        }
        setZoneWideCombat();
        getCreature()->getAIInterface()->instanceCombatProgress(true);
        setScriptPhase(Beasts::PHASE_COMBAT);
        castSpellOnSelf(Beasts::Gormok::SPELL_TANKING_GORMOK, true);
    }
}

void GormokAI::Throw(CreatureAIFunc pThis)
{
    GormokAI* script = static_cast<GormokAI*>(this);
    if (script && script->getInstanceScript())
    {
        for (uint8_t i = 0; i < Beasts::MAX_SNOBOLDS; ++i)
        {
            Unit* snobold = getCreature()->getVehicleKit()->getPassenger(i);
            if (snobold && snobold->ToCreature())
            {
                snobold->exitVehicle();
                snobold->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                snobold->ToCreature()->GetScript()->DoAction(Beasts::ACTION_DISABLE_FIRE_BOMB);
                snobold->castSpell(getCreature(), Beasts::Gormok::SPELL_JUMP_TO_HAND, true);
                break;
            }
        }

        repeatFunctionFromScheduler(pThis, 20s);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Snobold
SnoboldAI::SnoboldAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    targetGUID = 0;
    mountedOnPlayer = false;
    gormokDead = false;
}

CreatureAIScript* SnoboldAI::Create(Creature* pCreature) { return new SnoboldAI(pCreature); }

void SnoboldAI::OnLoad()
{
    setRooted(true);
}

void SnoboldAI::InitOrReset()
{
    func_CheckMount = addAIFunction(&SnoboldAI::CheckMount, DoOnceScheduler(1s));
    func_FireBomb = addAISpell(SpellDesc(Beasts::Gormok::SPELL_FIRE_BOMB, FilterArgs(TargetFilter_Player), false), DoLoopScheduler(12s, 33.0f));
}

void SnoboldAI::OnDied(Unit* /*_killer*/)
{
    despawn();
}

void SnoboldAI::SetCreatureData64(uint32_t type, uint64_t data)
{
    if (type == Beasts::DATA_NEW_TARGET)
    {
        if (Unit* target = getInstanceScript()->getInstance()->getUnit(data))
        {
            attackStart(target);
            moveChase(target);

            targetGUID = data;

            func_Snowballed = addAISpell(SpellDesc(Beasts::Gormok::SPELL_SNOBOLLED, FilterArgs(TargetFilter(TargetFilter_AOE | TargetFilter_Player)), true), DoOnceScheduler(500ms));
            func_Batter = addAIFunction(&SnoboldAI::Batter, DoOnceScheduler(5s, 50.0f));
            func_HeadCrack = addAIFunction(&SnoboldAI::HeadCrack, DoOnceScheduler(1s));
        }
    }
}

bool SnoboldAI::canAttackTarget(Unit* target)
{
    if (mountedOnPlayer && target->getGuid() != targetGUID)
        return false;

    return true;
}

void SnoboldAI::DoAction(int32_t action)
{
    switch (action)
    {
        case Beasts::ACTION_ENABLE_FIRE_BOMB:
            enableFunctionFromScheduler(func_FireBomb);
            break;
        case Beasts::ACTION_DISABLE_FIRE_BOMB:
            disableFunctionFromScheduler(func_FireBomb);
            break;
        case Beasts::ACTION_ACTIVE_SNOBOLD:
            mountedOnPlayer = true;
            break;
        default:
            break;
    }
}

void SnoboldAI::mountOnBoss()
{
    Unit* gormok = getInstanceScript()->getCreatureFromData(DATA_GORMOK_THE_IMPALER);
    if (gormok && gormok->isAlive())
    {
        attackStop();
        targetGUID = 0;
        mountedOnPlayer = false;

        cancelFunctionFromScheduler(func_Batter);
        cancelFunctionFromScheduler(func_HeadCrack);

        for (uint8_t i = 0; i < Beasts::MAX_SNOBOLDS; i++)
        {
            if (!gormok->getVehicleKit()->getPassenger(i))
            {
                getCreature()->callEnterVehicle(gormok, i);
                DoAction(Beasts::ACTION_ENABLE_FIRE_BOMB);
                break;
            }
        }
    }
    else
    {
        // Disable Spells we Only Use Mounted
        cancelFunctionFromScheduler(func_CheckMount);
        cancelFunctionFromScheduler(func_FireBomb);

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        attackStop();

        setRooted(false);
        gormokDead = true;

        if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player)))
        {
            attackStart(target);
            moveChase(target);

            targetGUID = target->getGuid();

            func_Batter = addAIFunction(&SnoboldAI::Batter, DoOnceScheduler(5s, 50.0f));
            func_HeadCrack = addAIFunction(&SnoboldAI::HeadCrack, DoOnceScheduler(1s));
        }
    }
}

void SnoboldAI::CheckMount(CreatureAIFunc pThis)
{
    SnoboldAI* script = static_cast<SnoboldAI*>(this);
    if (script && script->getInstanceScript())
    {
        auto base = getCreature()->getVehicleBase();
        if (!getCreature()->getVehicleBase())
            mountOnBoss();        

        repeatFunctionFromScheduler(pThis, 3s);
    }
}

void SnoboldAI::Batter(CreatureAIFunc pThis)
{
    SnoboldAI* script = static_cast<SnoboldAI*>(this);
    if (script && script->getInstanceScript())
    {
        if (Unit* target = getCreature()->getVehicleBase())
            castSpell(target, Beasts::Gormok::SPELL_BATTER);
        else
            castSpellOnVictim(Beasts::Gormok::SPELL_BATTER);

        repeatFunctionFromScheduler(pThis, 10s);
    }
}

void SnoboldAI::HeadCrack(CreatureAIFunc pThis)
{
    SnoboldAI* script = static_cast<SnoboldAI*>(this);
    if (script && script->getInstanceScript())
    {
        if (Unit* target = getCreature()->getVehicleBase())
            castSpell(target, Beasts::Gormok::SPELL_HEAD_CRACK);
        else
            castSpellOnVictim(Beasts::Gormok::SPELL_HEAD_CRACK);

        repeatFunctionFromScheduler(pThis, 30s);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Firebomb
FireBombAI::FireBombAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    getCreature()->setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
}

CreatureAIScript* FireBombAI::Create(Creature* pCreature) { return new FireBombAI(pCreature); }

void FireBombAI::InitOrReset()
{
    // for whatever reason we cannot cast spells directly when spawned
    addAIFunction([this](CreatureAIFunc pThis)
        {
            castSpellAOE(Beasts::Gormok::SPELL_FIRE_BOMB_AURA);
        }, DoOnceScheduler(500ms, 100.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Dreadscale
DreadscaleAI::DreadscaleAI(Creature* pCreature) : NorthrendBeastsAI(pCreature)
{
    setUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT );
}

CreatureAIScript* DreadscaleAI::Create(Creature* pCreature) { return new DreadscaleAI(pCreature); }

void DreadscaleAI::InitOrReset()
{
    NorthrendBeastsAI::InitOrReset();

    wasMobile = true;
}

void DreadscaleAI::OnCombatStart(Unit* _target)
{
    NorthrendBeastsAI::OnCombatStart(_target);
    addTasks();
}

void DreadscaleAI::OnDied(Unit* _killer)
{
    if (CreatureAIScript* otherWorm = getLinkedCreatureAIScript())
    {
        if (otherWorm->isAlive())
        {
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_SPECIAL);
            otherWorm->DoAction(Beasts::ACTION_ENRAGE);
        }
        else
        {
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);

            despawn();
            otherWorm->despawn();

            if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
                combatStalker->GetScript()->DoAction(Beasts::ACTION_JORMUNGARS_DEAD);
        }
    }
    else
    {
        // We assume when the Link Broke That the Other Worm Died and got Removed from World
        getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);

        despawn();

        if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
            combatStalker->GetScript()->DoAction(Beasts::ACTION_JORMUNGARS_DEAD);
    }

    // Hide Encounter Frame
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
}

void DreadscaleAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == SPLINE_CHAIN_MOTION_TYPE && id == Beasts::POINT_INITIAL_MOVEMENT)
        addAIFunction(&DreadscaleAI::Engage, DoOnceScheduler(3s));
}

void DreadscaleAI::DoAction(int32_t action)
{
    if (action == Beasts::ACTION_ENRAGE)
    {
        addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_ENRAGE, FilterArgs(TargetFilter_Self), true), DoOnceScheduler());
        addMessage(Message(Beasts::Dreadscale_Acidmaw::EMOTE_ENRAGE), DoOnceScheduler());

        if (isScriptPhase(Beasts::PHASE_SUBMERGED))
        {
            repeatFunctionFromScheduler(func_Emerge, 1ms);
        }
    }
}

void DreadscaleAI::addTasks()
{
    DoLoopScheduler sprayScheduler;
    sprayScheduler.setInitialCooldown(16s);
    sprayScheduler.setAvailableForScriptPhase({ Beasts::PHASE_STATIONARY });
    sprayScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_BURNING_SPRAY, FilterArgs(TargetFilter_Player), false), sprayScheduler);

    DoLoopScheduler sweepScheduler;
    sweepScheduler.setInitialCooldown(17s);
    sweepScheduler.setAvailableForScriptPhase({ Beasts::PHASE_STATIONARY });
    sweepScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_SWEEP, FilterArgs(TargetFilter_AOE), false), sweepScheduler);

    DoLoopScheduler spitScheduler;
    spitScheduler.setInitialCooldown(2s);
    spitScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    spitScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_FIRE_SPIT, FilterArgs(TargetFilter_Current), false), spitScheduler);

    DoLoopScheduler biteScheduler;
    biteScheduler.setInitialCooldown(5s);
    biteScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_BURNING_BITE, FilterArgs(TargetFilter_Current), false), biteScheduler);

    DoLoopScheduler spewScheduler;
    spewScheduler.setInitialCooldown(10s);
    spewScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_MOLTEN_SPEW, FilterArgs(TargetFilter_AOE), false), spewScheduler);

    DoLoopScheduler poolScheduler;
    poolScheduler.setInitialCooldown(14s);
    poolScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SUMMON_SLIME_POOL, FilterArgs(TargetFilter_Self), false), poolScheduler);

    func_Submerge = addAIFunction(&DreadscaleAI::submerge, DoOnceScheduler(45s));
}

void DreadscaleAI::Engage(CreatureAIFunc)
{
    DreadscaleAI* script = static_cast<DreadscaleAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_CLOSE_GATE);

        setScriptPhase(Beasts::PHASE_MOBILE);

        setImmuneToPC(false);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
        // Spawn Acidmaw and Link our Scripts
        if (Creature* acidmaw = summonCreature(NPC_ACIDMAW, ToCCommonLoc[9]))
        {
            setLinkedCreatureAIScript(acidmaw->GetScript());
            acidmaw->GetScript()->setLinkedCreatureAIScript(this);
        }
    }
}

void DreadscaleAI::submerge(CreatureAIFunc pThis)
{
    DreadscaleAI* script = static_cast<DreadscaleAI*>(this);
    if (script && script->getInstanceScript())
    {
        setReactState(REACT_PASSIVE);
        attackStop();

        if (script->wasMobile)
        {
            addEmote(Emote(EMOTE_ONESHOT_SUBMERGE), DoOnceScheduler());
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE);

            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0, true);

            setScriptPhase(Beasts::PHASE_SUBMERGED);
            
            func_Emerge = addAIFunction(&DreadscaleAI::emerge, DoOnceScheduler(5s));
            setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        else
        {
            addEmote(Emote(EMOTE_ONESHOT_SUBMERGE), DoOnceScheduler());
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE_2);
            _removeAura(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_1);

            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0, true);

            setScriptPhase(Beasts::PHASE_SUBMERGED);

            setControlled(false, UNIT_STATE_ROOTED);

            func_Emerge = addAIFunction(&DreadscaleAI::emerge, DoOnceScheduler(5s));
            setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        movePoint(0, ToCCommonLoc[1].getPositionX() + Util::getRandomFloat(-40.0f, 40.0f), ToCCommonLoc[1].getPositionY() + Util::getRandomFloat(-40.0f, 40.0f), ToCCommonLoc[1].getPositionZ() + script->getCreature()->getCollisionHeight());
    }
}

void DreadscaleAI::emerge(CreatureAIFunc pThis)
{
    DreadscaleAI* script = static_cast<DreadscaleAI*>(this);
    if (script && script->getInstanceScript())
    {
        uint32_t submergeSpell = wasMobile ? Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE : Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE_2;

        _removeAura(submergeSpell);
        _removeAura(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0);

        castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_EMERGE);
        castSpellAOE(Beasts::Dreadscale_Acidmaw::SPELL_HATE_TO_ZERO, true);

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        if (Unit* target = selectUnitTarget(FilterArgs()))
            attackStart(target);

        // if the worm was mobile before submerging, make him stationary now
        if (wasMobile)
        {
            setControlled(true, UNIT_STATE_ROOTED);
            _setDisplayId(Beasts::Dreadscale_Acidmaw::MODEL_DREADSCALE_STATIONARY);
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_1, true);
            setScriptPhase(Beasts::PHASE_STATIONARY);
        }
        else
        {
            if (Unit* target = getCreature()->getAIInterface()->getCurrentTarget())
                moveChase(target);

            getCreature()->setDisplayId(Beasts::Dreadscale_Acidmaw::MODEL_DREADSCALE_MOBILE);
            setScriptPhase(Beasts::PHASE_MOBILE);
        }
        wasMobile = !wasMobile;

        cancelFunctionFromScheduler(pThis);
        resetAllFunctionsFromScheduler();
        func_Submerge = addAIFunction(&DreadscaleAI::submerge, DoOnceScheduler(45s));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Acidmaw
AcidmawAI::AcidmawAI(Creature* pCreature) : NorthrendBeastsAI(pCreature) { }

CreatureAIScript* AcidmawAI::Create(Creature* pCreature) { return new AcidmawAI(pCreature); }

void AcidmawAI::InitOrReset()
{
    NorthrendBeastsAI::InitOrReset();

    wasMobile = false;

    setScriptPhase(Beasts::PHASE_STATIONARY);

    setControlled(true, UNIT_STATE_ROOTED);

    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_1, FilterArgs(TargetFilter_Self), true), DoOnceScheduler());
    addAIFunction(&AcidmawAI::Engage, DoOnceScheduler(3s));
}

void AcidmawAI::OnCombatStart(Unit* _target)
{
    NorthrendBeastsAI::OnCombatStart(_target);

    addTasks();
}

void AcidmawAI::OnDied(Unit* _killer)
{
    if (CreatureAIScript* otherWorm = getLinkedCreatureAIScript())
    {
        if (otherWorm->isAlive())
        {
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_SPECIAL);
            otherWorm->DoAction(Beasts::ACTION_ENRAGE);
        }
        else
        {
            getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);

            despawn();
            otherWorm->despawn();

            if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
                combatStalker->GetScript()->DoAction(Beasts::ACTION_JORMUNGARS_DEAD);
        }
    }
    else
    {
        // We assume when the Link Broke That the Other Worm Died and got Removed from World
        getInstanceScript()->setLocalData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);

        despawn();

        if (Creature* combatStalker = getInstanceScript()->getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
            combatStalker->GetScript()->DoAction(Beasts::ACTION_JORMUNGARS_DEAD);
    }

    // Hide Encounter Frame
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
}

void AcidmawAI::DoAction(int32_t action)
{
    if (action == Beasts::ACTION_ENRAGE)
    {
        addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_ENRAGE, FilterArgs(TargetFilter_Self), true), DoOnceScheduler());
        addMessage(Message(Beasts::Dreadscale_Acidmaw::EMOTE_ENRAGE), DoOnceScheduler());

        if (isScriptPhase(Beasts::PHASE_SUBMERGED))
        {
            repeatFunctionFromScheduler(func_Emerge, 1ms);
        }
    }
}

void AcidmawAI::addTasks()
{
    DoLoopScheduler sprayScheduler;
    sprayScheduler.setInitialCooldown(16s);
    sprayScheduler.setAvailableForScriptPhase({ Beasts::PHASE_STATIONARY });
    sprayScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_PARALYTIC_SPRAY, FilterArgs(TargetFilter_Player), false), sprayScheduler);

    DoLoopScheduler sweepScheduler;
    sweepScheduler.setInitialCooldown(17s);
    sweepScheduler.setAvailableForScriptPhase({ Beasts::PHASE_STATIONARY });
    sweepScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_SWEEP, FilterArgs(TargetFilter_AOE), false), sweepScheduler);

    DoLoopScheduler spitScheduler;
    spitScheduler.setInitialCooldown(2s);
    spitScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    spitScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_ACID_SPIT, FilterArgs(TargetFilter_Current), false), spitScheduler);

    DoLoopScheduler biteScheduler;
    biteScheduler.setInitialCooldown(5s);
    biteScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_PARALYTIC_BITE, FilterArgs(TargetFilter_Current), false), biteScheduler);

    DoLoopScheduler spewScheduler;
    spewScheduler.setInitialCooldown(10s);
    spewScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SPELL_ACID_SPEW, FilterArgs(TargetFilter_AOE), false), spewScheduler);

    DoLoopScheduler poolScheduler;
    poolScheduler.setInitialCooldown(14s);
    poolScheduler.setAvailableForScriptPhase({ Beasts::PHASE_MOBILE });
    addAISpell(SpellDesc(Beasts::Dreadscale_Acidmaw::SUMMON_SLIME_POOL, FilterArgs(TargetFilter_Self), false), poolScheduler);

    func_Submerge = addAIFunction(&AcidmawAI::submerge, DoOnceScheduler(45s));
}

void AcidmawAI::Engage(CreatureAIFunc)
{
    AcidmawAI* script = static_cast<AcidmawAI*>(this);
    if (script && script->getInstanceScript())
    {
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }
}

void AcidmawAI::submerge(CreatureAIFunc pThis)
{
    AcidmawAI* script = static_cast<AcidmawAI*>(this);
    if (script && script->getInstanceScript())
    {
        setReactState(REACT_PASSIVE);
        attackStop();

        if (script->wasMobile)
        {
            addEmote(Emote(EMOTE_ONESHOT_SUBMERGE), DoOnceScheduler());
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE);

            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0, true);

            setScriptPhase(Beasts::PHASE_SUBMERGED);

            func_Emerge = addAIFunction(&AcidmawAI::emerge, DoOnceScheduler(5s));
            setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        else
        {
            addEmote(Emote(EMOTE_ONESHOT_SUBMERGE), DoOnceScheduler());
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE_2);
            _removeAura(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_1);

            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0, true);

            setScriptPhase(Beasts::PHASE_SUBMERGED);

            setControlled(false, UNIT_STATE_ROOTED);

            func_Emerge = addAIFunction(&AcidmawAI::emerge, DoOnceScheduler(5s));
            setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        movePoint(0, ToCCommonLoc[1].getPositionX() + Util::getRandomFloat(-40.0f, 40.0f), ToCCommonLoc[1].getPositionY() + Util::getRandomFloat(-40.0f, 40.0f), ToCCommonLoc[1].getPositionZ() + script->getCreature()->getCollisionHeight());
    }
}

void AcidmawAI::emerge(CreatureAIFunc pThis)
{
    AcidmawAI* script = static_cast<AcidmawAI*>(this);
    if (script && script->getInstanceScript())
    {
        uint32_t submergeSpell = wasMobile ? Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE : Beasts::Dreadscale_Acidmaw::SPELL_SUBMERGE_2;

        _removeAura(submergeSpell);
        _removeAura(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_0);

        castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_EMERGE);
        castSpellAOE(Beasts::Dreadscale_Acidmaw::SPELL_HATE_TO_ZERO, true);

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        if (Unit* target = selectUnitTarget(FilterArgs()))
            attackStart(target);

        // if the worm was mobile before submerging, make him stationary now
        if (wasMobile)
        {
            setControlled(true, UNIT_STATE_ROOTED);
            _setDisplayId(Beasts::Dreadscale_Acidmaw::MODEL_ACIDMAW_STATIONARY);
            castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_GROUND_VISUAL_1, true);
            setScriptPhase(Beasts::PHASE_STATIONARY);
        }
        else
        {
            if (Unit* target = getCreature()->getAIInterface()->getCurrentTarget())
                moveChase(target);

            _setDisplayId(Beasts::Dreadscale_Acidmaw::MODEL_ACIDMAW_MOBILE);
            setScriptPhase(Beasts::PHASE_MOBILE);
        }
        wasMobile = !wasMobile;

        cancelFunctionFromScheduler(pThis);
        resetAllFunctionsFromScheduler();
        func_Submerge = addAIFunction(&AcidmawAI::submerge, DoOnceScheduler(45s));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Slime Pool
SlimePoolAI::SlimePoolAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* SlimePoolAI::Create(Creature* pCreature) { return new SlimePoolAI(pCreature); }

void SlimePoolAI::OnLoad()
{
    addAIFunction(&SlimePoolAI::slimeEffect, DoOnceScheduler(1s));
}

void SlimePoolAI::slimeEffect(CreatureAIFunc pThis)
{
    castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_SLIME_POOL_EFFECT, true);
    castSpellOnSelf(Beasts::Dreadscale_Acidmaw::SPELL_PACIFY_SELF, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Icehowl
IcehowlAI::IcehowlAI(Creature* pCreature) : NorthrendBeastsAI(pCreature)
{
    setUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_SWIMMING);
}

CreatureAIScript* IcehowlAI::Create(Creature* pCreature) { return new IcehowlAI(pCreature); }

void IcehowlAI::OnCombatStart(Unit* _target)
{
    NorthrendBeastsAI::OnCombatStart(_target);

    addTasks();
}

void IcehowlAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE && type != SPLINE_CHAIN_MOTION_TYPE)
        return;

    switch (id)
    {
        case Beasts::POINT_INITIAL_MOVEMENT:
            addAIFunction(&IcehowlAI::Engage, DoOnceScheduler(3s));
            break;
        case Beasts::POINT_MIDDLE:
            castSpellOnSelf(Beasts::Icehowl::SPELL_MASSIVE_CRASH);
            addAIFunction(&IcehowlAI::Charge, DoOnceScheduler(4s));
            break;
        case Beasts::POINT_ICEHOWL_CHARGE:
            removeAllFunctionsFromScheduler();
            setScriptPhase(Beasts::PHASE_COMBAT);
            addTasks();
            setReactState(REACT_AGGRESSIVE);
            castSpellOnSelf(Beasts::Icehowl::SPELL_TRAMPLE);
            break;
        default:
            break;
    }
}

void IcehowlAI::DoAction(int32_t action)
{
    if (action == Beasts::ACTION_ENRAGE)
    {
        castSpellOnSelf(Beasts::Icehowl::SPELL_FROTHING_RAGE, true);
        addMessage(Message(Beasts::Icehowl::EMOTE_TRAMPLE_ENRAGE), DoOnceScheduler());
    }
    else if (action == Beasts::ACTION_TRAMPLE_FAIL)
    {
        castSpellOnSelf(Beasts::Icehowl::SPELL_STAGGERED_DAZE, true);
        addMessage(Message(Beasts::Icehowl::EMOTE_TRAMPLE_FAIL), DoOnceScheduler());
        delayAllFunctions(15s);
    }
}

void IcehowlAI::addTasks()
{
    DoLoopScheduler ferociousScheduler;
    ferociousScheduler.setInitialCooldown(15s);
    ferociousScheduler.setAvailableForScriptPhase({ Beasts::PHASE_COMBAT });
    ferociousScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Icehowl::SPELL_FEROCIOUS_BUTT, FilterArgs(TargetFilter_Current)), ferociousScheduler);

    DoLoopScheduler arcticBreathScheduler;
    arcticBreathScheduler.setInitialCooldown(20s);
    arcticBreathScheduler.setAvailableForScriptPhase({ Beasts::PHASE_COMBAT });
    arcticBreathScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Icehowl::SPELL_ARCTIC_BREATH, FilterArgs(TargetFilter_Player)), arcticBreathScheduler);

    DoLoopScheduler WhirlScheduler;
    WhirlScheduler.setInitialCooldown(15s);
    WhirlScheduler.setAvailableForScriptPhase({ Beasts::PHASE_COMBAT });
    WhirlScheduler.setChance(75.0f);
    addAISpell(SpellDesc(Beasts::Icehowl::SPELL_ARCTIC_BREATH, FilterArgs(TargetFilter_Self)), WhirlScheduler);

    DoLoopScheduler crashScheduler;
    crashScheduler.setInitialCooldown(35s);
    crashScheduler.setAvailableForScriptPhase({ Beasts::PHASE_COMBAT });
    crashScheduler.setChance(75.0f);
    addAIFunction(&IcehowlAI::Crash, crashScheduler);
}

void IcehowlAI::Engage(CreatureAIFunc pThis)
{
    getInstanceScript()->DoAction(ACTION_CLOSE_GATE);
    setImmuneToPC(false);
    setScriptPhase(Beasts::PHASE_COMBAT);
    setReactState(REACT_AGGRESSIVE);
    setZoneWideCombat();
}

void IcehowlAI::Charge(CreatureAIFunc pThis)
{
    if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player)))
    {
        // Spell is not in dbcs so hackfix it until we support custom spells
        //castSpell(target, Beasts::Icehowl::SPELL_FURIOUS_CHARGE_SUMMON, true);
        summonCreature(NPC_FURIOUS_CHARGE_STALKER, target->GetPosition(), CreatureSummonDespawnType::TIMED_DESPAWN, 18 * TimeVarsMs::Second);
        
        addMessage(Message(Beasts::Icehowl::EMOTE_TRAMPLE_ROAR, target), DoOnceScheduler());
        addAIFunction(&IcehowlAI::Roar, DoOnceScheduler(1s));
    }
}

void IcehowlAI::Roar(CreatureAIFunc pThis)
{
    if (Creature* stalker = getInstanceScript()->getCreatureFromData(DATA_FURIOUS_CHARGE))
        castSpell(stalker, Beasts::Icehowl::SPELL_ROAR);

    DoOnceScheduler mSchedulerArgs;
    mSchedulerArgs.setAvailableForScriptPhase({ Beasts::PHASE_CHARGE });
    mSchedulerArgs.setInitialCooldown(3s);
    addAIFunction(&IcehowlAI::JumpBack, mSchedulerArgs);
}

void IcehowlAI::JumpBack(CreatureAIFunc pThis)
{
    if (Creature* stalker = getInstanceScript()->getCreatureFromData(DATA_FURIOUS_CHARGE))
        castSpell(stalker, Beasts::Icehowl::SPELL_JUMP_BACK);

    DoOnceScheduler mSchedulerArgs;
    mSchedulerArgs.setAvailableForScriptPhase({ Beasts::PHASE_CHARGE });
    mSchedulerArgs.setInitialCooldown(2s);
    addAIFunction(&IcehowlAI::Trample, mSchedulerArgs);
}

void IcehowlAI::Crash(CreatureAIFunc pThis)
{
    setReactState(REACT_PASSIVE);
    attackStop();
    setScriptPhase(Beasts::PHASE_CHARGE);
    moveJump(ToCCommonLoc[1], 20.0f, 20.0f, POINT_MIDDLE);
}

void IcehowlAI::Trample(CreatureAIFunc pThis)
{
    if (Creature* stalker = getInstanceScript()->getCreatureFromData(DATA_FURIOUS_CHARGE))
        moveCharge(stalker->GetPositionX(), stalker->GetPositionY(), stalker->GetPositionZ(), 42.0f, Beasts::POINT_ICEHOWL_CHARGE);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Ride Player
void RidePlayer::onAuraCreate(Aura* aur)
{
    Unit* target = aur->GetPlayerTarget();
    if (!target->isPlayer() || !target->IsInWorld())
        return;

    if (!target->createVehicleKit(PLAYER_VEHICLE_ID, 0))
        return;

    if (Unit* caster = aur->GetUnitCaster())
    {
        if (caster->ToCreature() && caster->ToCreature()->GetScript())
            caster->ToCreature()->GetScript()->SetCreatureData64(Beasts::DATA_NEW_TARGET, target->getGuid());
    }      
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Snobolled
SpellScriptExecuteState Snobolled::onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage)
{
    if (!aur->GetUnitTarget()->hasAurasWithId(Beasts::Gormok::SPELL_RIDE_PLAYER))
        aur->removeAura();

    if (Vehicle* vehicle = aur->GetUnitTarget()->getVehicleKit())
    {
        if (!vehicle->getPassenger(0))
            aur->removeAura();
    }

    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Firebomb
SpellScriptExecuteState Firebomb::beforeSpellEffect(Spell* spell, uint8_t effIndex)
{
    //if (effIndex == EFF_INDEX_1)
    {
        spell->getUnitCaster()->summonCreature(NPC_FIREBOMB, spell->getDestination(), CreatureSummonDespawnType::TIMED_DESPAWN, 35 * TimeVarsMs::Second);

        //return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Paralytic Spray
SpellScriptExecuteState ParalyticSpray::beforeSpellEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex != EFF_INDEX_0)
        return SpellScriptExecuteState::EXECUTE_PREVENT;

    spell->getUnitCaster()->castSpell(spell->getUnitTarget(), Beasts::Dreadscale_Acidmaw::SPELL_PARALYTIC_TOXIN, true);
    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Paralytic Toxin
void ParalyticToxin::onAuraApply(Aura* aur)
{
    counter = -10;

    Unit* caster = aur->GetUnitCaster();
    if (caster && caster->getEntry() == NPC_ACIDMAW)
        if (Creature* acidmaw = caster->ToCreature())
            acidmaw->GetScript()->sendDBChatMessageByIndex(Beasts::Dreadscale_Acidmaw::SAY_SPECIAL, aur->GetUnitTarget());
}

void ParalyticToxin::onAuraRemove(Aura* aur, AuraRemoveMode mode)
{
    aur->GetUnitTarget()->removeAllAurasById(Beasts::Dreadscale_Acidmaw::SPELL_PARALYSIS);
}

void ParalyticToxin::onAuraRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount)
{
    if (auto slowEff = aur->getAuraEffect(EFF_INDEX_0))
    {
        int32_t newAmount = counter - 10;
        if (counter < -100)
            counter = -90;

        counter = newAmount;

        if (newAmount == -100 && !aur->GetUnitTarget()->hasAurasWithId(Beasts::Dreadscale_Acidmaw::SPELL_PARALYSIS))
            aur->GetUnitTarget()->castSpell(aur->GetUnitTarget(), Beasts::Dreadscale_Acidmaw::SPELL_PARALYSIS);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Burning Spray
SpellScriptExecuteState BurningSpray::beforeSpellEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex != EFF_INDEX_0)
        return SpellScriptExecuteState::EXECUTE_PREVENT;

    spell->getUnitCaster()->castSpell(spell->getUnitTarget(), Beasts::Dreadscale_Acidmaw::SPELL_BURNING_BILE, true);
    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Burning Bile
SpellScriptExecuteState BurningBile::beforeSpellEffect(Spell* spell, uint8_t effectIndex)
{
    if (effectIndex != EFF_INDEX_1)
        return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

    spell->getUnitTarget()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_DECREASE_SPEED);
    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Slime Pool
SpellScriptExecuteState SlimePool::onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage)
{
    if (aurEff->getEffectIndex() != EFF_INDEX_0)
        return SpellScriptExecuteState::EXECUTE_OK;

    ++stackCounter;
    int32_t const radius = static_cast<int32_t>(((stackCounter / 60.f) * 0.9f + 0.1f) * 10000.f * 2.f / 3.f);

    SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(aur->getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex()));
    Unit* caster = aur->GetUnitTarget();
    
    // Cast Spell
    // todo spellradius should change...
    caster->castSpell(nullptr, spellInfo, true);
    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell paralysis
void Paralysis::onAuraApply(Aura* aur)
{
    if (Unit* caster = aur->GetUnitCaster())
    {
        if (caster->IsInWorld())
        {
            if (InstanceScript* instance = caster->getWorldMap()->getScript())
            {
                if (instance->getBossState(DATA_NORTHREND_BEASTS) == EncounterStates::Performed)
                    return;
            }
        }
    }

    aur->removeAura();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Arctic Breath
bool ArcticBreathEffect(uint8_t effectIndex, Spell* pSpell)
{
    if (effectIndex == EFF_INDEX_0)
    {
        uint32_t spellId = pSpell->calculateEffect(effectIndex);
        pSpell->getUnitCaster()->castSpell(pSpell->getUnitTarget(), spellId, true);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Trample
void Trample::filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    if (effectIndex == EFF_INDEX_0)
    {
        Creature* caster = spell->getCaster()->ToCreature();
        if (!caster || !caster->GetScript())
            return;

        if (!effectTargets->size())
            caster->GetScript()->DoAction(Beasts::ACTION_TRAMPLE_FAIL);
        else
            caster->GetScript()->DoAction(Beasts::ACTION_ENRAGE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Massive Crash
SpellScriptExecuteState MassiceCrash::beforeSpellEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex == EFF_INDEX_2)
    {
        if (Player* target = spell->getUnitTarget()->ToPlayer())
            if (target->getRaidDifficulty() != InstanceDifficulty::RAID_10MAN_HEROIC || target->getRaidDifficulty() != InstanceDifficulty::RAID_25MAN_HEROIC)
                target->castSpell(target, Beasts::Icehowl::SPELL_SURGE_OF_ADRENALINE, true);
    }

    return SpellScriptExecuteState::EXECUTE_OK;
}
