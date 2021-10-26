/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VMapFactory.h"
#include "MMapManager.h"
#include "MMapFactory.h"
#include "Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Macros/ScriptMacros.hpp"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Spell/SpellMgr.hpp"
#include "Macros/AIInterfaceMacros.hpp"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellRanged.hpp"
#include "Spell/Definitions/LockTypes.hpp"
#include "Spell/Definitions/SpellIsFlags.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Pet.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/Movement/CreatureMovement.h"
#include "Map/AreaBoundary.h"
#include "Map/MapScriptInterface.h"
#include "Movement/WaypointManager.h"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Server/Script/CreatureAIScript.h"

#ifndef UNIX
#include <cmath>
#endif

// Random and guessed values for Internal Spell cast chance
float spellChanceModifierDispell[12] =
{
    1.4f,   // None
    2.5f,   // Magic
    1.8f,   // Curse
    1.5f,   // Diseas
    1.5f,   // Poison
    1.0f,   // Stealth
    1.0f,   // Invis
    2.8f,   // All
    1.0f,   // Special
    2.0f,   // Frenzy
    1.0f,   // Unk
    1.0f,   // Unk
};

float spellChanceModifierType[12] =
{
    1.0f,    // None
    0.75f,   // Rooted
    0.75f,   // Heal
    0.75f,   // Stun
    0.50f,   // Fear
    0.75f,   // Silence
    0.95f,   // Curse
    0.95f,   // AOE Damage
    0.95f,   // Damage
    0.75f,   // Summon
    1.0f,    // Buff
    1.0f,    // Debuff
};

AIInterface::AIInterface()
    :
    canEnterCombat(true),
    m_isEngaged(false),
    m_Unit(nullptr),
    m_PetOwner(nullptr),
    m_target(nullptr),
    m_UnitToFollow(nullptr),
    m_AiState(AI_STATE_IDLE),
    m_AiScriptType(AI_SCRIPT_LONER),
    m_AiCurrentAgent(AGENT_NULL),
    internalPhase(0),

    m_reactState(REACT_AGGRESSIVE),
    m_lasttargetPosition(0, 0, 0, 0),
    m_AlreadyCallAssistance(false),
    m_AlreadySearchedAssistance(false),
    faction_visibility(0),
    m_boundaryCheckTime(2500),
    _negateBoundary(false),

    m_totemspelltimer(0),
    m_totemspelltime(0),
    totemspell(nullptr),

    m_CallForHelpHealth(0.0f),
    m_FleeHealth(0.0f),
    m_FleeDuration(0),
    m_canFlee(false),
    m_hasFleed(false),
    m_canCallForHelp(false),

    mShowWayPoints(false),

    m_isNeutralGuard(false),
    mIsCombatDisabled(false),
    mIsMeleeDisabled(false),
    mIsRangedDisabled(false),
    mIsCastDisabled(false),
    mIsTargetingDisabled(false),
    m_canRangedAttack(false),
    m_is_in_instance(false),
    timed_emotes(nullptr),
    timed_emote_expire(0xFFFFFFFF)
{
    _boundary.clear();
    m_assistTargets.clear();

    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();

    mLastCastedSpell = nullptr;
    mCurrentSpellTarget = nullptr;
    setCannotReachTarget(false);
    m_fleeTimer.resetInterval(0);
    mSpellWaitTimer.resetInterval(1500);
    m_cannotReachTimer.resetInterval(500);
    m_updateAssistTimer.resetInterval(1000);
    m_updateTargetsTimer.resetInterval(TARGET_UPDATE_INTERVAL);
};

AIInterface::~AIInterface()
{
    mLastCastedSpell = nullptr;
    mCurrentSpellTarget = nullptr;
    mCreatureAISpells.clear();
    clearBoundary();
    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();
}

void AIInterface::Init(Unit* un, AiScriptTypes at)
{
    if (at != AI_SCRIPT_PET)
    {
        setAiScriptType(at);
        setAiState(AI_STATE_IDLE);

        m_Unit = un;
    }
    else
    {
        sLogger.failure("AIInterface::Init something tried to initialise with type AI_SCRIPT_PET, return!");
    }
}

void AIInterface::Init(Unit* un, AiScriptTypes at, Unit* owner)
{
    if (at == AI_SCRIPT_PET || at == AI_SCRIPT_TOTEM)
    {
        setAiScriptType(at);
        setAiState(AI_STATE_IDLE);

        m_Unit = un;
        m_PetOwner = owner;
    }
    else
    {
        sLogger.failure("AIInterface::Init something tried to initialise with owner but type has to be AI_SCRIPT_PET or AI_SCRIPT_TOTEM, return!");
    }
}

void AIInterface::initialiseScripts(uint32_t entry)
{
    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();

    setCanCallForHelp(false);
    setCanFlee(false);

    auto scripts = sMySQLStore.getCreatureAiScripts(entry);

    uint32_t spellcountOnCombatStart = 1;
    uint32_t spellcountOnAIUpdate = 1;

    for (auto aiScripts : *scripts)
    {
        uint8_t eventId = aiScripts.event;

        // Skip not in Current Difficulty
        if (aiScripts.difficulty != 4 && aiScripts.difficulty != getDifficultyType())
            continue;

        switch (eventId)
        {
        case onLoad:
        {
            onLoadScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnLoad.push_back(*message);
            }
        }
            break;
        case onEnterCombat:
        {
            onCombatStartScripts.push_back(aiScripts);
            if (aiScripts.spellId)
                ++spellcountOnCombatStart;

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnCombatStart.push_back(*message);
            }
        }
            break;
        case onLeaveCombat:
        {
            onLeaveCombatScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnLeaveCombat.push_back(*message);
            }
        }
            break;
        case onDied:
        {
            onDiedScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnDied.push_back(*message);
            }
        }
            break;
        case onTargetDied:
        {
            onKilledScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnTargetDied.push_back(*message);
            }
        }
            break;
        case onAIUpdate:
        {
            onAIUpdateScripts.push_back(aiScripts);
            if (aiScripts.spellId)
                ++spellcountOnAIUpdate;

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnAIUpdate.push_back(*message);
            }
        }
            break;
        case onCallForHelp:
        {
            onCallForHelpScripts.push_back(aiScripts);
            setCanCallForHelp(true);
            m_CallForHelpHealth = aiScripts.maxHealth;

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnCallForHelp.push_back(*message);
            }
        }
            break;
        case onRandomWaypoint:
        {
            onRandomWaypointScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnRandomWaypoint.push_back(*message);
            }
        }
            break;
        case onDamageTaken:
        {
            onDamageTakenScripts.push_back(aiScripts);

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnDamageTaken.push_back(*message);
            }
        }
            break;
        case onFlee:
        {
            onFleeScripts.push_back(aiScripts);
            setCanFlee(true);
            m_FleeHealth = aiScripts.maxHealth;

            if (aiScripts.action == actionSendMessage)
            {
                auto message = new AI_SCRIPT_SENDMESSAGES();

                message->textId = aiScripts.textId;
                message->canche = aiScripts.chance;
                message->phase = aiScripts.phase;
                message->healthPrecent = aiScripts.maxHealth;
                message->maxCount = aiScripts.maxCount;

                mEmotesOnFlee.push_back(*message);
            }

            // Incase we want a custom Flee Timer
            if (aiScripts.misc1)
                m_FleeDuration = aiScripts.misc1;
            else
                m_FleeDuration = 10000;
        }
            break;
        default:
            sLogger.debugFlag(AscEmu::Logging::LF_SCRIPT_MGR, "unhandled event with eventId %u", eventId);
        }
    }

    // On Combat Start
    for (auto onCombatStartScript : onCombatStartScripts)
    {
        if (onCombatStartScript.action == actionSpell)
        {
            const auto spellInfo = sSpellMgr.getSpellInfo(onCombatStartScript.spellId);
            float castChance;

            if (spellInfo != nullptr)
            {
                if (onCombatStartScript.chance)
                    castChance = onCombatStartScript.chance;
                else
                    castChance = ((75.0f / spellcountOnCombatStart) * spellChanceModifierDispell[spellInfo->getDispelType()] * spellChanceModifierType[onCombatStartScript.spell_type]);

                sLogger.debug("spell %u chance %f", onCombatStartScript.spellId, castChance);

                uint32_t spellCooldown = Util::getRandomUInt(onCombatStartScript.cooldownMin, onCombatStartScript.cooldownMax);
                if (spellCooldown == 0)
                    spellCooldown = spellInfo->getSpellDefaultDuration(nullptr);

                // Create AI Spell
                CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, onCombatStartScript.target, spellInfo->getSpellDefaultDuration(nullptr), spellCooldown, false, onCombatStartScript.triggered);
                newAISpell->addDBEmote(onCombatStartScript.textId);
                newAISpell->setMaxCastCount(onCombatStartScript.maxCount);
                newAISpell->scriptType = onCombatStartScript.event;
                newAISpell->spell_type = AI_SpellType(onCombatStartScript.spell_type);
                newAISpell->fromDB = true;

                // Ready add to our List
                mCreatureAISpells.push_back(newAISpell);
            }
            else
                sLogger.debug("Tried to Register Creature AI Spell without a valid Spell Id %u", onCombatStartScript.spellId);
        }
    }

    // On AI Update
    for (auto onAIUpdateScript : onAIUpdateScripts)
    {
        if (onAIUpdateScript.action == actionSpell)
        {
            const auto spellInfo = sSpellMgr.getSpellInfo(onAIUpdateScript.spellId);
            float castChance;

            if (spellInfo != nullptr)
            {
                if (onAIUpdateScript.chance)
                    castChance = onAIUpdateScript.chance;
                else
                    castChance = ((75.0f / spellcountOnAIUpdate) * spellChanceModifierDispell[spellInfo->getDispelType()] * spellChanceModifierType[onAIUpdateScript.spell_type]);

                sLogger.debug("spell %u chance %f", onAIUpdateScript.spellId, castChance);

                uint32_t spellCooldown = Util::getRandomUInt(onAIUpdateScript.cooldownMin, onAIUpdateScript.cooldownMax);
                if (spellCooldown == 0)
                    spellCooldown = spellInfo->getSpellDefaultDuration(nullptr);

                // Create AI Spell
                CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, onAIUpdateScript.target, spellInfo->getSpellDefaultDuration(nullptr), spellCooldown, false, onAIUpdateScript.triggered);
                newAISpell->addDBEmote(onAIUpdateScript.textId);
                newAISpell->setMaxCastCount(onAIUpdateScript.maxCount);
                newAISpell->scriptType = onAIUpdateScript.event;
                newAISpell->spell_type = AI_SpellType(onAIUpdateScript.spell_type);
                newAISpell->fromDB = true;

                if (onAIUpdateScript.maxHealth)
                    newAISpell->setMinMaxPercentHp(onAIUpdateScript.minHealth, onAIUpdateScript.maxHealth);

                // Ready add to our List
                mCreatureAISpells.push_back(newAISpell);
            }
            else
                sLogger.debug("Tried to Register Creature AI Spell without a valid Spell Id %u", onAIUpdateScript.spellId);
        }
    }
}

Unit* AIInterface::getUnit() const
{
    return m_Unit;
}

Unit* AIInterface::getPetOwner() const
{
    return m_PetOwner;
}

Unit* AIInterface::getCurrentTarget() const
{
    return m_target;
}

void AIInterface::handleEvent(uint32_t event, Unit* pUnit, uint32_t misc1)
{
    if (m_Unit == nullptr)
        return;

    if (event < NUM_AI_EVENTS && AIEventHandlers[event] != NULL)
        (*this.*AIEventHandlers[event])(pUnit, misc1);
}

bool AIInterface::canUnitEvade(unsigned long time_passed)
{
    // if we dont have a Valid target go in Evade Mode
    if (!getCurrentTarget() && !getUnit()->isInEvadeMode())
    {
        m_noTargetTimer.updateTimer(time_passed);
        if (m_noTargetTimer.isTimePassed())
        {
            m_noTargetTimer.resetInterval(500);
            return true;
        }
    }

    // if we cannot reach the Target go in Evade Mode
    if (canNotReachTarget() && !getUnit()->isInEvadeMode())
    {
        m_cannotReachTimer.updateTimer(time_passed);
        if (m_cannotReachTimer.isTimePassed())
            return true;
    }

    // periodic check to see if the creature has passed an evade boundary
    if (!getUnit()->isInEvadeMode())
    {
        m_boundaryCheckTime.updateTimer(time_passed);
        if (m_boundaryCheckTime.isTimePassed())
        {
            if (checkBoundary())
            {
                m_boundaryCheckTime.resetInterval(2500);
                return false;
            }
        }
    }

    return false;
}

bool AIInterface::_enterEvadeMode()
{
    if (getUnit()->isInEvadeMode())
        return false;

    if (!getUnit()->isAlive())
    {
        handleEvent(EVENT_LEAVECOMBAT, getUnit(), 0);
        return false;
    }
    handleEvent(EVENT_LEAVECOMBAT, getUnit(), 0);
    return true;
}

