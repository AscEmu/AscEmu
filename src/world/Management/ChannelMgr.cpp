/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Units/Players/Player.h"
#include "Server/Packets/SmsgChannelNotify.h"

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
    {
        for (auto& channelList : this->m_channelList[i])
            delete channelList.second;

        m_channelList[i].clear();
    }
}

void ChannelMgr::loadConfigSettings()
{
    auto bannedChannels = worldConfig.chat.bannedChannels;
    auto minimumLevel = worldConfig.chat.minimumTalkLevel;

    m_confSettingLock.Acquire();

    m_bannedChannels = Util::SplitStringBySeperator(bannedChannels, ";");
    m_minimumChannel = Util::SplitStringBySeperator(minimumLevel, ";");

    m_confSettingLock.Release();
}

void ChannelMgr::setSeperatedChannels(bool enabled)
{
    m_seperateChannels = enabled;
}

Channel* ChannelMgr::getOrCreateChannel(std::string name, Player* player, uint32_t typeId)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    m_lock.Acquire();

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
        {
            m_lock.Release();

            return channelListMember.second;
        }
    }

    m_confSettingLock.Acquire();

    for (auto& m_bannedChannel : m_bannedChannels)
    {
        if (name == m_bannedChannel)
        {
            m_lock.Release();

            m_confSettingLock.Release();

            return nullptr;
        }
    }

    auto channel = new Channel(name.c_str(), (m_seperateChannels && player) ? player->getTeam() : TEAM_ALLIANCE, typeId);

    m_confSettingLock.Release();

    channelList->insert(make_pair(channel->m_name, channel));

    m_lock.Release();

    return channel;
}

void ChannelMgr::removeChannel(Channel* channel)
{
    if (!channel)
        return;

    auto channelList = &m_channelList[0];
    if (m_seperateChannels)
        channelList = &m_channelList[channel->m_team];

    m_lock.Acquire();

    for (auto channelListMember = channelList->begin(); channelListMember != channelList->end(); ++channelListMember)
    {
        if (channelListMember->second == channel)
        {
            channelList->erase(channelListMember);
            delete channel;

            m_lock.Release();

            return;
        }
    }

    m_lock.Release();
}

Channel* ChannelMgr::getChannel(std::string name, Player* player)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    m_lock.Acquire();

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
        {
            m_lock.Release();
            return channelListMember.second;
        }
    }

    m_lock.Release();

    return nullptr;
}

Channel* ChannelMgr::getChannel(std::string name, uint32_t team)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[team];

    m_lock.Acquire();

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
        {
            m_lock.Release();
            return channelListMember.second;
        }
    }

    m_lock.Release();

    return nullptr;
}
