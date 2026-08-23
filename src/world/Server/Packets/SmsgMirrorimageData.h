/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgMirrorimageData : public ManagedPacket
    {
    public:
        uint64_t guid {0};
        Unit* caster {nullptr};

        SmsgMirrorimageData() : SmsgMirrorimageData(0, nullptr)
        {
        }

        SmsgMirrorimageData(uint64_t guid, Unit* caster) :
            ManagedPacket(SMSG_MIRRORIMAGE_DATA, 68),
            guid(guid),
            caster(caster)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(guid) + 4 + 1 + 1 + 1 + 5 + 8 + (11 * 4); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (caster == nullptr)
                return false;

            if (!m_protocol.isMop())
            {
                packet << uint64_t(guid);
                packet << uint32_t(caster->getDisplayId());
                packet << uint8_t(caster->getRace());
                packet << uint8_t(caster->getGender());
                packet << uint8_t(caster->getClass());

                if (caster->isPlayer())
                {
                    if (const auto pcaster = dynamic_cast<Player*>(caster))
                    {
                        // facial features
                        packet << uint8_t(pcaster->getSkinColor());
                        packet << uint8_t(pcaster->getFace());
                        packet << uint8_t(pcaster->getHairStyle());
                        packet << uint8_t(pcaster->getHairColor());
                        packet << uint8_t(pcaster->getFacialFeatures());

                        if (pcaster->isInGuild())
                            packet << uint64_t(pcaster->getGuild()->getGUID());
                        else
                            packet << uint64_t(0);

                        static const uint32_t imageitemslots[] =
                        {
                            EQUIPMENT_SLOT_HEAD,
                            EQUIPMENT_SLOT_SHOULDERS,
                            EQUIPMENT_SLOT_BODY,
                            EQUIPMENT_SLOT_CHEST,
                            EQUIPMENT_SLOT_WAIST,
                            EQUIPMENT_SLOT_LEGS,
                            EQUIPMENT_SLOT_FEET,
                            EQUIPMENT_SLOT_WRISTS,
                            EQUIPMENT_SLOT_HANDS,
                            EQUIPMENT_SLOT_BACK,
                            EQUIPMENT_SLOT_TABARD
                        };

                        for (uint8_t i = 0; i < 11; ++i)
                        {
                            Item* item = pcaster->getItemInterface()->GetInventoryItem(static_cast <int16_t>(imageitemslots[i]));
                            if (item != nullptr)
                                packet << uint32_t(item->getItemProperties()->DisplayInfoID);
                            else
                                packet << uint32_t(0);
                        }
                    }
                }
                else // do not send player data for creatures
                {
                    packet << uint8_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
