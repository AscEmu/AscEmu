/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Saurfang.hpp"
#include "Raid_IceCrownCitadel.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Muradin
void MuradinSeGossip::onHello(Object* pObject, Player* plr)
{
    pInstance = (IceCrownCitadelScript*)plr->getWorldMap()->getScript();

    if (pInstance && pInstance->getBossState(DATA_DEATHBRINGER_SAURFANG) != Performed)
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_MURADIN_START, 1);
        menu.sendGossipPacket(plr);
    }
}

void MuradinSeGossip::onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    switch (Id)
    {
    case 1:
        // Start Intro
        static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_START_EVENT);
        break;
    }
    GossipMenu::senGossipComplete(pPlayer);
}

MuradinSaurfangEvent::MuradinSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = (IceCrownCitadelScript*)getInstanceScript();
    getCreature()->setAItoUse(true);
    getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
}

CreatureAIScript* MuradinSaurfangEvent::Create(Creature* pCreature) { return new MuradinSaurfangEvent(pCreature); }

void MuradinSaurfangEvent::OnCombatStop(Unit* /*_target*/)
{
    Reset();
}

void MuradinSaurfangEvent::Reset()
{
    scriptEvents.resetEvents();
    resetScriptPhase();
    getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    getCreature()->setMoveDisableGravity(false);
    despawn(2000, 2000);

    _guardList.clear();

    GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 20.0f);
    for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
        (*itr)->GetScript()->DoAction(EVENT_WIPE);
}

void MuradinSaurfangEvent::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_INTRO_ALLIANCE_4_SE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_FIRST_STEP, firstStepPos.getPositionX(), firstStepPos.getPositionY(), firstStepPos.getPositionZ());
                break;
            }
            case EVENT_INTRO_ALLIANCE_5_SE:
            {
                sendDBChatMessage(SAY_INTRO_ALLIANCE_5_SE);

                // Charge
                getCreature()->getMovementManager()->moveCharge(chargePos[0], 8.5f, POINT_CHARGE);

                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                    (*itr)->GetScript()->DoAction(ACTION_CHARGE);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_1_SE:
            {
                _removeAura(SPELL_GRIP_OF_AGONY);
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_1_SE);
                getCreature()->setMoveDisableGravity(false);

                GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 20.0f);
                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                {
                    (*itr)->removeAllAurasById(SPELL_GRIP_OF_AGONY);
                    (*itr)->setMoveDisableGravity(false);
                }

                getCreature()->getMovementManager()->movePoint(POINT_LAND, chargePos[0]);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_2_SE:
            {
                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_2_SE);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_3_SE, 6000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_3_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_3_SE);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_4_SE, 6000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_4_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_4_SE);

                getCreature()->setMoveWalk(false);
                getCreature()->getMovementManager()->movePoint(POINT_TRANSPORT, alliTransPos[0], true, 1.45f);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_5_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_5_SE);

                GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 90.0f);
                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                    (*itr)->GetScript()->DoAction(ACTION_DEFEND_TRANSPORT);

                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_6_SE, 21000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_6_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_6_SE);
                
                Creature* outroNpc = mInstance->spawnCreature(NPC_SE_HIGH_OVERLORD_SAURFANG, unboardMightylPos.x, unboardMightylPos.y, unboardMightylPos.z, unboardMightylPos.o);

                if (outroNpc && outroNpc->GetScript())
                {
                    outroNpc->GetScript()->setCanEnterCombat(false);
                    outroNpc->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                    outroNpc->GetScript()->DoAction(ACTION_START_OUTRO);
                }

                break;
            }
            case EVENT_OUTRO_ALLIANCE_7_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_7_SE);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_9_SE, 13000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_9_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_9_SE);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_10_SE, 6000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_10_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_10_SE);

                // Spawn Portal and Jaina with Wryn
                GameObject* Portal = mInstance->spawnGameObject(GO_PORTAL_TO_STORMWIND, portalSpawn.x, portalSpawn.y, portalSpawn.z, portalSpawn.o);
                Portal->setScale(2);
                Portal->despawn(6000, 0);

                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_11_SE, 6000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_11_SE:
            {
                // They should appear with a blue beam
                Creature* varian = spawnCreature(NPC_SE_KING_VARIAN_WRYNN, varianSpawn);
                Creature* jaina = spawnCreature(NPC_SE_JAINA_PROUDMOORE, jainaSpawn);

                if (varian)
                {
                    varian->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_11_SE, 2000);
                    varian->castSpell(nullptr, SPELL_TELEPORT_VISUAL_GB);    // maybe not the correct spell
                }
                
                if (jaina)
                    jaina->castSpell(nullptr, SPELL_TELEPORT_VISUAL_GB);     // maybe not the correct spell

                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_12_SE, 6000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_12_SE:
            {
                // move guards and muradin out of the way
                getCreature()->getMovementManager()->movePoint(POINT_AWAY, alliAwayPos[0], true, alliAwayPos[0].o);
                GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 90.0f);
                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                    (*itr)->GetScript()->DoAction(ACTION_MOVE_AWAY);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_15_SE:
            {
                Creature* varian = findNearestCreature(NPC_SE_KING_VARIAN_WRYNN, 100.0f);
                if (varian)
                {
                    varian->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_18_SE, 3000);
                    varian->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_20_SE, 13000);
                }

                Creature* jaina = findNearestCreature(NPC_SE_JAINA_PROUDMOORE, 100.0f);
                if (jaina)
                {
                    jaina->emote(EMOTE_ONESHOT_CRY_JAINA);
                    jaina->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_17_SE, 2000);
                    jaina->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_19_SE, 6000);
                }
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_21_SE, 22000);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_21_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_21_SE);

                getCreature()->getMovementManager()->movePoint(POINT_FINAL, finalPos);

                GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 90.0f);
                for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                    (*itr)->GetScript()->DoAction(ACTION_DESPAWN);


                Creature* varian = findNearestCreature(NPC_SE_KING_VARIAN_WRYNN, 100.0f);
                if (varian)
                {
                    varian->castSpell(nullptr, SPELL_TELEPORT_VISUAL_GB);     // maybe not the correct spell
                    varian->Despawn(1000, 0);
                }

                Creature* jaina = findNearestCreature(NPC_SE_JAINA_PROUDMOORE, 100.0f);
                if (jaina)
                {
                    jaina->castSpell(nullptr, SPELL_TELEPORT_VISUAL_GB);     // maybe not the correct spell
                    jaina->Despawn(1000, 0);
                }

                break;
            }
            default:
                break;
        }
    }
}

