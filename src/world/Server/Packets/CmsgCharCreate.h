/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Objects/Units/Players/PlayerDefines.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct CharacterCustomizationChoice
    {
        uint32_t optionId{0};
        uint32_t choiceId{0};
    };

    class CmsgCharCreate : public ManagedPacket
    {
    public:
        CharCreate createStruct{};
        uint32_t timerunningSeasonId{0};
        uint32_t templateSet{0};
        std::vector<CharacterCustomizationChoice> customizations;

        CmsgCharCreate() : CmsgCharCreate(CharCreate())
        {
        }

        explicit CmsgCharCreate(CharCreate createStruct) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            createStruct(std::move(createStruct))
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
                return deserialiseForever(packet);

            if (!m_protocol.isMop())
            {
                packet >> createStruct.name >> createStruct._race >> createStruct._class >>
                    createStruct.gender >> createStruct.skin >> createStruct.face >> createStruct.hairStyle >>
                    createStruct.hairColor >> createStruct.facialHair >> createStruct.outfitId;
            }
            else // Mop
            {
                packet >> createStruct.outfitId >> createStruct.hairStyle >> createStruct._class >>
                    createStruct.skin >> createStruct.face >> createStruct._race >> createStruct.facialHair >>
                    createStruct.gender >> createStruct.hairColor;

                const auto nameLength = packet.readBits(6);
                uint8_t unknown = packet.readBit();
                createStruct.name = packet.readString(nameLength);

                if (unknown)
                    packet.read<uint32_t>();

                packet.rpos(0);
            }

            return true;
        }

    private:
        static uint32_t readUInt32LE(const uint8_t* data)
        {
            uint32_t value = 0;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }

        bool deserialiseForever(WorldPacket& packet)
        {
            constexpr size_t FixedPrefix = 18U;
            const uint8_t* payload = packet.contents();
            const size_t payloadSize = packet.size();

            if (payload == nullptr || payloadSize < FixedPrefix)
                return false;

            createStruct._race = payload[3];
            createStruct._class = payload[4];
            createStruct.gender = payload[5];
            const uint32_t customizationCount = readUInt32LE(payload + 6);
            timerunningSeasonId = readUInt32LE(payload + 10);
            templateSet = readUInt32LE(payload + 14);

            if (customizationCount > 250U)
                return false;

            const size_t customizationBytes = static_cast<size_t>(customizationCount) * 8U;
            if (payloadSize < FixedPrefix + customizationBytes)
                return false;

            const size_t nameLength = payloadSize - FixedPrefix - customizationBytes;
            if (nameLength == 0U || nameLength > 63U)
                return false;

            createStruct.name.assign(reinterpret_cast<const char*>(payload + FixedPrefix), nameLength);
            createStruct.skin = 0;
            createStruct.face = 0;
            createStruct.hairStyle = 0;
            createStruct.hairColor = 0;
            createStruct.facialHair = 0;
            createStruct.outfitId = 0;

            customizations.clear();
            customizations.reserve(customizationCount);

            const uint8_t* customizationData = payload + FixedPrefix + nameLength;
            for (uint32_t i = 0; i < customizationCount; ++i)
            {
                CharacterCustomizationChoice customization;
                customization.optionId = readUInt32LE(customizationData + static_cast<size_t>(i) * 8U);
                customization.choiceId = readUInt32LE(customizationData + static_cast<size_t>(i) * 8U + 4U);
                customizations.emplace_back(customization);
            }

            std::sort(customizations.begin(), customizations.end(), [](const CharacterCustomizationChoice& left, const CharacterCustomizationChoice& right) { return left.optionId < right.optionId; });
            packet.rfinish();
            return true;
        }
    };
}
