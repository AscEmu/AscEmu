/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MODELHEADERS_H
#define MODELHEADERS_H

#include <cstdint>

#pragma pack(push,1)

// Classic/TBC M2 header layout - three field groups (nD/ofsD, ofsViews,
// nI/ofsI) that WotLK's M2 format revision dropped. Only the four
// nBounding*/ofsBounding* fields near the end are ever actually read
// (Model::open()/convertToVMapModel()), but the full real layout is kept
// here rather than a trimmed-down stand-in, since every field before those
// four shifts their byte offset.
struct ModelHeaderLegacy
{
    char id[4];
    uint8_t version[4];
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
    float floats[14];
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

// WotLK+ (WotLK/Cata/Mop) M2 header layout.
struct ModelHeaderModern
{
    char id[4];
    uint8_t version[4];
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
    float floats[14];
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

#endif  //MODELHEADERS_H
