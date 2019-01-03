/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Spell/SpellAuras.h"
#include "Units/Creatures/Pet.h"

namespace AscEmu { namespace Packets
{
    class SmsgPartyMemberStatsFull : public ManagedPacket
    {
    public:
        uint64_t guid;
        Player* player;

        SmsgPartyMemberStatsFull() : SmsgPartyMemberStatsFull(0, nullptr)
        {
        }

        SmsgPartyMemberStatsFull(uint64_t guid, Player* player) :
            ManagedPacket(SMSG_PARTY_MEMBER_STATS_FULL, 9),
            guid(guid),
            player(player)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint8_t(0);

            if (player == nullptr)
            {
                packet << guid << uint32_t(GROUP_UPDATE_FLAG_STATUS) << uint16_t(MEMBER_STATUS_OFFLINE);
            }
            else
            {
                packet << player->getGuid();

                auto playerPet = player->GetSummon();
                if (playerPet)
                    packet << uint32_t(0x7FFFFFFF);
                else
                    packet << uint32_t(0x00040BFF);

                packet << uint16_t(MEMBER_STATUS_ONLINE);
                packet << uint32_t(player->getHealth());
                packet << uint32_t(player->getMaxHealth());
                packet << uint8_t(player->getPowerType());
                packet << uint16_t(player->getPower(player->getPowerType()));
                packet << uint16_t(player->getMaxPower(player->getPowerType()));
                packet << uint16_t(player->getLevel());
                packet << uint16_t(player->GetZoneId());
                packet << uint16_t(player->GetPositionX());
                packet << uint16_t(player->GetPositionY());

                uint64_t auramask = 0;
                const auto maskPos = packet.wpos();
                packet << uint64_t(auramask);
                for (uint8_t i = 0; i < 64; ++i)
                {
                    if (const auto aurApp = player->GetAuraWithSlot(i))
                    {
                        auramask |= (uint64_t(1) << i);
                        packet << uint32_t(aurApp->GetSpellId());
                        packet << uint8_t(1);
                    }
                }
                packet.put<uint64_t>(maskPos, auramask);

                if (playerPet)
                {
                    const uint8_t petPowerType = playerPet->getPowerType();
                    packet << uint64_t(playerPet->getGuid());
                    packet << playerPet->GetName();
                    packet << uint16_t(playerPet->getDisplayId());
                    packet << uint32_t(playerPet->getHealth());
                    packet << uint32_t(playerPet->getMaxHealth());
                    packet << uint8_t(petPowerType);
                    packet << uint16_t(playerPet->getPower(petPowerType));
                    packet << uint16_t(playerPet->getMaxPower(petPowerType));

                    uint64_t petauramask = 0;
                    const auto petMaskPos = packet.wpos();
                    packet << uint64_t(petauramask);
                    for (uint8_t i = 0; i < 64; ++i)
                    {
                        if (const auto auraApp = playerPet->GetAuraWithSlot(i))
                        {
                            petauramask |= (uint64_t(1) << i);
                            packet << uint32_t(auraApp->GetSpellId());
                            packet << uint8_t(1);
                        }
                    }
                    packet.put<uint64_t>(petMaskPos, petauramask);
                }

                //\todo vehicle!
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
