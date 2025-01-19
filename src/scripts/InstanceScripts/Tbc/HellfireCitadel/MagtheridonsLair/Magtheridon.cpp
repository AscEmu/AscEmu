/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Magtheridon.hpp"
#include "Raid_Magtheridons_Lair.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Magtheridon
MagtheridonAI::MagtheridonAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    shadowCage = addAISpell(SPELL_SHADOW_CAGE_C, 0.0f, TARGET_SELF);
    shadowCage->mIsTriggered = true;
    cleave = addAISpell(SPELL_CLEAVE, 0.0f, TARGET_ATTACKING);
    blaseTarget = addAISpell(SPELL_BLAZE_TARGET, 0.0f, TARGET_VARIOUS);
    quake = addAISpell(SPELL_QUAKE, 0.0f, TARGET_VARIOUS);
    blastNova = addAISpell(SPELL_BLAST_NOVA, 0.0f, TARGET_VARIOUS);
    berserk = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
    shake = addAISpell(SPELL_CAMERA_SHAKE, 0.0f, TARGET_VARIOUS);
}

CreatureAIScript* MagtheridonAI::Create(Creature* pCreature) { return new MagtheridonAI(pCreature); }

void MagtheridonAI::OnLoad()
{
    Reset();
}

void MagtheridonAI::CombatStart()
{
    scriptEvents.removeEvent(EVENT_START_FIGHT);
    scriptEvents.removeEvent(EVENT_NEARLY_EMOTE);
    scriptEvents.addEvent(EVENT_RELEASED, 6000);

    getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s breaks free!");
    sendDBChatMessage(SAY_FREE);
    _removeAura(SPELL_SHADOW_CAGE_C);
}

void MagtheridonAI::OnCombatStop(Unit* /*mTarget*/)
{
    scriptEvents.resetEvents();
    getInstanceScript()->setLocalData(DATA_DOOR, ACTION_ENABLE);
    getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_DISABLE);
    getInstanceScript()->setLocalData(DATA_COLLAPSE, ACTION_DISABLE);
    getInstanceScript()->setLocalData(DATA_COLLAPSE_2, ACTION_DISABLE);

    Reset();
}

void MagtheridonAI::OnDied(Unit* /*killer*/)
{
    sendDBChatMessage(SAY_DEATH);
    getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_DISABLE);
}

void MagtheridonAI::OnTargetDied(Unit* target)
{
    if (target->getObjectTypeId() == TYPEID_PLAYER)
        sendDBChatMessage(SAY_SLAY);
}  

