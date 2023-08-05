/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ChannelMgr.hpp"
#include "Channel.hpp"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/SmsgChannelNotify.h"
#include "Utilities/Strings.hpp"

using namespace AscEmu::Packets;

ChannelMgr& ChannelMgr::getInstance()
{
    static ChannelMgr mInstance;
    return mInstance;
}

void ChannelMgr::initialize()
{
    m_seperateChannels = false;
}

void ChannelMgr::finalize()
{
    for (uint8 i = 0; i < 2; ++i)
        m_channelList[i].clear();
}

void ChannelMgr::loadConfigSettings()
{
    const auto bannedChannels = worldConfig.chat.bannedChannels;
    const auto minimumLevel = worldConfig.chat.minimumTalkLevel;

    std::lock_guard<std::mutex> guard(m_mutexConfig);

    m_bannedChannels = AscEmu::Util::Strings::split(bannedChannels, ";");
    m_minimumChannel = AscEmu::Util::Strings::split(minimumLevel, ";");
}

void ChannelMgr::setSeperatedChannels(bool enabled)
{
    m_seperateChannels = enabled;
}

std::shared_ptr<Channel> ChannelMgr::getOrCreateChannel(std::string name, Player const* player, uint32_t typeId)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    std::lock_guard<std::mutex> channelGuard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    std::lock_guard<std::mutex> configGuard(m_mutexConfig);

    for (auto& m_bannedChannel : m_bannedChannels)
    {
        if (name == m_bannedChannel)
            return nullptr;
    }

    auto channel = std::make_shared<Channel>(name, (m_seperateChannels && player) ? player->getTeam() : TEAM_ALLIANCE, typeId);

    channelList->insert(make_pair(channel->getChannelName(), channel));

    return channel;
}

void ChannelMgr::removeChannel(std::shared_ptr<Channel> channel)
{
    if (!channel)
        return;

    auto channelList = &m_channelList[0];
    if (m_seperateChannels)
        channelList = &m_channelList[channel->getChannelTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto channelListMember = channelList->begin(); channelListMember != channelList->end(); ++channelListMember)
    {
        if (channelListMember->second == channel)
        {
            channelList->erase(channelListMember);
            return;
        }
    }
}

std::shared_ptr<Channel> ChannelMgr::getChannel(std::string name, Player const* player) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    return nullptr;
}

std::shared_ptr<Channel> ChannelMgr::getChannel(std::string name, uint32_t team) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[team];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    return nullptr;
}

bool ChannelMgr::canPlayerJoinDefaultChannel(Player const* player, DBC::Structures::AreaTableEntry const* areaEntry, DBC::Structures::ChatChannelsEntry const* channelDbc) const
{
    if (player == nullptr || channelDbc == nullptr)
        return false;

    // City specific channels
    if (channelDbc->flags & (CHANNEL_DBC_CITY_ONLY_1 | CHANNEL_DBC_CITY_ONLY_2))
    {
        if (areaEntry == nullptr || !(areaEntry->flags & MapManagement::AreaManagement::AREA_CITY_AREA))
            return false;
    }

    // Channels with zone name
    if (channelDbc->flags & CHANNEL_DBC_HAS_ZONENAME)
    {
        if (areaEntry == nullptr)
            return false;
    }

    // Guild recruitment channel
    if (channelDbc->flags & CHANNEL_DBC_GUILD_RECRUIT)
    {
        if (player->getGuild() != nullptr)
            return false;
    }

    return true;
}

std::string ChannelMgr::generateChannelName(DBC::Structures::ChatChannelsEntry const* channelDbc, DBC::Structures::AreaTableEntry const* areaEntry) const
{
#if VERSION_STRING < Cata
    char* channelNameDbc = channelDbc->name_pattern[sWorld.getDbcLocaleLanguageId()];
#else
    char* channelNameDbc = channelDbc->name_pattern[0];
#endif

    if (channelDbc->flags & CHANNEL_DBC_GLOBAL || !(channelDbc->flags & CHANNEL_DBC_HAS_ZONENAME))
        return std::string(channelNameDbc);

    char channelName[95];
    char const* defaultAreaName = "City";

    if (const auto defaultArea = MapManagement::AreaManagement::AreaStorage::GetAreaById(3459))
    {
#if VERSION_STRING < Cata
        defaultAreaName = defaultArea->area_name[sWorld.getDbcLocaleLanguageId()];
#else
        defaultAreaName = defaultArea->area_name[0];
#endif
    }

    // City specific channels
    if (channelDbc->flags & (CHANNEL_DBC_CITY_ONLY_1 | CHANNEL_DBC_CITY_ONLY_2))
    {
        std::snprintf(channelName, 95, channelNameDbc, defaultAreaName);
    }
    else
    {
        if (areaEntry != nullptr)
        {
#if VERSION_STRING < Cata
            std::snprintf(channelName, 95, channelNameDbc, areaEntry->area_name[sWorld.getDbcLocaleLanguageId()]);
#else
            std::snprintf(channelName, 95, channelNameDbc, areaEntry->area_name);
#endif
        }
        else
        {
            std::snprintf(channelName, 95, channelNameDbc, defaultAreaName);
        }
    }

    return std::string(channelName);
}
