/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.h"
#include "SpellAuras.h"
#include "SpellTarget.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/AuraStates.h"
#include "Definitions/CastInterruptFlags.h"
#include "Definitions/ChannelInterruptFlags.h"
#include "Definitions/DispelType.h"
#include "Definitions/LockTypes.h"
#include "Definitions/PreventionType.h"
#include "Definitions/ProcFlags.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Definitions/SpellDamageType.h"
#include "Definitions/SpellDidHitResult.h"
#include "Definitions/SpellEffectTarget.h"
#include "Definitions/SpellFamily.h"
#include "Definitions/SpellInFrontStatus.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/SpellPacketFlags.h"
#include "Definitions/SpellState.h"
#include "Definitions/SpellRanged.h"

#include "Data/Flags.hpp"
#include "Management/Battleground/Battleground.h"
#include "Management/ItemInterface.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Objects/DynamicObject.h"
#include "Objects/Faction.h"
#include "Objects/GameObject.h"
#include "Objects/ObjectMgr.h"
#include "Server/Definitions.h"
#include "Server/Packets/SmsgCancelCombat.h"
#include "Server/Packets/MsgChannelUpdate.h"
#include "Server/Packets/MsgChannelStart.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/CreatureDefines.hpp"
#include "Units/Creatures/Pet.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Units/UnitDefines.hpp"
#include "Util.hpp"

using namespace AscEmu::Packets;

extern pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS];

//////////////////////////////////////////////////////////////////////////////////////////
// Main control flow
SpellCastResult Spell::prepare(SpellCastTargets* targets)
{
    if (!m_caster->IsInWorld())
    {
        LogDebugFlag(LF_SPELL, "Object " I64FMT " is casting spell ID %u while not in world", m_caster->getGuid(), getSpellInfo()->getId());
        delete this;
        return SPELL_FAILED_DONT_REPORT;
    }

    //\ todo: handle this in creature AI...
    if (u_caster != nullptr && u_caster->isCreature())
    {
        const auto aiInterface = u_caster->GetAIInterface();
        if (aiInterface->isAiState(AI_STATE_FEAR) || aiInterface->isAiState(AI_STATE_WANDER))
        {
            u_caster->addGarbageSpell(this);
            return SPELL_FAILED_NOT_READY;
        }
    }

    if (p_caster != nullptr)
    {
        // Call Lua script hook
        if (!sHookInterface.OnCastSpell(p_caster, getSpellInfo(), this))
        {
            p_caster->addGarbageSpell(this);
            return SPELL_FAILED_UNKNOWN;
        }

        //\ todo: convert this hack to spell script
        if (p_caster->cannibalize)
        {
            sEventMgr.RemoveEvents(p_caster, EVENT_CANNIBALIZE);
            p_caster->setEmoteState(EMOTE_ONESHOT_NONE);
            p_caster->cannibalize = false;
        }
    }

    // Check if spell is disabled
    if (sObjectMgr.IsSpellDisabled(getSpellInfo()->getId()))
    {
        sendCastResult(m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_SPELL_UNAVAILABLE);
        finish(false);
        return m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    // Check if caster is casting another spell
    if (!m_triggeredSpell && m_caster->isCastingSpell(true, true))
    {
        sendCastResult(SPELL_FAILED_SPELL_IN_PROGRESS);
        finish(false);
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    }

    m_spellState = SPELL_STATE_PREPARING;
    m_targets = *targets;

    // Set casting position
    m_castPositionX = m_caster->GetPositionX();
    m_castPositionY = m_caster->GetPositionY();
    m_castPositionZ = m_caster->GetPositionZ();
    m_castPositionO = m_caster->GetOrientation();

    // Initialize spell cast time
    m_castTime = 0;
    if (!(m_triggeredByAura != nullptr || m_triggeredSpell && (getSpellInfo()->getManaCost() > 0 || getSpellInfo()->getManaCostPercentage() > 0)))
    {
        m_castTime = GetCastTime(sSpellCastTimesStore.LookupEntry(getSpellInfo()->getCastingTimeIndex()));
        if (m_castTime > 0 && u_caster != nullptr)
        {
            // Apply cast time modifiers
            u_caster->applySpellModifiers(SPELLMOD_CAST_TIME, &m_castTime, getSpellInfo(), this);

            // Apply haste modifier to non-tradeskill spells
            if (!(getSpellInfo()->getAttributes() & (ATTRIBUTES_ABILITY | ATTRIBUTES_TRADESPELL)))
            {
                m_castTime = static_cast<int32_t>(m_castTime * u_caster->getModCastSpeed());
            }
            // Apply ranged haste modifier to ranged spells with cast time
            else if (getSpellInfo()->getAttributes() & ATTRIBUTES_RANGED && !getSpellInfo()->isRangedAutoRepeat())
            {
                m_castTime = static_cast<int32_t>(m_castTime * u_caster->getAttackSpeedModifier(RANGED));
            }

            // Instant cast if target is in a non-traded trade slot
            if (p_caster != nullptr && p_caster->getTradeData() != nullptr)
            {
                const auto tradeItem = p_caster->getTradeData()->getTargetTradeData()->getTradeItem(TRADE_SLOT_NONTRADED);
                if (tradeItem != nullptr && tradeItem->getGuid() == m_targets.getItemTarget())
                    m_castTime = 0;
            }
        }
    }

    // Check for cast time cheat
    if (m_castTime < 0 || (p_caster != nullptr && p_caster->m_cheats.hasCastTimeCheat))
        m_castTime = 0;

    // Initialize power cost
    // Spells casted from items should not use any power
    m_powerCost = i_caster != nullptr ? 0 : calculatePowerCost();

    // Check if spell can be casted
    uint32_t parameter1 = 0, parameter2 = 0;
    cancastresult = canCast(false, &parameter1, &parameter2);
    if (cancastresult != SPELL_CAST_SUCCESS)
    {
        // Triggered spells also need to go through cancast check but they do not pop a error message
        sendCastResult(m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : cancastresult, parameter1, parameter2);
        // Also need to send SMSG_SPELL_FAILED_OTHER, otherwise spell button gets stuck
        SendInterrupted(0);

        if (m_triggeredByAura != nullptr)
        {
            sendChannelUpdate(0);
            if (u_caster != nullptr)
                u_caster->RemoveAura(m_triggeredByAura);
        }

        LogDebugFlag(LF_SPELL, "Spell::prepare : canCast result %u for spell id %u (refer to SpellFailure.h to work out why)", cancastresult, getSpellInfo()->getId());

        finish(false);
        return cancastresult;
    }

    m_timer = m_castTime;

    if (!m_triggeredSpell || getSpellInfo()->isChanneled())
        m_caster->setCurrentSpell(this);

    if (!m_triggeredSpell)
    {
        if (m_timer > 0)
        {
            // Remove stealth here only if spell has cast time
            // Stealth is removed for instant spells in ::cast
            if (p_caster != nullptr)
            {
                if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
                    p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

                // Remove Feign Death auras
                p_caster->removeAllAurasByAuraEffect(SPELL_AURA_FEIGN_DEATH);
            }

            // Call for scripted at start casting hook
            sScriptMgr.callScriptedSpellAtStartCasting(this);
        }

        // Send cast bar
        sendSpellStart();

        // Send global cooldown
        if (p_caster != nullptr && !p_caster->m_cheats.hasCastTimeCheat)
            p_caster->addGlobalCooldown(getSpellInfo(), this);

        // Handle instant and non-channeled spells instantly. Other spells will be handled in ::update on next tick.
        // First autorepeat casts are actually never casted, only set as current spell. Player::updateAutoRepeatSpell handles the shooting.
        if (m_castTime == 0 && !getSpellInfo()->isChanneled() && !getSpellInfo()->isRangedAutoRepeat())
            castMe(false);
    }
    else
    {
        castMe(false);
    }

    // TODO: for future reference, I removed aurastate removal here if spell had any aurastate set in SpellInfo::CasterAuraState
    // this is not handled anywhere yet -Appled

    return cancastresult;
}

void Spell::castMe(const bool doReCheck)
{
    if (DuelSpellNoMoreValid())
    {
        sendCastResult(m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_INTERRUPTED);
        SendInterrupted(0);
        finish(false);
        return;
    }

    // Debug logging
    if (m_caster->isPlayer())
    {
        const auto plr = static_cast<Player*>(m_caster);
        LogDebugFlag(LF_SPELL, "Spell::castMe : Player guid %u casted spell %s (id %u)",
            plr->getGuidLow(), getSpellInfo()->getName().c_str(), getSpellInfo()->getId());
    }
    else if (m_caster->isCreature())
    {
        const auto creature = static_cast<Creature*>(m_caster);
        LogDebugFlag(LF_SPELL, "Spell::castMe : Creature guid %u (entry %u) casted spell %s (id %u)",
            creature->spawnid, creature->getEntry(), getSpellInfo()->getName().c_str(), getSpellInfo()->getId());
    }
    else
    {
        LogDebugFlag(LF_SPELL, "Spell::castMe : Spell id %u casted, caster guid %u", getSpellInfo()->getId(), m_caster->getGuid());
    }

    // Check cast again if spell had cast time
    if (doReCheck)
    {
        // Recalculate power cost in case caster gained a mana reduction buff while casting (blizzlike)
        m_powerCost = i_caster != nullptr ? 0 : calculatePowerCost();

        uint32_t parameter1 = 0, parameter2 = 0;
        cancastresult = canCast(true, &parameter1, &parameter2);
        if (cancastresult != SPELL_CAST_SUCCESS)
        {
            sendCastResult(cancastresult, parameter1, parameter2);
            SendInterrupted(0);
            finish(false);
            return;
        }
    }

    m_isCasting = true;

    // Remove stealth if spell is an instant cast
    if (!m_triggeredSpell && m_castTime == 0 && p_caster != nullptr)
    {
        if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
            p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        // Remove Feign Death auras
        p_caster->removeAllAurasByAuraEffect(SPELL_AURA_FEIGN_DEATH);
    }

    // Initialize targets
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        const auto requiredTargetMask = getSpellInfo()->getRequiredTargetMaskForEffect(i);
        if (requiredTargetMask & SPELL_TARGET_AREA_CURTARGET)
        {
            // If target type is area around target, set destination correctly
            const auto targetObj = m_caster->GetMapMgrObject(m_targets.getUnitTarget());
            if (targetObj != nullptr)
            {
                m_targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
                m_targets.setDestination(targetObj->GetPosition());
            }
        }

        if (getSpellInfo()->getEffect(i) != 0 && getSpellInfo()->getEffect(i) != SPELL_EFFECT_PERSISTENT_AREA_AURA)
            FillTargetMap(i);

        // Call for scripted filter target hook
        sScriptMgr.callScriptedSpellFilterTargets(this, i, &m_effectTargets[i]);
    }

    // Check for magnet target (Grounding Totem)
    //\ todo: move this to ::handleeffects or something
    if (m_magnetTarget != 0)
    {
        // Spell was redirected
        // Grounding Totem gets destroyed after redirecting 1 spell
        const auto magnetTarget = m_caster->GetMapMgrUnit(m_magnetTarget);
        if (magnetTarget != nullptr && magnetTarget->isCreature())
        {
            const auto creatureMagnet = static_cast<Creature*>(magnetTarget);
            if (creatureMagnet->isTotem())
                creatureMagnet->Despawn(1, 0);
        }
        m_magnetTarget = 0;
    }

    // Send cooldown
    if (p_caster != nullptr && !(getSpellInfo()->getAttributes() & (ATTRIBUTES_PASSIVE | ATTRIBUTES_TRIGGER_COOLDOWN)))
    {
        // Ranged shoot spells (throw, wand shoot etc) don't have cooldowns set in DBC
        // The cooldown is the speed of equipped ranged weapon
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_PLAYER_RANGED_SPELLS)
        {
            const auto cooldown = static_cast<int32_t>(p_caster->getBaseAttackTime(RANGED));
            p_caster->addSpellCooldown(getSpellInfo(), i_caster, this, cooldown);
        }
        else
        {
            p_caster->addSpellCooldown(getSpellInfo(), i_caster, this);
        }
    }

    // Take power
    if (getSpellInfo()->isOnNextMeleeAttack())
    {
        if (m_triggeredSpell)
        {
            // When on next melee spell is actually casted it will be a triggered spell
            // Need to check for power here (canCast skips power check for triggered spells)
            const auto powerResult = checkPower();
            if (powerResult != SPELL_CAST_SUCCESS)
            {
                // Normally error messages are not sent for triggered spells but this is an exception
                sendCastResult(powerResult);
                SendInterrupted(0);
                finish(false);
                return;
            }

            takePower();
        }
    }
    else
    {
        takePower();
    }

    // Remove reagents before handling effects so crafted item can be put in same slot
    removeReagents();
#if VERSION_STRING < Cata
    removeAmmo();
#endif

    // TODO: REMOVE ME; hackfixes from legacy Spell::castMe()
    castMeOld();

    // Activate on next melee spell
    // Spell is casted on next melee spell as a triggered spell
    if (getSpellInfo()->isOnNextMeleeAttack() && !m_triggeredSpell)
    {
        m_isCasting = false;
        if (u_caster != nullptr)
        {
            if (m_triggeredByAura == nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
            {
                // we're much better to remove this here, because otherwise spells that change powers etc,
                // don't get applied.
                u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
                u_caster->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_CAST);
            }

            u_caster->SetOnMeleeSpell(getSpellInfo()->getId(), extra_cast_number);
        }

        finish();
        return;
    }

    // Fill unique hitted targets
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffect(i) == 0)
            continue;

        for (const auto& targetGuid : m_effectTargets[i])
        {
            if (targetGuid == 0)
                continue;

            // Add only if target is not already stored in the vector
            auto add = true;
            for (const auto& uniqueTarget : uniqueHittedTargets)
            {
                if (uniqueTarget.first == targetGuid)
                {
                    add = false;
                    break;
                }
            }

            if (add)
                uniqueHittedTargets.push_back(std::make_pair(targetGuid, DamageInfo()));
        }
    }

    // Cleanup missed targets; spell either hits or misses target, not both
    // Current spell target system is bullshit
    if (!missedTargets.empty())
    {
        for (const auto& targetGuid : uniqueHittedTargets)
        {
            auto missedTarget = missedTargets.begin();
            while (missedTarget != missedTargets.end())
            {
                if (missedTarget->targetGuid == targetGuid.first)
                    missedTarget = missedTargets.erase(missedTarget);
                else
                    ++missedTarget;
            }
        }
    }

    // Send spell missile/visual effect
    sendSpellGo();

    uint32_t channelDuration = 0;
    if (getSpellInfo()->isChanneled())
    {
        channelDuration = GetDuration();
        // Apply haste modifer to channel duration
        if (u_caster != nullptr)
            channelDuration = static_cast<uint32_t>(channelDuration * u_caster->getModCastSpeed());

        if (channelDuration > 0)
        {
            m_spellState = SPELL_STATE_CHANNELING;
            sendChannelStart(channelDuration);
        }
    }

    // Take cast item after SMSG_SPELL_GO but before effect handling
    if (!GetSpellFailed())
        RemoveItems();

#if VERSION_STRING < Cata
    /*
    Five Second Rule
    After a character expends mana in casting a spell, the effective amount of mana gained per tick from spirit-based regeneration becomes a ratio of the normal
    listed above, for a period of 5 seconds. During this period mana regeneration is said to be interrupted. This is commonly referred to as the five second rule.
    By default, your interrupted mana regeneration ratio is 0%, meaning that spirit-based mana regeneration is suspended for 5 seconds after casting.

    Channeled spells are handled a little differently. The five second rule starts when the spell's channeling starts; i.e. when you pay the mana for it.
    The rule continues for at least five seconds, and longer if the spell is channeled for more than five seconds. For example,
    Mind Flay channels for 3 seconds and interrupts your regeneration for 5 seconds, while Tranquility channels for 10 seconds
    and interrupts your regeneration for the full 10 seconds.
    */
    if (m_usesMana && u_caster != nullptr)
        u_caster->interruptPowerRegeneration(std::max(channelDuration, 5000u));
#endif

    // we're much better to remove this here, because otherwise spells that change powers etc,
    // don't get applied.
    if (u_caster != nullptr && !m_triggeredSpell && m_triggeredByAura == nullptr && !(m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
    {
        u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
        u_caster->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_CAST);
    }

    // Prepare proc flags for caster and targets
    _prepareProcFlags();
    m_casterDamageInfo.schoolMask = SchoolMask(getSpellInfo()->getSchoolMask());

    // Loop through spell effects and process the spell effect on each target
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        const auto spellEffect = getSpellInfo()->getEffect(i);
        if (spellEffect == 0)
            continue;

        // Call for scripted before hit hook
        sScriptMgr.callScriptedSpellBeforeHit(this, i);

        if (!m_effectTargets[i].empty())
        {
            for (const auto& targetGuid : m_effectTargets[i])
            {
                handleHittedTarget(targetGuid, i);
            }
        }
        else
        {
            handleHittedTarget(0, i);
        }
    }

    // If spell applies an aura, handle it to targets after all effects have been processed
    if (!m_pendingAuras.empty())
    {
        for (auto itr = m_pendingAuras.begin(); itr != m_pendingAuras.end();)
        {
            const auto pendingAur = *itr;
            // Handle only instant auras here
            if (pendingAur.second.travelTime > 0)
            {
                ++itr;
                continue;
            }

            HandleAddAura(pendingAur.first);
            itr = m_pendingAuras.erase(itr);
        }
    }

    // Handle targets who did not get hit by this spell (miss/resist etc)
    auto targetMissed = false, targetDodged = false, targetParried = false;
    for (const auto& missedTarget : missedTargets)
    {
        handleMissedTarget(missedTarget);

        // Check if any target missed, dodged or parried the spell
        if (missedTarget.hitResult == SPELL_DID_HIT_MISS)
            targetMissed = true;
        else if (missedTarget.hitResult == SPELL_DID_HIT_DODGE)
            targetDodged = true;
        else if (missedTarget.hitResult == SPELL_DID_HIT_PARRY)
            targetParried = true;
    }

    // Handle used spell modifiers
    takeUsedSpellModifiers();

    if (u_caster != nullptr)
    {
        // Reset attack timer
        if (!m_triggeredSpell && getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_AUTOATTACK && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS))
        {
            u_caster->setAttackTimer(MELEE, u_caster->getBaseAttackTime(MELEE));
            //\ todo: fix this for creatures
            if (p_caster != nullptr && p_caster->hasOffHandWeapon())
                p_caster->setAttackTimer(OFFHAND, p_caster->getBaseAttackTime(OFFHAND));
        }

        if (p_caster != nullptr)
        {
            // Druids and rogues are refunded for 82% of the energy cost on miss, dodge or parry
            if (getSpellInfo()->getPowerType() == POWER_TYPE_ENERGY && getSpellInfo()->hasEffect(SPELL_EFFECT_ADD_COMBO_POINTS) &&
                (targetMissed || targetDodged || targetParried))
            {
                const auto refundCost = float2int32(getPowerCost() * 0.82f);
                p_caster->modPower(POWER_TYPE_ENERGY, refundCost);
            }
            // Druids and warriors are refunded for 82% of the rage cost on dodge or parry
            else if (getSpellInfo()->getPowerType() == POWER_TYPE_RAGE &&
                (targetDodged || targetParried))
            {
                const auto refundCost = float2int32(getPowerCost() * 0.82f);
                p_caster->modPower(POWER_TYPE_RAGE, refundCost);
            }

            // This is wrong but leaving this here commented out for now -Appled
            // This needs to be handled somewhere
            //Target->removeAuraStateAndAuras(static_cast<AuraState>(getSpellInfo()->getTargetAuraState()));
        }
    }

    // If spell is not channeled, the spell cast has finished successfully and the spell is traveling
    // Spells without travel time are also finished on next update tick
    if (m_spellState != SPELL_STATE_CHANNELING)
    {
        m_spellState = SPELL_STATE_TRAVELING;
        m_caster->addTravelingSpell(this);
    }

    m_isCasting = false;
}

void Spell::handleHittedTarget(const uint64_t targetGuid, uint8_t effIndex)
{
    const auto travelTime = _getSpellTravelTimeForTarget(targetGuid);
    if (travelTime < 0)
        return;

    _updateTargetPointers(targetGuid);
    const auto effDamage = calculateEffect(effIndex);

    // If effect applies an aura, create it instantly but add it later to target
    if (getSpellInfo()->doesEffectApplyAura(effIndex))
    {
        handleHittedEffect(targetGuid, effIndex, effDamage);

        // Add travel time to aura
        auto itr = m_pendingAuras.find(targetGuid);
        if (itr != m_pendingAuras.end())
            itr->second.travelTime = float2int32(travelTime);

        return;
    }

    if (travelTime == 0.0f)
    {
        handleHittedEffect(targetGuid, effIndex, effDamage);
    }
    else
    {
        HitSpellEffect hitEffect;
        hitEffect.damage = effDamage;
        hitEffect.effIndex = effIndex;
        hitEffect.travelTime = float2int32(travelTime);

        m_hitEffects.insert(std::make_pair(targetGuid, hitEffect));
    }
}

