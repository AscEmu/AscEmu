/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgSetActiveMover.h"
using namespace AscEmu::Packets;

void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    CmsgSetActiveMover cmsg;
    if (!cmsg.deserialise(recvData))
        return;

    if (cmsg.guid == m_MoverWoWGuid.GetOldGuid())
        return;

    if (_player->m_CurrentCharm != cmsg.guid.GetOldGuid() || _player->GetGUID() != cmsg.guid.GetOldGuid())
    {
        auto bad_packet = true;
#if VERSION_STRING >= TBC
        if (const auto vehicle = _player->GetCurrentVehicle())
            if (const auto owner = vehicle->GetOwner())
                if (owner->getGuid() == cmsg.guid.GetOldGuid())
                    bad_packet = false;
#endif
        if (bad_packet)
            return;
    }

    m_MoverWoWGuid.Init(cmsg.guid.GetOldGuid() == 0 ? _player->GetGUID() : cmsg.guid);

    // set up to the movement packet
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
}