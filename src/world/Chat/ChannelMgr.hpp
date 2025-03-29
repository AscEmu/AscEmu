/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <map>
#include <mutex>
#include <vector>

class Player;
class Channel;

namespace WDB::Structures
{
    struct ChatChannelsEntry;
    struct AreaTableEntry;
}

class SERVER_DECL ChannelMgr
{
private:
    ChannelMgr();
    ~ChannelMgr();

public:
    ChannelMgr(ChannelMgr&&) = delete;
    ChannelMgr(ChannelMgr const&) = delete;
    ChannelMgr& operator=(ChannelMgr&&) = delete;
    ChannelMgr& operator=(ChannelMgr const&) = delete;

    static ChannelMgr& getInstance();

    void initialize();
    void finalize();

    void loadConfigSettings();
    void setSeperatedChannels(bool enabled);

    Channel* getOrCreateChannel(std::string name, Player const* player, uint32_t typeId);
    void removeChannel(Channel const* channel);

    Channel* getChannel(std::string name, Player const* player) const;
    Channel* getChannel(std::string name, uint32_t team) const;

    bool canPlayerJoinDefaultChannel(Player const* player, WDB::Structures::AreaTableEntry const* areaEntry, WDB::Structures::ChatChannelsEntry const* channelDbc) const;
    std::string generateChannelName(WDB::Structures::ChatChannelsEntry const* channelDbc, WDB::Structures::AreaTableEntry const* areaEntry) const;

    std::vector<std::string> m_bannedChannels;
    std::vector<std::string> m_minimumChannel;

private:
    typedef std::map<std::string, std::unique_ptr<Channel>> ChannelList;
    ChannelList m_channelList[2];

    bool m_seperateChannels = false;

    mutable std::mutex m_mutexConfig;
    mutable std::mutex m_mutexChannels;
};

#define sChannelMgr ChannelMgr::getInstance()