void MuradinSaurfangEvent::OnReachWP(uint32_t type, uint32_t iWaypointId)
{
    if (type == POINT_MOTION_TYPE)
    {
        switch (iWaypointId)
        {
            case POINT_FIRST_STEP:
            {
                sendDBChatMessage(SAY_INTRO_ALLIANCE_4_SE);

                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_5_SE, 5000, PHASE_INTRO_A);

                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                    deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
                break;
            }
            case POINT_LAND:
            {
                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    float x, y, z;
                    deathbringer->getClosePoint(x, y, z, deathbringer->getCombatReach());
                    getCreature()->setMoveWalk(true);
                    getCreature()->getMovementManager()->movePoint(POINT_CORPSE, x, y, z);
                }
                break;
            }
            case POINT_CORPSE:
            {
                getCreature()->emote(EMOTE_ONESHOT_KNEEL);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_2_SE, 2000);
                break;
            }
            case POINT_TRANSPORT:
            {
                getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_5_SE, 1000);
                break;
            }
            case POINT_FINAL:
            {
                mInstance->DoAction(ACTION_SPAWN_GOS);
                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                getCreature()->Despawn(1000, 1000);
                break;
            }
            default:
                break;
        }
    }
}

void MuradinSaurfangEvent::OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/)
{
    if (_spellId == SPELL_GRIP_OF_AGONY)
    {
        getCreature()->setMoveDisableGravity(true);
        getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[0]);
    }
}

void MuradinSaurfangEvent::DoAction(int32_t const action)
{
    switch (action)
    {
        case EVENT_WIPE:
        {
            Reset();
            break;
        }
        case ACTION_START_EVENT:
        {            
            // Prevent crashes
            if (getScriptPhase() == PHASE_INTRO_A)
                return;

            // Guards
            uint32_t x = 1;
            GetCreatureListWithEntryInGrid(_guardList, NPC_SE_SKYBREAKER_MARINE, 20.0f);
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                (*itr)->GetScript()->SetCreatureData(0, x);
            //

            setScriptPhase(PHASE_INTRO_A);
            sendDBChatMessage(SAY_INTRO_ALLIANCE_1_SE);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_4_SE, 29500, PHASE_INTRO_A);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_A);

            // Clear NPC FLAGS
            getCreature()->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
            break;
        }
        case ACTION_START_OUTRO:
        {
            scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_1_SE, 1000);
            mInstance->DoAction(ACTION_SPAWN_TRANSPORT);
            break;
        }
        case ACTION_CONTINUE_OUTRO:
        {
            scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_7_SE, 1000);
            break;
        }
        case ACTION_CONTINUE_OUTRO2:
        {
            scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_15_SE, 1000);
            break;
        }
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Overlord Saurfang
void OverlordSeGossip::onHello(Object* pObject, Player* plr)
{
    GossipMenu menu(pObject->getGuid(), 14500);
    menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_SAURFANG_START, 1);
    menu.sendGossipPacket(plr);
}

void OverlordSeGossip::onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->getWorldMap()->getScript();
    if (!pInstance)
        return;

    switch (Id)
    {
    case 1:
        // Start Intro
        static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_START_EVENT);
        break;
    }
    GossipMenu::senGossipComplete(pPlayer);
}

OverlordSaurfangEvent::OverlordSaurfangEvent(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = (IceCrownCitadelScript*)getInstanceScript();
    getCreature()->setAItoUse(true);
    getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
}

CreatureAIScript* OverlordSaurfangEvent::Create(Creature* pCreature) { return new OverlordSaurfangEvent(pCreature); }

void OverlordSaurfangEvent::OnCombatStop(Unit* /*_target*/)
{
    Reset();
}

void OverlordSaurfangEvent::Reset()
{
    scriptEvents.resetEvents();
    resetScriptPhase();
    getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    getCreature()->setMoveDisableGravity(false);
    despawn(2000, 2000);

    _guardList.clear();

    GetCreatureListWithEntryInGrid(_guardList, NPC_SE_KOR_KRON_REAVER, 20.0f);
    for (auto itr = _guardList.begin(); itr != _guardList.end(); ++itr)
        (*itr)->GetScript()->DoAction(EVENT_WIPE);
}