void AIInterface::enterEvadeMode()
{
    if (!_enterEvadeMode())
        return;

    setNoCallAssistance(false);

    if (m_Unit->isAlive())
    {
        if (getAiScriptType() == AI_SCRIPT_PET)
        {
            if (m_Unit->isPet())
            {
                static_cast<Pet*>(m_Unit)->SetPetAction(PET_ACTION_FOLLOW);
                if (m_Unit->isAlive() && m_Unit->IsInWorld())
                {
                    static_cast<Pet*>(m_Unit)->HandleAutoCastEvent(AUTOCAST_EVENT_LEAVE_COMBAT);
                }
            }
            handleEvent(EVENT_FOLLOWOWNER, 0, 0);
        }
        else
        {
            getUnit()->addUnitStateFlag(UNIT_STATE_EVADING);
            getUnit()->getMovementManager()->moveTargetedHome();
        }
    }
}

void AIInterface::Update(unsigned long time_passed)
{
    if (m_Unit->isPlayer() || m_Unit->GetMapMgr() == nullptr)
        return;

    if (getAiState() == AI_STATE_FEAR)
        return;

    UpdateAgent(time_passed);

    if (isEngaged() || getUnit()->isInCombat())
        if(canUnitEvade(time_passed))
            enterEvadeMode();

    if (getUnit()->isCastingSpell() && !getUnit()->isInEvadeMode())
        setAiState(AI_STATE_CASTING);
    else
        setAiState(AI_STATE_IDLE);

    if (getAllowedToEnterCombat())
    {
        // Handle Different Script Types
        switch (getAiScriptType())
        {
        case AI_SCRIPT_LONER:
            updateTargets(time_passed);
            updateCombat(time_passed);
            break;
        case AI_SCRIPT_AGRO:
            updateTargets(time_passed);
            updateCombat(time_passed);
            break;
        case AI_SCRIPT_SOCIAL:
            updateTargets(time_passed);
            updateCombat(time_passed);
            break;
        case AI_SCRIPT_PET:
            updateTargets(time_passed);
            updateCombat(time_passed);
            break;
        case AI_SCRIPT_TOTEM:
            updateTargets(time_passed);
            updateTotem(time_passed);
            break;
        case AI_SCRIPT_GUARDIAN:
            updateTargets(time_passed);
            updateCombat(time_passed);
            break;
        case AI_SCRIPT_PASSIVE:
            //Nothing here
            break;
        }
    }
    else
    {
        // Wipe targets
        // Remove Combat
        if (getUnit()->isInCombat())
        {
            enterEvadeMode();
        }
    }

    // Timed Emotes
    updateEmotes(time_passed);
}

void AIInterface::UpdateAgent(unsigned long time_passed)
{
    mSpellWaitTimer.updateTimer(time_passed);

    // Update Spells
    if (getUnit()->isInCombat())
    {
        // Update Internal Spell Timers
        for (auto spells : mCreatureAISpells)
        {
            if (!getUnit()->isCastingSpell())
                spells->mCooldownTimer.updateTimer(time_passed);

            spells->mDurationTimer.updateTimer(time_passed);
        }

        UpdateAISpells();
    }

    // On AIUpdate Scripts
    for (auto itr = onAIUpdateScripts.begin(); itr != onAIUpdateScripts.end(); ++itr)
    {
        uint8_t actionId = itr->action;

        switch (actionId)
        {
        case actionPhaseChange:
            if (itr->phase > 0 || itr->phase == internalPhase)
            {
                if (float(getUnit()->getHealthPct()) <= itr->maxHealth && itr->maxCount)
                {
                    internalPhase = itr->misc1;

                    itr->maxCount = itr->maxCount - 1;

                    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(itr->textId);
                    if (npcScriptText != nullptr)
                        getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text);

                    if (npcScriptText->sound != 0)
                        getUnit()->PlaySoundToSet(npcScriptText->sound);
                }
            }
            break;
        default:
            break;
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnAIUpdate, nullptr);
}

