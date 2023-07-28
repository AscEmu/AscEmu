/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <functional>
#include "AIUtils.hpp"

class SERVER_DECL CreatureAIFunction
{
public:
    CreatureAIFunction(CreatureAIScript* owner, Function pFunction, SchedulerArgs const& schedArgs, FunctionArgs const& funcArgs = {})
    {
        mOwner = owner;
        mFunction = pFunction;
        schedulerArgs = schedArgs;
        functionArgs = funcArgs;
        mCooldownTimer = schedArgs.getInitialCooldown();

        // Set CastTime and SpellDefault CD when its a Spell
        if (const auto spellInfo = sSpellMgr.getSpellInfo(functionArgs.getSpellId()))
        {
            uint32_t castTime = GetCastTime(sSpellCastTimesStore.LookupEntry(spellInfo->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.LookupEntry(spellInfo->getCastingTimeIndex())) : 500;
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

    ~CreatureAIFunction() = default;

    // Reset to make all Charges fill up
    void reset();

    // Flow
    void setEnabled(bool set) { schedulerArgs.setEnabled(set); }
    bool isEnabled() { return schedulerArgs.isEnabled(); }

    // Combat
    bool getCombatUsage() { return schedulerArgs.getCombatUsage(); }

    // Timers
    Milliseconds getInitialCooldown() { return schedulerArgs.getInitialCooldown(); }
    Milliseconds getSpellCooldown() { return schedulerArgs.getInitialCooldown(); }
    Milliseconds getCooldownTimer() { return mCooldownTimer; }
    void setCooldownTimer(Milliseconds time) { mCooldownTimer = time; }
    void decreaseCooldownTimer(Milliseconds time) { mCooldownTimer = mCooldownTimer - time; }

    void setCastTimer(Milliseconds time) { functionArgs.setCastTimer(time); }
    Milliseconds getCastTimer() { return functionArgs.getCastTimer(); }

    // Usages
    void setCount(uint32_t count) { mCount = count; }
    const uint32_t getCount() { return mCount; }
    void increaseCount() { ++mCount; }
    bool usesLeft() { return schedulerArgs.getMaxCount() ? getCount() < schedulerArgs.getMaxCount() : true; }

    // Health
    const bool isHpInPercentRange(float targetHp);

    // Chances
    bool isChanceMeet(float rolled) { return schedulerArgs.getChance() ? (rolled < schedulerArgs.getChance()) : true; }

    // Phases
    bool isAvailableForScriptPhase(uint32_t scriptPhase);

    // Messages
    uint32_t getRandomMessage();

    // Main Execution
    Function getFunction() const { return mFunction; }

    // Arguments
    void setFunctionArgs(FunctionArgs& arguments) { functionArgs = arguments; }
    FunctionArgs getFunctionArgs() { return functionArgs; }
    void setSchedulerArgs(SchedulerArgs& arguments) { schedulerArgs = arguments; }
    SchedulerArgs getSchedulerArgs() { return schedulerArgs; }

    bool mGarbage = false;      // when this dont gets Reset the Function Deletes itself upon completion

private:
    Function        mFunction           = nullptr;          // Function to be executed
    int32_t         mCount              = 0;                // Usages Tracker
    Milliseconds    mCooldownTimer      = Milliseconds();   // Function cooldown Timer

    // Arguments for Scheduer
    SchedulerArgs schedulerArgs;

    // Arguments for Function
    FunctionArgs functionArgs;

    // Function Owner
    CreatureAIScript* mOwner        = nullptr;
};
