/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>
#include <map>

class SpellTargetConstraint
{
    std::vector<int> m_creatureTargets;
    std::vector<int> m_gameobjectTargets;
    std::map<uint32_t, uint32_t> m_targetFocus;
public:
    SpellTargetConstraint();
    ~SpellTargetConstraint();

    bool hasCreature(int id);
    bool hasGameObject(int id);

    void addCreature(int id);
    void addGameObject(int id);

    void addFocused(uint32_t value, uint32_t type);
    bool isFocused(uint32_t value);

    std::vector<int> getCreatures() const;
    std::vector<int> getGameObjects() const;
};
