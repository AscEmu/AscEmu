/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CREATURE_DEFINES_HPP
#define CREATURE_DEFINES_HPP


#include "CommonTypes.hpp"
#include "Storage/DBC/DBCStores.h"
#include "Storage/DBC/DBCStructures.hpp"
#if VERSION_STRING == Cata
#include "Storage/DB2/DB2Structures.h"
#endif
#include "Units/UnitDefines.hpp"

#include "Spell/Definitions/School.h"

#include <ctime>

#include "Util.hpp"

struct AI_Spell;

// #define MAX_CREATURE_LOOT 8 Zyres 2015/12/30 unused
// #define MAX_PET_SPELL 4 Zyres 2015/12/30 unused

const uint8 creatureMaxProtoSpells = 8;
const uint32 creatureMaxInventoryItems = 150;

const time_t vendorItemsUpdate = 3600000;

enum creatureguardtype
{
    GUARDTYPE_NONE,
    GUARDTYPE_CITY,
    GUARDTYPE_NEUTRAL
};

struct CreatureItem
{
    uint32 itemid;
    uint32 amount;                              /// stack amount.
    uint32 available_amount;
    uint32 max_amount;
    uint32 incrtime;
#if VERSION_STRING != Cata
    DBC::Structures::ItemExtendedCostEntry const* extended_cost;
#else
    DB2::Structures::ItemExtendedCostEntry const* extended_cost;
#endif
};

enum CreatureAISpellFlags
{
    CREATURE_AI_FLAG_NONE               = 0x00,
    CREATURE_AI_FLAG_RANDOMCAST         = 0x01,
    CREATURE_AI_FLAG_CASTOUTOFCOMBAT    = 0x02,
    CREATURE_AI_FLAG_PLAYERGCD          = 0x04
};

enum VendorRestrictionFlag
{
    RESTRICTION_CHECK_ALL           = 0x00,     /// this checks for all possible values in table
    RESTRICTION_CHECK_MOUNT_VENDOR  = 0x01      /// this one check for race, if race dont match checks for reputation
};

struct spawn_timed_emotes
{
    uint8 type;             //1 standstate, 2 emotestate, 3 emoteoneshot
    uint32 value;           //get yar list elsewhere
    char* msg;              //maybe we wish to say smething while changing emote state
    uint8 msg_type;         //yell ? say ?
    uint8 msg_lang;         //yell ? say ?
    uint32 expire_after;    //going to nex faze in
};
typedef std::list<spawn_timed_emotes*> TimedEmoteList;


enum MONSTER_SAY_EVENTS
{
    MONSTER_SAY_EVENT_ENTER_COMBAT      = 0,
    MONSTER_SAY_EVENT_RANDOM_WAYPOINT   = 1,
    MONSTER_SAY_EVENT_CALL_HELP         = 2,
    MONSTER_SAY_EVENT_ON_COMBAT_STOP    = 3,
    MONSTER_SAY_EVENT_ON_DAMAGE_TAKEN   = 4,
    MONSTER_SAY_EVENT_ON_DIED           = 5,
    NUM_MONSTER_SAY_EVENTS
};

struct CreatureProperties
{
    uint32 Id;
    uint32 killcredit[2];
    uint32 Male_DisplayID;
    uint32 Female_DisplayID;
    uint32 Male_DisplayID2;
    uint32 Female_DisplayID2;
    std::string Name;
    std::string SubName;
    std::string info_str;
    uint32 typeFlags;
    uint32 Type;
    uint32 Family;
    uint32 Rank;
    uint32 Encounter;
    float baseAttackMod;
    float rangeAttackMod;
    uint8  Leader;
    uint32 MinLevel;
    uint32 MaxLevel;
    uint32 Faction;
    uint32 MinHealth;
    uint32 MaxHealth;
    uint32 Mana;
    float Scale;
    uint32 NPCFLags;
    uint32 AttackTime;
    uint32 attackSchool;
    float MinDamage;
    float MaxDamage;
    uint32 CanRanged;
    uint32 RangedAttackTime;
    float RangedMinDamage;
    float RangedMaxDamage;
    uint32 RespawnTime;
    uint32 Resistances[SCHOOL_COUNT];
    float CombatReach;
    float BoundingRadius;
    std::string aura_string;
    bool isBoss;
    uint32 money;
    uint32 invisibility_type;
    float walk_speed;       /// base movement
    float run_speed;        /// most of the time mobs use this
    float fly_speed;
    uint32 extra_a9_flags;
    uint32 AISpells[creatureMaxProtoSpells];
    uint32 AISpellsFlags;
    uint32 modImmunities;
    bool isTrainingDummy;
    uint32 guardtype;
    uint32 summonguard;
    uint32 spelldataid;
    uint32 vehicleid;
    bool rooted;
    uint32 QuestItems[6];
    uint32 waypointid;

    std::string lowercase_name;