void OverlordSaurfangEvent::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_INTRO_HORDE_3_SE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_FIRST_STEP, firstStepPos.getPositionX(), firstStepPos.getPositionY(), firstStepPos.getPositionZ());
                break;
            }
            case EVENT_INTRO_HORDE_5_SE:
            {
                sendDBChatMessage(SAY_INTRO_HORDE_5_SE);
                break;
            }
            case EVENT_INTRO_HORDE_6_SE:
            {
                sendDBChatMessage(SAY_INTRO_HORDE_6_SE);
                break;
            }
            case EVENT_INTRO_HORDE_7_SE:
            {
                sendDBChatMessage(SAY_INTRO_HORDE_7_SE);
                break;
            }
            case EVENT_INTRO_HORDE_8_SE:
            {
                sendDBChatMessage(SAY_INTRO_HORDE_8_SE);

                // Charge
                getCreature()->getMovementManager()->moveCharge(chargePos[0], 8.5f, POINT_CHARGE);
                break;
            }
            case EVENT_OUTRO_HORDE_2_SE:   // say
            {
                sendDBChatMessage(SAY_OUTRO_HORDE_2_SE);
                break;
            }
            case EVENT_OUTRO_HORDE_3_SE:   // say
            {
                sendDBChatMessage(SAY_OUTRO_HORDE_3_SE);
                break;
            }
            case EVENT_OUTRO_HORDE_4_SE:   // move
            {
                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    float x, y, z;
                    deathbringer->getClosePoint(x, y, z, deathbringer->getCombatReach());
                    getCreature()->setMoveWalk(true);
                    getCreature()->getMovementManager()->movePoint(POINT_CORPSE, x, y, z);
                }
                break;
            }
            case EVENT_OUTRO_HORDE_5_SE:   // move
            {
                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    deathbringer->castSpell(getCreature(), SPELL_RIDE_VEHICLE, true);
                    deathbringer->setEmoteState(EMOTE_STATE_DROWNED);
                }

                getCreature()->getMovementManager()->movePoint(POINT_FINAL, finalPos);
                break;
            }
            case EVENT_OUTRO_HORDE_6_SE:   // say
            {
                sendDBChatMessage(SAY_OUTRO_HORDE_4_SE);
                break;
            }
            case EVENT_OUTRO_HORDE_7_SE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_EXIT, finalPos);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_7_SE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_TRANSPORT, faceMuradinPos, true, faceMuradinPos.o);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_8_SE:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_8_SE);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_9_SE:
            {
                // Move to our sons corpse
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_12_SE);
            
                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    float x, y, z;
                    deathbringer->getClosePoint(x, y, z, deathbringer->getCombatReach());
                    getCreature()->setMoveWalk(true);
                    getCreature()->getMovementManager()->movePoint(POINT_CORPSE, x, y, z);
                }
                break;
            }
            case EVENT_OUTRO_ALLIANCE_14_SE:
            {
                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    deathbringer->castSpell(getCreature(), SPELL_RIDE_VEHICLE, true);
                    deathbringer->setEmoteState(EMOTE_STATE_DROWNED);
                }

                sendDBChatMessage(SAY_OUTRO_ALLIANCE_14_SE);
                getCreature()->getMovementManager()->movePoint(POINT_VARIAN, faceVarianPos, true, faceVarianPos.o);
                break;
            }
            case EVENT_OUTRO_ALLIANCE_15_SE:
            {
                mInstance->DoAction(ACTION_TRANSPORT_FLY);
                getCreature()->getMovementManager()->movePoint(POINT_FINAL, unboardMightylPos);
                break;
            }
            default:
                break;
        }
    }
}

void OverlordSaurfangEvent::OnReachWP(uint32_t type, uint32_t iWaypointId)
{
    if (type == POINT_MOTION_TYPE)
    {
        switch (iWaypointId)
        {
            case POINT_FIRST_STEP:
            {
                sendDBChatMessage(SAY_INTRO_HORDE_3_SE);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_5_SE, 15500, PHASE_INTRO_H);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_6_SE, 29500, PHASE_INTRO_H);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_7_SE, 43800, PHASE_INTRO_H);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_8_SE, 47000, PHASE_INTRO_H);

                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                    deathbringer->GetScript()->DoAction(ACTION_CONTINUE_INTRO);
                break;
            }
            case POINT_CORPSE:
            {
                if (mInstance->getInstance()->getTeamIdInInstance() == TEAM_HORDE)
                {
                    sendDBChatMessage(SAY_OUTRO_HORDE_3_SE);
                    scriptEvents.addEvent(EVENT_OUTRO_HORDE_5_SE, 2000);    // move
                }
                else
                {
                    sendDBChatMessage(SAY_OUTRO_ALLIANCE_13_SE);
                    scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_14_SE, 2000);
                }
                break;
            }
            case POINT_VARIAN:
            {
                sendDBChatMessage(SAY_OUTRO_ALLIANCE_15_SE);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_15_SE, 30000);

                Creature* varian = findNearestCreature(NPC_SE_KING_VARIAN_WRYNN, 30.0f);
                if (varian)
                    varian->SendTimedScriptTextChatMessage(SAY_OUTRO_ALLIANCE_16_SE, 7000);
                break;
            }
            case POINT_TRANSPORT:
            {
                Creature* Commander = mInstance->getInstance()->getInterface()->findNearestCreature(getCreature(), NPC_SE_MURADIN_BRONZEBEARD, 200.0f);
                if (Commander)
                    Commander->GetScript()->DoAction(ACTION_CONTINUE_OUTRO);

                getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);

                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_8_SE, 6000);
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_9_SE, 34000);
                break;
            }
            case POINT_FINAL:
            {
                if (mInstance->getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
                {
                    Creature* Commander = mInstance->getInstance()->getInterface()->findNearestCreature(getCreature(), NPC_SE_MURADIN_BRONZEBEARD, 200.0f);
                    if (Commander)
                        Commander->GetScript()->DoAction(ACTION_CONTINUE_OUTRO2);

                    if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                        deathbringer->Despawn(1000, 0);
                    getCreature()->Despawn(1000, 0);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_OUTRO_HORDE_6_SE, 4000);    // say
                    scriptEvents.addEvent(EVENT_OUTRO_HORDE_7_SE, 6000);    // move
                }                    
                break;
            }
            case POINT_EXIT:
            {
                mInstance->DoAction(ACTION_SPAWN_GOS);
                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

                if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                    deathbringer->Despawn(1000, 0);
                getCreature()->Despawn(1000, 0);
                break;
            }
            default:
                break;
        }
    }
}

