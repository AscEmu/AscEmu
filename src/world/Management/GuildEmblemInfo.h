/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldPacket.h"
#include "Database/Field.h"

class EmblemInfo
{
    public:

        EmblemInfo();

        void loadEmblemInfoFromDB(Field* fields);
        void saveEmblemInfoToDB(uint32_t guildId) const;

        void readEmblemInfoFromPacket(WorldPacket& recv);
        void writeEmblemInfoToPacket(WorldPacket& data) const;

        uint32_t getStyle() const;
        uint32_t getColor() const;
        uint32_t getBorderStyle() const;
        uint32_t getBorderColor() const;
        uint32_t getBackgroundColor() const;

    private:

        uint32_t mStyle;
        uint32_t mColor;
        uint32_t mBorderStyle;
        uint32_t mBorderColor;
        uint32_t mBackgroundColor;
};
