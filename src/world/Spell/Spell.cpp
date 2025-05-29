/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.hpp"

#include "SpellAura.hpp"
#include "SpellTarget.h"
#include "SpellInfo.hpp"
#include "Definitions/AuraInterruptFlags.hpp"
#include "Definitions/AuraStates.hpp"
#include "Definitions/CastInterruptFlags.hpp"
#include "Definitions/ChannelInterruptFlags.hpp"
#include "Definitions/DispelType.hpp"
#include "Definitions/LockTypes.hpp"
#include "Definitions/PreventionType.hpp"
#include "Definitions/ProcFlags.hpp"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "Definitions/SpellDamageType.hpp"
#include "Definitions/SpellDidHitResult.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Definitions/SpellEffectTarget.hpp"
#include "Definitions/SpellFamily.hpp"
#include "Definitions/SpellInFrontStatus.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Definitions/SpellPacketFlags.hpp"
#include "Definitions/SpellState.hpp"
#include "Definitions/SpellRanged.hpp"
#include "Definitions/SummonControlTypes.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/Loot/LootMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Management/ItemInterface.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Object.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/GameObject.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Objects/Item.hpp"
#include "Server/Definitions.h"
#include "Server/Packets/SmsgCancelCombat.h"
#include "Server/Packets/MsgChannelUpdate.h"
#include "Server/Packets/MsgChannelStart.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Players/TradeData.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Script/HookInterface.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

using namespace AscEmu::Packets;

extern pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS];

Spell::Spell(Object* _caster, SpellInfo const* _spellInfo, bool _triggered, Aura* _aura)
{
    if (_caster == nullptr)
    {
        sLogger.failure("Spell::Spell cant initialize without caster!");
        return;
    }

    if (_spellInfo == nullptr)
    {
        sLogger.failure("Spell::Spell cant initialize without valid spell info!");
        return;
    }

    _caster->m_pendingSpells.insert(this);
    chaindamage = 0;
    damage = 0;

    m_DelayStep = 0;

    m_AreaAura = false;

    damageToHit = 0;
    castedItemId = 0;

    m_Spell_Failed = false;

    add_damage = 0;
    m_Delayed = false;
    pSpellId = 0;
    ProcedOnSpell = nullptr;
    extra_cast_number = 0;
    m_glyphslot = 0;
    m_charges = _spellInfo->getProcCharges();

    // create rune avail snapshot
    if (p_caster && p_caster->isClassDeathKnight())
        m_rune_avail_before = static_cast<DeathKnight*>(p_caster)->GetRuneFlags();
    else
        m_rune_avail_before = 0;

    m_targetConstraint = sSpellMgr.getSpellTargetConstraintForSpell(_spellInfo->getId());

    m_missilePitch = 0;
    m_missileTravelTime = 0;
    m_IsCastedOnSelf = false;
    m_magnetTarget = 0;

    m_spellInfo = _spellInfo;

    // Get spell difficulty
    if (_spellInfo->getSpellDifficultyID() != 0 && _caster->getObjectTypeId() != TYPEID_PLAYER && _caster->getWorldMap() != nullptr)
    {
        auto SpellDiffEntry = sSpellMgr.getSpellInfoByDifficulty(_spellInfo->getSpellDifficultyID(), _caster->getWorldMap()->getDifficulty());
        if (SpellDiffEntry != nullptr)
            m_spellInfo = SpellDiffEntry;
    }

    // Initialize caster pointers
    _updateCasterPointers(_caster);

    // Check if spell is casted in a duel
    switch (_caster->getObjectTypeId())
    {
    case TYPEID_PLAYER:
    case TYPEID_UNIT:
        if (u_caster && u_caster->getPlayerOwnerOrSelf() != nullptr && u_caster->getPlayerOwnerOrSelf()->getDuelState() == DUEL_STATE_STARTED)
            duelSpell = true;
        break;
    case TYPEID_ITEM:
    case TYPEID_CONTAINER:
        if (i_caster->getOwner() != nullptr && i_caster->getOwner()->getDuelState() == DUEL_STATE_STARTED)
            duelSpell = true;
        break;
    case TYPEID_GAMEOBJECT:
        if (g_caster->getPlayerOwner() != nullptr && g_caster->getPlayerOwner()->getDuelState() == DUEL_STATE_STARTED)
            duelSpell = true;
        break;
    default:
        break;
    }

    if (u_caster && getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_CAST_BY_CHARMER)
    {
        const auto unitCharmer = u_caster->getWorldMapUnit(u_caster->getCharmedByGuid());
        if (unitCharmer != nullptr)
        {
            u_caster = unitCharmer;
            if (unitCharmer->isPlayer())
                p_caster = dynamic_cast<Player*>(unitCharmer);
        }
    }

    m_triggeredSpell = _triggered;
    m_triggeredByAura = _aura;
    if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_TRIGGERED)
        m_triggeredSpell = true;

    m_requiresCP = getSpellInfo()->getAttributesEx() & (ATTRIBUTESEX_REQ_COMBO_POINTS1 | ATTRIBUTESEX_REQ_COMBO_POINTS2);

    m_uniqueHittedTargets.clear();
    m_missedTargets.clear();

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        isEffectDamageStatic[i] = false;
        effectPctModifier[i] = 1.0f;

        m_effectTargets[i].clear();
    }

    // Check if spell is reflectable
    if (getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_MAGIC && !getSpellInfo()->isPassive() &&
        !(getSpellInfo()->getAttributes() & ATTRIBUTES_ABILITY) && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_BE_REFLECTED) &&
        !(getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
    {
        //\ todo: this is not correct but it works for now
        //\ need to check for effect rather than target type
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            switch (getSpellInfo()->getEffectImplicitTargetA(i))
            {
                case EFF_TARGET_SINGLE_ENEMY:
                case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
                case EFF_TARGET_IN_FRONT_OF_CASTER:
                case EFF_TARGET_DUEL:
                    m_canBeReflected = true;
                    break;
                default:
                    break;
            }

            if (m_canBeReflected)
                break;
        }
    }

    forced_basepoints = std::make_shared<SpellForcedBasePoints>();
}

Spell::~Spell()
{
#if VERSION_STRING >= WotLK
    // If this spell deals with rune power, send spell_go to update client
    // For instance, when Dk cast Empower Rune Weapon, if we don't send spell_go, the client won't update
    if (getSpellInfo()->getFirstSchoolFromSchoolMask() && getSpellInfo()->getPowerType() == POWER_TYPE_RUNES)
        sendSpellGo();
#endif

    m_caster->m_pendingSpells.erase(this);

    ///////////////////////////// This is from the virtual_destructor shit ///////////////
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_caster->getCurrentSpell(CurrentSpellType(i)) == this)
            m_caster->interruptSpellWithSpellType(CurrentSpellType(i));
    }

    if (m_spellInfo_override)
        delete[] m_spellInfo_override;
    ////////////////////////////////////////////////////////////////////////////////////////


    for (auto& effectTarget : m_effectTargets)
        effectTarget.clear();

    m_uniqueHittedTargets.clear();
    m_missedTargets.clear();

    m_hitEffects.clear();
    m_missEffects.clear();
    m_critTargets.clear();

    m_usedModifiers.clear();
    m_pendingAuras.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Main control flow
SpellCastResult Spell::prepare(SpellCastTargets* targets)
{
    if (!m_caster->IsInWorld())
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Object {} is casting spell ID {} while not in world", std::to_string(m_caster->getGuid()), getSpellInfo()->getId());
        delete this;
        return SPELL_FAILED_DONT_REPORT;
    }

    //\ todo: handle this in creature AI...
    if (u_caster != nullptr)
    {
        if (u_caster->isCreature() || u_caster->isMoving())
        {
            if (u_caster->hasUnitStateFlag(UNIT_STATE_FLEEING))
            {
                u_caster->addGarbageSpell(this);
                return SPELL_FAILED_NOT_READY;
            }
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
        if (p_caster->m_cannibalize)
        {
            sEventMgr.RemoveEvents(p_caster, EVENT_CANNIBALIZE);
            p_caster->setEmoteState(EMOTE_ONESHOT_NONE);
            p_caster->m_cannibalize = false;
        }
    }

    // Check if spell is disabled
    if (sSpellMgr.isSpellDisabled(getSpellInfo()->getId()))
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
        m_castTime = static_cast<int32_t>(GetCastTime(sSpellCastTimesStore.lookupEntry(getSpellInfo()->getCastingTimeIndex())));
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
                if (tradeItem != nullptr && tradeItem->getGuid() == m_targets.getItemTargetGuid())
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

    // Item spells or triggered spells should not require combo points
    if (m_triggeredSpell || i_caster != nullptr)
        m_requiresCP = false;

    _loadInitialTargetPointers();

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
            m_triggeredByAura->removeAura();
        }

        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::prepare : canCast result {} for spell id {} (refer to SpellFailure.hpp to work out why)", cancastresult, getSpellInfo()->getId());

        finish(false);
        return cancastresult;
    }

    m_timer = m_castTime;

    _loadInitialTargetPointers(true);

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
        else
            m_spellState = SPELL_STATE_CASTING;
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
    m_spellState = SPELL_STATE_CASTED;

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
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::castMe : Player guid {} casted spell {} (id {})",
            plr->getGuidLow(), getSpellInfo()->getName(), getSpellInfo()->getId());
    }
    else if (m_caster->isCreature())
    {
        const auto creature = static_cast<Creature*>(m_caster);
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::castMe : Creature guid {} (entry {}) casted spell {} (id {})",
            creature->spawnid, creature->getEntry(), getSpellInfo()->getName(), getSpellInfo()->getId());
    }
    else
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::castMe : Spell id {} casted, caster guid {}", getSpellInfo()->getId(), m_caster->getGuid());
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
            const auto targetObj = m_caster->getWorldMapObject(m_targets.getUnitTargetGuid());
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
        const auto magnetTarget = m_caster->getWorldMapUnit(m_magnetTarget);
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
        if (u_caster != nullptr)
        {
            if (m_triggeredByAura == nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NOT_BREAK_STEALTH))
            {
                // we're much better to remove this here, because otherwise spells that change powers etc,
                // don't get applied.
                u_caster->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
                u_caster->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_CAST);
            }

            u_caster->setOnMeleeSpell(getSpellInfo()->getId(), extra_cast_number);
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
            for (const auto& uniqueTarget : m_uniqueHittedTargets)
            {
                if (uniqueTarget.first == targetGuid)
                {
                    add = false;
                    break;
                }
            }

            if (add)
                m_uniqueHittedTargets.push_back(std::make_pair(targetGuid, DamageInfo()));
        }
    }

    // Cleanup missed targets; spell either hits or misses target, not both
    // Current spell target system is bullshit
    if (!m_missedTargets.empty())
    {
        for (const auto& targetGuid : m_uniqueHittedTargets)
        {
            auto missedTarget = m_missedTargets.begin();
            while (missedTarget != m_missedTargets.end())
            {
                if (missedTarget->targetGuid == targetGuid.first)
                    missedTarget = m_missedTargets.erase(missedTarget);
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
        if (getDuration() > 0)
        {
            channelDuration = static_cast<uint32_t>(getDuration());
            sendChannelStart(channelDuration);
            m_spellState = SPELL_STATE_CHANNELING;
        }
        else if (getDuration() == -1)
        {
            sendChannelStart(static_cast<uint32_t>(getDuration()));
            m_spellState = SPELL_STATE_CHANNELING;
        }
    }

    // Set cooldown on item after SMSG_SPELL_GO
    if (i_caster != nullptr && i_caster->getOwner() != nullptr && !GetSpellFailed())
    {
        // Potion cooldown starts after leaving combat
        if (i_caster->getItemProperties()->Class == ITEM_CLASS_CONSUMABLE && i_caster->getItemProperties()->SubClass == ITEM_SUBCLASS_POTION)
        {
            i_caster->getOwner()->setLastPotion(i_caster->getItemProperties()->ItemId);
            i_caster->getOwner()->updatePotionCooldown();
        }
        else
        {
            for (uint8_t spellIndex = 0; spellIndex < MAX_ITEM_PROTO_SPELLS; ++spellIndex)
            {
                const auto& itemSpell = i_caster->getItemProperties()->Spells[spellIndex];
                if (itemSpell.Id != 0 && itemSpell.Trigger == USE)
                    i_caster->getOwner()->cooldownAddItem(i_caster->getItemProperties(), spellIndex);
            }
        }
    }

    // Take cast item after SMSG_SPELL_GO but before effect handling
    if (!GetSpellFailed())
        removeCastItem();

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
        u_caster->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_CAST_SPELL, getSpellInfo()->getId());
        u_caster->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_CAST);
    }

    // Prepare proc flags for caster and targets
    _prepareProcFlags();
    m_casterDamageInfo.schoolMask = SchoolMask(getSpellInfo()->getSchoolMask());

    // Loop through spell effects and process the spell effect on each target
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        const auto spellEffect = getSpellInfo()->getEffect(i);
        if (spellEffect == SPELL_EFFECT_NULL)
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
            const auto& pendingAur = *itr;
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
    for (const auto& missedTarget : m_missedTargets)
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
                const auto refundCost = Util::float2int32(getPowerCost() * 0.82f);
                p_caster->modPower(POWER_TYPE_ENERGY, refundCost);
            }
            // Druids and warriors are refunded for 82% of the rage cost on dodge or parry
            else if (getSpellInfo()->getPowerType() == POWER_TYPE_RAGE &&
                (targetDodged || targetParried))
            {
                const auto refundCost = Util::float2int32(getPowerCost() * 0.82f);
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
}