void AIInterface::castAISpell(CreatureAISpells* aiSpell)
{
    Unit* target = getCurrentTarget();
    switch (aiSpell->mTargetType)
    {
    case TARGET_SELF:
    case TARGET_VARIOUS:
    {
        getUnit()->castSpell(getUnit(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_ATTACKING:
    {
        getUnit()->castSpell(target, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mCurrentSpellTarget = target;
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_SOURCE:
        getUnit()->castSpellLoc(getUnit()->GetPosition(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mLastCastedSpell = aiSpell;
        break;
    case TARGET_DESTINATION:
    {
        getUnit()->castSpellLoc(target->GetPosition(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mCurrentSpellTarget = target;
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_RANDOM_FRIEND:
    case TARGET_RANDOM_SINGLE:
    case TARGET_RANDOM_DESTINATION:
    {
        castSpellOnRandomTarget(aiSpell);
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_CLOSEST:
    {
        mCurrentSpellTarget = getBestUnitTarget(TargetFilter_Closest);
        mLastCastedSpell = aiSpell;
        getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
    } break;
    case TARGET_FURTHEST:
    {
        mCurrentSpellTarget = getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f);
        mLastCastedSpell = aiSpell;
        getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
    } break;
    case TARGET_CUSTOM:
    {
        if (aiSpell->getCustomTarget() != nullptr)
            getUnit()->castSpell(aiSpell->getCustomTarget(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
    } break;
    default:
        break;
    }
}

void AIInterface::castAISpell(uint32_t aiSpellId)
{
    CreatureAISpells* aiSpell = nullptr;

    // Lets find the stored Spellinfo
    for (auto spell : mCreatureAISpells)
        if (spell->mSpellInfo && spell->mSpellInfo->getId() == aiSpellId)
            aiSpell = spell;

    // when no valid Spell is found return
    if (!aiSpell)
        return;

    castAISpell(aiSpell);
}

void AIInterface::castSpellOnRandomTarget(CreatureAISpells* AiSpell)
{
    if (AiSpell == nullptr)
        return;

    // helper for following code
    bool isTargetRandFriend = (AiSpell->mTargetType == TARGET_RANDOM_FRIEND ? true : false);

    // if we already cast a spell, do not set/cast another one!
    if (!getUnit()->isCastingSpell()
        && getUnit()->GetAIInterface()->getCurrentTarget())
    {
        // set up targets in range by position, relation and hp range
        std::vector<Unit*> possibleUnitTargets;

        for (const auto& inRangeObject : getUnit()->getInRangeObjectsSet())
        {
            if (((isTargetRandFriend && isFriendly(getUnit(), inRangeObject))
                || (!isTargetRandFriend && isHostile(getUnit(), inRangeObject) && inRangeObject != getUnit())) && inRangeObject->isCreatureOrPlayer())
            {
                Unit* inRangeTarget = static_cast<Unit*>(inRangeObject);

                if (
                    inRangeTarget->isAlive() && AiSpell->isDistanceInRange(getUnit()->GetDistance2dSq(inRangeTarget))
                    && ((AiSpell->isHpInPercentRange(inRangeTarget->getHealthPct()) && isTargetRandFriend)
                        || (getUnit()->getThreatManager().getThreat(inRangeTarget) > 0 && isHostile(getUnit(), inRangeTarget))))
                {
                    possibleUnitTargets.push_back(inRangeTarget);
                }
            }
        }

        // add us as a friendly target.
        if (AiSpell->isHpInPercentRange(getUnit()->getHealthPct()) && isTargetRandFriend)
            possibleUnitTargets.push_back(getUnit());

        // no targets in our range for hp range and firendly targets
        if (possibleUnitTargets.empty())
            return;

        // get a random target
        uint32_t randomIndex = Util::getRandomUInt(0, static_cast<uint32_t>(possibleUnitTargets.size() - 1));
        Unit* randomTarget = possibleUnitTargets[randomIndex];

        if (randomTarget == nullptr)
            return;

        switch (AiSpell->mTargetType)
        {
        case TARGET_RANDOM_FRIEND:
        case TARGET_RANDOM_SINGLE:
        {
            getUnit()->castSpell(randomTarget, AiSpell->mSpellInfo, AiSpell->mIsTriggered);
            mCurrentSpellTarget = randomTarget;
        } break;
        case TARGET_RANDOM_DESTINATION:
            getUnit()->castSpellLoc(randomTarget->GetPosition(), AiSpell->mSpellInfo, AiSpell->mIsTriggered);
            break;
        }

        possibleUnitTargets.clear();
    }
}

void AIInterface::updateEmotes(unsigned long time_passed)
{
    if (!getUnit()->getThreatManager().getCurrentVictim() && isAiState(AI_STATE_IDLE) && m_Unit->isAlive())
    {
        if (timed_emote_expire <= time_passed)    // note that creature might go idle and time_passed might get big next time ...We do not skip emotes because of lost time
        {
            if ((*next_timed_emote)->type == 1)   //standstate
            {
                m_Unit->setStandState(static_cast<uint8_t>((*next_timed_emote)->value));
                m_Unit->setEmoteState(0);
            }
            else if ((*next_timed_emote)->type == 2)   //emotestate
            {
                m_Unit->setEmoteState((*next_timed_emote)->value);
                m_Unit->setStandState(STANDSTATE_STAND);
            }
            else if ((*next_timed_emote)->type == 3)   //oneshot emote
            {
                m_Unit->setEmoteState(0);
                m_Unit->setStandState(STANDSTATE_STAND);
                m_Unit->emote((EmoteType)(*next_timed_emote)->value);           // Animation
            }

            if ((*next_timed_emote)->msg)
                m_Unit->sendChatMessage((*next_timed_emote)->msg_type, (*next_timed_emote)->msg_lang, (*next_timed_emote)->msg);

            timed_emote_expire = (*next_timed_emote)->expire_after; //should we keep lost time ? I think not
            ++next_timed_emote;

            if (next_timed_emote == timed_emotes->end())
                next_timed_emote = timed_emotes->begin();
        }
        else
        {
            timed_emote_expire -= time_passed;
        }
    }
}

void AIInterface::eventAiInterfaceParamsetFinish()
{
    if (timed_emotes && timed_emotes->begin() != timed_emotes->end())
    {
        next_timed_emote = timed_emotes->begin();
        timed_emote_expire = (*next_timed_emote)->expire_after;
    }
}

void AIInterface::updateTargets(unsigned long time_passed)
{
    // Do not update target while confused or fleeing
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
        return;

    //Find Target on Threat List
    if (getUnit()->getThreatManager().getCurrentVictim())
        setCurrentTarget(getUnit()->getThreatManager().getCurrentVictim());

    //Find Target when no Threat List is available
    if (!getCurrentTarget() && !(isAiScriptType(AI_SCRIPT_PET)))
    {
        m_updateTargetsTimer.updateTimer(time_passed);
        setCurrentTarget(findTarget());
    }
    else if (!getCurrentTarget() && (isAiScriptType(AI_SCRIPT_PET) && (m_Unit->isPet() && static_cast<Pet*>(m_Unit)->GetPetState() == PET_STATE_AGGRESSIVE)))
    {
        m_updateTargetsTimer.updateTimer(time_passed);
        setCurrentTarget(findTarget());
    }

    m_updateAssistTimer.updateTimer(time_passed);

    // Find Assist Targets to assist us in our Fight
    if (m_updateAssistTimer.isTimePassed())
    {
        m_updateAssistTimer.resetInterval(1000);

        // find nearby allies
        findAssistance();

        // Clear Assist Targets
        if (m_assistTargets.size())
        {
            for (auto i = m_assistTargets.begin(); i != m_assistTargets.end();)
            {
                auto i2 = i++;
                if ((*i2) == NULL || (*i2)->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() ||
                    !(*i2)->isAlive() || m_Unit->getDistanceSq((*i2)) >= 2500.0f || !(*i2)->CombatStatus.IsInCombat() || !((*i2)->m_phase & m_Unit->m_phase))
                {
                    m_assistTargets.erase(i2);
                }
            }
        }
    }

    // set the target first
    if (getCurrentTarget())
    {
        if (getCurrentTarget()->isAlive())
        {
            if (getCurrentTarget()->GetInstanceID() == getUnit()->GetInstanceID())
            {
                getUnit()->setTargetGuid(getCurrentTarget()->getGuid());
            }
        }
        else
            enterEvadeMode();
    }
    else
        getUnit()->setTargetGuid(0);

    // When target is out of Possible Range evade.
    if ((getCurrentTarget() && !getUnit()->GetMapMgr()->GetMapInfo()->isInstanceMap()) || (getCurrentTarget() && getUnit()->GetMapId() != getCurrentTarget()->GetMapMgr()->GetMapId()))
    {
        if (getCurrentTarget()->getDistance(getUnit()->GetPosition()) > 50.0f)
            enterEvadeMode();
    }
}

// Called in TheratHandler when Target is changed
void AIInterface::updateVictim(Unit* victim)
{
    if (getAiScriptType() == AI_SCRIPT_PASSIVE)
        return;

    if (!victim)
        return;

    // Do not update target while confused or fleeing
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
        return;

    //set our new Target
    setCurrentTarget(victim);

    // set the target first
    if (getCurrentTarget())
    {
        // we dont want to get chased by a Totem
        if (getCurrentTarget()->isAlive())
        {
            if (getCurrentTarget()->GetInstanceID() == getUnit()->GetInstanceID() && !getCurrentTarget()->isTotem() && !getUnit()->isRooted())
            {
                getUnit()->setTargetGuid(getCurrentTarget()->getGuid());
                getUnit()->getMovementManager()->moveChase(getCurrentTarget());
            }
        }
        else
            enterEvadeMode();
    }
    else
        getUnit()->setTargetGuid(0);
}

void AIInterface::updateCombat(uint32_t p_time)
{
    if (getUnit()->isCreature() && static_cast<Creature*>(getUnit())->GetCreatureProperties()->Type == UNIT_TYPE_CRITTER && static_cast<Creature*>(getUnit())->GetType() != CREATURE_TYPE_GUARDIAN)
        return;

    if (getUnit()->GetMapMgr() == nullptr)
        return;

    if (getUnit()->isCastingSpell())
        return;

    if (!getCurrentTarget())
        return;

    spellEvents.updateEvents(p_time, AGENT_SPELL);

    if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_POLYMORPHED | UNIT_STATE_EVADING))
        return;

    // Do not update combat if unit is feared
    // but update if the fear is self caused by on low health fleeing
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_FLEEING) && m_fleeTimer.getExpireTime() <= 0)
        return;

    // Selects Current Agent Type For Unit
    uint32_t spellId = spellEvents.getFinishedEvent();
    selectCurrentAgent(getCurrentTarget(), spellId);

    // Handle Different Agent Types
    switch (getCurrentAgent())
    {
    case AGENT_NULL:
        // Nothing here
        break;
    case AGENT_MELEE:
    {
        setAiState(AI_STATE_IDLE);
        if (getUnit()->isWithinCombatRange(getCurrentTarget(), getUnit()->getMeleeRange(getCurrentTarget()))) // Target is in Range -> Attack
        {
            //FIX ME: offhand shit
            if (getUnit()->isAttackReady(MELEE))
            {
                setAiState(AI_STATE_ATTACKING);

                bool infront = getUnit()->isInFront(getCurrentTarget());

                if (!infront) // set InFront
                {
                    //prevent mob from rotating while stunned
                    if (!getUnit()->IsStunned())
                    {
                        getUnit()->setFacingToObject(getCurrentTarget());
                        infront = true;
                    }
                }
                if (infront)
                {
                    getUnit()->setAttackTimer(MELEE, m_Unit->getBaseAttackTime(MELEE));
                    if (getUnit()->GetOnMeleeSpell() != 0)
                    {
                        getUnit()->CastOnMeleeSpell();
                    }
                    else
                        getUnit()->Strike(getCurrentTarget(), MELEE, NULL, 0, 0, 0, false, false);
                }
            }
            /* Not Fully Supportet atm
            if (getUnit()->getVirtualItemSlotId(OFFHAND) && getUnit()->isAttackReady(OFFHAND))
            {
                getUnit()->setAttackTimer(OFFHAND, m_Unit->getBaseAttackTime(OFFHAND));
                getUnit()->Strike(getCurrentTarget(), OFFHAND, NULL, 0, 0, 0, false, false);
            }*/
        }
    }
        break;
    case AGENT_RANGED:
    {
        float combatReach[2]; // Calculate Combat Reach
        float distance = m_Unit->CalcDistance(getCurrentTarget());

        combatReach[0] = 8.0f;
        combatReach[1] = 30.0f;

        if (distance >= combatReach[0] && distance <= combatReach[1]) // Target is in Range -> Attack
        {
            if (m_Unit->isAttackReady(RANGED) && !getUnit()->isInEvadeMode())
            {
                setAiState(AI_STATE_ATTACKING);

                bool infront = m_Unit->isInFront(getCurrentTarget());

                if (!infront) // set InFront
                {
                    //prevent mob from rotating while stunned
                    if (!m_Unit->IsStunned())
                    {
                        getUnit()->setFacingToObject(getCurrentTarget());
                        infront = true;
                    }
                }

                if (infront)
                {
                    m_Unit->setAttackTimer(RANGED, m_Unit->getBaseAttackTime(RANGED));
                    SpellInfo const* info = sSpellMgr.getSpellInfo(SPELL_RANGED_GENERAL);
                    if (info)
                    {
                        Spell* sp = sSpellMgr.newSpell(m_Unit, info, false, NULL);
                        SpellCastTargets targets(getCurrentTarget()->getGuid());
                        sp->prepare(&targets);
                    }
                }
            }
        }
    }
    break;
    case AGENT_SPELL:
    {
        auto AIspell = getSpell(spellId);
        bool canCastSpell = false;

        if (AIspell->agent == AGENT_SPELL)
        {
            if (AIspell->spellType == STYPE_BUFF)
            {
                // cast the buff at requested percent only if we don't have it already
                if (Util::checkChance(AIspell->procChance))
                {
                    if (!m_Unit->HasBuff(AIspell->spell->getId()))
                    {
                        canCastSpell = true;
                    }
                }
            }
            else
            {
                // cast the spell at requested percent.
                if (Util::checkChance(AIspell->procChance))
                {
                    //focus/mana requirement
                    switch (AIspell->spell->getPowerType())
                    {
                    case POWER_TYPE_MANA:
                    {
                        if (m_Unit->getPower(POWER_TYPE_MANA) > AIspell->spell->getManaCost())
                            canCastSpell = true;
                    }
                    break;
                    case POWER_TYPE_FOCUS:
                    {
                        if (m_Unit->getPower(POWER_TYPE_FOCUS) > AIspell->spell->getManaCost())
                            canCastSpell = true;
                    }
                    break;
                    }
                }
            }
        }

        const auto maxRange = GetMaxRange(sSpellRangeStore.LookupEntry(getSpellEntry(spellId)->getRangeIndex()));
        if (canCastSpell && (maxRange == 0.0f || getUnit()->isWithinCombatRange(getCurrentTarget(), maxRange)))
        {
            SpellInfo const* spellInfo = getSpellEntry(spellId);
            auto targettype = AIspell->spelltargetType;
            SpellCastTargets targets = setSpellTargets(spellInfo, getCurrentTarget(), targettype);

            switch (targettype)
            {
                case TTYPE_CASTER:
                case TTYPE_SINGLETARGET:
                {
                    castSpell(getUnit(), spellInfo, targets);
                }
                    break;
                case TTYPE_SOURCE:
                {
                    getUnit()->castSpellLoc(targets.getSource(), spellInfo, true);
                }
                    break;
                case TTYPE_DESTINATION:
                {
                    getUnit()->castSpellLoc(targets.getDestination(), spellInfo, true);
                }
                    break;
                default:
                    sLogger.failure("AI Agents: Targettype of AI agent spell %u for creature %u not set", spellInfo->getId(), static_cast<Creature*>(getUnit())->GetCreatureProperties()->Id);
            }
        }
        uint32_t casttime = (GetCastTime(sSpellCastTimesStore.LookupEntry(AIspell->spell->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.LookupEntry(AIspell->spell->getCastingTimeIndex())) : 500);
        uint32_t cooldown = (AIspell->cooldown ? AIspell->cooldown : 500);
        // Delay all Spells by our casttime
        spellEvents.delayAllEvents(casttime, AGENT_SPELL);
        // Re add Spell to scheduler
        spellEvents.addEvent(spellId, cooldown, AGENT_SPELL);
    }
    break;
    case AGENT_FLEE:
    {
        if (getUnit()->isInEvadeMode())
            return;

        m_fleeTimer.updateTimer(p_time);
        if (m_fleeTimer.isTimePassed())
        {
            getUnit()->setControlled(false, UNIT_STATE_FLEEING);
            setCurrentAgent(AGENT_NULL);
        }

        if (m_hasFleed)
            return;

        m_fleeTimer.resetInterval(m_FleeDuration);

        CALL_SCRIPT_EVENT(m_Unit, OnFlee)(getCurrentTarget());
        getUnit()->setControlled(true, UNIT_STATE_FLEEING);

        std::string msg = "%s attempts to run away in fear!";
        getUnit()->sendChatMessage(CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, msg.c_str());

        // On Flee Scripts
        for (auto onFleeScript : onFleeScripts)
        {
            if (onFleeScript.action == actionSpell)
            {
                castAISpell(onFleeScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnFlee, nullptr);

        m_hasFleed = true;
    }
    break;
    case AGENT_CALLFORHELP:
    {
        setNoCallAssistance(true);
        callForHelp(30.0f);

        if (m_Unit->isCreature())
        {          
            // On Call for Help Scripts
            for (auto onCallForHelpScript : onCallForHelpScripts)
            {
                if (onCallForHelpScript.action == actionSpell)
                {
                    castAISpell(onCallForHelpScript.spellId);
                }
            }

            // Send a Chatmessage from our AI Scripts
            sendStoredText(mEmotesOnCallForHelp, nullptr);
        }

        CALL_SCRIPT_EVENT(m_Unit, OnCallForHelp)();
    }
    break;
    }
}

void AIInterface::doFleeToGetAssistance()
{
    if (!getCurrentTarget())
        return;

    if (getUnit()->getAuraWithAuraEffect(SPELL_AURA_PREVENTS_FLEEING))
        return;

    // maybe move to Config file
    float radius = 30.0f;
    if (radius > 0)
    {
        Creature* creature = getUnit()->GetMapMgr()->GetInterface()->getNearestAssistCreatureInGrid(getUnit()->ToCreature(), getCurrentTarget(), radius);

        setNoSearchAssistance(true);

        if (!creature)
            getUnit()->setControlled(true, UNIT_STATE_FLEEING);
        else
            getUnit()->getMovementManager()->moveSeekAssistance(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    }
}

void AIInterface::callAssistance()
{
    if (!m_AlreadyCallAssistance && getCurrentTarget() && !getUnit()->isPet() && !getUnit()->getCharmerOrOwnerGUID())
    {
        setNoCallAssistance(true);

        // maybe move to Config file
        float radius = 10.0f;

        if (radius > 0)
        {
            Creature* creature = getUnit()->GetMapMgr()->GetInterface()->getNearestAssistCreatureInGrid(getUnit()->ToCreature(), getCurrentTarget(), radius);

            if (creature)
                creature->GetAIInterface()->onHostileAction(getCurrentTarget());
        }
    }
}

void AIInterface::findAssistance()
{
    if (!getUnit()->GetMapMgr() || !getCurrentTarget())
        return;

    for (const auto& itr : getUnit()->getInRangeObjectsSet())
    {
        if (itr->isCreature())
        {
            Creature* helper = itr->ToCreature();

            float DistToMe = getUnit()->CalcDistance(helper);

            if (DistToMe <= 25.0f && helper->isInCombat() && !isAlreadyAssisting(helper)) // Also add targets if already in fight
                m_assistTargets.insert(helper);

            if (DistToMe <= 10.0f) // what should be correct also maybe differ instances/raids to normal world?
            {
                if (helper->GetAIInterface()->canAssistTo(getUnit(), getCurrentTarget(), false))
                {
                    m_assistTargets.insert(helper);
                    if (!helper->isInCombat())
                        helper->GetAIInterface()->onHostileAction(getCurrentTarget());
                }
            }
        }
    }
}

bool AIInterface::isAlreadyAssisting(Creature* helper)
{
    if (m_assistTargets.find(helper) != m_assistTargets.end())
        return true;
    
    return false;
}

void AIInterface::callForHelp(float radius)
{
    if (radius <= 0.0f || !isEngaged() || !getUnit()->isAlive() || getUnit()->isPet() || getUnit()->isCharmed())
        return;

    Unit* target = getUnit()->getThreatManager().getCurrentVictim();
    if (!target)
        target = getUnit()->getThreatManager().getAnyTarget();

    if (!target)
        return;

    // todo
}

bool AIInterface::canAssistTo(Unit* u, Unit* enemy, bool checkfaction /*= true*/)
{
    // is it true?
    if (!hasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!getUnit()->isAlive())
        return false;

    // we cannot assist in evade mode
    if (getUnit()->isInEvadeMode())
        return false;

    // or if enemy is in evade mode
    if (enemy->getObjectTypeId() == TYPEID_UNIT && enemy->ToCreature()->isInEvadeMode())
        return false;

    if (getUnit()->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE) || isImmuneToNPC())
        return false;

    // skip fighting creature
    if (isEngaged())
        return false;

    // only free creature
    if (getUnit()->getCharmerOrOwnerGUID())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getUnit()->getFactionTemplate() != u->getFactionTemplate())
            return false;
    }
    else
    {
        if (!isFriendly(getUnit(), u))
            return false;
    }

    // skip non hostile to caster enemy creatures
    if (!isHostile(getUnit(), enemy))
        return false;

    if (isFriendly(getUnit(), enemy))
        return false;

    return true;
}

void AIInterface::selectCurrentAgent(Unit* target, uint32_t spellid)
{
    // If mob is currently fleeing
    if (m_fleeTimer.getExpireTime() > 0)
    {
        setCurrentAgent(AGENT_FLEE);
        return;
    }

    if (this->isMeleeDisabled() && getCurrentAgent() == AGENT_MELEE)
        setCurrentAgent(AGENT_NULL);

    if (this->isRangedDisabled() && getCurrentAgent() == AGENT_RANGED)
        setCurrentAgent(AGENT_NULL);

    if (this->isCastDisabled() && getCurrentAgent() == AGENT_SPELL)
        setCurrentAgent(AGENT_NULL);

    if (target->isAlive() && !getUnit()->isInEvadeMode())
    {
        if (canFlee() && !m_hasFleed && ((static_cast<float>(m_Unit->getHealth()) / static_cast<float>(m_Unit->getMaxHealth()) < m_FleeHealth)))
        {
            setCurrentAgent(AGENT_FLEE);
            return;
        }
        else if (canCallForHelp() && !m_AlreadyCallAssistance && ((static_cast<float>(m_Unit->getHealth()) / static_cast<float>(m_Unit->getMaxHealth()) < m_CallForHelpHealth)))
        {
            setCurrentAgent(AGENT_CALLFORHELP);
            return;
        }

        if (spellid)
        {
            AI_Spell* m_nextSpell = getSpell(spellid);
            if (m_nextSpell->agent != AGENT_NULL)
            {
                setCurrentAgent(AI_Agent(m_nextSpell->agent));
            }
            else
            {
                setCurrentAgent(AGENT_MELEE);
            }
        }
        else
        {
            setCurrentAgent(AGENT_MELEE);
        }

        if (getCurrentAgent() == AGENT_RANGED || getCurrentAgent() == AGENT_MELEE)
        {
            if (m_canRangedAttack)
            {
                setCurrentAgent(AGENT_MELEE);
                if (target->isPlayer())
                {
                    float dist = m_Unit->getDistanceSq(target);
                    if (target->hasUnitMovementFlag(MOVEFLAG_ROOTED) || dist >= 64.0f)
                    {
                        setCurrentAgent(AGENT_RANGED);
                    }
                }
                else if (target->m_canMove == false || m_Unit->getDistanceSq(target) >= 64.0f)
                {
                    setCurrentAgent(AGENT_RANGED);
                }
            }
            else
            {
                setCurrentAgent(AGENT_MELEE);
            }
        }
    }
}

void AIInterface::addSpellToList(AI_Spell* sp)
{
    if (!sp || !sp->spell)
        return;

    AI_Spell* sp2 = new AI_Spell;
    memcpy(sp2, sp, sizeof(AI_Spell));
    m_spells.push_back(sp2);
}

void AIInterface::initializeSpells()
{
    if (m_spells.size())
    {
        for (auto itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        {
            uint32_t cooldown = (*itr)->cooldown;
            uint32_t spellid = (*itr)->spell->getId();
            spellEvents.addEvent(spellid, cooldown, AGENT_SPELL);
        }
    }
}

AI_Spell* AIInterface::getSpell(uint32_t entry)
{
    if (m_spells.size())
    {
        for (const auto& aiSpell : m_spells)
        {
            if (aiSpell->spell->getId() == entry)
                return aiSpell;
        }
    }
    return nullptr;
}

void AIInterface::setNextSpell(uint32_t spellId)
{
    if (getSpell(spellId))
        spellEvents.addEvent(spellId, 1);
}

void AIInterface::removeNextSpell(uint32_t spellId)
{
    if (getSpell(spellId))
        spellEvents.removeEvent(spellId);
}

void AIInterface::castSpell(Unit* caster, SpellInfo const* spellInfo, SpellCastTargets targets)
{
    if (spellInfo != nullptr)
    {
        if (!isAiScriptType(AI_SCRIPT_PET) && isCastDisabled())
            return;

        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "AI DEBUG: Unit %u casting spell %u on target " I64FMT " ", caster->getEntry(),
            spellInfo->getId(), targets.getUnitTarget());

        //i wonder if this will lead to a memory leak :S
        Spell* nspell = sSpellMgr.newSpell(caster, spellInfo, false, nullptr);
        nspell->prepare(&targets);
    }
    else
    {
        sLogger.failure("AIInterface::castSpell tried to cast invalid spell!");
    }
}

SpellInfo const* AIInterface::getSpellEntry(uint32_t spellId)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);

    if (!spellInfo)
    {
        sLogger.failure("WORLD: unknown spell id %i", spellId);
        return NULL;
    }

    return spellInfo;
}

SpellCastTargets AIInterface::setSpellTargets(SpellInfo const* /*spellInfo*/, Unit* target,uint8_t targettype) const
{
    SpellCastTargets targets;
    targets.setGameObjectTarget(0);
    targets.setUnitTarget(target ? target->getGuid() : 0);
    targets.setItemTarget(0);
    targets.setSource(m_Unit->GetPosition());
    targets.setDestination(m_Unit->GetPosition());
    
    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.setTargetMask(TARGET_FLAG_SOURCE_LOCATION);
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
        if (target)
        {
            targets.setDestination(target->GetPosition());
        }
    }
    else if (targettype == TTYPE_CASTER)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(m_Unit->getGuid());
    }

    return targets;
}

//function is designed to make a quick check on target to decide if we can attack it
bool AIInterface::canOwnerAttackUnit(Unit* pUnit)
{
    if (!isHostile(m_Unit, pUnit))
        return false;

    if (isFriendly(m_Unit, pUnit))
        return false;

    if (!pUnit->isAlive())
        return false;

    //do not agro units that are faking death. Should this be based on chance ?
    if (pUnit->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
        return false;

    //don't attack owner
    if (m_Unit->getCreatedByGuid() == pUnit->getGuid())
        return false;

    //make sure we do not agro flying stuff
    if (abs(pUnit->GetPositionZ() - m_Unit->GetPositionZ()) > calcCombatRange(pUnit, false))
        return false; //blizz has this set to 250 but uses pathfinding

    return true;
}

float AIInterface::calcCombatRange(Unit* target, bool ranged)
{
    if (!target)
        return 0.0f;

    float rang = 0.0f;
    if (ranged)
        rang = 5.0f;

    float selfreach = m_Unit->getCombatReach();
    float targetradius = target->GetModelHalfSize();
    float selfradius = m_Unit->GetModelHalfSize();

    float range = targetradius + selfreach + selfradius + rang;

    return range;
}

Unit* AIInterface::findTarget()
{
    // find nearest hostile Target to attack
    if (!getAllowedToEnterCombat())
        return nullptr;

    if (m_Unit->GetMapMgr() == nullptr)
        return nullptr;

    Unit* target = nullptr;

    float distance = 999999.0f; // that should do it.. :p

    //target is immune to all form of attacks, cant attack either.
    // not attackable creatures sometimes fight enemies in scripted fights though
    if (m_Unit->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE))
    {
        return nullptr;
    }

    // Should not look for new target while creature is in these states
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED | UNIT_STATE_POLYMORPHED | UNIT_STATE_EVADING))
        return nullptr;

    // Start of neutralguard snippet
    if (m_isNeutralGuard)
    {
        for (const auto& itrPlr : m_Unit->getInRangePlayersSet())
        {
            Player* tmpPlr = static_cast<Player*>(itrPlr);

            if (tmpPlr == nullptr)
                continue;

            if (tmpPlr->isDead())
                continue;

            if (tmpPlr->isOnTaxi())
                continue;

            if (tmpPlr->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                continue;

            if (tmpPlr->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT))
                continue;

            if (tmpPlr->hasAuraWithAuraEffect(SPELL_AURA_MOD_INVISIBILITY))
                continue;

            if (!tmpPlr->hasPlayerFlags(PLAYER_FLAG_PVP_GUARD_ATTACKABLE))    //PvP Guard Attackable.
                continue;

            if (!(tmpPlr->m_phase & m_Unit->m_phase))   //Not in the same phase, skip this target
                continue;

            float dist = m_Unit->getDistanceSq(tmpPlr);
            if (dist > 2500.0f)
                continue;

            if (distance > dist)
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                    bool los = mgr->isInLineOfSight(m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), tmpPlr->GetPositionX(), tmpPlr->GetPositionY(), tmpPlr->GetPositionZ());
                    if (los)
                    {
                        distance = dist;
                        target = static_cast<Unit*>(tmpPlr);
                    }
                }
                else
                {
                    distance = dist;
                    target = static_cast<Unit*>(tmpPlr);
                }
            }
        }

        if (target)
        {
            onHostileAction(target);
            return target;
        }
        distance = 999999.0f; //Reset Distance for normal check
    }
    // End of neutralguard snippet

    //we have a high chance that we will agro a player
    //this is slower then oppfaction list BUT it has a lower chance that contains invalid pointers
    for (const auto& pitr2 : m_Unit->getInRangePlayersSet())
    {
        if (pitr2)
        {
            Unit* pUnit = static_cast<Player*>(pitr2);

            if (canOwnerAttackUnit(pUnit) == false)
                continue;

            //on blizz there is no Z limit check
            float dist = m_Unit->GetDistance2dSq(pUnit);
            if (dist > distance)     // we want to find the CLOSEST target
                continue;

            if (dist <= calcAggroRange(pUnit))
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    if (m_Unit->GetMapMgr()->isInLineOfSight(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ() + 2, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ() + 2))
                    {
                        distance = dist;
                        target = pUnit;
                    }
                }
                else
                {
                    distance = dist;
                    target = pUnit;
                }
            }
        }
    }

    Unit* critterTarget = nullptr;

    //a lot less times are check inter faction mob wars :)
    if (m_updateTargetsTimer.isTimePassed())
    {
        m_updateTargetsTimer.resetInterval(TARGET_UPDATE_INTERVAL);

        for (const auto& itr2 : m_Unit->getInRangeObjectsSet())
        {
            if (itr2)
            {
                if (!itr2->isCreatureOrPlayer())
                    continue;

                Unit* pUnit = static_cast<Unit*>(itr2);

                if (canOwnerAttackUnit(pUnit) == false)
                    continue;

                //on blizz there is no Z limit check
                float dist = m_Unit->GetDistance2dSq(pUnit);

                if (pUnit->m_factionTemplate && pUnit->m_factionTemplate->Faction == 28)// only Attack a critter if there is no other Enemy in range
                {
                    if (dist < 225.0f)    // was 10
                        critterTarget = pUnit;

                    continue;
                }

                if (dist > distance)     // we want to find the CLOSEST target
                    continue;

                if (dist <= calcAggroRange(pUnit))
                {
                    if (worldConfig.terrainCollision.isCollisionEnabled)
                    {
                        if (m_Unit->GetMapMgr()->isInLineOfSight(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ() + 2, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ() + 2))
                        {
                            distance = dist;
                            target = pUnit;
                        }
                    }
                    else
                    {
                        distance = dist;
                        target = pUnit;
                    }
                }
            }
        }
    }

    if (!target)
    {
        target = critterTarget;
    }

    if (target)
    {
        onHostileAction(target);

        // Appled todo: creatures can be owners too
        if (const auto targetOwner = target->getPlayerOwner())
            onHostileAction(targetOwner);
    }
    return target;
}

