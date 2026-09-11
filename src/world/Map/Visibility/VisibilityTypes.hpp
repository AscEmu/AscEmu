/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>

#include "WoWGuid.hpp"
#include "Utilities/LocationVector.hpp"

class Object;
class Player;
class Creature;
class GameObject;
class Transporter;
class DynamicObject;
class Pet;
class Corpse;

//////////////////////////////////////////////////////////////////////////////////////////
/// Object Handle and Hash
//////////////////////////////////////////////////////////////////////////////////////////
namespace visibility
{
    /// Runtime visibility state belongs to the WorldMap thread.
    /// Basic object handle with generation for safe reuse protection.
    struct ObjectHandle
    {
        std::uint32_t id{ 0 };
        std::uint32_t gen{ 0 };
    };

    inline bool operator==(const ObjectHandle& a, const ObjectHandle& b)
    {
        return a.id == b.id && a.gen == b.gen;
    }
    inline bool operator!=(const ObjectHandle& a, const ObjectHandle& b)
    {
        return !(a == b);
    }
};

namespace std
{
    /// Hash support for unordered containers.
    template<>
    struct hash<visibility::ObjectHandle>
    {
        size_t operator()(const visibility::ObjectHandle& h) const noexcept
        {
            return (static_cast<size_t>(h.id) << 32) ^ static_cast<size_t>(h.gen);
        }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Visibility / Grid System
//////////////////////////////////////////////////////////////////////////////////////////
namespace visibility
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Terrain constants and derived map metrics.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct Terrain
    {
        inline static constexpr float InvalidHeight = -50000.0f;
        inline static constexpr float TileSize = 533.3333333f;
        inline static constexpr int TilesCount = 64;               /// 64x64 tiles
        inline static constexpr int MapResolution = 128;
        inline static constexpr float MapSize = TileSize * TilesCount;
        inline static constexpr float MapHalf = MapSize * 0.5f;
        inline static constexpr int MapCenter = (TilesCount / 2);
        inline static constexpr float MapCenterOffset = (TileSize / 2);
        inline static constexpr float MinX = -MapHalf;
        inline static constexpr float MinY = -MapHalf;
        inline static constexpr float MaxX = MapHalf;
        inline static constexpr float MaxY = MapHalf;
    };

    /// Logical visibility cell within a terrain tile.
    struct Cell
    {
        inline static constexpr int CellsPerTile = 8;                /// 8x8 per tile
        inline static constexpr float Size = Terrain::TileSize / CellsPerTile;
        inline static constexpr int SizeX = Terrain::TilesCount * CellsPerTile; /// 512
        inline static constexpr int SizeY = Terrain::TilesCount * CellsPerTile;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Object container categories.
    //////////////////////////////////////////////////////////////////////////////////////////
    enum class Container : std::uint8_t
    {
        Corpses                         = 0,
        Creatures                       = 1,
        DynamicObjects                  = 2,
        GameObjects                     = 3,
        Players                         = 4,
        Transporter                     = 5,
        Pets                            = 6,
        Unknown                             = 7,
        Count                           = 8
    };

    template<typename T> struct TypeMap;
    template<>
    struct TypeMap<Player>
    {
        static constexpr Container container = Container::Players;
    };
    template<>
    struct TypeMap<Creature>
    {
        static constexpr Container container = Container::Creatures;
    };
    template<>
    struct TypeMap<GameObject>
    {
        static constexpr Container container = Container::GameObjects;
    };
    template<>
    struct TypeMap<Transporter>
    {
        static constexpr Container container = Container::Transporter;
    };
    template<>
    struct TypeMap<DynamicObject>
    {
        static constexpr Container container = Container::DynamicObjects;
    };
    template<>
    struct TypeMap<Pet>
    {
        static constexpr Container container = Container::Pets;
    };
    template<>
    struct TypeMap<Corpse>
    {
        static constexpr Container container = Container::Corpses;
    };
    template<>
    struct TypeMap<Object>
    {
        static constexpr Container container = Container::Unknown;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Sparse cell and grid data.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct CellChunk
    {
        std::array<std::vector<ObjectHandle>, static_cast<std::size_t>(Container::Count)> byContainer{};
        std::vector<ObjectHandle> viewers;     /// Visibility receivers
        std::vector<ObjectHandle> activators;  /// Grid/cell activators
        uint32_t epoch{ 0 };                    /// Increment on structure change
    };

