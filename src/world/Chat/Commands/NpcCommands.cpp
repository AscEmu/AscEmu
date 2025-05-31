/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

//.npc addagent
bool ChatHandler::HandleNpcAddAgentCommand(const char* args, WorldSession* m_session)
{
    //new
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t ai_type;
    uint32_t procEvent;
    uint32_t procChance;
    uint32_t maxcount;
    uint32_t spellId;
    uint32_t spellType;
    uint32_t spelltargetType;
    uint32_t spellCooldown;
    float floatMisc1;
    uint32_t Misc2;

    if (sscanf(args, "%u %u %u %u %u %u %u %u %f %u", &ai_type, &procEvent, &procChance, &maxcount, &spellId, &spellType, &spelltargetType, &spellCooldown, &floatMisc1, &Misc2) != 10)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add trainerspell <ai_type> <procEvent> <procChance> <maxcount> <spellId> <spellType> <spelltarget_overwrite> <spellCooldown> <floatMisc1> <Misc2>.");
        return true;
    }

    auto spell_entry = sSpellMgr.getSpellInfo(spellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Spell %u is not invalid!", spellId);
        return true;
    }

    SystemMessage(m_session, "Added agent_type %u for spell %u to creature %s (%u).", ai_type, spellId, creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    sGMLog.writefromsession(m_session, "added agent_type %u for spell %u to creature %s (%u).", ai_type, spellId, creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    WorldDatabase.Execute("INSERT INTO ai_agents VALUES(%u, 4, %u, %u, %u, %u, %u, %u, %u, %u, %f, %u",
        creature_target->getEntry(), ai_type, procEvent, procChance, maxcount, spellId, spellType, spelltargetType, spellCooldown, floatMisc1, Misc2);

    auto ai_spell = std::make_unique<AI_Spell>();
    ai_spell->agent = static_cast<uint16_t>(ai_type);
    ai_spell->procChance = procChance;
    ai_spell->procCount = maxcount;
    ai_spell->spell = spell_entry;
    ai_spell->spellType = static_cast<uint8_t>(spellType);
    ai_spell->spelltargetType = static_cast<uint8_t>(spelltargetType);
    ai_spell->floatMisc1 = floatMisc1;
    ai_spell->Misc2 = Misc2;
    ai_spell->cooldown = spellCooldown;
    ai_spell->procCounter = 0;
    ai_spell->cooldowntime = 0;
    ai_spell->minrange = spell_entry->getMinRange();
    ai_spell->maxrange = spell_entry->getMaxRange();

    if (auto* creatureProperties = const_cast<CreatureProperties*>(creature_target->GetCreatureProperties()))
    {
        if (!spell_entry->isPassive())
            creatureProperties->castable_spells.emplace_back(spell_entry->getId());
        else
            creatureProperties->start_auras.emplace(spell_entry->getId());
    }

    switch (ai_type)
    {
        case AGENT_MELEE:
            creature_target->getAIInterface()->setMeleeDisabled(false);
            break;
        case AGENT_RANGED:
            creature_target->getAIInterface()->m_canRangedAttack = true;
            break;
        case AGENT_FLEE:
            creature_target->getAIInterface()->m_canFlee = true;
            break;
        case AGENT_SPELL:
            creature_target->getAIInterface()->addSpellToList(std::move(ai_spell));
            break;
        case AGENT_CALLFORHELP:
            creature_target->getAIInterface()->m_canCallForHelp = true;
            break;
        default:
        {
            RedSystemMessage(m_session, "Invalid ai_type %u", ai_type);
            break;
        }
    }
    return true;
}

bool ChatHandler::HandleNpcAppearCommand(const char* /*_*/, WorldSession* session)
{
    const auto target = GetSelectedCreature(session);
    if (!target) {
        return true;
    }

    session->GetPlayer()->teleport(target->GetPosition(), target->getWorldMap());
    return true;
}

//.npc addtrainerspell
bool ChatHandler::HandleNpcAddTrainerSpellCommand(const char* args, WorldSession* m_session)
{
    /*auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t spellid;
    uint32_t cost;
    uint32_t reqlevel;
    uint32_t reqspell;
    uint32_t delspell;

    if (sscanf(args, "%u %u %u %u %u", &spellid, &cost, &reqspell, &reqlevel, &delspell) != 5)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add trainerspell <spell_id> <cost> <required_spell> <required_player_level> <delete_spell_id>.");
        return true;
    }

    auto creature_trainer = creature_target->GetTrainer();
    if (creature_trainer == nullptr)
    {
        RedSystemMessage(m_session, "%s (%u) is not a trainer!", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
        return true;
    }

    auto learn_spell = sSpellMgr.getSpellInfo(spellid);
    if (learn_spell == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell %u.", spellid);
        return true;
    }

    if (learn_spell->getEffect(0) == SPELL_EFFECT_INSTANT_KILL || learn_spell->getEffect(1) == SPELL_EFFECT_INSTANT_KILL || learn_spell->getEffect(2) == SPELL_EFFECT_INSTANT_KILL)
    {
        RedSystemMessage(m_session, "You are not allowed to learn spells with instant kill effect!");
        return true;
    }

    TrainerSpell sp;
    sp.cost = cost;
    sp.learnSpell = learn_spell;
    sp.requiredLevel = reqlevel;
    sp.requiredSpell[0] = reqspell;
    sp.deleteSpell = delspell;

    creature_trainer->Spells.push_back(sp);
    creature_trainer->SpellCount++;

    SystemMessage(m_session, "Added spell %s (%u) to trainer %s (%u).", learn_spell->getName().c_str(), learn_spell->getId(), creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    sGMLog.writefromsession(m_session, "added spell  %s (%u) to trainer %s (%u)", learn_spell->getName().c_str(), learn_spell->getId(), creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    WorldDatabase.Execute("REPLACE INTO trainer_spells VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
        creature_target->getEntry(), 0, learn_spell->getId(), cost, reqspell, 0, 0, reqlevel, delspell, 0);*/

    return true;
}

//.npc cast
bool ChatHandler::HandleNpcCastCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t spell_id;
    if (sscanf(args, "%u", &spell_id) != 1)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc cast <spellid>.");
        return true;
    }

    auto spell_entry = sSpellMgr.getSpellInfo(spell_id);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Invalid Spell ID: %u !", spell_id);
        return true;
    }

    auto unit_target = static_cast<Unit*>(creature_target);
    unit_target->castSpell(unit_target, spell_id, false);

    return true;
}

