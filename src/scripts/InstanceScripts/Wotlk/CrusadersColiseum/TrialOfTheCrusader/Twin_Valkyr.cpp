/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_TrialOfTheCrusader.hpp"
#include "Twin_Valkyr.hpp"

#include "Management/Gossip/GossipMenu.hpp"
#include "Map/AreaBoundary.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Twins Base AI
TwinsAI::TwinsAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Add Boundary
    pCreature->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(LocationVector(563.26f, 139.6f), 75.0));

    AuraState = 0;
    Weapon = 0;
    MyEmphatySpellId = 0;
    OtherEssenceSpellId = 0;
    SurgeSpellId = 0;
    VortexSpellId = 0;
    ShieldSpellId = 0;
    TwinPactSpellId = 0;
    SpikeSpellId = 0;
    TouchSpellId = 0;

    // Events
    addEmoteForEventByIndex(Event_OnCombatStart, twins::SAY_AGGRO);
    addEmoteForEventByIndex(Event_OnTargetDied, twins::SAY_KILL_PLAYER);
    addEmoteForEventByIndex(Event_OnDied, twins::SAY_DEATH);
}

CreatureAIScript* TwinsAI::Create(Creature* pCreature) { return new TwinsAI(pCreature); }

void TwinsAI::InitOrReset()
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    setDisableGravity(true);
    getCreature()->setHoverHeight(6);
    setReactState(REACT_PASSIVE);
    getCreature()->addAuraState(AuraState);
    summons.despawnAll();
}

void TwinsAI::OnLoad()
{
    setScriptPhase(twins::PHASE_EVENT);
    addAIFunction(&TwinsAI::intro, DoOnceScheduler(4s));
}

void TwinsAI::OnCombatStart(Unit*)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
    setZoneWideCombat();

    if (CreatureAIScript* pSister = getLinkedCreatureAIScript())
    {
        SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(MyEmphatySpellId);
        auto pAura = sSpellMgr.newAura(spellInfo, (int32_t)GetDuration(sSpellDurationStore.lookupEntry(spellInfo->getDurationIndex())), getCreature(), pSister->getCreature());
        getCreature()->addAura(std::move(pAura));
        setZoneWideCombat(pSister->getCreature());
    }

    getInstanceScript()->setBossState(DATA_TWIN_VALKIRIES, EncounterStates::InProgress);

    castSpellOnSelf(SurgeSpellId);

    SpellDesc mBerserkInfo(twins::SPELL_BERSERK, FilterArgs(TargetFilter_Self), false);
    mBerserkInfo.setUseSpellCD(true);
    mBerserkInfo.addDBMessage(twins::SAY_BERSERK);
    addAISpell(mBerserkInfo, DoLoopScheduler(isHeroic() ? 6min : 8min, 100.0f));

    SpellDesc mTwinSpikeInfo(SpikeSpellId, FilterArgs(TargetFilter_Current), false);
    addAISpell(mTwinSpikeInfo, DoLoopScheduler(20s, 100.0f));

    if (isHeroic())
    {
        SpellDesc mTouchInfo(TouchSpellId, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 200.0f, OtherEssenceSpellId), false);
        addAISpell(mTouchInfo, DoLoopScheduler(10s, 20s, 33.0f));
    }
}

void TwinsAI::justReachedSpawn()
{
    getInstanceScript()->setBossState(DATA_TWIN_VALKIRIES, EncounterStates::Failed);
    handleRemoveAuras();
    summons.despawnAll();
    despawn();
}

void TwinsAI::OnDied(Unit* /*_killer*/)
{
    getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

    if (CreatureAIScript* pSister = getLinkedCreatureAIScript())
    {
        if (!pSister->isAlive())
        {
            getCreature()->setDynamicFlags(U_DYN_FLAG_LOOTABLE);
            pSister->getCreature()->setDynamicFlags(U_DYN_FLAG_LOOTABLE);

            summons.despawnAll();
            handleRemoveAuras();
            getInstanceScript()->setBossState(DATA_TWIN_VALKIRIES, EncounterStates::Performed);
        }
        else
        {
            getCreature()->removeDynamicFlags(U_DYN_FLAG_LOOTABLE);
            getInstanceScript()->setBossState(DATA_TWIN_VALKIRIES, EncounterStates::PreProgress);
        }
    }
    summons.despawnAll();
}


