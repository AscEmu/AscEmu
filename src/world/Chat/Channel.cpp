/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/PacketBroadcast.hpp"
#include "Channel.hpp"
#include "ChannelMgr.hpp"
#include "ChatDefines.hpp"
#include "Network/WorldPacket.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgChannelList.h"
#include "Server/Packets/SmsgChannelNotify.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

Channel::Channel(std::string name, uint8_t team, uint32_t channelId/* = 0*/) :
    m_channelName(name), m_channelTeam(team), m_channelId(channelId)
{
    const auto channelDbc = sChatChannelsStore.lookupEntry(channelId);
    if (channelDbc != nullptr)
    {
        // Default channels
        m_channelFlags |= CHANNEL_FLAGS_GENERAL;
        m_announcePlayers = false;

        // Trade channel
        if (channelDbc->flags & CHANNEL_DBC_TRADE)
            m_channelFlags |= CHANNEL_FLAGS_TRADE;

        // Channels that are active in cities
        if (channelDbc->flags & (CHANNEL_DBC_CITY_ONLY_1 | CHANNEL_DBC_CITY_ONLY_2))
            m_channelFlags |= CHANNEL_FLAGS_CITY;

        // LookingForGroup channel
        if (channelDbc->flags & CHANNEL_DBC_LFG)
            m_channelFlags |= CHANNEL_FLAGS_LFG;
        else
            m_channelFlags |= CHANNEL_FLAGS_NOT_LFG;
    }
    else
    {
        // Player custom created channels
        m_channelFlags = CHANNEL_FLAGS_CUSTOM;
        m_channelId = 0;
    }

    for (const auto& channelName : sChannelMgr.m_minimumChannel)
    {
        if (name != channelName)
        {
            m_minimumLevel = 10;
            m_announcePlayers = false;
            break;
        }
    }
}

Channel::~Channel()
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    for (const auto& member : m_members)
        member.first->leftChannel(this);
}

std::string Channel::getChannelName() const { return m_channelName; }
std::string Channel::getChannelPassword() const { return m_channelPassword; }
uint32_t Channel::getChannelId() const { return m_channelId; }
uint8_t Channel::getChannelFlags() const { return m_channelFlags; }
uint8_t Channel::getChannelTeam() const { return m_channelTeam; }

void Channel::setChannelName(std::string name)
{
    // Default channel names cannot be changed
    if (m_channelFlags & CHANNEL_FLAGS_GENERAL)
        return;

    m_channelName = name;
}

void Channel::attemptJoin(Player* plr, std::string password, bool skipCheck/* = false*/)
{
    if (!skipCheck && m_channelFlags & CHANNEL_FLAGS_GENERAL)
    {
        const auto areaEntry = plr->GetArea();

        const auto channelDbc = sChatChannelsStore.lookupEntry(getChannelId());
        if (!sChannelMgr.canPlayerJoinDefaultChannel(plr, areaEntry, channelDbc))
            return;
    }

    uint8_t memberFlags = CHANNEL_MEMBER_FLAG_NONE;

    if (!(m_channelFlags & CHANNEL_FLAGS_GENERAL) && plr->getSession()->CanUseCommand('c'))
        memberFlags |= CHANNEL_MEMBER_FLAG_MODERATOR;

    if (!m_channelPassword.empty() && strcmp(m_channelPassword.c_str(), password.c_str()) != 0)
    {
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_WRONGPASS, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        return;
    }

    m_mutexChannel.lock();

    if (m_bannedMembers.find(plr->getGuidLow()) != m_bannedMembers.end())
    {
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOURBANNED, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (m_members.find(plr) != m_members.end())
    {
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (m_members.empty() && !(m_channelFlags & CHANNEL_FLAGS_GENERAL))
        memberFlags |= CHANNEL_MEMBER_FLAG_OWNER;

    m_members.insert(std::make_pair(plr, memberFlags));
    m_mutexChannel.unlock();

    plr->joinedChannel(this);

    // Announce player join to other members in channel
    if (m_announcePlayers)
    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_JOINED, m_channelName, plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOUJOINED, m_channelName, 0, m_channelFlags, m_channelId);
        if (auto* targetSession = plr->getSession())
            targetSession->sendManagedPacket(sendPacket);
    }

#if VERSION_STRING == Mop
    WorldPacket data(m_channelId != 0 ? SMSG_USERLIST_ADD : SMSG_USERLIST_UPDATE, 8 + 1 + 1 + 4 + m_channelName.size());
    WoWGuid guid = plr->getGuid();
    if (m_channelId != 0)
    {
        data << uint32_t(m_channelId);
        data << uint8_t(m_channelFlags);
        data << uint8_t(memberFlags);
        data.writeBit(guid[7]);
        data.writeBits(m_channelName.size(), 7);
        data.writeBit(guid[0]);
        data.writeBit(guid[5]);
        data.writeBit(guid[4]);
        data.writeBit(guid[6]);
        data.writeBit(guid[1]);
        data.writeBit(guid[3]);
        data.writeBit(guid[2]);

        data.flushBits();
        data.writeByteSeq(guid[4]);
        data.writeByteSeq(guid[5]);
        data.writeByteSeq(guid[7]);
        data.writeByteSeq(guid[1]);
        data.writeByteSeq(guid[2]);
        data.writeByteSeq(guid[3]);
        data.writeByteSeq(guid[6]);
        data.writeByteSeq(guid[0]);
        data << m_channelName;
        sendToAll(&data, plr);
    }
    else
    {
        data.writeBit(guid[2]);
        data.writeBit(guid[6]);
        data.writeBit(guid[3]);
        data.writeBit(guid[7]);
        data.writeBit(guid[5]);
        data.writeBit(guid[1]);
        data.writeBit(guid[0]);
        data.writeBits(m_channelName.size(), 7);
        data.writeBit(guid[4]);

        data.flushBits();
        data.writeByteSeq(guid[0]);
        data.writeByteSeq(guid[2]);
        data.writeByteSeq(guid[6]);
        data.writeByteSeq(guid[5]);
        data << uint8_t(m_channelFlags);
        data.writeByteSeq(guid[7]);
        data.writeByteSeq(guid[3]);
        data << uint32_t(m_channelId);
        data << m_channelName;
        data.writeByteSeq(guid[1]);
        data.writeByteSeq(guid[4]);
        data << uint8_t(memberFlags);
        sendToAll(&data);
    }
#endif
}

