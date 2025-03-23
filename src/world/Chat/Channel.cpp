/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Channel.hpp"
#include "ChannelMgr.hpp"
#include "ChatDefines.hpp"
#include "WorldPacket.h"
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
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_WRONGPASS, m_channelName).serialise().get());
        return;
    }

    m_mutexChannel.lock();

    if (m_bannedMembers.find(plr->getGuidLow()) != m_bannedMembers.end())
    {
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOURBANNED, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (m_members.find(plr) != m_members.end())
    {
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_channelName).serialise().get());
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
        sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_JOINED, m_channelName, plr->getGuid()).serialise().get(), nullptr);

    plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUJOINED, m_channelName, 0, m_channelFlags, m_channelId).serialise().get());
}

void Channel::leaveChannel(Player* plr, bool sendPacket/* = true*/)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on this channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
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
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOULEFT, m_channelName, 0, 0, m_channelId).serialise().get());

    // Announce player leave to other members in channel
    if (m_announcePlayers)
        sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_LEFT, m_channelName, plr->getGuid()).serialise().get());

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
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
            return;
        }

        if (itr->second & CHANNEL_MEMBER_FLAG_MUTED)
        {
            // Player is muted
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_channelName).serialise().get());
            return;
        }

        if (m_muted && !(itr->second & CHANNEL_MEMBER_FLAG_VOICED) && !(itr->second & CHANNEL_MEMBER_FLAG_MODERATOR) && !(itr->second & CHANNEL_MEMBER_FLAG_OWNER))
        {
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK, m_channelName).serialise().get());
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
        for_gm_client->sendPacket(SmsgMessageChat(CHAT_MSG_CHANNEL, LANG_UNIVERSAL, gmFlag, message, plr->getGuid(), "", 0, m_channelName).serialise().get());
    else
        sendToAll(SmsgMessageChat(CHAT_MSG_CHANNEL, LANG_UNIVERSAL, gmFlag, message, plr->getGuid(), "", 0, m_channelName).serialise().get());
}

void Channel::invitePlayer(Player* plr, Player* new_player)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    if (m_members.find(plr) == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        return;
    }

    if (m_members.find(new_player) != m_members.end())
    {
        // Invited player is already on the channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_ALREADY_ON, m_channelName, new_player->getGuid()).serialise().get());
        return;
    }

    new_player->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_INVITED, m_channelName, plr->getGuid()).serialise().get());
    plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOU_INVITED, m_channelName, 0, 0, 0, 0, new_player->getName()).serialise().get());
}

void Channel::kickOrBanPlayer(Player* plr, Player* die_player, bool ban)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator me_itr = m_members.find(plr);
    if (me_itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::const_iterator itr = m_members.find(die_player);
    if (itr == m_members.end())
    {
        // Kicked player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(me_itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_KICKED, m_channelName, die_player->getGuid(), 0, 0, 0, std::string(), plr->getGuid()).serialise().get());

    if (ban)
        sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_BANNED, m_channelName, die_player->getGuid(), 0, 0, 0, std::string(), plr->getGuid()).serialise().get());

    m_mutexChannel.lock();

    const auto memberFlags = itr->second;
    m_members.erase(itr);
    if (ban)
        m_bannedMembers.insert(die_player->getGuidLow());

    m_mutexChannel.unlock();

    // Find new owner for channel
    if (memberFlags & CHANNEL_MEMBER_FLAG_OWNER)
        setOwner(nullptr, nullptr);

    die_player->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_YOULEFT, m_channelName, 0, 0, m_channelId).serialise().get());
}

void Channel::unBanPlayer(Player* plr, CachedCharacterInfo const* bplr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const std::set<uint32_t>::const_iterator it2 = m_bannedMembers.find(bplr->guid);
    if (it2 == m_bannedMembers.end())
    {
        // Player is not banned
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, bplr->name).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    m_bannedMembers.erase(it2);
    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_UNBANNED, m_channelName, bplr->guid, 0, 0, 0, std::string(), plr->getGuid()).serialise().get());
}