void TwinsAI::DoAction(int32_t action)
{
    switch (action)
    {
        case twins::ACTION_VORTEX:
        {
            sendDBChatMessageByIndex(twins::EMOTE_VORTEX);
            castSpellAOE(VortexSpellId);
        } break;
        case twins::ACTION_PACT:
        {
            sendDBChatMessageByIndex(twins::EMOTE_TWIN_PACT);
            sendDBChatMessageByIndex(twins::SAY_TWIN_PACT);

            if (CreatureAIScript* sister = getLinkedCreatureAIScript())
                sister->castSpellOnSelf(twins::SPELL_POWER_TWINS, false);

            castSpellOnSelf(ShieldSpellId);
            castSpellOnSelf(TwinPactSpellId);
        } break;
        default:
            break;
    }
}

void TwinsAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == SPLINE_CHAIN_MOTION_TYPE && id == twins::POINT_INITIAL_MOVEMENT)
    {
        setImmuneToPC(false);
        setReactState(REACT_AGGRESSIVE);

        if (getCreature()->getEntry() == NPC_FJOLA_LIGHTBANE)
            getInstanceScript()->DoAction(ACTION_CLOSE_GATE);
    }
}

void TwinsAI::handleRemoveAuras()
{
    for (const auto& itr : getInstanceScript()->getInstance()->getPlayers())
    {
        if (Player* pPlayer = itr.second)
        {
            pPlayer->removeAllAurasById(getRaidModeValue(65686, 67222, 67223, 67224));
            pPlayer->removeAllAurasById(getRaidModeValue(67590, 67602, 67603, 67604));
            pPlayer->removeAllAurasById(getRaidModeValue(65684, 67176, 67177, 67178));
            pPlayer->removeAllAurasById(getRaidModeValue(67590, 67602, 67603, 67604));
        }
    }
}

void TwinsAI::intro(CreatureAIFunc pThis)
{
    setScriptPhase(twins::PHASE_COMBAT);
    moveAlongSplineChain(twins::POINT_INITIAL_MOVEMENT, SPLINE_INITIAL_MOVEMENT, false);
}

void TwinsAI::enableDualWield(bool mode)
{
    _setDisplayWeaponIds(Weapon, mode ? Weapon : 0);
    getCreature()->setDualWield(mode);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Fjola Lightbane
FjolaAI::FjolaAI(Creature* pCreature) : TwinsAI(pCreature)
{
    generateStageSequence();
}

CreatureAIScript* FjolaAI::Create(Creature* pCreature) { return new FjolaAI(pCreature); }

void FjolaAI::InitOrReset()
{
    _setDisplayWeaponIds(twins::EQUIP_MAIN_1, 0);

    Weapon                  = twins::EQUIP_MAIN_1;
    AuraState               = 22;
    MyEmphatySpellId        = twins::SPELL_TWIN_EMPATHY_DARK;
    OtherEssenceSpellId     = getRaidModeValue(65684, 67176, 67177, 67178);
    SurgeSpellId            = twins::SPELL_LIGHT_SURGE;
    VortexSpellId           = twins::SPELL_LIGHT_VORTEX;
    ShieldSpellId           = twins::SPELL_LIGHT_SHIELD;
    TwinPactSpellId         = twins::SPELL_LIGHT_TWIN_PACT;
    TouchSpellId            = twins::SPELL_LIGHT_TOUCH;
    SpikeSpellId            = twins::SPELL_LIGHT_TWIN_SPIKE;

    TwinsAI::InitOrReset();
}

void FjolaAI::OnCombatStart(Unit* _target)
{
    scriptEvents.addEvent(twins::EVENT_SPECIAL_ABILITY, 45000);

    summonCreature(twins::NPC_BULLET_CONTROLLER, ToCCommonLoc[1].getPositionX(), ToCCommonLoc[1].getPositionY(), ToCCommonLoc[1].getPositionZ(), 0.0f);
    TwinsAI::OnCombatStart(_target);
}

void FjolaAI::OnCombatStop(Unit* _target)
{
    getInstanceScript()->DoAction(ACTION_OPEN_GATE);
    TwinsAI::OnCombatStop(_target);
}

void FjolaAI::justReachedSpawn()
{
    getInstanceScript()->DoAction(ACTION_CLOSE_GATE);
    TwinsAI::justReachedSpawn();
}

void FjolaAI::AIUpdate(unsigned long time_passed)
{
    if (!getCreature()->getAIInterface()->getCurrentTarget() && !getScriptPhase() != twins::PHASE_EVENT)
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());
    
    if (_isCasting())
        return;

    if (scriptEvents.getFinishedEvent() == twins::EVENT_SPECIAL_ABILITY)
    {
        if (CurrentStage == twins::MAX_STAGES)
            generateStageSequence();

        switch (Stage[CurrentStage])
        {
            case twins::STAGE_DARK_VORTEX:
                if (CreatureAIScript* sister = getLinkedCreatureAIScript())
                    sister->DoAction(twins::ACTION_VORTEX);
                break;
            case twins::STAGE_DARK_PACT:
                if (CreatureAIScript* sister = getLinkedCreatureAIScript())
                    sister->DoAction(twins::ACTION_PACT);
                break;
            case twins::STAGE_LIGHT_VORTEX:
                DoAction(twins::ACTION_VORTEX);
                break;
            case twins::STAGE_LIGHT_PACT:
                DoAction(twins::ACTION_PACT);
                break;
            default:
                break;
        }
        ++CurrentStage;
        scriptEvents.addEvent(twins::EVENT_SPECIAL_ABILITY, 45000);
    }
}

void FjolaAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
}

void FjolaAI::generateStageSequence()
{
    CurrentStage = 0;

    // Initialize and clean up.
    for (int i = 0; i < twins::MAX_STAGES; ++i)
        Stage[i] = i;

    // Allocate an unique random stage to each position in the array.
    for (int i = 0; i < twins::MAX_STAGES - 1; ++i)
    {
        int random = i + Util::getRandomUInt(0, twins::MAX_STAGES - i);
        std::swap(Stage[i], Stage[random]);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Eydis Darkbane
EydisAI::EydisAI(Creature* pCreature) : TwinsAI(pCreature) { }

CreatureAIScript* EydisAI::Create(Creature* pCreature) { return new EydisAI(pCreature); }

void EydisAI::InitOrReset()
{
    _setDisplayWeaponIds(twins::EQUIP_MAIN_2, 0);

    Weapon                  = twins::EQUIP_MAIN_2;
    AuraState               = 19;
    MyEmphatySpellId        = twins::SPELL_TWIN_EMPATHY_LIGHT;
    OtherEssenceSpellId     = getRaidModeValue(65686, 67222, 67223, 67224);
    SurgeSpellId            = twins::SPELL_DARK_SURGE;
    VortexSpellId           = twins::SPELL_DARK_VORTEX;
    ShieldSpellId           = twins::SPELL_DARK_SHIELD;
    TwinPactSpellId         = twins::SPELL_DARK_TWIN_PACT;
    TouchSpellId            = twins::SPELL_DARK_TOUCH;
    SpikeSpellId            = twins::SPELL_DARK_TWIN_SPIKE;

    TwinsAI::InitOrReset();
}

void EydisAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Essence
void EssenceGossip::onHello(Object* pObject, Player* plr)
{
    GossipMenu menu(pObject->getGuid(), 0);

    plr->removeAllAurasById(getData(pObject->ToCreature(), false));
    plr->castSpell(plr, getData(pObject->ToCreature(), true), true);

    menu.sendGossipPacket(plr);
}

uint32_t EssenceGossip::getData(Creature* pCreature, bool data) const
{
    uint32_t spellReturned = 0;
    switch (pCreature->getEntry())
    {
        case NPC_LIGHT_ESSENCE:
            spellReturned = data ? RAID_MODE<uint32_t>(pCreature, 65684, 67176, 67177, 67178) : RAID_MODE<uint32_t>(pCreature, 65686, 67222, 67223, 67224);
            break;
        case NPC_DARK_ESSENCE:
            spellReturned = data ? RAID_MODE<uint32_t>(pCreature, 65686, 67222, 67223, 67224) : RAID_MODE<uint32_t>(pCreature, 65684, 67176, 67177, 67178);
            break;
        default:
            break;
    }

    return spellReturned;
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Unleashed Ball
UnleashedBallAI::UnleashedBallAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mInstance = static_cast<TrialOfTheCrusaderInstanceScript*>(getInstanceScript());
    RangeCheckTimer = 500;
    stalkerGUIDS.clear();
}

CreatureAIScript* UnleashedBallAI::Create(Creature* pCreature) { return new UnleashedBallAI(pCreature); }

void UnleashedBallAI::OnLoad()
{
    setUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    setReactState(REACT_PASSIVE);
    setDisableGravity(false);
    setFlyMode(true);
    setRooted(false);

    // get the list of summoned stalkers and move to a randome one
    if (mInstance)
        mInstance->getStalkersGuidVector(stalkerGUIDS);

    if (stalkerGUIDS.empty())
        return;

    const auto maxStalkers = static_cast<uint32_t>(stalkerGUIDS.size() - 1);
    if (Creature* pStalker = getInstanceScript()->GetCreatureByGuid(stalkerGUIDS[Util::getRandomUInt(0, maxStalkers)]))
        getMovementManager()->movePoint(1, pStalker->GetPositionX(), pStalker->GetPositionY(), pStalker->GetPositionZ());

    RangeCheckTimer = 500;
}

void UnleashedBallAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type != POINT_MOTION_TYPE)
        return;

    // move to another random stalker
    const auto maxStalkers = static_cast<uint32_t>(stalkerGUIDS.size() - 1);
    if (Creature* pStalker = getInstanceScript()->GetCreatureByGuid(stalkerGUIDS[Util::getRandomUInt(0, maxStalkers)]))
        getMovementManager()->movePoint(1, pStalker->GetPositionX(), pStalker->GetPositionY(), pStalker->GetPositionZ());
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Unleashed Dark
UnleashedDarkAI::UnleashedDarkAI(Creature* pCreature) : UnleashedBallAI(pCreature) { }

CreatureAIScript* UnleashedDarkAI::Create(Creature* pCreature) { return new UnleashedDarkAI(pCreature); }

void UnleashedDarkAI::AIUpdate(unsigned long time_passed)
{
    if (RangeCheckTimer < time_passed)
    {
        if (selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_InRangeOnly | TargetFilter_Player), 0.0f, 3.0f)))
        {
            castSpellAOE(getRaidModeValue(65808, 67172, 67173, 67174));
            getMovementManager()->moveIdle();
            despawn(1000, 0);
        }
        RangeCheckTimer = 500;
    }
    else
    {
        RangeCheckTimer -= time_passed;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Unleashed Light
UnleashedLightAI::UnleashedLightAI(Creature* pCreature) : UnleashedBallAI(pCreature) { }

CreatureAIScript* UnleashedLightAI::Create(Creature* pCreature) { return new UnleashedLightAI(pCreature); }

void UnleashedLightAI::AIUpdate(unsigned long time_passed)
{
    if (RangeCheckTimer < time_passed)
    {
        if (selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_InRangeOnly | TargetFilter_Player), 0.0f, 3.0f)))
        {
            castSpellAOE(getRaidModeValue(65795, 67238, 67239, 67240));
            getMovementManager()->moveIdle();
            despawn(1000, 0);
        }
        RangeCheckTimer = 500;
    }
    else
    {
        RangeCheckTimer -= time_passed;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Bullet Controller
BulletCotrollerAI::BulletCotrollerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mInstance = static_cast<TrialOfTheCrusaderInstanceScript*>(getInstanceScript());
    stalkerGUIDS.clear();
}

