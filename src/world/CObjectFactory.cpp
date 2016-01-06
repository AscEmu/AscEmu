/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CObjectFactory.h"
#include "ObjectStorage.h"

GameObject* CObjectFactory::CreateGameObject(uint32 Id, uint32 LowGUID)
{
    GameObjectInfo* gameobject_info = GameObjectNameStorage.LookupEntry(Id);
    if (gameobject_info == NULL)
        return NULL;

    GameObject* gameobject = nullptr;

    uint64 GUID = uint64((uint64(HIGHGUID_TYPE_GAMEOBJECT) << 32) | LowGUID);

    switch (gameobject_info->type)
    {

        case GAMEOBJECT_TYPE_DOOR:
            gameobject = new GameObject_Door(GUID);
            break;

        case GAMEOBJECT_TYPE_BUTTON:
            gameobject = new GameObject_Button(GUID);
            break;

        case GAMEOBJECT_TYPE_QUESTGIVER:
            gameobject = new GameObject_QuestGiver(GUID);
            break;

        case GAMEOBJECT_TYPE_CHEST:
            gameobject = new GameObject_Chest(GUID);
            break;

        case GAMEOBJECT_TYPE_TRAP:
            gameobject = new GameObject_Trap(GUID);
            break;

        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            gameobject = new GameObject_SpellFocus(GUID);
            break;

        case GAMEOBJECT_TYPE_GOOBER:
            gameobject = new GameObject_Goober(GUID);
            break;

        case GAMEOBJECT_TYPE_FISHINGNODE:
            gameobject = new GameObject_FishingNode(GUID);
            break;

        case GAMEOBJECT_TYPE_RITUAL:
            gameobject = new GameObject_Ritual(GUID);
            break;

        case GAMEOBJECT_TYPE_SPELLCASTER:
            gameobject = new GameObject_SpellCaster(GUID);
            break;

        case GAMEOBJECT_TYPE_FISHINGHOLE:
            gameobject = new GameObject_FishingHole(GUID);
            break;

        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            gameobject = new GameObject_Destructible(GUID);
            break;

        default:
            gameobject = new GameObject(GUID);
            break;
    }

    gameobject->SetInfo(gameobject_info);

    return gameobject;
}

void CObjectFactory::DisposeOf(Object* obj)
{
    delete obj;
}
