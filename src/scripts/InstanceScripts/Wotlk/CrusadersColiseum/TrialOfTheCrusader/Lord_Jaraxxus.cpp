/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Lord_Jaraxxus.hpp"
#include "Raid_TrialOfTheCrusader.hpp"
#include "Map/AreaBoundary.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "CommonTime.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
///  Jaraxxus
JaraxxusAI::JaraxxusAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Add Boundary
    pCreature->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(LocationVector(563.26f, 139.6f), 75.0));

    setUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT | UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_PLUS_MOB);

    addEmoteForEventByIndex(Event_OnCombatStart, jaraxxus::SAY_AGGRO);
    addEmoteForEventByIndex(Event_OnTargetDied, jaraxxus::SAY_KILL_PLAYER);
    addEmoteForEventByIndex(Event_OnDied, jaraxxus::SAY_DEATH);
}

CreatureAIScript* JaraxxusAI::Create(Creature* pCreature) { return new JaraxxusAI(pCreature); }

void JaraxxusAI::InitOrReset()
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) == EncounterStates::NotStarted)
    {
        DoAction(ACTION_JARAXXUS_INTRO);
    }
    else if (getInstanceScript()->getBossState(DATA_JARAXXUS) == EncounterStates::Failed)
    {
        castSpellOnSelf(SPELL_JARAXXUS_CHAINS);
        setImmuneToPC(true);
        setReactState(REACT_PASSIVE);
    }
}

void JaraxxusAI::OnCombatStart(Unit* _target)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());

    /////////////////////////////////////////
    // Fel Fireball
    DoLoopScheduler mFelFireballArgs;
    mFelFireballArgs.setInitialCooldown(8s);
    mFelFireballArgs.setChance(50.0f);
    addAISpell(SpellDesc(jaraxxus::SPELL_FEL_FIREBALL, FilterArgs(TargetFilter_Player), false), mFelFireballArgs);

    /////////////////////////////////////////
    // Fel Lightning
    DoLoopScheduler mFelLightningArgs;
    mFelLightningArgs.setInitialCooldown(15s);
    mFelLightningArgs.setChance(33.0f);
    addAISpell(SpellDesc(jaraxxus::SPELL_FEL_LIGHTNING, FilterArgs(TargetFilter_Player), false), mFelFireballArgs);

    /////////////////////////////////////////
    // Incinerate Flesh
    DoLoopScheduler mIncinerateFleshgArgs;
    mIncinerateFleshgArgs.setInitialCooldown(16s);
    mIncinerateFleshgArgs.setChance(33.0f);

    SpellDesc mSpellIncinerateFlesh;
    mSpellIncinerateFlesh.addDBMessage(jaraxxus::EMOTE_INCINERATE, SLOT_DEFAULT, true);
    mSpellIncinerateFlesh.addDBMessage(jaraxxus::SAY_INCINERATE, SLOT_BONUS);
    mSpellIncinerateFlesh.setSpellId(jaraxxus::SPELL_INCINERATE_FLESH);
    mSpellIncinerateFlesh.setTargetFilters(FilterArgs(TargetFilter_Player));
    addAISpell(mSpellIncinerateFlesh, mIncinerateFleshgArgs);

    /////////////////////////////////////////
    // Nether Power
    DoLoopScheduler mNetherPowerArgs;
    mNetherPowerArgs.setInitialCooldown(22s);
    mNetherPowerArgs.setChance(33.0f);
    addAISpell(SpellDesc(jaraxxus::SPELL_NETHER_POWER, FilterArgs(TargetFilter_Self), true), mNetherPowerArgs);

    /////////////////////////////////////////
    // Legion Flame
    DoLoopScheduler mLegionFlameArgs;
    mLegionFlameArgs.setInitialCooldown(20s);
    mLegionFlameArgs.setChance(25.0f);

    SpellDesc mSpellLegionFlame;
    mSpellLegionFlame.addDBMessage(jaraxxus::EMOTE_LEGION_FLAME);
    mSpellLegionFlame.setSpellId(jaraxxus::SPELL_LEGION_FLAME);
    mSpellLegionFlame.setTargetFilters(FilterArgs(TargetFilter(TargetFilter_NotCurrent | TargetFilter_Player)));
    addAISpell(mSpellLegionFlame, mLegionFlameArgs);

    /////////////////////////////////////////
    // Summon Nether Portal
    DoOnceScheduler mSummonPortalArgs;
    mSummonPortalArgs.setInitialCooldown(20s);
    mSummonPortalArgs.setChance(75.0f);
    addAIFunction(&JaraxxusAI::summonPortal, mSummonPortalArgs);

    /////////////////////////////////////////
    // Summon Infernal
    DoOnceScheduler mSummonInfernalArgs;
    mSummonInfernalArgs.setInitialCooldown(1min + 20s);
    mSummonInfernalArgs.setChance(75.0f);
    addAIFunction(&JaraxxusAI::summonInfernal, mSummonInfernalArgs);

    /////////////////////////////////////////
    // Enrage
    DoOnceScheduler mEnrageArgs;
    mEnrageArgs.setInitialCooldown(10min);
    mEnrageArgs.setChance(100.0f);

    SpellDesc mSpellEnrage;
    mSpellEnrage.addDBMessage(jaraxxus::SAY_BERSERK);
    mSpellEnrage.setSpellId(jaraxxus::SPELL_BERSERK);
    mSpellEnrage.setTriggered(true);
    mSpellEnrage.setTargetFilters(FilterArgs(TargetFilter_Self));
    addAISpell(mSpellEnrage, mEnrageArgs);
}

