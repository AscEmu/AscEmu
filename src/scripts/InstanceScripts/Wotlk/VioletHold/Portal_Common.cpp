/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheVioletHold.hpp"
#include "Portal_Common.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Common Portal AI
CommonPortalAI::CommonPortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
    portalLocation = 0;
}

CreatureAIScript* CommonPortalAI::Create(Creature* pCreature) { return new CommonPortalAI(pCreature); }

void CommonPortalAI::OnLoad()
{
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->castSpell(getCreature(), SPELL_PORTAL_PERIODIC, true);
    getCreature()->setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
}

void CommonPortalAI::AIUpdate(unsigned long time_passed)
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
                if (Util::getRandomInt(0, 1))
                {
                    uint32_t entrys[] = { NPC_PORTAL_GUARDIAN, NPC_PORTAL_KEEPER };
                    uint32_t entry = entrys[Util::getRandomInt(0, 1)];
                    if (Creature* portalKeeper = summonCreature(entry, getCreature()->GetPosition(), CORPSE_TIMED_DESPAWN, 20 * TimeVarsMs::Second))
                        getCreature()->castSpell(portalKeeper, SPELL_PORTAL_CHANNEL, false);

                    if (Creature* sinclariTrigger = mInstance->getCreatureFromData(DATA_SINCLARI_TRIGGER))
                    {
                        if (entry == NPC_PORTAL_GUARDIAN)
                            sinclariTrigger->GetScript()->sendDBChatMessage(SAY_SINCLARI_PORTAL_GUARDIAN);
                        else if (entry == NPC_PORTAL_KEEPER)
                            sinclariTrigger->GetScript()->sendDBChatMessage(SAY_SINCLARI_PORTAL_KEEPER);
                    }
                }
                else
                {
                    uint8_t k = mInstance->getLocalData(DATA_WAVE_COUNT) < 12 ? 3 : 4;
                    while (k--)
                    {
                        uint32_t entrys[] = { NPC_AZURE_INVADER_1, NPC_AZURE_INVADER_2, NPC_AZURE_SPELLBREAKER_1, NPC_AZURE_SPELLBREAKER_2, NPC_AZURE_MAGE_SLAYER_1, NPC_AZURE_MAGE_SLAYER_2, NPC_AZURE_BINDER_1, NPC_AZURE_BINDER_2 };
                        uint32_t entry = entrys[Util::getRandomInt(0, 6)];
                        summonCreature(entry, getCreature()->GetPosition(), CORPSE_TIMED_DESPAWN, 20 * TimeVarsMs::Second);
                    }
                }

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

void CommonPortalAI::SetCreatureData(uint32_t type, uint32_t data)
{
    if (type == DATA_PORTAL_LOCATION)
        portalLocation = uint8_t(data);
}

void CommonPortalAI::OnSummon(Unit* /*summoner*/)
{
    scriptEvents.addEvent(1, 15 * TimeVarsMs::Second);
}

void CommonPortalAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);

    // Tell Our Summon From Where he has To Run
    if (summon->GetScript())
        summon->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, portalLocation);
}

void CommonPortalAI::OnSummonDies(Creature* summon, Unit* /*killer*/)
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
