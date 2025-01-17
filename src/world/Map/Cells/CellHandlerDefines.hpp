/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Map
{
    struct Terrain
    {
        inline static const float InvalidHeight = -50000.0f;

        inline static const float TileSize = 533.3333333f;

        inline static const int TilesCount = 64;
        inline static const int MapResoloution = 128;
        inline static const float MapSize = (TileSize * TilesCount);
        inline static const float MapHalfSize = (MapSize / 2);

        inline static const int MapCenter = (TilesCount / 2);
        inline static const float MapCenterOffset = (TileSize / 2);

        inline static const float _minY = (-TilesCount * TileSize / 2);
        inline static const float _minX = (-TilesCount * TileSize / 2);

        inline static const float _maxY = (TilesCount * TileSize / 2);
        inline static const float _maxX = (TilesCount * TileSize / 2);
    };

    struct Cell
    {
        inline static const int CellsPerTile = 8;
        inline static const float cellSize = (Terrain::TileSize / CellsPerTile);
        inline static const int _sizeX = (Terrain::TilesCount* CellsPerTile);
        inline static const int _sizeY = (Terrain::TilesCount * CellsPerTile);
    };
}