void Spell::handleHittedTarget(const uint64_t targetGuid, uint8_t effIndex)
{
    const auto travelTime = _getSpellTravelTimeForTarget(targetGuid);
    if (travelTime < 0)
        return;

    _updateTargetPointers(targetGuid);
    const auto effDamage = calculateEffect(effIndex);

    // Enter combat or keep combat alive if spell had at least one target that was either
    // a) hostile
    // b) friendly who was in combat
    // TODO: confirm if instant kill spells should skip combat (atm needed for .kill command)
    if (getUnitCaster() != nullptr && getUnitTarget() != nullptr && getUnitCaster()->getGuid() != getUnitTarget()->getGuid()
        && getSpellInfo()->getEffect(effIndex) != SPELL_EFFECT_INSTANT_KILL)
    {
        // Combat is applied instantly to caster if spell had cast time and target is hostile
        // Instant spells on hostile targets and all spells on friendly targets will have combat delayed
        if (getUnitCaster()->isFriendlyTo(getUnitTarget()))
            getUnitCaster()->getCombatHandler().onFriendlyAction(getUnitTarget());
        else if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
            getUnitCaster()->getCombatHandler().onHostileAction(getUnitTarget(), getFullCastTime() > 0);
    }

    // If effect applies an aura, create it instantly but add it later to target
    if (getSpellInfo()->doesEffectApplyAura(effIndex))
    {
        handleHittedEffect(targetGuid, effIndex, effDamage);

        // Add travel time to aura
        auto itr = m_pendingAuras.find(targetGuid);
        if (itr != m_pendingAuras.end())
            itr->second.travelTime = Util::float2int32(travelTime);

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
        hitEffect.travelTime = Util::float2int32(travelTime);

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

    // TODO: in the future, consider having two damage variables; one for integer and one for float
    damage = effDamage;

    // Skip auras here
    // TODO: confirm if instant kill spells should skip combat (atm needed for .kill command)
    if (!getSpellInfo()->doesEffectApplyAura(effIndex) && getUnitCaster() != nullptr && getUnitTarget() != nullptr && getUnitCaster() != getUnitTarget()
        && getSpellInfo()->getEffect(effIndex) != SPELL_EFFECT_INSTANT_KILL)
    {
        if (getUnitCaster()->isFriendlyTo(getUnitTarget()))
        {
            getUnitTarget()->getCombatHandler().takeCombatAction(getUnitCaster(), true);
        }
        else if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
        {
            // Add initial threat
            // Real threat is sent in damage code, in heal code or in apply aura code
            if (getUnitTarget()->isCreature())
                getUnitTarget()->getAIInterface()->onHostileAction(getUnitCaster());

            // Target should enter combat when spell lands on target
            getUnitTarget()->getCombatHandler().takeCombatAction(getUnitCaster());
        }
    }

    // Clear DamageInfo before effect
    m_targetDamageInfo = DamageInfo();
    m_targetDamageInfo.schoolMask = SchoolMask(getSpellInfo()->getSchoolMask());
    isTargetDamageInfoSet = false;
    isForcedCrit = false;

    const auto effectId = getSpellInfo()->getEffect(effIndex);
    if (effectId >= TOTAL_SPELL_EFFECTS)
    {
        sLogger.failure("Spell::handleHittedEffect : Unknown spell effect {} in spell id {}, index {}", effectId, getSpellInfo()->getId(), effIndex);
        return;
    }

    sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::handleHittedEffect : Spell effect {}, spell id {}, damage {}", effectId, getSpellInfo()->getId(), damage);

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
        for (auto& uniqueTarget : m_uniqueHittedTargets)
        {
            if (uniqueTarget.first == targetGuid)
                uniqueTarget.second = m_targetDamageInfo;
        }
    }

    // Check if target's procs have been handled in spell effects
    // If not, they will be processed in Spell::finish
    // Caster procs are also handled in Spell::finish
    if (m_targetDamageInfo.victimProcFlags != PROC_NULL)
        m_doneTargetProcs.insert(targetGuid);

    if (m_uniqueHittedTargets.size() == 1)
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
    DoAfterHandleEffect(getUnitTarget(), effIndex);
}

void Spell::handleMissedTarget(SpellTargetMod const missedTarget)
{
    // No need to handle this target if it was in evade mode
    if (missedTarget.hitResult == SPELL_DID_HIT_EVADE)
        return;

    const auto didReflect = missedTarget.hitResult == SPELL_DID_HIT_REFLECT && missedTarget.extendedHitResult == SPELL_DID_HIT_SUCCESS;

    auto travelTime = _getSpellTravelTimeForTarget(missedTarget.targetGuid);
    if (travelTime < 0)
        return;

    _updateTargetPointers(missedTarget.targetGuid);

    // Enter combat or keep combat alive if spell had at least one target that was either
    // a) hostile
    // b) friendly who was in combat
    if (getUnitCaster() != nullptr && getUnitTarget() != nullptr)
    {
        // Combat is applied instantly to caster if spell had cast time and target is hostile
        // Instant spells on hostile targets and all spells on friendly targets will have combat delayed
        if (getUnitCaster()->isFriendlyTo(getUnitTarget()))
            getUnitCaster()->getCombatHandler().onFriendlyAction(getUnitTarget());
        else if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
            getUnitCaster()->getCombatHandler().onHostileAction(getUnitTarget(), getFullCastTime() > 0);
    }

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
            handleMissedEffect(missedTarget);
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
                    hitEffect.travelTime = Util::float2int32(travelTime);

                    m_hitEffects.insert(std::make_pair(guid, hitEffect));
                }
            }
        }
        else
        {
            // Spell was not reflected and it did not hit target
            MissSpellEffect missEffect;
            missEffect.missInfo = missedTarget;
            missEffect.travelTime = Util::float2int32(travelTime);

            m_missEffects.insert(std::make_pair(missedTarget.targetGuid, missEffect));
        }
    }
}

void Spell::handleMissedEffect(SpellTargetMod const missedTarget, bool reCheckTarget/* = false*/)
{
    if (reCheckTarget)
        _updateTargetPointers(missedTarget.targetGuid);

    // Spell was not reflected and it did not hit target
    if (getUnitTarget() != nullptr)
    {
        if (getUnitCaster() != nullptr)
        {
            if (getUnitCaster()->isFriendlyTo(getUnitTarget()))
            {
                getUnitTarget()->getCombatHandler().takeCombatAction(getUnitCaster(), true);
            }
            else if (!(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
            {
                // Let target creature know that someone tried to cast spell on it
                if (getUnitTarget()->isCreature())
                    getUnitTarget()->getAIInterface()->onHostileAction(getUnitCaster());

                // Target should enter combat when spell lands on target
                getUnitTarget()->getCombatHandler().takeCombatAction(getUnitCaster());
            }
        }

        // Call scripted after spell missed hook
        sScriptMgr.callScriptedSpellAfterMiss(this, getUnitTarget());
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

    // Unit spell script hooks
    if (getUnitCaster() != nullptr)
    {
        if (getUnitCaster()->IsInWorld() && getUnitCaster()->isCreature() && dynamic_cast<Creature*>(getUnitCaster())->GetScript())
            dynamic_cast<Creature*>(getUnitCaster())->GetScript()->OnCastSpell(getSpellInfo()->getId());

        if (!sEventMgr.HasEvent(getUnitCaster(), EVENT_CREATURE_RESPAWN))
        {
            for (const auto& uniqueTarget : m_uniqueHittedTargets)
            {
                auto* const targetUnit = getUnitCaster()->getWorldMapUnit(uniqueTarget.first);
                if (targetUnit == nullptr)
                    continue;

                if (getUnitCaster()->IsInWorld() && getUnitCaster()->isCreature() && dynamic_cast<Creature*>(getUnitCaster())->GetScript())
                    dynamic_cast<Creature*>(getUnitCaster())->GetScript()->OnSpellHitTarget(targetUnit, getSpellInfo());

                if (!targetUnit->isCreature())
                    continue;

                auto* const targetCreature = dynamic_cast<Creature*>(targetUnit);
                if (targetCreature->IsInWorld() && targetCreature->isCreature() && targetCreature->GetScript())
                    targetCreature->GetScript()->OnHitBySpell(getSpellInfo()->getId(), getUnitCaster());
            }
        }

        u_caster->m_canMove = true;
    }

    // Recheck used spell modifiers
    takeUsedSpellModifiers();

    if (getPlayerCaster() != nullptr)
    {
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && getPlayerCaster()->isAttacking())
        {
            getPlayerCaster()->eventAttackStop();
            getPlayerCaster()->smsg_AttackStop(getPlayerCaster()->getWorldMapUnit(getPlayerCaster()->getTargetGuid()));
            getPlayerCaster()->sendPacket(SmsgCancelCombat().serialise().get());
        }

        if (m_Delayed)
        {
            auto target = getPlayerCaster()->getWorldMapUnit(getPlayerCaster()->getChannelObjectGuid());
            if (target == nullptr)
                target = getPlayerCaster()->getWorldMapUnit(getPlayerCaster()->getTargetGuid());

            if (target != nullptr)
                target->removeAllAurasByIdForGuid(getSpellInfo()->getId(), getCaster()->getGuid());
        }

        if (getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON_OBJECT))
            getPlayerCaster()->setSummonedObject(nullptr);

        // Clear combo points before spell procs
        if (m_requiresCP && !GetSpellFailed())
        {
            // Save used combo point count for some proc spells
            m_usedComboPoints = getPlayerCaster()->getComboPoints();

            getPlayerCaster()->clearComboPoints();
        }
    }

    // Handle procs for each target
    Unit* targetUnit = nullptr;
    if (m_targetProcFlags != 0)
    {
        // Handle each target's procs to caster
        for (const auto& uniqueTarget : m_uniqueHittedTargets)
        {
            // Check if this target has already handled procs
            if (m_doneTargetProcs.find(uniqueTarget.first) != m_doneTargetProcs.end())
                continue;

            targetUnit = getCaster()->getWorldMapUnit(uniqueTarget.first);
            if (targetUnit == nullptr)
                continue;

            const auto targetProcFlags = m_targetProcFlags | m_casterDamageInfo.victimProcFlags;
            targetUnit->handleProc(targetProcFlags, getUnitCaster(), getSpellInfo(), uniqueTarget.second, m_triggeredSpell, PROC_EVENT_DO_ALL, m_triggeredByAura);
        }
    }

    // Handle caster procs
    targetUnit = nullptr;
    if (getUnitCaster() != nullptr && m_casterProcFlags != 0)
    {
        // Handle caster's procs to each target
        for (const auto& uniqueTarget : m_uniqueHittedTargets)
        {
            auto casterProcFlags = m_casterProcFlags | m_casterDamageInfo.attackerProcFlags;
            // If caster is target, remove following proc flags
            if (uniqueTarget.first == m_caster->getGuid())
                casterProcFlags &= ~(PROC_ON_DONE_MELEE_SPELL_HIT | PROC_ON_DONE_RANGED_SPELL_HIT);

            targetUnit = getCaster()->getWorldMapUnit(uniqueTarget.first);
            if (targetUnit == nullptr)
                continue;

            getUnitCaster()->handleProc(casterProcFlags, targetUnit, getSpellInfo(), uniqueTarget.second, m_triggeredSpell, PROC_EVENT_DO_TARGET_PROCS_ONLY, m_triggeredByAura);
        }

        // Use victim only if there was one target
        if (m_uniqueHittedTargets.size() == 1)
            targetUnit = getCaster()->getWorldMapUnit(m_uniqueHittedTargets.front().first);
        else
            targetUnit = nullptr;

        // Handle caster's self procs
        const auto casterProcFlags = m_casterProcFlags | m_casterDamageInfo.attackerProcFlags;
        getUnitCaster()->handleProc(casterProcFlags, targetUnit, getSpellInfo(), m_casterDamageInfo, m_triggeredSpell, PROC_EVENT_DO_CASTER_PROCS_ONLY, m_triggeredByAura);
    }

    // QuestMgr spell hooks and achievement calls
    if (getUnitCaster() != nullptr)
    {
        // Skip 'on next attack' spells if spell is not triggered
        // this will be handled on the actual spell cast
        if (!(getSpellInfo()->isOnNextMeleeAttack() && !m_triggeredSpell))
        {
            uint32_t targetCount = 0;
            for (auto& target : m_uniqueHittedTargets)
            {
                WoWGuid wowGuid;
                wowGuid.Init(target.first);
                // If target is creature
                if (wowGuid.isUnit() && getPlayerCaster() != nullptr && getPlayerCaster()->IsInWorld())
                {
                    ++targetCount;
                    sQuestMgr.OnPlayerCast(getPlayerCaster(), getSpellInfo()->getId(), target.first);
                }
#ifdef FT_ACHIEVEMENTS
                else if (wowGuid.isPlayer())
                {
                    auto* const targetPlayer = getUnitCaster()->getWorldMapPlayer(target.first);
                    if (targetPlayer != nullptr)
                    {
                        targetPlayer->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, getSpellInfo()->getId(), 0, 0, u_caster);
                        targetPlayer->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, getSpellInfo()->getId(), 0, 0, u_caster);
                    }
                }
#endif
            }

            if (getPlayerCaster() != nullptr && getPlayerCaster()->IsInWorld())
            {
                if (targetCount == 0)
                {
                    auto guid = getPlayerCaster()->getTargetGuid();
                    sQuestMgr.OnPlayerCast(getPlayerCaster(), getSpellInfo()->getId(), guid);
                }

#ifdef FT_ACHIEVEMENTS
                // Set target for spell cast achievement only if spell had one target
                Object* spellTarget = nullptr;
                if (m_uniqueHittedTargets.size() == 1)
                    spellTarget = getPlayerCaster()->getWorldMapObject(m_uniqueHittedTargets.front().first);

                if (spellTarget && spellTarget->isCreatureOrPlayer())
                    getPlayerCaster()->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2, getSpellInfo()->getId(), 0, 0, spellTarget->ToUnit());
#endif
            }
        }
    }

    // Spell is finished, remove it from traveling spells and delete it on next update tick
    getCaster()->removeTravelingSpell(this);
}

