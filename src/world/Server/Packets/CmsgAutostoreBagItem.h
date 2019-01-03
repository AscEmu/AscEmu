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
    class CmsgAutostoreBagItem : public ManagedPacket
    {
    public:
        int8_t srcContainerSlot;
        int8_t srcSlot;
        int8_t dstContainerSlot;

        CmsgAutostoreBagItem() : CmsgAutostoreBagItem(0, 0, 0)
        {
        }

        CmsgAutostoreBagItem(int8_t srcContainerSlot, int8_t srcSlot, int8_t dstContainerSlot) :
            ManagedPacket(CMSG_AUTOSTORE_BAG_ITEM, 3),
            srcContainerSlot(srcContainerSlot),
            srcSlot(srcSlot),
            dstContainerSlot(dstContainerSlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> srcContainerSlot >> srcSlot >> dstContainerSlot;
            return true;
        }
    };
}}
