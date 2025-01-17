/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <vector>
#include <cstdint>
#include <functional>
#include <string>
#include <chrono>

class Unit;
enum TargetFilter : uint32_t;
enum EmoteType : uint32_t;

// Makes std::chrono_literals globally available.
using namespace std::chrono_literals;

// forward declerations
class CreatureAIFunction;
class CreatureAIScript;
struct AIMessage;

// typedefinitions
typedef std::chrono::milliseconds Milliseconds;
typedef std::vector<AIMessage> AIMessagesArray;
typedef std::vector<AIMessagesArray> Messages;
typedef std::vector<uint32_t> ScriptPhaseList;
typedef std::weak_ptr<CreatureAIFunction> CreatureAIFunc;
typedef std::function<void(CreatureAIFunc)> Function;

// Target Filtering Arguments
struct SERVER_DECL FilterArgs
{
public:
    FilterArgs() = default;
    FilterArgs(TargetFilter filter) { addFilter(filter); }
    FilterArgs(TargetFilter filter, float pMinRange, float pMaxRange, int32_t auraId = 0) : minRange(pMinRange), maxRange(pMaxRange), auraId(auraId) { addFilter(filter); }
    FilterArgs(TargetFilter filter, float pMinRange, float pMaxRange, float pMinHealth, float pMaxHealth, int32_t auraId = 0) : minRange(pMinRange), maxRange(pMaxRange), minHPRange(pMinHealth), maxHPRange(pMaxHealth), auraId(auraId) { addFilter(filter); }

    void setMinMaxRange(float min, float max) { minRange = min; maxRange = max; }
    void setMinMaxHPRange(float min, float max) { minHPRange = min; maxHPRange = max; }
    void setAuraId(int32_t pAuraId) { auraId = pAuraId; }

    TargetFilter getTargetFilter() const { return TargetFilter(pFilter); }
    void addFilter(uint32_t filter) { pFilter |= filter; }
    void removeFilter(uint32_t filter) { pFilter &= ~filter; }
    bool hasFilter(TargetFilter filter) const { return (getTargetFilter() & filter) != 0; }

    const float getMinRange() { return minRange; }
    const float getMaxRange() { return maxRange; }
    const float getMinHPRange() { return minHPRange; }
    const float getMaxHPRange() { return maxHPRange; }
    const int32_t getAuraId() { return auraId; }

private:
    uint32_t pFilter = 0;
    float minRange = 0.0f;
    float maxRange = 100.0f;

    float minHPRange = 1.0f;
    float maxHPRange = 100.0f;

    int32_t auraId = 0;
};

enum MessageSlot : uint8_t
{
    SLOT_DEFAULT,
    SLOT_BONUS,
    SLOT_MAX
};

struct AIMessage
{
    uint32_t messageId;
    bool useTarget;
};

struct SERVER_DECL SchedulerArgs
{
public:
    SchedulerArgs() {}

    // Flow
    bool isEnabled() { return !mDisabled; }
    void setEnabled(bool set) { mDisabled = !set; }

    // Combat
    void setOnlyInCombat(bool value) { mOnlyInCombat = value; }
    bool getCombatUsage() { return mOnlyInCombat; }

    // Timers
    void setInitialCooldown(Milliseconds cooldown) { mCooldown = cooldown; }
    Milliseconds getInitialCooldown() const { return mCooldown; }

    void setSpellCooldown(Milliseconds cooldown) { mSpellCooldown = cooldown; }
    Milliseconds getSpellCooldown() const { return mSpellCooldown; }

    // Usages
    void setMaxCount(uint32_t count) { mMaxCount = count; }
    const uint32_t getMaxCount() { return mMaxCount; }

    // Chances
    void setChance(float chance) { mChance = chance; }
    float getChance() { return mChance; }

    // Health
    void setMinMaxPercentHp(float minHp, float maxHp) { mMinHpRange = minHp; mMaxHpRange = maxHp; }
    float getMinHPRange() { return mMinHpRange; }
    float getMaxHPRange() { return mMaxHpRange; }

    // Phases
    void setAvailableForScriptPhase(std::vector<uint32_t> phaseVector) { for (const auto& phase : phaseVector) mPhaseList.push_back(phase); }

    // Random Time
    std::chrono::milliseconds randtime(std::chrono::milliseconds min, std::chrono::milliseconds max);

    ScriptPhaseList mPhaseList = {};

private:
    float           mChance         = 0.0f;             // Percent of the Function to Be Executed
    int32_t         mMaxCount       = 0;                // How Often This Event Should Happen
    Milliseconds    mCooldown       = Milliseconds();   // Function cooldown
    Milliseconds    mSpellCooldown  = Milliseconds();   // Spell cooldown
    float           mMinHpRange     = 0.0f;             // Min HP To Schedule This Event
    float           mMaxHpRange     = 100.0f;           // Max HP To Schedile this Event
    bool            mOnlyInCombat   = false;            // Only Schedule this Event in Combat
    bool            mDisabled       = false;            // Disables Function
};

