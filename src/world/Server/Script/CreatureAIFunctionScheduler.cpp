/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureAIFunctionScheduler.hpp"
#include "CreatureAIFunction.hpp"
#include "CreatureAIScript.hpp"
#include "Logging/Logger.hpp"
#include "Spell/SpellInfo.hpp"
#include "Utilities/Random.hpp"

CreatureAIFunctionScheduler::CreatureAIFunctionScheduler(CreatureAIScript* script) : mOwner(script) { }

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

    sLogger.failure("tried to add invalid spell with id {}", funcArgs.getSpellId());

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
    // Copy all temporary functions to the scheduler
    mCreatureAIFunctions.insert(mCreatureAIFunctions.end(), mCreatureAIFunctionsTemporary.begin(), mCreatureAIFunctionsTemporary.end());
    mCreatureAIFunctionsTemporary.clear(); // Empty temporary function vector

    if (!mCreatureAIFunctions.size())
        return; // Nothing to do, so skip

    // Randomize the order of functions
    // Warning when 2 functions have 2 seconds timer the first always would executes before the second one...
    // Randomizing the order could lead to unexpected behaviour when u rely on the order of insertion
    Util::randomShuffleVector(&mCreatureAIFunctions);

    // Delete functions marked for removal
    mCreatureAIFunctions.erase(std::remove_if(mCreatureAIFunctions.begin(), mCreatureAIFunctions.end(), [](const auto& function) {
        return function->mGarbage;
        }), mCreatureAIFunctions.end());

    for (auto functions_iterator = mCreatureAIFunctions.begin(); functions_iterator != mCreatureAIFunctions.end(); ++functions_iterator)
    {
        auto function = *functions_iterator;

        // Roll the Dice :)
        float rolled = Util::getRandomFloat(100.0f);

        if (!function->isEnabled())
            continue; // Function is currently disabled, skip

        if (function->getCombatUsage() && !mOwner->_isInCombat())
            continue; // Function should only execute in combat, but not in combat now, skip

        if (!function->isAvailableForScriptPhase(mOwner->getScriptPhase()))
            continue; // Execution should happen in another phase, skip

        if (!function->getInitialCooldown().count() && !function->isChanceMeet(0.0f))
            continue; // Chance is 0 and no cooldown timer, assume it should execute always, but it didn't meet the chance, skip

        if (!function->usesLeft())
        {
            function->mGarbage = true; // No uses left, mark for removal
            continue;
        }

        if (!function->isHpInPercentRange(static_cast<float>(mOwner->_getHealthPercent())))
            continue; // Not in HP range, skip

        // We are a Spell Function so lets do some cheks dependant on that
        if (function->getFunctionArgs().getSpellId())
        {
            if (mOwner->getCreature()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_CHARMED | UNIT_STATE_CONFUSED))
                continue; // do not cast any spell while stunned/feared/silenced/charmed/confused

            if (mOwner->_isCasting())
                continue; // Creature is casting, skip

            // Check if creature can cast the spell based on required Mana/Power
            auto mSpellInfo = sSpellMgr.getSpellInfo(function->getFunctionArgs().getSpellId());
            if (mSpellInfo->getPowerType() == POWER_TYPE_MANA)
            {
                if (mSpellInfo->getManaCost() > mOwner->getCreature()->getPower(POWER_TYPE_MANA))
                    continue; // Not enough Mana, skip
            }
            else if (mSpellInfo->getPowerType() == POWER_TYPE_FOCUS)
            {
                if (mSpellInfo->getManaCost() > mOwner->getCreature()->getPower(POWER_TYPE_FOCUS))
                    continue; // Not enough Focus, skip
            }

            // Aura stacking: Only cast the spell when it's under the specified amount
            if (mOwner->getCreature()->getAuraCountForId(mSpellInfo->getId()) >= function->getFunctionArgs().getMaxStackCount())
                continue; // Aura stack count reached, skip
        }

        // Casting Should be Possible reduce Timers
        function->decreaseCooldownTimer(Milliseconds(time_passed));

        // When our Timer is over Execute the Function
        if (function->getCooldownTimer().count() <= 0)
        {
            if (function->getFunction())
            {
                // If the chance is not met, try again in 1.5 seconds
                if (!function->isChanceMeet(rolled))
                {
                    function->setCooldownTimer(1500ms);
                    continue;
                }

                // Execute the Function
                function->increaseCount();
                function->getFunction()(function);

                // Reset the cooldown timer
                if (function->getFunctionArgs().getSpellId() && function->getFunctionArgs().getUseSpellCD())
                    function->setCooldownTimer(function->getSpellCooldown());
                else
                    function->setCooldownTimer(function->getInitialCooldown());
            }
        }
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
