/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Server/Script/ScriptMgr.hpp"

namespace AscEmu::Packets
{
    class SmsgUpdateInstanceEncounterUnit : public ManagedPacket
    {
    public:
        uint32_t type;
        WoWGuid guid;
        uint8_t valueA;
        uint8_t valueB;

        SmsgUpdateInstanceEncounterUnit() : SmsgUpdateInstanceEncounterUnit(0, WoWGuid(), 0, 0)
        {
        }

        SmsgUpdateInstanceEncounterUnit(uint32_t type, WoWGuid guid, uint8_t valueA, uint8_t valueB) :
            ManagedPacket(SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT, 13),
            type(type), guid(guid), valueA(valueA), valueB(valueB)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type;

            switch (type)
            {
                case EncounterFrameEngage:
                case EncounterFrameDisengaged:
                case EncounterFrameUpdatePriority:
                {
                    if (guid)
                    {
                        packet << guid << valueA;
                    }
                } break;

                case EncounterFrameAddTimer:
                case EncounterFrameEnableObjective:
                case EncounterFrameDisableObjective:
#if VERSION_STRING > WotLK
                case EncounterFrameSetCombatResLimit:
#endif
                {
                    packet << valueA;
                } break;

                case EncounterFrameUpdateObjective:
                {
                    packet << valueA << valueB;
                } break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