void JaraxxusAI::OnCombatStop(Unit* /*_target*/)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
}

void JaraxxusAI::OnDied(Unit* /*_killer*/)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
}

void JaraxxusAI::OnCastSpell(uint32_t spellId)
{
    if (spellId == jaraxxus::SPELL_NETHER_POWER)
        if (Aura* aura = getCreature()->getAuraWithId(jaraxxus::SPELL_NETHER_POWER))
            aura->refreshOrModifyStack(false, getRaidModeValue(5, 10, 5, 10));
}

void JaraxxusAI::DoAction(int32_t action)
{
    if (action == ACTION_JARAXXUS_INTRO)
    {
        setReactState(REACT_PASSIVE);
        setScriptPhase(jaraxxus::PHASE_INTRO);
        addAIFunction(&JaraxxusAI::intro, DoOnceScheduler(1s));
    }
    else if (action == ACTION_JARAXXUS_ENGAGE)
    {
        _removeAura(SPELL_JARAXXUS_CHAINS);
        setImmuneToPC(false);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }
}

void JaraxxusAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == SPLINE_CHAIN_MOTION_TYPE && id == POINT_SUMMON)
    {
        if (Creature* wilfred = getInstanceScript()->getCreatureFromData(DATA_FIZZLEBANG))
        {
            getCreature()->setFacingToObject(wilfred);

            addAIFunction(&JaraxxusAI::taunt, DoOnceScheduler(9s));
        }
    }
}

void JaraxxusAI::intro(CreatureAIFunc pThis)
{
    castSpellOnSelf(jaraxxus::SPELL_LORD_JARAXXUS_HITTIN_YA, true);
    moveAlongSplineChain(jaraxxus::POINT_SUMMONED, SPLINE_INITIAL_MOVEMENT, true);
}

void JaraxxusAI::taunt(CreatureAIFunc pThis)
{
    addMessage(Message(jaraxxus::SAY_INTRO), DoOnceScheduler());
    addAIFunction(&JaraxxusAI::killGnome, DoOnceScheduler(9s));
}

void JaraxxusAI::killGnome(CreatureAIFunc pThis)
{
    if (Creature* wilfred = getInstanceScript()->getCreatureFromData(DATA_FIZZLEBANG))
        castSpell(wilfred, jaraxxus::SPELL_FEL_LIGHTNING_INTRO);

    addAIFunction(&JaraxxusAI::faceto, DoOnceScheduler(3s));
}

void JaraxxusAI::faceto(CreatureAIFunc pThis)
{
    getCreature()->setFacingTo(4.729842f);
    addAIFunction(&JaraxxusAI::startCombat, DoOnceScheduler(7s));
}

void JaraxxusAI::startCombat(CreatureAIFunc pThis)
{
    setImmuneToPC(false);
    setReactState(REACT_AGGRESSIVE);
    setZoneWideCombat();
}

void JaraxxusAI::summonPortal(CreatureAIFunc pThis)
{
    addMessage(Message(jaraxxus::EMOTE_NETHER_PORTAL), DoOnceScheduler());
    addMessage(Message(jaraxxus::SAY_MISTRESS_OF_PAIN), DoOnceScheduler());
    castSpellOnSelf(jaraxxus::SPELL_NETHER_PORTAL);

    repeatFunctionFromScheduler(pThis, 2min);
}

void JaraxxusAI::summonInfernal(CreatureAIFunc pThis)
{
    addMessage(Message(jaraxxus::EMOTE_INFERNAL_ERUPTION), DoOnceScheduler());
    addMessage(Message(jaraxxus::SAY_INFERNAL_ERUPTION), DoOnceScheduler());
    castSpellOnSelf(jaraxxus::SPELL_INFERNAL_ERUPTION);

    repeatFunctionFromScheduler(pThis, 2min);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Wilfred Frizzlebang
FrizzlebangAI::FrizzlebangAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    getCreature()->setAItoUse(false);
}