float AIInterface::calcAggroRange(Unit* target)
{
    float baseAR[17] = { 19.0f, 18.5f, 18.0f, 17.5f, 17.0f, 16.5f, 16.0f, 15.5f, 15.0f, 14.5f, 12.0f, 10.5f, 8.5f, 7.5f, 6.5f, 6.5f, 5.0f };
    // Lvl Diff -8 -7 -6 -5 -4 -3 -2 -1 +0 +1 +2  +3  +4  +5  +6  +7  +8
    // Arr Pos   0  1  2  3  4  5  6  7  8  9 10  11  12  13  14  15  16
    int8_t lvlDiff = static_cast<int8_t>(target->getLevel() - m_Unit->getLevel());
    uint8_t realLvlDiff = lvlDiff;
    if (lvlDiff > 8)
    {
        lvlDiff = 8;
    }

    if (lvlDiff < -8)
    {
        lvlDiff = -8;
    }

    if (!m_Unit->canSee(target))
        return 0;

    // Retrieve aggrorange from table
    float AggroRange = baseAR[lvlDiff + 8];

    // Check to see if the target is a player mining a node
    bool isMining = false;
    if (target->isPlayer())
    {
        if (target->isCastingSpell())
        {
            // If nearby miners weren't spotted already we'll give them a little surprise.
            Spell* sp = target->getCurrentSpell(CURRENT_GENERIC_SPELL);
            if (sp != nullptr && sp->getSpellInfo()->getEffect(0) == SPELL_EFFECT_OPEN_LOCK && sp->getSpellInfo()->getEffectMiscValue(0) == LOCKTYPE_MINING)
            {
                isMining = true;
            }
        }
    }

    // If the target is of a much higher level the aggro range must be scaled down, unless the target is mining a nearby resource node
    if (realLvlDiff > 8 && !isMining)
    {
        AggroRange += AggroRange * ((lvlDiff - 8) * 5 / 100);
    }

    // Multiply by elite value
    if (m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank > 0)
    {
        AggroRange *= (static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank) * 1.50f;
    }

    // Cap Aggro range at 40.0f
    if (AggroRange > 40.0f)
    {
        AggroRange = 40.0f;
    }

    // SPELL_AURA_MOD_DETECT_RANGE
    int32_t modDetectRange = target->getDetectRangeMod(m_Unit->getGuid());
    AggroRange += modDetectRange;
    if (target->isPlayer())
    {
        AggroRange += static_cast<Player*>(target)->DetectedRange;
    }

    // Re-check if aggro range exceeds Minimum/Maximum caps
    if (AggroRange < 3.0f)
    {
        AggroRange = 3.0f;
    }

    if (AggroRange > 40.0f)
    {
        AggroRange = 40.0f;
    }

    return (AggroRange * AggroRange);
}

