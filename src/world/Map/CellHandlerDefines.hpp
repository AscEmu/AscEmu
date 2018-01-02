/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#define TilesCount 64
#define TileSize 533.33333f
#define _minY (-TilesCount * TileSize / 2)
#define _minX (-TilesCount * TileSize / 2)

#define _maxY (TilesCount * TileSize / 2)
#define _maxX (TilesCount * TileSize / 2)

#define CellsPerTile 8
#define _cellSize (TileSize / CellsPerTile)
#define _sizeX (TilesCount * CellsPerTile)
#define _sizeY (TilesCount * CellsPerTile)

#define GetRelatCoord(Coord, CellCoord) ((_maxX - Coord) - (CellCoord * _cellSize))
