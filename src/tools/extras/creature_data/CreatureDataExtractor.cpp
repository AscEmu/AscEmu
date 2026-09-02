#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "mpqlib/ClientArchiveData.hpp"
#include "mpqlib/ClientVersion.hpp"
#include "mpqlib/DBCFile.hpp"
#include "mpqlib/MpqPatchChain.hpp"

#include "CreatureDataStructures.hpp"

namespace fs = std::filesystem;
using mpqlib::ClientVersion;

// Single implicit archive chain covering everything this tool reads -
// mirrors the old global ArchiveSet's flat priority list: the first archive
// opened becomes the base, everything after is a patch.
std::unique_ptr<mpqlib::MpqPatchChain> gMpqChain;
ClientVersion gClientVersion = ClientVersion::WrathOfTheLichKing;

bool IsLegacyClient()
{
    return gClientVersion == ClientVersion::Vanilla || gClientVersion == ClientVersion::BurningCrusade
        || gClientVersion == ClientVersion::WrathOfTheLichKing;
}

bool FileExists(std::string const& path)
{
    return fs::exists(path);
}

bool OpenMpqArchive(std::string const& filename)
{
    if (!gMpqChain)
    {
        gMpqChain = std::make_unique<mpqlib::MpqPatchChain>(filename);
        if (gMpqChain->isOpen())
            return true;

        gMpqChain.reset();
        return false;
    }

    return gMpqChain->addPatch(filename);
}

// --- Classic/TBC/WotLK: flat, priority-ordered archive list. Kept in sync
// by hand with src/tools/map_extractor/System.cpp's legacyMpqList - mpqlib
// has no "open this client's archives for me" helper, so every tool that
// needs one (map_extractor, vmap4_extractor, this one) currently builds its
// own. ---

enum VersionMask : uint8_t
{
    MaskNone = 0x0,
    MaskClassic = 0x01,
    MaskBC = 0x02,
    MaskWotLK = 0x04,
    MaskBCWotLK = MaskBC | MaskWotLK,
    MaskClassicBCWotLK = MaskClassic | MaskBCWotLK,
};

VersionMask getLegacyVersionMask()
{
    switch (gClientVersion)
    {
        case ClientVersion::Vanilla: return MaskClassic;
        case ClientVersion::BurningCrusade: return MaskBC;
        case ClientVersion::WrathOfTheLichKing: return MaskWotLK;
        default: return MaskNone;
    }
}

struct MpqList
{
    uint32_t versionMask;
    std::string fileName;
};

std::vector<MpqList> const kLegacyMpqList{
    {MaskClassic, "dbc.MPQ"},
    {MaskClassic, "terrain.MPQ"},
    {MaskClassicBCWotLK, "patch.MPQ"},
    {MaskClassicBCWotLK, "patch-2.MPQ"},
    {MaskBCWotLK, "common.MPQ"},
    {MaskWotLK, "common-2.MPQ"},
    {MaskWotLK, "lichking.MPQ"},
    {MaskBCWotLK, "expansion.MPQ"},
    {MaskWotLK, "patch-3.MPQ"},
};

void LoadLegacyCommonMPQFiles()
{
    for (auto const& mpq : kLegacyMpqList)
    {
        if (mpq.versionMask & getLegacyVersionMask())
        {
            std::string const fileName = "Data/" + mpq.fileName;
            if (FileExists(fileName))
                OpenMpqArchive(fileName);
        }
    }
}

// Every locale actually present gets opened, rather than picking a single
// "detected" one like map_extractor does - this tool only ever reads
// locale-independent model/DBC data, so it doesn't matter which locale
// archive answers a lookup, and opening all of them matches this tool's own
// pre-existing (simpler) behavior of never needing to pick just one.
void LoadLegacyLocaleMPQFiles()
{
    for (char const* locale : mpqlib::kLocales)
    {
        std::string const localeMpq = std::string("Data/") + locale + "/locale-" + locale + ".MPQ";
        if (!FileExists(localeMpq))
            continue;

        OpenMpqArchive(localeMpq);
        for (int i = 1; i <= 4; ++i)
        {
            std::string const suffix = i > 1 ? ("-" + std::to_string(i)) : std::string();
            std::string const patchMpq = std::string("Data/") + locale + "/patch-" + locale + suffix + ".MPQ";
            if (FileExists(patchMpq))
                OpenMpqArchive(patchMpq);
        }
    }
}

