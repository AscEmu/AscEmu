/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellTargetConstraint.h"
#include <algorithm>

SpellTargetConstraint::SpellTargetConstraint()
{
}

SpellTargetConstraint::~SpellTargetConstraint()
{
    m_creatureTargets.clear();
    m_gameobjectTargets.clear();
}

bool SpellTargetConstraint::hasCreature(int id)
{
    return find(begin(m_creatureTargets), end(m_creatureTargets), id) != end(m_creatureTargets);
}

bool SpellTargetConstraint::hasGameObject(int id)
{
    return find(begin(m_gameobjectTargets), end(m_gameobjectTargets), id) != end(m_gameobjectTargets);
}

void SpellTargetConstraint::addCreature(int id)
{
    if (!hasCreature(id))
    {
        m_creatureTargets.push_back(id);
    }
}

void SpellTargetConstraint::addGameObject(int id)
{
    if (!hasGameObject(id))
    {
        m_gameobjectTargets.push_back(id);
    }
}

void SpellTargetConstraint::addFocused(uint32_t value, uint32_t type)
{
    m_targetFocus.insert(std::make_pair(value, type));
}

bool SpellTargetConstraint::isFocused(uint32_t value)
{
    auto target = m_targetFocus.find(value);
    return target != m_targetFocus.end() ? target->second != 0 : false;
}

std::vector<int> SpellTargetConstraint::getCreatures() const
{
    return m_creatureTargets;
}

std::vector<int> SpellTargetConstraint::getGameObjects() const
{
    return m_gameobjectTargets;
}
