/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Spell/Definitions/SpellFailure.hpp"
#include "Server/ServerState.h"
#include "Objects/ObjectMgr.h"
#include "Management/WeatherMgr.h"
#include "Server/Script/CreatureAIScript.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/ThreatHandler.h"

bool ChatHandler::HandleMoveHardcodedScriptsToDBCommand(const char* args, WorldSession* session)
{
    uint32_t map = uint32_t(atoi(args));
    if (map == 0)
        return true;

    std::vector<uint32_t> creatureEntries;

    QueryResult* creature_spawn_result = WorldDatabase.Query("SELECT entry FROM creature_spawns WHERE map = %u GROUP BY(entry)", map);
    if (creature_spawn_result)
    {
        {
            do
            {
                Field* fields = creature_spawn_result->Fetch();
                creatureEntries.push_back(fields[0].GetUInt32());

            } while (creature_spawn_result->NextRow());
        }

        delete creature_spawn_result;
    }

    //prepare new table for dump
    char my_table[1400];
    sprintf(my_table, "CREATE TABLE `creature_ai_scripts_%s` (`min_build` int NOT NULL DEFAULT '12340',`max_build` int NOT NULL DEFAULT '12340',`entry` int unsigned NOT NULL,\
            `difficulty` tinyint unsigned NOT NULL DEFAULT '0',`phase` tinyint unsigned NOT NULL DEFAULT '0',`event` tinyint unsigned NOT NULL DEFAULT '0',`action` tinyint unsigned NOT NULL DEFAULT '0',\
            `maxCount` tinyint unsigned NOT NULL DEFAULT '0',`chance` float unsigned NOT NULL DEFAULT '1',`spell` int unsigned NOT NULL DEFAULT '0',`spell_type` int NOT NULL DEFAULT '0',`triggered` tinyint(1) NOT NULL DEFAULT '0',\
            `target` tinyint NOT NULL DEFAULT '0',`cooldownMin` int NOT NULL DEFAULT '0',`cooldownMax` int unsigned NOT NULL DEFAULT '0',`minHealth` float NOT NULL DEFAULT '0',\
            `maxHealth` float NOT NULL DEFAULT '100',`textId` int unsigned NOT NULL DEFAULT '0',`misc1` int NOT NULL DEFAULT '0',`comments` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,\
            UNIQUE KEY `entry` (`min_build`,`max_build`,`entry`,`difficulty`,`phase`,`spell`,`event`,`action`,`textId`) USING BTREE) ENGINE = MyISAM DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = 'AI System'", args);

    WorldDatabase.Execute(my_table);

    uint32_t count = 0;
    for (auto entry : creatureEntries)
    {
        auto creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            RedSystemMessage(session, "Creature with entry %u is not a valid entry (no properties information in database)", entry);
            return true;
        }

        auto creature_spawn = new MySQLStructure::CreatureSpawn;
        uint8 gender = creature_properties->GetGenderAndCreateRandomDisplayID(&creature_spawn->displayid);
        creature_spawn->entry = entry;
        creature_spawn->id = sObjectMgr.GenerateCreatureSpawnID();
        creature_spawn->movetype = 0;
        creature_spawn->x = session->GetPlayer()->GetPositionX();
        creature_spawn->y = session->GetPlayer()->GetPositionY();
        creature_spawn->z = session->GetPlayer()->GetPositionZ();
        creature_spawn->o = session->GetPlayer()->GetOrientation();
        creature_spawn->emote_state = 0;
        creature_spawn->flags = creature_properties->NPCFLags;
        creature_spawn->factionid = creature_properties->Faction;
        creature_spawn->bytes0 = creature_spawn->setbyte(0, 2, gender);
        creature_spawn->bytes1 = 0;
        creature_spawn->bytes2 = 0;
        creature_spawn->stand_state = 0;
        creature_spawn->death_state = 0;
        creature_spawn->channel_target_creature = creature_spawn->channel_target_go = creature_spawn->channel_spell = 0;
        creature_spawn->MountedDisplayID = 0;

        creature_spawn->Item1SlotEntry = creature_properties->itemslot_1;
        creature_spawn->Item2SlotEntry = creature_properties->itemslot_2;
        creature_spawn->Item3SlotEntry = creature_properties->itemslot_3;

        creature_spawn->Item1SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(creature_spawn->Item1SlotEntry);
        creature_spawn->Item2SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(creature_spawn->Item2SlotEntry);
        creature_spawn->Item3SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(creature_spawn->Item3SlotEntry);
        creature_spawn->CanFly = 0;
        creature_spawn->phase = session->GetPlayer()->GetPhase();

        if (auto creature = session->GetPlayer()->GetMapMgr()->CreateCreature(entry))
        {
            creature->Load(creature_spawn, 0, nullptr);
            creature->m_loadedFromDB = true;
            creature->PushToWorld(session->GetPlayer()->GetMapMgr());

            // Add to map
            uint32 x = session->GetPlayer()->GetMapMgr()->GetPosX(session->GetPlayer()->GetPositionX());
            uint32 y = session->GetPlayer()->GetMapMgr()->GetPosY(session->GetPlayer()->GetPositionY());
            session->GetPlayer()->GetMapMgr()->GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(creature_spawn);
            MapCell* map_cell = session->GetPlayer()->GetMapMgr()->GetCell(x, y);
            if (map_cell != nullptr)
                map_cell->SetLoaded();

            for (auto aiSpells : creature->GetAIInterface()->mCreatureAISpells)
            {
                if (aiSpells->fromDB)
                    continue;

                float chance = aiSpells->mCastChance;
                uint32_t spell = aiSpells->mSpellInfo->getId();
                uint32_t spelltype = aiSpells->spell_type;
                uint32_t target = aiSpells->mTargetType;
                uint32_t cooldown = aiSpells->mCooldown;
                if (cooldown == 0xFFFFFFFF) //4294967295
                    cooldown = 10000;

                std::string remove = "'";
                std::string name = sMySQLStore.getCreatureProperties(entry)->Name;
                name.erase(std::remove_if(name.begin(), name.end(),
                    [&remove](const char& c) {
                        return remove.find(c) != std::string::npos;
                    }),
                    name.end());

                std::string spellname = aiSpells->mSpellInfo->getName();
                spellname.erase(std::remove_if(spellname.begin(), spellname.end(),
                    [&remove](const char& c) {
                        return remove.find(c) != std::string::npos;
                    }),
                    spellname.end());

                std::string comment = name + " - " + spellname;

                char my_insert1[700];
                sprintf(my_insert1, "INSERT INTO creature_ai_scripts_%s VALUES (5875,12340,%u,4,0,5,1,0,%f,%u,%u,0,%u,%u,%u,0,100,0,0,'%s')", args, entry, chance, spell, spelltype, target, cooldown, cooldown, comment.c_str());

                WorldDatabase.Execute(my_insert1);
                ++count;
            }

            creature->RemoveFromWorld(false, true);
        }
    }

    SystemMessage(session, "Dumped: %u hardcoded scripts to creature_ai_scripts_dump", count);

    return true;
}