    uint8 GetGenderAndCreateRandomDisplayID(uint32* des) const
    {
        uint32 models[] = { Male_DisplayID, Male_DisplayID2, Female_DisplayID, Female_DisplayID2 };
        if (!models[0] && !models[1] && !models[2] && !models[3])
        {
            // All models are invalid.
            LogNotice("CreatureSpawn : All model IDs are invalid for creature %u", Id);
            return 0;
        }

        while (true)
        {
            uint32 res = Util::getRandomUInt(3);
            if (models[res])
            {
                *des = models[res];
                return res < 2 ? 0 : 1;
            }
        }
    }

    uint32 GetRandomModelId() const
    {
        uint8 counter = 0;
        uint32 model_ids[4];

        if (Male_DisplayID)
            model_ids[counter++] = Male_DisplayID;
        if (Male_DisplayID2)
            model_ids[counter++] = Male_DisplayID2;
        if (Female_DisplayID)
            model_ids[counter++] = Female_DisplayID;
        if (Female_DisplayID2)
            model_ids[counter++] = Female_DisplayID2;

        if (counter > 0)
            return model_ids[Util::getRandomUInt(0, counter - 1)];

        LogError("CreatureProperties : No random display_id found for entry %u", Id);
        return 0;
    }

    //itemslots
    uint32 itemslot_1;
    uint32 itemslot_2;
    uint32 itemslot_3;

    // AI Stuff
    bool m_canRangedAttack;
    bool m_canFlee;
    float m_fleeHealth;
    uint32 m_fleeDuration;
    bool m_canCallForHelp;
    float m_callForHelpHealth;

    std::set<uint32> start_auras;
    std::vector<uint32> castable_spells;
    std::list<AI_Spell*> spells;
};

enum UNIT_TYPE
{
    UNIT_TYPE_NONE              = 0,
    UNIT_TYPE_BEAST             = 1,
    UNIT_TYPE_DRAGONKIN         = 2,
    UNIT_TYPE_DEMON             = 3,
    UNIT_TYPE_ELEMENTAL         = 4,
    UNIT_TYPE_GIANT             = 5,
    UNIT_TYPE_UNDEAD            = 6,
    UNIT_TYPE_HUMANOID          = 7,
    UNIT_TYPE_CRITTER           = 8,
    UNIT_TYPE_MECHANICAL        = 9,
    UNIT_TYPE_MISC              = 10,
    UNIT_TYPE_TOTEM             = 11,
    UNIT_TYPE_NONCOMBAT_PET     = 12,
    UNIT_TYPE_GAS_CLOUD         = 13,
    UNIT_TYPE_NUM               = 14
};

enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,
    UNIT_NPC_FLAG_UNK1                  = 0x00000004,
    UNIT_NPC_FLAG_UNK2                  = 0x00000008,
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,
    UNIT_NPC_FLAG_TRAINER_CLASS         = 0x00000020,
    UNIT_NPC_FLAG_TRAINER_PROF          = 0x00000040,
    UNIT_NPC_FLAG_VENDOR                = 0x00000080,
    UNIT_NPC_FLAG_VENDOR_AMMO           = 0x00000100,
    UNIT_NPC_FLAG_VENDOR_FOOD           = 0x00000200,
    UNIT_NPC_FLAG_VENDOR_POISON         = 0x00000400,
    UNIT_NPC_FLAG_VENDOR_REAGENT        = 0x00000800,
    UNIT_NPC_FLAG_ARMORER               = 0x00001000,
    UNIT_NPC_FLAG_TAXIVENDOR            = 0x00002000,
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00004000,
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00008000,
    UNIT_NPC_FLAG_INNKEEPER             = 0x00010000,
    UNIT_NPC_FLAG_BANKER                = 0x00020000,
    UNIT_NPC_FLAG_ARENACHARTER          = 0x00040000,
    UNIT_NPC_FLAG_TABARDCHANGER         = 0x00080000,
    UNIT_NPC_FLAG_BATTLEFIELDPERSON     = 0x00100000,
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00200000,
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00400000,
    UNIT_NPC_FLAG_GUILD_BANK            = 0x00800000,
    UNIT_NPC_FLAG_SPELLCLICK            = 0x01000000,
    UNIT_NPC_FLAG_PLAYER_VEHICLE        = 0x02000000,
    UNIT_NPC_FLAG_MAILBOX               = 0x04000000,
    UNIT_NPC_FLAG_DISABLE_REGEN         = 0x08000000
};

