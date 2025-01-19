/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Portal_Elite.hpp"
#include "Instance_TheVioletHold.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Elite Portal AI
ElitePortalAI::ElitePortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
    portalLocation = 0;
}

CreatureAIScript* ElitePortalAI::Create(Creature* pCreature) { return new ElitePortalAI(pCreature); }

void ElitePortalAI::OnLoad()
{
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
}

void ElitePortalAI::AIUpdate(unsigned long time_passed)
{
    if (!mInstance || mInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::NotStarted)
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
                uint8_t k = mInstance->getLocalData(DATA_WAVE_COUNT) < 12 ? 3 : 4;
                while (k--)
                {
                    uint32_t entrys[] = { NPC_AZURE_CAPTAIN_1, NPC_AZURE_RAIDER_1, NPC_AZURE_STALKER_1, NPC_AZURE_SORCEROR_1 };
                    summonCreature(entrys[Util::getRandomInt(0, 3)], getCreature()->GetPosition(), CORPSE_TIMED_DESPAWN, 20 * TimeVarsMs::Second);
                }

                if (Creature* sinclariTrigger = mInstance->getCreatureFromData(DATA_SINCLARI_TRIGGER))
                    if (sinclariTrigger->GetScript())
                        sinclariTrigger->GetScript()->sendDBChatMessage(SAY_SINCLARI_ELITE_SQUAD);

                scriptEvents.addEvent(2, 1 * TimeVarsMs::Second);
            } break;
            case 2:
            {
                getCreature()->setVisible(false);
            } break;
        default:
            break;
        }
    }
}

void ElitePortalAI::SetCreatureData(uint32_t type, uint32_t data)
{
    if (type == DATA_PORTAL_LOCATION)
        portalLocation = uint8_t(data);
}

void ElitePortalAI::OnSummon(Unit* /*summoner*/)
{
    scriptEvents.addEvent(1, 15 * TimeVarsMs::Second);
}

void ElitePortalAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);

    // Tell Our Summon From Where he has To Run
    if (summon->GetScript())
        summon->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, portalLocation);
}

void ElitePortalAI::OnSummonDies(Creature* summon, Unit* /*killer*/)
{
    summons.despawn(summon);

    // When all Summons Died Increase Wavecounter
    if (summons.empty())
    {
        if (mInstance)
            mInstance->setLocalData(DATA_WAVE_COUNT, mInstance->getLocalData(DATA_WAVE_COUNT) + 1);
        getCreature()->ToSummon()->unSummon();
    }
}
