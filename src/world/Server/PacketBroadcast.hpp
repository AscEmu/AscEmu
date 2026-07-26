/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Management/Guild/Guild.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"

#include <shared_mutex>
#include <type_traits>

class World;
class WorldSession;

namespace AscEmu::Packets
{
    class PacketBroadcast final
    {
    public:
        PacketBroadcast() = delete;

        template <typename TPacket>
        static void sendToSet(Object& source, TPacket& packet, bool sendToSelf = false,
            bool ownTeamOnly = false, bool isChatMessage = false)
        {
            if (!source.IsInWorld())
                return;

            if (source.isPlayer())
            {
                sendFromPlayer(static_cast<Player&>(source), packet, sendToSelf,
                    ownTeamOnly, isChatMessage);

                return;
            }

            sendFromObject(source, packet);
        }

        template <typename TPacket>
        static void sendFromChannel(Channel& source, TPacket& packet, Player* skipPlayer = nullptr)
        {
            std::lock_guard<std::mutex> guard(source.m_mutexChannel);

            for (const auto& memberEntry : source.m_members)
            {
                Player* targetPlayer = memberEntry.first;
                if (targetPlayer == nullptr || targetPlayer == skipPlayer)
                    continue;

                WorldSession* targetSession = targetPlayer->getSession();
                if (targetSession == nullptr)
                    continue;

                targetSession->sendManagedPacket(packet);
            }
        }

        template <typename TPacket>
        static void sendFromGuild(Guild const& source, TPacket& packet)
        {
            for (const auto& guildMember : source.getGuildMembers())
            {
                Player* targetPlayer = guildMember.second->getPlayerByGuid(guildMember.second->getGUID());
                if (targetPlayer == nullptr)
                    continue;

                sendToPlayer(targetPlayer, packet);
            }
        }

        template <typename TPacket>
        static void sendFromGroup(Group const& source, TPacket& packet, Player* skipPlayer = nullptr)
        {
            for (uint32_t i = 0; i < source.GetSubGroupCount(); ++i)
            {
                for (auto groupMember : source.GetSubGroup(i)->getGroupMembers())
                {
                    Player* targetPlayer = sObjectMgr.getPlayer(groupMember->guid);
                    if (targetPlayer == nullptr)
                        continue;

                    if (targetPlayer == skipPlayer)
                        continue;

                    sendToPlayer(targetPlayer, packet);
                }
            }
        }

        template <typename TSource, typename TPacket>
        static void sendFromZone(TSource& source, TPacket& packet, uint32_t zoneId, WorldSession* skipSession = nullptr)
        {
            static_assert(
                std::is_same_v<std::remove_cv_t<TSource>, World>,
                "PacketBroadcast::sendFromZone requires World as source."
                );

            std::lock_guard<std::mutex> guard(source.mSessionLock);

            for (const auto& sessionEntry : source.mActiveSessionMapStore)
            {
                auto* targetSession = sessionEntry.second.get();

                if (targetSession == nullptr || targetSession == skipSession)
                    continue;

                auto* targetPlayer = targetSession->GetPlayer();

                if (targetPlayer == nullptr || !targetPlayer->IsInWorld())
                    continue;

                if (targetPlayer->getZoneId() != zoneId)
                    continue;

                targetSession->sendManagedPacket(packet);
            }
        }

    private:
        template <typename TPacket>
        static void sendToPlayer(Player* targetPlayer, TPacket& packet)
        {
            if (targetPlayer == nullptr)
                return;

            WorldSession* targetSession = targetPlayer->getSession();
            if (targetSession == nullptr)
                return;

            targetSession->sendManagedPacket(packet);
        }

        template <typename TPacket>
        static void sendFromPlayer(Player& sourcePlayer, TPacket& packet, bool sendToSelf,
            bool ownTeamOnly, bool isChatMessage)
        {
            WorldSession* sourceSession = sourcePlayer.getSession();
            if (sendToSelf && sourceSession != nullptr)
                sourceSession->sendManagedPacket(packet);

            for (const auto& inRangeObject : sourcePlayer.getInRangePlayersSet())
            {
                Player* targetPlayer = static_cast<Player*>(inRangeObject);
                if (targetPlayer == nullptr)
                    continue;

                WorldSession* targetSession = targetPlayer->getSession();
                if (targetSession == nullptr)
                    continue;

                if (ownTeamOnly && targetPlayer->getTeam() != sourcePlayer.getTeam())
                    continue;

                if ((targetPlayer->GetPhase() & sourcePlayer.GetPhase()) == 0)
                    continue;

                if (isChatMessage)
                {
                    if (!targetPlayer->isIgnored(sourcePlayer.getGuidLow()))
                        targetSession->sendManagedPacket(packet);

                    continue;
                }

                if (sourcePlayer.m_isGmInvisible && !targetSession->hasPermissions())
                    continue;

                if (targetPlayer->isVisibleObject(sourcePlayer.getGuid()))
                    targetSession->sendManagedPacket(packet);
            }
        }

        template <typename TPacket>
        static void sendFromObject(Object& source, TPacket& packet)
        {
            for (const auto& inRangeObject : source.getInRangePlayersSet())
            {
                Player* targetPlayer = static_cast<Player*>(inRangeObject);
                if (targetPlayer == nullptr)
                    continue;

                WorldSession* targetSession = targetPlayer->getSession();
                if (targetSession == nullptr)
                    continue;

                if ((targetPlayer->GetPhase() & source.GetPhase()) == 0)
                    continue;

                targetSession->sendManagedPacket(packet);
            }
        }
    };
}
