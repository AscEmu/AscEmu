/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MapDefines.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Movement/Spline/MoveSplineInitArgs.h"
#include <G3D/Vector3.h>
#include "Macros/AIInterfaceMacros.hpp"

class Unit;
class Object;

enum PathType
{
    PATHFIND_BLANK             = 0x00,   // path not built yet
    PATHFIND_NORMAL            = 0x01,   // normal path
    PATHFIND_SHORTCUT          = 0x02,   // travel through obstacles, terrain, air, etc (old behavior)
    PATHFIND_INCOMPLETE        = 0x04,   // we have partial path to follow - getting closer to target
    PATHFIND_NOPATH            = 0x08,   // no valid path at all or error in generating one
    PATHFIND_NOT_USING_PATH    = 0x10,   // used when we are either flying/swiming or on map w/o mmaps
    PATHFIND_SHORT             = 0x20,   // path is longer or equal to its limited path length
    PATHFIND_FARFROMPOLY_START = 0x40,   // start position is far from the mmap poligon
    PATHFIND_FARFROMPOLY_END   = 0x80,   // end positions is far from the mmap poligon
    PATHFIND_FARFROMPOLY       = PATHFIND_FARFROMPOLY_START | PATHFIND_FARFROMPOLY_END, // start or end positions are far from the mmap poligon
};

class SERVER_DECL PathGenerator
{
public:
    explicit PathGenerator(Object* owner);
    ~PathGenerator() = default;

    // Calculate the path from owner to given destination
    // return: true if new path was calculated, false otherwise (no change needed)
    bool calculatePath(float destX, float destY, float destZ, bool forceDest = false);
    bool isInvalidDestinationZ(Unit const* target) const;

    // option setters - use optional
    void setUseStraightPath(bool useStraightPath) { _useStraightPath = useStraightPath; }
    void setPathLengthLimit(float distance) { _pointPathLimit = std::min<uint32_t>(uint32_t(distance/SMOOTH_PATH_STEP_SIZE), MAX_POINT_PATH_LENGTH); }
    void setUseRaycast(bool useRaycast) { _useRaycast = useRaycast; }

    // result getters
    G3D::Vector3 const& getStartPosition() const { return _startPosition; }
    G3D::Vector3 const& getEndPosition() const { return _endPosition; }
    G3D::Vector3 const& getActualEndPosition() const { return _actualEndPosition; }

    MovementMgr::PointsArray const& getPath() const { return _pathPoints; }

    PathType getPathType() const { return _type; }

    // shortens the path until the destination is the specified distance from the target point
    void shortenPathUntilDist(G3D::Vector3 const& point, float dist);

private:
    dtPolyRef _pathPolyRefs[MAX_PATH_LENGTH];   // array of detour polygon references
    uint32_t _polyLength;                       // number of polygons in the path

    MovementMgr::PointsArray _pathPoints;       // our actual (x,y,z) path to the target
    PathType _type;                             // tells what kind of path this is

    bool _useStraightPath;                      // type of path will be generated
    bool _forceDestination;                     // when set, we will always arrive at given point
    uint32_t _pointPathLimit;                   // limit point path size; min(this, MAX_POINT_PATH_LENGTH)
    bool _useRaycast;                           // use raycast if true for a straight line path

    G3D::Vector3 _startPosition;                // {x, y, z} of current location
    G3D::Vector3 _endPosition;                  // {x, y, z} of the destination
    G3D::Vector3 _actualEndPosition;            // {x, y, z} of the closest possible point to given destination

    Object* _source;                            // the object that is moving
    dtNavMesh const* _navMesh;                  // the nav mesh
    dtNavMeshQuery const* _navMeshQuery;        // the nav mesh query used to find the path

    dtQueryFilter _filter;                      // use single filter for all movements, update it when needed

    void setStartPosition(G3D::Vector3 const& point) { _startPosition = point; }
    void setEndPosition(G3D::Vector3 const& point) { _actualEndPosition = point; _endPosition = point; }
    void setActualEndPosition(G3D::Vector3 const& point) { _actualEndPosition = point; }
    void normalizePath();

    void clear()
    {
        _polyLength = 0;
        _pathPoints.clear();
    }

    bool inRange(G3D::Vector3 const& p1, G3D::Vector3 const& p2, float r, float h) const;
    float dist3DSqr(G3D::Vector3 const& p1, G3D::Vector3 const& p2) const;
    bool inRangeYZX(float const* v1, float const* v2, float r, float h) const;

    dtPolyRef getPathPolyByPosition(dtPolyRef const* polyPath, uint32_t polyPathSize, float const* Point, float* Distance = nullptr) const;
    dtPolyRef getPolyByLocation(float const* Point, float* Distance) const;
    bool haveTile(G3D::Vector3 const& p) const;

    void buildPolyPath(G3D::Vector3 const& startPos, G3D::Vector3 const& endPos);
    void buildPointPath(float const* startPoint, float const* endPoint);
    void buildShortcut();

    NavTerrainFlag getNavTerrain(float x, float y, float z);
    void createFilter();
    void updateFilter();

    // smooth path aux functions
    uint32_t fixupCorridor(dtPolyRef* path, uint32_t npath, uint32_t maxPath, dtPolyRef const* visited, uint32_t nvisited);
    bool getSteerTarget(float const* startPos, float const* endPos, float minTargetDist, dtPolyRef const* path, uint32_t pathSize, float* steerPos,
                        unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
    dtStatus findSmoothPath(float const* startPos, float const* endPos,
                          dtPolyRef const* polyPath, uint32_t polyPathSize,
                          float* smoothPath, int* smoothPathSize, uint32_t smoothPathMaxSize);

    void addFarFromPolyFlags(bool startFarFromPoly, bool endFarFromPoly);
};
