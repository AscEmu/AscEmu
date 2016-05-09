/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.npc addagent
bool ChatHandler::HandleNpcAddAgentCommand(const char* args, WorldSession* m_session)
{
    //new
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32 ai_type;
    uint32 procEvent;
    uint32 procChance;
    uint32 maxcount;
    uint32 spellId;
    uint32 spellType;
    uint32 spelltargetType;
    uint32 spellCooldown;
    float floatMisc1;
    uint32 Misc2;

    if (sscanf(args, "%u %u %u %u %u %u %u %u %f %u", &ai_type, &procEvent, &procChance, &maxcount, &spellId, &spellType, &spelltargetType, &spellCooldown, &floatMisc1, &Misc2) != 10)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add trainerspell <ai_type> <procEvent> <procChance> <maxcount> <spellId> <spellType> <spelltarget_overwrite> <spellCooldown> <floatMisc1> <Misc2>.");
        return true;
    }

    auto spell_entry = dbcSpell.LookupEntry(spellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Spell %u is not invalid!", spellId);
        return true;
    }

    SystemMessage(m_session, "Added agent_type %u for spell %u to creature %s (%u).", ai_type, spellId, creature_target->GetCreatureInfo()->Name, creature_target->GetEntry());
    sGMLog.writefromsession(m_session, "added agent_type %u for spell %u to creature %s (%u).", ai_type, spellId, creature_target->GetCreatureInfo()->Name, creature_target->GetEntry());
    WorldDatabase.Execute("INSERT INTO ai_agents VALUES(%u, 4, %u, %u, %u, %u, %u, %u, %u, %u, %f, %u",
        creature_target->GetEntry(), ai_type, procEvent, procChance, maxcount, spellId, spellType, spelltargetType, spellCooldown, floatMisc1, Misc2);


    AI_Spell* ai_spell = new AI_Spell;
    ai_spell->agent = static_cast<uint16>(ai_type);
    ai_spell->procChance = procChance;
    ai_spell->procCount = maxcount;
    ai_spell->spell = spell_entry;
    ai_spell->spellType = static_cast<uint8>(spellType);
    ai_spell->spelltargetType = spelltargetType;
    ai_spell->floatMisc1 = floatMisc1;
    ai_spell->Misc2 = Misc2;
    ai_spell->cooldown = spellCooldown;
    ai_spell->procCount = 0;
    ai_spell->procCounter = 0;
    ai_spell->cooldowntime = 0;
    ai_spell->minrange = GetMinRange(sSpellRangeStore.LookupEntry(spell_entry->rangeIndex));
    ai_spell->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(spell_entry->rangeIndex));

    creature_target->GetProto()->spells.push_back(ai_spell);

    switch (ai_type)
    {
        case AGENT_MELEE:
            creature_target->GetAIInterface()->disable_melee = false;
            break;
        case AGENT_RANGED:
            creature_target->GetAIInterface()->m_canRangedAttack = true;
            break;
        case AGENT_FLEE:
            creature_target->GetAIInterface()->m_canFlee = true;
            break;
        case AGENT_SPELL:
            creature_target->GetAIInterface()->addSpellToList(ai_spell);
            break;
        case AGENT_CALLFORHELP:
            creature_target->GetAIInterface()->m_canCallForHelp = true;
            break;
        default:
        {
            RedSystemMessage(m_session, "Invalid ai_type %u", ai_type);
            break;
        }
    }

    return true;
}