// --- Cata/Mop: incremental wow-update patch chains. The base archive set,
// build-number list, and DBC-folder/base-set cutoff builds all live in
// mpqlib::ClientArchiveData - this tool gets mpqlib::modelAndVmapArchiveList()
// (the same one vmap4_extractor uses), since it opens individual .m2 model
// files directly, just like vmap4_extractor does and map_extractor doesn't -
// Mop's set includes model.MPQ, which map_extractor's DBC/map-only list
// doesn't carry and this tool would otherwise be missing creature models
// from. This tool never reads locale-specific string data, so unlike
// map_extractor/vmap4_extractor, the separate locale-MPQ chain (which needs
// a real detected build number via a component.wow-<locale>.txt read inside
// an already-open locale archive) is skipped entirely; the fixed "latest
// known build for this expansion" target is enough to walk every
// wow-update-* patch that could plausibly exist, with any the installed
// client doesn't actually have silently skipped below. ---

void LoadModernCommonMPQFiles()
{
    std::vector<std::string> const& mpqList = mpqlib::modelAndVmapArchiveList(gClientVersion);
    std::vector<uint32_t> const& buildList = mpqlib::incrementalPatchBuilds(gClientVersion);
    uint32_t const newBaseSetBuild = mpqlib::newBaseSetBuild(gClientVersion);
    uint32_t const lastDbcInDataBuild = mpqlib::lastDbcInDataBuild(gClientVersion);
    // The latest build this expansion's incremental patch list knows about -
    // enough to walk every wow-update-* patch that could plausibly exist;
    // any the installed client doesn't actually have are silently skipped
    // below, same as map_extractor's LoadModernCommonMPQFiles() does.
    uint32_t const targetBuild = buildList.back();

    std::string const baseMpq = "Data/world.MPQ";
    gMpqChain = std::make_unique<mpqlib::MpqPatchChain>(baseMpq);
    if (!gMpqChain->isOpen())
    {
        printf("Cannot open archive %s\n", baseMpq.c_str());
        gMpqChain.reset();
        return;
    }

    for (size_t i = 1; i < mpqList.size(); ++i)
    {
        if (targetBuild < newBaseSetBuild && mpqList[i] == "world2.MPQ")
            continue;

        std::string const filename = "Data/" + mpqList[i];
        if (!gMpqChain->addPatch(filename))
            printf("Not found %s\n", filename.c_str());
        else
            printf("Loaded %s\n", filename.c_str());
    }

    for (uint32_t patchBuild : buildList)
    {
        if (patchBuild > targetBuild)
            break;
        if (targetBuild >= newBaseSetBuild && patchBuild < newBaseSetBuild)
            continue;

        std::string const filename = patchBuild > lastDbcInDataBuild
            ? "Data/wow-update-base-" + std::to_string(patchBuild) + ".MPQ"
            : "Data/wow-update-" + std::to_string(patchBuild) + ".MPQ";

        if (!gMpqChain->addPatch(filename))
            printf("Not found %s\n", filename.c_str());
        else
            printf("Loaded %s\n", filename.c_str());
    }
}

void InitMPQs()
{
    auto const detected = mpqlib::detectClientVersion(".");
    gClientVersion = detected.value_or(ClientVersion::WrathOfTheLichKing);
    if (!detected)
        printf("Warning: couldn't detect client version from Wow.exe, defaulting to WotLK archive layout\n");

    if (IsLegacyClient())
    {
        LoadLegacyCommonMPQFiles();
        if (gClientVersion != ClientVersion::Vanilla)
            LoadLegacyLocaleMPQFiles();
    }
    else
    {
        LoadModernCommonMPQFiles();
    }

    if (!gMpqChain)
        printf("FATAL ERROR: could not open any archives - is this tool running from the client's root directory?\n");
}