void OverlordSaurfangEvent::OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/)
{
    if (_spellId == SPELL_GRIP_OF_AGONY)
    {
        getCreature()->setMoveDisableGravity(true);
        getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[0]);
    }
}

void OverlordSaurfangEvent::DoAction(int32_t const action)
{
    switch (action)
    {
        case EVENT_WIPE:
        {
            Reset();
            break;
        }
        case ACTION_START_EVENT:
        {
            // Prevent crashes
            if (getScriptPhase() == PHASE_INTRO_H)
                return;

            // Guards
            uint32_t x = 1;
            GetCreatureListWithEntryInGrid(_guardList, NPC_SE_KOR_KRON_REAVER, 20.0f);
            for (auto itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                (*itr)->GetScript()->SetCreatureData(0, x);
            //

            sendDBChatMessage(SAY_INTRO_HORDE_1_SE);
            setScriptPhase(PHASE_INTRO_H);

            // Start Intro
            scriptEvents.addEvent(EVENT_INTRO_HORDE_3_SE, 18500, PHASE_INTRO_H);

            // Open Suarfangs Door
            if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
                Door->setState(GO_STATE_OPEN);

            // Start Intro on Suarfang        
            if (Creature* deathbringer = mInstance->getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                deathbringer->GetScript()->DoAction(PHASE_INTRO_H);

            // Clear NPC FLAGS
            getCreature()->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);

            break;
        }
        case ACTION_START_OUTRO:
        {
            if (mInstance->getInstance()->getTeamIdInInstance() == TEAM_HORDE)
            {
                // Horde Outro
                _removeAura(SPELL_GRIP_OF_AGONY);
                getCreature()->setMoveDisableGravity(false);
                getCreature()->getMovementManager()->movePoint(POINT_LAND, chargePos[0]);

                sendDBChatMessage(SAY_OUTRO_HORDE_1_SE);
                scriptEvents.addEvent(EVENT_OUTRO_HORDE_2_SE, 10000);   // say
                scriptEvents.addEvent(EVENT_OUTRO_HORDE_3_SE, 18000);   // say
                scriptEvents.addEvent(EVENT_OUTRO_HORDE_4_SE, 24000);   // cast
            }
            else
            {
                // Alliance Outro
                scriptEvents.addEvent(EVENT_OUTRO_ALLIANCE_7_SE, 1000);
            }
            break;
        }
        default:
            break;
    }
}

static void updateBloodPowerAura(Aura* aur, int32_t value)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        auto aurEff = aur->getModifiableAuraEffect(i);
        if (aurEff->getAuraEffectType() == SPELL_AURA_NONE)
            continue;

        aurEff->setEffectBaseDamage(value);
    }

    aur->refreshOrModifyStack();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Deathbringer Saurfang
DeathbringerSaurfangAI::DeathbringerSaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = (IceCrownCitadelScript*)getInstanceScript();
    getCreature()->setAItoUse(true);
    getCreature()->setPower(POWER_TYPE_ENERGY, 0);

    // disable Power regen
    getCreature()->addNpcFlags(UNIT_NPC_FLAG_DISABLE_PWREGEN);

    _introDone = false;
    _frenzied = false;
    _dead = false;
    FightWonValue = 50000;
    summons.clear();

    // Spells Auto Casted
    BerserkSpell = addAISpell(SPELL_BERSERK, 66.0f, TARGET_SELF, 0, 360);
    BerserkSpell->addDBEmote(SAY_DEATHBRINGER_BERSERK);
    BerserkSpell->setAvailableForScriptPhase({ PHASE_COMBAT });

    if (isHeroic())
    {
        BerserkSpell->mCooldown = 480000;
        BerserkSpell->setCooldownTimer(480000);
    }

    BoilingBloodSpell = addAISpell(SPELL_BOILING_BLOOD, 66.0f, TARGET_SELF, 0, 15);
    BoilingBloodSpell->setAvailableForScriptPhase({ PHASE_COMBAT });
    BloodNovaSpell = addAISpell(SPELL_BLOOD_NOVA_TRIGGER, 66.0f, TARGET_SELF, 0, 17);
    BloodNovaSpell->setAvailableForScriptPhase({ PHASE_COMBAT });
    RuneOfBloodSpell = addAISpell(SPELL_RUNE_OF_BLOOD, 66.0f, TARGET_ATTACKING, 0, 20);
    RuneOfBloodSpell->setAvailableForScriptPhase({ PHASE_COMBAT });
    FrenzySpell = addAISpell(SPELL_FRENZY, 100.0f, TARGET_SELF);
    FrenzySpell->mIsTriggered = true;
    FrenzySpell->setMinMaxPercentHp(0.0f, 30.0f);
    FrenzySpell->addDBEmote(SAY_DEATHBRINGER_FRENZY);
    FrenzySpell->setAvailableForScriptPhase({ PHASE_COMBAT });

    // Scripted Spells not autocastet
    GripOfAgonySpell = addAISpell(SPELL_GRIP_OF_AGONY, 0.0f, TARGET_SELF);
    SummonBloodBeast = addAISpell(SPELL_SUMMON_BLOOD_BEAST, 0.0f, TARGET_SELF);
    SummonBloodBeast25 = addAISpell(SPELL_SUMMON_BLOOD_BEAST_25_MAN, 0.0f, TARGET_SELF);
    ScentOfBloodSpell = addAISpell(SPELL_SCENT_OF_BLOOD, 0.0f, TARGET_SOURCE);
    ScentOfBloodSpell->addDBEmote(EMOTE_DEATHBRINGER_SCENT_OF_BLOOD);
    ZeroPowerSpell = addAISpell(SPELL_ZERO_POWER, 0.0f, TARGET_SELF, 0, 0, false, true);
    BloodLinkSpell = addAISpell(SPELL_BLOOD_LINK, 0.0f, TARGET_SELF, 0, 0, false, true);
    BloodPowerSpell = addAISpell(SPELL_BLOOD_POWER, 0.0f, TARGET_SELF, 0, 0, false, true);
    MarkOfTheFallenSpell_Self = addAISpell(SPELL_MARK_OF_THE_FALLEN_CHAMPION_S, 0.0f, TARGET_SELF, 0, 0, false, true);
    MarkOfTheFallenSpell = addAISpell(SPELL_MARK_OF_THE_FALLEN_CHAMPION, 0.0f, TARGET_CUSTOM, 0, 0);
    MarkOfTheFallenSpell->addDBEmote(SAY_DEATHBRINGER_MARK);
    RuneOfBloodSSpell = addAISpell(SPELL_RUNE_OF_BLOOD_S, 0.0f, TARGET_SELF, 0, 0, false, true);
    RemoveMarksSpell = addAISpell(SPELL_REMOVE_MARKS_OF_THE_FALLEN_CHAMPION, 0.0f, TARGET_SELF);
    AchievementSpell = addAISpell(SPELL_ACHIEVEMENT_SE, 0.0f, TARGET_SELF);
    AchievementSpell->mIsTriggered = true;
    ReputationBossSpell = addAISpell(SPELL_AWARD_REPUTATION_BOSS_KILL, 0.0f, TARGET_SELF);
    ReputationBossSpell->mIsTriggered = true;
    PermanentFeignSpell = addAISpell(SPELL_PERMANENT_FEIGN_DEATH, 0.0f, TARGET_SELF);

    addEmoteForEvent(Event_OnCombatStart, SAY_DEATHBRINGER_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, SAY_DEATHBRINGER_KILL);
    addEmoteForEvent(Event_OnDied, SAY_DEATHBRINGER_DEATH);
}


