/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/InstanceMgr.hpp"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/CalendarMgr.hpp"
#include "Management/Guild/Guild.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/MailMgr.h"
#include "Management/GameEvent.hpp"
#include "Management/GameEventDefines.hpp"
#include "Management/GameEventMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/Packets/SmsgCalendarSendNumPending.h"
#include "Server/Packets/SmsgCalendarRaidLockoutRemoved.h"
#include "Server/Packets/SmsgCalendarRaidLockoutUpdated.h"
#include "Server/Packets/SmsgCalendarSendCalendar.h"
#include "Server/Packets/SmsgCalendarSendEvent.h"
#include "Server/Packets/SmsgCalendarCommandResult.h"
#include "Server/Packets/SmsgCalendarFilterGuild.h"
#include "Server/Packets/SmsgCalendarEventInvite.h"
#include "Server/Packets/SmsgCalendarEventInviteAlert.h"
#include "Server/Packets/SmsgCalendarEventStatus.h"
#include "Server/Packets/SmsgCalendarEventInviteStatusAlert.h"
#include "Server/Packets/SmsgCalendarEventInviteRemoved.h"
#include "Server/Packets/SmsgCalendarEventInviteRemovedAlert.h"
#include "Server/Packets/SmsgCalendarEventModeratorStatusAlert.h"
#include "Server/Packets/SmsgCalendarEventRemovedAlert.h"
#include "Server/Packets/SmsgCalendarEventUpdatedAlert.h"
#include "Server/Packets/SmsgCalendarClearPendingAction.h"
#include "Server/Packets/SmsgCalendarEventInviteNotes.h"
#include "Server/Packets/SmsgCalendarEventInviteNotesAlert.h"
#include "Server/Packets/CmsgCalendarGetEvent.h"
#include "Server/Packets/CmsgCalendarAddEvent.h"
#include "Server/Packets/CmsgCalendarUpdateEvent.h"
#include "Server/Packets/CmsgCalendarRemoveEvent.h"
#include "Server/Packets/CmsgCalendarCopyEvent.h"
#include "Server/Packets/CmsgCalendarGuildFilter.h"
#include "Server/Packets/CmsgCalendarArenaTeam.h"
#include "Server/Packets/CmsgCalendarEventInvite.h"
#include "Server/Packets/CmsgCalendarEventRsvp.h"
#include "Server/Packets/CmsgCalendarEventRemoveInvite.h"
#include "Server/Packets/CmsgCalendarEventStatus.h"
#include "Server/Packets/CmsgCalendarEventSignup.h"
#include "Server/Packets/CmsgCalendarEventInviteNotes.h"
#include "Server/Packets/CmsgSetSavedInstanceExtend.h"

#include <algorithm>
#include <cstdio>
#include <map>

using namespace AscEmu::Packets;

namespace
{
    // events older than a day can no longer be planned
    constexpr time_t CalendarPastTolerance = 86400;

    uint32_t guildOfPlayer(uint32_t lowGuid)
    {
        if (const Player* player = sObjectMgr.getPlayer(lowGuid))
            return player->getGuildId();

        if (const auto* info = sObjectMgr.getCachedCharacterInfo(lowGuid))
            return info->m_guild;

        return 0;
    }

    uint8_t levelOfPlayer(uint32_t lowGuid)
    {
        if (const Player* player = sObjectMgr.getPlayer(lowGuid))
            return static_cast<uint8_t>(player->getLevel());

        if (const auto* info = sObjectMgr.getCachedCharacterInfo(lowGuid))
            return static_cast<uint8_t>(info->lastLevel);

        return 0;
    }

    CalendarEventData eventData(CalendarEvent* calendarEvent)
    {
        CalendarEventData data;
        data.eventId = calendarEvent->m_entry;
        data.title = calendarEvent->m_title;
        data.description = calendarEvent->m_description;
        data.type = static_cast<uint8_t>(calendarEvent->m_type);
        data.textureId = static_cast<int32_t>(calendarEvent->m_dungeon);
        data.date = calendarEvent->m_date;
        data.flags = calendarEvent->m_flags;
        data.ownerGuid = calendarEvent->m_creator;
        data.guildId = sCalendarMgr.getEventGuildId(calendarEvent);
        return data;
    }

    CalendarInviteData inviteData(CalendarInvite const* invite, CalendarEvent* calendarEvent)
    {
        CalendarInviteData data;
        data.inviteId = invite->m_inviteId;
        data.eventId = invite->m_event;
        data.inviteeGuid = invite->m_invitee;
        data.senderGuid = invite->m_sender;
        data.level = levelOfPlayer(invite->m_invitee);
        data.status = static_cast<uint8_t>(invite->m_status);
        data.rank = static_cast<uint8_t>(invite->m_rank);
        data.responseTime = invite->m_statusTime;
        data.notes = invite->m_text;

        if (calendarEvent != nullptr)
        {
            const uint32_t guildId = sCalendarMgr.getEventGuildId(calendarEvent);
            data.ownerGuid = calendarEvent->m_creator;
            data.guildEvent = calendarEvent->isGuildEvent();
            data.guildMember = calendarEvent->isGuildEvent() && guildId != 0 && guildOfPlayer(invite->m_invitee) == guildId;
        }

        return data;
    }