//.npc addtrainerspell
bool ChatHandler::HandleNpcAddTrainerSpellCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32 spellid;
    uint32 cost;
    uint32 reqspell;
    uint32 reqlevel;
    uint32 delspell;

    if (sscanf(args, "%u %u %u %u %u", &spellid, &cost, &reqspell, &reqlevel, &delspell) != 5)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add trainerspell <spell_id> <cost> <required_spell> <required_player_level> <delete_spell_id>.");
        return true;
    }

    auto creature_trainer = creature_target->GetTrainer();
    if (creature_trainer == nullptr)
    {
        RedSystemMessage(m_session, "%s (%u) is not a trainer!", creature_target->GetCreatureInfo()->Name, creature_target->GetEntry());
        return true;
    }

    auto learn_spell = dbcSpell.LookupEntryForced(spellid);
    if (learn_spell == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell %u.", spellid);
        return true;
    }

    if (learn_spell->Effect[0] == SPELL_EFFECT_INSTANT_KILL || learn_spell->Effect[1] == SPELL_EFFECT_INSTANT_KILL || learn_spell->Effect[2] == SPELL_EFFECT_INSTANT_KILL)
    {
        RedSystemMessage(m_session, "You are not allowed to learn spells with instant kill effect!");
        return true;
    }

    TrainerSpell sp;
    sp.Cost = cost;
    sp.IsProfession = false;
    sp.pLearnSpell = learn_spell;
    sp.pCastRealSpell = nullptr;
    sp.pCastSpell = nullptr;
    sp.RequiredLevel = reqlevel;
    sp.RequiredSpell = reqspell;
    sp.DeleteSpell = delspell;

    creature_trainer->Spells.push_back(sp);
    creature_trainer->SpellCount++;

    SystemMessage(m_session, "Added spell %s (%u) to trainer %s (%u).", learn_spell->Name, learn_spell->Id, creature_target->GetCreatureInfo()->Name, creature_target->GetEntry());
    sGMLog.writefromsession(m_session, "added spell  %s (%u) to trainer %s (%u)", learn_spell->Name, learn_spell->Id, creature_target->GetCreatureInfo()->Name, creature_target->GetEntry());
    WorldDatabase.Execute("REPLACE INTO trainer_spells VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
        creature_target->GetEntry(), (int)0, learn_spell->Id, cost, reqspell, (int)0, (int)0, reqlevel, delspell, (int)0);

    return true;
}

//.npc come
bool ChatHandler::HandleNpcComeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto player = m_session->GetPlayer();
    creature_target->GetAIInterface()->MoveTo(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
    sGMLog.writefromsession(m_session, "used .npc come on %s spawn ID: %u", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

//.npc follow
bool ChatHandler::HandleNpcFollowCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->GetAIInterface()->SetUnitToFollow(m_session->GetPlayer());
    sGMLog.writefromsession(m_session, "used npc follow command on %s, sqlid %u", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

//.npc stopfollow
bool ChatHandler::HandleNpcStopFollowCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->GetAIInterface()->SetAIState(STATE_IDLE);
    creature_target->GetAIInterface()->ResetUnitToFollow();

    sGMLog.writefromsession(m_session, "cancelled npc follow command on %s, sqlid %u", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

//.npc respawn
bool ChatHandler::HandleNpcRespawnCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (creature_target->IsCreature() && creature_target->getDeathState() == CORPSE && creature_target->spawnid != 0)
    {
        sEventMgr.RemoveEvents(creature_target, EVENT_CREATURE_RESPAWN);

        BlueSystemMessage(m_session, "Respawning Creature: `%s` with entry: %u on map: %u spawnid: %u", creature_target->GetCreatureInfo()->Name,
            creature_target->GetEntry(), creature_target->GetMapMgr()->GetMapId(), creature_target->spawnid);
        sGMLog.writefromsession(m_session, "respawned Creature: `%s` with entry: %u on map: %u sqlid: %u", creature_target->GetCreatureInfo()->Name,
            creature_target->GetEntry(), creature_target->GetMapMgr()->GetMapId(), creature_target->spawnid);

        creature_target->Despawn(0, 1000);
    }
    else
    {
        RedSystemMessage(m_session, "You must select a creature's corpse with a valid spawnid.");
        return true;
    }

    return true;
}

//.npc return
bool ChatHandler::HandleNpcReturnCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    float x = creature_target->m_spawn->x;
    float y = creature_target->m_spawn->y;
    float z = creature_target->m_spawn->z;
    float o = creature_target->m_spawn->o;

    creature_target->GetAIInterface()->SetAIState(STATE_IDLE);
    creature_target->GetAIInterface()->WipeHateList();
    creature_target->GetAIInterface()->WipeTargetList();
    creature_target->GetAIInterface()->MoveTo(x, y, z, o);

    sGMLog.writefromsession(m_session, "returned NPC %s (%u)", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);

    return true;
}

//.npc say
bool ChatHandler::HandleNpcSayCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (!args)
    {
        RedSystemMessage(m_session, "No text set. Use .npc say <text>!");
        return true;
    }

    creature_target->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, args);

    return true;
}