bool ChatHandler::HandleDoPercentDamageCommand(const char* args, WorldSession* session)
{
    Creature* selected_unit = GetSelectedCreature(session);
    if (selected_unit == nullptr)
        return false;

    uint32_t percentDamage = uint32_t(atoi(args));
    if (percentDamage == 0)
        return true;

    uint32_t health = selected_unit->getHealth();

    uint32_t calculatedDamage = health / 100 * percentDamage;

    selected_unit->takeDamage(session->GetPlayer(), calculatedDamage, 0);

    SystemMessage(session, "Send damage percent: %u (%u hp) for Creature %s", percentDamage, calculatedDamage, selected_unit->GetCreatureProperties()->Name.c_str());

    return true;
}

bool ChatHandler::HandleSetScriptPhaseCommand(const char* args, WorldSession* session)
{
    Creature* selected_unit = GetSelectedCreature(session);
    if (selected_unit == nullptr)
        return false;

    uint32_t scriptPhase = uint32_t(atoi(args));

    if (auto creatureScript = selected_unit->GetScript())
    {
        creatureScript->setScriptPhase(scriptPhase);
        SystemMessage(session, "ScriptPhase %u set for Creature %s", scriptPhase, selected_unit->GetCreatureProperties()->Name.c_str());
    }
    return true;
}

bool ChatHandler::HandleAiChargeCommand(const char* /*args*/, WorldSession* session)
{
    Unit* selected_unit = GetSelectedUnit(session);
    if (selected_unit == nullptr)
        return true;

    selected_unit->getMovementManager()->moveCharge(session->GetPlayer()->GetPositionX(), session->GetPlayer()->GetPositionY(), session->GetPlayer()->GetPositionZ());
    return true;
}

