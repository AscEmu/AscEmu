/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/Packets/CmsgGroupInvite.h"
#include "Server/Packets/SmsgGroupInvite.h"
#include "Server/Packets/SmsgPartyCommandResult.h"
#include "Server/Packets/SmsgGroupDecline.h"
#include "Server/Packets/CmsgGroupUninvite.h"
#include "Server/Packets/CmsgGroupUninviteGuid.h"
#include "Server/Packets/MsgMinimapPing.h"
#include "Server/Packets/CmsgGroupSetLeader.h"
#include "Server/Packets/CmsgLootMethod.h"
#include "Server/Packets/MsgRaidTargetUpdate.h"
#include "Server/Packets/CmsgRequestPartyMemberStats.h"
#include "Server/Packets/SmsgPartyMemberStatsFull.h"
#include "Server/WorldSession.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/Packets/CmsgGroupChangeSubGroup.h"
#include "Server/Packets/CmsgGroupAssistantLeader.h"
#include "Server/Packets/MsgPartyAssign.h"
#include "Server/Packets/MsgRaidReadyCheck.h"
#include "Server/Packets/MsgRaidReadyCheckConfirm.h"
#include "Server/Packets/CmsgGroupInviteResponse.h"
#include "Server/Packets/CmsgGroupSetRoles.h"
#include "Server/Packets/SmsgGroupSetRoles.h"
#include "Server/Packets/SmsgRealGroupUpdate.h"
#include "Server/Packets/SmsgRoleCheckBegin.h"
#include "Server/Packets/CmsgGroupRaidConvert.h"
#include "Server/PacketBroadcast.hpp"

#if VERSION_STRING >= Cata
#include "Server/Packets/SmsgGroupList.h"
#endif

#include "Server/Packets/CmsgMinimapPing.h"
#include "Server/Packets/SmsgMinimapPing.h"
#include "Server/Packets/CmsgRaidTargetUpdate.h"
#include "Server/Packets/SmsgRaidTargetUpdateAll.h"
#include "Server/Packets/SmsgRaidTargetUpdateSingle.h"
#include "Server/Packets/SmsgRaidReadyCheck.h"
#include "Server/Packets/CmsgRaidReadyCheckConfirm.h"
#include "Server/Packets/SmsgRaidReadyCheckConfirm.h"
#include "Server/Packets/SmsgRaidReadyCheckCompleted.h"
#include "Server/Packets/CmsgSetPartyAssignment.h"
#include "Server/Packets/CmsgGroupInitiateRolePoll.h"
#include "Server/Packets/SmsgGroupRolePollInform.h"

using namespace AscEmu::Packets;


void WorldSession::sendEmptyGroupList([[maybe_unused]] Player* player)
{
#if VERSION_STRING >= Cata
    SmsgGroupList managedPacket;
    player->getSession()->sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleGroupInviteResponseOpcode(WorldPacket& recvPacket)
{
    CmsgGroupInviteResponse srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

#if VERSION_STRING >= Cata

    if (srlPacket.isAccepted)
    {
        if (_player->getGroup() != nullptr)
            return;

        Player* group_inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
        if (!group_inviter)
            return;

        group_inviter->setGroupInviterId(0);
        _player->setGroupInviterId(0);

        auto group = group_inviter->getGroup();
        if (group != nullptr)
        {
            group->AddMember(_player->m_playerInfo);
            _player->m_dungeonDifficulty = group->m_difficulty;
            _player->sendDungeonDifficultyPacket();
        }
        else
        {
            group = sObjectMgr.createGroup();
            group->m_difficulty = group_inviter->m_dungeonDifficulty;
            group->AddMember(group_inviter->m_playerInfo);
            group->AddMember(_player->m_playerInfo);
            _player->m_dungeonDifficulty = group->m_difficulty;
            _player->sendDungeonDifficultyPacket();
        }
    }
    else
    {
        Player* group_inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
        if (group_inviter == nullptr)
            return;

        group_inviter->setGroupInviterId(0);
        _player->setGroupInviterId(0);

        SmsgGroupDecline declinePacket(_player->getName());
        group_inviter->getSession()->sendManagedPacket(declinePacket);
    }
#endif
}

void WorldSession::handleGroupSetRolesOpcode(WorldPacket& recvPacket)
{
    CmsgGroupSetRoles srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    WoWGuid playerGuid = _player->getGuid();

    SmsgGroupSetRoles managedPacket(srlPacket.targetGuid, playerGuid, srlPacket.newRole);

    if (_player->getGroup())
        PacketBroadcast::sendFromGroup(*_player->getGroup(), managedPacket);
    else
        sendManagedPacket(managedPacket);
}

void WorldSession::handleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    auto group = _player->getGroup();
    if (group != nullptr)
    {
        SmsgRealGroupUpdate managedPacket(uint8_t(group->getGroupType()), uint32_t(group->GetMembersCount()));
        sendManagedPacket(managedPacket);
    }
#endif
}