void Spell::update(unsigned long timePassed)
{
    // Check for moving while casting or channeling
    if (getPlayerCaster() != nullptr && (m_spellState == SPELL_STATE_CASTING || m_spellState == SPELL_STATE_CHANNELING))
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
                if (!u_caster->hasNoInterrupt() && !m_triggeredSpell && !getSpellInfo()->isOnNextMeleeAttack() && !getSpellInfo()->isRangedAutoRepeat())
                {
                    cancel();
                    return;
                }
            }
        }
    }

    switch (m_spellState)
    {
        case SPELL_STATE_CASTING:
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

                if (timePassed >= static_cast<uint32_t>(m_timer))
                    m_timer = 0;
                else
                    m_timer -= timePassed;
            }

            // Channeling finishes
            if (m_timer == 0)
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
                if (missEff.second.travelTime > timePassed)
                {
                    missEff.second.travelTime -= timePassed;
                    ++missItr;
                    continue;
                }

                handleMissedEffect(missEff.second.missInfo, true);
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
        case SPELL_STATE_CASTING:
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
                    auto channelTarget = getUnitCaster()->getWorldMapUnit(getUnitCaster()->getChannelObjectGuid());
                    if (channelTarget == nullptr && getPlayerCaster() != nullptr)
                        channelTarget = getPlayerCaster()->getWorldMapUnit(getPlayerCaster()->getTargetGuid());

                    if (channelTarget != nullptr)
                        channelTarget->removeAllAurasByIdForGuid(getSpellInfo()->getId(), getCaster()->getGuid());

                    // Remove dynamic objects (area aura effects from Blizzard, Rain of Fire etc)
                    if (m_AreaAura)
                    {
                        const auto dynObj = getUnitCaster()->getWorldMapDynamicObject(getUnitCaster()->getChannelObjectGuid());
                        if (dynObj != nullptr)
                            dynObj->remove();
                    }

                    if (getPlayerCaster() != nullptr && getPlayerCaster()->getSummonedObject() != nullptr)
                    {
                        auto obj = getPlayerCaster()->getSummonedObject();
                        if (obj->IsInWorld())
                            obj->RemoveFromWorld(true);

                        delete obj;
                        getPlayerCaster()->setSummonedObject(nullptr);
                    }

                    if (m_timer > 0)
                        removeCastItem();
                }

                getUnitCaster()->removeAllAurasByIdForGuid(getSpellInfo()->getId(), getCaster()->getGuid());
            }
        } break;
        case SPELL_STATE_CASTED:
            break;
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
    if (m_spellState != SPELL_STATE_CASTED)
        finish(false);
}

int32_t Spell::calculateEffect(uint8_t effIndex)
{
    auto value = getSpellInfo()->calculateEffectValue(effIndex, getUnitCaster(), getItemCaster(), *forced_basepoints);

    // Legacy script hook
    value = DoCalculateEffect(effIndex, getUnitTarget(), value);

    const auto scriptResult = sScriptMgr.callScriptedSpellDoCalculateEffect(this, effIndex, &value);

    // If effect damage was recalculated in script, send static damage in effect handlers
    // so for example spell power bonus won't get calculated twice
    isEffectDamageStatic[effIndex] = scriptResult != SpellScriptEffectDamage::DAMAGE_DEFAULT;
    if (scriptResult == SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION)
        return value;

    if (getPlayerCaster() != nullptr)
    {
        const auto itr = getPlayerCaster()->m_spellOverrideMap.find(getSpellInfo()->getId());
        if (itr != getPlayerCaster()->m_spellOverrideMap.end())
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

        effectPctModifier[effIndex] = spellPctMods / 100.0f;
    }
    else if (getItemCaster() != nullptr && getUnitTarget() != nullptr)
    {
        // Apply spell modifiers from the item owner
        const auto itemCreator = getUnitTarget()->getWorldMapUnit(getItemCaster()->getCreatorGuid());
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
#if VERSION_STRING >= WotLK
                // You can't cast other spells if you have the player flag preventing cast
                if (p_caster->hasPlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST))
                    return SPELL_FAILED_SPELL_IN_PROGRESS;
#endif

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
        if (getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IS_CHEAT_SPELL && !p_caster->getSession()->HasGMPermissions())
        {
            *parameter1 = SPELL_EXTRA_ERROR_GM_ONLY;
            return SPELL_FAILED_CUSTOM_ERROR;
        }
#endif

        // Battleground checks
        if (p_caster->getBattleground())
        {
#if VERSION_STRING >= TBC
            // Arena checks
            if (p_caster->getBattleground()->isArena())
            {
                // Spells with longer than 10 minute cooldown cannot be casted in arena
                const auto spellCooldown = getSpellInfo()->getRecoveryTime() > getSpellInfo()->getCategoryRecoveryTime() ? getSpellInfo()->getRecoveryTime() : getSpellInfo()->getCategoryRecoveryTime();
                if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS || (spellCooldown > 10 * MINUTE * IN_MILLISECONDS && !(getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS)))
                    return SPELL_FAILED_NOT_IN_ARENA;
            }
#endif

            // If battleground has ended, don't allow spell casting
            if (!m_triggeredSpell && p_caster->getBattleground()->hasEnded())
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
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNAFFECTED_BY_SCHOOL_IMMUNITY && !u_caster->getCombatHandler().isInCombat())
            return SPELL_FAILED_CASTER_AURASTATE;

        auto requireCombat = true;
