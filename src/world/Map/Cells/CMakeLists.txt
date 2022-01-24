# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Map/Cells)

set(SRC_MAP_CELLS_FILES
    ${PATH_PREFIX}/CellHandler.hpp
    ${PATH_PREFIX}/CellHandlerDefines.hpp
    ${PATH_PREFIX}/MapCell.cpp
    ${PATH_PREFIX}/MapCell.hpp
    ${PATH_PREFIX}/TerrainMgr.cpp
    ${PATH_PREFIX}/TerrainMgr.hpp
)

source_group(Map\\Cells FILES ${SRC_MAP_CELLS_FILES})
unset(PATH_PREFIX)