    std::vector<CalendarInviteData> eventInvites(CalendarEvent* calendarEvent)
    {
        std::vector<CalendarInviteData> result;
        for (const auto* invite : sCalendarMgr.getEventInvites(calendarEvent->m_entry))
            result.push_back(inviteData(invite, calendarEvent));

        return result;
    }

    std::vector<Player*> onlineGuildMembers(uint32_t guildId)
    {
        std::vector<Player*> result;
        if (guildId == 0)
            return result;

        if (const Guild* guild = sGuildMgr.getGuildById(guildId))
        {
            for (const auto& [_, member] : guild->getGuildMembers())
                if (Player* player = sObjectMgr.getPlayer(WoWGuid(member->getGUID()).getLowGuid()))
                    result.push_back(player);
        }

        return result;
    }

    // the players an event concerns: owner, the online guild of a guild event and every online invited player
    std::vector<Player*> eventRelatives(CalendarEvent* calendarEvent)
    {
        std::vector<Player*> result;
        const auto addPlayer = [&result](Player* player)
        {
            if (player != nullptr && std::find(result.begin(), result.end(), player) == result.end())
                result.push_back(player);
        };

        if (calendarEvent->belongsToGuild())
            for (Player* member : onlineGuildMembers(sCalendarMgr.getEventGuildId(calendarEvent)))
                addPlayer(member);

        addPlayer(sObjectMgr.getPlayer(calendarEvent->m_creator));
        for (const auto* invite : sCalendarMgr.getEventInvites(calendarEvent->m_entry))
            addPlayer(sObjectMgr.getPlayer(invite->m_invitee));

        return result;
    }

    template <typename TPacket>
    void sendTo(Player* player, TPacket& managedPacket)
    {
        if (player != nullptr && player->getSession() != nullptr)
            player->getSession()->sendManagedPacket(managedPacket);
    }

    void sendCommandResult(Player* player, CalendarError error, std::string const& name = "")
    {
        SmsgCalendarCommandResult managedPacket(error, name);
        sendTo(player, managedPacket);
    }

    void sendClearPending(Player* player)
    {
        SmsgCalendarClearPendingAction managedPacket;
        sendTo(player, managedPacket);
    }

    // the owner and the moderators may change an event and its invites
    bool canModerate(CalendarEvent* calendarEvent, uint32_t playerLowGuid)
    {
        if (calendarEvent->m_creator == playerLowGuid)
            return true;

        const auto* invite = sCalendarMgr.getInviteOfPlayer(calendarEvent->m_entry, playerLowGuid);
        return invite != nullptr && invite->m_rank >= CALENDAR_RANK_MODERATOR;
    }

    // the invite is announced to the event and to the invited player (to the guild for a guild event), then stored
    void addInvite(CalendarEvent* calendarEvent, std::unique_ptr<CalendarInvite> invite)
    {
        const CalendarInviteData data = inviteData(invite.get(), calendarEvent);

        if (!calendarEvent->isGuildAnnouncement() && invite->m_invitee != calendarEvent->m_creator)
        {
            for (Player* player : eventRelatives(calendarEvent))
            {
                SmsgCalendarEventInvite managedPacket(data, !calendarEvent->isGuildEvent());
                sendTo(player, managedPacket);
            }
        }

        if (!calendarEvent->isGuildEvent() || invite->m_invitee == calendarEvent->m_creator)
        {
            if (calendarEvent->belongsToGuild())
            {
                for (Player* member : onlineGuildMembers(sCalendarMgr.getEventGuildId(calendarEvent)))
                {
                    SmsgCalendarEventInviteAlert managedPacket(eventData(calendarEvent), data);
                    sendTo(member, managedPacket);
                }
            }
            else
            {
                SmsgCalendarEventInviteAlert managedPacket(eventData(calendarEvent), data);
                sendTo(sObjectMgr.getPlayer(invite->m_invitee), managedPacket);
            }
        }

        if (!calendarEvent->isGuildAnnouncement())
            sCalendarMgr.addInvite(calendarEvent->m_entry, std::move(invite));
    }

    // the new status of an invite goes to the event, the invited player sees an alert on the clients of 5.x
    void sendInviteStatus(CalendarEvent* calendarEvent, CalendarInvite const* invite)
    {
        const CalendarInviteData data = inviteData(invite, calendarEvent);
        for (Player* player : eventRelatives(calendarEvent))
        {
            SmsgCalendarEventStatus managedPacket(eventData(calendarEvent), data, player->getGuidLow() == invite->m_invitee);
            sendTo(player, managedPacket);
        }

        Player* invitee = sObjectMgr.getPlayer(invite->m_invitee);
        if (invitee != nullptr && invitee->getSession() != nullptr && invitee->getSession()->getClientProtocol().isMop())
        {
            SmsgCalendarEventInviteStatusAlert managedPacket(calendarEvent->m_entry, calendarEvent->m_date, calendarEvent->m_flags, data.status);
            sendTo(invitee, managedPacket);
        }
    }

