/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Storage/CharacterCustomizations.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgCharCreate : public ManagedPacket
    {
    public:
        CharCreate createStruct;
        // 9.x and later: the look as the customization choices of the client
        std::vector<CharacterCustomizations::ChrCustomizationChoice> customizations;

        CmsgCharCreate() : CmsgCharCreate(CharCreate())
        {
        }

        CmsgCharCreate(CharCreate createStruct) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            createStruct(createStruct)
        {
        }

    protected:
        void readCustomizations(WorldPacket& packet, uint32_t count)
        {
            if (count > 256)
                return;

            customizations.reserve(count);
            for (uint32_t i = 0; i < count; ++i)
            {
                CharacterCustomizations::ChrCustomizationChoice choice{};
                packet >> choice.optionId >> choice.choiceId;
                customizations.push_back(choice);
            }
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 10.x and 11.x: name length, template set, trial boost, new player experience, hardcore self found; race,
                // class, sex; the customization choices; the timerunning season; the name; the template set; the choices
                const uint32_t nameLength = packet.readBits(6);
                const bool hasTemplateSet = packet.readBit();
                packet.readBit();                       // trial boost
                packet.readBit();                       // new player experience
                packet.readBit();                       // hardcore self found
                packet.resetBitPos();

                packet >> createStruct._race >> createStruct._class >> createStruct.gender;
                const uint32_t customizationCount = packet.read<uint32_t>();
                packet.read<int32_t>();                 // timerunning season
                createStruct.skin = createStruct.face = createStruct.hairStyle = createStruct.hairColor = createStruct.facialHair = 0;
                createStruct.outfitId = 0;
                createStruct.name = packet.readString(nameLength);

                if (hasTemplateSet)
                    packet.read<int32_t>();

                readCustomizations(packet, customizationCount);

                return !packet.hadReadFailure();
            }

            if (m_protocol.isShadowlands())
            {
                // name length, template set, trial boost, new player experience; race, class, sex; the customization
                // choices; the name; the template set
                const uint32_t nameLength = packet.readBits(6);
                const bool hasTemplateSet = packet.readBit();
                packet.readBit();                       // trial boost
                packet.readBit();                       // new player experience
                packet.resetBitPos();

                packet >> createStruct._race >> createStruct._class >> createStruct.gender;
                const uint32_t customizationCount = packet.read<uint32_t>();
                createStruct.skin = createStruct.face = createStruct.hairStyle = createStruct.hairColor = createStruct.facialHair = 0;
                createStruct.outfitId = 0;
                createStruct.name = packet.readString(nameLength);

                if (hasTemplateSet)
                    packet.read<int32_t>();

                readCustomizations(packet, customizationCount);

                return !packet.hadReadFailure();
            }

            if (m_protocol.isWoD() || m_protocol.isLegion())
            {
                // name length, template set; the look; the name; the template set
                const uint32_t nameLength = packet.readBits(6);
                const bool hasTemplateSet = packet.readBit();
                packet.resetBitPos();

                packet >> createStruct._race >> createStruct._class >> createStruct.gender >> createStruct.skin >>
                    createStruct.face >> createStruct.hairStyle >> createStruct.hairColor >> createStruct.facialHair >>
                    createStruct.outfitId;
                createStruct.name = packet.readString(nameLength);

                if (hasTemplateSet)
                    packet.read<int32_t>();

                return !packet.hadReadFailure();
            }

            if (m_protocol.isBfA())
            {
                // name length, template set, trial boost; the look; custom display; the name; the template set
                const uint32_t nameLength = packet.readBits(6);
                const bool hasTemplateSet = packet.readBit();
                packet.readBit();                       // trial boost
                packet.resetBitPos();

                packet >> createStruct._race >> createStruct._class >> createStruct.gender >> createStruct.skin >>
                    createStruct.face >> createStruct.hairStyle >> createStruct.hairColor >> createStruct.facialHair >>
                    createStruct.outfitId;
                packet.read<uint8_t>();
                packet.read<uint8_t>();
                packet.read<uint8_t>();
                createStruct.name = packet.readString(nameLength);

                if (hasTemplateSet)
                    packet.read<int32_t>();

                return true;
            }

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
    };
}