void replace(std::string &str, const char* find, const char* rep, uint32_t limit)
{
    uint32_t i = 0;
    std::string::size_type pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos)
    {
        str.erase(pos, strlen(find));
        str.insert(pos, rep);
        pos += strlen(rep);

        ++i;
        if (limit != 0 && i == limit)
            break;
    }
}

struct ModelCache
{
    M2Header header;
    M2Attachment* attachments;
    M2Bone* bones;
    uint16_t* bonelookups;
};

// M2's on-disk header layout differs between Classic/TBC and WotLK+ (see
// CreatureDataStructures.hpp's M2RawHeaderLegacy/M2RawHeaderModern doc
// comments) - reads whichever one matches the detected client version and
// copies out just the fields this tool needs into the version-independent
// M2Header. Returns false if modelData is too small to hold the real raw
// header for this client version.
bool readM2Header(std::vector<uint8_t> const& modelData, M2Header& out)
{
    if (IsLegacyClient())
    {
        if (modelData.size() < sizeof(M2RawHeaderLegacy))
            return false;

        M2RawHeaderLegacy raw;
        memcpy(&raw, modelData.data(), sizeof(raw));
        out.nAttachments = raw.nAttachments;
        out.ofsAttachments = raw.ofsAttachments;
        out.nBoneLookupTable = raw.nBoneLookupTable;
        out.ofsBoneLookupTable = raw.ofsBoneLookupTable;
        out.nBones = raw.nBones;
        out.ofsBones = raw.ofsBones;
        memcpy(out.boundingbox1, raw.boundingbox1, sizeof(out.boundingbox1));
        memcpy(out.boundingbox2, raw.boundingbox2, sizeof(out.boundingbox2));
        out.boundingradius = raw.boundingradius;
    }
    else
    {
        if (modelData.size() < sizeof(M2RawHeaderModern))
            return false;

        M2RawHeaderModern raw;
        memcpy(&raw, modelData.data(), sizeof(raw));
        out.nAttachments = raw.nAttachments;
        out.ofsAttachments = raw.ofsAttachments;
        out.nBoneLookupTable = raw.nBoneLookupTable;
        out.ofsBoneLookupTable = raw.ofsBoneLookupTable;
        out.nBones = raw.nBones;
        out.ofsBones = raw.ofsBones;
        memcpy(out.boundingbox1, raw.boundingbox1, sizeof(out.boundingbox1));
        memcpy(out.boundingbox2, raw.boundingbox2, sizeof(out.boundingbox2));
        out.boundingradius = raw.boundingradius;
    }

    return true;
}