struct DoOnceScheduler : SchedulerArgs
{
    DoOnceScheduler() { setMaxCount(1); }
    DoOnceScheduler(bool combat) { setMaxCount(1); setOnlyInCombat(combat); }
    DoOnceScheduler(Milliseconds cooldown, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setMaxCount(1); setInitialCooldown(cooldown); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
    DoOnceScheduler(Milliseconds cooldownMin, Milliseconds cooldownMax, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setMaxCount(1); setInitialCooldown(randtime(cooldownMin, cooldownMax)); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
};

struct DoLoopScheduler : SchedulerArgs
{
    DoLoopScheduler() {}
    DoLoopScheduler(bool combat) { setOnlyInCombat(combat); }
    DoLoopScheduler(Milliseconds cooldown, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setInitialCooldown(cooldown); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
    DoLoopScheduler(Milliseconds cooldownMin, Milliseconds cooldownMax, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setInitialCooldown(randtime(cooldownMin, cooldownMax)); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
};

struct DoCountScheduler : SchedulerArgs
{
    DoCountScheduler(int32_t count) { setMaxCount(count); }
    DoCountScheduler(int32_t count, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setMaxCount(count); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
    DoCountScheduler(int32_t count, Milliseconds cooldown, bool combat = false, std::vector<uint32_t> phaseSet = {}) { setMaxCount(count); setInitialCooldown(cooldown); setOnlyInCombat(combat); setAvailableForScriptPhase(phaseSet); }
    DoCountScheduler(int32_t count, Milliseconds cooldownMin, Milliseconds cooldownMax, float chance = 0.0f, std::vector<uint32_t> phaseSet = {}) { setMaxCount(count); setInitialCooldown(randtime(cooldownMin, cooldownMax)); setChance(chance); setOnlyInCombat(false); setAvailableForScriptPhase(phaseSet); }
};

// Function Arguments
struct FunctionArgs
{
public:
    FunctionArgs() : mTargetFilters() { mAIMessages.resize(SLOT_MAX); }

    // Spells
    void setSpellId(uint32_t spellId) { mSpellId = spellId; }

    void setTriggered(bool triggered) { mTriggered = triggered; }
    void setMinMaxHPRange(float min, float max) { mMinHpRange = min; mMaxHpRange = max; }
    void setMaxStackCount(uint32_t stackCount) { mMaxStackCount = stackCount; }
    void setTargetFilters(FilterArgs filters) { mTargetFilters = filters; }

    void setCastTimer(Milliseconds time) { mCastTime = time; }
    void addAnnouncement(std::string announcement) { mAnnouncement = announcement; }
    void setUseSpellCD(bool value) { mUseSpellColdown = value; }

    uint32_t getSpellId() const { return mSpellId; }
    bool isTriggered() const { return mTriggered; }
    float getMinHPRange() const { return mMinHpRange; }
    float getMaxHPRange() const { return mMaxHpRange; }
    uint32_t getMaxStackCount() const { return mMaxStackCount; }
    //std::function<Unit* ()> getTargetFunction() const { return mTargetFunction; }
    FilterArgs getTargetFilters() const { return mTargetFilters; }

    Milliseconds getCastTimer() { return mCastTime; }
    std::string getAnnouncement() const { return mAnnouncement; }
    bool getUseSpellCD() const { return mUseSpellColdown; }

    // Emotes
    EmoteType getEmote() { return mEmote; }
    void setEmote(EmoteType emote) { mEmote = emote; }
    void setEmoteState(bool state) { mEmoteState = state; }
    bool useEmoteState() const { return mEmoteState; }

    // Messages also works for Spells which should send a message on Cast
    AIMessagesArray getAIMessages(uint8_t slot = 0) const { return mAIMessages[slot]; }
    void setMessageTarget(Unit* target) { mTarget = target; }
    Unit* getMessageTarget() const { return mTarget; }

    void addDBMessage(uint32_t textId, MessageSlot slot = MessageSlot::SLOT_DEFAULT, bool useTarget = false)
    {
        if (slot >= mAIMessages.size()) 
        {
            mAIMessages.resize(slot + 1);
        }

        mAIMessages[slot].emplace_back(textId, useTarget);
    }

private:
    // Spell Casts
    uint32_t                mSpellId    = 0;                // Spellid to use
    bool                    mTriggered  = false;            // is Spell Triggered?
    float                   mMinHpRange = 0.0f;             // Min HP To Schedule This Event
    float                   mMaxHpRange = 100.0f;           // Max HP To Schedile this Event
    uint32_t                mMaxStackCount = 1;             // Max Stack count for Auras
    Milliseconds            mCastTime = Milliseconds();     // CastTime for CurrentSpell
    std::function<Unit* ()> mTargetFunction = nullptr;      // Custom Targeting Function
    FilterArgs              mTargetFilters;
    std::string             mAnnouncement;                  // Announcements On Begin Cast
    bool                    mUseSpellColdown = false;       // Uses Spell Cooldown on Reschedule

    // Messages
    Unit*                   mTarget = nullptr;              // Target for Message
    Messages                mAIMessages;                    // Vector for Messages

    // Emote
    EmoteType               mEmote;
    bool                    mEmoteState = false;
};

struct Message : FunctionArgs
{
    Message() {}
    Message(uint32_t dbTextId, Unit* target = nullptr) { addDBMessage(dbTextId, MessageSlot::SLOT_DEFAULT, true); setMessageTarget(target); }
    Message(uint32_t dbTextId, bool useTarget, Unit* target = nullptr) { addDBMessage(dbTextId, MessageSlot::SLOT_DEFAULT, useTarget); setMessageTarget(target); }
};

struct SpellDesc : FunctionArgs
{
    SpellDesc() {}
    SpellDesc(uint32_t pSpellId, FilterArgs filter, bool triggered = false) { setSpellId(pSpellId); setTargetFilters(filter); setTriggered(triggered); }
};

struct Emote : FunctionArgs
{
    Emote() {}
    Emote(EmoteType emote, bool useState = false) { setEmote(emote); setEmoteState(useState); }
};

enum FunctionType : uint8_t
{
    Type_None       = 0,
    Type_Spell,
    Type_Message,
    Type_Emote
};
