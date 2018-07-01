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
        CharCreate charCreateContent;

        CmsgCharCreate() : CmsgCharCreate(CharCreate())
        {
        }

        CmsgCharCreate(CharCreate charCreateContent) :
            ManagedPacket(CMSG_CHAR_CREATE, 10),
            charCreateContent(charCreateContent)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> charCreateContent.name >> charCreateContent._race >> charCreateContent._class >>
                charCreateContent.gender >> charCreateContent.skin >> charCreateContent.face >> charCreateContent.hairStyle >>
                charCreateContent.hairColor >> charCreateContent.facialHair >> charCreateContent.outfitId;

            return true;
        }
    };
}}