void MagtheridonAI::AIUpdate(unsigned long time_passed)
{
    if(isScriptPhase(PHASE_BANISH))
        _castAISpell(shadowCage);

    if (!isScriptPhase(PHASE_BANISH) && !isScriptPhase(PHASE_1) && !getCreature()->getAIInterface()->getCurrentTarget())
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    if (_isCasting())
        return;

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
        case EVENT_BERSERK :
            _castAISpell(berserk);
            break;
        case EVENT_CLEAVE:
            _castAISpell(cleave);
            scriptEvents.addEvent(EVENT_CLEAVE, 10000);
            break;
        case EVENT_BLAZE:
            _castAISpell(blaseTarget);
            scriptEvents.addEvent(EVENT_BLAZE, 20000);
            break;
        case EVENT_QUAKE:
            _castAISpell(quake);
            scriptEvents.addEvent(EVENT_QUAKE, 60000);
            break;
        case EVENT_START_FIGHT:
            CombatStart();
            break;
        case EVENT_RELEASED:
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
            getCreature()->getAIInterface()->setImmuneToPC(false);
            getCreature()->getAIInterface()->setImmuneToNPC(false);
            getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            setScriptPhase(PHASE_2);

            getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_ENABLE);
            scriptEvents.addEvent(EVENT_CLEAVE, 10000);
            scriptEvents.addEvent(EVENT_BLAST_NOVA, 60000);
            scriptEvents.addEvent(EVENT_BLAZE, 20000);
            scriptEvents.addEvent(EVENT_QUAKE, 35000);
            scriptEvents.addEvent(EVENT_BERSERK, 12000000);
            break;
        case EVENT_COLLAPSE:
            getInstanceScript()->setLocalData(DATA_COLLAPSE_2, ACTION_ENABLE);
            scriptEvents.addEvent(EVENT_DEBRIS_KNOCKDOWN, 4000);
            break;
        case EVENT_DEBRIS_KNOCKDOWN:
            {
                if (Creature* trigger = ((MagtheridonsLairInstanceScript*)getInstanceScript())->worldTrigger)
                {
                    trigger->castSpell(trigger, SPELL_DEBRIS_KNOCKDOWN, true);
                    getCreature()->getAIInterface()->setReactState(REACT_AGGRESSIVE);
                }
                scriptEvents.addEvent(EVENT_DEBRIS, 20000);
            }
            break;
        case EVENT_DEBRIS:
            {
                Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent);
                if(!target)
                    target = getBestPlayerTarget(TargetFilter_Current);

                if (target)
                {
                    /*Creature* debris = */spawnCreature(NPC_MAGTHERIDON_ROOM, target->GetPosition());
                    target->castSpell(target, SPELL_DEBRIS_VISUAL, true); // hackfix invis creatures cant have visual spells atm -_-
                    scriptEvents.addEvent(EVENT_DEBRIS, 20000);
                }
            }
            break;
        case EVENT_NEARLY_EMOTE:
            getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s is nearly free of his bonds!");
            break;
        case EVENT_BLAST_NOVA:
            getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s begins to cast Blast Nova!");
            _castAISpell(blastNova);
            scriptEvents.addEvent(EVENT_BLAST_NOVA, 55000);
            break;
        case EVENT_TAUNT:
            switch (Util::getRandomUInt(0, 5))
            {
            case 0:
                sendDBChatMessage(SAY_TAUNT01);
                break;
            case 1:
                sendDBChatMessage(SAY_TAUNT02);
                break;
            case 2:
                sendDBChatMessage(SAY_TAUNT03);
                break;
            case 3:
                sendDBChatMessage(SAY_TAUNT04);
                break;
            case 4:
                sendDBChatMessage(SAY_TAUNT05);
                break;
            case 5:
                sendDBChatMessage(SAY_TAUNT06);
                break;
            }
            
            scriptEvents.addEvent(EVENT_TAUNT, Util::getRandomUInt(240000, 300000));
            break;
        default:
            break;
        }
    }

    // When we have 5 Auras from the Cubes Banish us
    if (getCreature()->getAuraCountForId(SPELL_SHADOW_GRASP_VISUAL) == 5)
        _castAISpell(shadowCage);
}

void MagtheridonAI::DoAction(int32_t action)
{
    if (action == ACTION_START_CHANNELERS_EVENT && isScriptPhase(PHASE_BANISH))
    {
        setScriptPhase(PHASE_1);
        getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s's bonds begin to weaken!");

        scriptEvents.addEvent(EVENT_START_FIGHT, 120000);
        scriptEvents.addEvent(EVENT_NEARLY_EMOTE, 60000);
        scriptEvents.removeEvent(EVENT_TAUNT);

        getInstanceScript()->setBossState(DATA_MAGTHERIDON, InProgress);
    }
}

void MagtheridonAI::OnHitBySpell(uint32_t spellId, Unit* /*caster*/)
{
    if (spellId == SPELL_SHADOW_CAGE)
        sendDBChatMessage(SAY_BANISHED);
}

void MagtheridonAI::OnDamageTaken(Unit* /*_attacker*/, uint32_t /*_amount*/)
{
    if (_getHealthPercent() < 30 && !isScriptPhase(PHASE_3))
    {
        setScriptPhase(PHASE_3);
        getCreature()->getAIInterface()->setReactState(REACT_PASSIVE);
        //me->AttackStop();
        sendDBChatMessage(SAY_COLLAPSE);
        getInstanceScript()->setLocalData(DATA_COLLAPSE, ACTION_ENABLE);
        _castAISpell(shake);
        scriptEvents.addEvent(EVENT_COLLAPSE, 6000);
    }
}

void MagtheridonAI::Reset()
{
    _castAISpell(shadowCage);
    setScriptPhase(PHASE_BANISH);
    scriptEvents.addEvent(EVENT_TAUNT, 240000);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Soul Transfer
void SoulTransfer::filterEffectTargets(Spell* spell, uint8_t /*effectIndex*/, std::vector<uint64_t>* effectTargets)
{
    // Hackfix shouldnt only cast on Channelers
    effectTargets->clear();

    for (const auto& itr : spell->getUnitCaster()->getInRangeObjectsSet())
    {
        float distance = spell->getUnitCaster()->CalcDistance(itr);
        if (itr->isCreature() && itr->getEntry() == NPC_HELLFIRE_CHANNELLER && distance <= 100.0f)
        {
            effectTargets->push_back(itr->getGuid());
        }
    }
}
