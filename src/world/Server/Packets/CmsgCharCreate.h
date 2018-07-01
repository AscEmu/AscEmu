/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgCharCreate : public ManagedPacket
    {
    public:
        std::string name;
        uint8_t _race;
        uint8_t _class;
        uint8_t gender;
        uint8_t skin;
        uint8_t face;
        uint8_t hairStyle;
        uint8_t hairColor;
        uint8_t facialHair;
        uint8_t outfitId;

        CmsgCharCreate() : CmsgCharCreate("", 0, 0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        CmsgCharCreate(std::string name, uint8_t _race, uint8_t _class, uint8_t gender, uint8_t skin, uint8_t face,
            uint8_t hairStyle, uint8_t hairColor, uint8_t facialHair, uint8_t outfitId) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            name(name),
            _race(_race),
            _class(_class),
            gender(gender),
            skin(skin),
            face(face),
            hairStyle(hairStyle),
            hairColor(hairColor),
            facialHair(facialHair),
            outfitId(outfitId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> _race >> _class >> gender >> skin >> face >> hairStyle >> hairColor >> facialHair >> outfitId;

            return true;
        }
    };
}}
