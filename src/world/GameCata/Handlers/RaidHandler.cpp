/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleGroupChangeSubGroup(WorldPacket& recvData)
{
    std::string name;
    uint8 subGroup;
    recvData >> name;
    recvData >> subGroup;

    PlayerInfo* inf = objmgr.GetPlayerInfoByName(name.c_str());
    if (inf == nullptr || inf->m_Group == nullptr || inf->m_Group != _player->m_playerInfo->m_Group)
        return;

    _player->GetGroup()->MovePlayer(inf, subGroup);
}

void WorldSession::HandleGroupAssistantLeader(WorldPacket& recvData)
{
    uint64 guid;
    uint8 on;

    if (_player->GetGroup() == nullptr)
        return;

    if (_player->GetGroup()->GetLeader() != _player->m_playerInfo)      //access denied
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    recvData >> guid >> on;
    if (on == 0)
        _player->GetGroup()->SetAssistantLeader(nullptr);
    else
    {
        PlayerInfo* np = objmgr.GetPlayerInfo(static_cast<uint32>(guid));
        if (np == nullptr)
            _player->GetGroup()->SetAssistantLeader(nullptr);
        else
        {
            if (_player->GetGroup()->HasMember(np))
                _player->GetGroup()->SetAssistantLeader(np);
        }
    }
}

void WorldSession::HandleGroupPromote(WorldPacket& recvData)
{
    uint8 promotetype;
    uint8 on;
    uint64 guid;

    if (_player->GetGroup() == nullptr)
        return;

    if (_player->GetGroup()->GetLeader() != _player->m_playerInfo)      //access denied
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    recvData >> promotetype;
    recvData >> on;
    recvData >> guid;

    void (Group::*function_to_call)(PlayerInfo*) = nullptr;

    if (promotetype == 0)
        function_to_call = &Group::SetMainTank;
    else if (promotetype == 1)
        function_to_call = &Group::SetMainAssist;

    if (on == 0)
        (_player->GetGroup()->*function_to_call)(nullptr);
    else
    {
        PlayerInfo* np = objmgr.GetPlayerInfo(static_cast<uint32>(guid));
        if (np == nullptr)
            (_player->GetGroup()->*function_to_call)(nullptr);
        else
        {
            if (_player->GetGroup()->HasMember(np))
                (_player->GetGroup()->*function_to_call)(np);
        }
    }
}

void WorldSession::HandleRequestRaidInfoOpcode(WorldPacket& /*recv_data*/)
{
    //          SMSG_RAID_INSTANCE_INFO             = 716,  //(0x2CC)
    //sInstanceSavingManager.BuildRaidSavedInstancesForPlayer(_player);
    sInstanceMgr.BuildRaidSavedInstancesForPlayer(_player);
}

void WorldSession::HandleReadyCheckOpcode(WorldPacket& recvData)
{
    Group* pGroup = _player->GetGroup();

    if (!pGroup)
        return;

    if (recvData.size() == 0)
    {
        // only leader or leader assistant can perform the ready check
        if (pGroup->GetLeader() == _player->m_playerInfo || pGroup->GetAssistantLeader() == _player->m_playerInfo)
        {
            WorldPacket data(MSG_RAID_READY_CHECK, 8);
            data << GetPlayer()->GetGUID();
            /* send packet to group */
            pGroup->SendPacketToAll(&data);
        }
        else
        {
            SendNotification(NOTIFICATION_MESSAGE_NO_PERMISSION);
        }
    }
    else
    {
        uint8 ready;
        recvData >> ready;

        WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
        data << _player->GetGUID();
        data << ready;

        if (pGroup->GetLeader() && pGroup->GetLeader()->m_loggedInPlayer)
            pGroup->GetLeader()->m_loggedInPlayer->GetSession()->SendPacket(&data);
        if (pGroup->GetAssistantLeader() && pGroup->GetAssistantLeader()->m_loggedInPlayer)
            pGroup->GetAssistantLeader()->m_loggedInPlayer->GetSession()->SendPacket(&data);
    }
}