CreatureAIScript* FrizzlebangAI::Create(Creature* pCreature) { return new FrizzlebangAI(pCreature); }

void FrizzlebangAI::OnLoad()
{
    setReactState(REACT_PASSIVE);
    addAIFunction(&FrizzlebangAI::startMove, DoOnceScheduler(1s));
}

void FrizzlebangAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == SPLINE_CHAIN_MOTION_TYPE && id == POINT_SUMMON)
    {
        getInstanceScript()->DoAction(ACTION_CLOSE_GATE);
        addMessage(Message(WILFRED_SAY_INTRO), DoOnceScheduler());

        addEmote(Emote(EMOTE_STATE_TALK, true), DoOnceScheduler(2s));
        addEmote(Emote(EMOTE_STATE_TALK, true), DoOnceScheduler(9s));

        addAIFunction(&FrizzlebangAI::oblivion, DoOnceScheduler(11s));
    }
}

void FrizzlebangAI::OnHitBySpell(uint32_t spellId, Unit* caster)
{
    if (spellId == jaraxxus::SPELL_FEL_LIGHTNING_INTRO)
        getCreature()->die(caster, getCreature()->getHealth(), spellId);
}

void FrizzlebangAI::startMove(CreatureAIFunc pThis)
{
    moveAlongSplineChain(POINT_SUMMON, SPLINE_INITIAL_MOVEMENT, true);
}

void FrizzlebangAI::oblivion(CreatureAIFunc pThis)
{
    Creature* portal = summonCreature(NPC_WILFRED_PORTAL, PortalTargetSpawnPosition);
    Creature* ground = summonCreature(NPC_PURPLE_GROUND, PurpleGroundSpawnPosition, CreatureSummonDespawnType::TIMED_DESPAWN, 16 * TimeVarsMs::Second);
    ground->setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    addMessage(Message(WILFRED_SAY_OBLIVION), DoOnceScheduler());
    castSpell(portal, SPELL_OPEN_PORTAL);
    addAIFunction(&FrizzlebangAI::summonJaraxus, DoOnceScheduler(11s));
}

void FrizzlebangAI::summonJaraxus(CreatureAIFunc pThis)
{
    if (Creature* fordring = getInstanceScript()->getCreatureFromData(DATA_FORDRING))
        fordring->GetScript()->DoAction(ACTION_SUMMON_JARAXXUS);

    addMessage(Message(WILFRED_SAY_MASTER), DoOnceScheduler());

    addEmote(Emote(EMOTE_STATE_TALK, true), DoOnceScheduler(2s));
    addEmote(Emote(EMOTE_STATE_TALK, true), DoOnceScheduler(7s));

    addAIFunction(&FrizzlebangAI::setTarget, DoOnceScheduler(4s));
}

void FrizzlebangAI::setTarget(CreatureAIFunc pThis)
{
    if (Creature* jaraxxus = getInstanceScript()->getCreatureFromData(DATA_JARAXXUS))
        getCreature()->setFacingToObject(jaraxxus);

    addEmote(Emote(EMOTE_ONESHOT_TALK_NOSHEATHE), DoOnceScheduler(6s));
    addAIFunction(&FrizzlebangAI::lastTalk, DoOnceScheduler(9s));
}

void FrizzlebangAI::lastTalk(CreatureAIFunc pThis)
{
    addMessage(Message(WILFRED_SAY_DEAD), DoOnceScheduler());

    if (Creature* fordring = getInstanceScript()->getCreatureFromData(DATA_FORDRING))
        fordring->GetScript()->DoAction(ACTION_KILL_JARAXXUS);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Portal
PortalAI::PortalAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* PortalAI::Create(Creature* pCreature) { return new PortalAI(pCreature); }

void PortalAI::InitOrReset()
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    getCreature()->setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
    setReactState(REACT_PASSIVE);

    setDisableGravity(true);
    removeAllFunctionsFromScheduler();
}

void PortalAI::OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/)
{
    if (_spellId == SPELL_OPEN_PORTAL)
    {
        addAIFunction(&PortalAI::portalOpening, DoOnceScheduler(2s));
    }
}

void PortalAI::portalOpening(CreatureAIFunc pThis)
{
    castSpellOnSelf(SPELL_WILFRED_PORTAL);
    despawn(9000);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Legion Flame
LegionFlameAI::LegionFlameAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* LegionFlameAI::Create(Creature* pCreature) { return new LegionFlameAI(pCreature); }

void LegionFlameAI::OnLoad()
{
    setRooted(true);
    setReactState(REACT_PASSIVE);
    castSpellOnSelf(jaraxxus::SPELL_LEGION_FLAME_EFFECT, true);
}

void LegionFlameAI::OnCombatStop(Unit* /*_target*/)
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) != EncounterStates::InProgress)
        despawn();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Infernal Volcano
