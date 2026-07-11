/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

struct MultipleTalents
{
    uint32_t talentId;
    uint32_t talentRank;
};

namespace AscEmu::Packets
{
    class CmsgLearnTalentMultiple : public ManagedPacket
    {
    public:
        uint32_t talentCount;
        std::vector<MultipleTalents> multipleTalents;

        CmsgLearnTalentMultiple() : CmsgLearnTalentMultiple(0, {})
        {
        }

        CmsgLearnTalentMultiple(uint32_t talentCount, std::vector<MultipleTalents> multipleTalents) :
            ManagedPacket(CMSG_LEARN_TALENTS_MULTIPLE, 4),
            talentCount(talentCount),
            multipleTalents(multipleTalents)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> talentCount;

                MultipleTalents multiTalent{};

                for (uint32_t i = 0; i < talentCount; ++i)
                {
                    packet >> multiTalent.talentId >> multiTalent.talentRank;

                    multipleTalents.push_back(multiTalent);
                }
                return true;
            }

            return false;
        }
    };
}
