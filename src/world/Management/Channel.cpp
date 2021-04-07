/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Chat/ChatDefines.hpp"
#include "WorldPacket.h"
#include "Units/Players/Player.h"
#include "Server/Packets/SmsgChannelNotify.h"
#include "Server/Packets/SmsgChannelList.h"

using namespace AscEmu::Packets;

bool Channel::HasMember(Player* pPlayer)
{
    m_lock.Acquire();
    if (m_members.find(pPlayer) == m_members.end())
    {
        m_lock.Release();
        return false;
    }

    m_lock.Release();
    return true;
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

    const auto chat_channels = sChatChannelsStore.LookupEntry(type_id);
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
    {
        m_flags = CHANNEL_FLAGS_CUSTOM;         // old 0x01;
    }

    for (const auto& channelName : sChannelMgr.m_minimumChannel)
    {
        if (name != channelName)
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
    m_lock.Acquire();
    uint32 flags = CHANNEL_MEMBER_FLAG_NONE;

    if (!m_general && plr->GetSession()->CanUseCommand('c'))
        flags |= CHANNEL_MEMBER_FLAG_MODERATOR;

    if (!m_password.empty() && strcmp(m_password.c_str(), password) != 0)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_WRONGPASS, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (m_bannedMembers.find(plr->getGuidLow()) != m_bannedMembers.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOURBANNED, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (m_members.find(plr) != m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (m_members.empty() && !m_general)
        flags |= CHANNEL_MEMBER_FLAG_OWNER;

    plr->JoinedChannel(this);
    m_members.insert(std::make_pair(plr, flags));

    if (m_announce)
    {
        SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_JOINED, m_name, plr->getGuid()).serialise().get(), nullptr);
    }

    if (m_flags & CHANNEL_FLAGS_LFG && !plr->GetSession()->HasFlag(ACCOUNT_FLAG_NO_AUTOJOIN))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUJOINED, m_name, 0, 0x1A).serialise().get());
    }
    else
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUJOINED, m_name, 0, m_flags, m_id).serialise().get());
    }
    m_lock.Release();
}

void Channel::Part(Player* plr, bool send_packet)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());

        m_lock.Release();
        return;
    }

    uint32 flags = itr->second;
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
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOULEFT, m_name, 0, 0, m_id).serialise().get());
    }

    if (m_announce)
    {
        SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_LEFT, m_name, plr->getGuid()).serialise().get());
    }

    if (m_members.empty())
    {
        sChannelMgr.removeChannel(this);
    }

    m_lock.Release();
}

void Channel::SetOwner(Player* oldpl, Player* plr)
{
    m_lock.Acquire();
    Player* pOwner = NULL;
    uint32 oldflags = 0, oldflags2 = 0;
    if (oldpl && plr)
    {
        MemberMap::iterator itr = m_members.find(oldpl);
        if (m_members.end() == itr)
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
            m_lock.Release();
            return;
        }

        if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_OWNER, m_name).serialise().get());
            m_lock.Release();
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
                SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, itr->first->getGuid(), oldflags2, 0, itr->second).serialise().get());
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
                SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, itr->first->getGuid(), oldflags2, 0, itr->second).serialise().get());
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

    if (pOwner == nullptr)
    {
        m_lock.Release();
        return;        // obviously no members
    }

    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_CHGOWNER, m_name, pOwner->getGuid()).serialise().get());

    // send the mode changes
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_OWNER, m_name, pOwner->getGuid(), oldflags, 0, (oldflags | CHANNEL_MEMBER_FLAG_OWNER)).serialise().get());
    m_lock.Release();
}

void Channel::Invite(Player* plr, Player* new_player)
{
    m_lock.Acquire();

    if (m_members.find(plr) == m_members.end())
    {
        SendNotOn(plr);
        m_lock.Release();
        return;
    }

    if (m_members.find(new_player) != m_members.end())
    {
        SendAlreadyOn(plr, new_player);
        m_lock.Release();
        return;
    }

    new_player->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_INVITED, m_name, plr->getGuid()).serialise().get());

    plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOU_INVITED, m_name, new_player->getGuid()).serialise().get());
    m_lock.Release();
}

void Channel::Moderate(Player* plr)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('c'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    m_muted = !m_muted;
    SendToAll(SmsgChannelNotify(m_muted ? CHANNEL_NOTIFY_FLAG_MODERATED : CHANNEL_NOTIFY_FLAG_UNMODERATED, m_name, plr->getGuid()).serialise().get());
    m_lock.Release();
}

void Channel::Say(Player* plr, const char* message, Player* for_gm_client, bool forced)
{
    m_lock.Acquire();

    if (!forced)
    {
        MemberMap::iterator itr = m_members.find(plr);
        if (m_members.end() == itr)
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
            m_lock.Release();
            return;
        }

        if (itr->second & CHANNEL_MEMBER_FLAG_MUTED)
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_name).serialise().get());
            m_lock.Release();
            return;
        }

        if (m_muted && !(itr->second & CHANNEL_MEMBER_FLAG_VOICED) && !(itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_name).serialise().get());
            m_lock.Release();
            return;
        }
    }

    // not blizzlike but meh
    if (plr->getLevel() < m_minimumLevel)
    {
        plr->BroadcastMessage("You must be level %u to speak in the channel, '%s'.", m_minimumLevel, m_name.c_str());
        m_lock.Release();
        return;
    }

    WorldPacket data(SMSG_MESSAGECHAT, strlen(message) + 100);
    data << uint8(CHAT_MSG_CHANNEL);
    data << uint32(0);        // language
    data << plr->getGuid();    // guid
    data << uint32(0);        // rank?
    data << m_name;            // channel name
    data << plr->getGuid();    // guid again?
    data << uint32(strlen(message) + 1);
    data << message;
    data << (uint8)(plr->isGMFlagSet() ? 4 : 0);
    if (for_gm_client != nullptr)
        for_gm_client->SendPacket(&data);
    else
        SendToAll(&data);

    m_lock.Release();
}

