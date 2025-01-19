/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GameObjectAIScript.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Server/EventMgr.h"

GameObjectAIScript::GameObjectAIScript(GameObject* _goinstance) : _gameobject(_goinstance)
{
}

void GameObjectAIScript::ModifyAIUpdateEvent(uint32_t _newfrequency)
{
    sEventMgr.ModifyEventTimeAndTimeLeft(_gameobject, EVENT_SCRIPT_UPDATE_EVENT, _newfrequency);
}

void GameObjectAIScript::RemoveAIUpdateEvent()
{
    sEventMgr.RemoveEvents(_gameobject, EVENT_SCRIPT_UPDATE_EVENT);
}

void GameObjectAIScript::RegisterAIUpdateEvent(uint32_t _frequency)
{
    sEventMgr.AddEvent(_gameobject, &GameObject::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, _frequency, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance
InstanceScript* GameObjectAIScript::getInstanceScript() const
{
    WorldMap* mapMgr = _gameobject->getWorldMap();
    return mapMgr ? mapMgr->getScript() : nullptr;
}

bool GameObjectAIScript::isHeroic() const
{
    WorldMap* mapMgr = _gameobject->getWorldMap();
    if (mapMgr == nullptr || mapMgr->getDifficulty() != InstanceDifficulty::DUNGEON_HEROIC)
        return false;

    return true;
}

unsigned GameObjectAIScript::getRaidModeValue(const unsigned& normal10, const unsigned& normal25, const unsigned& heroic10, const unsigned& heroic25) const
{
    if (_gameobject->getWorldMap()->getInstance())
    {
        switch (_gameobject->getWorldMap()->getDifficulty())
        {
            case InstanceDifficulty::RAID_10MAN_NORMAL:
                return normal10;
            case InstanceDifficulty::RAID_25MAN_NORMAL:
                return normal25;
            case InstanceDifficulty::RAID_10MAN_HEROIC:
                return heroic10;
            case InstanceDifficulty::RAID_25MAN_HEROIC:
                return heroic25;
            default:
                break;
        }
    }

    return normal10;
}