void Spell::handleHittedEffect(const uint64_t targetGuid, uint8_t effIndex, int32_t effDamage, bool reCheckTarget/* = false*/)
{
    // If duel has ended before spell cast was finished, do not handle this target and effect
    // but do not cancel entire spell
    // i.e AoE spells can still hit other targets
    if (DuelSpellNoMoreValid())
        return;

    if (reCheckTarget)
        _updateTargetPointers(targetGuid);

    const auto targetType = getSpellInfo()->getRequiredTargetMaskForEffect(effIndex);
    // TODO: in the future, consider having two damage variables; one for integer and one for float
    damage = effDamage;

    // todo: this is not how it should be done
    if (getUnitCaster() != nullptr && GetUnitTarget() != nullptr && GetUnitTarget()->isCreature()
        && targetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
    {
        GetUnitTarget()->GetAIInterface()->AttackReaction(getUnitCaster(), 1, 0);
        GetUnitTarget()->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, getUnitCaster(), 0);
    }

    // Clear DamageInfo before effect
    m_targetDamageInfo = DamageInfo();
    m_targetDamageInfo.schoolMask = SchoolMask(getSpellInfo()->getSchoolMask());
    isTargetDamageInfoSet = false;
    isForcedCrit = false;

    const auto effectId = getSpellInfo()->getEffect(effIndex);
    if (effectId >= TOTAL_SPELL_EFFECTS)
    {
        LogError("Spell::handleHittedEffect : Unknown spell effect %u in spell id %u, index %u", effectId, getSpellInfo()->getId(), effIndex);
        return;
    }

    LogDebugFlag(LF_SPELL, "Spell::handleHittedEffect : Spell effect %u, spell id %u, damage %d", effectId, getSpellInfo()->getId(), damage);

    const auto scriptResult = sScriptMgr.callScriptedSpellBeforeSpellEffect(this, effIndex);

    // Check if spell is forced to have critical effect on this target
    for (const auto& critGuid : m_critTargets)
    {
        if (critGuid == targetGuid)
        {
            isForcedCrit = true;
            break;
        }
    }

    // Call effect handler
    if (scriptResult != SpellScriptExecuteState::EXECUTE_PREVENT)
        (*this.*SpellEffectsHandler[effectId])(effIndex);

    sScriptMgr.callScriptedSpellAfterSpellEffect(this, effIndex);

    // Create proc events
    if (isTargetDamageInfoSet)
    {
        // Add the DamageInfo to target vector if it was set
        for (auto& uniqueTarget : uniqueHittedTargets)
        {
            if (uniqueTarget.first == targetGuid)
                uniqueTarget.second = m_targetDamageInfo;
        }
    }

    if (uniqueHittedTargets.size() == 1)
    {
        // If spell has only this target, use full DamageInfo for caster's DamageInfo
        if (isTargetDamageInfoSet)
            m_casterDamageInfo = m_targetDamageInfo;
    }
    else
    {
        // If spell has multiple targets, just check if the spell critted for this target
        if (m_targetDamageInfo.isCritical)
            m_casterDamageInfo.isCritical = true;
    }

    // Legacy script hook
    DoAfterHandleEffect(GetUnitTarget(), effIndex);
}

void Spell::handleMissedTarget(SpellTargetMod const missedTarget)
{
    const auto didReflect = missedTarget.hitResult == SPELL_DID_HIT_REFLECT && missedTarget.extendedHitResult == SPELL_DID_HIT_SUCCESS;

    auto travelTime = _getSpellTravelTimeForTarget(missedTarget.targetGuid);
    if (travelTime < 0)
        return;

    // If there is no distance between caster and target, handle effect instantly
    if (travelTime == 0.0f)
    {
        if (didReflect)
        {
            const auto guid = m_caster->getGuid();
            _updateTargetPointers(guid);

            // Process each effect from the spell on the original caster
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (getSpellInfo()->getEffect(i) != 0)
                    handleHittedEffect(guid, i, calculateEffect(i));
            }
        }
        else
        {
            // Spell was not reflected and it did not hit target
            handleMissedEffect(missedTarget.targetGuid);
        }
    }
    else
    {
        if (didReflect)
        {
            const auto guid = m_caster->getGuid();
            _updateTargetPointers(guid);

            // Reflected projectiles move back 4x faster
            travelTime *= 1.25f;

            // Process each effect from the spell on the original caster
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (getSpellInfo()->getEffect(i) != 0)
                {
                    HitSpellEffect hitEffect;
                    hitEffect.damage = calculateEffect(i);
                    hitEffect.effIndex = i;
                    hitEffect.travelTime = float2int32(travelTime);

                    m_hitEffects.insert(std::make_pair(guid, hitEffect));
                }
            }
        }
        else
        {
            // Spell was not reflected and it did not hit target
            m_missEffects.insert(std::make_pair(missedTarget.targetGuid, float2int32(travelTime)));
        }
    }
}

void Spell::handleMissedEffect(const uint64_t targetGuid)
{
    // Spell was not reflected and it did not hit target
    const auto targetUnit = m_caster->GetMapMgrUnit(targetGuid);
    if (targetUnit != nullptr)
    {
        if (u_caster != nullptr && targetUnit->isCreature() && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
        {
            // Let target creature know that someone tried to cast spell on it
            static_cast<Creature*>(targetUnit)->GetAIInterface()->AttackReaction(u_caster, 0, 0);
            static_cast<Creature*>(targetUnit)->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, u_caster, 0);
        }

        // Call scripted after spell missed hook
        sScriptMgr.callScriptedSpellAfterMiss(this, targetUnit);
    }
}

void Spell::finish(bool successful)
{
    if (getState() == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;

    // Caster could be nullptr at this point
    if (getCaster() == nullptr)
    {
        delete this;
        return;
    }

    // Clear spell cooldown if player has cooldown cheat
    if (!m_triggeredSpell && getPlayerCaster() != nullptr && getPlayerCaster()->m_cheats.hasCooldownCheat)
        getPlayerCaster()->clearCooldownForSpell(getSpellInfo()->getId());

    // No need to do anything else on failed spells
    if (!successful)
    {
        getCaster()->removeTravelingSpell(this);
        return;
    }

    // Lua spell hooks
    if (getUnitCaster() != nullptr)
    {
        CALL_SCRIPT_EVENT(getUnitCaster(), OnCastSpell)(getSpellInfo()->getId());

        if (!sEventMgr.HasEvent(getUnitCaster(), EVENT_CREATURE_RESPAWN))
        {
            for (const auto& uniqueTarget : uniqueHittedTargets)
            {
                const auto target = getUnitCaster()->GetMapMgrCreature(uniqueTarget.first);
                if (target == nullptr)
                    continue;

                if (target->GetScript())
                    CALL_SCRIPT_EVENT(target, OnHitBySpell)(getSpellInfo()->getId(), getUnitCaster());
            }
        }

        u_caster->m_canMove = true;
    }

    // Recheck used spell modifiers
    takeUsedSpellModifiers();

    // Set cooldown on item
    if (getItemCaster() != nullptr && getItemCaster()->getOwner() != nullptr && cancastresult == SPELL_CAST_SUCCESS && !GetSpellFailed())
    {
        uint8_t i = 0;
        for (; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (getItemCaster()->getItemProperties()->Spells[i].Trigger == USE)
            {
                if (getItemCaster()->getItemProperties()->Spells[i].Id != 0)
                    break;
            }
        }

        // Potion cooldown starts after leaving combat
        if (getItemCaster()->getItemProperties()->Class == ITEM_CLASS_CONSUMABLE && getItemCaster()->getItemProperties()->SubClass == 1)
        {
            getItemCaster()->getOwner()->SetLastPotion(getItemCaster()->getItemProperties()->ItemId);
            if (!getItemCaster()->getOwner()->CombatStatus.IsInCombat())
                getItemCaster()->getOwner()->UpdatePotionCooldown();
        }
        else
        {
            if (i < MAX_ITEM_PROTO_SPELLS)
                getItemCaster()->getOwner()->Cooldown_AddItem(getItemCaster()->getItemProperties(), i);
        }
    }

    if (getPlayerCaster() != nullptr)
    {
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && getPlayerCaster()->IsAttacking())
        {
            getPlayerCaster()->EventAttackStop();
            getPlayerCaster()->smsg_AttackStop(getPlayerCaster()->GetMapMgrUnit(getPlayerCaster()->GetSelection()));
            getPlayerCaster()->SendPacket(SmsgCancelCombat().serialise().get());
        }

        if (m_Delayed)
        {
            auto target = getPlayerCaster()->GetMapMgrUnit(getPlayerCaster()->getChannelObjectGuid());
            if (target == nullptr)
                target = getPlayerCaster()->GetMapMgrUnit(getPlayerCaster()->GetSelection());

            if (target != nullptr)
                target->RemoveAura(getSpellInfo()->getId(), getCaster()->getGuid());
        }

        if (getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON_OBJECT))
            getPlayerCaster()->SetSummonedObject(nullptr);

        // Update combo points before spell procs
        if (m_requiresCP && !GetSpellFailed())
        {
            if (getPlayerCaster()->m_spellcomboPoints > 0)
            {
                p_caster->m_comboPoints = p_caster->m_spellcomboPoints;
                getPlayerCaster()->UpdateComboPoints();
            }
            else
            {
                getPlayerCaster()->NullComboPoints();
            }
        }
    }

    // Handle procs for each target
    Unit* targetUnit = nullptr;
    if (m_targetProcFlags != 0)
    {
        // Handle each target's procs to caster
        for (const auto& uniqueTarget : uniqueHittedTargets)
        {
            targetUnit = getCaster()->GetMapMgrUnit(uniqueTarget.first);
            if (targetUnit == nullptr)
                continue;

            targetUnit->HandleProc(m_targetProcFlags, getUnitCaster(), getSpellInfo(), uniqueTarget.second, m_triggeredSpell, PROC_EVENT_DO_ALL, m_triggeredByAura);
        }
    }

    // Handle caster procs
    if (getUnitCaster() != nullptr && m_casterProcFlags != 0)
    {
        // Handle caster's procs to each target
        for (const auto& uniqueTarget : uniqueHittedTargets)
        {
            targetUnit = getCaster()->GetMapMgrUnit(uniqueTarget.first);
            if (targetUnit == nullptr)
                continue;

            getUnitCaster()->HandleProc(m_casterProcFlags, targetUnit, getSpellInfo(), uniqueTarget.second, m_triggeredSpell, PROC_EVENT_DO_TARGET_PROCS_ONLY, m_triggeredByAura);
        }

        // Use victim only if there was one target
        if (uniqueHittedTargets.size() > 1)
            targetUnit = nullptr;

        // Handle caster's self procs
        getUnitCaster()->HandleProc(m_casterProcFlags, targetUnit, getSpellInfo(), m_casterDamageInfo, m_triggeredSpell, PROC_EVENT_DO_CASTER_PROCS_ONLY, m_triggeredByAura);
    }

    // QuestMgr spell hooks
    if (getPlayerCaster() != nullptr && getPlayerCaster()->IsInWorld())
    {
        // Do not call QuestMgr::OnPlayerCast for 'on next attack' spells
        // It will be called on the actual spell cast
        if (!(getSpellInfo()->isOnNextMeleeAttack() && !m_triggeredSpell))
        {
            uint32_t targetCount = 0;
            for (auto& target : uniqueHittedTargets)
            {
                WoWGuid wowGuid;
                wowGuid.Init(target.first);
                if (wowGuid.isUnit())
                {
                    ++targetCount;
                    sQuestMgr.OnPlayerCast(getPlayerCaster(), getSpellInfo()->getId(), target.first);
                }
            }

            if (targetCount == 0)
            {
                auto guid = getPlayerCaster()->getTargetGuid();
                sQuestMgr.OnPlayerCast(getPlayerCaster(), getSpellInfo()->getId(), guid);
            }
        }
    }

    // Spell is finished, remove it from traveling spells and delete it on next update tick
    getCaster()->removeTravelingSpell(this);
}

void Spell::update(unsigned long timePassed)
{
    // Check for moving while casting or channeling
    if (m_spellState == SPELL_STATE_PREPARING || m_spellState == SPELL_STATE_CHANNELING)
    {
        // but allow slight error
        if (u_caster != nullptr &&
            (std::fabs(u_caster->GetPositionX() - m_castPositionX) > 0.5f ||
                std::fabs(u_caster->GetPositionY() - m_castPositionY) > 0.5f ||
                std::fabs(u_caster->GetPositionZ() - m_castPositionZ) > 0.5f))
        {
            // TODO: remove this hackfix when movement is sorted out
            if (m_spellState == SPELL_STATE_CHANNELING && !getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_MOD_POSSESS))
            {
                // Cancel channeled spells which don't have ATTRIBUTESEXE_CAN_MOVE_WHILE_CHANNELING flag
                if (getSpellInfo()->getChannelInterruptFlags() & CHANNEL_INTERRUPT_ON_MOVEMENT && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_CAN_MOVE_WHILE_CHANNELING))
                {
                    cancel();
                    return;
                }
            }
            ///\ todo: determine which spells can be cast while moving
            else if (getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_MOVEMENT)
            {
                // Don't cancel on next melee, autorepeat or triggered spells
                if (!u_caster->HasNoInterrupt() && !m_triggeredSpell && !getSpellInfo()->isOnNextMeleeAttack() && !getSpellInfo()->isRangedAutoRepeat())
                {
                    cancel();
                    return;
                }
            }
        }
    }

    switch (m_spellState)
    {
        case SPELL_STATE_PREPARING:
        {
            m_timer -= timePassed;

            if (m_timer <= 0 && !getSpellInfo()->isOnNextMeleeAttack() && !getSpellInfo()->isRangedAutoRepeat())
            {
                // Skip checks for instant spells
                castMe(m_castTime > 0);
            }
        } break;
        case SPELL_STATE_CHANNELING:
        {
            if (m_timer > 0)
            {
                if (p_caster != nullptr)
                {
                    // Check if channeled spell is cancelled when turning
                    if (m_castPositionO != p_caster->GetOrientation() && getSpellInfo()->getChannelInterruptFlags() & CHANNEL_INTERRUPT_ON_TURNING)
                        cancel();
                }

                m_timer -= timePassed;
            }

            // Channeling finishes
            if (m_timer <= 0)
            {
                sendChannelUpdate(0, timePassed);
                finish();
            }
        } break;
        case SPELL_STATE_TRAVELING:
        {
            for (auto hitItr = m_hitEffects.begin(); hitItr != m_hitEffects.end();)
            {
                auto& hitEff = *hitItr;
                if (hitEff.second.travelTime > timePassed)
                {
                    hitEff.second.travelTime -= timePassed;
                    ++hitItr;
                    continue;
                }

                handleHittedEffect(hitEff.first, hitEff.second.effIndex, hitEff.second.damage, true);
                hitItr = m_hitEffects.erase(hitItr);
            }

            for (auto missItr = m_missEffects.begin(); missItr != m_missEffects.end();)
            {
                auto& missEff = *missItr;
                if (missEff.second > timePassed)
                {
                    missEff.second -= timePassed;
                    ++missItr;
                    continue;
                }

                handleMissedEffect(missEff.first);
                missItr = m_missEffects.erase(missItr);
            }

            for (auto auraItr = m_pendingAuras.begin(); auraItr != m_pendingAuras.end();)
            {
                auto& auraEff = *auraItr;
                if (auraEff.second.travelTime > timePassed)
                {
                    auraEff.second.travelTime -= timePassed;
                    ++auraItr;
                    continue;
                }

                HandleAddAura(auraEff.first);
                auraItr = m_pendingAuras.erase(auraItr);
            }

            // If all effects and targets have been processed, finish the spell
            if (m_hitEffects.empty() && m_missEffects.empty() && m_pendingAuras.empty())
                finish();
        } break;
        default:
            break;
    }
}

void Spell::cancel()
{
    switch (getState())
    {
        case SPELL_STATE_PREPARING:
        {
            if (getPlayerCaster() != nullptr)
                getPlayerCaster()->clearGlobalCooldown();

            SendInterrupted(0);
            sendCastResult(SPELL_FAILED_INTERRUPTED);
        } break;
        case SPELL_STATE_CHANNELING:
        {
            sendChannelUpdate(0);
            SendInterrupted(0);
            sendCastResult(SPELL_FAILED_INTERRUPTED);

            if (getUnitCaster() != nullptr)
            {
                if (m_timer > 0 || m_Delayed)
                {
                    auto channelTarget = getUnitCaster()->GetMapMgrUnit(getUnitCaster()->getChannelObjectGuid());
                    if (channelTarget == nullptr && getPlayerCaster() != nullptr)
                        channelTarget = getPlayerCaster()->GetMapMgrUnit(getPlayerCaster()->GetSelection());

                    if (channelTarget != nullptr)
                        channelTarget->RemoveAura(getSpellInfo()->getId(), getCaster()->getGuid());

                    // Remove dynamic objects (area aura effects from Blizzard, Rain of Fire etc)
                    if (m_AreaAura)
                    {
                        const auto dynObj = getUnitCaster()->GetMapMgrDynamicObject(getUnitCaster()->getChannelObjectGuid());
                        if (dynObj != nullptr)
                            dynObj->Remove();
                    }

                    if (getPlayerCaster() != nullptr && getPlayerCaster()->GetSummonedObject() != nullptr)
                    {
                        auto obj = getPlayerCaster()->GetSummonedObject();
                        if (obj->IsInWorld())
                            obj->RemoveFromWorld(true);

                        delete obj;
                        getPlayerCaster()->SetSummonedObject(nullptr);
                    }

                    if (m_timer > 0)
                        RemoveItems();
                }

                getUnitCaster()->RemoveAura(getSpellInfo()->getId(), getCaster()->getGuid());
            }
        } break;
        default:
        {
            if (getState() == SPELL_STATE_NULL)
            {
                // just in case
                if (getCaster() != nullptr)
                    getCaster()->removeTravelingSpell(this);
                else
                    delete this;
            }
        } return;
    }

    // If this is true, the spell is somewhere in ::castMe() function
    // In that case, ::finish() will be called when the spell has hitted targets
    if (!m_isCasting)
        finish(false);
}

