/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Server/Script/ScriptMgr.hpp"

#include <cstdint>
#include <optional>

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

        static std::optional<uint32_t> getEncounterFrameTypeValue(uint32_t type, WoW::ClientProtocol const& protocol)
        {
            if (protocol.expansion > WoW::Expansion::_WotLK)
            {
                switch (type)
                {
                    case EncounterFrameType::EncounterFrameSetCombatResLimit:
                        return 0;

                    case EncounterFrameType::EncounterFrameResetCombatResLimit:
                        return 1;

                    case EncounterFrameType::EncounterFrameEngage:
                        return 2;

                    case EncounterFrameType::EncounterFrameDisengaged:
                        return 3;

                    case EncounterFrameType::EncounterFrameUpdatePriority:
                        return 4;

                    case EncounterFrameType::EncounterFrameAddTimer:
                        return 5;

                    case EncounterFrameType::EncounterFrameEnableObjective:
                        return 6;

                    case EncounterFrameType::EncounterFrameUpdateObjective:
                        return 7;

                    case EncounterFrameType::EncounterFrameDisableObjective:
                        return 8;

                    case EncounterFrameType::EncounterFrameUnknown:
                        return 9;

                    case EncounterFrameType::EncounterFrameAddCombatResLimit:
                        return 10;
                }
            }
            else
            {
                switch (type)
                {
                    case EncounterFrameType::EncounterFrameEngage:
                        return 0;

                    case EncounterFrameType::EncounterFrameDisengaged:
                        return 1;

                    case EncounterFrameType::EncounterFrameUpdatePriority:
                        return 2;

                    case EncounterFrameType::EncounterFrameAddTimer:
                        return 3;

                    case EncounterFrameType::EncounterFrameEnableObjective:
                        return 4;

                    case EncounterFrameType::EncounterFrameUpdateObjective:
                        return 5;

                    case EncounterFrameType::EncounterFrameDisableObjective:
                        return 6;

                    case EncounterFrameType::EncounterFrameUnknown:
                        return 7;

                    case EncounterFrameType::EncounterFrameSetCombatResLimit:
                    case EncounterFrameType::EncounterFrameResetCombatResLimit:
                    case EncounterFrameType::EncounterFrameAddCombatResLimit:
                        return std::nullopt;
                }
            }

            return std::nullopt;
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            auto const encodedType = getEncounterFrameTypeValue(type, m_protocol);

            if (!encodedType)
                return false;

            packet << *encodedType;

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
                {
                    packet << valueA;
                } break;
                case EncounterFrameSetCombatResLimit:
                {
                    if (m_protocol.expansion > WoW::Expansion::_WotLK)
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
