/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct KeyFrame;
struct TransportTemplate;
struct GameObjectProperties;
class Transporter;

typedef std::vector<KeyFrame>                               KeyFrameVec;
typedef std::unordered_map<uint32, TransportTemplate>       TransportTemplates;
typedef std::set<Transporter*>                              TransporterSet;
typedef std::unordered_map<uint32, Transporter*>            TransporterMap;
typedef std::unordered_map<uint32, std::set<uint32> >       TransportInstanceMap;

typedef std::vector<DBC::Structures::TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

struct PathNode
{
    uint32 mapid;
    float x, y, z;
    uint32 flags;
    uint32 delay;
    uint32 ArrivalEventID;
    uint32 DepartureEventID;
};

class TransportPath
{
public:
    inline void SetLength(const unsigned int sz)
    {
        i_nodes.resize(sz);
    }

    inline size_t Size(void) const { return i_nodes.size(); }
    inline void Resize(unsigned int sz) { i_nodes.resize(sz); }
    inline void Clear(void) { i_nodes.clear(); }
    inline PathNode* GetNodes(void) { return static_cast<PathNode*>(&i_nodes[0]); }
    float GetTotalLength(void)
    {
        float len = 0, xd, yd, zd;
        for (unsigned int idx = 1; idx < i_nodes.size(); ++idx)
        {
            xd = i_nodes[idx].x - i_nodes[idx - 1].x;
            yd = i_nodes[idx].y - i_nodes[idx - 1].y;
            zd = i_nodes[idx].z - i_nodes[idx - 1].z;
            len += (float)std::sqrt(xd * xd + yd * yd + zd * zd);
        }
        return len;
    }

    PathNode & operator[](const unsigned int idx) { return i_nodes[idx]; }
    const PathNode & operator()(const unsigned int idx) const { return i_nodes[idx]; }

protected:
    std::vector<PathNode> i_nodes;
};

struct KeyFrame
{
    explicit KeyFrame(PathNode node) : Index(0), Node(node), InitialOrientation(0.0f),
        DistSinceStop(-1.0f), DistUntilStop(-1.0f), DistFromPrev(-1.0f), TimeFrom(0.0f), TimeTo(0.0f),
        Teleport(false), ArriveTime(0), DepartureTime(0), NextDistFromPrev(0.0f), NextArriveTime(0)
    {
    }

    uint32 Index;
    PathNode Node;
    float InitialOrientation;
    float DistSinceStop;
    float DistUntilStop;
    float DistFromPrev;
    float TimeFrom;
    float TimeTo;
    bool Teleport;
    uint32 ArriveTime;
    uint32 DepartureTime;

    // Data needed for next frame
    float NextDistFromPrev;
    uint32 NextArriveTime;

    bool IsTeleportFrame() const { return Teleport; }
    bool IsStopFrame() const { return Node.flags == 2; }
};

struct TransportTemplate
{
    TransportTemplate() : inInstance(false), pathTime(0), accelTime(0.0f), accelDist(0.0f), entry(0) { }
    ~TransportTemplate();

    uint32_t lowguid;
    std::set<uint32> mapsUsed;
    bool inInstance;
    uint32 pathTime;
    KeyFrameVec keyFrames;
    float accelTime;
    float accelDist;

    uint32 entry;
};

typedef std::map<uint32, DBC::Structures::TransportAnimationEntry const*> TransportPathContainer;
typedef std::map<uint32, DBC::Structures::TransportRotationEntry const*> TransportPathRotationContainer;

struct SERVER_DECL TransportAnimation
{
    TransportAnimation() : TotalTime(0) { }

    TransportPathContainer Path;
    TransportPathRotationContainer Rotations;
    uint32 TotalTime;

    DBC::Structures::TransportAnimationEntry const* GetAnimNode(uint32 time) const;
    DBC::Structures::TransportRotationEntry const* GetAnimRotation(uint32 time) const;
};

typedef std::map<uint32, TransportAnimation> TransportAnimationContainer;

bool FillTransporterPathVector(uint32 PathID, TransportPath & Path);

class SERVER_DECL TransportHandler
{
public:
    static TransportHandler& getInstance();

    void Unload();

    void LoadTransportTemplates();

    void LoadTransportAnimationAndRotation();

    // Creates a transport using given GameObject template entry
    Transporter* CreateTransport(uint32 entry, MapMgr* map = nullptr);

    // Spawns all continent transports, used at startup
    void SpawnContinentTransports();

    // constrain arbitrary radian orientation to interval [0,2*PI)
    static float NormalizeOrientation(float o);

    void AddTransport(Transporter* transport);
    Transporter* GetTransporter(uint32 guid);

    TransportTemplate const* GetTransportTemplate(uint32 entry) const
    {
        TransportTemplates::const_iterator itr = _transportTemplates.find(entry);
        if (itr != _transportTemplates.end())
            return &itr->second;
        return nullptr;
    }

    TransportAnimation const* GetTransportAnimInfo(uint32 entry) const
    {
        TransportAnimationContainer::const_iterator itr = _transportAnimations.find(entry);
        if (itr != _transportAnimations.end())
            return &itr->second;

        return nullptr;
    }

    Mutex _TransportLock;

private:
    TransportHandler();
    ~TransportHandler();
    TransportHandler(TransportHandler const&) = delete;
    TransportHandler& operator=(TransportHandler const&) = delete;

    void AddPathNodeToTransport(uint32 transportEntry, uint32 timeSeg, DBC::Structures::TransportAnimationEntry const* node);

    void AddPathRotationToTransport(uint32 transportEntry, uint32 timeSeg, DBC::Structures::TransportRotationEntry const* node)
    {
        _transportAnimations[transportEntry].Rotations[timeSeg] = node;
    }

    // Container storing transport animations
    TransportAnimationContainer _transportAnimations;

    // Generates and precaches a path for transport
    void GeneratePath(GameObjectProperties const* goInfo, TransportTemplate* transport);

    // Container storing transport templates
    TransportTemplates _transportTemplates;

    // Container storing transport entries to create for instanced maps
    TransportInstanceMap _instanceTransports;

    // Transport Storing
    TransporterSet _Transports;

    // Container storing transport entries
    TransporterMap _Transporters;
};

#define sTransportHandler TransportHandler::getInstance()