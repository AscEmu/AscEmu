/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildEmblemInfo.h"
#include "Database/Database.h"
#include "Server/MainServerDefines.h"

EmblemInfo::EmblemInfo() : mStyle(0), mColor(0), mBorderStyle(0), mBorderColor(0), mBackgroundColor(0)
{
}

void EmblemInfo::loadEmblemInfoFromDB(Field* fields)
{
    mStyle = fields[3].GetUInt8();
    mColor = fields[4].GetUInt8();
    mBorderStyle = fields[5].GetUInt8();
    mBorderColor = fields[6].GetUInt8();
    mBackgroundColor = fields[7].GetUInt8();
}

void EmblemInfo::saveEmblemInfoToDB(uint32_t guildId) const
{
    CharacterDatabase.Execute("UPDATE guild SET emblemStyle = %u, emblemColor = %u, borderStyle = %u, borderColor = %u, backgroundColor = %u WHERE guildId = %u",
        mStyle, mColor, mBorderStyle, mBorderColor, mBackgroundColor, guildId);
}

void EmblemInfo::readEmblemInfoFromPacket(WorldPacket& recv_data)
{
    recv_data >> mStyle;
    recv_data >> mColor;
    recv_data >> mBorderStyle;
    recv_data >> mBorderColor;
    recv_data >> mBackgroundColor;
}

void EmblemInfo::writeEmblemInfoToPacket(WorldPacket& data) const
{
    data << uint32_t(mStyle);
    data << uint32_t(mColor);
    data << uint32_t(mBorderStyle);
    data << uint32_t(mBorderColor);
    data << uint32_t(mBackgroundColor);
}

uint32_t EmblemInfo::getStyle() const
{
    return mStyle;
}

uint32_t EmblemInfo::getColor() const
{
    return mColor;
}

uint32_t EmblemInfo::getBorderStyle() const
{
    return mBorderStyle;
}

uint32_t EmblemInfo::getBorderColor() const
{
    return mBorderColor;
}

uint32_t EmblemInfo::getBackgroundColor() const
{
    return mBackgroundColor;
}