void Channel::leaveChannel(Player* plr, bool sendPacket/* = true*/)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on this channel
        {
            SmsgChannelNotify notifyPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(notifyPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto memberFlags = itr->second;
    m_members.erase(itr);
    m_mutexChannel.unlock();

    plr->leftChannel(this);

    // If player is channel owner, find new owner for channel
    if (memberFlags & CHANNEL_MEMBER_FLAG_OWNER)
        setOwner(nullptr, nullptr);

    // Do not send packet in teleport or logout
    if (sendPacket && !(plr->getSession() && (plr->getSession()->IsLoggingOut() || plr->getTeleportState() == 1)))
        {
            SmsgChannelNotify notifyPacket(CHANNEL_NOTIFY_FLAG_YOULEFT, m_channelName, 0, 0, m_channelId);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(notifyPacket);
        }

    // Announce player leave to other members in channel
    if (m_announcePlayers)
        {
            SmsgChannelNotify notifyPacket(CHANNEL_NOTIFY_FLAG_LEFT, m_channelName, plr->getGuid());
            PacketBroadcast::sendFromChannel(*this, notifyPacket);
        }

    // If channel is now empty, delete it
    if (m_members.empty())
        sChannelMgr.removeChannel(this);
}

size_t Channel::getMemberCount() const
{
    return m_members.size();
}

bool Channel::hasMember(Player* plr) const
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);
    return m_members.find(plr) != m_members.end();
}

void Channel::say(Player* plr, std::string message, Player* for_gm_client, bool forced)
{
    if (!forced)
    {
        std::lock_guard<std::mutex> guard(m_mutexChannel);

        const MemberMap::const_iterator itr = m_members.find(plr);
        if (itr == m_members.end())
        {
            // Player is not on channel
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }

        if (itr->second & CHANNEL_MEMBER_FLAG_MUTED)
        {
            // Player is muted
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_channelName);
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }

        if (m_muted && !(itr->second & CHANNEL_MEMBER_FLAG_VOICED) && !(itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_channelName);
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }
    }

    if (plr->getLevel() < m_minimumLevel)
    {
        plr->broadcastMessage("You must be level %u to speak in the channel, '%s'.", m_minimumLevel, m_channelName.c_str());
        return;
    }

    // Send message
    const uint8_t gmFlag = plr->isGMFlagSet() ? 4U : 0U;
    if (for_gm_client != nullptr)
    {
        SmsgMessageChat messagePacket(CHAT_MSG_CHANNEL, LANG_UNIVERSAL, gmFlag, message, plr->getGuid(), "", 0, m_channelName);
        for_gm_client->getSession()->sendManagedPacket(messagePacket);
    }
    else
    {
        SmsgMessageChat messagePacket(CHAT_MSG_CHANNEL, LANG_UNIVERSAL, gmFlag, message, plr->getGuid(), plr->getName(), 0, m_channelName);

        std::lock_guard<std::mutex> guard(m_mutexChannel);

        for (auto& member : m_members)
            if (member.first->getSession())
                member.first->getSession()->sendManagedPacket(messagePacket);
    }
}

