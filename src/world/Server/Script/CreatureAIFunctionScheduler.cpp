/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureAIFunctionScheduler.hpp"
#include "CreatureAIFunction.hpp"
#include "CreatureAIScript.h"

CreatureAIFunctionScheduler::CreatureAIFunctionScheduler(CreatureAIScript* script) : mOwner(script)
{
    functions_iterator = mCreatureAIFunctions.begin();
}

CreatureAIFunctionScheduler::~CreatureAIFunctionScheduler()
{
    mCreatureAIFunctions.clear();
}

template<typename T>
CreatureAIFunc CreatureAIFunctionScheduler::addAIFunction(void (T::* memberFunction)(CreatureAIFunc), SchedulerArgs const& schedulerArgs, FunctionArgs const& funcArgs)
{
    return addAIFunction([mThis = static_cast<T*>(mOwner), memberFunction](CreatureAIFunc pThis) { (mThis->*memberFunction)(pThis); }, schedulerArgs, funcArgs);
}

CreatureAIFunc CreatureAIFunctionScheduler::addAIFunction(Function pFunction, SchedulerArgs const& pScheduler, FunctionArgs const& funcArgs)
{
    // Create a new shared_ptr<CreatureAIFunction> object and initialize it
    std::shared_ptr<CreatureAIFunction> newAIFunction = std::make_shared<CreatureAIFunction>(mOwner, pFunction, pScheduler, funcArgs);

    // When there is no Timer set dont add it to the Scheduler Directly execute it.
    // This Skips all checks
    if (!newAIFunction->getInitialCooldown().count())
    {
        // Add One usage
        newAIFunction->increaseCount();

        // Execute our Function
        newAIFunction->getFunction()(newAIFunction);

        return newAIFunction;
    }

    // Add the new object to the mCreatureAIFunctions list
    mCreatureAIFunctionsTemporary.push_back(newAIFunction);

    // Return the pointer to the underlying CreatureAIFunction object
    return newAIFunction;
}

CreatureAIFunc CreatureAIFunctionScheduler::addAISpell(FunctionArgs funcArgs, SchedulerArgs const& pScheduler)
{
    // first lets check if this function already exists so we cannot add them multiple times by accident
    for (const auto& existingFunction : mCreatureAIFunctions)
    {
        if (existingFunction->getFunctionArgs().getSpellId() == funcArgs.getSpellId())
        {
            // Return the pointer to the underlying CreatureAIFunction object
            return existingFunction;
        }
    }

    const auto spellInfo = sSpellMgr.getSpellInfo(funcArgs.getSpellId());
    if (spellInfo != nullptr)
    {
        // Add a new Function
        return addAIFunction(&CreatureAIScript::CreatureAIFunc_CastSpell, pScheduler, funcArgs);
    }

    sLogger.failure("tried to add invalid spell with id %u", funcArgs.getSpellId());

    // Return an empty weak_ptr
    return CreatureAIFunc();
}

CreatureAIFunc CreatureAIFunctionScheduler::addMessage(FunctionArgs funcArgs, SchedulerArgs const& pScheduler)
{
    // first lets check if this function already exists so we cannot add them multiple times by accident
    for (const auto& existingFunction : mCreatureAIFunctions)
    {
        if (existingFunction->getInitialCooldown() == pScheduler.getInitialCooldown())
        {
            // Return the pointer to the underlying CreatureAIFunction object
            return existingFunction;
        }
    }

    // Add a new Function
    return addAIFunction(&CreatureAIScript::CreatureAIFunc_SendMessage, pScheduler, funcArgs);
}

CreatureAIFunc CreatureAIFunctionScheduler::addEmote(FunctionArgs funcArgs, SchedulerArgs const& pScheduler)
{
    // first lets check if this function already exists so we cannot add them multiple times by accident
    for (const auto& existingFunction : mCreatureAIFunctions)
    {
        if (existingFunction->getInitialCooldown() == pScheduler.getInitialCooldown() && existingFunction->getFunctionArgs().getEmote() == funcArgs.getEmote())
        {
            // Return the pointer to the underlying CreatureAIFunction object
            return existingFunction;
        }
    }

    // Add a new Function
    return addAIFunction(&CreatureAIScript::CreatureAIFunc_Emote, pScheduler, funcArgs);
}

