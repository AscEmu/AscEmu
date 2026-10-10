/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCharCreate : public ManagedPacket
    {
    public:
        CharCreate createStruct;

        CmsgCharCreate() : CmsgCharCreate(CharCreate())
        {
        }

        CmsgCharCreate(CharCreate createStruct) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            createStruct(createStruct)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isDragonflight())
            {
                // 10.x: name length, template set, trial boost, new player experience, hardcore self found; race, class,
                // sex; the customization choices; the timerunning season; the name; the template set; the choices
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

                for (uint32_t i = 0; i < customizationCount; ++i)
                {
                    packet.read<uint32_t>();            // option
                    packet.read<uint32_t>();            // choice
                }

                return !packet.hadReadFailure();
            }

            if (m_protocol.isShadowlands())
            {
                // name length, template set, trial boost, new player experience; race, class, sex; the customization
                // choices; the name; the template set; the choices are not stored yet
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

                for (uint32_t i = 0; i < customizationCount; ++i)
                {
                    packet.read<uint32_t>();            // option
                    packet.read<uint32_t>();            // choice
                }

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
