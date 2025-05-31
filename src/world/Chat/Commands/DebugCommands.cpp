/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VMapFactory.h"
#include "Chat/ChatHandler.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/WeatherMgr.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/ThreatHandler.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/ServerState.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellFailure.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Util.hpp"

bool ChatHandler::HandleMoveHardcodedScriptsToDBCommand(const char* args, WorldSession* session)
{
    uint32_t map = uint32_t(atoi(args));
    if (map == 0)
        return true;

    std::vector<uint32_t> creatureEntries;

    auto creature_spawn_result = WorldDatabase.Query("SELECT entry FROM creature_spawns WHERE map = %u GROUP BY(entry)", map);
    if (creature_spawn_result)
    {
        {
            do
            {
                Field* fields = creature_spawn_result->Fetch();
                creatureEntries.push_back(fields[0].asUint32());

            } while (creature_spawn_result->NextRow());
        }
    }

    //prepare new table for dump
    char my_table[1400];
    sprintf(my_table, "CREATE TABLE `creature_ai_scripts_%s` (`min_build` int NOT nullptr DEFAULT '8606',`max_build` int NOT nullptr DEFAULT '12340',`entry` int unsigned NOT nullptr,\
            `difficulty` tinyint unsigned NOT nullptr DEFAULT '0',`phase` tinyint unsigned NOT nullptr DEFAULT '0',`event` tinyint unsigned NOT nullptr DEFAULT '0',`action` tinyint unsigned NOT nullptr DEFAULT '0',\
            `maxCount` tinyint unsigned NOT nullptr DEFAULT '0',`chance` float unsigned NOT nullptr DEFAULT '1',`spell` int unsigned NOT nullptr DEFAULT '0',`spell_type` int NOT nullptr DEFAULT '0',`triggered` tinyint(1) NOT nullptr DEFAULT '0',\
            `target` tinyint NOT nullptr DEFAULT '0',`cooldownMin` int NOT nullptr DEFAULT '0',`cooldownMax` int unsigned NOT nullptr DEFAULT '0',`minHealth` float NOT nullptr DEFAULT '0',\
            `maxHealth` float NOT nullptr DEFAULT '100',`textId` int unsigned NOT nullptr DEFAULT '0',`misc1` int NOT nullptr DEFAULT '0',`comments` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,\
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
        uint8_t gender = creature_properties->generateRandomDisplayIdAndReturnGender(&creature_spawn->displayid);
        creature_spawn->entry = entry;
        creature_spawn->id = sObjectMgr.generateCreatureSpawnId();
        creature_spawn->movetype = 0;
        creature_spawn->x = session->GetPlayer()->GetPositionX();
        creature_spawn->y = session->GetPlayer()->GetPositionY();
        creature_spawn->z = session->GetPlayer()->GetPositionZ();
        creature_spawn->o = session->GetPlayer()->GetOrientation();
        creature_spawn->emote_state = 0;
        creature_spawn->flags = creature_properties->NPCFLags;
        creature_spawn->pvp_flagged = 0;
        creature_spawn->factionid = creature_properties->Faction;
        creature_spawn->bytes0 = creature_spawn->setbyte(0, 2, gender);
        creature_spawn->stand_state = 0;
        creature_spawn->death_state = 0;
        creature_spawn->channel_target_creature = creature_spawn->channel_target_go = creature_spawn->channel_spell = 0;
        creature_spawn->MountedDisplayID = 0;
        creature_spawn->sheath_state = 0;

        creature_spawn->Item1SlotEntry = creature_properties->itemslot_1;
        creature_spawn->Item2SlotEntry = creature_properties->itemslot_2;
        creature_spawn->Item3SlotEntry = creature_properties->itemslot_3;

        creature_spawn->CanFly = 0;
        creature_spawn->phase = session->GetPlayer()->GetPhase();

        if (auto creature = session->GetPlayer()->getWorldMap()->createCreature(entry))
        {
            creature->Load(creature_spawn, 0, nullptr);
            creature->m_loadedFromDB = true;
            creature->PushToWorld(session->GetPlayer()->getWorldMap());

            // Add to map
            uint32_t x = session->GetPlayer()->getWorldMap()->getPosX(session->GetPlayer()->GetPositionX());
            uint32_t y = session->GetPlayer()->getWorldMap()->getPosY(session->GetPlayer()->GetPositionY());
            session->GetPlayer()->getWorldMap()->getBaseMap()->getSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(creature_spawn);
            MapCell* map_cell = session->GetPlayer()->getWorldMap()->getCell(x, y);
            if (map_cell != nullptr)
                map_cell->setLoaded();

            for (const auto& aiSpells : creature->getAIInterface()->mCreatureAISpells)
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
                sprintf(my_insert1, "INSERT INTO creature_ai_scripts_%s VALUES (8606,12340,%u,4,0,5,1,0,%f,%u,%u,0,%u,%u,%u,0,100,0,0,'%s')", args, entry, chance, spell, spelltype, target, cooldown, cooldown, comment.c_str());

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

    SystemMessage(session, "Outdoor: %u", selected_unit->isOutdoors());
    SystemMessage(session, "posZ_floor: %f", selected_unit->getFloorZ());
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

    uint32_t ai_agent = selected_unit->getAIInterface()->getCurrentAgent();

    uint32_t attackerscount = static_cast<uint32_t>(selected_unit->getThreatManager().getThreatListSize());

    if (selected_unit->isCreature())
        BlueSystemMessage(m_session, "Showing creature moveinfo for %s", static_cast<Creature*>(selected_unit)->GetCreatureProperties()->Name.c_str());
    else
        BlueSystemMessage(m_session, "Showing player moveinfo for %s", static_cast<Player*>(selected_unit)->getName().c_str());

    SystemMessage(m_session, "=== Facing ===");
    SystemMessage(m_session, "Target is in front: %u", creature_in_front);
    SystemMessage(m_session, "In front of the target: %u", in_front_of_creature);
    SystemMessage(m_session, "Current distance to target: %f", distance_to_creature);
    SystemMessage(m_session, "=== States ===");
    SystemMessage(m_session, "AIAgent: %u", ai_agent);
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
    uint32_t rank;
    uint32_t points;
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

    uint32_t movie = std::stoul(args);

    selected_player->sendMovie(movie);

    if (selected_player != m_session->GetPlayer())
        GreenSystemMessage(selected_player->getSession(), "Movie started for player %s", selected_player->getName().c_str());

    return true;
}

//.sendfail
bool ChatHandler::HandleSendCastFailed(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32_t fail = std::stoul(args);
    if (fail > SPELL_FAILED_UNKNOWN)
    {
        RedSystemMessage(m_session, "Argument %u is out of range!", fail);
        return false;
    }
    selected_player->sendCastFailedPacket(1, static_cast<uint8_t>(fail), 0, 0);

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

//.debug dumpmovement
bool ChatHandler::HandleDebugDumpMovementCommand(const char* /*args*/, WorldSession* session)
{
    try
    {
        auto me = session->GetPlayerOrThrow();

        SystemMessage(session, "Position: [%f, %f, %f, %f]", me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
        SystemMessage(session, "On transport: %s", me->obj_movement_info.transport_guid != 0 ? "yes" : "no");
        SystemMessage(session, "Transport GUID: %lu", uint64_t(me->obj_movement_info.transport_guid));
        SystemMessage(session, "Transport relative position: [%f, %f, %f, %f]", me->obj_movement_info.transport_position.x,
            me->obj_movement_info.transport_position.y, me->obj_movement_info.transport_position.z, me->obj_movement_info.transport_position.o);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

//.debug infront
bool ChatHandler::HandleDebugInFrontCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char buf[256];
    snprintf((char*)buf, 256, "%d", m_session->GetPlayer()->isInFront(obj));

    SystemMessage(m_session, buf);

    return true;
}

//.debug showreact
bool ChatHandler::HandleShowReactionCommand(const char* args, WorldSession* m_session)
{
    Object* obj = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (wowGuid.getRawGuid() != 0)
    {
        obj = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (!obj)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }


    char* pReaction = strtok((char*)args, " ");
    if (!pReaction)
        return false;

    uint32_t Reaction = atoi(pReaction);

    obj->SendAIReaction(Reaction);

    std::stringstream sstext;
    sstext << "Sent Reaction of " << Reaction << " to " << obj->GetUIdFromGUID() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

//.debug dist
bool ChatHandler::HandleDistanceCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    float dist = m_session->GetPlayer()->CalcDistance(obj);
    std::stringstream sstext;
    sstext << "Distance is: " << dist << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

//.debug aimove
bool ChatHandler::HandleAIMoveCommand(const char* args, WorldSession* m_session)
{
    Creature* creature = nullptr;
    auto player = m_session->GetPlayer();
    WoWGuid wowGuid;
    wowGuid.Init(player->getTargetGuid());
    if (wowGuid.getRawGuid() != 0)
    {
        creature = player->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (creature == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32_t Move = 1;
    uint32_t Run = 0;
    uint32_t Time = 0;
    uint32_t Meth = 0;

    char* pMove = strtok((char*)args, " ");
    if (pMove)
        Move = atoi(pMove);

    char* pRun = strtok(nullptr, " ");
    if (pRun)
        Run = atoi(pRun);

    char* pTime = strtok(nullptr, " ");
    if (pTime)
        Time = atoi(pTime);

    char* pMeth = strtok(nullptr, " ");
    if (pMeth)
        Meth = atoi(pMeth);

    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    //float o = m_session->GetPlayer()->GetOrientation();

    MovementMgr::MoveSplineInit init(creature);

    float distance = creature->CalcDistance(x, y, z);
    if (Move == 1)
    {
        if (Meth == 1)
        {
            float q = distance - 0.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 2)
        {
            float q = distance - 1;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 3)
        {
            float q = distance - 2;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 4)
        {
            float q = distance - 2.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 5)
        {
            float q = distance - 3;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 6)
        {
            float q = distance - 3.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else
        {
            float q = distance - 4;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }

        init.MoveTo(x, y, z);
        if (Run)
            init.SetWalk(false);
        else
            init.SetWalk(true);

        creature->getMovementManager()->launchMoveSpline(std::move(init));
    }
    else
    {
        init.MoveTo(x, y, z);
        if (Run)
            init.SetWalk(false);
        else
            init.SetWalk(true);

        creature->getMovementManager()->launchMoveSpline(std::move(init));
    }

    return true;
}

//.debug face
bool ChatHandler::HandleFaceCommand(const char* args, WorldSession* m_session)
{
    Object* obj = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (wowGuid.getRawGuid() != 0)
    {
        obj = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (obj == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32_t Orentation = 0;
    char* pOrentation = strtok((char*)args, " ");
    if (pOrentation)
        Orentation = atoi(pOrentation);

    // Convert to Blizzards Format
    float theOrientation = Orentation / (180.0f / M_PI_FLOAT);

    obj->SetPosition(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), theOrientation, false);

    sLogger.debug("facing sent");
    return true;

}

//.debug landwalk
bool ChatHandler::HandleDebugLandWalk(const char* /*args*/, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    if (chr == nullptr)
        return true;

    char buf[256];

    chr->setMoveLandWalk();
    snprintf((char*)buf, 256, "Land Walk Test Ran.");
    SystemMessage(m_session, buf);

    return true;
}

//.debug aggrorange
bool ChatHandler::HandleAggroRangeCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    float aggroRange = unit->getAIInterface()->calcAggroRange(m_session->GetPlayer());

    GreenSystemMessage(m_session, "Aggrorange is %f", aggroRange);

    return true;
}

//.debug knockback
bool ChatHandler::HandleKnockBackCommand(const char* args, WorldSession* m_session)
{
    float f = args ? (float)atof(args) : 0.0f;
    if (f == 0.0f)
        f = 5.0f;

    float dx = sinf(m_session->GetPlayer()->GetOrientation());
    float dy = cosf(m_session->GetPlayer()->GetOrientation());

    float z = f * 0.66f;

    m_session->SendPacket(AscEmu::Packets::SmsgMoveKnockBack(m_session->GetPlayer()->GetNewGUID(), Util::getMSTime(), dy, dx, f, z).serialise().get());
    return true;
}

//.debug fade
bool ChatHandler::HandleFadeCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->modThreatModifyer(atoi(v));

    std::stringstream sstext;
    sstext << "threat is now reduced by: " << target->getThreatModifyer() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

//.debug threatMod
bool ChatHandler::HandleThreatModCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->modGeneratedThreatModifyer(0, atoi(v));

    std::stringstream sstext;
    sstext << "new threat caused is now reduced by: " << target->getGeneratedThreatModifyer(0) << "%" << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

//.debug movefall
bool ChatHandler::HandleMoveFallCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        return true;

    bool needsFalling = (target->IsFlying() || target->isHovering()) && !target->isUnderWater();
    target->setMoveHover(false);
    target->setMoveDisableGravity(false);

    if (needsFalling)
        target->getMovementManager()->moveFall();

    return true;
}

//.debug threatList
bool ChatHandler::HandleThreatListCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* target = nullptr;
    target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    std::stringstream sstext;
    sstext << "threatlist of creature: " << wowGuid.getGuidLowPart() << " " << wowGuid.getGuidHighPart() << '\n';

    for (ThreatReference* ref : target->getThreatManager().getModifiableThreatList())
    {
        sstext << "guid: " << ref->getOwner()->getGuid() << " | threat: " << ref->getThreat() << "\n";
    }

    SendMultilineMessage(m_session, sstext.str().c_str());
    return true;
}

//.debug dumpcoords
bool ChatHandler::HandleDebugDumpCoordsCommmand(const char* /*args*/, WorldSession* m_session)
{
    Player* p = m_session->GetPlayer();
    //char buffer[200] = {0};
    FILE* f = fopen("C:\\script_dump.txt", "a");
    if (!f)
        return false;

    fprintf(f, "mob.CreateWaypoint(%f, %f, %f, %f, 0, 0, 0);\n", p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetOrientation());
    fclose(f);

    return true;
}

//.debug spawnwar
bool ChatHandler::HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session)
{
    uint32_t count, npcid;
    uint32_t health = 0;

    // takes 2 or 3 arguments: npcid, count, (health)
    if (sscanf(args, "%u %u %u", &npcid, &count, &health) != 3)
    {
        if (sscanf(args, "%u %u", &count, &npcid) != 2)
            return false;
    }

    if (!count || !npcid)
        return false;

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(npcid);
    if (cp == nullptr)
        return false;

    WorldMap* m = m_session->GetPlayer()->getWorldMap();

    // if we have selected unit, use its position
    Unit* unit = m->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (unit == nullptr)
        unit = m_session->GetPlayer(); // otherwise ours

    float bx = unit->GetPositionX();
    float by = unit->GetPositionY();
    float x, y, z;

    float angle = 1;
    float r = 3; // starting radius
    for (; count > 0; --count)
    {
        // spawn in spiral
        x = r * sinf(angle);
        y = r * cosf(angle);
        z = unit->getMapHeight(LocationVector(bx + x, by + y, unit->GetPositionZ() + 2));

        Creature* c = m->createCreature(npcid);
        c->Load(cp, bx + x, by + y, z, 0.0f);
        if (health != 0)
        {
            c->setMaxHealth(health);
            c->setHealth(health);
        }
        c->setFactionTemplate((count % 2) ? 1 : 2);
        c->setServersideFaction();
        c->PushToWorld(m);

        r += 0.5;
        angle += 8 / r;
    }
    return true;
}

//.debug updateworldstate
bool ChatHandler::HandleUpdateWorldStateCommand(const char *args, WorldSession* session)
{
    if (*args == '\0')
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    uint32_t field = 0;
    uint32_t state = 0;

    std::stringstream ss(args);

    ss >> field;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    ss >> state;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    session->GetPlayer()->sendWorldStateUpdate(field, state);

    return true;
}

//.debug initworldstates
bool ChatHandler::HandleInitWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32_t zone = p->getZoneId();
    if (zone == 0)
        zone = p->getAreaId();

    BlueSystemMessage(session, "Sending initial worldstates for zone %u", zone);

    p->sendInitialWorldstates();

    return true;
}

//.debug clearworldstates
bool ChatHandler::HandleClearWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32_t zone = p->getZoneId();
    if (zone == 0)
        zone = p->getAreaId();

    BlueSystemMessage(session, "Clearing worldstates for zone %u", zone);

    WorldPacket data(SMSG_INIT_WORLD_STATES, 16);

    data << uint32_t(p->GetMapId());
    data << uint32_t(p->getZoneId());
    data << uint32_t(p->getAreaId());
    data << uint16_t(0);

    p->sendPacket(&data);

    return true;
}

//.debug auraremove
bool ChatHandler::HandleAuraUpdateRemove(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    char* pArgs = strtok((char*)args, " ");
    if (!pArgs)
        return false;
    uint8_t VisualSlot = (uint8_t)atoi(pArgs);
    Player* Pl = m_session->GetPlayer();
    Aura* AuraPtr = Pl->getAuraWithId(Pl->getVisualAuraList().at(VisualSlot));
    if (!AuraPtr)
    {
        SystemMessage(m_session, "No auraid found in slot %u", VisualSlot);
        return true;
    }
    SystemMessage(m_session, "SMSG_AURA_UPDATE (remove): VisualSlot %u - SpellID 0", VisualSlot);
    AuraPtr->removeAura();
    return true;
}

//.debug auraupdate
bool ChatHandler::HandleAuraUpdateAdd(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    uint32_t SpellID = 0;
    int Flags = 0;
    int StackCount = 0;
    if (sscanf(args, "%u 0x%X %i", &SpellID, &Flags, &StackCount) != 3 && sscanf(args, "%u %u %i", &SpellID, &Flags, &StackCount) != 3)
        return false;

    Player* Pl = m_session->GetPlayer();
    if (Aura* AuraPtr = Pl->getAuraWithId(SpellID))
    {
        uint8_t VisualSlot = AuraPtr->m_visualSlot;
        Pl->sendAuraUpdate(AuraPtr, false);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (update): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
    }
    else
    {
        SpellInfo const* Sp = sSpellMgr.getSpellInfo(SpellID);
        if (!Sp)
        {
            SystemMessage(m_session, "SpellID %u is invalid.", SpellID);
            return true;
        }
        Spell* SpellPtr = sSpellMgr.newSpell(Pl, Sp, false, nullptr);
        auto auraHolder = sSpellMgr.newAura(Sp, SpellPtr->getDuration(), Pl, Pl);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (add): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", auraHolder->m_visualSlot, SpellID, Flags, Flags, StackCount);
        Pl->addAura(std::move(auraHolder));       // Serves purpose to just add the aura to our auraslots

        delete SpellPtr;
    }
    return true;
}

float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

//.debug calcdist
bool ChatHandler::HandleSimpleDistanceCommand(const char* args, WorldSession* m_session)
{
    float toX, toY, toZ;
    if (sscanf(args, "%f %f %f", &toX, &toY, &toZ) != 3)
        return false;

    if (toX >= Map::Terrain::_maxX || toX <= Map::Terrain::_minX || toY <= Map::Terrain::_minY || toY >= Map::Terrain::_maxY)
        return false;

    float distance = CalculateDistance(
        m_session->GetPlayer()->GetPositionX(),
        m_session->GetPlayer()->GetPositionY(),
        m_session->GetPlayer()->GetPositionZ(),
        toX, toY, toZ);

    m_session->SystemMessage("Your distance to location (%f, %f, %f) is %0.2f meters.", toX, toY, toZ, distance);

    return true;
}

//.debug rangecheck
bool ChatHandler::HandleRangeCheckCommand(const char* /*args*/, WorldSession* m_session)
{
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    m_session->SystemMessage("=== RANGE CHECK ===");
    if (guid == 0)
    {
        m_session->SystemMessage("No selection.");
        return true;
    }

    Unit* unit = m_session->GetPlayer()->getWorldMap()->getUnit(guid);
    if (!unit)
    {
        m_session->SystemMessage("Invalid selection.");
        return true;
    }
    float DistSq = unit->getDistanceSq(m_session->GetPlayer());
    m_session->SystemMessage("getDistanceSq  :   %u", Util::float2int32(DistSq));
    LocationVector locvec(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    float DistReal = unit->CalcDistance(locvec);
    m_session->SystemMessage("CalcDistance   :   %u", Util::float2int32(DistReal));
    float Dist2DSq = unit->GetDistance2dSq(m_session->GetPlayer());
    m_session->SystemMessage("GetDistance2dSq:   %u", Util::float2int32(Dist2DSq));
    return true;
}

//.debug testindoor
bool ChatHandler::HandleCollisionTestIndoor(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        const LocationVector & loc = plr->GetPosition();
        bool res = !MapManagement::AreaManagement::AreaStorage::IsOutdoor(plr->GetMapId(), loc.x, loc.y, loc.z + 2.0f);
        SystemMessage(m_session, "Result was: %s.", res ? "indoors" : "outside");
        return true;
    }

    SystemMessage(m_session, "Collision is not enabled.");
    return true;
}

//.debug testlos
bool ChatHandler::HandleCollisionTestLOS(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Object* pObj = nullptr;
        Creature* pCreature = GetSelectedCreature(m_session, false);
        Player* pPlayer = GetSelectedPlayer(m_session, true, true);
        if (pCreature)
            pObj = pCreature;
        else if (pPlayer)
            pObj = pPlayer;

        if (pObj == nullptr)
        {
            SystemMessage(m_session, "Invalid target.");
            return true;
        }

        bool res = pObj->IsWithinLOSInMap(m_session->GetPlayer());

        SystemMessage(m_session, "Result was: %s.", res ? "in LOS" : "not in LOS");
        return true;
    }

    SystemMessage(m_session, "Collision is not enabled.");
    return true;
}

//.debug getheight
bool ChatHandler::HandleCollisionGetHeight(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        float radius = 5.0f;

        float posX = plr->GetPositionX();
        float posY = plr->GetPositionY();
        float posZ = plr->GetPositionZ();
        float ori = plr->GetOrientation();

        LocationVector src(posX, posY, posZ);

        LocationVector dest(posX + (radius * (cosf(ori))), posY + (radius * (sinf(ori))), posZ);

        const auto mgr = VMAP::VMapFactory::createOrGetVMapManager();
        float z = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 2.0f, 10000.0f);
        float z2 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 5.0f, 10000.0f);
        float z3 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ, 10000.0f);
        float z4 = plr->getWorldMap()->getGridHeight(plr->GetPositionX(), plr->GetPositionY());
        bool fp = mgr->getObjectHitPos(plr->GetMapId(), src.x, src.y, src.z, dest.x, dest.y, dest.z, dest.x, dest.y, dest.z, -1.5f);

        SystemMessage(m_session, "Results were: %f(offset2.0f) | %f(offset5.0f) | %f(org) | landheight:%f | target radius5 FP:%d", z, z2, z3, z4, fp);
        return true;
    }

    SystemMessage(m_session, "Collision is not enabled.");
    return true;
}

