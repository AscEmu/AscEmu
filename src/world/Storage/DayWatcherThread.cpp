/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Management/ArenaTeam.h"
#include "Server/MainServerDefines.h"
#include "Objects/ObjectMgr.h"
#include "DayWatcherThread.h"

using AscEmu::Threading::AEThread;
using std::chrono::milliseconds;
using std::make_unique;


DayWatcherThread::DayWatcherThread()
{
    m_updateDBSettings = false;

    m_lastArenaTime = 0;
    m_localLastArenaTime.tm_sec = 0;
    m_localLastArenaTime.tm_min = 0;
    m_localLastArenaTime.tm_hour = 0;
    m_localLastArenaTime.tm_mday = 0;
    m_localLastArenaTime.tm_mon = 0;
    m_localLastArenaTime.tm_year = 0;
    m_localLastArenaTime.tm_wday = 0;
    m_localLastArenaTime.tm_yday = 0;
    m_localLastArenaTime.tm_isdst = 0;
#ifndef WIN32
    m_localLastArenaTime.tm_gmtoff = 0;
    m_localLastArenaTime.tm_zone = 0;
#endif
    m_lastDailyTime = 0;
    m_localLastDailyTime.tm_sec = 0;
    m_localLastDailyTime.tm_min = 0;
    m_localLastDailyTime.tm_hour = 0;
    m_localLastDailyTime.tm_mday = 0;
    m_localLastDailyTime.tm_mon = 0;
    m_localLastDailyTime.tm_year = 0;
    m_localLastDailyTime.tm_wday = 0;
    m_localLastDailyTime.tm_yday = 0;
    m_localLastDailyTime.tm_isdst = 0;
#ifndef WIN32
    m_localLastDailyTime.tm_gmtoff = 0;
    m_localLastDailyTime.tm_zone = 0;
#endif
    m_arenaPeriod = WEEKLY;
    m_dailyPeriod = WEEKLY;

    m_currenttime = 0;
    m_localCurrenttime.tm_sec = 0;
    m_localCurrenttime.tm_min = 0;
    m_localCurrenttime.tm_hour = 0;
    m_localCurrenttime.tm_mday = 0;
    m_localCurrenttime.tm_mon = 0;
    m_localCurrenttime.tm_year = 0;
    m_localCurrenttime.tm_wday = 0;
    m_localCurrenttime.tm_yday = 0;
    m_localCurrenttime.tm_isdst = 0;
#ifndef WIN32
    m_localCurrenttime.tm_gmtoff = 0;
    m_localCurrenttime.tm_zone = 0;
#endif

    m_thread = make_unique<AEThread>("DayWatcherThread", [this](AEThread& thread) { this->threadRunner(thread); }, milliseconds(120000), false);
    this->threadInit();
}

DayWatcherThread::~DayWatcherThread() { m_thread->join(); }

void DayWatcherThread::threadInit()
{
    m_currenttime = UNIXTIME;
    dupe_tm_pointer(localtime(&m_currenttime), &m_localCurrenttime);
    load_settings();
    set_tm_pointers();

    LogNotice("DayWatcherThread : Started");
    m_thread->reboot();
}

void DayWatcherThread::threadRunner(AEThread& /*thread*/)
{
    m_currenttime = UNIXTIME;
    dupe_tm_pointer(localtime(&m_currenttime), &m_localCurrenttime);

    if (has_timeout_expired(&m_localCurrenttime, &m_localLastArenaTime, m_arenaPeriod))
        update_arena();

    if (has_timeout_expired(&m_localCurrenttime, &m_localLastDailyTime, m_dailyPeriod))
        update_daily();

    if (m_updateDBSettings)
        update_settings();
}

void DayWatcherThread::dupe_tm_pointer(tm* returnvalue, tm* mypointer)
{
    memcpy(mypointer, returnvalue, sizeof(tm));
}

void DayWatcherThread::update_settings()
{
    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\'last_arena_update_time\', %u);", m_lastArenaTime);
    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\'last_daily_update_time\', %u);", m_lastDailyTime);
}

void DayWatcherThread::load_settings()
{
    m_arenaPeriod = get_timeout_from_string(worldConfig.period.arenaUpdate, WEEKLY);
    QueryResult* result = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \'last_arena_update_time\'");
    if (result)
    {
        m_lastArenaTime = result->Fetch()[0].GetUInt32();
        delete result;
    }
    else
    {
        LogNotice("DayWatcherThread : Initializing Arena Updates to zero.");
        m_lastArenaTime = 0;
    }

    m_dailyPeriod = get_timeout_from_string(worldConfig.period.dailyUpdate, DAILY);
    QueryResult* result2 = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \'last_daily_update_time\'");
    if (result2)
    {
        m_lastDailyTime = result2->Fetch()[0].GetUInt32();
        delete result2;
    }
    else
    {
        LogNotice("DayWatcherThread : Initializing Daily Updates to zero.");
        m_lastDailyTime = 0;
    }
}

void DayWatcherThread::set_tm_pointers()
{
    dupe_tm_pointer(localtime(&m_lastArenaTime), &m_localLastArenaTime);
    dupe_tm_pointer(localtime(&m_lastDailyTime), &m_localLastDailyTime);
}