    // a free slot for an own event or one of the guild
    CalendarError checkEventLimits(Player* player, uint32_t flags)
    {
        const bool forGuild = (flags & (CALENDAR_FLAG_GUILD_EVENT | CALENDAR_FLAG_WITHOUT_INVITES)) != 0;
        if (forGuild)
        {
            if (player->getGuildId() == 0)
                return CALENDAR_ERROR_GUILD_PLAYER_NOT_IN_GUILD;

            if (sCalendarMgr.getGuildEventCount(player->getGuildId()) >= CALENDAR_MAX_GUILD_EVENTS)
                return CALENDAR_ERROR_GUILD_EVENTS_EXCEEDED;
        }
        else if (sCalendarMgr.getOwnedEventCount(player->getGuidLow()) >= CALENDAR_MAX_EVENTS)
        {
            return CALENDAR_ERROR_EVENTS_EXCEEDED;
        }

        return CALENDAR_OK;
    }

    bool isIgnoring(uint32_t playerLowGuid, uint32_t ignoredLowGuid)
    {
        if (const Player* player = sObjectMgr.getPlayer(playerLowGuid))
            return player->isIgnored(ignoredLowGuid);

        auto result = CharacterDatabase.query("SELECT 1 FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", playerLowGuid, ignoredLowGuid);
        return result != nullptr;
    }

    // the next reset of every raid map
    std::vector<CalendarRaidResetData> raidResets(time_t now)
    {
        std::map<uint32_t, time_t> nextReset;
        for (const auto& [key, resetTime] : sInstanceMgr.GetResetTimeMap())
        {
            DifficultyMap mapKey;
            mapKey.MapPair = key;
            const uint32_t mapId = mapKey.MapPairParts.mapid;

            const auto* mapEntry = sMapStore.lookupEntry(mapId);
            if (mapEntry == nullptr || !mapEntry->isRaid() || resetTime <= now)
                continue;

            auto [itr, added] = nextReset.try_emplace(mapId, resetTime);
            if (!added && resetTime < itr->second)
                itr->second = resetTime;
        }

        std::vector<CalendarRaidResetData> result;
        for (const auto& [mapId, resetTime] : nextReset)
        {
            CalendarRaidResetData reset;
            reset.mapId = mapId;
            reset.secondsLeft = static_cast<int32_t>(resetTime - now);
            result.push_back(reset);
        }

        return result;
    }

    // the clients of 3.x and 4.x know holiday dates only up to their release: the event schedule supplies the current ones
    std::vector<CalendarHolidayData> scheduledHolidays(WoW::ClientProtocol const& protocol, time_t now)
    {
        std::vector<CalendarHolidayData> result;
#if VERSION_STRING >= WotLK && VERSION_STRING <= Cata
        constexpr size_t maxDates = 26;

        std::map<uint32_t, std::vector<time_t>> startsByHoliday;
        std::map<uint32_t, uint32_t> lengthByHoliday;
        for (const auto& [_, gameEvent] : sGameEventMgr.mGameEvents)
        {
            const auto holidayId = static_cast<uint32_t>(gameEvent->holiday_id);
            if (holidayId == 0 || gameEvent->start == 0 || sHolidaysStore.lookupEntry(holidayId) == nullptr)
                continue;

            const time_t length = time_t(gameEvent->length) * 60;
            auto& starts = startsByHoliday[holidayId];
            lengthByHoliday[holidayId] = std::max(lengthByHoliday[holidayId], gameEvent->length);

            if (gameEvent->occurence == 0)
            {
                if (gameEvent->start + length > now)
                    starts.push_back(gameEvent->start);
                continue;
            }

            // the running or next run first, then the following ones
            const time_t period = time_t(gameEvent->occurence) * 60;
            const time_t passed = now - length - gameEvent->start;
            time_t start = gameEvent->start + (passed < 0 ? 0 : (passed / period + 1) * period);
            for (; starts.size() < maxDates && (gameEvent->end == 0 || start < gameEvent->end); start += period)
                starts.push_back(start);
        }

        for (auto& [holidayId, starts] : startsByHoliday)
        {
            if (starts.empty())
                continue;

            std::sort(starts.begin(), starts.end());
            starts.erase(std::unique(starts.begin(), starts.end()), starts.end());

            const auto* entry = sHolidaysStore.lookupEntry(holidayId);
            CalendarHolidayData holiday;
            holiday.id = entry->Id;
            holiday.region = entry->Region;
            holiday.looping = entry->Looping;
            holiday.priority = entry->Priority;
            holiday.filterType = static_cast<int32_t>(entry->CalendarFilterType);
            holiday.texture = entry->TextureFilename != nullptr ? entry->TextureFilename : "";

            for (size_t i = 0; i < MAX_HOLIDAY_FLAGS; ++i)
                holiday.flags[i] = entry->CalendarFlags[i];

            // rotating holidays keep their stages, the others last as long as the event
            if (entry->Looping != 0)
            {
                for (size_t i = 0; i < MAX_HOLIDAY_DURATIONS; ++i)
                    holiday.durations[i] = entry->Duration[i];
            }
            else
            {
                holiday.durations[0] = (lengthByHoliday[holidayId] + 59) / 60;
            }

            for (size_t i = 0; i < starts.size() && i < maxDates; ++i)
                holiday.dates[i] = CalendarWire::dateValue(protocol, starts[i]);

            result.push_back(std::move(holiday));
        }
#else
        (void)protocol;
        (void)now;
#endif
        return result;
    }

