/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
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
#include "Server/Packets/SmsgChannelNotify.h"

using namespace AscEmu::Packets;

ChannelMgr& ChannelMgr::getInstance()
{
    static ChannelMgr mInstance;
    return mInstance;
}

void ChannelMgr::finalize()
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

void ChannelMgr::loadConfigSettings()
{
    std::string BannedChannels = worldConfig.chat.bannedChannels;
    std::string MinimumLevel = worldConfig.chat.minimumTalkLevel;

    m_confSettingLock.Acquire();
    m_bannedChannels = Util::SplitStringBySeperator(BannedChannels, ";");
    m_minimumChannel = Util::SplitStringBySeperator(MinimumLevel, ";");
    m_confSettingLock.Release();
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

void ChannelMgr::initialize()
{
    seperatechannels = false;
}
