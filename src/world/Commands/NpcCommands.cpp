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
