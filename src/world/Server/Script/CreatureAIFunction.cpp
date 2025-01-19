/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureAIFunction.hpp"
#include "CreatureAIScript.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Utilities/Random.hpp"

CreatureAIFunction::CreatureAIFunction(CreatureAIScript* owner, Function pFunction, SchedulerArgs const& schedArgs, FunctionArgs const& funcArgs)
{
    mOwner = owner;
    mFunction = pFunction;
    schedulerArgs = schedArgs;
    functionArgs = funcArgs;
    mCooldownTimer = schedArgs.getInitialCooldown();

    // Set CastTime and SpellDefault CD when its a Spell
    if (const auto spellInfo = sSpellMgr.getSpellInfo(functionArgs.getSpellId()))
    {
        uint32_t castTime = GetCastTime(sSpellCastTimesStore.lookupEntry(spellInfo->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.lookupEntry(spellInfo->getCastingTimeIndex())) : 500;
        setCastTimer(Milliseconds(castTime));

        // Set RecoveryTimer
        // When not Available use Default Time
        Milliseconds cooldown = Milliseconds(spellInfo->getRecoveryTime()) + getCastTimer();
        if (!cooldown.count())
            schedulerArgs.setSpellCooldown(mCooldownTimer);
        else
            schedulerArgs.setSpellCooldown(Milliseconds(cooldown));
    }
}

void CreatureAIFunction::reset()
{
    // Reset Timers and Charges
    setCooldownTimer(getInitialCooldown());
    setCount(0);
}

// Flow
void CreatureAIFunction::setEnabled(bool set) { schedulerArgs.setEnabled(set); }
bool CreatureAIFunction::isEnabled() { return schedulerArgs.isEnabled(); }

// Combat
bool CreatureAIFunction::getCombatUsage() { return schedulerArgs.getCombatUsage(); }

// Timers
Milliseconds CreatureAIFunction::getInitialCooldown() { return schedulerArgs.getInitialCooldown(); }
Milliseconds CreatureAIFunction::getSpellCooldown() { return schedulerArgs.getInitialCooldown(); }
Milliseconds CreatureAIFunction::getCooldownTimer() { return mCooldownTimer; }
void CreatureAIFunction::setCooldownTimer(Milliseconds time) { mCooldownTimer = time; }
void CreatureAIFunction::decreaseCooldownTimer(Milliseconds time) { mCooldownTimer = mCooldownTimer - time; }

void CreatureAIFunction::setCastTimer(Milliseconds time) { functionArgs.setCastTimer(time); }
Milliseconds CreatureAIFunction::getCastTimer() { return functionArgs.getCastTimer(); }

// Usages
void CreatureAIFunction::setCount(uint32_t count) { mCount = count; }
const uint32_t CreatureAIFunction::getCount() { return mCount; }
void CreatureAIFunction::increaseCount() { ++mCount; }
bool CreatureAIFunction::usesLeft() { return schedulerArgs.getMaxCount() ? getCount() < schedulerArgs.getMaxCount() : true; }

const bool CreatureAIFunction::isHpInPercentRange(float targetHp)
{
    if (targetHp >= schedulerArgs.getMinHPRange() && targetHp <= schedulerArgs.getMaxHPRange())
        return true;

    return false;
}

bool CreatureAIFunction::isChanceMeet(float rolled) { return schedulerArgs.getChance() ? (rolled < schedulerArgs.getChance()) : true; }

bool CreatureAIFunction::isAvailableForScriptPhase(uint32_t scriptPhase)
{
    if (schedulerArgs.mPhaseList.empty())
        return true;

    for (const auto& availablePhase : schedulerArgs.mPhaseList)
    {
        if (availablePhase == scriptPhase)
            return true;
    }

    return false;
}

uint32_t CreatureAIFunction::getRandomMessage()
{
    AIMessagesArray mEmotes = functionArgs.getAIMessages();
    if (!mEmotes.empty())
    {
        // Send Random Stores Message
        uint32_t randomUInt = (mEmotes.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mEmotes.size() - 1)) : 0;

        return mEmotes[randomUInt].messageId;
    }

    return 0;
}

Function CreatureAIFunction::getFunction() const { return mFunction; }

void CreatureAIFunction::setFunctionArgs(FunctionArgs& arguments) { functionArgs = arguments; }
FunctionArgs CreatureAIFunction::getFunctionArgs() { return functionArgs; }
void CreatureAIFunction::setSchedulerArgs(SchedulerArgs& arguments) { schedulerArgs = arguments; }
SchedulerArgs CreatureAIFunction::getSchedulerArgs() { return schedulerArgs; }