void WorldSession::handleGroupRoleCheckBeginOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    auto group = _player->getGroup();
    if (!group)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader()->guid != _player->getGuid() && group->GetMainAssist()->guid != _player->getGuid())
            return;

        WoWGuid guid = _player->getGuid();

        SmsgRoleCheckBegin managedPacket(guid.getRawGuid());
        PacketBroadcast::sendFromGroup(*group, managedPacket);
    }
#endif
}

void WorldSession::handleGroupInviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupInvite srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto invitedPlayer = sObjectMgr.getPlayer(srlPacket.name.c_str(), false);
    if (invitedPlayer == nullptr)
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_CANNOT_FIND);
        sendManagedPacket(managedPacket);
        return;
    }

    if (invitedPlayer == _player || _player->isAlreadyInvitedToGroup())
        return;

    if (_player->isInGroup() && !_player->isGroupLeader())
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    if (_player->getGroup() != nullptr)
    {
        if (_player->getGroup()->IsFull())
        {
            SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_IS_FULL);
            sendManagedPacket(managedPacket);
            return;
        }
    }

    if (invitedPlayer->isInGroup())
    {
        SmsgPartyCommandResult managedPacket(invitedPlayer->getGroup()->getGroupType(), srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP);
        sendManagedPacket(managedPacket);

        SmsgGroupInvite managedInvitePacket(0, _player->getName(), _player->getGuid(), invitedPlayer->getGuid(), "", _player->getGroup() != nullptr);
        invitedPlayer->getSession()->sendManagedPacket(managedInvitePacket);
        return;
    }

    if (invitedPlayer->getTeam() != _player->getTeam() && !_player->getSession()->hasPermissions() && !worldConfig.player.isInterfactionGroupEnabled)
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_WRONG_FACTION);
        sendManagedPacket(managedPacket);
        return;
    }

    if (invitedPlayer->isAlreadyInvitedToGroup())
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP);
        sendManagedPacket(managedPacket);
        return;
    }

    if (invitedPlayer->isIgnored(_player->getGuidLow()))
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_IS_IGNORING_YOU);
        sendManagedPacket(managedPacket);
        return;
    }

    if (invitedPlayer->isGMFlagSet() && !_player->getSession()->hasPermissions())
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_CANNOT_FIND);
        sendManagedPacket(managedPacket);
        return;
    }

    SmsgGroupInvite managedInvitePacket(1, _player->getName(), _player->getGuid(), invitedPlayer->getGuid(), "", _player->getGroup() != nullptr);
    invitedPlayer->getSession()->sendManagedPacket(managedInvitePacket);

    SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_NO_ERROR);
    sendManagedPacket(managedPacket);

    invitedPlayer->setGroupInviterId(_player->getGuidLow());
}


//\brief Not used for cata - the client sends a response
//       Check out handleGroupInviteResponseOpcode!
void WorldSession::handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    const auto inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
    if (inviter == nullptr)
        return;

    SmsgGroupDecline managedPacket(_player->getName());
    inviter->getSession()->sendManagedPacket(managedPacket);
    inviter->setGroupInviterId(0);
    _player->setGroupInviterId(0);
}

