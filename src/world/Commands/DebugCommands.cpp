/*
Copyright (c) 2015 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

bool ChatHandler::HandleDebugMoveInfo(const char* /*args*/, WorldSession* m_session)
{
    uint32 guid = Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->GetSelection());
    Creature* creature = getSelectedCreature(m_session);
    if (creature == nullptr)
        return true;

    bool creature_in_front = creature->isInFront(m_session->GetPlayer());
    bool in_front_of_creature = m_session->GetPlayer()->isInFront(creature);
    float distance_to_creature = m_session->GetPlayer()->CalcDistance(creature);

    uint32 creature_state = creature->GetAIInterface()->m_creatureState;
    uint32 ai_state = creature->GetAIInterface()->getAIState();
    uint32 ai_type = creature->GetAIInterface()->getAIType();
    uint32 ai_agent = creature->GetAIInterface()->getCurrentAgent();

    uint32 current_wp = creature->GetAIInterface()->getCurrentWaypoint();
    uint32 wp_script_type = creature->GetAIInterface()->GetWaypointScriptType();

    //bool run = creature->GetAIInterface()->HasWalkMode(WALKMODE_RUN) || creature->GetAIInterface()->HasWalkMode(WALKMODE_SPRINT);

    uint32 attackerscount = creature->GetAIInterface()->getAITargetsCount();

    BlueSystemMessage(m_session, "Showing creature moveinfo for %s", creature->GetCreatureInfo()->Name);
    SystemMessage(m_session, "=== Facing ===");
    SystemMessage(m_session, "Creature is in front: %u", creature_in_front);
    SystemMessage(m_session, "In front of the creature: %u", in_front_of_creature);
    SystemMessage(m_session, "Current distance to creature: %f", distance_to_creature);
    SystemMessage(m_session, "=== States ===");
    SystemMessage(m_session, "Current state: %u", creature_state);
    SystemMessage(m_session, "Current AI state: %u | AIType: %u | AIAgent: ", ai_state, ai_type, ai_agent);
    SystemMessage(m_session, "Current waypoint id: %u | wp script type: %u", current_wp, wp_script_type);
    SystemMessage(m_session, "=== Misc ===");
    SystemMessage(m_session, "Attackers count: %u", attackerscount);

    return true;
}