int main()
{
    InitMPQs();
    if (!gMpqChain)
        return 1;

    FILE* fo = fopen("display_bounding_boxes.sql", "w");
    DBCFile displayInfo(*gMpqChain, "DBFilesClient\\CreatureDisplayInfo.dbc");
    DBCFile modelInfo(*gMpqChain, "DBFilesClient\\CreatureModelData.dbc");
    displayInfo.open();
    modelInfo.open();

    std::map<uint32_t, DBCFile::Row> modelInfoEntries;
    std::map<std::string, ModelCache> modelCache;

    for (DBCFile::Cursor itr = modelInfo.begin(); itr != modelInfo.end(); ++itr)
    {
        unsigned int entry = itr->getInt(0);
        modelInfoEntries.insert(std::make_pair(entry, *itr));
    }

    for (DBCFile::Cursor itr = displayInfo.begin(); itr != displayInfo.end(); ++itr)
    {
        unsigned int displayid = itr->getInt(0);
        unsigned int modelentry = itr->getInt(1);
        float modelscale = itr->getFloat(4);

        std::map<uint32_t, DBCFile::Row>::iterator  modelitr = modelInfoEntries.find(modelentry);

        if (modelitr == modelInfoEntries.end())
        {
            printf("Cannot find model entry for display %u (entry %u)\n", displayid, modelentry);
            continue;
        }

        DBCFile::Row modelrec = modelitr->second;

        const char* modelname = modelrec.getString(2);

        std::string strmodelname(modelname);

        replace(strmodelname, ".mdx", ".m2", 0);
        replace(strmodelname, ".MDX", ".m2", 0);

        M2Attachment* attachments;
        M2Bone* bones;
        uint16_t* bonelookups;

        std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);

        if (cacheitr == modelCache.end())
        {
            std::vector<uint8_t> modelData;
            if (!gMpqChain->readFile(strmodelname, modelData))
            {
                printf("Error: cannot open %s\n", strmodelname.c_str());
                continue;
            }

            M2Header header;
            if (!readM2Header(modelData, header))
            {
                printf("Error: %s is too small for its M2 header\n", strmodelname.c_str());
                continue;
            }

            printf("Processing %u", displayid);
            printf(" %u attachments %u bone lookups %u bones\n", header.nAttachments, header.nBoneLookupTable, header.nBones);

            attachments = (M2Attachment*)malloc(header.nAttachments * sizeof(M2Attachment));
            memcpy(attachments, modelData.data() + header.ofsAttachments, header.nAttachments * sizeof(M2Attachment));

            bonelookups = (uint16_t*)malloc(header.nBoneLookupTable * sizeof(uint16_t));
            memcpy(bonelookups, modelData.data() + header.ofsBoneLookupTable, header.nBoneLookupTable * sizeof(uint16_t));

            bones = (M2Bone*)malloc(header.nBones * sizeof(M2Bone));
            memcpy(bones, modelData.data() + header.ofsBones, header.nBones * sizeof(M2Bone));

            ModelCache cacheentry;
            cacheentry.attachments = attachments;
            cacheentry.bones = bones;
            cacheentry.bonelookups = bonelookups;
            cacheentry.header = header;
            modelCache.insert(std::make_pair(modelname, cacheentry));
            cacheitr = modelCache.find(modelname);
        }
        else
        {
            bones = cacheitr->second.bones;
            bonelookups = cacheitr->second.bonelookups;
            attachments = cacheitr->second.attachments;
        }

        M2Header const& header = cacheitr->second.header;

        //try and get the bone
        for (uint32_t i = 0; i < header.nAttachments; ++i)
        {
            if (attachments[i].bone > header.nBoneLookupTable)
            {
                printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header.nBoneLookupTable);
                continue;
            }
            uint16_t boneindex = bonelookups[attachments[i].bone];
            if (boneindex > header.nBones)
            {
                printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header.nBones);
                continue;
            }
            // Only used by the attachment-point SQL output below, which is
            // currently disabled (commented out) - kept for when that output
            // is re-enabled rather than deleted outright.
            [[maybe_unused]] M2Bone & bone = bones[boneindex];
            //printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

            [[maybe_unused]] float realpos[3];
            realpos[0] = (/*bone.pivotpoint[0] +*/ attachments[i].pos[0]) * modelscale;
            realpos[1] = (/*bone.pivotpoint[1] +*/ attachments[i].pos[1]) * modelscale;
            realpos[2] = (/*bone.pivotpoint[2] +*/ attachments[i].pos[2]) * modelscale;

            //fix coord system
            //             float tmp = realpos[2];
            //             realpos[2] = realpos[1];
            //             realpos[1] = -tmp;
            //fprintf(fo, "insert into `display_attachment_points` VALUES (%u, %u, %f, %f, %f);\n", displayid, attachments[i].id, attachments[i].pos[0], attachments[i].pos[1], attachments[i].pos[2]);
            //printf("Attachmnent %u point %f %f %f pivot %f %f %f\n", attachments[i].id, realpos[0], realpos[1], realpos[2], bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2]);
        }

        float boundlow[3];
        boundlow[0] = header.boundingbox1[0] * modelscale;
        boundlow[1] = header.boundingbox1[1] * modelscale;
        boundlow[2] = header.boundingbox1[2] * modelscale;
        float boundhigh[3];
        boundhigh[0] = header.boundingbox2[0] * modelscale;
        boundhigh[1] = header.boundingbox2[1] * modelscale;
        boundhigh[2] = header.boundingbox2[2] * modelscale;
        float boundradius = header.boundingradius * modelscale;
        fprintf(fo, "insert into `display_bounding_boxes` VALUES (%u, %f, %f, %f, %f, %f, %f, %f);\n", displayid, boundlow[0], boundlow[1], boundlow[2], boundhigh[0], boundhigh[1], boundhigh[2], boundradius);
    }
    fclose(fo);
    return 0;
}