void WorldSession::handleGroupAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->getGroup())
        return;

    const auto player = sObjectMgr.getPlayer(_player->getGroupInviterId());
    if (player == nullptr)
        return;

    _player->setGroupInviterId(0);
    player->setGroupInviterId(0);

    auto group = player->getGroup();
    if (group == nullptr)
    {
        group = sObjectMgr.createGroup();
        group->AddMember(player->getPlayerInfo());
        group->AddMember(_player->getPlayerInfo());
        group->m_difficulty = player->m_dungeonDifficulty;
        _player->m_dungeonDifficulty = player->m_dungeonDifficulty;
        _player->sendDungeonDifficultyPacket();
    }
    else
    {
        group->AddMember(_player->getPlayerInfo());
        _player->m_dungeonDifficulty = group->m_difficulty;
        _player->sendDungeonDifficultyPacket();
    }
}

void WorldSession::handleGroupUninviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninvite srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_GROUP_UNINVITE: {} (name).", srlPacket.name);

    const auto uninvitePlayer = sObjectMgr.getPlayer(srlPacket.name.c_str(), false);
    if (uninvitePlayer == nullptr)
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_CANNOT_FIND);
        sendManagedPacket(managedPacket);
        return;
    }

    if (!_player->isInGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->getGroup())
    {
        SmsgPartyCommandResult managedPacket(0, srlPacket.name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        sendManagedPacket(managedPacket);
        return;
    }

    if (!_player->isGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
            sendManagedPacket(managedPacket);
            return;
        }
    }

    const auto group = _player->getGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupUninviteGuidOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninviteGuid srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_GROUP_UNINVITE_GUID: {} (guidLow).", srlPacket.guid.getGuidLow());

    const auto uninvitePlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (uninvitePlayer == nullptr)
    {
        SmsgPartyCommandResult managedPacket(0, "unknown", ERR_PARTY_CANNOT_FIND);
        sendManagedPacket(managedPacket);
        return;
    }

    const std::string name = uninvitePlayer->getName();

    if (!_player->isInGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->getGroup())
    {
        SmsgPartyCommandResult managedPacket(0, name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        sendManagedPacket(managedPacket);
        return;
    }

    if (!_player->isGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
            sendManagedPacket(managedPacket);
            return;
        }
    }

    const auto group = _player->getGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->getGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(_player->getPlayerInfo());
}

void WorldSession::handleMinimapPingOpcode(WorldPacket& recvPacket)
{
    if (_socket->getClientProtocol().isMop())
    {
        CmsgMinimapPing srlPacket;
        if (!parsePacket(recvPacket, srlPacket))
            return;

        sLogger.debugOpcode("Received CMSG_MINIMAP_PING: {} (x), {} (y).", srlPacket.posX, srlPacket.posY);

        if (!_player->isInGroup())
            return;

        const auto group = _player->getGroup();
        if (group == nullptr)
            return;

        SmsgMinimapPing managedPacket(_player->getGuid(), srlPacket.posX, srlPacket.posY);
        PacketBroadcast::sendFromGroup(*group, managedPacket, _player);
        return;
    }

    MsgMinimapPing srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_MINIMAP_PING: {} (x), {} (y).", srlPacket.posX, srlPacket.posY);

    if (!_player->isInGroup())
        return;

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    MsgMinimapPing managedPacket(_player->getGuid(), srlPacket.posX, srlPacket.posY);
    PacketBroadcast::sendFromGroup(*group, managedPacket, _player);
}

