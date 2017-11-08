/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Base.h"
#include "Objects/Faction.h"


//////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
TargetType::TargetType(uint32 pTargetGen, TargetFilter pTargetFilter, uint32 pMinTargetNumber, uint32 pMaxTargetNumber)
{
    mTargetGenerator = pTargetGen;
    mTargetFilter = pTargetFilter;
    mTargetNumber[0] = pMinTargetNumber;    // Unused array for now
    mTargetNumber[1] = pMaxTargetNumber;
};

TargetType::~TargetType()
{
};

//////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
SpellDesc::SpellDesc(SpellInfo* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange,
                     bool pStrictRange, const char* pText, uint8 pTextType, uint32 pSoundId, const char* pAnnouncement)
{
    mInfo = pInfo;
    mSpellFunc = pFnc;
    mTargetType = pTargetType;
    mChance = std::max(std::min(pChance, 100.0f), 0.0f);
    mCastTime = pCastTime;
    mCooldown = pCooldown;
    mMinRange = pMinRange;
    mMaxRange = pMaxRange;
    mStrictRange = pStrictRange;
    mEnabled = true;
    mPredefinedTarget = nullptr;
    mLastCastTime = 0;
    AddAnnouncement(pAnnouncement);
    AddEmote(pText, pTextType, pSoundId);
}

SpellDesc::~SpellDesc()
{
    DeleteArray(mEmotes);
}

EmoteDesc* SpellDesc::AddEmote(const char* pText, uint8 pType, uint32 pSoundId)
{
    EmoteDesc* NewEmote = nullptr;
    if (pText || pSoundId)
    {
        NewEmote = new EmoteDesc(pText, pType, pSoundId);
        mEmotes.push_back(NewEmote);
    }
    return NewEmote;
}

void SpellDesc::TriggerCooldown(uint32 pCurrentTime)
{
    uint32 CurrentTime = (pCurrentTime == 0) ? (uint32)time(nullptr) : pCurrentTime;
    mLastCastTime = CurrentTime;
}

void SpellDesc::AddAnnouncement(const char* pText)
{
    mAnnouncement = (pText && strlen(pText) > 0) ? pText : "";
}

//////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptCreatureAI
MoonScriptCreatureAI::MoonScriptCreatureAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mPhaseIndex = -1;
    mEnrageSpell = nullptr;
    mEnrageTimerDuration = -1;
    mEnrageTimer = INVALIDATE_TIMER;

    mRunToTargetCache = nullptr;
    mRunToTargetSpellCache = nullptr;
    mAIUpdateFrequency = defaultUpdateFrequency;
    mBaseAttackTime = getCreature()->GetBaseAttackTime(MELEE);
}

MoonScriptCreatureAI::~MoonScriptCreatureAI()
{
    mPhaseSpells.clear();

    DeleteArray(mOnDiedEmotes);
    DeleteArray(mOnTargetDiedEmotes);
    DeleteArray(mOnCombatStartEmotes);
    DeleteArray(mOnTauntEmotes);
    DeleteArray(mSpells);
}

void MoonScriptCreatureAI::MoveTo(Unit* pUnit, RangeStatusPair pRangeStatus)
{
    if (pRangeStatus.first == RangeStatus_TooClose)
        getCreature()->GetAIInterface()->_CalcDestinationAndMove(pUnit, pRangeStatus.second);
    else if (pRangeStatus.first == RangeStatus_TooFar)
        moveTo(pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ());
};

void MoonScriptCreatureAI::AggroNearestUnit(uint32 pInitialThreat)
{
    //Pay attention: if this is called before pushing the Creature to world, OnCombatStart will NOT be called.
    Unit* NearestRandomTarget = GetBestUnitTarget(TargetFilter_Closest);
    if (NearestRandomTarget)
        getCreature()->GetAIInterface()->AttackReaction(NearestRandomTarget, pInitialThreat);
}

