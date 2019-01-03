/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Config/Config.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Chat/ChatDefines.hpp"
#include "WorldPacket.h"
#include "Units/Players/Player.h"

Mutex m_confSettingLock;
std::vector<std::string> m_bannedChannels;
std::vector<std::string> m_minimumChannel;
uint64 voicechannelhigh = 0;

void Channel::LoadConfSettings()
{
    std::string BannedChannels = worldConfig.chat.bannedChannels;
    std::string MinimumLevel = worldConfig.chat.minimumTalkLevel;
    m_confSettingLock.Acquire();
    m_bannedChannels = Util::SplitStringBySeperator(BannedChannels, ";");
    m_minimumChannel = Util::SplitStringBySeperator(MinimumLevel, ";");
    m_confSettingLock.Release();
}

bool Channel::HasMember(Player* pPlayer)
{
    m_lock.Acquire();
    if (m_members.find(pPlayer) == m_members.end())
    {
        m_lock.Release();
        return false;
    }
    else
    {
        m_lock.Release();
        return true;
    }
}

Channel::Channel(const char* name, uint32 team, uint32 type_id)
{
    m_flags = 0;
    m_announce = true;
    m_muted = false;
    m_general = false;
    m_name = std::string(name);
    m_team = team;
    m_id = type_id;
    m_minimumLevel = 1;

    auto chat_channels = sChatChannelsStore.LookupEntry(type_id);
    if (chat_channels != nullptr)
    {
        m_general = true;
        m_announce = false;

        m_flags |= CHANNEL_FLAGS_GENERAL;       // old 0x10;            // general flag
        // flags (0x01 = custom?, 0x04 = trade?, 0x20 = city?, 0x40 = lfg?, , 0x80 = voice?,

        if (chat_channels->flags & CHANNEL_DBC_TRADE)
            m_flags |= CHANNEL_FLAGS_TRADE;     // old 0x08;        // trade

        if (chat_channels->flags & CHANNEL_DBC_CITY_ONLY_1 || chat_channels->flags & CHANNEL_DBC_CITY_ONLY_2)
            m_flags |= CHANNEL_FLAGS_CITY;      // old 0x20;        // city flag

        if (chat_channels->flags & CHANNEL_DBC_LFG)
            m_flags |= CHANNEL_FLAGS_LFG;       // old 0x40;        // lfg flag
    }
    else
        m_flags = CHANNEL_FLAGS_CUSTOM;         // old 0x01;

    for (std::vector<std::string>::iterator itr = m_minimumChannel.begin(); itr != m_minimumChannel.end(); ++itr)
    {
        if (stricmp(name, itr->c_str()))
        {
            m_minimumLevel = 10;
            m_general = true;
            m_announce = false;
            break;
        }
    }
}

void Channel::AttemptJoin(Player* plr, const char* password)
{
    Guard mGuard(m_lock);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    uint32 flags = CHANNEL_MEMBER_FLAG_NONE;

    if (!m_general && plr->GetSession()->CanUseCommand('c'))
        flags |= CHANNEL_MEMBER_FLAG_MODERATOR;

    if (!m_password.empty() && strcmp(m_password.c_str(), password) != 0)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_WRONGPASS);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_bannedMembers.find(plr->getGuidLow()) != m_bannedMembers.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_YOURBANNED);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.find(plr) != m_members.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_ALREADY_ON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.empty() && !m_general)
        flags |= CHANNEL_MEMBER_FLAG_OWNER;

    plr->JoinedChannel(this);
    m_members.insert(std::make_pair(plr, flags));

    if (m_announce)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_JOINED);
        data << m_name;
        data << plr->getGuid();
        SendToAll(&data, NULL);
    }

    data.clear();
    if (m_flags & CHANNEL_FLAGS_LFG && !plr->GetSession()->HasFlag(ACCOUNT_FLAG_NO_AUTOJOIN))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_YOUJOINED);
        data << m_name;
        data << uint8(0x1A);
        data << uint32(0);
        data << uint32(0);
    }
    else
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_YOUJOINED);
        data << m_name;
        data << m_flags;
        data << m_id;
        data << uint32(0);
    }

    plr->GetSession()->SendPacket(&data);
}