void WorldSession::handleGroupSetLeaderOpcode(WorldPacket& recvPacket)
{
    CmsgGroupSetLeader srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_GROUP_SET_LEADER: {} (guidLow).", srlPacket.guid.getGuidLow());

    const auto targetPlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (targetPlayer == nullptr)
    {
        SmsgPartyCommandResult managedPacket(0, _player->getName(), ERR_PARTY_CANNOT_FIND);
        sendManagedPacket(managedPacket);
        return;
    }

    if (!_player->isGroupLeader())
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    if (targetPlayer->getGroup() != _player->getGroup())
    {
        SmsgPartyCommandResult managedPacket(0, _player->getName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto group = _player->getGroup();
    if (group)
        group->SetLeader(targetPlayer, false);
}

void WorldSession::handleLootMethodOpcode(WorldPacket& recvPacket)
{
    CmsgLootMethod srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_LOOT_METHOD: {} (method), {} (guidLow), {} (theshold).", srlPacket.method, srlPacket.guid.getGuidLow(), srlPacket.threshold);

    if (!_player->isGroupLeader())
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    const auto lootMasterPlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (lootMasterPlayer == nullptr)
        group->SetLooter(_player, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));
    else
        group->SetLooter(lootMasterPlayer, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));

}

void WorldSession::handleSetPlayerIconOpcode(WorldPacket& recvPacket)
{
    if (_socket->getClientProtocol().isMop())
    {
        CmsgRaidTargetUpdate srlPacket;
        if (!parsePacket(recvPacket, srlPacket))
            return;

        sLogger.debugOpcode("Received CMSG_RAID_TARGET_UPDATE: {} (icon).", srlPacket.symbol);

        const auto group = _player->getGroup();
        if (group == nullptr)
            return;

        if (srlPacket.symbol == 0xFF)
        {
            SmsgRaidTargetUpdateAll managedPacket(group);
            sendManagedPacket(managedPacket);
        }
        else if (_player->isGroupLeader())
        {
            if (srlPacket.symbol >= 8)
                return;

            for (uint8_t i = 0; i < 8; ++i)
            {
                if (group->m_targetIcons[i] == srlPacket.targetGuid)
                {
                    group->m_targetIcons[i] = 0;
                    SmsgRaidTargetUpdateSingle managedPacket(_player->getGuid(), 0, i, 0);
                    PacketBroadcast::sendFromGroup(*group, managedPacket);
                }
            }

            SmsgRaidTargetUpdateSingle managedPacket(_player->getGuid(), srlPacket.targetGuid, srlPacket.symbol, srlPacket.index);
            PacketBroadcast::sendFromGroup(*group, managedPacket);

            group->m_targetIcons[srlPacket.symbol] = srlPacket.targetGuid;
        }

        return;
    }

    MsgRaidTargetUpdate srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_RAID_TARGET_UPDATE: {} (icon).", srlPacket.icon);

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (srlPacket.icon == 0xFF)
    {
        MsgRaidTargetUpdate managedPacket(1, 0, 0, 0, group);
        sendManagedPacket(managedPacket);
    }
    else if (_player->isGroupLeader())
    {
        if (srlPacket.icon >= 8)
            return;

        for (uint8_t i = 0; i < 8; ++i)
        {
            if (group->m_targetIcons[i] == srlPacket.guid)
            {
                group->m_targetIcons[i] = 0;
                MsgRaidTargetUpdate managedPacket(0, 0, i, 0, nullptr);
                PacketBroadcast::sendFromGroup(*group, managedPacket);
            }
        }

        MsgRaidTargetUpdate managedPacket(0, _player->getGuid(), srlPacket.icon, srlPacket.guid, nullptr);
        PacketBroadcast::sendFromGroup(*group, managedPacket);

        group->m_targetIcons[srlPacket.icon] = srlPacket.guid;
    }
}