void Channel::moderateChannel(Player* plr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('c'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_muted = !m_muted;

    sendToAll(SmsgChannelNotify(m_muted ? CHANNEL_NOTIFY_FLAG_MODERATED : CHANNEL_NOTIFY_FLAG_UNMODERATED, m_channelName, plr->getGuid()).serialise().get());
}

void Channel::giveModerator(Player* plr, Player* new_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(new_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, new_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberflags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MODERATOR;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, new_player->getGuid(), oldMemberflags, 0, itr2->second).serialise().get());
}

void Channel::takeModerator(Player* plr, Player* new_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(new_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, new_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MODERATOR;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, new_player->getGuid(), oldMemberFlags, 0, itr2->second).serialise().get());
}

void Channel::mutePlayer(Player* plr, Player* die_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(die_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_MUTED;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, die_player->getGuid(), oldMemberFlags, 0, itr2->second).serialise().get());
}

void Channel::unMutePlayer(Player* plr, Player* die_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(die_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, die_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_MUTED;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, die_player->getGuid(), oldMemberFlags, 0, itr2->second).serialise().get());
}

void Channel::giveVoice(Player* plr, Player* v_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(v_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, v_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second |= CHANNEL_MEMBER_FLAG_VOICED;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, v_player->getGuid(), oldMemberFlags, 0, itr2->second).serialise().get());
}

void Channel::takeVoice(Player* plr, Player* v_player)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const MemberMap::iterator itr2 = m_members.find(v_player);
    if (itr2 == m_members.end())
    {
        // Target player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOT_ON_2, m_channelName, 0, 0, 0, 0, v_player->getName()).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    const auto oldMemberFlags = itr2->second;
    itr2->second &= ~CHANNEL_MEMBER_FLAG_VOICED;

    m_mutexChannel.unlock();

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, v_player->getGuid(), oldMemberFlags, 0, itr2->second).serialise().get());
}

void Channel::setPassword(Player* plr, std::string pass)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_channelPassword = pass;

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_SETPASS, m_channelName, plr->getGuid()).serialise().get());
}

void Channel::enableAnnouncements(Player* plr)
{
    m_mutexChannel.lock();

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
    {
        // Player is not a moderator
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
        m_mutexChannel.unlock();
        return;
    }

    m_mutexChannel.unlock();
    m_announcePlayers = !m_announcePlayers;

    sendToAll(SmsgChannelNotify(m_announcePlayers ? CHANNEL_NOTIFY_FLAG_ENABLE_ANN : CHANNEL_NOTIFY_FLAG_DISABLE_ANN, m_channelName, plr->getGuid()).serialise().get());
}

void Channel::getOwner(Player* plr)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
        return;
    }

    for (const auto& member : m_members)
    {
        if (member.second & CHANNEL_MEMBER_FLAG_OWNER)
        {
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_WHO_OWNER, m_channelName, 0, 0, 0, 0, member.first->getName()).serialise().get());
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
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
            return;
        }

        if (!(itr->second & (CHANNEL_MEMBER_FLAG_OWNER | CHANNEL_MEMBER_FLAG_MODERATOR)) && !plr->getSession()->CanUseCommand('a'))
        {
            // Player is not a moderator
            plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTMOD, m_channelName).serialise().get());
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
        sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, oldOwner->getGuid(), oldOwnerFlags, 0, (oldOwnerFlags &= ~CHANNEL_MEMBER_FLAG_OWNER)).serialise().get());

    // Channel possibly empty
    if (owner == nullptr)
        return;

    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_CHGOWNER, m_channelName, owner->getGuid()).serialise().get());

    // Send the mode change
    sendToAll(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_MODE_CHG, m_channelName, owner->getGuid(), oldMemberFlags, 0, (oldMemberFlags |= CHANNEL_MEMBER_FLAG_OWNER)).serialise().get());
}

void Channel::listMembers(Player* plr, bool chatQuery)
{
    std::lock_guard<std::mutex> guard(m_mutexChannel);

    const MemberMap::const_iterator itr = m_members.find(plr);
    if (itr == m_members.end())
    {
        // Player is not on channel
        plr->sendPacket(SmsgChannelNotify(CHANNEL_NOTIFY_FLAG_NOTON, m_channelName).serialise().get());
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

    plr->sendPacket(SmsgChannelList(chatQuery, m_channelName, m_channelFlags, members).serialise().get());
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