CreatureAIScript* BulletCotrollerAI::Create(Creature* pCreature) { return new BulletCotrollerAI(pCreature); }

void BulletCotrollerAI::OnLoad()
{
    setRooted(true);

    // get the list of summoned stalkers and spawn the balls on a random one
    if (mInstance)
        mInstance->getStalkersGuidVector(stalkerGUIDS);

    if (stalkerGUIDS.empty())
        return;

    addAIFunction([this](CreatureAIFunc pThis)
        {
            const auto maxStalkers = static_cast<uint32_t>(stalkerGUIDS.size() - 1);
            if (darkCounter < 36)
                if (Creature* pStalker = getInstanceScript()->GetCreatureByGuid(stalkerGUIDS[Util::getRandomUInt(0, maxStalkers)]))
                    summonCreature(NPC_UNLEASHED_DARK, pStalker->GetPosition());

            if (lightCounter < 36)
                if (Creature* pStalker = getInstanceScript()->GetCreatureByGuid(stalkerGUIDS[Util::getRandomUInt(0, maxStalkers)]))
                    summonCreature(NPC_UNLEASHED_LIGHT, pStalker->GetPosition());

            repeatFunctionFromScheduler(pThis, 1s);
        }, DoOnceScheduler(1s, 100.0f));
}

void BulletCotrollerAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);

    switch (summon->getEntry())
    {
        case NPC_UNLEASHED_DARK:
            darkCounter++;
            break;
        case NPC_UNLEASHED_LIGHT:
            lightCounter++;
            break;
    }
}