    // the name of the player in the subject of a calendar mail, as the client of the version writes it
    std::string mailGuidToken(WoW::ClientProtocol const& protocol, uint32_t lowGuid)
    {
        const WoWGuid guid = CalendarWire::playerGuid(lowGuid);
        char buffer[64];

        if (protocol.expansion >= WoW::Expansion::_BfA)
        {
            std::snprintf(buffer, sizeof(buffer), "Player-%u-%08X", static_cast<uint32_t>(protocol.realmId), lowGuid);
        }
        else if (protocol.expansion >= WoW::Expansion::_WoD)
        {
            const WoWGuid128 wide = guid.toGuid128(protocol.realmId, 0);
            std::snprintf(buffer, sizeof(buffer), "%016llx%016llx", static_cast<unsigned long long>(wide.high), static_cast<unsigned long long>(wide.low));
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(guid.getRawGuid()));
        }

        return buffer;
    }

    // every invited player except the one who deleted the event gets a mail, the client writes the text from subject and body
    void sendRemovalMails(CalendarEvent* calendarEvent, Player* remover)
    {
        const WoW::ClientProtocol protocol = remover->getSession()->getClientProtocol();
        const std::string subject = mailGuidToken(protocol, remover->getGuidLow()) + ":" + calendarEvent->m_title;
        const std::string body = std::to_string(CalendarWire::dateValue(protocol, calendarEvent->m_date));

        uint32_t mask = MAIL_CHECK_MASK_COPIED;
        if (protocol.expansion == WoW::Expansion::_WotLK || protocol.expansion >= WoW::Expansion::_TheWarWithin)
            mask |= MAIL_CHECK_MASK_CALENDAR_INVITE;

        for (const auto* invite : sCalendarMgr.getEventInvites(calendarEvent->m_entry))
        {
            if (invite->m_invitee == remover->getGuidLow())
                continue;

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_CALENDAR, calendarEvent->m_entry, CalendarWire::playerGuid(invite->m_invitee).getRawGuid(),
                subject, body, 0, 0, uint64_t(0), MAIL_STATIONERY_TEST2, static_cast<MailCheckMask>(mask));
        }
    }
}

void WorldSession::handleCalendarGetCalendar(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_CALENDAR.");

    const uint32_t lowGuid = _player->getGuidLow();

    std::vector<CalendarInviteData> invites;
    for (const auto* invite : sCalendarMgr.getPlayerInvites(lowGuid))
        invites.push_back(inviteData(invite, sCalendarMgr.getEvent(invite->m_event)));

    std::vector<CalendarEventData> events;
    for (auto* calendarEvent : sCalendarMgr.getCalendarEvents(lowGuid, _player->getGuildId()))
        events.push_back(eventData(calendarEvent));

    // the permanent raid saves of the player
    std::vector<CalendarLockoutData> lockouts;
    const time_t now = Util::getTimeNow();
    for (uint8_t difficulty = 0; difficulty < InstanceDifficulty::MAX_BIND_DIFFICULTY; ++difficulty)
    {
        for (const auto& [mapId, bind] : _player->getBoundInstances(static_cast<InstanceDifficulty::Difficulties>(difficulty)))
        {
            if (!bind.perm || bind.save == nullptr)
                continue;

            CalendarLockoutData lockout;
            lockout.instanceId = bind.save->getInstanceId();
            lockout.mapId = mapId;
            lockout.difficulty = difficulty;
            lockout.secondsLeft = static_cast<int32_t>(std::max<time_t>(0, bind.save->getResetTime() - now));
            lockouts.push_back(lockout);
        }
    }

    SmsgCalendarSendCalendar managedPacket(std::move(invites), std::move(events), std::move(lockouts), raidResets(now), scheduledHolidays(getClientProtocol(), now));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarComplain(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_COMPLAIN, complaints are not processed.");
}

void WorldSession::handleCalendarGetNumPending(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_NUM_PENDING.");

    SmsgCalendarSendNumPending managedPacket(sCalendarMgr.getPlayerNumPending(_player->getGuidLow()));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarAddEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_ADD_EVENT.");

    CmsgCalendarAddEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (time_t(srlPacket.eventTime) < Util::getTimeNow() - CalendarPastTolerance)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_PASSED);
        return;
    }

    if (const auto error = checkEventLimits(_player, srlPacket.flags); error != CALENDAR_OK)
    {
        sendCommandResult(_player, error);
        return;
    }

    const uint32_t lowGuid = _player->getGuidLow();
    auto newEvent = std::make_unique<CalendarEvent>(sCalendarMgr.generateEventId(), lowGuid, srlPacket.title, srlPacket.description,
        static_cast<CalendarEventType>(srlPacket.type), static_cast<uint32_t>(srlPacket.dungeonId), time_t(srlPacket.eventTime), srlPacket.flags);

    if (newEvent->belongsToGuild())
        newEvent->m_guildId = _player->getGuildId();

    CalendarEvent* calendarEvent = sCalendarMgr.addEvent(std::move(newEvent));

    if (calendarEvent->isGuildAnnouncement())
    {
        // an announcement has no invites, the guild learns about it once
        addInvite(calendarEvent, std::make_unique<CalendarInvite>(0, calendarEvent->m_entry, lowGuid, lowGuid, CALENDAR_STATUS_NOT_SIGNED_UP,
            CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_RANK_PLAYER, ""));
    }
    else
    {
        for (const auto& invitee : srlPacket.invitees)
        {
            addInvite(calendarEvent, std::make_unique<CalendarInvite>(sCalendarMgr.generateInviteId(), calendarEvent->m_entry, invitee.guid,
                lowGuid, static_cast<CalendarInviteStatus>(invitee.status), CALENDAR_DEFAULT_RESPONSE_TIME, invitee.rank, ""));
        }
    }

    SmsgCalendarSendEvent managedPacket(CALENDAR_SENDTYPE_ADD, eventData(calendarEvent), eventInvites(calendarEvent));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarGetEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_EVENT.");

    CmsgCalendarGetEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    SmsgCalendarSendEvent managedPacket(CALENDAR_SENDTYPE_GET, eventData(calendarEvent), eventInvites(calendarEvent));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarGuildFilter(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GUILD_FILTER.");

    CmsgCalendarGuildFilter srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = sGuildMgr.getGuildById(_player->getGuildId()))
        guild->massInviteToEvent(this, srlPacket.minLevel, srlPacket.maxLevel, srlPacket.maxRank);
}

