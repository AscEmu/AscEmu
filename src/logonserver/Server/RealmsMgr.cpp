/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include "RealmsMgr.h"
#include "Util.hpp"
#include <Threading/AEThreadPool.h>

initialiseSingleton(RealmsMgr);


void RealmsMgr::LoadRealms()
{
    QueryResult* result = sLogonSQL->Query("SELECT id, status FROM realms");
    if (result)
    {
        do
        {
            Field* field = result->Fetch();

            auto realms = std::make_unique<Realms>();
            realms->id = field[0].GetUInt32();
            realms->status = field[1].GetUInt8();
            realms->lastPing = Util::TimeNow();

            _realmStore.push_back(std::move(realms));
        } while (result->NextRow());
    }
}

void RealmsMgr::setStatusForRealm(uint8_t realm_id, uint32_t status)
{
    if (_realmStore.empty())
    {
        auto realms = std::make_unique<Realms>();
        realms->id = realm_id;
        realms->status = uint8_t(status);
        realms->lastPing = Util::TimeNow();

        _realmStore.push_back(std::move(realms));

        sLogonSQL->Query("REPLACE INTO realms(id, status, status_change_time) VALUES(%u, %u, NOW())", status, uint32_t(realm_id));
    }
    else
    {
        for (auto& realm : _realmStore)
        {
            if (realm->id == realm_id)
                realm->status = status;
        }

        sLogonSQL->Query("UPDATE realms SET status = %u WHERE id = %u", status, uint32_t(realm_id));
    }
}

void RealmsMgr::setLastPing(uint8_t realm_id)
{
    for (auto& realm : _realmStore)
    {
        if (realm->id == realm_id)
            realm->lastPing = Util::TimeNow();
    }
}

void RealmsMgr::checkRealmStatus()
{
    for (auto& realm : _realmStore)
    {
        // if there was no ping in the last 2 minutes (in miliseconds) we set the status to the realm to offline.
        if (Util::GetTimeDifferenceToNow(realm->lastPing) > 2 * 60 * 1000 && realm->status != 0)
        {
            realm->status = 0;
            LogDetail("Realm %u status gets set to 0 (offline) since there was no ping the last 2 minutes (%u).", uint32_t(realm->id), Util::GetTimeDifferenceToNow(realm->lastPing));
            sLogonSQL->Query("UPDATE realms SET status = 0 WHERE id = %u", uint32_t(realm->id));
        }
    }
}
