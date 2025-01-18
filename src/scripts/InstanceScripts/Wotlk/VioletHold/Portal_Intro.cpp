/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Portal_Intro.hpp"
#include "Instance_TheVioletHold.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Intro Portal AI
IntroPortalAI::IntroPortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
    portalLocation = 0;
}

CreatureAIScript* IntroPortalAI::Create(Creature* pCreature) { return new IntroPortalAI(pCreature); }

void IntroPortalAI::OnLoad()
{
    // Delete Portals Which are no Summons
    if (!getCreature()->isSummon())
        getCreature()->Despawn(10, 0);

    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
}

void IntroPortalAI::AIUpdate(unsigned long time_passed)
{
    if (!mInstance || mInstance->getLocalData(DATA_MAIN_EVENT_STATE) != EncounterStates::NotStarted)
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        despawn();
        return;
    }

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                // Limit the number of current summons
                if (summons.size() < 3)
                {
                    uint32_t entrys[] = { NPC_AZURE_INVADER_1, NPC_AZURE_MAGE_SLAYER_1, NPC_AZURE_BINDER_1 };                    
                    summonCreature(entrys[Util::getRandomInt(0, 2)], getCreature()->GetPosition(), CORPSE_TIMED_DESPAWN, 20 * TimeVarsMs::Second);
                }

                scriptEvents.addEvent(1, 15 * TimeVarsMs::Second);
            } break;
        default:
            break;
        }
    }
}

void IntroPortalAI::SetCreatureData(uint32_t type, uint32_t data)
{
    if (type == DATA_PORTAL_LOCATION)
        portalLocation = uint8_t(data);
}

void IntroPortalAI::OnSummon(Unit* /*summoner*/)
{
    scriptEvents.addEvent(1, 15 * TimeVarsMs::Second);
}

void IntroPortalAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);

    // Tell Our Summon From Where he has To Run
    if (summon->GetScript())
        summon->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, portalLocation);
}

void IntroPortalAI::OnSummonDespawn(Creature* summon)
{
    summons.despawn(summon);
}