//.npc come
bool ChatHandler::HandleNpcComeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto player = m_session->GetPlayer();
    creature_target->getMovementManager()->movePoint(0, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), false, player->GetOrientation());
    sGMLog.writefromsession(m_session, "used .npc come on %s spawn ID: %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);

    return true;
}

//.npc delete
bool ChatHandler::HandleNpcDeleteCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (creature_target->isPet())
    {
        SystemMessage(m_session, "You can't delete a pet.");
        return true;
    }

    if (creature_target->isSummon())
    {
        creature_target->Delete();
    }
    else
    {
        //creature_target->getAIInterface()->hideWayPoints(m_session->GetPlayer());

        uint32_t spawn_id = creature_target->spawnid;
        if (spawn_id != 0)
        {
            BlueSystemMessage(m_session, "Creature %s (%u) deleted from creature_spawn table.", creature_target->GetCreatureProperties()->Name.c_str(), spawn_id);
            sGMLog.writefromsession(m_session, "used npc delete on creature %s (%u), pos %f %f %f", creature_target->GetCreatureProperties()->Name.c_str(), spawn_id, creature_target->GetPositionX(), creature_target->GetPositionY(), creature_target->GetPositionZ());
            creature_target->DeleteFromDB();
        }

        if (creature_target->m_spawn)
        {
            uint32_t cellx = uint32_t(((Map::Terrain::_maxX - creature_target->m_spawn->x) / Map::Cell::cellSize));
            uint32_t celly = uint32_t(((Map::Terrain::_maxY - creature_target->m_spawn->y) / Map::Cell::cellSize));

            if (cellx <= Map::Cell::_sizeX && celly <= Map::Cell::_sizeY)
            {
                CellSpawns* sp = creature_target->getWorldMap()->getBaseMap()->getSpawnsList(cellx, celly);
                if (sp != nullptr)
                {
                    for (CreatureSpawnList::iterator itr = sp->CreatureSpawns.begin(); itr != sp->CreatureSpawns.end(); ++itr)
                        if ((*itr) == creature_target->m_spawn)
                        {
                            sp->CreatureSpawns.erase(itr);
                            break;
                        }
                }
                delete creature_target->m_spawn;
                creature_target->m_spawn = nullptr;
            }
        }

        creature_target->RemoveFromWorld(false, true);
    }

    return true;
}

