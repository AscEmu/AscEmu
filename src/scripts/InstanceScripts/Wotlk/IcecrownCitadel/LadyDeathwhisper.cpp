/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "LadyDeathwhisper.hpp"
#include "Raid_IceCrownCitadel.hpp"
#include "Movement/MovementManager.h"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lady Deathwhisper
LadyDeathwhisperAI::LadyDeathwhisperAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
   
    dominateMindCount = static_cast<uint8_t>(getRaidModeValue(0, 1, 1, 3));
    introDone = false;

    // Scripted Spells not autocastet
    shadowChannelingSpell           = addAISpell(SPELL_SHADOW_CHANNELING, 0.0f, TARGET_SELF);
    manaBarrierSpell                = addAISpell(SPELL_MANA_BARRIER, 0.0f, TARGET_SELF);
    manaBarrierSpell->mIsTriggered = true;
    deathAndDecaySpell              = addAISpell(SPELL_DEATH_AND_DECAY, 0.0f, TARGET_CUSTOM);
    dominateMindHeroSpell           = addAISpell(SPELL_DOMINATE_MIND_H, 0.0f, TARGET_CUSTOM);
    shadowBoltSpell                 = addAISpell(SPELL_SHADOW_BOLT, 0.0f, TARGET_CUSTOM);
    frostBoltSpell                  = addAISpell(SPELL_FROSTBOLT, 0.0f, TARGET_RANDOM_SINGLE);
    frostBoltVolleySpell            = addAISpell(SPELL_FROSTBOLT_VOLLEY, 0.0f, TARGET_ATTACKING);
    touchOfInsignifcanceSpell       = addAISpell(SPELL_TOUCH_OF_INSIGNIFICANCE, 0.0f, TARGET_ATTACKING);
    summonShadeSpell                = addAISpell(SPELL_SUMMON_SHADE, 0.0f, TARGET_CUSTOM);
    berserkSpell                    = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
    darkMartydromSpell              = addAISpell(SPELL_DARK_MARTYRDOM_T, 0.0f, TARGET_CUSTOM);
    darkMartydromSpell->addDBEmote(SAY_LADY_DEAD);
    darkTransformationSpell         = addAISpell(SPELL_DARK_TRANSFORMATION_T, 0.0f, TARGET_CUSTOM);
    darkTransformationSpell->mIsTriggered = true;
    darkTransformationSpell->addDBEmote(SAY_LADY_TRANSFORMATION);
    darkEmpowermentSpell            = addAISpell(SPELL_DARK_EMPOWERMENT_T, 0.0f, TARGET_CUSTOM);
    darkEmpowermentSpell->mIsTriggered = true;
    darkEmpowermentSpell->addDBEmote(SAY_LADY_EMPOWERMENT);

    waveCounter = 0;
    nextVengefulShadeTargetGUID = 0;
          

    // Messages
    addEmoteForEvent(Event_OnCombatStart, SAY_LADY_AGGRO);     
    addEmoteForEvent(Event_OnTargetDied, SAY_LADY_DEAD);            
    addEmoteForEvent(Event_OnDied, SAY_LADY_DEATH);            
}

CreatureAIScript* LadyDeathwhisperAI::Create(Creature* pCreature) { return new LadyDeathwhisperAI(pCreature); }

void LadyDeathwhisperAI::IntroStart()
{
    if (!_isInCombat() && getScriptPhase() >= PHASE_INTRO)
        return;

    ///\todo Add SpellImmunities
    sendDBChatMessage(SAY_LADY_INTRO_1);
    setScriptPhase(PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_2, 11000, PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_3, 21000, PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_4, 31500, PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_5, 39500, PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_6, 48500, PHASE_INTRO);
    scriptEvents.addEvent(EVENT_INTRO_7, 58000, PHASE_INTRO);
    _castAISpell(shadowChannelingSpell);
    introDone = true;
}

void LadyDeathwhisperAI::OnCombatStart(Unit* /*pTarget*/)
{
    scriptEvents.resetEvents();       
    setScriptPhase(PHASE_ONE);

    // common events
    scriptEvents.addEvent(EVENT_BERSERK, 600000);
    scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, 10000);

    // phase one events
    scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, 5000, PHASE_ONE);
    scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5500, 6000), PHASE_ONE);
    scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(20000, 30000), PHASE_ONE);

    if (mInstance->GetDifficulty() != InstanceDifficulty::RAID_10MAN_NORMAL)
        scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, 27000);

    _setMeleeDisabled(true);
    setRooted(true);

    getCreature()->removeAllAurasById(SPELL_SHADOW_CHANNELING);
    _castAISpell(manaBarrierSpell);
}

