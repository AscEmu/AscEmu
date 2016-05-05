/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

// Zyres: not only for npc!
bool ChatHandler::HandlePossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);
    if (unit_target != nullptr)
    {
        if (unit_target->IsPet() || unit_target->GetCreatedByGUID() != 0)
        {
            RedSystemMessage(m_session, "You can not possess a pet!");
            return false;
        }
        else if (unit_target->IsPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            if (player == nullptr)
                return false;

            BlueSystemMessage(m_session, "Player %s selected.", player->GetName());
            sGMLog.writefromsession(m_session, "used possess command on PLAYER %s", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            if (creature == nullptr)
                return false;

            BlueSystemMessage(m_session, "Creature %s selected.", creature->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "used possess command on Creature spawn_id %u", creature->GetCreatureInfo()->Name, creature->GetSQL_id());
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->Possess(unit_target);

    return true;
}

bool ChatHandler::HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);

    if (unit_target != nullptr)
    {
        if (unit_target->IsPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            if (player == nullptr)
                return false;

            BlueSystemMessage(m_session, "Player %s is no longer possessed by you.", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            if (creature == nullptr)
                return false;

            BlueSystemMessage(m_session, "Creature %s is no longer possessed by you.", creature->GetCreatureInfo()->Name);
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->UnPossess();

    return true;
}
