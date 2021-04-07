/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct KeyFrame;
struct TransportTemplate;
struct GameObjectProperties;
class Transporter;

namespace MovementNew
{
    template <typename length_type> class Spline;
}

typedef MovementNew::Spline<double>                         TransportSpline;
typedef std::vector<KeyFrame>                               KeyFrameVec;
typedef std::unordered_map<uint32_t, TransportTemplate>     TransportTemplates;
typedef std::set<Transporter*>                              TransporterSet;
typedef std::unordered_map<uint32_t, Transporter*>          TransporterMap;
typedef std::unordered_map<uint32_t, TransporterSet>        TransporterInstancedMap;
typedef std::unordered_map<uint32_t, std::set<uint32_t>>    TransportInstanceMap;

typedef std::vector<DBC::Structures::TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

typedef std::map<uint32_t, DBC::Structures::TransportAnimationEntry const*> TransportPathContainer;
#if VERSION_STRING >= WotLK
typedef std::map<uint32_t, DBC::Structures::TransportRotationEntry const*> TransportPathRotationContainer;
#endif

struct PathNode
{
    uint32_t mapid;
    float x, y, z;
    uint32_t flags;
    uint32_t delay;
    uint32_t ArrivalEventID;
    uint32_t DepartureEventID;
};

class TransportPath
{
public:
    inline void setLength(const unsigned int sz)
    {
        i_nodes.resize(sz);
    }

    inline size_t size(void) const { return i_nodes.size(); }
    inline void resize(unsigned int sz) { i_nodes.resize(sz); }
    inline void clear(void) { i_nodes.clear(); }
    inline PathNode* getNodes(void) { return static_cast<PathNode*>(&i_nodes[0]); }
    float getTotalLength(void)
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
        Teleport(false), ArriveTime(0), DepartureTime(0), Spline(nullptr), NextDistFromPrev(0.0f), NextArriveTime(0)
    {
    }

    uint32_t Index;
    PathNode Node;
    float InitialOrientation;
    float DistSinceStop;
    float DistUntilStop;
    float DistFromPrev;
    float TimeFrom;
    float TimeTo;
    bool Teleport;
    uint32_t ArriveTime;
    uint32_t DepartureTime;
    std::shared_ptr<TransportSpline> Spline;

    // Data needed for next frame
    float NextDistFromPrev;
    uint32_t NextArriveTime;

    bool isTeleportFrame() const { return Teleport; }
    bool isStopFrame() const { return Node.flags == 2; }
};

struct TransportTemplate
{
    TransportTemplate() : lowguid(0), inInstance(false), pathTime(0), accelTime(0.0f), accelDist(0.0f), entry(0) { }
    ~TransportTemplate() = default;

    uint32_t lowguid;
    std::set<uint32_t> mapsUsed;
    bool inInstance;
    uint32_t pathTime;
    KeyFrameVec keyFrames;
    float accelTime;
    float accelDist;

    uint32_t entry;
};

struct SERVER_DECL TransportAnimation
{
    TransportAnimation() : TotalTime(0) { }

    TransportPathContainer Path;
#if VERSION_STRING >= WotLK
    TransportPathRotationContainer Rotations;
#endif
    uint32_t TotalTime;

    DBC::Structures::TransportAnimationEntry const* getAnimNode(uint32_t time) const;
#if VERSION_STRING >= WotLK
    DBC::Structures::TransportRotationEntry const* getAnimRotation(uint32_t time) const;
#endif
};

typedef std::map<uint32_t, TransportAnimation> TransportAnimationContainer;

class SERVER_DECL TransportHandler
{
public:
    static TransportHandler& getInstance();

    void unload();

    void loadTransportTemplates();
    void loadTransportAnimationAndRotation();
    void loadTransportForPlayers(Player* player);

    // Creates a transport using given GameObject template entry
    Transporter* createTransport(uint32_t entry, MapMgr* map = nullptr);

    // Spawns all continent transports, used at startup
    void spawnContinentTransports();

    // constrain arbitrary radian orientation to interval [0,2*PI)
    static float normalizeOrientation(float o);

    void addTransport(Transporter* transport);

    Transporter* getTransporter(uint32_t guid);
    TransportTemplate const* getTransportTemplate(uint32_t entry) const;
    TransportAnimation const* getTransportAnimInfo(uint32_t entry) const;

    Mutex _TransportLock;

private:
    TransportHandler() = default;
    ~TransportHandler() = default;
    TransportHandler(TransportHandler const&) = delete;
    TransportHandler& operator=(TransportHandler const&) = delete;

    void addPathNodeToTransport(uint32_t transportEntry, uint32_t timeSeg, DBC::Structures::TransportAnimationEntry const* node);
#if VERSION_STRING >= WotLK
    void addPathRotationToTransport(uint32_t transportEntry, uint32_t timeSeg, DBC::Structures::TransportRotationEntry const* node);
#endif

    // Container storing transport animations
    TransportAnimationContainer _transportAnimations;

    // Generates and precaches a path for transport
    void generatePath(GameObjectProperties const* goInfo, TransportTemplate* transport);

    // Container storing transport templates
    TransportTemplates _transportTemplates;

    // Container storing transport entries to create for instanced maps
    TransportInstanceMap _instanceTransports;

    // Transport Storing
    TransporterSet _Transports;

    // Container storing transport entries
    TransporterMap _Transporters;

    // Container storing transport entries in instances
    TransporterInstancedMap _TransportersByInstanceIdMap;
};

#define sTransportHandler TransportHandler::getInstance()
