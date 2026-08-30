#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <memory>
#include <vector>
#include <map>

#include "mpqlib/DBCFile.hpp"
#include "mpqlib/MpqPatchChain.hpp"

#include "CreatureDataStructures.h"

// Single implicit archive chain covering everything this tool reads -
// mirrors the old global ArchiveSet's flat priority list: the first archive
// opened becomes the base, everything after is a patch.
std::unique_ptr<mpqlib::MpqPatchChain> gMpqChain;

bool OpenMpqArchive(char const* filename)
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

void InitMPQs()
{
    //COPIED FROM MAP EXTRACTOR
    const char* localeNames[] = { "enUS", "enGB", "deDE", "frFR", "koKR", "zhCN", "zhTW", "esES", "ruRU", 0 };
    int maxPatches = 3;
    int locale = -1;
    char tmp[100];

    FILE* tf = fopen("Data/common-2.MPQ", "r");
    if (!tf)
    {
        printf("Could not find Data/common.MPQ-2\n");
        return;
    }
    fclose(tf);
    OpenMpqArchive("Data/common-2.MPQ");

    for (size_t i = 0; localeNames[i] != 0; i++)
    {
        sprintf(tmp, "Data/%s/locale-%s.MPQ", localeNames[i], localeNames[i]);
        tf = fopen(tmp, "r");
        if (!tf)
            continue;
        fclose(tf);
        locale = i;
        OpenMpqArchive(tmp);
    }

    tf = fopen("Data/expansion.MPQ", "r");
    if (tf)
    {
        fclose(tf);
        OpenMpqArchive("Data/expansion.MPQ");
        if (-1 != locale)
        {
            sprintf(tmp, "Data/%s/expansion-locale-%s.MPQ", localeNames[locale], localeNames[locale]);
            OpenMpqArchive(tmp);
        }
    }

    tf = fopen("Data/lichking.MPQ", "r");
    if (tf)
    {
        fclose(tf);
        OpenMpqArchive("Data/lichking.MPQ");
        if (-1 != locale)
        {
            sprintf(tmp, "Data/%s/lichking-locale-%s.MPQ", localeNames[locale], localeNames[locale]);
            OpenMpqArchive(tmp);
        }
    }

    tf = fopen("Data/patch.MPQ", "r");
    if (tf)
    {
        fclose(tf);
        OpenMpqArchive("Data/patch.MPQ");
        for (int i = 2; i <= maxPatches; i++)
        {
            sprintf(tmp, "Data/patch-%d.MPQ", i);
            tf = fopen(tmp, "r");
            if (!tf)
                continue;
            fclose(tf);
            OpenMpqArchive(tmp);
        }
        if (-1 != locale)
        {
            sprintf(tmp, "Data/%s/patch-%s.MPQ", localeNames[locale], localeNames[locale]);
            tf = fopen(tmp, "r");
            if (tf)
            {
                fclose(tf);
                OpenMpqArchive(tmp);
                for (int i = 2; i <= maxPatches; i++)
                {
                    sprintf(tmp, "Data/%s/patch-%s-%d.MPQ", localeNames[locale], localeNames[locale], i);
                    tf = fopen(tmp, "r");
                    if (!tf)
                        continue;
                    fclose(tf);
                    OpenMpqArchive(tmp);
                }
            }
        }
    }
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
    M2Header* header;
    M2Attachment* attachments;
    M2Bone* bones;
    uint16_t* bonelookups;
};

int main()
{
    InitMPQs();
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

        M2Header* header;
        M2Attachment* attachments;
        M2Bone* bones;
        uint16_t* bonelookups;

        std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);

        if (cacheitr == modelCache.end())
        {

            std::vector<uint8_t> modelData;
            if (!gMpqChain->readFile(strmodelname, modelData) || modelData.size() < sizeof(M2Header))
            {
                printf("Error: cannot open %s\n", strmodelname.c_str());
                continue;
            }

            printf("Processing %u", displayid);

            header = (M2Header*)malloc(sizeof(M2Header));
            memcpy(header, modelData.data(), sizeof(M2Header));

            printf(" %u attachments %u bone lookups %u bones\n", header->nAttachments, header->nBoneLookupTable, header->nBones);

            attachments = (M2Attachment*)malloc(header->nAttachments * sizeof(M2Attachment));
            memcpy(attachments, modelData.data() + header->ofsAttachments, header->nAttachments * sizeof(M2Attachment));

            bonelookups = (uint16_t*)malloc(header->nBoneLookupTable * sizeof(uint16_t));
            memcpy(bonelookups, modelData.data() + header->ofsBoneLookupTable, header->nBoneLookupTable * sizeof(uint16_t));

            bones = (M2Bone*)malloc(header->nBones * sizeof(M2Bone));
            memcpy(bones, modelData.data() + header->ofsBones, header->nBones * sizeof(M2Bone));

            ModelCache cacheentry;
            cacheentry.attachments = attachments;
            cacheentry.bones = bones;
            cacheentry.bonelookups = bonelookups;
            cacheentry.header = header;
            modelCache.insert(std::make_pair(modelname, cacheentry));
        }
        else
        {
            header = cacheitr->second.header;
            bones = cacheitr->second.bones;
            bonelookups = cacheitr->second.bonelookups;
            attachments = cacheitr->second.attachments;
        }

        //try and get the bone
        for (uint32_t i = 0; i < header->nAttachments; ++i)
        {
            if (attachments[i].bone > header->nBoneLookupTable)
            {
                printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header->nBoneLookupTable);
                continue;
            }
            uint16_t boneindex = bonelookups[attachments[i].bone];
            if (boneindex > header->nBones)
            {
                printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header->nBones);
                continue;
            }
            M2Bone & bone = bones[boneindex];
            //printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

            float realpos[3];
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
        boundlow[0] = header->boundingbox1[0] * modelscale;
        boundlow[1] = header->boundingbox1[1] * modelscale;
        boundlow[2] = header->boundingbox1[2] * modelscale;
        float boundhigh[3];
        boundhigh[0] = header->boundingbox2[0] * modelscale;
        boundhigh[1] = header->boundingbox2[1] * modelscale;
        boundhigh[2] = header->boundingbox2[2] * modelscale;
        float boundradius = header->boundingradius * modelscale;
        fprintf(fo, "insert into `display_bounding_boxes` VALUES (%u, %f, %f, %f, %f, %f, %f, %f);\n", displayid, boundlow[0], boundlow[1], boundlow[2], boundhigh[0], boundhigh[1], boundhigh[2], boundradius);
    }
    fclose(fo);
    return 0;
}
