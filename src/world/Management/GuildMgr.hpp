/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _GUILD_MGR_HPP
#define _GUILD_MGR_HPP

#include "StdAfx.h"

class SERVER_DECL GuildMgr : public Singleton < GuildMgr >
{
    public:

        GuildMgr();
        ~GuildMgr();

    public:

        Guild* GetGuildByLeader(uint64 guid) const;
        Guild* GetGuildById(uint32 guildId) const;
        Guild* GetGuildByName(std::string const& guildName) const;
        std::string GetGuildNameById(uint32 guildId) const;

        void LoadGuildXpForLevel();
        void LoadGuildRewards();

        void LoadGuilds();
        void AddGuild(Guild* guild);
        void RemoveGuild(uint32 guildId);

        void Update(uint32 diff);
        void SaveGuilds();

        void ResetReputationCaps();

        uint32 GenerateGuildId();
        //void SetNextGuildId(uint32 Id) { NextGuildId = Id; }

        uint32 GetXPForGuildLevel(uint8 level) const;
        std::vector<GuildReward> const& GetGuildRewards() const { return GuildRewards; }

        void ResetTimes(bool week);

        /*uint32 GetGuildIdForMember(uint32 member_guid);*/

        uint32 lastSave;
        bool firstSave;

    protected:

        typedef std::unordered_map<uint32, Guild*> GuildContainer;
        //uint32 NextGuildId;
        GuildContainer GuildStore;
        std::vector<uint64> GuildXPperLevel;
        std::vector<GuildReward> GuildRewards;
};

#define sGuildMgr GuildMgr::getSingleton()


#endif // _GUILD_MGR_HPP
