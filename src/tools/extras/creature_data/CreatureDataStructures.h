struct M2Header
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
    uint32_t osdAnimationLookup;
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
    uint32_t nUVAnimation;
    uint32_t ofsUVAnimation;
    uint32_t nTexReplace;
    uint32_t ofsTexReplace;

    uint32_t nRenderFlags;
    uint32_t ofsRenderFlags;
    uint32_t nBoneLookupTable;
    uint32_t ofsBoneLookupTable;

    uint32_t nTexLookup;
    uint32_t ofsTexLookup;

    uint32_t nTexUnitLookup;
    uint32_t ofsTexUnitLookup;
    uint32_t nTransparencyLookup;
    uint32_t ofsTransparencyLookup;
    uint32_t nUVAnimLookup;
    uint32_t ofsUVAnimLookup;

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
    uint32_t nAttachmentLookup;
    uint32_t ofsAttachmentLookup;
    uint32_t nEvents;
    uint32_t ofsEvents;
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
