/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellTargetConstraint.h"
#include <algorithm>

SpellTargetConstraint::~SpellTargetConstraint()
{
    m_creatureTargets.clear();
    m_gameobjectTargets.clear();
    m_explicitTargets.clear();
}

bool SpellTargetConstraint::hasCreature(uint32_t entryId) const
{
    return find(begin(m_creatureTargets), end(m_creatureTargets), entryId) != end(m_creatureTargets);
}

bool SpellTargetConstraint::hasGameObject(uint32_t entryId) const
{
    return find(begin(m_gameobjectTargets), end(m_gameobjectTargets), entryId) != end(m_gameobjectTargets);
}

void SpellTargetConstraint::addCreature(uint32_t entryId)
{
    if (!hasCreature(entryId))
        m_creatureTargets.push_back(entryId);
}

void SpellTargetConstraint::addGameObject(uint32_t entryId)
{
    if (!hasGameObject(entryId))
        m_gameobjectTargets.push_back(entryId);
}

void SpellTargetConstraint::addExplicitTarget(uint32_t entryId)
{
    if (!hasExplicitTarget(entryId))
        m_explicitTargets.push_back(entryId);
}

bool SpellTargetConstraint::hasExplicitTarget(uint32_t entryId) const
{
    return find(begin(m_explicitTargets), end(m_explicitTargets), entryId) != end(m_explicitTargets);
}

std::vector<uint32_t> SpellTargetConstraint::getCreatures() const
{
    return m_creatureTargets;
}

std::vector<uint32_t> SpellTargetConstraint::getGameObjects() const
{
    return m_gameobjectTargets;
}
