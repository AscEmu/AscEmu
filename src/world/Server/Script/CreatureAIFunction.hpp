/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <cstdint>

#include <functional>

#include "AIUtils.hpp"

class CreatureAIScript;

class SERVER_DECL CreatureAIFunction
{
public:
    CreatureAIFunction(CreatureAIScript* owner, Function pFunction, SchedulerArgs const& schedArgs, FunctionArgs const& funcArgs = {});

    ~CreatureAIFunction() = default;

    // Reset to make all Charges fill up
    void reset();

    // Flow
    void setEnabled(bool set);
    bool isEnabled();

    // Combat
    bool getCombatUsage();

    // Timers
    Milliseconds getInitialCooldown();
    Milliseconds getSpellCooldown();
    Milliseconds getCooldownTimer();
    void setCooldownTimer(Milliseconds time);
    void decreaseCooldownTimer(Milliseconds time);

    void setCastTimer(Milliseconds time);
    Milliseconds getCastTimer();

    // Usages
    void setCount(uint32_t count);
    const uint32_t getCount();
    void increaseCount();
    bool usesLeft();

    // Health
    const bool isHpInPercentRange(float targetHp);

    // Chances
    bool isChanceMeet(float rolled);

    // Phases
    bool isAvailableForScriptPhase(uint32_t scriptPhase);

    // Messages
    uint32_t getRandomMessage();

    // Main Execution
    Function getFunction() const;

    // Arguments
    void setFunctionArgs(FunctionArgs& arguments);
    FunctionArgs getFunctionArgs();
    void setSchedulerArgs(SchedulerArgs& arguments);
    SchedulerArgs getSchedulerArgs();

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
