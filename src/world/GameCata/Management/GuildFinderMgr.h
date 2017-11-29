/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Players/PlayerDefines.hpp"
#include "Singleton.h"
#include "Guild.h"
#include <string>
#include <map>
#include <vector>

enum GuildFinderOptionsInterest
{
    INTEREST_QUESTING        = 0x01,
    INTEREST_DUNGEONS        = 0x02,
    INTEREST_RAIDS           = 0x04,
    INTEREST_PVP             = 0x08,
    INTEREST_ROLE_PLAYING    = 0x10,
    ALL_INTERESTS            = INTEREST_QUESTING | INTEREST_DUNGEONS | INTEREST_RAIDS | INTEREST_PVP | INTEREST_ROLE_PLAYING
};

enum GuildFinderOptionsAvailability
{
    AVAILABILITY_WEEKDAYS     = 0x1,
    AVAILABILITY_WEEKENDS     = 0x2,
    AVAILABILITY_ALWAYS       = AVAILABILITY_WEEKDAYS | AVAILABILITY_WEEKENDS
};

enum GuildFinderOptionsRoles
{
    GUILDFINDER_ROLE_TANK        = 0x1,
    GUILDFINDER_ROLE_HEALER      = 0x2,
    GUILDFINDER_ROLE_DPS         = 0x4,
    GUILDFINDER_ALL_ROLES        = GUILDFINDER_ROLE_TANK | GUILDFINDER_ROLE_HEALER | GUILDFINDER_ROLE_DPS
};

enum GuildFinderOptionsLevel
{
    ANY_FINDER_LEVEL       = 0x1,
    MAX_FINDER_LEVEL       = 0x2,
    ALL_GUILDFINDER_LEVELS = ANY_FINDER_LEVEL | MAX_FINDER_LEVEL
};

struct MembershipRequest
{
    public:

        MembershipRequest(MembershipRequest const& settings) : mComment(settings.getComment())
        {
            mAvailability = settings.getAvailability();
            mClassRoles = settings.getClassRoles();
            mInterests = settings.getInterests();
            mGuildId = settings.getGuildId();
            mPlayerGUID = settings.getPlayerGUID();
            mTime = settings.getSubmitTime();
        }

        MembershipRequest(uint32_t playerGUID, uint32_t guildId, uint32_t availability, uint32_t classRoles, uint32_t interests, std::string& comment, time_t submitTime) :
            mComment(comment), mGuildId(guildId), mPlayerGUID(playerGUID), mAvailability(static_cast<uint8_t>(availability)),
            mClassRoles(static_cast<uint8_t>(classRoles)), mInterests(static_cast<uint8_t>(interests)), mTime(submitTime)  {}

        MembershipRequest() : mGuildId(0), mPlayerGUID(0), mAvailability(0), mClassRoles(0),
            mInterests(0), mTime(time(nullptr)) {}

        uint32_t getGuildId() const { return mGuildId; }
        uint32_t getPlayerGUID() const { return mPlayerGUID; }
        uint8_t getAvailability() const { return mAvailability; }
        uint8_t getClassRoles() const { return mClassRoles; }
        uint8_t getInterests() const { return mInterests; }
        time_t getSubmitTime() const { return mTime; }
        time_t getExpiryTime() const { return time_t(mTime + 30 * 24 * 3600); }
        std::string const& getComment() const { return mComment; }

    private:

        std::string mComment;

        uint32_t mGuildId;
        uint32_t mPlayerGUID;

        uint8_t mAvailability;
        uint8_t mClassRoles;
        uint8_t mInterests;

        time_t mTime;
};

struct LFGuildPlayer
{
    public:

        LFGuildPlayer()
        {
            mGuid = 0;
            mRoles = 0;
            mAvailability = 0;
            mInterests = 0;
            mLevel = 0;
        }

        LFGuildPlayer(uint32_t guid, uint8_t role, uint8_t availability, uint8_t interests, uint8_t level)
        {
            mGuid = guid;
            mRoles = role;
            mAvailability = availability;
            mInterests = interests;
            mLevel = level;
        }

