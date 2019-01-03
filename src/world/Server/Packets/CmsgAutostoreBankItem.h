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
    class CmsgAutostoreBankItem : public ManagedPacket
    {
    public:
        int8_t srcInventorySlot;
        int8_t srcSlot;

        CmsgAutostoreBankItem() : CmsgAutostoreBankItem(0, 0)
        {
        }

        CmsgAutostoreBankItem(int8_t srcInventorySlot, int8_t srcSlot) :
            ManagedPacket(CMSG_AUTOSTORE_BANK_ITEM, 2),
            srcInventorySlot(srcInventorySlot),
            srcSlot(srcSlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> srcInventorySlot >> srcSlot;
            return true;
        }
    };
}}
