/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CellHandlerDefines.hpp"
#include "Map/Cells/MapCell.hpp"

#include <memory>
#include <algorithm>

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

    bool allocated(uint32_t x, uint32_t y) { return _cells != nullptr && (*_cells)[x] != nullptr && (*(*_cells)[x])[y] != nullptr; }

    static uint32_t getPosX(float x);
    static uint32_t getPosY(float y);

    BaseMap* getBaseMap() { return _map; }

protected:
    void _Init();

    std::unique_ptr<std::array<std::unique_ptr<std::array<std::unique_ptr<Class>, Map::Cell::_sizeY>>, Map::Cell::_sizeX>> _cells = nullptr;

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
    _cells = std::make_unique<std::array<std::unique_ptr<std::array<std::unique_ptr<Class>, Map::Cell::_sizeY>>, Map::Cell::_sizeX>>();

    if (_cells != nullptr)
    {
        std::fill(_cells->begin(), _cells->end(), nullptr);
    }
}

template <class Class>
CellHandler<Class>::~CellHandler()
{
    if (_cells != nullptr)
    {
        for (uint32_t i = 0; i < Map::Cell::_sizeX; i++)
        {
            if ((*_cells)[i] == nullptr)
                continue;

            std::fill((*_cells)[i]->begin(), (*_cells)[i]->end(), nullptr);
            (*_cells)[i] = nullptr;
        }
        _cells = nullptr;
    }
}

template <class Class>
Class* CellHandler<Class>::create(uint32_t x, uint32_t y)
{
    if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY)
        return nullptr;

    if (_cells == nullptr)
        return nullptr;

    if ((*_cells)[x] == nullptr)
        (*_cells)[x] = std::make_unique<std::array<std::unique_ptr<Class>, Map::Cell::_sizeY>>();

    if ((*(*_cells)[x])[y] == nullptr)
    {
        (*(*_cells)[x])[y] = std::make_unique<Class>();

        return (*(*_cells)[x])[y].get();
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

    if (_cells == nullptr || (*_cells)[x] == nullptr)
        return;

    (*(*_cells)[x])[y] = nullptr;
}

template <class Class>
Class* CellHandler<Class>::getCell(uint32_t x, uint32_t y)
{
    if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY || _cells == nullptr || (*_cells)[x] == nullptr)
        return nullptr;
    return (*(*_cells)[x])[y].get();
}

template <class Class>
Class* CellHandler<Class>::getCellByCoords(float x, float y)
{
    return getCell(getPosX(x), getPosY(y));
}

template <class Class>
uint32_t CellHandler<Class>::getPosX(float x)
{
    if ((x >= Map::Terrain::_minX) && (x <= Map::Terrain::_maxX))
        return (uint32_t)((Map::Terrain::_maxX - x) / Map::Cell::cellSize);
    return 0;
}

template <class Class>
uint32_t CellHandler<Class>::getPosY(float y)
{
    if ((y >= Map::Terrain::_minY) && (y <= Map::Terrain::_maxY))
        return (uint32_t)((Map::Terrain::_maxY - y) / Map::Cell::cellSize);
    return 0;
}