//.npc follow
bool ChatHandler::HandleNpcFollowCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->getMovementManager()->moveFollow(m_session->GetPlayer(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    sGMLog.writefromsession(m_session, "used npc follow command on %s, sqlid %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);
    return true;
}

//.npc info
bool ChatHandler::HandleNpcInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t guid = WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayer()->getTargetGuid());

    SystemMessage(m_session, "Showing Creature info of %s =============", creature_target->GetCreatureProperties()->Name.c_str());
    RedSystemMessage(m_session, "EntryID: %d", creature_target->getEntry());
    RedSystemMessage(m_session, "SpawnID: %d", creature_target->GetSQL_id());
    SystemMessage(m_session, "GUID: %u", guid);
    SystemMessage(m_session, "Faction: %u", creature_target->getFactionTemplate());
    SystemMessage(m_session, "Phase: %u", creature_target->GetPhase());

    SystemMessage(m_session, "DisplayID: %u", creature_target->getDisplayId());

    uint8_t creature_gender = creature_target->getGender();
    if (creature_gender <= 2)
        SystemMessage(m_session, "Gender: %s", GENDER[creature_gender].data());
    else
        SystemMessage(m_session, "Gender: invalid %u", creature_gender);

    uint8_t creature_class = creature_target->getClass();
    if (creature_class <= 11)
        SystemMessage(m_session, "Class: %s", CLASS[creature_class].data());
    else
        SystemMessage(m_session, "Class: invalid %u", creature_class);

    SystemMessage(m_session, "Health (cur / max): %u / %u", creature_target->getHealth(), creature_target->getMaxHealth());

    auto powertype = creature_target->getPowerType();
    if (powertype <= 6)
    {
        SystemMessage(m_session, "Powertype: %s", POWERTYPE[powertype].data());
        SystemMessage(m_session, "Power (cur / max): %u / %u", creature_target->getPower(powertype), creature_target->getMaxPower(powertype));
    }

    SystemMessage(m_session, "Damage (min / max): %f / %f", creature_target->getMinDamage(), creature_target->getMaxDamage());

#if VERSION_STRING < WotLK
    if (creature_target->getPetLoyalty() != 0)
        SystemMessage(m_session, "Pet loyalty level: %u", creature_target->getPetLoyalty());
#elif VERSION_STRING < Mop
    if (creature_target->getPetTalentPoints() != 0)
        SystemMessage(m_session, "Free pet talent points: %u", creature_target->getPetTalentPoints());
#endif

    if (creature_target->GetCreatureProperties()->vehicleid > 0)
        SystemMessage(m_session, "VehicleID: %u", creature_target->GetCreatureProperties()->vehicleid);

    if (creature_target->m_factionTemplate)
        SystemMessage(m_session, "Combat Support: 0x%.3X", creature_target->m_factionTemplate->FriendlyMask);

    if (creature_target->getCombatHandler().isInCombat())
        SystemMessage(m_session, "Is in combat!");
    else
        SystemMessage(m_session, "Not in combat!");

    uint8_t sheat = creature_target->getSheathType();
    if (sheat <= 2)
        SystemMessage(m_session, "Sheat state: %s", SHEATSTATE[sheat].data());

    SystemMessage(m_session, "=================================");

    //////////////////////////////////////////////////////////////////////////////////////////
    // resistance
    if (creature_target->getResistance(SCHOOL_NORMAL) || creature_target->getResistance(SCHOOL_HOLY) ||
        creature_target->getResistance(SCHOOL_FIRE) || creature_target->getResistance(SCHOOL_NATURE) ||
        creature_target->getResistance(SCHOOL_FROST) || creature_target->getResistance(SCHOOL_SHADOW) ||
        creature_target->getResistance(SCHOOL_ARCANE))
    {
        GreenSystemMessage(m_session, "Resistance =======================");
        if (creature_target->getResistance(SCHOOL_NORMAL))
            GreenSystemMessage(m_session, "-- Armor: %u", creature_target->getResistance(SCHOOL_NORMAL));
        if (creature_target->getResistance(SCHOOL_HOLY))
            GreenSystemMessage(m_session, "-- Holy: %u", creature_target->getResistance(SCHOOL_HOLY));
        if (creature_target->getResistance(SCHOOL_FIRE))
            GreenSystemMessage(m_session, "-- Fire: %u", creature_target->getResistance(SCHOOL_FIRE));
        if (creature_target->getResistance(SCHOOL_NATURE))
            GreenSystemMessage(m_session, "-- Nature: %u", creature_target->getResistance(SCHOOL_NATURE));
        if (creature_target->getResistance(SCHOOL_FROST))
            GreenSystemMessage(m_session, "-- Frost: %u", creature_target->getResistance(SCHOOL_FROST));
        if (creature_target->getResistance(SCHOOL_SHADOW))
            GreenSystemMessage(m_session, "-- Shadow: %u", creature_target->getResistance(SCHOOL_SHADOW));
        if (creature_target->getResistance(SCHOOL_ARCANE))
            GreenSystemMessage(m_session, "-- Arcane: %u", creature_target->getResistance(SCHOOL_ARCANE));
        GreenSystemMessage(m_session, "=================================");
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // show byte
    std::stringstream sstext;
    sstext << "UNIT_FIELD_BYTES_0 are:" << '\n';
    sstext << " -Race: " << std::to_string(creature_target->getRace()) << '\n';
    sstext << " -Class: " << std::to_string(creature_target->getClass()) << '\n';
    sstext << " -Gender: " << std::to_string(creature_target->getGender()) << '\n';
    sstext << " -Power Type: " << std::to_string(creature_target->getPowerType()) << '\n';
    sstext << '\n';

    sstext << "UNIT_FIELD_BYTES_1 are:" << '\n';
    sstext << " -StandState: " << std::to_string(creature_target->getStandState()) << '\n';
#if VERSION_STRING < WotLK
    sstext << " -Pet Loyalty: " << std::to_string(creature_target->getPetLoyalty()) << '\n';
#elif VERSION_STRING < Mop
    sstext << " -Pet TP: " << std::to_string(creature_target->getPetTalentPoints()) << '\n';
#else
    const auto theBytes = creature_target->getBytes1();
    sstext << " -Unk1: " << std::to_string(uint16_t((uint8_t)(theBytes >> 8) & 0xFF)) << '\n';
#endif
#if VERSION_STRING == Classic
    sstext << " -ShapeShift Form: " << std::to_string(creature_target->getShapeShiftForm()) << '\n';
    sstext << " -StandState Flag: " << std::to_string(creature_target->getStandStateFlags()) << '\n';
#else
    sstext << " -StandState Flag: " << std::to_string(creature_target->getStandStateFlags()) << '\n';
    sstext << " -Animation Flag: " << std::to_string(creature_target->getAnimationFlags()) << '\n';
#endif
    sstext << '\n';

    sstext << "UNIT_FIELD_BYTES_2 are:" << '\n';
    sstext << " -SheathType: " << std::to_string(creature_target->getSheathType()) << '\n';
#if VERSION_STRING == Classic
    const auto theBytes = creature_target->getBytes2();
    sstext << " -Unk1: " << uint16_t((uint8_t)(theBytes >> 8) & 0xFF) << '\n';
    sstext << " -Unk2: " << uint16_t((uint8_t)(theBytes >> 16) & 0xFF) << '\n';
    sstext << " -Unk3: " << uint16_t((uint8_t)(theBytes >> 24) & 0xFF) << '\n';
#else
#if VERSION_STRING == TBC
    sstext << " -Positive Aura Limit: " << std::to_string(creature_target->getPositiveAuraLimit()) << '\n';
#else
    sstext << " -PvP Flag: " << std::to_string(creature_target->getPvpFlags()) << '\n';
#endif
    sstext << " -Pet Flag: " << std::to_string(creature_target->getPetFlags()) << '\n';
    sstext << " -ShapeShift Form: " << std::to_string(creature_target->getShapeShiftForm()) << '\n';
#endif
    sstext << '\0';

    SystemMessage(m_session, "UNIT_FIELD_BYTES =================");
    SendMultilineMessage(m_session, sstext.str().c_str());
    SystemMessage(m_session, "=================================");

    //////////////////////////////////////////////////////////////////////////////////////////
    // flags
    GreenSystemMessage(m_session, "Flags ============================");
    std::string s = GetNpcFlagString(creature_target);
    GreenSystemMessage(m_session, "NpcFlags: %u%s", creature_target->getNpcFlags(), s.c_str());

#if VERSION_STRING >= WotLK
    uint8_t pvp_flags = creature_target->getPvpFlags();
    GreenSystemMessage(m_session, "PvPFlags: %u", pvp_flags);

    for (uint32_t i = 0; i < numpvpflags; i++)
        if ((pvp_flags & UnitPvPFlagToName[i].Flag) != 0)
            GreenSystemMessage(m_session, "%s", UnitPvPFlagToName[i].Name.data());
#endif

#if VERSION_STRING >= TBC
    uint8_t pet_flags = creature_target->getPetFlags();
    if (pet_flags != 0)
    {
        GreenSystemMessage(m_session, "PetFlags: %u", pet_flags);
        for (uint32_t i = 0; i < numpetflags; i++)
            if ((pet_flags & PetFlagToName[i].Flag) != 0)
                GreenSystemMessage(m_session, "%s", PetFlagToName[i].Name.data());
    }
#endif

    uint32_t unit_flags = creature_target->getUnitFlags();
    GreenSystemMessage(m_session, "UnitFlags: %u", unit_flags);

    for (uint32_t i = 0; i < numflags; i++)
        if ((unit_flags & UnitFlagToName[i].Flag) != 0)
            GreenSystemMessage(m_session, "-- %s", UnitFlagToName[i].Name.data());

#if VERSION_STRING > Classic
    uint32_t unit_flags2 = creature_target->getUnitFlags2();
    GreenSystemMessage(m_session, "UnitFlags2: %u", unit_flags2);

    for (uint32_t i = 0; i < numflags2; i++)
        if ((unit_flags2 & UnitFlagToName2[i].Flag) != 0)
            GreenSystemMessage(m_session, "-- %s", UnitFlagToName2[i].Name.data());
#endif

    uint32_t dyn_flags = creature_target->getDynamicFlags();
    GreenSystemMessage(m_session, "UnitDynamicFlags: %u", dyn_flags);

    for (uint32_t i = 0; i < numdynflags; i++)
        if ((dyn_flags & UnitDynFlagToName[i].Flag) != 0)
            GreenSystemMessage(m_session, "%s", UnitDynFlagToName[i].Name.data());

    GreenSystemMessage(m_session, "=================================");

    //////////////////////////////////////////////////////////////////////////////////////////
    // owner/summoner
    Unit* unit_owner = nullptr;
    bool owner_header_set = false;
    if (creature_target->isSummon())
        unit_owner = static_cast<Summon*>(creature_target)->getUnitOwner();

    if (unit_owner != nullptr)
    {
        SystemMessage(m_session, "Owner/Summoner ===================");

        if (unit_owner->isPlayer())
            SystemMessage(m_session, "Owner is Player: %s", static_cast<Player*>(unit_owner)->getName().c_str());
        if (unit_owner->isPet())
            SystemMessage(m_session, "Owner is Pet: %s", static_cast<Creature*>(unit_owner)->GetCreatureProperties()->Name.c_str());
        if (unit_owner->isCreature())
            SystemMessage(m_session, "Owner is Creature: %s", static_cast<Creature*>(unit_owner)->GetCreatureProperties()->Name.c_str());

        owner_header_set = true;
    }

    if (creature_target->getCreatedByGuid() || creature_target->getSummonedByGuid() ||
        creature_target->getCharmedByGuid() || creature_target->getCreatedBySpellId())
    {
        if (!owner_header_set)
        {
            SystemMessage(m_session, "Owner/Summoner ===================");
            owner_header_set = true;
        }

        if (creature_target->getCreatedByGuid())
            SystemMessage(m_session, "Creator GUID: %u", WoWGuid::getGuidLowPartFromUInt64(creature_target->getCreatedByGuid()));
        if (creature_target->getSummonedByGuid())
            SystemMessage(m_session, "Summoner GUID: %u", WoWGuid::getGuidLowPartFromUInt64(creature_target->getSummonedByGuid()));
        if (creature_target->getCharmedByGuid())
            SystemMessage(m_session, "Charmer GUID: %u", WoWGuid::getGuidLowPartFromUInt64(creature_target->getCharmedByGuid()));
        if (creature_target->getCreatedBySpellId())
            SystemMessage(m_session, "Creator Spell: %u", WoWGuid::getGuidLowPartFromUInt64(creature_target->getCreatedBySpellId()));
    }

    if (owner_header_set)
        SystemMessage(m_session, "=================================");

    if (creature_target->m_spawn != nullptr)
        SystemMessage(m_session, "Is part of table: %s", creature_target->m_spawn->origine.c_str());
    else
        SystemMessage(m_session, "Is spawnd by an internal script");

    //////////////////////////////////////////////////////////////////////////////////////////
    // equipment
    GreenSystemMessage(m_session, "Equipment ============================");
#if VERSION_STRING < WotLK
    GreenSystemMessage(m_session, "-- Melee: %u", creature_target->getVirtualItemEntry(MELEE));
    GreenSystemMessage(m_session, "-- Offhand: %u", creature_target->getVirtualItemEntry(OFFHAND));
    GreenSystemMessage(m_session, "-- Ranged: %u", creature_target->getVirtualItemEntry(RANGED));
#else
    GreenSystemMessage(m_session, "-- Melee: %u", creature_target->getVirtualItemSlotId(MELEE));
    GreenSystemMessage(m_session, "-- Offhand: %u", creature_target->getVirtualItemSlotId(OFFHAND));
    GreenSystemMessage(m_session, "-- Ranged: %u", creature_target->getVirtualItemSlotId(RANGED));
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // transport
    if (auto transporter = creature_target->GetTransport())
    {
        SystemMessage(m_session, "Creature is on Transporter!");
#if VERSION_STRING < Cata
        if (creature_target->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT))
            SystemMessage(m_session, "Creature has MovementFlag MOVEFLAG_TRANSPORT");
        else
            SystemMessage(m_session, "!!!!!!!!! NO MovementFlag MOVEFLAG_TRANSPORT !!!!!!!!!!!!");
#endif
    }

    if (sScriptMgr.has_creature_script(creature_target->getEntry()))
        SystemMessage(m_session, "Creature has C++/LUA script");
    else
        SystemMessage(m_session, "Creature doesn't have C++/LUA script");

    if (sScriptMgr.has_creature_gossip(creature_target->getEntry()))
        SystemMessage(m_session, "Creature has C++/LUA gossip script");
    else
        SystemMessage(m_session, "Creature doesn't have C++/LUA gossip script");

    RedSystemMessage(m_session, "EntryID: %d", creature_target->getEntry());
    RedSystemMessage(m_session, "SpawnID: %d", creature_target->GetSQL_id());

    return true;
}