CreatureAIScript* DeathbringerSaurfangAI::Create(Creature* pCreature) { return new DeathbringerSaurfangAI(pCreature); }

void DeathbringerSaurfangAI::clearMarksFromTargets()
{
    // Spell SPELL_REMOVE_MARKS_OF_THE_FALLEN_CHAMPION removes marks from alive units
    // If player resurrects spirit and spawns at graveyard, mark is also removed because marks are bound to ICC map
    _castAISpell(RemoveMarksSpell);

    // However if a marked player dies and he's resurrected by his friend after killing boss, he would still have a mark
    // so make sure all marked units lose the aura
    auto itr = _markedTargetGuids.begin();
    while (itr != _markedTargetGuids.end())
    {
        auto* const markedUnit = getCreature()->getWorldMapUnit(*itr);
        if (markedUnit != nullptr && markedUnit->IsInWorld())
            markedUnit->removeAllAurasById(SPELL_MARK_OF_THE_FALLEN_CHAMPION);

        itr = _markedTargetGuids.erase(itr);
    }
}

void DeathbringerSaurfangAI::OnCombatStop(Unit* /*_target*/)
{
    Reset();
}

void DeathbringerSaurfangAI::Reset()
{
    if (_dead)
        return;

    summons.despawnAll();
    scriptEvents.resetEvents();
    resetScriptPhase();
    _introDone = false;
    _frenzied = false;
    _dead = false;

    setCanEnterCombat(false);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);

    getCreature()->setPower(POWER_TYPE_ENERGY, 0);
    _castAISpell(ZeroPowerSpell);
    _castAISpell(BloodLinkSpell);
    _castAISpell(BloodPowerSpell);
    _castAISpell(MarkOfTheFallenSpell_Self);
    _castAISpell(RuneOfBloodSSpell);
    _removeAura(SPELL_BERSERK);
    _removeAura(SPELL_FRENZY);

    clearMarksFromTargets();

    Creature* Commander = mInstance->getInstance()->getInterface()->findNearestCreature(getCreature(), mInstance->getInstance()->getTeamIdInInstance() ? NPC_SE_HIGH_OVERLORD_SAURFANG : NPC_SE_MURADIN_BRONZEBEARD, 90.0f);
    if (Commander)
        Commander->GetScript()->DoAction(EVENT_WIPE);
}

void DeathbringerSaurfangAI::AIUpdate(unsigned long time_passed)
{
    if (_isCasting())
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_INTRO_ALLIANCE_2_SE:
            {
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->setFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_2);
                break;
            }
            case EVENT_INTRO_ALLIANCE_3_SE:
            {
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_3);
                break;
            }
            case EVENT_INTRO_ALLIANCE_6_SE:
            {
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_6);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_ALLIANCE_7);
                _castAISpell(GripOfAgonySpell);
                setCanEnterCombat(true);
                getCreature()->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                break;
            }
            case EVENT_INTRO_HORDE_2_SE:
            {
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->setFaction(974);
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_2);
                break;
            }
            case EVENT_INTRO_HORDE_4_SE:
            {
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_4);
                break;
            }
            case EVENT_INTRO_HORDE_9_SE:
            {
                sendDBChatMessage(SAY_DEATHBRINGER_INTRO_HORDE_9);
                _castAISpell(GripOfAgonySpell);
                setCanEnterCombat(true);
                getCreature()->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                break;
            }
            case EVENT_INTRO_FINISH_SE:
            {
                setScriptPhase(PHASE_COMBAT);
                break;
            }
            case EVENT_SUMMON_BLOOD_BEAST_SE:
            {
                for (uint32_t i10 = 0; i10 < 2; ++i10)
                    _castAISpell(SummonBloodBeast);

                if (mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || mInstance->GetDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
                    for (uint32_t i25 = 0; i25 < 3; ++i25)
                        _castAISpell(SummonBloodBeast25);

                sendDBChatMessage(SAY_DEATHBRINGER_BLOOD_BEASTS);

                scriptEvents.addEvent(EVENT_SUMMON_BLOOD_BEAST_SE, 40000,PHASE_COMBAT);

                if (isHeroic())
                    scriptEvents.addEvent(EVENT_SCENT_OF_BLOOD_SE, 10000, PHASE_COMBAT);
                break;
            }
            case EVENT_SCENT_OF_BLOOD_SE:
            {
                if (!summons.empty())
                {
                    _castAISpell(ScentOfBloodSpell);
                }
                break;
            }
            default:
                break;
        }
    }
}

