/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AzureSaboteur.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "CommonTime.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// AzureSaboteur AI
AzureSaboteurAI::AzureSaboteurAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);

    mbossId = 0;
}

CreatureAIScript* AzureSaboteurAI::Create(Creature* pCreature) { return new AzureSaboteurAI(pCreature); }

void AzureSaboteurAI::OnSummon(Unit* /*summoner*/)
{
    if (mInstance->getLocalData(DATA_WAVE_COUNT) == 6)
        mbossId = mInstance->getLocalData(DATA_1ST_BOSS);
    else
        mbossId = mInstance->getLocalData(DATA_2ND_BOSS);

    scriptEvents.addEvent(1, 2 * TimeVarsMs::Second);
}

void AzureSaboteurAI::AIUpdate(unsigned long time_passed)
{
    if (!mInstance || mInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::NotStarted)
    {
        scriptEvents.resetEvents();
        return;
    }

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                startMovement();
            } break;
            case 2:
            {
                if (castCounter < 2)
                {
                    ++castCounter;
                    getCreature()->castSpell(getCreature(), SPELL_SHIELD_DISRUPTION, false);
                    scriptEvents.addEvent(2, 1 * TimeVarsMs::Second);
                }
                else
                {
                    scriptEvents.addEvent(3, 2 * TimeVarsMs::Second);
                }
            } break;
            case 3:
            {
                mInstance->setLocalData(DATA_START_BOSS_ENCOUNTER, 1);
                getCreature()->castSpell(getCreature(), SPELL_TELEPORT_VISUAL, false);
                scriptEvents.addEvent(4, 1 * TimeVarsMs::Second);
            } break;
            case 4:
            {
                getCreature()->ToSummon()->unSummon();
            } break;
            default:
                break;
        }
    }
}

void AzureSaboteurAI::OnReachWP(uint32_t type, uint32_t pointId)
{
    if (type == EFFECT_MOTION_TYPE && pointId == POINT_INTRO)
    {
        // Start The Summoning Upon Reaching The Cell
        scriptEvents.addEvent(2, 100);
    }
}

void AzureSaboteurAI::startMovement()
{
    switch (mbossId)
    {
        case DATA_MORAGG:
            startSmoothPath(SaboteurMoraggPath);
            break;
        case DATA_EREKEM:
            startSmoothPath(SaboteurErekemPath);
            break;
        case DATA_ICHORON:
            startSmoothPath(SaboteurIchoronPath);
            break;
        case DATA_LAVANTHOR:
            startSmoothPath(SaboteurLavanthorPath);
            break;
        case DATA_XEVOZZ:
            startSmoothPath(SaboteurXevozzPath);
            break;
        case DATA_ZURAMAT:
            startSmoothPath(SaboteurZuramatPath);
            break;
    }
}
