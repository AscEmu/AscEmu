/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Management/EquipmentSetMgr.h"

namespace AscEmu::Packets
{
    class SmsgEquipmentSetList : public ManagedPacket
    {
    public:
        const EquipmentSetStorage& equipmentSetStore;

        SmsgEquipmentSetList(const EquipmentSetStorage& equipmentSetStore) :
            ManagedPacket(SMSG_EQUIPMENT_SET_LIST, 0),
            equipmentSetStore(equipmentSetStore)
        {
        }

    protected:
        size_t expectedSize() const override { return equipmentSetStore.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_TBC && !m_protocol.isMop())
            {
                packet << uint32_t(equipmentSetStore.size());

                for (const auto& [setGuid, set] : equipmentSetStore)
                {
                    if (!set)
                        continue;

                    packet << WoWGuid(uint64_t(set->setGuid));
                    packet << uint32_t(set->setId);
                    packet << std::string(set->setName);
                    packet << std::string(set->iconName);

                    for (uint32_t i = 0; i < set->itemGuid.size(); ++i)
                    {
                        packet << WoWGuid(WoWGuid::createItemGuid(set->itemGuid[i]));
                    }
                }
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