void MoonScriptCreatureAI::AggroRandomUnit(uint32 pInitialThreat)
{
    Unit* RandomTarget = GetBestUnitTarget();
    if (RandomTarget)
    {
        getCreature()->GetAIInterface()->AttackReaction(RandomTarget, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(RandomTarget);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

void MoonScriptCreatureAI::AggroNearestPlayer(uint32 pInitialThreat)
{
    Unit* NearestRandomPlayer = GetBestPlayerTarget(TargetFilter_Closest);
    if (NearestRandomPlayer)
    {
        getCreature()->GetAIInterface()->AttackReaction(NearestRandomPlayer, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(NearestRandomPlayer);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

void MoonScriptCreatureAI::AggroRandomPlayer(uint32 pInitialThreat)
{
    Unit* RandomPlayer = GetBestPlayerTarget();
    if (RandomPlayer)
    {
        getCreature()->GetAIInterface()->AttackReaction(RandomPlayer, pInitialThreat);
        if (!_isInCombat())
            OnCombatStart(RandomPlayer);    //Patch, for some reason, OnCombatStart isn't called in this case
    }
}

MoonScriptCreatureAI* MoonScriptCreatureAI::GetNearestCreature(uint32 pCreatureId)
{
    Creature* NearestCreature = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), pCreatureId);
    return (NearestCreature) ? static_cast< MoonScriptCreatureAI* >(NearestCreature->GetScript()) : nullptr;
}

MoonScriptCreatureAI* MoonScriptCreatureAI::SpawnCreature(uint32 pCreatureId, float pX, float pY, float pZ, float pO, bool pForceSameFaction, uint32 pPhase)
{
    Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(pCreatureId, pX, pY, pZ, pO, true, false, 0, 0, pPhase);
    MoonScriptCreatureAI* CreatureScriptAI = (NewCreature) ? static_cast< MoonScriptCreatureAI* >(NewCreature->GetScript()) : nullptr;
    if (pForceSameFaction && NewCreature)
    {
        uint32 FactionTemplate = getCreature()->GetFaction();
        NewCreature->SetFaction(FactionTemplate);
    }
    return CreatureScriptAI;
}

SpellDesc* MoonScriptCreatureAI::AddSpell(uint32 pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange,
                                          bool pStrictRange, const char* pText, uint8 pTextType, uint32 pSoundId, const char* pAnnouncement)
{
    //Cannot add twice same spell id    - M4ksiu: Disabled, until I rewrite SetPhase(...) function to not disable same spells that are in different phases
    //SpellDesc* NewSpell = FindSpellById(pSpellId);
    //if (NewSpell) return NewSpell;
    SpellDesc* NewSpell = nullptr;

    //Find spell info from spell id
    SpellInfo* Info = sSpellCustomizations.GetSpellInfo(pSpellId);

#ifdef USE_DBC_SPELL_INFO
    float CastTime = (Info->CastingTimeIndex) ? GetCastTime(sSpellCastTimesStore.LookupEntry(Info->CastingTimeIndex)) : pCastTime;
    int32 Cooldown = Info->RecoveryTime;
    float MinRange = (Info->rangeIndex) ? GetMinRange(sSpellRangeStore.LookupEntry(Info->rangeIndex)) : pMinRange;
    float MaxRange = (Info->rangeIndex) ? GetMaxRange(sSpellRangeStore.LookupEntry(Info->rangeIndex)) : pMaxRange;
    LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::AddSpell(%u) : casttime=%.1f cooldown=%d minrange=%.1f maxrange=%.1f", pSpellId, CastTime, Cooldown, MinRange, MaxRange);
#else
    float CastTime = pCastTime;
    int32 Cooldown = pCooldown;
    float MinRange = pMinRange;
    float MaxRange = pMaxRange;
#endif

    //Create new spell
    NewSpell = new SpellDesc(Info, 0, pTargetType, pChance, CastTime, Cooldown, MinRange, MaxRange, pStrictRange, pText, pTextType, pSoundId, pAnnouncement);
    mSpells.push_back(NewSpell);
    return NewSpell;
}

SpellDesc* MoonScriptCreatureAI::AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange,
                                              bool pStrictRange, const char* pText, uint8 pTextType, uint32 pSoundId, const char* pAnnouncement)
{
    if (!pFnc)
        return nullptr;

    //Create new spell
    SpellDesc* NewSpell = new SpellDesc(nullptr, pFnc, pTargetType, pChance, pCastTime, pCooldown, pMinRange, pMaxRange, pStrictRange, pText, pTextType, pSoundId, pAnnouncement);
    mSpells.push_back(NewSpell);
    return NewSpell;
}

void MoonScriptCreatureAI::CastSpell(SpellDesc* pSpell)
{
    if (!IsSpellScheduled(pSpell))
        mQueuedSpells.push_back(pSpell);
}

void MoonScriptCreatureAI::CastSpellNowNoScheduling(SpellDesc* pSpell)
{
    if (CastSpellInternal(pSpell))
        _delayNextAttack(CalcSpellAttackTime(pSpell));
}

SpellDesc* MoonScriptCreatureAI::FindSpellById(uint32 pSpellId)
{
    for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
    {
        if ((*SpellIter)->mInfo && (*SpellIter)->mInfo->getId() == pSpellId)
            return (*SpellIter);
    }
    return nullptr;
}

SpellDesc* MoonScriptCreatureAI::FindSpellByFunc(SpellFunc pFnc)
{
    for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
    {
        if ((*SpellIter)->mSpellFunc == pFnc)
            return (*SpellIter);
    }
    return nullptr;
}

void MoonScriptCreatureAI::TriggerCooldownOnAllSpells()
{
    uint32 CurrentTime = (uint32)time(nullptr);
    for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
    {
        (*SpellIter)->TriggerCooldown(CurrentTime);
    }
}

void MoonScriptCreatureAI::CancelAllCooldowns()
{
    for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
    {
        (*SpellIter)->mLastCastTime = 0;
    }
}

EmoteDesc* MoonScriptCreatureAI::AddEmote(EventType pEventType, const char* pText, uint8 pType, uint32 pSoundId)
{
    EmoteDesc* NewEmote = nullptr;
    if (pText || pSoundId)
    {
        NewEmote = new EmoteDesc(pText, pType, pSoundId);
        switch (pEventType)
        {
            case Event_OnCombatStart:
                mOnCombatStartEmotes.push_back(NewEmote);
                break;
            case Event_OnTargetDied:
                mOnTargetDiedEmotes.push_back(NewEmote);
                break;
            case Event_OnDied:
                mOnDiedEmotes.push_back(NewEmote);
                break;
            case Event_OnTaunt:
                mOnTauntEmotes.push_back(NewEmote);
                break;
            default:
                LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::AddEmote() : Invalid event type!");
                break;
        }
    }
    return NewEmote;
}

EmoteDesc* MoonScriptCreatureAI::AddEmote(EventType pEventType, uint32_t scripttext)
{
    EmoteDesc* NewEmote = nullptr;
    MySQLStructure::NpcScriptText const* ct = sMySQLStore.getNpcScriptText(scripttext);
    uint8 pType = CHAT_MSG_MONSTER_SAY;

    if (ct != nullptr)
    {
        if (ct->type != CHAT_MSG_MONSTER_SAY && ct->type != CHAT_MSG_MONSTER_YELL && ct->type != CHAT_MSG_MONSTER_EMOTE)
        {
            LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::AddEmote() : Invalid Message Type: %u !", ct->type);
        }
        else
        {
            pType = ct->type;
        }

        NewEmote = new EmoteDesc(ct->text.c_str(), pType, ct->sound);
        switch (pEventType)
        {
            case Event_OnCombatStart:
                mOnCombatStartEmotes.push_back(NewEmote);
                break;
            case Event_OnTargetDied:
                mOnTargetDiedEmotes.push_back(NewEmote);
                break;
            case Event_OnDied:
                mOnDiedEmotes.push_back(NewEmote);
                break;
            case Event_OnTaunt:
                mOnTauntEmotes.push_back(NewEmote);
                break;
            default:
                LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::AddEmote() : Invalid event type: %u !", pEventType);
                break;
        }
    }
    return NewEmote;
}

void MoonScriptCreatureAI::RemoveAllEmotes(EventType pEventType)
{
    switch (pEventType)
    {
        case Event_OnCombatStart:
            DeleteArray(mOnCombatStartEmotes);
            break;
        case Event_OnTargetDied:
            DeleteArray(mOnTargetDiedEmotes);
            break;
        case Event_OnDied:
            DeleteArray(mOnDiedEmotes);
            break;
        case Event_OnTaunt:
            DeleteArray(mOnTauntEmotes);
            break;
        default:
            LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::RemoveAllEmotes() : Invalid event type!");
            break;
    }
}

void MoonScriptCreatureAI::Announce(const char* pText)
{
    if (pText && strlen(pText) > 0)
        getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, pText);
}

void MoonScriptCreatureAI::SetTargetToChannel(Unit* pTarget, uint32 pSpellId)
{
    if (pTarget == nullptr)
        getCreature()->SetChannelSpellTargetGUID(0);
    else
        getCreature()->SetChannelSpellTargetGUID(pTarget->GetGUID());

    getCreature()->SetChannelSpellId(pSpellId);
}

Unit* MoonScriptCreatureAI::GetTargetToChannel()
{
    return getCreature()->GetMapMgr()->GetUnit(getCreature()->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
}

void MoonScriptCreatureAI::SetAIUpdateFreq(uint32 pUpdateFreq)
{
    if (mAIUpdateFrequency != pUpdateFreq)
    {
        mAIUpdateFrequency = pUpdateFreq;
        ModifyAIUpdateEvent(mAIUpdateFrequency);
    }
}

uint32 MoonScriptCreatureAI::GetAIUpdateFreq()
{
    return mAIUpdateFrequency;
}

void MoonScriptCreatureAI::OnCombatStart(Unit* pTarget)
{
    SetPhase(1);
    if (mEnrageSpell && mEnrageTimerDuration > 0)
    {
        mEnrageTimer = _addTimer(mEnrageTimerDuration);
    }

    TriggerCooldownOnAllSpells();

    RandomEmote(mOnCombatStartEmotes);
    setAIAgent(AGENT_MELEE);
    RegisterAIUpdateEvent(mAIUpdateFrequency);
}

void MoonScriptCreatureAI::OnCombatStop(Unit* pTarget)
{
    SetPhase(1);
    _removeTimer(mEnrageTimer);

    CancelAllSpells();
    _cancelAllTimers();
    _removeAllAuras();
    setAIAgent(AGENT_NULL);
    RemoveAIUpdateEvent();

    if (_isDespawnWhenInactiveSet())
        despawn(DEFAULT_DESPAWN_TIMER);
}

void MoonScriptCreatureAI::OnTargetDied(Unit* pTarget)
{
    if (_getHealthPercent() > 0)    //Prevent double yelling (OnDied and OnTargetDied)
        RandomEmote(mOnTargetDiedEmotes);
}

void MoonScriptCreatureAI::OnDied(Unit* pKiller)
{
    RandomEmote(mOnDiedEmotes);
    CancelAllSpells();
}

void MoonScriptCreatureAI::AIUpdate()
{
    if (mEnrageSpell && mEnrageTimerDuration > 0 && _isTimerFinished(mEnrageTimer))
    {
        CastSpell(mEnrageSpell);
        _removeTimer(mEnrageTimer);
    }

    SpellDesc* Spell;
    uint32 CurrentTime = (uint32)time(nullptr);

    if (!_isInCombat())
        return;

    //Check if we have a spell scheduled to be cast
    for (SpellDescList::iterator SpellIter = mScheduledSpells.begin(); SpellIter != mScheduledSpells.end();)
    {
        Spell = (*SpellIter);
        if (CastSpellInternal(Spell, CurrentTime))    //Can fail if we are already casting a spell, or if the spell is on cooldown
        {
            if (!mScheduledSpells.empty())            // \todo temporary crashfix, we should use mutax here, but it needs more investigation
                mScheduledSpells.erase(SpellIter);

            break;
        }
        else
            ++SpellIter;
    }

    //Do not schedule spell if we are *currently* casting a non-instant cast spell
    if (!_isCasting() && !mRunToTargetCache)
    {
        //Check if have queued spells that needs to be scheduled before we go back to random casting
        if (!mQueuedSpells.empty())
        {
            Spell = mQueuedSpells.front();
            mScheduledSpells.push_back(Spell);
            mQueuedSpells.pop_front();

            //Stop melee attack for a short while for scheduled spell cast
            if (Spell->mCastTime >= 0)
            {
                _delayNextAttack(mAIUpdateFrequency);
                if (Spell->mCastTime > 0)
                {
                    setRooted(false);
                    setAIAgent(AGENT_SPELL);
                }
            }
            return;    //Scheduling one spell at a time, exit now
        }

        //Try our chance at casting a spell (Will actually be cast on next ai update, so we just
        //schedule it. This is needed to avoid next dealt melee damage while we cast the spell.)
        float ChanceRoll = RandomFloat(100), ChanceTotal = 0;
        for (SpellDescArray::iterator SpellIter = mSpells.begin(); SpellIter != mSpells.end(); ++SpellIter)
        {
            Spell = (*SpellIter);
            if (Spell->mEnabled == false)
                continue;
            if (Spell->mChance == 0)
                continue;

            //Check if spell won the roll
            if ((Spell->mChance == 100 || (ChanceRoll >= ChanceTotal && ChanceRoll < ChanceTotal + Spell->mChance)) &&
                (Spell->mLastCastTime + Spell->mCooldown <= CurrentTime) &&
                !IsSpellScheduled(Spell))
            {
                mScheduledSpells.push_back(Spell);

                //Stop melee attack for a short while for scheduled spell cast
                if (Spell->mCastTime >= 0)
                {
                    _delayNextAttack(mAIUpdateFrequency + CalcSpellAttackTime(Spell));
                    if (Spell->mCastTime > 0)
                    {
                        setRooted(true);
                        setAIAgent(AGENT_SPELL);
                    }
                }
                return;    //Scheduling one spell at a time, exit now
            }
            else if (Spell->mChance != 100)
                ChanceTotal += Spell->mChance;    //Only add spells that aren't 100% chance of casting
        }

        //Go back to default behavior since we didn't decide anything
        setRooted(false);
        setAIAgent(AGENT_MELEE);

        //Random taunts
        if (ChanceRoll >= 95)
            RandomEmote(mOnTauntEmotes);
    }
}

void MoonScriptCreatureAI::Destroy()
{
    delete this;
};

bool MoonScriptCreatureAI::IsSpellScheduled(SpellDesc* pSpell)
{
    return (std::find(mScheduledSpells.begin(), mScheduledSpells.end(), pSpell) == mScheduledSpells.end()) ? false : true;
}

void MoonScriptCreatureAI::CancelAllSpells()
{
    mQueuedSpells.clear();
    mScheduledSpells.clear();
    PopRunToTargetCache();
}

bool MoonScriptCreatureAI::CastSpellInternal(SpellDesc* pSpell, uint32 pCurrentTime)
{
    if (pSpell == nullptr)
        return false;

    //Do not cast if we are already casting
    if (_isCasting())
        return false;

    //We do not cast in special states such as : stunned, feared, silenced, charmed, asleep, confused and if they are not ignored
    if ((~pSpell->mTargetType.mTargetFilter & TargetFilter_IgnoreSpecialStates) && getCreature()->hasUnitStateFlag(
        (UNIT_STATE_STUN | UNIT_STATE_FEAR | UNIT_STATE_SILENCE | UNIT_STATE_CHARM | UNIT_STATE_CONFUSE)))
        return false;

    //Do not cast if we are in cooldown
    uint32 CurrentTime = (pCurrentTime == 0) ? (uint32)time(nullptr) : pCurrentTime;
    if (pSpell->mLastCastTime + pSpell->mCooldown > CurrentTime)
        return false;

    //Check range before casting
    Unit* Target = GetTargetForSpell(pSpell);
    if (Target)
    {
        RangeStatusPair Status;
        if (pSpell->mTargetType.mTargetFilter & TargetFilter_InRangeOnly || (Status = GetSpellRangeStatusToUnit(Target, pSpell)).first == RangeStatus_Ok)
        {
            //Safe-delay if we got special state flag before
            _delayNextAttack(CalcSpellAttackTime(pSpell));

            //If we were running to a target, stop because we're in range now
            PopRunToTargetCache();

            //Do emote associated with this spell
            RandomEmote(pSpell->mEmotes);
            Announce(pSpell->mAnnouncement);

            //Cast spell now
            if (pSpell->mInfo)
                CastSpellOnTarget(Target, pSpell->mTargetType, pSpell->mInfo, (pSpell->mCastTime == 0) ? true : false);
            else if
                (pSpell->mSpellFunc) pSpell->mSpellFunc(pSpell, this, Target, pSpell->mTargetType);
            else
                LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::CastSpellInternal() : Invalid spell!");

            //Store cast time for cooldown
            pSpell->mLastCastTime = CurrentTime;
            return true;
        }
        else if (!pSpell->mStrictRange)   //Target is out of range, run to it
        {
            PushRunToTargetCache(Target, pSpell, Status);
            return false;
        }
    }

    //If we get here, its because the RunToTarget changed type, so its no longer valid, clear it
    PopRunToTargetCache();
    _delayNextAttack(0);        //Cancel attack delay
    return true;            //No targets possible? Consider spell casted nonetheless
}

void MoonScriptCreatureAI::CastSpellOnTarget(Unit* pTarget, TargetType pType, SpellInfo* pEntry, bool pInstant)
{
    switch (pType.mTargetGenerator)
    {
        case TargetGen_Self:
        case TargetGen_Current:
        case TargetGen_Predefined:
        case TargetGen_RandomUnit:
        case TargetGen_RandomPlayer:
            getCreature()->CastSpell(pTarget, pEntry, pInstant);
            break;

        case TargetGen_RandomUnitApplyAura:
        case TargetGen_RandomPlayerApplyAura:
            pTarget->CastSpell(pTarget, pEntry, pInstant);
            break;

        case TargetGen_Destination:
        case TargetGen_RandomUnitDestination:
        case TargetGen_RandomPlayerDestination:
            getCreature()->CastSpellAoF(pTarget->GetPosition(), pEntry, pInstant);
            break;

        default:
            LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::CastSpellOnTarget() : Invalid target type!");
            break;
    };
};

int32 MoonScriptCreatureAI::CalcSpellAttackTime(SpellDesc* pSpell)
{
#ifdef USE_DBC_SPELL_INFO
    return mBaseAttackTime + int32(pSpell->mCastTime);
#else
    return mBaseAttackTime + int32(pSpell->mCastTime * 1000);
#endif
}

RangeStatusPair MoonScriptCreatureAI::GetSpellRangeStatusToUnit(Unit* pTarget, SpellDesc* pSpell)
{
    if (pSpell->mTargetType.mTargetGenerator != TargetGen_Self && pTarget != getCreature() && (pSpell->mMinRange > 0 || pSpell->mMaxRange > 0))
    {
        float Range = getRangeToObject(pTarget);
        if (pSpell->mMinRange > 0 && (Range < pSpell->mMinRange))
            return std::make_pair(RangeStatus_TooClose, pSpell->mMinRange);
        if (pSpell->mMaxRange > 0 && (Range > pSpell->mMaxRange))
            return std::make_pair(RangeStatus_TooFar, pSpell->mMaxRange);
    }

    return std::make_pair(RangeStatus_Ok, 0.0f);
};

Unit* MoonScriptCreatureAI::GetTargetForSpell(SpellDesc* pSpell)
{
    //Check if run-to-target cache and return it if its valid
    if (mRunToTargetCache && mRunToTargetSpellCache == pSpell && IsValidUnitTarget(mRunToTargetCache, TargetFilter_None))
        return mRunToTargetCache;

    //Find a suitable target for the described situation :)
    switch (pSpell->mTargetType.mTargetGenerator)
    {
        case TargetGen_Self:
            if (!isAlive())
                return nullptr;
            if ((pSpell->mTargetType.mTargetFilter & TargetFilter_Wounded) && getCreature()->GetHealthPct() >= 99)
                return nullptr;

            return getCreature();
        case TargetGen_SecondMostHated:
            return getCreature()->GetAIInterface()->GetSecondHated();
        case TargetGen_Current:
        case TargetGen_Destination:
            return getCreature()->GetAIInterface()->getNextTarget();
        case TargetGen_Predefined:
            return pSpell->mPredefinedTarget;
        case TargetGen_RandomPlayer:
        case TargetGen_RandomPlayerApplyAura:
        case TargetGen_RandomPlayerDestination:
            return GetBestPlayerTarget(pSpell->mTargetType.mTargetFilter, pSpell->mMinRange, pSpell->mMaxRange);
        case TargetGen_RandomUnit:
        case TargetGen_RandomUnitApplyAura:
        case TargetGen_RandomUnitDestination:
            return GetBestUnitTarget(pSpell->mTargetType.mTargetFilter, pSpell->mMinRange, pSpell->mMaxRange);
        default:
            LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::GetTargetForSpell() : Invalid target type!");
            return nullptr;
    }
};

Unit* MoonScriptCreatureAI::GetBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    for (std::set< Object* >::iterator PlayerIter = getCreature()->GetInRangePlayerSetBegin(); PlayerIter != getCreature()->GetInRangePlayerSetEnd(); ++PlayerIter)
    {
        if (IsValidUnitTarget(*PlayerIter, pTargetFilter, pMinRange, pMaxRange))
            TargetArray.push_back(static_cast<Unit*>(*PlayerIter));
    }

    return ChooseBestTargetInArray(TargetArray, pTargetFilter);
};

Unit* MoonScriptCreatureAI::GetBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (std::set< Object* >::iterator ObjectIter = getCreature()->GetInRangeSetBegin(); ObjectIter != getCreature()->GetInRangeSetEnd(); ++ObjectIter)
        {
            if (IsValidUnitTarget(*ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(*ObjectIter));
        }

        if (IsValidUnitTarget(getCreature(), pTargetFilter))
            TargetArray.push_back(getCreature());    //Also add self as possible friendly target
    }
    else
    {
        for (std::set< Object* >::iterator ObjectIter = getCreature()->GetInRangeOppFactsSetBegin(); ObjectIter != getCreature()->GetInRangeOppFactsSetEnd(); ++ObjectIter)
        {
            if (IsValidUnitTarget(*ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(*ObjectIter));
        }
    }

    return ChooseBestTargetInArray(TargetArray, pTargetFilter);
};

Unit* MoonScriptCreatureAI::ChooseBestTargetInArray(UnitArray & pTargetArray, TargetFilter pTargetFilter)
{
    //If only one possible target, return it
    if (pTargetArray.size() == 1)
        return pTargetArray[0];

    //Find closest unit if requested
    if (pTargetFilter & TargetFilter_Closest)
        return GetNearestTargetInArray(pTargetArray);

    //Find second most hated if requested
    if (pTargetFilter & TargetFilter_SecondMostHated)
        return GetSecondMostHatedTargetInArray(pTargetArray);

    //Choose random unit in array
    return (pTargetArray.size() > 1) ? pTargetArray[RandomUInt((uint32)pTargetArray.size() - 1)] : NULL;
};

Unit* MoonScriptCreatureAI::GetNearestTargetInArray(UnitArray & pTargetArray)
{
    Unit* NearestUnit = nullptr;
    float Distance, NearestDistance = 99999;
    for (UnitArray::iterator UnitIter = pTargetArray.begin(); UnitIter != pTargetArray.end(); ++UnitIter)
    {
        Distance = getRangeToObject(static_cast<Unit*>(*UnitIter));
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestUnit = (*UnitIter);
        }
    }

    return NearestUnit;
};

Unit* MoonScriptCreatureAI::GetSecondMostHatedTargetInArray(UnitArray & pTargetArray)
{
    Unit* TargetUnit = nullptr;
    Unit* MostHatedUnit = nullptr;
    Unit* CurrentTarget = static_cast<Unit*>(getCreature()->GetAIInterface()->getNextTarget());
    uint32 Threat = 0;
    uint32 HighestThreat = 0;
    for (UnitArray::iterator UnitIter = pTargetArray.begin(); UnitIter != pTargetArray.end(); ++UnitIter)
    {
        TargetUnit = static_cast<Unit*>(*UnitIter);
        if (TargetUnit != CurrentTarget)
        {
            Threat = getCreature()->GetAIInterface()->getThreatByPtr(TargetUnit);
            if (Threat > HighestThreat)
            {
                MostHatedUnit = TargetUnit;
                HighestThreat = Threat;
            }
        }
    }

    return MostHatedUnit;
};

bool MoonScriptCreatureAI::IsValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange)
{
    //Make sure its a valid unit
    if (!pObject->IsUnit())
        return false;
    if (pObject->GetInstanceID() != getCreature()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->IsCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->IsPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;
    if (UnitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
        return false;

    //Check if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        //Skip units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) == 0)
            return false;

        //Skip current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCreature()->GetAIInterface()->getNextTarget())
            return false;

        //Keep only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->GetHealthPct() >= 99)
            return false;

        //Skip targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && getRangeToObject(UnitTarget) > getCreature()->GetAIInterface()->_CalcCombatRange(UnitTarget, false))
            return false;

        //Skip targets not in strict range if requested
        if ((pFilter & TargetFilter_InRangeOnly) && (pMinRange > 0 || pMaxRange > 0))
        {
            float Range = getRangeToObject(UnitTarget);
            if (pMinRange > 0 && Range < pMinRange)
                return false;
            if (pMaxRange > 0 && Range > pMaxRange)
                return false;
        }

        //Skip targets not in Line Of Sight if requested
        if ((~pFilter & TargetFilter_IgnoreLineOfSight) && !getCreature()->IsWithinLOSInMap(UnitTarget))
            return false;

        //Handle hostile/friendly
        if ((~pFilter & TargetFilter_Corpse) && (pFilter & TargetFilter_Friendly))
        {
            if (!UnitTarget->CombatStatus.IsInCombat())
                return false; //Skip not-in-combat targets if friendly
            if (isHostile(getCreature(), UnitTarget) || getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) > 0)
                return false;
        }
    }

    return true; //This is a valid unit target
};