enum CreatureFlag1
{
    CREATURE_FLAG1_TAMEABLE         = 0x00000001,       // creature is tameable by hunter
    CREATURE_FLAG1_GHOST            = 0x00000002,       // creature are also visible for dead players.
    CREATURE_FLAG1_BOSS             = 0x00000004,       // creature is a boss "??"
    CREATURE_FLAG1_UNK3             = 0x00000008,
    CREATURE_FLAG1_UNK4             = 0x00000010,
    CREATURE_FLAG1_UNK5             = 0x00000020,
    CREATURE_FLAG1_UNK6             = 0x00000040,
    CREATURE_FLAG1S_DEAD_INTERACT   = 0x00000080,       // player can interact with the creature while creature is dead.
    CREATURE_FLAG1_HERBLOOT         = 0x00000100,       // lootable by herbalist
    CREATURE_FLAG1_MININGLOOT       = 0x00000200,       // lootable by miner
    CREATURE_FLAG1_DONT_LOG_DEATH   = 0x00000400,       // death event will not show up in combat log
    CREATURE_FLAG1_FIGHT_MOUNTED    = 0x00000800,       // creature keeps mounted by entering combat
    CREATURE_FLAG1_AID_PLAYERS      = 0x00001000,
    CREATURE_FLAG1_UNK13            = 0x00002000,
    CREATURE_FLAG1_UNK14            = 0x00004000,
    CREATURE_FLAG1_ENGINEERLOOT     = 0x00008000,
    CREATURE_FLAG1_EXOTIC           = 0x00010000,
    CREATURE_FLAG1_UNK17            = 0x00020000,
    CREATURE_FLAG1_UNK18            = 0x00040000,
    CREATURE_FLAG1S_PROJECT_COLL    = 0x00080000,
    CREATURE_FLAG1_UNK20            = 0x00100000,
    CREATURE_FLAG1_UNK21            = 0x00200000,
    CREATURE_FLAG1_UNK22            = 0x00400000,
    CREATURE_FLAG1_UNK23            = 0x00800000,
    CREATURE_FLAG1_UNK24            = 0x01000000,
    CREATURE_FLAG1_UNK25            = 0x02000000,
    CREATURE_FLAG1_PARTY_MEMBER     = 0x04000000,
    CREATURE_FLAG1_UNK27            = 0x08000000,
    CREATURE_FLAG1_UNK28            = 0x10000000,
    CREATURE_FLAG1_UNK29            = 0x20000000,
    CREATURE_FLAG1_UNK30            = 0x40000000,
    CREATURE_FLAG1_UNK31            = 0x80000000
};

enum FAMILY
{
    FAMILY_WOLF             = 1,
    FAMILY_CAT              = 2,
    FAMILY_SPIDER           = 3,
    FAMILY_BEAR             = 4,
    FAMILY_BOAR             = 5,
    FAMILY_CROCOLISK        = 6,
    FAMILY_CARRION_BIRD     = 7,
    FAMILY_CRAB             = 8,
    FAMILY_GORILLA          = 9,
    FAMILY_RAPTOR           = 11,
    FAMILY_TALLSTRIDER      = 12,
    FAMILY_FELHUNTER        = 15,
    FAMILY_VOIDWALKER       = 16,
    FAMILY_SUCCUBUS         = 17,
    FAMILY_DOOMGUARD        = 19,
    FAMILY_SCORPID          = 20,
    FAMILY_TURTLE           = 21,
    FAMILY_IMP              = 23,
    FAMILY_BAT              = 24,
    FAMILY_HYENA            = 25,
    FAMILY_OWL              = 26,
    FAMILY_WIND_SERPENT     = 27,
    FAMILY_REMOTE_CONTROL   = 28,
    FAMILY_FELGUARD         = 29,
    FAMILY_DRAGONHAWK       = 30,
    FAMILY_RAVAGER          = 31,
    FAMILY_WARP_STALKER     = 32,
    FAMILY_SPOREBAT         = 33,
    FAMILY_NETHER_RAY       = 34,
    FAMILY_SERPENT          = 35,
    FAMILY_MOTH             = 37,
    FAMILY_CHIMAERA         = 38,
    FAMILY_DEVILSAUR        = 39,
    FAMILY_GHOUL            = 40,   /// DK's minion
    FAMILY_SILITHID         = 41,
    FAMILY_WORM             = 42,
    FAMILY_RHINO            = 43,
    FAMILY_WASP             = 44,
    FAMILY_CORE_HOUND       = 45,
    FAMILY_SPIRIT_BEAST     = 46
};

enum ELITE
{
    ELITE_NORMAL    = 0,
    ELITE_ELITE     = 1,
    ELITE_RAREELITE = 2,
    ELITE_WORLDBOSS = 3,
    ELITE_RARE      = 4
    // ELITE_UNKNOWN = 5
};

// THIS IS NOT SAME AS DEATH STATE IN Unit.h
enum CREATURE_DEATH_STATE
{
    CREATURE_STATE_ALIVE        = 0,    /// no special death state
    CREATURE_STATE_APPEAR_DEAD  = 1,    /// these creatures are actually alive but appears as dead for client
    CREATURE_STATE_DEAD         = 2     /// these creatures are dead
};

enum CREATURE_TYPE
{
    CREATURE_TYPE_NONE      = 0,
    CREATURE_TYPE_GUARDIAN  = 1
};

//\Todo not used?
//struct PetSpellCooldown
//{
//    uint32 spellId;
//    int32 cooldown;
//};

#endif // _CREATURE_DEFINES_HPP