//.debug deathstate
bool ChatHandler::HandleGetDeathState(const char* /*args*/, WorldSession* m_session)
{
    Player* SelectedPlayer = GetSelectedPlayer(m_session, true, true);
    if (!SelectedPlayer)
        return true;

    SystemMessage(m_session, "Death State: %d", SelectedPlayer->getDeathState());
    return true;
}

struct spell_thingo
{
    uint32_t type;
    uint32_t target;
};

std::list<SpellInfo const*> aiagent_spells;
std::map<uint32_t, spell_thingo> aiagent_extra;

SpellCastTargets SetTargets(SpellInfo const* /*sp*/, uint32_t /*type*/, uint32_t targettype, Unit* dst, Creature* src)
{
    SpellCastTargets targets;
    targets.setUnitTarget(0);
    targets.setItemTarget(0);
    targets.setSource(LocationVector(0, 0, 0));
    targets.setDestination(LocationVector(0, 0, 0));

    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(dst->getGuid());
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.setTargetMask(TARGET_FLAG_SOURCE_LOCATION);
        targets.setSource(src->GetPosition());
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
        targets.setDestination(dst->GetPosition());
    }

    return targets;
};

//.debug castspell
bool ChatHandler::HandleCastSpellCommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32_t spellid = std::stoul(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellMgr.newSpell(caster, spellentry, false, nullptr);

    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets(target->getGuid());
    sp->prepare(&targets);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellid, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

//.debug castspellne
bool ChatHandler::HandleCastSpellNECommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32_t spellId = std::stoul(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellId);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellId);

    WorldPacket data;

    data.Initialize(SMSG_SPELL_START);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8_t(0);
    data << uint16_t(0);
    data << uint32_t(0);
    data << uint16_t(2);
    data << target->getGuid();
    m_session->SendPacket(&data);

    data.Initialize(SMSG_SPELL_GO);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8_t(0) << uint8_t(1) << uint8_t(1);
    data << target->getGuid();
    data << uint8_t(0);
    data << uint16_t(2);
    data << target->getGuid();
    m_session->SendPacket(&data);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellId, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellId, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

//.debug castself
bool ChatHandler::HandleCastSelfCommand(const char* args, WorldSession* m_session)
{
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32_t spellid = std::stoul(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellMgr.newSpell(target, spellentry, false, nullptr);

    BlueSystemMessage(m_session, "Target is casting spell %d on himself.", spellid);
    SpellCastTargets targets(target->getGuid());
    sp->prepare(&targets);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (m_session->GetPlayer() != target)
                sGMLog.writefromsession(m_session, "used castself with spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "used castself with spell %d on CREATURE %u [%s], sqlid %u", spellid, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}
