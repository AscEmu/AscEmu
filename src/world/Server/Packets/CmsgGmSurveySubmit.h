/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

struct GmSubServey
{
    uint32_t subSurveyId;
    uint8_t answerId;
    std::string comment;
};

namespace AscEmu { namespace Packets
{
    class CmsgGmSurveySubmit : public ManagedPacket
    {
    public:
        uint32_t mainSurveyId;
        std::vector<GmSubServey> subSurvey;
        std::string mainComment;

        CmsgGmSurveySubmit() : CmsgGmSurveySubmit(0, { }, "")
        {
        }

        CmsgGmSurveySubmit(uint32_t mainSurveyId, std::vector<GmSubServey> subSurvey, std::string mainComment) :
            ManagedPacket(CMSG_GMSURVEY_SUBMIT, 0),
            mainSurveyId(mainSurveyId),
            subSurvey(subSurvey),
            mainComment(mainComment)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> mainSurveyId;
            for (int i = 0; i < 10; ++i)
            {
                GmSubServey gmSurvey;
                packet >> gmSurvey.subSurveyId >> gmSurvey.answerId >> gmSurvey.comment;

                subSurvey.push_back(gmSurvey);
            }

            packet >> mainComment;
            return true;
        }
    };
}}