void BulletCotrollerAI::OnSummonDespawn(Creature* summon)
{
    summons.despawn(summon);

    switch (summon->getEntry())
    {
        case NPC_UNLEASHED_DARK:
            darkCounter--;
            break;
        case NPC_UNLEASHED_LIGHT:
            lightCounter--;
            break;
    }
}

void BulletCotrollerAI::OnDespawn()
{
    summons.despawnAll();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// PoweringUp
bool PoweringUpEffect(uint8_t effectIndex, Spell* pSpell)
{
    if (effectIndex == EFF_INDEX_1)
    {
        if (Unit* target = pSpell->getUnitTarget())
        {
            if (Aura* pAura = target->getAuraWithId(twins::SPELL_POWERING_UP))
            {
                if (pAura->getStackCount() >= 100)
                {
                    if (target->hasAurasWithId(twins::SPELL_DARK_ESSENCE))
                        target->castSpell(target, twins::SPELL_EMPOWERED_DARK, true);

                    if (target->hasAurasWithId(twins::SPELL_LIGHT_ESSENCE))
                        target->castSpell(target, twins::SPELL_EMPOWERED_LIGHT, true);

                    target->removeAllAurasById(twins::SPELL_POWERING_UP);
                }
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Dark Essence and Light Essence
SpellScriptExecuteState EssenceScript::beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectIndex() == EFF_INDEX_0)
    {
        if (Unit* owner = aur->getOwner())
        {
            if (aur->getSpellInfo())
            {
                if (uint32_t poweringUp = twins::SPELL_POWERING_UP)
                {
                    if (Util::getRandomUInt(0, 99) < 5)
                        aur->GetUnitTarget()->castSpell(aur->GetUnitTarget(), twins::SPELL_SURGE_OF_SPEED, true);

                    // Twin Vortex part
                    int32_t stacksCount = aur->getSpellInfo()->calculateEffectValue(EFF_INDEX_0) / 1000 - 1;

                    if (stacksCount)
                    {
                        if (aur->getSpellInfo()->getId() == twins::SPELL_DARK_VORTEX_DAMAGE || aur->getSpellInfo()->getId() == twins::SPELL_LIGHT_VORTEX_DAMAGE)
                        {
                            if (Aura* aura = owner->getAuraWithId(poweringUp))
                            {
                                aura->refreshOrModifyStack(false, stacksCount);
                                owner->castSpell(owner, poweringUp, true);
                            }
                            else
                            {
                                owner->castSpell(owner, poweringUp, true);
                                if (Aura* newAura = owner->getAuraWithId(poweringUp))
                                    newAura->refreshOrModifyStack(false, stacksCount);
                            }
                        }
                    }

                    // Picking floating balls
                    if (aur->getSpellInfo()->getId() == twins::SPELL_UNLEASHED_DARK || aur->getSpellInfo()->getId() == twins::SPELL_UNLEASHED_LIGHT)
                    {
                        // need to do the things in this order, else players might have 100 charges of Powering Up without anything happening
                        if (Aura* aura = owner->getAuraWithId(poweringUp))
                        {
                            // 2 lines together add the correct amount of buff stacks
                            aura->refreshOrModifyStack(false, stacksCount);
                            owner->castSpell(owner, poweringUp, true);
                        }
                        else
                        {
                            owner->castSpell(owner, poweringUp, true);
                            if (Aura* newAura = owner->getAuraWithId(poweringUp))
                                newAura->refreshOrModifyStack(false, stacksCount);
                        }
                    }
                }
            }
        }
    }

    return SpellScriptExecuteState();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Power Of the Twins
void PowerOfTwinsScript::onAuraApply(Aura* aur)
{
    if (!aur->getCaster()->isCreature())
        return;

    if (CreatureAIScript* sister = aur->getCaster()->ToCreature()->GetScript())
    {
        if (TwinsAI* valk = static_cast<TwinsAI*>(sister->getLinkedCreatureAIScript()))
            valk->enableDualWield(true);
    }
}

void PowerOfTwinsScript::onAuraRemove(Aura* aur, AuraRemoveMode mode)
{
    if (!aur->getCaster()->isCreature())
        return;

    if (CreatureAIScript* sister = aur->getCaster()->ToCreature()->GetScript())
    {
        if (TwinsAI* valk = static_cast<TwinsAI*>(sister->getLinkedCreatureAIScript()))
            valk->enableDualWield(false);
    }
}