void DeathbringerSaurfangAI::OnScriptPhaseChange(uint32_t _phaseId)
{
    switch (_phaseId)
    {
        case PHASE_COMBAT:
        {
            _introDone = true;

            _castAISpell(ZeroPowerSpell);
            _castAISpell(BloodLinkSpell);
            _castAISpell(BloodPowerSpell);
            _castAISpell(MarkOfTheFallenSpell_Self);
            _castAISpell(RuneOfBloodSSpell);

            scriptEvents.addEvent(EVENT_SUMMON_BLOOD_BEAST_SE, 30000, PHASE_COMBAT);
            break;
        }
        default:
            break;
    }
}

void DeathbringerSaurfangAI::DamageTaken(Unit* _attacker, uint32_t* damage)
{
    if (*damage >= getCreature()->getHealth())
        *damage = getCreature()->getHealth() - 1;

    if (!_dead && getCreature()->getHealth() - *damage < FightWonValue)
    {
        _dead = true;
        getCreature()->addUnitStateFlag(UNIT_STATE_EVADING);

        getCreature()->getAIInterface()->eventUnitDied(_attacker, 0);
        getCreature()->getAIInterface()->combatStop();

        _castAISpell(AchievementSpell);
        _castAISpell(ReputationBossSpell);
        _castAISpell(PermanentFeignSpell);

        clearMarksFromTargets();

        // Prepare for Outro
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);

        Creature* Commander = mInstance->getInstance()->getInterface()->findNearestCreature(getCreature(), mInstance->getInstance()->getTeamIdInInstance() ? NPC_SE_HIGH_OVERLORD_SAURFANG : NPC_SE_MURADIN_BRONZEBEARD, 250.0f);
        if (Commander)
            Commander->GetScript()->DoAction(ACTION_START_OUTRO);
    }
}

void DeathbringerSaurfangAI::OnReachWP(uint32_t type, uint32_t iWaypointId)
{
    if (type != POINT_MOTION_TYPE && iWaypointId != POINT_SAURFANG)
        return;

    // Close Suarfangs Door
    if (GameObject* Door = mInstance->GetGameObjectByGuid(mInstance->getLocalData(DATA_SAURFANG_DOOR)))
        Door->setState(GO_STATE_CLOSED);
}

void DeathbringerSaurfangAI::DoAction(int32_t const action)
{
    switch (action)
    {
        case PHASE_INTRO_A:
        case PHASE_INTRO_H:
        {     
            setScriptPhase(uint32_t(action));

            // Move
            getCreature()->getMovementManager()->movePoint(POINT_SAURFANG, deathbringerPos.getPositionX(), deathbringerPos.getPositionY(), deathbringerPos.getPositionZ());

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_2_SE, 2500, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_3_SE, 20000, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_2_SE, 5000, PHASE_INTRO_H);
            break;
        }
        case ACTION_CONTINUE_INTRO:
        {
            if (_introDone)
                return;

            scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_6_SE, 7000, PHASE_INTRO_A);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_4_SE, 6500, PHASE_INTRO_H);
            scriptEvents.addEvent(EVENT_INTRO_HORDE_9_SE, 48200, PHASE_INTRO_H);

            if (mInstance->getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
                scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 8000, PHASE_INTRO_A);
            else
                scriptEvents.addEvent(EVENT_INTRO_FINISH_SE, 55700, PHASE_INTRO_H);
            break;
        }
        case ACTION_MARK_OF_THE_FALLEN_CHAMPION:
        {
            if (Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent, 0.0f, 0.0f, -SPELL_MARK_OF_THE_FALLEN_CHAMPION))
            {
                _markedTargetGuids.push_back(target->getGuid());
                MarkOfTheFallenSpell->setCustomTarget(target);
                _castAISpell(MarkOfTheFallenSpell);

                getCreature()->setPower(POWER_TYPE_ENERGY, 0);

                // Reset Blood Power aura values
                if (Aura* bloodPower = getCreature()->getAuraWithId(SPELL_BLOOD_POWER))
                    updateBloodPowerAura(bloodPower, 1);
            }
            break;
        }
        default:
            break;
    }
}

void DeathbringerSaurfangAI::onSummonedCreature(Creature* summon)
{
    if (Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent))
    {
        if (target->GetTransport())
        {
            summon->Despawn(100, 0);
            return;
        }

        summon->getAIInterface()->onHostileAction(target);
    }
    summon->castSpell(summon, SPELL_BLOOD_LINK_BEAST, true);
    summon->castSpell(summon, SPELL_RESISTANT_SKIN, true);
    summons.summon(summon);
}

void DeathbringerSaurfangAI::OnSummonDies(Creature* summon, Unit* /*killer*/)
{
    summons.despawn(summon);
}

void DeathbringerSaurfangAI::OnCastSpell(uint32_t _spellId)
{
    switch (_spellId)
    {
        case  SPELL_FRENZY:
        {
            _frenzied = true;
            break;
        }
        default:
            break;
    }
}

