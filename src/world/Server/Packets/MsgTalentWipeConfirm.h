/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgTalentWipeConfirm : public ManagedPacket
    {
    public:
        uint64_t playerGuid;
        uint32_t talentResetCost;

        MsgTalentWipeConfirm() : MsgTalentWipeConfirm(0, 0)
        {
        }

        MsgTalentWipeConfirm(uint64_t playerGuid, uint32_t talentResetCost) :
            ManagedPacket(MSG_TALENT_WIPE_CONFIRM, 0),
            playerGuid(playerGuid),
            talentResetCost(talentResetCost)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerGuid << talentResetCost;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