void Channel::Part(Player* plr, bool send_packet)
{
    m_lock.Acquire();
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    uint32 flags;
    MemberMap::iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        m_lock.Release();
        return;
    }

    flags = itr->second;
    m_members.erase(itr);

    plr->LeftChannel(this);

    if (flags & CHANNEL_MEMBER_FLAG_OWNER)
    {
        // we need to find a new owner
        SetOwner(NULL, NULL);
    }

    if (plr->GetSession() && (plr->GetSession()->IsLoggingOut() || plr->m_TeleportState == 1))
    {

    }
    else if (send_packet)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_YOULEFT);
        data << m_name;
        data << m_id;
        data << uint32(0);
        data << uint8(0);
        plr->GetSession()->SendPacket(&data);
    }

    if (m_announce)
    {
        data.clear();
        data << uint8(CHANNEL_NOTIFY_FLAG_LEFT);
        data << m_name;
        data << plr->getGuid();
        SendToAll(&data);
    }

    if (m_members.size() == 0)
    {
        m_lock.Release();
        channelmgr.RemoveChannel(this);
    }
    else
        m_lock.Release();
}

void Channel::SetOwner(Player* oldpl, Player* plr)
{
    Guard mGuard(m_lock);
    Player* pOwner = NULL;
    uint32 oldflags = 0, oldflags2 = 0;
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (oldpl != NULL)
    {
        MemberMap::iterator itr = m_members.find(oldpl);
        if (m_members.end() == itr)
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
            data << m_name;
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_NOT_OWNER);
            data << m_name;
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }

    if (plr == NULL)
    {
        for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        {
            if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
            {
                // remove the old owner
                oldflags2 = itr->second;
                itr->second &= ~CHANNEL_MEMBER_FLAG_OWNER;
                data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
                data << m_name;
                data << itr->first->getGuid();
                data << uint8(oldflags2);
                data << uint8(itr->second);
                SendToAll(&data);
            }
            else
            {
                if (pOwner == NULL)
                {
                    pOwner = itr->first;
                    oldflags = itr->second;
                    itr->second |= CHANNEL_MEMBER_FLAG_OWNER;
                }
            }
        }
    }
    else
    {
        for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        {
            if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
            {
                // remove the old owner
                oldflags2 = itr->second;
                itr->second &= ~CHANNEL_MEMBER_FLAG_OWNER;
                data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
                data << m_name;
                data << itr->first->getGuid();
                data << uint8(oldflags2);
                data << uint8(itr->second);
                SendToAll(&data);
            }
            else
            {
                if (plr == itr->first)
                {
                    pOwner = itr->first;
                    oldflags = itr->second;
                    itr->second |= CHANNEL_MEMBER_FLAG_OWNER;
                }
            }
        }
    }

    if (pOwner == NULL)
        return;        // obviously no members

    data.clear();
    data << uint8(CHANNEL_NOTIFY_FLAG_CHGOWNER);
    data << m_name;
    data << pOwner->getGuid();
    SendToAll(&data);

    // send the mode changes
    data.clear();
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << pOwner->getGuid();
    data << uint8(oldflags);
    data << uint8(oldflags | CHANNEL_MEMBER_FLAG_OWNER);
    SendToAll(&data);
}

void Channel::Invite(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    if (m_members.find(plr) == m_members.end())
    {
        SendNotOn(plr);
        return;
    }

    if (m_members.find(new_player) != m_members.end())
    {
        SendAlreadyOn(plr, new_player);
        return;
    }

    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    data << uint8(CHANNEL_NOTIFY_FLAG_INVITED);
    data << m_name;
    data << plr->getGuid();
    new_player->GetSession()->SendPacket(&data);

    data.clear();
    data << uint8(CHANNEL_NOTIFY_FLAG_YOU_INVITED);
    data << m_name;
    data << new_player->getGuid();
    plr->GetSession()->SendPacket(&data);
}

void Channel::Moderate(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('c'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_muted = !m_muted;
    data << uint8(m_muted ? CHANNEL_NOTIFY_FLAG_MODERATED : CHANNEL_NOTIFY_FLAG_UNMODERATED);
    data << m_name;
    data << plr->getGuid();
    SendToAll(&data);
}

void Channel::Say(Player* plr, const char* message, Player* for_gm_client, bool forced)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, strlen(message) + 100);
    if (!forced)
    {
        if (m_members.end() == itr)
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
            data << m_name;
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if (itr->second & CHANNEL_MEMBER_FLAG_MUTED)
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK);
            data << m_name;
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if (m_muted && !(itr->second & CHANNEL_MEMBER_FLAG_VOICED) && !(itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK);
            data << m_name;
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }

    // not blizzlike but meh
    if (plr->getLevel() < m_minimumLevel)
    {
        plr->BroadcastMessage("You must be level %u to speak in the channel, '%s'.", m_minimumLevel, m_name.c_str());
        return;
    }

    data.SetOpcode(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_CHANNEL);
    data << uint32(0);        // language
    data << plr->getGuid();    // guid
    data << uint32(0);        // rank?
    data << m_name;            // channel name
    data << plr->getGuid();    // guid again?
    data << uint32(strlen(message) + 1);
    data << message;
    data << (uint8)(plr->isGMFlagSet() ? 4 : 0);
    if (for_gm_client != NULL)
        for_gm_client->GetSession()->SendPacket(&data);
    else
        SendToAll(&data);
}

