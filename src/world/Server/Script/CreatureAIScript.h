/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Chat/ChatDefines.hpp"

class Creature;
class CreatureAIScript;

enum AISpellTargetType
{
    TARGET_SELF,
    TARGET_VARIOUS,
    TARGET_ATTACKING,
    TARGET_DESTINATION,
    TARGET_SOURCE,
    TARGET_RANDOM_FRIEND,
    TARGET_RANDOM_SINGLE,
    TARGET_RANDOM_DESTINATION,
    TARGET_CUSTOM
};

class SERVER_DECL CreatureAISpells
{
public:
    CreatureAISpells(SpellInfo* spellInfo, float castChance, uint32_t targetType, uint32_t duration, uint32_t cooldown, bool forceRemove, bool isTriggered)
    {
        mSpellInfo = spellInfo;
        mCastChance = castChance;
        mTargetType = targetType;
        mDuration = duration;

        mDurationTimerId = 0;

        mCooldown = cooldown;
        mCooldownTimerId = 0;
        mForceRemoveAura = forceRemove;
        mIsTriggered = isTriggered;

        mMaxStackCount = 1;

        mMinPositionRangeToCast = 0.0f;
        mMaxPositionRangeToCast = 0.0f;

        mMinHpRangeToCast = 0;
        mMaxHpRangeToCast = 100;

        if (mSpellInfo != nullptr)
        {
            mMinPositionRangeToCast = GetMinRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
            mMaxPositionRangeToCast = GetMaxRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
        }

        mAttackStopTimer = 0;

        mCustomTargetCreature = nullptr;
    }

    ~CreatureAISpells()
    {
    }

    SpellInfo* mSpellInfo;
    float mCastChance;
    uint32_t mTargetType;
    uint32_t mDuration;

    void setdurationTimer(uint32_t durationTimer);

    uint32_t mDurationTimerId;

    void setCooldownTimerId(uint32_t cooldownTimer);

    uint32_t mCooldown;
    uint32_t mCooldownTimerId;

    bool mForceRemoveAura;
    bool mIsTriggered;

    // non db script messages
    struct AISpellEmotes
    {
        AISpellEmotes(std::string pText, uint8_t pType, uint32_t pSoundId)
        {
            mText = (!pText.empty() ? pText : "");
            mType = pType;
            mSoundId = pSoundId;
        }

        std::string mText;
        uint8_t mType;
        uint32_t mSoundId;
    };
    typedef std::vector<AISpellEmotes> AISpellEmoteArray;
    AISpellEmoteArray mAISpellEmote;

    void addDBEmote(uint32_t textId);
    void addEmote(std::string pText, uint8_t pType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0);

    void sendRandomEmote(CreatureAIScript* creatureAI);

    uint32_t mMaxStackCount;

    void setMaxStackCount(uint32_t stackCount);
    uint32_t getMaxStackCount();

    float mMinPositionRangeToCast;
    float mMaxPositionRangeToCast;

    bool isDistanceInRange(float targetDistance);
    void setMinMaxDistance(float minDistance, float maxDistance);

    // if it is not a random target type it sets the hp range when the creature can cast this spell
    // if it is a random target it controles when the spell can be cast based on the target hp
    int mMinHpRangeToCast;
    int mMaxHpRangeToCast;

    bool isHpInPercentRange(int targetHp);
    void setMinMaxPercentHp(int minHp, int maxHp);

    typedef std::vector<uint32_t> ScriptPhaseList;
    ScriptPhaseList mPhaseList;

    void setAvailableForScriptPhase(std::vector<uint32_t> phaseVector);
    bool isAvailableForScriptPhase(uint32_t scriptPhase);

    uint32_t mAttackStopTimer;
    void setAttackStopTimer(uint32_t attackStopTime);
    uint32_t getAttackStopTimer();

    std::string mAnnouncement;
    void setAnnouncement(std::string announcement);
    void sendAnnouncement(CreatureAIScript* creatureAI);

    Creature* mCustomTargetCreature;
    void setCustomTarget(Creature* targetCreature);
    Creature* getCustomTarget();
};