bool ChatHandler::HandleAiKnockbackCommand(const char* /*args*/, WorldSession* session)
{
    Unit* selected_unit = GetSelectedUnit(session);
    if (selected_unit == nullptr)
        return true;

    LocationVector pos = session->GetPlayer()->GetPosition();

    selected_unit->getMovementManager()->moveKnockbackFrom(pos.x, pos.y, 10.0f, 5.f);
    return true;
}

bool ChatHandler::HandleAiJumpCommand(const char* /*args*/, WorldSession* session)
{
    Unit* selected_unit = GetSelectedUnit(session);
    if (selected_unit == nullptr)
        return true;

    LocationVector pos = session->GetPlayer()->GetPosition();

    selected_unit->getMovementManager()->moveJump(pos, 1.0f, 5.0f);
    return true;
}

bool ChatHandler::HandleAiFallingCommand(const char* /*args*/, WorldSession* session)
{
    Unit* selected_unit = GetSelectedUnit(session);
    if (selected_unit == nullptr)
        return true;

    selected_unit->getMovementManager()->moveFall();
    return true;
}

bool ChatHandler::HandleMoveToSpawnCommand(const char* /*args*/, WorldSession* session)
{
    Unit* selected_unit = GetSelectedUnit(session);
    if (selected_unit == nullptr)
        return true;

    selected_unit->getMovementManager()->moveTargetedHome();

    return true;
}

bool ChatHandler::HandlePositionCommand(const char* /*args*/, WorldSession* session)
{
    Creature* selected_unit = GetSelectedCreature(session);
    if (selected_unit == nullptr)
        return true;

    LocationVector spawnPos = selected_unit->GetSpawnPosition();
    LocationVector pos = selected_unit->GetPosition();

    SystemMessage(session, "=== Spawn Position ===");
    SystemMessage(session, "spawnX: %f", spawnPos.x);
    SystemMessage(session, "spawnY: %f", spawnPos.y);
    SystemMessage(session, "spawnZ: %f", spawnPos.z);
    SystemMessage(session, "spawnO: %f", spawnPos.o);
    SystemMessage(session, "=== Packet Position ===");
    SystemMessage(session, "posX: %f", pos.x);
    SystemMessage(session, "posY: %f", pos.y);
    SystemMessage(session, "posZ: %f", pos.z);
    SystemMessage(session, "posO: %f", pos.o);
    return true;
}

bool ChatHandler::HandleSetOrientationCommand(const char* args, WorldSession* session)
{
    Creature* selected_unit = GetSelectedCreature(session);
    if (selected_unit == nullptr)
        return false;

    float orientation = float(atof(args));
    if (orientation == 0.0f)
    {
        SystemMessage(session, "No orientation set, applying yours on npc.");
        orientation = session->GetPlayer()->GetOrientation();
    }

    selected_unit->SetOrientation(orientation);
    SystemMessage(session, "Orientation %f set on npc %s", orientation, selected_unit->GetCreatureProperties()->Name.c_str());
    return true;
}

bool ChatHandler::HandleDebugDumpState(const char* /*args*/, WorldSession* session)
{
    auto state = ServerState::instance();
    SystemMessage(session, "Delta: %u", static_cast<uint32_t>(state->getDelta()));
    return true;
}

bool ChatHandler::HandleDebugMoveInfo(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return true;

    bool creature_in_front = selected_unit->isInFront(m_session->GetPlayer());
    bool in_front_of_creature = m_session->GetPlayer()->isInFront(selected_unit);
    float distance_to_creature = m_session->GetPlayer()->CalcDistance(selected_unit);

    uint32 ai_state = selected_unit->GetAIInterface()->getAiState();
    uint32 ai_type = selected_unit->GetAIInterface()->getAiScriptType();
    uint32 ai_agent = selected_unit->GetAIInterface()->getCurrentAgent();

    uint32 attackerscount = static_cast<uint32>(selected_unit->getThreatManager().getThreatListSize());

    if (selected_unit->isCreature())
        BlueSystemMessage(m_session, "Showing creature moveinfo for %s", static_cast<Creature*>(selected_unit)->GetCreatureProperties()->Name.c_str());
    else
        BlueSystemMessage(m_session, "Showing player moveinfo for %s", static_cast<Player*>(selected_unit)->getName().c_str());

    SystemMessage(m_session, "=== Facing ===");
    SystemMessage(m_session, "Target is in front: %u", creature_in_front);
    SystemMessage(m_session, "In front of the target: %u", in_front_of_creature);
    SystemMessage(m_session, "Current distance to target: %f", distance_to_creature);
    SystemMessage(m_session, "=== States ===");
    SystemMessage(m_session, "Current AI state: %u | AIType: %u | AIAgent: %u", ai_state, ai_type, ai_agent);
    SystemMessage(m_session, "=== Misc ===");
    SystemMessage(m_session, "Attackers count: %u", attackerscount);
    SystemMessage(m_session, "=== UnitMovementFlags ===");
    SystemMessage(m_session, "MovementFlags: %u", selected_unit->getUnitMovementFlags());
    return true;
}

