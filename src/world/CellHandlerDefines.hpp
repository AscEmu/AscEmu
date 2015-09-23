/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CELLHANDLER_DEFINES_H
#define _CELLHANDLER_DEFINES_H

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

#endif // _CELLHANDLER_DEFINES_H
