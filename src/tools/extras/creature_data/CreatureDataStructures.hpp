#pragma once

#include <cstdint>

// The subset of M2 header fields this tool actually needs, independent of
// which of the two on-disk header layouts below applies to a given client's
// models. CreatureDataExtractor.cpp's readM2Header() fills one of these from
// whichever raw layout matches the detected client version.
struct M2Header
{
    uint32_t nAttachments;
    uint32_t ofsAttachments;
    uint32_t nBoneLookupTable;
    uint32_t ofsBoneLookupTable;
    uint32_t nBones;
    uint32_t ofsBones;
    float boundingbox1[3];
    float boundingbox2[3];
    float boundingradius;
};

#pragma pack(push, 1)

// Classic/TBC on-disk M2 header layout. Kept as the full real layout (not
// trimmed to just the fields M2Header above needs) since every field before
// nBones/nAttachments/boundingbox* shifts their byte offset - mirrors
// src/tools/vmap_tools/vmap4_extractor/modelheaders.h's ModelHeaderLegacy
// field-for-field (that file's trailing float floats[14] is split out here
// into the named vertexbox1/vertexbox2/vertexradius/boundingbox1/
// boundingbox2/boundingradius sub-fields this tool actually reads).
struct M2RawHeaderLegacy
{
    char id[4];
    unsigned char version[4];
    uint32_t nameLength;
    uint32_t nameOfs;
    uint32_t type;
    uint32_t nGlobalSequences;
    uint32_t ofsGlobalSequences;
    uint32_t nAnimations;
    uint32_t ofsAnimations;
    uint32_t nAnimationLookup;
    uint32_t ofsAnimationLookup;
    uint32_t nD;
    uint32_t ofsD;
    uint32_t nBones;
    uint32_t ofsBones;
    uint32_t nKeyBoneLookup;
    uint32_t ofsKeyBoneLookup;
    uint32_t nVertices;
    uint32_t ofsVertices;
    uint32_t nViews;
    uint32_t ofsViews;
    uint32_t nColors;
    uint32_t ofsColors;
    uint32_t nTextures;
    uint32_t ofsTextures;
    uint32_t nTransparency;
    uint32_t ofsTransparency;
    uint32_t nI;
    uint32_t ofsI;
    uint32_t nTextureanimations;
    uint32_t ofsTextureanimations;
    uint32_t nTexReplace;
    uint32_t ofsTexReplace;
    uint32_t nRenderFlags;
    uint32_t ofsRenderFlags;
    uint32_t nBoneLookupTable;
    uint32_t ofsBoneLookupTable;
    uint32_t nTexLookup;
    uint32_t ofsTexLookup;
    uint32_t nTexUnits;
    uint32_t ofsTexUnits;
    uint32_t nTransLookup;
    uint32_t ofsTransLookup;
    uint32_t nTexAnimLookup;
    uint32_t ofsTexAnimLookup;
    float vertexbox1[3];
    float vertexbox2[3];
    float vertexradius;
    float boundingbox1[3];
    float boundingbox2[3];
    float boundingradius;
    uint32_t nBoundingTriangles;
    uint32_t ofsBoundingTriangles;
    uint32_t nBoundingVertices;
    uint32_t ofsBoundingVertices;
    uint32_t nBoundingNormals;
    uint32_t ofsBoundingNormals;
    uint32_t nAttachments;
    uint32_t ofsAttachments;
    uint32_t nAttachLookup;
    uint32_t ofsAttachLookup;
    uint32_t nAttachments_2;
    uint32_t ofsAttachments_2;
    uint32_t nLights;
    uint32_t ofsLights;
    uint32_t nCameras;
    uint32_t ofsCameras;
    uint32_t nCameraLookup;
    uint32_t ofsCameraLookup;
    uint32_t nRibbonEmitters;
    uint32_t ofsRibbonEmitters;
    uint32_t nParticleEmitters;
    uint32_t ofsParticleEmitters;
};

// WotLK/Cata/Mop on-disk M2 header layout - three field groups Classic/TBC
// had (nD/ofsD before nBones, a second ofsViews, nI/ofsI) were dropped here,
// shifting every field after nAnimationLookup versus M2RawHeaderLegacy above.
// Mirrors modelheaders.h's ModelHeaderModern the same way M2RawHeaderLegacy
// mirrors ModelHeaderLegacy.
struct M2RawHeaderModern
{
    char id[4];
    unsigned char version[4];
    uint32_t nameLength;
    uint32_t nameOfs;
    uint32_t type;
    uint32_t nGlobalSequences;
    uint32_t ofsGlobalSequences;
    uint32_t nAnimations;
    uint32_t ofsAnimations;
    uint32_t nAnimationLookup;
    uint32_t ofsAnimationLookup;
    uint32_t nBones;
    uint32_t ofsBones;
    uint32_t nKeyBoneLookup;
    uint32_t ofsKeyBoneLookup;
    uint32_t nVertices;
    uint32_t ofsVertices;
    uint32_t nViews;
    uint32_t nColors;
    uint32_t ofsColors;
    uint32_t nTextures;
    uint32_t ofsTextures;
    uint32_t nTransparency;
    uint32_t ofsTransparency;
    uint32_t nTextureanimations;
    uint32_t ofsTextureanimations;
    uint32_t nTexReplace;
    uint32_t ofsTexReplace;
    uint32_t nRenderFlags;
    uint32_t ofsRenderFlags;
    uint32_t nBoneLookupTable;
    uint32_t ofsBoneLookupTable;
    uint32_t nTexLookup;
    uint32_t ofsTexLookup;
    uint32_t nTexUnits;
    uint32_t ofsTexUnits;
    uint32_t nTransLookup;
    uint32_t ofsTransLookup;
    uint32_t nTexAnimLookup;
    uint32_t ofsTexAnimLookup;
    float vertexbox1[3];
    float vertexbox2[3];
    float vertexradius;
    float boundingbox1[3];
    float boundingbox2[3];
    float boundingradius;
    uint32_t nBoundingTriangles;
    uint32_t ofsBoundingTriangles;
    uint32_t nBoundingVertices;
    uint32_t ofsBoundingVertices;
    uint32_t nBoundingNormals;
    uint32_t ofsBoundingNormals;
    uint32_t nAttachments;
    uint32_t ofsAttachments;
    uint32_t nAttachLookup;
    uint32_t ofsAttachLookup;
    uint32_t nAttachments_2;
    uint32_t ofsAttachments_2;
    uint32_t nLights;
    uint32_t ofsLights;
    uint32_t nCameras;
    uint32_t ofsCameras;
    uint32_t nCameraLookup;
    uint32_t ofsCameraLookup;
    uint32_t nRibbonEmitters;
    uint32_t ofsRibbonEmitters;
    uint32_t nParticleEmitters;
    uint32_t ofsParticleEmitters;
};

#pragma pack(pop)

struct AnimationBlock
{
    uint16_t interpolation;
    uint16_t globalsequenceid;
    uint32_t list1offset;
    uint32_t timestampdataoffset;
    uint32_t list2offset;
    uint32_t keysoffset;
};

struct M2Attachment
{
    uint32_t id;
    uint32_t bone;
    float pos[3];
    AnimationBlock unk;
};

struct M2Vertex
{
    float position[3];
    unsigned char boneweight[4];
    unsigned char boneindex[4];
    float normal[3];
    float textcoord[2];
    float unk[2];
};

struct M2Bone
{
    int keyboneid;
    uint32_t flags;
    short parentbone;
    uint16_t unk[3];
    AnimationBlock translation;
    AnimationBlock rotation;
    AnimationBlock scaling;
    float pivotpoint[3];
};
