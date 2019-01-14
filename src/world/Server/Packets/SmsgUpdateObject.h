/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

#ifdef AE_TBC
namespace AscEmu
{
    namespace Packets
    {
        class SmsgUpdateObject : public ManagedPacket
        {
            std::vector<WoWGuid> m_out_of_range_guids;
            uint32_t m_creation_count;

        public:
            SmsgUpdateObject() : ManagedPacket(SMSG_UPDATE_OBJECT, sizeof(uint8_t) + sizeof(uint32_t))
            {
            }

        protected:
            size_t expectedSize() const override
            {
                return m_minimum_size;
            }

            bool internalSerialise(WorldPacket& packet) override
            {
                packet << ((m_out_of_range_guids.size() > 0) ? m_creation_count + 1 : m_creation_count);

                return true;
            }

            bool internalDeserialise(WorldPacket& /*packet*/) override
            {
                return true;
            }
        };
    }
}
#endif