//\brief: Not used on Cata
void WorldSession::handlePartyMemberStatsOpcode(WorldPacket& recvPacket)
{
    CmsgRequestPartyMemberStats srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_REQUEST_PARTY_MEMBER_STATS: {} (guidLow).", srlPacket.guid.getGuidLow());

    if (_player->getWorldMap() == nullptr)
    {
        sLogger.debugOpcode("Received CMSG_REQUEST_PARTY_MEMBER_STATS: But MapMgr is not ready!");
        return;
    }

    const auto requestedPlayer = _player->getWorldMap()->getPlayer(srlPacket.guid.getGuidLow());
    if (_player->getGroup() == nullptr || requestedPlayer == nullptr)
    {
#if VERSION_STRING < Mop
        SmsgPartyMemberStatsFull managedPacket(srlPacket.guid, nullptr);
        sendManagedPacket(managedPacket);
#endif
        return;
    }

    if (!_player->getGroup()->HasMember(requestedPlayer))
        return;

    if (_player->isVisibleObject(requestedPlayer->getGuid()))
        return;

    // SMSG_PARTY_MEMBER_STATS_FULL was replaced by SMSG_PARTY_MEMBER_STATE in Mop, which is not implemented yet
    if (_socket->getClientProtocol().isMop())
        return;

    SmsgPartyMemberStatsFull managedPacket(requestedPlayer->getGuid(), requestedPlayer);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleConvertGroupToRaidOpcode(WorldPacket& recvPacket)
{
    CmsgGroupRaidConvert srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto const group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->m_playerInfo)
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    if (srlPacket.toRaid)
        group->ExpandToRaid();

    SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_NO_ERROR);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleRequestRaidInfoOpcode(WorldPacket& /*recvPacket*/)
{
    _player->sendRaidInfo();
}

void WorldSession::handleGroupChangeSubGroup(WorldPacket& recvPacket)
{
    CmsgGroupChangeSubGroup srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto playerInfo = _socket->getClientProtocol().isMop()
        ? sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow())
        : sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (playerInfo == nullptr || playerInfo->m_Group == nullptr)
        return;

    if (playerInfo->m_Group != _player->getGroup())
        return;

    _player->getGroup()->MovePlayer(playerInfo, srlPacket.subGroup);
}

void WorldSession::handleGroupAssistantLeader(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    CmsgGroupAssistantLeader srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (srlPacket.isActivated)
    {
        const auto playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());
        if (playerInfo == nullptr)
        {
            group->SetAssistantLeader(nullptr);
        }
        else
        {
            if (group->HasMember(playerInfo))
                group->SetAssistantLeader(playerInfo);
        }
    }
}

void WorldSession::handleGroupPromote(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SmsgPartyCommandResult managedPacket(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        sendManagedPacket(managedPacket);
        return;
    }

    MsgPartyAssign srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CachedCharacterInfo* playerInfo = nullptr;

    if (srlPacket.isActivated)
        playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());

    if (srlPacket.promoteType == 1)
        group->SetMainAssist(playerInfo);
    else if (srlPacket.promoteType == 0)
        group->SetMainTank(playerInfo);
}

void WorldSession::handleReadyCheckOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader() == _player->getPlayerInfo() || group->GetAssistantLeader() == _player->getPlayerInfo())
        {
            MsgRaidReadyCheck managedPacket(_player->getGuid(), 0, true);
            PacketBroadcast::sendFromGroup(*group, managedPacket);

            // offline members can't answer - report them as "not ready" to the leader/assistant
            for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
            {
                for (auto* memberInfo : group->GetSubGroup(i)->getGroupMembers())
                {
                    const auto memberPlayer = sObjectMgr.getPlayer(memberInfo->guid);
                    if (memberPlayer != nullptr && memberPlayer->getSession() != nullptr)
                        continue;

                    MsgRaidReadyCheckConfirm offlineConfirm(memberInfo->guid, 0);

                    if (group->GetLeader())
                    {
                        if (Player* leader = sObjectMgr.getPlayer(group->GetLeader()->guid))
                            if (leader->getSession())
                                leader->getSession()->sendManagedPacket(offlineConfirm);
                    }

                    if (group->GetAssistantLeader())
                    {
                        if (Player* assistant = sObjectMgr.getPlayer(group->GetAssistantLeader()->guid))
                            if (assistant->getSession())
                                assistant->getSession()->sendManagedPacket(offlineConfirm);
                    }
                }
            }
        }
        else
        {
            sendNotification("You do not have permission to perform that function.");
        }
    }
    else
    {
        MsgRaidReadyCheck srlPacket;
        if (!parsePacket(recvPacket, srlPacket))
            return;

        MsgRaidReadyCheckConfirm managedPacket(_player->getGuid(), srlPacket.isReady);

        if (group->GetLeader())
        {
            if (Player* leader = sObjectMgr.getPlayer(group->GetLeader()->guid))
                if (leader->getSession())
                    leader->getSession()->sendManagedPacket(managedPacket);
        }

        if (group->GetAssistantLeader())
        {
            if (Player* assistant = sObjectMgr.getPlayer(group->GetAssistantLeader()->guid))
                if (assistant->getSession())
                    assistant->getSession()->sendManagedPacket(managedPacket);
        }
    }
}