void Channel::SendNotOn(Player* plr)
{
    plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
}

void Channel::SendAlreadyOn(Player* plr, Player* plr2)
{
    plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_name, plr2->getGuid()).serialise().get());
}

void Channel::Kick(Player* plr, Player* die_player, bool ban)
{
    m_lock.Acquire();

    MemberMap::iterator me_itr = m_members.find(plr);
    if (me_itr == m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr = m_members.find(die_player);
    if (itr == m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, die_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(me_itr->second & CHANNEL_MEMBER_FLAG_OWNER || me_itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 flags = itr->second;

    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_KICKED, m_name, die_player->getGuid()).serialise().get());

    if (ban)
    {
        SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_BANNED, m_name, die_player->getGuid()).serialise().get());
    }

    m_members.erase(itr);

    if (flags & CHANNEL_MEMBER_FLAG_OWNER)
        SetOwner(NULL, NULL);

    if (ban)
        m_bannedMembers.insert(die_player->getGuidLow());

    die_player->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOULEFT, m_name, 0, 0, m_id).serialise().get());
    m_lock.Release();
}

void Channel::Unban(Player* plr, PlayerInfo* bplr)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    std::set<uint32>::iterator it2 = m_bannedMembers.find(bplr->guid);
    if (it2 == m_bannedMembers.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, bplr->guid).serialise().get());
        m_lock.Release();
        return;
    }

    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_UNBANNED, m_name, bplr->guid).serialise().get());
    m_bannedMembers.erase(it2);
    m_lock.Release();
}

void Channel::Voice(Player* plr, Player* v_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(v_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, v_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_VOICED;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, v_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::Devoice(Player* plr, Player* v_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(v_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, v_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_VOICED;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, v_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::Mute(Player* plr, Player* die_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(die_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, die_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MUTED;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, die_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::Unmute(Player* plr, Player* die_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(die_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, die_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MUTED;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, die_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::GiveModerator(Player* plr, Player* new_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(new_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, new_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MODERATOR;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, new_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::TakeModerator(Player* plr, Player* new_player)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    MemberMap::iterator itr2 = m_members.find(new_player);
    if (m_members.end() == itr2)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_name, new_player->getGuid()).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MODERATOR;
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_name, new_player->getGuid(), oldflags, 0, itr2->second).serialise().get());
    m_lock.Release();
}

void Channel::Announce(Player* plr)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    m_announce = !m_announce;
    SendToAll(SmsgChannelNotify(m_announce ? CHANNEL_NOTIFY_FLAG_ENABLE_ANN : CHANNEL_NOTIFY_FLAG_DISABLE_ANN, m_name, plr->getGuid()).serialise().get());
    m_lock.Release();
}

void Channel::Password(Player* plr, const char* pass)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (m_members.end() == itr)
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    if (!(itr->second & CHANNEL_MEMBER_FLAG_OWNER || itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('a'))
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    m_password = std::string(pass);
    SendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_SETPASS, m_name, plr->getGuid()).serialise().get());
    m_lock.Release();
}

void Channel::List(Player* plr)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    std::vector<SmsgChannelListMembers> members;
    for (itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        uint8 flags = 0;
        if (!(itr->second & CHANNEL_MEMBER_FLAG_MUTED))
            flags |= CHANNEL_MEMBER_FLAG_VOICED;

        if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
            flags |= CHANNEL_MEMBER_FLAG_OWNER;

        if (itr->second & CHANNEL_MEMBER_FLAG_MODERATOR)
            flags |= CHANNEL_MEMBER_FLAG_MODERATOR;

        if (!m_general)
            flags |= CHANNEL_MEMBER_FLAG_CUSTOM;

        members.push_back({itr->first->getGuid(), flags});
    }

    plr->SendPacket(SmsgChannelList(m_name, members).serialise().get());
    m_lock.Release();
}

void Channel::GetOwner(Player* plr)
{
    m_lock.Acquire();

    MemberMap::iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_name).serialise().get());
        m_lock.Release();
        return;
    }

    for (itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->second & CHANNEL_MEMBER_FLAG_OWNER)
        {
            plr->SendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_WHO_OWNER, m_name, itr->first->getGuid()).serialise().get());
            m_lock.Release();
            return;
        }
    }
    m_lock.Release();
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
    m_lock.Acquire();
    for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        itr->first->SendPacket(data);

    m_lock.Release();
}

void Channel::SendToAll(WorldPacket* data, Player* plr)
{
    m_lock.Acquire();
    for (MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->first != plr)
            itr->first->SendPacket(data);
    }

    m_lock.Release();
}
