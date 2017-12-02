/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "CreatureAIScript.h"
#include "Storage/MySQLDataStore.hpp"

//////////////////////////////////////////////////////////////////////////////////////////

void CreatureAISpells::setdurationTimer(uint32_t durationTimer)
{
    mDurationTimerId = durationTimer;
}

void CreatureAISpells::setCooldownTimerId(uint32_t cooldownTimer)
{
    mCooldownTimerId = cooldownTimer;
}

void CreatureAISpells::addDBEmote(uint32_t textId)
{
    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
    if (npcScriptText != nullptr)
        addEmote(npcScriptText->text, npcScriptText->type, npcScriptText->sound);
    else
        LogDebugFlag(LF_SCRIPT_MGR, "A script tried to add a spell emote with %u! Id is not available in table npc_script_text.", textId);
}

void CreatureAISpells::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mAISpellEmote.push_back(AISpellEmotes(pText, pType, pSoundId));
}

void CreatureAISpells::sendRandomEmote(CreatureAIScript* creatureAI)
{
    if (!mAISpellEmote.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendRandomEmote() : called");

        uint32_t randomUInt = (mAISpellEmote.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mAISpellEmote.size() - 1)) : 0;
        creatureAI->getCreature()->SendChatMessage(mAISpellEmote[randomUInt].mType, LANG_UNIVERSAL, mAISpellEmote[randomUInt].mText.c_str());

        if (mAISpellEmote[randomUInt].mSoundId != 0)
            creatureAI->getCreature()->PlaySoundToSet(mAISpellEmote[randomUInt].mSoundId);
    }
}

void CreatureAISpells::setMaxStackCount(uint32_t stackCount)
{
    mMaxStackCount = stackCount;
}

uint32_t CreatureAISpells::getMaxStackCount()
{
    return mMaxStackCount;
}

bool CreatureAISpells::isDistanceInRange(float targetDistance)
{
    if (targetDistance >= mMinPositionRangeToCast && targetDistance <= mMaxPositionRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxDistance(float minDistance, float maxDistance)
{
    mMinPositionRangeToCast = minDistance;
    mMaxPositionRangeToCast = maxDistance;
}

bool CreatureAISpells::isHpInPercentRange(int targetHp)
{
    if (targetHp >= mMinHpRangeToCast && targetHp <= mMaxHpRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxPercentHp(int minHp, int maxHp)
{
    mMinHpRangeToCast = minHp;
    mMaxHpRangeToCast = maxHp;
}

void CreatureAISpells::setAvailableForScriptPhase(std::vector<uint32_t> phaseVector)
{
    for (const auto& phase : phaseVector)
    {
        mPhaseList.push_back(phase);
    }
}

bool CreatureAISpells::isAvailableForScriptPhase(uint32_t scriptPhase)
{
    if (mPhaseList.empty())
        return true;

    for (const auto& availablePhase : mPhaseList)
    {
        if (availablePhase == scriptPhase)
            return true;
    }

    return false;
}

void CreatureAISpells::setAttackStopTimer(uint32_t attackStopTime)
{
    mAttackStopTimer = attackStopTime;
}

uint32_t CreatureAISpells::getAttackStopTimer()
{
    return mAttackStopTimer;
}

void CreatureAISpells::setAnnouncement(std::string announcement)
{
    mAnnouncement = announcement;
}

void CreatureAISpells::sendAnnouncement(CreatureAIScript* creatureAI)
{
    if (!mAnnouncement.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendAnnouncement() : called");

        creatureAI->getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, mAnnouncement.c_str());
    }
}

void CreatureAISpells::setCustomTarget(Creature* targetCreature)
{
    mCustomTargetCreature = targetCreature;
}

Creature* CreatureAISpells::getCustomTarget()
{
    return mCustomTargetCreature;
}
