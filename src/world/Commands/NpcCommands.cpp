/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

bool ChatHandler::HandleNpcComeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
    {
        RedSystemMessage(m_session, "You must select a Creature.");
        return false;
    }

    auto player = m_session->GetPlayer();
    creature_target->GetAIInterface()->MoveTo(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
    sGMLog.writefromsession(m_session, "used .npc come on %s spawn ID: %u", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

// Zyres: not only for npc!
bool ChatHandler::HandlePossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);
    if (unit_target != nullptr)
    {
        if (unit_target->IsPet() || unit_target->GetCreatedByGUID() != 0)
        {
            RedSystemMessage(m_session, "You can not possess a pet!");
            return false;
        }
        else if (unit_target->IsPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            if (player == nullptr)
                return false;

            BlueSystemMessage(m_session, "Player %s selected.", player->GetName());
            sGMLog.writefromsession(m_session, "used possess command on PLAYER %s", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            if (creature == nullptr)
                return false;

            BlueSystemMessage(m_session, "Creature %s selected.", creature->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "used possess command on Creature spawn_id %u", creature->GetCreatureInfo()->Name, creature->GetSQL_id());
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->Possess(unit_target);

    return true;
}

bool ChatHandler::HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);

    if (unit_target != nullptr)
    {
        if (unit_target->IsPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            if (player == nullptr)
                return false;

            BlueSystemMessage(m_session, "Player %s is no longer possessed by you.", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            if (creature == nullptr)
                return false;

            BlueSystemMessage(m_session, "Creature %s is no longer possessed by you.", creature->GetCreatureInfo()->Name);
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->UnPossess();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .npc set commands
bool ChatHandler::HandleNpcSetEquipCommand(const char* args, WorldSession* m_session)
{
    uint32 equipment_slot;
    uint32 item_id;

    if (sscanf(args, "%u %u", (unsigned int*)&equipment_slot, (unsigned int*)&item_id) != 2)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add equipment <slot> <item_id>.");
        RedSystemMessage(m_session, "Slots: (0)melee, (1)offhand, (2)ranged");
        return true;
    }

    Creature* creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto item_entry = sItemStore.LookupEntry(item_id);
    if (item_entry == nullptr)
    {
        RedSystemMessage(m_session, "Item ID: %u is not a valid item!", item_id);
        return true;
    }

    switch (equipment_slot)
    {
        case MELEE:
        {
            GreenSystemMessage(m_session, "Melee slot successfull changed from %u to %u for Creature %s", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "changed melee slot from %u to %u for creature spawn %u", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->spawnid);
            break;
        }
        case OFFHAND:
        {
            GreenSystemMessage(m_session, "Offhand slot successfull changed from %u to %u for Creature %s", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "changed offhand slot from %u to %u for creature spawn %u", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->spawnid);
            break;
        }
        case RANGED:
        {
            GreenSystemMessage(m_session, "Ranged slot successfull changed from %u to %u for Creature %s", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->GetCreatureInfo()->Name);
            sGMLog.writefromsession(m_session, "changed ranged slot from %u to %u for creature spawn %u", creature_target->GetEquippedItem(equipment_slot), item_id, creature_target->spawnid);
            break;
        }
        default:
        {
            RedSystemMessage(m_session, "Slot: %u is not a valid slot! Use: (0)melee, (1)offhand, (2)ranged.", equipment_slot);
            return true;
        }
    }

    creature_target->SetEquippedItem(equipment_slot, item_id);
    creature_target->SaveToDB();
    return true;
}

bool ChatHandler::HandleNpcSetEmoteCommand(const char* args, WorldSession* m_session)
{
    uint32 emote;
    uint32 save = 0;

    if (sscanf(args, "%u %u", (unsigned int*)&emote, (unsigned int*)&save) != 2)
    {
        if (sscanf(args, "%u", (unsigned int*)&emote) != 1)
        {
            RedSystemMessage(m_session, "Command must be at least in format: .npc set emote <emote>.");
            RedSystemMessage(m_session, "Use the following format to save the emote: .npc set emote <emote> 1.");
            return true;
        }
    }

    auto creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32 old_emote = creature_target->GetEmoteState();
    creature_target->SetEmoteState(emote);
    if (save == 0)
    {
        GreenSystemMessage(m_session, "Emote temporarily set from %u to %u for spawn ID: %u.", old_emote, emote, creature_target->spawnid);
    }
    else if (save == 1)
    {
        WorldDatabase.Execute("UPDATE creature_spawns SET emote_state = '%lu' WHERE id = %lu", emote, creature_target->spawnid);
        GreenSystemMessage(m_session, "Emote permanent set from %u to %u for spawn ID: %u.", old_emote, emote, creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc emote of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_emote, emote);
    }

    return true;
}

bool ChatHandler::HandleNpcSetFormationMasterCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    m_session->GetPlayer()->linkTarget = creature_target;
    BlueSystemMessage(m_session, "Formation Master set to %s spawn ID: %u.", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

bool ChatHandler::HandleNpcSetFormationSlaveCommand(const char* args, WorldSession* m_session)
{
    // set formation "slave" with distance and angle
    float angle;
    float distance;
    uint32 save = 0;

    if (sscanf(args, "%f %f %u", &angle, &distance, &save) < 2)
    {
        RedSystemMessage(m_session, "You must specify angle and distance.");
        RedSystemMessage(m_session, ".npc set formationslave <angle> <distance>");
        return true;
    }

    if (m_session->GetPlayer()->linkTarget == nullptr)
    {
        RedSystemMessage(m_session, "Master not set! Use .npc set formationmaster first.");
        return true;
    }

    if (m_session->GetPlayer()->linkTarget->IsPet())
    {
        RedSystemMessage(m_session, "A pet can not be a master of a formation!");
        return true;
    }

    auto creature_slave = getSelectedCreature(m_session, true);
    if (creature_slave == nullptr)
        return true;

    creature_slave->GetAIInterface()->m_formationFollowDistance = distance;
    creature_slave->GetAIInterface()->m_formationFollowAngle = angle;
    creature_slave->GetAIInterface()->m_formationLinkTarget = m_session->GetPlayer()->linkTarget->GetGUID();
    creature_slave->GetAIInterface()->m_formationLinkSqlId = m_session->GetPlayer()->linkTarget->GetSQL_id();
    creature_slave->GetAIInterface()->SetUnitToFollowAngle(angle);

    BlueSystemMessage(m_session, "%s linked to %s with a distance of %f at %f radians.", creature_slave->GetCreatureInfo()->Name,
        m_session->GetPlayer()->linkTarget->GetCreatureInfo()->Name, distance, angle);

    if (save == 1)
    {
        WorldDatabase.Execute("REPLACE INTO creature_formations VALUES(%u, %u, '%f', '%f')",
            creature_slave->GetSQL_id(), creature_slave->GetAIInterface()->m_formationLinkSqlId, angle, distance);
        sGMLog.writefromsession(m_session, "changed npc formation of creature_spawn ID: %u [%s]", creature_slave->spawnid, creature_slave->GetCreatureInfo()->Name);
    }

    return true;
}

bool ChatHandler::HandleNpcSetFormationClearCommand(const char* args, WorldSession* m_session)
{
    uint32 save = atol(args);

    auto creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->GetAIInterface()->m_formationLinkSqlId = 0;
    creature_target->GetAIInterface()->m_formationLinkTarget = 0;
    creature_target->GetAIInterface()->m_formationFollowAngle = 0.0f;
    creature_target->GetAIInterface()->m_formationFollowDistance = 0.0f;
    creature_target->GetAIInterface()->ResetUnitToFollow();

    if (save == 1)
    {
        WorldDatabase.Execute("DELETE FROM creature_formations WHERE spawn_id=%u", creature_target->GetSQL_id());
        sGMLog.writefromsession(m_session, "removed npc formation for creature_spawn ID: %u [%s]", creature_target->spawnid, creature_target->GetCreatureInfo()->Name);
        return true;
    }
}

bool ChatHandler::HandleNpcSetFlagsCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "You need to define the flag value!");
        return false;
    }

    auto creature_target = getSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return false;

    uint32 npc_flags = atol(args);
    uint32 old_npc_flags = creature_target->GetUInt32Value(UNIT_NPC_FLAGS);

    creature_target->SetUInt32Value(UNIT_NPC_FLAGS, npc_flags);
    WorldDatabase.Execute("UPDATE creature_spawns SET flags = '%lu' WHERE id = %lu", npc_flags, creature_target->spawnid);

    GreenSystemMessage(m_session, "Flags set from %u to %u for spawn ID: %u. You may need to clean your client cache.", old_npc_flags, npc_flags, creature_target->spawnid);
    sGMLog.writefromsession(m_session, "changed npc flags of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_npc_flags, npc_flags);

    return true;
}