void MoonScriptCreatureAI::PushRunToTargetCache(Unit* pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus)
{
    if (mRunToTargetCache != pTarget)
    {
        mRunToTargetCache = pTarget;
        mRunToTargetSpellCache = pSpell;
        setRooted(false);
        _setMeleeDisabled(true);
        _setRangedDisabled(true);
        _setCastDisabled(true);
    }

    if (mRunToTargetCache)
        MoveTo(mRunToTargetCache, pRangeStatus);
};

void MoonScriptCreatureAI::PopRunToTargetCache()
{
    if (mRunToTargetCache)
    {
        mRunToTargetCache = nullptr;
        mRunToTargetSpellCache = nullptr;
        _setMeleeDisabled(false);
        _setRangedDisabled(false);
        _setCastDisabled(false);
        stopMovement();
    }
};

void MoonScriptCreatureAI::RandomEmote(EmoteArray & pEmoteArray)
{
    int32 ArraySize = (int32)pEmoteArray.size();
    if (ArraySize > 0)
    {
        uint32 Roll = (ArraySize > 1) ? RandomUInt(ArraySize - 1) : 0;
        uint8 type = pEmoteArray[Roll]->mType;
        if (type != CHAT_MSG_MONSTER_SAY && type != CHAT_MSG_MONSTER_YELL && type != CHAT_MSG_MONSTER_EMOTE)
        {
            LogDebugFlag(LF_SCRIPT_MGR, "MoonScriptCreatureAI::RandomEmote() : Invalid text type %u!", type);
            return;
        }
        else
        {
            getCreature()->SendChatMessage(type, LANG_UNIVERSAL, pEmoteArray[Roll]->mText.c_str());
        }

        if (pEmoteArray[Roll]->mSoundId > 0)
            getCreature()->PlaySoundToSet(pEmoteArray[Roll]->mSoundId);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
//MoonScriptBossAI

SpellDesc* MoonScriptCreatureAI::AddPhaseSpell(int32 pPhase, SpellDesc* pSpell)
{
    mPhaseSpells.push_back(std::make_pair(pPhase, pSpell));
    return pSpell;
}

int32 MoonScriptCreatureAI::GetPhase()
{
    return mPhaseIndex;
}

void MoonScriptCreatureAI::SetPhase(int32 pPhase, SpellDesc* pPhaseChangeSpell)
{
    if (mPhaseIndex != pPhase)
    {
        //Cancel all spells
        CancelAllSpells();

        //Enable spells related to that phase
        for (PhaseSpellArray::iterator SpellIter = mPhaseSpells.begin(); SpellIter != mPhaseSpells.end(); ++SpellIter)
        {
            if (SpellIter->first == pPhase)
                SpellIter->second->mEnabled = true;
            else
                SpellIter->second->mEnabled = false;
        }

        //Remember phase index
        mPhaseIndex = pPhase;

        //Cast phase change spell now if available
        if (pPhaseChangeSpell)
            CastSpellNowNoScheduling(pPhaseChangeSpell);
    }
}

void MoonScriptCreatureAI::SetEnrageInfo(SpellDesc* pSpell, int32 pTriggerMilliseconds)
{
    mEnrageSpell = pSpell;
    mEnrageTimerDuration = pTriggerMilliseconds;
}


//////////////////////////////////////////////////////////////////////////////////////////
//Premade Spell Functions
const uint32 SPELLFUNC_VANISH = 24699;

void SpellFunc_ClearHateList(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    pCreatureAI->_clearHateList();
}

void SpellFunc_Disappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    pCreatureAI->_clearHateList();
    pCreatureAI->setRooted(true);
    pCreatureAI->setCanEnterCombat(false);
    pCreatureAI->_applyAura(SPELLFUNC_VANISH);
}

void SpellFunc_Reappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    pCreatureAI->setRooted(false);
    pCreatureAI->setCanEnterCombat(true);
    pCreatureAI->_removeAura(SPELLFUNC_VANISH);
}

void EventFunc_ApplyAura(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    pCreatureAI->_applyAura(uint32(pMiscVal));
    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}

void EventFunc_ChangeGoState(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    MapMgr* pInstance = pCreatureAI->getCreature()->GetMapMgr();
    if (!pInstance)
        return;

    GameObject* pSelectedGO = nullptr;
    uint32 pGOEntry = static_cast<uint32>(pMiscVal);
    for (std::vector< GameObject* >::iterator GOIter = pInstance->GOStorage.begin(); GOIter != pInstance->GOStorage.end(); ++GOIter)
    {
        pSelectedGO = (*GOIter);
        if (pSelectedGO->GetEntry() == pGOEntry)
        {
            pSelectedGO->SetState(pSelectedGO->GetState() == 1 ? 0 : 1);
        }
    }

    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}

void EventFunc_RemoveUnitFieldFlags(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal)
{
    if (!pCreatureAI || pMiscVal <= 0)
        return;

    pCreatureAI->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

    if (!pCreatureAI->_isInCombat() && pCreatureAI->_getTimerCount() == 0)
        pCreatureAI->RemoveAIUpdateEvent();
}