void DeathbringerSaurfangAI::OnSpellHitTarget(Object* target, SpellInfo const* info)
{
    switch (info->getId())
    {
        case 72255: // Mark of the Fallen Champion, triggered id
        case 72444:
        case 72445:
        case 72446:
        {
            dynamic_cast<Unit*>(target)->castSpell(getCreature(), SPELL_BLOOD_LINK_DUMMY, true);
            break;
        }
        default:
            break;
    }
}

uint32_t DeathbringerSaurfangAI::GetCreatureData(uint32_t type) const
{
    if (type == DATA_MADE_A_MESS && _dead)
        if (_markedTargetGuids.size() < getRaidModeValue(3, 5, 3, 5))
            return 1;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Saurfang Event Outro Npc
NpcSaurfangEventAI::NpcSaurfangEventAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = (IceCrownCitadelScript*)getInstanceScript();
    _index = 0;
}

CreatureAIScript* NpcSaurfangEventAI::Create(Creature* pCreature) { return new NpcSaurfangEventAI(pCreature); }

void NpcSaurfangEventAI::SetCreatureData(uint32_t type, uint32_t data)
{
    if (!(!type && data && data < 6))
        return;
    _index = data;
}

void NpcSaurfangEventAI::OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/)
{
    if (_spellId == SPELL_GRIP_OF_AGONY)
    {
        getCreature()->setMoveDisableGravity(true);
    }
}

void NpcSaurfangEventAI::OnReachWP(uint32_t type, uint32_t iWaypointId)
{
    if (type == POINT_MOTION_TYPE)
    {
        switch (iWaypointId)
        {
            case POINT_CHARGE:
            {
                getCreature()->getMovementManager()->movePoint(POINT_CHOKE, chokePos[_index]);
                break;
            }
            case POINT_TRANSPORT:
            {
                getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                break;
            }
            case POINT_FINAL:
            {
                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                getCreature()->Despawn(1000, 1000);
                break;
            }
            default:
                break;
        }
    }
}

