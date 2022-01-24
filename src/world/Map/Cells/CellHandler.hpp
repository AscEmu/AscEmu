/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CellHandlerDefines.hpp"

class BaseMap;

template <class Class>
class CellHandler
{
public:

    CellHandler(BaseMap* map);
    ~CellHandler();

    Class* getCell(uint32_t x, uint32_t y);
    Class* getCellByCoords(float x, float y);
    Class* create(uint32_t x, uint32_t y);
    Class* createByCoords(float x, float y);
    void remove(uint32_t x, uint32_t y);

    bool allocated(uint32_t x, uint32_t y) { return _cells[x][y] != NULL; }

    static uint32_t getPosX(float x);
    static uint32_t getPosY(float y);

    BaseMap* getBaseMap() { return _map; }

protected:

    void _Init();

    Class*** _cells;

    BaseMap* _map;
};

template <class Class>
CellHandler<Class>::CellHandler(BaseMap* map)
{
    _map = map;
    _Init();
}

template <class Class>
void CellHandler<Class>::_Init()
{
    _cells = new Class * *[Map::Cell::_sizeX];

    if (_cells != nullptr)
    {
        for (uint32_t i = 0; i < Map::Cell::_sizeX; i++)
        {
            _cells[i] = nullptr;
        }
    }
}

template <class Class>
CellHandler<Class>::~CellHandler()
{
    if (_cells)
    {
        for (uint32_t i = 0; i < Map::Cell::_sizeX; i++)
        {
            if (!_cells[i])
                continue;

            for (uint32_t j = 0; j < Map::Cell::_sizeY; j++)
            {
                if (_cells[i][j])
                    delete _cells[i][j];
            }
            delete[] _cells[i];
        }
        delete[] _cells;
    }
}

template <class Class>
Class* CellHandler<Class>::create(uint32_t x, uint32_t y)
{
    if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY)
        return nullptr;

    if (!_cells[x])
    {
        _cells[x] = new Class * [Map::Cell::_sizeY];
        memset(_cells[x], 0, sizeof(Class*) * Map::Cell::_sizeY);
    }

    if (_cells[x][y] == nullptr)
    {
        Class* cls = new Class;
        _cells[x][y] = cls;

        return cls;
    }
    return nullptr;
}

template <class Class>
Class* CellHandler<Class>::createByCoords(float x, float y)
{
    return create(getPosX(x), getPosY(y));
}

template <class Class>
void CellHandler<Class>::remove(uint32_t x, uint32_t y)
{
    if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY)
        return;

    if (!_cells[x])
        return;

    if (_cells[x][y] != nullptr)
    {
        Class* cls = _cells[x][y];
        _cells[x][y] = NULL;

        delete cls;
    }
}

template <class Class>
Class* CellHandler<Class>::getCell(uint32_t x, uint32_t y)
{
    if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY || !_cells[x])
        return nullptr;
    return _cells[x][y];
}

template <class Class>
Class* CellHandler<Class>::getCellByCoords(float x, float y)
{
    return getCell(getPosX(x), getPosY(y));
}

template <class Class>
uint32 CellHandler<Class>::getPosX(float x)
{
    if ((x >= Map::Terrain::_minX) && (x <= Map::Terrain::_maxX))
        return (uint32)((Map::Terrain::_maxX - x) / Map::Cell::cellSize);
    return 0;
}

template <class Class>
uint32 CellHandler<Class>::getPosY(float y)
{
    if ((y >= Map::Terrain::_minY) && (y <= Map::Terrain::_maxY))
        return (uint32)((Map::Terrain::_maxY - y) / Map::Cell::cellSize);
    return 0;
}