#if VERSION_STRING >= Mop
void WorldSession::handleRaidReadyCheckOpcode(WorldPacket& /*recvPacket*/)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo() && group->GetAssistantLeader() != _player->getPlayerInfo())
    {
        sendNotification("You do not have permission to perform that function.");
        return;
    }

    if (group->readyCheckInProgress())
        return;

    const WoWGuid groupGuid(group->GetID(), 0, HIGHGUID_TYPE_GROUP);

    group->readyCheckStart();
    group->readyCheckMemberResponded(_player->getGuidLow());

    SmsgRaidReadyCheck managedPacket(groupGuid, _player->getGuid(), 35000);
    PacketBroadcast::sendFromGroup(*group, managedPacket);

    // offline members can't answer - count them as "not ready" immediately
    for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
    {
        for (auto* memberInfo : group->GetSubGroup(i)->getGroupMembers())
        {
            const auto memberPlayer = sObjectMgr.getPlayer(memberInfo->guid);
            if (memberPlayer != nullptr && memberPlayer->getSession() != nullptr)
                continue;

            group->readyCheckMemberResponded(memberInfo->guid);

            const WoWGuid offlinePlayerGuid(memberInfo->guid, 0, HIGHGUID_TYPE_PLAYER);
            SmsgRaidReadyCheckConfirm offlineConfirm(groupGuid, offlinePlayerGuid, false);
            PacketBroadcast::sendFromGroup(*group, offlineConfirm);
        }
    }

    if (group->readyCheckAllResponded())
    {
        group->readyCheckStop();

        SmsgRaidReadyCheckCompleted completedPacket(groupGuid);
        PacketBroadcast::sendFromGroup(*group, completedPacket);
    }
}

void WorldSession::handleRaidReadyCheckConfirmOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (!group->readyCheckInProgress())
        return;

    CmsgRaidReadyCheckConfirm srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const WoWGuid groupGuid(group->GetID(), 0, HIGHGUID_TYPE_GROUP);

    SmsgRaidReadyCheckConfirm managedPacket(groupGuid, _player->getGuid(), srlPacket.status);
    PacketBroadcast::sendFromGroup(*group, managedPacket);

    group->readyCheckMemberResponded(_player->getGuidLow());

    if (group->readyCheckAllResponded())
    {
        group->readyCheckStop();

        SmsgRaidReadyCheckCompleted completedPacket(groupGuid);
        PacketBroadcast::sendFromGroup(*group, completedPacket);
    }
}

void WorldSession::handleSetPartyAssignmentOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo() && group->GetAssistantLeader() != _player->getPlayerInfo())
        return;

    CmsgSetPartyAssignment srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CachedCharacterInfo* playerInfo = nullptr;
    if (srlPacket.apply)
        playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());

    if (srlPacket.assignment == 1)
        group->SetMainAssist(playerInfo);
    else if (srlPacket.assignment == 0)
        group->SetMainTank(playerInfo);
}

void WorldSession::handleGroupInitiateRolePollOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo() && group->GetAssistantLeader() != _player->getPlayerInfo())
        return;

    CmsgGroupInitiateRolePoll srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    SmsgGroupRolePollInform managedPacket(_player->getGuid(), srlPacket.partyIndex);
    PacketBroadcast::sendFromGroup(*group, managedPacket);
}
#endif
