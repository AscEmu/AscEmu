/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureAISummonList.hpp"
#include "CreatureAIScript.hpp"
#include "Objects/Units/Creatures/Creature.h"

void SummonList::summon(Creature const* summon)
{
    _storage.push_back(summon->getGuid());
}

void SummonList::despawn(Creature const* summon)
{
    _storage.remove(summon->getGuid());
}

void SummonList::despawnEntry(uint32_t entry)
{
    for (StorageType::iterator i = _storage.begin(); i != _storage.end();)
    {
        Creature* summon = _creature->getWorldMapCreature(*i);
        if (!summon)
        {
            i = _storage.erase(i);
        }
        else if (summon->getEntry() == entry)
        {
            i = _storage.erase(i);
            summon->Despawn(1000, 0);
        }
        else
        {
            ++i;
        }
    }
}

void SummonList::despawnAll()
{
    while (!_storage.empty())
    {
        Creature* summon = _creature->getWorldMapCreature(_storage.front());
        _storage.pop_front();
        if (summon)
            summon->Despawn(1000, 0);
    }
}

void SummonList::DoActionForEntry(int32_t info, uint32_t entry)
{
    StorageType listCopy = _storage;
    for (const auto& element : _storage)
    {
        if (Creature* creature = _creature->getWorldMapCreature(element))
        {
            if (creature->getEntry() == entry) {
                listCopy.push_back(element);
            }
        }
    }

    doAction(info, listCopy);
}

void SummonList::removeNotExisting()
{
    for (StorageType::iterator i = _storage.begin(); i != _storage.end();)
    {
        if (_creature->getWorldMapCreature(*i))
            ++i;
        else
            i = _storage.erase(i);
    }
}

bool SummonList::hasEntry(uint32_t entry) const
{
    for (uint64_t const& guid : _storage)
    {
        Creature* summon = _creature->getWorldMapCreature(guid);
        if (summon && summon->getEntry() == entry)
            return true;
    }

    return false;
}

void SummonList::doAction(int32_t action, StorageType const& summons)
{
    for (uint64_t const& guid : _storage)
    {
        Creature* summon = _creature->getWorldMapCreature(guid);
        if (summon && summon->GetScript())
            summon->GetScript()->DoAction(action);
    }
}