//.npc spawn
bool ChatHandler::HandleNpcSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32 entry = atol(args);
    if (entry == 0)
        return false;

    auto creature_proto = CreatureProtoStorage.LookupEntry(entry);
    auto creature_info = CreatureNameStorage.LookupEntry(entry);
    if (creature_proto == nullptr || creature_info == nullptr)
    {
        RedSystemMessage(m_session, "Creature with entry %u is not a valid entry (no proto/name information in database)", entry);
        return true;
    }

    auto creature_spawn = new CreatureSpawn;
    uint8 gender = creature_info->GenerateModelId(&creature_spawn->displayid);
    creature_spawn->entry = entry;
    creature_spawn->form = 0;
    creature_spawn->id = objmgr.GenerateCreatureSpawnID();
    creature_spawn->movetype = 0;
    creature_spawn->x = m_session->GetPlayer()->GetPositionX();
    creature_spawn->y = m_session->GetPlayer()->GetPositionY();
    creature_spawn->z = m_session->GetPlayer()->GetPositionZ();
    creature_spawn->o = m_session->GetPlayer()->GetOrientation();
    creature_spawn->emote_state = 0;
    creature_spawn->flags = creature_proto->NPCFLags;
    creature_spawn->factionid = creature_proto->Faction;
    creature_spawn->bytes0 = creature_spawn->setbyte(0, 2, gender);
    creature_spawn->bytes1 = 0;
    creature_spawn->bytes2 = 0;
    creature_spawn->stand_state = 0;
    creature_spawn->death_state = 0;
    creature_spawn->channel_target_creature = creature_spawn->channel_target_go = creature_spawn->channel_spell = 0;
    creature_spawn->MountedDisplayID = 0;
    creature_spawn->Item1SlotDisplay = creature_proto->itemslot_1;
    creature_spawn->Item2SlotDisplay = creature_proto->itemslot_2;
    creature_spawn->Item3SlotDisplay = creature_proto->itemslot_3;
    creature_spawn->CanFly = 0;
    creature_spawn->phase = m_session->GetPlayer()->GetPhase();

    auto creature = m_session->GetPlayer()->GetMapMgr()->CreateCreature(entry);
    ARCEMU_ASSERT(creature != nullptr);
    creature->Load(creature_spawn, 0, nullptr);
    creature->m_loadedFromDB = true;
    creature->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    // Add to map
    uint32 x = m_session->GetPlayer()->GetMapMgr()->GetPosX(m_session->GetPlayer()->GetPositionX());
    uint32 y = m_session->GetPlayer()->GetMapMgr()->GetPosY(m_session->GetPlayer()->GetPositionY());
    m_session->GetPlayer()->GetMapMgr()->GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(creature_spawn);
    MapCell* map_cell = m_session->GetPlayer()->GetMapMgr()->GetCell(x, y);
    if (map_cell != nullptr)
        map_cell->SetLoaded();

    creature->SaveToDB();

    BlueSystemMessage(m_session, "Spawned a creature `%s` with entry %u at %f %f %f on map %u", creature_info->Name,
        entry, creature_spawn->x, creature_spawn->y, creature_spawn->z, m_session->GetPlayer()->GetMapId());
    sGMLog.writefromsession(m_session, "spawned a %s at %u %f %f %f", creature_info->Name, m_session->GetPlayer()->GetMapId(),
        creature_spawn->x, creature_spawn->y, creature_spawn->z);

    return true;
}

//.npc yell
bool ChatHandler::HandleNpcYellCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (!args)
    {
        RedSystemMessage(m_session, "No text set. Use .npc say <text>!");
        return true;
    }

    creature_target->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, args);

    return true;
}

