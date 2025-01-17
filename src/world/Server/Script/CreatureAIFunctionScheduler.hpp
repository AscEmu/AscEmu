/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>
#include <memory>
#include "CreatureAIFunction.hpp"
#include "AIUtils.hpp"

class CreatureAIFunction;
class CreatureAIScript;

class SERVER_DECL CreatureAIFunctionScheduler
{
public:
    CreatureAIFunctionScheduler(CreatureAIScript* script);
    virtual ~CreatureAIFunctionScheduler();

    // Custom Code
    template<typename T>
    CreatureAIFunc addAIFunction(void (T::* memberFunction)(CreatureAIFunc), SchedulerArgs const& schedulerArgs = {}, FunctionArgs const& funcArgs = {});

    CreatureAIFunc addAIFunction(Function pFunction, SchedulerArgs const& pScheduler = {}, FunctionArgs const& funcArgs = {});
    // Spells
    CreatureAIFunc addAISpell(FunctionArgs funcArgs, SchedulerArgs const& pScheduler = {});
    // Messages
    CreatureAIFunc addMessage(FunctionArgs funcArgs, SchedulerArgs const& pScheduler = {});
    // Emote
    CreatureAIFunc addEmote(FunctionArgs funcArgs, SchedulerArgs const& pScheduler = {});

    // Function
    void executeFunction(CreatureAIFunc functionToExec);

    // Cancels Selected Function
    void cancelFunction(CreatureAIFunc functionToExec);

    // Repeats the given Function
    void repeatFunctionFromScheduler(CreatureAIFunc& functionToExec, Milliseconds newTimer = {});

    // Enables Selected Function
    void enableFunction(CreatureAIFunc functionToEnable);

    // Disables Selected Function
    void disableFunction(CreatureAIFunc functionToDisable);

    // Cyclic Update all Events to Schedule
    void update(unsigned long time_passed);

    // Reset all Events
    void resetEvents();

    // Remove All Events
    void removeAll();

    // Delays All Events by the Given Time
    void delayAllEvents(Milliseconds time);

    // Container
    typedef std::vector< std::shared_ptr<CreatureAIFunction>> CreatureAIFunctionsArray;

    // Storage for Functions ready to get Executed
    CreatureAIFunctionsArray mCreatureAIFunctions;

    // Storage for Functions added while iterating Main Vector
    CreatureAIFunctionsArray mCreatureAIFunctionsTemporary;

protected:
    CreatureAIScript* mOwner = nullptr;
};