InfernalVolcanoAI::InfernalVolcanoAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* InfernalVolcanoAI::Create(Creature* pCreature) { return new InfernalVolcanoAI(pCreature); }

void InfernalVolcanoAI::OnLoad()
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    getCreature()->setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
    setRooted(true);
    setReactState(REACT_PASSIVE);
    castSpellOnSelf(jaraxxus::SPELL_INFERNAL_ERUPTION_EFFECT, true);

    if (isHeroic())
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
}

void InfernalVolcanoAI::OnCombatStop(Unit* /*_target*/)
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) != EncounterStates::InProgress)
        despawn();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Fel Infernal
FelInfernalAI::FelInfernalAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* FelInfernalAI::Create(Creature* pCreature) { return new FelInfernalAI(pCreature); }

void FelInfernalAI::OnLoad()
{
    addAIFunction(&FelInfernalAI::felStreak, DoOnceScheduler(2s));
    castSpellOnSelf(jaraxxus::SPELL_LORD_JARAXXUS_HITTIN_YA, true);
}

void FelInfernalAI::OnCombatStop(Unit* /*_target*/)
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) != EncounterStates::InProgress)
        despawn();
}

void FelInfernalAI::felStreak(CreatureAIFunc pThis)
{
    if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_Player)))
        castSpell(target, jaraxxus::SPELL_FEL_STREAK_VISUAL);

    repeatFunctionFromScheduler(pThis, 15s);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Nether Portal
NetherPortalAI::NetherPortalAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* NetherPortalAI::Create(Creature* pCreature) { return new NetherPortalAI(pCreature); }

void NetherPortalAI::OnLoad()
{   
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    getCreature()->setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);

    setRooted(true);
    setReactState(REACT_PASSIVE);

    addAISpell(SpellDesc(jaraxxus::SPELL_NETHER_PORTAL_EFFECT, FilterArgs(TargetFilter_Self), false), DoOnceScheduler(1s));

    if (isHeroic())
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
}

void NetherPortalAI::OnCombatStop(Unit*)
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) != EncounterStates::InProgress)
        despawn();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Mistress Of Pain
MistressOfPainAI::MistressOfPainAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    getCreature()->setAItoUse(true);
}

CreatureAIScript* MistressOfPainAI::Create(Creature* pCreature) { return new MistressOfPainAI(pCreature); }

void MistressOfPainAI::OnLoad()
{
    castSpellOnSelf(jaraxxus::SPELL_LORD_JARAXXUS_HITTIN_YA, true);
}

void MistressOfPainAI::OnCombatStart(Unit* /*_target*/)
{
    /////////////////////////////////////////
    // Shivan Slash
    DoLoopScheduler mShivanSlashArgs;
    mShivanSlashArgs.setInitialCooldown(4s);
    mShivanSlashArgs.setChance(33.0f);
    addAISpell(SpellDesc(jaraxxus::SPELL_SHIVAN_SLASH, FilterArgs(TargetFilter_Current), false), mShivanSlashArgs);

    /////////////////////////////////////////
    // Spinning Spike
    DoLoopScheduler mSpinningSpikeArgs;
    mSpinningSpikeArgs.setInitialCooldown(9s);
    mSpinningSpikeArgs.setChance(33.0f);
    addAISpell(SpellDesc(jaraxxus::SPELL_SPINNING_SPIKE, FilterArgs(TargetFilter_Player), false), mSpinningSpikeArgs);

    /////////////////////////////////////////
    // Mistress Kiss
    DoLoopScheduler mMistressKissArgs;
    mMistressKissArgs.setInitialCooldown(15s);
    mMistressKissArgs.setChance(33.0f);
    if (isHeroic())
        addAISpell(SpellDesc(jaraxxus::SPELL_MISTRESS_KISS, FilterArgs(TargetFilter_Self), false), mMistressKissArgs);
}

void MistressOfPainAI::OnCombatStop(Unit*)
{
    if (getInstanceScript()->getBossState(DATA_JARAXXUS) != EncounterStates::InProgress)
        despawn();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Felstreak
bool FelStreakEffect(uint8_t effectIndex, Spell* pSpell)
{
    if (effectIndex == EFF_INDEX_0)
    {
        uint32_t spellId = pSpell->damage;
        pSpell->getUnitCaster()->castSpell(pSpell->getUnitTarget(), spellId, true);
    }

    return true;
}