// Zyres: following commands are for units
//.npc possess
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
            BlueSystemMessage(m_session, "Player %s selected.", player->GetName());
            sGMLog.writefromsession(m_session, "used possess command on PLAYER %s", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
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

//.npc unpossess
bool ChatHandler::HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);

    if (unit_target != nullptr)
    {
        if (unit_target->IsPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            BlueSystemMessage(m_session, "Player %s is no longer possessed by you.", player->GetName());
        }
        else if (unit_target->IsCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
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
//.npc set canfly
bool ChatHandler::HandleNpcSetCanFlyCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    bool save_to_db = atoi(args) == 1 ? true : false;

    if (creature_target->GetAIInterface()->Flying())
    {
        creature_target->GetAIInterface()->StopFlying();
        if (save_to_db)
        {
            WorldDatabase.Execute("UPDATE creature_spawns SET CanFly = 1 WHERE id = %lu", creature_target->spawnid);
            GreenSystemMessage(m_session, "CanFly permanent set from 0 to 1 for Creature %s (%u).", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
            sGMLog.writefromsession(m_session, "changed npc CanFly for creature_spawn ID: %u [%s] from 0 to 1", creature_target->spawnid, creature_target->GetCreatureInfo()->Name);
        }
        else
        {
            GreenSystemMessage(m_session, "CanFly temporarily set from 0 to 1 for Creature %s (%u).", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
        }
    }
    else
    {
        creature_target->GetAIInterface()->SetFly();
        if (save_to_db)
        {
            WorldDatabase.Execute("UPDATE creature_spawns SET CanFly = 0 WHERE id = %lu", creature_target->spawnid);
            GreenSystemMessage(m_session, "CanFly permanent set from 1 to 0 for Creature %s (%u).", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
            sGMLog.writefromsession(m_session, "changed npc CanFly for creature_spawn ID: %u [%s] from 1 to 0", creature_target->spawnid, creature_target->GetCreatureInfo()->Name);
        }
        else
        {
            GreenSystemMessage(m_session, "CanFly temporarily set from 1 to 0 for Creature %s (%u).", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
        }
    }

    return true;
}

//.npc set equip
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

    Creature* creature_target = GetSelectedCreature(m_session, true);
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

//.npc set emote
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

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32 old_emote = creature_target->GetEmoteState();
    creature_target->SetEmoteState(emote);
    if (save == 1)
    {
        WorldDatabase.Execute("UPDATE creature_spawns SET emote_state = '%lu' WHERE id = %lu", emote, creature_target->spawnid);
        GreenSystemMessage(m_session, "Emote permanent set from %u to %u for spawn ID: %u.", old_emote, emote, creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc emote of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_emote, emote);
    }
    else
    {
        GreenSystemMessage(m_session, "Emote temporarily set from %u to %u for spawn ID: %u.", old_emote, emote, creature_target->spawnid);
    }

    return true;
}

//.npc set formationmaster
bool ChatHandler::HandleNpcSetFormationMasterCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    m_session->GetPlayer()->linkTarget = creature_target;
    BlueSystemMessage(m_session, "Formation Master set to %s spawn ID: %u.", creature_target->GetCreatureInfo()->Name, creature_target->spawnid);
    return true;
}

//.npc set formationslave
bool ChatHandler::HandleNpcSetFormationSlaveCommand(const char* args, WorldSession* m_session)
{
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

    auto creature_slave = GetSelectedCreature(m_session, true);
    if (creature_slave == nullptr)
        return true;

    creature_slave->GetAIInterface()->m_formationFollowDistance = distance;
    creature_slave->GetAIInterface()->m_formationFollowAngle = angle;
    creature_slave->GetAIInterface()->m_formationLinkTarget = m_session->GetPlayer()->linkTarget->GetGUID();
    creature_slave->GetAIInterface()->m_formationLinkSqlId = m_session->GetPlayer()->linkTarget->GetSQL_id();
    creature_slave->GetAIInterface()->SetUnitToFollowAngle(angle);

    BlueSystemMessage(m_session, "%s linked to %s with a distance of %f at %f radians.", creature_slave->GetCreatureInfo()->Name, m_session->GetPlayer()->linkTarget->GetCreatureInfo()->Name, distance, angle);

    if (save == 1)
    {
        WorldDatabase.Execute("REPLACE INTO creature_formations VALUES(%u, %u, '%f', '%f')", creature_slave->GetSQL_id(), creature_slave->GetAIInterface()->m_formationLinkSqlId, angle, distance);
        BlueSystemMessage(m_session, "%s linked to %s with a distance of %f at %f radians.", creature_slave->GetCreatureInfo()->Name, m_session->GetPlayer()->linkTarget->GetCreatureInfo()->Name, distance, angle);
        sGMLog.writefromsession(m_session, "changed npc formation of creature_spawn ID: %u [%s]", creature_slave->spawnid, creature_slave->GetCreatureInfo()->Name);
    }
    else
    {
        BlueSystemMessage(m_session, "%s temporarily linked to %s with a distance of %f at %f radians.", creature_slave->GetCreatureInfo()->Name, m_session->GetPlayer()->linkTarget->GetCreatureInfo()->Name, distance, angle);
    }

    return true;
}

//.npc set formationclear
bool ChatHandler::HandleNpcSetFormationClearCommand(const char* args, WorldSession* m_session)
{
    uint32 save = atol(args);

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->GetAIInterface()->m_formationLinkSqlId = 0;
    creature_target->GetAIInterface()->m_formationLinkTarget = 0;
    creature_target->GetAIInterface()->m_formationFollowAngle = 0.0f;
    creature_target->GetAIInterface()->m_formationFollowDistance = 0.0f;
    creature_target->GetAIInterface()->ResetUnitToFollow();

    if (save == 1)
    {
        BlueSystemMessage(m_session, "%s linked formation cleared in database.", creature_target->GetCreatureInfo()->Name);
        WorldDatabase.Execute("DELETE FROM creature_formations WHERE spawn_id=%u", creature_target->GetSQL_id());
        sGMLog.writefromsession(m_session, "removed npc formation for creature_spawn ID: %u [%s]", creature_target->spawnid, creature_target->GetCreatureInfo()->Name);
    }
    else
    {
        BlueSystemMessage(m_session, "%s linked formation temporarily cleared.", creature_target->GetCreatureInfo()->Name);
    }

    return true;
}

//.npc set flags
bool ChatHandler::HandleNpcSetFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32 npc_flags;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &npc_flags, &save) < 1)
    {
        RedSystemMessage(m_session, "You need to define the flag value!");
        RedSystemMessage(m_session, ".npc set flags <npc_flag>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return false;

    uint32 old_npc_flags = creature_target->GetUInt32Value(UNIT_NPC_FLAGS);
    creature_target->SetUInt32Value(UNIT_NPC_FLAGS, npc_flags);

    if (save == 1)
    {
        GreenSystemMessage(m_session, "Flags changed in spawns table from %u to %u for spawn ID: %u. You may need to clean your client cache.", old_npc_flags, npc_flags, creature_target->spawnid);
        WorldDatabase.Execute("UPDATE creature_spawns SET flags = '%lu' WHERE id = %lu", npc_flags, creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc flags of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_npc_flags, npc_flags);
    }
    else
    {
        GreenSystemMessage(m_session, "Flags temporarily set from %u to %u for spawn ID: %u. You may need to clean your client cache.", old_npc_flags, npc_flags, creature_target->spawnid);
    }

    return true;
}

//.npc set phase
bool ChatHandler::HandleNpcSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32 npc_phase;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &npc_phase, &save) < 1)
    {
        RedSystemMessage(m_session, "You need to define the phase!");
        RedSystemMessage(m_session, ".npc set phase <npc_phase>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return false;

    uint32 old_npc_phase = creature_target->m_spawn->phase;
    creature_target->Phase(PHASE_SET, npc_phase);

    if (save == 1)
    {
        GreenSystemMessage(m_session, "Phase changed in spawns table from %u to %u for spawn ID: %u.", old_npc_phase, npc_phase, creature_target->spawnid);
        WorldDatabase.Execute("UPDATE creature_spawns SET phase = '%lu' WHERE id = %lu", npc_phase, creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc phase of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_npc_phase, npc_phase);
    }
    else
    {
        GreenSystemMessage(m_session, "Phase temporarily set from %u to %u for spawn ID: %u.", old_npc_phase, npc_phase, creature_target->spawnid);
    }

    return true;
}

//.npc set standstate
bool ChatHandler::HandleNpcSetStandstateCommand(const char* args, WorldSession* m_session)
{
    uint32 standstate;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &standstate, &save) < 1)
    {
        RedSystemMessage(m_session, "You must specify a standstate value.");
        RedSystemMessage(m_session, ".npc set standstate <standstate>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint8 old_standstate = creature_target->getStandState();
    creature_target->SetStandState(standstate);

    if (save == 1)
    {
        GreenSystemMessage(m_session, "Standstate changed in spawns table from %u to %u for spawn ID: %u.", old_standstate, standstate, creature_target->spawnid);
        WorldDatabase.Execute("UPDATE creature_spawns SET standstate = '%lu' WHERE id = %lu", standstate, creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc standstate of creature_spawn ID: %u [%s] from %u to %u", creature_target->spawnid, creature_target->GetCreatureInfo()->Name, old_standstate, standstate);
    }
    else
    {
        GreenSystemMessage(m_session, "Standstate temporarily set from %u to %u for spawn ID: %u.", old_standstate, standstate, creature_target->spawnid);
    }

    return true;

}