void CreatureAIFunctionScheduler::executeFunction(CreatureAIFunc functionToExec)
{
    auto sharedFunction = functionToExec.lock();
    if (sharedFunction)
    {
        for (const auto& function : mCreatureAIFunctions)
        {
            if (function == sharedFunction)
            {
                function->getFunction()(function);
                break;
            }
        }
    }
}

void CreatureAIFunctionScheduler::cancelFunction(CreatureAIFunc functionToExec)
{
    auto sharedFunction = functionToExec.lock();
    if (sharedFunction)
    {
        sharedFunction->setEnabled(false);
        sharedFunction->mGarbage = true;
    }
}

void CreatureAIFunctionScheduler::repeatFunctionFromScheduler(CreatureAIFunc& functionToExec, Milliseconds newTimer)
{
    if (auto sharedFunction = functionToExec.lock())
    {
        auto schedulerArgs = sharedFunction->getSchedulerArgs();

        if (newTimer.count())
            schedulerArgs.setInitialCooldown(newTimer);

        sharedFunction->setSchedulerArgs(schedulerArgs);
        sharedFunction->mGarbage = false;
        sharedFunction->reset();
    }
}

void CreatureAIFunctionScheduler::enableFunction(CreatureAIFunc functionToEnable)
{
    auto sharedFunction = functionToEnable.lock();
    if (sharedFunction)
    {
        sharedFunction->reset();
        sharedFunction->setEnabled(true);
    }
}

void CreatureAIFunctionScheduler::disableFunction(CreatureAIFunc functionToDisable)
{
    auto sharedFunction = functionToDisable.lock();
    if (sharedFunction)
    {
        sharedFunction->setEnabled(false);
    }
}