//.npc listagent
bool ChatHandler::HandleNpcListAIAgentCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto result = WorldDatabase.Query("SELECT * FROM ai_agents where entry=%u", creature_target->getEntry());
    if (result == nullptr)
    {
        RedSystemMessage(m_session, "Selected Creature %s (%u) has no entries in ai_agents table!", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
        return true;
    }
    SystemMessage(m_session, "Agent list for Creature %s (%u)", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    do
    {
        Field* fields = result->Fetch();
        SystemMessage(m_session, "-- agent: %u | spellId: %u | event: %u | chance: %u | maxcount: %u", fields[1].asUint32(), fields[5].asUint32(), fields[2].asUint32(), fields[3].asUint32(), fields[4].asUint32());
    } while (result->NextRow());

    return true;
}

//.npc listloot
bool ChatHandler::HandleNpcListLootCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto loot_result = WorldDatabase.Query("SELECT itemid, normal10percentchance, heroic10percentchance, normal25percentchance, heroic25percentchance, mincount, maxcount FROM loot_creatures WHERE entryid=%u;", creature_target->getEntry());
    if (loot_result != nullptr)
    {
        uint8_t numFound = 0;

        uint32_t minQuality = 0;
        if (*args)
            minQuality = std::stoul(args);

        SystemMessage(m_session, "Listing loot for Creature %s (%u)", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());

        do
        {
            Field* field = loot_result->Fetch();

            auto item_proto = sMySQLStore.getItemProperties(field[0].asUint32());
            if (item_proto == nullptr || item_proto->Quality < minQuality)
                continue;

            RedSystemMessage(m_session, "ItemID: %u %s", item_proto->ItemId, sMySQLStore.getItemLinkByProto(item_proto, m_session->language).c_str());
            SystemMessage(m_session, "-- N10 (%3.2lf) N25 (%3.2lf) H10 (%3.2lf) H25 (%3.2lf) min/max (%u/%u)", field[1].asFloat(), field[3].asFloat(), field[2].asFloat(), field[4].asFloat(), field[5].asUint32(), field[6].asUint32());

            ++numFound;
        } while (loot_result->NextRow() && (numFound <= 25));

        if (numFound > 25)
        {
            RedSystemMessage(m_session, "More than 25 results found. Use .npc listloot <min quality> to increase the results.");
        }
        else
        {
            SystemMessage(m_session, "%hhu results found.", numFound);
        }
    }
    else
    {
        RedSystemMessage(m_session, "No loot in loot_creatures table for %s (%u).", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->getEntry());
    }
    return true;
}

