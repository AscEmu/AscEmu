/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Group.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/SpellAura.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPartyMemberStats : public ManagedPacket
    {
    public:
        WoWGuid memberGuid;
        uint32_t mask {0};
        Player* playerMember {nullptr};

        SmsgPartyMemberStats(WoWGuid memberGuid, uint32_t mask) :
            ManagedPacket(SMSG_PARTY_MEMBER_STATS, 0),
            memberGuid(memberGuid),
            mask(mask)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            Player* player = playerMember;
            if (player == nullptr)
                return false;

            if (m_protocol.isMop())
            {
                packet.writeBit(memberGuid[0]);
                packet.writeBit(memberGuid[5]);

                packet.writeBit(0);                 // unk

                packet.writeBit(memberGuid[1]);
                packet.writeBit(memberGuid[4]);

                packet.writeBit(1);                 // unk2 full update?

                packet.writeBit(memberGuid[6]);
                packet.writeBit(memberGuid[2]);
                packet.writeBit(memberGuid[7]);
                packet.writeBit(memberGuid[3]);

                packet.flushBits();

                packet.writeByteSeq(memberGuid[3]);
                packet.writeByteSeq(memberGuid[2]);
                packet.writeByteSeq(memberGuid[6]);
                packet.writeByteSeq(memberGuid[7]);
                packet.writeByteSeq(memberGuid[5]);

                packet << mask;

                packet.writeByteSeq(memberGuid[1]);
                packet.writeByteSeq(memberGuid[4]);
                packet.writeByteSeq(memberGuid[0]);
            }
            else
            {
                packet << memberGuid;
                packet << mask;
            }

            ByteBuffer buffer;

            if (mask & GROUP_UPDATE_FLAG_STATUS)
            {
                if (!player->m_isGmInvisible)
                    buffer << uint16_t(player->getGroupStatus());
                else
                    buffer << uint16_t(MEMBER_STATUS_OFFLINE);
            }

            if (mask & GROUP_UPDATE_FLAG_CUR_HP)
                buffer << uint32_t(player->getHealth());

            if (mask & GROUP_UPDATE_FLAG_MAX_HP)
                buffer << uint32_t(player->getMaxHealth());

            auto powerType = player->getPowerType();
            if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
                buffer << uint8_t(powerType);

            if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
                buffer << uint16_t(player->getPower(powerType));

            if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
                buffer << uint16_t(player->getMaxPower(powerType));

            if (mask & GROUP_UPDATE_FLAG_LEVEL)
                buffer << uint16_t(player->getLevel());

            if (mask & GROUP_UPDATE_FLAG_ZONE)
                buffer << uint16_t(player->getZoneId());

            if (mask & GROUP_UPDATE_FLAG_POSITION)
            {
                buffer << uint16_t(player->GetPositionX());
                buffer << uint16_t(player->GetPositionY());
            }

            if (!m_protocol.isMop())
            {
                if (mask & GROUP_UPDATE_FLAG_AURAS)
                {
                    uint64_t auramask = player->getAuraUpdateMaskForRaid();
                    buffer << uint64_t(auramask);
                    for (uint8_t i = 0; i < 64; ++i)
                    {
                        if (auramask & (uint64_t(1) << i))
                        {
                            Aura* aurApp = player->getAuraWithVisualSlot(i);
                            buffer << uint32_t(aurApp ? aurApp->getSpellId() : 0);
                            buffer << uint8_t(1);
                        }
                    }
                }
            }

            Pet* pet = player->getPet();
            if (mask & GROUP_UPDATE_FLAG_PET_GUID)
                buffer << uint64_t(pet ? pet->getGuid() : 0);

            if (mask & GROUP_UPDATE_FLAG_PET_NAME)
            {
                if (pet)
                    buffer << pet->getName().c_str();
                else
                    buffer << uint8_t(0);
            }

            if (mask & GROUP_UPDATE_FLAG_PET_MODEL_ID)
                buffer << uint16_t(pet ? pet->getDisplayId() : 0);


            if (mask & GROUP_UPDATE_FLAG_PET_CUR_HP)
                buffer << uint32_t(pet ? pet->getHealth() : 0);


            if (mask & GROUP_UPDATE_FLAG_PET_MAX_HP)
                buffer << uint32_t(pet ? pet->getMaxHealth() : 0);


            if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)
                buffer << uint8_t(pet ? pet->getPowerType() : 0);

            if (mask & GROUP_UPDATE_FLAG_PET_CUR_POWER)
                buffer << uint16_t(pet ? pet->getPower(pet->getPowerType()) : 0);


            if (mask & GROUP_UPDATE_FLAG_PET_MAX_POWER)
                buffer << uint16_t(pet ? pet->getMaxPower(pet->getPowerType()) : 0);


            if (mask & GROUP_UPDATE_FLAG_VEHICLE_SEAT)
            {
#ifdef FT_VEHICLES
                if (Vehicle* veh = player->getVehicleKit())
                    buffer << uint32_t(veh->getVehicleInfo()->seatID[player->getMovementInfo()->transport_seat]);
                else
                    buffer << uint32_t(0);
#endif
            }

            if (!m_protocol.isMop())
            {
                if (mask & GROUP_UPDATE_FLAG_PET_AURAS)
                {
                    if (pet)
                    {
                        uint64_t auramask = pet->getAuraUpdateMaskForRaid();
                        buffer << uint64_t(auramask);
                        for (uint8_t i = 0; i < 64; ++i)
                        {
                            if (auramask & (uint64_t(1) << i))
                            {
                                Aura* aurApp = pet->getAuraWithVisualSlot(i);
                                buffer << uint32_t(aurApp ? aurApp->getSpellId() : 0);
                                buffer << uint8_t(1);
                            }
                        }
                    }
                    else
                    {
                        buffer << uint64_t(0);
                    }
                }
            }

            if (m_protocol.isMop())
                packet << uint32_t(buffer.size());

            packet.append(buffer);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
