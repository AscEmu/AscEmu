/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureAIFunction.hpp"
#include "CreatureAIScript.h"

void CreatureAIFunction::reset()
{
    // Reset Timers and Charges
    setCooldownTimer(getInitialCooldown());
    setCount(0);
}

const bool CreatureAIFunction::isHpInPercentRange(float targetHp)
{
    if (targetHp >= schedulerArgs.getMinHPRange() && targetHp <= schedulerArgs.getMaxHPRange())
        return true;

    return false;
}

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