void AIInterface::updateTotem(uint32_t p_time)
{
    if (totemspell != nullptr)
    {
        if (p_time >= m_totemspelltimer)
        {
            Spell* pSpell = sSpellMgr.newSpell(m_Unit, totemspell, true, 0);
            Unit* nextTarget = getCurrentTarget();
            if (nextTarget == NULL ||
                (!m_Unit->GetMapMgr()->GetUnit(nextTarget->getGuid()) ||
                    !nextTarget->isAlive() ||
                    !(m_Unit->isInRange(nextTarget->GetPosition(), pSpell->getSpellInfo()->custom_base_range_or_radius_sqr)) ||
                    !isAttackable(m_Unit, nextTarget, !(pSpell->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED))
                    )
                )
            {
                //we set no target and see if we managed to fid a new one
                m_target = nullptr;
                //something happened to our target, pick another one
                SpellCastTargets targets(0);
                pSpell->GenerateTargets(&targets);
                if (targets.getTargetMask() & TARGET_FLAG_UNIT)
                    m_target = getUnit()->GetMapMgrUnit(targets.getUnitTarget());
            }
            nextTarget = getCurrentTarget();
            if (nextTarget)
            {
                SpellCastTargets targets(nextTarget->getGuid());
                pSpell->prepare(&targets);
                // need proper cooldown time!
                m_totemspelltimer = m_totemspelltime;
            }
            else
            {
                delete pSpell;
                pSpell = nullptr;
            }
        }
        else
        {
            m_totemspelltimer -= p_time;
        }
    }
    else
    {
        sLogger.failure("AIInterface::updateTotem tried to update invalid totemspell");
    }
}

void AIInterface::justEnteredCombat(Unit* pUnit)
{
    if (!getAllowedToEnterCombat())
        return;

    if (hasReactState(REACT_PASSIVE))
        return;

    if (getUnit()->isInEvadeMode() || isAiScriptType(AI_SCRIPT_PASSIVE))
        return;

    // Can this even happen can a creature attack itself?
    if ((getUnit() == pUnit))
        return;

    if ((pUnit->isPlayer() && isImmuneToPC()) || (pUnit->isCreature() && isImmuneToNPC()))
    {
        sLogger.debug("Player or Creature %s tried to enter Combat but victim has Flags Immune to PC/NPC ");
        return;
    }     

    if (isEngaged())
        return;

    m_isEngaged = true;

    // make AI group attack
    auto group = sMySQLStore.getSpawnGroupDataBySpawn(getUnit()->ToCreature()->getSpawnId());

    if (group)
    {
        for (auto members : group->spawns)
        {
            if (members.second && members.second->isAlive())
                members.second->GetAIInterface()->onHostileAction(pUnit, nullptr, false);
        }
    }

    // find assistance
    findAssistance();

    setDefaultBoundary();

    handleEvent(EVENT_ENTERCOMBAT, pUnit, 0);
}

void AIInterface::setImmuneToNPC(bool apply)
{
    if (apply)
    {
        m_Unit->setUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
    }
    else
    {
        m_Unit->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
    }
}

void AIInterface::setImmuneToPC(bool apply)
{
    if (apply)
    {
        m_Unit->setUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
    else
    {
        m_Unit->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
}

void AIInterface::onHostileAction(Unit* pUnit, SpellInfo const* spellInfo/* = nullptr*/, bool ignoreThreatRedirects/* = false*/)
{
    const auto wasEngaged = isEngaged();

    // Start combat
    justEnteredCombat(pUnit);
    // Add initial threat
    if (getUnit()->getThreatManager().canHaveThreatList())
        getUnit()->getThreatManager().addThreat(pUnit, 0.0f, spellInfo, true, ignoreThreatRedirects);

    // Send hostile action event if unit was already engaged
    // no need to send this if unit just started combat
    if (wasEngaged)
        handleEvent(EVENT_HOSTILEACTION, pUnit, 0);
}

void AIInterface::setCreatureProtoDifficulty(uint32_t entry)
{
    if (getDifficultyType() != 0)
    {
        uint32_t creature_difficulty_entry = sMySQLStore.getCreatureDifficulty(entry, getDifficultyType());
        auto properties_difficulty = sMySQLStore.getCreatureProperties(creature_difficulty_entry);
        Creature* creature = static_cast<Creature*>(m_Unit);
        if (properties_difficulty != nullptr)
        {
            if (!properties_difficulty->isTrainingDummy && !getUnit()->isVehicle())
            {
                getUnit()->GetAIInterface()->setAllowedToEnterCombat(true);
            }
            else
            {
                getUnit()->GetAIInterface()->setAllowedToEnterCombat(false);
                getUnit()->GetAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
            }

            getUnit()->setSpeedRate(TYPE_WALK, properties_difficulty->walk_speed, false);
            getUnit()->setSpeedRate(TYPE_RUN, properties_difficulty->run_speed, false);
            getUnit()->setSpeedRate(TYPE_FLY, properties_difficulty->fly_speed, false);

            getUnit()->setScale(properties_difficulty->Scale);

            uint32_t health = properties_difficulty->MinHealth + Util::getRandomUInt(properties_difficulty->MaxHealth - properties_difficulty->MinHealth);

            getUnit()->setMaxHealth(health);
            getUnit()->setHealth(health);
            getUnit()->setBaseHealth(health);

            getUnit()->setMaxPower(POWER_TYPE_MANA, properties_difficulty->Mana);
            getUnit()->setBaseMana(properties_difficulty->Mana);
            getUnit()->setPower(POWER_TYPE_MANA, properties_difficulty->Mana);

            getUnit()->setLevel(properties_difficulty->MinLevel + (Util::getRandomUInt(properties_difficulty->MaxLevel - properties_difficulty->MinLevel)));

            for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
            {
                getUnit()->setResistance(i, properties_difficulty->Resistances[i]);
            }

            getUnit()->setBaseAttackTime(MELEE, properties_difficulty->AttackTime);

            getUnit()->setMinDamage(properties_difficulty->MinDamage);
            getUnit()->setMaxDamage(properties_difficulty->MaxDamage);

            getUnit()->setBaseAttackTime(RANGED, properties_difficulty->RangedAttackTime);
            getUnit()->setMinRangedDamage(properties_difficulty->RangedMinDamage);
            getUnit()->setMaxRangedDamage(properties_difficulty->RangedMaxDamage);


            getUnit()->SetFaction(properties_difficulty->Faction);

            if (!(getUnit()->m_factionEntry->RepListId == -1 && getUnit()->m_factionTemplate->HostileMask == 0 && getUnit()->m_factionTemplate->FriendlyMask == 0))
            {
                m_Unit->GetAIInterface()->setCanCallForHelp(true);
            }

            if (properties_difficulty->CanRanged == 1)
                getUnit()->GetAIInterface()->m_canRangedAttack = true;
            else
                getUnit()->m_aiInterface->m_canRangedAttack = false;

            getUnit()->setBoundingRadius(properties_difficulty->BoundingRadius);

            getUnit()->setCombatReach(properties_difficulty->CombatReach);

            getUnit()->setNpcFlags(properties_difficulty->NPCFLags);

            // resistances
            for (uint8_t j = 0; j < TOTAL_SPELL_SCHOOLS; ++j)
            {
                getUnit()->BaseResistance[j] = getUnit()->getResistance(j);
            }

            for (uint8_t j = 0; j < STAT_COUNT; ++j)
            {
                getUnit()->BaseStats[j] = getUnit()->getStat(j);
            }

            getUnit()->BaseDamage[0] = getUnit()->getMinDamage();
            getUnit()->BaseDamage[1] = getUnit()->getMaxDamage();
            getUnit()->BaseOffhandDamage[0] = getUnit()->getMinOffhandDamage();
            getUnit()->BaseOffhandDamage[1] = getUnit()->getMaxOffhandDamage();
            getUnit()->BaseRangedDamage[0] = getUnit()->getMinRangedDamage();
            getUnit()->BaseRangedDamage[1] = getUnit()->getMaxRangedDamage();

            creature->BaseAttackType = properties_difficulty->attackSchool;

            /*  // Dont was Used in old AIInterface left the code here if needed at other Date
            if (properties_difficulty->guardtype == GUARDTYPE_CITY)
                getUnit()->GetAIInterface()->setGuard(true);
            else
                getUnit()->GetAIInterface()->setGuard(false);*/

            if (properties_difficulty->guardtype == GUARDTYPE_NEUTRAL)
                getUnit()->GetAIInterface()->setGuard(true);
            else
                getUnit()->GetAIInterface()->setGuard(false);

            //invisibility
            if (properties_difficulty->invisibility_type > INVIS_FLAG_NORMAL)
                // TODO: currently only invisibility type 15 is used for invisible trigger NPCs
                // these are always invisible to players
                getUnit()->modInvisibilityLevel(InvisibilityFlag(properties_difficulty->invisibility_type), 1);

            if (getUnit()->isVehicle())
            {
                getUnit()->addVehicleComponent(properties_difficulty->Id, properties_difficulty->vehicleid);
                getUnit()->addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
                getUnit()->setAItoUse(false);
            }

            if (properties_difficulty->rooted)
                getUnit()->setMoveRoot(true);
        }
    }
}

uint8_t AIInterface::getDifficultyType()
{
    uint8_t difficulty_type;

    Instance* instance = sInstanceMgr.GetInstanceByIds(MAX_NUM_MAPS, m_Unit->GetInstanceID());
    if (instance != nullptr)
        difficulty_type = instance->m_difficulty;
    else
        difficulty_type = 0;    // standard MODE_NORMAL / MODE_NORMAL_10MEN

    return difficulty_type;
}

void AIInterface::eventForceRedirected(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventHostileAction(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    // On hostile action, set new default boundary
    setDefaultBoundary();
}

void AIInterface::eventWander(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventUnwander(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventFear(Unit* pUnit, uint32_t /*misc1*/)
{
    if (pUnit == nullptr)
        return;

    CALL_SCRIPT_EVENT(m_Unit, OnFear)(pUnit, 0);

    getUnit()->setControlled(true, UNIT_STATE_FLEEING);

    setAiState(AI_STATE_FEAR);
}

void AIInterface::eventUnfear(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    if (getUnit()->isInEvadeMode())
        return;

    getUnit()->setControlled(false, UNIT_STATE_FLEEING);

    setAiState(AI_STATE_UNFEARED); // let future reactions put us back into combat without bugging return positions
}

void AIInterface::eventFollowOwner(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    getUnit()->getMovementManager()->clear();
    getUnit()->getMovementManager()->moveFollow(getPetOwner(), PET_FOLLOW_DIST, getUnit()->getFollowAngle());
}

void AIInterface::eventDamageTaken(Unit* pUnit, uint32_t misc1)
{
    if (getUnit()->isInEvadeMode())
        return;

    if (pUnit == nullptr)
        return;

    if (m_Unit->isCreature())
    {
        // On Damage Taken Scripts
        for (auto onDamageTakenScript : onDamageTakenScripts)
        {
            if (onDamageTakenScript.action == actionSpell)
            {
                castAISpell(onDamageTakenScript.spellId);
            } 
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnDamageTaken, pUnit);
    }

    pUnit->RemoveAura(24575);

    CALL_SCRIPT_EVENT(m_Unit, OnDamageTaken)(pUnit, misc1);
    pUnit->CombatStatus.OnDamageDealt(m_Unit);
}

void AIInterface::eventEnterCombat(Unit* pUnit, uint32_t /*misc1*/)
{
    if (pUnit == nullptr || pUnit->isDead() || m_Unit->isDead())
        return;

    /* send the message */
    if (m_Unit->isCreature())
    {
        Creature* creature = static_cast<Creature*>(m_Unit);

        CALL_SCRIPT_EVENT(m_Unit, _internalOnCombatStart)(pUnit);
        CALL_SCRIPT_EVENT(m_Unit, OnCombatStart)(pUnit);

        // set encounter state = InProgress
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(m_Unit->getEntry(), InProgress);

        if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
        {
            m_Unit->setChannelSpellId(0);
            m_Unit->setChannelObjectGuid(0);
        }

        // Enter Combat Scripts
        for (auto onCombatStartScript : onCombatStartScripts)
        {
            if (onCombatStartScript.action == actionSpell)
            {
                castAISpell(onCombatStartScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnCombatStart, pUnit);
    }

    // Stop the emote - change to fight emote
    m_Unit->setEmoteState(EMOTE_STATE_READY1H);

    // dismount if mounted
    if (m_Unit->isCreature() && !(static_cast<Creature*>(m_Unit)->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_FIGHT_MOUNTED))
        m_Unit->setMountDisplayId(0);

    // Instance Combat
    instanceCombatProgress(true);

    // Init Spells
    initializeSpells();

    // If the Player is in A Group add all Players to the Threat List
    initGroupThreat(pUnit);

    // Send attack sound
    if (getUnit()->isCreature())
        getUnit()->SendAIReaction();

    // Put mob into combat animation. Take out weapons and start to look serious :P
    m_Unit->smsg_AttackStart(pUnit);
}

void AIInterface::instanceCombatProgress(bool activate)
{
    if (getUnit()->GetMapMgr() && getUnit()->GetMapMgr()->GetMapInfo() && getUnit()->GetMapMgr()->GetMapInfo()->isRaid())
    {
        if (getUnit()->isCreature())
        {
            if (static_cast<Creature*>(getUnit())->GetCreatureProperties()->Rank == 3)
            {
                if (activate)
                    getUnit()->GetMapMgr()->AddCombatInProgress(getUnit()->getGuid());
                else
                    getUnit()->GetMapMgr()->RemoveCombatInProgress(getUnit()->getGuid());
            }
        }
    }
}

void AIInterface::initGroupThreat(Unit* target)
{
    if (target->isPlayer() && static_cast<Player*>(target)->isInGroup())
    {
        Group* pGroup = static_cast<Player*>(target)->getGroup();

        Player* pGroupGuy;
        GroupMembersSet::iterator itr;
        pGroup->Lock();
        for (uint32_t i = 0; i < pGroup->GetSubGroupCount(); i++)
        {
            for (itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
            {
                pGroupGuy = (*itr)->m_loggedInPlayer;
                if (pGroupGuy && pGroupGuy->isAlive() && m_Unit->GetMapMgr() == pGroupGuy->GetMapMgr() && pGroupGuy->getDistanceSq(target) <= 40 * 40) //50 yards for now. lets see if it works
                {
                    m_Unit->getThreatManager().addThreat(pGroupGuy, 0.0f, nullptr, true, true);
                }
            }
        }
        pGroup->Unlock();
    }
}

void AIInterface::eventLeaveCombat(Unit* pUnit, uint32_t /*misc1*/)
{
    m_isEngaged = false;
    mCreatureAISpells.clear();
    internalPhase = 0;
    spellEvents.resetEvents();
    setUnitToFollow(nullptr);
    setCannotReachTarget(false);
    setNoCallAssistance(false);
    setCurrentTarget(nullptr);
    getUnit()->setTargetGuid(0);

    if (pUnit == nullptr)
        return;

    if (pUnit->isCreature())
    {
        if (pUnit->isDead())
            pUnit->RemoveAllAuras();
        else
            pUnit->RemoveNegativeAuras();
    }

    // restart emote
    if (m_Unit->isCreature())
    {
        Creature* creature = static_cast<Creature*>(m_Unit);

        if (creature->original_emotestate)
            m_Unit->setEmoteState(creature->original_emotestate);
        else
            m_Unit->setEmoteState(EMOTE_ONESHOT_NONE);

        if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
        {
            if (creature->m_spawn->channel_target_go)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpGO, creature->m_spawn->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);

            if (creature->m_spawn->channel_target_creature)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpCreature, creature->m_spawn->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);
        }

        // Leave Combat Scripts
        for (auto onLeaveCombatScript : onLeaveCombatScripts)
        {
            if (onLeaveCombatScript.action == actionSpell)
            {
                castAISpell(onLeaveCombatScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnLeaveCombat, nullptr);
    }

    initialiseScripts(getUnit()->getEntry());

    m_Unit->CombatStatus.Vanished();
    m_Unit->getThreatManager().clearAllThreat();
    m_Unit->getThreatManager().removeMeFromThreatLists();

    CALL_SCRIPT_EVENT(m_Unit, _internalOnCombatStop)();
    CALL_SCRIPT_EVENT(m_Unit, OnCombatStop)(getUnit());

    if (m_Unit->isCreature() && m_Unit->isAlive())
    {
        // Reset Instance Data
        // set encounter state back to NotStarted
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(m_Unit->getEntry(), NotStarted);

        // Respawn all Npcs from Current Group if needed
        auto data = sMySQLStore.getSpawnGroupDataBySpawn(getUnit()->ToCreature()->getSpawnId());
        if (data && data->spawnFlags & SPAWFLAG_FLAG_FULLPACK)
        {
            for (auto spawns : data->spawns)
            {
                if (spawns.second && spawns.second->m_spawn && !spawns.second->isAlive())
                    spawns.second->Despawn(0, 1000);
            }
        }

        // Remount if mounted
        Creature* creature = static_cast<Creature*>(m_Unit);
        if (creature->m_spawn)
            m_Unit->setMountDisplayId(creature->m_spawn->MountedDisplayID);
    }

    m_hasFleed = false;
    m_fleeTimer.resetInterval(0);
    setCurrentAgent(AGENT_NULL);

    if (!m_disableDynamicBoundary)
        clearBoundary();

    // Remove Instance Combat
    instanceCombatProgress(false);

    m_Unit->smsg_AttackStop(pUnit);
}

void AIInterface::eventUnitDied(Unit* pUnit, uint32_t /*misc1*/)
{
    m_isEngaged = false;
    spellEvents.resetEvents();
    internalPhase = 0;
    mCreatureAISpells.clear();
    setCurrentTarget(nullptr);
    setUnitToFollow(nullptr);
    if (pUnit == nullptr)
        return;

    CALL_SCRIPT_EVENT(m_Unit, _internalOnDied)(pUnit);
    CALL_SCRIPT_EVENT(m_Unit, OnDied)(pUnit);

    if (m_Unit->isCreature())
    {
        // Died Scripts
        for (auto onDiedScript : onDiedScripts)
        {
            // Placeholder for further actions
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnDied, pUnit);

        initialiseScripts(getUnit()->getEntry());

        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), OnCreatureDeath)(static_cast<Creature*>(m_Unit), pUnit);

        // set encounter state to finished
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(m_Unit->getEntry(), Finished);

#if VERSION_STRING >= WotLK
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), UpdateEncountersStateForCreature)(m_Unit->getEntry(), m_Unit->GetMapMgr()->pInstance->m_difficulty);
#endif
    }

    m_hasFleed = false;
    m_fleeTimer.resetInterval(0);
    setCurrentAgent(AGENT_NULL);

    if (!m_disableDynamicBoundary)
        clearBoundary();

    setAiState(AI_STATE_IDLE);

    m_Unit->setMountDisplayId(0);

    Instance* pInstance = nullptr;
    if (m_Unit->GetMapMgr())
        pInstance = m_Unit->GetMapMgr()->pInstance;

    if (m_Unit->GetMapMgr()
        && m_Unit->isCreature()
        && !m_Unit->isPet()
        && pInstance
        && pInstance->m_mapInfo->isInstanceMap())
    {
        auto encounters = sObjectMgr.GetDungeonEncounterList(m_Unit->GetMapMgr()->GetMapId(), pInstance->m_difficulty);

        Creature* pCreature = static_cast<Creature*>(m_Unit);
        bool found = false;

        if (encounters != NULL)
        {
            uint32_t npcGuid = pCreature->GetCreatureProperties()->Id;

            for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr)
            {
                DungeonEncounter const* encounter = *itr;
                if (encounter->creditType == ENCOUNTER_CREDIT_KILL_CREATURE && encounter->creditEntry == npcGuid)
                {
                    found = true;

                    // Bosses get added via entry and not by spawnid
                    m_Unit->GetMapMgr()->pInstance->m_killedNpcs.insert(npcGuid);
                    sInstanceMgr.SaveInstanceToDB(m_Unit->GetMapMgr()->pInstance);

                    if (!pInstance->m_persistent && !pInstance->isResetable())
                    {
                        pInstance->m_persistent = true;
                        sInstanceMgr.SaveInstanceToDB(pInstance);
                        for (PlayerStorageMap::iterator pItr = m_Unit->GetMapMgr()->m_PlayerStorage.begin(); pItr != m_Unit->GetMapMgr()->m_PlayerStorage.end(); ++pItr)
                        {
                            (*pItr).second->SetPersistentInstanceId(pInstance);
                        }
                    }
                }
            }
        }

        if (found == false)
        {
            // No instance boss information ... so fallback ...
            uint32_t npcGuid = pCreature->GetSQL_id();
            m_Unit->GetMapMgr()->pInstance->m_killedNpcs.insert(npcGuid);
            sInstanceMgr.SaveInstanceToDB(m_Unit->GetMapMgr()->pInstance);
        }

        // Killed Group checks
        auto spawnGroupData = sMySQLStore.getSpawnGroupDataBySpawn(pCreature->spawnid);

        // Spawn Group Handling
        if (spawnGroupData && spawnGroupData->groupId)
        {
            bool killed = true;
            for (auto spawns : spawnGroupData->spawns)
            {
                if (m_Unit->GetMapMgr()->pInstance->m_killedNpcs.find(spawns.first) == m_Unit->GetMapMgr()->pInstance->m_killedNpcs.end())
                    killed = false;
            }

            if (killed)
                CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), OnSpawnGroupKilled)(spawnGroupData->groupId);
        }
    }
    if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo() && m_Unit->GetMapMgr()->GetMapInfo()->isRaid())
    {
        if (m_Unit->isCreature())
        {
            if (static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank == 3)
            {
                m_Unit->GetMapMgr()->RemoveCombatInProgress(m_Unit->getGuid());
            }
        }
    }
}

