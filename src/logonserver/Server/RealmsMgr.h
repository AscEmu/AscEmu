/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct Realms
{
    uint32_t id;
    std::string password;
    uint8_t status;

    //internal
    std::chrono::high_resolution_clock::time_point lastPing;
};

class RealmsMgr : public Singleton<RealmsMgr>
{
public:

    void LoadRealms();
    std::vector<std::shared_ptr<Realms>> _realmStore;

    std::shared_ptr<Realms> getRealmById(uint32_t id);

    void setStatusForRealm(uint8_t realm_id, uint32_t status);

    void setLastPing(uint8_t realm_id);

    void checkRealmStatus();
};

#define sRealmsMgr RealmsMgr::getSingleton()