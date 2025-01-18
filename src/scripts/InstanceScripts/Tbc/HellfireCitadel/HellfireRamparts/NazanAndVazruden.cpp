/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "NazanAndVazruden.hpp"
#include "Instance_HellfireRamparts.hpp"
#include "Movement/MovementManager.h"
#include "Spell/SpellInfo.hpp"
#include "CommonTime.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Nazan
NazanAI::NazanAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // All
    m_FireballSpell = addAISpell(SPELL_FIREBALL, 30.0f, TARGET_RANDOM_SINGLE, 0, 4, false, true);
    m_FireballSpell->setAvailableForScriptPhase({ AIR_PHASE, GROUND_PHASE });

    m_ConeOfFireSpell = addAISpell(SPELL_CONE_OF_FIRE, 30.0f, TARGET_SELF, 0, 6);
    m_ConeOfFireSpell->setAvailableForScriptPhase({ GROUND_PHASE });

    // Heroic
    if (isHeroic())
    {
        m_BelowingRoarSpell = addAISpell(SPELL_BELLOWING_ROAR, 8.0f, TARGET_SELF, 0, 12);
        m_BelowingRoarSpell->setAvailableForScriptPhase({ GROUND_PHASE });
    }
}

CreatureAIScript* NazanAI::Create(Creature* pCreature) { return new NazanAI(pCreature); }

void NazanAI::OnSummon(Unit* summoner)
{
    WoWGuid guid = summoner->getGuid();

    if (summoner->getEntry() == NPC_VAZRUDEN_HERALD)
    {
        VazrudenGUID = guid.getGuidLowPart();
        setScriptPhase(AIR_PHASE);
    }
}

void NazanAI::AIUpdate(unsigned long time_passed)
{
    if (getScriptPhase() == AIR_PHASE)
    {
        Creature* Vazruden = getInstanceScript()->GetCreatureByGuid(VazrudenGUID);
        if (Fly_Timer < time_passed || !(Vazruden && Vazruden->isAlive() && Vazruden->getHealthPct() > 20))
        {
            getCreature()->setMoveDisableGravity(false);
            getCreature()->setMoveWalk(true);
            getCreature()->getMovementManager()->clear();

            if (Unit* victim = getBestPlayerTarget(TargetFilter_Closest))
            {
                getCreature()->getAIInterface()->onHostileAction(victim);
                getCreature()->getMovementManager()->moveChase(victim);
            }

            sendDBChatMessage(NAZAN_EMOTE);
            setScriptPhase(GROUND_PHASE);
            return;
        }

        Fly_Timer -= time_passed;
    }
}

void NazanAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == POINT_MOTION_TYPE && id == 0)
    {
        uint32_t waypoint = (Fly_Timer / 10000) % 2;
        getCreature()->getMovementManager()->movePoint(0, VazrudenRing[waypoint][0], VazrudenRing[waypoint][1], VazrudenRing[waypoint][2]);
    }
}

void NazanAI::OnSpellHitTarget(Object* target, SpellInfo const* info)
{
    if (info->getId() == SPELL_FIREBALL)
    {
        if (Creature* fire = summonCreature(NPC_LIQUID_FIRE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TIMED_DESPAWN, 30 * TimeVarsMs::Second))
            fire->m_noRespawn = true;
    }
}

void NazanAI::onSummonedCreature(Creature* summon)
{
    if (summon && summon->getEntry() == NPC_LIQUID_FIRE)
    {
        summon->setFaction(getCreature()->getFactionTemplate());

        if (!isHeroic())
            summon->castSpell(nullptr, SPELL_SUMMON_LIQUID_FIRE);
        else
            summon->castSpell(nullptr, SPELL_SUMMON_LIQUID_FIRE_H);

        summon->castSpell(nullptr, SPELL_FIRE_NOVA_VISUAL);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Vazruden
VazrudenAI::VazrudenAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Normal
    if (!isHeroic())
        m_RenevgeSpell = addAISpell(SPELL_REVENGE, 30.0f, TARGET_SELF, 0, 5);
    else
        m_RenevgeSpell = addAISpell(SPELL_REVENGE_H, 30.0f, TARGET_SELF, 0, 5);

    m_ConeOfFireSpell = 0;

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, VAZRUDEN_AGGRO1);
    addEmoteForEvent(Event_OnCombatStart, VAZRUDEN_AGGRO2);
    addEmoteForEvent(Event_OnCombatStart, VAZRUDEN_AGGRO3);
    addEmoteForEvent(Event_OnTargetDied, VAZRUDEN_KILL1);
    addEmoteForEvent(Event_OnTargetDied, VAZRUDEN_KILL2);
    addEmoteForEvent(Event_OnDied, VAZRUDEN_DIE);
}

CreatureAIScript* VazrudenAI::Create(Creature* pCreature) { return new VazrudenAI(pCreature); }