void Channel::SendNotOn(Player* plr)
{
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
    data << m_name;
    plr->GetSession()->SendPacket(&data);
}

void Channel::SendAlreadyOn(Player* plr, Player* plr2)
{
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    data << uint8(CHANNEL_NOTIFY_FLAG_ALREADY_ON);
    data << m_name;
    data << plr2->getGuid();
    plr->GetSession()->SendPacket(&data);
}

void Channel::Kick(Player* plr, Player* die_player, bool ban)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(die_player);
    MemberMap::iterator me_itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    uint32 flags;

    if (me_itr == m_members.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (itr == m_members.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << die_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(me_itr->second & CHANNEL_MEMBER_FLAG_OWNER || me_itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    flags = itr->second;
    data << uint8(CHANNEL_NOTIFY_FLAG_KICKED);
    data << m_name;
    data << die_player->getGuid();
    SendToAll(&data);

    if (ban)
    {
        data.clear();
        data << uint8(CHANNEL_NOTIFY_FLAG_BANNED);
        data << m_name;
        data << die_player->getGuid();
        SendToAll(&data);
    }

    m_members.erase(itr);

    if (flags & CHANNEL_MEMBER_FLAG_OWNER)
        SetOwner(NULL, NULL);

    if (ban)
        m_bannedMembers.insert(die_player->getGuidLow());

    data.clear();
    data << uint8(CHANNEL_NOTIFY_FLAG_YOULEFT);
    data << m_name;
    data << m_id;
    data << uint32(0);
    data << uint8(0);
    die_player->GetSession()->SendPacket(&data);
}

void Channel::Unban(Player* plr, PlayerInfo* bplr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    std::set<uint32>::iterator it2 = m_bannedMembers.find(bplr->guid);
    if (it2 == m_bannedMembers.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << uint64(bplr->guid);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    data << uint8(CHANNEL_NOTIFY_FLAG_UNBANNED);
    data << m_name;
    data << uint64(bplr->guid);
    SendToAll(&data);
    m_bannedMembers.erase(it2);
}

void Channel::Voice(Player* plr, Player* v_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(v_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << v_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_VOICED;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << v_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Devoice(Player* plr, Player* v_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(v_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << v_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_VOICED;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << v_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Mute(Player* plr, Player* die_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(die_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << die_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MUTED;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << die_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Unmute(Player* plr, Player* die_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(die_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << die_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MUTED;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << die_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::GiveModerator(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(new_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << new_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MODERATOR;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << new_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::TakeModerator(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(new_player);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (m_members.end() == itr2)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << m_name;
        data << new_player->getGuid();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MODERATOR;
    data << uint8(CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << m_name;
    data << new_player->getGuid();
    data << uint8(oldflags);
    data << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Announce(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_announce = !m_announce;
    data << uint8(m_announce ? CHANNEL_NOTIFY_FLAG_ENABLE_ANN : CHANNEL_NOTIFY_FLAG_DISABLE_ANN);
    data << m_name;
    data << plr->getGuid();
    SendToAll(&data);
}

void Channel::Password(Player* plr, const char* pass)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (m_members.end() == itr)
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTMOD);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_password = std::string(pass);
    data << uint8(CHANNEL_NOTIFY_FLAG_SETPASS);
    data << m_name;
    data << plr->getGuid();
    SendToAll(&data);
}

void Channel::List(Player* plr)
{
    Guard mGuard(m_lock);
    WorldPacket data(SMSG_CHANNEL_LIST, 50 + (m_members.size() * 9));
    MemberMap::iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        data.Initialize(SMSG_CHANNEL_NOTIFY);
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint8 flags;
    data << uint8(1);
    data << m_name;
    data << uint8(m_flags);
    data << uint32(m_members.size());
    for (itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        data << itr->first->getGuid();
        flags = 0;
        if (!(itr->second & CHANNEL_MEMBER_FLAG_MUTED))
            flags |= CHANNEL_MEMBER_FLAG_VOICED;

        if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
            flags |= CHANNEL_MEMBER_FLAG_OWNER;

        if (itr->second & CHANNEL_MEMBER_FLAG_MODERATOR)
            flags |= CHANNEL_MEMBER_FLAG_MODERATOR;

        if (!m_general)
            flags |= CHANNEL_MEMBER_FLAG_CUSTOM;

        data << flags;
    }

    plr->GetSession()->SendPacket(&data);
}

void Channel::GetOwner(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(SMSG_CHANNEL_NOTIFY, 100);
    if (itr == m_members.end())
    {
        data << uint8(CHANNEL_NOTIFY_FLAG_NOTON);
        data << m_name;
        plr->GetSession()->SendPacket(&data);
        return;
    }

    for (itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
        {
            data << uint8(CHANNEL_NOTIFY_FLAG_WHO_OWNER);
            data << m_name;
            data << itr->first->getGuid();
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }
}
ChannelMgr::~ChannelMgr()
{
    for (uint8 i = 0; i < 2; ++i)
    {
        ChannelList::iterator itr = this->Channels[i].begin();
        for (; itr != this->Channels[i].end(); ++itr)
        {
            delete itr->second;
        }
        Channels[i].clear();
    }
}

Channel::~Channel()
{
    m_lock.Acquire();
    for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        itr->first->LeftChannel(this);
    m_lock.Release();
}

void Channel::SendToAll(WorldPacket* data)
{
    Guard guard(m_lock);
    for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        itr->first->GetSession()->SendPacket(data);
}

void Channel::SendToAll(WorldPacket* data, Player* plr)
{
    Guard guard(m_lock);
    for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->first != plr)
            itr->first->GetSession()->SendPacket(data);
    }
}

Channel* ChannelMgr::GetCreateChannel(const char* name, Player* p, uint32 type_id)
{
    ChannelList::iterator itr;
    ChannelList* cl = &Channels[0];
    Channel* chn;
    if (seperatechannels && p != NULL && stricmp(name, worldConfig.getGmClientChannelName().c_str()))
        cl = &Channels[p->getTeam()];

    lock.Acquire();
    for (itr = cl->begin(); itr != cl->end(); ++itr)
    {
        if (!stricmp(name, itr->first.c_str()))
        {
            lock.Release();
            return itr->second;
        }
    }

    // make sure the name isn't banned
    m_confSettingLock.Acquire();
    for (std::vector<std::string>::iterator itr2 = m_bannedChannels.begin(); itr2 != m_bannedChannels.end(); ++itr2)
    {
        if (!strnicmp(name, itr2->c_str(), itr2->size()))
        {
            lock.Release();
            m_confSettingLock.Release();
            return NULL;
        }
    }

    chn = new Channel(name, (seperatechannels && p != NULL) ? p->getTeam() : TEAM_ALLIANCE, type_id);
    m_confSettingLock.Release();//Channel::Channel() reads configs so we release the lock after we create the Channel.
    cl->insert(make_pair(chn->m_name, chn));
    lock.Release();
    return chn;
}

Channel* ChannelMgr::GetChannel(const char* name, Player* p)
{
    ChannelList::iterator itr;
    ChannelList* cl = &Channels[0];
    if (seperatechannels && stricmp(name, worldConfig.getGmClientChannelName().c_str()))
        cl = &Channels[p->getTeam()];

    lock.Acquire();
    for (itr = cl->begin(); itr != cl->end(); ++itr)
    {
        if (!stricmp(name, itr->first.c_str()))
        {
            lock.Release();
            return itr->second;
        }
    }

    lock.Release();
    return NULL;
}

Channel* ChannelMgr::GetChannel(const char* name, uint32 team)
{
    ChannelList::iterator itr;
    ChannelList* cl = &Channels[0];
    if (seperatechannels && stricmp(name, worldConfig.getGmClientChannelName().c_str()))
        cl = &Channels[team];

    lock.Acquire();
    for (itr = cl->begin(); itr != cl->end(); ++itr)
    {
        if (!stricmp(name, itr->first.c_str()))
        {
            lock.Release();
            return itr->second;
        }
    }

    lock.Release();
    return NULL;
}

void ChannelMgr::RemoveChannel(Channel* chn)
{
    ChannelList::iterator itr;
    ChannelList* cl = &Channels[0];
    if (seperatechannels)
        cl = &Channels[chn->m_team];

    lock.Acquire();
    for (itr = cl->begin(); itr != cl->end(); ++itr)
    {
        if (itr->second == chn)
        {
            cl->erase(itr);
            delete chn;
            lock.Release();
            return;
        }
    }

    lock.Release();
}

ChannelMgr::ChannelMgr()
{
    seperatechannels = false;
}