void AIInterface::eventOnLoad()
{
    // On Load Scripts
    if (onLoadScripts.size())
    {
        for (auto onLoadScript : onLoadScripts)
        {
            switch (onLoadScript.action)
            {
                case actionSpell:
                {
                    castAISpell(onLoadScript.spellId);
                }
                    break;
                default:
                    break;
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnLoad, nullptr);
    }
}

void AIInterface::onDeath(Object* pKiller)
{
    if (pKiller->isCreatureOrPlayer())
        handleEvent(EVENT_UNITDIED, static_cast<Unit*>(pKiller), 0);
    else
        handleEvent(EVENT_UNITDIED, m_Unit, 0);

    // Killed Scripts
    for (auto onKilledScript : onKilledScripts)
    {
        switch (onKilledScript.action)
        {
            case actionPhaseChange:
            {
                if (onKilledScript.phase > 0 || onKilledScript.phase == internalPhase)
                {
                    if (float(getUnit()->getHealthPct()) <= onKilledScript.maxHealth && onKilledScript.maxCount)
                    {
                        internalPhase = onKilledScript.misc1;

                        onKilledScript.maxCount = onKilledScript.maxCount - 1;

                        MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(onKilledScript.textId);
                        if (npcScriptText != nullptr)
                            getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text);

                        if (npcScriptText->sound != 0)
                            getUnit()->PlaySoundToSet(npcScriptText->sound);
                    }
                }
            }
                break;
            case actionSpell:
            {
                castAISpell(onKilledScript.spellId);
            }
                break;
            default:
                break;
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnTargetDied, nullptr);
}

void AIInterface::setCannotReachTarget(bool cannotReach)
{
    if (cannotReach == m_cannotReachTarget)
        return;
    m_cannotReachTarget = cannotReach;
    m_cannotReachTimer.resetInterval(5000);
}

void AIInterface::initializeReactState()
{
    if (getUnit()->isTotem() || getUnit()->isCritter() || getUnit()->isTrainingDummy())
        setReactState(REACT_PASSIVE);
    else
        setReactState(REACT_AGGRESSIVE);
}

bool AIInterface::checkBoundary()
{
    if (!isInBoundary(getUnit()->GetPosition()))
    {
        enterEvadeMode();
        return false;
    }

    return true;
}

bool AIInterface::isInBoundary(LocationVector who) const
{
    if (_boundary.empty())
        return true;

    return AIInterface::isInBounds(&_boundary, who) != _negateBoundary;
}

bool AIInterface::isInBoundary() const
{
    if (_boundary.empty())
        return true;

    return AIInterface::isInBounds(&_boundary, getUnit()->GetPosition()) != _negateBoundary;
}

/*static*/ bool AIInterface::isInBounds(CreatureBoundary const* boundary, LocationVector pos)
{
    for (AreaBoundary const* areaBoundary : *boundary)
        if (!areaBoundary->isWithinBoundary(pos))
            return false;

    return true;
}

void AIInterface::addBoundary(AreaBoundary const* boundary, bool overrideDefault/* = false*/, bool negateBoundaries /*= false*/)
{
    if (boundary == nullptr)
        return;

    if (overrideDefault && !m_disableDynamicBoundary)
    {
        // On first custom boundary, clear existing default/dynamic boundaries
        clearBoundary();
        m_disableDynamicBoundary = true;
    }

    _boundary.push_back(boundary);
    _negateBoundary = negateBoundaries;
    doImmediateBoundaryCheck();
}

void AIInterface::setDefaultBoundary()
{
    if (m_disableDynamicBoundary)
        return;

    // Do net set default boundaries to creatures in raids or dungeons
    // Mobs and bosses will chase players to instance portal unless custom boundaries are set
    if (m_Unit->GetMapMgr()->GetMapInfo()->isInstanceMap())
        return;

    if (m_Unit->isPet() || m_Unit->isSummon())
        return;

    // Clear existing boundaries
    clearBoundary();

    // Default boundary 50 yards
    addBoundary(new CircleBoundary(getUnit()->GetPosition(), 50.0f));
}

void AIInterface::clearBoundary()
{
    for (auto& boundaryItr : _boundary)
        delete boundaryItr;

    _boundary.clear();
}

void AIInterface::movementInform(uint32_t type, uint32_t id)
{
    sendStoredText(mEmotesOnRandomWaypoint, nullptr);
    CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(type, id);
}

void AIInterface::eventChangeFaction(Unit* ForceAttackersToHateThisInstead)
{
    getUnit()->getThreatManager().removeMeFromThreatLists();

    //we need a new assist list
    m_assistTargets.clear();

    //Clear targettable
    if (ForceAttackersToHateThisInstead == nullptr)
    {
        for (const auto& itr : m_Unit->getInRangeObjectsSet())
        {
            if (itr && itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->GetAIInterface())
            {
                static_cast<Unit*>(itr)->getThreatManager().clearThreat(m_Unit);
            }
        }
    }
    else
    {
        for (const auto& itr : m_Unit->getInRangeObjectsSet())
        {
            if (itr && itr->isCreatureOrPlayer())   //this guy will join me in fight since I'm telling him "sorry i was controlled"
            {
                static_cast<Unit*>(itr)->getThreatManager().addThreat(ForceAttackersToHateThisInstead, 10.0f);   //just aping to be bale to hate him in case we got nothing else
                static_cast<Unit*>(itr)->getThreatManager().clearThreat(m_Unit);
            }
        }

        getUnit()->getThreatManager().addThreat(ForceAttackersToHateThisInstead, 0.0f);
    }
}

bool AIInterface::moveTo(float x, float y, float z, float /*o = 0.0f*/, bool running/*= false*/)
{
    if (m_Unit->isRooted() || m_Unit->IsStunned())
    {
        m_Unit->stopMoving() ; //Just Stop
        return false;
    }

    MovementNew::MoveSplineInit init(m_Unit);
    init.MoveTo(x, y, z);
    if (running)
        init.SetWalk(false);
    else
        init.SetWalk(true);

    m_Unit->getMovementManager()->launchMoveSpline(std::move(init));

    return true;
}

void AIInterface::calcDestinationAndMove(Unit* target, float dist)
{
    if (m_Unit->isRooted() || m_Unit->IsStunned())
    {
        m_Unit->stopMoving(); //Just Stop
        return;
    }

    float newx, newy, newz;

    if (target != nullptr)
    {
        newx = target->GetPositionX();
        newy = target->GetPositionY();
        newz = target->GetPositionZ();

        //avoid eating bandwidth with useless movement packets when target did not move since last position
        //this will work since it turned into a common myth that when you pull mob you should not move :D
        if (abs(m_lasttargetPosition.x - newx) < MIN_WALK_DISTANCE
            && abs(m_lasttargetPosition.y - newy) < MIN_WALK_DISTANCE)
            return;

        m_lasttargetPosition.x = newx;
        m_lasttargetPosition.y = newy;

        float angle = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), newx, newy) * M_PI_FLOAT / 180.0f;
        float x = dist * cosf(angle);
        float y = dist * sinf(angle);

        if (target->isPlayer() && static_cast<Player*>(target)->isMoving())
        {
            // cater for moving player vector based on orientation
            x -= cosf(target->GetOrientation());
            y -= sinf(target->GetOrientation());
        }

        newx -= x;
        newy -= y;
    }
    else
    {
        newx = m_Unit->GetPositionX();
        newy = m_Unit->GetPositionY();
        newz = m_Unit->GetPositionZ();
    }

    MovementNew::MoveSplineInit init(m_Unit);
    init.MoveTo(newx, newy, newz);
    init.SetWalk(true);
    m_Unit->getMovementManager()->launchMoveSpline(std::move(init));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Waypoint functions