int32_t Spell::calculateEffect(uint8_t effIndex)
{
    auto value = getSpellInfo()->calculateEffectValue(effIndex, getUnitCaster(), getItemCaster(), forced_basepoints[effIndex]);

    // Legacy script hook
    value = DoCalculateEffect(effIndex, GetUnitTarget(), value);

    const auto scriptResult = sScriptMgr.callScriptedSpellDoCalculateEffect(this, effIndex, &value);

    // If effect damage was recalculated in script, send static damage in effect handlers
    // so for example spell power bonus won't get calculated twice
    isEffectDamageStatic[effIndex] = scriptResult != SpellScriptEffectDamage::DAMAGE_DEFAULT;
    if (scriptResult == SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION)
        return value;

    if (getPlayerCaster() != nullptr)
    {
        const auto itr = getPlayerCaster()->mSpellOverrideMap.find(getSpellInfo()->getId());
        if (itr != getPlayerCaster()->mSpellOverrideMap.end())
        {
            for (auto scriptOverride = itr->second->begin(); scriptOverride != itr->second->end(); ++scriptOverride)
            {
                value += Util::getRandomUInt((*scriptOverride)->damage);
            }
        }
    }

    if (getUnitCaster() != nullptr)
    {
        // Calculate spell and attack power bonus (must be calculated on launch, not on spell hit!)
        if (!isEffectDamageStatic[effIndex])
        {
            switch (getSpellInfo()->getEffect(effIndex))
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                    value = static_cast<int32_t>(std::ceil(getUnitCaster()->applySpellDamageBonus(getSpellInfo(), value, 1.0f, false, this)));
                    break;
                case SPELL_EFFECT_HEAL:
                case SPELL_EFFECT_HEAL_MECHANICAL:
                    value = static_cast<int32_t>(std::ceil(getUnitCaster()->applySpellHealingBonus(getSpellInfo(), value, 1.0f, false, this)));
                    break;
                default:
                    break;
            }
        }

        // Save pct modifiers before applying so they can be readded properly later
        int32_t spellFlatMods = 0, spellPctMods = 100;

        getUnitCaster()->getTotalSpellModifiers(SPELLMOD_ALL_EFFECTS, value, &spellFlatMods, &spellPctMods, getSpellInfo(), this, nullptr, true);
        getUnitCaster()->applySpellModifiers(SPELLMOD_ALL_EFFECTS, &value, getSpellInfo(), this);

        getUnitCaster()->getTotalSpellModifiers(SPELLMOD_EFFECT_BONUS, value, &spellFlatMods, &spellPctMods, getSpellInfo(), this, nullptr, true);
        getUnitCaster()->applySpellModifiers(SPELLMOD_EFFECT_BONUS, &value, getSpellInfo(), this);

        switch (effIndex)
        {
            case 0:
                getUnitCaster()->getTotalSpellModifiers(SPELLMOD_EFFECT_1, value, &spellFlatMods, &spellPctMods, getSpellInfo(), this, nullptr, true);
                getUnitCaster()->applySpellModifiers(SPELLMOD_EFFECT_1, &value, getSpellInfo(), this);
                break;
            case 1:
                getUnitCaster()->getTotalSpellModifiers(SPELLMOD_EFFECT_2, value, &spellFlatMods, &spellPctMods, getSpellInfo(), this, nullptr, true);
                getUnitCaster()->applySpellModifiers(SPELLMOD_EFFECT_2, &value, getSpellInfo(), this);
                break;
            case 2:
                getUnitCaster()->getTotalSpellModifiers(SPELLMOD_EFFECT_3, value, &spellFlatMods, &spellPctMods, getSpellInfo(), this, nullptr, true);
                getUnitCaster()->applySpellModifiers(SPELLMOD_EFFECT_3, &value, getSpellInfo(), this);
                break;
            default:
                break;
        }

        effectPctModifier[effIndex] = static_cast<float_t>(spellPctMods / 100.0f);
    }
    else if (getItemCaster() != nullptr && GetUnitTarget() != nullptr)
    {
        // Apply spell modifiers from the item owner
        const auto itemCreator = GetUnitTarget()->GetMapMgrUnit(getItemCaster()->getCreatorGuid());
        if (itemCreator != nullptr)
        {
            itemCreator->applySpellModifiers(SPELLMOD_ALL_EFFECTS, &value, getSpellInfo(), this);
            itemCreator->applySpellModifiers(SPELLMOD_EFFECT_BONUS, &value, getSpellInfo(), this);
        }
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spell cast checks
SpellCastResult Spell::canCast(const bool secondCheck, uint32_t* parameter1, uint32_t* parameter2)
{
    ////////////////////////////////////////////////////////
    // Caster checks

    if (p_caster != nullptr)
    {
        if (!m_triggeredSpell)
        {
            if (!getSpellInfo()->isPassive())
            {
                // You can't cast other spells if you have the player flag preventing cast
                if (p_caster->hasPlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST))
                    return SPELL_FAILED_SPELL_IN_PROGRESS;

                // Check for cooldown
                if (p_caster->hasSpellOnCooldown(getSpellInfo()))
                {
                    if (m_triggeredByAura)
                        return SPELL_FAILED_DONT_REPORT;
                    else
                        return SPELL_FAILED_NOT_READY;
                }
            }

            // Check for global cooldown
            // but do not check it on second check
            ///\ todo: need to check for units as well, like when player is controlling a creature
            if (!secondCheck && p_caster->hasSpellGlobalCooldown(getSpellInfo()))
            {
                if (m_triggeredByAura)
                    return SPELL_FAILED_DONT_REPORT;
                else
                    return SPELL_FAILED_NOT_READY;
            }
        }

#if VERSION_STRING >= WotLK
        if (getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IS_CHEAT_SPELL && !p_caster->GetSession()->HasGMPermissions())
        {
            *parameter1 = SPELL_EXTRA_ERROR_GM_ONLY;
            return SPELL_FAILED_CUSTOM_ERROR;
        }
#endif

        // Battleground checks
        if (p_caster->m_bg != nullptr)
        {
#if VERSION_STRING >= TBC
            // Arena checks
            if (isArena(p_caster->m_bg->GetType()))
            {
                // Spells with longer than 10 minute cooldown cannot be casted in arena
                const auto spellCooldown = getSpellInfo()->getRecoveryTime() > getSpellInfo()->getCategoryRecoveryTime() ? getSpellInfo()->getRecoveryTime() : getSpellInfo()->getCategoryRecoveryTime();
                if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS || (spellCooldown > 10 * MINUTE * IN_MILLISECONDS && !(getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS)))
                    return SPELL_FAILED_NOT_IN_ARENA;
            }
#endif

            // If battleground has ended, don't allow spell casting
            if (!m_triggeredSpell && p_caster->m_bg->HasEnded())
                return SPELL_FAILED_DONT_REPORT;
        }
        else if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_BG_ONLY)
        {
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }

        // Movement check
        if (p_caster->isMoving())
        {
            // No need to check for other interrupt flags, client does that for us
            // Also don't cast first ranged autorepeat spell if we're moving but activate it
            // TODO: Missing cata checks, in cata you can cast some spells while moving
            if (getSpellInfo()->isRangedAutoRepeat() || getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                return SPELL_FAILED_MOVING;
        }

        // Prevent casting while sitting unless the spell allows it
        if (!m_triggeredSpell && p_caster->isSitting() && !(getSpellInfo()->getAttributes() & ATTRIBUTES_CASTABLE_WHILE_SITTING))
            return SPELL_FAILED_NOT_STANDING;
    }

    if (u_caster != nullptr)
    {
        // Check if caster is alive
        if (!u_caster->isAlive() && !(getSpellInfo()->getAttributes() & ATTRIBUTES_DEAD_CASTABLE || (m_triggeredSpell && !m_triggeredByAura)))
        {
            // but allow casting while in Spirit of Redemption form
            if (!u_caster->hasAuraWithAuraEffect(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                return SPELL_FAILED_CASTER_DEAD;
        }

        // Check if spell requires caster to be in combat
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNAFFECTED_BY_SCHOOL_IMMUNITY && !u_caster->CombatStatus.IsInCombat())
            return SPELL_FAILED_CASTER_AURASTATE;

        auto requireCombat = true;
        if (u_caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
        {
            for (const auto& aura : u_caster->m_auras)
            {
                if (aura == nullptr)
                    continue;
                if (!aura->getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
                    continue;
                if (aura->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_IGNORE_TARGET_AURA_STATE, getSpellInfo()))
                {
                    // Warrior's Overpower uses "combo points" based on dbc data
                    // This allows usage of Overpower if we have an affecting aura (i.e. Taste for Blood)
                    m_requiresCP = false;

                    // All these aura effects use effect index 0
                    // Allow Warrior's Charge to be casted on combat if caster has Juggernaut or Warbringer talent
                    if (aura->getSpellInfo()->getEffectMiscValue(0) == 1)
                    {
                        // TODO: currently not working, serverside everything was OK but client still gives "You are in combat" error
                        requireCombat = false;
                        break;
                    }
                }
            }
        }

        // Caster's aura state requirements
        if (getSpellInfo()->getCasterAuraState() > 0 && !u_caster->hasAuraState(static_cast<AuraState>(getSpellInfo()->getCasterAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraStateNot() > 0 && u_caster->hasAuraState(static_cast<AuraState>(getSpellInfo()->getCasterAuraStateNot()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;

        // Caster's aura spell requirements
        if (getSpellInfo()->getCasterAuraSpell() > 0 && !u_caster->HasAura(getSpellInfo()->getCasterAuraSpell()))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getCasterAuraSpellNot() == 61988)
            {
                if (u_caster->HasAura(61987))
                    return SPELL_FAILED_CASTER_AURASTATE;
            }
            else if (u_caster->HasAura(getSpellInfo()->getCasterAuraSpellNot()))
            {
                return SPELL_FAILED_CASTER_AURASTATE;
            }
        }

        if (!m_triggeredSpell)
        {
            // Out of combat spells should not be able to be casted in combat
            if (requireCombat && (getSpellInfo()->getAttributes() & ATTRIBUTES_REQ_OOC) && u_caster->CombatStatus.IsInCombat())
                return SPELL_FAILED_AFFECTING_COMBAT;

            if (!secondCheck)
            {
                // Shapeshift check
                auto hasIgnoreShapeshiftAura = false;
                for (const auto& aura : u_caster->m_auras)
                {
                    if (aura == nullptr)
                        continue;
                    // If aura has ignore shapeshift type, you can use spells regardless of stance / form
                    // Auras with this type: Shadow Dance, Metamorphosis, Warbringer (in 3.3.5a)
                    if (!aura->getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_SHAPESHIFT))
                        continue;
                    if (aura->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_IGNORE_SHAPESHIFT, getSpellInfo()))
                    {
                        hasIgnoreShapeshiftAura = true;
                        break;
                    }
                }

                if (!hasIgnoreShapeshiftAura)
                {
                    SpellCastResult shapeError = checkShapeshift(getSpellInfo(), u_caster->getShapeShiftForm());
                    if (shapeError != SPELL_CAST_SUCCESS)
                        return shapeError;

                    // Stealth check
                    if (getSpellInfo()->getAttributes() & ATTRIBUTES_REQ_STEALTH && !u_caster->hasAuraWithAuraEffect(SPELL_AURA_MOD_STEALTH))
                        return SPELL_FAILED_ONLY_STEALTHED;
                }
            }
        }
    }

    // Indoor and outdoor specific spells
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS &&
            !MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if (getSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_INDOORS &&
            MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_INDOORS;
    }

    ////////////////////////////////////////////////////////
    // Target checks

    // Unit target
    const auto target = m_caster->GetMapMgrUnit(m_targets.getUnitTarget());
    if (target != nullptr)
    {
        // Target's aura state requirements
        if (!m_triggeredSpell && getSpellInfo()->getTargetAuraState() > 0 && !target->hasAuraState(static_cast<AuraState>(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraStateNot() > 0 && target->hasAuraState(static_cast<AuraState>(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;

        // Target's aura spell requirements
        if (getSpellInfo()->getTargetAuraSpell() > 0 && !target->HasAura(getSpellInfo()->getTargetAuraSpell()))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getTargetAuraSpellNot() == 61988)
            {
                if (target->HasAura(61987))
                    return SPELL_FAILED_TARGET_AURASTATE;
            }
            else if (target->HasAura(getSpellInfo()->getTargetAuraSpellNot()))
            {
                return SPELL_FAILED_TARGET_AURASTATE;
            }
        }

        if (target->isCorpse())
        {
            // Player can't cast spells on corpses with bones only left
            const auto targetCorpse = sObjectMgr.GetCorpseByOwner(target->getGuidLow());
            if (targetCorpse == nullptr || !targetCorpse->IsInWorld() || targetCorpse->GetCorpseState() == CORPSE_STATE_BONES)
                return SPELL_FAILED_BAD_TARGETS;
        }

        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_TARGET_SELF && m_caster == target)
            return SPELL_FAILED_BAD_TARGETS;

        // Check if spell requires target to be out of combat
        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_OOC_TARGET && target->getcombatstatus()->IsInCombat())
            return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

        if (!(getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_CAN_TARGET_INVISIBLE) && (u_caster != nullptr && !u_caster->canSee(target)))
            return SPELL_FAILED_BAD_TARGETS;

        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_GHOSTS)
        {
            if (!target->hasAuraWithAuraEffect(SPELL_AURA_GHOST))
                return SPELL_FAILED_TARGET_NOT_GHOST;
        }
        else
        {
            if (target->hasAuraWithAuraEffect(SPELL_AURA_GHOST))
                return SPELL_FAILED_BAD_TARGETS;
        }

        // Check for max level
        if (getSpellInfo()->getMaxTargetLevel() != 0 && getSpellInfo()->getMaxTargetLevel() < target->getLevel())
            return SPELL_FAILED_HIGHLEVEL;

        if (m_caster != target)
        {
            if (p_caster != nullptr)
            {
                // Check if caster can attack this creature type
                if (target->isCreature())
                {
                    if (!canAttackCreatureType(dynamic_cast<Creature*>(target)))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // Check if target is already tagged
                // Several spells cannot be casted at already tagged creatures
                // TODO: implement this error message for skinning, mining and herbalism (mining and herbalism cata only)
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CANT_TARGET_TAGGED && target->IsTagged() && !target->isTaggedByPlayerOrItsGroup(p_caster))
                    return SPELL_FAILED_CANT_CAST_ON_TAPPED;

                // GM flagged players should be immune to other players' casts, but not their own
                if (target->isPlayer() && (dynamic_cast<Player*>(target)->hasPlayerFlags(PLAYER_FLAG_GM) || dynamic_cast<Player*>(target)->m_isGmInvisible))
                {
#if VERSION_STRING == Classic
                    return SPELL_FAILED_BAD_TARGETS;
#else
                    return SPELL_FAILED_BM_OR_INVISGOD;
#endif
                }

                // Check if target can be tamed
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_TAME_BEAST)
                {
                    auto targetUnit = p_caster->GetMapMgrUnit(m_targets.getUnitTarget());
                    // If spell is triggered, target may need to be picked manually
                    if (targetUnit == nullptr)
                    {
                        if (p_caster->GetSelection() != 0)
                            targetUnit = p_caster->GetMapMgrUnit(p_caster->GetSelection());
                    }

                    if (targetUnit == nullptr)
                    {
                        SendTameFailure(PETTAME_INVALIDCREATURE);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    const auto creatureTarget = targetUnit->isCreature() ? dynamic_cast<Creature*>(targetUnit) : nullptr;
                    if (creatureTarget == nullptr)
                    {
                        SendTameFailure(PETTAME_INVALIDCREATURE);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (!creatureTarget->isAlive())
                    {
                        SendTameFailure(PETTAME_DEAD);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (creatureTarget->isPet())
                    {
                        SendTameFailure(PETTAME_CREATUREALREADYOWNED);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (creatureTarget->GetCreatureProperties()->Type != UNIT_TYPE_BEAST || creatureTarget->GetCreatureProperties()->Family == 0 || !(creatureTarget->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_TAMEABLE))
                    {
                        SendTameFailure(PETTAME_NOTTAMEABLE);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (p_caster->getClass() != HUNTER)
                    {
                        SendTameFailure(PETTAME_UNITSCANTTAME);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (creatureTarget->getLevel() > p_caster->getLevel())
                    {
                        SendTameFailure(PETTAME_TOOHIGHLEVEL);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (p_caster->GetSummon() != nullptr || p_caster->GetUnstabledPetNumber() != 0)
                    {
                        SendTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (p_caster->GetPetCount() >= 5)
                    {
                        SendTameFailure(PETTAME_TOOMANY);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    // Check for Beast Mastery spell with exotic creatures
                    ///\ todo: move this to spell script
                    if (!p_caster->HasSpell(53270) && creatureTarget->IsExotic())
                    {
                        SendTameFailure(PETTAME_CANTCONTROLEXOTIC);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    // All good so far, check creature's family
                    const auto creatureFamily = sCreatureFamilyStore.LookupEntry(creatureTarget->GetCreatureProperties()->Family);
                    if (creatureFamily == nullptr || creatureFamily->tameable == 0)
                    {
                        SendTameFailure(PETTAME_NOTTAMEABLE);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
            }

            // Do facing checks only for unit casters
            if (u_caster != nullptr)
            {
                // Target must be in front of caster
                // Check for generic ranged spells as well
                if (getSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INFRONT || getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET || getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED)
                {
                    if (!u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }

                // Target must be behind caster
                if (getSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INBACK)
                {
                    if (u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_BEHIND;
                }

                // Caster must be behind the target
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_BEHIND_TARGET && getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET && target->isInFront(u_caster))
                {
                    // Throw spell has these attributes in 3.3.5a, ignore
                    if (getSpellInfo()->getId() != SPELL_RANGED_THROW
#if VERSION_STRING >= TBC
                        // Druid - Pounce, "Patch 2.0.3 - Pounce no longer requires the druid to be behind the target."
                        && !(getSpellInfo()->getSpellFamilyName() == SPELLFAMILY_DRUID && getSpellInfo()->getSpellFamilyFlags(0) == 0x20000)
#endif
#if VERSION_STRING >= WotLK
                        // Rogue - Mutilate, "Patch 3.0.2 - Mutilate no longer requires you be behind the target."
                        && !(getSpellInfo()->getSpellFamilyName() == SPELLFAMILY_ROGUE && getSpellInfo()->getSpellFamilyFlags(1) == 0x200000)
#endif
                        )
                        return SPELL_FAILED_NOT_BEHIND;
                }

                // Caster must be in front of target
                if (getSpellInfo()->getAttributes() == (ATTRIBUTES_ABILITY | ATTRIBUTES_NOT_SHAPESHIFT | ATTRIBUTES_UNK20 | ATTRIBUTES_STOP_ATTACK) && !target->isInFront(u_caster))
                    return SPELL_FAILED_NOT_INFRONT;
            }

            // Check if spell can be casted on dead target
            if (!((getSpellInfo()->getTargets() & (TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNIT_CORPSE)) ||
                getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CAN_BE_CASTED_ON_DEAD_TARGET) && !target->isAlive())
                return SPELL_FAILED_TARGETS_DEAD;

            if (target->hasAuraWithAuraEffect(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                return SPELL_FAILED_BAD_TARGETS;

            // Line of Sight check
            if (!m_triggeredSpell && worldConfig.terrainCollision.isCollisionEnabled)
            {
                if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
                    (m_caster->GetMapId() != target->GetMapId() || !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ())))
                    return SPELL_FAILED_LINE_OF_SIGHT;
            }

            if (target->isPlayer())
            {
                // Check if target is dueling
                // but allow spell cast if target is unfriendly
                const auto targetPlayer = dynamic_cast<Player*>(target);
                if (targetPlayer->GetDuelState() == DUEL_STATE_STARTED)
                {
                    if (m_caster->getPlayerOwner() != nullptr && targetPlayer->DuelingWith != m_caster->getPlayerOwner() && isFriendly(m_caster->getPlayerOwner(), targetPlayer))
                        return SPELL_FAILED_TARGET_DUELING;
                }

                // Check if caster or target is in a sanctuary area
                // but allow spell casting in duels
                if (m_caster->getPlayerOwner() != nullptr && targetPlayer->DuelingWith != m_caster->getPlayerOwner() && !isFriendly(m_caster->getPlayerOwner(), targetPlayer))
                {
                    if ((m_caster->GetArea() != nullptr && m_caster->GetArea()->flags & MapManagement::AreaManagement::AREA_SANCTUARY) ||
                        (targetPlayer->GetArea() != nullptr && targetPlayer->GetArea()->flags & MapManagement::AreaManagement::AREA_SANCTUARY))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // Do not allow spell casts on players when they are on a taxi
                // unless it's a summoning spell
                if (targetPlayer->isOnTaxi() && !getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON_PLAYER))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_PLAYERS)
                return SPELL_FAILED_TARGET_NOT_PLAYER;

            // Check if target has stronger aura active
            const AuraCheckResponse auraCheckResponse = target->AuraCheck(getSpellInfo(), m_caster);
            if (auraCheckResponse.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                return SPELL_FAILED_AURA_BOUNCED;

            // Check if target is immune to this dispel type
            //\ TODO: fix me (move to DidHit?) -Appled
            if (target->dispels[getSpellInfo()->getDispelType()])
                return SPELL_FAILED_IMMUNE;
        }
    }

    // Check if spell effect requires pet target
    if (p_caster != nullptr)
    {
        if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_DEAD_PET)
        {
            const auto pet = p_caster->GetSummon();
            if (pet == nullptr)
                return SPELL_FAILED_NO_PET;
            if (pet->isAlive())
                return SPELL_FAILED_TARGET_NOT_DEAD;
        }

        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
            {
                const auto pet = p_caster->GetSummon();
                if (pet == nullptr)
                    return m_triggeredByAura ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NO_PET;
                else if (!pet->isAlive())
                    return m_triggeredByAura ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_TARGETS_DEAD;
                // Check Line of Sight with pets as well
                if (!m_triggeredSpell && worldConfig.terrainCollision.isCollisionEnabled)
                {
                    if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
                        (m_caster->GetMapId() != pet->GetMapId() || !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), pet->GetPositionX(), pet->GetPositionY(), pet->GetPositionZ())))
                        return SPELL_FAILED_LINE_OF_SIGHT;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////
    // Area checks

    // Check Line of Sight for spells with a destination
    if (m_targets.hasDestination() && !m_triggeredSpell && worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
            !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_targets.getDestination().x, m_targets.getDestination().y, m_targets.getDestination().z))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    if (p_caster != nullptr)
    {
        // Check if spell requires certain area
        if (getSpellInfo()->getRequiresAreaId() > 0)
        {
            auto areaEntry = p_caster->GetArea();
            if (areaEntry == nullptr)
                areaEntry = sAreaStore.LookupEntry(p_caster->GetZoneId());
            if (areaEntry == nullptr)
                return SPELL_FAILED_NOT_HERE;

            const auto requireAreaId = static_cast<uint32_t>(getSpellInfo()->getRequiresAreaId());
#if VERSION_STRING == TBC
            if (requireAreaId != areaEntry->id && requireAreaId != areaEntry->zone)
            {
                *parameter1 = getSpellInfo()->getRequiresAreaId();
                return SPELL_FAILED_REQUIRES_AREA;
            }
#elif VERSION_STRING >= WotLK
            auto found = false;
            auto areaGroup = sAreaGroupStore.LookupEntry(requireAreaId);
            while (areaGroup != nullptr)
            {
                for (const auto& i : areaGroup->AreaId)
                {
                    if (i == areaEntry->id || (areaEntry->zone != 0 && i == areaEntry->zone))
                    {
                        found = true;
                        *parameter1 = 0;
                        break;
                    }
                    else if (i != 0)
                    {
                        *parameter1 = i;
                    }
                }

                if (found || areaGroup->next_group == 0)
                    break;

                areaGroup = sAreaGroupStore.LookupEntry(areaGroup->next_group);
            }

            if (!found)
                return SPELL_FAILED_REQUIRES_AREA;
#endif
        }

        // Flying mount check
        if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_ONLY_IN_OUTLANDS)
        {
            if (!p_caster->canUseFlyingMountHere())
            {
                if (p_caster->GetMapId() != 571 || !(getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING))
                    return SPELL_FAILED_NOT_HERE;
            }
        }

        // Check if spell can be casted while mounted or on a taxi
        // but skip triggered and passive spells
        if ((p_caster->hasUnitFlags(UNIT_FLAG_MOUNT) || p_caster->hasUnitFlags(UNIT_FLAG_MOUNTED_TAXI)) && !m_triggeredSpell && !getSpellInfo()->isPassive())
        {
            if (p_caster->isOnTaxi())
            {
                return SPELL_FAILED_NOT_ON_TAXI;
            }
            else
            {
                if (!(getSpellInfo()->getAttributes() & ATTRIBUTES_MOUNT_CASTABLE))
                    return SPELL_FAILED_NOT_MOUNTED;
            }
        }

        // Check if spell can be casted in heroic dungeons or in raids
        if (getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_NOT_IN_RAIDS_OR_HEROIC_DUNGEONS)
        {
            if (p_caster->IsInWorld() && p_caster->GetMapMgr()->GetMapInfo() != nullptr && (p_caster->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID || p_caster->GetMapMgr()->iInstanceMode == MODE_HEROIC))
            {
#if VERSION_STRING < WotLK
                return SPELL_FAILED_NOT_HERE;
#else
                return SPELL_FAILED_NOT_IN_RAID_INSTANCE;
#endif
            }
        }
    }

    ////////////////////////////////////////////////////////
    // Item, state, range and power checks

    const SpellCastResult itemCastResult = checkItems(parameter1, parameter2);
    if (itemCastResult != SPELL_CAST_SUCCESS)
        return itemCastResult;

    if (!m_triggeredSpell)
    {
        const SpellCastResult casterStateResult = checkCasterState();
        if (casterStateResult != SPELL_CAST_SUCCESS)
            return casterStateResult;

        const SpellCastResult rangeResult = checkRange(secondCheck);
        if (rangeResult != SPELL_CAST_SUCCESS)
            return rangeResult;

        const SpellCastResult powerResult = checkPower();
        if (powerResult != SPELL_CAST_SUCCESS)
            return powerResult;
    }

    ////////////////////////////////////////////////////////
    // Spell focus object check

    if (p_caster != nullptr && getSpellInfo()->getRequiresSpellFocus() > 0)
    {
        auto found = false;
        for (const auto& itr : p_caster->getInRangeObjectsSet())
        {
            if (itr == nullptr || !itr->isGameObject())
                continue;

            if (const auto obj = dynamic_cast<GameObject*>(itr))
            {
                if (obj->getGoType() != GAMEOBJECT_TYPE_SPELL_FOCUS)
                    continue;

                // Skip objects from other phases
                if (!(p_caster->GetPhase() & obj->GetPhase()))
                    continue;

                const auto gameObjectInfo = obj->GetGameObjectProperties();
                if (gameObjectInfo == nullptr)
                {
                    LogDebugFlag(LF_SPELL, "Spell::canCast : Found gameobject entry %u with invalid gameobject properties, spawn id %u", obj->getEntry(), obj->getGuidLow());
                    continue;
                }

                // Prefer to use range from gameobject_properties instead of spell's range
                // That is required at least for profession spells since their range is set to 0 yards in DBC files
                float_t distance = 0.0f;
                if (gameObjectInfo->spell_focus.distance > 0)
                {
                    // Database seems to already have squared distances
                    distance = static_cast<float_t>(gameObjectInfo->spell_focus.distance);
                }
                else
                {
                    distance = GetMaxRange(sSpellRangeStore.LookupEntry(getSpellInfo()->getRangeIndex()));
                    distance *= distance;
                }

                // Skip objects which are out of range
                if (!p_caster->isInRange(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), distance))
                    continue;

                if (gameObjectInfo->spell_focus.focus_id == getSpellInfo()->getRequiresSpellFocus())
                {
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            *parameter1 = getSpellInfo()->getRequiresSpellFocus();
            return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
        }
    }

    ////////////////////////////////////////////////////////
    // Spell target constraint check (checks if spell is castable only on certain creature or gameobject)

    if (m_target_constraint != nullptr)
    {
        // Search for target constraint from within spell's max range
        float_t range = 0.0f;
        const auto rangeEntry = sSpellRangeStore.LookupEntry(getSpellInfo()->getRangeIndex());
        if (rangeEntry != nullptr)
            range = rangeEntry->maxRange;

        auto foundTarget = false;

        // Check if target needs to be a certain creature
        for (const auto& entryId : m_target_constraint->getCreatures())
        {
            if (!m_target_constraint->hasExplicitTarget(entryId))
            {
                // Spell requires an implicit target
                // Find closest creature with the required entry id
                const auto creatureTarget = m_caster->IsInWorld() ? m_caster->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), entryId) : nullptr;
                if (creatureTarget != nullptr)
                {
                    // Check that the creature is within spell's range
                    if (m_caster->isInRange(creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), range * range))
                    {
                        SetTargetConstraintCreature(creatureTarget);
                        foundTarget = true;
                        break;
                    }
                }
            }
            else
            {
                // Spell requires an explicit target
                // Most these spells are casted from items and then client does NOT send target guid in cast spell packet
                // Use player's selection guid as target
                Unit* creatureTarget = nullptr;
                if (p_caster != nullptr)
                    creatureTarget = p_caster->GetMapMgrUnit(p_caster->GetSelection());

                if (creatureTarget == nullptr)
                    continue;

                if (!creatureTarget->isCreature() || creatureTarget->getEntry() != entryId)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check that the creature is within spell's range
                if (!m_caster->isInRange(creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), range * range))
                    return SPELL_FAILED_OUT_OF_RANGE;

                // Found target
                SetTargetConstraintCreature(dynamic_cast<Creature*>(creatureTarget));
                foundTarget = true;
                break;
            }
        }

        // Check if target needs to be a certain gameobject
        for (const auto& entryId : m_target_constraint->getGameObjects())
        {
            if (!m_target_constraint->hasExplicitTarget(entryId))
            {
                // Spell requires an implicit target
                // Find closest gameobject with the required entry id
                const auto gobTarget = m_caster->IsInWorld() ? m_caster->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), entryId) : nullptr;
                if (gobTarget != nullptr)
                {
                    // Check that the gameobject is within spell's range
                    if (m_caster->isInRange(gobTarget->GetPositionX(), gobTarget->GetPositionY(), gobTarget->GetPositionZ(), range * range))
                    {
                        SetTargetConstraintGameObject(gobTarget);
                        foundTarget = true;
                    }
                }
            }
            else
            {
                // Spell requires an explicit target
                const auto objectTarget = m_caster->GetMapMgrObject(m_targets.getGameObjectTarget());
                if (objectTarget == nullptr)
                    continue;

                if (!objectTarget->isGameObject() || objectTarget->getEntry() != entryId)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check that the gameobject is within the spell's range
                if (!m_caster->isInRange(objectTarget->GetPositionX(), objectTarget->GetPositionY(), objectTarget->GetPositionZ(), range * range))
                    return SPELL_FAILED_OUT_OF_RANGE;

                // Found target
                SetTargetConstraintGameObject(dynamic_cast<GameObject*>(objectTarget));
                foundTarget = true;
                break;
            }
        }

        if (!foundTarget)
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
    }

    ////////////////////////////////////////////////////////
    // Check for scripted cast check

    const SpellCastResult scriptCheck = sScriptMgr.callScriptedSpellCanCast(this, parameter1, parameter2);
    if (scriptCheck != SPELL_CAST_SUCCESS)
        return scriptCheck;

    ////////////////////////////////////////////////////////
    // Special checks for different spell effects

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (getSpellInfo()->getEffect(i))
        {
            case SPELL_EFFECT_RESURRECT:
            case SPELL_EFFECT_RESURRECT_FLAT:
            {
                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
                if (target->isAlive())
                    return SPELL_FAILED_TARGET_NOT_DEAD;
#if VERSION_STRING >= WotLK
                if (target->hasAuraWithAuraEffect(SPELL_AURA_PREVENT_RESURRECTION))
                    return SPELL_FAILED_TARGET_CANNOT_BE_RESURRECTED;
#endif
            } break;
            case SPELL_EFFECT_SUMMON:
            {
                if (p_caster == nullptr)
                    break;

                if (p_caster->GetSummon() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if (p_caster->getCharmGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
            } break;
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
                if (p_caster == nullptr)
                    break;

                // Don't allow these effects in battlegrounds if the battleground hasn't yet started
                if (p_caster->m_bg != nullptr && !p_caster->m_bg->HasStarted())
                    return SPELL_FAILED_TRY_AGAIN;
            } break;
            case SPELL_EFFECT_SUMMON_PET:
            {
                if (p_caster == nullptr)
                    break;

                if (p_caster->GetSummon() != nullptr)
                {
                    if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                }

                if (p_caster->getCharmGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
            } break;
            case SPELL_EFFECT_OPEN_LOCK:
            case SPELL_EFFECT_OPEN_LOCK_ITEM:
            {
                if (p_caster == nullptr)
                    break;

                uint32_t lockId = 0;
                if (m_targets.getGameObjectTarget() != 0)
                {
                    const auto objectTarget = p_caster->GetMapMgrGameObject(m_targets.getGameObjectTarget());
                    if (objectTarget != nullptr && objectTarget->getGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                    {
                        // Get lock id
                        switch (objectTarget->getGoType())
                        {
                            case GAMEOBJECT_TYPE_DOOR:
                                lockId = objectTarget->GetGameObjectProperties()->door.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_BUTTON:
                                lockId = objectTarget->GetGameObjectProperties()->button.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_CHEST:
                                lockId = objectTarget->GetGameObjectProperties()->chest.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_TRAP:
                                lockId = objectTarget->GetGameObjectProperties()->trap.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_GOOBER:
                                lockId = objectTarget->GetGameObjectProperties()->goober.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_CAMERA:
                                lockId = objectTarget->GetGameObjectProperties()->camera.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_FISHINGHOLE:
                                lockId = objectTarget->GetGameObjectProperties()->fishinghole.lock_id;
                                break;
                            default:
                                break;
                        }

                        if (lockId == 0)
                            return SPELL_FAILED_ALREADY_OPEN;
                    }
                }
                else if (m_targets.getItemTarget() != 0)
                {
                    Item const* targetItem = nullptr;
                    if (m_targets.isTradeItem())
                    {
                        const auto playerTrader = p_caster->getTradeTarget();
                        if (playerTrader != nullptr)
                            targetItem = playerTrader->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTarget()));
                    }
                    else
                    {
                        targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                    }

                    if (targetItem == nullptr)
                        return SPELL_FAILED_ITEM_GONE;

                    // Check if item is already unlocked
                    if (targetItem->getItemProperties()->LockId == 0 || !targetItem->locked)
                        return SPELL_FAILED_ALREADY_OPEN;

                    lockId = targetItem->getItemProperties()->LockId;
                }

                if (lockId == 0)
                    break;

                const auto lockInfo = sLockStore.LookupEntry(lockId);
                if (lockInfo == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                auto successfulOpening = false;
                for (uint8_t x = 0; x < LOCK_NUM_CASES; ++x)
                {
                    // Check if object requires an item for unlocking
                    if (lockInfo->locktype[x] == LOCK_KEY_ITEM)
                    {
                        if (i_caster == nullptr || lockInfo->lockmisc[x] == 0)
                            return SPELL_FAILED_BAD_TARGETS;
                        // No need to check further on a successful match
                        if (i_caster->getEntry() == lockInfo->lockmisc[x])
                        {
                            successfulOpening = true;
                            break;
                        }
                    }
                    // Check if object requires a skill for unlocking
                    else if (lockInfo->locktype[x] == LOCK_KEY_SKILL)
                    {
                        // Check if spell's skill matches with the required skill
                        if (static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i)) != lockInfo->lockmisc[x])
                            continue;

                        // Get required skill line
                        uint32_t skillId = 0;
                        switch (lockInfo->lockmisc[x])
                        {
                            case LOCKTYPE_PICKLOCK:
                                skillId = SKILL_LOCKPICKING;
                                break;
                            case LOCKTYPE_HERBALISM:
                                skillId = SKILL_HERBALISM;
                                break;
                            case LOCKTYPE_MINING:
                                skillId = SKILL_MINING;
                                break;
                            case LOCKTYPE_FISHING:
                                skillId = SKILL_FISHING;
                                break;
                            case LOCKTYPE_INSCRIPTION:
                                skillId = SKILL_INSCRIPTION;
                                break;
                            default:
                                break;
                        }

                        if (skillId != 0 || lockInfo->lockmisc[x] == LOCKTYPE_BLASTING)
                        {
                            // If item is used for opening, do not use player's skill level
                            uint32_t skillLevel = i_caster != nullptr || p_caster == nullptr ? 0 : p_caster->_GetSkillLineCurrent(skillId);
                            // Add skill bonuses from the spell
                            skillLevel += getSpellInfo()->calculateEffectValue(i);;

                            // Check for low skill level
                            if (skillLevel < lockInfo->minlockskill[x])
                                return SPELL_FAILED_LOW_CASTLEVEL;

#if VERSION_STRING >= WotLK
                            // Patch 3.2.0: In addition to the normal requirements, mining deposits in Northrend now require a minimum character level of 65 to mine.
                            if (skillId == SKILL_MINING && p_caster->GetMapId() == 571 && p_caster->getLevel() < 65)
                            {
                                *parameter1 = SPELL_EXTRA_ERROR_NORTHREND_MINING;
                                return SPELL_FAILED_CUSTOM_ERROR;
                            }
#endif

#if VERSION_STRING < Cata
                            // Check for failed attempt only at the end of cast
                            // Patch 3.1.0: You can no longer fail when Mining, Herbing, and Skinning
                            if (secondCheck && (skillId == SKILL_LOCKPICKING
#if VERSION_STRING < WotLK
                                || skillId == SKILL_HERBALISM || skillId == SKILL_MINING
#endif
                                ))
                            {
                                // Failed attempt can only happen at orange gather/pick lock and can also happen at max skill level
                                // In gathering professions orange most of the time turns to green after gaining 25 skill points
                                const auto skillDifference = skillLevel - lockInfo->minlockskill[x];
                                uint8_t failChance = 0;
                                // TODO: these values are some what correct for Classic but not confirmed
                                // need more research for TBC -Appled
                                if (skillDifference < 5)
                                    failChance = 50;
                                else if (skillDifference < 10)
                                    failChance = 35;
                                else if (skillDifference < 15)
                                    failChance = 20;
                                else if (skillDifference < 20)
                                    failChance = 10;
                                else if (skillDifference < 25)
                                    failChance = 5;

                                if (failChance > 0 && Util::getRandomUInt(100) < failChance)
                                    return SPELL_FAILED_TRY_AGAIN;
                            }
#endif
                        }

                        successfulOpening = true;
                        break;
                    }
                }

                if (!successfulOpening)
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if (getSpellInfo()->getEffectImplicitTargetA(i) != EFF_TARGET_PET)
                    break;
            }
            // no break here
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                if (p_caster == nullptr)
                    break;

                const auto pet = p_caster->GetSummon();
                if (pet == nullptr)
                    return SPELL_FAILED_NO_PET;

                const auto newSpell = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(i));
                if (newSpell == nullptr)
                    return SPELL_FAILED_NOT_KNOWN;

                const auto learnStatus = pet->CanLearnSpell(newSpell);
                if (learnStatus != 0)
                    return SpellCastResult(learnStatus);
            } break;
            case SPELL_EFFECT_POWER_BURN:
            case SPELL_EFFECT_POWER_DRAIN:
            {
                if (u_caster == nullptr)
                    break;

                if (target == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                // Do not check further for self casts
                if (u_caster == target)
                    break;

                // Check for correct power type
                if (target->getMaxPower(target->getPowerType()) == 0 || target->getPowerType() != static_cast<uint8_t>(getSpellInfo()->getEffectMiscValue(i)))
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
            case SPELL_EFFECT_PICKPOCKET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                if (!target->isCreature())
                    return SPELL_FAILED_BAD_TARGETS;

                // Check if target is already pick pocketed
                if (dynamic_cast<Creature*>(target)->IsPickPocketed())
                    return SPELL_FAILED_TARGET_NO_POCKETS;

                const auto itr = sLootMgr.PickpocketingLoot.find(dynamic_cast<Creature*>(target)->getEntry());
                if (itr == sLootMgr.PickpocketingLoot.end())
                    return SPELL_FAILED_TARGET_NO_POCKETS;
            } break;
#if VERSION_STRING >= WotLK
            case SPELL_EFFECT_USE_GLYPH:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                const auto glyphId = getSpellInfo()->getEffectMiscValue(i);
                const auto glyphEntry = sGlyphPropertiesStore.LookupEntry(glyphId);
                if (glyphEntry == nullptr)
                    return SPELL_FAILED_INVALID_GLYPH;

                // Check if glyph slot is locked
                if (!(p_caster->getGlyphsEnabled() & (1 << m_glyphslot)))
                    return SPELL_FAILED_GLYPH_SOCKET_LOCKED;

                // Check if player already has this glyph
                if (p_caster->HasAura(glyphEntry->SpellID))
                    return SPELL_FAILED_UNIQUE_GLYPH;
            } break;
#endif
            case SPELL_EFFECT_DUEL:
            {
                if (p_caster == nullptr)
                    break;

                if (p_caster->GetArea() != nullptr && p_caster->GetArea()->flags & MapManagement::AreaManagement::AREA_CITY_AREA)
                    return SPELL_FAILED_NO_DUELING;

                if (p_caster->isStealthed())
                    return SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED;

                if (p_caster->isInvisible())
                    return SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE;

                // Check if caster is in dungeon or raid
                if (p_caster->IsInWorld() && p_caster->GetMapMgr()->GetMapInfo() != nullptr && p_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
                    return SPELL_FAILED_NO_DUELING;

                const auto targetPlayer = p_caster->GetMapMgrPlayer(m_targets.getUnitTarget());
                if (targetPlayer != nullptr && targetPlayer->GetTransport() != p_caster->GetTransport())
                    return SPELL_FAILED_NOT_ON_TRANSPORT;

                if (targetPlayer != nullptr && targetPlayer->DuelingWith != nullptr)
                    return SPELL_FAILED_TARGET_DUELING;
            } break;
            case SPELL_EFFECT_SUMMON_PLAYER:
            {
                if (p_caster == nullptr || target == nullptr || !target->isPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                const auto targetPlayer = dynamic_cast<Player*>(target);
                // Check if target is in same group/raid with the caster
                if (targetPlayer == p_caster || p_caster->getGroup() == nullptr || !p_caster->getGroup()->HasMember(targetPlayer))
                    return SPELL_FAILED_TARGET_NOT_IN_RAID;

                // Check if caster is in an instance map
                if (p_caster->IsInWorld() && p_caster->GetMapMgr()->GetMapInfo() != nullptr && p_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
                {
                    if (!p_caster->IsInMap(targetPlayer))
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;

                    const auto mapInfo = p_caster->GetMapMgr()->GetMapInfo();
                    if (p_caster->GetMapMgr()->iInstanceMode == MODE_HEROIC)
                    {
                        if (mapInfo->minlevel_heroic > targetPlayer->getLevel())
                            return SPELL_FAILED_LOWLEVEL;
                    }
                    else
                    {
                        if (mapInfo->minlevel > targetPlayer->getLevel())
                            return SPELL_FAILED_LOWLEVEL;
                    }

                    // Check if caster is in a battleground
                    if (mapInfo->type == INSTANCE_BATTLEGROUND || p_caster->m_bg != nullptr)
                    {
#if VERSION_STRING == Classic
                        return SPELL_FAILED_NOT_HERE;
#else
                        return SPELL_FAILED_NOT_IN_BATTLEGROUND;
#endif
                    }
                }
            } break;
            case SPELL_EFFECT_SELF_RESURRECT:
            {
#if VERSION_STRING >= WotLK
                if (u_caster != nullptr && u_caster->hasAuraWithAuraEffect(SPELL_AURA_PREVENT_RESURRECTION))
                    return SPELL_FAILED_TARGET_CANNOT_BE_RESURRECTED;
#endif
            } break;
            case SPELL_EFFECT_SKINNING:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                if (target == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                if (target->isAlive())
                    return SPELL_FAILED_TARGET_NOT_DEAD;

                if (!target->hasUnitFlags(UNIT_FLAG_SKINNABLE) || !target->isCreature())
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                // Check if creature is already skinned
                const auto creatureTarget = dynamic_cast<Creature*>(target);
                if (creatureTarget->Skinned)
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                // Check if creature is looted
                if (creatureTarget->loot.any() && creatureTarget->IsTagged())
                {
                    const auto taggerPlayer = creatureTarget->GetMapMgrPlayer(creatureTarget->GetTaggerGUID());
                    if (taggerPlayer != nullptr && creatureTarget->HasLootForPlayer(taggerPlayer))
                        return SPELL_FAILED_TARGET_NOT_LOOTED;
                }

                // Check if caster has required skinning level for target
                const auto skillLevel = p_caster->_GetSkillLineCurrent(creatureTarget->GetRequiredLootSkill());
                // Required skinning level is calculated by multiplying the target's level by 5
                // but if player's skill level is below 100, then player's skill level is incremented by 100 and target's level is multiplied by 10
                const int32_t skillDiff = skillLevel >= 100 ? skillLevel - (creatureTarget->getLevel() * 5) : (skillLevel + 100) - (creatureTarget->getLevel() * 10);
                if (skillDiff < 0)
                    return SPELL_FAILED_LOW_CASTLEVEL;

#if VERSION_STRING < WotLK
                // Check for failed attempt at the end of cast
                // Patch 3.1.0: You can no longer fail when Mining, Herbing, and Skinning
                if (secondCheck)
                {
                    uint8_t failChance = 0;
                    // TODO: these values are some what correct for Classic but not confirmed
                    // need more research for TBC -Appled
                    if (skillDiff < 5)
                        failChance = 50;
                    else if (skillDiff < 10)
                        failChance = 35;
                    else if (skillDiff < 15)
                        failChance = 20;
                    else if (skillDiff < 20)
                        failChance = 10;
                    else if (skillDiff < 25)
                        failChance = 5;

                    if (failChance > 0 && Util::getRandomUInt(100) < failChance)
                        return SPELL_FAILED_TRY_AGAIN;
                }
#endif
            } break;
            case SPELL_EFFECT_CHARGE:
            {
                if (u_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                if (u_caster->isRooted())
                    return SPELL_FAILED_ROOTED;

                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (worldConfig.terrainCollision.isPathfindingEnabled)
                {
                    // Check if caster is able to create path to target
                    if (!u_caster->GetAIInterface()->CanCreatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()))
                        return SPELL_FAILED_NOPATH;
                }
            } break;
            case SPELL_EFFECT_FEED_PET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                const auto pet = p_caster->GetSummon();
                if (pet == nullptr)
                    return SPELL_FAILED_NO_PET;

                if (!pet->isAlive())
                    return SPELL_FAILED_TARGETS_DEAD;

                // Get the food
                const auto foodItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                if (foodItem == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check if the item is food
                const auto itemProto = foodItem->getItemProperties();
                if (itemProto->FoodType == 0)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check if the food type matches pet's diet
                if (!(pet->GetPetDiet() & (1 << (itemProto->FoodType - 1))))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                // Check if the food level is at most 30 levels below pet's level
                if (pet->getLevel() > (itemProto->ItemLevel + 30))
                    return SPELL_FAILED_FOOD_LOWLEVEL;
            } break;
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_NO_PET;

                const auto petTarget = p_caster->GetSummon();
                if (petTarget == nullptr)
                    return SPELL_FAILED_NO_PET;

                if (petTarget->isAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;
            } break;
            case SPELL_EFFECT_SPELL_STEAL:
            {
                if (m_targets.getUnitTarget() == m_caster->getGuid())
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
            default:
                break;
        }
    }

    ////////////////////////////////////////////////////////
    // Special checks for different aura effects

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (getSpellInfo()->getEffectApplyAuraName(i))
        {
            case SPELL_AURA_MOD_POSSESS:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (target == p_caster)
                    return SPELL_FAILED_BAD_TARGETS;

                if (p_caster->GetSummon() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if (p_caster->getCharmGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                // Check if caster is charmed
                if (p_caster->getCharmedByGuid() != 0)
                    return SPELL_FAILED_CHARMED;

                // Check if target is already charmed
                if (target->getCharmedByGuid() != 0)
                    return SPELL_FAILED_CANT_BE_CHARMED;

                // Check if target is owned by player
                if (!target->isPlayer() && target->getPlayerOwner() != nullptr)
                {
#if VERSION_STRING == Classic
                    return SPELL_FAILED_BAD_TARGETS;
#else
                    return SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED;
#endif
                }

                if (static_cast<int32_t>(target->getLevel()) > calculateEffect(i))
                    return SPELL_FAILED_HIGHLEVEL;
            } break;
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_AREA_CHARM:
            {
                if (u_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (target == u_caster)
                    return SPELL_FAILED_BAD_TARGETS;

                if (getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_CHARM)
                {
                    if (p_caster != nullptr && p_caster->GetSummon() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    // Player can have only one charm at time
                    if (p_caster != nullptr && p_caster->getCharmGuid() != 0)
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }

                // Check if caster is charmed
                if (u_caster->getCharmedByGuid() != 0)
                    return SPELL_FAILED_CHARMED;

                // Check if target is already charmed
                if (target->getCharmedByGuid() != 0)
                    return SPELL_FAILED_CANT_BE_CHARMED;

                // Check if target is owned by player
                if (!target->isPlayer() && target->getPlayerOwner() != nullptr)
                {
#if VERSION_STRING == Classic
                    return SPELL_FAILED_BAD_TARGETS;
#else
                    return SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED;
#endif
                }

                if (static_cast<int32_t>(target->getLevel()) > calculateEffect(i))
                    return SPELL_FAILED_HIGHLEVEL;

                const auto targetCreature = dynamic_cast<Creature*>(target);
                if (p_caster != nullptr && target->isCreature() && targetCreature->IsTagged() && !targetCreature->isTaggedByPlayerOrItsGroup(p_caster))
                    return SPELL_FAILED_CANT_CAST_ON_TAPPED;
            } break;
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            {
                // Skip for non-player and item casters
                if (p_caster == nullptr || i_caster != nullptr)
                    break;

                if (target != nullptr && (target->getMaxPower(POWER_TYPE_MANA) == 0 || target->getPowerType() != POWER_TYPE_MANA))
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
            case SPELL_AURA_MOD_DISARM:
            {
                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // Check if target is not already disarmed
                if (target->getUnitFlags() & UNIT_FLAG_DISARMED)
                    return SPELL_FAILED_TARGET_NO_WEAPONS;

                if (target->isPlayer())
                {
                    // Check if player is even wielding a weapon
                    const auto mainHandWeapon = dynamic_cast<Player*>(target)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (mainHandWeapon == nullptr || !mainHandWeapon->isWeapon())
                        return SPELL_FAILED_TARGET_NO_WEAPONS;
                }
                else
                {
                    if (const auto creature = dynamic_cast<Creature*>(target))
                    {
                        if (const auto creatureProto = creature->GetCreatureProperties())
                        {
                            if (creatureProto->Type != UNIT_TYPE_HUMANOID && creatureProto->Type != UNIT_TYPE_DEMON &&
                                creatureProto->Type != UNIT_TYPE_GIANT && creatureProto->Type != UNIT_TYPE_UNDEAD)
                                return SPELL_FAILED_TARGET_NO_WEAPONS;
                        }
                    }

                    // Check if creature is even wielding a weapon
                    if (target->getVirtualItemSlotId(MELEE) == 0)
                        return SPELL_FAILED_TARGET_NO_WEAPONS;
                }
            } break;
            case SPELL_AURA_MOUNTED:
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    if (!MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionNC().x, m_caster->GetPositionNC().y, m_caster->GetPositionNC().z))
                        return SPELL_FAILED_NO_MOUNTS_ALLOWED;
                }

                if (p_caster != nullptr)
                {
                    if (p_caster->GetTransport() != nullptr)
                        return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                    if (p_caster->hasUnitFlags(UNIT_FLAG_LOOTING))
                    {
                        p_caster->sendMountResultPacket(ERR_MOUNT_LOOTING);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    // Check shapeshift form
                    if (p_caster->getShapeShiftForm() != FORM_NORMAL)
                    {
                        switch (p_caster->getShapeShiftForm())
                        {
                            case FORM_CAT:
                            case FORM_TREE:
                            case FORM_TRAVEL:
                            case FORM_AQUA:
                            case FORM_BEAR:
                            case FORM_GHOUL:
                            case FORM_DIREBEAR:
                            case FORM_CREATUREBEAR:
                            case FORM_CREATURECAT:
                            case FORM_GHOSTWOLF:
                            case FORM_ZOMBIE:
                            case FORM_METAMORPHOSIS:
                            case FORM_DEMON:
                            case FORM_FLIGHT:
                            case FORM_MOONKIN:
                            case FORM_SPIRITOFREDEMPTION:
                            {
                                p_caster->sendMountResultPacket(ERR_MOUNT_SHAPESHIFTED);
                                return SPELL_FAILED_DONT_REPORT;
                            } break;
                            default:
                                break;
                        }
                    }
                }
            } break;
            case SPELL_AURA_MOD_POSSESS_PET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                if (p_caster->getCharmGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (p_caster->getCharmedByGuid() != 0)
                    return SPELL_FAILED_CHARMED;

                const auto petTarget = p_caster->GetSummon();
                if (petTarget == nullptr)
                    return SPELL_FAILED_NO_PET;

                if (petTarget->getCharmedByGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
            } break;
            case SPELL_AURA_FLY:
            case SPELL_AURA_ENABLE_FLIGHT2:
            {
                if (p_caster != nullptr && p_caster->isAlive())
                {
                    if (!p_caster->canUseFlyingMountHere())
                        return m_triggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NOT_HERE;
                }
            } break;
            case SPELL_AURA_MIRROR_IMAGE:
            {
                // Clone effects require creature or player target
                if (target == nullptr)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;;

                if (target->getGuid() == m_caster->getGuid())
                    return SPELL_FAILED_BAD_TARGETS;

                // Cloned targets cannot be cloned
                if (target->hasAuraWithAuraEffect(SPELL_AURA_MIRROR_IMAGE))
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
            default:
                break;
        }
    }

    if (m_targets.isTradeItem())
    {
        if (p_caster == nullptr)
            return SPELL_FAILED_SPELL_UNAVAILABLE;

        // Slot must be the lowest
        if (TradeSlots(m_targets.getItemTarget()) != TRADE_SLOT_NONTRADED)
            return SPELL_FAILED_ITEM_NOT_FOUND;

        // Check if player is even trading
        if (p_caster->getTradeData() == nullptr)
            return SPELL_FAILED_NOT_TRADING;

        // Cast the trade spell only when both parties have accepted the trade
        const auto tradeData = p_caster->getTradeData();
        if (!tradeData->isTradeAccepted() || !tradeData->getTargetTradeData()->isTradeAccepted())
        {
            // If either one hasn't accepted the trade, save the spell and cast it on trade complete
            tradeData->setTradeSpell(getSpellInfo()->getId(), i_caster);
            return SPELL_FAILED_DONT_REPORT;
        }
    }

    // Call legacy CanCast for yet unhandled cases
    return m_triggeredSpell || ProcedOnSpell != nullptr ? SPELL_CAST_SUCCESS : SpellCastResult(CanCast(secondCheck));
}

SpellCastResult Spell::checkPower()
{
    if (m_powerCost == 0)
        return SPELL_CAST_SUCCESS;

    if (p_caster != nullptr && p_caster->m_cheats.hasPowerCheat)
        return SPELL_CAST_SUCCESS;

    // Check if caster has enough health points if health is used for power
    if (getSpellInfo()->getPowerType() == POWER_TYPE_HEALTH)
    {
        if (u_caster != nullptr)
        {
            if (u_caster->getHealth() <= m_powerCost)
                return SPELL_FAILED_FIZZLE;
        }

        // No need to do any further checking
        return SPELL_CAST_SUCCESS;
    }

    // Invalid power types
    if (!getSpellInfo()->hasValidPowerType())
    {
        LogError("Spell::checkPower : Unknown power type %u for spell id %u", getSpellInfo()->getPowerType(), getSpellInfo()->getId());
        return SPELL_FAILED_ERROR;
    }

#if VERSION_STRING >= WotLK
    // Check runes for spells which have runes in power type
    if (getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
    {
        const auto runeResult = checkRunes(false);
        if (runeResult != SPELL_CAST_SUCCESS)
            return runeResult;
    }
#endif

    // Normal case
    if (u_caster != nullptr && u_caster->getPower(getSpellInfo()->getPowerType()) < m_powerCost)
        return SPELL_FAILED_NO_POWER;

    return SPELL_CAST_SUCCESS;
}

SpellCastResult Spell::checkItems(uint32_t* parameter1, uint32_t* parameter2) const
{
    // Skip for non-player casters
    if (p_caster == nullptr)
        return SPELL_CAST_SUCCESS;

    // If spell is casted from an enchant scroll
    auto scrollItem = false;
    // If spell is casted on an armor vellum or on a weapon vellum
    auto vellumTarget = false;

    // Casted by an item
    if (i_caster != nullptr)
    {
        if (!p_caster->hasItem(i_caster->getEntry()))
            return SPELL_FAILED_ITEM_GONE;

        // Check if the item is in trade window
        if (p_caster->getTradeData() != nullptr && p_caster->getTradeData()->hasPlayerOrTraderItemInTrade(i_caster->getGuid()))
            return SPELL_FAILED_NOT_WHILE_TRADING;

        const auto itemProperties = i_caster->getItemProperties();
        if (itemProperties == nullptr)
            return SPELL_FAILED_ITEM_GONE;

        // Check if the item is an enchant scroll
        if (itemProperties->Flags & ITEM_FLAG_ENCHANT_SCROLL)
            scrollItem = true;

        // Check if the item has any charges left
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (itemProperties->Spells[i].Charges > 0 && i_caster->getSpellCharges(i) == 0)
                return SPELL_FAILED_NO_CHARGES_REMAIN;
        }

#if VERSION_STRING < WotLK
        // Check zone
        if (itemProperties->ZoneNameID > 0 && itemProperties->ZoneNameID != p_caster->GetZoneId())
            return SPELL_FAILED_NOT_HERE;
        // Check map
        if (itemProperties->MapID > 0 && itemProperties->MapID != p_caster->GetMapId())
            return SPELL_FAILED_NOT_HERE;
#else
        // Check zone
        if (itemProperties->ZoneNameID > 0 && itemProperties->ZoneNameID != p_caster->GetZoneId())
            return SPELL_FAILED_INCORRECT_AREA;
        // Check map
        if (itemProperties->MapID > 0 && itemProperties->MapID != p_caster->GetMapId())
            return SPELL_FAILED_INCORRECT_AREA;
#endif

        if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
        {
            if (p_caster->CombatStatus.IsInCombat())
            {
                p_caster->getItemInterface()->buildInventoryChangeError(i_caster, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
                return SPELL_FAILED_DONT_REPORT;
            }
            else if (p_caster->IsMounted())
            {
                return SPELL_FAILED_NOT_MOUNTED;
            }
        }

        // Check health and power for consumables (potions, healthstones, mana items etc)
        if (itemProperties->Class == ITEM_CLASS_CONSUMABLE)
        {
            const auto targetUnit = p_caster->GetMapMgrUnit(m_targets.getUnitTarget());
            if (targetUnit != nullptr)
            {
                SpellCastResult errorMessage = SPELL_CAST_SUCCESS;
                for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    // Pet related effects are handled later
                    if (getSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
                        continue;

                    // +HP items
                    if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_HEAL)
                    {
                        // Check if target has full health
                        if (targetUnit->getHealthPct() == 100)
                        {
                            errorMessage = SPELL_FAILED_ALREADY_AT_FULL_HEALTH;
                            continue;
                        }
                        else
                        {
                            errorMessage = SPELL_CAST_SUCCESS;
                            break;
                        }
                    }

                    // +Mana/Power items
                    if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_ENERGIZE)
                    {
                        // Check if the spell has valid power type
                        if (getSpellInfo()->getEffectMiscValue(i) < 0
#if VERSION_STRING <= TBC
                            || getSpellInfo()->getEffectMiscValue(i) > POWER_TYPE_HAPPINESS)
#else
                            || getSpellInfo()->getEffectMiscValue(i) > POWER_TYPE_RUNIC_POWER)
#endif
                        {
                            errorMessage = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }

                        // Check if target has full powers
                        const auto powerType = static_cast<PowerType>(getSpellInfo()->getEffectMiscValue(i));
                        if (targetUnit->getPowerPct(powerType) == 100)
                        {
#if VERSION_STRING == Classic
                            errorMessage = SPELL_FAILED_ALREADY_AT_FULL_POWER;
#else
                            errorMessage = powerType == POWER_TYPE_MANA ? SPELL_FAILED_ALREADY_AT_FULL_MANA : SPELL_FAILED_ALREADY_AT_FULL_POWER;
#endif
                            continue;
                        }
                        else
                        {
                            errorMessage = SPELL_CAST_SUCCESS;
                            break;
                        }
                    }
                }

                if (errorMessage != SPELL_CAST_SUCCESS)
                    return errorMessage;
            }
        }

        // Check if item can be used while in shapeshift form
        if (p_caster->getShapeShiftForm() != FORM_NORMAL)
        {
            const auto shapeShift = sSpellShapeshiftFormStore.LookupEntry(p_caster->getShapeShiftForm());
            if (shapeShift != nullptr && !(shapeShift->Flags & 1))
            {
                if (!(i_caster->getItemProperties()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
                    return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;
            }
        }
    }

    // Casted on an item
    if (m_targets.getItemTarget() > 0)
    {
        Item const* targetItem = nullptr;
        // Check if the targeted item is in the trade window
        if (m_targets.isTradeItem())
        {
            // Only enchanting and lockpicking effects can be used in trade window
            if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_OPEN_LOCK ||
                getSpellInfo()->getEffect(0) == SPELL_EFFECT_ENCHANT_ITEM ||
                getSpellInfo()->getEffect(0) == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
            {
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_ENCHANT_OWN_ONLY)
                    return SPELL_FAILED_NOT_TRADEABLE;

                if (p_caster->getTradeTarget() != nullptr)
                    targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTarget()));
            }
            else
                return SPELL_FAILED_NOT_TRADEABLE;
        }
        else
        {
            targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
        }

        if (targetItem == nullptr)
            return SPELL_FAILED_ITEM_GONE;

        if (!targetItem->fitsToSpellRequirements(getSpellInfo()))
            return SPELL_FAILED_BAD_TARGETS;

        // Prevent exploiting (enchanting broken items and stacking them)
        if (targetItem->getDurability() == 0 && targetItem->getMaxDurability() != 0)
            return SPELL_FAILED_BAD_TARGETS;

        if ((getSpellInfo()->getEquippedItemClass() == ITEM_CLASS_ARMOR && targetItem->getItemProperties()->Class == ITEM_CLASS_TRADEGOODS && targetItem->getItemProperties()->SubClass == ITEM_SUBCLASS_ARMOR_ENCHANTMENT) ||
            (getSpellInfo()->getEquippedItemClass() == ITEM_CLASS_WEAPON && targetItem->getItemProperties()->Class == ITEM_CLASS_TRADEGOODS && targetItem->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_ENCHANTMENT))
            vellumTarget = true;
    }
    // Spell requires an item to be equipped
    else if (m_targets.getItemTarget() == 0 && getSpellInfo()->getEquippedItemClass() >= 0)
    {
        auto hasItemWithProperType = false;
        switch (getSpellInfo()->getEquippedItemClass())
        {
            // Spell requires a melee weapon or a ranged weapon
            case ITEM_CLASS_WEAPON:
            {
                for (int16_t i = EQUIPMENT_SLOT_MAINHAND; i <= EQUIPMENT_SLOT_RANGED; ++i)
                {
                    const auto inventoryItem = p_caster->getItemInterface()->GetInventoryItem(i);
                    if (inventoryItem != nullptr)
                    {
                        // Check if the weapon slot is disarmed
                        if ((i == EQUIPMENT_SLOT_MAINHAND && p_caster->hasUnitFlags(UNIT_FLAG_DISARMED))
#if VERSION_STRING >= TBC
                            || (i == EQUIPMENT_SLOT_OFFHAND && p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_OFFHAND))
                            || (i == EQUIPMENT_SLOT_RANGED && p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_RANGED))
#endif
                            )
                            continue;

                        // Check for proper item class and subclass
                        if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                        {
                            hasItemWithProperType = true;
                            break;
                        }
                    }
                }
            } break;
            // Spell requires an armor piece (like shield)
            case ITEM_CLASS_ARMOR:
            {
                // Check first if spell requires a shield equipped
                Item* inventoryItem;
                if (getSpellInfo()->getEquippedItemSubClass() & (1 << ITEM_SUBCLASS_ARMOR_SHIELD))
                {
                    inventoryItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (inventoryItem != nullptr)
                    {
#if VERSION_STRING >= TBC
                        // Check for offhand disarm
                        if (!p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_OFFHAND))
#endif
                        {
                            // Check for proper item class and subclass
                            if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                            {
                                hasItemWithProperType = true;
                                break;
                            }
                        }
                    }
                }

                // Check for other armor pieces
                for (int16_t i = EQUIPMENT_SLOT_HEAD; i < EQUIPMENT_SLOT_MAINHAND; ++i)
                {
                    inventoryItem = p_caster->getItemInterface()->GetInventoryItem(i);
                    if (inventoryItem != nullptr)
                    {
                        // Check for proper item class and subclass
                        if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                        {
                            hasItemWithProperType = true;
                            break;
                        }
                    }
                }

                // No need to check further if found already
                if (hasItemWithProperType)
                    break;

                // Ranged slot can have an item classified as armor (no need to check for disarm in these cases)
                inventoryItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (inventoryItem != nullptr)
                {
                    // Check for proper item class and subclass
                    if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                    {
                        hasItemWithProperType = true;
                        break;
                    }
                }
            } break;
            default:
                break;
        }

        if (!hasItemWithProperType)
        {
            *parameter1 = getSpellInfo()->getEquippedItemClass();
            *parameter2 = getSpellInfo()->getEquippedItemSubClass();
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }

        // Temporary helper lambda
        const auto hasEquippableWeapon = [&](Item const* weapon) -> bool
        {
            if (weapon == nullptr)
                return false;
            if (weapon->getItemProperties()->MaxDurability > 0 && weapon->getDurability() == 0)
                return false;
            return weapon->fitsToSpellRequirements(getSpellInfo());
        };

        // Check if spell explicitly requires a main hand weapon
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_MAIN_HAND_WEAPON)
        {
            if (!hasEquippableWeapon(p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND;
        }

        // Check if spell explicitly requires an offhand weapon
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON)
        {
            if (!hasEquippableWeapon(p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND)))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND;
        }
    }

    // Check if the spell requires any reagents or tools (skip enchant scrolls)
    if (i_caster == nullptr || !(i_caster->getItemProperties()->Flags & ITEM_FLAG_ENCHANT_SCROLL))
    {
        // Spells with ATTRIBUTESEXE_REAGENT_REMOVAL attribute won't take reagents if player has UNIT_FLAG_NO_REAGANT_COST flag
        auto checkForReagents = !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_REAGENT_REMOVAL && p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST));
        if (checkForReagents)
        {
#if VERSION_STRING >= WotLK
            // Check for spells which remove the reagent cost for a spell
            // e.g. Glyph of Slow Fall or Glyph of Levitate
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (getSpellInfo()->getSpellFamilyFlags(i) == 0)
                    continue;
                if (getSpellInfo()->getSpellFamilyFlags(i) & p_caster->getNoReagentCost(i))
                {
                    checkForReagents = false;
                    break;
                }
            }
#endif
        }
        // Reagents will always be checked for items in trade window
        else if (m_targets.getItemTarget() != 0 && m_targets.isTradeItem())
        {
            checkForReagents = true;
        }

        if (checkForReagents)
        {
            for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                if (getSpellInfo()->getReagent(i) == 0)
                    continue;

                const auto itemId = static_cast<uint32_t>(getSpellInfo()->getReagent(i));
                auto itemCount = getSpellInfo()->getReagentCount(i);

                // Some spells include the used item as one of the reagents
                // So in these cases itemCount must be incremented by one
                // e.g. item id 24502 requires 7 items but DBC data requires only 6, because the one missing item is the caster
                if (i_caster != nullptr && i_caster->getEntry() == itemId)
                {
                    const auto itemProperties = i_caster->getItemProperties();
                    for (uint8_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
                    {
                        if (itemProperties->Spells[x].Id == 0)
                            continue;
                        if (itemProperties->Spells[x].Charges == -1 && i_caster->getSpellCharges(x) <= 1)
                        {
                            ++itemCount;
                            break;
                        }
                    }
                }

                if (!p_caster->hasItem(itemId, itemCount))
                {
#if VERSION_STRING == Classic
                    //\ todo: figure out correct error message
                    return SPELL_FAILED_ITEM_NOT_READY;
#else
                    *parameter1 = itemId;
                    return SPELL_FAILED_REAGENTS;
#endif
                }
            }
        }

        // Check for totem items
        for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
        {
            if (getSpellInfo()->getTotem(i) != 0)
            {
                if (!p_caster->hasItem(getSpellInfo()->getTotem(i)))
                {
#if VERSION_STRING == Classic
                    //\ todo: figure out correct error message
                    return SPELL_FAILED_ITEM_NOT_READY;
#else
                    *parameter1 = getSpellInfo()->getTotem(i);
                    return SPELL_FAILED_TOTEMS;
#endif
                }
            }
        }

#if VERSION_STRING >= TBC
        // Check for totem category items
        for (uint8_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
        {
            if (getSpellInfo()->getTotemCategory(i) != 0 && !p_caster->getItemInterface()->hasItemForTotemCategory(getSpellInfo()->getTotemCategory(i)))
            {
                *parameter1 = getSpellInfo()->getTotemCategory(i);
                return SPELL_FAILED_TOTEM_CATEGORY;
            }
        }
#endif
    }

    // Special checks for different spell effects
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffect(i) == 0)
            continue;

        switch (getSpellInfo()->getEffect(i))
        {
            case SPELL_EFFECT_CREATE_ITEM:
            case SPELL_EFFECT_CREATE_ITEM2:
                if (getSpellInfo()->getEffectItemType(i) != 0)
                {
                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        LogError("Spell::checkItems: Spell entry %u has unknown item id (%u) in SPELL_EFFECT_CREATE_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_ERROR;
                    }

                    // Check if player has any free slots in the inventory
                    if (p_caster->getItemInterface()->CalculateFreeSlots(itemProperties) == 0)
                    {
                        p_caster->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                    
                    // Check other limitations
                    const auto itemErrorMessage = p_caster->getItemInterface()->CanReceiveItem(itemProperties, 1);
                    if (itemErrorMessage != INV_ERR_OK)
                    {
                        p_caster->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, itemErrorMessage, getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_DONT_REPORT;
                    }
                } break;
            case SPELL_EFFECT_ENCHANT_ITEM:
                // Check only for vellums here, normal checks are done in the next case
                if (getSpellInfo()->getEffectItemType(i) != 0 && m_targets.getItemTarget() != 0 && vellumTarget)
                {
                    // Player can only enchant their own vellums
                    if (m_targets.isTradeItem())
                        return SPELL_FAILED_NOT_TRADEABLE;
                    // Scrolls (enchanted vellums) cannot be enchanted into another vellum (duping)
                    if (scrollItem)
                        return SPELL_FAILED_BAD_TARGETS;

                    const auto vellumItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                    if (vellumItem == nullptr)
                        return SPELL_FAILED_ITEM_NOT_FOUND;
                    // Check if vellum is appropriate target for the enchant
                    if (getSpellInfo()->getBaseLevel() > vellumItem->getItemProperties()->ItemLevel)
                        return SPELL_FAILED_LOWLEVEL;

                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        LogError("Spell::checkItems: Spell entry %u has unknown item id (%u) in SPELL_EFFECT_ENCHANT_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_ERROR;
                    }

                    // Check if player has any free slots in the inventory
                    if (p_caster->getItemInterface()->CalculateFreeSlots(itemProperties) == 0)
                    {
                        p_caster->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    // Check other limitations
                    const auto itemErrorMessage = p_caster->getItemInterface()->CanReceiveItem(itemProperties, 1);
                    if (itemErrorMessage != INV_ERR_OK)
                    {
                        p_caster->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, itemErrorMessage, getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
            // no break here
            case SPELL_EFFECT_ADD_SOCKET:
            {
                if (m_targets.getItemTarget() == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item* targetItem = nullptr;
                if (m_targets.isTradeItem())
                {
                    if (p_caster->getTradeTarget() != nullptr)
                        targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTarget()));
                }
                else
                {
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                }

                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                // Check if the item's level is high enough for the enchantment
                if (targetItem->getItemProperties()->ItemLevel < getSpellInfo()->getBaseLevel())
                    return SPELL_FAILED_LOWLEVEL;

                auto hasOnUseEffect = false;
                const auto itemProperties = targetItem->getItemProperties();
                for (const auto& spell : itemProperties->Spells)
                {
                    if (spell.Id == 0)
                        continue;
                    if (spell.Trigger == USE || spell.Trigger == APPLY_AURA_ON_PICKUP)
                    {
                        hasOnUseEffect = true;
                        break;
                    }
                }

                const auto enchantEntry = sSpellItemEnchantmentStore.LookupEntry(getSpellInfo()->getEffectMiscValue(i));
                if (enchantEntry == nullptr)
                {
                    LogError("Spell::checkItems: Spell entry %u has no valid enchantment (%u)", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
                    return SPELL_FAILED_ERROR;
                }

                // Loop through enchantment's types
                for (const auto& type : enchantEntry->type)
                {
                    switch (type)
                    {
                        // todo: declare these in a header file and figure out other values
                        case 7: // Enchants 'on use' enchantment to item
                            // Check if the item already has a 'on use' enchantment
                            if (hasOnUseEffect)
                            {
#if VERSION_STRING < WotLK
                                return SPELL_FAILED_BAD_TARGETS;
#else
                                return SPELL_FAILED_ON_USE_ENCHANT;
#endif
                            }
                            break;
                        case 8: // Enchants a new prismatic socket slot to item
#if VERSION_STRING > TBC
                            // Check if the item already has a prismatic gem slot enchanted
                            if (targetItem->getEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0)
                            {
                                return SPELL_FAILED_ITEM_ALREADY_ENCHANTED;
                            }
#endif
                            // or if the item already has the maximum amount of socket slots
                            if (targetItem->GetSocketsCount() >= MAX_ITEM_PROTO_SOCKETS)
                            {
#if VERSION_STRING < WotLK
                                return SPELL_FAILED_BAD_TARGETS;
#else
                                return SPELL_FAILED_MAX_SOCKETS;
#endif
                            }
                            break;
                        default:
                            break;
                    }
                }

                // Check item owner in cases where enchantment makes item soulbound
                if (targetItem->getOwner() != p_caster)
                {
                    if (enchantEntry->EnchantGroups & 0x01) // Makes item soulbound
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            {
                if (m_targets.getItemTarget() == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item const* targetItem = nullptr;
                if (m_targets.isTradeItem())
                {
                    if (p_caster->getTradeTarget() != nullptr)
                        targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTarget()));
                }
                else
                {
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                }

                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                const auto enchantmentEntry = sSpellItemEnchantmentStore.LookupEntry(getSpellInfo()->getEffectMiscValue(i));
                if (enchantmentEntry == nullptr)
                {
                    LogError("Spell::checkItems: Spell entry %u has no valid enchantment (%u)", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
                    return SPELL_FAILED_ERROR;
                }

                // Check item owner in cases where enchantment makes item soulbound
                if (targetItem->getOwner() != p_caster)
                {
                    if (enchantmentEntry->EnchantGroups & 0x01) // Makes item soulbound
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_DISENCHANT:
            {
                if (m_targets.getItemTarget() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Only armor and weapon items can be disenchanted
                if (itemProperties->Class != ITEM_CLASS_ARMOR && itemProperties->Class != ITEM_CLASS_WEAPON)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                // Only items with uncommon, rare and epic quality can be disenchanted
                if (itemProperties->Quality > ITEM_QUALITY_EPIC_PURPLE || itemProperties->Quality < ITEM_QUALITY_UNCOMMON_GREEN)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                // Some items are not disenchantable
                if (itemProperties->DisenchantReqSkill <= 0)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
#if VERSION_STRING >= TBC
                // As of patch 2.0.1 disenchanting an item requires minimum skill level
                if (static_cast<uint32_t>(itemProperties->DisenchantReqSkill) > p_caster->_GetSkillLineCurrent(SKILL_ENCHANTING))
                    return SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL;
#endif
                // TODO: check does the item even have disenchant loot
                break;
            }
#if VERSION_STRING >= TBC
            case SPELL_EFFECT_PROSPECTING:
            {
                if (m_targets.getItemTarget() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is prospectable
                if (!(itemProperties->Flags & ITEM_FLAG_PROSPECTABLE))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                // Check if player has enough skill in Jewelcrafting
                if (itemProperties->RequiredSkillRank > p_caster->_GetSkillLineCurrent(SKILL_JEWELCRAFTING))
                {
                    *parameter1 = itemProperties->RequiredSkill;
                    *parameter2 = itemProperties->RequiredSkillRank;
                    return SPELL_FAILED_MIN_SKILL;
                }
                // Check if player has enough ores for prospecting
                if (!p_caster->hasItem(targetItem->getEntry(), 5))
                {
                    *parameter1 = targetItem->getEntry();
                    *parameter2 = 5;
#if VERSION_STRING == TBC
                    return SPELL_FAILED_PROSPECT_NEED_MORE;
#else
                    return SPELL_FAILED_NEED_MORE_ITEMS;
#endif
                }

                // TODO: check does the item even have prospecting loot
                break;
            }
#endif
#if VERSION_STRING >= WotLK
            case SPELL_EFFECT_MILLING:
            {
                if (m_targets.getItemTarget() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is millable
                if (!(itemProperties->Flags & ITEM_FLAG_MILLABLE))
                    return SPELL_FAILED_CANT_BE_MILLED;
                // Check if player has enough skill in Inscription
                if (itemProperties->RequiredSkillRank > p_caster->_GetSkillLineCurrent(SKILL_INSCRIPTION))
                {
                    *parameter1 = itemProperties->RequiredSkill;
                    *parameter2 = itemProperties->RequiredSkillRank;
                    return SPELL_FAILED_MIN_SKILL;
                }
                if (!p_caster->hasItem(targetItem->getEntry(), 5))
                {
                    *parameter1 = targetItem->getEntry();
                    *parameter2 = 5;
                    return SPELL_FAILED_NEED_MORE_ITEMS;
                }

                // TODO: check does the item even have milling loot
                break;
            }
#endif
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                // Check if spell is not ranged type
                if (getSpellInfo()->getDmgClass() != SPELL_DMG_TYPE_RANGED)
                    break;

                const auto rangedWeapon = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (rangedWeapon == nullptr || !rangedWeapon->isWeapon())
                    return SPELL_FAILED_EQUIPPED_ITEM;
                // Check if the item has any durability left
                if (rangedWeapon->getMaxDurability() > 0 && rangedWeapon->getDurability() == 0)
                    return SPELL_FAILED_EQUIPPED_ITEM;

#if VERSION_STRING <= WotLK
                // Check for ammunitation
                switch (rangedWeapon->getItemProperties()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                        // In classic throwing weapons use stack count
                        // In TBC throwing weapons use durability
                        // In Wotlk+ throwing weapons use neither
#if VERSION_STRING == Classic
                        if (p_caster->getItemInterface()->GetItemCount(rangedWeapon->getEntry()) == 0)
                            return SPELL_FAILED_NO_AMMO;
#elif VERSION_STRING == TBC
                        if (rangedWeapon->getDurability() == 0)
                            return SPELL_FAILED_NO_AMMO;
#endif
                        break;
                    // Check ammo for ranged weapons
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        // Thori'dal, the Stars' Fury has a dummy aura which makes it generate magical arrows
                        // iirc the only item with this kind of effect?
                        if (p_caster->m_requiresNoAmmo)
                            break;
                        const auto ammoId = p_caster->getAmmoId();
                        if (ammoId == 0)
                            return SPELL_FAILED_NEED_AMMO;

                        const auto ammoProperties = sMySQLStore.getItemProperties(ammoId);
                        if (ammoProperties == nullptr)
                            return SPELL_FAILED_NEED_AMMO;
                        if (ammoProperties->Class != ITEM_CLASS_PROJECTILE)
                            return SPELL_FAILED_NEED_AMMO;
                        if (ammoProperties->RequiredLevel > p_caster->getLevel())
                            return SPELL_FAILED_NEED_AMMO;

                        // Check for correct projectile type
                        if (rangedWeapon->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
                        {
                            if (ammoProperties->SubClass != ITEM_SUBCLASS_PROJECTILE_BULLET)
                                return SPELL_FAILED_NEED_AMMO;
                        }
                        else
                        {
                            if (ammoProperties->SubClass != ITEM_SUBCLASS_PROJECTILE_ARROW)
                                return SPELL_FAILED_NEED_AMMO;
                        }

                        // Check if player is out of ammos
                        if (!p_caster->hasItem(ammoId))
                        {
                            p_caster->setAmmoId(0);
                            return SPELL_FAILED_NO_AMMO;
                        }
                    } break;
                    default:
                        break;
                }
#endif
                break;
            }
            default:
                break;
        }
    }

    return SPELL_CAST_SUCCESS;
}

SpellCastResult Spell::checkCasterState() const
{
    // Skip for non-unit casters
    if (u_caster == nullptr)
        return SPELL_CAST_SUCCESS;

    // Spells with this attribute are casted regardless of caster's state or auras
    if (getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_IGNORE_CASTER_STATE_AND_AURAS)
        return SPELL_CAST_SUCCESS;

    // Spells that have following attributes should be casted regardless of caster's state
    // Includes tons of quest and achievement credit spells, and some battleground spells (flag drops, marks, honor spells)
    if (getSpellInfo()->getAttributes() & ATTRIBUTES_DEAD_CASTABLE &&
        getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT &&
        getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_HIGH_PRIORITY)
        return SPELL_CAST_SUCCESS;

    uint16_t schoolImmunityMask = 0, dispelImmunityMask = 0;
    uint32_t mechanicImmunityMask = 0;
    if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISPEL_AURAS_ON_IMMUNITY)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            switch (getSpellInfo()->getEffectApplyAuraName(i))
            {
                case SPELL_AURA_SCHOOL_IMMUNITY:
                    // This is already stored in bitmask
                    schoolImmunityMask |= static_cast<uint16_t>(getSpellInfo()->getEffectMiscValue(i));
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                    mechanicImmunityMask |= 1 << static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i) - 1);
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY_MASK:
                    mechanicImmunityMask |= static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i));
                    break;
                case SPELL_AURA_DISPEL_IMMUNITY:
                {
                    const uint16_t dispelMaskAll = (1 << DISPEL_MAGIC) | (1 << DISPEL_CURSE) | (1 << DISPEL_DISEASE) | (1 << DISPEL_POISON);
                    dispelImmunityMask |= getSpellInfo()->getEffectMiscValue(i) == DISPEL_ALL ? dispelMaskAll : static_cast<uint16_t>(1 << getSpellInfo()->getEffectMiscValue(i));
                } break;
                default:
                    break;
            }
        }

        // Check if the spell is a pvp trinket alike spell (removes all movement impairement and loss of control effects)
        if (getSpellInfo()->getEffectApplyAuraName(0) == SPELL_AURA_MECHANIC_IMMUNITY &&
            getSpellInfo()->getEffectMiscValue(0) == 1 &&
            getSpellInfo()->getEffectApplyAuraName(1) == 0 && getSpellInfo()->getEffectApplyAuraName(2) == 0)
            mechanicImmunityMask = MOVEMENT_IMPAIRMENTS_AND_LOSS_OF_CONTROL_MASK;
    }

    // Helper lambda for checking if spell has a mechanic
    const auto hasSpellMechanic = [](SpellInfo const* spellInfo, SpellMechanic mechanic) -> bool
    {
        if (spellInfo->getMechanicsType() == mechanic)
            return true;

        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->getEffectMechanic(i) == mechanic)
                return true;
        }

        return false;
    };

    SpellCastResult errorMsg = SPELL_CAST_SUCCESS;
    if (u_caster->hasUnitStateFlag(UNIT_STATE_STUN))
    {
        if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_STUNNED)
        {
            // Spell is usable while stunned, but there are some spells with stun effect which are not classified as normal stun spells
            for (const auto& stunAura : u_caster->m_auras)
            {
                if (stunAura == nullptr)
                    continue;
                if (!stunAura->getSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_MOD_STUN))
                    continue;

                // Frozen mechanic acts like stunned mechanic
                if (!hasSpellMechanic(stunAura->getSpellInfo(), MECHANIC_STUNNED)
                    && !hasSpellMechanic(stunAura->getSpellInfo(), MECHANIC_FROZEN))
                {
                    // The stun aura has a stun effect but has no stun or frozen mechanic
                    // This is not a normal stun aura
                    errorMsg = SPELL_FAILED_STUNNED;
                    break;
                }
            }
        }
        else
        {
            errorMsg = SPELL_FAILED_STUNNED;
        }
    }
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_CONFUSE) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_CONFUSED))
    {
        errorMsg = SPELL_FAILED_CONFUSED;
    }
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_FEAR) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_FEARED))
    {
        errorMsg = SPELL_FAILED_FLEEING;
    }
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_SILENCE) && getSpellInfo()->getPreventionType() == PREVENTION_TYPE_SILENCE)
    {
        errorMsg = SPELL_FAILED_SILENCED;
    }
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_PACIFY) && getSpellInfo()->getPreventionType() == PREVENTION_TYPE_PACIFY)
    {
        errorMsg = SPELL_FAILED_PACIFIED;
    }

    if (errorMsg != SPELL_CAST_SUCCESS)
    {
        if (schoolImmunityMask > 0 || dispelImmunityMask > 0 || mechanicImmunityMask > 0)
        {
            // The spell cast is prevented by some state but check if the spell is unaffected by those states or grants immunity to those states
            for (const auto& aur : u_caster->m_auras)
            {
                if (aur == nullptr)
                    continue;

                // Check if the spell, which is being casted, is unaffected by this aura due to school immunity
                if (aur->getSpellInfo()->getSchoolMask() & schoolImmunityMask && !(aur->getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_UNAFFECTED_BY_SCHOOL_IMMUNE))
                    continue;

                // Check if the spell, which is being casted, is unaffected by this aura due to dispel immunity
                if ((1 << aur->getSpellInfo()->getDispelType()) & dispelImmunityMask)
                    continue;

                for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (aur->getSpellInfo()->getEffectApplyAuraName(i) == 0)
                        continue;

                    // Get aura's mechanics in one mask
                    uint32_t mechanicMask = 0;
                    if (aur->getSpellInfo()->getMechanicsType() > 0)
                        mechanicMask |= 1 << (aur->getSpellInfo()->getMechanicsType() - 1);
                    if (aur->getSpellInfo()->getEffectMechanic(i) > 0)
                        mechanicMask |= 1 << (aur->getSpellInfo()->getEffectMechanic(i) - 1);

                    // Check if the spell, which is being casted, is unaffected by this aura due to mechanic immunity
                    if (mechanicMask & mechanicImmunityMask)
                        continue;

                    // Spell cast is prevented by this aura and by this effect index, return correct error message
                    switch (aur->getSpellInfo()->getEffectApplyAuraName(i))
                    {
                        case SPELL_AURA_MOD_STUN:
                            if (!(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_STUNNED) || !hasSpellMechanic(getSpellInfo(), MECHANIC_STUNNED))
                                return SPELL_FAILED_STUNNED;
                            break;
                        case SPELL_AURA_MOD_CONFUSE:
                            if (!(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_CONFUSED))
                                return SPELL_FAILED_CONFUSED;
                            break;
                        case SPELL_AURA_MOD_FEAR:
                            if (!(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_FEARED))
                                return SPELL_FAILED_FLEEING;
                            break;
                        case SPELL_AURA_MOD_SILENCE:
                        case SPELL_AURA_MOD_PACIFY:
                        case SPELL_AURA_MOD_PACIFY_SILENCE:
                            if (getSpellInfo()->getPreventionType() == PREVENTION_TYPE_SILENCE)
                                return SPELL_FAILED_SILENCED;
                            if (getSpellInfo()->getPreventionType() == PREVENTION_TYPE_PACIFY)
                                return SPELL_FAILED_PACIFIED;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        else
        {
            // Spell cast is prevented by some state and the spell does not grant immunity to that state
            return errorMsg;
        }
    }

    return SPELL_CAST_SUCCESS;
}

SpellCastResult Spell::checkRange(const bool secondCheck)
{
    const auto rangeEntry = sSpellRangeStore.LookupEntry(getSpellInfo()->getRangeIndex());
    if (rangeEntry == nullptr)
        return SPELL_CAST_SUCCESS;

    // Players can activate "on next attack" abilities before being at melee range
    if (!secondCheck && getSpellInfo()->isOnNextMeleeAttack())
        return SPELL_CAST_SUCCESS;

    auto targetUnit = m_caster->GetMapMgrUnit(m_targets.getUnitTarget());

    // Self cast spells don't need range check
    if (getSpellInfo()->getRangeIndex() == 1 || targetUnit == m_caster)
        return SPELL_CAST_SUCCESS;

    if (p_caster != nullptr)
    {
        // If pet is the effect target, check range to pet
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getSpellInfo()->getEffectImplicitTargetA(i) != EFF_TARGET_PET)
                continue;

            if (p_caster->GetSummon() != nullptr)
            {
                targetUnit = p_caster->GetSummon();
                break;
            }
        }
    }

    float_t minRange = 0.0f, maxRange = 0.0f, rangeMod = 0.0f;
    if (rangeEntry->range_type & SPELL_RANGE_TYPE_MASK_MELEE)
    {
        if (u_caster != nullptr)
        {
            // Use caster's combat reach if target is not found
            const float_t combatReach = targetUnit != nullptr ? targetUnit->getCombatReach() : u_caster->getCombatReach();
            // Caster's combat reach + 1.333... + target's (or caster's again) combat reach
            rangeMod = std::max(5.0f, u_caster->getCombatReach() + (4.0f / 3.0f) + combatReach);
        }
    }
    else
    {
        if (u_caster != nullptr && rangeEntry->range_type & SPELL_RANGE_TYPE_MASK_RANGED)
        {
            // Use caster's combat reach if target is not found
            const float_t combatReach = targetUnit != nullptr ? targetUnit->getCombatReach() : u_caster->getCombatReach();
            // Caster's combat reach + 1.33f + target's (or caster's again) combat reach
            minRange = std::max(5.0f, u_caster->getCombatReach() + (4.0f / 3.0f) + combatReach);
        }

        // Get minimum range
#if VERSION_STRING < WotLK
        minRange += rangeEntry->minRange;
#else
        if (targetUnit == nullptr)
            minRange += rangeEntry->minRange;
        else
            minRange += isFriendly(m_caster, targetUnit) ? rangeEntry->minRangeFriendly : rangeEntry->minRange;
#endif

        // Get maximum range
#if VERSION_STRING < WotLK
        maxRange = rangeEntry->maxRange;
#else
        if (targetUnit == nullptr)
            maxRange = rangeEntry->maxRange;
        else
            maxRange = isFriendly(m_caster, targetUnit) ? rangeEntry->maxRangeFriendly : rangeEntry->maxRange;
#endif

        // Player, creature or corpse target
        if (u_caster != nullptr && (targetUnit != nullptr || m_targets.getTargetMask() & (TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2)))
        {
            const float_t combatReach = targetUnit != nullptr ? targetUnit->getCombatReach() : u_caster->getCombatReach();
            rangeMod = u_caster->getCombatReach() + combatReach;
            if (minRange > 0.0f && !(rangeEntry->range_type & SPELL_RANGE_TYPE_MASK_RANGED))
                minRange += rangeMod;
        }
    }

    // Spell leeway - Client increases spell's range if the caster is moving (walking is not accounted)
    ///\ todo: Needs retesting after movement system has been rewritten
    if (u_caster != nullptr && u_caster->hasUnitMovementFlag(MOVEFLAG_MOVING_MASK) && !u_caster->hasUnitMovementFlag(MOVEFLAG_WALK))
    {
        // Leeway mechanic also depends on target - target also needs to be moving (again, walking is not accounted)
        if (targetUnit != nullptr && targetUnit->hasUnitMovementFlag(MOVEFLAG_MOVING_MASK) && !targetUnit->hasUnitMovementFlag(MOVEFLAG_WALK)
            && (rangeEntry->range_type & SPELL_RANGE_TYPE_MASK_MELEE || targetUnit->isPlayer()))
            rangeMod += 8.0f / 3.0f; // 2.6666... yards
    }

    // Add range from ranged weapon to max range
    if (p_caster != nullptr && getSpellInfo()->getAttributes() & ATTRIBUTES_RANGED)
    {
        const auto rangedWeapon = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        if (rangedWeapon != nullptr)
            maxRange *= rangedWeapon->getItemProperties()->Range * 0.01f;
    }

    // Add 5 yards to range check for spell landing because some spells have a delay before landing
    rangeMod += secondCheck ? (m_caster->isPlayer() ? 6.25f : 2.25f) : (m_caster->isPlayer() ? 1.25f : 0.0f);

    // Apply spell modifiers to range
    if (u_caster != nullptr)
        u_caster->applySpellModifiers(SPELLMOD_RANGE, &maxRange, getSpellInfo(), this);

    maxRange += rangeMod;

    // Square values for range check
    minRange *= minRange;
    maxRange *= maxRange;

    if (targetUnit != nullptr && targetUnit != m_caster)
    {
        const float_t distance = m_caster->getDistanceSq(targetUnit);
        if (minRange > 0.0f && distance < minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (distance > maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
    }

    // AoE spells on targeted location
    if (m_targets.hasDestination())
    {
        const float_t distance = m_caster->getDistanceSq(m_targets.getDestination());
        if (minRange > 0.0f && distance < minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (distance > maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
    }

    return SPELL_CAST_SUCCESS;
}

#if VERSION_STRING >= WotLK
SpellCastResult Spell::checkRunes(bool takeRunes)
{
    // Check only for players and for spells which have rune cost
    if (getSpellInfo()->getRuneCostID() > 0 && p_caster != nullptr)
    {
        // Classes other than Death Knights should not cast spells which use runes
        if (!p_caster->isClassDeathKnight())
            return SPELL_FAILED_NO_POWER;

        const auto spellRuneCost = sSpellRuneCostStore.LookupEntry(getSpellInfo()->getRuneCostID());
        if (spellRuneCost != nullptr && (spellRuneCost->bloodRuneCost > 0 || spellRuneCost->frostRuneCost > 0 || spellRuneCost->unholyRuneCost > 0))
        {
            int32_t runeCost[3];
            runeCost[RUNE_BLOOD] = spellRuneCost->bloodRuneCost;
            runeCost[RUNE_FROST] = spellRuneCost->frostRuneCost;
            runeCost[RUNE_UNHOLY] = spellRuneCost->unholyRuneCost;

            // Apply modifers
            for (uint8_t i = 0; i < RUNE_DEATH; ++i)
            {
                p_caster->applySpellModifiers(SPELLMOD_COST, &runeCost[i], getSpellInfo(), this);
            }

            if (const auto dkPlayer = dynamic_cast<DeathKnight*>(p_caster))
            {
                // Get available runes and subtract them from the power cost
                // If the outcome is over zero, it means player doesn't have enough runes available
                auto missingRunes = dkPlayer->HasRunes(RUNE_BLOOD, runeCost[RUNE_BLOOD]) + dkPlayer->HasRunes(RUNE_FROST, runeCost[RUNE_FROST]) + dkPlayer->HasRunes(RUNE_UNHOLY, runeCost[RUNE_UNHOLY]);
                // If there aren't enough normal runes available, try death runes
                if (missingRunes > 0 && dkPlayer->HasRunes(RUNE_DEATH, missingRunes) > 0)
                    return SPELL_FAILED_NO_POWER;

                if (takeRunes)
                {
                    missingRunes = dkPlayer->TakeRunes(RUNE_BLOOD, runeCost[RUNE_BLOOD]) + dkPlayer->TakeRunes(RUNE_FROST, runeCost[RUNE_FROST]) + dkPlayer->TakeRunes(RUNE_UNHOLY, runeCost[RUNE_UNHOLY]);
                    if (missingRunes > 0 && dkPlayer->TakeRunes(RUNE_DEATH, missingRunes) > 0)
                        return SPELL_FAILED_NO_POWER;

                    // Death knights gains some runic power when using runes
                    if (spellRuneCost->runePowerGain > 0)
                    {
                        const auto runicPowerAmount = static_cast<uint32_t>((spellRuneCost->runePowerGain + dkPlayer->getPower(POWER_TYPE_RUNIC_POWER)) * worldConfig.getFloatRate(RATE_POWER7));
                        dkPlayer->setPower(POWER_TYPE_RUNIC_POWER, runicPowerAmount);
                    }
                }
            }
        }
    }

    return SPELL_CAST_SUCCESS;
}
#endif

SpellCastResult Spell::checkShapeshift(SpellInfo const* spellInfo, const uint32_t shapeshiftForm) const
{
    // No need to check requirements for talents that learn spells
    uint8_t talentRank = 0;
    const auto talentInfo = sTalentStore.LookupEntry(spellInfo->getId());
    if (talentInfo != nullptr)
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            if (talentInfo->RankID[i] != 0)
                talentRank = i + 1;
        }
    }

    // This is client-side only
    if (talentRank > 0 && spellInfo->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        return SPELL_CAST_SUCCESS;

    const uint32_t stanceMask = shapeshiftForm ? 1 << (shapeshiftForm - 1) : 0;

    // Cannot explicitly be casted in this stance/form
    if (spellInfo->getShapeshiftExclude() > 0 && spellInfo->getShapeshiftExclude() & stanceMask)
        return SPELL_FAILED_NOT_SHAPESHIFT;

    // Can explicitly be casted in this stance/form
    if (spellInfo->getRequiredShapeShift() > 0 && spellInfo->getRequiredShapeShift() & stanceMask)
        return SPELL_CAST_SUCCESS;

    auto actAsShifted = false;
    if (stanceMask > FORM_NORMAL)
    {
        auto shapeShift = sSpellShapeshiftFormStore.LookupEntry(shapeshiftForm);
        if (shapeShift == nullptr)
        {
            LogError("Spell::checkShapeshift: Caster has unknown shapeshift form %u", shapeshiftForm);
            return SPELL_CAST_SUCCESS;
        }

        // Check if shapeshift acts as normal form for spells
        actAsShifted = !(shapeShift->Flags & 1);
    }

    if (actAsShifted)
    {
        // Cannot be casted while shapeshifted
        if (spellInfo->getAttributes() & ATTRIBUTES_NOT_SHAPESHIFT)
            return SPELL_FAILED_NOT_SHAPESHIFT;
        // Needs another shapeshift form
        else if (spellInfo->getRequiredShapeShift() != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // Check if spell even requires shapeshift
        if (!(spellInfo->getAttributesExB() & ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT) && spellInfo->getRequiredShapeShift() != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    return SPELL_CAST_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spell packets
void Spell::sendCastResult(SpellCastResult result, uint32_t parameter1 /*= 0*/, uint32_t parameter2 /*= 0*/)
{
    if (result == SPELL_CAST_SUCCESS)
        return;

    SetSpellFailed();

    if (!m_caster->IsInWorld())
        return;

    Player* plr = p_caster;
    if (plr == nullptr && u_caster != nullptr)
        plr = u_caster->m_redirectSpellPackets;
    if (plr == nullptr)
        return;

    sendCastResult(plr, 0, result, parameter1, parameter2);
}

void Spell::sendChannelUpdate(const uint32_t time, const uint32_t diff/* = 0*/)
{
    if (time == 0)
    {
        if (u_caster != nullptr)
        {
            const auto channelGuid = u_caster->getChannelObjectGuid();

            // Make sure last periodic tick happens
            if (diff > 0)
            {
                const auto casterGuid = u_caster->getGuid();
                const auto aur = u_caster->getAuraWithIdForGuid(getSpellInfo()->getId(), casterGuid);
                const auto target = u_caster->GetMapMgrUnit(channelGuid);

                if (aur != nullptr)
                    aur->update(diff, true);

                if (target != nullptr)
                {
                    const auto targetAur = target->getAuraWithIdForGuid(getSpellInfo()->getId(), casterGuid);
                    if (targetAur != nullptr)
                        targetAur->update(diff, true);
                }
            }

            const auto dynamicObject = u_caster->GetMapMgrDynamicObject(WoWGuid::getGuidLowPartFromUInt64(channelGuid));
            if (dynamicObject != nullptr)
                dynamicObject->Remove();

            u_caster->setChannelObjectGuid(0);
            u_caster->setChannelSpellId(0);

            // Remove temporary summons which were created by this channeled spell (i.e Eye of Kilrogg)
            if (p_caster != nullptr && p_caster->getCharmGuid() != 0 && getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON))
            {
                const auto charmedUnit = p_caster->GetMapMgrUnit(p_caster->getCharmGuid());
                if (charmedUnit != nullptr && charmedUnit->getCreatedBySpellId() == getSpellInfo()->getId())
                    p_caster->UnPossess();
            }

            // Channel ended, remove the aura
            //\ todo: if aura is stackable, need to remove only one stack from aura instead of whole aura!
            u_caster->RemoveAura(getSpellInfo()->getId(), u_caster->getGuid());
        }
    }

    m_caster->SendMessageToSet(MsgChannelUpdate(m_caster->GetNewGUID(), time).serialise().get(), true);
}

void Spell::sendSpellStart()
{
    if (!m_caster->IsInWorld())
        return;

    // If spell has no visuals, it's not channeled and it's triggered, no need to send packet
    if (!(getSpellInfo()->isChanneled() || getSpellInfo()->getSpeed() > 0.0f || getSpellInfo()->getSpellVisual(0) != 0 ||
        getSpellInfo()->getSpellVisual(1) != 0 || (!m_triggeredSpell && m_triggeredByAura == nullptr)))
        return;

    // Not sure about the size -Appled
    WorldPacket data(SMSG_SPELL_START, 30);

    // Set cast flags
    uint32_t castFlags = SPELL_PACKET_FLAGS_DEFAULT;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        castFlags |= SPELL_PACKET_FLAGS_RANGED;

#if VERSION_STRING >= WotLK
    // Power update for players and their summons
    if ((p_caster != nullptr || (u_caster != nullptr && u_caster->getPlayerOwner() != nullptr)) &&
        getSpellInfo()->getPowerType() != POWER_TYPE_HEALTH)
        castFlags |= SPELL_PACKET_FLAGS_POWER_UPDATE;
#endif

#if VERSION_STRING >= Cata
    // Health update for healing spells
    ///\ todo: fix me!
    /*for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if ((m_castTime > 0 && getSpellInfo()->getEffect(i) == SPELL_EFFECT_HEAL) ||
            getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL)
        {
            castFlags |= SPELL_PACKET_FLAGS_HEALTH_UPDATE;
            break;
        }
    }*/
#endif

    if (i_caster != nullptr)
        data << i_caster->GetNewGUID();
    else
        data << m_caster->GetNewGUID();
    data << m_caster->GetNewGUID();

#if VERSION_STRING >= WotLK
    data << uint8_t(extra_cast_number);
    data << uint32_t(getSpellInfo()->getId());
    data << uint32_t(castFlags);
#else
    data << uint32_t(getSpellInfo()->getId());
#if VERSION_STRING != Classic
    data << uint8_t(extra_cast_number);
#endif
    data << uint16_t(castFlags);
#endif
    data << uint32_t(m_timer);
#if VERSION_STRING >= Cata
    data << uint32_t(m_castTime);
#endif

    m_targets.write(data);

#if VERSION_STRING >= WotLK
    if (castFlags & SPELL_PACKET_FLAGS_POWER_UPDATE && u_caster != nullptr)
        data << uint32_t(u_caster->getPower(getSpellInfo()->getPowerType()));
#endif

    if (castFlags & SPELL_PACKET_FLAGS_RANGED)
        writeProjectileDataToPacket(&data);

    m_caster->SendMessageToSet(&data, true);
}

void Spell::sendSpellGo()
{
    if (!m_caster->IsInWorld())
        return;

    // If spell has no visuals and it's not channeled and it's triggered, no need to send packet
    if (!(getSpellInfo()->isChanneled() || getSpellInfo()->getSpeed() > 0.0f || getSpellInfo()->getSpellVisual(0) != 0 ||
        getSpellInfo()->getSpellVisual(1) != 0 || (!m_triggeredSpell && m_triggeredByAura == nullptr)))
        return;

    // Size should be enough
    WorldPacket data(SMSG_SPELL_GO, 60);

    // Set cast flags
    uint32_t castFlags = 0;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        castFlags |= SPELL_PACKET_FLAGS_RANGED;

    if (i_caster != nullptr)
        castFlags |= SPELL_PACKET_FLAGS_ITEM_CASTER;

    if (!missedTargets.empty())
        castFlags |= SPELL_PACKET_FLAGS_EXTRA_MESSAGE;

#if VERSION_STRING >= WotLK
    if (m_missileTravelTime != 0)
        castFlags |= SPELL_PACKET_FLAGS_UPDATE_MISSILE;

    // Rune update
    uint8_t currentRunes = 0;
    if (p_caster != nullptr && p_caster->isClassDeathKnight())
    {
        // Get current available runes in bitmask
        currentRunes = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();

        if (getSpellInfo()->getRuneCostID() > 0 || currentRunes != m_rune_avail_before)
            castFlags |= SPELL_PACKET_FLAGS_RUNE_UPDATE | SPELL_PACKET_FLAGS_UNK40000;
    }

    // Power update for players and their summons
    if ((p_caster != nullptr || (u_caster != nullptr && u_caster->getPlayerOwner() != nullptr)) &&
        getSpellInfo()->getPowerType() != POWER_TYPE_HEALTH)
        castFlags |= SPELL_PACKET_FLAGS_POWER_UPDATE;
#endif

    if (i_caster != nullptr)
        data << i_caster->GetNewGUID();
    else
        data << m_caster->GetNewGUID();
    data << m_caster->GetNewGUID();

#if VERSION_STRING >= WotLK
    data << uint8_t(extra_cast_number);
    data << uint32_t(getSpellInfo()->getId());
    data << uint32_t(castFlags);
#else
    data << uint32_t(getSpellInfo()->getId());
    data << uint16_t(castFlags);
#endif
#if VERSION_STRING >= Cata
    data << uint32_t(m_timer);
#endif
#if VERSION_STRING != Classic
    data << uint32_t(Util::getMSTime());
#endif

    // Add hitted targets
    data << uint8_t(uniqueHittedTargets.size());
    for (const auto& uniqueTarget : uniqueHittedTargets)
    {
        data << uint64_t(uniqueTarget.first);
    }

    // Add missed targets
    if (castFlags & SPELL_PACKET_FLAGS_EXTRA_MESSAGE)
    {
        data << uint8_t(missedTargets.size());
        writeSpellMissedTargets(&data);
    }
    else
    {
        data << uint8_t(0);
    }

    m_targets.write(data);

#if VERSION_STRING >= WotLK
    if (castFlags & SPELL_PACKET_FLAGS_POWER_UPDATE && u_caster != nullptr)
        data << uint32_t(u_caster->getPower(getSpellInfo()->getPowerType()));
#endif

    if (castFlags & SPELL_PACKET_FLAGS_RANGED)
        writeProjectileDataToPacket(&data);

#if VERSION_STRING >= WotLK
    //data order depending on flags : 0x800, 0x200000, 0x20000, 0x20, 0x80000, 0x40 (this is not spellgoflag but seems to be from spellentry or packet..)
    //.text:00401110                 mov     eax, [ecx+14h] -> them
    //.text:00401115                 cmp     eax, [ecx+10h] -> us
    if (castFlags & SPELL_PACKET_FLAGS_RUNE_UPDATE)
    {
        data << uint8_t(m_rune_avail_before);
        data << uint8_t(currentRunes);
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            const uint8_t runeMask = 1 << i;
            if ((runeMask & m_rune_avail_before) != (runeMask & currentRunes))
                data << uint8_t(0); // Value of the rune converted into byte. We just think it is 0 but maybe it is not
        }
    }

    if (castFlags & SPELL_PACKET_FLAGS_UPDATE_MISSILE)
    {
        data << float(m_missilePitch);
        data << uint32_t(m_missileTravelTime);
    }
#endif

    // Some spells require this
    if (m_targets.hasDestination())
        data << uint8_t(0);

    m_caster->SendMessageToSet(&data, true);
}

void Spell::sendChannelStart(const uint32_t duration)
{
    m_caster->SendMessageToSet(MsgChannelStart(m_caster->GetNewGUID(), getSpellInfo()->getId(), duration).serialise().get(), true);

    Object const* channelTarget = nullptr;
    if (!uniqueHittedTargets.empty())
    {
        // Select first target from uniqueHittedTargets
        // brief: the channel target is properly set in SpellEffects.cpp for persistent dynamic objects
        for (const auto& targetGuid : uniqueHittedTargets)
        {
            const auto targetUnit = m_caster->GetMapMgrUnit(targetGuid.first);
            if (targetUnit != nullptr)
            {
                channelTarget = targetUnit;
                break;
            }

            const auto objTarget = m_caster->GetMapMgrGameObject(targetGuid.first);
            if (objTarget != nullptr)
            {
                channelTarget = objTarget;
                break;
            }
        }
    }

    if (u_caster != nullptr)
    {
        u_caster->setChannelSpellId(getSpellInfo()->getId());
        if (channelTarget != nullptr)
            u_caster->setChannelObjectGuid(channelTarget->getGuid());
    }

    m_castTime = m_timer = duration;
}

void Spell::sendCastResult(Player* caster, uint8_t castCount, SpellCastResult result, uint32_t parameter1, uint32_t parameter2)
{
    if (caster == nullptr)
        return;

    // Include missing parameters to error messages
    switch (result)
    {
        case SPELL_FAILED_ONLY_SHAPESHIFT:
            if (parameter1 == 0)
                parameter1 = getSpellInfo()->getRequiredShapeShift();
            break;
        case SPELL_FAILED_REQUIRES_AREA:
            if (parameter1 == 0)
            {
#if VERSION_STRING == TBC
                parameter1 = getSpellInfo()->getRequiresAreaId();
#elif VERSION_STRING >= WotLK
                // Send the first area id from areagroup to player
                auto areaGroup = sAreaGroupStore.LookupEntry(getSpellInfo()->getRequiresAreaId());
                for (const auto& areaId : areaGroup->AreaId)
                {
                    if (areaId != 0)
                    {
                        parameter1 = areaId;
                        break;
                    }
                }
#endif
            } break;
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND:
            if (parameter1 == 0 && parameter2 == 0)
            {
                parameter1 = getSpellInfo()->getEquippedItemClass();
                parameter2 = getSpellInfo()->getEquippedItemSubClass();
            } break;
#if VERSION_STRING >= TBC
        case SPELL_FAILED_REAGENTS:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    if (getSpellInfo()->getReagent(i) == 0)
                        continue;
                    if (!caster->hasItem(getSpellInfo()->getReagent(i), getSpellInfo()->getReagentCount(i)))
                    {
                        parameter1 = getSpellInfo()->getReagent(i);
                        break;
                    }
                }
            } break;
        case SPELL_FAILED_TOTEMS:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
                {
                    if (getSpellInfo()->getTotem(i) == 0)
                        continue;
                    if (!caster->hasItem(getSpellInfo()->getTotem(i)))
                    {
                        parameter1 = getSpellInfo()->getTotem(i);
                        break;
                    }
                }
            } break;
        case SPELL_FAILED_TOTEM_CATEGORY:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                {
                    if (getSpellInfo()->getTotemCategory(i) == 0)
                        continue;
                    if (!caster->getItemInterface()->hasItemForTotemCategory(getSpellInfo()->getTotemCategory(i)))
                    {
                        parameter1 = getSpellInfo()->getTotemCategory(i);
                        break;
                    }
                }
            } break;
#endif
        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
            if (parameter1 == 0)
                parameter1 = getSpellInfo()->getRequiresSpellFocus();
            break;
        default:
            break;
    }

    caster->sendCastFailedPacket(getSpellInfo()->getId(), result, castCount, parameter1, parameter2);
}

void Spell::writeProjectileDataToPacket(WorldPacket *data)
{
    ItemProperties const* ammoItem = nullptr;
#if VERSION_STRING < Cata
    if (p_caster != nullptr)
    {
        const auto rangedItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        if (rangedItem != nullptr)
        {
            if (getSpellInfo()->getId() == SPELL_RANGED_THROW)
            {
                ammoItem = rangedItem->getItemProperties();
            }
            else
            {
                if (p_caster->getAmmoId() != 0)
                {
                    ammoItem = sMySQLStore.getItemProperties(p_caster->getAmmoId());
                }
                else
                {
                    // Use Rough Arrow if ammo id is not found
                    ammoItem = sMySQLStore.getItemProperties(2512);
                }
            }
        }
    }
    else if (u_caster != nullptr)
    {
        // Get creature's ranged weapon
        // Need to loop through all weapon slots because NPCs can have the ranged weapon in main hand
        for (uint8_t i = 0; i <= RANGED; ++i)
        {
            const auto entryId = u_caster->getVirtualItemSlotId(i);
            if (entryId == 0)
                continue;

#if VERSION_STRING < WotLK
            //\ todo: fix for classic and tbc
            // hackfixing Rough Arrow for projectile
            ammoItem = sMySQLStore.getItemProperties(2512);
#else
            // Get the item data from DBC files
            const auto itemDBC = sItemStore.LookupEntry(entryId);
            if (itemDBC == nullptr || itemDBC->Class != ITEM_CLASS_WEAPON)
                continue;

            switch (itemDBC->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    // Use Rough Arrow for bows
                    ammoItem = sMySQLStore.getItemProperties(2512);
                    break;
                case ITEM_SUBCLASS_WEAPON_GUN:
                    // Use Light Shot for guns
                    ammoItem = sMySQLStore.getItemProperties(2516);
                    break;
                case ITEM_SUBCLASS_WEAPON_THROWN:
                    ammoItem = sMySQLStore.getItemProperties(entryId);
                    break;
                default:
                    break;
            }
#endif

            // No need to continue if ammo has been found
            if (ammoItem != nullptr)
                break;
        }
    }
#endif

    if (ammoItem != nullptr)
    {
        *data << uint32_t(ammoItem->DisplayInfoID);
        *data << uint32_t(ammoItem->InventoryType);
    }
    else
    {
        *data << uint32_t(0);
        *data << uint32_t(0);
    }
}

void Spell::writeSpellMissedTargets(WorldPacket *data)
{
    if (u_caster != nullptr && u_caster->isAlive())
    {
        for (const auto& target : missedTargets)
        {
            *data << uint64_t(target.targetGuid);
            *data << uint8_t(target.hitResult);
            // Need to send hit result for the reflected spell
            if (target.hitResult == SPELL_DID_HIT_REFLECT)
                *data << uint8_t(target.extendedHitResult);

            const auto targetUnit = u_caster->GetMapMgrUnit(target.targetGuid);
            if (targetUnit != nullptr && targetUnit->isAlive())
            {
                targetUnit->CombatStatusHandler_ResetPvPTimeout(); // aaa
                u_caster->CombatStatusHandler_ResetPvPTimeout(); // bbb
            }
        }
    }
    else
    {
        for (const auto& target : missedTargets)
        {
            *data << uint64_t(target.targetGuid);
            *data << uint8_t(target.hitResult);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Cast time
int32_t Spell::getFullCastTime() const
{
    return m_castTime;
}

int32_t Spell::getCastTimeLeft() const
{
    return m_timer;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Power
uint32_t Spell::getPowerCost() const
{
    return m_powerCost;
}

void Spell::takePower()
{
    // Items do not use caster's power and only units have power
    if (i_caster != nullptr || u_caster == nullptr || m_triggeredByAura != nullptr)
        return;

    if (p_caster != nullptr && p_caster->m_cheats.hasPowerCheat)
        return;

    if (getSpellInfo()->getPowerType() == POWER_TYPE_HEALTH)
    {
        //\ todo: is this correct order?
        u_caster->sendSpellNonMeleeDamageLog(u_caster, u_caster, getSpellInfo(), getPowerCost(), 0, 0, 0, 0, false, false);
        u_caster->dealDamage(u_caster, getPowerCost(), getSpellInfo()->getId(), false);
        return;
    }
#if VERSION_STRING >= WotLK
    else if (getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
    {
        checkRunes(true);
        return;
    }
#endif
    // Check also that the caster's power type is mana
    // otherwise spell procs, which use no power but have default power type (= mana), will set this to true
    // and that will show for example rage inaccurately later
    else if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA && u_caster->getPowerType() == POWER_TYPE_MANA)
    {
        // Start five second timer later at spell cast
        m_usesMana = true;
    }

    if (!getSpellInfo()->hasValidPowerType())
    {
        LogError("Spell::takePower : Unknown power type %u for spell id %u", getSpellInfo()->getPowerType(), getSpellInfo()->getId());
        return;
    }

    const auto powerType = getSpellInfo()->getPowerType();
    u_caster->modPower(powerType, -static_cast<int32_t>(getPowerCost()));
}

uint32_t Spell::calculatePowerCost()
{
    // Null for non-unit casters
    if (u_caster == nullptr)
        return 0;

    auto powerCost = getSpellInfo()->getBasePowerCost(u_caster);

    // Use first school found from mask
    const auto spellSchool = getSpellInfo()->getFirstSchoolFromSchoolMask();

    // Include power cost modifiers from that school
    powerCost += u_caster->getPowerCostModifier(spellSchool);

    // Special case for rogue's Shiv - power cost depends on the speed of offhand weapon
    if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_SHIV)
    {
        // Formula seems to be 20 + offhand weapon speed in seconds * 10
        powerCost += u_caster->getBaseAttackTime(OFFHAND) / 100;
    }

    // Apply modifiers
    u_caster->applySpellModifiers(SPELLMOD_COST, &powerCost, getSpellInfo(), this);

    // Include power cost multipliers from that school
    powerCost = static_cast<int32_t>(powerCost * (1.0f + u_caster->getPowerCostMultiplier(spellSchool)));

    if (powerCost < 0)
        powerCost = 0;

    return static_cast<uint32_t>(powerCost);
}
//////////////////////////////////////////////////////////////////////////////////////////
// Caster
Object* Spell::getCaster() const
{
    return m_caster;
}

Unit* Spell::getUnitCaster() const
{
    return u_caster;
}

Player* Spell::getPlayerCaster() const
{
    return p_caster;
}

GameObject* Spell::getGameObjectCaster() const
{
    return g_caster;
}

Item* Spell::getItemCaster() const
{
    return i_caster;
}

void Spell::setItemCaster(Item* itemCaster)
{
    i_caster = itemCaster;
}

bool Spell::wasCastedinDuel() const
{
    return duelSpell;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
SpellInfo const* Spell::getSpellInfo() const
{
    return m_spellInfo_override != nullptr ? m_spellInfo_override : m_spellInfo;
}

Aura* Spell::getTriggeredByAura() const
{
    return m_triggeredByAura;
}

void Spell::addUsedSpellModifier(AuraEffectModifier const* aurEff)
{
    for (const auto& usedMod : m_usedModifiers)
    {
        if (usedMod.first == aurEff)
            return;
    }

    m_usedModifiers.insert(std::make_pair(aurEff, false));
}

void Spell::removeUsedSpellModifier(AuraEffectModifier const* aurEff)
{
    for (auto& usedMod : m_usedModifiers)
    {
        // Mark the spell modifier as removed to prevent memory corruption
        if (usedMod.first == aurEff)
        {
            usedMod.second = true;
            break;
        }
    }

    // Also, remove the spell modifier from pending auras
    for (auto& pendingAur : m_pendingAuras)
    {
        if (pendingAur.second.aur != nullptr)
            pendingAur.second.aur->removeUsedSpellModifier(aurEff);
    }
}

void Spell::takeUsedSpellModifiers()
{
    if (m_usedModifiers.empty())
        return;

    for (auto itr = m_usedModifiers.begin(); itr != m_usedModifiers.end();)
    {
        auto aurEff = (*itr).first;
        // Check for faulty entry
        if (aurEff->getAura() == nullptr || (*itr).second)
        {
            itr = m_usedModifiers.erase(itr);
            continue;
        }

        aurEff->getAura()->removeCharge();
        itr = m_usedModifiers.erase(itr);
    }
}

void Spell::setForceCritOnTarget(Unit const* target)
{
    if (target == nullptr || target->GetMapMgr() == nullptr)
        return;

    m_critTargets.push_back(target->getGuid());
}

bool Spell::canAttackCreatureType(Creature* target) const
{
    // Skip check for Grounding Totem
    if (target->getCreatedBySpellId() == 8177)
        return true;

    const auto typeMask = getSpellInfo()->getTargetCreatureType();
    const auto mask = 1 << (target->GetCreatureProperties()->Type - 1);
    return !(target->GetCreatureProperties()->Type != 0 && typeMask != 0 && (typeMask & mask) == 0);
}

void Spell::removeReagents()
{
    if (p_caster == nullptr)
        return;

    if (!(p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST) && getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_REAGENT_REMOVAL))
    {
        for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            if (getSpellInfo()->getReagent(i) == 0)
                continue;

            p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(getSpellInfo()->getReagent(i), getSpellInfo()->getReagentCount(i), &i_caster);
        }
    }
}

#if VERSION_STRING < Cata
void Spell::removeAmmo()
{
    if (p_caster == nullptr)
        return;

    // Remove ammo only if spell is a ranged spell
    if (!(getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED && (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS || getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_PLAYER_RANGED_SPELLS)))
        return;

    const auto rangedWeapon = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (rangedWeapon == nullptr)
        return;

    if (rangedWeapon->getItemProperties()->InventoryType == INVTYPE_THROWN)
    {
        // In classic throwing weapons use stack count
        // In TBC throwing weapons use durability
        // In Wotlk+ throwing weapons use neither
#if VERSION_STRING == Classic
        // Remove 1 weapon from the stack amount
        p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(rangedWeapon->getEntry(), 1, &i_caster);
#elif VERSION_STRING == TBC
        // Remove 1 point from durability
        if (rangedWeapon->getDurability() > 0)
        {
            rangedWeapon->setDurability(rangedWeapon->getDurability() - 1);
            if (rangedWeapon->getDurability() == 0)
                p_caster->ApplyItemMods(rangedWeapon, EQUIPMENT_SLOT_RANGED, false, true);
        }
#endif
    }
    else
    {
        if (!p_caster->m_requiresNoAmmo)
            p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(p_caster->getAmmoId(), 1, &i_caster);
    }
}
#endif

void Spell::_updateCasterPointers(Object* caster)
{
    p_caster = nullptr;
    u_caster = nullptr;
    i_caster = nullptr;
    g_caster = nullptr;

    m_caster = caster;
    switch (caster->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            p_caster = dynamic_cast<Player*>(caster);
        // no break here
        case TYPEID_UNIT:
            u_caster = dynamic_cast<Unit*>(caster);
            break;
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            i_caster = dynamic_cast<Item*>(caster);
            break;
        case TYPEID_GAMEOBJECT:
            g_caster = dynamic_cast<GameObject*>(caster);
            break;
        default:
            LogDebugFlag(LF_SPELL, "Spell::_updateCasterPointers : Incompatible object type (type %u) for spell caster", caster->getObjectTypeId());
            break;
    }
}

void Spell::_updateTargetPointers(const uint64_t targetGuid)
{
    unitTarget = nullptr;
    itemTarget = nullptr;
    gameObjTarget = nullptr;
    playerTarget = nullptr;
    corpseTarget = nullptr;

    if (targetGuid == 0)
    {
        if (getPlayerCaster() != nullptr)
        {
            if (m_targets.getTargetMask() & TARGET_FLAG_ITEM)
                itemTarget = getPlayerCaster()->getItemInterface()->GetItemByGUID(m_targets.getItemTarget());

            if (m_targets.isTradeItem())
            {
                const auto trader = getPlayerCaster()->getTradeTarget();
                if (trader != nullptr)
                    itemTarget = trader->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTarget()));
            }
        }
    }
    else if (targetGuid == getCaster()->getGuid())
    {
        unitTarget = getUnitCaster();
        itemTarget = getItemCaster();
        gameObjTarget = getGameObjectCaster();
        playerTarget = getPlayerCaster();
    }
    else
    {
        if (!getCaster()->IsInWorld())
            return;

        if (m_targets.isTradeItem())
        {
            if (getPlayerCaster() != nullptr)
            {
                const auto trader = getPlayerCaster()->getTradeTarget();
                if (trader != nullptr)
                    itemTarget = trader->getTradeData()->getTradeItem(TradeSlots(targetGuid));
            }
        }
        else
        {
            WoWGuid wowGuid;
            wowGuid.Init(targetGuid);

            switch (wowGuid.getHigh())
            {
                case HighGuid::Unit:
                case HighGuid::Vehicle:
                    unitTarget = getCaster()->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Pet:
                    unitTarget = getCaster()->GetMapMgr()->GetPet(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Player:
                    unitTarget = getCaster()->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart());
                    playerTarget = dynamic_cast<Player*>(unitTarget);
                    break;
                case HighGuid::Item:
                    if (getPlayerCaster() != nullptr)
                        itemTarget = getPlayerCaster()->getItemInterface()->GetItemByGUID(targetGuid);
                    break;
                case HighGuid::GameObject:
                    gameObjTarget = getCaster()->GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Corpse:
                    corpseTarget = sObjectMgr.GetCorpse(wowGuid.getGuidLowPart());
                    break;
                default:
                    LogError("Spell::_updateTargetPointers : Invalid object type for spell target (low guid %u) in spell %u", wowGuid.getGuidLowPart(), getSpellInfo()->getId());
                    break;
            }
        }
    }
}

float_t Spell::_getSpellTravelTimeForTarget(uint64_t guid) const
{
    // Handle instant spells instantly
    if (getSpellInfo()->getSpeed() == 0)
        return 0.0f;

    float_t destX = 0.0f, destY = 0.0f, destZ = 0.0f, distance = 0.0f;

    // Use destination only if the spell has no unit target mask set
    if (m_targets.hasDestination() && !(m_targets.getTargetMask() & TARGET_FLAG_UNIT))
    {
        const auto dest = m_targets.getDestination();
        destX = dest.x;
        destY = dest.y;
        destZ = dest.z;

        distance = m_caster->CalcDistance(destX, destY, destZ);
    }
    else if (guid == 0)
    {
        return -1.0f;
    }
    else
    {
        if (!m_caster->IsInWorld())
            return -1.0f;

        if (m_caster->getGuid() != guid)
        {
            const auto obj = m_caster->GetMapMgrObject(guid);
            if (obj == nullptr)
                return -1.0f;

            destX = obj->GetPositionX();
            destY = obj->GetPositionY();
            //\todo this should be destz = obj->GetPositionZ() + (obj->GetModelHighBoundZ() / 2 * obj->getScale())
            if (obj->isCreatureOrPlayer())
                destZ = obj->GetPositionZ() + static_cast<Unit*>(obj)->GetModelHalfSize();
            else
                destZ = obj->GetPositionZ();

            distance = m_caster->CalcDistance(destX, destY, destZ);
        }
    }

    if (distance == 0.0f)
        return 0.0f;

    if (m_missileTravelTime != 0)
        return static_cast<float_t>(m_missileTravelTime);

    // Calculate time it takes for spell to hit target
    return static_cast<float_t>((distance * 1000.0f) / getSpellInfo()->getSpeed());
}

void Spell::_prepareProcFlags()
{
    // Skip melee spells
    if (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_MELEE || getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED)
        return;

    // Setup spell target mask
    uint32_t spellTargetMask = 0;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffect(i) == 0)
            continue;

        // Skip targets on following effects
        // Procs for these effects are handled when the real spell is casted
        if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_DUMMY ||
            getSpellInfo()->getEffect(i) == SPELL_EFFECT_SCRIPT_EFFECT ||
            getSpellInfo()->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL ||
            getSpellInfo()->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE)
            continue;

        spellTargetMask |= getSpellInfo()->getRequiredTargetMaskForEffect(i);
    }

    if (spellTargetMask == 0)
        return;

    // Consider the spell negative if it requires an attackable target
    const auto spellDamageType = getSpellInfo()->getDmgClass();
    if (spellTargetMask & SPELL_TARGET_REQUIRE_ATTACKABLE)
    {
        if (spellDamageType == SPELL_DMG_TYPE_NONE)
        {
            m_casterProcFlags = PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_NONE;
            m_targetProcFlags = PROC_ON_TAKEN_NEGATIVE_SPELL_DAMAGE_CLASS_NONE;
        }
        else if (spellDamageType == SPELL_DMG_TYPE_MAGIC)
        {
            m_casterProcFlags = PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC;
            m_targetProcFlags = PROC_ON_TAKEN_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC;
        }
    }
    else
    {
        if (spellDamageType == SPELL_DMG_TYPE_NONE)
        {
            m_casterProcFlags = PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE;
            m_targetProcFlags = PROC_ON_TAKEN_POSITIVE_SPELL_DAMAGE_CLASS_NONE;
        }
        else if (spellDamageType == SPELL_DMG_TYPE_MAGIC)
        {
            m_casterProcFlags = PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_MAGIC;
            m_targetProcFlags = PROC_ON_TAKEN_POSITIVE_SPELL_DAMAGE_CLASS_MAGIC;
        }
    }
}
