/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class SERVER_DECL ChannelMgr
{

    ChannelMgr() = default;
    ~ChannelMgr() = default;

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

    Channel* getOrCreateChannel(std::string name, Player* player, uint32_t typeId);
    void removeChannel(Channel* channel);

    Channel* getChannel(std::string name, Player* player);
    Channel* getChannel(std::string name, uint32_t team);

    Mutex m_confSettingLock;
    std::vector<std::string> m_bannedChannels;
    std::vector<std::string> m_minimumChannel;

private:

    typedef std::map<std::string, Channel*> ChannelList;
    ChannelList m_channelList[2];

    bool m_seperateChannels;

    Mutex m_lock;
};

#define sChannelMgr ChannelMgr::getInstance()
