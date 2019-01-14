/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgComplaint : public ManagedPacket
    {
#if VERSION_STRING < Cata
    public:
        // 0 - mail, 1 - chat
        uint8_t spam_type;
        WoWGuid spammer_guid;
        uint32_t unk1;
        uint32_t unk2;
        uint32_t unk3;
        uint32_t unk4;
        std::string description;

        CmsgComplaint() : CmsgComplaint(0, 0, 0, 0, 0, 0, "")
        {
        }

        CmsgComplaint(uint8_t spam_type, uint64_t spammer_guid, uint32_t unk1, uint32_t unk2,
            uint32_t unk3, uint32_t unk4, std::string description) :
            ManagedPacket(CMSG_COMPLAIN, 0),
            spam_type(spam_type),
            spammer_guid(spammer_guid),
            unk1(unk1),
            unk2(unk2),
            unk3(unk3),
            unk4(unk4),
            description(description)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> spam_type >> unpacked_guid;
            spammer_guid.Init(unpacked_guid);

            if (spam_type == 0)
                packet >> unk1 >> unk2 >> unk3;
            else
                packet >> unk1 >> unk2 >> unk3 >> unk4 >> description;

            return true;
        }
#endif
    };
}}