bool AIInterface::hasWayPoints()
{
    if (getUnit()->ToCreature()->getWaypointPath())
        return true;
    else
        return false;
}

uint32_t AIInterface::getCurrentWayPointId()
{
    return getUnit()->ToCreature()->getCurrentWaypointInfo().first;
}

uint32_t AIInterface::getWayPointsCount()
{
    if (hasWayPoints())
        return static_cast<uint32_t>(sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath())->nodes.size());
    else
        return 0;
}

void AIInterface::setWayPointToMove(uint32_t waypointId)
{
    auto _path = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());
    WaypointNode const &waypoint = _path->nodes[waypointId];

    MovementNew::MoveSplineInit init(getUnit());
    init.MoveTo(waypoint.x, waypoint.y, waypoint.z);

    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
    if (waypoint.orientation && waypoint.delay)
        init.SetFacing(waypoint.orientation);

    switch (waypoint.moveType)
    {
    case WAYPOINT_MOVE_TYPE_LAND:
        init.SetAnimation(UnitBytes1_AnimationFlags::UNIT_BYTE1_FLAG_GROUND);
        break;
    case WAYPOINT_MOVE_TYPE_TAKEOFF:
        init.SetAnimation(UnitBytes1_AnimationFlags::UNIT_BYTE1_FLAG_HOVER);
        break;
    case WAYPOINT_MOVE_TYPE_RUN:
        init.SetWalk(false);
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        init.SetWalk(true);
        break;
    default:
        break;
    }

    init.Launch();
}

bool AIInterface::activateShowWayPoints(Player* player, bool showBackwards)
{
    if (sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath()) == nullptr)
        return false;

    auto mWayPointMap = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());

    if (mShowWayPoints == true)
        return false;

    mShowWayPoints = true;

    for (auto wayPoint : mWayPointMap->nodes)
    {
        Creature* targetCreature = static_cast<Creature*>(getUnit());

        Creature* wpCreature = new Creature((uint64)HIGHGUID_TYPE_WAYPOINT << 32 | wayPoint.id);
        wpCreature->CreateWayPoint(wayPoint.id, player->GetMapId(), wayPoint.x, wayPoint.y, wayPoint.z, 0);
        wpCreature->SetCreatureProperties(targetCreature->GetCreatureProperties());
        wpCreature->setEntry(1);
        wpCreature->setScale(0.5f);

        const uint32_t displayId = getUnit()->getNativeDisplayId();

        wpCreature->setDisplayId(displayId);

        wpCreature->setLevel(wayPoint.id);
        wpCreature->setNpcFlags(UNIT_NPC_FLAG_NONE);
        wpCreature->SetFaction(player->getFactionTemplate());
        wpCreature->setMaxHealth(1);
        wpCreature->setHealth(1);

        ByteBuffer buf(3000);
        uint32_t count = wpCreature->buildCreateUpdateBlockForPlayer(&buf, player);
        player->getUpdateMgr().pushCreationData(&buf, count);

        wpCreature->setMoveRoot(true);

        delete wpCreature;
    }
    return true;
}

bool AIInterface::isShowWayPointsActive()
{
    return mShowWayPoints;
}

bool AIInterface::hideWayPoints(Player* player)
{
    auto mWayPointMap = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());

    if (mWayPointMap == nullptr)
        return false;

    if (mShowWayPoints != true)
        return false;

    mShowWayPoints = false;

    for (auto wayPoint : mWayPointMap->nodes)
    {
        uint64_t guid = ((uint64_t)HIGHGUID_TYPE_WAYPOINT << 32) | wayPoint.id;
        WoWGuid wowguid(guid);
        player->getUpdateMgr().pushOutOfRangeGuid(wowguid);
    }
    return true;
}

bool AIInterface::canCreatePath(float x, float y, float z)
{
    //make sure current spline is updated
    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(m_Unit->GetMapId()));
    dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(m_Unit->GetMapId(), m_Unit->GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(m_Unit->GetMapId());

    if (nav == nullptr)
        return false;

    float start[VERTEX_SIZE] = { m_Unit->GetPositionY(), m_Unit->GetPositionZ(), m_Unit->GetPositionX() };
    float end[VERTEX_SIZE] = { y, z, x };
    float extents[VERTEX_SIZE] = { 3.0f, 5.0f, 3.0f };
    float closest_point[VERTEX_SIZE] = { 0.0f, 0.0f, 0.0f };

    dtQueryFilter filter;
    filter.setIncludeFlags(NAV_GROUND | NAV_WATER | NAV_MAGMA_SLIME);

    dtPolyRef startref, endref;

    if (dtStatusFailed(nav_query->findNearestPoly(start, extents, &filter, &startref, closest_point)))
        return false;

    if (dtStatusFailed(nav_query->findNearestPoly(end, extents, &filter, &endref, closest_point)))
        return false;

    if (startref == 0 || endref == 0)
        return false;

    dtPolyRef path[256];
    int pathcount;

    if (dtStatusFailed(nav_query->findPath(startref, endref, start, end, &filter, path, &pathcount, 256)))
        return false;

    if (pathcount == 0 || path[pathcount - 1] != endref)
        return false;

    float points[MAX_PATH_LENGTH * 3];
    int32 pointcount;
    bool usedoffmesh;

    if (dtStatusFailed(findSmoothPath(start, end, path, pathcount, points, &pointcount, usedoffmesh, MAX_PATH_LENGTH, nav, nav_query, filter)))
        return false;

    return true;
}

dtStatus AIInterface::findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32 polyPathSize, float* smoothPath, int* smoothPathSize, bool & usedOffmesh, const uint32 maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter)
{
    *smoothPathSize = 0;
    uint32 nsmoothPath = 0;
    usedOffmesh = false;

    dtPolyRef polys[MAX_PATH_LENGTH];
    memcpy(polys, polyPath, sizeof(dtPolyRef)*polyPathSize);
    uint32 npolys = polyPathSize;

    float iterPos[VERTEX_SIZE], targetPos[VERTEX_SIZE];
    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[0], startPos, iterPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[npolys - 1], endPos, targetPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
    nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[VERTEX_SIZE];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = 0;

        if (!getSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, polys, npolys, steerPos, steerPosFlag, steerPosRef, query))
            break;

        bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
        bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

        // Find movement delta.
        float delta[VERTEX_SIZE];
        dtVsub(delta, steerPos, iterPos);
        float len = dtMathSqrtf(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
            len = 1.0f;
        else
            len = SMOOTH_PATH_STEP_SIZE / len;

        float moveTgt[VERTEX_SIZE];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[VERTEX_SIZE];
        const static uint32 MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32 nvisited = 0;
        query->moveAlongSurface(polys[0], iterPos, moveTgt, &filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
        npolys = fixupCorridor(polys, npolys, MAX_PATH_LENGTH, visited, nvisited);

        query->getPolyHeight(visited[nvisited - 1], result, &result[1]);
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
                nsmoothPath++;
            }
            break;
        }
        else if (offMeshConnection && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached off-mesh connection.
            usedOffmesh = true;

            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = 0;
            dtPolyRef polyRef = polys[0];
            uint32 npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = polys[npos];
                npos++;
            }

            for (uint32 i = npos; i < npolys; ++i)
            {
                polys[i - npos] = polys[i];
            }

            npolys -= npos;

            // Handle the connection.
            float startPos2[VERTEX_SIZE], endPos2[VERTEX_SIZE];
            if (!dtStatusFailed(mesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos2, endPos2)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], startPos2);
                    nsmoothPath++;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, endPos2);
                query->getPolyHeight(polys[0], iterPos, &iterPos[1]);
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
            nsmoothPath++;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely loop
    return nsmoothPath < maxSmoothPathSize ? DT_SUCCESS : DT_FAILURE;
}