    struct GridChunk
    {
        std::array<std::vector<ObjectHandle>, static_cast<std::size_t>(Container::Count)> owners{};
        std::array<std::unique_ptr<CellChunk>, static_cast<std::size_t>(Cell::CellsPerTile * Cell::CellsPerTile)> cells{};

        int activeCells{ 0 };
        std::unordered_set<ObjectHandle> autonomousResidents; /// Runtime objects that keep only this grid resident.
        std::chrono::steady_clock::time_point idleSince{};
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Minimal meta data per object used for visibility logic.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct ObjectMeta
    {
        WoWGuid   guid{ };
        Container container{ Container::Unknown };
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Coordinate helpers.
    //////////////////////////////////////////////////////////////////////////////////////////
    using GridCoord = std::pair<int, int>; /// 0..63
    using LocalCell = std::pair<int, int>; /// 0..7 inside a tile
    using CellRef = std::pair<int, int>;

    inline int packGridId(int gridX, int gridY)
    {
        return (gridY * Terrain::TilesCount) + gridX;
    }
    inline auto unpackGridId(int gridId)
    {
        return std::pair{ gridId % Terrain::TilesCount, gridId / Terrain::TilesCount };
    }
    inline int packCellId(int cellX, int cellY)
    {
        return (cellY * Cell::CellsPerTile) + cellX;
    }
    inline std::pair<int, int> unpackCellId2(int localCellId)
    {
        const int cellsPerTile = visibility::Cell::CellsPerTile;
        return { localCellId % cellsPerTile, localCellId / cellsPerTile };
    }

    /// Converts world coordinates to grid coordinates (clamped).
    inline GridCoord worldToGrid(const LocationVector& p)
    {
        const float ox = p.x - Terrain::MinX;
        const float oy = p.y - Terrain::MinY;
        int gx = static_cast<int>(std::floor(ox / Terrain::TileSize));
        int gy = static_cast<int>(std::floor(oy / Terrain::TileSize));
        gx = std::clamp(gx, 0, Terrain::TilesCount - 1);
        gy = std::clamp(gy, 0, Terrain::TilesCount - 1);
        return { gx, gy };
    }

    /// Converts world coordinates to local cell coordinates (clamped).
    inline LocalCell worldToLocal(const LocationVector& p) noexcept
    {
        const float ox = p.x - Terrain::MinX;
        const float oy = p.y - Terrain::MinY;
        const int gx = static_cast<int>(std::floor(ox / Terrain::TileSize));
        const int gy = static_cast<int>(std::floor(oy / Terrain::TileSize));
        const float lx = ox - gx * Terrain::TileSize;
        const float ly = oy - gy * Terrain::TileSize;
        int cx = static_cast<int>(std::floor(lx / Cell::Size));
        int cy = static_cast<int>(std::floor(ly / Cell::Size));
        cx = std::clamp(cx, 0, Cell::CellsPerTile - 1);
        cy = std::clamp(cy, 0, Cell::CellsPerTile - 1);
        return { cx, cy };
    }

    /// Calculates Chebyshev distance between two cells across grid boundaries.
    inline int cellChebDistGlobal(int gidA, int lcxA, int lcyA, int gidB, int lcidB)
    {
        auto [gxA, gyA] = unpackGridId(gidA);
        auto [cxB, cyB] = unpackCellId2(lcidB);
        auto [gxB, gyB] = unpackGridId(gidB);

        const int C = visibility::Cell::CellsPerTile;
        const int ax = gxA * C + lcxA;
        const int ay = gyA * C + lcyA;
        const int bx = gxB * C + cxB;
        const int by = gyB * C + cyB;
        const int dx = std::abs(bx - ax);
        const int dy = std::abs(by - ay);
        return dx > dy ? dx : dy;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Subscription state (per object).
    //////////////////////////////////////////////////////////////////////////////////////////
    struct SubState
    {
        int  radius{ 0 };          /// Visibility radius in cells (Chebyshev)
        bool viewer{ false };      /// Is a visibility receiver
        bool activator{ false };   /// Activates surrounding cells
        bool active{ false };      /// Currently active
        int  gid{ 0 };             /// Current grid ID
        int  lcx{ 0 };             /// Local cell X
        int  lcy{ 0 };             /// Local cell Y

        /// Last position that triggered an expensive viewer-interest refresh
        /// (proximitySweepViewer + published/special refresh). This throttles
        /// same-cell micro movement without delaying cell-boundary updates.
        bool lastInterestRefreshValid{ false };
        LocationVector lastInterestRefreshPos{};
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Broadcast state (per object).
    //////////////////////////////////////////////////////////////////////////////////////////
    enum class PublishMode : std::uint8_t
    {
        None,
        CellRadius,
        GridWide
    };

    struct InterestProfile
    {
        bool viewer{ false };
        bool activator{ false };
        bool autonomous{ false }; /// Exists/updates independently of normal grid activation.

        int viewerSubscribeCells{ 0 };
        int activatorSubscribeCells{ 0 };

        PublishMode publishMode{ PublishMode::None };
        int publishCells{ 0 };
        bool publishPlayersOnly{ true };
    };

    struct PubState
    {
        int extraCells { 0 };
        bool playersOnly { false };
        PublishMode mode { PublishMode::None };
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Cached visibility lists per object.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct VisibleCache
    {
        std::unordered_set<uint64_t> any;

        inline void reserve(std::size_t n)
        {
            any.reserve(n);
        }

        inline void clear()
        {
            any.clear();
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Event hub types and containers.
    //////////////////////////////////////////////////////////////////////////////////////////
    using VisibleCb = std::function<void(WoWGuid viewer, WoWGuid object)>;
    using HiddenCb = std::function<void(WoWGuid viewer, WoWGuid object)>;
    using GridMoveCb = std::function<void(WoWGuid object, int fromGrid, int toGrid)>;
    using GridEventCb = std::function<void(int gid)>;
    using InterestProfileCb = std::function<void(WoWGuid object, const InterestProfile& profile)>;

    struct EventHub
    {
        std::vector<VisibleCb>      onVisible;
        std::vector<HiddenCb>       onHidden;
        std::vector<GridMoveCb>     onGridChanged;
        std::vector<GridEventCb>    onGridActivated;
        std::vector<GridEventCb>    onGridDeactivated;
        std::vector<GridEventCb>    onGridUnload;
        std::vector<InterestProfileCb> onInterestProfileChanged;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Runtime configuration for the visibility system.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct Config
    {
        int defaultViewerRadius{ 2 };
        int defaultActivatorRadius{ defaultViewerRadius + 1 };
        /// Cell radius used by grid-wide publishers.
        int gridWidePublishCells{ 16 };
        /// Extra Player-to-Player publish radius.
        int playerPublishCells{ 4 };
        /// Publish radius for gameobjects flagged as large.
        int largeGameObjectPublishCells{ 4 };
        /// Distance before same-cell viewer movement refreshes interest again.
        float viewerInterestRefreshDistance{ 4.0f };
        /// Distance before same-cell object movement refreshes nearby viewers.
        float movedObjectInterestRefreshDistance{ 4.0f };
        std::chrono::milliseconds cellUnloadDelay{ std::chrono::minutes(5) };
    };

} /// namespace visibility
