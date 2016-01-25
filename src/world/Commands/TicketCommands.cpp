/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"

bool ChatHandler::HandleTicketListCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();

    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE deleted=0");

    if (!result)
        return false;

    std::stringstream sstext;
    sstext << "List of active tickets: " << '\n';

    do
    {
        Field* fields = result->Fetch();
        sstext << "TicketID: " << fields[0].GetUInt16()
            << " | Player: " << fields[2].GetString()
            << '\n';
    } while (result->NextRow());

    delete result;

    SendMultilineMessage(m_session, sstext.str().c_str());
}
