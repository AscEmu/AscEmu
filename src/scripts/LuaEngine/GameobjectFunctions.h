/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
extern "C"
{
    #include <lua/lua.h>
}

class GameObject;
class LuaGameObject
{
public:
    static int GossipCreateMenu          (lua_State* L, GameObject* ptr);
    static int GossipMenuAddItem         (lua_State* L, GameObject* ptr);
    static int GossipSendMenu            (lua_State* L, GameObject* ptr);
    static int GossipComplete            (lua_State* L, GameObject* ptr);
    static int GossipSendPOI             (lua_State* L, GameObject* ptr);
    static int GossipSendQuickMenu       (lua_State* L, GameObject* ptr);
    static int RegisterAIUpdate          (lua_State* L, GameObject* ptr);
    static int ModAIUpdate               (lua_State* L, GameObject* ptr);
    static int RemoveAIUpdate            (lua_State* L, GameObject* ptr);
    static int GetMapId                  (lua_State* L, GameObject* ptr);
    static int RemoveFromWorld           (lua_State* L, GameObject* ptr);
    static int GetName                   (lua_State* L, GameObject* ptr);
    static int GetCreatureNearestCoords  (lua_State* L, GameObject* ptr);
    static int GetGameObjectNearestCoords(lua_State* L, GameObject* ptr);
    static int GetClosestPlayer          (lua_State* L, GameObject* ptr);
    static int GetDistance               (lua_State* L, GameObject* ptr);
    static int IsInWorld                 (lua_State* L, GameObject* ptr);
    static int GetZoneId                 (lua_State* L, GameObject* ptr);
    static int PlaySoundToSet            (lua_State* L, GameObject* ptr);
    static int SpawnCreature             (lua_State* L, GameObject* ptr);
    static int SpawnGameObject           (lua_State* L, GameObject* ptr);
    static int GetSpawnX                 (lua_State* L, GameObject* ptr);
    static int GetSpawnY                 (lua_State* L, GameObject* ptr);
    static int GetSpawnZ                 (lua_State* L, GameObject* ptr);
    static int GetSpawnO                 (lua_State* L, GameObject* ptr);
    static int GetX                      (lua_State* L, GameObject* ptr);
    static int GetY                      (lua_State* L, GameObject* ptr);
    static int GetZ                      (lua_State* L, GameObject* ptr);
    static int GetO                      (lua_State* L, GameObject* ptr);
    static int GetInRangePlayersCount    (lua_State* L, GameObject* ptr);
    static int GetEntry                  (lua_State* L, GameObject* ptr);
    static int SetOrientation            (lua_State* L, GameObject* ptr);
    static int CalcRadAngle              (lua_State* L, GameObject* ptr);
    static int GetInstanceID             (lua_State* L, GameObject* ptr);
    static int GetInRangePlayers         (lua_State* L, GameObject* ptr);
    static int GetInRangeGameObjects     (lua_State* L, GameObject* ptr);
    static int GetInRangeUnits           (lua_State* L, GameObject* ptr);
    static int IsInFront                 (lua_State* L, GameObject* ptr);
    static int IsInBack                  (lua_State* L, GameObject* ptr);
    static int GetUInt32Value            (lua_State* L, GameObject* ptr);
    static int GetUInt64Value            (lua_State* L, GameObject* ptr);
    static int SetUInt32Value            (lua_State* L, GameObject* ptr);
    static int SetUInt64Value            (lua_State* L, GameObject* ptr);
    static int SetFloatValue             (lua_State* L, GameObject* ptr);
    static int RemoveFlag                (lua_State* L, GameObject* ptr);
    static int SetFlag                   (lua_State* L, GameObject* ptr);
    static int Update                    (lua_State* L, GameObject* ptr);
    static int GetFloatValue             (lua_State* L, GameObject* ptr);
    static int ModUInt32Value            (lua_State* L, GameObject* ptr);
    static int CastSpell                 (lua_State* L, GameObject* ptr);
    static int CastSpellOnTarget         (lua_State* L, GameObject* ptr);
    static int GetLandHeight             (lua_State* L, GameObject* ptr);
    static int SetZoneWeather            (lua_State* L, GameObject* ptr);
    static int GetDistanceYards          (lua_State* L, GameObject* ptr);
    static int PhaseSet                  (lua_State* L, GameObject* ptr);
    static int PhaseAdd                  (lua_State* L, GameObject* ptr);
    static int PhaseDelete               (lua_State* L, GameObject* ptr);
    static int GetPhase                  (lua_State* L, GameObject* ptr);
    static int SendPacket                (lua_State* L, GameObject* ptr);
    static int GetGUID                   (lua_State* L, GameObject* ptr);
    static int IsActive                  (lua_State* L, GameObject* ptr);
    static int Activate                  (lua_State* L, GameObject* ptr);
    static int DespawnObject             (lua_State* L, GameObject* ptr);
    static int AddLoot                   (lua_State* L, GameObject* ptr);
    static int GetInstanceOwner          (lua_State* L, GameObject* ptr);
    static int GetDungeonDifficulty      (lua_State* L, GameObject* ptr);
    static int SetDungeonDifficulty      (lua_State* L, GameObject* ptr);
    static int HasFlag                   (lua_State* L, GameObject* ptr);
    static int IsInPhase                 (lua_State* L, GameObject* ptr);
    static int GetSpawnId                (lua_State* L, GameObject* ptr);
    static int GetAreaId                 (lua_State* L, GameObject* ptr);
    static int SetPosition               (lua_State* L, GameObject* ptr);
    static int GetObjectType             (lua_State* L, GameObject* ptr);
    static int ChangeScale               (lua_State* L, GameObject* ptr);
    static int GetByte                   (lua_State* L, GameObject* ptr);
    static int SetByte                   (lua_State* L, GameObject* ptr);
    static int FullCastSpellOnTarget     (lua_State* L, GameObject* ptr);
    static int FullCastSpell             (lua_State* L, GameObject* ptr);
    static int CustomAnimate             (lua_State* L, GameObject* ptr);
    static int GetLocation               (lua_State* L, GameObject* ptr);
    static int GetSpawnLocation          (lua_State* L, GameObject* ptr);
    static int GetWoWObject              (lua_State* L, GameObject* ptr);
    static int RegisterEvent             (lua_State* L, GameObject* ptr);
    static int RemoveEvents              (lua_State* L, GameObject* ptr);
    static int SetScale                  (lua_State* L, GameObject* ptr);
    static int GetScale                  (lua_State* L, GameObject* ptr);
    static int GetClosestUnit            (lua_State* L, GameObject* ptr);
    static int Damage                    (lua_State* L, GameObject* ptr);
    static int Rebuild                   (lua_State* L, GameObject* ptr);
    static int GetHP                     (lua_State* L, GameObject* ptr);
    static int GetMaxHP                  (lua_State* L, GameObject* ptr);
    static int GetWorldStateForZone      (lua_State* L, GameObject* ptr);
    static int SetWorldStateForZone      (lua_State* L, GameObject* ptr);
};
