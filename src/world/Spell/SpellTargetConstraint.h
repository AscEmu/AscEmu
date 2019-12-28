/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>

enum SpellTargetConstraintType : uint8_t
{
    SPELL_CONSTRAINT_EXPLICIT_CREATURE    = 0,
    SPELL_CONSTRAINT_EXPLICIT_GAMEOBJECT,
    SPELL_CONSTRAINT_IMPLICIT_CREATURE,
    SPELL_CONSTRAINT_IMPLICIT_GAMEOBJECT
};

class SpellTargetConstraint
{
    private:
        std::vector<uint32_t> m_creatureTargets;
        std::vector<uint32_t> m_gameobjectTargets;
        std::vector<uint32_t> m_explicitTargets;

    public:
        SpellTargetConstraint() = default;
        ~SpellTargetConstraint();

        bool hasCreature(uint32_t entryId) const;
        bool hasGameObject(uint32_t entryId) const;

        void addCreature(uint32_t entryId);
        void addGameObject(uint32_t entryId);

        // Explicit target = requires caster to target it
        void addExplicitTarget(uint32_t entryId);
        bool hasExplicitTarget(uint32_t value) const;

        std::vector<uint32_t> getCreatures() const;
        std::vector<uint32_t> getGameObjects() const;
};