void Channel::invitePlayer(Player* plr, Player* new_player)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    if (m_members.find(plr) == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        return;
    }

    if (m_members.find(new_player) != m_members.end())
    {
        // Invited player is already on the channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_channelName, new_player->getGuid());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        return;
    }

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_INVITED, m_channelName, plr->getGuid());
        if (auto* targetSession = new_player->getSession())
            targetSession->sendManagedPacket(sendPacket);
    }
    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOU_INVITED, m_channelName, 0, 0, 0, 0, new_player->getName());
        if (auto* targetSession = plr->getSession())
            targetSession->sendManagedPacket(sendPacket);
    }
}

void Channel::kickOrBanPlayer(Player* plr, Player* die_player, bool ban)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator me_itr = m_members.find(plr);
    if (me_itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::const_iterator itr = m_members.find(die_player);
    if (itr == m_members.end())
    {
        // Kicked player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(me_itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_KICKED, m_channelName, die_player->getGuid(), 0, 0, 0, std::string(), plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }

    if (ban)
    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_BANNED, m_channelName, die_player->getGuid(), 0, 0, 0, std::string(), plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }

    m_mutexChannel.lock();

    const auto memberFlags = itr->second;
    m_members.erase(itr);
    if (ban)
        m_bannedMembers.insert(die_player->getGuidLow());

    m_mutexChannel.unlock();

    // Find new owner for channel
    if (memberFlags & CHANNEL_MEMBER_FLAG_OWNER)
        setOwner(nullptr, nullptr);

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_YOULEFT, m_channelName, 0, 0, m_channelId);
        if (auto* targetSession = die_player->getSession())
            targetSession->sendManagedPacket(sendPacket);
    }
}