//.npc stopfollow
bool ChatHandler::HandleNpcStopFollowCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    creature_target->getMovementManager()->remove(FOLLOW_MOTION_TYPE);

    sGMLog.writefromsession(m_session, "cancelled npc follow command on %s, sqlid %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);
    return true;
}

//.npc respawn
bool ChatHandler::HandleNpcRespawnCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (creature_target->isCreature() && creature_target->getDeathState() == CORPSE && creature_target->spawnid != 0)
    {
        sEventMgr.RemoveEvents(creature_target, EVENT_CREATURE_RESPAWN);

        BlueSystemMessage(m_session, "Respawning Creature: `%s` with entry: %u on map: %u spawnid: %u", creature_target->GetCreatureProperties()->Name.c_str(),
            creature_target->getEntry(), creature_target->getWorldMap()->getBaseMap()->getMapId(), creature_target->spawnid);
        sGMLog.writefromsession(m_session, "respawned Creature: `%s` with entry: %u on map: %u sqlid: %u", creature_target->GetCreatureProperties()->Name.c_str(),
            creature_target->getEntry(), creature_target->getWorldMap()->getBaseMap()->getMapId(), creature_target->spawnid);

        /*
        if (creature_target->getWorldMap()->pInstance != nullptr)
        {
            creature_target->getWorldMap()->pInstance->m_killedNpcs.erase(creature_target->getSpawnId());
            creature_target->getWorldMap()->pInstance->m_killedNpcs.erase(creature_target->getEntry());
        }*/

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

    creature_target->getMovementManager()->moveTargetedHome();

    sGMLog.writefromsession(m_session, "returned NPC %s (%u)", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);

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

    creature_target->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, args);

    return true;
}

//.npc select
bool ChatHandler::HandleNpcSelectCommand(const char* /*args*/, WorldSession* m_session)
{
    Creature* near_creature = nullptr;
    float dist = 999999.0f;
    float dist2;

    auto player = m_session->GetPlayer();
    for (const auto& itr : player->getInRangeObjectsSet())
    {
        if (itr && (dist2 = player->GetDistance2dSq(itr)) < dist && (itr)->isCreature())
        {
            near_creature = static_cast<Creature*>(itr);
            dist = dist2;
        }
    }

    if (near_creature == nullptr)
    {
        RedSystemMessage(m_session, "No inrange creatures found.");
        return true;
    }

    player->setTargetGuid(near_creature->getGuid());
    std::stringstream message;
    message << "Nearest Creature " << near_creature->GetCreatureProperties()->Name << " spawnID: " << near_creature->spawnid << " GUID: " << near_creature->getGuid() << " selected";
    SystemMessage(m_session, message.str().c_str());
    return true;
}