void WorldSession::handleCalendarArenaTeam(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_ARENA_TEAM.");

    CmsgCalendarArenaTeam srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const ArenaTeam* arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.arenaTeamId);
    if (arenaTeam == nullptr)
        return;

    std::vector<SmsgCalendarFilterGuildEntry> members;
    for (uint32_t i = 0; i < arenaTeam->m_memberCount; ++i)
    {
        const auto* info = arenaTeam->m_members[i].Info;
        if (info == nullptr || info->guid == _player->getGuidLow())
            continue;

        SmsgCalendarFilterGuildEntry entry;
        entry.guid = CalendarWire::playerGuid(info->guid).getRawGuid();
        entry.level = levelOfPlayer(info->guid);
        members.push_back(entry);
    }

    SmsgCalendarFilterGuild managedPacket(std::move(members), true);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarUpdateEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_UPDATE_EVENT.");

    CmsgCalendarUpdateEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (!canModerate(calendarEvent, _player->getGuidLow()))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    if (time_t(srlPacket.eventPackedTime) < Util::getTimeNow() - CalendarPastTolerance)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_PASSED);
        return;
    }

    const time_t originalDate = calendarEvent->m_date;

    calendarEvent->m_type = static_cast<CalendarEventType>(srlPacket.type);
    calendarEvent->m_flags = srlPacket.flags;
    calendarEvent->m_date = time_t(srlPacket.eventPackedTime);
    calendarEvent->m_dungeon = srlPacket.dungeonId;
    calendarEvent->m_title = srlPacket.title;
    calendarEvent->m_description = srlPacket.description;
    sCalendarMgr.updateEvent(calendarEvent);

    for (Player* player : eventRelatives(calendarEvent))
    {
        SmsgCalendarEventUpdatedAlert managedPacket(eventData(calendarEvent), originalDate, player->getGuidLow() == calendarEvent->m_creator);
        sendTo(player, managedPacket);
    }
}

void WorldSession::handleCalendarRemoveEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_REMOVE_EVENT.");

    CmsgCalendarRemoveEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (calendarEvent->m_creator != _player->getGuidLow())
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    for (Player* player : eventRelatives(calendarEvent))
    {
        SmsgCalendarEventRemovedAlert managedPacket(calendarEvent->m_entry, calendarEvent->m_date, player->getGuidLow() == calendarEvent->m_creator);
        sendTo(player, managedPacket);
    }

    sendRemovalMails(calendarEvent, _player);
    sCalendarMgr.removeEvent(srlPacket.eventId);
}