#if VERSION_STRING >= WotLK
        if (u_caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
        {
            for (const auto& aurEff : getUnitCaster()->getAuraEffectList(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
            {
                if (aurEff->getAura()->getSpellInfo()->isAuraEffectAffectingSpell(aurEff->getAuraEffectType(), getSpellInfo()))
                {
                    // Warrior's Overpower uses "combo points" based on dbc data
                    // This allows usage of Overpower if we have an affecting aura (i.e. Taste for Blood)
                    m_requiresCP = false;

                    // All these aura effects use effect index 0
                    // Allow Warrior's Charge to be casted on combat if caster has Juggernaut or Warbringer talent
                    if (aurEff->getAura()->getSpellInfo()->getEffectMiscValue(0) == 1)
                    {
                        requireCombat = false;
                        break;
                    }
                }
            }
        }
#endif

        // Caster's aura state requirements
        if (getSpellInfo()->getCasterAuraState() > 0 && !u_caster->hasAuraState(static_cast<AuraState>(getSpellInfo()->getCasterAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraStateNot() > 0 && u_caster->hasAuraState(static_cast<AuraState>(getSpellInfo()->getCasterAuraStateNot()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;

        // Caster's aura spell requirements
        if (getSpellInfo()->getCasterAuraSpell() > 0 && !u_caster->hasAurasWithId(getSpellInfo()->getCasterAuraSpell()))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getCasterAuraSpellNot() == 61988)
            {
                if (u_caster->hasAurasWithId(61987))
                    return SPELL_FAILED_CASTER_AURASTATE;
            }
            else if (u_caster->hasAurasWithId(getSpellInfo()->getCasterAuraSpellNot()))
            {
                return SPELL_FAILED_CASTER_AURASTATE;
            }
        }

        if (!m_triggeredSpell)
        {
            // Out of combat spells should not be able to be casted in combat
            if (requireCombat && (getSpellInfo()->getAttributes() & ATTRIBUTES_REQ_OOC) && u_caster->getCombatHandler().isInCombat())
                return SPELL_FAILED_AFFECTING_COMBAT;

            if (!secondCheck)
            {
                // Shapeshift check
#if VERSION_STRING >= WotLK
                auto hasIgnoreShapeshiftAura = false;
                for (const auto& aurEff : getUnitCaster()->getAuraEffectList(SPELL_AURA_IGNORE_SHAPESHIFT))
                {
                    // If aura has ignore shapeshift type, you can use spells regardless of stance / form
                    // Auras with this type: Shadow Dance, Metamorphosis, Warbringer (in 3.3.5a)
                    if (aurEff->getAura()->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_IGNORE_SHAPESHIFT, getSpellInfo()))
                    {
                        hasIgnoreShapeshiftAura = true;
                        break;
                    }
                }

                if (!hasIgnoreShapeshiftAura)
#endif
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

    const auto explicitTargetMask = getSpellInfo()->getRequiredTargetMask(true);

    // Check explicit gameobject target
    if (m_targets.getGameObjectTargetGuid() != 0)
    {
        const auto objTarget = m_caster->getWorldMapGameObject(m_targets.getGameObjectTargetGuid());
        const auto targetCheck = checkExplicitTarget(objTarget, explicitTargetMask);
        if (targetCheck != SPELL_CAST_SUCCESS)
            return targetCheck;
    }

    // Unit target
    const auto target = m_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());
    if (target != nullptr)
    {
        // Check explicit unit target
        // but skip spells with pet target here
        if (!getSpellInfo()->hasTargetType(EFF_TARGET_PET))
        {
            const auto targetCheck = checkExplicitTarget(target, explicitTargetMask);
            if (targetCheck != SPELL_CAST_SUCCESS)
                return targetCheck;
        }

        // Target's aura state requirements
        if (!m_triggeredSpell && getSpellInfo()->getTargetAuraState() > 0 && !target->hasAuraState(static_cast<AuraState>(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraStateNot() > 0 && target->hasAuraState(static_cast<AuraState>(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;

        // Target's aura spell requirements
        if (getSpellInfo()->getTargetAuraSpell() > 0 && !target->hasAurasWithId(getSpellInfo()->getTargetAuraSpell()))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getTargetAuraSpellNot() == 61988)
            {
                if (target->hasAurasWithId(61987))
                    return SPELL_FAILED_TARGET_AURASTATE;
            }
            else if (target->hasAurasWithId(getSpellInfo()->getTargetAuraSpellNot()))
            {
                return SPELL_FAILED_TARGET_AURASTATE;
            }
        }

        if (target->isCorpse())
        {
            // Player can't cast spells on corpses with bones only left
            const auto targetCorpse = sObjectMgr.getCorpseByOwner(target->getGuidLow());
            if (targetCorpse == nullptr || !targetCorpse->IsInWorld() || targetCorpse->getCorpseState() == CORPSE_STATE_BONES)
                return SPELL_FAILED_BAD_TARGETS;
        }

        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_TARGET_SELF && m_caster == target)
            return SPELL_FAILED_BAD_TARGETS;

        // Check if spell requires target to be out of combat
        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_OOC_TARGET && target->getCombatHandler().isInCombat())
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

        // Check combo points
        if (m_requiresCP && getPlayerCaster() != nullptr)
        {
            if (!(getPlayerCaster()->getComboPoints() > 0 && getPlayerCaster()->getComboPointTarget() == target->getGuid()))
                return SPELL_FAILED_NO_COMBO_POINTS;
        }

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
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CANT_TARGET_TAGGED && target->isTagged() && !target->isTaggedByPlayerOrItsGroup(p_caster))
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
                    auto targetUnit = p_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());
                    // If spell is triggered, target may need to be picked manually
                    if (targetUnit == nullptr)
                    {
                        if (p_caster->getTargetGuid() != 0)
                            targetUnit = p_caster->getWorldMapUnit(p_caster->getTargetGuid());
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

                    if (p_caster->getPet() != nullptr || !p_caster->findFreeActivePetSlot().has_value())
                    {
                        SendTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (p_caster->getPetCount() >= PET_SLOT_MAX_TOTAL_PET_COUNT)
                    {
                        SendTameFailure(PETTAME_TOOMANY);
                        return SPELL_FAILED_DONT_REPORT;
                    }

#if VERSION_STRING >= WotLK
                    // Check for Beast Mastery spell with exotic creatures
                    if (!p_caster->hasAuraWithAuraEffect(SPELL_AURA_ALLOW_TAME_PET_TYPE) && creatureTarget->IsExotic())
                    {
                        SendTameFailure(PETTAME_CANTCONTROLEXOTIC);
                        return SPELL_FAILED_DONT_REPORT;
                    }
#endif

                    // All good so far, check creature's family
                    const auto creatureFamily = sCreatureFamilyStore.lookupEntry(creatureTarget->GetCreatureProperties()->Family);
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
                // Check for generic ranged spells as well, if caster is player
                if (getSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INFRONT || getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET || (getPlayerCaster() != nullptr && getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED))
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
                    (m_caster->GetMapId() != target->GetMapId() || !m_caster->IsWithinLOSInMap(target)))
                    return SPELL_FAILED_LINE_OF_SIGHT;
            }

            if (target->isPlayer())
            {
                // Check if target is dueling
                // but allow spell cast if target is unfriendly
                const auto targetPlayer = dynamic_cast<Player*>(target);
                if (targetPlayer->getDuelState() == DUEL_STATE_STARTED)
                {
                    if (auto* const playerOwner = getCaster()->getPlayerOwnerOrSelf())
                    {
                        if (targetPlayer->getDuelPlayer() != playerOwner && playerOwner->isFriendlyTo(targetPlayer))
                            return SPELL_FAILED_TARGET_DUELING;
                    }
                }

                // Check if caster or target is in a sanctuary area
                // but allow spell casting in duels
                if (auto* const playerOwner = getCaster()->getPlayerOwnerOrSelf())
                {
                    if (targetPlayer->getDuelPlayer() != playerOwner && !playerOwner->isFriendlyTo(targetPlayer))
                    {
                        if ((m_caster->GetArea() != nullptr && m_caster->GetArea()->flags & MapManagement::AreaManagement::AREA_SANCTUARY) ||
                            (targetPlayer->GetArea() != nullptr && targetPlayer->GetArea()->flags & MapManagement::AreaManagement::AREA_SANCTUARY))
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                }

                // Do not allow spell casts on players when they are on a taxi
                // unless it's a summoning spell
                if (targetPlayer->isOnTaxi() && !getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON_PLAYER))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_PLAYERS)
            {
                // Check only single target spells here
                // Spell target system handles this for area spells
                if (!(explicitTargetMask & SPELL_TARGET_AREA_MASK))
                    return SPELL_FAILED_TARGET_NOT_PLAYER;
            }

            // Check if target has stronger aura active
            const AuraCheckResponse auraCheckResponse = target->auraCheck(getSpellInfo(), m_caster);
            if (auraCheckResponse.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                return SPELL_FAILED_AURA_BOUNCED;

            // Check if target is immune to this dispel type
            //\ TODO: fix me (move to DidHit?) -Appled
            if (target->m_dispels[getSpellInfo()->getDispelType()])
                return SPELL_FAILED_IMMUNE;
        }
    }

    if (p_caster != nullptr)
    {
        // Check if spell requires a dead pet
        if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_DEAD_PET)
        {
            const auto pet = p_caster->getPet();
            if (pet != nullptr)
            {
                if (pet->isAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;
            }
            else
            {
                // Find dead pet from any active slot
                auto foundDeadPet = false;
                for (const auto& [petSlot, petId] : p_caster->getPetCachedSlotMap())
                {
                    if (petSlot >= PET_SLOT_MAX_ACTIVE_SLOT)
                        break;

                    const auto petCache = p_caster->getPetCache(petId);
                    if (petCache == nullptr)
                        continue;

                    if (!petCache->alive)
                    {
                        foundDeadPet = true;
                        // Save pet id for later use
                        add_damage = petCache->number;
                        break;
                    }
                }

                // todo: probably not correct error message
                if (!foundDeadPet)
                    return SPELL_FAILED_BAD_TARGETS;
            }
        }

        // Check if spell effect requires pet target
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
            {
                const auto pet = p_caster->getPet();
                if (pet == nullptr)
                    return m_triggeredByAura ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NO_PET;
                else if (!pet->isAlive())
                    return m_triggeredByAura ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_TARGETS_DEAD;
                // Check Line of Sight with pets as well
                if (!m_triggeredSpell && worldConfig.terrainCollision.isCollisionEnabled)
                {
                    if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
                        (m_caster->GetMapId() != pet->GetMapId() || !m_caster->IsWithinLOSInMap(pet)))
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
            !m_caster->IsWithinLOS(m_targets.getDestination()))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    if (p_caster != nullptr)
    {
        // Check if spell requires certain area
        if (getSpellInfo()->getRequiresAreaId() > 0)
        {
            auto areaEntry = p_caster->GetArea();
            if (areaEntry == nullptr)
                areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(p_caster->getZoneId());
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
            auto areaGroup = sAreaGroupStore.lookupEntry(requireAreaId);
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

                areaGroup = sAreaGroupStore.lookupEntry(areaGroup->next_group);
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
        if ((p_caster->isMounted() || p_caster->hasUnitFlags(UNIT_FLAG_MOUNTED_TAXI)) && !m_triggeredSpell && !getSpellInfo()->isPassive())
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
            if (p_caster->IsInWorld() && p_caster->getWorldMap()->getBaseMap()->getMapInfo() != nullptr && (p_caster->getWorldMap()->getBaseMap()->getMapInfo()->isRaid() || p_caster->getWorldMap()->getDifficulty() == InstanceDifficulty::DUNGEON_HEROIC))
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
                    sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::canCast : Found gameobject entry {} with invalid gameobject properties, spawn id {}", obj->getEntry(), obj->getGuidLow());
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
                    distance = getSpellInfo()->getMaxRange(false, p_caster, this);
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

    if (m_targetConstraint != nullptr)
    {
        // Search for target constraint from within spell's max range
        float_t range = 0.0f;
        const auto rangeEntry = sSpellRangeStore.lookupEntry(getSpellInfo()->getRangeIndex());
        if (rangeEntry != nullptr)
            range = rangeEntry->maxRange;

        auto foundTarget = false;

        // Check if target needs to be a certain creature
        for (const auto& entryId : m_targetConstraint->getCreatures())
        {
            if (!m_targetConstraint->hasExplicitTarget(entryId))
            {
                // Spell requires an implicit target
                // Find closest creature with the required entry id
                const auto creatureTarget = m_caster->IsInWorld() ? m_caster->getWorldMap()->getInterface()->getCreatureNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), entryId) : nullptr;
                if (creatureTarget != nullptr)
                {
                    // Check that the creature is within spell's range
                    if (m_caster->isInRange(creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), range * range))
                    {
                        setTargetConstraintCreature(creatureTarget);
                        foundTarget = true;
                        break;
                    }
                }
            }
            else
            {
                // Spell requires an explicit target
                // Most these spells are casted from items and then client does NOT send target guid in cast spell packet
                Unit* creatureTarget = nullptr;
                if (p_caster != nullptr)
                {
                    // If caster is player, use player's selected target
                    creatureTarget = p_caster->getWorldMapUnit(p_caster->getTargetGuid());
                }
                else if (u_caster != nullptr)
                {
                    // If caster is creature, use the one set in castSpell function
                    creatureTarget = u_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());
                    if (creatureTarget == nullptr)
                        creatureTarget = u_caster->getWorldMapUnit(u_caster->getTargetGuid());
                }

                if (creatureTarget == nullptr)
                    continue;

                if (!creatureTarget->isCreature() || creatureTarget->getEntry() != entryId)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check that the creature is within spell's range
                if (!m_caster->isInRange(creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), range * range))
                    return SPELL_FAILED_OUT_OF_RANGE;

                // Found target
                setTargetConstraintCreature(dynamic_cast<Creature*>(creatureTarget));
                foundTarget = true;
                break;
            }
        }

        // Check if target needs to be a certain gameobject
        for (const auto& entryId : m_targetConstraint->getGameObjects())
        {
            if (!m_targetConstraint->hasExplicitTarget(entryId))
            {
                // Spell requires an implicit target
                // Find closest gameobject with the required entry id
                const auto gobTarget = m_caster->IsInWorld() ? m_caster->getWorldMap()->getInterface()->getGameObjectNearestCoords(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), entryId) : nullptr;
                if (gobTarget != nullptr)
                {
                    // Check that the gameobject is within spell's range
                    if (m_caster->isInRange(gobTarget->GetPositionX(), gobTarget->GetPositionY(), gobTarget->GetPositionZ(), range * range))
                    {
                        setTargetConstraintGameObject(gobTarget);
                        foundTarget = true;
                    }
                }
            }
            else
            {
                // Spell requires an explicit target
                const auto objectTarget = m_caster->getWorldMapObject(m_targets.getGameObjectTargetGuid());
                if (objectTarget == nullptr)
                    continue;

                if (!objectTarget->isGameObject() || objectTarget->getEntry() != entryId)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check that the gameobject is within the spell's range
                if (!m_caster->isInRange(objectTarget->GetPositionX(), objectTarget->GetPositionY(), objectTarget->GetPositionZ(), range * range))
                    return SPELL_FAILED_OUT_OF_RANGE;

                // Found target
                setTargetConstraintGameObject(dynamic_cast<GameObject*>(objectTarget));
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
            case SPELL_EFFECT_SUMMON_PET:
            {
                if (u_caster == nullptr)
                    break;

                if (u_caster->getPet() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
                {
                    if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_SUMMON)
                    {
                        // Check from summon properties if this new pet is actually a pet
                        if (const auto summonProperties = sSummonPropertiesStore.lookupEntry(getSpellInfo()->getEffectMiscValueB(i)))
                        {
                            if (summonProperties->ControlType == SUMMON_CONTROL_TYPE_PET || summonProperties->Type == SUMMONTYPE_PET)
                                return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                        }
                    }
                    else
                    {
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                    }
                }

                if (u_caster->getCharmGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
            } break;
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
                if (p_caster == nullptr)
                    break;

                // Don't allow these effects in battlegrounds if the battleground hasn't yet started
                if (p_caster->getBattleground() && !p_caster->getBattleground()->hasStarted())
                    return SPELL_FAILED_TRY_AGAIN;
            } break;
            case SPELL_EFFECT_OPEN_LOCK:
            case SPELL_EFFECT_OPEN_LOCK_ITEM:
            {
                if (p_caster == nullptr)
                    break;

                uint32_t lockId = 0;
                if (m_targets.getGameObjectTargetGuid() != 0)
                {
                    const auto objectTarget = p_caster->getWorldMapGameObject(m_targets.getGameObjectTargetGuid());
                    if (objectTarget != nullptr &&
                        objectTarget->getGoType() != GAMEOBJECT_TYPE_QUESTGIVER &&
                        objectTarget->getGoType() != GAMEOBJECT_TYPE_AREADAMAGE &&
                        objectTarget->getGoType() != GAMEOBJECT_TYPE_FLAGSTAND &&
                        objectTarget->getGoType() != GAMEOBJECT_TYPE_FLAGDROP)
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
                            // TODO: implement questgiver gameobjects
                            //case GAMEOBJECT_TYPE_QUESTGIVER:
                            case GAMEOBJECT_TYPE_CHEST:
                                lockId = objectTarget->GetGameObjectProperties()->chest.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_TRAP:
                                lockId = objectTarget->GetGameObjectProperties()->trap.lock_id;
                                break;
                            case GAMEOBJECT_TYPE_GOOBER:
                                lockId = objectTarget->GetGameObjectProperties()->goober.lock_id;
                                break;
                            // TODO: implement areadamage gameobjects
                            //case GAMEOBJECT_TYPE_AREADAMAGE:
                            case GAMEOBJECT_TYPE_CAMERA:
                                lockId = objectTarget->GetGameObjectProperties()->camera.lock_id;
                                break;
                            // TODO: implement flagstand gameobjects
                            //case GAMEOBJECT_TYPE_FLAGSTAND:
                            case GAMEOBJECT_TYPE_FISHINGHOLE:
                                lockId = objectTarget->GetGameObjectProperties()->fishinghole.lock_id;
                                break;
                            // TODO: implement flagdrop gameobjects
                            //case GAMEOBJECT_TYPE_FLAGDROP:
                            default:
                                break;
                        }

                        if (lockId == 0)
                            return SPELL_FAILED_ALREADY_OPEN;
                    }
                }
                else if (m_targets.getItemTargetGuid() != 0)
                {
                    Item const* targetItem = nullptr;
                    if (m_targets.isTradeItem())
                    {
                        const auto playerTrader = p_caster->getTradeTarget();
                        if (playerTrader != nullptr)
                            targetItem = playerTrader->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
                    }
                    else
                    {
                        targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                    }

                    if (targetItem == nullptr)
                        return SPELL_FAILED_ITEM_GONE;

                    // Check if item is already unlocked
                    if (targetItem->getItemProperties()->LockId == 0 || !targetItem->m_isLocked)
                        return SPELL_FAILED_ALREADY_OPEN;

                    lockId = targetItem->getItemProperties()->LockId;
                }

                if (lockId == 0)
                    break;

                const auto lockInfo = sLockStore.lookupEntry(lockId);
                if (lockInfo == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                auto successfulOpening = false;
                for (uint8_t x = 0; x < LOCK_NUM_CASES; ++x)
                {
                    // Check if object requires an item for unlocking
                    if (lockInfo->locktype[x] == LOCK_KEY_ITEM)
                    {
                        if (i_caster == nullptr || lockInfo->lockmisc[x] == 0)
                            continue;
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
                        uint16_t skillId = 0;
                        switch (lockInfo->lockmisc[x])
                        {
#if VERSION_STRING <= WotLK
                            case LOCKTYPE_PICKLOCK:
                                skillId = SKILL_LOCKPICKING;
                                break;
#endif
                            case LOCKTYPE_HERBALISM:
                                skillId = SKILL_HERBALISM;
                                break;
                            case LOCKTYPE_MINING:
                                skillId = SKILL_MINING;
                                break;
                            case LOCKTYPE_FISHING:
                                skillId = SKILL_FISHING;
                                break;
#if VERSION_STRING >= WotLK
                            case LOCKTYPE_INSCRIPTION:
                                skillId = SKILL_INSCRIPTION;
                                break;
#endif
                            default:
                                break;
                        }

                        if (skillId != 0 || lockInfo->lockmisc[x] == LOCKTYPE_BLASTING)
                        {
                            // If item is used for opening, do not use player's skill level
                            uint32_t skillLevel = i_caster != nullptr || p_caster == nullptr ? 0 : p_caster->getSkillLineCurrent(skillId);
                            // Add skill bonuses from the spell
                            skillLevel += getSpellInfo()->calculateEffectValue(i);;

                            // Check for low skill level
                            if (skillLevel < lockInfo->minlockskill[x])
                                return SPELL_FAILED_LOW_CASTLEVEL;

#if VERSION_STRING >= WotLK
                            // Patch 3.2.0: In addition to the normal requirements, mining deposits in Northrend now require a minimum character level of 65 to mine.
                            if (p_caster)
                            {
                                if (skillId == SKILL_MINING && p_caster->GetMapId() == 571 && p_caster->getLevel() < 65)
                                {
                                    *parameter1 = SPELL_EXTRA_ERROR_NORTHREND_MINING;
                                    return SPELL_FAILED_CUSTOM_ERROR;
                                }
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
            [[fallthrough]];
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                if (p_caster == nullptr)
                    break;

                const auto pet = p_caster->getPet();
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

                const auto itr = sLootMgr.getPickpocketingLoot().find(target->getEntry());
                if (itr == sLootMgr.getPickpocketingLoot().end())
                    return SPELL_FAILED_TARGET_NO_POCKETS;
            } break;
#if VERSION_STRING >= WotLK
            case SPELL_EFFECT_USE_GLYPH:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

                const auto glyphId = static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i));
                const auto glyphEntry = sGlyphPropertiesStore.lookupEntry(glyphId);
                if (glyphEntry == nullptr)
                    return SPELL_FAILED_INVALID_GLYPH;

                // Check if glyph slot is locked
                if (!(p_caster->getGlyphsEnabled() & (1 << m_glyphslot)))
                    return SPELL_FAILED_GLYPH_SOCKET_LOCKED;

                // Check if player already has this glyph
                if (p_caster->hasAurasWithId(glyphEntry->SpellID))
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
                if (p_caster->IsInWorld() && p_caster->getWorldMap()->getBaseMap()->getMapInfo() != nullptr && !p_caster->getWorldMap()->getBaseMap()->getMapInfo()->isNonInstanceMap())
                    return SPELL_FAILED_NO_DUELING;

                const auto targetPlayer = p_caster->getWorldMapPlayer(m_targets.getUnitTargetGuid());
                if (targetPlayer != nullptr && targetPlayer->GetTransport() != p_caster->GetTransport())
                    return SPELL_FAILED_NOT_ON_TRANSPORT;

                if (targetPlayer != nullptr && targetPlayer->getDuelPlayer() != nullptr)
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
                if (p_caster->IsInWorld() && p_caster->getWorldMap()->getBaseMap()->getMapInfo() != nullptr && !p_caster->getWorldMap()->getBaseMap()->getMapInfo()->isNonInstanceMap())
                {
                    if (!p_caster->IsInMap(targetPlayer))
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;

                    const auto mapInfo = p_caster->getWorldMap()->getBaseMap()->getMapInfo();
                    if (p_caster->getWorldMap()->getDifficulty() == InstanceDifficulty::DUNGEON_HEROIC)
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
                    if (mapInfo->isBattleground() || p_caster->getBattleground())
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
                if (creatureTarget->loot.isLooted() && creatureTarget->isTagged())
                {
                    const auto taggerPlayer = creatureTarget->getWorldMapPlayer(creatureTarget->getTaggerGuid());
                    if (taggerPlayer != nullptr && creatureTarget->HasLootForPlayer(taggerPlayer))
                        return SPELL_FAILED_TARGET_NOT_LOOTED;
                }

                // Check if caster has required skinning level for target
                const auto skillLevel = p_caster->getSkillLineCurrent(creatureTarget->GetRequiredLootSkill());
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
                    if (!u_caster->getAIInterface()->canCreatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()))
                        return SPELL_FAILED_NOPATH;
                }
            } break;
            case SPELL_EFFECT_FEED_PET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                const auto pet = p_caster->getPet();
                if (pet == nullptr)
                    return SPELL_FAILED_NO_PET;

                if (!pet->isAlive())
                    return SPELL_FAILED_TARGETS_DEAD;

                // Get the food
                const auto foodItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                if (foodItem == nullptr)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check if the item is food
                const auto itemProto = foodItem->getItemProperties();
                if (itemProto->FoodType == 0)
                    return SPELL_FAILED_BAD_TARGETS;

                // Check if the food type matches pet's diet
                if (!(pet->getPetDiet() & (1 << (itemProto->FoodType - 1))))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                // Check if the food level is at most 30 levels below pet's level
                if (pet->getLevel() > (itemProto->ItemLevel + 30))
                    return SPELL_FAILED_FOOD_LOWLEVEL;
            } break;
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                if (p_caster == nullptr)
                    return SPELL_FAILED_NO_PET;

                // Spells with this attribute were checked already
                if (!(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_DEAD_PET))
                {
                    const auto petTarget = p_caster->getPet();
                    if (petTarget != nullptr)
                    {
                        if (petTarget->isAlive())
                            return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                    }
                    else
                    {
                        // Find dead pet from any active slot
                        auto foundDeadPet = false;
                        for (const auto& [petSlot, petId] : p_caster->getPetCachedSlotMap())
                        {
                            if (petSlot >= PET_SLOT_MAX_ACTIVE_SLOT)
                                break;

                            const auto petCache = p_caster->getPetCache(petId);
                            if (petCache == nullptr)
                                continue;

                            if (!petCache->alive)
                            {
                                foundDeadPet = true;
                                // Save pet id for later use
                                add_damage = petCache->number;
                                break;
                            }
                        }

                        // todo: probably not correct error message
                        if (!foundDeadPet)
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                }
            } break;
            case SPELL_EFFECT_SPELL_STEAL:
            {
                if (m_targets.getUnitTargetGuid() == m_caster->getGuid())
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

                if (p_caster->getPet() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
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
                    if (p_caster != nullptr && p_caster->getPet() != nullptr && !(getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_DISMISS_CURRENT_PET))
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
                if (p_caster != nullptr && target->isCreature() && targetCreature->isTagged() && !targetCreature->isTaggedByPlayerOrItsGroup(p_caster))
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
#if VERSION_STRING < WotLK
                    if (target->getVirtualItemDisplayId(MELEE) == 0)
#else
                    if (target->getVirtualItemSlotId(MELEE) == 0)
#endif
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
#if VERSION_STRING == Cata
                    if (getSpellInfo()->getEffectMiscValueB(i) && !p_caster->getMountCapability(getSpellInfo()->getEffectMiscValueB(i)))
                        return SPELL_FAILED_NOT_HERE;
#endif

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

                const auto petTarget = p_caster->getPet();
                if (petTarget == nullptr)
                    return SPELL_FAILED_NO_PET;

                if (petTarget->getCharmedByGuid() != 0)
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
            } break;
#if VERSION_STRING >= TBC
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
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (target->getGuid() == m_caster->getGuid())
                    return SPELL_FAILED_BAD_TARGETS;

                // Cloned targets cannot be cloned
                if (target->hasAuraWithAuraEffect(SPELL_AURA_MIRROR_IMAGE))
                    return SPELL_FAILED_BAD_TARGETS;
            } break;
#endif
            default:
                break;
        }
    }

    if (m_targets.isTradeItem())
    {
        if (p_caster == nullptr)
            return SPELL_FAILED_SPELL_UNAVAILABLE;

        // Slot must be the lowest
        if (TradeSlots(m_targets.getItemTargetGuid()) != TRADE_SLOT_NONTRADED)
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
        sLogger.failure("Spell::checkPower : Unknown power type {} for spell id {}", getSpellInfo()->getPowerType(), getSpellInfo()->getId());
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
        if (itemProperties->ZoneNameID > 0 && itemProperties->ZoneNameID != p_caster->getZoneId())
            return SPELL_FAILED_NOT_HERE;
        // Check map
        if (itemProperties->MapID > 0 && itemProperties->MapID != p_caster->GetMapId())
            return SPELL_FAILED_NOT_HERE;
#else
        // Check zone
        if (itemProperties->ZoneNameID > 0 && itemProperties->ZoneNameID != p_caster->getZoneId())
            return SPELL_FAILED_INCORRECT_AREA;
        // Check map
        if (itemProperties->MapID > 0 && itemProperties->MapID != p_caster->GetMapId())
            return SPELL_FAILED_INCORRECT_AREA;
#endif

        if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
        {
            if (p_caster->getCombatHandler().isInCombat())
            {
                p_caster->getItemInterface()->buildInventoryChangeError(i_caster, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
                return SPELL_FAILED_DONT_REPORT;
            }
            else if (p_caster->isMounted())
            {
                return SPELL_FAILED_NOT_MOUNTED;
            }
        }

        // Check health and power for consumables (potions, healthstones, mana items etc)
        if (itemProperties->Class == ITEM_CLASS_CONSUMABLE)
        {
            const auto targetUnit = p_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());
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
                        if (getSpellInfo()->getEffectMiscValue(i) < 0 || getSpellInfo()->getEffectMiscValue(i) >= TOTAL_PLAYER_POWER_TYPES)
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
            const auto shapeShift = sSpellShapeshiftFormStore.lookupEntry(p_caster->getShapeShiftForm());
            if (shapeShift != nullptr && !(shapeShift->Flags & 1))
            {
                if (!(i_caster->getItemProperties()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
                    return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;
            }
        }
    }

    // Casted on an item
    if (m_targets.getItemTargetGuid() > 0)
    {
        Item* targetItem = nullptr;
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
                    targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
            }
            else
            {
                return SPELL_FAILED_NOT_TRADEABLE;
            }
        }
        else
        {
            targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
        }

        if (targetItem == nullptr)
            return SPELL_FAILED_ITEM_GONE;

        // Check explicit item target
        const auto targetCheck = checkExplicitTarget(targetItem, getSpellInfo()->getRequiredTargetMask(true));
        if (targetCheck != SPELL_CAST_SUCCESS)
            return targetCheck;

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
    else if (m_targets.getItemTargetGuid() == 0 && getSpellInfo()->getEquippedItemClass() >= 0)
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
#if VERSION_STRING >= WotLK
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
#if VERSION_STRING >= WotLK
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

        // These triggered learn spell effects shouldn't fail here
        if (m_triggeredSpell)
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                const auto eff = getSpellInfo()->getEffect(i);
                if (eff == SPELL_EFFECT_NULL)
                    continue;

                if (eff == SPELL_EFFECT_BLOCK || eff == SPELL_EFFECT_WEAPON || eff == SPELL_EFFECT_PROFICIENCY)
                {
                    hasItemWithProperType = true;
                    break;
                }
            }
        }

        if (!hasItemWithProperType)
        {
            *parameter1 = static_cast<uint32_t>(getSpellInfo()->getEquippedItemClass());
            *parameter2 = static_cast<uint32_t>(getSpellInfo()->getEquippedItemSubClass());
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
#if VERSION_STRING == Classic
        auto checkForReagents = true;
#else
        // Spells with ATTRIBUTESEXE_REAGENT_REMOVAL attribute won't take reagents if player has UNIT_FLAG_NO_REAGANT_COST flag
        auto checkForReagents = !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_REAGENT_REMOVAL && p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST));
