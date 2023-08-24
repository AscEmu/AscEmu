/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstdint>
#include <list>
#include <map>
#include <string>

//\TODO handle it, if possible, the same way in all versions
#define STANDARD_ADDON_CRC 0x4C1C776D

#if VERSION_STRING < Cata
class ByteBuffer;
class WorldSession;
class WorldPacket;
static uint8_t PublicKey[265] =
{
    0x02, 0x01, 0x01, 0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA,
    0x0E, 0x5D, 0x54, 0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84,
    0x93, 0x45, 0x75, 0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40,
    0x85, 0x59, 0x34, 0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B,
    0x95, 0x28, 0xA8, 0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A,
    0x1A, 0x2C, 0xC8, 0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50,
    0x37, 0x8F, 0x5A, 0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34,
    0xC6, 0xC2, 0xC3, 0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57,
    0xED, 0x46, 0xB9, 0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77,
    0xCF, 0xAD, 0x22, 0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18,
    0xF2, 0x92, 0x7E, 0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85,
    0x9C, 0xCB, 0xFB, 0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85,
    0xBA, 0xA8, 0xF7, 0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7,
    0xCE, 0x36, 0xF0, 0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3,
    0xF5, 0xE0, 0xA6, 0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE,
    0x79, 0xC3, 0x9A, 0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E,
    0xC7, 0x89, 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct AddonEntry
{
    uint64_t crc;
    std::string name;
    bool banned;
    bool isNew;
    bool showinlist;
};

typedef std::map<std::string, AddonEntry*> KnownAddons;
typedef KnownAddons::iterator KnownAddonsItr;

typedef std::map<std::string, ByteBuffer> AddonData;
typedef AddonData::iterator AddonDataItr;

#else

static uint8_t PublicKey[256] =
{
    0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA, 0x0E, 0x5D, 0x54,
    0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84, 0x93, 0x45, 0x75,
    0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40, 0x85, 0x59, 0x34,
    0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B, 0x95, 0x28, 0xA8,
    0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A, 0x1A, 0x2C, 0xC8,
    0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50, 0x37, 0x8F, 0x5A,
    0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34, 0xC6, 0xC2, 0xC3,
    0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57, 0xED, 0x46, 0xB9,
    0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77, 0xCF, 0xAD, 0x22,
    0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18, 0xF2, 0x92, 0x7E,
    0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85, 0x9C, 0xCB, 0xFB,
    0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85, 0xBA, 0xA8, 0xF7,
    0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7, 0xCE, 0x36, 0xF0,
    0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3, 0xF5, 0xE0, 0xA6,
    0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE, 0x79, 0xC3, 0x9A,
    0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E, 0xC7, 0x89, 0xD2
};

static uint8_t publicKeyOrder[256] =
{
    0x05, 0xB0, 0x94, 0x2B, 0x1C, 0x87, 0x40, 0x08, 0xA0, 0x91, 0xE2, 0x77, 0xB5, 0xC0, 0xF0, 0x48,
    0xF3, 0xD4, 0xD1, 0xAC, 0x15, 0xED, 0x55, 0x0A, 0x4B, 0x75, 0xF4, 0x52, 0x18, 0x14, 0x12, 0x4C,
    0x43, 0x39, 0x9D, 0x3B, 0xC6, 0x5A, 0x16, 0x06, 0x31, 0x0C, 0x5F, 0xC1, 0x76, 0x5E, 0x28, 0x62,
    0xFF, 0xA9, 0xD6, 0x53, 0x80, 0xDB, 0x49, 0xF7, 0x84, 0xCA, 0xDA, 0x9A, 0x70, 0x83, 0xB1, 0x6F,
    0x90, 0x38, 0x27, 0x98, 0x30, 0x3F, 0x19, 0x72, 0x26, 0x54, 0x63, 0xA5, 0x7E, 0x22, 0x45, 0xB7,
    0xB9, 0x34, 0x67, 0x24, 0xE9, 0x03, 0x2F, 0x8D, 0xA2, 0xE8, 0xC2, 0xFD, 0x74, 0x1B, 0x50, 0x2E,
    0x59, 0x6B, 0xBD, 0x0E, 0xE1, 0xA7, 0x8C, 0xFA, 0xBC, 0x11, 0x1D, 0x89, 0x85, 0x4A, 0xB2, 0x3E,
    0xEC, 0x1F, 0x65, 0x09, 0xA4, 0xC8, 0x88, 0x9F, 0xC5, 0xD8, 0xF6, 0x86, 0x00, 0x61, 0xEA, 0xA6,
    0xCC, 0x41, 0x3C, 0xDF, 0x7A, 0x02, 0x04, 0xEF, 0xF9, 0x1E, 0xFC, 0xD3, 0x7C, 0x1A, 0x17, 0xA1,
    0x5C, 0x8A, 0x25, 0xE3, 0x78, 0x99, 0x73, 0x97, 0xFE, 0xAD, 0xAF, 0x6C, 0x82, 0xFB, 0xAA, 0x9E,
    0x0B, 0xF5, 0xBE, 0x68, 0xD9, 0x07, 0x4E, 0xE7, 0x9B, 0xAB, 0x37, 0x51, 0x8F, 0xCE, 0x46, 0x9C,
    0x58, 0x2D, 0xC9, 0xB6, 0xB4, 0x10, 0xD7, 0xE6, 0x32, 0x95, 0xCB, 0xA8, 0xDC, 0xBB, 0x29, 0x3D,
    0xEE, 0xD0, 0xE0, 0x6A, 0xCD, 0xDE, 0x2A, 0x44, 0x7F, 0xD2, 0x4D, 0x81, 0xD5, 0x0F, 0x66, 0x92,
    0x36, 0x23, 0x5B, 0x13, 0xC7, 0x20, 0x8B, 0x96, 0xC4, 0x7D, 0x35, 0x64, 0x71, 0x6E, 0x47, 0xBF,
    0x3A, 0xF2, 0xF8, 0x0D, 0xB8, 0xA3, 0x93, 0x4F, 0x5D, 0xE5, 0xE4, 0xBA, 0xCF, 0x01, 0x42, 0x21,
    0x79, 0x60, 0x7B, 0xB3, 0xEB, 0xF1, 0x6D, 0x8E, 0x2C, 0x56, 0xC3, 0xAE, 0x57, 0x69, 0x33, 0xDD,
};

struct AddonEntry
{
    AddonEntry(const std::string& _name, uint8_t _enabled, uint32_t _crc, uint8_t _state, bool _crcOrPubKey)
        : name(_name), enabled(_enabled), crc(_crc), state(_state), usePublicKeyOrCRC(_crcOrPubKey)
    { }

    std::string name;
    uint8_t enabled;
    uint32_t crc;
    uint8_t state;
    bool usePublicKeyOrCRC;
};

struct SavedAddon
{
    SavedAddon(std::string const& _name, uint32_t _crc) : name(_name)
    {
        crc = _crc;
    }

    std::string name;
    uint32_t crc;
};

struct BannedAddon
{
    uint32_t id;
    uint8_t nameMD5[16];
    uint8_t versionMD5[16];
    uint32_t timestamp;
};

typedef std::list<BannedAddon> BannedAddonList;
typedef std::list<SavedAddon> SavedAddonsList;

#endif

class AddonMgr
{
#if VERSION_STRING < Cata
    private:

        AddonMgr() = default;
        ~AddonMgr() = default;

    public:

        static AddonMgr& getInstance();
        void initialize();
        void finalize();

        AddonMgr(AddonMgr&&) = delete;
        AddonMgr(AddonMgr const&) = delete;
        AddonMgr& operator=(AddonMgr&&) = delete;
        AddonMgr& operator=(AddonMgr const&) = delete;

        void LoadFromDB();
        void SaveToDB();

        void SendAddonInfoPacket(std::shared_ptr<WorldPacket> source, uint32_t pos, WorldSession* m_session);
        bool AppendPublicKey(WorldPacket& data, std::string& AddonName, uint32_t CRC);

    private:

        bool IsAddonBanned(uint64_t crc, std::string name = "");
        bool IsAddonBanned(std::string name, uint64_t crc = 0);
        bool ShouldShowInList(std::string name);

        KnownAddons mKnownAddons;
        AddonData mAddonData;
#else

    private:
    
        AddonMgr() = default;
        ~AddonMgr() = default;
    
    public:
    
        static AddonMgr& getInstance();
        void initialize();

        AddonMgr(AddonMgr&&) = delete;
        AddonMgr(AddonMgr const&) = delete;
        AddonMgr& operator=(AddonMgr&&) = delete;
        AddonMgr& operator=(AddonMgr const&) = delete;

        void LoadFromDB();
        void SaveAddon(AddonEntry const& addon);
        SavedAddon const* getAddonInfoForAddonName(const std::string& name);

        BannedAddonList const* getBannedAddonsList();

        SavedAddonsList mKnownAddons;

        BannedAddonList mBannedAddons;
#endif
};

#define sAddonMgr AddonMgr::getInstance()
