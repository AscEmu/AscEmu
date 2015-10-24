/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"

bool ChatHandler::HandleGetTransporterTime(const char* args, WorldSession* m_session)
{
    /*
    //Player* plyr = m_session->GetPlayer();
    Creature* crt = getSelectedCreature(m_session, false);
    if (crt == NULL)
    return false;

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 1000);
    data << uint32(0x00000102);
    data << crt->GetNewGUID();
    data << m_session->GetPlayer()->GetNewGUID();

    data << uint32(6);
    data << uint8(1);
    data << uint32(1);
    data << uint32(0x40c00000);
    data << uint32(6);
    data << uint32(0);
    data << uint32(0);
    data << uint32(1);
    data << uint32(0x000003e8);
    data << uint32(0);
    data << uint32(0);
    m_session->SendPacket(&data);
    */

    Transporter* t = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->transporter_info.guid));

    if (t == NULL)
        return false;

    //t->Relocate(-377.184021f, 2073.548584f, 445.753387f);

    return true;
}

bool ChatHandler::HandleModPeriodCommand(const char* args, WorldSession* m_session)
{
    Transporter* trans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->transporter_info.guid));
    if (trans == 0)
    {
        RedSystemMessage(m_session, "You must be on a transporter.");
        return true;
    }

    uint32 np = args ? atol(args) : 0;
    if (np == 0)
    {
        RedSystemMessage(m_session, "A time in ms must be specified.");
        return true;
    }

    trans->SetPeriod(np);
    BlueSystemMessage(m_session, "Period of %s set to %u.", trans->GetInfo()->name, np);

    return true;
}

bool ChatHandler::HandleStopTransport(const char* args, WorldSession* m_session)
{
    Transporter* trans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->transporter_info.guid));
    if (trans == NULL)
    {
        BlueSystemMessage(m_session, "You are not on a Transporter");
        return false;
    }

    if (trans->GetState() == GAMEOBJECT_STATE_OPEN)
    {
        trans->m_WayPoints.clear();
        trans->RemoveFlag(GAMEOBJECT_FLAGS, 1);
        trans->SetState(GAMEOBJECT_STATE_CLOSED);
    }

    return true;
}

bool ChatHandler::HandleStartTransport(const char* args, WorldSession* m_session)
{
    Transporter* trans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->transporter_info.guid));
    if (trans == NULL)
    {
        BlueSystemMessage(m_session, "You are not on a Transporter");
        return false;
    }

    if (trans->GetState() == GAMEOBJECT_STATE_CLOSED)
    {
        trans->SetFlag(GAMEOBJECT_FLAGS, 1);
        trans->SetState(GAMEOBJECT_STATE_OPEN);
        trans->SetUInt32Value(GAMEOBJECT_DYNAMIC, 0x10830010); // Seen in sniffs
        trans->SetFloatValue(GAMEOBJECT_PARENTROTATION + 3, 1.0f);
        std::set<uint32> mapsUsed;
        GameObjectInfo const* goinfo = trans->GetInfo();

        trans->GenerateWaypoints(goinfo->parameter_0);
    }

    return true;
}

bool ChatHandler::HandleSpawnInstanceTransport(const char* args, WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();

    char* pEntry = strtok((char*)args, " ");
    if (!pEntry)
        return false;

    uint32 entry = atoi(pEntry);

    char* pPeriod = strtok(NULL, " ");
    if (!pPeriod)
        return false;

    uint32 period = atoi(pPeriod);

    Transporter* trans = objmgr.LoadTransporterInInstance(plr->GetMapMgr(), entry, period);

    return true;
}

bool ChatHandler::HandleDespawnInstanceTransport(const char* args, WorldSession* m_session)
{
    Transporter* trans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(m_session->GetPlayer()->transporter_info.guid));
    if (trans == NULL)
        return false;

    objmgr.UnLoadTransporterFromInstance(trans);

    return true;
}
