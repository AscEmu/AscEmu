/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Spell/SpellFailure.h"

bool ChatHandler::HandleDebugMoveInfo(const char* /*args*/, WorldSession* m_session)
{
    uint32 guid = Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->GetSelection());
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return true;

    bool creature_in_front = selected_unit->isInFront(m_session->GetPlayer());
    bool in_front_of_creature = m_session->GetPlayer()->isInFront(selected_unit);
    float distance_to_creature = m_session->GetPlayer()->CalcDistance(selected_unit);

    uint32 creature_state = selected_unit->GetAIInterface()->m_creatureState;
    uint32 ai_state = selected_unit->GetAIInterface()->getAIState();
    uint32 ai_type = selected_unit->GetAIInterface()->getAIType();
    uint32 ai_agent = selected_unit->GetAIInterface()->getCurrentAgent();

    uint32 current_wp = selected_unit->GetAIInterface()->getCurrentWaypoint();
    uint32 wp_script_type = selected_unit->GetAIInterface()->GetWaypointScriptType();

    uint32 walk_mode = selected_unit->GetAIInterface()->GetWalkMode();

    uint32 attackerscount = static_cast<uint32>(selected_unit->GetAIInterface()->getAITargetsCount());

    if (selected_unit->IsCreature())
        BlueSystemMessage(m_session, "Showing creature moveinfo for %s", static_cast<Creature*>(selected_unit)->GetCreatureProperties()->Name.c_str());
    else
        BlueSystemMessage(m_session, "Showing player moveinfo for %s", static_cast<Player*>(selected_unit)->GetName());

    SystemMessage(m_session, "=== Facing ===");
    SystemMessage(m_session, "Target is in front: %u", creature_in_front);
    SystemMessage(m_session, "In front of the target: %u", in_front_of_creature);
    SystemMessage(m_session, "Current distance to target: %f", distance_to_creature);
    SystemMessage(m_session, "=== States ===");
    SystemMessage(m_session, "Current state: %u", creature_state);
    SystemMessage(m_session, "Current AI state: %u | AIType: %u | AIAgent: ", ai_state, ai_type, ai_agent);
    SystemMessage(m_session, "Current waypoint id: %u | wp script type: %u", current_wp, wp_script_type);
    SystemMessage(m_session, "Walkmode: %u", walk_mode);
    SystemMessage(m_session, "=== Misc ===");
    SystemMessage(m_session, "Attackers count: %u", attackerscount);
    SystemMessage(m_session, "=== UnitMovementFlags ===");
    SystemMessage(m_session, "MovementFlags: %u", selected_unit->GetUnitMovementFlags());

    return true;
}

//.debug hover
bool ChatHandler::HandleDebugHover(const char* /*args*/, WorldSession* m_session)
{
    uint32 guid = Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->GetSelection());
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    if (selected_unit->HasUnitMovementFlag(MOVEFLAG_HOVER))
    {
        GreenSystemMessage(m_session, "Unset Hover for target.");
        selected_unit->SetMoveHover(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Set Hover for target.");
        selected_unit->SetMoveHover(true);
    }

    return true;
}

//.debug states
bool ChatHandler::HandleDebugState(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    GreenSystemMessage(m_session, "Display unitStateFlag: %u", selected_unit->GetUnitStateFlags());
    
    return true;
}

//.debug swim
bool ChatHandler::HandleDebugSwim(const char* /*args*/, WorldSession* m_session)
{
    Creature* selected_creature = GetSelectedCreature(m_session);
    if (selected_creature == nullptr)
        return false;

    if (selected_creature->HasUnitMovementFlag(MOVEFLAG_SWIMMING))
    {
        GreenSystemMessage(m_session, "Unset Swim for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->SetMoveSwim(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Set Swim for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->SetMoveSwim(true);
    }

    return true;
}

//.debug pvpcredit
bool ChatHandler::HandleDebugPVPCreditCommand(const char* args, WorldSession* m_session)
{
    uint32 rank;
    uint32 points;
    if (sscanf(args, "%u %u", &rank, &points) != 2)
    {
        RedSystemMessage(m_session, "Command must be in format <rank> <points>.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    points *= 10;

    GreenSystemMessage(m_session, "Building packet with Rank %u, Points %u, for Player %s.", rank, points, player_target->GetName());

    WorldPacket data(SMSG_PVP_CREDIT, 12);
    data << points;
    data << player_target->GetGUID();
    data << rank;
    m_session->SendPacket(&data);

    return true;
}

//.playmovie
bool ChatHandler::HandlePlayMovie(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32 movie = atol(args);

    selected_player->SendTriggerMovie(movie);

    if (selected_player != m_session->GetPlayer())
        GreenSystemMessage(selected_player->GetSession(), "Movie started for player %s", selected_player->GetName());

    return true;
}

//.sendfail
bool ChatHandler::HandleSendCastFailed(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32 fail = atol(args);
    if (SPELL_CANCAST_OK < fail)
    {
        RedSystemMessage(m_session, "Argument %u is out of range!", fail);
        return false;
    }
    selected_player->SendCastResult(1, (uint8)fail, 0, 0);

    return true;
}
