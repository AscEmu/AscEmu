/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#include "StdAfx.h"

#include <iostream>
#include <sstream>

#include "Management/QuestLogEntry.hpp"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Server/Packets/Opcode.h"
#include "Objects/DynamicObject.h"
#include "AuthCodes.h"
#include "VMapFactory.h"
#include "Server/Packets/Handlers/HonorHandler.h"
#include "Storage/WorldStrings.h"
#include "Management/TaxiMgr.h"
#include "Management/WeatherMgr.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/Battleground/Battleground.h"
#include "Management/ArenaTeam.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/Warden/SpeedDetector.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/Spec.h"
#include "Spell/SpellHelpers.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Pet.h"

#if VERSION_STRING == Cata
#include "GameCata/Management/GuildMgr.h"
#endif

using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;
using ascemu::World::Spell::Helpers::spellModFlatFloatValue;
using ascemu::World::Spell::Helpers::spellModPercentageFloatValue;

UpdateMask Player::m_visibleUpdateMask;

#define COLLISION_INDOOR_CHECK_INTERVAL 1000

static const uint8 glyphMask[81/*DBC_PLAYER_LEVEL_CAP+1*/] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                         // lvl 0-14, no glyphs
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,                                         // lvl 15-29, 1 Minor 1 Major
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,      // lvl 30-49, 1 Minor 2 Major
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,      // lvl 50-69, 2 Minor 2 Major
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31,                                              // lvl 70-79, 3 Minor 2 Major
    63                                                                                   // lvl 80, 3 Minor 3 Major
};

static const float crit_to_dodge[MAX_PLAYER_CLASSES] =
{
    0.0f,      // empty
    1.1f,      // Warrior
    1.0f,      // Paladin
    1.6f,      // Hunter
    2.0f,      // Rogue
    1.0f,      // Priest
    1.0f,      // DK?
    1.0f,      // Shaman
    1.0f,      // Mage
    1.0f,      // Warlock
    0.0f,      // empty
    1.7f       // Druid
};

bool Player::Teleport(const LocationVector& vec, MapMgr* map)
{
    if (map == nullptr)
        return false;

    if (map->GetPlayer(this->GetLowGUID()))
    {
        this->SetPosition(vec);
    }
    else
    {
        if (map->GetMapId() == 530 && !this->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_01))
        {
            return false;
        }

        if (map->GetMapId() == 571 && !this->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            return false;
        }

        this->SafeTeleport(map, vec);
    }

    return true;
}

Player::Player(uint32 guid)
    :
    disableAppear(false),
    disableSummon(false),
    taxi_model_id(0),
    lastNode(0),
    m_taxi_ride_time(0),
    m_taxi_pos_x(0),
    m_taxi_pos_y(0),
    m_taxi_pos_z(0),
    m_onTaxi(false),
    m_questSharer(0),
    pctReputationMod(0),
    DuelingWith(nullptr),
    m_lootGuid(0),
    m_currentLoot(0),
    bShouldHaveLootableOnCorpse(false),
    offhand_dmg_mod(0.5),
    m_isMoving(false),
    moving(false),
    strafing(false),
    jumping(false),
    m_isGmInvisible(false),
    SpellHasteRatingBonus(1.0f),
    m_furorChance(0),
    //WayPoint
    waypointunit(nullptr),
    m_nextSave(Util::getMSTime() + worldConfig.getIntRate(INTRATE_SAVE)),
    m_lifetapbonus(0),
    m_bUnlimitedBreath(false),
    m_UnderwaterTime(180000),
    m_UnderwaterState(0),
    m_AllowAreaTriggerPort(true),
    // Battleground
    m_bg(nullptr),
    m_bgEntryPointMap(0),
    m_bgEntryPointX(0),
    m_bgEntryPointY(0),
    m_bgEntryPointZ(0),
    m_bgEntryPointO(0),
    m_bgEntryPointInstance(0),
    m_bgHasFlag(false),
    m_bgIsQueued(false),
    m_bgQueueType(0),
    m_bgQueueInstanceId(0),
    m_bgIsRbg(false),
    m_bgIsRbgWon(false),
    m_AutoShotAttackTimer(0),
    DetectedRange(0),
    PctIgnoreRegenModifier(0.0f),
    m_retainedrage(0),
    SoulStone(0),
    SoulStoneReceiver(0),
    misdirectionTarget(0),
    bReincarnation(false),
    ignoreShapeShiftChecks(false),
    ignoreAuraStateCheck(false),
    m_GM_SelectedGO(0),
    m_ShapeShifted(0),
    m_MountSpellId(0),
    bHasBindDialogOpen(false),
    TrackingSpell(0),
    m_CurrentCharm(0),
    // gm stuff
    //m_invincible(false),
    TaxiCheat(false),
    CooldownCheat(false),
    CastTimeCheat(false),
    GodModeCheat(false),
    PowerCheat(false),
    FlyCheat(false),
    AuraStackCheat(false),
    ItemStackCheat(false),
    TriggerpassCheat(false),
    SaveAllChangesCommand(false),
    m_Autojoin(false),
    m_AutoAddMem(false),
    m_UnderwaterMaxTime(180000),
    m_UnderwaterLastDmg(Util::getMSTime()),
    m_resurrectHealth(0),
    m_resurrectMana(0),
    m_resurrectInstanceID(0),
    m_resurrectMapId(0),
    m_mailBox(guid),
    m_finishingmovesdodge(false),
    mUpdateCount(0),
    mCreationCount(0),
    mOutOfRangeIdCount(0),
    //Trade
#if VERSION_STRING == Cata
    m_TradeData(nullptr),
#else
    mTradeTarget(0),
#endif
    info(nullptr), // Playercreate info
    m_AttackMsgTimer(0),
    //PVP
    //PvPTimeoutEnabled(false),
    m_banned(false),
    m_Summons(),
    m_PetNumberMax(0),
    //DK
    m_invitersGuid(0),
    //Bind position
    m_bind_pos_x(0),
    m_bind_pos_y(0),
    m_bind_pos_z(0),
    m_bind_mapid(0),
    m_bind_zoneid(0),
    //Duel
    m_duelCountdownTimer(0),
    m_duelStatus(0),
    m_duelState(DUEL_STATE_FINISHED),        // finished
#if VERSION_STRING == Cata
    m_GuildIdInvited(0),
#endif
    // Rest
    m_timeLogoff(0),
    m_isResting(0),
    m_restState(0),
    m_restAmount(0),
    // Attack related variables
    m_blockfromspellPCT(0),
    m_critfromspell(0),
    m_spellcritfromspell(0),
    m_hitfromspell(0),
    m_healthfromspell(0),
    m_manafromspell(0),
    m_healthfromitems(0),
    m_manafromitems(0),
    //FIX for shit like shirt etc
    armor_proficiency(1),
    //FIX for professions
    weapon_proficiency(0x4000), //2^14
    m_talentresettimes(0),
    m_status(0),
    m_curTarget(0),
    m_curSelection(0),
    m_targetIcon(0),
    m_session(nullptr),
    m_GroupInviter(0),
    m_SummonedObject(nullptr),
    myCorpseLocation()
#if VERSION_STRING > TBC
        ,
    m_achievementMgr(this)
#endif
{
    m_cache = new PlayerCache;
    m_cache->SetUInt32Value(CACHE_PLAYER_LOWGUID, guid);
    objmgr.AddPlayerCache(guid, m_cache);
    int i, j;

    m_H_regenTimer = 0;
    m_P_regenTimer = 0;
    //////////////////////////////////////////////////////////////////////////
    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;
    m_valuesCount = PLAYER_END;
    //////////////////////////////////////////////////////////////////////////

    ///\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;

    memset(m_uint32Values, 0, (PLAYER_END)* sizeof(uint32));
    m_updateMask.SetCount(PLAYER_END);
    setUInt32Value(OBJECT_FIELD_TYPE, TYPE_PLAYER | TYPE_UNIT | TYPE_OBJECT);
    SetLowGUID(guid);
    m_wowGuid.Init(GetGUID());
#if VERSION_STRING != Classic
    setUInt32Value(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

#if VERSION_STRING > TBC
    setFloatValue(PLAYER_RUNE_REGEN_1, 0.100000f);
    setFloatValue(PLAYER_RUNE_REGEN_1 + 1, 0.100000f);
    setFloatValue(PLAYER_RUNE_REGEN_1 + 2, 0.100000f);
    setFloatValue(PLAYER_RUNE_REGEN_1 + 3, 0.100000f);
#endif

    for (i = 0; i < 28; i++)
    {
        MechanicDurationPctMod[i] = 0;
    }

    //Trade
#if VERSION_STRING != Cata
    ResetTradeVariables();
#endif

    //Tutorials
    for (i = 0; i < 8; i++)
        m_Tutorials[i] = 0x00;

    m_playedtime[0] = 0;
    m_playedtime[1] = 0;
    m_playedtime[2] = (uint32)UNIXTIME;

    for (i = 0; i < 7; i++)
    {
        FlatResistanceModifierPos[i] = 0;
        FlatResistanceModifierNeg[i] = 0;
        BaseResistanceModPctPos[i] = 0;
        BaseResistanceModPctNeg[i] = 0;
        ResistanceModPctPos[i] = 0;
        ResistanceModPctNeg[i] = 0;
        SpellDelayResist[i] = 0;
        m_casted_amount[i] = 0;
    }

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 7; j++)
        {
            SpellHealDoneByAttribute[i][j] = 0;
        }

        FlatStatModPos[i] = 0;
        FlatStatModNeg[i] = 0;
        StatModPctPos[i] = 0;
        StatModPctNeg[i] = 0;
        TotalStatModPctPos[i] = 0;
        TotalStatModPctNeg[i] = 0;
    }

    for (i = 0; i < 12; i++)
    {
        IncreaseDamageByType[i] = 0;
        IncreaseDamageByTypePCT[i] = 0;
        IncreaseCricticalByTypePCT[i] = 0;
    }

    bCreationBuffer.reserve(40000);
    bUpdateBuffer.reserve(30000);//ought to be > than enough ;)
    mOutOfRangeIds.reserve(1000);


    bProcessPending = false;
    for (i = 0; i < 25; ++i)
        m_questlog[i] = nullptr;

    m_ItemInterface = new ItemInterface(this);

    SDetector = new SpeedCheatDetector;

    cannibalize = false;
    mAvengingWrath = true;
    m_AreaID = 0;
    cannibalizeCount = 0;
    rageFromDamageDealt = 0;
    rageFromDamageTaken = 0;

    m_honorToday = 0;
    m_honorYesterday = 0;
    m_honorPoints = 0;
    m_killsToday = 0;
    m_killsYesterday = 0;
    m_killsLifetime = 0;
    m_honorless = 0;
    m_lastSeenWeather = 0;
    m_attacking = false;

    myCorpseInstanceId = 0;
    bCorpseCreateable = true;
    blinked = false;
    m_explorationTimer = Util::getMSTime();
    linkTarget = nullptr;
    m_pvpTimer = 0;
    m_globalCooldown = 0;
    m_lastHonorResetTime = 0;
    tutorialsDirty = true;
    m_TeleportState = 1;
    m_beingPushed = false;
    for (i = 0; i < NUM_CHARTER_TYPES; ++i)
        m_charters[i] = nullptr;
    for (i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
        m_arenaTeams[i] = nullptr;
    flying_aura = 0;
    resend_speed = false;
    login_flags = LOGIN_NO_FLAG;
    DualWield2H = false;
    iInstanceType = 0;
    m_RaidDifficulty = 0;
    m_XpGain = true;
    resettalents = false;
    memset(reputationByListId, 0, sizeof(FactionReputation*) * 128);

    m_comboTarget = 0;
    m_comboPoints = 0;

    SetAttackPowerMultiplier(0.0f);
    SetRangedAttackPowerMultiplier(0.0f);

    m_resist_critical[0] = m_resist_critical[1] = 0;
    m_castFilterEnabled = false;

    for (i = 0; i < 3; i++)
    {
        m_attack_speed[i] = 1.0f;
        m_castFilter[i] = 0;
    }

    for (i = 0; i < SCHOOL_COUNT; i++)
        m_resist_hit_spell[i] = 0;

    m_resist_hit[MOD_MELEE] = 0.0f;
    m_resist_hit[MOD_RANGED] = 0.0f;

    m_maxTalentPoints = 0; //VLack: 3 Aspire values initialized
    m_talentActiveSpec = 0;
    m_talentSpecsCount = 1;
#if VERSION_STRING == Cata
    m_FirstTalentTreeLock = 0;

    m_GuildId = 0;
#endif

    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        m_specs[s].talents.clear();
        memset(m_specs[s].glyphs, 0, GLYPHS_COUNT * sizeof(uint16));
        memset(m_specs[s].mActions, 0, PLAYER_ACTION_BUTTON_SIZE);
    }

    m_drunkTimer = 0;
    m_drunk = 0;

    ok_to_remove = false;
    m_modphyscritdmgPCT = 0;
    m_RootedCritChanceBonus = 0;
    m_IncreaseDmgSnaredSlowed = 0;
    m_ModInterrMRegenPCT = 0;
    m_ModInterrMRegen = 0;
    m_RegenManaOnSpellResist = 0;
    m_rap_mod_pct = 0;//only initialized to 0: why?
    m_modblockabsorbvalue = 0;
    m_modblockvaluefromspells = 0;
    m_summoner = m_summonInstanceId = m_summonMapId = 0;
    m_spellcomboPoints = 0;
    m_pendingBattleground = nullptr;
    m_deathVision = false;
    m_resurrecter = 0;
    m_retainComboPoints = false;
    last_heal_spell = nullptr;
    m_playerInfo = nullptr;
    m_sentTeleportPosition.ChangeCoords(999999.0f, 999999.0f, 999999.0f);
    m_speedChangeCounter = 1;
    memset(&m_bgScore, 0, sizeof(BGScore));
    m_arenateaminviteguid = 0;
    m_arenaPoints = 0;
    m_honorRolloverTime = 0;
    hearth_of_wild_pct = 0;
    raidgrouponlysent = false;
    loot.gold = 0;
    m_areaSpiritHealer_guid = 0;
    m_CurrentTaxiPath = nullptr;

    m_fallDisabledUntil = 0;
    m_lfgMatch = nullptr;
    m_lfgInviterGuid = 0;
    m_indoorCheckTimer = 0;
    m_taxiMapChangeNode = 0;
    this->OnLogin();

    m_requiresNoAmmo = false;
    m_KickDelay = 0;
    m_passOnLoot = false;
    m_changingMaps = true;
    m_outStealthDamageBonusPct = m_outStealthDamageBonusPeriod = m_outStealthDamageBonusTimer = 0;
    m_flyhackCheckTimer = 0;

    m_skills.clear();
    m_wratings.clear();
    m_taxiPaths.clear();
    m_removequests.clear();
    m_finishedQuests.clear();
    m_finishedDailies.clear();
    quest_spells.clear();
    quest_mobs.clear();

    m_onStrikeSpells.clear();
    m_onStrikeSpellDmg.clear();
    mSpellOverrideMap.clear();
    mSpells.clear();
    mDeletedSpells.clear();
    mShapeShiftSpells.clear();
    m_Pets.clear();
    m_itemsets.clear();
    m_reputation.clear();
    m_channels.clear();
    m_visibleObjects.clear();
    m_forcedReactions.clear();

    loginauras.clear();
    damagedone.clear();
    tocritchance.clear();
    m_visibleFarsightObjects.clear();
    SummonSpells.clear();
    PetSpells.clear();
    delayedPackets.clear();
    _splineMap.clear();

    m_lastPotionId = 0;
    for (i = 0; i < NUM_COOLDOWN_TYPES; i++)
    {
        m_cooldownMap[i].clear();
    }
    //    m_achievement_points = 0;

    ChampioningFactionID = 0;
    mountvehicleid = 0;

    camControle = false;

    isTurning = false;
    m_bgTeam = 0;
    myRace = nullptr;
    myClass = nullptr;
    OnlineTime = (uint32)UNIXTIME;
    lvlinfo = nullptr;
    load_health = 0;
    load_mana = 0;
    m_noseLevel = 0;
    m_StableSlotCount = 0;
    m_team = 0;
    m_timeSyncCounter = 0;
    m_timeSyncTimer = 0;
    m_timeSyncClient = 0;
    m_timeSyncServer = 0;
    m_roles = 0;
    GroupUpdateFlags = GROUP_UPDATE_FLAG_NONE;
    m_FirstLogin = false;

    // command
    go_last_x_rotation = 0.0f;
    go_last_y_rotation = 0.0f;
}

void Player::OnLogin()
{}

Player::~Player()
{
    objmgr.RemovePlayerCache(GetLowGUID());
    m_cache->DecRef();
    m_cache = nullptr;

    if (!ok_to_remove)
    {
        LOG_ERROR("Player deleted from non-logoutplayer!");
        printStackTrace(); // Win32 Debug

        objmgr.RemovePlayer(this);
    }

    if (m_session)
    {
        m_session->SetPlayer(nullptr);
        if (!ok_to_remove)
            m_session->Disconnect();
    }

    Player* pTarget;
#if VERSION_STRING != Cata
    if (mTradeTarget != 0)
    {
        pTarget = GetTradeTarget();
        if (pTarget)
            pTarget->mTradeTarget = 0;
    }
#else
    if (m_TradeData != nullptr)
        cancelTrade(false);
#endif

    pTarget = objmgr.GetPlayer(GetInviter());
    if (pTarget)
        pTarget->SetInviter(0);

    DismissActivePets();
#if VERSION_STRING != Cata
    mTradeTarget = 0;
#endif

    if (DuelingWith != nullptr)
        DuelingWith->DuelingWith = nullptr;
    DuelingWith = nullptr;

    ARCEMU_ASSERT(!IsInWorld());

    // delete m_talenttree

    for (uint8 i = 0; i < 25; ++i)
    {
        if (m_questlog[i] != nullptr)
        {
            delete m_questlog[i];
            m_questlog[i] = nullptr;
        }
    }

    for (SplineMap::iterator itr = _splineMap.begin(); itr != _splineMap.end(); ++itr)
        delete itr->second;
    _splineMap.clear();

    delete m_ItemInterface;
    m_ItemInterface = nullptr;

    for (ReputationMap::iterator itr = m_reputation.begin(); itr != m_reputation.end(); ++itr)
        delete itr->second;
    m_reputation.clear();

    if (m_playerInfo)
        m_playerInfo->m_loggedInPlayer = nullptr;

    delete SDetector;
    SDetector = nullptr;

    while (delayedPackets.size())
    {
        WorldPacket* pck = delayedPackets.next();
        delete pck;
    }

    /*std::map<uint32,AchievementVal*>::iterator itr;
    for (itr=m_achievements.begin();itr!=m_achievements.end();itr++)
    delete itr->second;*/

    std::map< uint32, PlayerPet* >::iterator itr = m_Pets.begin();
    for (; itr != m_Pets.end(); ++itr)
        delete itr->second;
    m_Pets.clear();

    RemoveGarbageItems();
}

uint32 GetSpellForLanguage(uint32 SkillID)
{
    switch (SkillID)
    {
        case SKILL_LANG_COMMON:
            return 668;
            break;

        case SKILL_LANG_ORCISH:
            return 669;
            break;

        case SKILL_LANG_TAURAHE:
            return 670;
            break;

        case SKILL_LANG_DARNASSIAN:
            return 671;
            break;

        case SKILL_LANG_DWARVEN:
            return 672;
            break;

        case SKILL_LANG_THALASSIAN:
            return 813;
            break;

        case SKILL_LANG_DRACONIC:
            return 814;
            break;

        case SKILL_LANG_DEMON_TONGUE:
            return 815;
            break;

        case SKILL_LANG_TITAN:
            return 816;
            break;

        case SKILL_LANG_OLD_TONGUE:
            return 817;
            break;

        case SKILL_LANG_GNOMISH:
            return 7430;
            break;

        case SKILL_LANG_TROLL:
            return 7341;
            break;

        case SKILL_LANG_GUTTERSPEAK:
            return 17737;
            break;

        case SKILL_LANG_DRAENEI:
            return 29932;
            break;
#if VERSION_STRING == Cata
        case SKILL_LANG_GOBLIN:
            return 69269;
            break;

        case SKILL_LANG_GILNEAN:
            return 69270;
            break;
#endif
    }

    return 0;
}

void Player::CharChange_Looks(uint64 GUID, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair)
{
    QueryResult* result = CharacterDatabase.Query("SELECT bytes2 FROM `characters` WHERE guid = '%u'", (uint32)GUID);
    if (!result)
        return;

    Field* fields = result->Fetch();

    uint32 player_bytes2 = fields[0].GetUInt32();
    player_bytes2 &= ~0xFF;
    player_bytes2 |= facialHair;

    CharacterDatabase.Execute("UPDATE `characters` SET gender = '%u', bytes = '%u', bytes2 = '%u' WHERE guid = '%u'", gender, skin | (face << 8) | (hairStyle << 16) | (hairColor << 24), player_bytes2, (uint32)GUID);

    delete result;
}

//Begining of code for phase two of character customization (Race/Faction) Change.
void Player::CharChange_Language(uint64 GUID, uint8 race)
{
#if VERSION_STRING != Cata
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u', '%u');", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH), GetSpellForLanguage(SKILL_LANG_TAURAHE), GetSpellForLanguage(SKILL_LANG_TROLL), GetSpellForLanguage(SKILL_LANG_GUTTERSPEAK), GetSpellForLanguage(SKILL_LANG_THALASSIAN), GetSpellForLanguage(SKILL_LANG_COMMON), GetSpellForLanguage(SKILL_LANG_DARNASSIAN), GetSpellForLanguage(SKILL_LANG_DRAENEI), GetSpellForLanguage(SKILL_LANG_DWARVEN), GetSpellForLanguage(SKILL_LANG_GNOMISH));
#else
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u', '%u');", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH), GetSpellForLanguage(SKILL_LANG_TAURAHE), GetSpellForLanguage(SKILL_LANG_TROLL), GetSpellForLanguage(SKILL_LANG_GUTTERSPEAK), GetSpellForLanguage(SKILL_LANG_THALASSIAN), GetSpellForLanguage(SKILL_LANG_COMMON), GetSpellForLanguage(SKILL_LANG_DARNASSIAN), GetSpellForLanguage(SKILL_LANG_DRAENEI), GetSpellForLanguage(SKILL_LANG_DWARVEN), GetSpellForLanguage(SKILL_LANG_GNOMISH), GetSpellForLanguage(SKILL_LANG_GILNEAN), GetSpellForLanguage(SKILL_LANG_GOBLIN));
#endif
    switch (race)
    {
        case RACE_DWARF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_DWARVEN));
            break;
        case RACE_DRAENEI:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_DRAENEI));
            break;
        case RACE_GNOME:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_GNOMISH));
            break;
        case RACE_NIGHTELF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_DARNASSIAN));
            break;
        case RACE_UNDEAD:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_GUTTERSPEAK));
            break;
        case RACE_TAUREN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_TAURAHE));
            break;
        case RACE_TROLL:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_TROLL));
            break;
        case RACE_BLOODELF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_THALASSIAN));
            break;
#if VERSION_STRING == Cata
        case RACE_WORGEN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_GILNEAN));
            break;
        case RACE_GOBLIN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_GOBLIN));
            break;
#endif
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Create data from client to create a new character
/// \param p_newChar
//////////////////////////////////////////////////////////////////////////////////////////
bool Player::Create(WorldPacket& data)
{
    uint8 race;
    uint8 class_;
    uint8 gender;
    uint8 skin;
    uint8 face;
    uint8 hairStyle;
    uint8 hairColor;
    uint8 facialHair;
    uint8 outfitId;

    // unpack data into member variables
    data >> m_name;

    // correct capitalization
    Util::CapitalizeString(m_name);

    data >> race;
    data >> class_;
    data >> gender;
    data >> skin;
    data >> face;
    data >> hairStyle;
    data >> hairColor;
    data >> facialHair;
    data >> outfitId;

    info = sMySQLStore.getPlayerCreateInfo(race, class_);
    if (info == nullptr)
    {
        // info not found... disconnect
        //sCheatLog.writefromsession(m_session, "tried to create invalid player with race %u and class %u", race, class_);
        m_session->Disconnect();
        if (class_ == DEATHKNIGHT)
            LOG_ERROR("Account Name: %s tried to create a deathknight, however your playercreateinfo table does not support this class, please update your database.", m_session->GetAccountName().c_str());
        else
            LOG_ERROR("Account Name: %s tried to create an invalid character with race %u and class %u, if this is intended please update your playercreateinfo table inside your database.", m_session->GetAccountName().c_str(), race, class_);
        return false;
    }

    // check that the account can create TBC characters, if we're making some
    if (race >= RACE_BLOODELF && !(m_session->_accountFlags & ACCOUNT_FLAG_XPACK_01))
    {
        m_session->Disconnect();
        return false;
    }

    // check that the account can create deathknights, if we're making one
    if (class_ == DEATHKNIGHT && !(m_session->_accountFlags & ACCOUNT_FLAG_XPACK_02))
    {
        m_session->Disconnect();
        return false;
    }

    m_mapId = info->mapId;
    SetZoneId(info->zoneId);
    m_position.ChangeCoords(info->positionX, info->positionY, info->positionZ, info->orientation);
    m_bind_pos_x = info->positionX;
    m_bind_pos_y = info->positionY;
    m_bind_pos_z = info->positionZ;
    m_bind_mapid = info->mapId;
    m_bind_zoneid = info->zoneId;
    m_isResting = 0;
    m_restAmount = 0;
    m_restState = 0;

    // set race dbc
    myRace = sChrRacesStore.LookupEntry(race);
    myClass = sChrClassesStore.LookupEntry(class_);
    if (!myRace || !myClass)
    {
        // information not found
        sCheatLog.writefromsession(m_session, "tried to create invalid player with race %u and class %u, dbc info not found", race, class_);
        m_session->Disconnect();
        return false;
    }

    if (myRace->team_id == 7)
        m_team = 0;
    else
        m_team = 1;
    m_cache->SetUInt32Value(CACHE_PLAYER_INITIALTEAM, m_team);

    uint8 powertype = static_cast<uint8>(myClass->power_type);

    // Automatically add the race's taxi hub to the character's taximask at creation time (1 << (taxi_node_id-1))
    // this is defined in table playercreateinfo, field taximask
    memcpy(m_taximask, info->taximask, sizeof(m_taximask));

    // Set Starting stats for char
    //SetScale( ((race==RACE_TAUREN)?1.3f:1.0f));
    SetScale(1.0f);
    SetHealth(info->health);
    SetPower(POWER_TYPE_MANA, info->mana);
    SetPower(POWER_TYPE_RAGE, 0);
    SetPower(POWER_TYPE_FOCUS, info->focus); // focus
    SetPower(POWER_TYPE_ENERGY, info->energy);
    SetPower(POWER_TYPE_RUNES, 8);

    SetMaxHealth(info->health);
    SetMaxPower(POWER_TYPE_MANA, info->mana);
    SetMaxPower(POWER_TYPE_RAGE, info->rage);
    SetMaxPower(POWER_TYPE_FOCUS, info->focus);
    SetMaxPower(POWER_TYPE_ENERGY, info->energy);
    SetMaxPower(POWER_TYPE_RUNES, 8);
    SetMaxPower(POWER_TYPE_RUNIC_POWER, 1000);

    //THIS IS NEEDED
    SetBaseHealth(info->health);
    SetBaseMana(info->mana);
    SetFaction(info->factiontemplate);

    if (class_ == DEATHKNIGHT)
        SetTalentPointsForAllSpec(worldConfig.player.deathKnightStartTalentPoints); // Default is 0 in case you do not want to modify it
    else
        SetTalentPointsForAllSpec(0);
    if (class_ != DEATHKNIGHT || worldConfig.player.playerStartingLevel > 55)
    {
        setLevel(worldConfig.player.playerStartingLevel);
        if (worldConfig.player.playerStartingLevel >= 10 && class_ != DEATHKNIGHT)
            SetTalentPointsForAllSpec(worldConfig.player.playerStartingLevel - 9);
    }
    else
    {
        setLevel(55);
        SetNextLevelXp(148200);
    }

#if VERSION_STRING > TBC
    UpdateGlyphs();
#endif

    SetPrimaryProfessionPoints(worldConfig.player.maxProfessions);

    setRace(race);
    setClass(class_);
    setGender(gender);
    SetPowerType(powertype);

#if VERSION_STRING != Cata
    setUInt32Value(UNIT_FIELD_BYTES_2, (U_FIELD_BYTES_FLAG_PVP << 8));
#else
    setByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

    if (class_ == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

#if VERSION_STRING != Cata
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
#endif

    SetStat(STAT_STRENGTH, info->strength);
    SetStat(STAT_AGILITY, info->ability);
    SetStat(STAT_STAMINA, info->stamina);
    SetStat(STAT_INTELLECT, info->intellect);
    SetStat(STAT_SPIRIT, info->spirit);
    SetBoundingRadius(0.388999998569489f);
    SetCombatReach(1.5f);
    if (race != RACE_BLOODELF)
    {
        SetDisplayId(info->displayId + gender);
        SetNativeDisplayId(info->displayId + gender);
    }
    else
    {
        SetDisplayId(info->displayId - gender);
        SetNativeDisplayId(info->displayId - gender);
    }
    EventModelChange();
    //SetMinDamage(info->mindmg);
    //SetMaxDamage(info->maxdmg);
    SetAttackPower(info->attackpower);
#if VERSION_STRING != Cata
    setUInt32Value(PLAYER_BYTES, ((skin) | (face << 8) | (hairStyle << 16) | (hairColor << 24)));
    //PLAYER_BYTES_2                               GM ON/OFF     BANKBAGSLOTS   RESTEDSTATE
    setUInt32Value(PLAYER_BYTES_2, (facialHair /*| (0xEE << 8)*/ | (0x02 << 24)));//no bank slot by default!

    //PLAYER_BYTES_3                           DRUNKENSTATE                 PVPRANK
    setUInt32Value(PLAYER_BYTES_3, ((gender) | (0x00 << 8) | (0x00 << 16) | (GetPVPRank() << 24)));
#else
    // Set Byte Values
    setByteValue(PLAYER_BYTES, 0, skin);
    setByteValue(PLAYER_BYTES, 1, face);
    setByteValue(PLAYER_BYTES, 2, hairStyle);
    setByteValue(PLAYER_BYTES, 3, hairColor);

    //Set Byte2 Values
    setByteValue(PLAYER_BYTES_2, 0, facialHair);
    //SetByte(PLAYER_BYTES_2, 1, 0);  // unknown
    //SetByte(PLAYER_BYTES_2, 2, 0);  // unknown
    setByteValue(PLAYER_BYTES_2, 3, RESTSTATE_NORMAL);

    //Set Byte3 Values
    setByteValue(PLAYER_BYTES_3, 0, gender);
    setByteValue(PLAYER_BYTES_3, 1, 0);  // drunkenstate?
    setByteValue(PLAYER_BYTES_3, 2, 0);  // unknown
    setByteValue(PLAYER_BYTES_3, 3, GetPVPRank());  // pvp rank
#endif
    SetNextLevelXp(400);
    setUInt32Value(PLAYER_FIELD_BYTES, 0x08);
    SetCastSpeedMod(1.0f);
#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_FIELD_MAX_LEVEL, worldConfig.player.playerLevelCap);
#endif

    // Gold Starting Amount
    SetGold(worldConfig.player.startGoldAmount);


    for (uint16 x = 0; x < 7; x++)
        setFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + x, 1.00);

    setUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, 0xEEEEEEEE);

    m_StableSlotCount = 0;
    Item* item;

    for (std::set<uint32>::iterator sp = info->spell_list.begin(); sp != info->spell_list.end(); ++sp)
    {
        mSpells.insert((*sp));
    }

    m_FirstLogin = true;


    for (std::list<CreateInfo_SkillStruct>::const_iterator ss = info->skills.begin(); ss != info->skills.end(); ++ss)
    {
        auto skill_line = sSkillLineStore.LookupEntry(ss->skillid);
        if (skill_line == nullptr)
            continue;

#if VERSION_STRING != Cata
        if (skill_line->type != SKILL_TYPE_LANGUAGE)
            _AddSkillLine(skill_line->id, ss->currentval, ss->maxval);
#else
        _AddSkillLine(skill_line->id, ss->currentval, ss->maxval);
#endif
    }
    _UpdateMaxSkillCounts();
    //Chances depend on stats must be in this order!
    //UpdateStats();
    //UpdateChances();

    _InitialReputation();

    // Add actionbars
    for (std::list<CreateInfo_ActionBarStruct>::const_iterator itr = info->actionbars.begin(); itr != info->actionbars.end(); ++itr)
    {
        setAction(static_cast<uint8>(itr->button), static_cast<uint16>(itr->action), static_cast<uint8>(itr->type), static_cast<uint8>(itr->misc));
    }

    for (std::list<CreateInfo_ItemStruct>::const_iterator is = info->items.begin(); is != info->items.end(); ++is)
    {
        if ((*is).protoid != 0)
        {
            item = objmgr.CreateItem((*is).protoid, this);
            if (item)
            {
                item->SetStackCount((*is).amount);
                if ((*is).slot < INVENTORY_SLOT_BAG_END)
                {
                    if (!GetItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, (*is).slot))
                        item->DeleteMe();
                }
                else
                {
                    if (!GetItemInterface()->AddItemToFreeSlot(item))
                        item->DeleteMe();
                }
            }
        }
    }

    sHookInterface.OnCharacterCreate(this);
    load_health = GetHealth();
    load_mana = GetPower(POWER_TYPE_MANA);
    return true;
}

void Player::Update(unsigned long time_passed)
{
    if (!IsInWorld())
        return;

    Unit::Update(time_passed);
    uint32 mstime = Util::getMSTime();

    RemoveGarbageItems();

    if (m_attacking)
    {
        // Check attack timer.
        if (mstime >= m_attackTimer)
            _EventAttack(false);

        if (m_dualWield && mstime >= m_attackTimer_1)
            _EventAttack(true);
    }

    if (m_AutoShotAttackTimer > 0)
    {
        if (m_AutoShotAttackTimer > time_passed)
            m_AutoShotAttackTimer -= time_passed;
        else
            m_AutoShotAttackTimer = 0;
    }

    // Breathing
    if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        // keep subtracting timer
        if (m_UnderwaterTime)
        {
            // not taking dmg yet
            if (time_passed >= m_UnderwaterTime)
                m_UnderwaterTime = 0;
            else
                m_UnderwaterTime -= time_passed;
        }

        if (!m_UnderwaterTime)
        {
            // check last damage dealt timestamp, and if enough time has elapsed deal damage
            if (mstime >= m_UnderwaterLastDmg)
            {
                uint32 damage = m_uint32Values[UNIT_FIELD_MAXHEALTH] / 10;

                SendEnvironmentalDamageLog(GetGUID(), uint8(DAMAGE_DROWNING), damage);
                DealDamage(this, damage, 0, 0, 0);
                m_UnderwaterLastDmg = mstime + 1000;
            }
        }
    }
    else
    {
        // check if we're not on a full breath timer
        if (m_UnderwaterTime < m_UnderwaterMaxTime)
        {
            // regenning
            m_UnderwaterTime += (time_passed * 10);

            if (m_UnderwaterTime >= m_UnderwaterMaxTime)
            {
                m_UnderwaterTime = m_UnderwaterMaxTime;
                StopMirrorTimer(MIRROR_TYPE_BREATH);
            }
        }
    }

    // Lava Damage
    if (m_UnderwaterState & UNDERWATERSTATE_LAVA)
    {
        // check last damage dealt timestamp, and if enough time has elapsed deal damage
        if (mstime >= m_UnderwaterLastDmg)
        {
            uint32 damage = m_uint32Values[UNIT_FIELD_MAXHEALTH] / 5;

            SendEnvironmentalDamageLog(GetGUID(), uint8(DAMAGE_LAVA), damage);
            DealDamage(this, damage, 0, 0, 0);
            m_UnderwaterLastDmg = mstime + 1000;
        }
    }

    // Autosave
    if (mstime >= m_nextSave)
        SaveToDB(false);

    // Exploration
    if (mstime >= m_explorationTimer)
    {
        _EventExploration();
        m_explorationTimer = mstime + 3000;
    }

    //Autocast Spells in Area
    CastSpellArea();

    if (m_pvpTimer)
    {
        if (time_passed >= m_pvpTimer)
        {
            RemovePvPFlag();
            m_pvpTimer = 0;
        }
        else
            m_pvpTimer -= time_passed;
    }

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (mstime >= m_indoorCheckTimer)
        {
            if (!MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_mapId, m_position.x, m_position.y, m_position.z))
            {
                // this is duplicated check, but some mount auras comes w/o this flag set, maybe due to spellfixes.cpp line:663
                Dismount();

                for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
                {
                    if (m_auras[x] && m_auras[x]->GetSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS)
                        RemoveAura(m_auras[x]);
                }
            }
            m_indoorCheckTimer = mstime + COLLISION_INDOOR_CHECK_INTERVAL;
        }

    }

    if (m_drunk > 0)
    {
        m_drunkTimer += time_passed;

        if (m_drunkTimer > 10000)
            HandleSobering();
    }

    WorldPacket* pending_packet = m_cache->m_pendingPackets.pop();
    while (pending_packet != nullptr)
    {
        SendPacket(pending_packet);
        delete pending_packet;
        pending_packet = m_cache->m_pendingPackets.pop();
    }

    SendUpdateToOutOfRangeGroupMembers();
}

void Player::EventDismount(uint32 money, float x, float y, float z)
{
    ModGold(-(int32)money);

    if (money > 0 && m_fallDisabledUntil < time(nullptr) + 5) m_fallDisabledUntil = time(nullptr) + 5; //VLack: If the ride wasn't free, the player shouldn't die after arrival because of fall damage... So we'll disable it for 5 seconds.

    SetPosition(x, y, z, true);
    if (!m_taxiPaths.size())
        SetTaxiState(false);

    SetTaxiPath(nullptr);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_INTERPOLATE);

    // Save to database on dismount
    SaveToDB(false);

    // If we have multiple "trips" to do, "jump" on the next one :p
    if (m_taxiPaths.size())
    {
        TaxiPath* p = *m_taxiPaths.begin();
        m_taxiPaths.erase(m_taxiPaths.begin());
        TaxiStart(p, taxi_model_id, 0);
    }
}

void Player::_EventAttack(bool offhand)
{
    if (isCastingNonMeleeSpell())
    {
        // Non-channeled
        if (getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr) // this is a channeled spell - ignore the attack event
            return;
        interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
        setAttackTimer(500, offhand);
        return;
    }

    if (IsFeared() || IsStunned())
        return;

    Unit* pVictim = nullptr;
    if (m_curSelection)
        pVictim = GetMapMgr()->GetUnit(m_curSelection);

    //Can't find victim, stop attacking
    if (!pVictim)
    {
        LOG_DETAIL("Player::Update:  No valid current selection to attack, stopping attack");
        setHRegenTimer(5000); //prevent clicking off creature for a quick heal
        EventAttackStop();
        return;
    }

    if (!isAttackable(this, pVictim))
    {
        setHRegenTimer(5000);
        EventAttackStop();
        return;
    }

    if (!canReachWithAttack(pVictim))
    {
        if (m_AttackMsgTimer != 1)
        {
            m_session->OutPacket(SMSG_ATTACKSWING_NOTINRANGE);
            m_AttackMsgTimer = 1;
        }
        setAttackTimer(300, offhand);
    }
    else if (!isInFront(pVictim))
    {
        // We still have to do this one.
        if (m_AttackMsgTimer != 2)
        {
            m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
            m_AttackMsgTimer = 2;
        }
        setAttackTimer(300, offhand);
    }
    else
    {
        m_AttackMsgTimer = 0;

        // Set to weapon time.
        setAttackTimer(0, offhand);

        //pvp timeout reset
        if (pVictim->IsPlayer())
        {
            if (static_cast< Player* >(pVictim)->cannibalize)
            {
                sEventMgr.RemoveEvents(pVictim, EVENT_CANNIBALIZE);
                pVictim->SetEmoteState(EMOTE_ONESHOT_NONE);
                static_cast< Player* >(pVictim)->cannibalize = false;
            }
        }

        if (this->IsStealth())
        {
            RemoveAura(m_stealth);
            SetStealth(0);
        }

        if ((GetOnMeleeSpell() == 0) || offhand)
            Strike(pVictim, (offhand ? OFFHAND : MELEE), nullptr, 0, 0, 0, false, false);
        else
            CastOnMeleeSpell();
    }
}

void Player::_EventCharmAttack()
{
    if (!m_CurrentCharm)
        return;

    Unit* pVictim = nullptr;
    if (!IsInWorld())
    {
        m_CurrentCharm = 0;
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    if (m_curSelection == 0)
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    pVictim = GetMapMgr()->GetUnit(m_curSelection);

    //Can't find victim, stop attacking
    if (!pVictim)
    {
        LOG_ERROR("WORLD: " I64FMT " doesn't exist.", m_curSelection);
        LOG_DETAIL("Player::Update:  No valid current selection to attack, stopping attack");
        this->setHRegenTimer(5000); //prevent clicking off creature for a quick heal
        removeUnitStateFlag(UNIT_STATE_ATTACKING);
        EventAttackStop();
    }
    else
    {
        Unit* currentCharm = GetMapMgr()->GetUnit(m_CurrentCharm);

        if (!currentCharm)
            return;

        if (!currentCharm->canReachWithAttack(pVictim))
        {
            if (m_AttackMsgTimer == 0)
            {
                //m_session->OutPacket(SMSG_ATTACKSWING_NOTINRANGE);
                // 2 sec till next msg.
                m_AttackMsgTimer = 2000;
            }
            // Shorten, so there isnt a delay when the client IS in the right position.
            sEventMgr.ModifyEventTimeLeft(this, EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else if (!currentCharm->isInFront(pVictim))
        {
            if (m_AttackMsgTimer == 0)
            {
                m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
                m_AttackMsgTimer = 2000;        // 2 sec till next msg.
            }
            // Shorten, so there isnt a delay when the client IS in the right position.
            sEventMgr.ModifyEventTimeLeft(this, EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else
        {
            //if (pVictim->GetTypeId() == TYPEID_UNIT)
            //    pVictim->GetAIInterface()->StopMovement(5000);

            //pvp timeout reset
            /*if (pVictim->IsPlayer())
            {
            if (TO<Player*>(pVictim)->DuelingWith == NULL)//Dueling doesn't trigger PVP
            TO<Player*>(pVictim)->PvPTimeoutUpdate(false); //update targets timer

            if (DuelingWith == NULL)//Dueling doesn't trigger PVP
            PvPTimeoutUpdate(false); //update casters timer
            }*/

            if (!currentCharm->GetOnMeleeSpell())
            {
                currentCharm->Strike(pVictim, MELEE, nullptr, 0, 0, 0, false, false);
            }
            else
            {
                SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(currentCharm->GetOnMeleeSpell());
                currentCharm->SetOnMeleeSpell(0);
                Spell* spell = sSpellFactoryMgr.NewSpell(currentCharm, spellInfo, true, nullptr);
                SpellCastTargets targets;
                targets.m_unitTarget = GetSelection();
                spell->prepare(&targets);
                //delete spell;         // deleted automatically, no need to do this.
            }
        }
    }
}

void Player::EventAttackStart()
{
    m_attacking = true;
    Dismount();
}

void Player::EventAttackStop()
{
    if (m_CurrentCharm != 0)
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);

    m_attacking = false;
}

bool Player::HasOverlayUncovered(uint32 overlayID)
{
    auto overlay = sWorldMapOverlayStore.LookupEntry(overlayID);
    if (overlay == nullptr)
        return false;

    if (overlay->areaID && HasAreaExplored(MapManagement::AreaManagement::AreaStorage::GetAreaById(overlay->areaID)))
        return true;
    if (overlay->areaID_2 && HasAreaExplored(MapManagement::AreaManagement::AreaStorage::GetAreaById(overlay->areaID_2)))
        return true;
    if (overlay->areaID_3 && HasAreaExplored(MapManagement::AreaManagement::AreaStorage::GetAreaById(overlay->areaID_3)))
        return true;
    if (overlay->areaID_4 && HasAreaExplored(MapManagement::AreaManagement::AreaStorage::GetAreaById(overlay->areaID_4)))
        return true;

    return false;
}

bool Player::HasAreaExplored(::DBC::Structures::AreaTableEntry const* at)
{
    if (at == nullptr)
        return false;

    uint16_t offset = static_cast<uint16_t>(at->explore_flag / 32);
    offset += PLAYER_EXPLORED_ZONES_1;

    uint32 val = (uint32)(1 << (at->explore_flag % 32));
    uint32 currFields = getUInt32Value(offset);

    return (currFields & val) != 0;
}

void Player::_EventExploration()
{
    if (IsDead())
        return;

    if (!IsInWorld())
        return;

    if (m_position.x > _maxX || m_position.x < _minX || m_position.y > _maxY || m_position.y < _minY)
        return;

    if (GetMapMgr()->GetCellByCoords(GetPositionX(), GetPositionY()) == nullptr)
        return;

    auto at = this->GetArea();
    if (at == nullptr)
        return;

    uint32 AreaId = at->id;

    uint16_t offset = static_cast<uint16_t>(at->explore_flag / 32);
    offset += PLAYER_EXPLORED_ZONES_1;

    uint32 val = (uint32)(1 << (at->explore_flag % 32));
    uint32 currFields = getUInt32Value(offset);

    if (AreaId != m_AreaID)
    {
        m_AreaID = AreaId;
        UpdatePvPArea();

        AddGroupUpdateFlag(GROUP_UPDATE_FULL);

        if (GetGroup())
            GetGroup()->UpdateOutOfRangePlayer(this, true, nullptr);
    }

    // Zone update, this really should update to a parent zone if one exists.
    //  Will show correct location on your character screen, as well zoneid in DB will have correct value
    //  for any web sites that access that data.
    if (at->zone == 0 && m_zoneId != AreaId)
    {
        ZoneUpdate(AreaId);
    }
    else if (at->zone != 0 && m_zoneId != at->zone)
    {
        ZoneUpdate(at->zone);
    }


    if (at->zone != 0 && m_zoneId != at->zone)
        ZoneUpdate(at->zone);

    bool rest_on = false;
    // Check for a restable area
    if (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY)
    {
        // check faction
        if ((at->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at->team == AREAC_HORDE_TERRITORY && IsTeamHorde()))
        {
            rest_on = true;
        }
        else if (at->team != AREAC_ALLIANCE_TERRITORY && at->team != AREAC_HORDE_TERRITORY)
        {
            rest_on = true;
        }
    }
    else
    {
        //second AT check for subzones.
        if (at->zone)
        {
            auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
            if (at2 && (at2->flags & AREA_CITY_AREA || at2->flags & AREA_CITY))
            {
                if ((at2->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && IsTeamHorde()))
                {
                    rest_on = true;
                }
                else if (at2->team != AREAC_ALLIANCE_TERRITORY && at2->team != AREAC_HORDE_TERRITORY)
                {
                    rest_on = true;
                }
            }
        }
    }

    if (rest_on)
    {
        if (!m_isResting)
            ApplyPlayerRestState(true);
    }
    else
    {
        if (m_isResting)
            ApplyPlayerRestState(false);
    }

#if VERSION_STRING != Cata
    if (!(currFields & val) && !GetTaxiState() && !obj_movement_info.transporter_info.guid) //Unexplored Area        // bur: we don't want to explore new areas when on taxi
#else
    if (!(currFields & val) && !GetTaxiState() && obj_movement_info.getTransportGuid().IsEmpty()) //Unexplored Area        // bur: we don't want to explore new areas when on taxi
#endif
    {
        setUInt32Value(offset, (uint32)(currFields | val));

        uint32 explore_xp = at->area_level * 10;
        explore_xp *= float2int32(worldConfig.getFloatRate(RATE_EXPLOREXP));

#if VERSION_STRING > TBC
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA);
#endif
        uint32 maxlevel = GetMaxLevel();

        if (getLevel() < maxlevel && explore_xp > 0)
        {
            SendExploreXP(at->id, explore_xp);
            GiveXP(explore_xp, 0, false);
        }
        else
        {
            SendExploreXP(at->id, 0);
        }
    }
}

void Player::EventDeath()
{
    if (hasUnitStateFlag(UNIT_STATE_ATTACKING))
        EventAttackStop();

    if (m_onTaxi)
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_DISMOUNT);

    if (!IS_INSTANCE(GetMapId()) && !sEventMgr.HasEvent(this, EVENT_PLAYER_FORCED_RESURRECT)) //Should never be true
        sEventMgr.AddEvent(this, &Player::RepopRequestedPlayer, EVENT_PLAYER_FORCED_RESURRECT, forcedResurrectInterval, 1, 0); //in case he forgets to release spirit (afk or something)

    RemoveNegativeAuras();

    SetDrunkValue(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  This function sends the message displaying the purple XP gain for the char
///  It assumes you will send out an UpdateObject packet at a later time.
//////////////////////////////////////////////////////////////////////////////////////////
void Player::GiveXP(uint32 xp, const uint64 & guid, bool allowbonus)
{
    if (xp < 1)
        return;

#if VERSION_STRING == Cata
    //this is new since 403. As we gain XP we also gain XP with our guild
    if (m_playerInfo && m_playerInfo->m_guild)
    {
        uint32 guild_share = xp / 100;

        Guild* guild = sGuildMgr.getGuildById(m_playerInfo->m_guild);

        if (guild)
            guild->giveXP(guild_share, this);
    }
#endif

    // Obviously if Xp gaining is disabled we don't want to gain XP
    if (!m_XpGain)
        return;

    if (getLevel() >= GetMaxLevel())
        return;

    uint32 restxp = xp;

    //add reststate bonus (except for quests)
    if (m_restState == RESTSTATE_RESTED && allowbonus)
    {
        restxp = SubtractRestXP(xp);
        xp += restxp;
    }

    UpdateRestState();
    SendLogXPGain(guid, xp, restxp, guid == 0 ? true : false);

    int32 newxp = GetXp() + xp;
    int32 nextlevelxp = GetXpToLevel();
    uint32 level = getLevel();
    LevelInfo* li;
    bool levelup = false;

    while (newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        li = objmgr.GetLevelInfo(getRace(), getClass(), level);
        if (li == nullptr) return;
        newxp -= nextlevelxp;
        nextlevelxp = sMySQLStore.getPlayerXPForLevel(level);
        levelup = true;

        if (level > 9)
            AddTalentPointsToAllSpec(1);

        if (level >= GetMaxLevel())
            break;
    }

    if (level > GetMaxLevel())
        level = GetMaxLevel();

    if (levelup)
    {
        m_playedtime[0] = 0; //Reset the "Current level played time"

        setLevel(level);
        LevelInfo* oldlevel = lvlinfo;
        lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), level);
        if (lvlinfo == nullptr) return;
        CalculateBaseStats();

        // Generate Level Info Packet and Send to client
        SendLevelupInfo(
            level,
            lvlinfo->HP - oldlevel->HP,
            lvlinfo->Mana - oldlevel->Mana,
            lvlinfo->Stat[0] - oldlevel->Stat[0],
            lvlinfo->Stat[1] - oldlevel->Stat[1],
            lvlinfo->Stat[2] - oldlevel->Stat[2],
            lvlinfo->Stat[3] - oldlevel->Stat[3],
            lvlinfo->Stat[4] - oldlevel->Stat[4]);

        _UpdateMaxSkillCounts();
        UpdateStats();
        //UpdateChances();
#if VERSION_STRING > TBC
        UpdateGlyphs();
#endif

        // ScriptMgr hook for OnPostLevelUp
        sHookInterface.OnPostLevelUp(this);

        // Set stats
        for (uint8 i = 0; i < 5; ++i)
        {
            BaseStats[i] = lvlinfo->Stat[i];
            CalcStat(i);
        }

        // Small chance you die and levelup at the same time, and you enter a weird state.
        if (IsDead())
            ResurrectPlayer();

        //set full hp and mana
        SetHealth(GetMaxHealth());
        SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));

        // if warlock has summoned pet, increase its level too
        if (info->class_ == WARLOCK)
        {
            Pet* summon = GetSummon();
            if ((summon != nullptr) && (summon->IsInWorld()) && (summon->isAlive()))
            {
                summon->modLevel(1);
                summon->ApplyStatsForLevel();
                summon->UpdateSpellList();
            }
        }
#if VERSION_STRING > TBC
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
#endif

        //VLack: 3.1.3, as a final step, send the player's talents, this will set the talent points right too...
        smsg_TalentsInfo(false);
    }

    // Set the update bit
    SetXp(newxp);

}

void Player::smsg_InitialSpells()
{
    PlayerCooldownMap::iterator itr, itr2;

    uint16 spellCount = (uint16)mSpells.size();
    size_t itemCount = m_cooldownMap[0].size() + m_cooldownMap[1].size();
    uint32 mstime = Util::getMSTime();
    size_t pos;

    WorldPacket data(SMSG_INITIAL_SPELLS, 5 + (spellCount * 4) + (itemCount * 4));
    data << uint8(0);
    data << uint16(spellCount); // spell count

    SpellSet::iterator sitr;
    for (sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        ///\todo check out when we should send 0x0 and when we should send 0xeeee this is not slot, values is always eeee or 0, seems to be cooldown
#if VERSION_STRING == TBC
        data << uint16(*sitr);                   // spell id
#else
        data << uint32(*sitr);                   // spell id
#endif
        data << uint16(0x0000);
    }

    pos = data.wpos();
    data << uint16(0);        // placeholder

    itemCount = 0;
    for (itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end();)
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if (itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000)
        {
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr2);
            continue;
        }

#if VERSION_STRING == TBC
        data << uint16(itr2->first);                        // spell id
#else
        data << uint32(itr2->first);                        // spell id
#endif
#if VERSION_STRING == WotLK
        data << uint16(itr2->second.ItemId);                // item id
#else
        data << uint32(itr2->second.ItemId);                // item id
#endif
        data << uint16(0);                                  // spell category
        data << uint32(itr2->second.ExpireTime - mstime);   // cooldown remaining in ms (for spell)
        data << uint32(0);                                  // cooldown remaining in ms (for category)

        ++itemCount;

        LogDebugFlag(LF_OPCODE, "InitialSpells sending spell cooldown for spell %u to %u ms", itr2->first, itr2->second.ExpireTime - mstime);
    }

    for (itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end();)
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if (itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000)
        {
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr2);
            continue;
        }

#if VERSION_STRING == TBC
        data << uint16(itr2->second.SpellId);                // spell id
#else
        data << uint32(itr2->second.SpellId);                // spell id
#endif

        data << uint16(itr2->second.ItemId);                // item id

        data << uint16(itr2->first);                        // spell category
        data << uint32(0);                                // cooldown remaining in ms (for spell)
        data << uint32(itr2->second.ExpireTime - mstime);    // cooldown remaining in ms (for category)

        ++itemCount;

        LogDebugFlag(LF_OPCODE, "InitialSpells sending category cooldown for cat %u to %u ms", itr2->first, itr2->second.ExpireTime - mstime);
    }


    *(uint16*)&data.contents()[pos] = (uint16)itemCount;

    GetSession()->SendPacket(&data);

#if VERSION_STRING != TBC
    uint32 v = 0;
    GetSession()->OutPacket(0x041d, 4, &v);
#endif
    //Log::getSingleton().outDetail("CHARACTER: Sent Initial Spells");
}

#if VERSION_STRING != Cata
void Player::smsg_TalentsInfo(bool SendPetTalents)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_TALENTS_INFO, 1000);
    data << uint8(SendPetTalents ? 1 : 0);
    if (SendPetTalents)
    {
        if (GetSummon())
            GetSummon()->SendTalentsToOwner();
        return;
    }
    else
    {
        //data << uint32(GetTalentPoints(SPEC_PRIMARY)); // Wrong, calculate the amount of talent points per spec
        data << uint32(m_specs[m_talentActiveSpec].GetTP());
        data << uint8(m_talentSpecsCount);
        data << uint8(m_talentActiveSpec);
        for (uint8 s = 0; s < m_talentSpecsCount; s++)
        {
            PlayerSpec spec = m_specs[s];
            data << uint8(spec.talents.size());
            std::map<uint32, uint8>::iterator itr;
            for (itr = spec.talents.begin(); itr != spec.talents.end(); ++itr)
            {
                data << uint32(itr->first);     // TalentId
                data << uint8(itr->second);     // TalentRank
            }

            // Send Glyph info
            data << uint8(GLYPHS_COUNT);
            for (uint8 i = 0; i < GLYPHS_COUNT; i++)
            {
                data << uint16(spec.glyphs[i]);
            }
        }
    }
    GetSession()->SendPacket(&data);
#endif
}
#else
void Player::smsg_TalentsInfo(bool SendPetTalents)
{
    WorldPacket data(SMSG_TALENTS_INFO, 1000);
    data << uint8(SendPetTalents ? 1 : 0);
    if (SendPetTalents)
    {
        if (GetSummon())
            GetSummon()->SendTalentsToOwner();
        return;
    }
    else
    {
        data << uint32(GetCurrentTalentPoints());
        data << uint8(m_talentSpecsCount);
        data << uint8(m_talentActiveSpec);

        if (m_talentSpecsCount)
        {
            if (m_talentSpecsCount > MAX_SPEC_COUNT)
            {
                m_talentSpecsCount = MAX_SPEC_COUNT;
            }

            for (uint8 s = 0; s < m_talentSpecsCount; ++s)
            {
                PlayerSpec spec = m_specs[s];
                data << uint32(m_FirstTalentTreeLock);

                uint8 talentCount = 0;
                size_t pos = data.wpos();
                data << uint8(talentCount);

                uint32 const* talentTabPages = getTalentTabPages(getClass());

                for (uint8 i = 0; i < 3; ++i)
                {
                    uint32 talentTabPage = talentTabPages[i];

                    for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
                    {
                        DBC::Structures::TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);
                        if (!talentInfo)
                        {
                            continue;
                        }

                        if (talentInfo->TalentTree != talentTabPage)
                        {
                            continue;
                        }

                        int8 currrentTalentMaxRank = -1;
                        for (int8 rank = 5 - 1; rank >= 0; --rank)
                        {
                            if (talentInfo->RankID[rank] && spec.HasTalent(talentInfo->RankID[rank], s))
                            {
                                currrentTalentMaxRank = rank;
                                break;
                            }
                        }

                        if (currrentTalentMaxRank < 0)
                        {
                            continue;
                        }

                        data << uint32(talentInfo->TalentID);
                        data << uint8(currrentTalentMaxRank);

                        ++talentCount;
                    }
                }

                data.put<uint8>(pos, talentCount);

                data << uint8(GLYPHS_COUNT);

                for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
                {
                    data << uint16(GetGlyph(s, i));
                }
            }
        }
    }
    GetSession()->SendPacket(&data);
}
#endif

void Player::ActivateSpec(uint8 spec)
{
    if (spec >= MAX_SPEC_COUNT || m_talentActiveSpec >= MAX_SPEC_COUNT)
        return;

    uint8 OldSpec = m_talentActiveSpec;
    m_talentActiveSpec = spec;

#if VERSION_STRING > TBC
    // remove old glyphs
    for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
    {
        auto glyph_properties = sGlyphPropertiesStore.LookupEntry(m_specs[OldSpec].glyphs[i]);
        if (glyph_properties == nullptr)
            continue;

        RemoveAura(glyph_properties->SpellID);
    }
#endif

    // remove old talents
    for (std::map<uint32, uint8>::iterator itr = m_specs[OldSpec].talents.begin(); itr != m_specs[OldSpec].talents.end(); ++itr)
    {
        auto talent_info = sTalentStore.LookupEntry(itr->first);
        if (talent_info == nullptr)
            continue;

        removeSpell(talent_info->RankID[itr->second], true, false, 0);
    }

#if VERSION_STRING > TBC
    // add new glyphs
    for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
    {
        auto glyph_properties = sGlyphPropertiesStore.LookupEntry(m_specs[m_talentActiveSpec].glyphs[i]);
        if (glyph_properties == nullptr)
            continue;

        CastSpell(this, glyph_properties->SpellID, true);
    }
#endif

    //add talents from new spec
    for (std::map<uint32, uint8>::iterator itr = m_specs[m_talentActiveSpec].talents.begin(); itr != m_specs[m_talentActiveSpec].talents.end(); ++itr)
    {
        auto talent_info = sTalentStore.LookupEntry(itr->first);
        if (talent_info == nullptr)
            continue;

        addSpell(talent_info->RankID[itr->second]);
    }
#if VERSION_STRING != Cata
    setUInt32Value(PLAYER_CHARACTER_POINTS1, m_specs[m_talentActiveSpec].GetTP());
#else
    setUInt32Value(PLAYER_CHARACTER_POINTS, m_specs[m_talentActiveSpec].GetTP());
#endif
    smsg_TalentsInfo(false);
}

void PlayerSpec::AddTalent(uint32 talentid, uint8 rankid)
{
    std::map<uint32, uint8>::iterator itr = talents.find(talentid);
    if (itr != talents.end())
        itr->second = rankid;
    else
        talents.insert(std::make_pair(talentid, rankid));
}

void Player::_SavePet(QueryBuffer* buf)
{
    // Remove any existing info
    if (buf == nullptr)
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", GetLowGUID());
    else
        buf->AddQuery("DELETE FROM playerpets WHERE ownerguid = %u", GetLowGUID());

    Pet* summon = GetSummon();
    if (summon && summon->IsInWorld() && summon->GetPetOwner() == this)    // update PlayerPets array with current pet's info
    {
        PlayerPet* pPet = GetPlayerPet(summon->m_PetNumber);
        if (!pPet || pPet->active == false)
            summon->UpdatePetInfo(true);
        else summon->UpdatePetInfo(false);

        if (!summon->Summon)       // is a pet
        {
            // save pet spellz
            PetSpellMap::iterator itr = summon->mSpells.begin();
            uint32 pn = summon->m_PetNumber;
            if (buf == nullptr)
                CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", GetLowGUID(), pn);
            else
                buf->AddQuery("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", GetLowGUID(), pn);

            for (; itr != summon->mSpells.end(); ++itr)
            {
                if (buf == nullptr)
                    CharacterDatabase.Execute("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", GetLowGUID(), pn, itr->first->getId(), itr->second);
                else
                    buf->AddQuery("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", GetLowGUID(), pn, itr->first->getId(), itr->second);
            }
        }
    }

    std::stringstream ss;

    ss.rdbuf()->str("");

    for (std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); ++itr)
    {
        ss.rdbuf()->str("");

        ss << "REPLACE INTO playerpets VALUES('"
            << GetLowGUID() << "','"
            << itr->second->number << "','"
            << itr->second->name << "','"
            << itr->second->entry << "','"
            << itr->second->xp << "','"
            << (itr->second->active ? 1 : 0) + itr->second->stablestate * 10 << "','"
            << itr->second->level << "','"
            << itr->second->actionbar << "','"
            << itr->second->happinessupdate << "','"
            << (long)itr->second->reset_time << "','"
            << itr->second->reset_cost << "','"
            << itr->second->spellid << "','"
            << itr->second->petstate << "','"
            << itr->second->alive << "','"
            << itr->second->talentpoints << "','"
            << itr->second->current_power << "','"
            << itr->second->current_hp << "','"
            << itr->second->current_happiness << "','"
            << itr->second->renamable << "','"
            << itr->second->type << "')";

        if (buf == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryStr(ss.str());
    }
}

void Player::_SavePetSpells(QueryBuffer* buf)
{
    // Remove any existing
    if (buf == nullptr)
        CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid=%u", GetLowGUID());
    else
        buf->AddQuery("DELETE FROM playersummonspells WHERE ownerguid=%u", GetLowGUID());

    // Save summon spells
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.begin();
    for (; itr != SummonSpells.end(); ++itr)
    {
        std::set<uint32>::iterator it = itr->second.begin();
        for (; it != itr->second.end(); ++it)
        {
            if (buf == nullptr)
                CharacterDatabase.Execute("INSERT INTO playersummonspells VALUES(%u, %u, %u)", GetLowGUID(), itr->first, (*it));
            else
                buf->AddQuery("INSERT INTO playersummonspells VALUES(%u, %u, %u)", GetLowGUID(), itr->first, (*it));
        }
    }
}

void Player::AddSummonSpell(uint32 Entry, uint32 SpellID)
{
    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(SpellID);
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if (itr == SummonSpells.end())
        SummonSpells[Entry].insert(SpellID);
    else
    {
        std::set<uint32>::iterator it3;
        for (std::set<uint32>::iterator it2 = itr->second.begin(); it2 != itr->second.end();)
        {
            it3 = it2++;
            if (sSpellCustomizations.GetSpellInfo(*it3)->custom_NameHash == sp->custom_NameHash)
                itr->second.erase(it3);
        }
        itr->second.insert(SpellID);
    }
}

void Player::RemoveSummonSpell(uint32 Entry, uint32 SpellID)
{
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if (itr != SummonSpells.end())
    {
        itr->second.erase(SpellID);
        if (itr->second.size() == 0)
            SummonSpells.erase(itr);
    }
}

std::set<uint32>* Player::GetSummonSpells(uint32 Entry)
{
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if (itr != SummonSpells.end())
    {
        return &(itr->second);
    }
    return nullptr;
}

void Player::_LoadPet(QueryResult* result)
{
    m_PetNumberMax = 0;
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        PlayerPet* pet = new PlayerPet;
        pet->number = fields[1].GetUInt32();
        pet->name = fields[2].GetString();
        pet->entry = fields[3].GetUInt32();

        pet->xp = fields[4].GetUInt32();
        pet->active = fields[5].GetInt8() % 10 > 0 ? true : false;
        pet->stablestate = fields[5].GetInt8() / 10;
        pet->level = fields[6].GetUInt32();
        pet->actionbar = fields[7].GetString();
        pet->happinessupdate = fields[8].GetUInt32();
        pet->reset_time = fields[9].GetUInt32();
        pet->reset_cost = fields[10].GetUInt32();
        pet->spellid = fields[11].GetUInt32();
        pet->petstate = fields[12].GetUInt32();
        pet->alive = fields[13].GetBool();
        pet->talentpoints = fields[14].GetUInt32();
        pet->current_power = fields[15].GetUInt32();
        pet->current_hp = fields[16].GetUInt32();
        pet->current_happiness = fields[17].GetUInt32();
        pet->renamable = fields[18].GetUInt32();
        pet->type = fields[19].GetUInt32();

        m_Pets[pet->number] = pet;

        if (pet->number > m_PetNumberMax)
            m_PetNumberMax = pet->number;
    }
    while (result->NextRow());
}

void Player::SpawnPet(uint32 pet_number)
{
    std::map<uint32, PlayerPet* >::iterator itr = m_Pets.find(pet_number);
    if (itr == m_Pets.end())
    {
        LOG_ERROR("PET SYSTEM: " I64FMT " Tried to load invalid pet %d", GetGUID(), pet_number);
        return;
    }
    Pet* pPet = objmgr.CreatePet(itr->second->entry);
    pPet->LoadFromDB(this, itr->second);

    if (this->IsPvPFlagged())
        pPet->SetPvPFlag();
    else
        pPet->RemovePvPFlag();

    if (this->IsFFAPvPFlagged())
        pPet->SetFFAPvPFlag();
    else
        pPet->RemoveFFAPvPFlag();

    if (this->IsSanctuaryFlagged())
        pPet->SetSanctuaryFlag();
    else
        pPet->RemoveSanctuaryFlag();

    pPet->SetFaction(this->GetFaction());

    if (itr->second->spellid)
    {
        //if demonic sacrifice auras are still active, remove them
        RemoveAura(18789);
        RemoveAura(18790);
        RemoveAura(18791);
        RemoveAura(18792);
        RemoveAura(35701);
    }
}

void Player::SpawnActivePet()
{
    if (GetSummon() != nullptr || !isAlive() || !IsInWorld())   ///\todo  only hunters for now
        return;

    std::map<uint32, PlayerPet* >::iterator itr = m_Pets.begin();
    for (; itr != m_Pets.end(); ++itr)
        if (itr->second->stablestate == STABLE_STATE_ACTIVE && itr->second->active)
        {
            if (itr->second->alive)
                SpawnPet(itr->first);
            return;
        }
}

void Player::DismissActivePets()
{
    for (std::list<Pet*>::reverse_iterator itr = m_Summons.rbegin(); itr != m_Summons.rend();)
    {
        Pet* summon = (*itr);
        if (summon->IsSummonedPet())
            summon->Dismiss();            // summons
        else
            summon->Remove(true, false);  // hunter pets
    }
}

void Player::_LoadPetSpells(QueryResult* result)
{
    std::map<uint32, std::list<uint32>* >::iterator itr;
    uint32 entry = 0;
    uint32 spell = 0;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            entry = fields[1].GetUInt32();
            spell = fields[2].GetUInt32();
            AddSummonSpell(entry, spell);
        }
        while (result->NextRow());
    }
}

void Player::addSpell(uint32 spell_id)
{
    SpellSet::iterator iter = mSpells.find(spell_id);
    if (iter != mSpells.end())
        return;

    mSpells.insert(spell_id);
    if (IsInWorld())
    {
        WorldPacket data(SMSG_LEARNED_SPELL, 6);
        data << uint32(spell_id);
#if VERSION_STRING != Cata
        data << uint16(0);
#else
        data << uint32(0);
#endif
        m_session->SendPacket(&data);
    }

    // Check if we're a deleted spell
    iter = mDeletedSpells.find(spell_id);
    if (iter != mDeletedSpells.end())
        mDeletedSpells.erase(iter);

    // Check if we're logging in.
    if (!IsInWorld())
        return;

    // Add the skill line for this spell if we don't already have it.
    auto skill_line_ability = objmgr.GetSpellSkill(spell_id);
    SpellInfo* spell = sSpellCustomizations.GetSpellInfo(spell_id);
    if (skill_line_ability && !_HasSkillLine(skill_line_ability->skilline))
    {
        auto skill_line = sSkillLineStore.LookupEntry(skill_line_ability->skilline);
        uint32 max = 1;
        if (skill_line != nullptr)
        {
            switch (skill_line->type)
            {
                case SKILL_TYPE_PROFESSION:
                    max = 75 * ((spell->custom_RankNumber) + 1);
                    ModPrimaryProfessionPoints(-1);   // we are learning a profession, so subtract a point.
                    break;
                case SKILL_TYPE_SECONDARY:
                    max = 75 * ((spell->custom_RankNumber) + 1);
                    break;
                case SKILL_TYPE_WEAPON:
                    max = 5 * getLevel();
                    break;
                case SKILL_TYPE_CLASS:
                case SKILL_TYPE_ARMOR:
                    if (skill_line->id == SKILL_LOCKPICKING)
                        max = 5 * getLevel();
                    break;
            };
        }

        _AddSkillLine(skill_line_ability->skilline, 1, max);
        _UpdateMaxSkillCounts();
    }

#if VERSION_STRING > TBC
    m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL, spell_id, 1, 0);
    if (spell->getMechanicsType() == MECHANIC_MOUNTED) // Mounts
    {
        // miscvalue1==777 for mounts, 778 for pets
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS, 777, 0, 0);
    }
    else if (spell->getEffect(0) == SPELL_EFFECT_SUMMON) // Companion pet?
    {
        // miscvalue1==777 for mounts, 778 for pets
        // make sure it's a companion pet, not some other summon-type spell
        if (strncmp(spell->getDescription().c_str(), "Right Cl", 8) == 0) // "Right Click to summon and dismiss " ...
        {
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS, 778, 0, 0);
        }
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Set Create Player Bits -- Sets bits required for creating a player in the updateMask.
/// Note:  Doesn't set Quest or Inventory bits
/// updateMask - the updatemask to hold the set bits
//////////////////////////////////////////////////////////////////////////////////////////
void Player::_SetCreateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::_SetCreateBits(updateMask, target);
    }
    else
    {
        for (uint32 index = 0; index < m_valuesCount; index++)
        {
            if (m_uint32Values[index] != 0 && Player::m_visibleUpdateMask.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}

void Player::_SetUpdateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::_SetUpdateBits(updateMask, target);
    }
    else
    {
        Object::_SetUpdateBits(updateMask, target);
        *updateMask &= Player::m_visibleUpdateMask;
    }
}

void Player::InitVisibleUpdateBits()
{
    Player::m_visibleUpdateMask.SetCount(PLAYER_END);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_GUID);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_TYPE);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_ENTRY);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_SCALE_X);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_SUMMON);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_SUMMON + 1);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_TARGET);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_TARGET + 1);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_HEALTH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER3);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER4);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER5);
#if VERSION_STRING == WotLK
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER6);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER7);
#endif

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXHEALTH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER3);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER4);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER5);
#if VERSION_STRING == WotLK
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER6);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER7);
#endif

#if VERSION_STRING > TBC
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID);
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID + 1);
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID + 2);
#endif

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_LEVEL);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_0);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FLAGS);
#if VERSION_STRING != Classic
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FLAGS_2);
#endif

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASEATTACKTIME);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASEATTACKTIME + 1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_COMBATREACH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_DISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_PETNUMBER);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT + 1);
    Player::m_visibleUpdateMask.SetBit(UNIT_CHANNEL_SPELL);
    Player::m_visibleUpdateMask.SetBit(UNIT_DYNAMIC_FLAGS);
    Player::m_visibleUpdateMask.SetBit(UNIT_NPC_FLAGS);
#if VERSION_STRING > TBC
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_HOVERHEIGHT);
#endif

    Player::m_visibleUpdateMask.SetBit(PLAYER_FLAGS);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES_2);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES_3);
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILD_TIMESTAMP);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_TEAM);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_ARBITER);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_ARBITER + 1);
#if VERSION_STRING != Cata
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILDID);
#endif
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILDRANK);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASE_MANA);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_AURASTATE);

    // Players visible items are not inventory stuff
    for (uint16 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
#if VERSION_STRING > TBC
        uint32 offset = i * 2; //VLack: for 3.1.1 "* 18" is a bad idea, now it's "* 2"; but this could have been calculated based on UpdateFields.h! This is PLAYER_VISIBLE_ITEM_LENGTH

        // item entry
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_ENTRYID + offset);
        // enchant
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_ENCHANTMENT + offset);
#else
        uint32 offset = i * 16;
        // item entry
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_0 + offset);
        // enchant
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_0 + 1 + offset);
#endif
    }

    //VLack: we have to send our quest list to the members of our group all the time for quest sharing's "who's on that quest" feature to work (in the quest log this way a number will be shown before the quest's name).
    //Unfortunately we don't have code for doing this only on our group's members, so everyone will receive it. The non-group member's client will do whatever it wants with it, probably wasting a few CPU cycles, but that's fine with me.
#if VERSION_STRING == Classic
    for (uint16 i = PLAYER_QUEST_LOG_1_1; i <= PLAYER_QUEST_LOG_15_1; i += 5)
#else
    for (uint16 i = PLAYER_QUEST_LOG_1_1; i <= PLAYER_QUEST_LOG_25_1; i += 5)
#endif
    {
        Player::m_visibleUpdateMask.SetBit(i);
    }

#if VERSION_STRING != Classic
    Player::m_visibleUpdateMask.SetBit(PLAYER_CHOSEN_TITLE);
#endif
}

void Player::SaveToDB(bool bNewCharacter /* =false */)
{
    bool in_arena = false;
    QueryBuffer* buf = nullptr;
    if (!bNewCharacter)
        buf = new QueryBuffer;

    if (m_bg != nullptr && isArena(m_bg->GetType()))
        in_arena = true;

    if (GetPrimaryProfessionPoints() > worldConfig.player.maxProfessions)
        SetPrimaryProfessionPoints(worldConfig.player.maxProfessions);

    //Calc played times
    uint32 playedt = (uint32)UNIXTIME - m_playedtime[2];
    m_playedtime[0] += playedt;
    m_playedtime[1] += playedt;
    m_playedtime[2] += playedt;

    // active cheats
    uint32 active_cheats = PLAYER_CHEAT_NONE;
    if (CooldownCheat)
        active_cheats |= PLAYER_CHEAT_COOLDOWN;
    if (CastTimeCheat)
        active_cheats |= PLAYER_CHEAT_CAST_TIME;
    if (GodModeCheat)
        active_cheats |= PLAYER_CHEAT_GOD_MODE;
    if (PowerCheat)
        active_cheats |= PLAYER_CHEAT_POWER;
    if (FlyCheat)
        active_cheats |= PLAYER_CHEAT_FLY;
    if (AuraStackCheat)
        active_cheats |= PLAYER_CHEAT_AURA_STACK;
    if (ItemStackCheat)
        active_cheats |= PLAYER_CHEAT_ITEM_STACK;
    if (TriggerpassCheat)
        active_cheats |= PLAYER_CHEAT_TRIGGERPASS;
    if (TaxiCheat)
        active_cheats |= PLAYER_CHEAT_TAXI;

    std::stringstream ss;

    ss << "DELETE FROM characters WHERE guid = " << GetLowGUID() << ";";

    if (bNewCharacter)
        CharacterDatabase.WaitExecuteNA(ss.str().c_str());
    else
        buf->AddQueryNA(ss.str().c_str());


    ss.rdbuf()->str("");

    ss << "INSERT INTO characters VALUES ("

        << GetLowGUID() << ", "
        << GetSession()->GetAccountId() << ","

        // stat saving
        << "'" << m_name << "', "
        << uint32(getRace()) << ","
        << uint32(getClass()) << ","
        << uint32(getGender()) << ",";

    if (GetFaction() != info->factiontemplate)
        ss << GetFaction() << ",";
    else
        ss << "0,";

    ss << uint32(getLevel()) << ","
        << GetXp() << ","
        << active_cheats << ","

        // dump exploration data
        << "'";

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_LENGTH; ++i)
        ss << m_uint32Values[PLAYER_EXPLORED_ZONES_1 + i] << ",";

    ss << "',";

    SaveSkills(bNewCharacter, buf);

    ss << m_uint32Values[PLAYER_FIELD_WATCHED_FACTION_INDEX] << ","
#if VERSION_STRING != Classic
        << m_uint32Values[PLAYER_CHOSEN_TITLE] << ","
#else
        << uint32(0) << ","
#endif
        << getUInt64Value(PLAYER_FIELD_KNOWN_TITLES) << ","
#if VERSION_STRING < WotLK
        << uint32(0) << ","
        << uint32(0) << ","
#else
        << getUInt64Value(PLAYER_FIELD_KNOWN_TITLES1) << ","
        << getUInt64Value(PLAYER_FIELD_KNOWN_TITLES2) << ","
#endif
        << m_uint32Values[PLAYER_FIELD_COINAGE] << ",";

    if ((getClass() == MAGE) || (getClass() == PRIEST) || (getClass() == WARLOCK))
        ss << (uint32)0 << ","; // make sure ammo slot is 0 for these classes, otherwise it can mess up wand shoot
    else
#if VERSION_STRING != Cata
        ss << m_uint32Values[PLAYER_AMMO_ID] << ",";
#else
        ss << (uint32)0 << ",";
#endif
    ss << GetPrimaryProfessionPoints() << ",";

    ss << load_health << ","
        << load_mana << ","
        << uint32(GetPVPRank()) << ","
        << m_uint32Values[PLAYER_BYTES] << ","
        << m_uint32Values[PLAYER_BYTES_2] << ",";

    uint32 player_flags = m_uint32Values[PLAYER_FLAGS];

    // Remove un-needed and problematic player flags from being saved :p
    if (player_flags & PLAYER_FLAG_PARTY_LEADER)
        player_flags &= ~PLAYER_FLAG_PARTY_LEADER;

    if (player_flags & PLAYER_FLAG_AFK)
        player_flags &= ~PLAYER_FLAG_AFK;

    if (player_flags & PLAYER_FLAG_DND)
        player_flags &= ~PLAYER_FLAG_DND;

    if (player_flags & PLAYER_FLAG_GM)
        player_flags &= ~PLAYER_FLAG_GM;

    if (player_flags & PLAYER_FLAG_PVP_TOGGLE)
        player_flags &= ~PLAYER_FLAG_PVP_TOGGLE;

    if (player_flags & PLAYER_FLAG_FREE_FOR_ALL_PVP)
        player_flags &= ~PLAYER_FLAG_FREE_FOR_ALL_PVP;

    ss << player_flags << ","
        << m_uint32Values[PLAYER_FIELD_BYTES] << ",";

    if (in_arena)
    {
        // if its an arena, save the entry coords instead
        ss << m_bgEntryPointX << ", ";
        ss << m_bgEntryPointY << ", ";
        ss << m_bgEntryPointZ << ", ";
        ss << m_bgEntryPointO << ", ";
        ss << m_bgEntryPointMap << ", ";
    }
    else
    {
        // save the normal position
        ss << m_position.x << ", "
            << m_position.y << ", "
            << m_position.z << ", "
            << m_position.o << ", "
            << m_mapId << ", ";
    }

    ss << m_zoneId << ", '";

    for (uint8 i = 0; i < 12; i++)
        ss << m_taximask[i] << " ";
    ss << "', "

        << m_banned << ", '"
        << CharacterDatabase.EscapeString(m_banreason) << "', "
        << uint32(UNIXTIME) << ",";

    //online state
    if (GetSession()->_loggingOut || bNewCharacter)
    {
        ss << "0,";
    }
    else
    {
        ss << "1,";
    }

    ss
        << m_bind_pos_x << ", "
        << m_bind_pos_y << ", "
        << m_bind_pos_z << ", "
        << m_bind_mapid << ", "
        << m_bind_zoneid << ", "

        << uint32(m_isResting) << ", "
        << uint32(m_restState) << ", "
        << uint32(m_restAmount) << ", '"

        << uint32(m_playedtime[0]) << " "
        << uint32(m_playedtime[1]) << " "
        << uint32(playedt) << " ', "
        << uint32(m_deathState) << ", "

        << m_talentresettimes << ", "
        << m_FirstLogin << ", "
        << login_flags
        << "," << m_arenaPoints << ","
        << (uint32)m_StableSlotCount << ",";

    // instances
    if (in_arena)
    {
        ss << m_bgEntryPointInstance << ", ";
    }
    else
    {
        ss << m_instanceId << ", ";
    }

    ss << m_bgEntryPointMap << ", "
        << m_bgEntryPointX << ", "
        << m_bgEntryPointY << ", "
        << m_bgEntryPointZ << ", "
        << m_bgEntryPointO << ", "
        << m_bgEntryPointInstance << ", ";

    // taxi
    if (m_onTaxi && m_CurrentTaxiPath)
    {
        ss << m_CurrentTaxiPath->GetID() << ", ";
        ss << lastNode << ", ";
        ss << GetMount();
    }
    else
    {
        ss << "0, 0, 0";
    }
    auto transport = this->GetTransport();
    if (!transport)
    {
        ss << "," << 0 << ",'0','0','0','0'";
    }
    else
    {
        ss << "," << transport->GetEntry();
        ss << ",'" << GetTransPositionX() << "','" << GetTransPositionY() << "','" << GetTransPositionZ() << "','" << GetTransPositionO() << "'";
    }
    ss << ",'";

    SaveSpells(bNewCharacter, buf);

    SaveDeletedSpells(bNewCharacter, buf);

    SaveReputations(bNewCharacter, buf);

    // Add player action bars
    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        {
            ss << uint32(m_specs[s].mActions[i].Action) << ","
                << uint32(m_specs[s].mActions[i].Type) << ","
                << uint32(m_specs[s].mActions[i].Misc) << ",";
        }
        ss << "','";
    }

    if (!bNewCharacter)
        SaveAuras(ss);

    ss << "','";

    // Add player finished quests
    std::set<uint32>::iterator fq = m_finishedQuests.begin();
    for (; fq != m_finishedQuests.end(); ++fq)
    {
        ss << (*fq) << ",";
    }

    ss << "', '";
    DailyMutex.Acquire();
    std::set<uint32>::iterator fdq = m_finishedDailies.begin();
    for (; fdq != m_finishedDailies.end(); ++fdq)
    {
        ss << (*fdq) << ",";
    }
    DailyMutex.Release();
    ss << "', ";
    ss << m_honorRolloverTime << ", ";
    ss << m_killsToday << ", ";
    ss << m_killsYesterday << ", ";
    ss << m_killsLifetime << ", ";
    ss << m_honorToday << ", ";
    ss << m_honorYesterday << ", ";
    ss << m_honorPoints << ", ";

    ss << (m_uint32Values[PLAYER_BYTES_3] & 0xFFFE) << ", ";

    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        ss << "'";
        for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
            ss << m_specs[s].glyphs[i] << ",";
        ss << "','";
        for (std::map<uint32, uint8>::iterator itr = m_specs[s].talents.begin(); itr != m_specs[s].talents.end(); ++itr)
            ss << itr->first << "," << uint32(itr->second) << ",";
        ss << "',";
    }
    ss << uint32(m_talentSpecsCount) << ", " << uint32(m_talentActiveSpec);
    ss << ", '";
    ss << uint32(m_specs[SPEC_PRIMARY].GetTP()) << " " << uint32(m_specs[SPEC_SECONDARY].GetTP());
    ss << "', ";

#if VERSION_STRING != Cata
    ss << "'" << uint32(0);
    ss << "', ";
#else
    ss << "'" << uint32(m_FirstTalentTreeLock);
    ss << "', ";
#endif

    ss << "'" << m_phase << "','";

    uint32 xpfield;

    if (m_XpGain)
        xpfield = 1;
    else
        xpfield = 0;

    ss << xpfield << "'" << ", '";

    bool saveData = worldConfig.server.saveExtendedCharData;
    if (saveData)
    {
        for (uint32 offset = OBJECT_END; offset < PLAYER_END; offset++)
            ss << uint32(m_uint32Values[offset]) << ";";
    }
    ss << "', '";

    if (resettalents)
        ss << uint32(1);
    else
        ss << uint32(0);

    ss << "', ";

    ss << uint32(this->HasWonRbgToday());
    ss << ", ";
    ss << uint32(iInstanceType) << ", ";
    ss << uint32(m_RaidDifficulty);
    ss << ")";

    if (bNewCharacter)
        CharacterDatabase.WaitExecuteNA(ss.str().c_str());
    else
        buf->AddQueryNA(ss.str().c_str());

    //Save Other related player stuff

    // Inventory
    GetItemInterface()->mSaveItemsToDatabase(bNewCharacter, buf);

    GetItemInterface()->m_EquipmentSets.SavetoDB(buf);

    // save quest progress
    _SaveQuestLogEntry(buf);

    // Tutorials
    _SaveTutorials(buf);

    // GM Ticket
    ///\todo Is this really necessary? Tickets will always be saved on creation, update and so on...
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetGUID());
    if (ticket != nullptr)
        objmgr.SaveGMTicket(ticket, buf);

    // Cooldown Items
    _SavePlayerCooldowns(buf);

    // Pets
    if (getClass() == HUNTER || getClass() == WARLOCK)
    {
        _SavePet(buf);
        _SavePetSpells(buf);
    }
    m_nextSave = Util::getMSTime() + worldConfig.getIntRate(INTRATE_SAVE);
#if VERSION_STRING > TBC
    m_achievementMgr.SaveToDB(buf);
#endif

    if (buf)
        CharacterDatabase.AddQueryBuffer(buf);
}

void Player::_SaveQuestLogEntry(QueryBuffer* buf)
{
    for (std::set<uint32>::iterator itr = m_removequests.begin(); itr != m_removequests.end(); ++itr)
    {
        if (buf == nullptr)
            CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", GetLowGUID(), (*itr));
        else
            buf->AddQuery("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", GetLowGUID(), (*itr));
    }

    m_removequests.clear();

    for (uint8 i = 0; i < 25; ++i)
    {
        if (m_questlog[i] != nullptr)
            m_questlog[i]->SaveToDB(buf);
    }
}

bool Player::canCast(SpellInfo* m_spellInfo)
{
    if (m_spellInfo->getEquippedItemClass() != 0)
    {
        if (this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
        {
            if ((int32)this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetItemProperties()->Class == m_spellInfo->getEquippedItemClass())
            {
                if (m_spellInfo->getEquippedItemSubClass() != 0)
                {
                    if (m_spellInfo->getEquippedItemSubClass() != 173555 && m_spellInfo->getEquippedItemSubClass() != 96 && m_spellInfo->getEquippedItemSubClass() != 262156)
                    {
                        if (pow(2.0, (this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetItemProperties()->SubClass) != m_spellInfo->getEquippedItemSubClass()))
                            return false;
                    }
                }
            }
        }
        else if (m_spellInfo->getEquippedItemSubClass() == 173555)
            return false;

        if (this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
        {
            if ((int32)this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetItemProperties()->Class == m_spellInfo->getEquippedItemClass())
            {
                if (m_spellInfo->getEquippedItemSubClass() != 0)
                {
                    if (m_spellInfo->getEquippedItemSubClass() != 173555 && m_spellInfo->getEquippedItemSubClass() != 96 && m_spellInfo->getEquippedItemSubClass() != 262156)
                    {
                        if (pow(2.0, (this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetItemProperties()->SubClass) != m_spellInfo->getEquippedItemSubClass()))                            return false;
                    }
                }
            }
        }
        else if
            (m_spellInfo->getEquippedItemSubClass() == 262156)
            return false;
    }
    return true;
}

void Player::RemovePendingPlayer()
{
    if (m_session)
    {
        uint8 respons = E_CHAR_LOGIN_NO_CHARACTER;
        m_session->OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &respons);
        m_session->m_loggingInPlayer = nullptr;
    }

    ok_to_remove = true;
    delete this;
}

bool Player::LoadFromDB(uint32 guid)
{
    AsyncQuery* q = new AsyncQuery(new SQLClassCallbackP0<Player>(this, &Player::LoadFromDBProc));

    q->AddQuery("SELECT * FROM characters WHERE guid = %u AND login_flags = %u", guid, (uint32)LOGIN_NO_FLAG); // 0
    q->AddQuery("SELECT * FROM tutorials WHERE playerId = %u", guid); // 1
    q->AddQuery("SELECT cooldown_type, cooldown_misc, cooldown_expire_time, cooldown_spellid, cooldown_itemid FROM playercooldowns WHERE player_guid = %u", guid); // 2
    q->AddQuery("SELECT * FROM questlog WHERE player_guid = %u", guid); // 3
    q->AddQuery("SELECT * FROM playeritems WHERE ownerguid = %u ORDER BY containerslot ASC", guid); // 4
    q->AddQuery("SELECT * FROM playerpets WHERE ownerguid = %u ORDER BY petnumber", guid); // 5
    q->AddQuery("SELECT * FROM playersummonspells where ownerguid = %u ORDER BY entryid", guid); // 6
    q->AddQuery("SELECT * FROM mailbox WHERE player_guid = %u", guid); // 7

    // social
    q->AddQuery("SELECT friend_guid, note FROM social_friends WHERE character_guid = %u", guid); // 8
    q->AddQuery("SELECT character_guid FROM social_friends WHERE friend_guid = %u", guid); // 9
    q->AddQuery("SELECT ignore_guid FROM social_ignores WHERE character_guid = %u", guid); // 10


    q->AddQuery("SELECT * FROM equipmentsets WHERE ownerguid = %u", guid);  // 11
    q->AddQuery("SELECT faction, flag, basestanding, standing FROM playerreputations WHERE guid = %u", guid); //12
    q->AddQuery("SELECT SpellID FROM playerspells WHERE GUID = %u", guid);  // 13
    q->AddQuery("SELECT SpellID FROM playerdeletedspells WHERE GUID = %u", guid);  // 14
    q->AddQuery("SELECT SkillID, CurrentValue, MaximumValue FROM playerskills WHERE GUID = %u", guid);  // 15

    //Achievements
    q->AddQuery("SELECT achievement, date FROM character_achievement WHERE guid = '%u'", guid); // 16
    q->AddQuery("SELECT criteria, counter, date FROM character_achievement_progress WHERE guid = '%u'", guid); // 17

    // queue it!
    SetLowGUID(guid);
    CharacterDatabase.QueueAsyncQuery(q);
    return true;

}

void Player::LoadFromDBProc(QueryResultVector & results)
{
    uint32 field_index = 2;
#define get_next_field fields[field_index++]

    if (GetSession() == nullptr || results.size() < 8)        // should have 8 queryresults for aplayer load.
    {
        RemovePendingPlayer();
        return;
    }

    QueryResult* result = results[0].result;
    if (!result)
    {
        LOG_ERROR("Player login query failed! guid = %u", GetLowGUID());
        RemovePendingPlayer();
        return;
    }

    const uint32 fieldcount = 95;

    if (result->GetFieldCount() != fieldcount)
    {
        LOG_ERROR("Expected %u fields from the database, but received %u!  You may need to update your character database.", fieldcount, uint32(result->GetFieldCount()));
        RemovePendingPlayer();
        return;
    }

    Field* fields = result->Fetch();

    if (fields[1].GetUInt32() != m_session->GetAccountId())
    {
        sCheatLog.writefromsession(m_session, "player tried to load character not belonging to them (guid %u, on account %u)",
                                   fields[0].GetUInt32(), fields[1].GetUInt32());
        RemovePendingPlayer();
        return;
    }

    uint32 banned = fields[34].GetUInt32();
    if (banned && (banned < 100 || banned >(uint32)UNIXTIME))
    {
        RemovePendingPlayer();
        return;
    }

    // Load name
    m_name = get_next_field.GetString();
    // Update Cache
    m_cache->SetStringValue(CACHE_PLAYER_NAME, m_name);

    // Load race/class from fields
    setRace(get_next_field.GetUInt8());
    setClass(get_next_field.GetUInt8());
    setGender(get_next_field.GetUInt8());
    uint32 cfaction = get_next_field.GetUInt32();

    // set race dbc
    myRace = sChrRacesStore.LookupEntry(getRace());
    myClass = sChrClassesStore.LookupEntry(getClass());
    if (!myClass || !myRace)
    {
        // bad character
        LOG_ERROR("guid %u failed to login, no race or class dbc found. (race %u class %u)", (unsigned int)GetLowGUID(), (unsigned int)getRace(), (unsigned int)getClass());
        RemovePendingPlayer();
        return;
    }

    if (myRace->team_id == 7)
    {
        m_bgTeam = m_team = 0;
    }
    else
    {
        m_bgTeam = m_team = 1;
    }
    m_cache->SetUInt32Value(CACHE_PLAYER_INITIALTEAM, m_team);

    SetNoseLevel();

    // set power type
    SetPowerType(static_cast<uint8>(myClass->power_type));

    // obtain player create info
    info = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    if (info == nullptr)
    {
        LOG_ERROR("player guid %u has no playerCreateInfo!", (unsigned int)GetLowGUID());
        RemovePendingPlayer();
        return;
    }

    // set level
    setLevel(get_next_field.GetUInt32());

    // obtain level/stats information
    lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), getLevel());

    if (!lvlinfo)
    {
        LOG_ERROR("guid %u level %u class %u race %u levelinfo not found!", (unsigned int)GetLowGUID(), (unsigned int)getLevel(), (unsigned int)getClass(), (unsigned int)getRace());
        RemovePendingPlayer();
        return;
    }

#if VERSION_STRING > TBC
    // load achievements before anything else otherwise skills would complete achievements already in the DB, leading to duplicate achievements and criterias(like achievement=126).
    m_achievementMgr.LoadFromDB(results[16].result, results[17].result);
#endif

    CalculateBaseStats();

    // set xp
    SetXp(get_next_field.GetUInt32());

    // Load active cheats
    uint32 active_cheats = get_next_field.GetUInt32();
    if (active_cheats & PLAYER_CHEAT_COOLDOWN)
        CooldownCheat = true;
    if (active_cheats & PLAYER_CHEAT_CAST_TIME)
        CastTimeCheat = true;
    if (active_cheats & PLAYER_CHEAT_GOD_MODE)
        GodModeCheat = true;
    if (active_cheats & PLAYER_CHEAT_POWER)
        PowerCheat = true;
    if (active_cheats & PLAYER_CHEAT_FLY)
        FlyCheat = true;
    if (active_cheats & PLAYER_CHEAT_AURA_STACK)
        AuraStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_ITEM_STACK)
        ItemStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_TRIGGERPASS)
        TriggerpassCheat = true;
    if (active_cheats & PLAYER_CHEAT_TAXI)
        TaxiCheat = true;

    // Process exploration data.
    LoadFieldsFromString(get_next_field.GetString(), PLAYER_EXPLORED_ZONES_1, PLAYER_EXPLORED_ZONES_LENGTH);

    // Process skill data.
    uint32 Counter = 0;
    char* start = nullptr;
    char* end = nullptr;

    // new format
    const ItemProf* prof1;

    LoadSkills(results[15].result);

    if (m_skills.empty())
    {
        /* no skills - reset to defaults */
        for (std::list<CreateInfo_SkillStruct>::const_iterator ss = info->skills.begin(); ss != info->skills.end(); ++ss)
        {
            if (ss->skillid && ss->currentval && ss->maxval && !::GetSpellForLanguage(ss->skillid))
                _AddSkillLine(ss->skillid, ss->currentval, ss->maxval);
        }
    }

    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
        if (itr->first == SKILL_RIDING)
        {
            itr->second.CurrentValue = itr->second.MaximumValue;
        }

        prof1 = GetProficiencyBySkill(itr->first);
        if (prof1)
        {
            if (prof1->itemclass == 4)
                armor_proficiency |= prof1->subclass;
            else
                weapon_proficiency |= prof1->subclass;
        }
        _LearnSkillSpells(itr->second.Skill->id, itr->second.CurrentValue);
    }

    // set the rest of the stuff
    m_uint32Values[PLAYER_FIELD_WATCHED_FACTION_INDEX] = get_next_field.GetUInt32();
    SetChosenTitle(get_next_field.GetUInt32());
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES, get_next_field.GetUInt64());
#if VERSION_STRING > TBC
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES1, get_next_field.GetUInt64());
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES2, get_next_field.GetUInt64());
#else
    get_next_field.GetUInt32();
    get_next_field.GetUInt32();
#endif
    m_uint32Values[PLAYER_FIELD_COINAGE] = get_next_field.GetUInt32();
#if VERSION_STRING != Cata
    m_uint32Values[PLAYER_AMMO_ID] = get_next_field.GetUInt32();
    m_uint32Values[PLAYER_CHARACTER_POINTS2] = get_next_field.GetUInt32();
#else
    get_next_field.GetUInt32();
    get_next_field.GetUInt32();
#endif
    load_health = get_next_field.GetUInt32();
    load_mana = get_next_field.GetUInt32();
    SetHealth(load_health);
    uint8 pvprank = get_next_field.GetUInt8();
    setUInt32Value(PLAYER_BYTES, get_next_field.GetUInt32());
    setUInt32Value(PLAYER_BYTES_2, get_next_field.GetUInt32());
    setUInt32Value(PLAYER_BYTES_3, getGender() | (pvprank << 24));
    setUInt32Value(PLAYER_FLAGS, get_next_field.GetUInt32());
    setUInt32Value(PLAYER_FIELD_BYTES, get_next_field.GetUInt32());
    //m_uint32Values[0x22]=(m_uint32Values[0x22]>0x46)?0x46:m_uint32Values[0x22];

    m_position.x = get_next_field.GetFloat();
    m_position.y = get_next_field.GetFloat();
    m_position.z = get_next_field.GetFloat();
    m_position.o = get_next_field.GetFloat();

    m_mapId = get_next_field.GetUInt32();
    m_zoneId = get_next_field.GetUInt32();
    SetZoneId(m_zoneId);

    // Calculate the base stats now they're all loaded
    for (uint8 i = 0; i < 5; ++i)
        CalcStat(i);

    //  for (uint32 x = PLAYER_SPELL_CRIT_PERCENTAGE1; x < PLAYER_SPELL_CRIT_PERCENTAGE06 + 1; ++x)
    ///    SetFloatValue(x, 0.0f);

#if VERSION_STRING != Classic
    for (uint16_t x = PLAYER_FIELD_MOD_DAMAGE_DONE_PCT; x < PLAYER_FIELD_MOD_HEALING_DONE_POS; ++x)
        setFloatValue(x, 1.0f);
#endif

    // Normal processing...
    UpdateStats();

    // Initialize 'normal' fields
    SetScale(1.0f);
#if VERSION_STRING > TBC
    setFloatValue(UNIT_FIELD_HOVERHEIGHT, 1.0f);
#endif

    //SetUInt32Value(UNIT_FIELD_POWER2, 0);
    SetPower(POWER_TYPE_FOCUS, info->focus); // focus
    SetPower(POWER_TYPE_ENERGY, info->energy);
    SetPower(POWER_TYPE_RUNES, 8);
    SetMaxPower(POWER_TYPE_RAGE, info->rage);
    SetMaxPower(POWER_TYPE_FOCUS, info->focus);
    SetMaxPower(POWER_TYPE_ENERGY, info->energy);
    SetMaxPower(POWER_TYPE_RUNES, 8);
    SetMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
    if (getClass() == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

    setUInt32Value(UNIT_FIELD_BYTES_2, (0x28 << 8));
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    SetBoundingRadius(0.388999998569489f);
    SetCombatReach(1.5f);

    if (getRace() != RACE_BLOODELF)
    {
        SetDisplayId(info->displayId + getGender());
        SetNativeDisplayId(info->displayId + getGender());
    }
    else
    {
        SetDisplayId(info->displayId - getGender());
        SetNativeDisplayId(info->displayId - getGender());
    }
    EventModelChange();

    SetCastSpeedMod(1.0f);
#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_FIELD_MAX_LEVEL, worldConfig.player.playerLevelCap);
#endif
    SetFaction(info->factiontemplate);
    if (cfaction)
    {
        SetFaction(cfaction);
        // re-calculate team
        switch (cfaction)
        {
            case 1:     // human
            case 3:     // dwarf
            case 4:     // ne
            case 8:     // gnome
            case 927:   // draenei
                m_team = m_bgTeam = 0;
                break;
            case 2:     // orc
            case 5:     // undead
            case 6:     // tauren
            case 9:     // troll
            case 914:   // bloodelf
                m_team = m_bgTeam = 1;
                break;
        }
    }

    LoadTaxiMask(get_next_field.GetString());

    m_banned = get_next_field.GetUInt32();      //Character ban
    m_banreason = get_next_field.GetString();
    m_timeLogoff = get_next_field.GetUInt32();
    field_index++;

    m_bind_pos_x = get_next_field.GetFloat();
    m_bind_pos_y = get_next_field.GetFloat();
    m_bind_pos_z = get_next_field.GetFloat();
    m_bind_mapid = get_next_field.GetUInt32();
    m_bind_zoneid = get_next_field.GetUInt32();

    m_isResting = get_next_field.GetUInt8();
    m_restState = get_next_field.GetUInt8();
    m_restAmount = get_next_field.GetUInt32();


    std::string tmpStr = get_next_field.GetString();
    m_playedtime[0] = (uint32)atoi((const char*)strtok((char*)tmpStr.c_str(), " "));
    m_playedtime[1] = (uint32)atoi((const char*)strtok(nullptr, " "));

    m_deathState = (DeathState)get_next_field.GetUInt32();
    m_talentresettimes = get_next_field.GetUInt32();
    m_FirstLogin = get_next_field.GetBool();
    login_flags = get_next_field.GetUInt32();
    m_arenaPoints = get_next_field.GetUInt32();
    if (m_arenaPoints > worldConfig.limit.maxArenaPoints)
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxArenaPoints << " arena points " << m_arenaPoints;
        sCheatLog.writefromsession(m_session, dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)          // report to online GMs
            sendReportToGmMessage(GetName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            m_session->Disconnect();
        }
        m_arenaPoints = worldConfig.limit.maxArenaPoints;
    }
    for (uint32 z = 0; z < NUM_CHARTER_TYPES; ++z)
        m_charters[z] = objmgr.GetCharterByGuid(GetGUID(), (CharterTypes)z);
    for (uint16 z = 0; z < NUM_ARENA_TEAM_TYPES; ++z)
    {
        m_arenaTeams[z] = objmgr.GetArenaTeamByGuid(GetLowGUID(), z);
        if (m_arenaTeams[z] != nullptr)
        {
#if VERSION_STRING != Classic
            setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z * 7), m_arenaTeams[z]->m_id);
            if (m_arenaTeams[z]->m_leader == GetLowGUID())
                setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z * 7) + 1, 0);
            else
                setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z * 7) + 1, 1);
#endif
        }
    }

    m_StableSlotCount = static_cast<uint8>(get_next_field.GetUInt32());
    m_instanceId = get_next_field.GetUInt32();
    m_bgEntryPointMap = get_next_field.GetUInt32();
    m_bgEntryPointX = get_next_field.GetFloat();
    m_bgEntryPointY = get_next_field.GetFloat();
    m_bgEntryPointZ = get_next_field.GetFloat();
    m_bgEntryPointO = get_next_field.GetFloat();
    m_bgEntryPointInstance = get_next_field.GetUInt32();

    uint32 taxipath = get_next_field.GetUInt32();
    TaxiPath* path = nullptr;
    if (taxipath)
    {
        path = sTaxiMgr.GetTaxiPath(taxipath);
        lastNode = get_next_field.GetUInt32();
        if (path)
        {
            SetMount(get_next_field.GetUInt32());
            SetTaxiPath(path);
            m_onTaxi = true;
        }
        else
            field_index++;
    }
    else
    {
        field_index++;
        field_index++;
    }

#if VERSION_STRING != Cata
    obj_movement_info.transporter_info.guid = get_next_field.GetUInt32();
    if (obj_movement_info.transporter_info.guid)
    {
        Transporter* t = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.transporter_info.guid));
        obj_movement_info.transporter_info.guid = t ? t->GetGUID() : 0;
    }

    obj_movement_info.transporter_info.position.x = get_next_field.GetFloat();
    obj_movement_info.transporter_info.position.y = get_next_field.GetFloat();
    obj_movement_info.transporter_info.position.z = get_next_field.GetFloat();
    obj_movement_info.transporter_info.position.o = get_next_field.GetFloat();
#else
    uint32_t transportGuid = get_next_field.GetUInt32();
    float transportX = get_next_field.GetFloat();
    float transportY = get_next_field.GetFloat();
    float transportZ = get_next_field.GetFloat();
    float transportO = get_next_field.GetFloat();

    if (transportGuid != 0)
    {
        obj_movement_info.setTransportData(transportGuid, transportX, transportY, transportZ, transportO, 0, 0);
    }
    else
    {
        obj_movement_info.clearTransportData();
    }

#endif

    LoadSpells(results[13].result);

    LoadDeletedSpells(results[14].result);

    LoadReputations(results[12].result);

    // Load saved actionbars
    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        start = (char*)get_next_field.GetString();
        Counter = 0;
        while (Counter < PLAYER_ACTION_BUTTON_COUNT)
        {
            if (start == nullptr)
                break;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[s].mActions[Counter].Action = (uint16)atol(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[s].mActions[Counter].Type = (uint8)atol(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[s].mActions[Counter].Misc = (uint8)atol(start);
            start = end + 1;

            Counter++;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Parse saved buffs
    std::istringstream savedPlayerBuffsStream(get_next_field.GetString());
    std::string auraId, auraDuration, auraPositivValue, auraCharges;

    while (std::getline(savedPlayerBuffsStream, auraId, ','))
    {
        LoginAura la;
        la.id = atol(auraId.c_str());

        std::getline(savedPlayerBuffsStream, auraDuration, ',');
        la.dur = atol(auraDuration.c_str());

        std::getline(savedPlayerBuffsStream, auraPositivValue, ',');
        la.positive = (auraPositivValue.c_str());

        std::getline(savedPlayerBuffsStream, auraCharges, ',');
        la.charges = atol(auraCharges.c_str());

        loginauras.push_back(la);
    }

    // Load saved finished quests

    start = (char*)get_next_field.GetString();
    while (true)
    {
        end = strchr(start, ',');
        if (!end)break;
        *end = 0;
        m_finishedQuests.insert(atol(start));
        start = end + 1;
    }

    start = (char*)get_next_field.GetString();
    while (true)
    {
        end = strchr(start, ',');
        if (!end) break;
        *end = 0;
        m_finishedDailies.insert(atol(start));
        start = end + 1;
    }

    m_honorRolloverTime = get_next_field.GetUInt32();
    m_killsToday = get_next_field.GetUInt32();
    m_killsYesterday = get_next_field.GetUInt32();
    m_killsLifetime = get_next_field.GetUInt32();

    m_honorToday = get_next_field.GetUInt32();
    m_honorYesterday = get_next_field.GetUInt32();
    m_honorPoints = get_next_field.GetUInt32();
    if (m_honorPoints > worldConfig.limit.maxHonorPoints)
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxHonorPoints << " honor points " << m_honorPoints;

        sCheatLog.writefromsession(m_session, dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)
            sendReportToGmMessage(GetName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            m_session->Disconnect();
        }
        m_honorPoints = worldConfig.limit.maxHonorPoints;
    }

    RolloverHonor();

    // Load drunk value and calculate sobering. after 15 minutes logged out, the player will be sober again
    uint32 timediff = (uint32)UNIXTIME - m_timeLogoff;
    uint32 soberFactor;
    if (timediff > 900)
        soberFactor = 0;
    else
        soberFactor = 1 - timediff / 900;
    SetDrunkValue(uint16(soberFactor * get_next_field.GetUInt32()));

    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        start = (char*)get_next_field.GetString();
        uint8 glyphid = 0;
        while (glyphid < GLYPHS_COUNT)
        {
            end = strchr(start, ',');
            if (!end)break;
            *end = 0;
            m_specs[s].glyphs[glyphid] = (uint16)atol(start);
            ++glyphid;
            start = end + 1;
        }

        //Load talents for spec
        start = (char*)get_next_field.GetString();
        while (end != nullptr)
        {
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint32 talentid = atol(start);
            start = end + 1;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint8 rank = (uint8)atol(start);
            start = end + 1;

            m_specs[s].talents.insert(std::pair<uint32, uint8>(talentid, rank));
        }
    }

    m_talentSpecsCount = get_next_field.GetUInt8();
    m_talentActiveSpec = get_next_field.GetUInt8();

    {
        std::stringstream ss(get_next_field.GetString());
        uint32 tp1 = 0;
        uint32 tp2 = 0;

        ss >> tp1;
        ss >> tp2;

        m_specs[SPEC_PRIMARY].SetTP(tp1);
        m_specs[SPEC_SECONDARY].SetTP(tp2);
#if VERSION_STRING != Cata
        setUInt32Value(PLAYER_CHARACTER_POINTS1, m_specs[m_talentActiveSpec].GetTP());
#else
        setUInt32Value(PLAYER_CHARACTER_POINTS, m_specs[m_talentActiveSpec].GetTP());
#endif
    }

#if VERSION_STRING != Cata
    get_next_field.GetUInt32();
#else
    m_FirstTalentTreeLock = get_next_field.GetUInt32(); // Load First Set Talent Tree
#endif

    m_phase = get_next_field.GetUInt32(); //Load the player's last phase

    uint32 xpfield = get_next_field.GetUInt32();

    if (xpfield == 0)
        m_XpGain = false;
    else
        m_XpGain = true;

    get_next_field;//skipping one

    if (get_next_field.GetUInt32() == 1)
        resettalents = true;
    else
        resettalents = false;

    // Load player's RGB daily data
    if (get_next_field.GetUInt32() == 1)
        m_bgIsRbgWon = true;
    else
        m_bgIsRbgWon = false;

    iInstanceType = get_next_field.GetUInt8();
    m_RaidDifficulty = get_next_field.GetUInt8();

    HonorHandler::RecalculateHonorFields(this);

    for (uint16 x = 0; x < 5; x++)
        BaseStats[x] = GetStat(x);

#if VERSION_STRING > TBC
    UpdateGlyphs();

    for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
    {
        SetGlyph(i, m_specs[m_talentActiveSpec].glyphs[i]);
    }
#endif
    //class fixes
    switch (getClass())
    {
        case PALADIN:
            armor_proficiency |= (1 << 7);  //LIBRAM
            break;
        case DRUID:
            armor_proficiency |= (1 << 8);  //IDOL
            break;
        case SHAMAN:
            armor_proficiency |= (1 << 9);  //TOTEM
            break;
        case DEATHKNIGHT:
            armor_proficiency |= (1 << 10);  //SIGIL
            break;
        case WARLOCK:
        case HUNTER:
            _LoadPet(results[5].result);
            _LoadPetSpells(results[6].result);
            break;
    }

#if VERSION_STRING != Cata
    if (m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else
        _AddLanguages(false);
#else
    _AddLanguages(false);
#endif

    if (GetGuildId())
        setUInt32Value(PLAYER_GUILD_TIMESTAMP, (uint32)UNIXTIME);

#undef get_next_field

    // load properties
    _LoadTutorials(results[1].result);
    _LoadPlayerCooldowns(results[2].result);
    _LoadQuestLogEntry(results[3].result);
    m_ItemInterface->mLoadItemsFromDatabase(results[4].result);
    m_ItemInterface->m_EquipmentSets.LoadfromDB(results[11].result);

    m_mailBox.Load(results[7].result);

    // SOCIAL
    if (results[8].result != nullptr)            // this query is "who are our friends?"
    {
        result = results[8].result;
        do
        {
            fields = result->Fetch();
            uint32 friendguid = fields[0].GetUInt32();
            const char* str = fields[1].GetString();
            char* note = nullptr;
            if (strlen(str) > 0)
                note = strdup(str);
            m_cache->InsertValue64(CACHE_SOCIAL_FRIENDLIST, friendguid, note);
        }
        while (result->NextRow());
    }

    if (results[9].result != nullptr)            // this query is "who has us in their friends?"
    {
        result = results[9].result;
        do
        {
            m_cache->InsertValue64(CACHE_SOCIAL_HASFRIENDLIST, result->Fetch()[0].GetUInt32());
        }
        while (result->NextRow());
    }

    if (results[10].result != nullptr)        // this query is "who are we ignoring"
    {
        result = results[10].result;
        do
        {
            uint32 guid = result->Fetch()[0].GetUInt32();
            m_cache->InsertValue64(CACHE_SOCIAL_IGNORELIST, guid);
        }
        while (result->NextRow());
    }

    // END SOCIAL

    // Check skills that player shouldn't have
    if (_HasSkillLine(SKILL_DUAL_WIELD) && !HasSpell(674))
    {
        _RemoveSkillLine(SKILL_DUAL_WIELD);
    }

#if VERSION_STRING > TBC
    // update achievements before adding player to World, otherwise we'll get a nice race condition.
    //move CheckAllAchievementCriteria() after FullLogin(this) and i'll cut your b***s.
    m_achievementMgr.CheckAllAchievementCriteria();
#endif

    m_session->FullLogin(this);
    m_session->m_loggingInPlayer = nullptr;

    if (!isAlive())
    {
        Corpse* myCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
        if (myCorpse != nullptr)
        {
            myCorpseLocation = myCorpse->GetPosition();
            myCorpseInstanceId = myCorpse->GetInstanceID();
        }
    }

    // check for multiple gems with unique-equipped flag
    uint32 count;
    uint32 uniques[64];
    int nuniques = 0;

    for (uint8 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        ItemInterface* itemi = GetItemInterface();
        Item* it = itemi->GetInventoryItem(x);

        if (it != nullptr)
        {
            for (count = 0; count < it->GetSocketsCount(); count++)
            {
                EnchantmentInstance* ei = it->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);

                if (ei && ei->Enchantment)
                {
                    ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);

                    if (ip && ip->Flags & ITEM_FLAG_UNIQUE_EQUIP &&
                        itemi->IsEquipped(ip->ItemId))
                    {
                        int i;

                        for (i = 0; i < nuniques; i++)
                        {
                            if (uniques[i] == ip->ItemId)
                            {
                                // found a duplicate unique-equipped gem, remove it
                                it->RemoveEnchantment(2 + count);
                                break;
                            }
                        }

                        if (i == nuniques)  // not found
                            uniques[nuniques++] = ip->ItemId;
                    }
                }
            }
        }
    }
}

void Player::SetPersistentInstanceId(Instance* pInstance)
{
    if (pInstance == nullptr)
        return;
    // Skip this handling for flagged GMs.
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
        return;
    // Bind instance to "my" group.
    if (m_playerInfo && m_playerInfo->m_Group && pInstance->m_creatorGroup == 0)
        pInstance->m_creatorGroup = m_playerInfo && m_playerInfo->m_Group->GetID();
    // Skip handling for non-persistent instances.
    if (!IS_PERSISTENT_INSTANCE(pInstance))
        return;
    // Set instance for group if not done yet.
    if (m_playerInfo && m_playerInfo->m_Group && (m_playerInfo->m_Group->m_instanceIds[pInstance->m_mapId][pInstance->m_difficulty] == 0 || !sInstanceMgr.InstanceExists(pInstance->m_mapId, m_playerInfo->m_Group->m_instanceIds[pInstance->m_mapId][pInstance->m_difficulty])))
    {
        m_playerInfo->m_Group->m_instanceIds[pInstance->m_mapId][pInstance->m_difficulty] = pInstance->m_instanceId;
        m_playerInfo->m_Group->SaveToDB();
    }
    // Instance is not saved yet (no bosskill)
    if (!pInstance->m_persistent)
    {
        SetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty, 0);
    }
    // Set instance id to player.
    else
    {
        SetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty, pInstance->m_instanceId);
    }
    LOG_DEBUG("Added player %u to saved instance %u on map %u.", (uint32)GetGUID(), pInstance->m_instanceId, pInstance->m_mapId);
}

void Player::SetPersistentInstanceId(uint32 mapId, uint8 difficulty, uint32 instanceId)
{
    if (mapId >= NUM_MAPS || difficulty >= NUM_INSTANCE_MODES || m_playerInfo == nullptr)
        return;
    m_playerInfo->savedInstanceIdsLock.Acquire();
    PlayerInstanceMap::iterator itr = m_playerInfo->savedInstanceIds[difficulty].find(mapId);
    if (itr == m_playerInfo->savedInstanceIds[difficulty].end())
    {
        if (instanceId != 0)
            m_playerInfo->savedInstanceIds[difficulty].insert(PlayerInstanceMap::value_type(mapId, instanceId));
    }
    else
    {
        if (instanceId == 0)
            m_playerInfo->savedInstanceIds[difficulty].erase(itr);
        else
            (*itr).second = instanceId;
    }
    m_playerInfo->savedInstanceIdsLock.Release();
    CharacterDatabase.Execute("DELETE FROM instanceids WHERE playerguid = %u AND mapid = %u AND mode = %u;", m_playerInfo->guid, mapId, difficulty);
    CharacterDatabase.Execute("INSERT INTO instanceids (playerguid, mapid, mode, instanceid) VALUES (%u, %u, %u, %u)", m_playerInfo->guid, mapId, difficulty, instanceId);
}

void Player::RolloverHonor()
{
    uint32 current_val = (g_localTime.tm_year << 16) | g_localTime.tm_yday;
    if (current_val != m_honorRolloverTime)
    {
        m_honorRolloverTime = current_val;
        m_honorYesterday = m_honorToday;
        m_killsYesterday = m_killsToday;
        m_honorToday = m_killsToday = 0;
    }
}

void Player::_LoadQuestLogEntry(QueryResult* result)
{
    QuestLogEntry* entry;
    QuestProperties const* quest;
    Field* fields;
    uint32 questid;

    // clear all fields
    for (uint8 i = 0; i < 25; ++i)
    {
        uint16_t baseindex = PLAYER_QUEST_LOG_1_1 + (i * 5);
        setUInt32Value(baseindex + 0, 0);
        setUInt32Value(baseindex + 1, 0);
        setUInt64Value(baseindex + 2, 0);
        setUInt32Value(baseindex + 4, 0);
    }

    uint16 slot = 0;

    if (result)
    {
        do
        {
            fields = result->Fetch();
            questid = fields[1].GetUInt32();
            quest = sMySQLStore.getQuestProperties(questid);
            slot = fields[2].GetUInt16();

            // remove on next save if bad quest
            if (!quest)
            {
                m_removequests.insert(questid);
                continue;
            }
            if (m_questlog[slot] != nullptr)
                continue;

            entry = new QuestLogEntry;
            entry->Init(quest, this, slot);
            entry->LoadFromDB(fields);
            entry->UpdatePlayerFields();

        }
        while (result->NextRow());
    }
}

QuestLogEntry* Player::GetQuestLogForEntry(uint32 quest)
{
    for (uint8 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        if (m_questlog[i] != nullptr)
        {
            if (m_questlog[i]->GetQuest()->id == quest)
                return m_questlog[i];
        }
    }
    return nullptr;
}

void Player::SetQuestLogSlot(QuestLogEntry* entry, uint32 slot)
{
    m_questlog[slot] = entry;
}

void Player::AddToWorld()
{
    auto transport = this->GetTransport();
    if (transport)
    {
        this->SetPosition(transport->GetPositionX() + GetTransPositionX(),
            transport->GetPositionY() + GetTransPositionY(),
            transport->GetPositionZ() + GetTransPositionZ(),
            GetOrientation(), false);
    }

    // If we join an invalid instance and get booted out, this will prevent our stats from doubling :P
    if (IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld();

    // Add failed.
    if (m_mapMgr == nullptr)
    {
        // eject from instance
        m_beingPushed = false;
        EjectFromInstance();
        return;
    }

    if (m_session)
        m_session->SetInstance(m_mapMgr->GetInstanceID());

#if VERSION_STRING > TBC
    SendInstanceDifficulty(m_mapMgr->iInstanceMode);
#endif
}

void Player::AddToWorld(MapMgr* pMapMgr)
{
    // check transporter
    auto transport = this->GetTransport();
    if (transport != nullptr)
    {
        auto t_loc = transport->GetPosition();
        this->SetPosition(t_loc.x + this->GetTransPositionX(),
            t_loc.y + this->GetTransPositionY(),
            t_loc.z + this->GetTransPositionZ(),
            this->GetOrientation(), false);
    }

    // If we join an invalid instance and get booted out, this will prevent our stats from doubling :P
    if (IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld(pMapMgr);

    // Add failed.
    if (m_mapMgr == nullptr)
    {
        // eject from instance
        m_beingPushed = false;
        EjectFromInstance();
        return;
    }

    if (m_session)
        m_session->SetInstance(m_mapMgr->GetInstanceID());

#if VERSION_STRING > TBC
    SendInstanceDifficulty(m_mapMgr->iInstanceMode);
#endif
}

void Player::OnPrePushToWorld()
{
    SendInitialLogonPackets();
#if VERSION_STRING > TBC
    m_achievementMgr.SendAllAchievementData(this);
#endif
}

void Player::OnPushToWorld()
{
    uint8 class_ = getClass();
    uint8 startlevel = 1;

    // Process create packet
    ProcessPendingUpdates();

    if (m_TeleportState == 2)   // Worldport Ack
        OnWorldPortAck();

    SpeedCheatReset();
    m_beingPushed = false;
    AddItemsToWorld();

    // delay the unlock movement packet
    WorldPacket* data = new WorldPacket(SMSG_TIME_SYNC_REQ, 4);
    *data << uint32(0);
    delayedPackets.add(data);

    // set fly if cheat is active
    setMoveCanFly(FlyCheat);

    // Update PVP Situation
    LoginPvPSetup();
    removeByteFlag(UNIT_FIELD_BYTES_2, 1, 0x28);

    if (m_playerInfo->lastOnline + 900 < UNIXTIME)    // did we logged out for more than 15 minutes?
        m_ItemInterface->RemoveAllConjured();

    Unit::OnPushToWorld();

    if (m_FirstLogin)
    {
        if (class_ == DEATHKNIGHT)
            startlevel = static_cast<uint8>(std::max(55, worldConfig.player.playerStartingLevel));
        else startlevel = static_cast<uint8>(worldConfig.player.playerStartingLevel);

        sHookInterface.OnFirstEnterWorld(this);
        LevelInfo* Info = objmgr.GetLevelInfo(getRace(), getClass(), startlevel);
        ApplyLevelInfo(Info, startlevel);
        m_FirstLogin = false;
    }

    sHookInterface.OnEnterWorld(this);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnZoneChange)(this, m_zoneId, 0);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnPlayerEnter)(this);

    if (m_TeleportState == 1)        // First world enter
        CompleteLoading();

    m_TeleportState = 0;

    if (GetTaxiState())
    {
        if (m_taxiMapChangeNode != 0)
        {
            lastNode = m_taxiMapChangeNode;
        }

        TaxiStart(GetTaxiPath(),
                  GetMount(),
                  lastNode);

        m_taxiMapChangeNode = 0;
    }

    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
        // can only fly in outlands or northrend (northrend requires cold weather flying)
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    /* send weather */
    sWeatherMgr.SendWeather(this);

    SetHealth((load_health > m_uint32Values[UNIT_FIELD_MAXHEALTH] ? m_uint32Values[UNIT_FIELD_MAXHEALTH] : load_health));
    SetPower(POWER_TYPE_MANA, (load_mana > GetMaxPower(POWER_TYPE_MANA) ? GetMaxPower(POWER_TYPE_MANA) : load_mana));

    if (!GetSession()->HasGMPermissions())
        GetItemInterface()->CheckAreaItems();

    if (m_mapMgr && m_mapMgr->m_battleground != nullptr && m_bg != m_mapMgr->m_battleground)
    {
        m_mapMgr->m_battleground->PortPlayer(this, true);
    }

    if (m_bg != nullptr)
    {
        m_bg->OnAddPlayer(this);   // add buffs and so, must be after zone update and related aura removal
        m_bg->OnPlayerPushed(this);
    }

    m_changingMaps = false;
    SendFullAuraUpdate();

    m_ItemInterface->HandleItemDurations();

    SendInitialWorldstates();

    if (resettalents)
    {
        Reset_AllTalents();
        resettalents = false;
    }
}

void Player::RemoveFromWorld()
{
    if (raidgrouponlysent)
        event_RemoveEvents(EVENT_PLAYER_EJECT_FROM_INSTANCE);

    load_health = GetHealth();
    load_mana = GetPower(POWER_TYPE_MANA);

    if (m_bg)
    {
        m_bg->RemovePlayer(this, true);
    }

#if VERSION_STRING != Cata
    // Cancel trade if it's active.
    Player* pTarget;
    if (mTradeTarget != 0)
    {
        pTarget = GetTradeTarget();
        if (pTarget)
            pTarget->ResetTradeVariables();

        ResetTradeVariables();
    }
#else
    if (m_TradeData != nullptr)
        cancelTrade(false);
#endif

    //stop dueling
    if (DuelingWith != nullptr)
        DuelingWith->EndDuel(DUEL_WINNER_RETREAT);

    //clear buyback
    GetItemInterface()->EmptyBuyBack();

    ClearSplinePackets();


    summonhandler.RemoveAllSummons();
    DismissActivePets();
    RemoveFieldSummon();



    if (m_SummonedObject)
    {
        if (m_SummonedObject->GetInstanceID() != GetInstanceID())
        {
            sEventMgr.AddEvent(m_SummonedObject, &Object::Delete, EVENT_GAMEOBJECT_EXPIRE, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }
        else
        {
            if (m_SummonedObject->IsInWorld())
            {
                m_SummonedObject->RemoveFromWorld(true);
            }
            ARCEMU_ASSERT(m_SummonedObject->IsGameObject());
            delete m_SummonedObject;
        }
        m_SummonedObject = nullptr;
    }

    if (IsInWorld())
    {
        RemoveItemsFromWorld();
        Unit::RemoveFromWorld(false);
    }

    if (GetTaxiState())
        event_RemoveEvents(EVENT_PLAYER_TAXI_INTERPOLATE);

    m_changingMaps = true;
    m_playerInfo->lastOnline = UNIXTIME; // don't destroy conjured items yet
}

///\todo perhaps item should just have a list of mods, that will simplify code
void Player::_ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown /* = false */, bool skip_stat_apply /* = false  */)
{
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    ARCEMU_ASSERT(item != NULL);
    ItemProperties const* proto = item->GetItemProperties();

    //fast check to skip mod applying if the item doesnt meat the requirements.
    if (!item->IsContainer() && item->GetDurability() == 0 && item->GetDurabilityMax() && justdrokedown == false)
    {
        return;
    }

    //check for rnd prop
    item->ApplyRandomProperties(true);

    //Items Set check
    int32 proto_setid = proto->ItemSet;

    int32 setid = 0;
    // These season pvp itemsets are interchangeable and each set group has the same
    // bonuses if you have a full set made up of parts from any of the 3 similar sets
    // you will get the highest sets bonus
    if (proto_setid < 0)
    {
        if (sMySQLStore.getItemSetLinkedBonus(proto_setid) != 0)
        {
            setid = sMySQLStore.getItemSetLinkedBonus(proto_setid);
        }
    }
    else
    {
        setid = proto_setid;
    }


    ///\todo make a config for server so they can configure which season is active season

    // Set
    if (setid != 0)
    {
        bool is_new_created = false;
        auto item_set_entry = sItemSetStore.LookupEntry(setid);
        if (item_set_entry == nullptr)
        {
            LOG_ERROR("Item %u has wrong ItemSet %u", proto->ItemId, setid);
        }
        else
        {
            ItemSet* Set = nullptr;
            std::list<ItemSet>::iterator i;
            for (i = m_itemsets.begin(); i != m_itemsets.end(); ++i)
            {
                if (i->setid == setid)
                {
                    Set = &(*i);
                    break;
                }
            }

            if (apply)
            {
                if (Set == nullptr)
                {
                    Set = new ItemSet;
                    memset(Set, 0, sizeof(ItemSet));
                    Set->itemscount = 1;
                    Set->setid = setid;
                    is_new_created = true;
                }
                else
                    Set->itemscount++;

                if (!item_set_entry->RequiredSkillID || (_GetSkillLineCurrent(item_set_entry->RequiredSkillID, true) >= item_set_entry->RequiredSkillAmt))
                {
                    for (uint8 x = 0; x < 8; x++)
                    {
                        if (Set->itemscount == item_set_entry->itemscount[x])
                        {
                            //cast new spell
                            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(item_set_entry->SpellID[x]);
                            Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, nullptr);
                            SpellCastTargets targets;
                            targets.m_unitTarget = this->GetGUID();
                            spell->prepare(&targets);
                        }
                    }
                }

                if (i == m_itemsets.end())
                {
                    m_itemsets.push_back(*Set);
                }
            }
            else
            {
                if (Set)
                {
                    for (uint8 x = 0; x < 8; x++)
                        if (Set->itemscount == item_set_entry->itemscount[x])
                        {
                            this->RemoveAura(item_set_entry->SpellID[x], GetGUID());
                        }

                    if (!(--Set->itemscount))
                        m_itemsets.erase(i);
                }
            }
            if (is_new_created)
                delete Set;
        }
    }

    // Resistances
    ///\todo FIX ME: can there be negative resistances from items?
    if (proto->FireRes)
    {
        if (apply)
            FlatResistanceModifierPos[2] += proto->FireRes;
        else
            FlatResistanceModifierPos[2] -= proto->FireRes;
        CalcResistance(2);
    }

    if (proto->NatureRes)
    {
        if (apply)
            FlatResistanceModifierPos[3] += proto->NatureRes;
        else
            FlatResistanceModifierPos[3] -= proto->NatureRes;
        CalcResistance(3);
    }

    if (proto->FrostRes)
    {
        if (apply)
            FlatResistanceModifierPos[4] += proto->FrostRes;
        else
            FlatResistanceModifierPos[4] -= proto->FrostRes;
        CalcResistance(4);
    }

    if (proto->ShadowRes)
    {
        if (apply)
            FlatResistanceModifierPos[5] += proto->ShadowRes;
        else
            FlatResistanceModifierPos[5] -= proto->ShadowRes;
        CalcResistance(5);
    }

    if (proto->ArcaneRes)
    {
        if (apply)
            FlatResistanceModifierPos[6] += proto->ArcaneRes;
        else
            FlatResistanceModifierPos[6] -= proto->ArcaneRes;
        CalcResistance(6);
    }

#if VERSION_STRING > TBC
    /* Heirloom scaling items */
    if (proto->ScalingStatsEntry != 0)
    {
        int i = 0;
        auto scaling_stat_distribution = sScalingStatDistributionStore.LookupEntry(proto->ScalingStatsEntry);
        DBC::Structures::ScalingStatValuesEntry const* ssvrow = nullptr;
        uint32 StatType;
        uint32 StatMod;
        uint32 plrLevel = getLevel();
        int32 StatMultiplier;
        int32 StatValue;
        int32 col = 0;

        // this is needed because the heirloom items don't scale over lvl80
        if (plrLevel > DBC_PLAYER_LEVEL_CAP)
            plrLevel = DBC_PLAYER_LEVEL_CAP;

        for (uint32 id = 0; id < sScalingStatValuesStore.GetNumRows(); ++id)
        {
            auto scaling_stat_values = sScalingStatValuesStore.LookupEntry(id);
            if (scaling_stat_values == nullptr)
                continue;

            if (scaling_stat_values->level == plrLevel)
            {
                ssvrow = scaling_stat_values;
                break;
            }
        }
        /* Not going to put a check here since unless you put a random id/flag in the tables these should never return NULL */

        /* Calculating the stats correct for our level and applying them */
        for (i = 0; scaling_stat_distribution->stat[i] != -1; ++i)
        {
            StatType = scaling_stat_distribution->stat[i];
            StatMod = scaling_stat_distribution->statmodifier[i];
            col = GetStatScalingStatValueColumn(proto, SCALINGSTATSTAT);
            if (col == -1)
                continue;

            if (ssvrow == nullptr)
                continue;

            StatMultiplier = ssvrow->multiplier[col];
            StatValue = StatMod * StatMultiplier / 10000;
            ModifyBonuses(StatType, StatValue, apply);
        }

        if ((proto->ScalingStatsFlag & 32768) && i < 10)
        {
            StatType = scaling_stat_distribution->stat[i];
            StatMod = scaling_stat_distribution->statmodifier[i];
            col = GetStatScalingStatValueColumn(proto, SCALINGSTATSPELLPOWER);
            if (col != -1)
            {
                if (ssvrow != nullptr)
                {
                    StatMultiplier = ssvrow->multiplier[col];
                    StatValue = StatMod * StatMultiplier / 10000;
                    ModifyBonuses(45, StatValue, apply);
                }
            }
        }

        /* Calculating the Armor correct for our level and applying it */
        col = GetStatScalingStatValueColumn(proto, SCALINGSTATARMOR);
        if (col != -1)
        {
            if (ssvrow != nullptr)
            {
                uint32 scaledarmorval = ssvrow->multiplier[col];

                if (apply)
                    BaseResistance[0] += scaledarmorval;
                else
                    BaseResistance[0] -= scaledarmorval;

                CalcResistance(0);
            }
        }

        /* Calculating the damages correct for our level and applying it */
        col = GetStatScalingStatValueColumn(proto, SCALINGSTATDAMAGE);
        if (col != -1)
        {
            uint32 scaleddps;

            if (ssvrow != nullptr)
            {
                scaleddps = ssvrow->multiplier[col];
            }
            else
            {
                scaleddps = 1;
            }

            float dpsmod = 1.0;

            if (proto->ScalingStatsFlag & 0x1400)
                dpsmod = 0.2f;
            else
                dpsmod = 0.3f;

            float scaledmindmg = (scaleddps - (scaleddps * dpsmod)) * (proto->Delay / 1000);
            float scaledmaxdmg = (scaleddps * (dpsmod + 1.0f)) * (proto->Delay / 1000);

            if (proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT || proto->InventoryType == INVTYPE_THROWN)
            {
                BaseRangedDamage[0] += apply ? scaledmindmg : -scaledmindmg;
                BaseRangedDamage[1] += apply ? scaledmaxdmg : -scaledmaxdmg;
            }
            else
            {
                if (slot == EQUIPMENT_SLOT_OFFHAND)
                {
                    BaseOffhandDamage[0] = apply ? scaledmindmg : 0;
                    BaseOffhandDamage[1] = apply ? scaledmaxdmg : 0;
                }
                else
                {
                    BaseDamage[0] = apply ? scaledmindmg : 0;
                    BaseDamage[1] = apply ? scaledmaxdmg : 0;
                }
            }
        }

        /* Normal items */
    }
    else
#endif
    {
        // Stats
        for (uint8 i = 0; i < proto->itemstatscount; ++i)
        {
            int32 val = proto->Stats[i].Value;
            /*
            if (val == 0)
            continue;
            */
            ModifyBonuses(proto->Stats[i].Type, val, apply);
        }

        // Armor
        if (proto->Armor)
        {
            if (apply)BaseResistance[0] += proto->Armor;
            else  BaseResistance[0] -= proto->Armor;
            CalcResistance(0);
        }

        // Damage
        if (proto->Damage[0].Min)
        {
            if (proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT || proto->InventoryType == INVTYPE_THROWN)
            {
                BaseRangedDamage[0] += apply ? proto->Damage[0].Min : -proto->Damage[0].Min;
                BaseRangedDamage[1] += apply ? proto->Damage[0].Max : -proto->Damage[0].Max;
            }
            else
            {
                if (slot == EQUIPMENT_SLOT_OFFHAND)
                {
                    BaseOffhandDamage[0] = apply ? proto->Damage[0].Min : 0;
                    BaseOffhandDamage[1] = apply ? proto->Damage[0].Max : 0;
                }
                else
                {
                    BaseDamage[0] = apply ? proto->Damage[0].Min : 0;
                    BaseDamage[1] = apply ? proto->Damage[0].Max : 0;
                }
            }
        }
    } // end of the scalingstats else branch

    if (this->getClass() == DRUID && slot == EQUIPMENT_SLOT_MAINHAND)
    {
        uint8 ss = GetShapeShift();
        if (ss == FORM_MOONKIN || ss == FORM_CAT || ss == FORM_BEAR || ss == FORM_DIREBEAR)
            this->ApplyFeralAttackPower(apply, item);
    }

    // Misc
    if (apply)
    {
        // Apply all enchantment bonuses
        item->ApplyEnchantmentBonuses();

        SpellInfo* spells;
        for (uint8 k = 0; k < 5; ++k)
        {
            if (item->GetItemProperties()->Spells[k].Id == 0)
                continue;//this isn't needed since the check below handles this case but it's a lot faster performance-wise.

            spells = sSpellCustomizations.GetSpellInfo(item->GetItemProperties()->Spells[k].Id);
            if (spells == nullptr)
                continue;

            if (item->GetItemProperties()->Spells[k].Trigger == ON_EQUIP)
            {
                if (spells->getRequiredShapeShift())
                {
                    AddShapeShiftSpell(spells->getId());
                    continue;
                }

                Spell* spell = sSpellFactoryMgr.NewSpell(this, spells, true, nullptr);
                SpellCastTargets targets;
                targets.m_unitTarget = this->GetGUID();
                spell->castedItemId = item->GetEntry();
                spell->prepare(&targets);

            }
            else if (item->GetItemProperties()->Spells[k].Trigger == CHANCE_ON_HIT)
            {
                this->AddProcTriggerSpell(spells, nullptr, this->GetGUID(), 5, PROC_ON_MELEE_ATTACK, 0, nullptr, nullptr);
            }
        }
    }
    else
    {
        // Remove all enchantment bonuses
        item->RemoveEnchantmentBonuses();
        for (uint8 k = 0; k < 5; ++k)
        {
            if (item->GetItemProperties()->Spells[k].Trigger == ON_EQUIP)
            {
                SpellInfo* spells = sSpellCustomizations.GetSpellInfo(item->GetItemProperties()->Spells[k].Id);
                if (spells != nullptr)
                {
                    if (spells->getRequiredShapeShift())
                        RemoveShapeShiftSpell(spells->getId());
                    else
                        RemoveAura(item->GetItemProperties()->Spells[k].Id);
                }
            }
            else if (item->GetItemProperties()->Spells[k].Trigger == CHANCE_ON_HIT)
            {
                this->RemoveProcTriggerSpell(item->GetItemProperties()->Spells[k].Id);
            }
        }
    }

    if (!apply)   // force remove auras added by using this item
    {
        for (uint32 k = MAX_POSITIVE_AURAS_EXTEDED_START; k < MAX_POSITIVE_AURAS_EXTEDED_END; ++k)
        {
            Aura* m_aura = this->m_auras[k];
            if (m_aura != nullptr && m_aura->m_castedItemId && m_aura->m_castedItemId == proto->ItemId)
                m_aura->Remove();
        }
    }

    if (!skip_stat_apply)
        UpdateStats();
}

void Player::BuildPlayerRepop()
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_PRE_RESURRECT, 8);
    FastGUIDPack(data, GetGUID());         // caster guid
    GetSession()->SendPacket(&data);

    // Cleanup first
    uint32 AuraIds[] = { 20584, 9036, 8326, 0 };
    RemoveAuras(AuraIds); // Cebernic: Removeaura just remove once(bug?).

    SetHealth(1);

    SpellCastTargets tgt;
    tgt.m_unitTarget = this->GetGUID();

    if (getRace() == RACE_NIGHTELF)
    {
        SpellInfo* inf = sSpellCustomizations.GetSpellInfo(9036);
        Spell* sp = sSpellFactoryMgr.NewSpell(this, inf, true, nullptr);
        sp->prepare(&tgt);
    }
    else
    {
        SpellInfo* inf = sSpellCustomizations.GetSpellInfo(8326);
        Spell* sp = sSpellFactoryMgr.NewSpell(this, inf, true, nullptr);
        sp->prepare(&tgt);
    }

    StopMirrorTimer(MIRROR_TYPE_FATIGUE);
    StopMirrorTimer(MIRROR_TYPE_BREATH);
    StopMirrorTimer(MIRROR_TYPE_FIRE);

    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);

    setMoveRoot(false);
    setMoveWaterWalk();
#endif
}

void Player::RepopRequestedPlayer()
{
    //if (HasAuraWithName(SPELL_AURA_PREVENT_RESURRECTION))
    //	return;

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHECKFORCHEATS); // cebernic:-> Remove this first
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FORCED_RESURRECT);   //in case somebody resurrected us before this event happened

    if (myCorpseInstanceId != 0)
    {
        // Cebernic: wOOo dead+dead = undead ? :D just resurrect player
        Corpse* myCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
        if (myCorpse != nullptr)
            myCorpse->ResetDeathClock();
        ResurrectPlayer();
        RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return;
    }

    auto transport = this->GetTransport();
    if (transport != nullptr)
    {
        transport->RemovePassenger(this);
#if VERSION_STRING != Cata
        this->obj_movement_info.transporter_info.guid = 0;
#else
        this->obj_movement_info.clearTransportData();
#endif

        //ResurrectPlayer();
        RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return;
    }

    MySQLStructure::MapInfo const* pMapinfo = nullptr;

    // Set death state to corpse, that way players will lose visibility
    setDeathState(CORPSE);

    // Update visibility, that way people wont see running corpses :P
    UpdateVisibility();

    // If we're in battleground, remove the skinnable flag.. has bad effects heheh
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    bool corpse = (m_bg != nullptr) ? m_bg->CreateCorpse(this) : true;

    if (corpse)
        CreateCorpse();


    BuildPlayerRepop();


    // Cebernic: don't do this.
    if (!m_bg || (m_bg && m_bg->HasStarted()))
    {
        pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());
        if (pMapinfo != nullptr)
        {
            if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
            {
                RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
            }
            else
            {
                RepopAtGraveyard(pMapinfo->repopx, pMapinfo->repopy, pMapinfo->repopz, pMapinfo->repopmapid);
            }
            switch (pMapinfo->mapid)
            {
                case 533: // Naxx
                case 550: // The Eye
                case 552: // The Arcatraz
                case 553: // The Botanica
                case 554: // The Mechanar
                    ResurrectPlayer();
                    return;
            }

        }
        else
        {
            // RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
            // Cebernic: Mapinfo NULL? let's search from bindposition.
            RepopAtGraveyard(GetBindPositionX(), GetBindPositionY(), GetBindPositionZ(), GetBindMapId());
        }
    }

    if (corpse)
    {
        SpawnCorpseBody();

        if (myCorpseInstanceId != 0)
        {
            Corpse* myCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
            if (myCorpse != nullptr)
                myCorpse->ResetDeathClock();
        }

        /* Send Spirit Healer Location */
        WorldPacket data(SMSG_DEATH_RELEASE_LOC, 16);

        data << m_mapId;
        data << m_position;

        m_session->SendPacket(&data);

        /* Corpse reclaim delay */
        WorldPacket data2(SMSG_CORPSE_RECLAIM_DELAY, 4);
        data2 << uint32(CORPSE_RECLAIM_TIME_MS);
        m_session->SendPacket(&data2);
    }

}

void Player::ResurrectPlayer()
{
    if (!sHookInterface.OnResurrect(this))
        return;

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FORCED_RESURRECT); // In case somebody resurrected us before this event happened
    if (m_resurrectHealth)
        SetHealth((uint32)std::min(m_resurrectHealth, m_uint32Values[UNIT_FIELD_MAXHEALTH]));
    if (m_resurrectMana)
        SetPower(POWER_TYPE_MANA, m_resurrectMana);

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();

    RemoveNegativeAuras();
    uint32 AuraIds[] = { 20584, 9036, 8326, 55164, 0 };
    RemoveAuras(AuraIds); // Cebernic: removeaura just remove once(bug?).

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);
    setDeathState(ALIVE);
    UpdateVisibility();
    if (m_resurrecter && IsInWorld()
        // Don't pull players inside instances with this trick. Also fixes the part where you were able to double item bonuses
        && m_resurrectInstanceID == static_cast<uint32>(GetInstanceID()))
    {
        SafeTeleport(m_resurrectMapId, m_resurrectInstanceID, m_resurrectPosition);
    }
    m_resurrecter = 0;
    setMoveLandWalk();

    // Zack : shit on grill. So auras should be removed on player death instead of making this :P
    // We can afford this bullshit atm since auras are lost upon death -> no immunities
    for (uint8 i = 0; i < 7; i++)
        SchoolImmunityList[i] = 0;

    SpawnActivePet();

    if (m_bg != nullptr)
        m_bg->HookOnPlayerResurrect(this);
}

void Player::KillPlayer()
{
    if (getDeathState() != ALIVE) //You can't kill what has no life.   - amg south park references ftw :P
        return;
    setDeathState(JUST_DIED);

    // Battleground stuff
    if (m_bg)
        m_bg->HookOnPlayerDeath(this);

    EventDeath();

    m_session->OutPacket(SMSG_CANCEL_COMBAT);
    m_session->OutPacket(SMSG_CANCEL_AUTO_REPEAT);

    setMoveRoot(true);
    StopMirrorTimer(MIRROR_TYPE_FATIGUE);
    StopMirrorTimer(MIRROR_TYPE_BREATH);
    StopMirrorTimer(MIRROR_TYPE_FIRE);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE); // Player death animation, also can be used with DYNAMIC_FLAGS <- huh???
    setUInt32Value(UNIT_DYNAMIC_FLAGS, 0x00);

    if (getClass() == WARRIOR)   // Rage resets on death
        SetPower(POWER_TYPE_RAGE, 0);
    else if (getClass() == DEATHKNIGHT)
        SetPower(POWER_TYPE_RUNIC_POWER, 0);

    summonhandler.RemoveAllSummons();
    DismissActivePets();

    // Player falls off vehicle on death
    if (currentvehicle != nullptr)
        currentvehicle->EjectPassenger(this);

    sHookInterface.OnDeath(this);

}

void Player::CreateCorpse()
{
    Corpse* pCorpse;
    uint32 _pb, _pb2, _cfb1, _cfb2;

    objmgr.DelinkPlayerCorpses(this);
    if (!bCorpseCreateable)
    {
        bCorpseCreateable = true;   // For next time
        return; // No corpse allowed!
    }

    pCorpse = objmgr.CreateCorpse();
    pCorpse->SetInstanceID(GetInstanceID());
    pCorpse->Create(this, GetMapId(), GetPositionX(),
                    GetPositionY(), GetPositionZ(), GetOrientation());

    _pb = getUInt32Value(PLAYER_BYTES);
    _pb2 = getUInt32Value(PLAYER_BYTES_2);

    uint8 race = getRace();
    uint8 skin = (uint8)(_pb);
    uint8 face = (uint8)(_pb >> 8);
    uint8 hairstyle = (uint8)(_pb >> 16);
    uint8 haircolor = (uint8)(_pb >> 24);
    uint8 facialhair = (uint8)(_pb2);

    _cfb1 = ((0x00) | (race << 8) | (0x00 << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    pCorpse->SetZoneId(GetZoneId());
    pCorpse->setUInt32Value(CORPSE_FIELD_BYTES_1, _cfb1);
    pCorpse->setUInt32Value(CORPSE_FIELD_BYTES_2, _cfb2);
    pCorpse->setUInt32Value(CORPSE_FIELD_FLAGS, 4);
    pCorpse->SetDisplayId(GetDisplayId());

    if (m_bg)
    {
        // Remove our lootable flags
        RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_LOOTABLE);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        loot.gold = 0;

        pCorpse->generateLoot();
        if (bShouldHaveLootableOnCorpse)
        {
            pCorpse->setUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, 1); // sets it so you can loot the plyr
        }
        else
        {
            // Hope this works
            pCorpse->setUInt32Value(CORPSE_FIELD_FLAGS, 60);
        }

        // Now that our corpse is created, don't do it again
        bShouldHaveLootableOnCorpse = false;
    }
    else
    {
        pCorpse->loot.gold = 0;
    }

    uint32 iDisplayID = 0;
    uint16 iIventoryType = 0;
    uint32 _cfi = 0;
    Item* pItem;
    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        if ((pItem = GetItemInterface()->GetInventoryItem(i)) != nullptr)
        {
            iDisplayID = pItem->GetItemProperties()->DisplayInfoID;
            iIventoryType = (uint16)pItem->GetItemProperties()->InventoryType;

            _cfi = (uint16(iDisplayID)) | (iIventoryType) << 24;
            pCorpse->setUInt32Value(CORPSE_FIELD_ITEM + i, _cfi);
        }
    }
    // Save corpse in db for future use
    pCorpse->SaveToDB();
}

void Player::SpawnCorpseBody()
{
    Corpse* pCorpse;

    pCorpse = objmgr.GetCorpseByOwner(this->GetLowGUID());
    if (pCorpse)
    {
        if (!pCorpse->IsInWorld())
        {
            if (bShouldHaveLootableOnCorpse && pCorpse->getUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS) != 1)
                pCorpse->setUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, 1); // sets it so you can loot the plyr

            if (m_mapMgr == nullptr)
                pCorpse->AddToWorld();
            else
                pCorpse->PushToWorld(m_mapMgr);
        }
        myCorpseLocation = pCorpse->GetPosition();
        myCorpseInstanceId = pCorpse->GetInstanceID();
    }
    else
    {
        myCorpseLocation.ChangeCoords(0, 0, 0, 0);
        myCorpseInstanceId = 0;
    }
}

void Player::SpawnCorpseBones()
{
    Corpse* pCorpse;
    pCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
    myCorpseLocation.ChangeCoords(0, 0, 0, 0);
    myCorpseInstanceId = 0;
    if (pCorpse)
    {
        if (pCorpse->IsInWorld() && pCorpse->GetCorpseState() == CORPSE_STATE_BODY)
        {
            if (pCorpse->GetInstanceID() != GetInstanceID())
            {
                sEventMgr.AddEvent(pCorpse, &Corpse::SpawnBones, EVENT_CORPSE_SPAWN_BONES, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
                pCorpse->SpawnBones();
        }
        else
        {
            //Cheater!
        }
    }
}

void Player::DeathDurabilityLoss(double percent)
{
    m_session->OutPacket(SMSG_DURABILITY_DAMAGE_DEATH);
    uint32 pDurability;
    uint32 pMaxDurability;
    int32 pNewDurability;
    Item* pItem;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if ((pItem = GetItemInterface()->GetInventoryItem(i)) != nullptr)
        {
            pMaxDurability = pItem->GetDurabilityMax();
            pDurability = pItem->GetDurability();
            if (pDurability)
            {
                pNewDurability = (uint32)(pMaxDurability * percent);
                pNewDurability = (pDurability - pNewDurability);
                if (pNewDurability < 0)
                    pNewDurability = 0;

                if (pNewDurability <= 0)
                {
                    ApplyItemMods(pItem, i, false, true);
                }

                pItem->SetDurability(static_cast<uint32>(pNewDurability));
                pItem->m_isDirty = true;
            }
        }
    }
}

void Player::RepopAtGraveyard(float ox, float oy, float oz, uint32 mapid)
{
    if (HasAuraWithName(SPELL_AURA_PREVENT_RESURRECTION))
        return;

    bool first = true;
    // float closestX = 0, closestY = 0, closestZ = 0, closestO = 0;

    LocationVector src(ox, oy, oz);
    LocationVector dest;
    LocationVector temp;
    float closest_dist = 999999.0f;
    float dist;

    if (m_bg && m_bg->HookHandleRepop(this))
    {
        return;
    }
    else
    {
        MySQLStructure::Graveyards const* pGrave = nullptr;
        MySQLDataStore::GraveyardsContainer const* its = sMySQLStore.getGraveyardsStore();
        for (MySQLDataStore::GraveyardsContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            pGrave = sMySQLStore.getGraveyard(itr->second.id);
            if (pGrave->mapId == mapid && (pGrave->factionId == GetTeam() || pGrave->factionId == 3))
            {
                temp.ChangeCoords(pGrave->position_x, pGrave->position_y, pGrave->position_z);
                dist = src.distanceSquare(temp);
                if (first || dist < closest_dist)
                {
                    first = false;
                    closest_dist = dist;
                    dest = temp;
                }
            }
        }
        /* Fix on 3/13/2010, defaults to last graveyard, if none fit the criteria.
        Keeps the player from hanging out to dry.*/
        if (first && pGrave != nullptr)//crappy Databases with no graveyards.
        {
            dest.ChangeCoords(pGrave->position_x, pGrave->position_y, pGrave->position_z);
            first = false;
        }

    }

    if (sHookInterface.OnRepop(this) && !first)//dest has now always a value != {0,0,0,0}//but there may be DBs with no graveyards
    {
        SafeTeleport(mapid, 0, dest);
    }

    ///\todo Generate error message here, compensate for failed teleport.
}

void Player::JoinedChannel(Channel* c)
{
    if (c != nullptr)
        m_channels.insert(c);
}

void Player::LeftChannel(Channel* c)
{
    if (c != nullptr)
        m_channels.erase(c);
}

void Player::CleanupChannels()
{
    std::set<Channel*>::iterator i;
    Channel* c;
    for (i = m_channels.begin(); i != m_channels.end();)
    {
        c = *i;
        ++i;

        c->Part(this);
    }
}

void Player::SendInitialActions()
{
#if VERSION_STRING == Cata
    WorldPacket data(SMSG_ACTION_BUTTONS, (PLAYER_ACTION_BUTTON_COUNT * 4) + 1);
#else
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);
#endif

#if VERSION_STRING != Cata
    data << uint8(0);         // VLack: 3.1, some bool - 0 or 1. seems to work both ways
#endif

    for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        data << m_specs[m_talentActiveSpec].mActions[i].Action;
        data << m_specs[m_talentActiveSpec].mActions[i].Misc; // 3.3.5 Misc have to be sent before Type (buttons with value over 0xFFFF)
        data << m_specs[m_talentActiveSpec].mActions[i].Type;
    }
#if VERSION_STRING == Cata
    data << uint8(1);
#endif
    m_session->SendPacket(&data);
}

void Player::setAction(uint8 button, uint16 action, uint8 type, uint8 misc)
{
    if (button >= PLAYER_ACTION_BUTTON_COUNT)
        return;

    m_specs[m_talentActiveSpec].mActions[button].Action = action;
    m_specs[m_talentActiveSpec].mActions[button].Misc = misc;
    m_specs[m_talentActiveSpec].mActions[button].Type = type;
}

// Groupcheck
bool Player::IsGroupMember(Player* plyr)
{
    if (m_playerInfo->m_Group != nullptr)
        return m_playerInfo->m_Group->HasMember(plyr->m_playerInfo);

    return false;
}

uint16 Player::GetOpenQuestSlot()
{
    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
        if (m_questlog[i] == nullptr)
            return i;

    return MAX_QUEST_SLOT + 1;
}

void Player::AddToFinishedQuests(uint32 quest_id)
{
    if (m_finishedQuests.find(quest_id) != m_finishedQuests.end())
        return;

    m_finishedQuests.insert(quest_id);
}

bool Player::HasFinishedQuest(uint32 quest_id)
{
    return (m_finishedQuests.find(quest_id) != m_finishedQuests.end());
}


bool Player::HasTimedQuest()
{
    for (uint8 i = 0; i < 25; i++)
        if ((m_questlog[i] != nullptr) && (m_questlog[i]->GetQuest()->time != 0))
            return true;

    return false;
}


void Player::ClearQuest(uint32 id)
{
    m_finishedQuests.erase(id);
    m_finishedDailies.erase(id);
}

bool Player::GetQuestRewardStatus(uint32 quest_id)
{
    return HasFinishedQuest(quest_id);
}

void Player::_LoadTutorials(QueryResult* result)
{
    if (result)
    {
        Field* fields = result->Fetch();
        for (uint8 iI = 0; iI < 8; iI++)
            m_Tutorials[iI] = fields[iI + 1].GetUInt32();
    }
    tutorialsDirty = false;
}

void Player::_SaveTutorials(QueryBuffer* buf)
{
    if (tutorialsDirty)
    {
        if (buf == nullptr)
        {
            CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerid = %u;", GetLowGUID());
            CharacterDatabase.Execute("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", GetLowGUID(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
        }
        else
        {
            buf->AddQuery("DELETE FROM tutorials WHERE playerid = %u;", GetLowGUID());
            buf->AddQuery("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", GetLowGUID(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
        }

        tutorialsDirty = false;
    }
}

uint32 Player::GetTutorialInt(uint32 intId)
{
    ARCEMU_ASSERT(intId < 8);
    return m_Tutorials[intId];
}

void Player::SetTutorialInt(uint32 intId, uint32 value)
{
    if (intId >= 8)
        return;

    ARCEMU_ASSERT(intId < 8);
    m_Tutorials[intId] = value;
    tutorialsDirty = true;
}

float Player::GetDefenseChance(uint32 opLevel)
{
    float chance = _GetSkillLineCurrent(SKILL_DEFENSE, true) - (opLevel * 5.0f);
    chance += CalcRating(PCR_DEFENCE);
    chance = floorf(chance) * 0.04f;   // defense skill is treated as an integer on retail

    return chance;
}

#define BASE_BLOCK_CHANCE 5.0f
#define BASE_PARRY_CHANCE 5.0f

// Gets dodge chances before defense skill is applied
float Player::GetDodgeChance()
{
    uint32 pClass = (uint32)getClass();
    float chance = 0.0f;
    uint32 level = getLevel();

    if (level > worldConfig.player.playerGeneratedInformationByLevelCap)
        level = worldConfig.player.playerGeneratedInformationByLevelCap;

    // Base dodge + dodge from agility

    auto baseCrit = sGtChanceToMeleeCritBaseStore.LookupEntry(pClass - 1);
    auto CritPerAgi = sGtChanceToMeleeCritStore.LookupEntry(level - 1 + (pClass - 1) * 100);
    uint32 agi = GetStat(STAT_AGILITY);

    float tmp = 100.0f * (baseCrit->val + agi * CritPerAgi->val);
    tmp *= crit_to_dodge[pClass];
    chance += tmp;

    // Dodge from dodge rating
    chance += CalcRating(PCR_DODGE);

    // Dodge from spells
    chance += GetDodgeFromSpell();

    return std::max(chance, 0.0f); // Make sure we don't have a negative chance
}

// Gets block chances before defense skill is applied
// Assumes the caller will check for shields
float Player::GetBlockChance()
{
    float chance;

    // Base block chance
    chance = BASE_BLOCK_CHANCE;

    // Block rating
    chance += CalcRating(PCR_BLOCK);

    // Block chance from spells
    chance += GetBlockFromSpell();

    return std::max(chance, 0.0f);   // Make sure we don't have a negative chance
}

// Get parry chances before defense skill is applied
float Player::GetParryChance()
{
    float chance;

    // Base parry chance
    chance = BASE_PARRY_CHANCE;

    // Parry rating
    chance += CalcRating(PCR_PARRY);

    // Parry chance from spells
    chance += GetParryFromSpell();

    return std::max(chance, 0.0f);   // Make sure we don't have a negative chance
}

void Player::UpdateChances()
{
    uint32 pClass = (uint32)getClass();
    uint32 pLevel = (getLevel() > DBC_PLAYER_LEVEL_CAP) ? DBC_PLAYER_LEVEL_CAP : getLevel();

    float tmp = 0;
    float defence_contribution = 0;

    // Avoidance from defense skill
    defence_contribution = GetDefenseChance(pLevel);

    // Dodge
    tmp = GetDodgeChance();
    tmp += defence_contribution;
    tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    setFloatValue(PLAYER_DODGE_PERCENTAGE, tmp);

    // Block
    Item* it = this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it != nullptr && it->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        tmp = GetBlockChance();
        tmp += defence_contribution;
        tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    }
    else
        tmp = 0.0f;

    setFloatValue(PLAYER_BLOCK_PERCENTAGE, tmp);

    // Parry (can only parry with something in main hand)
    it = this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (it != nullptr)
    {
        tmp = GetParryChance();
        tmp += defence_contribution;
        tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    }
    else
        tmp = 0.0f;

    setFloatValue(PLAYER_PARRY_PERCENTAGE, tmp);

    // Critical
    auto baseCrit = sGtChanceToMeleeCritBaseStore.LookupEntry(pClass - 1);

    auto CritPerAgi = sGtChanceToMeleeCritStore.LookupEntry(pLevel - 1 + (pClass - 1) * 100);
    if (CritPerAgi == nullptr)
        CritPerAgi = sGtChanceToMeleeCritStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (pClass - 1) * 100);

    tmp = 100 * (baseCrit->val + GetStat(STAT_AGILITY) * CritPerAgi->val);

    float melee_bonus = 0;
    float ranged_bonus = 0;

    if (tocritchance.size() > 0)    // Crashfix by Cebernic
    {
        std::map< uint32, WeaponModifier >::iterator itr = tocritchance.begin();

        Item* tItemMelee = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        Item* tItemRanged = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);

        //-1 = any weapon

        for (; itr != tocritchance.end(); ++itr)
        {
            if (itr->second.wclass == (uint32)-1 || (tItemMelee != nullptr && (1 << tItemMelee->GetItemProperties()->SubClass & itr->second.subclass)))
            {
                melee_bonus += itr->second.value;
            }
            if (itr->second.wclass == (uint32)-1 || (tItemRanged != nullptr && (1 << tItemRanged->GetItemProperties()->SubClass & itr->second.subclass)))
            {
                ranged_bonus += itr->second.value;
            }
        }
    }

    float cr = tmp + CalcRating(PCR_MELEE_CRIT) + melee_bonus;
    setFloatValue(PLAYER_CRIT_PERCENTAGE, std::min(cr, 95.0f));

    float rcr = tmp + CalcRating(PCR_RANGED_CRIT) + ranged_bonus;
    setFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, std::min(rcr, 95.0f));

    auto SpellCritBase = sGtChanceToSpellCritBaseStore.LookupEntry(pClass - 1);

    auto SpellCritPerInt = sGtChanceToSpellCritStore.LookupEntry(pLevel - 1 + (pClass - 1) * 100);
    if (SpellCritPerInt == nullptr)
        SpellCritPerInt = sGtChanceToSpellCritStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (pClass - 1) * 100);

    spellcritperc = 100 * (SpellCritBase->val + GetStat(STAT_INTELLECT) * SpellCritPerInt->val) +
        this->GetSpellCritFromSpell() +
        this->CalcRating(PCR_SPELL_CRIT);
    UpdateChanceFields();
}

void Player::UpdateChanceFields()
{
#if VERSION_STRING != Classic
    // Update spell crit values in fields
    for (uint8 i = 0; i < 7; ++i)
    {
        setFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, SpellCritChanceSchool[i] + spellcritperc);
    }
#endif
}

void Player::ModAttackSpeed(int32 mod, ModType type)
{
    if (mod == 0)
        return;

    if (mod > 0)
        m_attack_speed[type] *= 1.0f + ((float)mod / 100.0f);
    else
        m_attack_speed[type] /= 1.0f + ((float)(-mod) / 100.0f);

    if (type == MOD_SPELL)
        SetCastSpeedMod(1.0f / (m_attack_speed[MOD_SPELL] * SpellHasteRatingBonus));
}

void Player::UpdateAttackSpeed()
{
    uint32 speed = 2000;
    Item* weap;

    if (GetShapeShift() == FORM_CAT)
    {
        speed = 1000;
    }
    else if (GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR)
    {
        speed = 2500;
    }
    else if (!disarmed)  // Regular
    {
        weap = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (weap != nullptr)
            speed = weap->GetItemProperties()->Delay;
    }
    SetBaseAttackTime(MELEE,
                      (uint32)((float)speed / (m_attack_speed[MOD_MELEE] * (1.0f + CalcRating(PCR_MELEE_HASTE) / 100.0f))));

    weap = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (weap != nullptr && weap->GetItemProperties()->Class == ITEM_CLASS_WEAPON)
    {
        speed = weap->GetItemProperties()->Delay;
        SetBaseAttackTime(OFFHAND,
                          (uint32)((float)speed / (m_attack_speed[MOD_MELEE] * (1.0f + CalcRating(PCR_MELEE_HASTE) / 100.0f))));
    }

    weap = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (weap != nullptr)
    {
        speed = weap->GetItemProperties()->Delay;
        SetBaseAttackTime(RANGED,
                          (uint32)((float)speed / (m_attack_speed[MOD_RANGED] * (1.0f + CalcRating(PCR_RANGED_HASTE) / 100.0f))));
    }
}

void Player::UpdateStats()
{
    UpdateAttackSpeed();

    // Formulas from wowwiki

    int32 AP = 0;
    int32 RAP = 0;
    int32 hpdelta = 128;
    int32 manadelta = 128;

    uint32 str = GetStat(STAT_STRENGTH);
    uint32 agi = GetStat(STAT_AGILITY);
    uint32 lev = getLevel();

    // Attack power
    uint32 cl = getClass();
    switch (cl)
    {
        case DRUID:
            //(Strength x 2) - 20
            AP = str * 2 - 20;
            //Agility - 10
            RAP = agi - 10;

            if (GetShapeShift() == FORM_MOONKIN)
            {
                //(Strength x 2) + (Character Level x 1.5) - 20
                AP += float2int32(static_cast<float>(lev)* 1.5f);
            }
            if (GetShapeShift() == FORM_CAT)
            {
                //(Strength x 2) + Agility + (Character Level x 2) - 20
                AP += agi + (lev * 2);
            }
            if (GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR)
            {
                //(Strength x 2) + (Character Level x 3) - 20
                AP += (lev * 3);
            }
            break;


        case ROGUE:
            //Strength + Agility + (Character Level x 2) - 20
            AP = str + agi + (lev * 2) - 20;
            //Character Level + Agility - 10
            RAP = lev + agi - 10;

            break;


        case HUNTER:
            //Strength + Agility + (Character Level x 2) - 20
            AP = str + agi + (lev * 2) - 20;
            //(Character Level x 2) + Agility - 10
            RAP = (lev * 2) + agi - 10;

            break;

        case SHAMAN:
            //(Strength) + (Agility) + (Character Level x 2) - 20
            AP = str + agi + (lev * 2) - 20;
            //Agility - 10
            RAP = agi - 10;

            break;

        case PALADIN:
            //(Strength x 2) + (Character Level x 3) - 20
            AP = (str * 2) + (lev * 3) - 20;
            //Agility - 10
            RAP = agi - 10;

            break;


        case WARRIOR:
        case DEATHKNIGHT:
            //(Strength x 2) + (Character Level x 3) - 20
            AP = (str * 2) + (lev * 3) - 20;
            //Character Level + Agility - 10
            RAP = lev + agi - 10;

            break;

        default:    //mage,priest,warlock
            AP = agi - 10;
    }

    /* modifiers */
    RAP += m_rap_mod_pct * m_uint32Values[UNIT_FIELD_STAT3] / 100;

    if (RAP < 0)RAP = 0;
    if (AP < 0)AP = 0;

    SetAttackPower(AP);
    SetRangedAttackPower(RAP);

    LevelInfo* levelInfo = objmgr.GetLevelInfo(this->getRace(), this->getClass(), lev);

    if (levelInfo != nullptr)
    {
        hpdelta = levelInfo->Stat[2] * 10;
        manadelta = levelInfo->Stat[3] * 15;
    }

    levelInfo = objmgr.GetLevelInfo(this->getRace(), this->getClass(), 1);

    if (levelInfo != nullptr)
    {
        hpdelta -= levelInfo->Stat[2] * 10;
        manadelta -= levelInfo->Stat[3] * 15;
    }

    uint32 hp = GetBaseHealth();

#if VERSION_STRING != Classic
    int32 stat_bonus = getUInt32Value(UNIT_FIELD_POSSTAT2) - getUInt32Value(UNIT_FIELD_NEGSTAT2);
#else
    int32 stat_bonus = 0;
#endif
    if (stat_bonus < 0)
        stat_bonus = 0; // Avoid of having negative health
    int32 bonus = stat_bonus * 10 + m_healthfromspell + m_healthfromitems;

    uint32 res = hp + bonus + hpdelta;
    uint32 oldmaxhp = getUInt32Value(UNIT_FIELD_MAXHEALTH);

    if (res < hp)
        res = hp;

    if (worldConfig.limit.isLimitSystemEnabled && (worldConfig.limit.maxHealthCap > 0) && (res > worldConfig.limit.maxHealthCap) && GetSession()->GetPermissionCount() <= 0)   //hacker?
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxArenaPoints << " health " << res;

        sCheatLog.writefromsession(GetSession(), dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)
            sendReportToGmMessage(GetName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            GetSession()->Disconnect();
        }
        else // no disconnect, set it to the cap instead
        {
            res = worldConfig.limit.maxHealthCap;
        }
    }
    setUInt32Value(UNIT_FIELD_MAXHEALTH, res);

    if (getUInt32Value(UNIT_FIELD_HEALTH) > res)
        SetHealth(res);
    else if ((cl == DRUID) && (GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR))
    {
        res = getUInt32Value(UNIT_FIELD_MAXHEALTH) * getUInt32Value(UNIT_FIELD_HEALTH) / oldmaxhp;
        SetHealth(res);
    }

    if (cl != WARRIOR && cl != ROGUE && cl != DEATHKNIGHT)
    {
        // MP
        uint32 mana = GetBaseMana();
#if VERSION_STRING != Classic
        stat_bonus = getUInt32Value(UNIT_FIELD_POSSTAT3) - getUInt32Value(UNIT_FIELD_NEGSTAT3);
#endif
        if (stat_bonus < 0)
            stat_bonus = 0; // Avoid of having negative mana
        bonus = stat_bonus * 15 + m_manafromspell + m_manafromitems;

        res = mana + bonus + manadelta;
        if (res < mana)
            res = mana;
        if (worldConfig.limit.isLimitSystemEnabled && (worldConfig.limit.maxManaCap > 0) && (res > worldConfig.limit.maxManaCap) && GetSession()->GetPermissionCount() <= 0)   //hacker?
        {
            char logmsg[256];
            snprintf(logmsg, 256, "has over %u mana (%i)", worldConfig.limit.maxManaCap, res);
            sCheatLog.writefromsession(GetSession(), logmsg);

            if (worldConfig.limit.broadcastMessageToGmOnExceeding) // send info to online GM
                sendReportToGmMessage(GetName(), logmsg);

            if (worldConfig.limit.disconnectPlayerForExceedingLimits)
            {
                GetSession()->Disconnect();
            }
            else // no disconnect, set it to the cap instead
            {
                res = worldConfig.limit.maxManaCap;
            }
        }
        SetMaxPower(POWER_TYPE_MANA, res);

        if (GetPower(POWER_TYPE_MANA) > res)
            SetPower(POWER_TYPE_MANA, res);

        // Manaregen
        // table from http://www.wowwiki.com/Mana_regeneration
        const static float BaseRegen[80/*DBC_PLAYER_LEVEL_CAP*/] =
        {
            0.034965f, 0.034191f, 0.033465f, 0.032526f, 0.031661f, 0.031076f, 0.030523f, 0.029994f, 0.029307f, 0.028661f, 0.027584f, 0.026215f, 0.025381f, 0.024300f, 0.023345f, 0.022748f, 0.021958f, 0.021386f, 0.020790f, 0.020121f, 0.019733f, 0.019155f, 0.018819f, 0.018316f, 0.017936f, 0.017576f, 0.017201f, 0.016919f, 0.016581f, 0.016233f, 0.015994f, 0.015707f, 0.015464f, 0.015204f, 0.014956f, 0.014744f, 0.014495f, 0.014302f, 0.014094f, 0.013895f, 0.013724f, 0.013522f, 0.013363f, 0.013175f, 0.012996f, 0.012853f, 0.012687f, 0.012539f, 0.012384f, 0.012233f, 0.012113f, 0.011973f, 0.011859f, 0.011714f, 0.011575f, 0.011473f, 0.011342f, 0.011245f, 0.011110f, 0.010999f, 0.010700f, 0.010522f, 0.010290f, 0.010119f, 0.009968f, 0.009808f, 0.009651f, 0.009553f, 0.009445f, 0.009327f, 0.008859f, 0.008415f, 0.007993f, 0.007592f, 0.007211f, 0.006849f, 0.006506f, 0.006179f, 0.005869f, 0.005575f
        };

        uint32 level = getLevel();

        if (level > DBC_PLAYER_LEVEL_CAP)
            level = DBC_PLAYER_LEVEL_CAP;
        //float amt = (0.001f + sqrt((float)Intellect) * Spirit * BaseRegen[level-1])*PctPowerRegenModifier[POWER_TYPE_MANA];

        // Mesmer: new Manaregen formula.
        uint32 Spirit = GetStat(STAT_SPIRIT);
        uint32 Intellect = GetStat(STAT_INTELLECT);
        float amt = (0.001f + sqrt((float)Intellect) * Spirit * BaseRegen[level - 1]) * PctPowerRegenModifier[POWER_TYPE_MANA];
#if VERSION_STRING > TBC
        setFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, amt + m_ModInterrMRegen * 0.2f);
        setFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, amt * m_ModInterrMRegenPCT / 100.0f + m_ModInterrMRegen * 0.2f);
#endif
    }

    // Spell haste rating
    float haste = 1.0f + CalcRating(PCR_SPELL_HASTE) / 100.0f;
    if (haste != SpellHasteRatingBonus)
    {
        float value = GetCastSpeedMod() * SpellHasteRatingBonus / haste; // remove previous mod and apply current

        SetCastSpeedMod(value);
        SpellHasteRatingBonus = haste;    // keep value for next run
    }

    // Shield Block
    Item* shield = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (shield != nullptr && shield->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        float block_multiplier = (100.0f + m_modblockabsorbvalue) / 100.0f;
        if (block_multiplier < 1.0f)block_multiplier = 1.0f;

        int32 blockable_damage = float2int32((shield->GetItemProperties()->Block + m_modblockvaluefromspells + getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + (str / 2.0f) - 1.0f) * block_multiplier);
#if VERSION_STRING != Classic
        setUInt32Value(PLAYER_SHIELD_BLOCK, blockable_damage);
#endif
    }
    else
    {
#if VERSION_STRING != Classic
        setUInt32Value(PLAYER_SHIELD_BLOCK, 0);
#endif
    }

    // Dynamic aura application, auras 212, 268
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] != nullptr)
        {
            if (m_auras[x]->HasModType(SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT) || m_auras[x]->HasModType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT))
                m_auras[x]->UpdateModifiers();
        }

    UpdateChances();
    CalcDamage();
}

uint32 Player::SubtractRestXP(uint32 amount)
{
    if (getLevel() >= GetMaxLevel()) // Save CPU, don't waste time on this if you've reached max_level
        amount = 0;

    int32 restAmount = m_restAmount - (amount << 1); // remember , we are dealing with xp without restbonus, so multiply by 2

    if (restAmount < 0)
        m_restAmount = 0;
    else
        m_restAmount = restAmount;

    LOG_DEBUG("Subtracted %d rest XP to a total of %d", amount, m_restAmount);
    UpdateRestState(); // Update clients interface with new values.
    return amount;
}

void Player::AddCalculatedRestXP(uint32 seconds)
{
    // At level one, players will all start in the normal tier.
    // When a player rests in a city or at an inn they will gain rest bonus at a very slow rate.
    // Eight hours of rest will be needed for a player to gain one "bubble" of rest bonus.
    // At any given time, players will be able to accumulate a maximum of 30 "bubbles" worth of rest bonus which
    // translates into approximately 1.5 levels worth of rested play (before your character returns to normal rest state).
    // Thanks to the comforts of a warm bed and a hearty meal, players who rest or log out at an Inn will
    // accumulate rest credit four times faster than players logged off outside of an Inn or City.
    // Players who log out anywhere else in the world will earn rest credit four times slower.
    // http://www.worldofwarcraft.com/info/basics/resting.html


    // Define xp for a full bar (= 20 bubbles)
    uint32 xp_to_lvl = GetXpToLevel();

    // get RestXP multiplier from config.
    float bubblerate = worldConfig.getFloatRate(RATE_RESTXP);

    // One bubble (5% of xp_to_level) for every 8 hours logged out.
    // if multiplier RestXP (from ascent.config) is f.e 2, you only need 4hrs/bubble.
    uint32 rested_xp = uint32(0.05f * xp_to_lvl * (seconds / (3600 * (8 / bubblerate))));

    // if we are at a resting area rest_XP goes 4 times faster (making it 1 bubble every 2 hrs)
    if (m_isResting)
        rested_xp <<= 2;

    // Add result to accumulated rested XP
    m_restAmount += uint32(rested_xp);

    // and set limit to be max 1.5 * 20 bubbles * multiplier (1.5 * xp_to_level * multiplier)
    if (m_restAmount > xp_to_lvl + (uint32)((float)(xp_to_lvl >> 1) * bubblerate))
        m_restAmount = xp_to_lvl + (uint32)((float)(xp_to_lvl >> 1) * bubblerate);

    LOG_DEBUG("Add %d rest XP to a total of %d, RestState %d", rested_xp, m_restAmount, m_isResting);

    // Update clients interface with new values.
    UpdateRestState();
}

void Player::UpdateRestState()
{
    if (m_restAmount && getLevel() < GetMaxLevel())
        m_restState = RESTSTATE_RESTED;
    else
        m_restState = RESTSTATE_NORMAL;

    // Update RestState 100%/200%
    setUInt32Value(PLAYER_BYTES_2, ((getUInt32Value(PLAYER_BYTES_2) & 0x00FFFFFF) | (m_restState << 24)));

    //update needle (weird, works at 1/2 rate)
    setUInt32Value(PLAYER_REST_STATE_EXPERIENCE, m_restAmount >> 1);
}

void Player::ApplyPlayerRestState(bool apply)
{
    if (apply)
    {
        m_restState = RESTSTATE_RESTED;
        m_isResting = true;
        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_RESTING);    //put zZz icon
    }
    else
    {
        m_isResting = false;
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_RESTING);    //remove zZz icon
    }
    UpdateRestState();
}

#define CORPSE_VIEW_DISTANCE 1600 // 40*40

bool Player::CanSee(Object* obj) // * Invisibility & Stealth Detection - Partha *
{
    if (obj == this)
        return true;

    uint32 object_type = obj->GetTypeId();

    if (getDeathState() == CORPSE) // we are dead and we have released our spirit
    {
        if (obj->IsPlayer())
        {
            Player* pObj = static_cast< Player* >(obj);

            if (myCorpseInstanceId == GetInstanceID() && obj->getDistanceSq(myCorpseLocation) <= CORPSE_VIEW_DISTANCE)
                return !pObj->m_isGmInvisible; // we can see all players within range of our corpse except invisible GMs

            if (m_deathVision) // if we have arena death-vision we can see all players except invisible GMs
                return !pObj->m_isGmInvisible;

            return (pObj->getDeathState() == CORPSE); // we can only see players that are spirits
        }

        if (myCorpseInstanceId == GetInstanceID())
        {
            if (obj->IsCorpse() && static_cast< Corpse* >(obj)->GetOwner() == GetGUID())
                return true;

            if (obj->getDistanceSq(myCorpseLocation) <= CORPSE_VIEW_DISTANCE)
                return true; // we can see everything within range of our corpse
        }

        if (m_deathVision) // if we have arena death-vision we can see everything
            return true;

        if (obj->IsCreature() && static_cast<Creature*>(obj)->isSpiritHealer())
            return true; // we can see spirit healers

        return false;
    }
    //------------------------------------------------------------------

    if (!(m_phase & obj->m_phase))  //What you can't see, you can't see, no need to check things further.
        return false;

    switch (object_type) // we are alive or we haven't released our spirit yet
    {
        case TYPEID_PLAYER:
        {
            Player* pObj = static_cast< Player* >(obj);

            if (pObj->m_invisible) // Invisibility - Detection of Players
            {
                if (pObj->getDeathState() == CORPSE)
                    return (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) != 0); // only GM can see players that are spirits

                if (GetGroup() && pObj->GetGroup() == GetGroup() // can see invisible group members except when dueling them
                    && DuelingWith != pObj)
                    return true;

                if (pObj->stalkedby == GetGUID()) // Hunter's Mark / MindVision is visible to the caster
                    return true;

                if (m_invisDetect[INVIS_FLAG_NORMAL] < 1 // can't see invisible without proper detection
                    || pObj->m_isGmInvisible) // can't see invisible GM
                    return (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) != 0); // GM can see invisible players
            }

            if (m_invisible && pObj->m_invisDetect[m_invisFlag] < 1)   // Invisible - can see those that detect, but not others
                return m_isGmInvisible;

            if (pObj->IsStealth()) // Stealth Detection (I Hate Rogues :P)
            {
                if (GetGroup() && pObj->GetGroup() == GetGroup() // can see stealthed group members except when dueling them
                    && DuelingWith != pObj)
                    return true;

                if (pObj->stalkedby == GetGUID()) // Hunter's Mark / MindVision is visible to the caster
                    return true;

                if (isInFront(pObj)) // stealthed player is in front of us
                {
                    // Detection Range = 5yds + (Detection Skill - Stealth Skill)/5
                    detectRange = 5.0f + getLevel() + 0.2f * (float)(GetStealthDetectBonus() - pObj->GetStealthLevel());

                    // Hehe... stealth skill is increased by 5 each level and detection skill is increased by 5 each level too.
                    // This way, a level 70 should easily be able to detect a level 4 rogue (level 4 because that's when you get stealth)
                    //    detectRange += 0.2f * (getLevel() - pObj->getLevel());
                    if (detectRange < 1.0f)
                        detectRange = 1.0f;     // Minimum Detection Range = 1yd
                }
                else // stealthed player is behind us
                {
                    if (GetStealthDetectBonus() > 1000)
                        return true;            // immune to stealth
                    else
                        detectRange = 0.0f;
                }

                detectRange += GetBoundingRadius(); // adjust range for size of player
                detectRange += pObj->GetBoundingRadius(); // adjust range for size of stealthed player
                //LogDefault("Player::CanSee(%s): detect range = %f yards (%f ingame units), cansee = %s , distance = %f" , pObj->GetName() , detectRange , detectRange * detectRange , (GetDistance2dSq(pObj) > detectRange * detectRange) ? "yes" : "no" , getDistanceSq(pObj));
                if (getDistanceSq(pObj) > detectRange * detectRange)
                    return (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) != 0); // GM can see stealthed players
            }

            return !pObj->m_isGmInvisible;
        }
        //------------------------------------------------------------------

        case TYPEID_UNIT:
        {
            Unit* uObj = static_cast< Unit* >(obj);

            // can't see spirit-healers when alive
            if (uObj->IsCreature() && static_cast<Creature*>(uObj)->isSpiritHealer())
                return false;

            // always see our units
            if (GetGUID() == uObj->GetCreatedByGUID())
                return true;

            // unit is invisible
            if (uObj->m_invisible)
            {
                // gms can see invisible units
                /// \todo is invis detection missing here?
                if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
                    return true;
                else
                    return false;
            }

            if (uObj->GetAIInterface()->faction_visibility == 1)
                if (IsTeamHorde())
                    return true;
                else
                    return false;

            if (uObj->GetAIInterface()->faction_visibility == 2)
                if (IsTeamHorde())
                    return false;
                else
                    return true;


            /*if (uObj->m_invisible  // Invisibility - Detection of Units
                && m_invisDetect[uObj->m_invisFlag] < 1) // can't see invisible without proper detection
                return (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) != 0); // GM can see invisible units

            if (m_invisible && uObj->m_invisDetect[m_invisFlag] < 1)   // Invisible - can see those that detect, but not others
                return m_isGmInvisible;*/

            return true;
        }
        //------------------------------------------------------------------

        case TYPEID_GAMEOBJECT:
        {
            GameObject* gObj = static_cast< GameObject* >(obj);

            if (gObj->invisible) // Invisibility - Detection of GameObjects
            {
                uint64 owner = gObj->getUInt64Value(OBJECT_FIELD_CREATED_BY);

                if (GetGUID() == owner) // the owner of an object can always see it
                    return true;

                if (GetGroup())
                {
                    PlayerInfo* inf = objmgr.GetPlayerInfo((uint32)owner);
                    if (inf && GetGroup()->HasMember(inf))
                        return true;
                }

                if (m_invisDetect[gObj->invisibilityFlag] < 1) // can't see invisible without proper detection
                    return (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) != 0); // GM can see invisible objects
            }

            return true;
        }
        //------------------------------------------------------------------

        default:
            return true;
    }
}

void Player::AddInRangeObject(Object* pObj)
{
    //Send taxi move if we're on a taxi
    if (m_CurrentTaxiPath && pObj->IsPlayer())
    {
        uint32 ntime = Util::getMSTime();

        if (ntime > m_taxi_ride_time)
            m_CurrentTaxiPath->SendMoveForTime(this, static_cast< Player* >(pObj), ntime - m_taxi_ride_time);
        /*else
            m_CurrentTaxiPath->SendMoveForTime(this, TO< Player* >(pObj), m_taxi_ride_time - ntime);*/
    }

    Unit::AddInRangeObject(pObj);

    //if the object is a unit send a move packet if they have a destination
    if (pObj->IsCreature())
    {
        static_cast< Creature* >(pObj)->GetAIInterface()->SendCurrentMove(this);
    }
}
void Player::OnRemoveInRangeObject(Object* pObj)
{
    //object was deleted before reaching here
    if (pObj == nullptr)
        return;

    if (IsVisible(pObj->GetGUID()))
        PushOutOfRange(pObj->GetNewGUID());

    m_visibleObjects.erase(pObj->GetGUID());
    Unit::OnRemoveInRangeObject(pObj);

    if (pObj->GetGUID() == m_CurrentCharm)
    {
        Unit* p = GetMapMgr()->GetUnit(m_CurrentCharm);
        if (!p)
            return;

        UnPossess();
        if (isCastingNonMeleeSpell())
            interruptSpell();       // cancel the spell
        m_CurrentCharm = 0;

    }

    // We've just gone out of range of our pet :(
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end();)
    {
        Pet* summon = (*itr);
        ++itr;
        if (pObj == summon)
        {
            summon->DelayedRemove(false, false, 1);//delayed otherwise Object::RemoveInRangeObject() will remove twice the Pet from inrangeset. Refer to r3199
            return;
        }
    }

    if (pObj->GetGUID() == GetSummonedUnitGUID())
        sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::RemoveFieldSummon, EVENT_SUMMON_EXPIRE, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);//otherwise Creature::Update() will access free'd memory
}

void Player::ClearInRangeSet()
{
    m_visibleObjects.clear();
    Unit::ClearInRangeSet();
}

void Player::EventCannibalize(uint32 amount)
{
    if (GetChannelSpellId() != 20577)
    {
        sEventMgr.RemoveEvents(this, EVENT_CANNIBALIZE);
        cannibalize = false;
        cannibalizeCount = 0;
        return;
    }
    uint32 amt = (GetMaxHealth() * amount) / 100;

    uint32 newHealth = GetHealth() + amt;

    if (newHealth <= GetMaxHealth())
        SetHealth(newHealth);
    else
        SetHealth(GetMaxHealth());

    cannibalizeCount++;
    if (cannibalizeCount == 5)
        SetEmoteState(EMOTE_ONESHOT_NONE);

    SendPeriodicHealAuraLog(GetNewGUID(), GetNewGUID(), 20577, amt, 0, false);
}

///The player sobers by 256 every 10 seconds
void Player::HandleSobering()
{
    m_drunkTimer = 0;

    SetDrunkValue((m_drunk <= 256) ? 0 : (m_drunk - 256));
}

DrunkenState Player::GetDrunkenstateByValue(uint16 value)
{
    if (value >= 23000)
        return DRUNKEN_SMASHED;
    if (value >= 12800)
        return DRUNKEN_DRUNK;
    if (value & 0xFFFE)
        return DRUNKEN_TIPSY;
    return DRUNKEN_SOBER;
}

void Player::SetDrunkValue(uint16 newDrunkenValue, uint32 itemId)
{
    uint32 oldDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    m_drunk = newDrunkenValue;
    setUInt32Value(PLAYER_BYTES_3, (getUInt32Value(PLAYER_BYTES_3) & 0xFFFF0001) | (m_drunk & 0xFFFE));

    uint32 newDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    if (newDrunkenState == oldDrunkenState)
        return;

    // special drunk invisibility detection
    if (newDrunkenState >= DRUNKEN_DRUNK)
        m_invisDetect[INVIS_FLAG_UNKNOWN6] = 100;
    else
        m_invisDetect[INVIS_FLAG_UNKNOWN6] = 0;

    UpdateVisibility();

    SendNewDrunkState(newDrunkenState, itemId);
}

void Player::LoadTaxiMask(const char* data)
{
    std::vector<std::string> tokens = Util::SplitStringBySeperator(data, " ");

    int index;
    std::vector<std::string>::iterator iter;

    for (iter = tokens.begin(), index = 0;
         (index < 12) && (iter != tokens.end()); ++iter, ++index)
    {
        m_taximask[index] = atol((*iter).c_str());
    }
}

bool Player::HasQuestForItem(uint32 itemid)
{
    QuestProperties const* qst;
    for (uint8 i = 0; i < 25; ++i)
    {
        if (m_questlog[i] != nullptr)
        {
            qst = m_questlog[i]->GetQuest();

            // Check the item_quest_association table for an entry related to this item
            QuestAssociationList* tempList = QuestMgr::getSingleton().GetQuestAssociationListForItemId(itemid);
            if (tempList != nullptr)
            {
                QuestAssociationList::iterator it;
                for (it = tempList->begin(); it != tempList->end(); ++it)
                {
                    if (((*it)->qst == qst) && (GetItemInterface()->GetItemCount(itemid) < (*it)->item_count))
                    {
                        return true;
                    } // end if
                } // end for
            } // end if

            // No item_quest association found, check the quest requirements
            if (!qst->count_required_item)
                continue;

            for (uint32 j = 0; j < MAX_REQUIRED_QUEST_ITEM; ++j)
                if (qst->required_item[j] == itemid && (GetItemInterface()->GetItemCount(itemid) < qst->required_itemcount[j]))
                    return true;
        }
    }
    return false;
}

bool Player::HasItemCount(uint32 item, uint32 count, bool inBankAlso) const
{
    return (m_ItemInterface->GetItemCount(item, inBankAlso) == count);
}

void Player::EventAllowTiggerPort(bool enable)
{
    m_AllowAreaTriggerPort = enable;
}

uint32 Player::CalcTalentResetCost(uint32 resetnum)
{

    if (resetnum == 0)
        return  10000;
    else
    {
        if (resetnum > 10)
            return  500000;
        else return resetnum * 50000;
    }
}

int32 Player::CanShootRangedWeapon(uint32 spellid, Unit* target, bool autoshot)
{
    SpellInfo* spell_info = sSpellCustomizations.GetSpellInfo(spellid);
    if (spell_info == nullptr)
        return -1;

    LogDebugFlag(LF_SPELL, "Canshootwithrangedweapon!?!? spell: [%u] %s" , spell_info->getId() , spell_info->getName().c_str());

    // Check if Morphed
    if (polySpell > 0)
        return SPELL_FAILED_NOT_SHAPESHIFT;

    // Check ammo
    auto item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    auto item_proto = sMySQLStore.getItemProperties(GetAmmoId());
    if (item == nullptr || disarmed)           //Disarmed means disarmed, we shouldn't be able to cast Auto Shot while disarmed
        return SPELL_FAILED_NO_AMMO;        //In proper language means "Requires Ranged Weapon to be equipped"

    if (!m_requiresNoAmmo)                  //Thori'dal, Wild Quiver, but it still requires to have a weapon equipped
    {
        // Check ammo level
        if (item_proto && getLevel() < item_proto->RequiredLevel)
            return SPELL_FAILED_LOWLEVEL;

        // Check ammo type
        auto item_proto_ammo = sMySQLStore.getItemProperties(item->GetEntry());
        if (item_proto && item_proto_ammo && item_proto->SubClass != item_proto_ammo->AmmoType)
            return SPELL_FAILED_NEED_AMMO;
    }

    // Player has clicked off target. Fail spell.
    if (m_curSelection != getCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets.m_unitTarget)
        return SPELL_FAILED_INTERRUPTED;

    // Check if target is already dead
    if (target->IsDead())
        return SPELL_FAILED_TARGETS_DEAD;

    // Check if in line of sight (need collision detection).
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        bool isInLOS = mgr->isInLineOfSight(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
        if (GetMapId() == target->GetMapId() && !isInLOS)
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    // Check if we aren't casting another spell already
    if (isCastingNonMeleeSpell(true, false, true, autoshot))
        return -1;

    // Supalosa - The hunter ability Auto Shot is using Shoot range, which is 5 yards shorter.
    // So we'll use 114, which is the correct 35 yard range used by the other Hunter abilities (arcane shot, concussive shot...)
    uint8 fail = 0;
    uint32 rIndex = autoshot ? 114 : spell_info->getRangeIndex();

    auto spell_range = sSpellRangeStore.LookupEntry(rIndex);
    float minrange = GetMinRange(spell_range);
    float dist = CalcDistance(this, target);
    float maxr = GetMaxRange(spell_range) + 2.52f;

    spellModFlatFloatValue(this->SM_FRange, &maxr, spell_info->getSpellGroupType());
    spellModPercentageFloatValue(this->SM_PRange, &maxr, spell_info->getSpellGroupType());

    //float bonusRange = 0;
    // another hackfix: bonus range from hunter talent hawk eye: +2/4/6 yard range to ranged weapons
    //if (autoshot)
    //spellModFlatFloatValue(SM_FRange, &bonusRange, sSpellCustomizations.GetSpellInfo(75)->SpellGroupType); // HORRIBLE hackfixes :P
    // Partha: +2.52yds to max range, this matches the range the client is calculating.
    // see extra/supalosa_range_research.txt for more info
    //bonusRange = 2.52f;
    //LogDefault("Bonus range = %f" , bonusRange);

    // check if facing target
    if (!isInFront(target))
    {
        fail = SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    // Check ammo count
    if (!m_requiresNoAmmo && item_proto && item->GetItemProperties()->SubClass != ITEM_SUBCLASS_WEAPON_WAND)
    {
        uint32 ammocount = GetItemInterface()->GetItemCount(item_proto->ItemId);
        if (ammocount == 0)
            fail = SPELL_FAILED_NO_AMMO;
    }

    // Check for too close
    if (spellid != SPELL_RANGED_WAND)  //no min limit for wands
        if (minrange > dist)
            fail = SPELL_FAILED_TOO_CLOSE;

    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
    bool isInLOS = mgr->isInLineOfSight(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

    if (worldConfig.terrainCollision.isCollisionEnabled && GetMapId() == target->GetMapId() && !isInLOS)
        fail = SPELL_FAILED_LINE_OF_SIGHT;

    if (dist > maxr)
    {
        LogDebug("Auto shot failed: out of range (Maxr: %f, Dist: %f)" , maxr , dist);
        fail = SPELL_FAILED_OUT_OF_RANGE;
    }

    if (spellid == SPELL_RANGED_THROW)
    {
        if (GetItemInterface()->GetItemCount(item->GetItemProperties()->ItemId) == 0)
            fail = SPELL_FAILED_NO_AMMO;
    }

    if (fail > 0)  // && fail != SPELL_FAILED_OUT_OF_RANGE)
    {
        SendCastResult(autoshot ? 75 : spellid, fail, 0, 0);

        if (fail != SPELL_FAILED_OUT_OF_RANGE)
        {
            uint32 spellid2 = autoshot ? 75 : spellid;
            m_session->OutPacket(SMSG_CANCEL_AUTO_REPEAT, 4, &spellid2);
        }
        //LogDefault("Result for CanShootWIthRangedWeapon: %u" , fail);
        //LOG_DEBUG("Can't shoot with ranged weapon: %u (Timer: %u)" , fail , m_AutoShotAttackTimer);
        return fail;
    }

    return 0;
}

/*! \returns True if player's current battleground was queued for as a random battleground
 *  \sa Player::SetQueuedForRbg */
bool Player::QueuedForRbg()
{
    return this->m_bgIsRbg;
}

/*! Used to set whether player's current battleground was queued for as a random battleground
 *  \param value Value to assign to m_bgIsRbg
 *  \sa Player::QueuedForRbg */
void Player::SetQueuedForRbg(bool value)
{
    this->m_bgIsRbg = value;
}

/*! \returns True if player has won a Random Battleground today */
bool Player::HasWonRbgToday()
{
    return this->m_bgIsRbgWon;
}

/*! Used to set whether a player has won a random battleground today
 *  \param value Value to assign to m_bgIsRbgWon */
void Player::SetHasWonRbgToday(bool value)
{
    this->m_bgIsRbgWon = value;
}

bool Player::HasSpell(uint32 spell)
{
    return mSpells.find(spell) != mSpells.end();
}

bool Player::HasDeletedSpell(uint32 spell)
{
    return (mDeletedSpells.count(spell) > 0);
}

void Player::removeSpellByHashName(uint32 hash)
{
    SpellSet::iterator it, iter;

    for (iter = mSpells.begin(); iter != mSpells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellInfo* e = sSpellCustomizations.GetSpellInfo(SpellID);
        if (e->custom_NameHash == hash)
        {
            if (info->spell_list.find(e->getId()) != info->spell_list.end())
                continue;

            RemoveAura(SpellID, GetGUID());

            m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);

            mSpells.erase(it);
        }
    }

    for (iter = mDeletedSpells.begin(); iter != mDeletedSpells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellInfo* e = sSpellCustomizations.GetSpellInfo(SpellID);
        if (e->custom_NameHash == hash)
        {
            if (info->spell_list.find(e->getId()) != info->spell_list.end())
                continue;

            RemoveAura(SpellID, GetGUID());

            m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);

            mDeletedSpells.erase(it);
        }
    }
}

bool Player::removeSpell(uint32 SpellID, bool MoveToDeleted, bool SupercededSpell, uint32 SupercededSpellID)
{
    SpellSet::iterator it, iter = mSpells.find(SpellID);
    if (iter != mSpells.end())
    {
        mSpells.erase(iter);
        RemoveAura(SpellID, GetGUID());
    }
    else
    {
        iter = mDeletedSpells.find(SpellID);
        if (iter != mDeletedSpells.end())
        {
            mDeletedSpells.erase(iter);
        }
        else
        {
            return false;
        }
    }

    if (MoveToDeleted)
        mDeletedSpells.insert(SpellID);

    if (!IsInWorld())
        return true;

    if (SupercededSpell)
    {
        WorldPacket data(SMSG_SUPERCEDED_SPELL, 8);
        data << SpellID << SupercededSpellID;
        m_session->SendPacket(&data);
    }
    else
    {
        m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);
    }

    return true;
}

bool Player::removeDeletedSpell(uint32 SpellID)
{
    SpellSet::iterator it = mDeletedSpells.find(SpellID);
    if (it == mDeletedSpells.end())
        return false;

    mDeletedSpells.erase(it);
    return true;
}

void Player::EventActivateGameObject(GameObject* obj)
{
    obj->BuildFieldUpdatePacket(this, GAMEOBJECT_DYNAMIC, 1 | 8);
}

void Player::EventDeActivateGameObject(GameObject* obj)
{
    obj->BuildFieldUpdatePacket(this, GAMEOBJECT_DYNAMIC, 0);
}

void Player::EventTimedQuestExpire(uint32 questid)
{
    QuestLogEntry* qle = this->GetQuestLogForEntry(questid);
    if (qle == nullptr)
        return;

    QuestProperties const* qst = qle->GetQuest();

    sQuestMgr.SendQuestUpdateFailedTimer(qst, this);
    CALL_QUESTSCRIPT_EVENT(qle, OnQuestCancel)(this);
    qle->Fail(true);
}

void Player::AreaExploredOrEventHappens(uint32 questId)
{
    sQuestMgr.AreaExplored(this, questId);
}

void Player::Reset_Spells()
{
    PlayerCreateInfo const* playerCreateInfo = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    ARCEMU_ASSERT(playerCreateInfo != NULL);

    std::list<uint32> spelllist;

    for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        spelllist.push_back((*itr));
    }

    for (std::list<uint32>::iterator itr = spelllist.begin(); itr != spelllist.end(); ++itr)
    {
        removeSpell((*itr), false, false, 0);
    }

    for (std::set<uint32>::iterator sp = playerCreateInfo->spell_list.begin(); sp != playerCreateInfo->spell_list.end(); ++sp)
    {
        if (*sp)
        {
            addSpell(*sp);
        }
    }

    // cebernic ResetAll ? don't forget DeletedSpells
    mDeletedSpells.clear();
}

void Player::ResetDualWield2H()
{
    DualWield2H = false;

    Item* mainhand = GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
    Item* offhand = GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
    if (offhand && (offhand->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON ||
        (mainhand && mainhand->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON)))
    {
        // we need to de-equip this
        offhand = GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
        SlotResult result = GetItemInterface()->FindFreeInventorySlot(offhand->GetItemProperties());
        if (!result.Result)
        {
            // no free slots for this item, try to send it by mail
            offhand->RemoveFromWorld();
            offhand->SetOwner(nullptr);
            offhand->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, GetGUID(), GetGUID(), "Your offhand item", "", 0, 0, offhand->GetLowGUID(), MAIL_STATIONERY_GM);
            offhand->DeleteMe();
            offhand = nullptr;
        }
        else if (!GetItemInterface()->SafeAddItem(offhand, result.ContainerSlot, result.Slot) && !GetItemInterface()->AddItemToFreeSlot(offhand))      // shouldn't happen either.
        {
            offhand->DeleteMe();
            offhand = nullptr;
        }
    }
}

void Player::Reset_Talents()
{
    uint8 playerClass = getClass();
    SpellInfo* spellInfo;
    SpellInfo* spellInfo2;

    // Loop through all talents.
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        auto temp_talent = sTalentStore.LookupEntry(i);
        if (temp_talent == nullptr)
            continue;

        if (temp_talent->TalentTree != TalentTreesPerClass[playerClass][0] &&
            temp_talent->TalentTree != TalentTreesPerClass[playerClass][1] &&
            temp_talent->TalentTree != TalentTreesPerClass[playerClass][2])
        {
            // Not a talent for this class
            continue;
        }

        // this is a normal talent (i hope)
        for (uint8 j = 0; j < 5; ++j)
        {
            if (temp_talent->RankID[j] != 0)
            {
                spellInfo = sSpellCustomizations.GetSpellInfo(temp_talent->RankID[j]);
                if (spellInfo != nullptr)
                {
                    for (uint8 k = 0; k < 3; ++k)
                    {
                        if (spellInfo->getEffect(k) == SPELL_EFFECT_LEARN_SPELL)
                        {
                            // removeSpell(spellInfo->EffectTriggerSpell[k], false, 0, 0);
                            // remove higher ranks of this spell too (like earth shield lvl 1 is talent and the rest is taught from trainer)
                            spellInfo2 = sSpellCustomizations.GetSpellInfo(spellInfo->getEffectTriggerSpell(k));
                            if (spellInfo2 != nullptr)
                            {
                                removeSpellByHashName(spellInfo2->custom_NameHash);
                            }
                        }
                    }
                    // remove them all in 1 shot
                    removeSpellByHashName(spellInfo->custom_NameHash);
                }
            }
            else
            {
                break;
            }
        }
    }
    uint32 l = getLevel();
    if (l > 9)
    {
        SetCurrentTalentPoints(l - 9);
    }
    else
    {
        SetCurrentTalentPoints(0);
    }

    if (DualWield2H)
    {
        ResetDualWield2H();
    }

    m_specs[m_talentActiveSpec].talents.clear();
#if VERSION_STRING == Cata
    m_FirstTalentTreeLock = 0;
#endif
    smsg_TalentsInfo(false); //VLack: should we send this as Aspire? Yes...
}

void Player::Reset_AllTalents()
{
    uint8 originalspec = m_talentActiveSpec;
    Reset_Talents();

    if (originalspec == SPEC_PRIMARY)
        ActivateSpec(SPEC_SECONDARY);
    else
        ActivateSpec(SPEC_PRIMARY);

    Reset_Talents();
    ActivateSpec(originalspec);
}

void Player::CalcResistance(uint16 type)
{
    int32 res;
    int32 pos;
    int32 neg;
    ARCEMU_ASSERT(type < 7);
    pos = (BaseResistance[type] * BaseResistanceModPctPos[type]) / 100;
    neg = (BaseResistance[type] * BaseResistanceModPctNeg[type]) / 100;

    pos += FlatResistanceModifierPos[type];
    neg += FlatResistanceModifierNeg[type];
    res = BaseResistance[type] + pos - neg;
    if (type == 0)res += m_uint32Values[UNIT_FIELD_STAT1] * 2; //fix armor from agi
    if (res < 0)res = 0;
    pos += (res * ResistanceModPctPos[type]) / 100;
    neg += (res * ResistanceModPctNeg[type]) / 100;
    res = pos - neg + BaseResistance[type];
    if (type == 0)res += m_uint32Values[UNIT_FIELD_STAT1] * 2; //fix armor from agi

    // Dynamic aura 285 application, removing bonus
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] != nullptr)
        {
            if (m_auras[x]->HasModType(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR))
                m_auras[x]->SpellAuraModAttackPowerOfArmor(false);
        }
    }

    if (res < 0)
        res = 1;

#if VERSION_STRING != Classic
    setUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + type, pos);
    setUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + type, -neg);
#endif
    SetResistance(type, res > 0 ? res : 0);

    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->CalcResistance(type);  //Re-calculate pet's too.
    }

    // Dynamic aura 285 application, adding bonus
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] != nullptr)
        {
            if (m_auras[x]->HasModType(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR))
                m_auras[x]->SpellAuraModAttackPowerOfArmor(true);
        }
    }
}


void Player::UpdateNearbyGameObjects()
{

    for (Object::InRangeSet::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        Object* obj = (*itr);
        if (obj->IsGameObject())
        {
            bool activate_quest_object = false;
            GameObject* go = static_cast<GameObject*>(obj);
            QuestLogEntry* qle = nullptr;
            auto gameobject_info = go->GetGameObjectProperties();

            bool deactivate = false;
            if (gameobject_info && (gameobject_info->goMap.size() || gameobject_info->itemMap.size()))
            {
                for (GameObjectGOMap::const_iterator GOitr = gameobject_info->goMap.begin(); GOitr != gameobject_info->goMap.end(); ++GOitr)
                {
                    if ((qle = GetQuestLogForEntry(GOitr->first->id)) != nullptr)
                    {
                        for (uint32 i = 0; i < qle->GetQuest()->count_required_mob; ++i)
                        {
                            if (qle->GetQuest()->required_mob_or_go[i] == static_cast<int32>(go->GetEntry()) &&
                                qle->GetMobCount(i) < qle->GetQuest()->required_mob_or_go_count[i])
                            {
                                activate_quest_object = true;
                                break;
                            }
                        }
                        if (activate_quest_object)
                            break;
                    }
                }

                if (!activate_quest_object)
                {
                    for (GameObjectItemMap::const_iterator GOitr = gameobject_info->itemMap.begin(); GOitr != gameobject_info->itemMap.end(); ++GOitr)
                    {
                        for (std::map<uint32, uint32>::const_iterator it2 = GOitr->second.begin(); it2 != GOitr->second.end(); ++it2)
                        {
                            if (GetItemInterface()->GetItemCount(it2->first) < it2->second)
                            {
                                activate_quest_object = true;
                                break;
                            }
                        }
                        if (activate_quest_object)
                            break;
                    }
                }

                if (!activate_quest_object)
                {
                    deactivate = true;
                }
            }
            bool bPassed = !deactivate;
            if (go->GetType() == GAMEOBJECT_TYPE_QUESTGIVER)
            {
                GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(go);

                if (go_quest_giver->HasQuests() && go_quest_giver->NumOfQuests() > 0)
                {
                    std::list<QuestRelation*>::iterator itr2 = go_quest_giver->QuestsBegin();
                    for (; itr2 != go_quest_giver->QuestsEnd(); ++itr2)
                    {
                        QuestRelation* qr = (*itr2);

                        uint32 status = sQuestMgr.CalcQuestStatus(nullptr, this, qr->qst, qr->type, false);
                        if (status == QMGR_QUEST_CHAT
                            || (qr->type & QUESTGIVER_QUEST_START && (status == QMGR_QUEST_AVAILABLE || status == QMGR_QUEST_REPEATABLE))
                            || (qr->type & QUESTGIVER_QUEST_END && status == QMGR_QUEST_FINISHED))
                        {
                            // Activate gameobject
                            EventActivateGameObject(go);
                            bPassed = true;
                            break;
                        }
                    }
                }
            }
            if (!bPassed)
                EventDeActivateGameObject(static_cast<GameObject*>(*itr));
        }
    }
}


void Player::EventTaxiInterpolate()
{
    if (!m_CurrentTaxiPath || m_mapMgr == nullptr) return;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    uint32 ntime = Util::getMSTime();

    if (ntime > m_taxi_ride_time)
        m_CurrentTaxiPath->SetPosForTime(x, y, z, ntime - m_taxi_ride_time, &lastNode, m_mapId);
    /*else
        m_CurrentTaxiPath->SetPosForTime(x, y, z, m_taxi_ride_time - ntime, &lastNode);*/

    if (x < _minX || x > _maxX || y < _minY || y > _maxX)
        return;

    SetPosition(x, y, z, 0);
}

void Player::TaxiStart(TaxiPath* path, uint32 modelid, uint32 start_node)
{
    int32 mapchangeid = -1;
    float mapchangex = 0.0f, mapchangey = 0.0f, mapchangez = 0.0f;
    uint32 cn = m_taxiMapChangeNode;

    m_taxiMapChangeNode = 0;

    Dismount();

    if (currentvehicle != nullptr)
        currentvehicle->EjectPassenger(this);

    //also remove morph spells
    if (GetDisplayId() != GetNativeDisplayId())
    {
        RemoveAllAuraType(SPELL_AURA_TRANSFORM);
        RemoveAllAuraType(SPELL_AURA_MOD_SHAPESHIFT);
    }

    DismissActivePets();

    SetMount(modelid);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    SetTaxiPath(path);
    SetTaxiPos();
    SetTaxiState(true);
    m_taxi_ride_time = Util::getMSTime();

    //uint32 traveltime = uint32(path->getLength() * TAXI_TRAVEL_SPEED); // 36.7407
    float traveldist = 0;

    float lastx = 0, lasty = 0, lastz = 0;
    TaxiPathNode* firstNode = path->GetPathNode(start_node);
    uint32 add_time = 0;

    // temporary workaround for taximodes with changing map
    if (path->GetID() == 766 || path->GetID() == 767 || path->GetID() == 771 || path->GetID() == 772)
    {
        JumpToEndTaxiNode(path);
        return;
    }

    if (start_node)
    {
        TaxiPathNode* pn = path->GetPathNode(0);
        float dist = 0;
        lastx = pn->x;
        lasty = pn->y;
        lastz = pn->z;
        for (uint32 i = 1; i <= start_node; ++i)
        {
            pn = path->GetPathNode(i);
            if (!pn)
            {
                JumpToEndTaxiNode(path);
                return;
            }

            dist += CalcDistance(lastx, lasty, lastz, pn->x, pn->y, pn->z);
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
        add_time = uint32(dist * TAXI_TRAVEL_SPEED);
        lastx = lasty = lastz = 0;
    }
    size_t endn = path->GetNodeCount();
    if (m_taxiPaths.size())
        endn -= 2;

    for (uint32 i = start_node; i < endn; ++i)
    {
        TaxiPathNode* pn = path->GetPathNode(i);

        // temporary workaround for taximodes with changing map
        if (!pn || path->GetID() == 766 || path->GetID() == 767 || path->GetID() == 771 || path->GetID() == 772)
        {
            JumpToEndTaxiNode(path);
            return;
        }

        if (pn->mapid != m_mapId)
        {
            endn = (i - 1);
            m_taxiMapChangeNode = i;

            mapchangeid = (int32)pn->mapid;
            mapchangex = pn->x;
            mapchangey = pn->y;
            mapchangez = pn->z;
            break;
        }

        if (!lastx || !lasty || !lastz)
        {
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
        else
        {
            float dist = CalcDistance(lastx, lasty, lastz,
                                      pn->x, pn->y, pn->z);
            traveldist += dist;
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
    }

    uint32 traveltime = uint32(traveldist * TAXI_TRAVEL_SPEED);

    if (start_node > endn || (endn - start_node) > 200)
        return;

    WorldPacket data(SMSG_MONSTER_MOVE, 38 + ((endn - start_node) * 12));
    data << GetNewGUID();
    data << uint8(0); //VLack: it seems we have a 1 byte stuff after the new GUID
    data << firstNode->x;
    data << firstNode->y;
    data << firstNode->z;
    data << m_taxi_ride_time;
    data << uint8(0);
#if VERSION_STRING == Cata
    data << uint32(0x0C008400);
#else
    data << uint32(0x00003000);
#endif
    data << uint32(traveltime);

    if (!cn)
        m_taxi_ride_time -= add_time;

    data << uint32(endn - start_node);


    for (uint32 i = start_node; i < endn; i++)
    {
        TaxiPathNode* pn = path->GetPathNode(i);
        if (!pn)
        {
            JumpToEndTaxiNode(path);
            return;
        }

        data << pn->x;
        data << pn->y;
        data << pn->z;
    }

    SendMessageToSet(&data, true);

    sEventMgr.AddEvent(this, &Player::EventTaxiInterpolate,
                       EVENT_PLAYER_TAXI_INTERPOLATE, 900, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    if (mapchangeid < 0)
    {
        TaxiPathNode* pn = path->GetPathNode((uint32)path->GetNodeCount() - 1);
        sEventMgr.AddEvent(this, &Player::EventDismount, path->GetPrice(),
                           pn->x, pn->y, pn->z, EVENT_PLAYER_TAXI_DISMOUNT, traveltime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        sEventMgr.AddEvent(this, &Player::EventTeleportTaxi, (uint32)mapchangeid, mapchangex, mapchangey, mapchangez, EVENT_PLAYER_TELEPORT, traveltime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Player::JumpToEndTaxiNode(TaxiPath* path)
{
    // this should *always* be safe in case it cant build your position on the path!
    TaxiPathNode* pathnode = path->GetPathNode((uint32)path->GetNodeCount() - 1);
    if (!pathnode) return;

    ModGold(-(int32)path->GetPrice());

    SetTaxiState(false);
    SetTaxiPath(nullptr);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));

    SafeTeleport(pathnode->mapid, 0, LocationVector(pathnode->x, pathnode->y, pathnode->z));

    // Start next path if any remaining
    if (m_taxiPaths.size())
    {
        TaxiPath* p = *m_taxiPaths.begin();
        m_taxiPaths.erase(m_taxiPaths.begin());
        TaxiStart(p, taxi_model_id, 0);
    }
}

void Player::RemoveSpellsFromLine(uint32 skill_line)
{
    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(i);
        if (skill_line_ability)
        {
            if (skill_line_ability->skilline == skill_line)
            {
                // Check ourselves for this spell, and remove it..
                if (!removeSpell(skill_line_ability->spell, 0, 0, 0))
                    // if we didn't unlearned spell check deleted spells
                    removeDeletedSpell(skill_line_ability->spell);
            }
        }
    }
}

void Player::CalcStat(uint16 type)
{
    int32 res;
    ARCEMU_ASSERT(type < 5);

    int32 pos = (int32)((int32)BaseStats[type] * (int32)StatModPctPos[type]) / 100 + (int32)FlatStatModPos[type];
    int32 neg = (int32)((int32)BaseStats[type] * (int32)StatModPctNeg[type]) / 100 + (int32)FlatStatModNeg[type];
    res = pos + (int32)BaseStats[type] - neg;
    if (res <= 0)
        res = 1;
    pos += (res * (int32)static_cast< Player* >(this)->TotalStatModPctPos[type]) / 100;
    neg += (res * (int32)static_cast< Player* >(this)->TotalStatModPctNeg[type]) / 100;
    res = pos + BaseStats[type] - neg;
    if (res <= 0)
        res = 1;

#if VERSION_STRING != Classic
    setUInt32Value(UNIT_FIELD_POSSTAT0 + type, pos);

    if (neg < 0)
        setUInt32Value(UNIT_FIELD_NEGSTAT0 + type, -neg);
    else
        setUInt32Value(UNIT_FIELD_NEGSTAT0 + type, neg);
#endif

    SetStat(type, res);
    if (type == STAT_AGILITY)
        CalcResistance(0);

    if (type == STAT_STAMINA || type == STAT_INTELLECT)
    {
        std::list<Pet*> summons = GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            (*itr)->CalcStat(type);  //Re-calculate pet's too
        }
    }
}

void Player::RegenerateMana(bool is_interrupted)
{
    uint32 cur = GetPower(POWER_TYPE_MANA);
    uint32 mm = GetMaxPower(POWER_TYPE_MANA);
    if (cur >= mm)
        return;
    float wrate = worldConfig.getFloatRate(RATE_POWER1); // config file regen rate
#if VERSION_STRING == TBC
    float amt = (is_interrupted) ? getFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) : getFloatValue(PLAYER_FIELD_MOD_MANA_REGEN);
#elif VERSION_STRING == Classic
    float amt = 10;
#else
    float amt = (is_interrupted) ? getFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER) : getFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER);
#endif
    amt *= wrate * 2.0f;

    if ((amt <= 1.0) && (amt > 0)) //this fixes regen like 0.98
    {
        if (is_interrupted)
            return;
        ++cur;
    }
    else
        cur += float2int32(amt);
    //    Unit::SetPower() will call Object::SetUInt32Value(), which will (for players) call SendPowerUpdate(),
    //    which can be slightly out-of-sync with client regeneration [with latency] (and wastes packets since client can handle this on its own)
    if (wrate != 1.0) // our config has custom regen rate, so send new amount to player's client
    {
        SetPower(POWER_TYPE_MANA, cur);
    }
    else // let player's own client handle normal regen rates.
    {
        SetPower(POWER_TYPE_MANA, ((cur >= mm) ? mm : cur));
        SendPowerUpdate(false); // send update to other in-range players
    }
}

void Player::RegenerateHealth(bool inCombat)
{
    uint32 cur = GetHealth();
    uint32 mh = GetMaxHealth();

    if (cur == 0) return;   // cebernic: bugfix dying but regenerated?

    if (cur >= mh)
        return;

#if VERSION_STRING != Cata
    auto HPRegenBase = sGtRegenHPPerSptStore.LookupEntry(getLevel() - 1 + (getClass() - 1) * 100);
    if (HPRegenBase == nullptr)
        HPRegenBase = sGtRegenHPPerSptStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (getClass() - 1) * 100);

    auto HPRegen = sGtOCTRegenHPStore.LookupEntry(getLevel() - 1 + (getClass() - 1) * 100);
    if (HPRegen == nullptr)
        HPRegen = sGtOCTRegenHPStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (getClass() - 1) * 100);
#endif

    uint32 basespirit = m_uint32Values[UNIT_FIELD_STAT4];
    uint32 extraspirit = 0;

    if (basespirit > 50)
    {
        extraspirit = basespirit - 50;
        basespirit = 50;
    }

#if VERSION_STRING != Cata
    float amt = basespirit * HPRegen->ratio + extraspirit * HPRegenBase->ratio;
#else
    float amt = basespirit * 200+ extraspirit * 200;
#endif

    if (PctRegenModifier)
        amt += (amt * PctRegenModifier) / 100;

    amt *= worldConfig.getFloatRate(RATE_HEALTH);//Apply conf file rate
    //Near values from official
    // wowwiki: Health Regeneration is increased by 33% while sitting.
    if (m_isResting)
        amt = amt * 1.33f;

    if (inCombat)
        amt *= PctIgnoreRegenModifier;

    if (amt != 0)
    {
        if (amt > 0)
        {
            if (amt <= 1.0f)//this fixes regen like 0.98
                cur++;
            else
                cur += float2int32(amt);
            SetHealth((cur >= mh) ? mh : cur);
        }
        else
            DealDamage(this, float2int32(-amt), 0, 0, 0);
    }
}

void Player::LooseRage(int32 decayValue)
{
    //Rage is lost at a rate of 3 rage every 3 seconds.
    //The Anger Management talent changes this to 2 rage every 3 seconds.
    uint32 cur = GetPower(POWER_TYPE_RAGE);
    uint32 newrage = ((int)cur <= decayValue) ? 0 : cur - decayValue;
    if (newrage > 1000)
        newrage = 1000;
    // Object::SetUInt32Value() will (for players) call SendPowerUpdate(),
    // which can be slightly out-of-sync with client rage loss
    // config file rage rate is rage gained, not lost, so we don't need that here
    //    SetUInt32Value(UNIT_FIELD_POWER2,newrage);
    SetPower(POWER_TYPE_RAGE, newrage);
    SendPowerUpdate(false); // send update to other in-range players
}

void Player::RegenerateEnergy()
{
    uint32 cur = GetPower(POWER_TYPE_ENERGY);
    uint32 mh = GetMaxPower(POWER_TYPE_ENERGY);
    if (cur >= mh)
        return;

    float wrate = worldConfig.getFloatRate(RATE_POWER4);
    float amt = PctPowerRegenModifier[POWER_TYPE_ENERGY];
    amt *= wrate * 20.0f;

    cur += float2int32(amt);
    //    Object::SetUInt32Value() will (for players) call SendPowerUpdate(),
    //    which can be slightly out-of-sync with client regeneration [latency] (and wastes packets since client can handle this on its own)
    if (wrate != 1.0f) // our config has custom regen rate, so send new amount to player's client
    {
        SetPower(GetPowerType(), (cur >= mh) ? mh : cur);
    }
    else // let player's own client handle normal regen rates.
    {
        m_uint32Values[UNIT_FIELD_POWER4] = (cur >= mh) ? mh : cur;
        SendPowerUpdate(false); // send update to other in-range players
    }
}

uint32 Player::GeneratePetNumber()
{
    uint32 val = m_PetNumberMax + 1;
    for (uint32 i = 1; i < m_PetNumberMax; i++)
        if (m_Pets.find(i) == m_Pets.end())
            return i;                       // found a free one

    return val;
}

void Player::RemovePlayerPet(uint32 pet_number)
{
    std::map<uint32, PlayerPet*>::iterator itr = m_Pets.find(pet_number);
    if (itr != m_Pets.end())
    {
        delete itr->second;
        m_Pets.erase(itr);
    }
    CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", GetLowGUID(), pet_number);
}

void Player::_Relocate(uint32 mapid, const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id)
{
    //this func must only be called when switching between maps!
    WorldPacket data(41);
    if (sendpending && mapid != m_mapId && force_new_world)
    {
        data.SetOpcode(SMSG_TRANSFER_PENDING);

#if VERSION_STRING == Cata
        data.writeBit(0);
        data.writeBit(0);
#endif
        data << mapid;

        m_session->SendPacket(&data);
    }
    bool sendpacket = (mapid == m_mapId);
    //Dismount before teleport and before being removed from world,
    //otherwise we may spawn the active pet while not being in world.
    Dismount();

    if (!sendpacket || force_new_world)
    {
        uint32 status = sInstanceMgr.PreTeleport(mapid, this, instance_id);
        if (status != INSTANCE_OK)
        {
            data.Initialize(SMSG_TRANSFER_ABORTED);

            data << uint32(mapid);
            data << uint32(status);

            m_session->SendPacket(&data);

            return;
        }

        if (instance_id)
            m_instanceId = instance_id;

        if (IsInWorld())
        {
            RemoveFromWorld();
        }

        data.Initialize(SMSG_NEW_WORLD);

#if VERSION_STRING != Cata
        data << uint32(mapid);
        data << v;
        data << float(v.o);
#else
        data << float(v.x);
        data << float(v.o);
        data << float(v.z);
        data << uint32(mapid);
        data << float(v.y);
#endif

        m_session->SendPacket(&data);

        SetMapId(mapid);

    }
    else
    {
        SendTeleportAckPacket(v.x, v.y, v.z, v.o);
    }

    SetPlayerStatus(TRANSFER_PENDING);
    m_sentTeleportPosition = v;
    SetPosition(v);

    if (sendpacket)
    {
        SendTeleportPacket(v.x, v.y, v.z, v.o);
    }

    SpeedCheatReset();

    z_axisposition = 0.0f;
}

void Player::AddItemsToWorld()
{
    Item* pItem;
    for (uint8 i = 0; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        pItem = GetItemInterface()->GetInventoryItem(i);
        if (pItem != nullptr)
        {
            pItem->PushToWorld(m_mapMgr);

            if (i < INVENTORY_SLOT_BAG_END)      // only equipment slots get mods.
            {
                _ApplyItemMods(pItem, i, true, false, true);
            }

            if (i >= CURRENCYTOKEN_SLOT_START && i < CURRENCYTOKEN_SLOT_END)
            {
                UpdateKnownCurrencies(pItem->GetEntry(), true);
            }

            if (pItem->IsContainer() && GetItemInterface()->IsBagSlot(i))
            {
                for (uint32 e = 0; e < pItem->GetItemProperties()->ContainerSlots; ++e)
                {
                    Item* item = (static_cast< Container* >(pItem))->GetItem(static_cast<int16>(e));
                    if (item)
                    {
                        item->PushToWorld(m_mapMgr);
                    }
                }
            }
        }
    }

    UpdateStats();
}

void Player::RemoveItemsFromWorld()
{
    Item* pItem;
    for (uint8 i = 0; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        pItem = m_ItemInterface->GetInventoryItem((int8)i);
        if (pItem)
        {
            if (pItem->IsInWorld())
            {
                if (i < INVENTORY_SLOT_BAG_END)      // only equipment+bags slots get mods.
                {
                    _ApplyItemMods(pItem, static_cast<int16>(i), false, false, true);
                }
                pItem->RemoveFromWorld();
            }

            if (pItem->IsContainer() && GetItemInterface()->IsBagSlot(static_cast<int16>(i)))
            {
                for (uint32 e = 0; e < pItem->GetItemProperties()->ContainerSlots; e++)
                {
                    Item* item = (static_cast< Container* >(pItem))->GetItem(static_cast<int16>(e));
                    if (item && item->IsInWorld())
                    {
                        item->RemoveFromWorld();
                    }
                }
            }
        }
    }

    UpdateStats();
}

uint32 Player::BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    int count = 0;
    if (target == this)
    {
        // we need to send create objects for all items.
        count += GetItemInterface()->m_CreateForPlayer(data);
    }
    count += Unit::BuildCreateUpdateBlockForPlayer(data, target);
    return count;
}

void Player::Kick(uint32 delay /* = 0 */)
{
    if (!delay)
    {
        m_KickDelay = 0;
        _Kick();
    }
    else
    {
        m_KickDelay = delay;
        sEventMgr.AddEvent(this, &Player::_Kick, EVENT_PLAYER_KICK, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Player::_Kick()
{
    if (!m_KickDelay)
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_KICK);
        // remove now
        GetSession()->LogoutPlayer(true);
    }
    else
    {
        if (m_KickDelay < 1500)
        {
            m_KickDelay = 0;
        }
        else
        {
            m_KickDelay -= 1000;
        }
        sChatHandler.BlueSystemMessage(GetSession(), "You will be removed from the server in %u seconds.", (uint32)(m_KickDelay / 1000));
    }
}

void Player::ClearCooldownForSpell(uint32 spell_id)
{
    WorldPacket data(SMSG_CLEAR_COOLDOWN, 12);
    data << uint32_t(spell_id);
    data << uint64_t(GetGUID());
    GetSession()->SendPacket(&data);

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spell_id);
    if (spellInfo == nullptr)
    {
        return;
    }

    for (int i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (PlayerCooldownMap::iterator itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            PlayerCooldownMap::iterator itr2 = itr++;
            if ((i == COOLDOWN_TYPE_CATEGORY && itr2->first == spellInfo->getCategory()) ||
                (i == COOLDOWN_TYPE_SPELL && itr2->first == spellInfo->getId()))
            {
                m_cooldownMap[i].erase(itr2);
            }
        }
    }
}

void Player::ClearCooldownsOnLine(uint32 skill_line, uint32 called_from)
{
    // found an easier way.. loop spells, check skill line
    SpellSet::const_iterator itr = mSpells.begin();
    for (; itr != mSpells.end(); ++itr)
    {
        if ((*itr) == called_from)       // skip calling spell.. otherwise spammies! :D
            continue;

        auto skill_line_ability = objmgr.GetSpellSkill((*itr));
        if (skill_line_ability && skill_line_ability->skilline == skill_line)
            ClearCooldownForSpell((*itr));
    }
}

void Player::ResetAllCooldowns()
{
    uint32 guid = (uint32)GetSelection();

    switch (getClass())
    {
        case WARRIOR:
        {
            ClearCooldownsOnLine(26, guid);
            ClearCooldownsOnLine(256, guid);
            ClearCooldownsOnLine(257, guid);
        }
        break;
        case PALADIN:
        {
            ClearCooldownsOnLine(594, guid);
            ClearCooldownsOnLine(267, guid);
            ClearCooldownsOnLine(184, guid);
        }
        break;
        case HUNTER:
        {
            ClearCooldownsOnLine(50, guid);
            ClearCooldownsOnLine(51, guid);
            ClearCooldownsOnLine(163, guid);
        }
        break;
        case ROGUE:
        {
            ClearCooldownsOnLine(253, guid);
            ClearCooldownsOnLine(38, guid);
            ClearCooldownsOnLine(39, guid);
        }
        break;
        case PRIEST:
        {
            ClearCooldownsOnLine(56, guid);
            ClearCooldownsOnLine(78, guid);
            ClearCooldownsOnLine(613, guid);
        }
        break;

        case DEATHKNIGHT:
        {
            ClearCooldownsOnLine(770, guid);
            ClearCooldownsOnLine(771, guid);
            ClearCooldownsOnLine(772, guid);
        }
        break;

        case SHAMAN:
        {
            ClearCooldownsOnLine(373, guid);
            ClearCooldownsOnLine(374, guid);
            ClearCooldownsOnLine(375, guid);
        }
        break;
        case MAGE:
        {
            ClearCooldownsOnLine(6, guid);
            ClearCooldownsOnLine(8, guid);
            ClearCooldownsOnLine(237, guid);
        }
        break;
        case WARLOCK:
        {
            ClearCooldownsOnLine(355, guid);
            ClearCooldownsOnLine(354, guid);
            ClearCooldownsOnLine(593, guid);
        }
        break;
        case DRUID:
        {
            ClearCooldownsOnLine(573, guid);
            ClearCooldownsOnLine(574, guid);
            ClearCooldownsOnLine(134, guid);
        }
        break;

        default:
            return;
            break;
    }
}

void Player::PushUpdateData(ByteBuffer* data, uint32 updatecount)
{
    // imagine the bytebuffer getting appended from 2 threads at once! :D
    _bufferS.Acquire();

    // unfortunately there is no guarantee that all data will be compressed at a ratio
    // that will fit into 2^16 bytes (stupid client limitation on server packets)
    // so if we get more than 63KB of update data, force an update and then append it
    // to the clean buffer.
    if ((data->size() + bUpdateBuffer.size()) >= 63000)
        ProcessPendingUpdates();

    mUpdateCount += updatecount;
    bUpdateBuffer.append(*data);

    // add to process queue
    if (m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(this);
    }

    _bufferS.Release();
}

void Player::PushOutOfRange(const WoWGuid & guid)
{
    _bufferS.Acquire();
    mOutOfRangeIds << guid;
    ++mOutOfRangeIdCount;

    // add to process queue
    if (m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(this);
    }
    _bufferS.Release();
}

void Player::PushCreationData(ByteBuffer* data, uint32 updatecount)
{
    // imagine the bytebuffer getting appended from 2 threads at once! :D
    _bufferS.Acquire();

    // unfortunately there is no guarantee that all data will be compressed at a ratio
    // that will fit into 2^16 bytes (stupid client limitation on server packets)
    // so if we get more than 63KB of update data, force an update and then append it
    // to the clean buffer.
    if ((data->size() + bCreationBuffer.size() + mOutOfRangeIds.size()) >= 63000)
        ProcessPendingUpdates();

    mCreationCount += updatecount;
    bCreationBuffer.append(*data);

    // add to process queue
    if (m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(this);
    }

    _bufferS.Release();

}

void Player::ProcessPendingUpdates()
{
    _bufferS.Acquire();
    if (!bUpdateBuffer.size() && !mOutOfRangeIds.size() && !bCreationBuffer.size())
    {
        _bufferS.Release();
        return;
    }

    size_t bBuffer_size = (bCreationBuffer.size() > bUpdateBuffer.size() ? bCreationBuffer.size() : bUpdateBuffer.size()) + 10 + (mOutOfRangeIds.size() * 9);
    uint8* update_buffer = new uint8[bBuffer_size];
    size_t c = 0;

    //build out of range updates if creation updates are queued
    if (bCreationBuffer.size() || mOutOfRangeIdCount)
    {
#if VERSION_STRING == Cata
        //get map id
        *(uint16*)&update_buffer[c] = (uint16)GetMapId();
        c += 2;
#endif
        *(uint32*)&update_buffer[c] = ((mOutOfRangeIds.size() > 0) ? (mCreationCount + 1) : mCreationCount);
        c += 4;

        // append any out of range updates
        if (mOutOfRangeIdCount)
        {
            update_buffer[c] = UPDATETYPE_OUT_OF_RANGE_OBJECTS;
            ++c;

            *(uint32*)&update_buffer[c] = mOutOfRangeIdCount;
            c += 4;

            memcpy(&update_buffer[c], mOutOfRangeIds.contents(), mOutOfRangeIds.size());
            c += mOutOfRangeIds.size();
            mOutOfRangeIds.clear();
            mOutOfRangeIdCount = 0;
        }

        if (bCreationBuffer.size())
            memcpy(&update_buffer[c], bCreationBuffer.contents(), bCreationBuffer.size());
        c += bCreationBuffer.size();

        // clear our update buffer
        bCreationBuffer.clear();
        mCreationCount = 0;

        // compress update packet
        // while we said 350 before, I'm gonna make it 500 :D
#if VERSION_STRING != Cata
        if (c < (size_t)worldConfig.server.compressionThreshold || !CompressAndSendUpdateBuffer((uint32)c, update_buffer))
#endif
        {
            // send uncompressed packet -> because we failed
            m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)c, update_buffer);
        }
    }

    if (bUpdateBuffer.size())
    {
        c = 0;

#if VERSION_STRING == Cata
        //get map id
        *(uint16*)&update_buffer[c] = (uint16)GetMapId();
        c += 2;
#endif

        *(uint32*)&update_buffer[c] = ((mOutOfRangeIds.size() > 0) ? (mUpdateCount + 1) : mUpdateCount);
        c += 4;

        //update_buffer[c] = 1;
        memcpy(&update_buffer[c], bUpdateBuffer.contents(), bUpdateBuffer.size());
        c += bUpdateBuffer.size();

        // clear our update buffer
        bUpdateBuffer.clear();
        mUpdateCount = 0;

        // compress update packet
        // while we said 350 before, I'm gonna make it 500 :D
#if VERSION_STRING != Cata
        if (c < (size_t)worldConfig.server.compressionThreshold || !CompressAndSendUpdateBuffer((uint32)c, update_buffer))
#endif
        {
            // send uncompressed packet -> because we failed
            m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)c, update_buffer);
        }
    }

    bProcessPending = false;
    _bufferS.Release();
    delete[] update_buffer;

    // send any delayed packets
    WorldPacket* pck;
    while (delayedPackets.size())
    {
        pck = delayedPackets.next();
        //printf("Delayed packet opcode %u sent.\n", pck->GetOpcode());
        m_session->SendPacket(pck);
        delete pck;
    }

    // resend speed if needed
    if (resend_speed)
    {
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
        setSpeedForType(TYPE_FLY, getSpeedForType(TYPE_FLY));
        resend_speed = false;
    }
}

bool Player::CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer)
{
    uint32 destsize = size + size / 10 + 16;
    int rate = worldConfig.getIntRate(INTRATE_COMPRESSION);
    if (size >= 40000 && rate < 6)
        rate = 6;

    // set up stream
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;

    if (deflateInit(&stream, rate) != Z_OK)
    {
        LOG_ERROR("deflateInit failed.");
        return false;
    }

    uint8* buffer = new uint8[destsize];

    // set up stream pointers
    stream.next_out = (Bytef*)buffer + 4;
    stream.avail_out = destsize;
    stream.next_in = (Bytef*)update_buffer;
    stream.avail_in = size;

    // call the actual process
    if (deflate(&stream, Z_NO_FLUSH) != Z_OK ||
        stream.avail_in != 0)
    {
        LOG_ERROR("deflate failed.");
        delete[] buffer;
        return false;
    }

    // finish the deflate
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        LOG_ERROR("deflate failed: did not end stream");
        delete[] buffer;
        return false;
    }

    // finish up
    if (deflateEnd(&stream) != Z_OK)
    {
        LOG_ERROR("deflateEnd failed.");
        delete[] buffer;
        return false;
    }

    // fill in the full size of the compressed stream

    *(uint32*)&buffer[0] = size;

    // send it
#if VERSION_STRING != Cata
    m_session->OutPacket(SMSG_COMPRESSED_UPDATE_OBJECT, (uint16)stream.total_out + 4, buffer);
#else
    m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)stream.total_out + 4, buffer);
#endif

    // cleanup memory
    delete[] buffer;

    return true;
}

void Player::ClearAllPendingUpdates()
{
    _bufferS.Acquire();
    bProcessPending = false;
    mUpdateCount = 0;
    bUpdateBuffer.clear();
    _bufferS.Release();
}

void Player::AddSplinePacket(uint64 guid, ByteBuffer* packet)
{
    SplineMap::iterator itr = _splineMap.find(guid);
    if (itr != _splineMap.end())
    {
        delete itr->second;
        _splineMap.erase(itr);
    }
    _splineMap.insert(SplineMap::value_type(guid, packet));
}

ByteBuffer* Player::GetAndRemoveSplinePacket(uint64 guid)
{
    SplineMap::iterator itr = _splineMap.find(guid);
    if (itr != _splineMap.end())
    {
        ByteBuffer* buf = itr->second;
        _splineMap.erase(itr);
        return buf;
    }
    return nullptr;
}

void Player::ClearSplinePackets()
{
    SplineMap::iterator it2;
    for (SplineMap::iterator itr = _splineMap.begin(); itr != _splineMap.end();)
    {
        it2 = itr;
        ++itr;
        delete it2->second;
        _splineMap.erase(it2);
    }
    _splineMap.clear();
}

bool Player::ExitInstance()
{
    if (!m_bgEntryPointX)
        return false;

    RemoveFromWorld();

    SafeTeleport(m_bgEntryPointMap, m_bgEntryPointInstance, LocationVector(
        m_bgEntryPointX, m_bgEntryPointY, m_bgEntryPointZ, m_bgEntryPointO));

    return true;
}

void Player::SaveEntryPoint(uint32 mapId)
{
    if (IS_INSTANCE(GetMapId()))
        return; // don't save if we're not on the main continent.
    //otherwise we could end up in an endless loop :P
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(mapId);

    if (pMapinfo)
    {
        m_bgEntryPointX = pMapinfo->repopx;
        m_bgEntryPointY = pMapinfo->repopy;
        m_bgEntryPointZ = pMapinfo->repopz;
        m_bgEntryPointO = GetOrientation();
        m_bgEntryPointMap = pMapinfo->repopmapid;
        m_bgEntryPointInstance = GetInstanceID();
    }
    else
    {
        m_bgEntryPointMap = 0;
        m_bgEntryPointX = 0;
        m_bgEntryPointY = 0;
        m_bgEntryPointZ = 0;
        m_bgEntryPointO = 0;
        m_bgEntryPointInstance = 0;
    }
}

bool Player::IsInCity()
{
    auto at = GetMapMgr()->GetArea(GetPositionX(), GetPositionY(), GetPositionZ());
    if (at != nullptr)
    {
        ::DBC::Structures::AreaTableEntry const* zt = nullptr;
        if (at->zone)
            zt = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);

        bool areaIsCity = at->flags & AREA_CITY_AREA || at->flags & AREA_CITY;
        bool zoneIsCity = zt && (zt->flags & AREA_CITY_AREA || zt->flags & AREA_CITY);

        return (areaIsCity || zoneIsCity);
    }
    else
    {
        return false;
    }
}

void Player::ZoneUpdate(uint32 ZoneId)
{
    uint32 oldzone = m_zoneId;
    if (m_zoneId != ZoneId)
    {
        SetZoneId(ZoneId);
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_AREA);
    }

    /* how the f*ck is this happening */
    if (m_playerInfo == nullptr)
    {
        m_playerInfo = objmgr.GetPlayerInfo(GetLowGUID());
        if (m_playerInfo == nullptr)
        {
            m_session->Disconnect();
            return;
        }
    }

    m_playerInfo->lastZone = ZoneId;
    sHookInterface.OnZone(this, ZoneId, oldzone);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnZoneChange)(this, ZoneId, oldzone);

    auto at = GetMapMgr()->GetArea(GetPositionX(), GetPositionY(), GetPositionZ());
    if (at && (at->team == AREAC_SANCTUARY || at->flags & AREA_SANCTUARY))
    {
        Unit* pUnit = (GetSelection() == 0) ? 0 : (m_mapMgr ? m_mapMgr->GetUnit(GetSelection()) : 0);
        if (pUnit && DuelingWith != pUnit)
        {
            EventAttackStop();
            smsg_AttackStop(pUnit);
        }

        if (isCastingNonMeleeSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                if (getCurrentSpell(CurrentSpellType(i)) != nullptr)
                {
                    Unit* target = getCurrentSpell(CurrentSpellType(i))->GetUnitTarget();
                    if (target != nullptr && target != DuelingWith && target != this)
                    {
                        interruptSpellWithSpellType(CurrentSpellType(i));
                    }
                }
            }
        }
    }

    at = MapManagement::AreaManagement::AreaStorage::GetAreaById(ZoneId);

    if (!m_channels.empty() && at)
    {
        // change to zone name, not area name
        for (std::set<Channel*>::iterator itr = m_channels.begin(), nextitr; itr != m_channels.end(); itr = nextitr)
        {
            nextitr = itr;
            ++nextitr;
            Channel* chn;
            chn = (*itr);
            // Check if this is a custom channel (i.e. global)
            if (!((*itr)->m_flags & CHANNEL_FLAGS_CUSTOM))  // 0x10
                continue;

            if (chn->m_flags & CHANNEL_FLAGS_LFG)
                continue;

            char updatedName[95];

            auto chat_channels = sChatChannelsStore.LookupEntry(chn->m_id);
            if (!chat_channels)
            {
                LOG_ERROR("Invalid channel entry %u for %s", chn->m_id, chn->m_name.c_str());
                return;
            }

#if VERSION_STRING != Cata
            snprintf(updatedName, 95, chat_channels->name_pattern[0], at->area_name[0]);
#else
            snprintf(updatedName, 95, chat_channels->name_pattern, at->area_name);
#endif
            Channel* newChannel = channelmgr.GetCreateChannel(updatedName, nullptr, chn->m_id);
            if (newChannel == nullptr)
            {
                LOG_ERROR("Could not create channel %s!", updatedName);
                return; // whoops?
            }

            if (chn != newChannel)   // perhaps there's no need
            {
                // join new channel
                newChannel->AttemptJoin(this, "");
                // leave the old channel

                chn->Part(this, false);
            }
        }
    }

    SendInitialWorldstates();

    UpdateChannels(static_cast<int16>(ZoneId));
}

void Player::UpdateChannels(uint16 AreaID)
{
    std::set<Channel*>::iterator i;
    Channel* c;
    std::string channelname, AreaName;


    if (GetMapId() == 450)
        AreaID = 2917;
    else if (GetMapId() == 449)
        AreaID = 2918;

    auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(AreaID);
    if (!at2)
    {
        assert(false && ">>> REPORT THIS ERROR <<< - Could not find area with ID: " && AreaID);
        return;     // Zyres: CID 123873
    }

    //Check for instances?
    if (!AreaID || AreaID == 0xFFFF)
    {
        MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());
        if (IS_INSTANCE(GetMapId()))
            AreaName = pMapinfo->name;
        else
            return;//How'd we get here?
    }
    else
    {
        AreaName = at2->area_name[0];
        if (AreaName.length() < 2)
        {
            MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());
            AreaName = pMapinfo->name;
        }
    }

    for (i = m_channels.begin(); i != m_channels.end();)
    {
        c = *i;
        ++i;

        if (!c->m_general || c->m_name == "LookingForGroup")//Not an updatable channel.
            continue;

        if (strstr(c->m_name.c_str(), "General"))
            channelname = "General";
        else if (strstr(c->m_name.c_str(), "Trade"))
            channelname = "Trade";
        else if (strstr(c->m_name.c_str(), "LocalDefense"))
            channelname = "LocalDefense";
        else if (strstr(c->m_name.c_str(), "GuildRecruitment"))
            channelname = "GuildRecruitment";
        else
            continue;//Those 4 are the only ones we want updated.
        channelname += " - ";
        if ((strstr(c->m_name.c_str(), "Trade") || strstr(c->m_name.c_str(), "GuildRecruitment")) && (at2->flags & AREA_CITY || at2->flags & AREA_CITY_AREA))
        {
            channelname += "City";
        }
        else
            channelname += AreaName;

        Channel* chn = channelmgr.GetCreateChannel(channelname.c_str(), this, c->m_id);
        if (chn != nullptr && !chn->HasMember(this))
        {
            c->Part(this);
            chn->AttemptJoin(this, nullptr);
        }
    }
}

#if VERSION_STRING != Cata
void Player::SendTradeUpdate()
{
    Player* pTarget = GetTradeTarget();
    if (!pTarget)
        return;

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, 532);

    data << uint8(1);
    data << uint32(0x19);
    data << m_tradeSequence;
    data << m_tradeSequence++;
    data << mTradeGold;
    data << uint32(0);

    uint8 count = 0;
    // Items
    for (uint32 Index = 0; Index < 7; ++Index)
    {
        Item* pItem = mTradeItems[Index];
        if (pItem != 0)
        {
            count++;
            ItemProperties const* pProto = pItem->GetItemProperties();
            ARCEMU_ASSERT(pProto != NULL);

            data << uint8(Index);

            data << uint32(pProto->ItemId);
            data << uint32(pProto->DisplayInfoID);
            data << uint32(pItem->GetStackCount());    // Amount           OK

            // Enchantment stuff
            data << uint32(0);                                            // unknown
            data << uint64(pItem->GetGiftCreatorGUID());    // gift creator     OK
            data << uint32(pItem->GetEnchantmentId(0));    // Item Enchantment OK
            for (uint8 i = 2; i < 5; i++)                                // Gem enchantments
            {
                if (pItem->GetEnchantment(i) != NULL && pItem->GetEnchantment(i)->Enchantment != NULL)
                    data << uint32(pItem->GetEnchantment(i)->Enchantment->Id);
                else
                    data << uint32(0);
            }
            data << uint64(pItem->GetCreatorGUID());        // item creator     OK
            data << uint32(pItem->GetCharges(0));    // Spell Charges    OK
            data << uint32(pItem->GetItemRandomSuffixFactor());   // seems like time stamp or something like that
            data << uint32(pItem->GetItemRandomPropertyId());
            data << uint32(pProto->LockId);                                        // lock ID          OK
            data << uint32(pItem->GetDurabilityMax());
            data << uint32(pItem->GetDurability());
        }
    }
    data.resize(21 + count * 73);
    pTarget->SendPacket(&data);
}
#endif

void Player::RequestDuel(Player* pTarget)
{
    // We Already Dueling or have already Requested a Duel

    if (DuelingWith != nullptr)
        return;

    if (m_duelState != DUEL_STATE_FINISHED)
        return;

    SetDuelState(DUEL_STATE_REQUESTED);

    //Setup Duel
    pTarget->DuelingWith = this;
    DuelingWith = pTarget;

    //Get Flags position
    float dist = CalcDistance(pTarget);
    dist = dist * 0.5f; //half way
    float x = (GetPositionX() + pTarget->GetPositionX() * dist) / (1 + dist) + cos(GetOrientation() + (M_PI_FLOAT / 2)) * 2;
    float y = (GetPositionY() + pTarget->GetPositionY() * dist) / (1 + dist) + sin(GetOrientation() + (M_PI_FLOAT / 2)) * 2;
    float z = (GetPositionZ() + pTarget->GetPositionZ() * dist) / (1 + dist);

    //Create flag/arbiter
    GameObject* pGameObj = GetMapMgr()->CreateGameObject(21680);
    pGameObj->CreateFromProto(21680, GetMapId(), x, y, z, GetOrientation());

    //Spawn the Flag
    pGameObj->setUInt64Value(OBJECT_FIELD_CREATED_BY, GetGUID());
    pGameObj->SetFaction(GetFaction());
    pGameObj->SetLevel(getLevel());

    //Assign the Flag
    SetDuelArbiter(pGameObj->GetGUID());
    pTarget->SetDuelArbiter(pGameObj->GetGUID());

    pGameObj->PushToWorld(m_mapMgr);

    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << GetGUID();
    pTarget->GetSession()->SendPacket(&data);
}

void Player::DuelCountdown()
{
    if (DuelingWith == nullptr)
        return;

    m_duelCountdownTimer -= 1000;

    if (static_cast<int32>(m_duelCountdownTimer) < 0)
        m_duelCountdownTimer = 0;

    if (m_duelCountdownTimer == 0)
    {
        // Start Duel.
        SetPower(POWER_TYPE_RAGE, 0);
        DuelingWith->SetPower(POWER_TYPE_RAGE, 0);

        //Give the players a Team
        DuelingWith->SetDuelTeam(1);  // Duel Requester
        SetDuelTeam(2);

        SetDuelState(DUEL_STATE_STARTED);
        DuelingWith->SetDuelState(DUEL_STATE_STARTED);

        sEventMgr.AddEvent(this, &Player::DuelBoundaryTest, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        sEventMgr.AddEvent(DuelingWith, &Player::DuelBoundaryTest, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Player::DuelBoundaryTest()
{
    //check if in bounds
    if (!IsInWorld())
        return;

    GameObject* pGameObject = GetMapMgr()->GetGameObject(GET_LOWGUID_PART(GetDuelArbiter()));
    if (!pGameObject)
    {
        EndDuel(DUEL_WINNER_RETREAT);
        return;
    }

    float Dist = CalcDistance(pGameObject);

    if (Dist > 75.0f)
    {
        // Out of bounds
        if (m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
        {
            // we already know, decrease timer by 500
            m_duelCountdownTimer -= 500;
            if (m_duelCountdownTimer == 0)
            {
                // Times up :p
                DuelingWith->EndDuel(DUEL_WINNER_RETREAT);
            }
        }
        else
        {
            // we just went out of bounds
            // set timer
            m_duelCountdownTimer = 10000;

            // let us know

            m_session->OutPacket(SMSG_DUEL_OUTOFBOUNDS, 4, &m_duelCountdownTimer);

            m_duelStatus = DUEL_STATUS_OUTOFBOUNDS;
        }
    }
    else
    {
        // we're in range
        if (m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
        {
            // just came back in range
            m_session->OutPacket(SMSG_DUEL_INBOUNDS);
            m_duelStatus = DUEL_STATUS_INBOUNDS;
        }
    }
}

void Player::EndDuel(uint8 WinCondition)
{
    if (m_duelState == DUEL_STATE_FINISHED)
    {
        //if loggingout player requested a duel then we have to make the cleanups
        if (GET_LOWGUID_PART(GetDuelArbiter()))
        {
            GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(GET_LOWGUID_PART(GetDuelArbiter())) : 0;

            if (arbiter != nullptr)
            {
                arbiter->RemoveFromWorld(true);
                delete arbiter;
            }

            //we do not wish to lock the other player in duel state
            DuelingWith->SetDuelArbiter(0);
            DuelingWith->SetDuelTeam(0);
            SetDuelArbiter(0);
            SetDuelTeam(0);
            sEventMgr.RemoveEvents(DuelingWith, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);
            sEventMgr.RemoveEvents(DuelingWith, EVENT_PLAYER_DUEL_COUNTDOWN);
            DuelingWith->DuelingWith = nullptr;
            DuelingWith = nullptr;
            //the duel did not start so we are not in combat or cast any spells yet.
        }
        return;
    }

    // Remove the events
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_DUEL_COUNTDOWN);
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);

    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; ++x)
    {
        if (m_auras[x] == nullptr)
            continue;
        if (m_auras[x]->WasCastInDuel())
            m_auras[x]->Remove();
    }

    m_duelState = DUEL_STATE_FINISHED;

    if (DuelingWith == nullptr)
        return;

    sEventMgr.RemoveEvents(DuelingWith, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);
    sEventMgr.RemoveEvents(DuelingWith, EVENT_PLAYER_DUEL_COUNTDOWN);

    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; ++x)
    {
        if (DuelingWith->m_auras[x] == nullptr)
            continue;
        if (DuelingWith->m_auras[x]->WasCastInDuel())
            DuelingWith->m_auras[x]->Remove();
    }

    DuelingWith->m_duelState = DUEL_STATE_FINISHED;

    //Announce Winner
    WorldPacket data(SMSG_DUEL_WINNER, 500);
    data << uint8(WinCondition);
    data << GetName() << DuelingWith->GetName();
    SendMessageToSet(&data, true);

    data.Initialize(SMSG_DUEL_COMPLETE);
    data << uint8(1);
    SendMessageToSet(&data, true);

    //Send hook OnDuelFinished

    if (WinCondition != 0)
        sHookInterface.OnDuelFinished(DuelingWith, this);
    else
        sHookInterface.OnDuelFinished(this, DuelingWith);

    //Clear Duel Related Stuff

    GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(GET_LOWGUID_PART(GetDuelArbiter())) : 0;

    if (arbiter != nullptr)
    {
        arbiter->RemoveFromWorld(true);
        delete arbiter;
    }

    SetDuelArbiter(0);
    SetDuelTeam(0);
    DuelingWith->SetDuelArbiter(0);
    DuelingWith->SetDuelTeam(0);

    EventAttackStop();
    DuelingWith->EventAttackStop();

    // Call off pet
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->CombatStatus.Vanished();
        (*itr)->GetAIInterface()->SetUnitToFollow(this);
        (*itr)->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, *itr, 0);
        (*itr)->GetAIInterface()->WipeTargetList();
    }

    std::list<Pet*> duelingWithSummons = DuelingWith->GetSummons();
    for (std::list<Pet*>::iterator itr = duelingWithSummons.begin(); itr != duelingWithSummons.end(); ++itr)
    {
        (*itr)->CombatStatus.Vanished();
        (*itr)->GetAIInterface()->SetUnitToFollow(this);
        (*itr)->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, *itr, 0);
        (*itr)->GetAIInterface()->WipeTargetList();
    }

    // removing auras that kills players after if low HP
    /*RemoveNegativeAuras(); NOT NEEDED. External targets can always gank both duelers with DoTs. :D
    DuelingWith->RemoveNegativeAuras();*/
    //Same as above only cleaner.
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_REMOVABLE_AURAS_END; x++)
    {
        if (DuelingWith->m_auras[x])
        {
            if (DuelingWith->m_auras[x]->WasCastInDuel())
                DuelingWith->m_auras[x]->Remove();
        }
        if (m_auras[x])
        {
            if (m_auras[x]->WasCastInDuel())
                m_auras[x]->Remove();
        }
    }

    //Stop Players attacking so they don't kill the other player
    m_session->OutPacket(SMSG_CANCEL_COMBAT);
    DuelingWith->m_session->OutPacket(SMSG_CANCEL_COMBAT);

    smsg_AttackStop(DuelingWith);
    DuelingWith->smsg_AttackStop(this);

    DuelingWith->m_duelCountdownTimer = 0;
    m_duelCountdownTimer = 0;

    DuelingWith->DuelingWith = nullptr;
    DuelingWith = nullptr;
}

void Player::SendMirrorTimer(MirrorTimerTypes Type, uint32 max, uint32 current, int32 regen)
{
    if (int(max) == -1)
    {
        if (int(current) != -1)
            StopMirrorTimer(Type);
        return;
    }
    WorldPacket data(SMSG_START_MIRROR_TIMER, 21);
    data << uint32(Type);
    data << uint32(current);
    data << uint32(max);
    data << int32(regen);
    data << uint8(0);
    data << uint32(0);                   // spell id
    GetSession()->SendPacket(&data);
}

void Player::StopMirrorTimer(MirrorTimerTypes Type)
{
    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    GetSession()->SendPacket(&data);
}

void Player::EventTeleport(uint32 mapid, float x, float y, float z)
{
    SafeTeleport(mapid, 0, LocationVector(x, y, z));
}

void Player::EventTeleportTaxi(uint32 mapid, float x, float y, float z)
{
    if (mapid == 530 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01))
    {
        WorldPacket msg(CMSG_SERVER_BROADCAST, 50);
        msg << uint32(3);
        msg << GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_BC);
        msg << uint8(0);
        m_session->SendPacket(&msg);

        RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return;
    }
    _Relocate(mapid, LocationVector(x, y, z), (mapid == GetMapId() ? false : true), true, 0);
    ForceZoneUpdate();
}

void Player::ApplyLevelInfo(LevelInfo* Info, uint32 Level)
{
    ARCEMU_ASSERT(Info != NULL);

    // Apply level
    uint32 PreviousLevel = getLevel();
    setLevel(Level);

    // Set stats
    for (uint8 i = 0; i < 5; ++i)
    {
        BaseStats[i] = Info->Stat[i];
        CalcStat(i);
    }

    // Set health / mana
    SetHealth(Info->HP);
    SetMaxHealth(Info->HP);
    SetMaxPower(POWER_TYPE_MANA, Info->Mana);
    SetPower(POWER_TYPE_MANA, Info->Mana);


    if (Level > PreviousLevel)
    {
        if (Level > 9)
            SetTalentPointsForAllSpec(Level - 9);
    }
    else
    {
        if (Level != PreviousLevel)
            Reset_AllTalents();
    }

    // Set base fields
    SetBaseHealth(Info->HP);
    SetBaseMana(Info->Mana);

    _UpdateMaxSkillCounts();
    UpdateStats();
    //UpdateChances();
#if VERSION_STRING > TBC
    UpdateGlyphs();
#endif
    m_playerInfo->lastLevel = Level;
#if VERSION_STRING > TBC
    GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
#endif
    //VLack: 3.1.3, as a final step, send the player's talents, this will set the talent points right too...
    smsg_TalentsInfo(false);

    LOG_DETAIL("Player %s set parameters to level %u", GetName(), Level);
}

void Player::BroadcastMessage(const char* Format, ...)
{
    va_list l;
    va_start(l, Format);
    char Message[1024];
    vsnprintf(Message, 1024, Format, l);
    va_end(l);

    WorldPacket* data = sChatHandler.FillSystemMessageData(Message);
    m_session->SendPacket(data);
    delete data;
}

float Player::CalcRating(PlayerCombatRating index)
{
    uint32 level = getLevel();
    if (level > 100)
        level = 100;

    uint32 rating = getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + index);

    DBC::Structures::GtCombatRatingsEntry const* combat_rating_entry = sGtCombatRatingsStore.LookupEntry(index * 100 + level - 1);
    if (combat_rating_entry == nullptr)
        return float(rating);
    else
        return (rating / combat_rating_entry->val);
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O)
{
    return SafeTeleport(MapID, InstanceID, LocationVector(X, Y, Z, O));
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, const LocationVector & vec)
{
#if VERSION_STRING != Cata
    // Checking if we have a unit whose waypoints are shown
    // If there is such, then we "unlink" it
    // Failing to do so leads to a crash if we try to show some other Unit's wps, after the map was shut down
    if (waypointunit != NULL)
        waypointunit->hideWayPoints(this);

    waypointunit = NULL;

    SpeedCheatDelay(10000);

    if (GetTaxiState())
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TELEPORT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_DISMOUNT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_INTERPOLATE);
        SetTaxiState(false);
        SetTaxiPath(NULL);
        UnSetTaxiPos();
        m_taxi_ride_time = 0;
        setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
    }

    if (obj_movement_info.transporter_info.guid)
    {
        Transporter* pTrans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.transporter_info.guid));
        if (pTrans)
        {
            pTrans->RemovePassenger(this);
            obj_movement_info.transporter_info.guid = 0;
        }
    }

    bool instance = false;
    MySQLStructure::MapInfo const* mi = sMySQLStore.getWorldMapInfo(MapID);

    if (InstanceID && (uint32)m_instanceId != InstanceID)
    {
        instance = true;
        this->SetInstanceID(InstanceID);
    }
    else if (m_mapId != MapID)
    {
        instance = true;
    }

    // make sure player does not drown when teleporting from under water
    if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
        // can only fly in outlands or northrend (northrend requires cold weather flying)
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    // Exit vehicle before teleporting
    if (GetVehicleBase() != NULL)
        GetVehicleBase()->GetVehicleComponent()->EjectPassenger(this);

    // Lookup map info
    if (mi && mi->flags & WMI_INSTANCE_XPACK_01 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        WorldPacket msg(SMSG_MOTD, 50); // Need to be replaced with correct one !
        msg << uint32(3) << GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_BC) << uint8(0);
        m_session->SendPacket(&msg);
        return false;
    }
    if (mi && mi->flags & WMI_INSTANCE_XPACK_02 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        WorldPacket msg(SMSG_MOTD, 50); // Need to be replaced with correct one !
        msg << uint32(3) << GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_WOTLK) << uint8(0);
        m_session->SendPacket(&msg);
        return false;
    }

    // cebernic: cleanup before teleport
    // seems BFleaveOpcode was breakdown,that will be causing big BUG with player leaving from the BG
    // now this much better:D RemoveAura & BG_DESERTER going to well as you go out from BG.
    if (m_bg && m_bg->GetMapMgr() && GetMapMgr()->GetMapInfo()->mapid != MapID)
    {
        m_bg->RemovePlayer(this, false);
    }

    _Relocate(MapID, vec, true, instance, InstanceID);
    SpeedCheatReset();
    ForceZoneUpdate();
#else

    if (waypointunit != nullptr)
        waypointunit->hideWayPoints(this);

    waypointunit = nullptr;

    SpeedCheatDelay(10000);

    if (GetTaxiState())
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TELEPORT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_DISMOUNT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_INTERPOLATE);
        SetTaxiState(false);
        SetTaxiPath(nullptr);
        UnSetTaxiPos();
        m_taxi_ride_time = 0;
        setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
    }

    if (obj_movement_info.getTransportGuid())
    {
        Transporter* pTrans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.getTransportGuid()));
        if (pTrans)
        {
            pTrans->RemovePassenger(this);
            obj_movement_info.clearTransportData();
        }
    }

    bool instance = false;
    MySQLStructure::MapInfo const* mi = sMySQLStore.getWorldMapInfo(MapID);

    if (InstanceID && (uint32)m_instanceId != InstanceID)
    {
        instance = true;
        this->SetInstanceID(InstanceID);
    }
    else if (m_mapId != MapID)
    {
        instance = true;
    }

    // make sure player does not drown when teleporting from under water
    if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
        // can only fly in outlands or northrend (northrend requires cold weather flying)
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    // Exit vehicle before teleporting
    if (GetVehicleBase() != nullptr)
        GetVehicleBase()->GetVehicleComponent()->EjectPassenger(this);

    // Lookup map info
    if (mi && mi->flags & WMI_INSTANCE_XPACK_01 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        WorldPacket msg(SMSG_MOTD, 50); // Need to be replaced with correct one !
        msg << uint32(3) << GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_BC) << uint8(0);
        m_session->SendPacket(&msg);
        return false;
    }
    if (mi && mi->flags & WMI_INSTANCE_XPACK_02 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        WorldPacket msg(SMSG_MOTD, 50); // Need to be replaced with correct one !
        msg << uint32(3) << GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_WOTLK) << uint8(0);
        m_session->SendPacket(&msg);
        return false;
    }

    // cebernic: cleanup before teleport
    // seems BFleaveOpcode was breakdown,that will be causing big BUG with player leaving from the BG
    // now this much better:D RemoveAura & BG_DESERTER going to well as you go out from BG.
    if (m_bg && m_bg->GetMapMgr() && GetMapMgr()->GetMapInfo()->mapid != MapID)
    {
        m_bg->RemovePlayer(this, false);
    }

    if (GetMapId() == MapID)
    {
        if (GetSession())
        {
            SetPlayerStatus(TRANSFER_PENDING);
            m_sentTeleportPosition = vec;

            SetPosition(vec);
            SendTeleportPacket(vec.x, vec.y, vec.z, vec.o);
        }

        SpeedCheatReset();
        ForceZoneUpdate();
    }
    else
    {
        // Do normal stuff here!
        _Relocate(MapID, vec, true, instance, InstanceID);

        SpeedCheatReset();
        ForceZoneUpdate();
    }
#endif
    return true;
}

void Player::ForceZoneUpdate()
{
    if (!GetMapMgr()) return;

    auto at = this->GetArea();
    if (!at) return;

    if (at->zone && at->zone != m_zoneId)
        ZoneUpdate(at->zone);

    SendInitialWorldstates();
}

void Player::SafeTeleport(MapMgr* mgr, const LocationVector & vec)
{
    if (mgr == nullptr)
        return;

    SpeedCheatDelay(10000);

    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
        // can only fly in outlands or northrend (northrend requires cold weather flying)
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    if (IsInWorld())
        RemoveFromWorld();

    m_mapId = mgr->GetMapId();
    m_instanceId = mgr->GetInstanceID();
    WorldPacket data(SMSG_TRANSFER_PENDING, 20);
    data << mgr->GetMapId();
    GetSession()->SendPacket(&data);

    data.Initialize(SMSG_NEW_WORLD);
    data << mgr->GetMapId();
    data << vec;
    data << vec.o;
    GetSession()->SendPacket(&data);

    SetPlayerStatus(TRANSFER_PENDING);
    m_sentTeleportPosition = vec;
    SetPosition(vec);
    SpeedCheatReset();
    ForceZoneUpdate();
}

void Player::SetGuildId(uint32 guildId)
{
#if VERSION_STRING != Cata
    if (IsInWorld())
    {
        const uint16 field = PLAYER_GUILDID;
        sEventMgr.AddEvent(static_cast<Object*>(this), &Object::setUInt32Value, field, guildId, EVENT_PLAYER_SEND_PACKET, 1,
                           1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        setUInt32Value(PLAYER_GUILDID, guildId);
    }
#else
    if (IsInWorld())
    {
        m_GuildId = guildId;
        if (m_GuildId == 0)
        {
            setUInt64Value(OBJECT_FIELD_DATA, 0);
            setUInt32Value(OBJECT_FIELD_TYPE, getUInt32Value(OBJECT_FIELD_TYPE) | 0x00010000);
        }
        else
        {
            setUInt64Value(OBJECT_FIELD_DATA, m_GuildId);
            setUInt32Value(OBJECT_FIELD_TYPE, getUInt32Value(OBJECT_FIELD_TYPE) & ~0x00010000);
        }

        //ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_GUILD_LEVEL_ENABLED, guildId != 0 );
        setUInt16Value(OBJECT_FIELD_TYPE, 1, m_GuildId != 0);
    }
#endif
}

#if VERSION_STRING == Cata
void Player::SetInGuild(uint32 guildId)
{
    if (IsInWorld())
    {
        m_GuildId = guildId;
        if (m_GuildId == 0)
        {
            setUInt64Value(OBJECT_FIELD_DATA, 0);
            setUInt32Value(OBJECT_FIELD_TYPE, getUInt32Value(OBJECT_FIELD_TYPE) | 0x00010000);
        }
        else
        {
            setUInt64Value(OBJECT_FIELD_DATA, m_GuildId);
            setUInt32Value(OBJECT_FIELD_TYPE, getUInt32Value(OBJECT_FIELD_TYPE) & ~0x00010000);
        }

        ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_GUILD_LVL_ENABLED, guildId != 0 );
        setUInt16Value(OBJECT_FIELD_TYPE, 1, m_GuildId != 0);
    }
}

Guild* Player::GetGuild()
{
    uint32 guildId = GetGuildId();
    return guildId ? sGuildMgr.getGuildById(guildId) : NULL;
}

uint32 Player::GetGuildIdFromDB(uint64 guid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT guildId, playerGuid FROM guild_member WHERE playerGuid = %u", Arcemu::Util::GUID_LOPART(guid));
    if (result)
    {
        Field* fields = result->Fetch();
        return fields[0].GetUInt32();
    }
    else
        return 0;
}

int8 Player::GetRankFromDB(uint64 guid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT playerGuid, rank FROM guild_member WHERE playerGuid = %u", Arcemu::Util::GUID_LOPART(guid));
    if (result)
    {
        Field* fields = result->Fetch();
        return fields[1].GetUInt8();
    }
    else
        return -1;
}

std::string Player::GetGuildName()
{
    return GetGuildId() ? sGuildMgr.getGuildById(GetGuildId())->getName() : "";
}
#endif

void Player::SetGuildRank(uint32 guildRank)
{
    if (IsInWorld())
    {
        const uint16 field = PLAYER_GUILDRANK;
        sEventMgr.AddEvent(static_cast<Object*>(this), &Object::setUInt32Value, field, guildRank, EVENT_PLAYER_SEND_PACKET, 1,
                           1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        setUInt32Value(PLAYER_GUILDRANK, guildRank);
    }
}

void Player::UpdatePvPArea()
{
    auto at = this->GetArea();
    if (at == nullptr)
        return;

    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
    {
        if (IsPvPFlagged())
            RemovePvPFlag();
        else
            StopPvPTimer();
        RemoveFFAPvPFlag();
        return;
    }

    // This is where all the magic happens :P
    if ((at->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at->team == AREAC_HORDE_TERRITORY && IsTeamHorde()))
    {
        if (!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
        {
            // I'm flagged and I just walked into a zone of my type. Start the 5min counter.
            ResetPvPTimer();
        }
        return;
    }
    else
    {
        //Enemy city check
        if (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY)
        {
            if ((at->team == AREAC_ALLIANCE_TERRITORY && IsTeamHorde()) || (at->team == AREAC_HORDE_TERRITORY && IsTeamAlliance()))
            {
                if (!IsPvPFlagged())
                    SetPvPFlag();
                else
                    StopPvPTimer();
                return;
            }
        }

        //fix for zone areas.
        if (at->zone)
        {
            auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
            if (at2 && ((at2->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && IsTeamHorde())))
            {
                if (!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
                {
                    // I'm flagged and I just walked into a zone of my type. Start the 5min counter.
                    ResetPvPTimer();
                }
                return;
            }
            //enemy territory check
            if (at2 && (at2->flags & AREA_CITY_AREA || at2->flags & AREA_CITY))
            {
                if ((at2->team == AREAC_ALLIANCE_TERRITORY && IsTeamHorde()) || (at2->team == AREAC_HORDE_TERRITORY && IsTeamAlliance()))
                {
                    if (!IsPvPFlagged())
                        SetPvPFlag();
                    else
                        StopPvPTimer();
                    return;
                }
            }
        }

        // I just walked into a sanctuary area
        // Force remove flag me if I'm not already.
        if (at->team == AREAC_SANCTUARY || at->flags & AREA_SANCTUARY)
        {
            if (IsPvPFlagged())
                RemovePvPFlag();
            else
                StopPvPTimer();

            RemoveFFAPvPFlag();
            SetSanctuaryFlag();
        }
        else
        {
            // if we are not in a sanctuary we don't need this flag
            RemoveSanctuaryFlag();

            //contested territory
            if (worldConfig.getRealmType() == REALM_PVP)
            {
                //automatically sets pvp flag on contested territories.
                if (!IsPvPFlagged())
                    SetPvPFlag();
                else
                    StopPvPTimer();
            }

            if (worldConfig.getRealmType() == REALM_PVE)
            {
                if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE))
                {
                    if (!IsPvPFlagged())
                        SetPvPFlag();
                }
                else if (!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) && IsPvPFlagged() && !m_pvpTimer)
                {
                    ResetPvPTimer();
                }
            }

            if (at->flags & AREA_PVP_ARENA)            /* ffa pvp arenas will come later */
            {
                if (!IsPvPFlagged())
                    SetPvPFlag();
                SetFFAPvPFlag();
            }
            else
            {
                RemoveFFAPvPFlag();
            }
        }
    }
}

void Player::BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag)
{
    Object* curObj;
    for (Object::InRangeSet::iterator iter = m_objectsInRange.begin(); iter != m_objectsInRange.end();)
    {
        curObj = *iter;
        ++iter;
        if (curObj->IsPlayer())
        {
            Group* pGroup = static_cast< Player* >(curObj)->GetGroup();
            if (!pGroup && pGroup != GetGroup())
            {
                BuildFieldUpdatePacket(static_cast< Player* >(curObj), index, flag);
            }
        }
    }
}

void Player::LoginPvPSetup()
{
    // Make sure we know our area ID.
    _EventExploration();

    auto at = this->GetArea();

    if (at != nullptr && isAlive() && (at->team == AREAC_CONTESTED || (IsTeamAlliance() && at->team == AREAC_HORDE_TERRITORY) || (IsTeamHorde() && at->team == AREAC_ALLIANCE_TERRITORY)))
        CastSpell(this, PLAYER_HONORLESS_TARGET_SPELL, true);

}

void Player::PvPToggle()
{
    if (worldConfig.getRealmType() == REALM_PVE)
    {
        if (m_pvpTimer > 0)
        {
            // Means that we typed /pvp while we were "cooling down". Stop the timer.
            StopPvPTimer();

            SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

            if (!IsPvPFlagged())
                SetPvPFlag();
        }
        else
        {
            if (IsPvPFlagged())
            {
                auto at = this->GetArea();
                if (at && (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY))
                {
                    if ((at->team == AREAC_ALLIANCE_TERRITORY && IsTeamHorde()) || (at->team == AREAC_HORDE_TERRITORY && IsTeamAlliance()))
                    {
                    }
                    else
                    {
                        // Start the "cooldown" timer.
                        ResetPvPTimer();
                    }
                }
                else
                {
                    // Start the "cooldown" timer.
                    ResetPvPTimer();
                }
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);
            }
            else
            {
                // Move into PvP state.
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                StopPvPTimer();
                SetPvPFlag();
            }
        }
    }
    else if (worldConfig.getRealmType() == REALM_PVP)
    {
        auto at = this->GetArea();
        if (at == nullptr)
            return;

        // This is where all the magic happens :P
        if ((at->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at->team == AREAC_HORDE_TERRITORY && IsTeamHorde()))
        {
            if (m_pvpTimer > 0)
            {
                // Means that we typed /pvp while we were "cooling down". Stop the timer.
                StopPvPTimer();

                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                if (!IsPvPFlagged())
                    SetPvPFlag();
            }
            else
            {
                if (IsPvPFlagged())
                {
                    // Start the "cooldown" timer.
                    ResetPvPTimer();

                    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);
                }
                else
                {
                    // Move into PvP state.
                    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                    StopPvPTimer();
                    SetPvPFlag();
                }
            }
        }
        else
        {
            if (at->zone)
            {
                auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
                if (at2 && ((at2->team == AREAC_ALLIANCE_TERRITORY && IsTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && IsTeamHorde())))
                {
                    if (m_pvpTimer > 0)
                    {
                        // Means that we typed /pvp while we were "cooling down". Stop the timer.
                        StopPvPTimer();

                        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                        if (!IsPvPFlagged())
                            SetPvPFlag();
                    }
                    else
                    {
                        if (IsPvPFlagged())
                        {
                            // Start the "cooldown" timer.
                            ResetPvPTimer();

                            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                            SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                        }
                        else
                        {
                            // Move into PvP state.
                            SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

                            StopPvPTimer();
                            SetPvPFlag();
                        }
                    }
                    return;
                }
            }

            if (!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE))
            {
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);
            }
            else
            {
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);
            }
        }
    }
}

/*! Increments the player's honor
 *  \param honorPoints Number of honor points to add to the player
 *  \param sendUpdate True if UpdateHonor should be called after applying change
 *  \todo Remove map check (func does not work on map 559, 562, 572) */
void Player::AddHonor(uint32 honorPoints, bool sendUpdate)
{
    this->HandleProc(PROC_ON_GAIN_EXPIERIENCE, this, nullptr);
    this->m_procCounter = 0;

    if (this->GetMapId() == 559 || this->GetMapId() == 562 || this->GetMapId() == 572)
        return;

    this->m_honorPoints += honorPoints;
    this->m_honorToday += honorPoints;
    if (this->m_honorPoints > worldConfig.limit.maxHonorPoints)
        this->m_honorPoints = worldConfig.limit.maxHonorPoints;

    if (sendUpdate)
        this->UpdateHonor();
}

/*! Updates the honor related fields and sends updated values to the player
 *  \todo Validate whether this function is unsafe to call while not in world */
void Player::UpdateHonor()
{
#if VERSION_STRING != Classic
    this->setUInt32Value(PLAYER_FIELD_KILLS, uint16(this->m_killsToday) | (this->m_killsYesterday << 16));
#if VERSION_STRING != Cata
    this->setUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, this->m_honorToday);
    this->setUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, this->m_honorYesterday);
#endif
#endif
    this->setUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, this->m_killsLifetime);
    this->SetHonorCurrency(this->m_honorPoints);

    this->UpdateKnownCurrencies(43308, true); //Honor Points
}

/*! Increments the player's arena points
*  \param arenaPoints Number of arena points to add to the player
*  \param sendUpdate True if UpdateArenaPoints should be called after applying change */
void Player::AddArenaPoints(uint32 arenaPoints, bool sendUpdate)
{
    this->m_arenaPoints += arenaPoints;
    if (this->m_arenaPoints > worldConfig.limit.maxArenaPoints)
        this->m_arenaPoints = worldConfig.limit.maxArenaPoints;

    if (sendUpdate)
        this->UpdateArenaPoints();
}

/*! Updates the arena point related fields and sends updated values to the player
*  \todo Validate whether this function is unsafe to call while not in world */
void Player::UpdateArenaPoints()
{
    this->SetArenaCurrency(this->m_arenaPoints);

    this->UpdateKnownCurrencies(43307, true);
}

void Player::ResetPvPTimer()
{
    m_pvpTimer = worldConfig.getIntRate(INTRATE_PVPTIMER);
}

void Player::CalculateBaseStats()
{
    if (!lvlinfo) return;

    memcpy(BaseStats, lvlinfo->Stat, sizeof(uint32) * 5);

    LevelInfo* levelone = objmgr.GetLevelInfo(this->getRace(), this->getClass(), 1);
    if (levelone == nullptr)
    {
        LOG_ERROR("%s (%d): NULL pointer", __FUNCTION__, __LINE__);
        return;
    }
    SetMaxHealth(lvlinfo->HP);
    SetBaseHealth(lvlinfo->HP - (lvlinfo->Stat[2] - levelone->Stat[2]) * 10);

    if (GetPowerType() == POWER_TYPE_MANA)
    {
        SetBaseMana(lvlinfo->Mana - (lvlinfo->Stat[3] - levelone->Stat[3]) * 15);
        SetMaxPower(POWER_TYPE_MANA, lvlinfo->Mana);
    }
}

void Player::CompleteLoading()
{
    // cast passive initial spells      -- grep note: these shouldn't require plyr to be in world
    SpellSet::iterator itr;
    SpellInfo* spellInfo;
    SpellCastTargets targets;
    targets.m_unitTarget = this->GetGUID();
    targets.m_targetMask = TARGET_FLAG_UNIT;

    // warrior has to have battle stance
    if (getClass() == WARRIOR)
    {
        // battle stance passive
        CastSpell(this, sSpellCustomizations.GetSpellInfo(2457), true);
    }

    for (itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        spellInfo = sSpellCustomizations.GetSpellInfo(*itr);

        if (spellInfo != nullptr
            && (spellInfo->IsPassive())  // passive
            && !(spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET))
        {
            if (spellInfo->getRequiredShapeShift())
            {
                if (!(((uint32)1 << (GetShapeShift() - 1)) & spellInfo->getRequiredShapeShift()))
                    continue;
            }

            Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, nullptr);
            spell->prepare(&targets);
        }
    }

    std::list<LoginAura>::iterator i = loginauras.begin();

    for (; i != loginauras.end(); ++i)
    {
        SpellInfo* sp = sSpellCustomizations.GetSpellInfo((*i).id);

        if (sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
            continue; //do not load auras that only exist while pet exist. We should recast these when pet is created anyway

        Aura* aura = sSpellFactoryMgr.NewAura(sp, (*i).dur, this, this, false);
        //if (!(*i).positive) // do we need this? - vojta
        //    aura->SetNegative();

        for (uint8 x = 0; x < 3; x++)
        {
            if (sp->getEffect(x) == SPELL_EFFECT_APPLY_AURA)
            {
                aura->AddMod(sp->getEffectApplyAuraName(x), sp->getEffectBasePoints(x) + 1, sp->getEffectMiscValue(x), x);
            }
        }

        if (sp->getProcCharges() > 0 && (*i).charges > 0)
        {
            for (uint32 x = 0; x < (*i).charges - 1; x++)
            {
                Aura* a = sSpellFactoryMgr.NewAura(sp, (*i).dur, this, this, false);
                this->AddAura(a);
            }
            if (m_chargeSpells.find(sp->getId()) == m_chargeSpells.end() && !(sp->getProcFlags() & PROC_REMOVEONUSE))
            {
                SpellCharge charge;
                if (sp->custom_proc_interval == 0)
                    charge.count = (*i).charges;
                else
                    charge.count = sp->getProcCharges();

                charge.spellId = sp->getId();
                charge.ProcFlag = sp->getProcFlags();
                charge.lastproc = 0;
                charge.procdiff = 0;
                m_chargeSpells.insert(std::make_pair(sp->getId(), charge));
            }
        }
        this->AddAura(aura);
    }

    // this needs to be after the cast of passive spells, because it will cast ghost form, after the remove making it in ghost alive, if no corpse.
    //death system checkout
    if (GetHealth() <= 0 && !HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        setDeathState(CORPSE);
    }
    else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        // Check if we have an existing corpse.
        Corpse* corpse = objmgr.GetCorpseByOwner(GetLowGUID());
        if (corpse == nullptr)
        {
            sEventMgr.AddEvent(this, &Player::RepopAtGraveyard, GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), EVENT_PLAYER_CHECKFORCHEATS, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        else
        {
            // Set proper deathstate
            setDeathState(CORPSE);
        }
    }

    if (IsDead())
    {
        if (myCorpseInstanceId != 0)
        {
            // cebernic: tempfix. This send a counter for player with just logging in.
            ///\todo counter will be follow with death time.
            Corpse* myCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
            if (myCorpse != nullptr)
                myCorpse->ResetDeathClock();
            WorldPacket SendCounter(SMSG_CORPSE_RECLAIM_DELAY, 4);
            SendCounter << (uint32)(CORPSE_RECLAIM_TIME_MS);
            GetSession()->SendPacket(&SendCounter);
        }
        //RepopRequestedPlayer();
        //sEventMgr.AddEvent(this, &Player::RepopRequestedPlayer, EVENT_PLAYER_CHECKFORCHEATS, 2000, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    if (!IsMounted())
        SpawnActivePet();

#if VERSION_STRING != TBC
    // useless logon spell
    Spell* logonspell = sSpellFactoryMgr.NewSpell(this, sSpellCustomizations.GetSpellInfo(836), false, nullptr);
    logonspell->prepare(&targets);
#endif

    if (IsBanned())
    {
        Kick(10000);
        BroadcastMessage(GetSession()->LocalizedWorldSrv(ServerString::SS_NOT_ALLOWED_TO_PLAY));
        BroadcastMessage(GetSession()->LocalizedWorldSrv(ServerString::SS_BANNED_FOR_TIME), GetBanReason().c_str());
    }

    if (m_playerInfo->m_Group)
    {
        sEventMgr.AddEvent(this, &Player::EventGroupFullUpdate, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    if (raidgrouponlysent)
    {
        WorldPacket data2(SMSG_RAID_GROUP_ONLY, 8);
        data2 << uint32(0xFFFFFFFF) << uint32(0);
        GetSession()->SendPacket(&data2);
        raidgrouponlysent = false;
    }

    sInstanceMgr.BuildSavedInstancesForPlayer(this);
    CombatStatus.UpdateFlag();

#if VERSION_STRING > TBC
    // add glyphs
    for (uint8 j = 0; j < GLYPHS_COUNT; ++j)
    {
        auto glyph_properties = sGlyphPropertiesStore.LookupEntry(m_specs[m_talentActiveSpec].glyphs[j]);
        if (glyph_properties == nullptr)
            continue;

        CastSpell(this, glyph_properties->SpellID, true);
    }

    //sEventMgr.AddEvent(this,&Player::SendAllAchievementData,EVENT_SEND_ACHIEVEMNTS_TO_PLAYER,ACHIEVEMENT_SEND_DELAY,1,0);
    sEventMgr.AddEvent(static_cast< Unit* >(this), &Unit::UpdatePowerAmm, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, LOGIN_CIENT_SEND_DELAY, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
#endif
}

void Player::OnWorldPortAck()
{
    //only resurrect if player is porting to a instance portal
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());
    if (IsDead())
    {
        if (pMapinfo)
        {
            if (pMapinfo->type != INSTANCE_NULL)
            {
                ResurrectPlayer();
            }
        }
    }

    if (pMapinfo)
    {
        WorldPacket data(4);
        if (pMapinfo->hasFlag(WMI_INSTANCE_WELCOME) && GetMapMgr())
        {
            std::string welcome_msg;
            welcome_msg = std::string(GetSession()->LocalizedWorldSrv(ServerString::SS_INSTANCE_WELCOME)) + " ";
            welcome_msg += std::string(GetSession()->LocalizedMapName(pMapinfo->mapid));
            welcome_msg += ". ";
            if (pMapinfo->type != INSTANCE_NONRAID && !(pMapinfo->type == INSTANCE_MULTIMODE && iInstanceType >= MODE_HEROIC) && m_mapMgr->pInstance)
            {
                /*welcome_msg += "This instance is scheduled to reset on ";
                welcome_msg += asctime(localtime(&m_mapMgr->pInstance->m_expiration));*/
                welcome_msg += std::string(GetSession()->LocalizedWorldSrv(ServerString::SS_INSTANCE_RESET_INF)) + " ";
                welcome_msg += Util::GetDateTimeStringFromTimeStamp((uint32)m_mapMgr->pInstance->m_expiration);
            }
            sChatHandler.SystemMessage(m_session, welcome_msg.c_str());
        }
    }

    SpeedCheatReset();
}

void Player::ModifyBonuses(uint32 type, int32 val, bool apply)
{
    // Added some updateXXXX calls so when an item modifies a stat they get updated
    // also since this is used by auras now it will handle it for those
    int32 _val = val;
    if (!apply)
        val = -val;

    switch (type)
    {
        case POWER:
        {
            ModMaxPower(POWER_TYPE_MANA, val);
            m_manafromitems += val;
        }
        break;
        case HEALTH:
        {
            ModMaxHealth(val);
            m_healthfromitems += val;
        }
        break;
        case AGILITY:    //modify agility
        case STRENGTH:    //modify strength
        case INTELLECT:    //modify intellect
        case SPIRIT:    //modify spirit
        case STAMINA:    //modify stamina
        {
            uint8 convert[] = { 1, 0, 3, 4, 2 };
            if (_val > 0)
                FlatStatModPos[convert[type - 3]] += val;
            else
                FlatStatModNeg[convert[type - 3]] -= val;
            CalcStat(convert[type - 3]);
        }
        break;
        case WEAPON_SKILL_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_SKILL, val);
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_MAIN_HAND_SKILL, val);   // melee main hand
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_OFF_HAND_SKILL, val);   // melee off hand
        }
        break;
        case DEFENSE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_DEFENCE, val);
        }
        break;
        case DODGE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_DODGE, val);
        }
        break;
        case PARRY_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_PARRY, val);
        }
        break;
        case SHIELD_BLOCK_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK, val);
        }
        break;
        case MELEE_HIT_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT, val);
        }
        break;
        case RANGED_HIT_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT, val);
        }
        break;
        case SPELL_HIT_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT, val);
        }
        break;
        case MELEE_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT, val);
        }
        break;
        case RANGED_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT, val);
        }
        break;
        case SPELL_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT, val);
        }
        break;
        case MELEE_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT_AVOIDANCE, val);
        }
        break;
        case RANGED_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT_AVOIDANCE, val);
        }
        break;
        case SPELL_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT_AVOIDANCE, val);
        }
        break;
        case MELEE_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case RANGED_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case SPELL_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case MELEE_HASTE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HASTE, val);  //melee
        }
        break;
        case RANGED_HASTE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HASTE, val);  //ranged
        }
        break;
        case SPELL_HASTE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HASTE, val);  //spell
        }
        break;
        case HIT_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT, val);  //melee
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT, val);  //ranged
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT, val);   //Spell
        }
        break;
        case CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT, val);  //melee
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT, val);  //ranged
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT, val);   //spell
        }
        break;
        case HIT_AVOIDANCE_RATING:// this is guessed based on layout of other fields
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT_AVOIDANCE, val);  //melee
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT_AVOIDANCE, val);  //ranged
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT_AVOIDANCE, val);  //spell
        }
        break;
        case CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case EXPERTISE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_EXPERTISE, val);
        }
        break;
        case RESILIENCE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT_RESILIENCE, val);  //melee
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT_RESILIENCE, val);  //ranged
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT_RESILIENCE, val);  //spell
        }
        break;
        case HASTE_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HASTE, val);  //melee
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HASTE, val);  //ranged
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HASTE, val);   // Spell
        }
        break;
        case ATTACK_POWER:
        {
            ModAttackPowerMods(val);
            ModRangedAttackPowerMods(val);
        }
        break;
        case RANGED_ATTACK_POWER:
        {
            ModRangedAttackPowerMods(val);
        }
        break;
        case FERAL_ATTACK_POWER:
        {
            ModAttackPowerMods(val);
        }
        break;
        case SPELL_HEALING_DONE:
        {
            for (uint8 school = 1; school < SCHOOL_COUNT; ++school)
            {
                HealDoneMod[school] += val;
            }
            ModHealingDoneMod(val);
        }
        break;
        case SPELL_DAMAGE_DONE:
        {
            for (uint8 school = 1; school < SCHOOL_COUNT; ++school)
            {
                ModPosDamageDoneMod(school, val);
            }
        }
        break;
        case MANA_REGENERATION:
        {
            m_ModInterrMRegen += val;
        }
        break;
        case ARMOR_PENETRATION_RATING:
        {
            ModUnsigned32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_ARMOR_PENETRATION_RATING, val);
        }
        break;
        case SPELL_POWER:
        {
            for (uint8 school = 1; school < 7; ++school)
            {
                ModPosDamageDoneMod(school, val);
                HealDoneMod[school] += val;
            }
            ModHealingDoneMod(val);
        }
        break;
    }
}

bool Player::CanSignCharter(Charter* charter, Player* requester)
{
    if (charter == nullptr || requester == nullptr)
        return false;
    if (charter->CharterType >= CHARTER_TYPE_ARENA_2V2 && m_arenaTeams[charter->CharterType - 1] != nullptr)
        return false;

#if VERSION_STRING != Cata
    if (charter->CharterType == CHARTER_TYPE_GUILD && IsInGuild())
        return false;
#else
    if (charter->CharterType == CHARTER_TYPE_GUILD && requester->GetGuild())
        return false;
#endif

    if (m_charters[charter->CharterType] || requester->GetTeam() != GetTeam() || this == requester)
        return false;
    else
        return true;
}

void Player::SaveAuras(std::stringstream & ss)
{
    uint32 charges = 0, prevX = 0;
    //cebernic: save all auras why only just positive?
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; x++)
    {
        if (m_auras[x] != nullptr && m_auras[x]->GetTimeLeft() > 3000)
        {
            Aura* aur = m_auras[x];
            for (uint8 i = 0; i < 3; ++i)
            {
                if (aur->m_spellInfo->getEffect(i) == SPELL_EFFECT_APPLY_GROUP_AREA_AURA || aur->m_spellInfo->getEffect(i) == SPELL_EFFECT_APPLY_RAID_AREA_AURA || aur->m_spellInfo->getEffect(i) == SPELL_EFFECT_ADD_FARSIGHT)
                {
                    continue;
                    break;
                }
            }

            if (aur->pSpellId)
                continue; //these auras were gained due to some proc. We do not save these either to avoid exploits of not removing them

            if (aur->m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
                continue;

            //we are going to cast passive spells anyway on login so no need to save auras for them
            if (aur->IsPassive() && !(aur->GetSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
                continue;

            if (charges > 0 && aur->GetSpellId() != m_auras[prevX]->GetSpellId())
            {
                ss << m_auras[prevX]->GetSpellId() << "," << m_auras[prevX]->GetTimeLeft() << "," << m_auras[prevX]->IsPositive() << "," << charges << ",";
                charges = 0;
            }

            if (aur->GetSpellInfo()->getProcCharges() == 0)
                ss << aur->GetSpellId() << "," << aur->GetTimeLeft() << "," << aur->IsPositive() << "," << 0 << ",";
            else
                charges++;

            prevX = x;
        }
    }
    if (charges > 0 && m_auras[prevX] != nullptr)
    {
        ss << m_auras[prevX]->GetSpellId() << "," << m_auras[prevX]->GetTimeLeft() << "," << m_auras[prevX]->IsPositive() << "," << charges << ",";
    }
}

void Player::SetShapeShift(uint8 ss)
{
    uint8 old_ss = GetShapeShift(); //GetByte(UNIT_FIELD_BYTES_2, 3);
    setByteValue(UNIT_FIELD_BYTES_2, 3, ss);

    //remove auras that we should not have
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] != nullptr)
        {
            uint32 reqss = m_auras[x]->GetSpellInfo()->getRequiredShapeShift();
            if (reqss != 0 && m_auras[x]->IsPositive())
            {
                if (old_ss > 0
                    && old_ss != FORM_SHADOW
                    && old_ss != FORM_STEALTH)
                {
                    if ((((uint32)1 << (old_ss - 1)) & reqss) &&        // we were in the form that required it
                        !(((uint32)1 << (ss - 1) & reqss)))            // new form doesn't have the right form
                    {
                        m_auras[x]->Remove();
                        continue;
                    }
                }
            }

            if (this->getClass() == DRUID)
            {
                switch (m_auras[x]->GetSpellInfo()->getMechanicsType())
                {
                    case MECHANIC_ROOTED: //Rooted
                    case MECHANIC_ENSNARED: //Movement speed
                    case MECHANIC_POLYMORPHED:  //Polymorphed
                    {
                        m_auras[x]->Remove();
                    }
                    break;
                    default:
                        break;
                }
            }
        }
    }

    // apply any talents/spells we have that apply only in this form.
    std::set<uint32>::iterator itr;
    SpellInfo* sp;
    Spell* spe;
    SpellCastTargets t(GetGUID());

    for (itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        sp = sSpellCustomizations.GetSpellInfo(*itr);
        if (sp == nullptr)
            continue;

        if (sp->custom_apply_on_shapeshift_change || sp->getAttributes() & ATTRIBUTES_PASSIVE)        // passive/talent
        {
            if (sp->getRequiredShapeShift() && ((uint32)1 << (ss - 1)) & sp->getRequiredShapeShift())
            {
                spe = sSpellFactoryMgr.NewSpell(this, sp, true, nullptr);
                spe->prepare(&t);
            }
        }
    }

    //feral attack power
    if (this->getClass() == DRUID)
    {
        // Changed from normal to feral form
        if (!(old_ss == FORM_MOONKIN || old_ss == FORM_CAT || old_ss == FORM_BEAR || old_ss == FORM_DIREBEAR) &&
            (ss == FORM_MOONKIN || ss == FORM_CAT || ss == FORM_BEAR || ss == FORM_DIREBEAR))
            this->ApplyFeralAttackPower(true);
        // Changed from feral to normal form
        else if ((old_ss == FORM_MOONKIN || old_ss == FORM_CAT || old_ss == FORM_BEAR || old_ss == FORM_DIREBEAR) &&
                 !(ss == FORM_MOONKIN || ss == FORM_CAT || ss == FORM_BEAR || ss == FORM_DIREBEAR))
                 this->ApplyFeralAttackPower(false);
    }

    // now dummy-handler stupid hacky fixed shapeshift spells (leader of the pack, etc)
    for (itr = mShapeShiftSpells.begin(); itr != mShapeShiftSpells.end(); ++itr)
    {
        sp = sSpellCustomizations.GetSpellInfo(*itr);
        if (sp->getRequiredShapeShift() && ((uint32)1 << (ss - 1)) & sp->getRequiredShapeShift())
        {
            spe = sSpellFactoryMgr.NewSpell(this, sp, true, nullptr);
            spe->prepare(&t);
        }
    }

    UpdateStats();
    UpdateChances();
}

void Player::CalcDamage()
{
    float delta;
    float r;
    int ss = GetShapeShift();
    /////////////////MAIN HAND
    float ap_bonus = GetAP() / 14000.0f;
    delta = (float)GetPosDamageDoneMod(SCHOOL_NORMAL) - (float)GetNegDamageDoneMod(SCHOOL_NORMAL);

    if (IsInFeralForm())
    {
        float tmp = 1; // multiplicative damage modifier
        for (std::map<uint32, WeaponModifier>::iterator i = damagedone.begin(); i != damagedone.end(); ++i)
        {
            if (i->second.wclass == (uint32)-1)  // applying only "any weapon" modifiers
                tmp += i->second.value;
        }
        uint32 lev = getLevel();
        float feral_damage; // average base damage before bonuses and modifiers
        uint32 x; // itemlevel of the two hand weapon with dps equal to cat or bear dps

        if (ss == FORM_CAT)
        {
            if (lev < 42) x = lev - 1;
            else if (lev < 46) x = lev;
            else if (lev < 49) x = 2 * lev - 45;
            else if (lev < 60) x = lev + 4;
            else x = 64;

            // 3rd grade polinom for calculating blue two-handed weapon dps based on itemlevel (from Hyzenthlei)
            if (x <= 28) feral_damage = 1.563e-03f * x * x * x - 1.219e-01f * x * x + 3.802e+00f * x - 2.227e+01f;
            else if (x <= 41) feral_damage = -3.817e-03f * x * x * x + 4.015e-01f * x * x - 1.289e+01f * x + 1.530e+02f;
            else feral_damage = 1.829e-04f * x * x * x - 2.692e-02f * x * x + 2.086e+00f * x - 1.645e+01f;

            r = feral_damage * 0.79f + delta + ap_bonus * 1000.0f;
            r *= tmp;
            SetMinDamage(r > 0 ? r : 0);

            r = feral_damage * 1.21f + delta + ap_bonus * 1000.0f;
            r *= tmp;
            SetMaxDamage(r > 0 ? r : 0);
        }
        else // Bear or Dire Bear Form
        {
            if (ss == FORM_BEAR) x = lev;
            else x = lev + 5; // DIRE_BEAR dps is slightly better than bear dps
            if (x > 70) x = 70;

            // 3rd grade polinom for calculating green two-handed weapon dps based on itemlevel (from Hyzenthlei)
            if (x <= 30) feral_damage = 7.638e-05f * x * x * x + 1.874e-03f * x * x + 4.967e-01f * x + 1.906e+00f;
            else if (x <= 44) feral_damage = -1.412e-03f * x * x * x + 1.870e-01f * x * x - 7.046e+00f * x + 1.018e+02f;
            else feral_damage = 2.268e-04f * x * x * x - 3.704e-02f * x * x + 2.784e+00f * x - 3.616e+01f;
            feral_damage *= 2.5f; // Bear Form attack speed

            r = feral_damage * 0.79f + delta + ap_bonus * 2500.0f;
            r *= tmp;
            SetMinDamage(r > 0 ? r : 0);

            r = feral_damage * 1.21f + delta + ap_bonus * 2500.0f;
            r *= tmp;
            SetMaxDamage(r > 0 ? r : 0);
        }

        return;
    }

    //////no druid ss
    uint32 speed = 2000;
    Item* it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (!disarmed)
    {
        if (it)
            speed = it->GetItemProperties()->Delay;
    }

    float bonus = ap_bonus * speed;
    float tmp = 1;
    std::map<uint32, WeaponModifier>::iterator i;
    for (i = damagedone.begin(); i != damagedone.end(); ++i)
    {
        if ((i->second.wclass == (uint32)-1) || //any weapon
            (it && ((1 << it->GetItemProperties()->SubClass) & i->second.subclass)))
            tmp += i->second.value;
    }

    r = BaseDamage[0] + delta + bonus;
    r *= tmp;
    SetMinDamage(r > 0 ? r : 0);

    r = BaseDamage[1] + delta + bonus;
    r *= tmp;
    SetMaxDamage(r > 0 ? r : 0);

    uint32 cr = 0;
    if (it)
    {
        if (static_cast< Player* >(this)->m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->GetItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_MAIN_HAND_SKILL, cr);
    /////////////// MAIN HAND END

    /////////////// OFF HAND START
    cr = 0;
    it = static_cast< Player* >(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it)
    {
        if (!disarmed)
        {
            speed = it->GetItemProperties()->Delay;
        }
        else speed = 2000;

        bonus = ap_bonus * speed;
        i = damagedone.begin();
        tmp = 1;
        for (; i != damagedone.end(); ++i)
        {
            if ((i->second.wclass == (uint32)-1) || //any weapon
                (((1 << it->GetItemProperties()->SubClass) & i->second.subclass))
                )
                tmp += i->second.value;
        }

        r = (BaseOffhandDamage[0] + delta + bonus) * offhand_dmg_mod;
        r *= tmp;
        SetMinOffhandDamage(r > 0 ? r : 0);
        r = (BaseOffhandDamage[1] + delta + bonus) * offhand_dmg_mod;
        r *= tmp;
        SetMaxOffhandDamage(r > 0 ? r : 0);
        if (m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->GetItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_OFF_HAND_SKILL, cr);

    /////////////second hand end
    ///////////////////////////RANGED
    cr = 0;
    if ((it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)) != nullptr)
    {
        i = damagedone.begin();
        tmp = 1;
        for (; i != damagedone.end(); ++i)
        {
            if ((i->second.wclass == (uint32)-1) || //any weapon
                (((1 << it->GetItemProperties()->SubClass) & i->second.subclass)))
            {
                tmp += i->second.value;
            }
        }

        if (it->GetItemProperties()->SubClass != 19)//wands do not have bonuses from RAP & ammo
        {
            //                ap_bonus = (GetRangedAttackPower()+(int32)GetRangedAttackPowerMods())/14000.0;
            //modified by Zack : please try to use premade functions if possible to avoid forgetting stuff
            ap_bonus = GetRAP() / 14000.0f;
            bonus = ap_bonus * it->GetItemProperties()->Delay;

            if (GetAmmoId() && !m_requiresNoAmmo)
            {
                ItemProperties const* xproto = sMySQLStore.getItemProperties(GetAmmoId());
                if (xproto)
                {
                    bonus += ((xproto->Damage[0].Min + xproto->Damage[0].Max) * it->GetItemProperties()->Delay) / 2000.0f;
                }
            }
        }
        else bonus = 0;

        r = BaseRangedDamage[0] + delta + bonus;
        r *= tmp;
        SetMinRangedDamage(r > 0 ? r : 0);

        r = BaseRangedDamage[1] + delta + bonus;
        r *= tmp;
        SetMaxRangedDamage(r > 0 ? r : 0);


        if (m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->GetItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }

    }
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_SKILL, cr);

    /////////////////////////////////RANGED end
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->CalcDamage();//Re-calculate pet's too
    }
}

uint32 Player::GetMainMeleeDamage(uint32 AP_owerride)
{
    float r;
    int ss = GetShapeShift();
    /////////////////MAIN HAND
    float ap_bonus;
    if (AP_owerride)
        ap_bonus = AP_owerride / 14000.0f;
    else
        ap_bonus = GetAP() / 14000.0f;
    if (IsInFeralForm())
    {
        if (ss == FORM_CAT)
            r = ap_bonus * 1000.0f;
        else
            r = ap_bonus * 2500.0f;
        return float2int32(r);
    }
    //////no druid ss
    uint32 speed = 2000;
    Item* it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (!disarmed)
    {
        if (it)
            speed = it->GetItemProperties()->Delay;
    }
    r = ap_bonus * speed;
    return float2int32(r);
}

void Player::EventPortToGM(Player* p)
{
    SafeTeleport(p->GetMapId(), p->GetInstanceID(), p->GetPosition());
}

/*! \returns TEAM_ALLIANCE if m_team equals 0, and TEAM_HORDE otherwise
 *  \note Use this function instead of Player::GetTeam for any new code
 *  \todo Refactor m_team to be a PlayerTeam instead of a uint32 and return m_team directly
 *  \sa Player::GetTeam */
PlayerTeam Player::GetTeamReal()
{
    return m_team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE;
}

void Player::AddComboPoints(uint64 target, int8 count)
{
    // GetTimeLeft() checked in SpellAura, so we won't lose points
    RemoveAllAuraType(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (m_comboTarget == target)
        m_comboPoints += count;
    else
    {
        m_comboTarget = target;
        m_comboPoints = count;
    }
    UpdateComboPoints();
}

void Player::UpdateComboPoints()
{
    // fuck bytebuffers :D
    unsigned char buffer[10];
    uint16 c = 2;

    // check for overflow
    if (m_comboPoints > 5)
        m_comboPoints = 5;

    if (m_comboPoints < 0)
        m_comboPoints = 0;

    if (m_comboTarget != 0)
    {
        Unit* target = (m_mapMgr != nullptr) ? m_mapMgr->GetUnit(m_comboTarget) : NULL;
        if (!target || target->IsDead() || GetSelection() != m_comboTarget)
        {
            buffer[0] = buffer[1] = 0;
        }
        else
        {
            c = static_cast<uint16>(FastGUIDPack(m_comboTarget, buffer, 0));
            buffer[c++] = m_comboPoints;
        }
    }
    else
        buffer[0] = buffer[1] = 0;

    m_session->OutPacket(SMSG_UPDATE_COMBO_POINTS, c, buffer);
}

void Player::SendAreaTriggerMessage(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
    char msg[500];
    vsnprintf(msg, 500, message, ap);
    va_end(ap);

    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 6 + strlen(msg));
    data << uint32(0);
    data << msg;
    data << uint8(0x00);
    m_session->SendPacket(&data);
}

void Player::removeSoulStone()
{
    if (!this->SoulStone) return;
    uint32 sSoulStone = 0;
    switch (this->SoulStone)
    {
        case 3026:
        {
            sSoulStone = 20707;
        }
        break;
        case 20758:
        {
            sSoulStone = 20762;
        }
        break;
        case 20759:
        {
            sSoulStone = 20763;
        }
        break;
        case 20760:
        {
            sSoulStone = 20764;
        }
        break;
        case 20761:
        {
            sSoulStone = 20765;
        }
        break;
        case 27240:
        {
            sSoulStone = 27239;
        }
        break;
        case 47882:
        {
            sSoulStone = 47883;
        }
        break;
    }
    this->RemoveAura(sSoulStone);
    this->SoulStone = this->SoulStoneReceiver = 0; //just incase
}

void Player::SoftDisconnect()
{
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_SOFT_DISCONNECT);
    WorldSession* session = GetSession();
    session->LogoutPlayer(true);
    session->Disconnect();
}

void Player::SetNoseLevel()
{
    // Set the height of the player
    switch (getRace())
    {
        case RACE_HUMAN:
            // female
            if (getGender()) m_noseLevel = 1.72f;
            // male
            else m_noseLevel = 1.78f;
            break;
        case RACE_ORC:
            if (getGender()) m_noseLevel = 1.82f;
            else m_noseLevel = 1.98f;
            break;
        case RACE_DWARF:
            if (getGender()) m_noseLevel = 1.27f;
            else m_noseLevel = 1.4f;
            break;
        case RACE_NIGHTELF:
            if (getGender()) m_noseLevel = 1.84f;
            else m_noseLevel = 2.13f;
            break;
        case RACE_UNDEAD:
            if (getGender()) m_noseLevel = 1.61f;
            else m_noseLevel = 1.8f;
            break;
        case RACE_TAUREN:
            if (getGender()) m_noseLevel = 2.48f;
            else m_noseLevel = 2.01f;
            break;
        case RACE_GNOME:
            if (getGender()) m_noseLevel = 1.06f;
            else m_noseLevel = 1.04f;
            break;
#if VERSION_STRING == Cata
        case RACE_GOBLIN:
            if (getGender()) m_noseLevel = 1.06f;
            else m_noseLevel = 1.04f;
            break;
#endif
        case RACE_TROLL:
            if (getGender()) m_noseLevel = 2.02f;
            else m_noseLevel = 1.93f;
            break;
        case RACE_BLOODELF:
            if (getGender()) m_noseLevel = 1.83f;
            else m_noseLevel = 1.93f;
            break;
        case RACE_DRAENEI:
            if (getGender()) m_noseLevel = 2.09f;
            else m_noseLevel = 2.36f;
            break;
#if VERSION_STRING == Cata
        case RACE_WORGEN:
            if (getGender()) m_noseLevel = 1.72f;
            else m_noseLevel = 1.78f;
            break;
#endif
    }
}

void Player::SummonRequest(uint32 Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position)
{
    m_summonInstanceId = InstanceID;
    m_summonPos = Position;
    m_summoner = Requestor;
    m_summonMapId = MapID;

    WorldPacket data(SMSG_SUMMON_REQUEST, 16);
    data << uint64(Requestor);
    data << ZoneID;
    data << uint32(120000);        // 2 minutes
    m_session->SendPacket(&data);
}

void Player::RemoveFromBattlegroundQueue()
{
    if (!m_pendingBattleground)
        return;

    m_pendingBattleground->RemovePendingPlayer(this);
    sChatHandler.SystemMessage(m_session, GetSession()->LocalizedWorldSrv(ServerString::SS_BG_REMOVE_QUEUE_INF));
}

void Player::_AddSkillLine(uint32 SkillLine, uint32 Curr_sk, uint32 Max_sk)
{
    auto skill_line = sSkillLineStore.LookupEntry(SkillLine);
    if (!skill_line)
        return;

    // force to be within limits
    Curr_sk = (Curr_sk > DBC_PLAYER_SKILL_MAX ? DBC_PLAYER_SKILL_MAX : (Curr_sk < 1 ? 1 : Curr_sk));
    Max_sk = (Max_sk > DBC_PLAYER_SKILL_MAX ? DBC_PLAYER_SKILL_MAX : Max_sk);

    SkillMap::iterator itr = m_skills.find(SkillLine);
    if (itr != m_skills.end())
    {
        if ((Curr_sk > itr->second.CurrentValue && Max_sk >= itr->second.MaximumValue) || (Curr_sk == itr->second.CurrentValue && Max_sk > itr->second.MaximumValue))
        {
            itr->second.CurrentValue = Curr_sk;
            itr->second.MaximumValue = Max_sk;
            _UpdateMaxSkillCounts();
        }
    }
    else
    {
        PlayerSkill inf;
        inf.Skill = skill_line;
        inf.MaximumValue = Max_sk;
        inf.CurrentValue = (inf.Skill->id != SKILL_RIDING ? Curr_sk : Max_sk);
        inf.BonusValue = 0;
        m_skills.insert(std::make_pair(SkillLine, inf));
        _UpdateSkillFields();
    }

    ItemProf* prof1;
    //Add to proficiency
    if ((prof1 = (ItemProf*)GetProficiencyBySkill(SkillLine)) != nullptr)
    {
        if (prof1->itemclass == 4)
        {
            armor_proficiency |= prof1->subclass;
            SendSetProficiency(prof1->itemclass, armor_proficiency);
        }
        else
        {
            weapon_proficiency |= prof1->subclass;
            SendSetProficiency(prof1->itemclass, weapon_proficiency);
        }
    }
    _LearnSkillSpells(SkillLine, Curr_sk);

    // Displaying bug fix
    _UpdateSkillFields();
#if VERSION_STRING > TBC
    m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, SkillLine, Max_sk / 75, 0);
    m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, SkillLine, Curr_sk, 0);
#endif
}

void Player::_UpdateSkillFields()
{
#if VERSION_STRING != Cata
    uint16 f = PLAYER_SKILL_INFO_1_1;
#else
    uint16 f = PLAYER_SKILL_LINEID_0;
#endif
    /* Set the valid skills */
    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end();)
    {
        if (!itr->first)
        {
            SkillMap::iterator it2 = itr++;
            m_skills.erase(it2);
            continue;
        }

#if VERSION_STRING != Cata
        ARCEMU_ASSERT(f <= PLAYER_CHARACTER_POINTS1);
#else
        ARCEMU_ASSERT(f <= PLAYER_CHARACTER_POINTS);
#endif
        if (itr->second.Skill->type == SKILL_TYPE_PROFESSION)
        {
            setUInt32Value(f++, itr->first | 0x10000);
#if VERSION_STRING > TBC
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, itr->second.Skill->id, itr->second.CurrentValue, 0);
#endif
        }
#if VERSION_STRING == Cata
        else if (itr->second.Skill->type == SKILL_TYPE_SECONDARY)
        {
            setUInt32Value(f++, itr->first | 0x40000);
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, itr->second.Skill->id, itr->second.CurrentValue, 0);
        }
#endif
        else
        {
            setUInt32Value(f++, itr->first);
#if VERSION_STRING > TBC
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, itr->second.Skill->id, itr->second.MaximumValue / 75, 0);
#endif
        }

        setUInt32Value(f++, (itr->second.MaximumValue << 16) | itr->second.CurrentValue);
        setUInt32Value(f++, itr->second.BonusValue);
        ++itr;
    }

    /* Null out the rest of the fields */
#if VERSION_STRING != Cata
    for (; f < PLAYER_CHARACTER_POINTS1; ++f)
#else
    for (; f < PLAYER_CHARACTER_POINTS; ++f)
#endif
    {
        if (m_uint32Values[f] != 0)
            setUInt32Value(f, 0);
    }
}

bool Player::_HasSkillLine(uint32 SkillLine)
{
    return (m_skills.find(SkillLine) != m_skills.end());
}

void Player::_AdvanceSkillLine(uint32 SkillLine, uint32 Count /* = 1 */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    uint32 curr_sk = Count;
    if (itr == m_skills.end())
    {
        /* Add it */
        _AddSkillLine(SkillLine, Count, getLevel() * 5);
        _UpdateMaxSkillCounts();
        sHookInterface.OnAdvanceSkillLine(this, SkillLine, Count);
#if VERSION_STRING > TBC
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, SkillLine, _GetSkillLineMax(SkillLine), 0);
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, SkillLine, Count, 0);
#endif
    }
    else
    {
        curr_sk = itr->second.CurrentValue;
        itr->second.CurrentValue = std::min(curr_sk + Count, itr->second.MaximumValue);
        if (itr->second.CurrentValue != curr_sk)
        {
            curr_sk = itr->second.CurrentValue;
            _UpdateSkillFields();
            sHookInterface.OnAdvanceSkillLine(this, SkillLine, curr_sk);
        }
#if VERSION_STRING > TBC
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, SkillLine, itr->second.MaximumValue / 75, 0);
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, SkillLine, itr->second.CurrentValue, 0);
#endif
    }
    _LearnSkillSpells(SkillLine, curr_sk);
}

void Player::_LearnSkillSpells(uint32 SkillLine, uint32 curr_sk)
{
    SpellInfo* sp;
    uint32 removeSpellId = 0;
    for (uint32 idx = 0; idx < sSkillLineAbilityStore.GetNumRows(); ++idx)
    {
        auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(idx);
        if (skill_line_ability == nullptr)
            continue;

        // add new "automatic-acquired" spell
        if ((skill_line_ability->skilline == SkillLine) && (skill_line_ability->acquireMethod == 1))
        {
            sp = sSpellCustomizations.GetSpellInfo(skill_line_ability->spell);
            if (sp && (curr_sk >= skill_line_ability->minSkillLineRank))
            {
                // Player is able to learn this spell; check if they already have it, or a higher rank (shouldn't, but just in case)
                bool addThisSpell = true;
                SpellInfo* se;
                for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
                {
                    se = sSpellCustomizations.GetSpellInfo(*itr);
                    if ((se->custom_NameHash == sp->custom_NameHash) && (se->custom_RankNumber >= sp->custom_RankNumber))
                    {
                        // Stupid profession related spells for "skinning" having the same namehash and not ranked
                        if (sp->getId() != 32605 && sp->getId() != 32606 && sp->getId() != 49383)
                        {
                            // Player already has this spell, or a higher rank. Don't add it.
                            addThisSpell = false;
                        }
                    }
                }
                if (addThisSpell)
                {
                    // Adding a spell, now check if there was a previous spell, to remove
                    for (uint32 idx2 = 0; idx2 < sSkillLineAbilityStore.GetNumRows(); ++idx2)
                    {
                        auto second_skill_line_ability = sSkillLineAbilityStore.LookupEntry(idx2);
                        if (second_skill_line_ability == nullptr)
                            continue;

                        if ((second_skill_line_ability->skilline == SkillLine) && (second_skill_line_ability->next == skill_line_ability->spell))
                        {
                            removeSpellId = second_skill_line_ability->spell;
                        }
                    }
                    addSpell(skill_line_ability->spell);
                    if (removeSpellId)
                    {
                        removeSpell(removeSpellId, true, true, skill_line_ability->next);
                    }
                    // if passive spell, apply it now
                    if (sp->IsPassive())
                    {
                        SpellCastTargets targets;
                        targets.m_unitTarget = this->GetGUID();
                        targets.m_targetMask = TARGET_FLAG_UNIT;
                        Spell* spell = sSpellFactoryMgr.NewSpell(this, sp, true, nullptr);
                        spell->prepare(&targets);
                    }
                }
            }
        }
    }
}

uint32 Player::_GetSkillLineMax(uint32 SkillLine)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    return (itr == m_skills.end()) ? 0 : itr->second.MaximumValue;
}

uint32 Player::_GetSkillLineCurrent(uint32 SkillLine, bool IncludeBonus /* = true */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if (itr == m_skills.end())
        return 0;

    return (IncludeBonus ? itr->second.CurrentValue + itr->second.BonusValue : itr->second.CurrentValue);
}

void Player::_RemoveSkillLine(uint32 SkillLine)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if (itr == m_skills.end())
        return;

    m_skills.erase(itr);
    _UpdateSkillFields();
}

void Player::_UpdateMaxSkillCounts()
{
    bool dirty = false;
    uint32 new_max;
    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
        if (itr->second.Skill->type == SKILL_TYPE_WEAPON || itr->second.Skill->id == SKILL_LOCKPICKING)
        {
            new_max = 5 * getLevel();
        }
        else if (itr->second.Skill->type == SKILL_TYPE_LANGUAGE)
        {
            new_max = 300;
        }
        else if (itr->second.Skill->type == SKILL_TYPE_PROFESSION || itr->second.Skill->type == SKILL_TYPE_SECONDARY)
        {
            new_max = itr->second.MaximumValue;
            if (new_max >= DBC_PLAYER_SKILL_MAX)
                new_max = DBC_PLAYER_SKILL_MAX;
        }
        else
        {
            new_max = 1;
        }

        // force to be within limits
        if (new_max > DBC_PLAYER_SKILL_MAX)
            new_max = DBC_PLAYER_SKILL_MAX;

        if (new_max < 1)
            new_max = 1;


        if (itr->second.MaximumValue != new_max)
        {
            dirty = true;
            itr->second.MaximumValue = new_max;
        }
        if (itr->second.CurrentValue > new_max)
        {
            dirty = true;
            itr->second.CurrentValue = new_max;
        }
    }

    if (dirty)
        _UpdateSkillFields();
}

void Player::_ModifySkillBonus(uint32 SkillLine, int32 Delta)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if (itr == m_skills.end())
        return;

    itr->second.BonusValue += Delta;
    _UpdateSkillFields();
}

void Player::_ModifySkillBonusByType(uint32 SkillType, int32 Delta)
{
    bool dirty = false;
    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
        if (itr->second.Skill->type == SkillType)
        {
            itr->second.BonusValue += Delta;
            dirty = true;
        }
    }

    if (dirty)
        _UpdateSkillFields();
}

///\todo check this formular
float PlayerSkill::GetSkillUpChance()
{
    float diff = float(MaximumValue - CurrentValue);
    return (diff * 100.0f / MaximumValue);
}

void Player::_RemoveLanguages()
{
    for (SkillMap::iterator itr = m_skills.begin(), it2; itr != m_skills.end();)
    {
        if (itr->second.Skill->type == SKILL_TYPE_LANGUAGE)
        {
            it2 = itr++;
            m_skills.erase(it2);
        }
        else
            ++itr;
    }
}

void PlayerSkill::Reset(uint32 Id)
{
    MaximumValue = 0;
    CurrentValue = 0;
    BonusValue = 0;
    Skill = (Id == 0) ? NULL : sSkillLineStore.LookupEntry(Id);
}

void Player::_AddLanguages(bool All)
{
    /** This function should only be used at login, and after _RemoveLanguages is called.
     * Otherwise weird stuff could happen :P
     * - Burlex
     */
    PlayerSkill sk;

    uint32 spell_id;
    static uint32 skills[] =
    {
        SKILL_LANG_COMMON,
        SKILL_LANG_ORCISH,
        SKILL_LANG_DWARVEN,
        SKILL_LANG_DARNASSIAN,
        SKILL_LANG_TAURAHE,
        SKILL_LANG_THALASSIAN,
        SKILL_LANG_TROLL,
        SKILL_LANG_GUTTERSPEAK,
        SKILL_LANG_DRAENEI,
#if VERSION_STRING == Cata
        SKILL_LANG_GOBLIN,
        SKILL_LANG_GILNEAN,
#endif
        0
    };

    if (All)
    {
        for (uint8 i = 0; skills[i] != 0; ++i)
        {
            if (!skills[i])
                break;

            sk.Reset(skills[i]);
            sk.MaximumValue = sk.CurrentValue = 300;
            m_skills.insert(std::make_pair(skills[i], sk));
            if ((spell_id = ::GetSpellForLanguage(skills[i])) != 0)
                addSpell(spell_id);
        }
    }
    else
    {
        for (std::list<CreateInfo_SkillStruct>::const_iterator itr = info->skills.begin(); itr != info->skills.end(); ++itr)
        {
            auto skill_line = sSkillLineStore.LookupEntry(itr->skillid);
            if (skill_line != nullptr)
            {
                if (skill_line->type == SKILL_TYPE_LANGUAGE)
                {
                    sk.Reset(itr->skillid);
                    sk.MaximumValue = sk.CurrentValue = 300;
                    m_skills.insert(std::make_pair(itr->skillid, sk));
                    if ((spell_id = ::GetSpellForLanguage(itr->skillid)) != 0)
                        addSpell(spell_id);
                }
            }
        }
    }

    _UpdateSkillFields();
}

float Player::GetSkillUpChance(uint32 id)
{
    SkillMap::iterator itr = m_skills.find(id);
    if (itr == m_skills.end())
        return 0.0f;

    return itr->second.GetSkillUpChance();
}

void Player::_RemoveAllSkills()
{
    m_skills.clear();
    _UpdateSkillFields();
}

void Player::_AdvanceAllSkills(uint32 count)
{
    bool dirty = false;
    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
        if (itr->second.CurrentValue != itr->second.MaximumValue)
        {
            itr->second.CurrentValue += count;
            if (itr->second.CurrentValue >= itr->second.MaximumValue)
                itr->second.CurrentValue = itr->second.MaximumValue;
            dirty = true;
        }
    }

    if (dirty)
        _UpdateSkillFields();
}

void Player::_ModifySkillMaximum(uint32 SkillLine, uint32 NewMax)
{
    // force to be within limits
    NewMax = (NewMax > DBC_PLAYER_SKILL_MAX ? DBC_PLAYER_SKILL_MAX : NewMax);

    SkillMap::iterator itr = m_skills.find(SkillLine);
    if (itr == m_skills.end())
        return;

    if (NewMax > itr->second.MaximumValue)
    {
        if (SkillLine == SKILL_RIDING)
            itr->second.CurrentValue = NewMax;

        itr->second.MaximumValue = NewMax;
        _UpdateSkillFields();
#if VERSION_STRING > TBC
        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, SkillLine, NewMax / 75, 0);
#endif
    }
}

/*! Calls UpdateHonor and UpdateArenaPoints
 *  \sa Player::UpdateHonor, Player::UpdateArenaPoints */
void Player::UpdatePvPCurrencies()
{
    this->UpdateHonor();
    this->UpdateArenaPoints();
}

/*! Fills parameters with reward for winning a random battleground
 *  \param wonBattleground True if the player won the battleground
 *  \param &honorPoints Amount of honor the player would receieve
 *  \param &arenaPoints Number of arena points the player would receive */
void Player::FillRandomBattlegroundReward(bool wonBattleground, uint32& honorPoints, uint32& arenaPoints)
{
    auto honorForSingleKill = HonorHandler::CalculateHonorPointsForKill(this->getLevel(), this->getLevel());

    if (wonBattleground)
    {
        if (this->m_bgIsRbgWon)
        {
            honorPoints = worldConfig.bg.honorableKillsRbg * honorForSingleKill;
            arenaPoints = worldConfig.bg.honorableArenaWinRbg;
        }
        else
        {
            honorPoints = worldConfig.bg.firstRbgHonorValueToday * honorForSingleKill;
            arenaPoints = worldConfig.bg.firstRbgArenaHonorValueToday;
        }
    }
    else
    {
        honorPoints = worldConfig.bg.honorByLosingRbg * honorForSingleKill;
        arenaPoints = worldConfig.bg.honorByLosingArenaRbg;
    }
}

/*! Increments the player's honor and arena points by the reward for winning an rbg
 *  \param wonBattleground True if the player won the battleground
 *  \note This does not set m_wonRbgToday */
void Player::ApplyRandomBattlegroundReward(bool wonBattleground)
{
    uint32 honorPoints, arenaPoints;
    this->FillRandomBattlegroundReward(wonBattleground, honorPoints, arenaPoints);
    this->AddHonor(honorPoints, false);
    this->AddArenaPoints(arenaPoints, false);
    this->UpdatePvPCurrencies();
}

//wooot, crappy code rulez.....NOT
void Player::EventTalentHearthOfWildChange(bool apply)
{
    if (!hearth_of_wild_pct)
        return;

    //druid hearth of the wild should add more features based on form
    int tval;
    if (apply)
        tval = hearth_of_wild_pct;
    else tval = -hearth_of_wild_pct;

    uint32 SS = GetShapeShift();

    //increase stamina if :
    if (SS == FORM_BEAR || SS == FORM_DIREBEAR)
    {
        TotalStatModPctPos[STAT_STAMINA] += tval;
        CalcStat(STAT_STAMINA);
        UpdateStats();
        UpdateChances();
    }
    //increase attackpower if :
    else if (SS == FORM_CAT)
    {
        SetAttackPowerMultiplier(getFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER) + tval / 200.0f);
        SetRangedAttackPowerMultiplier(GetRangedAttackPowerMultiplier() + tval / 200.0f);
        UpdateStats();
    }
}

void Player::EventGroupFullUpdate()
{
    if (m_playerInfo->m_Group)
    {
        //m_playerInfo->m_Group->Update();
        m_playerInfo->m_Group->UpdateAllOutOfRangePlayersFor(this);
    }
}

void Player::EjectFromInstance()
{
    if (m_bgEntryPointX && m_bgEntryPointY && m_bgEntryPointZ && !IS_INSTANCE(m_bgEntryPointMap))
    {
        if (SafeTeleport(m_bgEntryPointMap, m_bgEntryPointInstance, m_bgEntryPointX, m_bgEntryPointY, m_bgEntryPointZ, m_bgEntryPointO))
            return;
    }

    SafeTeleport(m_bind_mapid, 0, m_bind_pos_x, m_bind_pos_y, m_bind_pos_z, 0);
}

bool Player::HasQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size() > 0 && quest_spells.find(spellid) != quest_spells.end())
        return true;
    return false;
}

void Player::RemoveQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size() > 0)
        quest_spells.erase(spellid);
}

bool Player::HasQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size() > 0 && quest_mobs.find(entry) != quest_mobs.end())
        return true;
    return false;
}

bool Player::HasQuest(uint32 entry)
{
    for (uint8 i = 0; i < 25; i++)
    {
        if (m_questlog[i] != nullptr && m_questlog[i]->GetQuest()->id == entry)
            return true;
    }
    return false;
}

void Player::RemoveQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size() > 0)
        quest_mobs.erase(entry);
}

PlayerInfo::~PlayerInfo()
{
    if (m_Group != nullptr)
        m_Group->RemovePlayer(this);
}

void Player::CopyAndSendDelayedPacket(WorldPacket* data)
{
    WorldPacket* data2 = new WorldPacket(*data);
    delayedPackets.add(data2);
}

void Player::SendMeetingStoneQueue(uint32 DungeonId, uint8 Status)
{
    WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << DungeonId;
    data << Status;
    m_session->SendPacket(&data);
}

void Player::PartLFGChannel()
{
    Channel* pChannel = channelmgr.GetChannel("LookingForGroup", this);
    if (pChannel == nullptr)
        return;

    if (m_channels.find(pChannel) == m_channels.end())
        return;

    pChannel->Part(this);
}

//if we charmed or simply summoned a pet, this function should get called
void Player::EventSummonPet(Pet* new_pet)
{
    if (!new_pet)
        return; //another wtf error

    SpellSet::iterator it, iter;
    for (iter = mSpells.begin(); iter != mSpells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(SpellID);
        if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER)
        {
            this->removeAllAurasByIdForGuid(SpellID, this->GetGUID());   //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets(this->GetGUID());
            Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, nullptr);    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
        if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET)
        {
            this->removeAllAurasByIdForGuid(SpellID, this->GetGUID());   //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets(new_pet->GetGUID());
            Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, nullptr);    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
    }
    //there are talents that stop working after you gain pet
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_ON_PET)
            m_auras[x]->Remove();
    //pet should inherit some of the talents from caster
    //new_pet->InheritSMMods(); //not required yet. We cast full spell to have visual effect too
}

//if pet/charm died or whatever happened we should call this function
//!! note function might get called multiple times :P
void Player::EventDismissPet()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x])
            if (m_auras[x]->GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
                m_auras[x]->Remove();
}

void Player::AddShapeShiftSpell(uint32 id)
{
    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(id);
    mShapeShiftSpells.insert(id);

    if (sp->getRequiredShapeShift() && ((uint32)1 << (GetShapeShift() - 1)) & sp->getRequiredShapeShift())
    {
        Spell* spe = sSpellFactoryMgr.NewSpell(this, sp, true, nullptr);
        SpellCastTargets t(this->GetGUID());
        spe->prepare(&t);
    }
}

void Player::RemoveShapeShiftSpell(uint32 id)
{
    mShapeShiftSpells.erase(id);
    RemoveAura(id);
}

// COOLDOWNS
void Player::UpdatePotionCooldown()
{
    if (m_lastPotionId == 0 || CombatStatus.IsInCombat())
        return;

    ItemProperties const* proto = sMySQLStore.getItemProperties(m_lastPotionId);
    if (proto != nullptr)
    {
        for (uint8 i = 0; i < 5; ++i)
        {
            if (proto->Spells[i].Id && proto->Spells[i].Trigger == USE)
            {
                SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(proto->Spells[i].Id);
                if (spellInfo != nullptr)
                {
                    Cooldown_AddItem(proto, i);
                    SendSpellCooldownEvent(spellInfo->getId());
                }
            }
        }
    }

    m_lastPotionId = 0;
}

bool Player::HasSpellWithAuraNameAndBasePoints(uint32 auraname, uint32 basepoints)
{
    for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        SpellInfo *sp = sSpellCustomizations.GetSpellInfo(*itr);

        for (uint8_t i = 0; i < 3; ++i)
        {
            if (sp->getEffect(i) == SPELL_EFFECT_APPLY_AURA)
            {
                if (sp->getEffectApplyAuraName(i) == auraname && sp->getEffectBasePoints(i) == static_cast<int32>(basepoints - 1))
                    return true;
            }
        }

    }

    return false;
}

void Player::AddCategoryCooldown(uint32 category_id, uint32 time, uint32 SpellId, uint32 ItemId)
{
    PlayerCooldownMap::iterator itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(category_id);
    if (itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
    {
        if (itr->second.ExpireTime < time)
        {
            itr->second.ExpireTime = time;
            itr->second.ItemId = ItemId;
            itr->second.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown cd;
        cd.ExpireTime = time;
        cd.ItemId = ItemId;
        cd.SpellId = SpellId;

        m_cooldownMap[COOLDOWN_TYPE_CATEGORY].insert(std::make_pair(category_id, cd));
    }

    LogDebugFlag(LF_SPELL, "Player::AddCategoryCooldown added cooldown for COOLDOWN_TYPE_CATEGORY category_type %u time %u item %u spell %u", category_id, time - Util::getMSTime(), ItemId, SpellId);
}

void Player::_Cooldown_Add(uint32 Type, uint32 Misc, uint32 Time, uint32 SpellId, uint32 ItemId)
{
    PlayerCooldownMap::iterator itr = m_cooldownMap[Type].find(Misc);
    if (itr != m_cooldownMap[Type].end())
    {
        if (itr->second.ExpireTime < Time)
        {
            itr->second.ExpireTime = Time;
            itr->second.ItemId = ItemId;
            itr->second.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown cd;
        cd.ExpireTime = Time;
        cd.ItemId = ItemId;
        cd.SpellId = SpellId;

        m_cooldownMap[Type].insert(std::make_pair(Misc, cd));
    }

    LogDebugFlag(LF_SPELL, "Cooldown added cooldown for type %u misc %u time %u item %u spell %u", Type, Misc, Time - Util::getMSTime(), ItemId, SpellId);
}

void Player::Cooldown_Add(SpellInfo* pSpell, Item* pItemCaster)
{
    uint32 mstime = Util::getMSTime();
    int32 cool_time;
    uint32 spell_id = pSpell->getId();
    uint32 category_id = pSpell->getCategory();

    uint32 spell_category_recovery_time = pSpell->getCategoryRecoveryTime();
    if (spell_category_recovery_time > 0 && category_id)
    {
        spellModFlatIntValue(SM_FCooldownTime, &cool_time, pSpell->getSpellGroupType());
        spellModPercentageIntValue(SM_PCooldownTime, &cool_time, pSpell->getSpellGroupType());

        AddCategoryCooldown(category_id, spell_category_recovery_time + mstime, spell_id, pItemCaster ? pItemCaster->GetItemProperties()->ItemId : 0);
    }

    uint32 spell_recovery_t = pSpell->getRecoveryTime();
    if (spell_recovery_t > 0)
    {
        spellModFlatIntValue(SM_FCooldownTime, &cool_time, pSpell->getSpellGroupType());
        spellModPercentageIntValue(SM_PCooldownTime, &cool_time, pSpell->getSpellGroupType());

        _Cooldown_Add(COOLDOWN_TYPE_SPELL, spell_id, spell_recovery_t + mstime, spell_id, pItemCaster ? pItemCaster->GetItemProperties()->ItemId : 0);
    }
}

void Player::Cooldown_AddStart(SpellInfo* pSpell)
{
    if (pSpell->getStartRecoveryTime() == 0)
        return;

    uint32 mstime = Util::getMSTime();
    int32 atime; // = float2int32(float(pSpell->StartRecoveryTime) / SpellHasteRatingBonus);

    if (GetCastSpeedMod() >= 1.0f)
        atime = pSpell->getStartRecoveryTime();
    else
        atime = float2int32(pSpell->getStartRecoveryTime() * GetCastSpeedMod());

    spellModFlatIntValue(SM_FGlobalCooldown, &atime, pSpell->getSpellGroupType());
    spellModPercentageIntValue(SM_PGlobalCooldown, &atime, pSpell->getSpellGroupType());

    if (atime < 0)
        return;

    if (pSpell->getStartRecoveryCategory() && pSpell->getStartRecoveryCategory() != 133)        // if we have a different cool category to the actual spell category - only used by few spells
        AddCategoryCooldown(pSpell->getStartRecoveryCategory(), mstime + atime, pSpell->getId(), 0);
    //else if (pSpell->Category)                // cooldowns are grouped
    //_Cooldown_Add(COOLDOWN_TYPE_CATEGORY, pSpell->Category, mstime + pSpell->StartRecoveryTime, pSpell->getId(), 0);
    else                                    // no category, so it's a gcd
    {
        //LogDebugFlag(LF_SPELL, "Cooldown Global cooldown adding: %u ms", atime);
        m_globalCooldown = mstime + atime;

    }
}

bool Player::Cooldown_CanCast(SpellInfo* pSpell)
{
    PlayerCooldownMap::iterator itr;
    uint32 mstime = Util::getMSTime();

    if (pSpell->getCategory())
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(pSpell->getCategory());
        if (itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
        {
            if (mstime < itr->second.ExpireTime)
                return false;
            else
                m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(pSpell->getId());
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (mstime < itr->second.ExpireTime)
            return false;
        else
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    if (pSpell->getStartRecoveryTime() && m_globalCooldown && !this->CooldownCheat /* cebernic:GCD also cheat :D */)            /* gcd doesn't affect spells without a cooldown it seems */
    {
        if (mstime < m_globalCooldown)
            return false;
        else
            m_globalCooldown = 0;
    }

    return true;
}

void Player::Cooldown_AddItem(ItemProperties const* pProto, uint32 x)
{
    if (pProto->Spells[x].CategoryCooldown <= 0 && pProto->Spells[x].Cooldown <= 0)
        return;

    ItemSpell const* isp = &pProto->Spells[x];
    uint32 mstime = Util::getMSTime();

    uint32 item_spell_id = isp->Id;

    uint32 category_id = isp->Category;
    int32 category_cooldown_time = isp->CategoryCooldown;
    if (isp->CategoryCooldown > 0)
    {
        AddCategoryCooldown(category_id, category_cooldown_time + mstime, item_spell_id, pProto->ItemId);
    }

    int32 cooldown_time = isp->Cooldown;
    if (cooldown_time > 0)
        _Cooldown_Add(COOLDOWN_TYPE_SPELL, item_spell_id, cooldown_time + mstime, item_spell_id, pProto->ItemId);
}

bool Player::Cooldown_CanCast(ItemProperties const* pProto, uint32 x)
{
    PlayerCooldownMap::iterator itr;
    ItemSpell const* isp = &pProto->Spells[x];
    uint32 mstime = Util::getMSTime();

    if (isp->Category)
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(isp->Category);
        if (itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
        {
            if (mstime < itr->second.ExpireTime)
                return false;
            else
                m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(isp->Id);
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (mstime < itr->second.ExpireTime)
            return false;
        else
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    return true;
}

#define COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN 10000

void Player::_SavePlayerCooldowns(QueryBuffer* buf)
{
    PlayerCooldownMap::iterator itr;
    PlayerCooldownMap::iterator itr2;
    uint32 i;
    uint32 seconds;
    uint32 mstime = Util::getMSTime();

    // clear them (this should be replaced with an update queue later)
    if (buf != nullptr)
        buf->AddQuery("DELETE FROM playercooldowns WHERE player_guid = %u", GetLowGUID());        // 0 is guid always
    else
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", GetLowGUID());        // 0 is guid always

    for (i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        itr = m_cooldownMap[i].begin();
        for (; itr != m_cooldownMap[i].end();)
        {
            itr2 = itr++;

            // expired ones - no point saving, nor keeping them around, wipe em
            if (mstime >= itr2->second.ExpireTime)
            {
                m_cooldownMap[i].erase(itr2);
                continue;
            }

            // skip small cooldowns which will end up expiring by the time we log in anyway
            if ((itr2->second.ExpireTime - mstime) < COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN)
                continue;

            // work out the cooldown expire time in unix timestamp format
            // burlex's reason: 30 day overflow of 32bit integer, also
            // under windows we use GetTickCount() which is the system uptime, if we reboot
            // the server all these timestamps will appear to be messed up.

            seconds = (itr2->second.ExpireTime - mstime) / 1000;
            // this shouldn't ever be nonzero because of our check before, so no check needed

            if (buf != nullptr)
            {
                buf->AddQuery("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", GetLowGUID(),
                              i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId);
            }
            else
            {
                CharacterDatabase.Execute("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", GetLowGUID(),
                                          i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId);
            }
        }
    }
}

void Player::_LoadPlayerCooldowns(QueryResult* result)
{
    if (result == nullptr)
        return;

    // we should only really call Util::getMSTime() once to avoid user->system transitions, plus
    // the cost of calling a function for every cooldown the player has
    uint32 mstime = Util::getMSTime();
    uint32 type;
    uint32 misc;
    uint32 rtime;
    uint32 realtime;
    uint32 itemid;
    uint32 spellid;
    PlayerCooldown cd;

    do
    {
        type = result->Fetch()[0].GetUInt32();
        misc = result->Fetch()[1].GetUInt32();
        rtime = result->Fetch()[2].GetUInt32();
        spellid = result->Fetch()[3].GetUInt32();
        itemid = result->Fetch()[4].GetUInt32();

        if (type >= NUM_COOLDOWN_TYPES)
            continue;

        if ((uint32)UNIXTIME > rtime)
            continue;

        rtime -= (uint32)UNIXTIME;

        if (rtime < 10)
            continue;

        realtime = mstime + ((rtime)* 1000);

        // apply it back into cooldown map
        cd.ExpireTime = realtime;
        cd.ItemId = itemid;
        cd.SpellId = spellid;
        m_cooldownMap[type].insert(std::make_pair(misc, cd));

    }
    while (result->NextRow());
}

void Player::Social_AddFriend(const char* name, const char* note)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    std::map<uint32, char*>::iterator itr;

    // lookup the player
    PlayerInfo* playerInfo = objmgr.GetPlayerInfoByName(name);
    PlayerCache* playerCache = objmgr.GetPlayerCache(name, false);

    if (playerInfo == nullptr || (playerCache != nullptr && playerCache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM)))
    {
        data << uint8(FRIEND_NOT_FOUND);
        m_session->SendPacket(&data);

        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    // team check
    if (playerInfo->team != GetTeamInitial() && m_session->permissioncount == 0 && !worldConfig.player.isInterfactionFriendsEnabled)
    {
        data << uint8(FRIEND_ENEMY);
        data << uint64(playerInfo->guid);
        m_session->SendPacket(&data);
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    // are we ourselves?
    if (playerCache != nullptr && playerCache->GetUInt32Value(CACHE_PLAYER_LOWGUID) == GetLowGUID())
    {
        data << uint8(FRIEND_SELF);
        data << GetGUID();
        m_session->SendPacket(&data);
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    if (m_cache->CountValue64(CACHE_SOCIAL_FRIENDLIST, playerInfo->guid))
    {
        data << uint8(FRIEND_ALREADY);
        data << uint64(playerInfo->guid);
        m_session->SendPacket(&data);
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    if (playerCache != nullptr)   //hes online if he has a cache
    {
        data << uint8(FRIEND_ADDED_ONLINE);
        data << uint64(playerCache->GetUInt32Value(CACHE_PLAYER_LOWGUID));
        if (note != nullptr)
            data << note;
        else
            data << uint8(0);

        data << uint8(1);
        data << playerInfo->m_loggedInPlayer->GetZoneId();
        data << playerInfo->lastLevel;
        data << uint32(playerInfo->cl);

        playerCache->InsertValue64(CACHE_SOCIAL_HASFRIENDLIST, GetLowGUID());
    }
    else
    {
        data << uint8(FRIEND_ADDED_OFFLINE);
        data << uint64(playerInfo->guid);
    }

    char* notedup = note == nullptr ? NULL : strdup(note);
    m_cache->InsertValue64(CACHE_SOCIAL_FRIENDLIST, playerInfo->guid, notedup);

    m_session->SendPacket(&data);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')",
                              GetLowGUID(), playerInfo->guid, note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "");

    if (playerCache != nullptr)
        playerCache->DecRef();
}

void Player::Social_RemoveFriend(uint32 guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // are we ourselves?
    if (guid == GetLowGUID())
    {
        data << uint8(FRIEND_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    //free note first
    m_cache->AcquireLock64(CACHE_SOCIAL_FRIENDLIST);
    PlayerCacheMap::iterator itr = m_cache->Find64(CACHE_SOCIAL_FRIENDLIST, guid);
    if (itr != m_cache->End64(CACHE_SOCIAL_FRIENDLIST) && itr->second != nullptr)
    {
        free(itr->second);
        itr->second = nullptr;
    }
    m_cache->RemoveValue64(CACHE_SOCIAL_FRIENDLIST, guid);
    m_cache->ReleaseLock64(CACHE_SOCIAL_FRIENDLIST);

    data << uint8(FRIEND_REMOVED);
    data << uint64(guid);

    PlayerCache* cache = objmgr.GetPlayerCache((uint32)guid);
    if (cache != nullptr)
    {
        cache->RemoveValue64(CACHE_SOCIAL_HASFRIENDLIST, GetLowGUID());
        cache->DecRef();
    }

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u",
                              GetLowGUID(), (uint32)guid);
}

void Player::Social_SetNote(uint32 guid, const char* note)
{
    //free note first
    m_cache->AcquireLock64(CACHE_SOCIAL_FRIENDLIST);
    PlayerCacheMap::iterator itr = m_cache->Find64(CACHE_SOCIAL_FRIENDLIST, guid);
    if (itr != m_cache->End64(CACHE_SOCIAL_FRIENDLIST))
    {
        if (itr->second != nullptr)
            free(itr->second);
        itr->second = strdup(note);
    }
    m_cache->ReleaseLock64(CACHE_SOCIAL_FRIENDLIST);

    CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u",
                              note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "", GetLowGUID(), guid);
}

void Player::Social_AddIgnore(const char* name)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    PlayerInfo* playerInfo;

    // lookup the player
    playerInfo = objmgr.GetPlayerInfoByName(name);
    if (playerInfo == nullptr)
    {
        data << uint8(FRIEND_IGNORE_NOT_FOUND);
        m_session->SendPacket(&data);
        return;
    }

    // are we ourselves?
    if (playerInfo == m_playerInfo)
    {
        data << uint8(FRIEND_IGNORE_SELF);
        data << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    if (m_cache->CountValue64(CACHE_SOCIAL_IGNORELIST, playerInfo->guid) > 0)
    {
        data << uint8(FRIEND_IGNORE_ALREADY);
        data << uint64(playerInfo->guid);
        m_session->SendPacket(&data);
        return;
    }

    data << uint8(FRIEND_IGNORE_ADDED);
    data << uint64(playerInfo->guid);

    m_cache->InsertValue64(CACHE_SOCIAL_IGNORELIST, playerInfo->guid);
    m_session->SendPacket(&data);

    // dump into db
    CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", GetLowGUID(), playerInfo->guid);
}

void Player::Social_RemoveIgnore(uint32 guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // are we ourselves?
    if (guid == GetLowGUID())
    {
        data << uint8(FRIEND_IGNORE_SELF);
        data << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_cache->RemoveValue64(CACHE_SOCIAL_IGNORELIST, guid);
    data << uint8(FRIEND_IGNORE_REMOVED);
    data << uint64(guid);

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u",
                              GetLowGUID(), (uint32)guid);
}

bool Player::Social_IsIgnoring(PlayerInfo* m_info)
{
    return m_cache->CountValue64(CACHE_SOCIAL_IGNORELIST, m_info->guid) > 0;
}

bool Player::Social_IsIgnoring(uint32 guid)
{
    return m_cache->CountValue64(CACHE_SOCIAL_IGNORELIST, guid) > 0;
}

void Player::Social_TellFriendsOnline()
{
    if (m_cache->GetSize64(CACHE_SOCIAL_HASFRIENDLIST) == 0)
        return;

    WorldPacket data(SMSG_FRIEND_STATUS, 22);
    data << uint8(FRIEND_ONLINE);
    data << GetGUID();
    data << uint8(1);
    data << GetAreaID();
    data << getLevel();
    data << uint32(getClass());

    m_cache->AcquireLock64(CACHE_SOCIAL_HASFRIENDLIST);
    for (PlayerCacheMap::iterator itr = m_cache->Begin64(CACHE_SOCIAL_HASFRIENDLIST); itr != m_cache->End64(CACHE_SOCIAL_HASFRIENDLIST); ++itr)
    {
        PlayerCache* cache = objmgr.GetPlayerCache(uint32(itr->first));
        if (cache != nullptr)
        {
            cache->SendPacket(data);
            cache->DecRef();
        }
    }
    m_cache->ReleaseLock64(CACHE_SOCIAL_HASFRIENDLIST);
}

void Player::Social_TellFriendsOffline()
{
    if (m_cache->GetSize64(CACHE_SOCIAL_HASFRIENDLIST) == 0)
        return;

    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    data << uint8(FRIEND_OFFLINE);
    data << GetGUID();
    data << uint8(0);

    m_cache->AcquireLock64(CACHE_SOCIAL_HASFRIENDLIST);
    for (PlayerCacheMap::iterator itr = m_cache->Begin64(CACHE_SOCIAL_HASFRIENDLIST); itr != m_cache->End64(CACHE_SOCIAL_HASFRIENDLIST); ++itr)
    {
        PlayerCache* cache = objmgr.GetPlayerCache(uint32(itr->first));
        if (cache != nullptr)
        {
            cache->SendPacket(data);
            cache->DecRef();
        }
    }
    m_cache->ReleaseLock64(CACHE_SOCIAL_HASFRIENDLIST);
}

void Player::Social_SendFriendList(uint32 flag)
{
    WorldPacket data(SMSG_CONTACT_LIST, 500);
    data << flag;
    data << uint32(m_cache->GetSize64(CACHE_SOCIAL_FRIENDLIST) + m_cache->GetSize64(CACHE_SOCIAL_IGNORELIST));
    m_cache->AcquireLock64(CACHE_SOCIAL_FRIENDLIST);

    for (PlayerCacheMap::iterator itr = m_cache->Begin64(CACHE_SOCIAL_FRIENDLIST); itr != m_cache->End64(CACHE_SOCIAL_FRIENDLIST); ++itr)
    {
        // guid
        data << uint64(itr->first);

        // friend/ignore flag.
        // 1 - friend
        // 2 - ignore
        // 3 - muted?
        data << uint32(1);

        // player note
        if (itr->second != nullptr)
        {
            char* note = (char*)itr->second;
            data << note;
        }
        else
            data << uint8(0);

        // online/offline flag
        Player* plr = objmgr.GetPlayer((uint32)itr->first);
        PlayerCache* cache = objmgr.GetPlayerCache((uint32)itr->first);
        if (plr != nullptr)
        {
            data << uint8(1);
            data << plr->GetZoneId();
            data << plr->getLevel();
            data << uint32(plr->getClass());
        }
        else
            data << uint8(0);

        if (cache != nullptr)
            cache->DecRef();
    }
    m_cache->ReleaseLock64(CACHE_SOCIAL_FRIENDLIST);

    m_cache->AcquireLock64(CACHE_SOCIAL_IGNORELIST);
    PlayerCacheMap::iterator ignoreitr = m_cache->Begin64(CACHE_SOCIAL_IGNORELIST);
    for (; ignoreitr != m_cache->End64(CACHE_SOCIAL_IGNORELIST); ++ignoreitr)
    {
        // guid
        data << uint64(ignoreitr->first);
        // ignore flag - 2
        data << uint32(2);
        // no note
        data << uint8(0);
    }

    m_cache->ReleaseLock64(CACHE_SOCIAL_IGNORELIST);
    m_session->SendPacket(&data);
}

void Player::SpeedCheatDelay(uint32 ms_delay)
{
    //    SDetector->SkipSamplingUntil(Util::getMSTime() + ms_delay);
    //add triple latency to avoid client handling the spell effect with delay and we detect as cheat
    //    SDetector->SkipSamplingUntil(Util::getMSTime() + ms_delay + GetSession()->GetLatency() * 3);
    //add constant value to make sure the effect packet was sent to client from network pool
    SDetector->SkipSamplingUntil(Util::getMSTime() + ms_delay + GetSession()->GetLatency() * 2 + 2000);   //2 second should be enough to send our packets to client
}

// Reset GM speed hacks after a SafeTeleport
void Player::SpeedCheatReset()
{
    // wtf?
    SDetector->EventSpeedChange();

    /*
    setSpeedForType(TYPE_RUN, m_runSpeed);
    setSpeedForType(SWIM, m_runSpeed);
    setSpeedForType(RUNBACK, m_runSpeed / 2); // Backwards slower, it's more natural :P
    setSpeedForType(FLY, m_flySpeed);
    */
}

uint32 Player::GetMaxPersonalRating()
{
    uint32 maxrating = 0;
    int i;

    ARCEMU_ASSERT(m_playerInfo != NULL);

    for (i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
    {
        if (m_arenaTeams[i] != nullptr)
        {
            ArenaTeamMember* m = m_arenaTeams[i]->GetMemberByGuid(m_playerInfo->guid);
            if (m)
            {
                if (m->PersonalRating > maxrating) maxrating = m->PersonalRating;
            }
            else
            {
                LOG_ERROR("%s: GetMemberByGuid returned NULL for player guid = %u", __FUNCTION__, m_playerInfo->guid);
            }
        }
    }

    return maxrating;
}
/***********************************
* Give player full hp/mana
***********************************/

void Player::FullHPMP()
{
    if (IsDead())
        ResurrectPlayer();
    SetHealth(GetMaxHealth());
    SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
    SetPower(POWER_TYPE_ENERGY, GetMaxPower(POWER_TYPE_ENERGY));
}

/**********************************************
* Remove all temporary enchants from all items
**********************************************/
void Player::RemoveTempEnchantsOnArena()
{
    ItemInterface* itemi = GetItemInterface();

    // Loop through all equipment items
    for (uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        Item* it = itemi->GetInventoryItem(static_cast<int16>(x));

        if (it != nullptr)
        {
            it->RemoveAllEnchantments(true);
        }
    }

    // Loop through all your bags..
    for (uint32 x = INVENTORY_SLOT_BAG_START; x < INVENTORY_SLOT_BAG_END; ++x)
    {
        Item* it = itemi->GetInventoryItem(static_cast<int16>(x));

        if (it != nullptr)
        {
            if (it->IsContainer())
            {
                Container* bag = static_cast< Container* >(it);
                for (uint32 ci = 0; ci < bag->GetItemProperties()->ContainerSlots; ++ci)
                {
                    it = bag->GetItem(static_cast<int16>(ci));
                    if (it != nullptr)
                        it->RemoveAllEnchantments(true);
                }
            }
        }
    }

    // Loop through all your invintory items
    for (uint32 x = INVENTORY_SLOT_ITEM_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        Item* it = itemi->GetInventoryItem(static_cast<int16>(x));

        if (it != nullptr)
        {
            it->RemoveAllEnchantments(true);
        }
    }
}

void Player::PlaySoundToPlayer(uint64_t from_guid,  uint32_t sound_id)
{
    WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 12);
    data << uint32_t(sound_id);
    data << uint64_t(from_guid);
    SendMessageToSet(&data, true);
}

void Player::PlaySound(uint32 sound_id)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_id;
    GetSession()->SendPacket(&data);
}

void Player::UpdatePowerAmm()
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_POWER_UPDATE, 5);
    FastGUIDPack(data, GetGUID());
    data << uint8(GetPowerType());
    data << getUInt32Value(UNIT_FIELD_POWER1 + GetPowerType());
    SendMessageToSet(&data, true);
#endif
}

// Initialize Glyphs or update them after level change
#if VERSION_STRING == WotLK
void Player::UpdateGlyphs()
{
    uint32 level = getLevel();

    if (level >= 15)
    {
        uint16 y = 0;
        for (uint32 i = 0; i < sGlyphSlotStore.GetNumRows(); ++i)
        {
            auto glyph_slot = sGlyphSlotStore.LookupEntry(i);
            if (glyph_slot == nullptr)
                continue;

            if (glyph_slot->Slot > 0)
                setUInt32Value(PLAYER_FIELD_GLYPH_SLOTS_1 + y++, glyph_slot->Id);
        }
    }

    if (level > DBC_PLAYER_LEVEL_CAP)
        level = DBC_PLAYER_LEVEL_CAP;

    // Enable number of glyphs depending on level
    setUInt32Value(PLAYER_GLYPHS_ENABLED, glyphMask[level]);
}
#endif

#if VERSION_STRING == Cata
enum GlyphSlotMask
{
    GS_MASK_1 = 0x001,
    GS_MASK_2 = 0x002,
    GS_MASK_3 = 0x040,

    GS_MASK_4 = 0x004,
    GS_MASK_5 = 0x008,
    GS_MASK_6 = 0x080,

    GS_MASK_7 = 0x010,
    GS_MASK_8 = 0x020,
    GS_MASK_9 = 0x100,

    GS_MASK_LEVEL_25 = GS_MASK_1 | GS_MASK_2 | GS_MASK_3,
    GS_MASK_LEVEL_50 = GS_MASK_4 | GS_MASK_5 | GS_MASK_6,
    GS_MASK_LEVEL_75 = GS_MASK_7 | GS_MASK_8 | GS_MASK_9
};

void Player::UpdateGlyphs()
{
    uint32 slot = 0;
    for (uint32 i = 0; i < sGlyphSlotStore.GetNumRows(); ++i)
    {
        if (DBC::Structures::GlyphSlotEntry const* glyphSlot = sGlyphSlotStore.LookupEntry(i))
        {
            setUInt32Value(PLAYER_FIELD_GLYPH_SLOTS_1 + slot++, glyphSlot->Id);
        }
    }

    uint32 level = getLevel();
    uint32 slotMask = 0;

    if (level >= 25)
    {
        slotMask = GS_MASK_LEVEL_25;
    }
    if (level >= 50)
    {
        slotMask = GS_MASK_LEVEL_50;
    }
    if (level >= 75)
    {
        slotMask = GS_MASK_LEVEL_75;
    }

    setUInt32Value(PLAYER_GLYPHS_ENABLED, slotMask);
}
#endif

// Fills fields from firstField to firstField+fieldsNum-1 with integers from the string
void Player::LoadFieldsFromString(const char* string, uint16 firstField, uint32 fieldsNum)
{
    if (string == nullptr)
        return;
    char* end;
    char* start = (char*)string;
    for (uint16 Counter = 0; Counter < fieldsNum; Counter++)
    {
        end = strchr(start, ',');
        if (!end)
            break;
        *end = 0;
        setUInt32Value(firstField + Counter, atol(start));
        start = end + 1;
    }
}

void Player::SetKnownTitle(RankTitles title, bool set)
{
    if (!HasTitle(title) ^ set)
        return;

    uint64 current = getUInt64Value(PLAYER_FIELD_KNOWN_TITLES + ((title >> 6) << 1));
    if (set)
        setUInt64Value(PLAYER_FIELD_KNOWN_TITLES + ((title >> 6) << 1), current | uint64(1) << (title % 64));
    else
        setUInt64Value(PLAYER_FIELD_KNOWN_TITLES + ((title >> 6) << 1), current & ~uint64(1) << (title % 64));

    WorldPacket data(SMSG_TITLE_EARNED, 8);
    data << uint32(title);
    data << uint32(set ? 1 : 0);
    m_session->SendPacket(&data);
}

uint32 Player::GetInitialFactionId()
{

    PlayerCreateInfo const* pci = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    if (pci != nullptr)
        return pci->factiontemplate;
    else
        return 35;
}

void Player::CalcExpertise()
{
    int32 modifier = 0;
    int32 val = 0;
    SpellInfo* entry = nullptr;

#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_EXPERTISE, 0);
    setUInt32Value(PLAYER_OFFHAND_EXPERTISE, 0);
#endif

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] != nullptr && m_auras[x]->HasModType(SPELL_AURA_EXPERTISE))
        {
            entry = m_auras[x]->m_spellInfo;
            val = m_auras[x]->GetModAmountByMod();

            if (entry->getEquippedItemSubClass() != 0)
            {
                auto item_mainhand = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                auto item_offhand = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                uint32 reqskillMH = 0;
                uint32 reqskillOH = 0;

                if (item_mainhand)
                    reqskillMH = entry->getEquippedItemSubClass() & (((uint32)1) << item_mainhand->GetItemProperties()->SubClass);
                if (item_offhand)
                    reqskillOH = entry->getEquippedItemSubClass() & (((uint32)1) << item_offhand->GetItemProperties()->SubClass);

                if (reqskillMH != 0 || reqskillOH != 0)
                    modifier = +val;
            }
            else
                modifier += val;
        }
    }

#if VERSION_STRING != Classic
    ModUnsigned32Value(PLAYER_EXPERTISE, (int32)CalcRating(PCR_EXPERTISE) + modifier);
    ModUnsigned32Value(PLAYER_OFFHAND_EXPERTISE, (int32)CalcRating(PCR_EXPERTISE) + modifier);
#endif
    UpdateStats();
}

void Player::UpdateKnownCurrencies(uint32 itemId, bool apply)
{
#if VERSION_STRING == WotLK
    if (auto const* currency_type_entry = sCurrencyTypesStore.LookupEntry(itemId))
    {
        if (apply)
        {
            uint64 oldval = getUInt64Value(PLAYER_FIELD_KNOWN_CURRENCIES);
            uint64 newval = oldval | (1LL << (currency_type_entry->bit_index - 1));
            setUInt64Value(PLAYER_FIELD_KNOWN_CURRENCIES, newval);
        }
        else
        {
            uint64 oldval = getUInt64Value(PLAYER_FIELD_KNOWN_CURRENCIES);
            uint64 newval = oldval & ~(1LL << (currency_type_entry->bit_index - 1));
            setUInt64Value(PLAYER_FIELD_KNOWN_CURRENCIES, newval);
        }
    }
#else
    if (itemId == 0 || apply ) { return; }
#endif
}

void Player::RemoveItemByGuid(uint64 GUID)
{
    this->GetItemInterface()->SafeFullRemoveItemByGuid(GUID);
}

void Player::SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active)
{
    if (active)
    {
        if (!shapeshift_form)
            return;

        WorldPacket data(SMSG_PET_SPELLS, 8 * 4 + 20);
        data << GetGUID();
        data << uint32(0);
        data << uint32(0);
        data << uint8(0);
        data << uint8(0);
        data << uint16(0);

        // Send the spells
        for (uint8 i = 0; i < 8; i++)
        {
            data << uint16(shapeshift_form->spells[i]);
            data << uint16(DEFAULT_SPELL_STATE);
        }

        data << uint8(1);
        data << uint8(0);
        GetSession()->SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_PET_SPELLS, 10);
        data << uint64(0);
        data << uint32(0);
        GetSession()->SendPacket(&data);
    }
}

bool Player::IsPvPFlagged()
{
    return hasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Player::SetPvPFlag()
{
    StopPvPTimer();

    setByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

    summonhandler.SetPvPFlags();

    // flagging the pet too for PvP, if we have one
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->SetPvPFlag();
    }

    if (CombatStatus.IsInCombat())
        SetFlag(PLAYER_FLAGS, 0x100);

}

void Player::RemovePvPFlag()
{
    StopPvPTimer();
    removeByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP);

    summonhandler.RemovePvPFlags();

    // If we have a pet we will remove the pvp flag from that too
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->RemovePvPFlag();
    }
}

bool Player::IsFFAPvPFlagged()
{
    return hasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Player::SetFFAPvPFlag()
{
    StopPvPTimer();
    setByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_FREE_FOR_ALL_PVP);

    summonhandler.SetFFAPvPFlags();

    // flagging the pet too for FFAPvP, if we have one
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->SetFFAPvPFlag();
    }
}

void Player::RemoveFFAPvPFlag()
{
    StopPvPTimer();
    removeByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_FREE_FOR_ALL_PVP);

    summonhandler.RemoveFFAPvPFlags();

    // If we have a pet we will remove the FFA pvp flag from that too
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->RemoveFFAPvPFlag();
    }
}

bool Player::IsSanctuaryFlagged()
{
    return hasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_SANCTUARY);
}

void Player::SetSanctuaryFlag()
{
    setByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_SANCTUARY);

    summonhandler.SetSanctuaryFlags();

    // flagging the pet too for sanctuary, if we have one
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->SetSanctuaryFlag();
    }
}

void Player::RemoveSanctuaryFlag()
{
    removeByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_SANCTUARY);

    summonhandler.RemoveSanctuaryFlags();

    // If we have a pet we will remove the sanctuary flag from that too
    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->RemoveSanctuaryFlag();
    }
}

void Player::SendExploreXP(uint32 areaid, uint32 xp)
{
    WorldPacket data(SMSG_EXPLORATION_EXPERIENCE, 8);
    data << uint32(areaid);
    data << uint32(xp);
    m_session->SendPacket(&data);
}

void Player::HandleSpellLoot(uint32 itemid)
{
    Loot loot1;
    std::vector< __LootItem >::iterator itr;

    lootmgr.FillItemLoot(&loot1, itemid);

    for (itr = loot1.items.begin(); itr != loot1.items.end(); ++itr)
    {
        uint32 looteditemid = itr->item.itemproto->ItemId;
        uint32 count = itr->iItemsCount;

        m_ItemInterface->AddItemById(looteditemid, count, 0);
    }
}

#if VERSION_STRING != Cata
void Player::LearnTalent(uint32 talentid, uint32 rank, bool isPreviewed)
{
    uint32 CurTalentPoints = m_specs[m_talentActiveSpec].GetTP();   // Calculate free points in active spec

    if (CurTalentPoints == 0)
        return;

    if (rank > 4)
        return;

    auto talent_info = sTalentStore.LookupEntry(talentid);
    if (talent_info == nullptr)
        return;

    if (objmgr.IsSpellDisabled(talent_info->RankID[rank]))
    {
        if (IsInWorld())
            SendCastResult(talent_info->RankID[rank], SPELL_FAILED_SPELL_UNAVAILABLE, 0, 0);

        return;
    }

    // Check if it requires another talent
    if (talent_info->DependsOn > 0)
    {
        auto depends_talent = sTalentStore.LookupEntry(talent_info->DependsOn);
        if (depends_talent)
        {
            bool hasEnoughRank = false;
            for (uint8 i = 0; i < 5; ++i)
            {
                if (depends_talent->RankID[i] != 0)
                {
                    if (HasSpell(depends_talent->RankID[i]))
                    {
                        hasEnoughRank = true;
                        break;
                    }
                }
            }
            if (!hasEnoughRank)
                return;
        }
    }

    // Find out how many points we have in this field
    uint32 spentPoints = 0;

    // points we are spending now
    int32 points = 0;

    uint32 tTree = talent_info->TalentTree;
    uint32 cl = getClass();

    for (uint8 k = 0; k < 3; ++k)
    {
        if (tTree == TalentTreesPerClass[cl][k])
        {
            break;
        }
    }

    if (talent_info->Row > 0)
    {
        for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)           // Loop through all talents.
        {
            // Someday, someone needs to revamp
            auto temp_talent = sTalentStore.LookupEntry(i);
            if (temp_talent)                                   // the way talents are tracked
            {
                if (temp_talent->TalentTree == tTree)
                {
                    for (uint8 j = 0; j < 5; j++)
                    {
                        if (temp_talent->RankID[j] != 0)
                        {
                            if (HasSpell(temp_talent->RankID[j]))
                            {
                                spentPoints += j + 1;
                                //    break;
                            }
                        }
                        else
                            break;
                    }
                }
            }
        }
    }

    uint32 spellid = talent_info->RankID[rank];
    if (spellid == 0)
    {
        LOG_DETAIL("Talent: %u Rank: %u = 0", talentid, rank);
    }
    else
    {
        if (spentPoints < (talent_info->Row * 5))             // Min points spent
        {
            return;
        }


        // Check if we already have the talent with the same or higher rank
        for (uint32 i = rank; i < 5; ++i)
            if (talent_info->RankID[i] != 0 && HasSpell(talent_info->RankID[i]))
                return; // cheater

        if (rank > 0)
        {
            // If we are not learning thru the preview system, check if we have the lower rank of the talent
            if (talent_info->RankID[rank - 1] && !HasSpell(talent_info->RankID[rank - 1]) && !isPreviewed)
            {
                // cheater
                return;
            }

            int32 highest = 0;
            for (highest = 4; highest >= 0; highest--)
                if ((talent_info->RankID[highest] != 0) && HasSpell(talent_info->RankID[highest]))
                    break;

            points = static_cast<int32>(rank)-highest;
        }
        else
            points = 1;

        if (static_cast<uint32>(points) > CurTalentPoints)
            return;

        if (!(HasSpell(spellid)))
        {
            addSpell(spellid);

            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellid);

            if (rank > 0)
            {
                uint32 respellid = talent_info->RankID[rank - 1];
                if (respellid && !isPreviewed)
                {
                    removeSpell(respellid, false, false, 0);
                    RemoveAura(respellid);
                }
            }

            if (spellInfo->IsPassive() || ((spellInfo->getEffect(0) == SPELL_EFFECT_LEARN_SPELL ||
                spellInfo->getEffect(1) == SPELL_EFFECT_LEARN_SPELL ||
                spellInfo->getEffect(2) == SPELL_EFFECT_LEARN_SPELL)
                && ((spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) == 0 || ((spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) && GetSummon()))))
            {
                if (spellInfo->getRequiredShapeShift() && !((uint32)1 << (GetShapeShift() - 1) & spellInfo->getRequiredShapeShift()))
                {
                    // do nothing
                }
                else
                {
                    Spell* sp = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
                    SpellCastTargets tgt;
                    tgt.m_unitTarget = this->GetGUID();
                    sp->prepare(&tgt);
                }
            }

            SetCurrentTalentPoints(CurTalentPoints - static_cast<uint32>(points));
            m_specs[m_talentActiveSpec].AddTalent(talentid, uint8(rank));
            smsg_TalentsInfo(false);
        }
    }
}
#else
void Player::LearnTalent(uint32 talentid, uint32 rank, bool isPreviewed)
{
    uint32 CurTalentPoints = m_specs[m_talentActiveSpec].GetTP();   // Calculate free points in active spec

    int32 TP_needed;
    if (isPreviewed == false)
        TP_needed = 1;
    else
        TP_needed = rank + 1;

    if ((int32)CurTalentPoints < TP_needed)
    {
        LogDebug("LearnTalent: Have not enough talentpoints to spend");
        return;
    }

    if (rank > 4)
    {
        LogDebug("LearnTalent: Requested rank is greater then 4");
        return;
    }

    auto talent_info = sTalentStore.LookupEntry(talentid);
    if (talent_info == nullptr)
        return;

    if (objmgr.IsSpellDisabled(talent_info->RankID[rank]))
    {
        if (IsInWorld())
            SendCastResult(talent_info->RankID[rank], SPELL_FAILED_SPELL_UNAVAILABLE, 0, 0);

        return;
    }

    DBC::Structures::TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talent_info->TalentTree);

    if (!talentTabInfo)
    {
        LogDebug("Could not find talent tab %u in talent DBC \n", talent_info->TalentTree);
        return;
    }

    if (m_FirstTalentTreeLock == 0)
    {
        m_FirstTalentTreeLock = talent_info->TalentTree;
        //TODO Mastery Spells

    }

    //tree is locked until we have 31 points in it
    if (talentTabInfo->TalentTabID != m_FirstTalentTreeLock && CalcTalentPointsHaveSpent(m_talentActiveSpec) < 31)
    {
        LogDebug("Trying to learn non primary tab spell before we have full tab spec \n");
        return;
    }

    // Check if it requires another talent
    if (talent_info->DependsOn > 0)
    {
        auto depends_talent = sTalentStore.LookupEntry(talent_info->DependsOn);
        if (depends_talent)
        {
            bool hasEnoughRank = false;
            for (int32 i = (int32)talent_info->DependsOnRank - 1; i < 5; ++i)
            {
                if (depends_talent->RankID[i] != 0)
                {
                    if (HasSpell(depends_talent->RankID[i]))
                    {
                        hasEnoughRank = true;
                        break;
                    }
                }
            }
            if (!hasEnoughRank)
                return;
        }
        else
        {
            LogDebug("Could not find talent dependency %u for talent %u \n", talent_info->DependsOn, talentid);
            return;
        }
    }

    if ((talentTabInfo->ClassMask & (1 << (getClass() - 1))) == 0)
    {
        SoftDisconnect();
        LogDebug("LearnTalent: Talent is not for our class");
        return;
    }

    uint32 spellid = talent_info->RankID[rank];
    if (spellid == 0)
    {
        LogDetail("Talent: %u Rank: %u = 0", talentid, rank);
    }
    else
    {
        if (rank > 0)
        {
            if (talent_info->RankID[rank - 1] && !HasSpell(talent_info->RankID[rank - 1]) && !isPreviewed)
            {
                // cheater
                LogDebug("LearnTalent: Missing required rank");
                return;
            }
        }

        for (uint8 i = rank; i < 5; ++i)
        {
            if (talent_info->RankID[i] != 0 && HasSpell(talent_info->RankID[i]))
            {
                LogDebug("LearnTalent: Has rank %u higher already higher or equal.Spell id %u", i, talent_info->RankID[i]);
                //return; // cheater
            }
        }


        if (!HasSpell(spellid))
        {
            addSpell(spellid);

            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellid);
            if (spellInfo == nullptr)
            {
                LOG_ERROR("Your table `spells` miss skill spell %u!", spellid);
                return;
            }
            //make sure pets that get bonus from owner do not stack it up
            if (getClass() == HUNTER && GetSummon())
                GetSummon()->RemoveAura(spellInfo->getId());

            if (rank > 0)
            {
                uint32 respellid = talent_info->RankID[rank - 1];
                if (respellid)
                {
                    removeSpell(respellid, false, false, 0);
                    RemoveAura(respellid);

                    SpellInfo* spellInfoReq = sSpellCustomizations.GetSpellInfo(respellid);
                    if (spellInfoReq == nullptr)
                    {
                        LOG_ERROR("Your table `spells` miss skill spell %u!", spellid);
                        return;
                    }
                    if (getClass() == HUNTER && GetSummon())
                        GetSummon()->RemoveAura(spellInfoReq->getId());
                }
            }

            int32 ss = GetShapeShiftMask();
            if (spellInfo->RequiredShapeShift == 0 || (ss & spellInfo->RequiredShapeShift) != 0)
            {
                if (spellInfo->IsPassive()
                     || (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL ||
                         spellInfo->Effect[1] == SPELL_EFFECT_LEARN_SPELL ||
                         spellInfo->Effect[2] == SPELL_EFFECT_LEARN_SPELL))
                {
                    Spell* sp = sSpellFactoryMgr.NewSpell(this, spellInfo, true, nullptr);
                    SpellCastTargets tgt;
                    tgt.m_unitTarget = this->GetGUID();
                    sp->prepare(&tgt);
                }
            }

            m_specs[m_talentActiveSpec].AddTalent(talentid, uint8(rank));
            SetCurrentTalentPoints(CurTalentPoints - static_cast<uint32>(TP_needed));
            if (isPreviewed == false)
                smsg_TalentsInfo(false);
        }
    }
}

uint32 Player::CalcTalentPointsHaveSpent(uint32 spec)
{
    std::map<uint32, uint8>::iterator treeitr;
    int32 points_used_up = 0;

    for (treeitr = m_specs[spec].talents.begin(); treeitr != m_specs[spec].talents.end(); treeitr++)
        points_used_up += treeitr->second + 1;

    return points_used_up;
}
#endif

void Player::SetDungeonDifficulty(uint8 diff)
{
    iInstanceType = diff;
}

uint8 Player::GetDungeonDifficulty()
{
    return iInstanceType;
}

void Player::SetRaidDifficulty(uint8 diff)
{
    m_RaidDifficulty = diff;
}

uint8 Player::GetRaidDifficulty()
{
    return m_RaidDifficulty;
}

void Player::SendPreventSchoolCast(uint32 SpellSchool, uint32 unTimeMs)
{
    WorldPacket data(SMSG_SPELL_COOLDOWN, 8 + 1 + mSpells.size() * 8);
    data << GetGUID();
    data << uint8(0x0);

    SpellSet::iterator sitr;
    for (sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        uint32 SpellId = (*sitr);

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(SpellId);

        if (!spellInfo)
        {
            ASSERT(spellInfo);
            continue;
        }

        // Not send cooldown for this spells
        if (spellInfo->getAttributes() & ATTRIBUTES_TRIGGER_COOLDOWN)
            continue;

        if (spellInfo->getSchool() == SpellSchool)
        {
            data << uint32(SpellId);
            data << uint32(unTimeMs);                       // in m.secs
        }
    }
    GetSession()->SendPacket(&data);
}

void Player::ToggleXpGain()
{

    if (m_XpGain)
        m_XpGain = false;
    else
        m_XpGain = true;

}

bool Player::CanGainXp()
{
    return m_XpGain;
}

void Player::RemoveGarbageItems()
{
    std::list< Item* >::iterator itr;

    for (itr = m_GarbageItems.begin(); itr != m_GarbageItems.end(); ++itr)
    {
        Item* it = *itr;

        delete it;
    }

    m_GarbageItems.clear();
}

void Player::AddGarbageItem(Item* it)
{
    m_GarbageItems.push_back(it);
}

void Player::OutPacket(uint16 opcode, uint16 len, const void* data)
{
    ARCEMU_ASSERT(m_session != NULL);
    m_session->OutPacket(opcode, len, data);
}

void Player::SendPacket(WorldPacket* packet)
{
    ARCEMU_ASSERT(m_session != NULL);
    m_session->SendPacket(packet);
}

void Player::OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool self)
{
    if (!IsInWorld())
        return;

    bool gm = m_isGmInvisible;

    if (self)
        OutPacket(Opcode, Len, Data);

    for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Player* p = static_cast< Player* >(*itr);

        if (gm)
        {
            if (p->GetSession()->GetPermissionCount() > 0)
                p->OutPacket(Opcode, Len, Data);
        }
        else
        {
            p->OutPacket(Opcode, Len, Data);
        }
    }
}

void Player::SendMessageToSet(WorldPacket* data, bool bToSelf, bool myteam_only)
{
    if (!IsInWorld())
        return;

    bool gminvis = false;

    if (bToSelf)
    {
        SendPacket(data);
    }

    gminvis = m_isGmInvisible;
    uint32 myphase = GetPhase();

    if (myteam_only)
    {
        uint32 myteam = GetTeam();

        if (data->GetOpcode() != SMSG_MESSAGECHAT)
        {
            for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
            {
                Player* p = static_cast< Player* >(*itr);

                if (gminvis && ((p->GetSession() == nullptr) || (p->GetSession()->GetPermissionCount() <= 0)))
                    continue;

                if (p->GetTeam() == myteam
                    && (p->GetPhase() & myphase) != 0
                    && p->IsVisible(GetGUID()))
                    p->SendPacket(data);
            }
        }
        else
        {
            for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
            {
                Player* p = static_cast< Player* >(*itr);

                if (p->GetSession()
                    && p->GetTeam() == myteam
                    && !p->Social_IsIgnoring(GetLowGUID())
                    && (p->GetPhase() & myphase) != 0)
                    p->SendPacket(data);
            }
        }
    }
    else
    {
        if (data->GetOpcode() != SMSG_MESSAGECHAT)
        {
            for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
            {
                Player* p = static_cast< Player* >(*itr);

                if (gminvis && ((p->GetSession() == nullptr) || (p->GetSession()->GetPermissionCount() <= 0)))
                    continue;

                if ((p->GetPhase() & myphase) != 0
                    && p->IsVisible(GetGUID()))
                    p->SendPacket(data);
            }
        }
        else
        {
            for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
            {
                Player* p = static_cast< Player* >(*itr);

                if (p->GetSession()
                    && !p->Social_IsIgnoring(GetLowGUID())
                    && (p->GetPhase() & myphase) != 0)
                    p->SendPacket(data);
            }
        }
    }
}

uint32 Player::CheckDamageLimits(uint32 dmg, uint32 spellid)
{
    std::stringstream dmglog;

    if ((spellid != 0) && (worldConfig.limit.maxSpellDamageCap > 0))
    {
        if (dmg > worldConfig.limit.maxSpellDamageCap)
        {
            dmglog << "Dealt " << dmg << " with spell " << spellid;

            sCheatLog.writefromsession(m_session, dmglog.str().c_str());

            if (worldConfig.limit.disconnectPlayerForExceedingLimits != 0)
                m_session->Disconnect();

            dmg = worldConfig.limit.maxSpellDamageCap;
        }
    }
    else if ((worldConfig.limit.maxAutoAttackDamageCap > 0) && (dmg > worldConfig.limit.maxAutoAttackDamageCap))
    {
        dmglog << "Dealt " << dmg << " with auto attack";
        sCheatLog.writefromsession(m_session, dmglog.str().c_str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits != 0)
            m_session->Disconnect();

        dmg = worldConfig.limit.maxAutoAttackDamageCap;
    }

    if (worldConfig.limit.broadcastMessageToGmOnExceeding != 0)
        sendReportToGmMessage(GetName(), dmglog.str());

    return dmg;
}

void Player::DealDamage(Unit* pVictim, uint32 damage, uint32 /*targetEvent*/, uint32 /*unitEvent*/, uint32 spellId, bool no_remove_auras)
{
    if (!pVictim || !pVictim->isAlive() || !pVictim->IsInWorld() || !IsInWorld())
        return;
    if (pVictim->IsPlayer() && static_cast< Player* >(pVictim)->GodModeCheat == true)
        return;
    if (pVictim->bInvincible)
        return;
    if (pVictim->IsCreature() && static_cast<Creature*>(pVictim)->isSpiritHealer())
        return;

    if (this != pVictim)
    {
        if (!GetSession()->HasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
            damage = CheckDamageLimits(damage, spellId);

        CombatStatus.OnDamageDealt(pVictim);

        if (pVictim->IsPvPFlagged())
        {
            if (!IsPvPFlagged())
                PvPToggle();

            AggroPvPGuards();
        }
    }

    pVictim->SetStandState(STANDSTATE_STAND);

    if (CombatStatus.IsInCombat())
        sHookInterface.OnEnterCombat(this, this);

    ///////////////////////////////////////////////////// Hackatlon ///////////////////////////////////////////////////////////

    //the black sheep , no actually it is paladin : Ardent Defender
    if (DamageTakenPctModOnHP35 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35))
        damage = damage - float2int32(damage * DamageTakenPctModOnHP35) / 100;

    if (pVictim->IsCreature() && pVictim->IsTaggable())
    {
        pVictim->Tag(GetGUID());
        TagUnit(pVictim);
    }

    // Bg dmg counter
    if (pVictim != this)
    {
        if (m_bg != nullptr && GetMapMgr() == pVictim->GetMapMgr())
        {
            m_bgScore.DamageDone += damage;
            m_bg->UpdatePvPData();
        }
    }

    // Duel
    if (pVictim->IsPlayer() && DuelingWith != nullptr && DuelingWith->GetGUID() == pVictim->GetGUID())
    {
        if (pVictim->GetHealth() <= damage)
        {
            uint32 NewHP = pVictim->GetMaxHealth() / 100;

            if (NewHP < 5)
                NewHP = 5;

            pVictim->SetHealth(NewHP);
            EndDuel(DUEL_WINNER_KNOCKOUT);
            pVictim->Emote(EMOTE_ONESHOT_BEG);
            return;
        }
    }

    if (pVictim->GetHealth() <= damage)
    {

        if (pVictim->isTrainingDummy())
        {
            pVictim->SetHealth(1);
            return;
        }

        if (m_bg != nullptr)
        {
            m_bg->HookOnUnitKill(this, pVictim);

            if (pVictim->IsPlayer())
                m_bg->HookOnPlayerKill(this, static_cast< Player* >(pVictim));
        }

        if (pVictim->IsPlayer())
        {

            Player* playerVictim = static_cast<Player*>(pVictim);
            sHookInterface.OnKillPlayer(this, playerVictim);

            bool setAurastateFlag = false;

            if (getLevel() >= (pVictim->getLevel() - 8) && (GetGUID() != pVictim->GetGUID()))
            {
#if VERSION_STRING > TBC
                GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, GetAreaID(), 1, 0);
                GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL, 1, 0, 0);
#endif
                HonorHandler::OnPlayerKilled(this, playerVictim);
                setAurastateFlag = true;
            }

            if (setAurastateFlag)
            {
                SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_LASTKILLWITHHONOR);

                if (!sEventMgr.HasEvent(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                    sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::EventAurastateExpire, static_cast<uint32>(AURASTATE_FLAG_LASTKILLWITHHONOR), EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

            }
        }
        else
        {
            if (pVictim->IsCreature())
            {
                Reputation_OnKilledUnit(pVictim, false);

#if VERSION_STRING > TBC
                GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, GetMapId(), 0, 0);
#endif

                if (pVictim->getLevel() >= (getLevel() - 8) && (GetGUID() != pVictim->GetGUID()))
                {
                    HandleProc(PROC_ON_GAIN_EXPIERIENCE, this, nullptr);
                    m_procCounter = 0;
                }
            }
        }

        EventAttackStop();

        SendPartyKillLog(pVictim->GetGUID());

        if (pVictim->IsPvPFlagged())
        {
            uint32 team = GetTeam();

            if (team == TEAM_ALLIANCE)
                team = TEAM_HORDE;
            else
                team = TEAM_ALLIANCE;

            auto area = pVictim->GetArea();
            sWorld.sendZoneUnderAttackMessage(area ? area->id : GetZoneId(), static_cast<uint8>(team));
        }

        pVictim->Die(this, damage, spellId);

        ///////////////////////////////////////////////////////////// Loot  //////////////////////////////////////////////////////////////////////////////////////////////

        if (pVictim->isLootable())
        {
            Player* tagger = GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(pVictim->GetTaggerGUID()));

            // Tagger might have left the map so we need to check
            if (tagger != nullptr)
            {
                if (tagger->InGroup())
                    tagger->GetGroup()->SendLootUpdates(pVictim);
                else
                    tagger->SendLootUpdate(pVictim);
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        /////////////////////////////////////////////////////////// Experience /////////////////////////////////////////////////////////////////////////////////////////////

        if (pVictim->GetCreatedByGUID() == 0 && !pVictim->IsPet() && pVictim->IsTagged())
        {
            auto unit_tagger = pVictim->GetMapMgr()->GetUnit(pVictim->GetTaggerGUID());

            if (unit_tagger != nullptr)
            {
                Player* player_tagger = nullptr;

                if (unit_tagger->IsPlayer())
                    player_tagger = static_cast<Player*>(unit_tagger);

                if ((unit_tagger->IsPet() || unit_tagger->IsSummon()) && unit_tagger->GetPlayerOwner())
                    player_tagger = static_cast<Player*>(unit_tagger->GetPlayerOwner());

                if (player_tagger != nullptr)
                {
                    if (player_tagger->InGroup())
                    {
                        player_tagger->GiveGroupXP(pVictim, player_tagger);
                    }
                    else if (IsUnit())
                    {
                        uint32 xp = CalculateXpToGive(pVictim, unit_tagger);

                        if (xp > 0)
                        {
                            player_tagger->GiveXP(xp, pVictim->GetGUID(), true);

                            SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_LASTKILLWITHHONOR);

                            if (!sEventMgr.HasEvent(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                                sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                            else
                                sEventMgr.ModifyEventTimeLeft(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

                            // let's give the pet some experience too
                            if (player_tagger->GetSummon() && player_tagger->GetSummon()->CanGainXP())
                            {
                                xp = CalculateXpToGive(pVictim, player_tagger->GetSummon());

                                if (xp > 0)
                                    player_tagger->GetSummon()->GiveXP(xp);
                            }
                        }
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    if (pVictim->IsCreature())
                    {
                        sQuestMgr.OnPlayerKill(player_tagger, static_cast<Creature*>(pVictim), true);
#if VERSION_STRING > TBC
                        ///////////////////////////////////////////////// Kill creature/creature type Achievements /////////////////////////////////////////////////////////////////////
                        if (player_tagger->InGroup())
                        {
                            auto player_group = player_tagger->GetGroup();

                            player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
                            player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);

                        }
                        else
                        {
                            player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
                            player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);
                        }
#endif
                    }
                }
            }
        }

#if VERSION_STRING > TBC
        if (pVictim->isCritter())
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);
        }
#endif
    }
    else
    {
        pVictim->TakeDamage(this, damage, spellId, no_remove_auras);
    }
}


void Player::TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras)
{

    if (!no_remove_auras)
    {
        //zack 2007 04 24 : root should not remove self (and also other unknown spells)
        if (spellid)
        {
            RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellid);
            if (Rand(35.0f))
                RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_UNUSED2, spellid);
        }
        else
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
            if (Rand(35.0f))
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
        }
    }

    if (CombatStatus.IsInCombat())
        sHookInterface.OnEnterCombat(this, pAttacker);


    // Rage generation on damage
    if (GetPowerType() == POWER_TYPE_RAGE && pAttacker != this)
    {
        float val;
        float level = static_cast<float>(getLevel());
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        val = 2.5f * damage / c;
        uint32 rage = GetPower(POWER_TYPE_RAGE);

        if (rage + float2int32(val) > 1000)
            val = 1000.0f - static_cast<float>(GetPower(POWER_TYPE_RAGE));

        val *= 10.0;
        ModPower(POWER_TYPE_RAGE, static_cast<int32>(val));
    }


    // Cannibalize, when we are hit we need to stop munching that nice fresh corpse
    {
        if (cannibalize)
        {
            sEventMgr.RemoveEvents(this, EVENT_CANNIBALIZE);
            SetEmoteState(EMOTE_ONESHOT_NONE);
            cannibalize = false;
        }
    }

    if (pAttacker != this)
    {

        // Defensive pet
        std::list<Pet*> summons = GetSummons();

        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            Pet* pPet = *itr;

            if (pPet->GetPetState() != PET_STATE_PASSIVE)
            {
                pPet->GetAIInterface()->AttackReaction(pAttacker, 1, 0);
                pPet->HandleAutoCastEvent(AUTOCAST_EVENT_OWNER_ATTACKED);
            }
        }
    }

    ModHealth(-1 * static_cast<int32>(damage));
}

void Player::Die(Unit* pAttacker, uint32 /*damage*/, uint32 spellid)
{
    if (GetVehicleComponent() != nullptr)
    {
        GetVehicleComponent()->RemoveAccessories();
        GetVehicleComponent()->EjectAllPassengers();
    }

#if VERSION_STRING > TBC
    // A Player has died
    if (IsPlayer())
    {
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH, 1, 0, 0);
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP, GetMapId(), 1, 0);

        // A Player killed a Player
        if (pAttacker->IsPlayer())
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER, 1, 0, 0);
        }// A Creature killed a Player
        else if (pAttacker->IsCreature())
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE, 1, 0, 0);
        }
    }
#endif

    //general hook for die
    if (!sHookInterface.OnPreUnitDie(pAttacker, this))
        return;

    // on die and an target die proc
    {
        SpellInfo* killerspell;
        if (spellid)
            killerspell = sSpellCustomizations.GetSpellInfo(spellid);
        else killerspell = nullptr;

        HandleProc(PROC_ON_DIE, this, killerspell);
        m_procCounter = 0;
        pAttacker->HandleProc(PROC_ON_TARGET_DIE, this, killerspell);
        pAttacker->m_procCounter = 0;
    }

    if (!pAttacker->IsPlayer())
        DeathDurabilityLoss(0.10);


    if (GetChannelSpellTargetGUID() != 0)
    {

        Spell* spl = getCurrentSpell(CURRENT_CHANNELED_SPELL);

        if (spl != nullptr)
        {

            for (uint8 i = 0; i < 3; i++)
            {
                if (spl->GetSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64 guid = GetChannelSpellTargetGUID();
                    DynamicObject* dObj = GetMapMgr()->GetDynamicObject(Arcemu::Util::GUID_LOPART(guid));
                    if (!dObj)
                        continue;

                    dObj->Remove();
                }
            }

            if (spl->GetSpellInfo()->getChannelInterruptFlags() == 48140) interruptSpell(spl->GetSpellInfo()->getId());
        }
    }

    // Stop players from casting
    for (std::set< Object* >::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); ++itr)
    {
        Unit* attacker = static_cast< Unit* >(*itr);

        if (attacker->isCastingNonMeleeSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                if (attacker->getCurrentSpell(CurrentSpellType(i)) == nullptr)
                    continue;
                if (attacker->getCurrentSpell(CurrentSpellType(i))->m_targets.m_unitTarget == GetGUID())
                    attacker->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }
    }

    smsg_AttackStop(pAttacker);
    EventAttackStop();

    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnPlayerDeath)(this, pAttacker);

    {
        uint32 self_res_spell = 0;
        if (m_bg == nullptr || (m_bg != nullptr && !isArena(m_bg->GetType())))
        {
            self_res_spell = SoulStone;

            SoulStone = SoulStoneReceiver = 0;

            if (self_res_spell == 0 && bReincarnation)
            {
                SpellInfo* m_reincarnSpellInfo = sSpellCustomizations.GetSpellInfo(20608);
                if (Cooldown_CanCast(m_reincarnSpellInfo))
                {

                    uint32 ankh_count = GetItemInterface()->GetItemCount(17030);
                    if (ankh_count)
                        self_res_spell = 21169;
                }
            }
        }

        setUInt32Value(PLAYER_SELF_RES_SPELL, self_res_spell);
        setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    }

    // Wipe our attacker set on death
    CombatStatus.Vanished();

    CALL_SCRIPT_EVENT(pAttacker, OnTargetDied)(this);
    pAttacker->smsg_AttackStop(this);

    m_UnderwaterTime = 0;
    m_UnderwaterState = 0;

    summonhandler.RemoveAllSummons();
    DismissActivePets();

    SetHealth(0);

    //check for spirit of Redemption
    if (HasSpell(20711))
    {
        SpellInfo* sorInfo = sSpellCustomizations.GetSpellInfo(27827);

        if (sorInfo != nullptr)
        {
            Spell* sor = sSpellFactoryMgr.NewSpell(this, sorInfo, true, nullptr);
            SpellCastTargets targets;
            targets.m_unitTarget = GetGUID();
            sor->prepare(&targets);
        }
    }

    KillPlayer();
    if (m_mapMgr->m_battleground != nullptr)
        m_mapMgr->m_battleground->HookOnUnitDied(this);
}

void Player::HandleKnockback(Object* caster, float horizontal, float vertical)
{
    if (caster == nullptr)
        caster = this;

    float angle = calcRadAngle(caster->GetPositionX(), caster->GetPositionY(), GetPositionX(), GetPositionY());
    if (caster == this)
        angle = float(GetOrientation() + M_PI);

    float sin = sinf(angle);
    float cos = cosf(angle);

#if VERSION_STRING != Cata
    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);

    data << GetNewGUID();
    data << uint32(Util::getMSTime());
    data << float(cos);
    data << float(sin);
    data << float(horizontal);
    data << float(-vertical);

    GetSession()->SendPacket(&data);
#else
    ObjectGuid guid = GetGUID();

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
    data.WriteByteMask(guid[0]);
    data.WriteByteMask(guid[3]);
    data.WriteByteMask(guid[6]);
    data.WriteByteMask(guid[7]);
    data.WriteByteMask(guid[2]);
    data.WriteByteMask(guid[5]);
    data.WriteByteMask(guid[1]);
    data.WriteByteMask(guid[4]);

    data.WriteByteSeq(guid[1]);
    data << float(sin);
    data << uint32(0);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[7]);
    data << float(horizontal);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[3]);
    data << float(-vertical);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[0]);
#endif

    blinked = true;
    SpeedCheatDelay(10000);
}

void Player::RemoveIfVisible(uint64 obj)
{
    std::set< uint64 >::iterator itr = m_visibleObjects.find(obj);
    if (itr == m_visibleObjects.end())
        return;

    m_visibleObjects.erase(obj);
    PushOutOfRange(obj);
}

void Player::Phase(uint8 command, uint32 newphase)
{
    Unit::Phase(command, newphase);
#if VERSION_STRING == WotLK
    if (GetSession())
    {
        WorldPacket data(SMSG_SET_PHASE_SHIFT, 4);
        data << newphase;
        GetSession()->SendPacket(&data);
    }

    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        Pet* p = *itr;
        p->Phase(command, newphase);
    }
    //We should phase other, non-combat "pets" too...

    if (m_CurrentCharm != 0)
    {
        Unit* charm = m_mapMgr->GetUnit(m_CurrentCharm);
        if (charm == NULL)
            return;

        charm->Phase(command, newphase);
    }
#elif VERSION_STRING == Cata
    if (GetSession())
    {
        ObjectGuid guid = GetGUID();
        uint32 phaseFlags = 0;

        for (uint32 i = 0; i < sPhaseStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::PhaseEntry const* phase = sPhaseStore.LookupEntry(i))
            {
                if (phase->PhaseShift == newphase)
                {
                    phaseFlags = phase->Flags;
                    break;
                }
            }
        }

        WorldPacket data(SMSG_SET_PHASE_SHIFT, 30);
        data.WriteByteMask(guid[2]);
        data.WriteByteMask(guid[3]);
        data.WriteByteMask(guid[1]);
        data.WriteByteMask(guid[6]);
        data.WriteByteMask(guid[4]);
        data.WriteByteMask(guid[5]);
        data.WriteByteMask(guid[0]);
        data.WriteByteMask(guid[7]);

        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[4]);
        data << uint32(0);                          //unk
        data.WriteByteSeq(guid[1]);
        data << uint32(newphase ? phaseFlags : 8);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[6]);
        data << uint32(0);                          //unk
        data << uint32(newphase ? 2 : 0);           //unk

        if (newphase)
            data << uint16(newphase);

        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[0]);
        data << uint32(GetMapId() ? 2 : 0);

        data << uint16(GetMapId());

        data.WriteByteSeq(guid[5]);
        GetSession()->SendPacket(&data);
    }

    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        Pet* p = *itr;
        p->Phase(command, newphase);
    }
    //We should phase other, non-combat "pets" too...

    if (m_CurrentCharm != 0)
    {
        Unit* charm = m_mapMgr->GetUnit(m_CurrentCharm);
        if (charm == nullptr)
            return;

        charm->Phase(command, newphase);
    }
#endif
}

///\todo  Use this method all over source code
uint32 Player::GetBlockDamageReduction()
{
    Item* it = this->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it == nullptr || it->GetItemProperties()->InventoryType != INVTYPE_SHIELD)
        return 0;

    float block_multiplier = (100.0f + this->m_modblockabsorbvalue) / 100.0f;
    if (block_multiplier < 1.0f)
        block_multiplier = 1.0f;

    return float2int32((it->GetItemProperties()->Block + this->m_modblockvaluefromspells + this->getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + this->GetStat(STAT_STRENGTH) / 2.0f - 1.0f) * block_multiplier);
}

void Player::ApplyFeralAttackPower(bool apply, Item* item)
{
    float FeralAP = 0.0f;

    Item* it = item;
    if (it == nullptr)
        it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (it != nullptr)
    {
        float delay = (float)it->GetItemProperties()->Delay / 1000.0f;
        delay = std::max(1.0f, delay);
        float dps = ((it->GetItemProperties()->Damage[0].Min + it->GetItemProperties()->Damage[0].Max) / 2) / delay;
        if (dps > 54.8f)
            FeralAP = (dps - 54.8f) * 14;
    }
    ModifyBonuses(FERAL_ATTACK_POWER, (int)FeralAP, apply);
}

void Player::SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay)
{
    if (delay)
    {
        sEventMgr.AddEvent(this, &Player::SendChatMessage, type, lang, msg, uint32(0), EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    WorldPacket* data = sChatHandler.FillMessageData(type, lang, msg, GetGUID());
    SendMessageToSet(data, true);
    delete data;
}

void Player::SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr)
{
    if (plr == nullptr)
        return;
    WorldPacket* data = sChatHandler.FillMessageData(type, lang, msg, GetGUID());
    plr->SendPacket(data);
    delete data;
}

void Player::AcceptQuest(uint64 guid, uint32 quest_id)
{

    bool bValid = false;
    bool hasquest = true;
    bool bSkipLevelCheck = false;
    QuestProperties const* qst = nullptr;
    Object* qst_giver = nullptr;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = m_mapMgr->GetCreature(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        hasquest = quest_giver->HasQuest(quest_id, 1);
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            qst = sMySQLStore.getQuestProperties(quest_id);
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = m_mapMgr->GetGameObject(GET_LOWGUID_PART(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }
    else if (guidtype == HIGHGUID_TYPE_ITEM)
    {
        Item* quest_giver = m_ItemInterface->GetItemByGUID(guid);
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = true;
        bSkipLevelCheck = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }
    else if (guidtype == HIGHGUID_TYPE_PLAYER)
    {
        Player* quest_giver = m_mapMgr->GetPlayer((uint32)guid);
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }

    if (!qst_giver)
    {
        LOG_DEBUG("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == nullptr)
    {
        LOG_DEBUG("WORLD: Creature is not a questgiver.");
        return;
    }

    if (HasQuest(qst->id))
        return;

    if (qst_giver->IsCreature() && static_cast< Creature* >(qst_giver)->m_escorter != nullptr)
    {
        m_session->SystemMessage("You cannot accept this quest at this time.");
        return;
    }

    // Check the player hasn't already taken this quest, or
    // it isn't available.
    uint32 status = sQuestMgr.CalcQuestStatus(qst_giver, this, qst, 3, bSkipLevelCheck);

    if ((!sQuestMgr.IsQuestRepeatable(qst) && HasFinishedQuest(qst->id)) || (status != QMGR_QUEST_AVAILABLE && status != QMGR_QUEST_REPEATABLE && status != QMGR_QUEST_CHAT)
        || !hasquest)
    {
        // We've got a hacker. Disconnect them.
        //sCheatLog.writefromsession(this, "tried to accept incompatible quest %u from %u.", qst->id, qst_giver->GetEntry());
        //Disconnect();
        return;
    }

    uint16 log_slot = GetOpenQuestSlot();
    if (log_slot > MAX_QUEST_SLOT)
    {
        sQuestMgr.SendQuestLogFull(this);
        return;
    }

    if ((qst->time != 0) && HasTimedQuest())
    {
        sQuestMgr.SendQuestInvalid(INVALID_REASON_HAVE_TIMED_QUEST, this);
        return;
    }

    if (qst->count_receiveitems || qst->srcitem)
    {
        uint32 slots_required = qst->count_receiveitems;

        if (m_ItemInterface->CalculateFreeSlots(nullptr) < slots_required)
        {
            m_ItemInterface->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
            sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, this);
            return;
        }
    }

    QuestLogEntry* qle = new QuestLogEntry();
    qle->Init(qst, this, log_slot);
    qle->UpdatePlayerFields();

    // If the quest should give any items on begin, give them the items.
    for (uint8 i = 0; i < 4; ++i)
    {
        if (qst->receive_items[i])
        {
            Item* item = objmgr.CreateItem(qst->receive_items[i], this);
            if (item == nullptr)
                continue;
            if (!m_ItemInterface->AddItemToFreeSlot(item))
            {
                item->DeleteMe();
            }
            else
                SendItemPushResult(false, true, false, true,
                m_ItemInterface->LastSearchItemBagSlot(), m_ItemInterface->LastSearchItemSlot(),
                1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
        }
    }

    if (qst->srcitem && qst->srcitem != qst->receive_items[0])
    {
        if (!qst_giver->IsItem() || (qst_giver->GetEntry() != qst->srcitem))
        {
            Item *item = objmgr.CreateItem(qst->srcitem, this);
            if (item != nullptr)
            {
                item->SetStackCount(qst->srcitemcount ? qst->srcitemcount : 1);
                if (!m_ItemInterface->AddItemToFreeSlot(item))
                    item->DeleteMe();
            }
        }
    }

    if (qst->count_required_item || qst_giver->IsGameObject())    // gameobject quests deactivate
        UpdateNearbyGameObjects();

    // Some spells applied at quest activation
    SpellAreaForQuestMapBounds saBounds = sSpellFactoryMgr.GetSpellAreaForQuestMapBounds(quest_id, true);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second->autocast && itr->second->IsFitToRequirements(this, GetZoneId(), GetAreaID()))
                if (!HasAura(itr->second->spellId))
                    CastSpell(this, itr->second->spellId, true);
        }
    }

    sQuestMgr.OnQuestAccepted(this, qst, qst_giver);

    LOG_DEBUG("WORLD: Added new QLE.");
    sHookInterface.OnQuestAccept(this, qst, qst_giver);
}

bool Player::LoadReputations(QueryResult *result)
{
    if (result == nullptr)
        return false;

    DBC::Structures::FactionEntry const* faction = nullptr;
    FactionReputation *reputation = nullptr;

    uint32 id = 0;
    uint32 flag = 0;
    int32 basestanding = 0;
    int32 standing = 0;

    do {
        Field* field = result->Fetch();

        id = field[0].GetUInt32();
        flag = field[1].GetUInt32();
        basestanding = field[2].GetInt32();
        standing = field[3].GetInt32();

        faction = sFactionStore.LookupEntry(id);
        if ((faction == nullptr) || (faction->RepListId < 0))
            continue;

        ReputationMap::iterator itr = m_reputation.find(id);
        if (itr != m_reputation.end())
            delete itr->second;

        reputation = new FactionReputation;
        reputation->baseStanding = basestanding;
        reputation->standing = standing;
        reputation->flag = static_cast<uint8>(flag);
        m_reputation[id] = reputation;
        reputationByListId[faction->RepListId] = reputation;

    } while (result->NextRow());

    if (m_reputation.size() == 0)
        _InitialReputation();

    return true;
}

bool Player::SaveReputations(bool NewCharacter, QueryBuffer *buf)
{
    if (!NewCharacter && (buf == nullptr))
        return false;

    std::stringstream ds;
    uint32 guid = GetLowGUID();

    ds << "DELETE FROM playerreputations WHERE guid = '";
    ds << guid;
    ds << "';";

    if (!NewCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());


    for (ReputationMap::iterator itr = m_reputation.begin(); itr != m_reputation.end(); ++itr)
    {
        std::stringstream ss;

        ss << "INSERT INTO playerreputations VALUES('";
        ss << GetLowGUID() << "','";
        ss << itr->first << "','";
        ss << uint32(itr->second->flag) << "','";
        ss << itr->second->baseStanding << "','";
        ss << itr->second->standing << "');";

        if (!NewCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::LoadSpells(QueryResult* result)
{
    if (result == nullptr)
        return false;

    Field* fields = nullptr;

    do
    {
        fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();

        SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);
        if (sp != nullptr)
            mSpells.insert(spellid);

    }
    while (result->NextRow());

    return true;
}

bool Player::SaveSpells(bool NewCharacter, QueryBuffer* buf)
{
    if (!NewCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32 guid = GetLowGUID();

    ds << "DELETE FROM playerspells WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!NewCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        uint32 spellid = *itr;

        std::stringstream ss;

        ss << "INSERT INTO playerspells VALUES('";
        ss << guid << "','";
        ss << spellid << "');";

        if (!NewCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::LoadDeletedSpells(QueryResult* result)
{
    if (result == nullptr)
        return false;

    Field* fields = nullptr;

    do
    {
        fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();

        SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);
        if (sp != nullptr)
            mDeletedSpells.insert(spellid);

    }
    while (result->NextRow());

    return true;
}

bool Player::SaveDeletedSpells(bool NewCharacter, QueryBuffer* buf)
{
    if (!NewCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32 guid = GetLowGUID();

    ds << "DELETE FROM playerdeletedspells WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!NewCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (SpellSet::iterator itr = mDeletedSpells.begin(); itr != mDeletedSpells.end(); ++itr)
    {
        uint32 spellid = *itr;

        std::stringstream ss;

        ss << "INSERT INTO playerdeletedspells VALUES('";
        ss << guid << "','";
        ss << spellid << "');";

        if (!NewCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::LoadSkills(QueryResult* result)
{
    if (result == nullptr)
        return false;

    Field* fields = nullptr;

    do
    {
        fields = result->Fetch();

        uint32 skillid = fields[0].GetUInt32();
        uint32 currval = fields[1].GetUInt32();
        uint32 maxval = fields[2].GetUInt32();

        PlayerSkill sk;

        sk.Reset(skillid);
        sk.CurrentValue = currval;
        sk.MaximumValue = maxval;

        if (sk.CurrentValue == 0)
            sk.CurrentValue = 1;

        m_skills.insert(std::pair< uint32, PlayerSkill >(skillid, sk));

    }
    while (result->NextRow());

    return true;
}

bool Player::SaveSkills(bool NewCharacter, QueryBuffer* buf)
{
    if (!NewCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32 guid = GetLowGUID();

    ds << "DELETE FROM playerskills WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!NewCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
#if VERSION_STRING != Cata
        if (itr->second.Skill->type == SKILL_TYPE_LANGUAGE)
            continue;
#endif
        uint32 skillid = itr->first;
        uint32 currval = itr->second.CurrentValue;
        uint32 maxval = itr->second.MaximumValue;

        std::stringstream ss;

        ss << "INSERT INTO playerskills VALUES('";
        ss << guid << "','";
        ss << skillid << "','";
        ss << currval << "','";
        ss << maxval << "');";

        if (!NewCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

void Player::AddQuestKill(uint32 questid, uint8 reqid, uint32 delay)
{
    if (!HasQuest(questid))
        return;

    if (delay)
    {
        sEventMgr.AddEvent(this, &Player::AddQuestKill, questid, reqid, uint32(0), EVENT_PLAYER_UPDATE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    QuestLogEntry* quest_entry = GetQuestLogForEntry(questid);
    if (quest_entry == nullptr)
        return;

    QuestProperties const* quest = quest_entry->GetQuest();

    if (quest_entry->GetMobCount(reqid) >= quest->required_mob_or_go_count[reqid])
        return;

    quest_entry->IncrementMobCount(reqid);
    quest_entry->SendUpdateAddKill(reqid);
    quest_entry->UpdatePlayerFields();

    if (quest_entry->CanBeFinished())
        quest_entry->SendQuestComplete();
}

bool Player::CanBuyAt(MySQLStructure::VendorRestrictions const* vendor)
{
    if (vendor == nullptr)
        return true;

    if (vendor->flags == RESTRICTION_CHECK_ALL)
    {
        // check for race mask
        if ((vendor->racemask > 0) && !(getRaceMask() & vendor->racemask))
            return false;

        // check for class mask
        if ((vendor->classmask > 0) && !(getClassMask() & vendor->classmask))
            return false;

        // check for required reputation
        if (vendor->reqrepfaction)
        {
            uint32 plrep = GetStanding(vendor->reqrepfaction);
            if (plrep < vendor->reqrepvalue)
                return false;
        }
    }
    else if (vendor->flags == RESTRICTION_CHECK_MOUNT_VENDOR)
    {
        if ((vendor->racemask > 0) && (vendor->reqrepfaction))
        {
            uint32 plrep = GetStanding(vendor->reqrepfaction);
            if (!(getRaceMask() & vendor->racemask) && (plrep < vendor->reqrepvalue))
                return false;
        }
        else
        {
            LOG_ERROR("VendorRestrictions: Mount vendor specified, but not enough info for creature %u", vendor->entry);
        }
    }

    return true;
}

bool Player::CanTrainAt(Trainer* trn)
{
    if ((trn->RequiredClass && this->getClass() != trn->RequiredClass) ||
        ((trn->RequiredRace && this->getRace() != trn->RequiredRace) && ((trn->RequiredRepFaction && trn->RequiredRepValue) && this->GetStanding(trn->RequiredRepFaction) != static_cast<int32>(trn->RequiredRepValue))) ||
        (trn->RequiredSkill && !this->_HasSkillLine(trn->RequiredSkill)) ||
        (trn->RequiredSkillLine && this->_GetSkillLineCurrent(trn->RequiredSkill) < trn->RequiredSkillLine))
    {
        return false;
    }

    return true;
}

void Player::SendEmptyPetSpellList()
{
    WorldPacket data(SMSG_PET_SPELLS, 8);
    data << uint64(0);
    m_session->SendPacket(&data);
}

void Player::BuildPetSpellList(WorldPacket & data)
{
    data << uint64(0);
}

void Player::AddVehicleComponent(uint32 creature_entry, uint32 vehicleid)
{
    if (mountvehicleid == 0)
    {
        LOG_ERROR("Tried to add a vehicle component with 0 as vehicle id for player %u (%s)", GetLowGUID(), GetName());
        return;
    }

    if (vehicle != nullptr)
    {
        LOG_ERROR("Tried to add a vehicle component, but there's already one for player %u (%s)", GetLowGUID(), GetName());
        return;
    }

    vehicle = new Vehicle();
    vehicle->Load(this, creature_entry, vehicleid);
}

void Player::RemoveVehicleComponent()
{
    delete vehicle;
    vehicle = nullptr;
}

void Player::Gossip_SendSQLPOI(uint32 id)
{
    MySQLStructure::PointsOfInterest const* pPOI = sMySQLStore.getPointOfInterest(id);
    if (pPOI != nullptr)
        Gossip_SendPOI(pPOI->x, pPOI->y, pPOI->icon, pPOI->flags, pPOI->data, pPOI->iconName.c_str());
}

void Player::ResetTimeSync()
{
    m_timeSyncCounter = 0;
    m_timeSyncTimer = 0;
    m_timeSyncClient = 0;
    m_timeSyncServer = Util::getMSTime();
}

void Player::SendTimeSync()
{
    WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
    data << uint32(m_timeSyncCounter++);
    GetSession()->SendPacket(&data);

    // Schedule next sync in 10 sec
    m_timeSyncTimer = 10000;
    m_timeSyncServer = Util::getMSTime();
}

void Player::SetBattlegroundEntryPoint()
{
    m_bgEntryPointInstance = GetInstanceID();
    m_bgEntryPointMap = GetMapId();
    m_bgEntryPointX = GetPositionX();
    m_bgEntryPointY = GetPositionY();
    m_bgEntryPointZ = GetPositionZ();
    m_bgEntryPointO = GetOrientation();
}

void Player::SendTeleportPacket(float x, float y, float z, float o)
{
#if VERSION_STRING != Cata
    WorldPacket data2(MSG_MOVE_TELEPORT, 38);
    data2.append(GetNewGUID());
    BuildMovementPacket(&data2, x, y, z, o);
    SendMessageToSet(&data2, false);
#else
    LocationVector oldPos = LocationVector(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    LocationVector pos = LocationVector(x, y, z, o);

    if (GetTypeId() == TYPEID_UNIT)
        SetPosition(pos);

    ObjectGuid guid = GetGUID();

    WorldPacket data(SMSG_MOVE_UPDATE_TELEPORT, 38);
    movement_info.writeMovementInfo(data, SMSG_MOVE_UPDATE_TELEPORT);

    if (GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data2(MSG_MOVE_TELEPORT, 38);
        data2.writeBit(guid[6]);
        data2.writeBit(guid[0]);
        data2.writeBit(guid[3]);
        data2.writeBit(guid[2]);
        data2.writeBit(0); // unk
        //\TODO add transport
        data2.writeBit(uint64(0)); // transport guid
        data2.writeBit(guid[1]);

        data2.writeBit(guid[4]);
        data2.writeBit(guid[7]);
        data2.writeBit(guid[5]);
        data2.flushBits();


        data2 << uint32_t(0); // unk
        data2.WriteByteSeq(guid[1]);
        data2.WriteByteSeq(guid[2]);
        data2.WriteByteSeq(guid[3]);
        data2.WriteByteSeq(guid[5]);
        data2 << float(GetPositionX());
        data2.WriteByteSeq(guid[4]);
        data2 << float(GetOrientation());
        data2.WriteByteSeq(guid[7]);
        data2 << float(GetPositionZ());
        data2.WriteByteSeq(guid[0]);
        data2.WriteByteSeq(guid[6]);
        data2 << float(GetPositionY());
        SendPacket(&data2);
    }

    if (GetTypeId() == TYPEID_PLAYER)
        SetPosition(pos);
    else
        SetPosition(oldPos);

    SendMessageToSet(&data, false);
#endif
}

void Player::SendTeleportAckPacket(float x, float y, float z, float o)
{
    SetPlayerStatus(TRANSFER_PENDING);
    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetNewGUID();
    data << uint32(0);                                     // this value increments every time
    BuildMovementPacket(&data, x, y, z, o);
    GetSession()->SendPacket(&data);
}

void Player::CastSpellArea()
{
    if (!IsInWorld())
        return;

    if (m_position.x > _maxX || m_position.x < _minX || m_position.y > _maxY || m_position.y < _minY)
        return;

    if (GetMapMgr()->GetCellByCoords(GetPositionX(), GetPositionY()) == nullptr)
        return;

    auto at = GetMapMgr()->GetArea(GetPositionX(), GetPositionY(), GetPositionZ());
    if (at == nullptr)
        return;

    uint32 AreaId = at->id;
    uint32 ZoneId = at->zone;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Cheks for Casting a Spell in Specified Area / Zone :D										  //
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // Spells get Casted in specified Area
    SpellAreaForAreaMapBounds saBounds = sSpellFactoryMgr.GetSpellAreaForAreaMapBounds(AreaId);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        if (itr->second->autocast && itr->second->IsFitToRequirements(this, ZoneId, AreaId))
        {
            if (!HasAura(itr->second->spellId))
                CastSpell(this, itr->second->spellId, true);
        }


    // Some spells applied at enter into zone (with subzones)
    SpellAreaForAreaMapBounds szBounds = sSpellFactoryMgr.GetSpellAreaForAreaMapBounds(ZoneId);
    for (SpellAreaForAreaMap::const_iterator itr = szBounds.first; itr != szBounds.second; ++itr)
        if (itr->second->autocast && itr->second->IsFitToRequirements(this, ZoneId, AreaId))
            if (!HasAura(itr->second->spellId))
                CastSpell(this, itr->second->spellId, true);


    //Remove of Spells
    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != nullptr)
        {
            if (m_auras[i]->GetSpellInfo()->CheckLocation(GetMapId(), ZoneId, AreaId, this) == false)
            {
                SpellAreaMapBounds sab = sSpellFactoryMgr.GetSpellAreaMapBounds(m_auras[i]->GetSpellId());
                if (sab.first != sab.second)
                    RemoveAura(m_auras[i]->GetSpellId());
            }
        }
    }
}

void Player::SetGroupUpdateFlags(uint32 flags)
{
	if(GetGroup() == nullptr)
		return;
	GroupUpdateFlags = flags;
}

void Player::AddGroupUpdateFlag(uint32 flag)
{
	if(GetGroup() == nullptr)
		return;
	GroupUpdateFlags |= flag;
}

uint16 Player::GetGroupStatus()
{
    uint16 status = MEMBER_STATUS_ONLINE;
    if (IsPvPFlagged())
        status |= MEMBER_STATUS_PVP;
    if (getDeathState() == CORPSE)
        status |= MEMBER_STATUS_DEAD;
    else if (IsDead())
        status |= MEMBER_STATUS_GHOST;
    if (IsFFAPvPFlagged())
        status |= MEMBER_STATUS_PVP_FFA;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
        status |= MEMBER_STATUS_AFK;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
        status |= MEMBER_STATUS_DND;
    return status;
}

void Player::SendUpdateToOutOfRangeGroupMembers()
{
    if (GroupUpdateFlags == GROUP_UPDATE_FLAG_NONE)
        return;
    if (Group* group = GetGroup())
		group->UpdateOutOfRangePlayer(this, true, nullptr);

    GroupUpdateFlags = GROUP_UPDATE_FLAG_NONE;
	if (Pet* pet = GetSummon())
		pet->ResetAuraUpdateMaskForRaid();
}

void Player::SendGuildMOTD()
{
    if (!GetGuild())
        return;

    WorldPacket data(SMSG_GUILD_EVENT, 50);
#if VERSION_STRING != Cata
    data << uint8(GE_MOTD);
#else
    data << uint8(255);
#endif
    data << uint8(1);
    data << GetGuild()->getMOTD();
    SendPacket(&data);
}

void Player::SetClientControl(Unit* target, uint8 allowMove)
{
    WorldPacket ack(SMSG_CLIENT_CONTROL_UPDATE, 200);
    ack << target->GetNewGUID();
    ack << uint8(allowMove);
    SendPacket(&ack);

    if (target == this)
        SetMover(this);
}

void Player::SendCinematicCamera(uint32 id)
{
    camControle = true;
    GetMapMgr()->ChangeObjectLocation(this);
    SetPosition(float(GetPositionX() + 0.01), float(GetPositionY() + 0.01), float(GetPositionZ() + 0.01), GetOrientation());
    GetSession()->OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &id);
}

uint32 Player::GetTeamInitial()
{
    return myRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE;
}

void Player::ResetTeam()
{
    m_team = myRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE; m_bgTeam = m_team;
}

bool Player::IsBanned()
{
    if (m_banned)
    {
        if (m_banned < 100 || (uint32)UNIXTIME < m_banned)
            return true;
    }
    return false;
}

bool Player::IsGroupLeader()
{
    if (m_playerInfo->m_Group != nullptr)
    {
        if (m_playerInfo->m_Group->GetLeader() == m_playerInfo)
            return true;
    }
    return false;
}

void Player::RemoveSummon(Pet* pet)
{
    for (std::list<Pet*>::iterator itr = m_Summons.begin(); itr != m_Summons.end(); ++itr)
    {
        if ((*itr)->GetGUID() == pet->GetGUID())
        {
            m_Summons.erase(itr);
            break;
        }
    }
}

uint32 Player::GetUnstabledPetNumber(void)
{
    if (m_Pets.size() == 0)
        return 0;

    std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin();
    for (; itr != m_Pets.end(); ++itr)
        if (itr->second->stablestate == STABLE_STATE_ACTIVE)
            return itr->first;
    return 0;
}

void Player::SendDelayedPacket(WorldPacket* data, bool bDeleteOnSend)
{
    if (data == nullptr)
        return;

    if (GetSession() != nullptr)
        GetSession()->SendPacket(data);

    if (bDeleteOnSend)
        delete data;
}

GameObject* Player::GetSelectedGo()
{
    if (m_GM_SelectedGO)
        return GetMapMgr()->GetGameObject((uint32)m_GM_SelectedGO);

    return nullptr;
}

void Player::SendMountResult(uint32 result)
{
    WorldPacket data(SMSG_MOUNTRESULT, 4);
    data << (uint32)result;
    GetSession()->SendPacket(&data);
}

bool Player::IsMounted()
{
    if (m_MountSpellId != 0)
        return true;
    else
        return false;
}

void Player::SendDismountResult(uint32 result)
{
    WorldPacket data(SMSG_DISMOUNTRESULT, 4);
    data << (uint32)result;
    GetSession()->SendPacket(&data);
}

#if VERSION_STRING != Cata
Player* Player::GetTradeTarget()
{
    if (!IsInWorld())
        return 0;
    return m_mapMgr->GetPlayer((uint32)mTradeTarget);
}
#endif

void Player::RemoteRevive()
{
    ResurrectPlayer();
    setMoveRoot(false);
    setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN, true));
    setSpeedForType(TYPE_SWIM, getSpeedForType(TYPE_SWIM, true));
    setMoveLandWalk();
    SetHealth(getUInt32Value(UNIT_FIELD_MAXHEALTH));
}

void Player::SetMover(Unit* target)
{
    GetSession()->m_MoverWoWGuid.Init(target->GetGUID());
}