bool AIInterface::getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32 pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query)
{
    // Find steer target.
    static const uint32 MAX_STEER_POINTS = 3;
    float steerPath[MAX_STEER_POINTS * VERTEX_SIZE];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32 nsteerPath = 0;
    dtStatus dtResult = query->findStraightPath(startPos, endPos, path, pathSize,
        steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32 ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) || !inRangeYZX(&steerPath[ns * VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
        {
            break;
        }

        ns++;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtVcopy(steerPos, &steerPath[ns * VERTEX_SIZE]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

uint32 AIInterface::fixupCorridor(dtPolyRef* path, const uint32 npath, const uint32 maxPath, const dtPolyRef* visited, const uint32 nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (int32 i = npath - 1; i >= 0; --i)
    {
        bool found = false;
        for (int32 j = nvisited - 1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }

        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32 req = nvisited - furthestVisited;
    uint32 orig = uint32(furthestPath + 1) < npath ? furthestPath + 1 : npath;
    uint32 size = npath - orig > 0 ? npath - orig : 0;
    if (req + size > maxPath)
        size = maxPath - req;

    if (size)
        memmove(path + req, path + orig, size * sizeof(dtPolyRef));

    // Store visited
    for (uint32 i = 0; i < req; ++i)
    {
        path[i] = visited[(nvisited - 1) - i];
    }

    return req + size;
}

void CreatureAISpells::setdurationTimer(uint32_t durationTimer)
{
    mDurationTimer.resetInterval(durationTimer);
}

void CreatureAISpells::setCooldownTimer(uint32_t cooldownTimer)
{
    mCooldownTimer.resetInterval(cooldownTimer);
}

void CreatureAISpells::addDBEmote(uint32_t textId)
{
    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
    if (npcScriptText != nullptr)
        addEmote(npcScriptText->text, npcScriptText->type, npcScriptText->sound);
    else
        sLogger.debug("A script tried to add a spell emote with %u! Id is not available in table npc_script_text.", textId);
}

void CreatureAISpells::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mAISpellEmote.push_back(AISpellEmotes(pText, pType, pSoundId));
}

void CreatureAISpells::sendRandomEmote(Unit* creatureAI)
{
    if (!mAISpellEmote.empty() && creatureAI != nullptr)
    {
        sLogger.debug("AISpellEmotes::sendRandomEmote() : called");

        uint32_t randomUInt = (mAISpellEmote.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mAISpellEmote.size() - 1)) : 0;
        creatureAI->sendChatMessage(mAISpellEmote[randomUInt].mType, LANG_UNIVERSAL, mAISpellEmote[randomUInt].mText.c_str());

        if (mAISpellEmote[randomUInt].mSoundId != 0)
            creatureAI->PlaySoundToSet(mAISpellEmote[randomUInt].mSoundId);
    }
}

void CreatureAISpells::setMaxStackCount(uint32_t stackCount)
{
    mMaxStackCount = stackCount;
}

const uint32_t CreatureAISpells::getMaxStackCount()
{
    return mMaxStackCount;
}

void CreatureAISpells::setMaxCastCount(uint32_t castCount)
{
    mMaxCount = castCount;
}

const uint32_t CreatureAISpells::getMaxCastCount()
{
    return mMaxCount;
}

const uint32_t CreatureAISpells::getCastCount()
{
    return mCastCount;
}

const bool CreatureAISpells::isDistanceInRange(float targetDistance)
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

const bool CreatureAISpells::isHpInPercentRange(float targetHp)
{
    if (targetHp >= mMinHpRangeToCast && targetHp <= mMaxHpRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxPercentHp(float minHp, float maxHp)
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

void CreatureAISpells::sendAnnouncement(Unit* pUnit)
{
    if (!mAnnouncement.empty() && pUnit != nullptr)
    {
        sLogger.debug("AISpellEmotes::sendAnnouncement() : called");

        pUnit->sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, mAnnouncement.c_str());
    }
}

void CreatureAISpells::setCustomTarget(Unit* targetCreature)
{
    mCustomTargetCreature = targetCreature;
}

Unit* CreatureAISpells::getCustomTarget()
{
    return mCustomTargetCreature;
}

CreatureAISpells* AIInterface::addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration /*= 0*/, uint32_t cooldown /*= 0*/, bool forceRemove /*= false*/, bool isTriggered /*= false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo != nullptr)
    {
        uint32_t spellDuration = duration * 1000;
        if (spellDuration == 0)
            spellDuration = spellInfo->getSpellDefaultDuration(nullptr);

        uint32_t spellCooldown = cooldown * 1000;
        if (spellCooldown == 0)
            spellCooldown = spellInfo->getSpellDefaultDuration(nullptr);

        CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, targetType, spellDuration, spellCooldown, forceRemove, isTriggered);

        mCreatureAISpells.push_back(newAISpell);

        newAISpell->setdurationTimer(spellDuration);
        newAISpell->setCooldownTimer(spellCooldown);

        return newAISpell;
    }

    sLogger.failure("tried to add invalid spell with id %u", spellId);

    return nullptr;
}

void AIInterface::UpdateAISpells()
{
    if (mLastCastedSpell)
    {
        if (!mSpellWaitTimer.isTimePassed())
        {
            // spell has a min/max range
            if (!getUnit()->isCastingSpell() && (mLastCastedSpell->mMaxPositionRangeToCast > 0.0f || mLastCastedSpell->mMinPositionRangeToCast > 0.0f))
            {
                // if we have a current target and spell is not triggered
                if (mCurrentSpellTarget != nullptr && !mLastCastedSpell->mIsTriggered)
                {
                    // interrupt spell if we are not in  required range
                    const float targetDistance = getUnit()->GetPosition().Distance2DSq({ mCurrentSpellTarget->GetPositionX(), mCurrentSpellTarget->GetPositionY() });
                    if (!mLastCastedSpell->isDistanceInRange(targetDistance))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_SCRIPT_MGR, "Target outside of spell range (%u)! Min: %f Max: %f, distance to Target: %f", mLastCastedSpell->mSpellInfo->getId(), mLastCastedSpell->mMinPositionRangeToCast, mLastCastedSpell->mMaxPositionRangeToCast, targetDistance);
                        getUnit()->interruptSpell();
                        mLastCastedSpell = nullptr;
                    }
                }
            }
        }
        else
        {
            // spell gets not interupted after casttime(duration) so we can send the emote.
            mLastCastedSpell->sendRandomEmote(getUnit());

            // spell sucessfully castet increase cast Amount
            ++mLastCastedSpell->mCastCount;

            // override attack stop timer if needed
            if (mLastCastedSpell->getAttackStopTimer() != 0)
                getUnit()->setAttackTimer(MELEE, mLastCastedSpell->getAttackStopTimer());

            mLastCastedSpell = nullptr;
        }
    }

    // cleanup exeeded spells
    for (const auto& AISpell : mCreatureAISpells)
    {
        if (AISpell != nullptr)
        {
            // stop spells and remove aura in case of duration
            if (AISpell->mDurationTimer.isTimePassed() && AISpell->mForceRemoveAura)
            {
                getUnit()->interruptSpell();
                getUnit()->RemoveAura(AISpell->mSpellInfo->getId());
            }
        }
    }

    // cast one spell and check if spell is done (duration)
    if (mSpellWaitTimer.isTimePassed())
    {
        CreatureAISpells* usedSpell = nullptr;

        float randomChance = Util::getRandomFloat(100.0f);

        // Shuffle Around our Spells to randomize the cast
        if (mCreatureAISpells.size())
        {
            for (int i = 0; i < mCreatureAISpells.size() - 1; ++i)
            {
                int j = i + rand() % (mCreatureAISpells.size() - i);
                std::swap(mCreatureAISpells[i], mCreatureAISpells[j]);
            }
        }

        for (const auto& AISpell : mCreatureAISpells)
        {
            if (AISpell != nullptr)
            {
                // not on AIUpdate skip
                if (AISpell->scriptType != onAIUpdate)
                    continue;

                // spell was casted before, check if the wait time is done
                if (!AISpell->mCooldownTimer.isTimePassed())
                    continue;

                // check if creature has Mana/Power required to cast
                if(AISpell->mSpellInfo->getPowerType() == POWER_TYPE_MANA)
                {
                    if (AISpell->mSpellInfo->getManaCost() > m_Unit->getPower(POWER_TYPE_MANA))
                        continue;
                }

                if (AISpell->mSpellInfo->getPowerType() == POWER_TYPE_FOCUS)
                {
                    if (AISpell->mSpellInfo->getManaCost() > m_Unit->getPower(POWER_TYPE_FOCUS))
                        continue;
                }

                // is bound to a specific phase (all greater than 0) if creature has a scripted AI then use its phase
                if (getUnit()->ToCreature()->GetScript())
                {
                    if (!AISpell->isAvailableForScriptPhase(getUnit()->ToCreature()->GetScript()->getScriptPhase()))
                        continue;
                }
                else
                {
                    if (!AISpell->isAvailableForScriptPhase(internalPhase))
                        continue;
                }

                // max Spell cast amount
                if (AISpell->getMaxCastCount() && AISpell->getMaxCastCount() <= AISpell->getCastCount())
                    continue;

                // aura stacking
                if (getUnit()->getAuraCountForId(AISpell->mSpellInfo->getId()) >= AISpell->getMaxStackCount())
                    continue;

                // hp range
                if (!AISpell->isHpInPercentRange(getUnit()->getHealthPct()))
                    continue;

                // no random chance (cast in script)
                if (AISpell->mCastChance == 0.0f)
                    continue;

                // do not cast any spell while stunned/feared/silenced/charmed/confused
                if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_CHARMED | UNIT_STATE_CONFUSED))
                    break;

                // random chance for shuffeld array should do the job
                if (randomChance < AISpell->mCastChance)
                {
                    usedSpell = AISpell;
                    break;
                }
                else
                {
                    // When we dont had a canche to cast then add the default 1.5sec cooldown for the next cycle
                    AISpell->setCooldownTimer(1500);
                    continue;
                }
            }
        }

        if (usedSpell != nullptr)
        {
            Unit* target = getCurrentTarget();
            switch (usedSpell->mTargetType)
            {
            case TARGET_SELF:
            case TARGET_VARIOUS:
            {
                getUnit()->castSpell(getUnit(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                mLastCastedSpell = usedSpell;
            } break;
            case TARGET_ATTACKING:
            {
                getUnit()->castSpell(target, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                mCurrentSpellTarget = target;
                mLastCastedSpell = usedSpell;
            } break;
            case TARGET_DESTINATION:
            {
                getUnit()->castSpellLoc(target->GetPosition(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                mCurrentSpellTarget = target;
                mLastCastedSpell = usedSpell;
            } break;
            case TARGET_RANDOM_FRIEND:
            case TARGET_RANDOM_SINGLE:
            case TARGET_RANDOM_DESTINATION:
            {
                castSpellOnRandomTarget(usedSpell);
                mLastCastedSpell = usedSpell;
            } break;
            case TARGET_CUSTOM:
            {
                // nos custom target set, no spell cast.
                if (usedSpell->getCustomTarget() != nullptr)
                    getUnit()->castSpell(usedSpell->getCustomTarget(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
            } break;
            default:
                break;
            }

            // send announcements on casttime beginn
            usedSpell->sendAnnouncement(getUnit());

            uint32_t casttime = (GetCastTime(sSpellCastTimesStore.LookupEntry(usedSpell->mSpellInfo->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.LookupEntry(usedSpell->mSpellInfo->getCastingTimeIndex())) : 500);

            // reset cast wait timer
            mSpellWaitTimer.resetInterval(casttime);

            // reset spell timers to cleanup exceeded spells
            usedSpell->mDurationTimer.resetInterval(usedSpell->mDuration);
            usedSpell->mCooldownTimer.resetInterval(usedSpell->mCooldown);
        }
    }
}

Unit* AIInterface::getBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    for (const auto& PlayerIter : getUnit()->getInRangePlayersSet())
    {
        if (PlayerIter && isValidUnitTarget(PlayerIter, pTargetFilter, pMinRange, pMaxRange))
            TargetArray.push_back(static_cast<Unit*>(PlayerIter));
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* AIInterface::getBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (const auto& ObjectIter : getUnit()->getInRangeObjectsSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }

        if (isValidUnitTarget(getUnit(), pTargetFilter))
            TargetArray.push_back(getUnit());    //add self as possible friendly target
    }
    else
    {
        for (const auto& ObjectIter : getUnit()->getInRangeOppositeFactionSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* AIInterface::getBestTargetInArray(UnitArray & pTargetArray, TargetFilter pTargetFilter)
{
    //only one possible target, return it
    if (pTargetArray.size() == 1)
        return pTargetArray[0];

    //closest unit if requested
    if (pTargetFilter & TargetFilter_Closest)
        return getNearestTargetInArray(pTargetArray);

    //second most hated if requested
    if (pTargetFilter & TargetFilter_SecondMostHated)
        return getSecondMostHatedTargetInArray(pTargetArray);

    //random unit in array
    return (pTargetArray.size() > 1) ? pTargetArray[Util::getRandomUInt((uint32_t)pTargetArray.size() - 1)] : nullptr;
}

Unit* AIInterface::getNearestTargetInArray(UnitArray& pTargetArray)
{
    Unit* NearestUnit = nullptr;

    float Distance, NearestDistance = 99999;
    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            Distance = getUnit()->CalcDistance(static_cast<Unit*>(UnitIter));
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestUnit = UnitIter;
            }
        }
    }

    return NearestUnit;
}

Unit* AIInterface::getSecondMostHatedTargetInArray(UnitArray & pTargetArray)
{
    Unit* MostHatedUnit = nullptr;
    Unit* TargetUnit = nullptr;
    Unit* CurrentTarget = static_cast<Unit*>(getCurrentTarget());
    uint32_t Threat = 0;
    uint32_t HighestThreat = 0;

    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            TargetUnit = static_cast<Unit*>(UnitIter);
            if (TargetUnit != CurrentTarget)
            {
                Threat = static_cast<uint32_t>(getUnit()->getThreatManager().getThreat(TargetUnit));
                if (Threat > HighestThreat)
                {
                    MostHatedUnit = TargetUnit;
                    HighestThreat = Threat;
                }
            }
        }
    }

    return MostHatedUnit;
}

bool AIInterface::isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange)
{
    if (!pObject->isCreatureOrPlayer())
        return false;

    if (pObject->GetInstanceID() != getUnit()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->isCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->isPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;

    if (UnitTarget->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
        return false;

    // if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        // units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getUnit()->getThreatManager().getThreat(UnitTarget) == 0)
            return false;

        // current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCurrentTarget())
            return false;

        // only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->getHealthPct() >= 99)
            return false;

        // targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && !getUnit()->isWithinCombatRange(UnitTarget, getUnit()->getMeleeRange(UnitTarget)))
            return false;

        // targets not in strict range if requested
        if ((pFilter & TargetFilter_InRangeOnly) && (pMinRange > 0 || pMaxRange > 0))
        {
            float Range = getUnit()->CalcDistance(UnitTarget);
            if (pMinRange > 0 && Range < pMinRange)
                return false;

            if (pMaxRange > 0 && Range > pMaxRange)
                return false;
        }

        // targets not in Line Of Sight if requested
        if ((~pFilter & TargetFilter_IgnoreLineOfSight) && !getUnit()->IsWithinLOSInMap(UnitTarget))
            return false;

        // hostile/friendly
        if ((~pFilter & TargetFilter_Corpse) && (pFilter & TargetFilter_Friendly))
        {
            if (!UnitTarget->CombatStatus.IsInCombat())
                return false; // not-in-combat targets if friendly

            if (isHostile(getUnit(), UnitTarget) || getUnit()->getThreatManager().getThreat(UnitTarget) > 0)
                return false;
        }

        if ((pFilter & TargetFilter_Current) && UnitTarget != getCurrentTarget())
            return false;
    }

    return true;
}

void AIInterface::sendStoredText(definedEmoteVector store, Unit* target)
{
    float randomChance = Util::getRandomFloat(100.0f);

    // Shuffle Around our textIds to randomize it
    if (store.size())
    {
        for (int i = 0; i < store.size() - 1; ++i)
        {
            int j = i + rand() % (store.size() - i);
            std::swap(store[i], store[j]);
        }

        for (auto mEmotes : store)
        {
            if (mEmotes.phase && mEmotes.phase != internalPhase)
                continue;

            if (mEmotes.healthPrecent && float(getUnit()->getHealthPct()) < mEmotes.healthPrecent)
                continue;

            if (mEmotes.maxCount && mEmotes.count == mEmotes.maxCount)
                continue;

            if (randomChance < mEmotes.canche)
            {
                MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(mEmotes.textId);
                if (npcScriptText != nullptr)
                    getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text, target, 0);

                if (npcScriptText->sound != 0)
                    getUnit()->PlaySoundToSet(npcScriptText->sound);

                ++mEmotes.count;

                break;
            }
        }
    }
}