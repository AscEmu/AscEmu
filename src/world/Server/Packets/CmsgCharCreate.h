/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Objects/Units/Players/PlayerDefines.hpp"

#include <algorithm>
#include <cstdint>
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
        bool deserialiseForever(WorldPacket& packet)
        {
            // Forever 1.60.1 current character-create layout.
            // The first 18 bits contain string lengths / flags, followed by the
            // byte-aligned character data, strings, optional template set and
            // customization choices.
            if (packet.size() < 18U)
                return false;

            const uint32_t nameLength = packet.readBits(6);
            const bool hasTemplateSet = packet.readBit();
            packet.readBit(); // IsTrialBoost
            packet.readBit(); // UseNPE
            packet.readBit(); // HardcoreSelfFound
            packet.readBits(2); // Unknown
            const uint32_t surnameLength = packet.readBits(6);

            if (nameLength == 0U || nameLength > 63U || surnameLength > 63U)
                return false;

            // 18 bits consume exactly three bytes. Primitive reads continue at
            // the next byte because ByteBuffer::readBit() already advanced rpos().
            if (packet.remaining() < 15U)
                return false;

            packet >> createStruct._race;
            packet >> createStruct._class;
            packet >> createStruct.gender;

            uint32_t customizationCount = 0;
            packet >> customizationCount;

            int32_t unknown = 0;
            packet >> unknown;
            (void)unknown;

            packet >> timerunningSeasonId;

            if (customizationCount > 250U)
                return false;

            const size_t requiredTail =
                static_cast<size_t>(nameLength) +
                static_cast<size_t>(surnameLength) +
                (hasTemplateSet ? sizeof(uint32_t) : 0U) +
                static_cast<size_t>(customizationCount) * 8U;

            if (packet.remaining() < requiredTail)
                return false;

            createStruct.name = packet.readString(nameLength);

            // AscEmu does not persist character surnames yet, but the bytes must
            // still be consumed to keep the remainder of the request aligned.
            if (surnameLength != 0U)
                packet.readString(surnameLength);

            templateSet = 0U;
            if (hasTemplateSet)
                packet >> templateSet;

            createStruct.skin = 0;
            createStruct.face = 0;
            createStruct.hairStyle = 0;
            createStruct.hairColor = 0;
            createStruct.facialHair = 0;
            createStruct.outfitId = 0;

            customizations.clear();
            customizations.reserve(customizationCount);

            for (uint32_t i = 0; i < customizationCount; ++i)
            {
                CharacterCustomizationChoice customization;
                packet >> customization.optionId;
                packet >> customization.choiceId;
                customizations.emplace_back(customization);
            }

            std::sort(customizations.begin(), customizations.end(),
                [](const CharacterCustomizationChoice& left, const CharacterCustomizationChoice& right)
                {
                    return left.optionId < right.optionId;
                });

            return !packet.hadReadFailure();
        }

    };
}
