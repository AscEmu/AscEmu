/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

struct PlayerStablePetList
{
    uint32_t petNumber;
    uint32_t entry;
    uint32_t level;
    uint8_t stableState;
    std::string name;
};

namespace AscEmu { namespace Packets
{
    class MsgListStabledPets : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t listSize;
        uint8_t slotCount;
        std::vector<PlayerStablePetList> stableList;

        MsgListStabledPets() : MsgListStabledPets(0, 0, 0, {})
        {
        }

        MsgListStabledPets(uint64_t guid, uint8_t listSize, uint8_t slotCount, std::vector<PlayerStablePetList> stableList) :
            ManagedPacket(MSG_LIST_STABLED_PETS, 10 + listSize * 25),
            guid(guid),
            listSize(listSize),
            slotCount(slotCount),
            stableList(stableList)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.GetOldGuid() << listSize << slotCount;
            for (const auto& stablePet : stableList)
                packet << stablePet.petNumber << stablePet.entry << stablePet.level << stablePet.name << stablePet.stableState;

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