        LFGuildPlayer(uint32_t guid, uint8_t role, uint8_t availability, uint8_t interests, uint8_t level, std::string& comment) : mComment(comment)
        {
            mGuid = guid;
            mRoles = role;
            mAvailability = availability;
            mInterests = interests;
            mLevel = level;
        }

        LFGuildPlayer(LFGuildPlayer const& settings) : mComment(settings.getComment())
        {
            mGuid = settings.getGUID();
            mRoles = settings.getClassRoles();
            mAvailability = settings.getAvailability();
            mInterests = settings.getInterests();
            mLevel = settings.getLevel();
        }

        uint32_t getGUID() const { return mGuid; }
        uint8_t getClassRoles() const { return mRoles; }
        uint8_t getAvailability() const { return mAvailability; }
        uint8_t getInterests() const { return mInterests; }
        uint8_t getLevel() const { return mLevel; }
        std::string const& getComment() const { return mComment; }

    private:

        std::string mComment;
        uint32_t mGuid;
        uint8_t mRoles;
        uint8_t mAvailability;
        uint8_t mInterests;
        uint8_t mLevel;
};

struct LFGuildSettings : public LFGuildPlayer
{
    public:

        LFGuildSettings() : LFGuildPlayer(), mIsListed(false), mTeam(TEAM_ALLIANCE) {}

        LFGuildSettings(bool listed, PlayerTeam team) : LFGuildPlayer(), mIsListed(listed), mTeam(team) {}

        LFGuildSettings(bool listed, PlayerTeam team, uint32_t guid, uint8_t role, uint8_t availability, uint8_t interests, uint8_t level) :
            LFGuildPlayer(guid, role, availability, interests, level), mIsListed(listed), mTeam(team) {}

        LFGuildSettings(bool listed, PlayerTeam team, uint32_t guid, uint8_t role, uint8_t availability, uint8_t interests, uint8_t level, std::string& comment) :
            LFGuildPlayer(guid, role, availability, interests, level, comment), mIsListed(listed), mTeam(team) {}

        LFGuildSettings(LFGuildSettings const& settings) :
            LFGuildPlayer(settings), mIsListed(settings.isListed()), mTeam(settings.getTeam()) {}


        bool isListed() const { return mIsListed; }
        void setListed(bool state) { mIsListed = state; }

        PlayerTeam getTeam() const { return mTeam; }

    private:

        bool mIsListed;
        PlayerTeam mTeam;
};

typedef std::map<uint32_t, LFGuildSettings> LFGuildStore;
typedef std::map<uint32_t, std::vector<MembershipRequest>> MembershipRequestStore;

class SERVER_DECL GuildFinderMgr : public Singleton <GuildFinderMgr>
{
    public:

        GuildFinderMgr();
        ~GuildFinderMgr();

        LFGuildStore _lfgGuildStore;

        MembershipRequestStore _membershipRequestStore;

        void loadGuildSettingsFromDB();
        void loadMembershipRequestsFromDB();

    public:

        void loadGuildFinderDataFromDB();

        void setGuildSettings(uint32_t guildGuid, LFGuildSettings const& settings);
        LFGuildSettings getGuildSettings(uint32_t guildGuid) { return _lfgGuildStore[guildGuid]; }

        void addMembershipRequest(uint32_t guildGuid, MembershipRequest const& request);
        void removeAllMembershipRequestsFromPlayer(uint32_t playerId);
        void removeMembershipRequest(uint32_t playerId, uint32_t guildId);

        void deleteGuild(uint32_t guildId);

        std::vector<MembershipRequest> getAllMembershipRequestsForGuild(uint32_t guildGuid) { return _membershipRequestStore[guildGuid]; }
        std::list<MembershipRequest> getAllMembershipRequestsForPlayer(uint32_t playerGuid);
        LFGuildStore getGuildsMatchingSetting(LFGuildPlayer& settings, PlayerTeam faction);

        bool hasRequest(uint32_t playerId, uint32_t guildId);
        uint8_t countRequestsFromPlayer(uint32_t playerId);

        void sendApplicantListUpdate(Guild& guild);
        void sendMembershipRequestListUpdate(Player& player);
};

#define sGuildFinderMgr GuildFinderMgr::getSingleton()
