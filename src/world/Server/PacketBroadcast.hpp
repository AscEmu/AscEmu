/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Management/Guild/Guild.hpp"

#include <shared_mutex>
#include <type_traits>

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
        static void sendFromGuild(Guild const& source, TPacket& packet)
        {
            for (const auto& guildMember : source.getGuildMembers())
            {
                Player* targetPlayer = guildMember.second->getPlayerByGuid(guildMember.second->getGUID());
                if (targetPlayer == nullptr)
                    continue;

                WorldSession* targetSession = targetPlayer->getSession();
                if (targetSession == nullptr)
                    continue;

                targetSession->sendManagedPacket(packet);
            }
        }

    private:
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
