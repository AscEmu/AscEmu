/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <map>
#include <mutex>
#include <set>
#include <cstdint>
#include <string>

class CachedCharacterInfo;
class Player;
class WorldPacket;

typedef std::map<Player*, uint8_t> MemberMap;

class SERVER_DECL Channel
{
public:
    // Custom channels don't use channel id
    Channel(std::string name, uint8_t team, uint32_t channelId = 0);
    ~Channel();

    std::string getChannelName() const;
    std::string getChannelPassword() const;
    uint32_t getChannelId() const;
    uint8_t getChannelFlags() const;
    uint8_t getChannelTeam() const;

    void setChannelName(std::string name); // Used by Lua API

    void attemptJoin(Player* plr, std::string password, bool skipCheck = false);
    void leaveChannel(Player* plr, bool sendPacket = true);

    size_t getMemberCount() const;
    bool hasMember(Player* pPlayer) const;

    void say(Player* plr, std::string message, Player* for_gm_client, bool forced);

    void invitePlayer(Player* plr, Player* new_player);
    void kickOrBanPlayer(Player* plr, Player* die_player, bool ban);
    void unBanPlayer(Player* plr, CachedCharacterInfo const* bplr);

    void moderateChannel(Player* plr);
    void giveModerator(Player* plr, Player* new_player);
    void takeModerator(Player* plr, Player* new_player);

    void mutePlayer(Player* plr, Player* die_player);
    void unMutePlayer(Player* plr, Player* die_player);
    void giveVoice(Player* plr, Player* v_player);
    void takeVoice(Player* plr, Player* v_player);

    void setPassword(Player* plr, std::string pass);
    void enableAnnouncements(Player* plr);

    void getOwner(Player* plr);
    void setOwner(Player* plr, Player const* newOwner);

    void listMembers(Player* plr, bool chatQuery);

    // Packets
    void sendToAll(WorldPacket* data);
    void sendToAll(WorldPacket* data, Player* skipPlayer);

private:
    std::string m_channelName = std::string();
    std::string m_channelPassword = std::string();

    MemberMap m_members;
    std::set<uint32_t> m_bannedMembers;

    uint8_t m_channelTeam = 0;
    uint32_t m_channelId = 0;
    uint8_t m_channelFlags = 0;
    bool m_muted = false;
    bool m_announcePlayers = true;
    uint8_t m_minimumLevel = 1;

    mutable std::mutex m_mutexChannel;
};
