/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/CUFProfileMgr.h"

#include <sstream>

#include "Database/Field.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"

CUFProfile::CUFProfile(Field const* fields)
{
    ProfileName = fields[1].asCString();
    FrameHeight = fields[2].asUint16();
    FrameWidth = fields[3].asUint16();
    SortBy = fields[4].asUint8();
    HealthText = fields[5].asUint8();
    BoolOptions = std::bitset<CUF_BOOL_OPTIONS_COUNT>(fields[6].asUint32());
    TopPoint = fields[7].asUint8();
    BottomPoint = fields[8].asUint8();
    LeftPoint = fields[9].asUint8();
    TopOffset = fields[10].asUint16();
    BottomOffset = fields[11].asUint16();
    LeftOffset = fields[12].asUint16();
}

CUFProfileMgr::~CUFProfileMgr() = default;

CUFProfile* CUFProfileMgr::getCUFProfile(uint8_t id) const
{
    if (id >= MAX_CUF_PROFILES)
        return nullptr;

    return m_cufProfiles[id].get();
}

void CUFProfileMgr::setCUFProfile(uint8_t id, std::unique_ptr<CUFProfile> profile)
{
    if (id >= MAX_CUF_PROFILES)
        return;

    m_cufProfiles[id] = std::move(profile);
}

uint8_t CUFProfileMgr::getCUFProfileCount() const
{
    uint8_t count = 0;
    for (const auto& profile : m_cufProfiles)
        if (profile)
            ++count;

    return count;
}

bool CUFProfileMgr::loadFromDB(QueryResult* result)
{
    if (result == nullptr)
        return false;

    do
    {
        Field* fields = result->fetch();

        const uint8_t id = fields[0].asUint8();
        if (id >= MAX_CUF_PROFILES)
        {
            sLogger.failure("CUFProfileMgr::loadFromDB - GUID: {} has a CUF profile with invalid id (id: {}), max is {}.", m_ownerGuid, id, MAX_CUF_PROFILES);
            continue;
        }

        m_cufProfiles[id] = std::make_unique<CUFProfile>(fields);
    } while (result->nextRow());

    return true;
}

bool CUFProfileMgr::saveToDB(QueryBuffer* buffer)
{
    if (buffer == nullptr)
        return false;

    std::stringstream ds;
    ds << "DELETE FROM character_cuf_profiles WHERE ownerguid = ";
    ds << m_ownerGuid;

    buffer->addQueryNA(ds.str().c_str());

    for (uint8_t i = 0; i < MAX_CUF_PROFILES; ++i)
    {
        const auto& profile = m_cufProfiles[i];
        if (!profile)
            continue;

        std::stringstream ss;

        ss << "INSERT INTO character_cuf_profiles VALUES('";
        ss << m_ownerGuid << "','";
        ss << static_cast<uint32_t>(i) << "','";
        ss << CharacterDatabase.escapeString(profile->ProfileName) << "','";
        ss << profile->FrameHeight << "','";
        ss << profile->FrameWidth << "','";
        ss << static_cast<uint32_t>(profile->SortBy) << "','";
        ss << static_cast<uint32_t>(profile->HealthText) << "','";
        ss << profile->BoolOptions.to_ulong() << "','";
        ss << static_cast<uint32_t>(profile->TopPoint) << "','";
        ss << static_cast<uint32_t>(profile->BottomPoint) << "','";
        ss << static_cast<uint32_t>(profile->LeftPoint) << "','";
        ss << profile->TopOffset << "','";
        ss << profile->BottomOffset << "','";
        ss << profile->LeftOffset << "'";
        ss << ")";

        buffer->addQueryNA(ss.str().c_str());
    }

    return true;
}