void VazrudenAI::AIUpdate(unsigned long time_passed)
{
    if (!getCreature()->getAIInterface()->getCurrentTarget())
    {
        if (targetCheck < time_passed && getCreature()->isAlive())
        {
            if (!wipeSaid)
            {
                sendDBChatMessage(VAZRUDEN_WIPE);
                wipeSaid = true;
            }

            getCreature()->Despawn(100, 0);
        }
        else
            targetCheck -= time_passed;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Vazruden The Herald (Mounted form)
VazrudenTheHeraldAI::VazrudenTheHeraldAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addEmoteForEvent(Event_OnCombatStart, VAZRUDEN_INTRO);
}

CreatureAIScript* VazrudenTheHeraldAI::Create(Creature* pCreature) { return new VazrudenTheHeraldAI(pCreature); }


void VazrudenTheHeraldAI::OnCombatStop(Unit* /*pTarget*/)
{
    despawnAdds();
}

void VazrudenTheHeraldAI::AIUpdate(unsigned long /*time_passed*/)
{
    switch (getScriptPhase())
    {
        case CombatStart:
        {
            getCreature()->getMovementManager()->clear();
            getCreature()->getMovementManager()->movePoint(0, VazrudenMiddle[0], VazrudenMiddle[1], VazrudenMiddle[2]);
        } break;
        case SUMMON_PHASE:
        {
            Creature* Nazan = getInstanceScript()->GetCreatureByGuid(nazanGUID);
            Creature* Vazruden = getInstanceScript()->GetCreatureByGuid(vazrudenGUID);
            if ((Nazan && Nazan->isAlive()) || (Vazruden && Vazruden->isAlive()))
            {
                if ((Nazan && Nazan->getAIInterface()->getCurrentTarget()) || (Vazruden && Vazruden->getAIInterface()->getCurrentTarget()))
                    return;

                despawnAdds();
                getCreature()->getAIInterface()->enterEvadeMode();
                return;
            }

            if (!(Nazan && Nazan->isAlive()) && !(Vazruden && Vazruden->isAlive()))
            {
                getCreature()->Despawn(100, 0);
            }
        } break;
    }
}

void VazrudenTheHeraldAI::DoAction(int32_t /*action*/)
{
    if (Unit* player = getBestPlayerTarget(TargetFilter_Closest))
        getCreature()->getAIInterface()->onHostileAction(player);
}

void VazrudenTheHeraldAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == POINT_MOTION_TYPE && id == 0)
    {
        summonAdds();
        setScriptPhase(SUMMON_PHASE);
    }
}

void VazrudenTheHeraldAI::summonAdds()
{
    if (!summoned)
    {
        if (Creature* Vazruden = summonCreature(NPC_VAZRUDEN, VazrudenMiddle[0], VazrudenMiddle[1], VazrudenMiddle[2], 0, CORPSE_TIMED_DESPAWN, 100 * 60 * 1000))
        {
            WoWGuid guid = Vazruden->getGuid();
            vazrudenGUID = guid.getGuidLowPart();
            Vazruden->m_noRespawn = true;

            if (Unit* player = getBestPlayerTarget(TargetFilter_Closest))
                Vazruden->getAIInterface()->onHostileAction(player);
        }

        if (Creature* Nazan = summonCreature(NPC_NAZAN, VazrudenMiddle[0], VazrudenMiddle[1], VazrudenMiddle[2], 0, CORPSE_TIMED_DESPAWN, 100 * 60 * 1000))
        {
            WoWGuid guid = Nazan->getGuid();
            nazanGUID = guid.getGuidLowPart();
            Nazan->m_noRespawn = true;

            if (Unit* player = getBestPlayerTarget(TargetFilter_Closest))
                Nazan->getAIInterface()->onHostileAction(player);
        }

        summoned = true;
        getCreature()->setVisible(false);
        getCreature()->addUnitStateFlag(UNIT_STATE_ROOTED);
    }
}

void VazrudenTheHeraldAI::onSummonedCreature(Creature* summon)
{
    if (!summon)
        return;

    Unit* victim = getCreature()->getAIInterface()->getCurrentTarget();
    if (summon->getEntry() == NPC_NAZAN)
    {
        summon->setMoveDisableGravity(true);
        summon->setSpeedRate(TYPE_FLY, 2.5f, true);

        summon->getAIInterface()->onHostileAction(victim);

        summon->getMovementManager()->moveIdle();
        summon->getMovementManager()->movePoint(0, VazrudenRing[0][0], VazrudenRing[0][1], VazrudenRing[0][2]);
    }
    else
    {
        if (victim)
            summon->getAIInterface()->onHostileAction(victim);
    }
}

void VazrudenTheHeraldAI::despawnAdds()
{
    if (summoned)
    {
        Creature* Nazan = findNearestCreature(NPC_NAZAN, 5000);
        if (Nazan)
        {
            Nazan->Despawn(100, 0);
        }

        Creature* Vazruden = findNearestCreature(NPC_VAZRUDEN, 5000);
        if (Vazruden)
        {
            Vazruden->Despawn(100, 0);
        }

        summoned = false;
        getCreature()->removeUnitStateFlag(UNIT_STATE_ROOTED);
        getCreature()->setVisible(true);
    }
}