//.debug hover
bool ChatHandler::HandleDebugHover(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    if (selected_unit->hasUnitMovementFlag(MOVEFLAG_HOVER))
    {
        GreenSystemMessage(m_session, "Unset Hover for target.");
        selected_unit->setMoveHover(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Set Hover for target.");
        selected_unit->setMoveHover(true);
    }

    return true;
}

//.debug states
bool ChatHandler::HandleDebugState(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    GreenSystemMessage(m_session, "Display unitStateFlag: %u", selected_unit->getUnitStateFlags());
    
    return true;
}

//.debug swim
bool ChatHandler::HandleDebugSwim(const char* /*args*/, WorldSession* m_session)
{
    Creature* selected_creature = GetSelectedCreature(m_session);
    if (selected_creature == nullptr)
        return false;

    if (selected_creature->hasUnitMovementFlag(MOVEFLAG_SWIMMING))
    {
        GreenSystemMessage(m_session, "Unset Swim for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->setMoveSwim(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Set Swim for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->setMoveSwim(true);
    }

    return true;
}

//.debug fly
bool ChatHandler::HandleDebugFly(const char* /*args*/, WorldSession* m_session)
{
    Creature* selected_creature = GetSelectedCreature(m_session);
    if (selected_creature == nullptr)
        return false;

    if (selected_creature->hasUnitMovementFlag(MOVEFLAG_CAN_FLY))
    {
        GreenSystemMessage(m_session, "Unset Fly for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->setMoveCanFly(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Set Fly for creature %s.", selected_creature->GetCreatureProperties()->Name.c_str());
        selected_creature->setMoveCanFly(true);
    }
    return true;
}

//.debug disablegravity
bool ChatHandler::HandleDebugDisableGravity(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    if (selected_unit->hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
    {
        GreenSystemMessage(m_session, "Enable Gravity for target.");
        selected_unit->setMoveDisableGravity(false);
    }
    else
    {
        GreenSystemMessage(m_session, "Disable Gravity  for target.");
        selected_unit->setMoveDisableGravity(true);
    }

    return true;
}

//.debug waterwalk
bool ChatHandler::HandleDebugWaterWalk(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    if (selected_unit->hasUnitMovementFlag(MOVEFLAG_WATER_WALK))
    {
        GreenSystemMessage(m_session, "Disable WaterWalking for target.");
        selected_unit->setMoveLandWalk();
    }
    else
    {
        GreenSystemMessage(m_session, "Enabled WaterWalking for target.");
        selected_unit->setMoveWaterWalk();
    }

    return true;
}

//.debug featherfall
bool ChatHandler::HandleDebugFeatherFall(const char* /*args*/, WorldSession* m_session)
{
    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    if (selected_unit->hasUnitMovementFlag(MOVEFLAG_FEATHER_FALL))
    {
        GreenSystemMessage(m_session, "Disable FeatherFall for target.");
        selected_unit->setMoveNormalFall();
    }
    else
    {
        GreenSystemMessage(m_session, "Enabled FeatherFall for target.");
        selected_unit->setMoveFeatherFall();
    }

    return true;
}

//.debug speed
bool ChatHandler::HandleDebugSpeed(const char* args, WorldSession* m_session)
{
    float speed = float(atof(args));
    if (speed == 0.0f || speed > 255.0f || speed < 0.1f)
    {
        RedSystemMessage(m_session, "Invalid speed set. Value range 0.1f ... 255.0f Use .debug speed <speed>");
        return true;
    }

    Unit* selected_unit = GetSelectedUnit(m_session);
    if (selected_unit == nullptr)
        return false;

    BlueSystemMessage(m_session, "Setting speeds of selected unit to %3.2f.", speed);

    selected_unit->setSpeedRate(TYPE_WALK, speed, true);
    selected_unit->setSpeedRate(TYPE_RUN, (speed + speed / 2), true);
    selected_unit->setSpeedRate(TYPE_SWIM, speed, true);
    selected_unit->setSpeedRate(TYPE_RUN_BACK, speed / 2, true);
    selected_unit->setSpeedRate(TYPE_FLY, speed * 2, true);

    return true;
}

//.debug pvpcredit
bool ChatHandler::HandleDebugPVPCreditCommand(const char* args, WorldSession* m_session)
{
    uint32 rank;
    uint32 points;
    if (sscanf(args, "%u %u", &rank, &points) != 2)
    {
        RedSystemMessage(m_session, "Command must be in format <rank> <points>.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    points *= 10;

    GreenSystemMessage(m_session, "Building packet with Rank %u, Points %u, for Player %s.", rank, points, player_target->getName().c_str());

    m_session->GetPlayer()->sendPvpCredit(points, player_target->getGuid(), rank);

    return true;
}

//.debug setunitbyte
bool ChatHandler::HandleDebugSetUnitByteCommand(const char* args, WorldSession* m_session)
{
    uint32_t bytes;
    uint32_t offset;
    uint32_t value;
    if (sscanf(args, "%u %u %u", &bytes, &offset, &value) != 3)
    {
        RedSystemMessage(m_session, "Command must be in format <bytes> <offset> <value>.");
        return true;
    }

    auto unit_target = GetSelectedUnit(m_session, true);
    if (unit_target == nullptr)
        return true;

    if (offset > 3)
        return true;

    switch (bytes)
    {
        case 0:
        {
            unit_target->setBytes0ForOffset(offset, static_cast<uint8_t>(value));
        } break;
        case 1:
        {
            unit_target->setBytes1ForOffset(offset, static_cast<uint8_t>(value));
        } break;
        case 2:
        {
            unit_target->setBytes2ForOffset(offset, static_cast<uint8_t>(value));
        } break;
        default:
        {
            RedSystemMessage(m_session, "Bytes %u are not existent. Choose from 0, 1 or 2", bytes);
            return true;
        }
    }

    GreenSystemMessage(m_session, "Unit Bytes %u Offset %u set to Value %u", bytes, offset, value);

    return true;
}

//.debug setplayerflag
bool ChatHandler::HandleDebugSetPlayerFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32_t flags;
    if (sscanf(args, "%u", &flags) != 1)
    {
        RedSystemMessage(m_session, "Command must contain at least 1 flag.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true);
    if (player_target == nullptr)
        return true;

    const auto current_flags = player_target->getPlayerFlags();

    player_target->addPlayerFlags(flags);

    GreenSystemMessage(m_session, "Player flag %u added (before %u)", flags, current_flags);

    return true;
}

//.debug getplayerflag
bool ChatHandler::HandleDebugGetPlayerFlagsCommand(const char* /*args*/, WorldSession* m_session)
{
    const auto player_target = GetSelectedPlayer(m_session, true);
    if (player_target == nullptr)
        return true;

    const auto current_flags = player_target->getPlayerFlags();

    GreenSystemMessage(m_session, "Current player flags: %u", current_flags);

    return true;
}

//.playmovie
bool ChatHandler::HandlePlayMovie(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32 movie = atol(args);

    selected_player->sendMovie(movie);

    if (selected_player != m_session->GetPlayer())
        GreenSystemMessage(selected_player->GetSession(), "Movie started for player %s", selected_player->getName().c_str());

    return true;
}

//.sendfail
bool ChatHandler::HandleSendCastFailed(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32 fail = atol(args);
    if (fail > SPELL_FAILED_UNKNOWN)
    {
        RedSystemMessage(m_session, "Argument %u is out of range!", fail);
        return false;
    }
    selected_player->sendCastFailedPacket(1, static_cast<uint8>(fail), 0, 0);

    return true;
}

bool ChatHandler::HandleDebugSendCreatureMove(const char* /*args*/, WorldSession * m_session)
{
    const auto target = GetSelectedUnit(m_session);
    if (!target)
    {
        return true;
    }

    return true;
}

//.debug setweather
bool ChatHandler::HandleDebugSetWeatherCommand(const char* args, WorldSession* m_session)
{
    uint32_t type;
    float density;

    if (sscanf(args, "%u %f", &type, &density) != 2)
    {
        RedSystemMessage(m_session, "Command must be in format <type> <density>.");
        return true;
    }

    if (density < 0.30f)
        density = 0.10f;
    else if (density > 2.0f)
        density = 2.0f;

    sWeatherMgr.sendWeatherForPlayer(type, density, m_session->GetPlayer());

    GreenSystemMessage(m_session, "Weather changed to %u with density %f", type, density);

    return true;
}