void WorldSession::handleCalendarCopyEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_COPY_EVENT.");

    CmsgCalendarCopyEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* source = sCalendarMgr.getEvent(srlPacket.eventId);
    if (source == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    // own events, or guild events of the own guild
    const uint32_t lowGuid = _player->getGuidLow();
    const bool mayCopy = source->belongsToGuild() ? sCalendarMgr.getEventGuildId(source) == _player->getGuildId() : source->m_creator == lowGuid;
    if (!mayCopy)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (time_t(srlPacket.eventTime) < Util::getTimeNow() - CalendarPastTolerance)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_PASSED);
        return;
    }

    if (const auto error = checkEventLimits(_player, source->m_flags); error != CALENDAR_OK)
    {
        sendCommandResult(_player, error);
        return;
    }

    auto newEvent = std::make_unique<CalendarEvent>(sCalendarMgr.generateEventId(), lowGuid, source->m_title, source->m_description,
        source->m_type, source->m_dungeon, time_t(srlPacket.eventTime), source->m_flags, source->belongsToGuild() ? _player->getGuildId() : 0);
    CalendarEvent* calendarEvent = sCalendarMgr.addEvent(std::move(newEvent));

    // the copy belongs to the player, earlier owners become moderators
    for (const auto* sourceInvite : sCalendarMgr.getEventInvites(source->m_entry))
    {
        const bool self = sourceInvite->m_invitee == lowGuid;
        const uint32_t rank = self ? CALENDAR_RANK_OWNER : std::min<uint32_t>(sourceInvite->m_rank, CALENDAR_RANK_MODERATOR);
        addInvite(calendarEvent, std::make_unique<CalendarInvite>(sCalendarMgr.generateInviteId(), calendarEvent->m_entry, sourceInvite->m_invitee,
            lowGuid, self ? CALENDAR_STATUS_CONFIRMED : CALENDAR_STATUS_INVITED, CALENDAR_DEFAULT_RESPONSE_TIME, rank, ""));
    }

    SmsgCalendarSendEvent managedPacket(CALENDAR_SENDTYPE_COPY, eventData(calendarEvent), eventInvites(calendarEvent));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarEventInvite(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_INVITE.");

    CmsgCalendarEventInvite srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    std::string name = srlPacket.name;
    AscEmu::Util::Strings::capitalize(name);

    // the invited player, online or from the character cache
    uint32_t inviteeGuid = 0;
    uint32_t inviteeTeam = 0;
    if (const Player* invitee = sObjectMgr.getPlayer(name.c_str(), false))
    {
        inviteeGuid = invitee->getGuidLow();
        inviteeTeam = static_cast<uint32_t>(invitee->getTeam());
    }
    else if (const auto* info = sObjectMgr.getCachedCharacterInfoByName(name))
    {
        inviteeGuid = info->guid;
        inviteeTeam = info->team;
    }

    if (inviteeGuid == 0)
    {
        sendCommandResult(_player, CALENDAR_ERROR_PLAYER_NOT_FOUND);
        return;
    }

    if (inviteeTeam != static_cast<uint32_t>(_player->getTeam()) && !worldConfig.player.isInterfactionMiscEnabled && !HasGMPermissions())
    {
        sendCommandResult(_player, CALENDAR_ERROR_NOT_ALLIED);
        return;
    }

    const uint32_t lowGuid = _player->getGuidLow();
    if (isIgnoring(inviteeGuid, lowGuid))
    {
        sendCommandResult(_player, CALENDAR_ERROR_IGNORING_YOU_S, name);
        return;
    }

    const uint32_t inviteeGuild = guildOfPlayer(inviteeGuid);

    if (srlPacket.isPreInvite)
    {
        // the event is still in the planner: only the planner learns about the invite
        if (srlPacket.isSignUp && inviteeGuild != 0 && inviteeGuild == _player->getGuildId())
        {
            sendCommandResult(_player, CALENDAR_ERROR_NO_GUILD_INVITES);
            return;
        }

        CalendarInviteData data;
        data.inviteeGuid = inviteeGuid;
        data.senderGuid = lowGuid;
        data.level = levelOfPlayer(inviteeGuid);
        data.status = CALENDAR_STATUS_INVITED;
        data.responseTime = CALENDAR_DEFAULT_RESPONSE_TIME;

        SmsgCalendarEventInvite managedPacket(data, true);
        sendManagedPacket(managedPacket);
        return;
    }

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (!canModerate(calendarEvent, lowGuid))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    // members of the guild sign up for a guild event themselves
    if (calendarEvent->isGuildEvent() && inviteeGuild != 0 && inviteeGuild == sCalendarMgr.getEventGuildId(calendarEvent))
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_GUILD_INVITES);
        return;
    }

    if (sCalendarMgr.getInviteOfPlayer(calendarEvent->m_entry, inviteeGuid) != nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S, name);
        return;
    }

    if (sCalendarMgr.getEventInvites(calendarEvent->m_entry).size() >= CALENDAR_MAX_INVITES)
    {
        sendCommandResult(_player, CALENDAR_ERROR_INVITES_EXCEEDED);
        return;
    }

    addInvite(calendarEvent, std::make_unique<CalendarInvite>(sCalendarMgr.generateInviteId(), calendarEvent->m_entry, inviteeGuid, lowGuid,
        CALENDAR_STATUS_INVITED, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_RANK_PLAYER, ""));
}