//.npc spawn
bool ChatHandler::HandleNpcSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32_t entry = std::stoul(args);
    if (entry == 0)
        return false;

    auto creature_properties = sMySQLStore.getCreatureProperties(entry);
    if (creature_properties == nullptr)
    {
        RedSystemMessage(m_session, "Creature with entry %u is not a valid entry (no properties information in database)", entry);
        return true;
    }

    auto creature_spawn = new MySQLStructure::CreatureSpawn;
    uint8_t gender = creature_properties->generateRandomDisplayIdAndReturnGender(&creature_spawn->displayid);
    creature_spawn->entry = entry;
    creature_spawn->id = sObjectMgr.generateCreatureSpawnId();
    creature_spawn->movetype = 0;
    creature_spawn->x = m_session->GetPlayer()->GetPositionX();
    creature_spawn->y = m_session->GetPlayer()->GetPositionY();
    creature_spawn->z = m_session->GetPlayer()->GetPositionZ();
    creature_spawn->o = m_session->GetPlayer()->GetOrientation();
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
    creature_spawn->phase = m_session->GetPlayer()->GetPhase();
    creature_spawn->waypoint_id = 0;

    if (auto creature = m_session->GetPlayer()->getWorldMap()->createCreature(entry))
    {
        creature->Load(creature_spawn, 0, nullptr);
        creature->m_loadedFromDB = true;
        creature->PushToWorld(m_session->GetPlayer()->getWorldMap());

        // Add to map
        uint32_t x = m_session->GetPlayer()->getWorldMap()->getPosX(m_session->GetPlayer()->GetPositionX());
        uint32_t y = m_session->GetPlayer()->getWorldMap()->getPosY(m_session->GetPlayer()->GetPositionY());
        m_session->GetPlayer()->getWorldMap()->getBaseMap()->getSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(creature_spawn);
        MapCell* map_cell = m_session->GetPlayer()->getWorldMap()->getCell(x, y);
        if (map_cell != nullptr)
            map_cell->setLoaded();

        creature->SaveToDB();

        BlueSystemMessage(m_session, "Spawned a creature `%s` with entry %u at %f %f %f on map %u", creature_properties->Name.c_str(),
            entry, creature_spawn->x, creature_spawn->y, creature_spawn->z, m_session->GetPlayer()->GetMapId());
        sGMLog.writefromsession(m_session, "spawned a %s at %u %f %f %f", creature_properties->Name.c_str(), m_session->GetPlayer()->GetMapId(),
            creature_spawn->x, creature_spawn->y, creature_spawn->z);
    }
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

    creature_target->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, args);

    return true;
}