void LadyDeathwhisperAI::OnCombatStop(Unit* /*_target*/)
{
    Reset();
}

void LadyDeathwhisperAI::DoAction(int32_t const action)
{
    if (action == ACTION_LADY_INTRO_START)
        IntroStart();

    if (action == ACTION_MANABARRIER_DOWN)
    {
        // When Lady Deathwhsiper has her mana Barrier dont deal damage to her instead reduce her mana.
        // phase transition
        if (getScriptPhase() == PHASE_ONE)
        {
            sendDBChatMessage(SAY_LADY_PHASE_2);
            sendDBChatMessage(SAY_LADY_PHASE_2_EMOTE);
            setRooted(false);
            getCreature()->setPower(POWER_TYPE_MANA, 0);
            getCreature()->getMovementManager()->moveChase(getCreature()->getAIInterface()->getCurrentTarget());
            setScriptPhase(PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 12000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(19000, 21000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(6000, 9000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(12000, 15000), PHASE_TWO);
            // on heroic mode Lady Deathwhisper is immune to taunt effects in phase 2 and continues summoning adds
            if (isHeroic())
            {
                ///\todo Add SpellImmunities
                scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
            }
        }
    }
}

void LadyDeathwhisperAI::Reset()
{
    getCreature()->setPower(POWER_TYPE_MANA, getCreature()->getMaxPower(POWER_TYPE_MANA));

    ///\todo Add SpellImmunities
    setScriptPhase(PHASE_ONE);
    scriptEvents.resetEvents();

    waveCounter = 0;
    nextVengefulShadeTargetGUID = 0;

    DeleteSummons();

    _castAISpell(shadowChannelingSpell);
    getCreature()->removeAllAuras();
}

void LadyDeathwhisperAI::AIUpdate(unsigned long time_passed)
{
    if (!_isInCombat() && !getScriptPhase() == PHASE_INTRO)
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (_isCasting())
        return;

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_INTRO_2:
            {
                sendDBChatMessage(SAY_LADY_INTRO_2);
                break;
            }
            case EVENT_INTRO_3:
            {
                sendDBChatMessage(SAY_LADY_INTRO_3);
                break;
            }
            case EVENT_INTRO_4:
            {
                sendDBChatMessage(SAY_LADY_INTRO_4);
                break;
            }
            case EVENT_INTRO_5:
            {
                sendDBChatMessage(SAY_LADY_INTRO_5);
                break;
            }
            case EVENT_INTRO_6:
            {
                sendDBChatMessage(SAY_LADY_INTRO_6);
                break;
            }
            case EVENT_INTRO_7:
            {
                sendDBChatMessage(SAY_LADY_INTRO_7);
                setScriptPhase(PHASE_ONE);
                break;
            }
            case EVENT_DEATH_AND_DECAY:
            {
                if (Unit* target = getBestPlayerTarget())
                {
                    deathAndDecaySpell->setCustomTarget(target);
                    _castAISpell(deathAndDecaySpell);
                }
                scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, Util::getRandomUInt(22000, 30000));
                break;
            }
            case EVENT_DOMINATE_MIND_H:
            {
                sendDBChatMessage(SAY_LADY_DOMINATE_MIND);
                for (uint8_t i = 0; i < dominateMindCount; i++)
                    if (Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent))
                    {
                        dominateMindHeroSpell->setCustomTarget(target);
                        _castAISpell(dominateMindHeroSpell);
                    }
                scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, Util::getRandomUInt(40000, 45000));
                break;
            }
            case EVENT_P1_SUMMON_WAVE:
            {
                SummonWavePhaseOne();
                scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, isHeroic() ? 45000 : 60000, PHASE_ONE);
                break;
            }
            case EVENT_P1_SHADOW_BOLT:
            {
                if (Unit* target = getBestPlayerTarget())
                {
                    shadowBoltSpell->setCustomTarget(target);
                    _castAISpell(shadowBoltSpell);
                }
                scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5000, 8000), PHASE_ONE);
                break;
            }
            case EVENT_P1_REANIMATE_CULTIST:
            {
                ReanimateCultist();
                break;
            }
            case EVENT_P1_EMPOWER_CULTIST:
            {
                EmpowerCultist();
                scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(18000, 25000));
                break;
            }
            case EVENT_P2_FROSTBOLT:
            {
                _castAISpell(frostBoltSpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 11000), PHASE_TWO);
                break;
            }
            case EVENT_P2_FROSTBOLT_VOLLEY:
            {
                _castAISpell(frostBoltVolleySpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(13000, 15000), PHASE_TWO);
                break;
            }
            case EVENT_P2_TOUCH_OF_INSIGNIFICANCE:
            {
                _castAISpell(touchOfInsignifcanceSpell);
                scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(9000, 13000), PHASE_TWO);
                break;
            }
            case EVENT_P2_SUMMON_SHADE:
            {
                if (Unit* shadeTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
                {
                    summonShadeSpell->setCustomTarget(shadeTarget);
                    nextVengefulShadeTargetGUID = shadeTarget->getGuid();
                    _castAISpell(summonShadeSpell);
                }
                scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(18000, 23000), PHASE_TWO);
                break;
            }
            case EVENT_P2_SUMMON_WAVE:
            {
                SummonWavePhaseTwo();
                scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
                break;
            }
            case EVENT_BERSERK:
            {
                _castAISpell(berserkSpell);
                sendDBChatMessage(SAY_LADY_BERSERK);
                break;
            }
            default:
                break;
        }
    }

    // We should not melee attack when barrier is up
    if (getCreature()->hasAurasWithId(SPELL_MANA_BARRIER))
    {
        _setMeleeDisabled(true);
        return;
    }

    _setMeleeDisabled(false);
}