void WorldSession::handleCalendarEventSignup(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_SIGNUP.");

    CmsgCalendarEventSignup srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (calendarEvent->isGuildEvent() && sCalendarMgr.getEventGuildId(calendarEvent) != _player->getGuildId())
    {
        sendCommandResult(_player, CALENDAR_ERROR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    const uint32_t lowGuid = _player->getGuidLow();
    const auto status = srlPacket.tentative ? CALENDAR_STATUS_TENTATIVE : CALENDAR_STATUS_SIGNED_UP;

    // a second sign up only changes the answer
    if (CalendarInvite* invite = sCalendarMgr.getInviteOfPlayer(calendarEvent->m_entry, lowGuid))
    {
        invite->m_status = status;
        invite->m_statusTime = Util::getTimeNow();
        sCalendarMgr.updateInvite(invite);
        sendInviteStatus(calendarEvent, invite);
    }
    else
    {
        addInvite(calendarEvent, std::make_unique<CalendarInvite>(sCalendarMgr.generateInviteId(), calendarEvent->m_entry, lowGuid, lowGuid,
            status, Util::getTimeNow(), CALENDAR_RANK_PLAYER, ""));
    }

    sendClearPending(_player);
}

void WorldSession::handleCalendarEventRsvp(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_RSVP.");

    CmsgCalendarEventRsvp srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (srlPacket.status != CALENDAR_STATUS_REMOVED && calendarEvent->isLocked())
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_LOCKED);
        return;
    }

    // only the own invite is answered
    CalendarInvite* invite = sCalendarMgr.getInvite(srlPacket.eventId, srlPacket.inviteId);
    if (invite == nullptr || invite->m_invitee != _player->getGuidLow())
        invite = sCalendarMgr.getInviteOfPlayer(srlPacket.eventId, _player->getGuidLow());

    if (invite == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    invite->m_status = static_cast<CalendarInviteStatus>(srlPacket.status);
    invite->m_statusTime = Util::getTimeNow();
    sCalendarMgr.updateInvite(invite);

    sendInviteStatus(calendarEvent, invite);
    sendClearPending(_player);
}

void WorldSession::handleCalendarEventRemoveInvite(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_REMOVE_INVITE.");

    CmsgCalendarEventRemoveInvite srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    if (srlPacket.inviteeGuid == calendarEvent->m_creator)
    {
        sendCommandResult(_player, CALENDAR_ERROR_DELETE_CREATOR_FAILED);
        return;
    }

    // a moderator removes others, every player may leave on their own
    if (srlPacket.inviteeGuid != _player->getGuidLow() && !canModerate(calendarEvent, _player->getGuidLow()))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    CalendarInvite* invite = sCalendarMgr.getInvite(srlPacket.eventId, srlPacket.inviteId);
    if (invite == nullptr || invite->m_invitee != srlPacket.inviteeGuid)
        invite = sCalendarMgr.getInviteOfPlayer(srlPacket.eventId, srlPacket.inviteeGuid);

    if (invite == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    // the event and the removed player learn about it while the invite still exists
    for (Player* player : eventRelatives(calendarEvent))
    {
        SmsgCalendarEventInviteRemoved managedPacket(calendarEvent->m_entry, invite->m_invitee, calendarEvent->m_flags);
        sendTo(player, managedPacket);
    }

    if (!calendarEvent->isGuildEvent())
    {
        SmsgCalendarEventInviteRemovedAlert managedPacket(calendarEvent->m_entry, calendarEvent->m_date, calendarEvent->m_flags, CALENDAR_STATUS_REMOVED);
        sendTo(sObjectMgr.getPlayer(invite->m_invitee), managedPacket);
    }

    sCalendarMgr.removeInvite(srlPacket.eventId, invite->m_inviteId);
}

void WorldSession::handleCalendarEventStatus(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_STATUS.");

    CmsgCalendarEventStatus srlPacket(false);
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (!canModerate(calendarEvent, _player->getGuidLow()))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    CalendarInvite* invite = sCalendarMgr.getInvite(srlPacket.eventId, srlPacket.inviteId);
    if (invite == nullptr || invite->m_invitee != srlPacket.inviteeGuid)
        invite = sCalendarMgr.getInviteOfPlayer(srlPacket.eventId, srlPacket.inviteeGuid);

    if (invite == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    invite->m_status = static_cast<CalendarInviteStatus>(srlPacket.value);
    sCalendarMgr.updateInvite(invite);

    sendInviteStatus(calendarEvent, invite);
    sendClearPending(sObjectMgr.getPlayer(invite->m_invitee));
}

void WorldSession::handleCalendarEventModeratorStatus(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_MODERATOR_STATUS.");

    CmsgCalendarEventStatus srlPacket(true);
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    if (!canModerate(calendarEvent, _player->getGuidLow()))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    CalendarInvite* invite = sCalendarMgr.getInvite(srlPacket.eventId, srlPacket.inviteId);
    if (invite == nullptr || invite->m_invitee != srlPacket.inviteeGuid)
        invite = sCalendarMgr.getInviteOfPlayer(srlPacket.eventId, srlPacket.inviteeGuid);

    if (invite == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    // the owner rank stays with the owner
    invite->m_rank = invite->m_invitee == calendarEvent->m_creator ? uint32_t(CALENDAR_RANK_OWNER) : std::min<uint32_t>(srlPacket.value, CALENDAR_RANK_MODERATOR);
    sCalendarMgr.updateInvite(invite);

    for (Player* player : eventRelatives(calendarEvent))
    {
        SmsgCalendarEventModeratorStatusAlert managedPacket(calendarEvent->m_entry, invite->m_invitee, static_cast<uint8_t>(invite->m_rank));
        sendTo(player, managedPacket);
    }
}

void WorldSession::handleCalendarEventInviteNotes(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_EVENT_INVITE_NOTES.");

    CmsgCalendarEventInviteNotes srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    // the id names the event of the invited player or the invite itself
    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(static_cast<uint32_t>(srlPacket.id));
    CalendarInvite* invite = calendarEvent != nullptr ? sCalendarMgr.getInviteOfPlayer(calendarEvent->m_entry, srlPacket.inviteeGuid) : nullptr;
    if (invite == nullptr)
    {
        for (auto* playerInvite : sCalendarMgr.getPlayerInvites(srlPacket.inviteeGuid))
        {
            if (playerInvite->m_inviteId == srlPacket.id)
            {
                invite = playerInvite;
                calendarEvent = sCalendarMgr.getEvent(playerInvite->m_event);
                break;
            }
        }
    }

    if (invite == nullptr || calendarEvent == nullptr)
    {
        sendCommandResult(_player, CALENDAR_ERROR_NO_INVITE);
        return;
    }

    // the own invite or, for a moderator, every invite of the event
    if (invite->m_invitee != _player->getGuidLow() && !canModerate(calendarEvent, _player->getGuidLow()))
    {
        sendCommandResult(_player, CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    invite->m_text = srlPacket.notes;
    sCalendarMgr.updateInvite(invite);

    for (Player* player : eventRelatives(calendarEvent))
    {
        SmsgCalendarEventInviteNotes managedPacket(invite->m_invitee, calendarEvent->m_entry, invite->m_text, player == _player);
        sendTo(player, managedPacket);
    }

    if (invite->m_invitee != _player->getGuidLow())
    {
        SmsgCalendarEventInviteNotesAlert managedPacket(calendarEvent->m_entry, invite->m_text);
        sendTo(sObjectMgr.getPlayer(invite->m_invitee), managedPacket);
    }
}

void WorldSession::handleSetSavedInstanceExtend(WorldPacket& recvPacket)
{
    CmsgSetSavedInstanceExtend srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_SET_SAVED_INSTANCE_EXTEND: map {}, difficulty {}, extend {}.", srlPacket.mapId, srlPacket.difficulty, srlPacket.extend);

    // a save in use stays as it is
    if (_player->GetMapId() == srlPacket.mapId || !InstanceDifficulty::isBindDifficulty(srlPacket.difficulty))
        return;

    const auto difficulty = static_cast<InstanceDifficulty::Difficulties>(srlPacket.difficulty);
    InstancePlayerBind* bind = _player->getBoundInstance(srlPacket.mapId, difficulty, srlPacket.extend);
    if (bind == nullptr || bind->save == nullptr || !bind->perm)
        return;

    InstanceSaved* save = bind->save;
    const auto expiry = [save](BindExtensionState state)
    {
        return state == EXTEND_STATE_EXTENDED ? sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime()) : save->getResetTime();
    };

    // an expired save comes back as a normal one
    const BindExtensionState oldState = bind->extendState;
    const BindExtensionState newState = srlPacket.extend && oldState != EXTEND_STATE_EXPIRED ? EXTEND_STATE_EXTENDED : EXTEND_STATE_NORMAL;
    _player->bindToInstance(save, true, newState, false);

    const time_t now = Util::getTimeNow();
    const auto oldSeconds = static_cast<int32_t>(std::max<time_t>(0, expiry(oldState) - now));
    const auto newSeconds = static_cast<int32_t>(std::max<time_t>(0, expiry(newState) - now));

    SmsgCalendarRaidLockoutUpdated managedPacket(now, srlPacket.mapId, srlPacket.difficulty, oldSeconds, newSeconds);
    sendManagedPacket(managedPacket);
}

void WorldSession::sendCalendarRaidLockout(InstanceSaved const* save, bool add)
{
    sLogger.debugOpcode("SMSG_CALENDAR_RAID_LOCKOUT_ADDED/REMOVED.");
    const auto now = Util::getTimeNow();
    time_t currTime = now;

    SmsgCalendarRaidLockoutRemoved managedPacket(add, currTime, uint32_t(save->getMapId()), uint32_t(save->getDifficulty()), uint32_t(save->getResetTime() - currTime), uint64_t(save->getInstanceId()));
    sendManagedPacket(managedPacket);
}

void WorldSession::sendCalendarRaidLockoutUpdated(InstanceSaved const* save)
{
    if (!save)
        return;

    sLogger.debugOpcode("SMSG_CALENDAR_RAID_LOCKOUT_UPDATED [{}] Map: {}, Difficulty {}.", _player->getGuid(), save->getMapId(), save->getDifficulty());

    const auto now = Util::getTimeNow();
    time_t currTime = now;

    const auto secondsLeft = static_cast<int32_t>(save->getResetTime() - currTime);
    SmsgCalendarRaidLockoutUpdated managedPacket(currTime, uint32_t(save->getMapId()), uint32_t(save->getDifficulty()), secondsLeft, secondsLeft);
    sendManagedPacket(managedPacket);
}
