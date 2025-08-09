/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

template<class Entity, class BasePath>
class PathMovementBase
{
public:
    PathMovementBase() : _path(), _currentNode(0) { }
    virtual ~PathMovementBase() { };

    uint32_t getCurrentNode() const { return _currentNode; }

    virtual std::string getDebugInfo() const
    {
        return "Current Node: " + std::to_string(getCurrentNode());
    };

protected:
    BasePath _path;
    uint32_t _currentNode;
};
