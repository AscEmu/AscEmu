/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

// Zyres: not only for npc!
bool ChatHandler::HandlePossessCommand(const char* args, WorldSession* m_session)
{
    Unit* pTarget = GetSelectedUnit(m_session);
    if (pTarget != nullptr)
    {
        if (pTarget->IsPet() || pTarget->GetCreatedByGUID() != 0)
        {
            RedSystemMessage(m_session, "You can not possess a pet!");
            return false;
        }
        else if (pTarget->IsPlayer())
        {
            Player* player = static_cast<Player*>(pTarget);
            BlueSystemMessage(m_session, "Player %s selected.", player->GetName());
            sGMLog.writefromsession(m_session, "used possess command on PLAYER %s", player->GetName());
        }
        else if (pTarget->IsCreature())
        {
            Creature* creature = static_cast<Creature*>(pTarget);
            BlueSystemMessage(m_session, "Creature %s selected.", creature->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "used possess command on Creature spawn_id %u", creature->GetCreatureInfo()->Name, creature->GetSQL_id());
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return true;
    }

    m_session->GetPlayer()->Possess(pTarget);

    return true;
}