void CreatureAIFunctionScheduler::update(unsigned long time_passed)
{
    // Copy All Functions in our Scheduler
    if (mCreatureAIFunctionsTemporary.size())
        mCreatureAIFunctions.insert(mCreatureAIFunctions.end(), mCreatureAIFunctionsTemporary.begin(), mCreatureAIFunctionsTemporary.end());

    // Empty Temp Function Vector
    mCreatureAIFunctionsTemporary.clear();

    // Nothing here so skip
    if (!mCreatureAIFunctions.size())
        return;

    // Shuffle Around our Functions to randomize them
    // example when 2 functions have 2 seconds timer the first one always executes before the second one...
    // could lead to unexpected behaviour when u rely on the execution order
    if (mCreatureAIFunctions.size())
    {
        for (uint16_t i = 0; i < mCreatureAIFunctions.size() - 1; ++i)
        {
            const auto j = i + rand() % (mCreatureAIFunctions.size() - i);
            std::swap(mCreatureAIFunctions[i], mCreatureAIFunctions[j]);
        }
    }

    // Delete Exceeded Functions
    for (auto it = mCreatureAIFunctions.begin(); it != mCreatureAIFunctions.end();)
    {
        auto function = *it;
        
        if (function->mGarbage)
        {
            it = mCreatureAIFunctions.erase(it);
            continue;
        }
        ++it;
    }

    // Lets find us a Function to Execute
    functions_iterator = mCreatureAIFunctions.begin();
    for (;functions_iterator != mCreatureAIFunctions.end();)
    {
        auto function = *functions_iterator;

        // Roll the Dice :)
        float rolled = Util::getRandomFloat(100.0f);

        // When the Function is Currently Disabled skip.
        if (!function->isEnabled())
        {
            ++functions_iterator;
            continue;
        }

        // When the Function Only should Execute in Combat
        if (function->getCombatUsage() && !mOwner->_isInCombat())
        {
            ++functions_iterator;
            continue;
        }

        // When the execution should happen in other Phase not proceed
        // is bound to a specific phase (all greater than 0)
        if (!function->isAvailableForScriptPhase(mOwner->getScriptPhase()))
        {
            ++functions_iterator;
            continue;
        }

        // When Chance is 0 and a cooldowntimer is set we assume it should execute always
        // Manual events are always without timers by now
        if (!function->getInitialCooldown().count() && !function->isChanceMeet(0.0f))
        {
            ++functions_iterator;
            continue;
        }

        // When no Uses are left break
        if (!function->usesLeft())
        {
            // Flag us Ready to be Removed
            function->mGarbage = true;

            ++functions_iterator;
            continue;
        }

        // When not in Hp Range dont do anything
        if (!function->isHpInPercentRange(mOwner->_getHealthPercent()))
        {
            ++functions_iterator;
            continue;
        }

        // We are a Spell Function so lets do some cheks dependant on that
        if (function->getFunctionArgs().getSpellId())
        {
            // do not proceed when we are Casting
            if (mOwner->_isCasting())
            {
                ++functions_iterator;
                continue;
            }

            // do not cast any spell while stunned/feared/silenced/charmed/confused
            if (mOwner->getCreature()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_CHARMED | UNIT_STATE_CONFUSED))
            {
                ++functions_iterator;
                continue;
            }

            auto mSpellInfo = sSpellMgr.getSpellInfo(function->getFunctionArgs().getSpellId());

            // check if creature has Mana/Power required to cast
            if (mSpellInfo->getPowerType() == POWER_TYPE_MANA)
            {
                if (mSpellInfo->getManaCost() > mOwner->getCreature()->getPower(POWER_TYPE_MANA))
                {
                    ++functions_iterator;
                    continue;
                }
            }
            
            if (mSpellInfo->getPowerType() == POWER_TYPE_FOCUS)
            {
                if (mSpellInfo->getManaCost() > mOwner->getCreature()->getPower(POWER_TYPE_FOCUS))
                {
                    ++functions_iterator;
                    continue;
                }
            }

            // aura stacking only cast the spells when its under the specified amount
            if (mOwner->getCreature()->getAuraCountForId(mSpellInfo->getId()) >= function->getFunctionArgs().getMaxStackCount())
            {
                ++functions_iterator;
                continue;
            }
        }

        // Casting Should be Possible reduce Timers
        function->decreaseCooldownTimer(Milliseconds(time_passed));

        // When our Timer is over Execute the Function
        if (function->getCooldownTimer().count() <= 0)
        {
            if (function->getFunction())
            {
                // If we are not Lucky just try again in 1.5s
                if (!function->isChanceMeet(rolled))
                {
                    function->setCooldownTimer(1500ms);
                    ++functions_iterator;
                    continue;
                }

                // Add One usage
                function->increaseCount();

                // Execute the Function
                function->getFunction()(function);

                // Reset Our CooldownTimer
                if (function->getFunctionArgs().getSpellId() && function->getFunctionArgs().getUseSpellCD())
                    function->setCooldownTimer(function->getSpellCooldown());
                else
                    function->setCooldownTimer(function->getInitialCooldown());

                ++functions_iterator;
                continue;
            }
        }
        ++functions_iterator;
    }
}

void CreatureAIFunctionScheduler::resetEvents()
{
    if (!mCreatureAIFunctions.size())
        return;

    for (auto function : mCreatureAIFunctions)
    {
        function->reset();
    }
}

void CreatureAIFunctionScheduler::removeAll()
{
    mCreatureAIFunctions.clear();
}

void CreatureAIFunctionScheduler::delayAllEvents(Milliseconds time)
{
    if (!mCreatureAIFunctions.empty())
    {
        for (auto itr = mCreatureAIFunctions.begin(); itr != mCreatureAIFunctions.end();)
        {
            // Only Delay Timers that are not Finished and in our Current Phase
            if (itr->get()->getCooldownTimer().count() > 0 && (itr->get()->isAvailableForScriptPhase(mOwner->getScriptPhase())))
                itr->get()->setCooldownTimer(itr->get()->getCooldownTimer() + time);

            ++itr;
        }
    }
}