#endif
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
        else if (m_targets.getItemTargetGuid() != 0 && m_targets.isTradeItem())
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
#if VERSION_STRING >= WotLK
            case SPELL_EFFECT_CREATE_ITEM2:
#endif
                if (getSpellInfo()->getEffectItemType(i) != 0)
                {
                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        sLogger.failure("Spell::checkItems: Spell entry {} has unknown item id ({}) in SPELL_EFFECT_CREATE_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
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
            {
                // Check only for vellums here, normal checks are done in the next case
                if (getSpellInfo()->getEffectItemType(i) != 0 && m_targets.getItemTargetGuid() != 0 && vellumTarget)
                {
                    // Player can only enchant their own vellums
                    if (m_targets.isTradeItem())
                        return SPELL_FAILED_NOT_TRADEABLE;
                    // Scrolls (enchanted vellums) cannot be enchanted into another vellum (duping)
                    if (scrollItem)
                        return SPELL_FAILED_BAD_TARGETS;

                    const auto vellumItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                    if (vellumItem == nullptr)
                        return SPELL_FAILED_ITEM_NOT_FOUND;
                    // Check if vellum is appropriate target for the enchant
                    if (getSpellInfo()->getBaseLevel() > vellumItem->getItemProperties()->ItemLevel)
                        return SPELL_FAILED_LOWLEVEL;

                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        sLogger.failure("Spell::checkItems: Spell entry {} has unknown item id ({}) in SPELL_EFFECT_ENCHANT_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
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
#if VERSION_STRING >= WotLK
            }
            [[fallthrough]];
            case SPELL_EFFECT_ADD_SOCKET:
            {
#endif
                if (m_targets.getItemTargetGuid() == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item* targetItem = nullptr;
                if (m_targets.isTradeItem())
                {
                    if (p_caster->getTradeTarget() != nullptr)
                        targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
                }
                else
                {
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
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

                const auto enchantEntry = sSpellItemEnchantmentStore.lookupEntry(static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i)));
                if (enchantEntry == nullptr)
                {
                    sLogger.failure("Spell::checkItems: Spell entry {} has no valid enchantment ({})", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
                    return SPELL_FAILED_ERROR;
                }

                // Loop through enchantment's types
                for (const auto& type : enchantEntry->type)
                {
                    switch (type)
                    {
                        case ITEM_ENCHANTMENT_TYPE_USE_SPELL:
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
#if VERSION_STRING >= WotLK
                        case ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET:
                            // Check if the item already has a prismatic gem slot enchanted
                            if (targetItem->getEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0)
                                return SPELL_FAILED_ITEM_ALREADY_ENCHANTED;

                            // or if the item already has the maximum amount of socket slots
                            if (targetItem->getSocketSlotCount() >= MAX_ITEM_PROTO_SOCKETS)
                                return SPELL_FAILED_MAX_SOCKETS;
                            break;
#endif
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
                if (m_targets.getItemTargetGuid() == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item const* targetItem = nullptr;
                if (m_targets.isTradeItem())
                {
                    if (p_caster->getTradeTarget() != nullptr)
                        targetItem = p_caster->getTradeTarget()->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
                }
                else
                {
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                }

                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                const auto enchantmentEntry = sSpellItemEnchantmentStore.lookupEntry(static_cast<uint32_t>(getSpellInfo()->getEffectMiscValue(i)));
                if (enchantmentEntry == nullptr)
                {
                    sLogger.failure("Spell::checkItems: Spell entry {} has no valid enchantment ({})", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
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
                if (m_targets.getItemTargetGuid() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
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
                if (static_cast<uint32_t>(itemProperties->DisenchantReqSkill) > p_caster->getSkillLineCurrent(SKILL_ENCHANTING))
                    return SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL;
#endif
                // TODO: check does the item even have disenchant loot
                break;
            }
#if VERSION_STRING >= TBC
            case SPELL_EFFECT_PROSPECTING:
            {
                if (m_targets.getItemTargetGuid() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is prospectable
                if (!(itemProperties->Flags & ITEM_FLAG_PROSPECTABLE))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                // Check if player has enough skill in Jewelcrafting
                if (itemProperties->RequiredSkillRank > p_caster->getSkillLineCurrent(SKILL_JEWELCRAFTING))
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
                if (m_targets.getItemTargetGuid() == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.isTradeItem())
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is millable
                if (!(itemProperties->Flags & ITEM_FLAG_MILLABLE))
                    return SPELL_FAILED_CANT_BE_MILLED;
                // Check if player has enough skill in Inscription
                if (itemProperties->RequiredSkillRank > p_caster->getSkillLineCurrent(SKILL_INSCRIPTION))
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
    if (u_caster->hasUnitStateFlag(UNIT_STATE_STUNNED))
    {
        if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_STUNNED)
        {
            // Spell is usable while stunned, but there are some spells with stun effect which are not classified as normal stun spells
            for (const auto& aurEff : getUnitCaster()->getAuraEffectList(SPELL_AURA_MOD_STUN))
            {
                const auto* stunAura = aurEff->getAura();
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
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_CONFUSED) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_CONFUSED))
    {
        errorMsg = SPELL_FAILED_CONFUSED;
    }
    else if (u_caster->hasUnitStateFlag(UNIT_STATE_FLEEING) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_USABLE_WHILE_FEARED))
    {
        errorMsg = SPELL_FAILED_FLEEING;
    }
    else if (u_caster->hasUnitFlags(UNIT_FLAG_SILENCED) && getSpellInfo()->getPreventionType() == PREVENTION_TYPE_SILENCE)
    {
        errorMsg = SPELL_FAILED_SILENCED;
    }
    else if (u_caster->hasUnitStateFlag(UNIT_FLAG_PACIFIED) && getSpellInfo()->getPreventionType() == PREVENTION_TYPE_PACIFY)
    {
        errorMsg = SPELL_FAILED_PACIFIED;
    }

    if (errorMsg != SPELL_CAST_SUCCESS)
    {
        if (schoolImmunityMask > 0 || dispelImmunityMask > 0 || mechanicImmunityMask > 0)
        {
            // The spell cast is prevented by some state but check if the spell is unaffected by those states or grants immunity to those states
            for (const auto& aur : getUnitCaster()->getAuraList())
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
    const auto rangeEntry = sSpellRangeStore.lookupEntry(getSpellInfo()->getRangeIndex());
    if (rangeEntry == nullptr)
        return SPELL_CAST_SUCCESS;

    // Players can activate "on next attack" abilities before being at melee range
    if (!secondCheck && getSpellInfo()->isOnNextMeleeAttack())
        return SPELL_CAST_SUCCESS;

    auto targetUnit = m_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());

    // Self cast spells don't need range check
    if (getSpellInfo()->getRangeIndex() == 1 || targetUnit == m_caster)
        return SPELL_CAST_SUCCESS;

    if (u_caster != nullptr)
    {
        // If pet is the effect target, check range to pet
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getSpellInfo()->getEffectImplicitTargetA(i) != EFF_TARGET_PET)
                continue;

            if (u_caster->getPet() != nullptr)
            {
                targetUnit = u_caster->getPet();
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
            minRange += m_caster->isFriendlyTo(targetUnit) ? rangeEntry->minRangeFriendly : rangeEntry->minRange;
#endif

        // Get maximum range
#if VERSION_STRING < WotLK
        maxRange = rangeEntry->maxRange;
#else
        if (targetUnit == nullptr)
            maxRange = rangeEntry->maxRange;
        else
            maxRange = m_caster->isFriendlyTo(targetUnit) ? rangeEntry->maxRangeFriendly : rangeEntry->maxRange;
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

        const auto spellRuneCost = sSpellRuneCostStore.lookupEntry(getSpellInfo()->getRuneCostID());
        if (spellRuneCost != nullptr && (spellRuneCost->bloodRuneCost > 0 || spellRuneCost->frostRuneCost > 0 || spellRuneCost->unholyRuneCost > 0))
        {
            uint32_t runeCost[3];
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
#if VERSION_STRING < Mop
    // No need to check requirements for talents that learn spells
    uint8_t talentRank = 0;
    const auto talentInfo = sTalentStore.lookupEntry(spellInfo->getId());
    if (talentInfo != nullptr)
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            if (talentInfo->RankID[i] != 0)
                talentRank = i + 1U;
        }
    }

    // This is client-side only
    if (talentRank > 0 && spellInfo->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        return SPELL_CAST_SUCCESS;

    const uint32_t stanceMask = shapeshiftForm ? 1 << (shapeshiftForm - 1U) : 0U;

    // Cannot explicitly be casted in this stance/form
    if (spellInfo->getShapeshiftExclude() > 0 && spellInfo->getShapeshiftExclude() & stanceMask)
        return SPELL_FAILED_NOT_SHAPESHIFT;

    // Can explicitly be casted in this stance/form
    if (spellInfo->getRequiredShapeShift() > 0 && spellInfo->getRequiredShapeShift() & stanceMask)
        return SPELL_CAST_SUCCESS;

    auto actAsShifted = false;
    if (stanceMask > FORM_NORMAL)
    {
        auto shapeShift = sSpellShapeshiftFormStore.lookupEntry(shapeshiftForm);
        if (shapeShift == nullptr)
        {
            sLogger.failure("Spell::checkShapeshift: Caster has unknown shapeshift form {}", shapeshiftForm);
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
#else
    return SPELL_FAILED_ONLY_SHAPESHIFT;
#endif
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
    {
        plr = u_caster->m_redirectSpellPackets;

        if (plr == nullptr && getUnitCaster()->isVehicle())
            plr = getUnitCaster()->getPlayerOwner();
    }

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
                const auto target = u_caster->getWorldMapUnit(channelGuid);

                if (aur != nullptr)
                    aur->update(diff, true);

                if (target != nullptr)
                {
                    const auto targetAur = target->getAuraWithIdForGuid(getSpellInfo()->getId(), casterGuid);
                    if (targetAur != nullptr)
                        targetAur->update(diff, true);
                }
            }

            const auto dynamicObject = u_caster->getWorldMapDynamicObject(WoWGuid::getGuidLowPartFromUInt64(channelGuid));
            if (dynamicObject != nullptr)
                dynamicObject->remove();

            u_caster->setChannelObjectGuid(0);
            u_caster->setChannelSpellId(0);

            // Remove temporary summons which were created by this channeled spell (i.e Eye of Kilrogg)
            if (p_caster != nullptr && p_caster->getCharmGuid() != 0 && getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON))
            {
                const auto charmedUnit = p_caster->getWorldMapUnit(p_caster->getCharmGuid());
                if (charmedUnit != nullptr && charmedUnit->getCreatedBySpellId() == getSpellInfo()->getId())
                    p_caster->unPossess();
            }

            // Channel ended, remove the aura
            //\ todo: if aura is stackable, need to remove only one stack from aura instead of whole aura!
            u_caster->removeAllAurasByIdForGuid(getSpellInfo()->getId(), u_caster->getGuid());
        }
    }

    m_caster->sendMessageToSet(MsgChannelUpdate(m_caster->GetNewGUID(), time).serialise().get(), true);
}

void Spell::sendSpellStart()
{
#if VERSION_STRING == Mop
    if (!m_caster || !m_caster->IsInWorld())
        return;

    // If spell has no visuals, it's not channeled and it's triggered, no need to send packet
    if (!(getSpellInfo()->isChanneled() || getSpellInfo()->getSpeed() > 0.0f || getSpellInfo()->getSpellVisual(0) != 0 ||
        getSpellInfo()->getSpellVisual(1) != 0 || (!m_triggeredSpell && m_triggeredByAura == nullptr)))
        return;

    ObjectGuid casterGuid = i_caster ? i_caster ->getGuid() : m_caster->getGuid();
    ObjectGuid casterUnitGuid = m_caster->getGuid();
    ObjectGuid targetGuid = m_targets.getGameObjectTargetGuid();
    ObjectGuid itemTargetGuid = m_targets.getItemTargetGuid();
    ObjectGuid unkGuid = 0;
    bool hasDestLocation = (m_targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) && m_targets.getDestination().isSet();
    bool hasSourceLocation = (m_targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) && m_targets.getDestination().isSet();
    bool hasTargetString = false;// m_targets.getTargetMask()& TARGET_FLAG_STRING;
    bool hasPredictedHeal = false;
    bool hasPredictedType = false;
    bool hasTargetMask = m_targets.getTargetMask() != 0;
    bool hasCastImmunities = false;
    bool hasCastSchoolImmunities = false;
    bool hasElevation = false;
    bool hasVisualChain = false;
    bool hasAmmoInventoryType = false;
    bool hasAmmoDisplayId = false;
    uint8_t runeCooldownPassedCount = 0;
    uint8_t predictedPowerCount = 0;

    WorldPacket data(SMSG_SPELL_START, 25);

    data.writeBits(0, 24); // Miss Count (not used currently in SMSG_SPELL_START)
    data.writeBit(casterGuid[5]);

    //for (uint32_t i = 0; i < missCount; ++i)
    //{
    //}

    data.writeBit(1); // Unk read int8_t
    data.writeBit(0); // Fake Bit
    data.writeBit(casterUnitGuid[4]);
    data.writeBit(casterGuid[2]);
    data.writeBits(runeCooldownPassedCount, 3); // Rune Cooldown Passed Count
    data.writeBit(casterUnitGuid[2]);
    data.writeBit(casterUnitGuid[6]);
    data.writeBits(0, 25); // MissType Count (not used currently in SMSG_SPELL_START)
    data.writeBits(0, 13); // Unknown Bits
    data.writeBit(casterGuid[4]);
    data.writeBits(0, 24); // Hit Count (not used currently in SMSG_SPELL_START)
    data.writeBit(casterUnitGuid[7]);

    //for (uint32_t i = 0; i < hitCount; ++i)
    //{
    //}

    data.writeBit(hasSourceLocation);
    data.writeBits(predictedPowerCount, 21);

    data.writeBit(itemTargetGuid[3]);
    data.writeBit(itemTargetGuid[0]);
    data.writeBit(itemTargetGuid[1]);
    data.writeBit(itemTargetGuid[7]);
    data.writeBit(itemTargetGuid[2]);
    data.writeBit(itemTargetGuid[6]);
    data.writeBit(itemTargetGuid[4]);
    data.writeBit(itemTargetGuid[5]);

    data.writeBit(!hasElevation);
    data.writeBit(!hasTargetString);
    data.writeBit(!hasAmmoInventoryType);
    data.writeBit(hasDestLocation);
    data.writeBit(1); // Unk Read32
    data.writeBit(casterGuid[3]);

    if (hasDestLocation)
    {

    }

    data.writeBit(!hasAmmoDisplayId);

    if (hasSourceLocation)
    {

    }

    data.writeBit(0); // Fake Bit
    data.writeBit(casterGuid[6]);

    data.writeBit(unkGuid[2]);
    data.writeBit(unkGuid[1]);
    data.writeBit(unkGuid[7]);
    data.writeBit(unkGuid[6]);
    data.writeBit(unkGuid[0]);
    data.writeBit(unkGuid[5]);
    data.writeBit(unkGuid[3]);
    data.writeBit(unkGuid[4]);

    data.writeBit(!hasTargetMask);

    if (hasTargetMask)
        data.writeBits(m_targets.getTargetMask(), 20);

    data.writeBit(casterGuid[1]);
    data.writeBit(!hasPredictedHeal);
    data.writeBit(1); // Unk read int8_t
    data.writeBit(!hasCastSchoolImmunities);
    data.writeBit(casterUnitGuid[5]);
    data.writeBit(0); // Fake Bit
    data.writeBits(0, 20); // Extra Target Count (not used currently in SMSG_SPELL_START)

    //for (uint32_t i = 0; i < extraTargetCount; ++i)
    //{
    //}

    data.writeBit(targetGuid[1]);
    data.writeBit(targetGuid[4]);
    data.writeBit(targetGuid[6]);
    data.writeBit(targetGuid[7]);
    data.writeBit(targetGuid[5]);
    data.writeBit(targetGuid[3]);
    data.writeBit(targetGuid[0]);
    data.writeBit(targetGuid[2]);

    data.writeBit(casterGuid[0]);
    data.writeBit(casterUnitGuid[3]);
    data.writeBit(1); // Unk uint8_t



    //for (uint32_t i = 0; i < missTypeCount; ++i)
    //{
    //}

    data.writeBit(!hasCastImmunities);
    data.writeBit(casterUnitGuid[1]);
    data.writeBit(hasVisualChain);
    data.writeBit(casterGuid[7]);
    data.writeBit(!hasPredictedType);
    data.writeBit(casterUnitGuid[0]);

    data.flushBits();

    data.WriteByteSeq(itemTargetGuid[1]);
    data.WriteByteSeq(itemTargetGuid[7]);
    data.WriteByteSeq(itemTargetGuid[6]);
    data.WriteByteSeq(itemTargetGuid[0]);
    data.WriteByteSeq(itemTargetGuid[4]);
    data.WriteByteSeq(itemTargetGuid[2]);
    data.WriteByteSeq(itemTargetGuid[3]);
    data.WriteByteSeq(itemTargetGuid[5]);

    //for (uint32_t i = 0; i < hitCount; ++i)
    //{
    //}

    data.WriteByteSeq(targetGuid[4]);
    data.WriteByteSeq(targetGuid[5]);
    data.WriteByteSeq(targetGuid[1]);
    data.WriteByteSeq(targetGuid[7]);
    data.WriteByteSeq(targetGuid[6]);
    data.WriteByteSeq(targetGuid[3]);
    data.WriteByteSeq(targetGuid[2]);
    data.WriteByteSeq(targetGuid[0]);

    data << uint32_t(m_castTime);

    data.WriteByteSeq(unkGuid[4]);
    data.WriteByteSeq(unkGuid[5]);
    data.WriteByteSeq(unkGuid[3]);
    data.WriteByteSeq(unkGuid[2]);
    data.WriteByteSeq(unkGuid[1]);
    data.WriteByteSeq(unkGuid[6]);
    data.WriteByteSeq(unkGuid[7]);
    data.WriteByteSeq(unkGuid[0]);




    data.WriteByteSeq(casterGuid[4]);

    //for (uint32_t i = 0; i < missCount; ++i)
    //{
    //}

    if (hasCastSchoolImmunities)
        data << uint32_t(0);

    data.WriteByteSeq(casterGuid[2]);

    if (hasCastImmunities)
        data << uint32_t(0);

    if (hasVisualChain)
    {
        data << uint32_t(0);
        data << uint32_t(0);
    }


    data << uint32_t(0);

    data.WriteByteSeq(casterGuid[5]);
    data.WriteByteSeq(casterGuid[7]);
    data.WriteByteSeq(casterGuid[1]);

    data << uint8_t(1);

    data.WriteByteSeq(casterUnitGuid[7]);
    data.WriteByteSeq(casterUnitGuid[0]);
    data.WriteByteSeq(casterGuid[6]);
    data.WriteByteSeq(casterGuid[0]);
    data.WriteByteSeq(casterUnitGuid[1]);

    if (hasAmmoInventoryType)
        data << uint8_t(0);

    if (hasPredictedHeal)
        data << uint32_t(0);

    data.WriteByteSeq(casterUnitGuid[6]);
    data.WriteByteSeq(casterUnitGuid[3]);

    data << uint32_t(m_spellInfo->getId());

    if (hasAmmoDisplayId)
        data << uint32_t(0);

    data.WriteByteSeq(casterUnitGuid[4]);
    data.WriteByteSeq(casterUnitGuid[5]);
    data.WriteByteSeq(casterUnitGuid[2]);


    if (hasPredictedType)
        data << uint8_t(0);

    data.WriteByteSeq(casterGuid[3]);

    m_caster->sendMessageToSet(&data, true);
#else
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

    m_caster->sendMessageToSet(&data, true);
#endif
}

void Spell::sendSpellGo()
{
#if VERSION_STRING == Mop
    if (!m_caster || !m_caster->IsInWorld())
        return;

    // If spell has no visuals, it's not channeled and it's triggered, no need to send packet
    if (!(getSpellInfo()->isChanneled() || getSpellInfo()->getSpeed() > 0.0f || getSpellInfo()->getSpellVisual(0) != 0 ||
        getSpellInfo()->getSpellVisual(1) != 0 || (!m_triggeredSpell && m_triggeredByAura == nullptr)))
        return;

    ObjectGuid casterGuid = i_caster ? i_caster->getGuid() : m_caster->getGuid();
    ObjectGuid casterUnitGuid = m_caster->getGuid();
    ObjectGuid targetGuid = m_targets.getGameObjectTargetGuid();
    ObjectGuid itemTargetGuid = m_targets.getItemTargetGuid();
    ObjectGuid unkGuid = 0;
    bool hasDestLocation = (m_targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) && m_targets.getDestination().isSet();
    bool hasSourceLocation = (m_targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) && m_targets.getDestination().isSet();
    bool hasDestUnkByte = m_targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION;
    bool hasTargetString = m_targets.getTargetMask() & TARGET_FLAG_STRING;
    bool hasPredictedHeal = false;
    bool hasPredictedType = false;
    bool hasTargetMask = m_targets.getTargetMask() != 0;
    bool hasCastImmunities = false;
    bool hasCastSchoolImmunities = false;
    bool hasElevation = false;
    bool hasDelayTime = false;
    bool hasVisualChain = false;
    bool hasAmmoInventoryType = false;
    bool hasAmmoDisplayId = false;
    bool hasRunesStateBefore = false;
    bool hasRunesStateAfter = false;
    uint8_t predictedPowerCount = false;
    uint8_t runeCooldownPassedCount = false;

    WorldPacket data(SMSG_SPELL_GO, 60);

    data.writeBit(casterUnitGuid[2]);
    data.writeBit(1); // hasAmmoDisplayType
    data.writeBit(hasSourceLocation);
    data.writeBit(casterGuid[2]);


    data.writeBit(casterGuid[6]);
    data.writeBit(!hasDestUnkByte);
    data.writeBit(casterUnitGuid[7]);
    data.writeBits(0, 20); // Extra Target Count

    size_t missTypeCountPos = data.bitwpos();
    data.writeBits(0, 25); // Miss Type Count

    size_t missCountPos = data.bitwpos();
    data.writeBits(0, 24); // Miss Count

    data.writeBit(casterUnitGuid[1]);
    data.writeBit(casterGuid[0]);
    data.writeBits(0, 13); // Unknown bits


    data.writeBit(casterUnitGuid[5]);
    data.writeBit(0); // Fake bit
    data.writeBit(0); // Fake bit
    data.writeBit(!hasTargetString);

    data.writeBit(itemTargetGuid[7]);
    data.writeBit(itemTargetGuid[2]);
    data.writeBit(itemTargetGuid[1]);
    data.writeBit(itemTargetGuid[3]);
    data.writeBit(itemTargetGuid[6]);
    data.writeBit(itemTargetGuid[0]);
    data.writeBit(itemTargetGuid[5]);
    data.writeBit(itemTargetGuid[4]);

    data.writeBit(casterGuid[7]);

    data.writeBit(targetGuid[0]);
    data.writeBit(targetGuid[6]);
    data.writeBit(targetGuid[5]);
    data.writeBit(targetGuid[7]);
    data.writeBit(targetGuid[4]);
    data.writeBit(targetGuid[2]);
    data.writeBit(targetGuid[3]);
    data.writeBit(targetGuid[1]);

    data.writeBit(!hasRunesStateBefore);
    data.writeBits(predictedPowerCount, 21); // predictedPowerCount
    data.writeBit(casterGuid[1]);
    data.writeBit(!hasPredictedType);
    data.writeBit(!hasTargetMask);
    data.writeBit(casterUnitGuid[3]);

    data.writeBit(1); // Missing Predict heal
    data.writeBit(0); // hasPowerData
    data.writeBit(1); // has castImmunitiy
    data.writeBit(casterUnitGuid[6]);
    data.writeBit(0); // Fake bit
    data.writeBit(hasVisualChain);

    data.writeBit(unkGuid[7]);
    data.writeBit(unkGuid[6]);
    data.writeBit(unkGuid[1]);
    data.writeBit(unkGuid[2]);
    data.writeBit(unkGuid[0]);
    data.writeBit(unkGuid[5]);
    data.writeBit(unkGuid[3]);
    data.writeBit(unkGuid[4]);

    data.writeBit(!hasDelayTime);
    data.writeBit(1); // has School Immunities
    data.writeBits(runeCooldownPassedCount, 3); // runeCooldownPassedCount
    data.writeBit(casterUnitGuid[0]);

    if (hasTargetMask)
        data.writeBits(m_targets.getTargetMask(), 20);

    data.writeBit(!hasElevation);
    data.writeBit(!hasRunesStateAfter);
    data.writeBit(casterGuid[4]);
    data.writeBit(1); // hasAmmodisplayID
    data.writeBit(hasDestLocation);
    data.writeBit(casterGuid[5]);

    data.writeBits(0, 24); // Hit Count

    data.writeBit(casterUnitGuid[4]);

    data.writeBit(casterGuid[3]);
    data.flushBits();

    data.WriteByteSeq(targetGuid[5]);
    data.WriteByteSeq(targetGuid[2]);
    data.WriteByteSeq(targetGuid[1]);
    data.WriteByteSeq(targetGuid[6]);
    data.WriteByteSeq(targetGuid[0]);
    data.WriteByteSeq(targetGuid[3]);
    data.WriteByteSeq(targetGuid[4]);
    data.WriteByteSeq(targetGuid[7]);

    data.WriteByteSeq(itemTargetGuid[5]);
    data.WriteByteSeq(itemTargetGuid[2]);
    data.WriteByteSeq(itemTargetGuid[0]);
    data.WriteByteSeq(itemTargetGuid[6]);
    data.WriteByteSeq(itemTargetGuid[7]);
    data.WriteByteSeq(itemTargetGuid[3]);
    data.WriteByteSeq(itemTargetGuid[1]);
    data.WriteByteSeq(itemTargetGuid[4]);

    data.WriteByteSeq(casterGuid[2]);

    data.WriteByteSeq(unkGuid[6]);
    data.WriteByteSeq(unkGuid[2]);
    data.WriteByteSeq(unkGuid[7]);
    data.WriteByteSeq(unkGuid[1]);
    data.WriteByteSeq(unkGuid[4]);
    data.WriteByteSeq(unkGuid[3]);
    data.WriteByteSeq(unkGuid[5]);
    data.WriteByteSeq(unkGuid[0]);


    data << uint32_t(Util::getMSTime());

    data.WriteByteSeq(casterGuid[6]);
    data.WriteByteSeq(casterUnitGuid[7]);
    data.WriteByteSeq(casterGuid[1]);

    if (hasVisualChain)
    {
        data << uint32_t(0);
        data << uint32_t(0);
    }

    data << uint32_t(0);

    data.WriteByteSeq(casterUnitGuid[6]);

    if (hasPredictedType)
        data << uint8_t(0);

    data.WriteByteSeq(casterGuid[4]);
    data.WriteByteSeq(casterUnitGuid[1]);

    data.WriteByteSeq(casterGuid[0]);

    data << uint8_t(0);

    data.WriteByteSeq(casterGuid[5]);
    data.WriteByteSeq(casterUnitGuid[2]);
    data.WriteByteSeq(casterGuid[3]);
    data.WriteByteSeq(casterUnitGuid[5]);

    data << uint32_t(m_spellInfo->getId());

    data.WriteByteSeq(casterUnitGuid[0]);
    data.WriteByteSeq(casterUnitGuid[3]);
    data.WriteByteSeq(casterUnitGuid[4]);
    data.WriteByteSeq(casterGuid[7]);

    m_caster->sendMessageToSet(&data, true);
#else
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

    if (!m_missedTargets.empty())
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
    data << uint8_t(m_uniqueHittedTargets.size());
    for (const auto& uniqueTarget : m_uniqueHittedTargets)
    {
        data << uint64_t(uniqueTarget.first);
    }

#if VERSION_STRING >= WotLK
    // Add missed targets
    if (castFlags & SPELL_PACKET_FLAGS_EXTRA_MESSAGE)
    {
        data << uint8_t(m_missedTargets.size());
        writeSpellMissedTargets(&data);
    }
    else
    {
        data << uint8_t(0);
    }

    m_targets.write(data);


    if (castFlags & SPELL_PACKET_FLAGS_POWER_UPDATE && u_caster != nullptr)
        data << uint32_t(u_caster->getPower(getSpellInfo()->getPowerType()));
#else
    data << uint8_t(m_missedTargets.size());

    if (castFlags & SPELL_PACKET_FLAGS_EXTRA_MESSAGE)
        writeSpellMissedTargets(&data);

    m_targets.write(data);
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
            const uint8_t runeMask = 1U << i;
            if ((runeMask & m_rune_avail_before) != (runeMask & currentRunes))
                data << uint8_t(0); // Value of the rune converted into byte. We just think it is 0 but maybe it is not
        }
    }

    if (castFlags & SPELL_PACKET_FLAGS_UPDATE_MISSILE)
    {
        data << float(m_missilePitch);
        data << uint32_t(m_missileTravelTime);
    }

    // Some spells require this
    if (m_targets.hasDestination())
        data << uint8_t(0);
#endif

    m_caster->sendMessageToSet(&data, true);
#endif
}

void Spell::sendChannelStart(const uint32_t duration)
{
    m_caster->sendMessageToSet(MsgChannelStart(m_caster->GetNewGUID(), getSpellInfo()->getId(), duration).serialise().get(), true);

    Object const* channelTarget = nullptr;
    if (!m_uniqueHittedTargets.empty())
    {
        // Select first target from uniqueHittedTargets
        // brief: the channel target is properly set in SpellEffects.cpp for persistent dynamic objects
        for (const auto& targetGuid : m_uniqueHittedTargets)
        {
            const auto targetUnit = m_caster->getWorldMapUnit(targetGuid.first);
            if (targetUnit != nullptr)
            {
                channelTarget = targetUnit;
                break;
            }

            const auto objTarget = m_caster->getWorldMapGameObject(targetGuid.first);
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

    m_timer = duration;
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
                auto areaGroup = sAreaGroupStore.lookupEntry(static_cast<uint32_t>(getSpellInfo()->getRequiresAreaId()));
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
                parameter1 = static_cast<uint32_t>(getSpellInfo()->getEquippedItemClass());
                parameter2 = static_cast<uint32_t>(getSpellInfo()->getEquippedItemSubClass());
            } break;
#if VERSION_STRING >= TBC
        case SPELL_FAILED_REAGENTS:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    if (getSpellInfo()->getReagent(i) == 0)
                        continue;

                    const auto itemId = static_cast<uint32_t>(getSpellInfo()->getReagent(i));
                    if (!caster->hasItem(itemId, getSpellInfo()->getReagentCount(i)))
                    {
                        parameter1 = itemId;
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
#if VERSION_STRING > TBC
            const auto entryId = u_caster->getVirtualItemSlotId(i);
#else
            const auto entryId = dynamic_cast<Creature*>(u_caster)->getVirtualItemEntry(i);
#endif
            if (entryId == 0)
                continue;

#if VERSION_STRING > TBC
            // Get the item data from DBC files
            const auto itemDBC = sItemStore.lookupEntry(entryId);
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
#else
            // Get the item data from unitdata
            const auto itemData = u_caster->getVirtualItemInfoFields(i);
            if (itemData.fields.item_class != ITEM_CLASS_WEAPON)
                continue;

            switch (itemData.fields.item_subclass)
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
#if VERSION_STRING > TBC
    else
    {
        *data << uint32_t(0);
        *data << uint32_t(0);
    }
#endif
}

void Spell::writeSpellMissedTargets(WorldPacket *data)
{
    if (u_caster != nullptr && u_caster->isAlive())
    {
        for (const auto& target : m_missedTargets)
        {
            *data << uint64_t(target.targetGuid);
            *data << uint8_t(target.hitResult);
            // Need to send hit result for the reflected spell
            if (target.hitResult == SPELL_DID_HIT_REFLECT)
                *data << uint8_t(target.extendedHitResult);
        }
    }
    else
    {
        for (const auto& target : m_missedTargets)
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
    else if (getSpellInfo()->getPowerType() == POWER_TYPE_MANA && m_powerCost > 0)
    {
        // Start five second timer later at spell cast if spell has a mana cost
        m_usesMana = true;
    }

    if (!getSpellInfo()->hasValidPowerType())
    {
        sLogger.failure("Spell::takePower : Unknown power type {} for spell id {}", getSpellInfo()->getPowerType(), getSpellInfo()->getId());
        return;
    }

    u_caster->modPower(getSpellInfo()->getPowerType(), -static_cast<int32_t>(m_powerCost));
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
// Targets
bool Spell::hasTarget(const uint64_t& _guid, std::vector<uint64_t>* tmpGuidMap)
{
    for (const uint64_t& guid : *tmpGuidMap)
        if (guid == _guid)
            return true;

    for (const auto target: m_missedTargets)
        if (target.targetGuid == _guid)
            return true;

    return false;
}

Item* Spell::getItemTarget() const { return m_itemTarget; }

void Spell::setUnitTarget(Unit* _unit) { m_unitTarget = _unit; }
Unit* Spell::getUnitTarget() const { return m_unitTarget; }

Player* Spell::getPlayerTarget() const { return m_playerTarget; }

GameObject* Spell::getGameObjectTarget() const { return m_gameObjTarget; }

Corpse* Spell::getCorpseTarget() const { return m_corpseTarget; }

void Spell::unsetAllTargets()
{
    m_unitTarget = nullptr;
    m_itemTarget = nullptr;
    m_gameObjTarget = nullptr;
    m_playerTarget = nullptr;
    m_corpseTarget = nullptr;
}

void Spell::setTargetConstraintCreature(Creature* _creature) { m_targetConstraintCreature = _creature; }
Creature* Spell::getTargetConstraintCreature() const { return m_targetConstraintCreature; }

void Spell::setTargetConstraintGameObject(GameObject* _gameobject) { m_targetConstraintGameObject = _gameobject; }
GameObject* Spell::getTargetConstraintGameObject() const { return m_targetConstraintGameObject; }

SpellCastResult Spell::checkExplicitTarget(Object* target, uint32_t requiredTargetMask) const
{
    if (target == nullptr || !target->IsInWorld())
        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

    // Check if spell requires only gameobject targets
    if (requiredTargetMask == SPELL_TARGET_REQUIRE_GAMEOBJECT)
    {
        if (!target->isGameObject())
            return SPELL_FAILED_BAD_TARGETS;
    }

    // Check if spell requires either gameobject or item target
    if (requiredTargetMask == (SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ITEM))
    {
        if (!target->isGameObject() && !target->isItem())
            return SPELL_FAILED_BAD_TARGETS;
    }

    // Check if spell can target gameobjects
    if (target->isGameObject() && !m_triggeredSpell && !(requiredTargetMask & SPELL_TARGET_OBJECT_SCRIPTED) && !(requiredTargetMask & SPELL_TARGET_REQUIRE_GAMEOBJECT))
        return SPELL_FAILED_BAD_TARGETS;

    // Check if spell can target items
    if (target->isItem() && !m_triggeredSpell && !(requiredTargetMask & SPELL_TARGET_REQUIRE_ITEM))
        return SPELL_FAILED_BAD_TARGETS;

    // Check if spell can target friendly unit
    if (requiredTargetMask & SPELL_TARGET_REQUIRE_FRIENDLY && !m_caster->isFriendlyTo(target))
        return SPELL_FAILED_BAD_TARGETS;

    // Check if spell can target attackable unit
    if (requiredTargetMask & SPELL_TARGET_REQUIRE_ATTACKABLE && !(requiredTargetMask & SPELL_TARGET_AREA_SELF && m_caster == target) && !m_caster->isValidTarget(target, getSpellInfo()))
        return SPELL_FAILED_BAD_TARGETS;

    if (requiredTargetMask & SPELL_TARGET_OBJECT_TARCLASS)
    {
        const auto* const originalTarget = m_caster->getWorldMapObject(m_targets.getUnitTargetGuid());
        if (originalTarget == nullptr)
            return SPELL_FAILED_BAD_TARGETS;
        if (originalTarget->isPlayer() != target->isPlayer())
            return SPELL_FAILED_BAD_TARGETS;
        if ((originalTarget->isPlayer() && target->isPlayer() && static_cast<Player const*>(originalTarget)->getClass() != static_cast<Player const*>(target)->getClass()))
            return SPELL_FAILED_BAD_TARGETS;
    }

    // Check if spell requires pet target
    if (requiredTargetMask & SPELL_TARGET_OBJECT_CURPET && !target->isPet())
    {
        // If spell also requires item target, check for item
        if (requiredTargetMask & SPELL_TARGET_REQUIRE_ITEM)
        {
            if (!target->isItem())
                return SPELL_FAILED_BAD_TARGETS;
        }
        else
        {
            return SPELL_FAILED_BAD_TARGETS;
        }
    }

    // Area spells cannot target totems or dead units unless spell caster is the target
    if (m_caster != target &&
        ((target->isCreatureOrPlayer() && !static_cast<Unit const*>(target)->isAlive()) || (target->isCreature() && target->isTotem()))
        && (requiredTargetMask & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID)))
        return SPELL_FAILED_BAD_TARGETS;

    return SPELL_CAST_SUCCESS;
}

void Spell::safeAddMissedTarget(uint64_t targetGuid, SpellDidHitResult hitResult, SpellDidHitResult extendedHitResult)
{
    for (const auto& targetMod : m_missedTargets)
    {
        // Check if target is already in the vector
        if (targetMod.targetGuid == targetGuid)
            return;
    }

    m_missedTargets.push_back(SpellTargetMod(targetGuid, hitResult, extendedHitResult));
}

//////////////////////////////////////////////////////////////////////////////////////////
// SpellInfo
SpellInfo const* Spell::getSpellInfo() const
{
    return m_spellInfo_override != nullptr ? m_spellInfo_override : m_spellInfo;
}

bool Spell::hasAttribute(SpellAttributes _attribute) const
{
    return (getSpellInfo()->getAttributes() & _attribute) != 0;
}

bool Spell::hasAttributeEx(SpellAttributesEx _attribute) const
{
    return (getSpellInfo()->getAttributesEx() & _attribute) != 0;
}

bool Spell::hasAttributeExB(SpellAttributesExB _attribute) const
{
    return (getSpellInfo()->getAttributesExB() & _attribute) != 0;
}

bool Spell::hasAttributeExC(SpellAttributesExC _attribute) const
{
    return (getSpellInfo()->getAttributesExC() & _attribute) != 0;
}

bool Spell::hasAttributeExD(SpellAttributesExD _attribute) const
{
    return (getSpellInfo()->getAttributesExD() & _attribute) != 0;
}

bool Spell::hasAttributeExE(SpellAttributesExE _attribute) const
{
    return (getSpellInfo()->getAttributesExE() & _attribute) != 0;
}

bool Spell::hasAttributeExF(SpellAttributesExF _attribute) const
{
    return (getSpellInfo()->getAttributesExF() & _attribute) != 0;
}

bool Spell::hasAttributeExG(SpellAttributesExG _attribute) const
{
    return (getSpellInfo()->getAttributesExG() & _attribute) != 0;
}

void Spell::resetSpellInfoOverride()
{
    m_spellInfo_override = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
Aura* Spell::getTriggeredByAura() const
{
    return m_triggeredByAura;
}

int8_t Spell::getUsedComboPoints() const
{
    return m_usedComboPoints;
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
    if (target == nullptr || target->getWorldMap() == nullptr)
        return;

    m_critTargets.push_back(target->getGuid());
}

int32_t Spell::getDuration()
{
    if (isDurationSet)
        return m_duration;

    isDurationSet = true;

    if (!getSpellInfo()->getDurationIndex())
    {
        m_duration = -1;
        return m_duration;
    }

    const auto spellDuration = sSpellDurationStore.lookupEntry(getSpellInfo()->getDurationIndex());
    if (spellDuration == nullptr)
    {
        m_duration = -1;
        return m_duration;
    }

    // Duration affected by level
    if (getUnitCaster() != nullptr && spellDuration->Duration1 < 0 && spellDuration->Duration2)
    {
        m_duration = spellDuration->Duration1 + (spellDuration->Duration2 * static_cast<int32_t>(getUnitCaster()->getLevel()));

        if (m_duration > 0 && spellDuration->Duration3 > 0 && m_duration > spellDuration->Duration3)
            m_duration = spellDuration->Duration3;

        if (m_duration < 0)
            m_duration = 0;
    }

    if (m_duration == 0)
        m_duration = spellDuration->Duration1;

    // Check if duration is affected by combo points
    if (getPlayerCaster() != nullptr)
    {
        if (const auto comboPoints = getPlayerCaster()->getComboPoints())
        {
            const auto bonus = (comboPoints * (spellDuration->Duration3 - spellDuration->Duration1)) / 5;
            if (bonus)
                m_duration += bonus;
        }
    }

    if (getUnitCaster() != nullptr && m_duration > 0)
    {
        // Apply duration modifiers
        getUnitCaster()->applySpellModifiers(SPELLMOD_DURATION, &m_duration, getSpellInfo(), this);

        // Apply haste bonus
        if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HASTE_AFFECTS_DURATION)
            m_duration = static_cast<int32_t>(m_duration * getUnitCaster()->getModCastSpeed());
    }

    return m_duration;
}

float_t Spell::getEffectRadius(uint8_t effectIndex)
{
    if (m_isEffectRadiusSet[effectIndex])
        return m_effectRadius[effectIndex];

    m_isEffectRadiusSet[effectIndex] = true;
    m_effectRadius[effectIndex] = ::GetRadius(sSpellRadiusStore.lookupEntry(getSpellInfo()->getEffectRadiusIndex(effectIndex)));

    // If spell has no effect radius set, use spell range instead
    // but skip for effect target 87. Otherwise some teleport spells like ICC teleports will target
    // all units in infinite range
    if (G3D::fuzzyEq(m_effectRadius[effectIndex], 0.f) &&
        getSpellInfo()->getEffectImplicitTargetA(effectIndex) != EFF_TARGET_AREA_DESTINATION && getSpellInfo()->getEffectImplicitTargetB(effectIndex) != EFF_TARGET_AREA_DESTINATION)
    {
        const auto rangeEntry = sSpellRangeStore.lookupEntry(getSpellInfo()->getRangeIndex());
        if (rangeEntry != nullptr)
        {
#if VERSION_STRING < WotLK
            m_effectRadius[effectIndex] = rangeEntry->maxRange;
#else
            const auto effectTargetMask = getSpellInfo()->getRequiredTargetMaskForEffect(effectIndex);
            m_effectRadius[effectIndex] = effectTargetMask & SPELL_TARGET_REQUIRE_ATTACKABLE ? rangeEntry->maxRange : rangeEntry->maxRangeFriendly;
#endif
        }
    }

    // Apply radius modifiers
    if (getUnitCaster() != nullptr)
        getUnitCaster()->applySpellModifiers(SPELLMOD_RADIUS, &m_effectRadius[effectIndex], getSpellInfo(), this);

    return m_effectRadius[effectIndex];
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

void Spell::removeCastItem()
{
    if (getItemCaster() == nullptr)
        return;

    auto removable = false, chargesUsed = false;
    const auto proto = getItemCaster()->getItemProperties();

    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        const auto protoSpell = proto->Spells[i];
        if (protoSpell.Id > 0)
        {
            if (protoSpell.Charges == 0)
                continue;

            // Items with negative charges disappear when they reach 0 charges
            if (protoSpell.Charges < 0)
                removable = true;

            auto charges = getItemCaster()->getSpellCharges(i);
            if (charges != 0 && protoSpell.Id == getSpellInfo()->getId())
            {
                if (charges > 0)
                    --charges;
                else
                    ++charges;

                // If item is not stackable, modify charges
                if (proto->MaxCount == 1)
                    getItemCaster()->setSpellCharges(i, charges);

                getItemCaster()->m_isDirty = true;
            }

            chargesUsed = charges == 0;
        }
    }

    if (removable && chargesUsed)
    {
        // If the item is stacked, remove 1 from the stack
        if (getItemCaster()->getStackCount() > 1)
        {
            getItemCaster()->modStackCount(-1);
            getItemCaster()->m_isDirty = true;
        }
        else
        {
            getItemCaster()->getOwner()->getItemInterface()->SafeFullRemoveItemByGuid(getItemCaster()->getGuid());
        }

        i_caster = nullptr;
    }
}

void Spell::removeReagents()
{
    if (p_caster == nullptr)
        return;

#if VERSION_STRING >= TBC
    if (!(p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST) && getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_REAGENT_REMOVAL))
#endif
    {
        for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            if (getSpellInfo()->getReagent(i) == 0)
                continue;

            const auto itemId = static_cast<uint32_t>(getSpellInfo()->getReagent(i));
            p_caster->getItemInterface()->RemoveItemAmt_ProtectPointer(itemId, getSpellInfo()->getReagentCount(i), &i_caster);
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
                p_caster->applyItemMods(rangedWeapon, EQUIPMENT_SLOT_RANGED, false, true);
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
        [[fallthrough]];
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
            sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Spell::_updateCasterPointers : Incompatible object type (type {}) for spell caster", caster->getObjectTypeId());
            break;
    }
}

void Spell::_updateTargetPointers(const uint64_t targetGuid)
{
    unsetAllTargets();

    if (targetGuid == 0)
    {
        if (getPlayerCaster() != nullptr)
        {
            if (m_targets.getTargetMask() & TARGET_FLAG_ITEM)
                m_itemTarget = getPlayerCaster()->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());

            if (m_targets.isTradeItem())
            {
                const auto trader = getPlayerCaster()->getTradeTarget();
                if (trader != nullptr)
                    m_itemTarget = trader->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
            }
        }
    }
    else if (targetGuid == getCaster()->getGuid())
    {
        m_unitTarget = getUnitCaster();
        m_itemTarget = getItemCaster();
        m_gameObjTarget = getGameObjectCaster();
        m_playerTarget = getPlayerCaster();
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
                    m_itemTarget = trader->getTradeData()->getTradeItem(TradeSlots(targetGuid));
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
                    m_unitTarget = getCaster()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Pet:
                    m_unitTarget = getCaster()->getWorldMap()->getPet(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Player:
                    m_unitTarget = getCaster()->getWorldMap()->getPlayer(wowGuid.getGuidLowPart());
                    m_playerTarget = dynamic_cast<Player*>(m_unitTarget);
                    break;
                case HighGuid::Item:
                    if (getPlayerCaster() != nullptr)
                        m_itemTarget = getPlayerCaster()->getItemInterface()->GetItemByGUID(targetGuid);
                    break;
                case HighGuid::GameObject:
                    m_gameObjTarget = getCaster()->getWorldMap()->getGameObject(wowGuid.getGuidLowPart());
                    break;
                case HighGuid::Corpse:
                    m_corpseTarget = sObjectMgr.getCorpseByGuid(wowGuid.getGuidLowPart());
                    break;
                default:
                    sLogger.failure("Spell::_updateTargetPointers : Invalid object type for spell target (low guid {}) in spell {}", wowGuid.getGuidLowPart(), getSpellInfo()->getId());
                    break;
            }
        }
    }
}

void Spell::_loadInitialTargetPointers(bool reset/* = false*/)
{
    unsetAllTargets();

    if (reset)
        return;

    if (m_targets.getGameObjectTargetGuid() != 0)
        m_gameObjTarget = m_caster->getWorldMapGameObject(m_targets.getGameObjectTargetGuid());

    if (m_targets.getItemTargetGuid() != 0 && getPlayerCaster() != nullptr)
    {
        if (m_targets.isTradeItem())
        {
            const auto* const playerTrader = getPlayerCaster()->getTradeTarget();
            if (playerTrader != nullptr)
                m_itemTarget = playerTrader->getTradeData()->getTradeItem(TradeSlots(m_targets.getItemTargetGuid()));
        }
        else
        {
            m_itemTarget = getPlayerCaster()->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
        }
    }

    if (m_targets.getUnitTargetGuid() != 0)
    {
        m_unitTarget = m_caster->getWorldMapUnit(m_targets.getUnitTargetGuid());

        if (m_unitTarget != nullptr && m_unitTarget->isPlayer())
            m_playerTarget = dynamic_cast<Player*>(m_unitTarget);
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
            const auto obj = m_caster->getWorldMapObject(guid);
            if (obj == nullptr)
                return -1.0f;

            destX = obj->GetPositionX();
            destY = obj->GetPositionY();
            //\todo this should be destz = obj->GetPositionZ() + (obj->GetModelHighBoundZ() / 2 * obj->getScale())
            if (obj->isCreatureOrPlayer())
                destZ = obj->GetPositionZ() + static_cast<Unit*>(obj)->getModelHalfSize();
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
    return distance * 1000.0f / getSpellInfo()->getSpeed();
}

void Spell::_prepareProcFlags()
{
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
            getSpellInfo()->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL
#if VERSION_STRING >= TBC
            || getSpellInfo()->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
#endif
            )
            continue;

        spellTargetMask |= getSpellInfo()->getRequiredTargetMaskForEffect(i);
    }

    if (spellTargetMask == 0)
        return;

    // Set initial proc flags here, correct flags are set in Object::doSpellDamage and ::doSpellHealing
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

    auto isCasterOnlyTarget = false;
    if (m_uniqueHittedTargets.size() == 1)
        isCasterOnlyTarget = m_caster->getGuid() == m_uniqueHittedTargets.front().first;

    // These proc flags should not be applied if spell has no targets or is only targeting caster
    if (!(spellTargetMask & SPELL_TARGET_OBJECT_SELF) && m_uniqueHittedTargets.size() > 0 && !isCasterOnlyTarget)
    {
        // Set initial flags here, correct flags are set in Unit::Strike
        if (spellDamageType == SPELL_DMG_TYPE_MELEE)
        {
            m_casterProcFlags = PROC_ON_DONE_MELEE_SPELL_HIT;
            m_targetProcFlags = PROC_ON_TAKEN_MELEE_SPELL_HIT;
        }
        else if (spellDamageType == SPELL_DMG_TYPE_RANGED)
        {
            m_casterProcFlags = PROC_ON_DONE_RANGED_SPELL_HIT;
            m_targetProcFlags = PROC_ON_TAKEN_RANGED_SPELL_HIT;
        }
    }
}