uint32_t DayWatcherThread::get_timeout_from_string(std::string string, uint32_t def)
{
    if (string.compare("weekly") == 0)
        return WEEKLY;
    if (string.compare("monthly") == 0)
        return MONTHLY;
    if (string.compare("daily") == 0)
        return DAILY;
    if (string.compare("hourly") == 0)
        return HOURLY;

    return def;
}

bool DayWatcherThread::has_timeout_expired(tm* now_time, tm* last_time, uint32_t timeoutval)
{
    switch (timeoutval)
    {
        case WEEKLY:
            return abs(now_time->tm_yday - last_time->tm_yday) >= 7;
        case MONTHLY:
            return now_time->tm_mon != last_time->tm_mon;
        case HOURLY:
            return now_time->tm_hour != last_time->tm_hour || now_time->tm_mday != last_time->tm_mday || now_time->tm_mon != last_time->tm_mon;
        case DAILY:
            return now_time->tm_mday != last_time->tm_mday && now_time->tm_hour == 4;
    }

    return false;
}

void DayWatcherThread::update_daily()
{
    LogNotice("DayWatcherThread : Running Daily Quest Reset...");

    CharacterDatabase.WaitExecute("UPDATE characters SET finisheddailies = ''");
    CharacterDatabase.WaitExecute("UPDATE characters SET rbg_daily = '0'");     // Reset RBG

    objmgr.ResetDailies();
    m_lastDailyTime = UNIXTIME;
    dupe_tm_pointer(localtime(&m_lastDailyTime), &m_localLastDailyTime);
    m_updateDBSettings = true;
}

void DayWatcherThread::update_arena()
{
    LogNotice("DayWatcherThread : Running Weekly Arena Point Maintenance...");

    QueryResult* result = CharacterDatabase.Query("SELECT guid, arenaPoints FROM characters");
    uint32_t arenapointsPerTeam[3] = { 0 };
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            uint32_t guid = field[0].GetUInt32();
            uint32_t arenapoints = field[1].GetUInt32();
            uint32_t orig_arenapoints = arenapoints;

            for (uint8_t i = 0; i < 3; ++i)
                arenapointsPerTeam[i] = 0;

            // are we in any arena teams?
            for (uint8_t i = 0; i < 3; ++i)
            {
                ArenaTeam* team = objmgr.GetArenaTeamByGuid(guid, i);
                if (team != nullptr)
                {
                    const auto arenaTeamMember = team->GetMemberByGuid(guid);
                    if (arenaTeamMember == nullptr || team->m_stat_gamesplayedweek < 10 || arenaTeamMember->Played_ThisWeek * 100 / team->m_stat_gamesplayedweek < 30)
                        continue;

                    const double arenaStatsRating = static_cast<double>(team->m_stat_rating);
                    double anrenaPoints;

                    if (arenaStatsRating <= 510.0)
                        continue;

                    if (arenaStatsRating > 510.0 && arenaStatsRating <= 1500.0)        // 510 < X <= 1500"
                    {
                        anrenaPoints = 0.22 * arenaStatsRating + 14.0;
                    }
                    else
                    {
                        const double power = -0.00412 * arenaStatsRating;
                        double divisor = pow(static_cast<double>(2.71828), power);
                        divisor *= 1639.28;
                        divisor += 1.0;

                        anrenaPoints = 1511.26 / divisor;
                    }

                    if (team->m_type == ARENA_TEAM_TYPE_2V2)
                    {
                        anrenaPoints *= 0.76;
                        anrenaPoints *= worldConfig.getFloatRate(RATE_ARENAPOINTMULTIPLIER2X);
                    }
                    else if (team->m_type == ARENA_TEAM_TYPE_3V3)
                    {
                        anrenaPoints *= 0.88;
                        anrenaPoints *= worldConfig.getFloatRate(RATE_ARENAPOINTMULTIPLIER3X);
                    }
                    else
                    {
                        anrenaPoints *= worldConfig.getFloatRate(RATE_ARENAPOINTMULTIPLIER5X);
                    }

                    if (anrenaPoints > 1.0)
                        arenapointsPerTeam[i] += long2int32(double(ceil(anrenaPoints)));
                }
            }

            arenapointsPerTeam[0] = static_cast<uint32_t>(std::max(arenapointsPerTeam[0], arenapointsPerTeam[1]));
            arenapoints += static_cast<uint32_t>(std::max(arenapointsPerTeam[0], arenapointsPerTeam[2]));
            if (arenapoints > 5000)
                arenapoints = 5000;

            if (orig_arenapoints != arenapoints)
            {
                auto player = objmgr.GetPlayer(guid);
                if (player != nullptr)
                {
                    player->AddArenaPoints(arenapoints, false);

                    // update fields (no uint lock)
                    sEventMgr.AddEvent(player, &Player::UpdateArenaPoints, EVENT_PLAYER_UPDATE, 100, 1, 0);
                    sChatHandler.SystemMessage(player->GetSession(), "Your arena points have been updated! Check your PvP tab!");
                }

                CharacterDatabase.Execute("UPDATE characters SET arenaPoints = %u WHERE guid = %u", arenapoints, guid);
            }
        } while (result->NextRow());
        delete result;
    }

    objmgr.UpdateArenaTeamWeekly();

    m_lastArenaTime = UNIXTIME;
    dupe_tm_pointer(localtime(&m_lastArenaTime), &m_localLastArenaTime);
    m_updateDBSettings = true;
}
