/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <memory>
#include <string>

class Field;
class QueryBuffer;
class QueryResult;

/// Maximum number of CompactUnitFrames profiles
constexpr uint8_t MAX_CUF_PROFILES = 5;

/// Bit index used in the many bool options of CompactUnitFrames
enum CUFBoolOptions
{
    CUF_KEEP_GROUPS_TOGETHER,
    CUF_DISPLAY_PETS,
    CUF_DISPLAY_MAIN_TANK_AND_ASSIST,
    CUF_DISPLAY_HEAL_PREDICTION,
    CUF_DISPLAY_AGGRO_HIGHLIGHT,
    CUF_DISPLAY_ONLY_DISPELLABLE_DEBUFFS,
    CUF_DISPLAY_POWER_BAR,
    CUF_DISPLAY_BORDER,
    CUF_USE_CLASS_COLORS,
    CUF_DISPLAY_NON_BOSS_DEBUFFS,
    CUF_DISPLAY_HORIZONTAL_GROUPS,
    CUF_LOCKED,
    CUF_SHOWN,
    CUF_AUTO_ACTIVATE_2_PLAYERS,
    CUF_AUTO_ACTIVATE_3_PLAYERS,
    CUF_AUTO_ACTIVATE_5_PLAYERS,
    CUF_AUTO_ACTIVATE_10_PLAYERS,
    CUF_AUTO_ACTIVATE_15_PLAYERS,
    CUF_AUTO_ACTIVATE_25_PLAYERS,
    CUF_AUTO_ACTIVATE_40_PLAYERS,
    CUF_AUTO_ACTIVATE_SPEC_1,
    CUF_AUTO_ACTIVATE_SPEC_2,
    CUF_AUTO_ACTIVATE_PVP,
    CUF_AUTO_ACTIVATE_PVE,
    CUF_UNK_145,
    CUF_UNK_156,
    CUF_UNK_157,

    CUF_BOOL_OPTIONS_COUNT
};

/// Represents a CompactUnitFrame profile
struct CUFProfile
{
    CUFProfile() = default;
    CUFProfile(Field const* fields);

    CUFProfile(const std::string& name, uint16_t frameHeight, uint16_t frameWidth, uint8_t sortBy, uint8_t healthText,
        uint32_t boolOptions, uint8_t topPoint, uint8_t bottomPoint, uint8_t leftPoint, uint16_t topOffset,
        uint16_t bottomOffset, uint16_t leftOffset) :
        ProfileName(name), FrameHeight(frameHeight), FrameWidth(frameWidth), SortBy(sortBy), HealthText(healthText),
        TopPoint(topPoint), BottomPoint(bottomPoint), LeftPoint(leftPoint), TopOffset(topOffset),
        BottomOffset(bottomOffset), LeftOffset(leftOffset), BoolOptions(boolOptions)
    {
    }

    std::string ProfileName;
    uint16_t FrameHeight = 0;
    uint16_t FrameWidth = 0;
    uint8_t SortBy = 0;
    uint8_t HealthText = 0;

    uint8_t TopPoint = 0;
    uint8_t BottomPoint = 0;
    uint8_t LeftPoint = 0;

    uint16_t TopOffset = 0;
    uint16_t BottomOffset = 0;
    uint16_t LeftOffset = 0;

    std::bitset<CUF_BOOL_OPTIONS_COUNT> BoolOptions;
};

typedef std::array<std::unique_ptr<CUFProfile>, MAX_CUF_PROFILES> CUFProfileStorage;

class CUFProfileMgr
{
public:
    CUFProfileMgr() { m_ownerGuid = 1; }
    CUFProfileMgr(uint32_t ownerGuid) { this->m_ownerGuid = ownerGuid; }
    ~CUFProfileMgr();

    CUFProfile* getCUFProfile(uint8_t id) const;
    void setCUFProfile(uint8_t id, std::unique_ptr<CUFProfile> profile);
    uint8_t getCUFProfileCount() const;

    bool loadFromDB(QueryResult* result);
    bool saveToDB(QueryBuffer* buffer);

private:
    CUFProfileMgr(CUFProfileMgr& /*other*/) {}
    CUFProfileMgr& operator=(CUFProfileMgr& /*other*/) { return *this; }

    uint32_t m_ownerGuid = 0;

    CUFProfileStorage m_cufProfiles;
};
