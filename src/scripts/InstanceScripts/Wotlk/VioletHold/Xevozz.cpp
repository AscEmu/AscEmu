/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Xevozz.hpp"
#include "Instance_TheVioletHold.hpp"
#include "Movement/MovementManager.h"
#include "Spell/SpellInfo.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//  Xevozz AI
XevozzAI::XevozzAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Xevozz::SPELL_ARCANE_BARRAGE_VOLLEY, 50.0f, TARGET_SOURCE, 0, 8);
    addAISpell(Xevozz::SPELL_ARCANE_BUFFET, 33.0f, 15, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 45.0f); });

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, Xevozz::SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, Xevozz::SAY_SLAY1);
    addEmoteForEvent(Event_OnTargetDied, Xevozz::SAY_SLAY2);
    addEmoteForEvent(Event_OnTargetDied, Xevozz::SAY_SLAY3);
    addEmoteForEvent(Event_OnDied, Xevozz::SAY_DEATH);
}

CreatureAIScript* XevozzAI::Create(Creature* pCreature) { return new XevozzAI(pCreature); }

void XevozzAI::OnLoad()
{
    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
}

void XevozzAI::OnCombatStart(Unit* /*_target*/)
{
    scriptEvents.addEvent(1, 5000);
}

void XevozzAI::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                sendRandomDBChatMessage({ Xevozz::SAY_REPEAT_SUMMON1 , Xevozz::SAY_REPEAT_SUMMON2 }, nullptr);

                uint8_t spell = SelectRandomContainerElement(summonSpells);

                getCreature()->castSpell(getCreature(), Xevozz::EtherealSphereSummonSpells[spell]);
                summonSpells.remove(spell);

                scriptEvents.addEvent(1, Util::getRandomUInt(45000, 50000));

                if (isHeroic())
                    scriptEvents.addEvent(2, 2500);
            } break;
            case 2:
            {
                uint8_t spell = SelectRandomContainerElement(summonSpells);
                getCreature()->castSpell(getCreature(), Xevozz::EtherealSphereSummonSpells[spell]);
            } break;
            default:
                break;
        }
    }
}

void XevozzAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_XEVOZZ, Performed);
}

void XevozzAI::justReachedSpawn()
{
    // Reset Cell Door
    if (mInstance)
        mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_XEVOZZ);

    // Reset Summon Spells
    summonSpells = { 0, 1, 2 };
}

void XevozzAI::onSummonedCreature(Creature* summon)
{
    summon->getMovementManager()->moveFollow(getCreature()->ToUnit(), 0.0f, 0.0f);
}

void XevozzAI::OnSpellHitTarget(Object* /*target*/, SpellInfo const* info)
{
    if (info->getId() == Xevozz::SPELL_ARCANE_POWER || info->getId() == Xevozz::H_SPELL_ARCANE_POWER)
        sendDBChatMessage(Xevozz::SAY_SUMMON_ENERGY);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Ethereal Sphere AI
EtherealSphereAI::EtherealSphereAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
}

CreatureAIScript* EtherealSphereAI::Create(Creature* pCreature) { return new EtherealSphereAI(pCreature); }

void EtherealSphereAI::OnLoad()
{
    scriptEvents.addEvent(1, 8 * TimeVarsMs::Second);

    getCreature()->castSpell(getCreature(), Xevozz::SPELL_POWER_BALL_VISUAL);
    getCreature()->castSpell(getCreature(), Xevozz::SPELL_POWER_BALL_DAMAGE_TRIGGER);

    despawn(40 * TimeVarsMs::Second, 0);
}

void EtherealSphereAI::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                if (Creature* xevozz = mInstance->getCreatureFromData(DATA_XEVOZZ))
                {
                    if (getCreature()->IsWithinDistInMap(xevozz, 3.0f))
                    {
                        getCreature()->castSpell(nullptr, Xevozz::SPELL_ARCANE_POWER);
                        despawn(8 * TimeVarsMs::Second, 0);
                        return;
                    }
                }

                scriptEvents.addEvent(1, 8 * TimeVarsMs::Second);
            } break;
            default:
                break;
        }
    }
}