void Channel::unBanPlayer(Player* plr, CachedCharacterInfo const* bplr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const std::set<uint32_t>::const_iterator it2 = m_bannedMembers.find(bplr->guid);
    if (it2 == m_bannedMembers.end())
    {
        // Player is not banned
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, bplr->name);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    m_bannedMembers.erase(it2);
    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_UNBANNED, m_channelName, bplr->guid, 0, 0, 0, std::string(), plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::moderateChannel(Player* plr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('c'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_muted = !m_muted;

    {
        SmsgChannelNotify sendPacket(m_muted ? CHANNEL_NOTIFY_FLAG_MODERATED : CHANNEL_NOTIFY_FLAG_UNMODERATED, m_channelName, plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::giveModerator(Player* plr, Player* new_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(new_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, new_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MODERATOR;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, new_player->getGuid(), oldMemberflags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::takeModerator(Player* plr, Player* new_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(new_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, new_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MODERATOR;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, new_player->getGuid(), oldMemberFlags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::mutePlayer(Player* plr, Player* die_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(die_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MUTED;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, die_player->getGuid(), oldMemberFlags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::unMutePlayer(Player* plr, Player* die_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(die_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MUTED;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, die_player->getGuid(), oldMemberFlags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::giveVoice(Player* plr, Player* v_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(v_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, v_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_VOICED;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, v_player->getGuid(), oldMemberFlags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::takeVoice(Player* plr, Player* v_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(v_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, v_player->getName());
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_VOICED;

    m_mutexChannel.unlock();

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, v_player->getGuid(), oldMemberFlags, 0, itr2->second);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::setPassword(Player* plr, std::string pass)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_channelPassword = pass;

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_SETPASS, m_channelName, plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::enableAnnouncements(Player* plr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_announcePlayers = !m_announcePlayers;

    {
        SmsgChannelNotify sendPacket(m_announcePlayers ? CHANNEL_NOTIFY_FLAG_ENABLE_ANN : CHANNEL_NOTIFY_FLAG_DISABLE_ANN, m_channelName, plr->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::getOwner(Player* plr)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        return;
    }

    for (const auto& member : m_members)
    {
        if (member.second & CHANNEL_MEMBER_FLAG_OWNER)
        {
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_WHO_OWNER, m_channelName, 0, 0, 0, 0, member.first->getName());
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }
    }
}

void Channel::setOwner(Player* plr, Player const* newOwner)
{
    // Here both player and new owner can be nullptr
    if (plr != nullptr)
    {
        std::lock_guard<std::mutex> guard(m_mutexChannel);

        const MemberMap::const_iterator itr = m_members.find(plr);
        if (itr == m_members.end())
        {
            // Player is not on channel
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }

        if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
        {
            // Player is not a moderator
            {
                SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName);
                if (auto* targetSession = plr->getSession())
                    targetSession->sendManagedPacket(sendPacket);
            }
            return;
        }
    }

    Player const* owner = nullptr;
    uint8_t oldMemberFlags = CHANNEL_MEMBER_FLAG_NONE;

    Player const* oldOwner = nullptr;
    uint8_t oldOwnerFlags = CHANNEL_MEMBER_FLAG_NONE;

    if (newOwner == nullptr)
    {
        std::lock_guard<std::mutex> guard(m_mutexChannel);

        // Find new random owner
        for (auto& member : m_members)
        {
            if (member.second & CHANNEL_MEMBER_FLAG_OWNER)
            {
                // Remove the old owner
                oldOwner = member.first;
                oldOwnerFlags = member.second;
                member.second &= ~CHANNEL_MEMBER_FLAG_OWNER;
            }
            else if (owner == nullptr)
            {
                owner = member.first;
                oldMemberFlags = member.second;
                member.second |= CHANNEL_MEMBER_FLAG_OWNER;
            }
        }
    }
    else
    {
        std::lock_guard<std::mutex> guard(m_mutexChannel);

        // Set newOwner to owner
        for (auto& member : m_members)
        {
            if (member.second & CHANNEL_MEMBER_FLAG_OWNER)
            {
                // Remove the old owner
                oldOwner = member.first;
                oldOwnerFlags = member.second;
                member.second &= ~CHANNEL_MEMBER_FLAG_OWNER;
            }
            else
            {
                if (newOwner == member.first)
                {
                    owner = member.first;
                    oldMemberFlags = member.second;
                    member.second |= CHANNEL_MEMBER_FLAG_OWNER;
                }
            }
        }
    }

    if (oldOwner != nullptr)
    {
        uint8_t const flagsBefore = oldOwnerFlags;
        oldOwnerFlags &= ~CHANNEL_MEMBER_FLAG_OWNER;
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, oldOwner->getGuid(), flagsBefore, 0, oldOwnerFlags);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }

    // Channel possibly empty
    if (owner == nullptr)
        return;

    {
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_CHGOWNER, m_channelName, owner->getGuid());
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }

    // Send the mode change
    {
        const auto previousFlags = oldMemberFlags;
        oldMemberFlags |= CHANNEL_MEMBER_FLAG_OWNER;
        SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, owner->getGuid(), previousFlags, 0, oldMemberFlags);
        PacketBroadcast::sendFromChannel(*this, sendPacket);
    }
}

void Channel::listMembers(Player* plr, bool chatQuery)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        {
            SmsgChannelNotify sendPacket(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName);
            if (auto* targetSession = plr->getSession())
                targetSession->sendManagedPacket(sendPacket);
        }
        return;
    }

    const auto isPlayerGm = plr->getSession()->CanUseCommand('a');
    std::vector<SmsgChannelListMembers> members;
    for (const auto& member : m_members)
    {
        if (member.first->getSession() == nullptr)
            continue;

        if (!isPlayerGm)
        {
            // Players should not be able to see GMs in chat lists
            if (member.first->getSession()->CanUseCommand('a'))
                continue;
        }

        uint8_t memberFlags = CHANNEL_MEMBER_FLAG_NONE;
        if (!(member.second & CHANNEL_MEMBER_FLAG_MUTED))
            memberFlags |= CHANNEL_MEMBER_FLAG_VOICED;

        if (member.second & CHANNEL_MEMBER_FLAG_OWNER)
            memberFlags |= CHANNEL_MEMBER_FLAG_OWNER;

        if (member.second & CHANNEL_MEMBER_FLAG_MODERATOR)
            memberFlags |= CHANNEL_MEMBER_FLAG_MODERATOR;

        if (!(m_channelFlags & CHANNEL_FLAGS_GENERAL))
            memberFlags |= CHANNEL_MEMBER_FLAG_CUSTOM;

        members.push_back({ member.first->getGuid(), memberFlags });
    }

    {
        SmsgChannelList sendPacket(chatQuery, m_channelName, m_channelFlags, members);
        if (auto* targetSession = plr->getSession())
            targetSession->sendManagedPacket(sendPacket);
    }
}

void Channel::sendToAll(WorldPacket* data)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    for (auto& member : m_members)
    {
        member.first->sendPacket(data);
    }
}

void Channel::sendToAll(WorldPacket* data, Player* skipPlayer)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    for (auto& member : m_members)
    {
        if (member.first != skipPlayer)
            member.first->sendPacket(data);
    }
}