// summoning function for first phase
void LadyDeathwhisperAI::SummonWavePhaseOne()
{    
    uint8_t addIndex1 = waveCounter & 1;
    uint8_t addIndex2 = uint8_t(addIndex1 ^ 1);

    // Todo summon Darnavan when weekly quest is active
    if (waveCounter)
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);

    Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
    Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);

    if (mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
    {
        Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3]);
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 1]);
        Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 2]);
        Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
    }
    ++waveCounter;
}

// summoning function for second phase
void LadyDeathwhisperAI::SummonWavePhaseTwo()
{       
    if (mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
    {
        uint8_t addIndex1 = waveCounter & 1;
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);
        Summon(SummonEntries[addIndex1 ^ 1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);
    }
    else
        Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
    ++waveCounter;
}

void LadyDeathwhisperAI::Summon(uint32_t entry, const LocationVector& pos)
{
    Creature* summon = spawnCreature(entry, pos);

    if (summon)
    {
        summon->setSummonedByGuid(getCreature()->getGuid());
        summons.push_back(summon);
    }           
}

void LadyDeathwhisperAI::DeleteSummons()
{
    if (summons.empty())
        return;

    for (const auto& summon : summons)
    {
        if (summon->IsInWorld())
            summon->Despawn(100, 0);
    }

    summons.clear();
}

void LadyDeathwhisperAI::ReanimateCultist()
{
    if (reanimationQueue.empty())
        return;

    uint64_t cultistGUID = reanimationQueue.front();
    Creature* cultist = mInstance->GetCreatureByGuid(static_cast<uint32_t>(cultistGUID));
    reanimationQueue.pop_front();
    if (!cultist)
        return;

    darkMartydromSpell->setCustomTarget(cultist);
    _castAISpell(darkMartydromSpell);
}

void LadyDeathwhisperAI::EmpowerCultist()
{
    if (summons.empty())
        return;

    std::list<Creature*> temp;
    for (auto itr = summons.begin(); itr != summons.end(); ++itr)
        if ((*itr)->isAlive() && ((*itr)->getEntry() == NPC_CULT_FANATIC || (*itr)->getEntry() == NPC_CULT_ADHERENT))
            temp.push_back((*itr));
    
    if (temp.empty())
        return;

    // select random cultist
    uint8_t i = static_cast<uint8_t>(Util::getRandomUInt(0, static_cast<uint32_t>(temp.size() - 1)));
    auto it = temp.begin();
    std::advance(it, i);

    Creature* cultist = (*it);
    if (cultist->getEntry() == NPC_CULT_FANATIC)
    {
        darkTransformationSpell->setCustomTarget(cultist);
        _castAISpell(darkTransformationSpell);
    }
    else
    {
        darkEmpowermentSpell->setCustomTarget(cultist);
        _castAISpell(darkEmpowermentSpell);
    }
}

void LadyDeathwhisperAI::SetCreatureData64(uint32_t Type, uint64_t Data)
{
    switch (Type)
    {
        case DATA_CULTIST_GUID:
        {
            reanimationQueue.push_back(Data);
            scriptEvents.addEvent(EVENT_P1_REANIMATE_CULTIST, 3000, PHASE_ONE);
            break;
        }
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Mana Barrier
SpellScriptExecuteState ManaBarrier::onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/)
{
    // Aura should periodically trigger spell every 1 sec but that spell is serverside so we don't have it in DBC
    // Overwrite default periodic tick with converting mana to hp

    const auto auraOwner = aur->getOwner();
    // If aura owner doesn't use mana, remove aura
    if (auraOwner->getMaxPower(POWER_TYPE_MANA) == 0)
    {
        aur->removeAura();
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    const auto healthDelta = auraOwner->getMaxHealth() - auraOwner->getHealth();
    if (healthDelta == 0)
    {
        // Unit is already at max health, so do nothing
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    const auto currentMana = auraOwner->getPower(POWER_TYPE_MANA);
    if (healthDelta < currentMana)
    {
        // Restore health to max and remove equal amount of mana
        auraOwner->setPower(POWER_TYPE_MANA, currentMana - healthDelta);
        auraOwner->setHealth(auraOwner->getMaxHealth());
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    // Unit takes more damage than it has mana left => remove aura
    aur->removeAura();

    // Aura is used by Lady Deathwhisper, change phase when all mana is drained
    if (auraOwner->isCreature())
    {
        const auto creatureOwner = static_cast<Creature*>(auraOwner);
        if (creatureOwner->GetScript() != nullptr)
            creatureOwner->GetScript()->DoAction(ACTION_MANABARRIER_DOWN);
    }

    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Adherent
CultAdherentAI::CultAdherentAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
    frostFeverSpell             = addAISpell(SPELL_FROST_FEVER, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10, 12));
    deathchillSpell             = addAISpell(SPELL_DEATHCHILL_BLAST, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(14, 16));
    curseOfTorporSpell          = addAISpell(SPELL_CURSE_OF_TORPOR, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(14, 16));
    shroudOfTheOccultSpell      = addAISpell(SPELL_SHORUD_OF_THE_OCCULT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(32, 39));
    cultistDarkMartyrdomSpell   = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(18, 32));
}

CreatureAIScript* CultAdherentAI::Create(Creature* pCreature) { return new CultAdherentAI(pCreature); }

void CultAdherentAI::OnLoad()
{
    _castAISpell(temporalVisualSpell);
    auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
    if (NewTarget)
    {
        getCreature()->getAIInterface()->setCurrentTarget(NewTarget);
        getCreature()->getAIInterface()->onHostileAction(NewTarget);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Fanatic
CultFanaticAI::CultFanaticAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
    necroticStrikeSpell         = addAISpell(SPELL_NECROTIC_STRIKE, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10, 12));
    shadowCleaveSpell           = addAISpell(SPELL_SHADOW_CLEAVE, 100.0f, TARGET_ATTACKING, 0 , Util::getRandomUInt(14, 16));
    vampireMightSpell           = addAISpell(SPELL_VAMPIRIC_MIGHT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(20, 27));
    darkMartyrdomSpell          = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0 , Util::getRandomUInt(18, 32));
}

CreatureAIScript* CultFanaticAI::Create(Creature* pCreature) { return new CultFanaticAI(pCreature); }

void CultFanaticAI::OnLoad()
{
    _castAISpell(temporalVisualSpell);
    auto NewTarget = getBestPlayerTarget(TargetFilter_Closest);
    if (NewTarget)
    {
        getCreature()->getAIInterface()->setCurrentTarget(NewTarget);
        getCreature()->getAIInterface()->onHostileAction(NewTarget);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Cultist Dark Martyrdom
void DarkMartyrdom::afterSpellEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex != EFF_INDEX_0)
        return;

    if (Creature* owner = spell->getCaster()->getWorldMapCreature(spell->getUnitCaster()->getSummonedByGuid()))
        owner->GetScript()->SetCreatureData64(DATA_CULTIST_GUID, spell->getUnitCaster()->getGuidLow());
}
