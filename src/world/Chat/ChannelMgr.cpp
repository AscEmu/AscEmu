/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ChannelMgr.hpp"
#include "Channel.hpp"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/Packets/SmsgChannelNotify.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"

#if VERSION_STRING < Cata
#include "Server/World.h"
#endif

using namespace AscEmu::Packets;

ChannelMgr::ChannelMgr() = default;
ChannelMgr::~ChannelMgr() = default;

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
    for (uint8_t i = 0; i < 2; ++i)
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

Channel* ChannelMgr::getOrCreateChannel(std::string name, Player const* player, uint32_t typeId)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    {
        std::lock_guard<std::mutex> configGuard(m_mutexConfig);
        for (const auto& m_bannedChannel : m_bannedChannels)
        {
            if (name == m_bannedChannel)
                return nullptr;
        }
    }

    std::lock_guard<std::mutex> channelGuard(m_mutexChannels);
    const auto teamId = (m_seperateChannels && player) ? player->getTeam() : TEAM_ALLIANCE;

    const auto [channelItr, _] = channelList->try_emplace(name, Util::LazyInstanceCreator([&name, teamId, typeId] {
        return std::make_unique<Channel>(name, teamId, typeId);
    }));
    return channelItr->second.get();
}

void ChannelMgr::removeChannel(Channel const* channel)
{
    if (!channel)
        return;

    auto channelList = &m_channelList[0];
    if (m_seperateChannels)
        channelList = &m_channelList[channel->getChannelTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto channelListMember = channelList->begin(); channelListMember != channelList->end(); ++channelListMember)
    {
        if (channelListMember->second.get() == channel)
        {
            channelList->erase(channelListMember);
            return;
        }
    }
}

Channel* ChannelMgr::getChannel(std::string name, Player const* player) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (const auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second.get();
    }

    return nullptr;
}

Channel* ChannelMgr::getChannel(std::string name, uint32_t team) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[team];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (const auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second.get();
    }

    return nullptr;
}

bool ChannelMgr::canPlayerJoinDefaultChannel(Player const* player, WDB::Structures::AreaTableEntry const* areaEntry, WDB::Structures::ChatChannelsEntry const* channelDbc) const
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

std::string ChannelMgr::generateChannelName(WDB::Structures::ChatChannelsEntry const* channelDbc, WDB::Structures::AreaTableEntry const* areaEntry) const
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
