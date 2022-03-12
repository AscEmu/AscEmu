/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//////////////////////////////////////////////////////////////////////////////////////////
//\brief This script will add support for an SQL table called gameobject_teleports.
// This table can be used to teleport players when they use a game object such
// as a door or portal. Any object used in this table should exist in the
// gameobject_properties table, and be of type 10. Custom portal can use the generic
// display id of 6831. Portals also have the Sound2 field set to '1'.
//////////////////////////////////////////////////////////////////////////////////////////

#include "Setup.h"
#include "Server/MainServerDefines.h"

struct GameobjectTeleport
{
    uint32_t mapid;
    float x, y, z, o;
    uint32_t req_level;
    uint8_t req_class;
    uint32_t req_achievement;
};

std::map<uint32_t, GameobjectTeleport*> m_teleStorage;

class CustomTeleport : public GameObjectAIScript // Custom Portals
{
public:
    explicit CustomTeleport(GameObject* goinstance) : GameObjectAIScript(goinstance) {}

    ~CustomTeleport() override
    {}

    void OnActivate(Player* pPlayer) override
    {
        std::map<uint32_t, GameobjectTeleport*>::iterator itr = m_teleStorage.find(this->_gameobject->getEntry());
        if (itr != m_teleStorage.end())
        {
            GameobjectTeleport* gt = itr->second;
            uint32_t required_level = gt->req_level;
            uint8_t req_class = gt->req_class;
            uint32_t req_achievement = gt->req_achievement;

            if (required_level > pPlayer->getLevel())
            {
                pPlayer->broadcastMessage("You must be at least level %u to use this portal", required_level);
                return;
            }
            else if (req_class != 0 && req_class != pPlayer->getClass())
            {
                pPlayer->broadcastMessage("You do not have the required class to use this Portal");
                return;
            }
#if VERSION_STRING > TBC
            else if (req_achievement != 0 && pPlayer->getAchievementMgr().HasCompleted(req_achievement))
            {
                pPlayer->broadcastMessage("You do not have the required achievement to use this Portal");
                return;
            }
#endif
            else
            {
                uint32_t mapid = gt->mapid;
                LocationVector location;
                location.x = gt->x;
                location.y = gt->y;
                location.z = gt->z;
                location.o = gt->o;

                pPlayer->safeTeleport(mapid, 0, location);
            }
        }
    }
    static GameObjectAIScript* Create(GameObject* GO) { return new CustomTeleport(GO); }
};

void InitializeGameObjectTeleportTable(ScriptMgr* mgr)
{
    QueryResult* result = WorldDatabase.Query("SELECT entry, mapid, x_pos, y_pos, z_pos, orientation, required_level, required_class, required_achievement FROM gameobject_teleports");
    if (result != NULL)
    {
        // Check if the SQL table is setup correctly
        if (result->GetFieldCount() < 9)
        {
            DLLLogDetail("Error: Custom portals disabled, invalid 'gameobject_teleports' table.");
            delete result;
            return;
        }
        do
        {
            GameobjectTeleport* gt = new GameobjectTeleport;
            Field* fields = result->Fetch();
            uint32_t entry = fields[0].GetUInt32();
            gt->mapid = fields[1].GetUInt32();
            gt->x = fields[2].GetFloat();
            gt->y = fields[3].GetFloat();
            gt->z = fields[4].GetFloat();
            gt->o = fields[5].GetFloat();
            gt->req_level = fields[6].GetUInt32();
            gt->req_class = fields[7].GetUInt8();
            gt->req_achievement = fields[8].GetUInt32();
            m_teleStorage[entry] = gt;
            mgr->register_gameobject_script(entry, &CustomTeleport::Create);
        } while (result->NextRow());
        delete result;
    }
}