void NpcSaurfangEventAI::DoAction(int32_t const action)
{
    switch (action)
    {
        case ACTION_CHARGE:
        {
            if (_index)
            {
                getCreature()->getMovementManager()->moveCharge(chargePos[_index], 8.5f, POINT_CHARGE);
            }
            break;
        }
        case ACTION_DEFEND_TRANSPORT:
        {
            getCreature()->getMovementManager()->movePoint(POINT_TRANSPORT, alliTransPos[_index], true, 1.45f);
            break;
        }
        case ACTION_MOVE_AWAY:
        {
            getCreature()->getMovementManager()->movePoint(POINT_AWAY, alliAwayPos[_index], true, alliAwayPos[_index].o);
            break;
        }
        case ACTION_DESPAWN:
        {
            getCreature()->getMovementManager()->movePoint(POINT_FINAL, finalPos);
            break;
        }
        case EVENT_WIPE:
        {
            getCreature()->setMoveDisableGravity(false);
            getCreature()->Despawn(2000, 2000);
            break;
        }
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Grip Of Agony
void GripOfAgony::filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
{
    if (effectIndex != EFF_INDEX_0)
        return;

    // Hackfix shouldnt cast on self
    effectTargets->clear();

    for (const auto& itr : spell->getUnitCaster()->getInRangeObjectsSet())
    {
        float distance = spell->getUnitCaster()->CalcDistance(itr);
        if (itr->isCreature() && itr->getEntry() != CN_DEATHBRINGER_SAURFANG && distance <= 100.0f)
        {
            effectTargets->push_back(itr->getGuid());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Blood Link Trigger
// Blood Nova and Rune of Blood should cast Blood Link dummy on Saurfang
SpellScriptCheckDummy GenericBloodLinkTrigger::onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex)
{
    if (effIndex != EFF_INDEX_1)
        return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

    auto* const saurfang = spell->getUnitCaster();
    auto* const unitTarget = spell->getUnitTarget();
    const auto spellId = spell->getSpellInfo()->calculateEffectValue(effIndex);
    if (saurfang != nullptr && unitTarget != nullptr)
        unitTarget->castSpell(saurfang, spellId, true);

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Boiling Blood
void BoilingBlood::filterEffectTargets(Spell* spell, uint8_t /*effectIndex*/, std::vector<uint64_t>* effectTargets)
{
    // Should not be casted on current target
    effectTargets->erase(std::remove(effectTargets->begin(), effectTargets->end(), spell->getUnitCaster()->getTargetGuid()), effectTargets->end());
    if (effectTargets->empty())
        return;

    // Should be casted on 3 random targets
    if (effectTargets->size() > 3)
    {
        Util::randomShuffleVector(*effectTargets);
        effectTargets->erase(effectTargets->begin() + 3, effectTargets->end());
    }
}

SpellScriptExecuteState BoilingBlood::onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/)
{
    // On periodic damage, cast Blood Link on Saurfang
    if (aur->GetUnitCaster() != nullptr)
        aur->getOwner()->castSpell(aur->GetUnitCaster(), sSpellMgr.getSpellInfo(SPELL_BLOOD_LINK_DUMMY), true);

    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Blood Nova
void BloodNova::filterEffectTargets(Spell* spell, uint8_t effIndex, std::vector<uint64_t>* effectTargets)
{
    if (effIndex == EFF_INDEX_0)
        randomTargetGuid = 0;

    if (effectTargets->empty())
        return;

    // Same target for all effects
    if (effIndex != EFF_INDEX_0)
    {
        effectTargets->clear();
        if (randomTargetGuid == 0)
            return;

        effectTargets->push_back(randomTargetGuid);
        return;
    }

    // Find single random player target and prefer a ranged target

    // Get 10 possible targets in 25m and 4 targets in 10m
    const uint8_t minTargetCount = spell->getSpellInfo()->getId() != SPELL_BLOOD_NOVA_TRIGGER ? 10U : 4U;

    std::vector<uint64_t> rangedTargetGuids;
    uint32_t rangedTargetCount = 0;
    for (auto guid : *effectTargets)
    {
        auto* const unitTarget = spell->getCaster()->getWorldMapUnit(guid);
        if (unitTarget == nullptr)
            continue;

        if (spell->getCaster()->CalcDistance(unitTarget) >= 10.0f)
        {
            rangedTargetGuids.push_back(guid);
            ++rangedTargetCount;
        }
    }

    // If there are no enough ranged targets, pick any target
    if (rangedTargetCount < minTargetCount)
    {
        auto itr = std::begin(*effectTargets);
        std::advance(itr, Util::getRandomUInt(0, static_cast<uint32_t>(std::size(*effectTargets)) - 1));
        randomTargetGuid = *itr;
    }
    else
    {
        auto itr = std::begin(rangedTargetGuids);
        std::advance(itr, Util::getRandomUInt(0, static_cast<uint32_t>(std::size(rangedTargetGuids)) - 1));
        randomTargetGuid = *itr;
    }

    effectTargets->clear();
    effectTargets->push_back(randomTargetGuid);
}

SpellScriptCheckDummy BloodNova::onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/)
{
    if (spell->getUnitCaster() != nullptr && spell->getUnitTarget() != nullptr)
        spell->getUnitCaster()->castSpell(spell->getUnitTarget(), SPELL_BLOOD_NOVA_DAMAGE, true);

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Blood Link
SpellScriptCheckDummy BloodLink::onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    // On periodic dummy effect, check if Mark of the Fallen Champion can be casted
    // but if Saurfang is casting spell, try again on next tick
    if (aur->getOwner()->isCastingSpell())
        return SpellScriptCheckDummy::DUMMY_OK;

    if (aur->getOwner()->isCreature() && aur->getOwner()->getPower(POWER_TYPE_ENERGY) == aur->getOwner()->getMaxPower(POWER_TYPE_ENERGY))
        dynamic_cast<Creature*>(aur->getOwner())->GetScript()->DoAction(ACTION_MARK_OF_THE_FALLEN_CHAMPION);

    return SpellScriptCheckDummy::DUMMY_OK;
}

SpellScriptExecuteState BloodLinkDummy::onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/)
{
    auto* const saurfang = spellProc->getProcOwner()->getWorldMapCreature(spellProc->getProcOwner()->getSummonedByGuid());
    if (saurfang == nullptr)
        return SpellScriptExecuteState::EXECUTE_PREVENT;

    spellProc->getProcOwner()->castSpell(saurfang, spellProc->getSpell(), true);
    return SpellScriptExecuteState::EXECUTE_PREVENT;
}

SpellCastResult BloodLinkDummy::onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/)
{
    const auto* const target = spell->getUnitTarget();
    if (target == nullptr)
        return SPELL_FAILED_BAD_TARGETS;

    // Should not be casted if target is at full energy
    if (target->getPower(POWER_TYPE_ENERGY) == target->getMaxPower(POWER_TYPE_ENERGY))
        return SPELL_FAILED_DONT_REPORT;

    return SPELL_CAST_SUCCESS;
}

SpellScriptCheckDummy BloodLinkDummy::onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/)
{
    // On dummy effect, cast 72195 on spell target
    if (spell->getUnitTarget() != nullptr)
        spell->getUnitTarget()->castSpell(spell->getUnitTarget(), SPELL_BLOOD_LINK_POWER, true);

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Blood Link Energize
SpellScriptCheckDummy BloodLinkEnergize::onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/)
{
    // On scripted effect, update Saurfang's Blood Power aura
    auto aur = spell->getUnitCaster()->getAuraWithId(SPELL_BLOOD_POWER);
    if (aur == nullptr)
        return SpellScriptCheckDummy::DUMMY_OK;

    updateBloodPowerAura(aur, spell->getUnitCaster()->getPower(POWER_TYPE_ENERGY));
    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Remove Marks of The Fallen
SpellScriptCheckDummy RemoveMarksOfTheFallen::onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex)
{
    if (spell->getUnitTarget() != nullptr)
    {
        const auto spellId = spell->getSpellInfo()->calculateEffectValue(effIndex);
        spell->getUnitTarget()->removeAllAurasById(spellId);
    }

    return SpellScriptCheckDummy::DUMMY_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Achievement: Ive gone and made a mess
bool achievement_ive_gone_and_made_a_mess::canCompleteCriteria(uint32_t criteriaID, Player* /*pPlayer*/, Object* target)
{
    if (target)
    {
        if (Creature* saurfang = target->ToCreature())
        {
            if (saurfang->GetScript()->GetCreatureData(DATA_MADE_A_MESS))
            {
                switch (saurfang->getWorldMap()->getDifficulty())
                {
                    case InstanceDifficulty::RAID_10MAN_NORMAL:
                    {
                        if (criteriaID == 12778)
                            return true;
                    } break;
                    case InstanceDifficulty::RAID_25MAN_NORMAL:
                    {
                        if (criteriaID == 13036)
                            return true;
                    } break;
                    case InstanceDifficulty::RAID_10MAN_HEROIC:
                    {
                        if (criteriaID == 13035)
                            return true;
                    } break;
                    case InstanceDifficulty::RAID_25MAN_HEROIC:
                    {
                        if (criteriaID == 13037)
                            return true;
                    } break;
                    default:
                        break;
                }
            }
        }
    }

    return false;
}