// Zyres: following commands are for units
//.npc possess
bool ChatHandler::HandlePossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);
    if (unit_target != nullptr)
    {
        if (unit_target->isPet() || unit_target->getCreatedByGuid() != 0)
        {
            RedSystemMessage(m_session, "You can not possess a pet!");
            return false;
        }
        if (unit_target->isPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            BlueSystemMessage(m_session, "Player %s selected.", player->getName().c_str());
            sGMLog.writefromsession(m_session, "used possess command on PLAYER %s", player->getName().c_str());
        }
        else if (unit_target->isCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            BlueSystemMessage(m_session, "Creature %s selected.", creature->GetCreatureProperties()->Name.c_str());
            sGMLog.writefromsession(m_session, "used possess command on Creature %s spawn_id %u", creature->GetCreatureProperties()->Name.c_str(), creature->GetSQL_id());
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->possess(unit_target);

    return true;
}

//.npc vendoradditem
bool ChatHandler::HandleNpcVendorAddItemCommand(const char* args, WorldSession* m_session)
{
#if VERSION_STRING < Cata
    char* pitem = strtok(const_cast<char*>(args), " ");
    if (!pitem)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (wowGuid.getRawGuid() == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* selected_creature = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    if (selected_creature == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32_t item = atoi(pitem);
    int amount = -1;

    char* pamount = strtok(nullptr, " ");
    if (pamount)
        amount = atoi(pamount);

    if (amount == -1)
    {
        SystemMessage(m_session, "You need to specify an amount.");
        return true;
    }

    uint32_t costid = 0;
    char* pcostid = strtok(nullptr, " ");
    if (pcostid)
        costid = atoi(pcostid);

    auto item_extended_cost = (costid > 0) ? sItemExtendedCostStore.lookupEntry(costid) : nullptr;
    if (costid > 0 && sItemExtendedCostStore.lookupEntry(costid) == nullptr)
    {
        SystemMessage(m_session, "You've entered invalid extended cost id.");
        return true;
    }

    ItemProperties const* tmpItem = sMySQLStore.getItemProperties(item);
    if (tmpItem)
    {
        WorldDatabase.Execute("INSERT INTO vendors VALUES (%u, %u, %u, 0, 0, %u", selected_creature->getEntry(), item, amount, costid);

        selected_creature->AddVendorItem(item, amount, item_extended_cost);

        if (costid > 0)
            BlueSystemMessage(m_session, "Item %u (%s) added to vendorlist with extended cost %u.", item, tmpItem->Name.c_str(), costid);
        else
            BlueSystemMessage(m_session, "Item %u (%s) added to vendorlist.", item, tmpItem->Name.c_str());
    }
    else
    {
        RedSystemMessage(m_session, "Item %u not found in database", item);
    }

    sGMLog.writefromsession(m_session, "added item %u to vendor %u", item, selected_creature->getEntry());
#else
    char* pitem = strtok((char*)args, " ");
    if (!pitem)
        return false;

    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }
#endif
    return true;
}

//.npc vendorremoveitem
bool ChatHandler::HandleNpcVendorRemoveItemCommand(const char* args, WorldSession* m_session)
{
    char* iguid = strtok((char*)args, " ");
    if (!iguid)
        return false;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());
    if (wowGuid.getRawGuid() == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* selected_creature = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    if (selected_creature == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32_t itemguid = atoi(iguid);
    int slot = selected_creature->GetSlotByItemId(itemguid);
    if (slot != -1)
    {
        uint32_t creatureId = selected_creature->getEntry();

        WorldDatabase.Execute("DELETE FROM vendors WHERE entry = %u AND item = %u", creatureId, itemguid);

        selected_creature->RemoveVendorItem(itemguid);
        ItemProperties const* tmpItem = sMySQLStore.getItemProperties(itemguid);
        if (tmpItem)
        {
            BlueSystemMessage(m_session, "Item %u (%s) deleted from list.", itemguid, tmpItem->Name.c_str());
        }
        else
        {
            BlueSystemMessage(m_session, "Item %u deleted from list.", itemguid);
        }
        sGMLog.writefromsession(m_session, "removed item %u from vendor %u", itemguid, creatureId);
    }
    else
    {
        RedSystemMessage(m_session, "Item %u not found in vendorlist.", itemguid);
    }

    return true;
}

//.npc unpossess
bool ChatHandler::HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit_target = GetSelectedUnit(m_session);

    if (unit_target != nullptr)
    {
        if (unit_target->isPlayer())
        {
            auto player = static_cast<Player*>(unit_target);
            BlueSystemMessage(m_session, "Player %s is no longer possessed by you.", player->getName().c_str());
        }
        else if (unit_target->isCreature())
        {
            auto creature = static_cast<Creature*>(unit_target);
            BlueSystemMessage(m_session, "Creature %s is no longer possessed by you.", creature->GetCreatureProperties()->Name.c_str());
        }
    }
    else
    {
        RedSystemMessage(m_session, "You must select a Player/Creature.");
        return false;
    }

    m_session->GetPlayer()->unPossess();

    return true;
}

//.npc showtimers
bool ChatHandler::HandleNpcShowTimersCommand(const char* /*args*/, WorldSession* m_session)
{
    Creature* creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (CreatureAIScript* creatureScript = creature_target->GetScript())
        creatureScript->displayCreatureTimerList(m_session->GetPlayer());

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .npc set commands
//.npc set canfly
bool ChatHandler::HandleNpcSetCanFlyCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (creature_target->IsFlying())
    {
        creature_target->setMoveCanFly(false);

        if (creature_target->m_spawn != nullptr)
            WorldDatabase.Execute("UPDATE %s SET CanFly = 1 WHERE id = %u AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), creature_target->spawnid, VERSION_STRING, VERSION_STRING);

        GreenSystemMessage(m_session, "CanFly permanent set from 0 to 1 for Creature %s (%u).", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc CanFly for creature_spawns ID: %u [%s] from 0 to 1", creature_target->spawnid, creature_target->GetCreatureProperties()->Name.c_str());
    }
    else
    {
        creature_target->setMoveCanFly(true);

        if (creature_target->m_spawn != nullptr)
            WorldDatabase.Execute("UPDATE %s SET CanFly = 0 WHERE id = %u AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), creature_target->spawnid, VERSION_STRING, VERSION_STRING);

        GreenSystemMessage(m_session, "CanFly permanent set from 1 to 0 for Creature %s (%u).", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);
        sGMLog.writefromsession(m_session, "changed npc CanFly for creature_spawns ID: %u [%s] from 1 to 0", creature_target->spawnid, creature_target->GetCreatureProperties()->Name.c_str());
    }
    return true;
}

//.npc set equip
bool ChatHandler::HandleNpcSetEquipCommand(const char* args, WorldSession* m_session)
{
    uint8_t equipment_slot;
    uint32_t item_id;

    if (sscanf(args, "%hhu %u", &equipment_slot, &item_id) != 2)
    {
        RedSystemMessage(m_session, "Command must be in format: .npc add equipment <slot> <item_id>.");
        RedSystemMessage(m_session, "Slots: (0)melee, (1)offhand, (2)ranged");
        return true;
    }

    Creature* creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    auto item_entry = sItemStore.lookupEntry(item_id);
    if (item_entry == nullptr)
    {
        RedSystemMessage(m_session, "Item ID: %u is not a valid item!", item_id);
        return true;
    }

#if VERSION_STRING < WotLK
    const auto previousValue = creature_target->getVirtualItemEntry(equipment_slot);
#else
    const auto previousValue = creature_target->getVirtualItemSlotId(equipment_slot);
#endif


    switch (equipment_slot)
    {
        case MELEE:
        {
            if (creature_target->m_spawn != nullptr)
                creature_target->m_spawn->Item1SlotEntry = item_id;
            GreenSystemMessage(m_session, "Melee slot successfull changed from %u to %u for Creature %s", previousValue, item_id, creature_target->GetCreatureProperties()->Name.c_str());
            sGMLog.writefromsession(m_session, "changed melee slot from %u to %u for creature spawn %u", previousValue, item_id, creature_target->spawnid);
            break;
        }
        case OFFHAND:
        {
            if (creature_target->m_spawn != nullptr)
                creature_target->m_spawn->Item2SlotEntry = item_id;
            GreenSystemMessage(m_session, "Offhand slot successfull changed from %u to %u for Creature %s", previousValue, item_id, creature_target->GetCreatureProperties()->Name.c_str());
            sGMLog.writefromsession(m_session, "changed offhand slot from %u to %u for creature spawn %u", previousValue, item_id, creature_target->spawnid);
            break;
        }
        case RANGED:
        {
            if (creature_target->m_spawn != nullptr)
                creature_target->m_spawn->Item3SlotEntry = item_id;
            GreenSystemMessage(m_session, "Ranged slot successfull changed from %u to %u for Creature %s", previousValue, item_id, creature_target->GetCreatureProperties()->Name.c_str());
            sGMLog.writefromsession(m_session, "changed ranged slot from %u to %u for creature spawn %u", previousValue, item_id, creature_target->spawnid);
            break;
        }
        default:
        {
            RedSystemMessage(m_session, "Slot: %u is not a valid slot! Use: (0)melee, (1)offhand, (2)ranged.", equipment_slot);
            return true;
        }
    }

    creature_target->setVirtualItemSlotId(equipment_slot, item_id);
    creature_target->SaveToDB();
    return true;
}

//.npc set emote
bool ChatHandler::HandleNpcSetEmoteCommand(const char* args, WorldSession* m_session)
{
    uint32_t emote;

    if (sscanf(args, "%u", &emote) != 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .npc set emote <emote>.");
        RedSystemMessage(m_session, "Use the following format to save the emote: .npc set emote <emote> 1.");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t old_emote = creature_target->getEmoteState();
    creature_target->setEmoteState(emote);

    if (creature_target->m_spawn != nullptr)
        WorldDatabase.Execute("UPDATE %s SET emote_state = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), emote, creature_target->spawnid, VERSION_STRING, VERSION_STRING);

    GreenSystemMessage(m_session, "Emote permanent set from %u to %u for spawn ID: %u.", old_emote, emote, creature_target->spawnid);
    sGMLog.writefromsession(m_session, "changed npc emote of %s ID: %u from %u to %u", creature_target->spawnid, creature_target->GetCreatureProperties()->Name.c_str(), old_emote, emote);
    return true;
}

//.npc set formationmaster
bool ChatHandler::HandleNpcSetFormationMasterCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    m_session->GetPlayer()->m_formationMaster = creature_target;
    BlueSystemMessage(m_session, "Formation Master set to %s spawn ID: %u.", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid);
    return true;
}

//.npc set formationslave
bool ChatHandler::HandleNpcSetFormationSlaveCommand(const char* /*args*/, WorldSession* /*m_session*/)
{
    return true;
}

//.npc set formationclear
bool ChatHandler::HandleNpcSetFormationClearCommand(const char* /*args*/, WorldSession* /*m_session*/)
{
    return true;
}

//.npc set flags
bool ChatHandler::HandleNpcSetFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32_t npc_flags;
    if (sscanf(args, "%u", &npc_flags) < 1)
    {
        RedSystemMessage(m_session, "You need to define the flag value!");
        RedSystemMessage(m_session, ".npc set flags <npc_flag>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return false;

    uint32_t old_npc_flags = creature_target->getNpcFlags();
    creature_target->addNpcFlags(npc_flags);

    if (creature_target->m_spawn != nullptr)
        WorldDatabase.Execute("UPDATE %s SET flags = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), npc_flags, creature_target->spawnid, VERSION_STRING, VERSION_STRING);

    GreenSystemMessage(m_session, "Flags changed in spawns table from %u to %u for spawn ID: %u. You may need to clean your client cache.", old_npc_flags, npc_flags, creature_target->spawnid);
    sGMLog.writefromsession(m_session, "changed npc flags of %s ID: %u from %u to %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid, old_npc_flags, npc_flags);

    return true;
}

//.npc set phase
bool ChatHandler::HandleNpcSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32_t npc_phase;

    if (sscanf(args, "%u", &npc_phase) < 1)
    {
        RedSystemMessage(m_session, "You need to define the phase!");
        RedSystemMessage(m_session, ".npc set phase <npc_phase>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return false;

    uint32_t old_npc_phase = creature_target->GetPhase();
    creature_target->setPhase(PHASE_SET, npc_phase);

    if (creature_target->m_spawn != nullptr)
        WorldDatabase.Execute("UPDATE %s SET phase = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), npc_phase, creature_target->spawnid, VERSION_STRING, VERSION_STRING);


    GreenSystemMessage(m_session, "Phase changed in spawns table from %u to %u for spawn ID: %u.", old_npc_phase, npc_phase, creature_target->spawnid);
    sGMLog.writefromsession(m_session, "changed npc phase of %s ID: %u from %u to %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid, old_npc_phase, npc_phase);

    return true;
}

//.npc set standstate
bool ChatHandler::HandleNpcSetStandstateCommand(const char* args, WorldSession* m_session)
{
    uint8_t standstate;

    if (sscanf(args, "%hhu", &standstate) < 1)
    {
        RedSystemMessage(m_session, "You must specify a standstate value.");
        RedSystemMessage(m_session, ".npc set standstate <standstate>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint8_t old_standstate = creature_target->getStandState();
    creature_target->setStandState(standstate);

    if (creature_target->m_spawn != nullptr)
        WorldDatabase.Execute("UPDATE %s SET standstate = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u", creature_target->m_spawn->origine.c_str(), standstate, creature_target->spawnid, VERSION_STRING, VERSION_STRING);

    GreenSystemMessage(m_session, "Standstate changed in spawns table from %u to %u for spawn ID: %u.", old_standstate, standstate, creature_target->spawnid);
    sGMLog.writefromsession(m_session, "changed npc standstate of %s ID: %u from %u to %u", creature_target->GetCreatureProperties()->Name.c_str(), creature_target->spawnid, old_standstate, standstate);
    
    return true;
}

//.npc set entry
bool ChatHandler::HandleNpcChangeEntry(const char* args, WorldSession* m_session)
{
    uint32_t entry;

    if (sscanf(args, "%u", &entry) < 1)
    {
        RedSystemMessage(m_session, "You must specify a entry value.");
        RedSystemMessage(m_session, ".npc set entry <entry>");
        return true;
    }

    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    uint32_t old_entry = creature_target->getEntry();
    creature_target->updateEntry(entry);

    GreenSystemMessage(m_session, "CreatureEntry temporarily set from %u to %u for spawn ID: %u.", old_entry, entry, creature_target->spawnid);

    return true;
}
