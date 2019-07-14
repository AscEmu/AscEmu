/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Server/CharacterErrors.h"
#include "VMapFactory.h"
#include "Management/HonorHandler.h"
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
#include "Spell/SpellMgr.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/SmsgInitialSpells.h"
#include "Data/WoWPlayer.h"
#include "Data/WoWItem.h"
#include "Data/WoWContainer.h"
#include "Data/WoWGameObject.h"
#include "Data/WoWDynamicObject.h"
#include "Data/WoWCorpse.h"
#include <limits>
#include "Server/Packets/SmsgNewWorld.h"
#include "Server/Packets/SmsgFriendStatus.h"
#include "Management/GuildMgr.h"
#include "Server/Packets/SmsgDeathReleaseLoc.h"
#include "Server/Packets/SmsgCorpseReclaimDelay.h"
#include "Server/Packets/SmsgDuelWinner.h"
#include "Server/Packets/SmsgStopMirrorTimer.h"
#include "Server/Packets/SmsgSummonRequest.h"
#include "Server/Packets/SmsgTitleEarned.h"
#include "Server/Packets/SmsgSetPctSpellModifier.h"
#include "Server/Packets/SmsgSetFlatSpellModifier.h"
#include "Server/Packets/SmsgPowerUpdate.h"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Server/Packets/SmsgAreaTriggerMessage.h"

using namespace AscEmu::Packets;

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

    if (map->GetPlayer(this->getGuidLow()))
    {
        this->SetPosition(vec);
    }
    else
    {
        if (map->GetMapId() == 530 && !this->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_01))
            return false;

        if (map->GetMapId() == 571 && !this->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
            return false;

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
    DetectedRange(0),
    PctIgnoreRegenModifier(0.0f),
    m_retainedrage(0),
    SoulStone(0),
    SoulStoneReceiver(0),
    misdirectionTarget(0),
    bReincarnation(false),
    m_GM_SelectedGO(0),
    m_ShapeShifted(0),
    m_MountSpellId(0),
    bHasBindDialogOpen(false),
    TrackingSpell(0),
    m_CurrentCharm(0),
    // gm stuff
    //m_invincible(false),
    m_cheats { false },
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
    //Trade
#if VERSION_STRING >= Cata
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
    m_GuildIdInvited(0),
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
    myCorpseLocation(),
    max_level(60),
    m_updateMgr(this, (size_t)worldConfig.server.compressionThreshold, 40000, 30000, 1000),
    m_splineMgr()
#ifdef FT_ACHIEVEMENTS
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

    //\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;

    memset(m_uint32Values, 0, (PLAYER_END)* sizeof(uint32));
    m_updateMask.SetCount(PLAYER_END);

    setObjectType(TYPEID_PLAYER);
    setGuidLow(guid);
    m_wowGuid.Init(getGuid());
#if VERSION_STRING >= TBC
    addUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

#if VERSION_STRING >= WotLK
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
#if VERSION_STRING < Cata
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

    mControledUnit = this;
    mPlayerControler = this;
    for (i = 0; i < MAX_QUEST_SLOT; ++i)
        m_questlog[i] = nullptr;

    m_itemInterface = new ItemInterface(this);

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
    m_canDualWield2H = false;
    iInstanceType = 0;
    m_RaidDifficulty = 0;
    m_XpGain = true;
    resettalents = false;
    memset(reputationByListId, 0, sizeof(FactionReputation*) * 128);

    m_comboTarget = 0;
    m_comboPoints = 0;

    setAttackPowerMultiplier(0.f);
    setRangedAttackPowerMultiplier(0.f);

    m_resist_critical[0] = m_resist_critical[1] = 0;

    for (i = 0; i < 3; i++)
        m_attack_speed[i] = 1.0f;

    for (i = 0; i < SCHOOL_COUNT; i++)
        m_resist_hit_spell[i] = 0;

    m_resist_hit[MOD_MELEE] = 0.0f;
    m_resist_hit[MOD_RANGED] = 0.0f;

    m_maxTalentPoints = 0; //VLack: 3 Aspire values initialized
    m_talentActiveSpec = 0;
    m_talentSpecsCount = 1;
    m_talentPointsFromQuests = 0;
#if VERSION_STRING >= Cata
    m_FirstTalentTreeLock = 0;
#endif

#ifdef FT_DUAL_SPEC
    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        m_specs[s].talents.clear();
        memset(m_specs[s].glyphs, 0, GLYPHS_COUNT * sizeof(uint16));
        memset(m_specs[s].mActions, 0, PLAYER_ACTION_BUTTON_SIZE);
    }
#else
    m_spec.talents.clear();
    memset(m_spec.mActions, 0, PLAYER_ACTION_BUTTON_SIZE);
#endif

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
    m_sentTeleportPosition.ChangeCoords({ 999999.0f, 999999.0f, 999999.0f });
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

    m_lastPotionId = 0;
    for (i = 0; i < NUM_COOLDOWN_TYPES; i++)
        m_cooldownMap[i].clear();

    //    m_achievement_points = 0;

    ChampioningFactionID = 0;
    mountvehicleid = 0;

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

    m_FirstCastAutoRepeat = false;
}

void Player::OnLogin()
{}

Player::~Player()
{
    objmgr.RemovePlayerCache(getGuidLow());
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
#if VERSION_STRING < Cata
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
#if VERSION_STRING < Cata
    mTradeTarget = 0;
#endif

    if (DuelingWith != nullptr)
        DuelingWith->DuelingWith = nullptr;
    DuelingWith = nullptr;

    ARCEMU_ASSERT(!IsInWorld());

    // delete m_talenttree

    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (m_questlog[i] != nullptr)
        {
            delete m_questlog[i];
            m_questlog[i] = nullptr;
        }
    }

    delete m_itemInterface;
    m_itemInterface = nullptr;

    for (ReputationMap::iterator itr = m_reputation.begin(); itr != m_reputation.end(); ++itr)
        delete itr->second;

    m_reputation.clear();

    if (m_playerInfo)
        m_playerInfo->m_loggedInPlayer = nullptr;

    delete SDetector;
    SDetector = nullptr;

    /*std::map<uint32,AchievementVal*>::iterator itr;
    for (itr=m_achievements.begin();itr!=m_achievements.end();itr++)
    delete itr->second;*/

    for (std::map< uint32, PlayerPet* >::iterator itr = m_Pets.begin(); itr != m_Pets.end(); ++itr)
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
        case SKILL_LANG_ORCISH:
            return 669;
        case SKILL_LANG_TAURAHE:
            return 670;
        case SKILL_LANG_DARNASSIAN:
            return 671;
        case SKILL_LANG_DWARVEN:
            return 672;
        case SKILL_LANG_THALASSIAN:
            return 813;
        case SKILL_LANG_DRACONIC:
            return 814;
        case SKILL_LANG_DEMON_TONGUE:
            return 815;
        case SKILL_LANG_TITAN:
            return 816;
        case SKILL_LANG_OLD_TONGUE:
            return 817;
        case SKILL_LANG_GNOMISH:
            return 7430;
        case SKILL_LANG_TROLL:
            return 7341;
        case SKILL_LANG_GUTTERSPEAK:
            return 17737;
        case SKILL_LANG_DRAENEI:
            return 29932;
#if VERSION_STRING >= Cata
        case SKILL_LANG_GOBLIN:
            return 69269;
        case SKILL_LANG_GILNEAN:
            return 69270;
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
#if VERSION_STRING < Cata
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
#if VERSION_STRING > Classic
        case RACE_DRAENEI:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_DRAENEI));
            break;
#endif
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
#if VERSION_STRING > Classic
        case RACE_BLOODELF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", (uint32)GUID, GetSpellForLanguage(SKILL_LANG_THALASSIAN));
            break;
#endif
#if VERSION_STRING >= Cata
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
// Create data from client to create a new character
// \param p_newChar
//////////////////////////////////////////////////////////////////////////////////////////
bool Player::Create(CharCreate& charCreateContent)
{
    m_name = charCreateContent.name;
    Util::CapitalizeString(m_name);

    info = sMySQLStore.getPlayerCreateInfo(charCreateContent._race, charCreateContent._class);
    if (info == nullptr)
    {
        // info not found... disconnect
        //sCheatLog.writefromsession(m_session, "tried to create invalid player with race %u and class %u", race, class_);
        m_session->Disconnect();
#if VERSION_STRING > TBC
        if (charCreateContent._class == DEATHKNIGHT)
            LOG_ERROR("Account Name: %s tried to create a deathknight, however your playercreateinfo table does not support this class, please update your database.", m_session->GetAccountName().c_str());
        else
#endif
            LOG_ERROR("Account Name: %s tried to create an invalid character with race %u and class %u, if this is intended please update your playercreateinfo table inside your database.", m_session->GetAccountName().c_str(), charCreateContent._race, charCreateContent._class);
        return false;
    }

    // check that the account creates only new ones with available races, if we're making some
#if VERSION_STRING > Classic
    if (charCreateContent._race >= RACE_BLOODELF && !(m_session->_accountFlags & ACCOUNT_FLAG_XPACK_01))
#else
    if (charCreateContent._race >= RACE_TROLL)
#endif
    {
        m_session->Disconnect();
        return false;
    }

#if VERSION_STRING > TBC
    // check that the account can create deathknights, if we're making one
    if (charCreateContent._class == DEATHKNIGHT && !(m_session->_accountFlags & ACCOUNT_FLAG_XPACK_02))
    {
        LOG_ERROR("Account %s tried to create a DeathKnight, but Account flag is %u!", m_session->GetAccountName().c_str(), m_session->_accountFlags);
        m_session->Disconnect();
        return false;
    }
#endif

    m_mapId = info->mapId;
    SetZoneId(info->zoneId);
    m_position.ChangeCoords({ info->positionX, info->positionY, info->positionZ, info->orientation });
    m_bind_pos_x = info->positionX;
    m_bind_pos_y = info->positionY;
    m_bind_pos_z = info->positionZ;
    m_bind_mapid = info->mapId;
    m_bind_zoneid = info->zoneId;
    m_isResting = 0;
    m_restAmount = 0;
    m_restState = 0;

    // set race dbc
    myRace = sChrRacesStore.LookupEntry(charCreateContent._race);
    myClass = sChrClassesStore.LookupEntry(charCreateContent._class);
    if (!myRace || !myClass)
    {
        // information not found
        sCheatLog.writefromsession(m_session, "tried to create invalid player with race %u and class %u, dbc info not found", charCreateContent._race, charCreateContent._class);
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
    //setScale( ((race==RACE_TAUREN)?1.3f:1.0f));
    setScale(1.0f);
    setHealth(info->health);
    switch (charCreateContent._class)
    {
    case WARRIOR:
        setPower(POWER_TYPE_RAGE, 0);
        setMaxPower(POWER_TYPE_RAGE, info->rage);
        break;
    case ROGUE:
        setPower(POWER_TYPE_ENERGY, info->energy);
        setMaxPower(POWER_TYPE_ENERGY, info->energy);
        break;
#if VERSION_STRING >= WotLK
    case DEATHKNIGHT:
        setPower(POWER_TYPE_RUNES, 8);
        setMaxPower(POWER_TYPE_RUNES, 8);
        setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
        break;
#endif
    default:
        setPower(POWER_TYPE_MANA, info->mana);
        setMaxPower(POWER_TYPE_MANA, info->mana);
        setBaseMana(info->mana);
        if (info->focus)
        {
            setPower(POWER_TYPE_FOCUS, info->focus);
            setMaxPower(POWER_TYPE_FOCUS, info->focus);
        }
        break;
    }

    setMaxHealth(info->health);

#if VERSION_STRING == WotLK

#endif

    //THIS IS NEEDED
    setBaseHealth(info->health);

    if (const auto raceEntry = sChrRacesStore.LookupEntry(charCreateContent._race))
        SetFaction(raceEntry->faction_id);
    else
        SetFaction(0);

#if VERSION_STRING > TBC
    if (charCreateContent._class != DEATHKNIGHT || worldConfig.player.playerStartingLevel > 55)
#endif
    {
        setLevel(worldConfig.player.playerStartingLevel);
    }
#if VERSION_STRING > TBC
    else
    {
        setLevel(55);
    }
#endif

#if VERSION_STRING > TBC
    UpdateGlyphs();
#endif

    setFreePrimaryProfessionPoints(worldConfig.player.maxProfessions);

    setRace(charCreateContent._race);
    setClass(charCreateContent._class);
    setGender(charCreateContent.gender);
    setPowerType(powertype);

#if VERSION_STRING < Cata
    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_PVP);
#else
    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_PVP);
    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    addUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

    if (charCreateContent._class == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

#if VERSION_STRING < Cata
    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
#endif

    setStat(STAT_STRENGTH, info->strength);
    setStat(STAT_AGILITY, info->ability);
    setStat(STAT_STAMINA, info->stamina);
    setStat(STAT_INTELLECT, info->intellect);
    setStat(STAT_SPIRIT, info->spirit);
    setBoundingRadius(0.388999998569489f);
    setCombatReach(1.5f);

    setInitialDisplayIds(charCreateContent.gender, charCreateContent._race);

    EventModelChange();
    //setMinDamage(info->mindmg);
    //setMaxDamage(info->maxdmg);
    setAttackPower(info->attackpower);

    // PLAYER_BYTES
    setSkinColor(charCreateContent.skin);
    setFace(charCreateContent.face);
    setHairStyle(charCreateContent.hairStyle);
    setHairColor(charCreateContent.hairColor);

    // PLAYER_BYTES_2
    setFacialFeatures(charCreateContent.facialHair);
    //SetByte(PLAYER_BYTES_2, 1, 0);  // unknown
    //SetByte(PLAYER_BYTES_2, 2, 0);  // bank_slots
    setRestState(RESTSTATE_NORMAL);

    // PLAYER_BYTES_3
    setPlayerGender(charCreateContent.gender);
    //setByteValue(PLAYER_BYTES_3, 1, 0);  // drunkvalue?
    //setByteValue(PLAYER_BYTES_3, 2, 0);  // unknown
    setPvpRank(getPvpRank());   //useless

    setNextLevelXp(400);
    setPlayerFieldBytes(0x08);
    setModCastSpeed(1.0f);
    setMaxLevel(worldConfig.player.playerLevelCap);

    // Gold Starting Amount
    setCoinage(worldConfig.player.startGoldAmount);

    for (uint8_t x = 0; x < SCHOOL_COUNT; ++x)
        setModDamageDonePct(1.00f, x);

    //\todo not sure if this is what we want.
    setWatchedFaction(std::numeric_limits<uint32_t>::max());

    m_StableSlotCount = 0;

    for (std::set<uint32>::iterator sp = info->spell_list.begin(); sp != info->spell_list.end(); ++sp)
        mSpells.insert((*sp));

    m_FirstLogin = true;


    for (std::list<CreateInfo_SkillStruct>::const_iterator ss = info->skills.begin(); ss != info->skills.end(); ++ss)
    {
        auto skill_line = sSkillLineStore.LookupEntry(ss->skillid);
        if (skill_line == nullptr)
            continue;

#if VERSION_STRING < Cata
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

    // add dbc items
    if (const auto charStartOutfitEntry = getStartOutfitByRaceClass(charCreateContent._race, charCreateContent._class, charCreateContent.gender))
    {
        for (uint8_t j = 0; j < OUTFIT_ITEMS; ++j)
        {
            if (charStartOutfitEntry->ItemId[j] <= 0)
                continue;

            const uint32_t itemId = charStartOutfitEntry->ItemId[j];

            const auto itemProperties = sMySQLStore.getItemProperties(itemId);
            if (!itemProperties)
            {
                LogDebugFlag(LF_DB_TABLES, "StartOutfit - Item with entry %u not in item_properties table but in CharStartOutfit.dbc!", itemId);
                continue;
            }

            auto item = objmgr.CreateItem(itemId, this);
            if (item)
            {
                item->setStackCount(1);

                int8_t itemSlot = 0;

                //shitty db lets check for dbc/db2 values
                if (itemProperties->InventoryType == 0)
                {
                    if (const auto itemDB2Properties = sItemStore.LookupEntry(itemId))
                        itemSlot = getItemInterface()->GetItemSlotByType(itemDB2Properties->InventoryType);
                }
                else
                {
                    itemSlot = getItemInterface()->GetItemSlotByType(itemProperties->InventoryType);
                }

                //use safeadd only for equipmentset items... all other items will go to a free bag slot.
                if (itemSlot < INVENTORY_SLOT_BAG_END && (itemProperties->Class == ITEM_CLASS_ARMOR || itemProperties->Class == ITEM_CLASS_WEAPON || itemProperties->Class == ITEM_CLASS_CONTAINER || itemProperties->Class == ITEM_CLASS_QUIVER))
                {
                    if (!getItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, itemSlot))
                    {
                        LogDebugFlag(LF_DB_TABLES, "StartOutfit - Item with entry %u can not be added safe to slot %u!", itemId, static_cast<uint32_t>(itemSlot));
                        item->DeleteMe();
                    }
                }
                else
                {
                    item->setStackCount(itemProperties->MaxCount);
                    if (!getItemInterface()->AddItemToFreeSlot(item))
                    {
                        LogDebugFlag(LF_DB_TABLES, "StartOutfit - Item with entry %u can not be added to a free slot!", itemId);
                        item->DeleteMe();
                    }
                }
            }
        }
    }

    for (std::list<CreateInfo_ItemStruct>::const_iterator is = info->items.begin(); is != info->items.end(); ++is)
    {
        if ((*is).protoid != 0)
        {
            auto item = objmgr.CreateItem((*is).protoid, this);
            if (item)
            {
                item->setStackCount((*is).amount);
                if ((*is).slot < INVENTORY_SLOT_BAG_END)
                {
                    if (!getItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, (*is).slot))
                        item->DeleteMe();
                }
                else
                {
                    if (!getItemInterface()->AddItemToFreeSlot(item))
                        item->DeleteMe();
                }
            }
        }
    }

    sHookInterface.OnCharacterCreate(this);
    load_health = getMaxHealth();
    load_mana = getMaxPower(POWER_TYPE_MANA);
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
        if (isAttackReady(MELEE))
            _EventAttack(false);

        if (hasOffHandWeapon() && isAttackReady(OFFHAND))
            _EventAttack(true);
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
                uint32 damage = getMaxHealth() / 10;

                sendEnvironmentalDamageLogPacket(getGuid(), uint8(DAMAGE_DROWNING), damage);
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
                sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
            }
        }
    }

    // Lava Damage
    if (m_UnderwaterState & UNDERWATERSTATE_LAVA)
    {
        // check last damage dealt timestamp, and if enough time has elapsed deal damage
        if (mstime >= m_UnderwaterLastDmg)
        {
            uint32 damage = getMaxHealth() / 5;

            sendEnvironmentalDamageLogPacket(getGuid(), uint8(DAMAGE_LAVA), damage);
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
            removePvpFlag();
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
    modCoinage(-(int32)money);

    if (money > 0 && m_fallDisabledUntil < time(nullptr) + 5)
        m_fallDisabledUntil = time(nullptr) + 5; //VLack: If the ride wasn't free, the player shouldn't die after arrival because of fall damage... So we'll disable it for 5 seconds.

    SetPosition(x, y, z, true);
    if (!m_taxiPaths.size())
        SetTaxiState(false);

    SetTaxiPath(nullptr);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    setMountDisplayId(0);
    removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
    removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

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
    if (isCastingSpell())
    {
        if (getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr) // this is a channeled spell - ignore the attack event
            return;

        interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
        setAttackTimer(offhand == true ? OFFHAND : MELEE, 500);
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
        setAttackTimer(offhand == true ? OFFHAND : MELEE, 300);
    }
    else if (!isInFront(pVictim))
    {
        // We still have to do this one.
        if (m_AttackMsgTimer != 2)
        {
            m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
            m_AttackMsgTimer = 2;
        }
        setAttackTimer(offhand == true ? OFFHAND : MELEE, 300);
    }
    else
    {
        m_AttackMsgTimer = 0;

        // Set to weapon time.
        if (offhand)
            setAttackTimer(OFFHAND, getBaseAttackTime(OFFHAND));
        else
            setAttackTimer(MELEE, getBaseAttackTime(MELEE));

        //pvp timeout reset
        if (pVictim->isPlayer())
        {
            if (static_cast< Player* >(pVictim)->cannibalize)
            {
                sEventMgr.RemoveEvents(pVictim, EVENT_CANNIBALIZE);
                pVictim->setEmoteState(EMOTE_ONESHOT_NONE);
                static_cast< Player* >(pVictim)->cannibalize = false;
            }
        }

        if (this->isStealthed())
            removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        if (GetOnMeleeSpell() == 0 || offhand)
            Strike(pVictim, (offhand ? OFFHAND : MELEE), nullptr, 0, 0, 0, false, false);
        else
            CastOnMeleeSpell();
    }
}

void Player::_EventCharmAttack()
{
    if (!m_CurrentCharm)
        return;

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

    Unit* pVictim = GetMapMgr()->GetUnit(m_curSelection);
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
            //if (pVictim->getObjectTypeId() == TYPEID_UNIT)
            //    pVictim->GetAIInterface()->StopMovement(5000);

            //pvp timeout reset
            /*if (pVictim->isPlayer())
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
                const auto spellInfo = sSpellMgr.getSpellInfo(currentCharm->GetOnMeleeSpell());
                currentCharm->SetOnMeleeSpell(0);
                Spell* spell = sSpellMgr.newSpell(currentCharm, spellInfo, true, nullptr);
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
    if (isDead())
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
        ZoneUpdate(AreaId);
    else if (at->zone != 0 && m_zoneId != at->zone)
        ZoneUpdate(at->zone);


    if (at->zone != 0 && m_zoneId != at->zone)
        ZoneUpdate(at->zone);

    bool rest_on = false;
    // Check for a restable area
    if (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY)
    {
        // check faction
        if (at->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance() || (at->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
            rest_on = true;
        else if (at->team != AREAC_ALLIANCE_TERRITORY && at->team != AREAC_HORDE_TERRITORY)
            rest_on = true;
    }
    else
    {
        //second AT check for subzones.
        if (at->zone)
        {
            auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
            if (at2 && (at2->flags & AREA_CITY_AREA || at2->flags & AREA_CITY))
            {
                if (at2->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance() || (at2->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
                    rest_on = true;
                else if (at2->team != AREAC_ALLIANCE_TERRITORY && at2->team != AREAC_HORDE_TERRITORY)
                    rest_on = true;
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

#if VERSION_STRING < Cata
    if (!(currFields & val) && !isOnTaxi() && !obj_movement_info.transport_data.transportGuid) //Unexplored Area        // bur: we don't want to explore new areas when on taxi
#else
    if (!(currFields & val) && !isOnTaxi() && obj_movement_info.getTransportGuid().IsEmpty()) //Unexplored Area        // bur: we don't want to explore new areas when on taxi
#endif
    {
        setUInt32Value(offset, (uint32)(currFields | val));

        uint32 explore_xp = at->area_level * 10;
        explore_xp *= float2int32(worldConfig.getFloatRate(RATE_EXPLOREXP));

#if VERSION_STRING > TBC
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA);
#endif

        if (getLevel() < getMaxLevel() && explore_xp > 0)
        {
            sendExploreExperiencePacket(at->id, explore_xp);
            GiveXP(explore_xp, 0, false);
        }
        else
        {
            sendExploreExperiencePacket(at->id, 0);
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
//  This function sends the message displaying the purple XP gain for the char
//  It assumes you will send out an UpdateObject packet at a later time.
//////////////////////////////////////////////////////////////////////////////////////////
void Player::GiveXP(uint32 xp, const uint64 & guid, bool allowbonus)
{
    if (xp < 1)
        return;

#if VERSION_STRING >= Cata
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

    if (getLevel() >= getMaxLevel())
        return;

    uint32 restxp = xp;

    //add reststate bonus (except for quests)
    if (m_restState == RESTSTATE_RESTED && allowbonus)
    {
        restxp = SubtractRestXP(xp);
        xp += restxp;
    }

    UpdateRestState();
    sendLogXpGainPacket(guid, xp, restxp, guid == 0 ? true : false);

    int32 newxp = getXp() + xp;
    int32 nextlevelxp = getNextLevelXp();
    uint32 level = getLevel();
    bool levelup = false;

    while (newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        LevelInfo* li = objmgr.GetLevelInfo(getRace(), getClass(), level);
        if (li == nullptr)
            return;
        newxp -= nextlevelxp;
        nextlevelxp = sMySQLStore.getPlayerXPForLevel(level);
        levelup = true;

        if (level >= getMaxLevel())
            break;
    }

    if (level > getMaxLevel())
        level = getMaxLevel();

    if (levelup)
    {
        m_playedtime[0] = 0; //Reset the "Current level played time"

        setLevel(level);
        LevelInfo* oldlevel = lvlinfo;
        lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), level);
        if (lvlinfo == nullptr)
            return;
        CalculateBaseStats();

        // Generate Level Info Packet and Send to client
        sendLevelupInfoPacket(
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
        setInitialTalentPoints();
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
        if (isDead())
            ResurrectPlayer();

        //set full hp and mana
        setHealth(getMaxHealth());
        setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));

        // if warlock has summoned pet, increase its level too
        if (info->class_ == WARLOCK)
        {
            Pet* summon = GetSummon();
            if (summon != nullptr && (summon->IsInWorld()) && summon->isAlive())
            {
                summon->setLevel(1);
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
    setXp(newxp);

}

void Player::smsg_InitialSpells()
{
    uint16 spellCount = (uint16)mSpells.size();
    size_t itemCount = m_cooldownMap[0].size() + m_cooldownMap[1].size();
    uint32 mstime = Util::getMSTime();

    WorldPacket data(SMSG_INITIAL_SPELLS, 5 + (spellCount * 4) + (itemCount * 4));
#if VERSION_STRING == Mop
    data.writeBit(0);
    data.writeBits(spellCount, 22);
    data.flushBits();

    for (SpellSet::iterator sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        data << uint32(*sitr);                   // spell id
    }
    data.flushBits();
    GetSession()->SendPacket(&data);
#else
    data << uint8(0);
    data << uint16(spellCount); // spell count

    for (SpellSet::iterator sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        //\todo check out when we should send 0x0 and when we should send 0xeeee this is not slot, values is always eeee or 0, seems to be cooldown
#if VERSION_STRING == TBC
        data << uint16(*sitr);                   // spell id
#else
        data << uint32(*sitr);                   // spell id
#endif
        data << uint16(0x0000);
    }

    size_t pos = data.wpos();
    data << uint16(0);        // placeholder

    itemCount = 0;
    for (PlayerCooldownMap::iterator itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end();)
    {
        PlayerCooldownMap::iterator itr2 = itr++;

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

    for (PlayerCooldownMap::iterator itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end();)
    {
        PlayerCooldownMap::iterator itr2 = itr++;

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
    GetSession()->OutPacket(SMSG_SERVER_BUCK_DATA, 4, &v);
#endif
#endif
    //Log::getSingleton().outDetail("CHARACTER: Sent Initial Spells");
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
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", getGuidLow());
    else
        buf->AddQuery("DELETE FROM playerpets WHERE ownerguid = %u", getGuidLow());

    Pet* summon = GetSummon();
    if (summon && summon->IsInWorld() && summon->getPlayerOwner() == this)    // update PlayerPets array with current pet's info
    {
        PlayerPet* pPet = GetPlayerPet(summon->m_PetNumber);
        if (!pPet || pPet->active == false)
            summon->UpdatePetInfo(true);
        else
            summon->UpdatePetInfo(false);

        if (!summon->Summon)       // is a pet
        {
            // save pet spellz
            uint32 pn = summon->m_PetNumber;
            if (buf == nullptr)
                CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), pn);
            else
                buf->AddQuery("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), pn);

            for (PetSpellMap::iterator itr = summon->mSpells.begin(); itr != summon->mSpells.end(); ++itr)
            {
                if (buf == nullptr)
                    CharacterDatabase.Execute("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", getGuidLow(), pn, itr->first->getId(), itr->second);
                else
                    buf->AddQuery("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", getGuidLow(), pn, itr->first->getId(), itr->second);
            }
        }
    }

    std::stringstream ss;

    ss.rdbuf()->str("");

    for (std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); ++itr)
    {
        ss.rdbuf()->str("");

        ss << "REPLACE INTO playerpets VALUES('"
            << getGuidLow() << "','"
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
        CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid=%u", getGuidLow());
    else
        buf->AddQuery("DELETE FROM playersummonspells WHERE ownerguid=%u", getGuidLow());

    // Save summon spells
    for (std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.begin(); itr != SummonSpells.end(); ++itr)
    {
        for (std::set<uint32>::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            if (buf == nullptr)
                CharacterDatabase.Execute("INSERT INTO playersummonspells VALUES(%u, %u, %u)", getGuidLow(), itr->first, (*it));
            else
                buf->AddQuery("INSERT INTO playersummonspells VALUES(%u, %u, %u)", getGuidLow(), itr->first, (*it));
        }
    }
}

void Player::AddSummonSpell(uint32 Entry, uint32 SpellID)
{
    SpellInfo const* sp = sSpellMgr.getSpellInfo(SpellID);
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if (itr == SummonSpells.end())
    {
        SummonSpells[Entry].insert(SpellID);
    }
    else
    {
        std::set<uint32>::iterator it3;
        for (std::set<uint32>::iterator it2 = itr->second.begin(); it2 != itr->second.end();)
        {
            it3 = it2++;
            if (sSpellMgr.getSpellInfo(*it3)->custom_NameHash == sp->custom_NameHash)
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
        return &itr->second;

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
        LOG_ERROR("PET SYSTEM: " I64FMT " Tried to load invalid pet %d", getGuid(), pet_number);
        return;
    }
    Pet* pPet = objmgr.CreatePet(itr->second->entry);
    pPet->LoadFromDB(this, itr->second);

    if (this->isPvpFlagSet())
        pPet->setPvpFlag();
    else
        pPet->removePvpFlag();

    if (this->isFfaPvpFlagSet())
        pPet->setFfaPvpFlag();
    else
        pPet->removeFfaPvpFlag();

    if (this->isSanctuaryFlagSet())
        pPet->setSanctuaryFlag();
    else
        pPet->removeSanctuaryFlag();

    pPet->SetFaction(this->getFactionTemplate());

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
    if (GetSummon() != nullptr || !isAlive() || !IsInWorld())   //\todo  only hunters for now
        return;

    for (std::map<uint32, PlayerPet* >::iterator itr = m_Pets.begin(); itr != m_Pets.end(); ++itr)
    {
        if (itr->second->stablestate == STABLE_STATE_ACTIVE && itr->second->active)
        {
            if (itr->second->alive)
                SpawnPet(itr->first);

            return;
        }
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
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[1].GetUInt32();
            uint32 spell = fields[2].GetUInt32();
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
#if VERSION_STRING < Cata
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
    SpellInfo const* spell = sSpellMgr.getSpellInfo(spell_id);
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
        // temporary solution since spell description is no longer loaded -Appled
        const auto creatureEntry = spell->getEffectMiscValue(0);
        auto creatureProperties = sMySQLStore.getCreatureProperties(creatureEntry);
        if (creatureProperties != nullptr && creatureProperties->Type == UNIT_TYPE_NONCOMBAT_PET)
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS, 778, 0, 0);
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set Create Player Bits -- Sets bits required for creating a player in the updateMask.
// Note:  Doesn't set Quest or Inventory bits
// updateMask - the updatemask to hold the set bits
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
#if VERSION_STRING < Cata
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

    if (getFreePrimaryProfessionPoints() > worldConfig.player.maxProfessions)
        setFreePrimaryProfessionPoints(worldConfig.player.maxProfessions);

    //Calc played times
    uint32 playedt = (uint32)UNIXTIME - m_playedtime[2];
    m_playedtime[0] += playedt;
    m_playedtime[1] += playedt;
    m_playedtime[2] += playedt;

    // active cheats
    uint32 active_cheats = PLAYER_CHEAT_NONE;
    if (m_cheats.CooldownCheat)
        active_cheats |= PLAYER_CHEAT_COOLDOWN;
    if (m_cheats.CastTimeCheat)
        active_cheats |= PLAYER_CHEAT_CAST_TIME;
    if (m_cheats.GodModeCheat)
        active_cheats |= PLAYER_CHEAT_GOD_MODE;
    if (m_cheats.PowerCheat)
        active_cheats |= PLAYER_CHEAT_POWER;
    if (m_cheats.FlyCheat)
        active_cheats |= PLAYER_CHEAT_FLY;
    if (m_cheats.AuraStackCheat)
        active_cheats |= PLAYER_CHEAT_AURA_STACK;
    if (m_cheats.ItemStackCheat)
        active_cheats |= PLAYER_CHEAT_ITEM_STACK;
    if (m_cheats.TriggerpassCheat)
        active_cheats |= PLAYER_CHEAT_TRIGGERPASS;
    if (m_cheats.TaxiCheat)
        active_cheats |= PLAYER_CHEAT_TAXI;

    std::stringstream ss;

    ss << "DELETE FROM characters WHERE guid = " << getGuidLow() << ";";

    if (bNewCharacter)
        CharacterDatabase.WaitExecuteNA(ss.str().c_str());
    else
        buf->AddQueryNA(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO characters VALUES ("
        << getGuidLow() << ", "
        << GetSession()->GetAccountId() << ","
        // stat saving
        << "'" << m_name << "', "
        << uint32(getRace()) << ","
        << uint32(getClass()) << ","
        << uint32(getGender()) << ",";

    ss << getFactionTemplate() << ",";

    ss << uint32(getLevel()) << ","
        << getXp() << ","
        << active_cheats << ","
        // dump exploration data
        << "'";

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_LENGTH; ++i)
        ss << m_uint32Values[PLAYER_EXPLORED_ZONES_1 + i] << ",";

    ss << "',";

    SaveSkills(bNewCharacter, buf);

    ss << getWatchedFaction() << ","
#if VERSION_STRING > Classic
        << getChosenTitle() << ","
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

    if (getClass() == MAGE || getClass() == PRIEST || (getClass() == WARLOCK))
        ss << (uint32)0 << ","; // make sure ammo slot is 0 for these classes, otherwise it can mess up wand shoot
    else
#if VERSION_STRING < Cata
        ss << m_uint32Values[PLAYER_AMMO_ID] << ",";
#else
        ss << (uint32)0 << ",";
#endif
    ss << getFreePrimaryProfessionPoints() << ",";

    ss << load_health << ","
        << load_mana << ","
        << uint32(getPvpRank()) << ","
        << getPlayerBytes() << ","
        << getPlayerBytes2() << ",";

    // Remove un-needed and problematic player flags from being saved :p
    if (hasPlayerFlags(PLAYER_FLAG_PARTY_LEADER))
        removePlayerFlags(PLAYER_FLAG_PARTY_LEADER);

    if (hasPlayerFlags(PLAYER_FLAG_AFK))
        removePlayerFlags(PLAYER_FLAG_AFK);

    if (hasPlayerFlags(PLAYER_FLAG_DND))
        removePlayerFlags(PLAYER_FLAG_DND);

    if (hasPlayerFlags(PLAYER_FLAG_GM))
        removePlayerFlags(PLAYER_FLAG_GM);

    if (hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE))
        removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);

    if (hasPlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP))
        removePlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);

    ss << getPlayerFlags() << ","
        << getPlayerFieldBytes() << ",";

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
        ss << "0,";
    else
        ss << "1,";

    ss << m_bind_pos_x << ", "
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
        ss << m_bgEntryPointInstance << ", ";
    else
        ss << m_instanceId << ", ";

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
        ss << getMountDisplayId();
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
        ss << "," << transport->getEntry();
        ss << ",'" << GetTransPositionX() 
            << "','" << GetTransPositionY() 
            << "','" << GetTransPositionZ()
            << "','" << GetTransPositionO() << "'";
    }
    ss << ",'";

    SaveSpells(bNewCharacter, buf);

    SaveDeletedSpells(bNewCharacter, buf);

    SaveReputations(bNewCharacter, buf);

    // Add player action bars
#ifdef FT_DUAL_SPEC
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
#else
    for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        ss << uint32(m_spec.mActions[i].Action) << ","
                << uint32(m_spec.mActions[i].Type) << ","
                << uint32(m_spec.mActions[i].Misc) << ",";
    }
    ss << "','','";
#endif

    if (!bNewCharacter)
        SaveAuras(ss);

    ss << "','";

    // Add player finished quests
    for (std::set<uint32>::iterator fq = m_finishedQuests.begin(); fq != m_finishedQuests.end(); ++fq)
        ss << (*fq) << ",";

    ss << "', '";
    DailyMutex.Acquire();
    for (std::set<uint32>::iterator fdq = m_finishedDailies.begin(); fdq != m_finishedDailies.end(); ++fdq)
        ss << (*fdq) << ",";

    DailyMutex.Release();
    ss << "', ";
    ss << m_honorRolloverTime << ", ";
    ss << m_killsToday << ", ";
    ss << m_killsYesterday << ", ";
    ss << m_killsLifetime << ", ";
    ss << m_honorToday << ", ";
    ss << m_honorYesterday << ", ";
    ss << m_honorPoints << ", ";

    ss << getDrunkValue() << ", ";

    // TODO Remove
#ifdef FT_DUAL_SPEC
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
#else
    ss << "'', '";
    for (const auto talent : m_spec.talents)
        ss << talent.first << "," << talent.second << ",";

    ss << "', '', '', ";
#endif
    ss << uint32(m_talentSpecsCount) << ", " << uint32(m_talentActiveSpec);
    ss << ", '";
#ifdef FT_DUAL_SPEC
    ss << uint32(m_specs[SPEC_PRIMARY].GetTP()) << " " << uint32(m_specs[SPEC_SECONDARY].GetTP());
#else
    ss << uint32(m_spec.GetTP()) << " 0";
#endif
    ss << "', ";

#if VERSION_STRING < Cata
    ss << "'" << uint32(0);
    ss << "', ";
#else
    ss << "'" << uint32(m_FirstTalentTreeLock);
    ss << "', ";
#endif

    ss << "'" << m_phase << "','";

    uint32 xpfield = 0;

    if (m_XpGain)
        xpfield = 1;

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
    getItemInterface()->mSaveItemsToDatabase(bNewCharacter, buf);

    getItemInterface()->m_EquipmentSets.SavetoDB(buf);

    // save quest progress
    _SaveQuestLogEntry(buf);

    // Tutorials
    _SaveTutorials(buf);

    // GM Ticket
    //\todo Is this really necessary? Tickets will always be saved on creation, update and so on...
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(getGuid());
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
            CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", getGuidLow(), (*itr));
        else
            buf->AddQuery("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", getGuidLow(), (*itr));
    }

    m_removequests.clear();

    for (uint8 i = 0; i < 25; ++i)
    {
        if (m_questlog[i] != nullptr)
            m_questlog[i]->SaveToDB(buf);
    }
}

bool Player::canCast(SpellInfo const* m_spellInfo)
{
    if (m_spellInfo->getEquippedItemClass() != 0)
    {
        if (this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
        {
            if ((int32)this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->getItemProperties()->Class == m_spellInfo->getEquippedItemClass())
            {
                if (m_spellInfo->getEquippedItemSubClass() != 0)
                {
                    if (m_spellInfo->getEquippedItemSubClass() != 173555 && m_spellInfo->getEquippedItemSubClass() != 96 && m_spellInfo->getEquippedItemSubClass() != 262156)
                    {
                        if (pow(2.0, (this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->getItemProperties()->SubClass) != m_spellInfo->getEquippedItemSubClass()))
                            return false;
                    }
                }
            }
        }
        else if (m_spellInfo->getEquippedItemSubClass() == 173555)
            return false;

        if (this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
        {
            if ((int32)this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->getItemProperties()->Class == m_spellInfo->getEquippedItemClass())
            {
                if (m_spellInfo->getEquippedItemSubClass() != 0)
                {
                    if (m_spellInfo->getEquippedItemSubClass() != 173555 && m_spellInfo->getEquippedItemSubClass() != 96 && m_spellInfo->getEquippedItemSubClass() != 262156)
                    {
                        if (pow(2.0, (this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->getItemProperties()->SubClass) != m_spellInfo->getEquippedItemSubClass()))
                            return false;
                    }
                }
            }
        }
        else if (m_spellInfo->getEquippedItemSubClass() == 262156)
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

namespace PlayerQuery
{
    enum
    {
        LoginFlags = 0,
        Tutorials = 1,
        Cooldowns = 2,
        Questlog = 3,
        Items = 4,
        Pets = 5,
        SummonSpells = 6,
        Mailbox = 7,
        Friends = 8,
        FriendsFor = 9,
        Ignoring = 10,
        EquipmentSets = 11,
        Reputation = 12,
        Spells = 13,
        DeletedSpells = 14,
        Skills = 15,
        Achievements = 16,
        AchievementProgress = 17
    };
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
    setGuidLow(guid);
    CharacterDatabase.QueueAsyncQuery(q);
    return true;

}

#if VERSION_STRING <= TBC
void Player::LoadFromDBProc(QueryResultVector & results)
{
    uint32 field_index = 2;
#define get_next_field fields[field_index++]

    if (GetSession() == nullptr || results.size() < 8)        // should have 8 queryresults for aplayer load.
    {
        RemovePendingPlayer();
        return;
    }

    QueryResult* result = results[PlayerQuery::LoginFlags].result;
    if (!result)
    {
        LOG_ERROR("Player login query failed! guid = %u", getGuidLow());
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
        LOG_ERROR("guid %u failed to login, no race or class dbc found. (race %u class %u)", (unsigned int)getGuidLow(), (unsigned int)getRace(), (unsigned int)getClass());
        RemovePendingPlayer();
        return;
    }

    m_bgTeam = myRace->team_id == 7 ? 0 : 1;
    m_cache->SetUInt32Value(CACHE_PLAYER_INITIALTEAM, m_team);
    SetNoseLevel();
    setPowerType(myClass->power_type);

    // obtain player create info
    info = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    if (info == nullptr)
    {
        LOG_ERROR("player guid %u has no playerCreateInfo!", (unsigned int)getGuidLow());
        RemovePendingPlayer();
        return;
    }

    // set level
    setLevel(get_next_field.GetUInt32());

    // obtain level/stats information
    lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), getLevel());

    if (!lvlinfo)
    {
        LOG_ERROR("guid %u level %u class %u race %u levelinfo not found!", (unsigned int)getGuidLow(), (unsigned int)getLevel(), (unsigned int)getClass(), (unsigned int)getRace());
        RemovePendingPlayer();
        return;
    }

    CalculateBaseStats();

    // set xp
    setXp(get_next_field.GetUInt32());

    // Load active cheats
    uint32 active_cheats = get_next_field.GetUInt32();
    if (active_cheats & PLAYER_CHEAT_COOLDOWN)
        m_cheats.CooldownCheat = true;
    if (active_cheats & PLAYER_CHEAT_CAST_TIME)
        m_cheats.CastTimeCheat = true;
    if (active_cheats & PLAYER_CHEAT_GOD_MODE)
        m_cheats.GodModeCheat = true;
    if (active_cheats & PLAYER_CHEAT_POWER)
        m_cheats.PowerCheat = true;
    if (active_cheats & PLAYER_CHEAT_FLY)
        m_cheats.FlyCheat = true;
    if (active_cheats & PLAYER_CHEAT_AURA_STACK)
        m_cheats.AuraStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_ITEM_STACK)
        m_cheats.ItemStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_TRIGGERPASS)
        m_cheats.TriggerpassCheat = true;
    if (active_cheats & PLAYER_CHEAT_TAXI)
        m_cheats.TaxiCheat = true;

    // Process exploration data.
    LoadFieldsFromString(get_next_field.GetString(), PLAYER_EXPLORED_ZONES_1, PLAYER_EXPLORED_ZONES_LENGTH);

    // Process skill data.
    uint32 Counter = 0;
    char* start = nullptr;
    char* end = nullptr;

    // new format
    const ItemProf* prof1;

    LoadSkills(results[PlayerQuery::Skills].result);

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
    setWatchedFaction(get_next_field.GetUInt32());
#if VERSION_STRING > Classic
    setChosenTitle(get_next_field.GetUInt32());
#else
    get_next_field; //skip selected_pvp_titles
#endif
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES, get_next_field.GetUInt64());

    get_next_field; //skip available_pvp_titles1
    get_next_field; //skip available_pvp_titles2


    m_uint32Values[PLAYER_FIELD_COINAGE] = get_next_field.GetUInt32();

    m_uint32Values[PLAYER_AMMO_ID] = get_next_field.GetUInt32();
    m_uint32Values[PLAYER_CHARACTER_POINTS2] = get_next_field.GetUInt32();

    load_health = get_next_field.GetUInt32();
    load_mana = get_next_field.GetUInt32();
    setHealth(load_health);
    setPvpRank(get_next_field.GetUInt8());
    setPlayerBytes(get_next_field.GetUInt32());
    setPlayerBytes2(get_next_field.GetUInt32());
    setPlayerGender(getGender());
    setPlayerFlags(get_next_field.GetUInt32());
    setPlayerFieldBytes(get_next_field.GetUInt32());

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

#if VERSION_STRING != Classic
    for (uint8_t x = 0; x < SCHOOL_COUNT; ++x)
        setModDamageDonePct(1.00f, x);
#endif

    // Normal processing...
    //\todo investigate
#if VERSION_STRING != Classic
    UpdateStats();
#endif

    // Initialize 'normal' fields
    setScale(1.0f);

    //SetUInt32Value(UNIT_FIELD_POWER2, 0);
    setPower(POWER_TYPE_FOCUS, info->focus); // focus
    setPower(POWER_TYPE_ENERGY, info->energy);
    setMaxPower(POWER_TYPE_RAGE, info->rage);
    setMaxPower(POWER_TYPE_FOCUS, info->focus);
    setMaxPower(POWER_TYPE_ENERGY, info->energy);
    if (getClass() == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

    setPvpFlags(U_FIELD_BYTES_FLAG_UNK2 | U_FIELD_BYTES_FLAG_SANCTUARY);
    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    setBoundingRadius(0.388999998569489f);
    setCombatReach(1.5f);

    setInitialDisplayIds(getGender(), getRace());

    EventModelChange();

    setModCastSpeed(1.0f);
    setMaxLevel(worldConfig.player.playerLevelCap);

    if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
        SetFaction(raceEntry->faction_id);
    else
        SetFaction(0);
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

    get_next_field; //skip online

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
            sendReportToGmMessage(getName().c_str(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            m_session->Disconnect();
        }
        m_arenaPoints = worldConfig.limit.maxArenaPoints;
    }
    for (uint32 z = 0; z < NUM_CHARTER_TYPES; ++z)
        m_charters[z] = objmgr.GetCharterByGuid(getGuid(), (CharterTypes)z);
    for (uint16 z = 0; z < NUM_ARENA_TEAM_TYPES; ++z)
    {
        m_arenaTeams[z] = objmgr.GetArenaTeamByGuid(getGuidLow(), z);
        if (m_arenaTeams[z] != nullptr)
        {
#if VERSION_STRING != Classic
            setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z * 7), m_arenaTeams[z]->m_id);
            if (m_arenaTeams[z]->m_leader == getGuidLow())
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
            setMountDisplayId(get_next_field.GetUInt32());
            SetTaxiPath(path);
            m_onTaxi = true;
        }
        else
            get_next_field; //skip taxi_mount
    }
    else
    {
        get_next_field; //skip taxi_lastnode
        get_next_field; //skip taxi_mount
    }

    obj_movement_info.transport_data.transportGuid = get_next_field.GetUInt32();
    if (obj_movement_info.transport_data.transportGuid)
    {
        Transporter* t = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_data.transportGuid));
        obj_movement_info.transport_data.transportGuid = t ? t->getGuid() : 0;
    }

    obj_movement_info.transport_data.relativePosition.x = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.y = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.z = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.o = get_next_field.GetFloat();

    LoadSpells(results[PlayerQuery::Spells].result);

    LoadDeletedSpells(results[PlayerQuery::DeletedSpells].result);

    LoadReputations(results[PlayerQuery::Reputation].result);

    // Load saved actionbars
    for (uint8 s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        auto& spec = m_spec;

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
            spec.mActions[Counter].Action = (uint16)atol(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            spec.mActions[Counter].Type = (uint8)atol(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            spec.mActions[Counter].Misc = (uint8)atol(start);
            start = end + 1;

            Counter++;
        }
    }

    get_next_field; //skip actions2

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
        la.positive = auraPositivValue.c_str() == nullptr ? false : true;

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
            sendReportToGmMessage(getName().c_str(), dmgLog.str());

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
        get_next_field; //skip glyphs1

        auto& spec = m_spec;

        //Load talents for spec
        start = (char*)get_next_field.GetString();  // talents1
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

            spec.talents.insert(std::pair<uint32, uint8>(talentid, rank));
        }
    }

    get_next_field; //skip glyphs2
    get_next_field; //skip talents2

    m_talentSpecsCount = get_next_field.GetUInt8();
    m_talentActiveSpec = get_next_field.GetUInt8();

    {
        std::stringstream ss(get_next_field.GetString());
        uint32 tp1 = 0;
        uint32 tp2 = 0;

        ss >> tp1;
        ss >> tp2;

        m_spec.SetTP(tp1);
        setFreeTalentPoints(getActiveSpec().GetTP());
    }

    get_next_field;//skipping firsttalenttree

    m_phase = get_next_field.GetUInt32(); //Load the player's last phase field 89

    uint32 xpfield = get_next_field.GetUInt32();

    if (xpfield == 0)
        m_XpGain = false;
    else
        m_XpGain = true;

    get_next_field;//skipping data

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

    for (uint8_t x = 0; x < STAT_COUNT; x++)
        BaseStats[x] = getStat(x);

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
        case WARLOCK:
        case HUNTER:
            _LoadPet(results[PlayerQuery::Pets].result);
            _LoadPetSpells(results[PlayerQuery::SummonSpells].result);
            break;
    }

#if VERSION_STRING < Cata
    if (m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else
        _AddLanguages(false);
#else
    _AddLanguages(false);
#endif

    if (getGuildId())
        setUInt32Value(PLAYER_GUILD_TIMESTAMP, (uint32)UNIXTIME);

#undef get_next_field

    // load properties
    _LoadTutorials(results[PlayerQuery::Tutorials].result);
    _LoadPlayerCooldowns(results[PlayerQuery::Cooldowns].result);
    _LoadQuestLogEntry(results[PlayerQuery::Questlog].result);
    getItemInterface()->mLoadItemsFromDatabase(results[PlayerQuery::Items].result);
    getItemInterface()->m_EquipmentSets.LoadfromDB(results[PlayerQuery::EquipmentSets].result);

    m_mailBox.Load(results[PlayerQuery::Mailbox].result);

    // SOCIAL
    if (results[PlayerQuery::Friends].result != nullptr)            // this query is "who are our friends?"
    {
        result = results[PlayerQuery::Friends].result;
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

    if (results[PlayerQuery::FriendsFor].result != nullptr)            // this query is "who has us in their friends?"
    {
        result = results[PlayerQuery::FriendsFor].result;
        do
        {
            m_cache->InsertValue64(CACHE_SOCIAL_HASFRIENDLIST, result->Fetch()[0].GetUInt32());
        }
        while (result->NextRow());
    }

    if (results[PlayerQuery::Ignoring].result != nullptr)        // this query is "who are we ignoring"
    {
        result = results[PlayerQuery::Ignoring].result;
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

    m_session->fullLogin(this);
    m_session->m_loggingInPlayer = nullptr;

    if (!isAlive())
    {
        Corpse* myCorpse = objmgr.GetCorpseByOwner(getGuidLow());
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
        ItemInterface* itemi = getItemInterface();
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
#else

void Player::LoadFromDBProc(QueryResultVector & results)
{
    uint32 field_index = 2;
#define get_next_field fields[field_index++]

    if (GetSession() == nullptr || results.size() < 8)        // should have 8 queryresults for aplayer load.
    {
        RemovePendingPlayer();
        return;
    }

    QueryResult* result = results[PlayerQuery::LoginFlags].result;
    if (!result)
    {
        LOG_ERROR("Player login query failed! guid = %u", getGuidLow());
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
        LOG_ERROR("guid %u failed to login, no race or class dbc found. (race %u class %u)", (unsigned int)getGuidLow(), (unsigned int)getRace(), (unsigned int)getClass());
        RemovePendingPlayer();
        return;
    }

    if (myRace->team_id == 7)
        m_bgTeam = m_team = 0;
    else
        m_bgTeam = m_team = 1;

    m_cache->SetUInt32Value(CACHE_PLAYER_INITIALTEAM, m_team);

    SetNoseLevel();

    // set power type
    setPowerType(static_cast<uint8>(myClass->power_type));

    // obtain player create info
    info = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    if (info == nullptr)
    {
        LOG_ERROR("player guid %u has no playerCreateInfo!", (unsigned int)getGuidLow());
        RemovePendingPlayer();
        return;
    }

    // set level
    setLevel(get_next_field.GetUInt32());

    // obtain level/stats information
    lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), getLevel());

    if (!lvlinfo)
    {
        LOG_ERROR("guid %u level %u class %u race %u levelinfo not found!", (unsigned int)getGuidLow(), (unsigned int)getLevel(), (unsigned int)getClass(), (unsigned int)getRace());
        RemovePendingPlayer();
        return;
    }

#if VERSION_STRING > TBC
    // load achievements before anything else otherwise skills would complete achievements already in the DB, leading to duplicate achievements and criterias(like achievement=126).
    m_achievementMgr.LoadFromDB(results[PlayerQuery::Achievements].result, results[PlayerQuery::AchievementProgress].result);
#endif

    CalculateBaseStats();

    // set xp
    setXp(get_next_field.GetUInt32());

    // Load active cheats
    uint32 active_cheats = get_next_field.GetUInt32();
    if (active_cheats & PLAYER_CHEAT_COOLDOWN)
        m_cheats.CooldownCheat = true;
    if (active_cheats & PLAYER_CHEAT_CAST_TIME)
        m_cheats.CastTimeCheat = true;
    if (active_cheats & PLAYER_CHEAT_GOD_MODE)
        m_cheats.GodModeCheat = true;
    if (active_cheats & PLAYER_CHEAT_POWER)
        m_cheats.PowerCheat = true;
    if (active_cheats & PLAYER_CHEAT_FLY)
        m_cheats.FlyCheat = true;
    if (active_cheats & PLAYER_CHEAT_AURA_STACK)
        m_cheats.AuraStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_ITEM_STACK)
        m_cheats.ItemStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_TRIGGERPASS)
        m_cheats.TriggerpassCheat = true;
    if (active_cheats & PLAYER_CHEAT_TAXI)
        m_cheats.TaxiCheat = true;

    // Process exploration data.
    LoadFieldsFromString(get_next_field.GetString(), PLAYER_EXPLORED_ZONES_1, PLAYER_EXPLORED_ZONES_LENGTH);

    // Process skill data.
    uint32 Counter = 0;
    char* start = nullptr;
    char* end = nullptr;

    // new format
    const ItemProf* prof1;

    LoadSkills(results[PlayerQuery::Skills].result);

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
    setWatchedFaction(get_next_field.GetUInt32());
#if VERSION_STRING > Classic
    setChosenTitle(get_next_field.GetUInt32());
#endif
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES, get_next_field.GetUInt64());
#if VERSION_STRING > TBC
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES1, get_next_field.GetUInt64());
    setUInt64Value(PLAYER_FIELD_KNOWN_TITLES2, get_next_field.GetUInt64());
#else
    get_next_field.GetUInt32();
    get_next_field.GetUInt32();
#endif
    m_uint32Values[PLAYER_FIELD_COINAGE] = get_next_field.GetUInt32();
#if VERSION_STRING < Cata
    m_uint32Values[PLAYER_AMMO_ID] = get_next_field.GetUInt32();
    m_uint32Values[PLAYER_CHARACTER_POINTS2] = get_next_field.GetUInt32();
#else
    get_next_field.GetUInt32();
    get_next_field.GetUInt32();
#endif
    load_health = get_next_field.GetUInt32();
    load_mana = get_next_field.GetUInt32();
    setHealth(load_health);
    setPvpRank(get_next_field.GetUInt8());
    setPlayerBytes(get_next_field.GetUInt32());
    setPlayerBytes2(get_next_field.GetUInt32());
    setPlayerGender(getGender());
    setPlayerFlags(get_next_field.GetUInt32());
    setPlayerFieldBytes(get_next_field.GetUInt32());
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
    for (uint8_t x = 0; x < SCHOOL_COUNT; ++x)
        setModDamageDonePct(1.00f, x);
#endif

    // Normal processing...
    UpdateStats();

    // Initialize 'normal' fields
    setScale(1.0f);
#if VERSION_STRING > TBC
    setHoverHeight(1.0f);
#endif

    //SetUInt32Value(UNIT_FIELD_POWER2, 0);
    setPower(POWER_TYPE_FOCUS, info->focus); // focus
    setPower(POWER_TYPE_ENERGY, info->energy);
#if VERSION_STRING == WotLK
    setPower(POWER_TYPE_RUNES, 8);
#endif
    setMaxPower(POWER_TYPE_RAGE, info->rage);
    setMaxPower(POWER_TYPE_FOCUS, info->focus);
    setMaxPower(POWER_TYPE_ENERGY, info->energy);
#if VERSION_STRING == WotLK
    setMaxPower(POWER_TYPE_RUNES, 8);
    setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
#endif
    if (getClass() == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

    setPvpFlags(U_FIELD_BYTES_FLAG_UNK2 | U_FIELD_BYTES_FLAG_SANCTUARY);
    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    setBoundingRadius(0.388999998569489f);
    setCombatReach(1.5f);

    setInitialDisplayIds(getGender(), getRace());

    EventModelChange();

    setModCastSpeed(1.0f);
    setMaxLevel(worldConfig.player.playerLevelCap);

    if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
        SetFaction(raceEntry->faction_id);
    else
        SetFaction(0);
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
            sendReportToGmMessage(getName().c_str(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            m_session->Disconnect();
        }
        m_arenaPoints = worldConfig.limit.maxArenaPoints;
    }
    for (uint32 z = 0; z < NUM_CHARTER_TYPES; ++z)
        m_charters[z] = objmgr.GetCharterByGuid(getGuid(), (CharterTypes)z);
    for (uint16 z = 0; z < NUM_ARENA_TEAM_TYPES; ++z)
    {
        m_arenaTeams[z] = objmgr.GetArenaTeamByGuid(getGuidLow(), z);
        if (m_arenaTeams[z] != nullptr)
        {
#if VERSION_STRING != Classic
            setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z * 7), m_arenaTeams[z]->m_id);
            if (m_arenaTeams[z]->m_leader == getGuidLow())
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
            setMountDisplayId(get_next_field.GetUInt32());
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

#if VERSION_STRING < Cata
    obj_movement_info.transport_data.transportGuid = get_next_field.GetUInt32();
    if (obj_movement_info.transport_data.transportGuid)
    {
        Transporter* t = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_data.transportGuid));
        obj_movement_info.transport_data.transportGuid = t ? t->getGuid() : 0;
    }

    obj_movement_info.transport_data.relativePosition.x = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.y = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.z = get_next_field.GetFloat();
    obj_movement_info.transport_data.relativePosition.o = get_next_field.GetFloat();
#else
    uint32_t transportGuid = get_next_field.GetUInt32();
    float transportX = get_next_field.GetFloat();
    float transportY = get_next_field.GetFloat();
    float transportZ = get_next_field.GetFloat();
    float transportO = get_next_field.GetFloat();

    if (transportGuid != 0)
        obj_movement_info.setTransportData(transportGuid, transportX, transportY, transportZ, transportO, 0, 0);
    else
        obj_movement_info.clearTransportData();

#endif

    LoadSpells(results[PlayerQuery::Spells].result);

    LoadDeletedSpells(results[PlayerQuery::DeletedSpells].result);

    LoadReputations(results[PlayerQuery::Reputation].result);

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
        la.positive = auraPositivValue.c_str() == nullptr ? false : true;

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
        const uint32_t questEntry = atol(start);
        m_finishedQuests.insert(questEntry);

        // Load talent points from finished quests
        auto questProperties = sMySQLStore.getQuestProperties(questEntry);
        if (questProperties != nullptr && questProperties->rewardtalents > 0)
            m_talentPointsFromQuests += questProperties->rewardtalents;

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
            sendReportToGmMessage(getName().c_str(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
            m_session->Disconnect();

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
#if VERSION_STRING < Cata
        setFreeTalentPoints(getActiveSpec().GetTP());
#endif
    }

#if VERSION_STRING < Cata
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

    for (uint8_t x = 0; x < STAT_COUNT; x++)
        BaseStats[x] = getStat(x);

#if VERSION_STRING > TBC
    UpdateGlyphs();

    for (uint8 i = 0; i < GLYPHS_COUNT; ++i)
    {
        setGlyph(i, m_specs[m_talentActiveSpec].glyphs[i]);
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
            _LoadPet(results[PlayerQuery::Pets].result);
            _LoadPetSpells(results[PlayerQuery::SummonSpells].result);
            break;
    }

#if VERSION_STRING < Cata
    if (m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else
        _AddLanguages(false);
#else
    _AddLanguages(false);
#endif

    if (getGuildId())
        setGuildTimestamp(static_cast<uint32_t>(UNIXTIME));

#undef get_next_field

    // load properties
    _LoadTutorials(results[PlayerQuery::Tutorials].result);
    _LoadPlayerCooldowns(results[PlayerQuery::Cooldowns].result);
    _LoadQuestLogEntry(results[PlayerQuery::Questlog].result);
    getItemInterface()->mLoadItemsFromDatabase(results[PlayerQuery::Items].result);
    getItemInterface()->m_EquipmentSets.LoadfromDB(results[PlayerQuery::EquipmentSets].result);

    m_mailBox.Load(results[PlayerQuery::Mailbox].result);

    // SOCIAL
    if (results[PlayerQuery::Friends].result != nullptr)            // this query is "who are our friends?"
    {
        result = results[PlayerQuery::Friends].result;
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

    if (results[PlayerQuery::FriendsFor].result != nullptr)            // this query is "who has us in their friends?"
    {
        result = results[PlayerQuery::FriendsFor].result;
        do
        {
            m_cache->InsertValue64(CACHE_SOCIAL_HASFRIENDLIST, result->Fetch()[0].GetUInt32());
        }
        while (result->NextRow());
    }

    if (results[PlayerQuery::Ignoring].result != nullptr)        // this query is "who are we ignoring"
    {
        result = results[PlayerQuery::Ignoring].result;
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
        _RemoveSkillLine(SKILL_DUAL_WIELD);

#if VERSION_STRING > TBC
    // update achievements before adding player to World, otherwise we'll get a nice race condition.
    //move CheckAllAchievementCriteria() after FullLogin(this) and i'll cut your b***s.
    m_achievementMgr.CheckAllAchievementCriteria();
#endif

    m_session->fullLogin(this);
    m_session->m_loggingInPlayer = nullptr;

    if (!isAlive())
    {
        Corpse* myCorpse = objmgr.GetCorpseByOwner(getGuidLow());
        if (myCorpse != nullptr)
        {
            myCorpseLocation = myCorpse->GetPosition();
            myCorpseInstanceId = myCorpse->GetInstanceID();
        }
    }

    uint32 uniques[64];
    int nuniques = 0;

    for (uint8 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        ItemInterface* itemi = getItemInterface();
        Item* it = itemi->GetInventoryItem(x);

        if (it != nullptr)
        {
            for (uint32 count = 0; count < it->GetSocketsCount(); count++)
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

#endif


void Player::SetPersistentInstanceId(Instance* pInstance)
{
    if (pInstance == nullptr)
        return;

    // Skip this handling for flagged GMs.
    if (hasPlayerFlags(PLAYER_FLAG_GM))
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
        SetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty, 0);
    else // Set instance id to player.
        SetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty, pInstance->m_instanceId);

    LOG_DEBUG("Added player %u to saved instance %u on map %u.", (uint32)getGuid(), pInstance->m_instanceId, pInstance->m_mapId);
}

void Player::SetPersistentInstanceId(uint32 mapId, uint8 difficulty, uint32 instanceId)
{
    if (mapId >= MAX_NUM_MAPS || difficulty >= NUM_INSTANCE_MODES || m_playerInfo == nullptr)
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
    // clear all fields
    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
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
            Field* fields = result->Fetch();
            uint32 questid = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(questid);
            slot = fields[2].GetUInt16();

            // remove on next save if bad quest
            if (!quest)
            {
                m_removequests.insert(questid);
                continue;
            }
            if (m_questlog[slot] != nullptr)
                continue;

            QuestLogEntry* entry = new QuestLogEntry;
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
    sendInstanceDifficultyPacket(m_mapMgr->iInstanceMode);
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
    sendInstanceDifficultyPacket(m_mapMgr->iInstanceMode);
#endif
}

void Player::OnPrePushToWorld()
{
    SendInitialLogonPackets();
#if VERSION_STRING > TBC
    m_achievementMgr.SendAllAchievementData(this);
#endif
}

#ifdef AE_TBC
void Player::OnPushToWorld()
{
    if (m_TeleportState == 2)   // Worldport Ack
        OnWorldPortAck();

    SpeedCheatReset();
    m_beingPushed = false;
    AddItemsToWorld();

    // delay the unlock movement packet
    WorldPacket* data = new WorldPacket(SMSG_TIME_SYNC_REQ, 4);
    *data << uint32(0);
    getUpdateMgr().queueDelayedPacket(data);

    // set fly if cheat is active
    // TODO Validate that this isn't breaking logon by messaging player without delay
#ifndef AE_TBC
    setMoveCanFly(m_cheats.FlyCheat);
#endif

    // Update PVP Situation
    LoginPvPSetup();

    // TODO What is this?
#ifndef AE_TBC
    setPvpFlags(getPvpFlags() &~(U_FIELD_BYTES_FLAG_UNK2 | U_FIELD_BYTES_FLAG_SANCTUARY));
#endif

    if (m_playerInfo->lastOnline + 900 < UNIXTIME)    // did we logged out for more than 15 minutes?
        getItemInterface()->RemoveAllConjured();

    Unit::OnPushToWorld();

    if (m_FirstLogin)
    {
        uint8 my_class = getClass();
        uint8 start_level = 1;

        start_level = static_cast<uint8>(worldConfig.player.playerStartingLevel);

        sHookInterface.OnFirstEnterWorld(this);
        LevelInfo* Info = objmgr.GetLevelInfo(getRace(), getClass(), start_level);
        ApplyLevelInfo(Info, start_level);
        setInitialTalentPoints();
        m_FirstLogin = false;
    }

    sHookInterface.OnEnterWorld(this);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnZoneChange)(this, m_zoneId, 0);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnPlayerEnter)(this);

    if (m_TeleportState == 1)        // First world enter
        CompleteLoading();

    m_TeleportState = 0;

    if (isOnTaxi())
    {
        if (m_taxiMapChangeNode != 0)
        {
            lastNode = m_taxiMapChangeNode;
        }

        // Process create packet
        ProcessPendingUpdates();

        TaxiStart(GetTaxiPath(), getMountDisplayId(), lastNode);

        m_taxiMapChangeNode = 0;
    }

    // can only fly in outlands or northrend (northrend requires cold weather flying)
    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    // send weather
    sWeatherMgr.SendWeather(this);

    setHealth(load_health > getMaxHealth() ? getMaxHealth() : load_health);
    setPower(POWER_TYPE_MANA, (load_mana > getMaxPower(POWER_TYPE_MANA) ? getMaxPower(POWER_TYPE_MANA) : load_mana));

    if (!GetSession()->HasGMPermissions())
        getItemInterface()->CheckAreaItems();

    if (m_mapMgr && m_mapMgr->m_battleground != nullptr && m_bg != m_mapMgr->m_battleground)
        m_mapMgr->m_battleground->PortPlayer(this, true);

    if (m_bg != nullptr)
    {
        m_bg->OnAddPlayer(this);   // add buffs and so, must be after zone update and related aura removal
        m_bg->OnPlayerPushed(this);
    }

    m_changingMaps = false;
    SendFullAuraUpdate();

    getItemInterface()->HandleItemDurations();

    //SendInitialWorldstates();

    if (resettalents)
    {
        Reset_AllTalents();
        resettalents = false;
    }
}
#else
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
    getUpdateMgr().queueDelayedPacket(data);

    // set fly if cheat is active
    setMoveCanFly(m_cheats.FlyCheat);

    // Update PVP Situation
    LoginPvPSetup();
    setPvpFlags(getPvpFlags() &~(U_FIELD_BYTES_FLAG_UNK2 | U_FIELD_BYTES_FLAG_SANCTUARY));

    if (m_playerInfo->lastOnline + 900 < UNIXTIME)    // did we logged out for more than 15 minutes?
        getItemInterface()->RemoveAllConjured();

    Unit::OnPushToWorld();


    sHookInterface.OnEnterWorld(this);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnZoneChange)(this, m_zoneId, 0);
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnPlayerEnter)(this);

    if (m_TeleportState == 1)        // First world enter
        CompleteLoading();

    m_TeleportState = 0;

    if (isOnTaxi())
    {
        if (m_taxiMapChangeNode != 0)
            lastNode = m_taxiMapChangeNode;

        TaxiStart(GetTaxiPath(), getMountDisplayId(), lastNode);

        m_taxiMapChangeNode = 0;
    }

    // can only fly in outlands or northrend (northrend requires cold weather flying)
    if (flying_aura && ((m_mapId != 530) && (m_mapId != 571 || !HasSpell(54197) && getDeathState() == ALIVE)))
    {
        RemoveAura(flying_aura);
        flying_aura = 0;
    }

    // send weather
    sWeatherMgr.SendWeather(this);

    setHealth(load_health > getMaxHealth() ? getMaxHealth() : load_health);
    if (getPowerType() == POWER_TYPE_MANA)
        setPower(POWER_TYPE_MANA, (load_mana > getMaxPower(POWER_TYPE_MANA) ? getMaxPower(POWER_TYPE_MANA) : load_mana));

    if (m_FirstLogin)
    {
        if (class_ == DEATHKNIGHT)
            startlevel = static_cast<uint8>(std::max(55, worldConfig.player.playerStartingLevel));
        else
            startlevel = static_cast<uint8>(worldConfig.player.playerStartingLevel);

        sHookInterface.OnFirstEnterWorld(this);
        LevelInfo* Info = objmgr.GetLevelInfo(getRace(), getClass(), startlevel);
        ApplyLevelInfo(Info, startlevel);
        setInitialTalentPoints();
        SetHealthPct(100);

        // Sometimes power types aren't initialized - so initialize it again
        switch (getClass())
        {
        case WARRIOR:
            setMaxPower(POWER_TYPE_RAGE, info->rage);
            setPower(POWER_TYPE_RAGE, 0);
            break;
        case ROGUE:
            setMaxPower(POWER_TYPE_ENERGY, info->energy);
            setPower(POWER_TYPE_ENERGY, info->energy);
            break;
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
            setMaxPower(POWER_TYPE_RUNES, 8);
            setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
            setPower(POWER_TYPE_RUNES, 8);
            break;
#endif
        default:
            setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
            if (info->focus)
            {
                setPower(POWER_TYPE_FOCUS, 0);
                setMaxPower(POWER_TYPE_FOCUS, info->focus);
            }
            break;
        }
        m_FirstLogin = false;
    }

    if (!GetSession()->HasGMPermissions())
        getItemInterface()->CheckAreaItems();

    if (m_mapMgr && m_mapMgr->m_battleground != nullptr && m_bg != m_mapMgr->m_battleground)
        m_mapMgr->m_battleground->PortPlayer(this, true);

    if (m_bg != nullptr)
    {
        m_bg->OnAddPlayer(this);   // add buffs and so, must be after zone update and related aura removal
        m_bg->OnPlayerPushed(this);
    }

    m_changingMaps = false;
    SendFullAuraUpdate();

    getItemInterface()->HandleItemDurations();

    SendInitialWorldstates();

    if (resettalents)
    {
        Reset_AllTalents();
        resettalents = false;
    }
}
#endif

void Player::RemoveFromWorld()
{
    if (raidgrouponlysent)
        event_RemoveEvents(EVENT_PLAYER_EJECT_FROM_INSTANCE);

    load_health = getHealth();
    load_mana = getPower(POWER_TYPE_MANA);

    if (m_bg)
        m_bg->RemovePlayer(this, true);

#if VERSION_STRING < Cata
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
    getItemInterface()->EmptyBuyBack();

    getSplineMgr().clearSplinePackets();

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
            ARCEMU_ASSERT(m_SummonedObject->isGameObject());
            delete m_SummonedObject;
        }
        m_SummonedObject = nullptr;
    }

    if (IsInWorld())
    {
        RemoveItemsFromWorld();
        Unit::RemoveFromWorld(false);
    }

    if (isOnTaxi())
        event_RemoveEvents(EVENT_PLAYER_TAXI_INTERPOLATE);

    m_changingMaps = true;
    m_playerInfo->lastOnline = UNIXTIME; // don't destroy conjured items yet
}

//\todo perhaps item should just have a list of mods, that will simplify code
void Player::_ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown /* = false */, bool skip_stat_apply /* = false  */)
{
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    ARCEMU_ASSERT(item != NULL);
    ItemProperties const* proto = item->getItemProperties();

    //fast check to skip mod applying if the item doesnt meat the requirements.
    if (!item->isContainer() && item->getDurability() == 0 && item->getMaxDurability() && justdrokedown == false)
        return;

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
            setid = sMySQLStore.getItemSetLinkedBonus(proto_setid);
    }
    else
    {
        setid = proto_setid;
    }


    //\todo make a config for server so they can configure which season is active season

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
                            const auto spellInfo = sSpellMgr.getSpellInfo(item_set_entry->SpellID[x]);
                            Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
                            SpellCastTargets targets;
                            targets.m_unitTarget = this->getGuid();
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
                    {
                        if (Set->itemscount == item_set_entry->itemscount[x])
                            this->RemoveAura(item_set_entry->SpellID[x], getGuid());
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
    //\todo FIX ME: can there be negative resistances from items?
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
        // Not going to put a check here since unless you put a random id/flag in the tables these should never return NULL

        // Calculating the stats correct for our level and applying them
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

        if (proto->ScalingStatsFlag & 32768 && i < 10)
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
            uint32 scaleddps = 1;

            if (ssvrow != nullptr)
                scaleddps = ssvrow->multiplier[col];

            float dpsmod;

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
        uint8 ss = getShapeShiftForm();
        if (ss == FORM_MOONKIN || ss == FORM_CAT || ss == FORM_BEAR || ss == FORM_DIREBEAR)
            this->ApplyFeralAttackPower(apply, item);
    }

    // Misc
    if (apply)
    {
        // Apply all enchantment bonuses
        item->ApplyEnchantmentBonuses();

        for (uint8 k = 0; k < 5; ++k)
        {
            if (item->getItemProperties()->Spells[k].Id == 0)
                continue;//this isn't needed since the check below handles this case but it's a lot faster performance-wise.

            SpellInfo const* spells = sSpellMgr.getSpellInfo(item->getItemProperties()->Spells[k].Id);
            if (spells == nullptr)
                continue;

            if (item->getItemProperties()->Spells[k].Trigger == ON_EQUIP)
            {
                if (spells->getRequiredShapeShift())
                {
                    AddShapeShiftSpell(spells->getId());
                    continue;
                }

                Spell* spell = sSpellMgr.newSpell(this, spells, true, nullptr);
                SpellCastTargets targets;
                targets.m_unitTarget = this->getGuid();
                spell->castedItemId = item->getEntry();
                spell->prepare(&targets);

            }
            else if (item->getItemProperties()->Spells[k].Trigger == CHANCE_ON_HIT)
            {
                this->AddProcTriggerSpell(spells, nullptr, this->getGuid(), 5, PROC_ON_MELEE_ATTACK, 0, nullptr, nullptr);
            }
        }
    }
    else
    {
        // Remove all enchantment bonuses
        item->RemoveEnchantmentBonuses();
        for (uint8 k = 0; k < 5; ++k)
        {
            if (item->getItemProperties()->Spells[k].Trigger == ON_EQUIP)
            {
                SpellInfo const* spells = sSpellMgr.getSpellInfo(item->getItemProperties()->Spells[k].Id);
                if (spells != nullptr)
                {
                    if (spells->getRequiredShapeShift())
                        RemoveShapeShiftSpell(spells->getId());
                    else
                        RemoveAura(item->getItemProperties()->Spells[k].Id);
                }
            }
            else if (item->getItemProperties()->Spells[k].Trigger == CHANCE_ON_HIT)
            {
                this->RemoveProcTriggerSpell(item->getItemProperties()->Spells[k].Id);
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
    FastGUIDPack(data, getGuid());         // caster guid
    GetSession()->SendPacket(&data);

    // Cleanup first
    uint32 AuraIds[] = { 20584, 9036, 8326, 0 };
    RemoveAuras(AuraIds); // Cebernic: Removeaura just remove once(bug?).

    setHealth(1);

    SpellCastTargets tgt;
    tgt.m_unitTarget = this->getGuid();

    if (getRace() == RACE_NIGHTELF)
    {
        SpellInfo const* inf = sSpellMgr.getSpellInfo(9036);
        Spell* sp = sSpellMgr.newSpell(this, inf, true, nullptr);
        sp->prepare(&tgt);
    }
    else
    {
        SpellInfo const* inf = sSpellMgr.getSpellInfo(8326);
        Spell* sp = sSpellMgr.newSpell(this, inf, true, nullptr);
        sp->prepare(&tgt);
    }

    sendStopMirrorTimerPacket(MIRROR_TYPE_FATIGUE);
    sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
    sendStopMirrorTimerPacket(MIRROR_TYPE_FIRE);

    addPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE);

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
        Corpse* myCorpse = objmgr.GetCorpseByOwner(getGuidLow());
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
#if VERSION_STRING < Cata
        this->obj_movement_info.transport_data.transportGuid = 0;
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
    removeUnitFlags(UNIT_FLAG_SKINNABLE);

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
                RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
            else
                RepopAtGraveyard(pMapinfo->repopx, pMapinfo->repopy, pMapinfo->repopz, pMapinfo->repopmapid);

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
            Corpse* myCorpse = objmgr.GetCorpseByOwner(getGuidLow());
            if (myCorpse != nullptr)
                myCorpse->ResetDeathClock();
        }

        // Send Spirit Healer Location
        m_session->SendPacket(SmsgDeathReleaseLoc(m_mapId, m_position).serialise().get());

        // Corpse reclaim delay
        m_session->SendPacket(SmsgCorpseReclaimDelay(CORPSE_RECLAIM_TIME_MS).serialise().get());
    }
}

void Player::ResurrectPlayer()
{
    if (!sHookInterface.OnResurrect(this))
        return;

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FORCED_RESURRECT); // In case somebody resurrected us before this event happened
    if (m_resurrectHealth)
        setHealth((uint32)std::min(m_resurrectHealth, getMaxHealth()));
    if (m_resurrectMana)
        setPower(POWER_TYPE_MANA, m_resurrectMana);

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();

    RemoveNegativeAuras();
    uint32 AuraIds[] = { 20584, 9036, 8326, 55164, 0 };
    RemoveAuras(AuraIds); // Cebernic: removeaura just remove once(bug?).

    removePlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE);
    setDeathState(ALIVE);
    UpdateVisibility();

    // Don't pull players inside instances with this trick. Also fixes the part where you were able to double item bonuses
    if (m_resurrecter && IsInWorld() && m_resurrectInstanceID == static_cast<uint32>(GetInstanceID()))
        SafeTeleport(m_resurrectMapId, m_resurrectInstanceID, m_resurrectPosition);

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
    sendStopMirrorTimerPacket(MIRROR_TYPE_FATIGUE);
    sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
    sendStopMirrorTimerPacket(MIRROR_TYPE_FIRE);

    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE); // Player death animation, also can be used with DYNAMIC_FLAGS <- huh???
    //\note remove all dynamic flags
    setDynamicFlags(0);

    if (getClass() == WARRIOR)   // Rage resets on death
        setPower(POWER_TYPE_RAGE, 0);
#if VERSION_STRING == WotLK
    else if (getClass() == DEATHKNIGHT)
        setPower(POWER_TYPE_RUNIC_POWER, 0);
#endif

    summonhandler.RemoveAllSummons();
    DismissActivePets();

    // Player falls off vehicle on death
    if (m_currentVehicle != nullptr)
        m_currentVehicle->EjectPassenger(this);

    sHookInterface.OnDeath(this);
}

void Player::CreateCorpse()
{
    objmgr.DelinkPlayerCorpses(this);
    if (!bCorpseCreateable)
    {
        bCorpseCreateable = true;   // For next time
        return; // No corpse allowed!
    }

    Corpse* pCorpse = objmgr.CreateCorpse();
    pCorpse->SetInstanceID(GetInstanceID());
    pCorpse->Create(this, GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());

    pCorpse->SetZoneId(GetZoneId());

    //bytes1
    pCorpse->setRace(getRace());
    pCorpse->setSkinColor(getSkinColor());

    //bytes2
    pCorpse->setFace(getFace());
    pCorpse->setHairStyle(getHairStyle());
    pCorpse->setHairColor(getHairColor());
    pCorpse->setFacialFeatures(getFacialFeatures());

    pCorpse->setFlags(CORPSE_FLAG_UNK1);

    pCorpse->setDisplayId(getDisplayId());

    if (m_bg)
    {
        // Remove our lootable flags
        removeDynamicFlags(U_DYN_FLAG_LOOTABLE);
        removeUnitFlags(UNIT_FLAG_SKINNABLE);

        loot.gold = 0;

        pCorpse->generateLoot();
        if (bShouldHaveLootableOnCorpse)
            pCorpse->setDynamicFlags(1); // sets it so you can loot the plyr
        else // Hope this works
            pCorpse->setFlags(CORPSE_FLAG_UNK1 | CORPSE_FLAG_HIDDEN_HELM | CORPSE_FLAG_HIDDEN_CLOAK | CORPSE_FLAG_LOOT);

        // Now that our corpse is created, don't do it again
        bShouldHaveLootableOnCorpse = false;
    }
    else
    {
        pCorpse->loot.gold = 0;
    }

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* pItem = getItemInterface()->GetInventoryItem(i))
        {
            uint32 iDisplayID = pItem->getItemProperties()->DisplayInfoID;
            uint16 iIventoryType = (uint16)pItem->getItemProperties()->InventoryType;

            uint32 _cfi = (uint16(iDisplayID)) | (iIventoryType) << 24;
            pCorpse->setItem(i, _cfi);
        }
    }
    // Save corpse in db for future use
    pCorpse->SaveToDB();
}

void Player::SpawnCorpseBody()
{
    Corpse* pCorpse = objmgr.GetCorpseByOwner(this->getGuidLow());
    if (pCorpse)
    {
        if (!pCorpse->IsInWorld())
        {
            if (bShouldHaveLootableOnCorpse && pCorpse->getDynamicFlags() != 1)
                pCorpse->setDynamicFlags(1); // sets it so you can loot the plyr

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
        myCorpseLocation.ChangeCoords({ 0, 0, 0, 0 });
        myCorpseInstanceId = 0;
    }
}

void Player::SpawnCorpseBones()
{
    Corpse* pCorpse = objmgr.GetCorpseByOwner(getGuidLow());
    myCorpseLocation.ChangeCoords({ 0, 0, 0, 0 });
    myCorpseInstanceId = 0;
    if (pCorpse)
    {
        if (pCorpse->IsInWorld() && pCorpse->GetCorpseState() == CORPSE_STATE_BODY)
        {
            if (pCorpse->GetInstanceID() != GetInstanceID())
                sEventMgr.AddEvent(pCorpse, &Corpse::SpawnBones, EVENT_CORPSE_SPAWN_BONES, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if (Item* pItem = getItemInterface()->GetInventoryItem(i))
        {
            uint32 pMaxDurability = pItem->getMaxDurability();
            uint32 pDurability = pItem->getDurability();
            if (pDurability)
            {
                int32 pNewDurability = (uint32)(pMaxDurability * percent);
                pNewDurability = (pDurability - pNewDurability);
                if (pNewDurability < 0)
                    pNewDurability = 0;

                if (pNewDurability <= 0)
                    ApplyItemMods(pItem, i, false, true);

                pItem->setDurability(static_cast<uint32>(pNewDurability));
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

    if (!m_bg || !m_bg->HookHandleRepop(this))
    {
        MySQLStructure::Graveyards const* pGrave = nullptr;
        MySQLDataStore::GraveyardsContainer const* its = sMySQLStore.getGraveyardsStore();
        for (MySQLDataStore::GraveyardsContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            pGrave = sMySQLStore.getGraveyard(itr->second.id);
            if (pGrave->mapId == mapid && (pGrave->factionId == getTeam() || pGrave->factionId == 3))
            {
                temp.ChangeCoords({ pGrave->position_x, pGrave->position_y, pGrave->position_z });
                float dist = src.distanceSquare(temp);
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
            dest.ChangeCoords({ pGrave->position_x, pGrave->position_y, pGrave->position_z });
            first = false;
        }
    }
    else
    {
        return;
    }

    if (sHookInterface.OnRepop(this) && !first)//dest has now always a value != {0,0,0,0}//but there may be DBs with no graveyards
    {
        SafeTeleport(mapid, 0, dest);
    }

    //\todo Generate error message here, compensate for failed teleport.
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
    for (std::set<Channel*>::iterator i = m_channels.begin(); i != m_channels.end();)
    {
        Channel* c = *i;
        ++i;

        c->Part(this);
    }
}

#if VERSION_STRING <= TBC
void Player::SendInitialActions()
{
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);

    for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        data << getActiveSpec().mActions[i].Action;
        data << getActiveSpec().mActions[i].Type;
        data << getActiveSpec().mActions[i].Misc;
    }
    m_session->SendPacket(&data);
}
#else
void Player::SendInitialActions()
{
#if VERSION_STRING == Mop
    WorldPacket data(SMSG_ACTION_BUTTONS, (PLAYER_ACTION_BUTTON_COUNT * 8) + 1);

    uint8_t buttons[PLAYER_ACTION_BUTTON_COUNT][8];

    // Bits
    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][4]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][5]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][3]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][1]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][6]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][7]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][0]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][2]);

    // Data
    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][0]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][1]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][4]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][6]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][7]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][2]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][5]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][3]);

    data << uint8_t(0);

#elif VERSION_STRING != Mop
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_ACTION_BUTTONS, (PLAYER_ACTION_BUTTON_COUNT * 4) + 1);
#else
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);
#endif

#if VERSION_STRING < Cata
    data << uint8(0);         // VLack: 3.1, some bool - 0 or 1. seems to work both ways
#endif

    for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        data << m_specs[m_talentActiveSpec].mActions[i].Action;
        data << m_specs[m_talentActiveSpec].mActions[i].Misc; // 3.3.5 Misc have to be sent before Type (buttons with value over 0xFFFF)
        data << m_specs[m_talentActiveSpec].mActions[i].Type;
    }
#if VERSION_STRING >= Cata
    data << uint8(1);
#endif
#endif
    m_session->SendPacket(&data);
}
#endif

void Player::setAction(uint8 button, uint16 action, uint8 type, uint8 misc)
{
#if VERSION_STRING > TBC
    if (button >= PLAYER_ACTION_BUTTON_COUNT)
        return;
#else
    if (button >= 120)
        return;
#endif

    getActiveSpec().mActions[button].Action = action;
    getActiveSpec().mActions[button].Misc = misc;
    getActiveSpec().mActions[button].Type = type;
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
    return m_finishedQuests.find(quest_id) != m_finishedQuests.end();
}

bool Player::HasTimedQuest()
{
    for (uint8 i = 0; i < MAX_QUEST_SLOT; i++)
        if (m_questlog[i] != nullptr && m_questlog[i]->GetQuest()->time != 0)
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
            CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerid = %u;", getGuidLow());
            CharacterDatabase.Execute("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", getGuidLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
        }
        else
        {
            buf->AddQuery("DELETE FROM tutorials WHERE playerid = %u;", getGuidLow());
            buf->AddQuery("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", getGuidLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
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
    uint32 agi = getStat(STAT_AGILITY);

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
    Item* it = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it != nullptr && it->getItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        tmp = GetBlockChance();
        tmp += defence_contribution;
        tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    }
    else
        tmp = 0.0f;

    setFloatValue(PLAYER_BLOCK_PERCENTAGE, tmp);

    // Parry (can only parry with something in main hand)
    it = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
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

    tmp = 100 * (baseCrit->val + getStat(STAT_AGILITY) * CritPerAgi->val);

    float melee_bonus = 0;
    float ranged_bonus = 0;

    if (tocritchance.size() > 0)    // Crashfix by Cebernic
    {
        Item* tItemMelee = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        Item* tItemRanged = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);

        //-1 = any weapon

        for (std::map< uint32, WeaponModifier >::iterator itr = tocritchance.begin(); itr != tocritchance.end(); ++itr)
        {
            if (itr->second.wclass == (uint32)-1 || (tItemMelee != nullptr && (1 << tItemMelee->getItemProperties()->SubClass & itr->second.subclass)))
                melee_bonus += itr->second.value;

            if (itr->second.wclass == (uint32)-1 || (tItemRanged != nullptr && (1 << tItemRanged->getItemProperties()->SubClass & itr->second.subclass)))
                ranged_bonus += itr->second.value;
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

    spellcritperc = 100 * (SpellCritBase->val + getStat(STAT_INTELLECT) * SpellCritPerInt->val) +
        this->GetSpellCritFromSpell() +
        this->CalcRating(PCR_SPELL_CRIT);
    UpdateChanceFields();
}

void Player::UpdateChanceFields()
{
#if VERSION_STRING != Classic
    // Update spell crit values in fields
    for (uint8 i = 0; i < 7; ++i)
        setFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, SpellCritChanceSchool[i] + spellcritperc);
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
        setModCastSpeed(1.0f / (m_attack_speed[MOD_SPELL] * SpellHasteRatingBonus));
}

void Player::UpdateAttackSpeed()
{
    uint32 speed = 2000;
    Item* weap;

    if (getShapeShiftForm() == FORM_CAT)
    {
        speed = 1000;
    }
    else if (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR)
    {
        speed = 2500;
    }
    else if (!disarmed)  // Regular
    {
        weap = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (weap != nullptr)
            speed = weap->getItemProperties()->Delay;
    }
    setBaseAttackTime(MELEE,
                      (uint32)((float)speed / (m_attack_speed[MOD_MELEE] * (1.0f + CalcRating(PCR_MELEE_HASTE) / 100.0f))));

    weap = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (weap != nullptr && weap->getItemProperties()->Class == ITEM_CLASS_WEAPON)
    {
        speed = weap->getItemProperties()->Delay;
        setBaseAttackTime(OFFHAND,
                          (uint32)((float)speed / (m_attack_speed[MOD_MELEE] * (1.0f + CalcRating(PCR_MELEE_HASTE) / 100.0f))));
    }

    weap = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (weap != nullptr)
    {
        speed = weap->getItemProperties()->Delay;
        setBaseAttackTime(RANGED,
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

    uint32 str = getStat(STAT_STRENGTH);
    uint32 agi = getStat(STAT_AGILITY);
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

            if (getShapeShiftForm() == FORM_MOONKIN)
            {
                //(Strength x 2) + (Character Level x 1.5) - 20
                AP += float2int32(static_cast<float>(lev)* 1.5f);
            }
            if (getShapeShiftForm() == FORM_CAT)
            {
                //(Strength x 2) + Agility + (Character Level x 2) - 20
                AP += agi + (lev * 2);
            }
            if (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR)
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
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
            //(Strength x 2) + (Character Level x 3) - 20
            AP = (str * 2) + (lev * 3) - 20;
            //Character Level + Agility - 10
            RAP = lev + agi - 10;

            break;
#endif
        default:    //mage,priest,warlock
            AP = agi - 10;
    }

    /* modifiers */
    RAP += m_rap_mod_pct * getStat(STAT_INTELLECT) / 100;

    if (RAP < 0)
        RAP = 0;

    if (AP < 0)
        AP = 0;

    setAttackPower(AP);
    setRangedAttackPower(RAP);

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

    uint32 hp = getBaseHealth();

#if VERSION_STRING != Classic
    int32 stat_bonus = getPosStat(STAT_STAMINA) - getNegStat(STAT_STAMINA);
#else
    int32 stat_bonus = 0;
#endif
    if (stat_bonus < 0)
        stat_bonus = 0; // Avoid of having negative health
    int32 bonus = stat_bonus * 10 + m_healthfromspell + m_healthfromitems;

    uint32 res = hp + bonus + hpdelta;
    uint32 oldmaxhp = getMaxHealth();

    if (res < hp)
        res = hp;

    if (worldConfig.limit.isLimitSystemEnabled && (worldConfig.limit.maxHealthCap > 0) && (res > worldConfig.limit.maxHealthCap) && GetSession()->GetPermissionCount() <= 0)   //hacker?
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxArenaPoints << " health " << res;

        sCheatLog.writefromsession(GetSession(), dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)
            sendReportToGmMessage(getName().c_str(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
            GetSession()->Disconnect();
        else // no disconnect, set it to the cap instead
            res = worldConfig.limit.maxHealthCap;
    }
    setMaxHealth(res);

    if (getHealth() > res)
    {
        setHealth(res);
    }
    else if (cl == DRUID && (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR))
    {
        res = getMaxHealth() * getHealth() / oldmaxhp;
        setHealth(res);
    }

    if (cl != WARRIOR && cl != ROGUE
#if VERSION_STRING > TBC
        && cl != DEATHKNIGHT
#endif
        )
    {
        // MP
        uint32 mana = getBaseMana();
#if VERSION_STRING != Classic
        stat_bonus = getPosStat(STAT_INTELLECT) - getNegStat(STAT_INTELLECT);
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
                sendReportToGmMessage(getName().c_str(), logmsg);

            if (worldConfig.limit.disconnectPlayerForExceedingLimits)
                GetSession()->Disconnect();
            else // no disconnect, set it to the cap instead
                res = worldConfig.limit.maxManaCap;
        }
        setMaxPower(POWER_TYPE_MANA, res);

        if (getPower(POWER_TYPE_MANA) > res)
            setPower(POWER_TYPE_MANA, res);

        uint32 level = getLevel();
        if (level > DBC_PLAYER_LEVEL_CAP)
            level = DBC_PLAYER_LEVEL_CAP;
        //float amt = (0.001f + sqrt((float)Intellect) * Spirit * getBaseManaRegen(level))*PctPowerRegenModifier[POWER_TYPE_MANA];

        // Mesmer: new Manaregen formula.
        uint32 Spirit = getStat(STAT_SPIRIT);
        uint32 Intellect = getStat(STAT_INTELLECT);
        float amt = (0.001f + sqrt((float)Intellect) * Spirit * getBaseManaRegen(level)) * PctPowerRegenModifier[POWER_TYPE_MANA];
#if VERSION_STRING > TBC
        setFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, amt + m_ModInterrMRegen * 0.2f);
        setFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, amt * m_ModInterrMRegenPCT / 100.0f + m_ModInterrMRegen * 0.2f);
#elif VERSION_STRING == TBC
        const static float BaseRegen[70] = { 0.034965f, 0.034191f, 0.033465f, 0.032526f, 0.031661f, 0.031076f, 0.030523f, 0.029994f, 0.029307f, 0.028661f, 0.027584f, 0.026215f, 0.025381f, 0.024300f, 0.023345f, 0.022748f, 0.021958f, 0.021386f, 0.020790f, 0.020121f, 0.019733f, 0.019155f, 0.018819f, 0.018316f, 0.017936f, 0.017576f, 0.017201f, 0.016919f, 0.016581f, 0.016233f, 0.015994f, 0.015707f, 0.015464f, 0.015204f, 0.014956f, 0.014744f, 0.014495f, 0.014302f, 0.014094f, 0.013895f, 0.013724f, 0.013522f, 0.013363f, 0.013175f, 0.012996f, 0.012853f, 0.012687f, 0.012539f, 0.012384f, 0.012233f, 0.012113f, 0.011973f, 0.011859f, 0.011714f, 0.011575f, 0.011473f, 0.011342f, 0.011245f, 0.011110f, 0.010999f, 0.010700f, 0.010522f, 0.010290f, 0.010119f, 0.009968f, 0.009808f, 0.009651f, 0.009553f, 0.009445f, 0.009327f };
        if (level > 70)
            level = 70;

        float BCamt = (0.001f + sqrt((float)Intellect) * Spirit * BaseRegen[level - 1])*PctPowerRegenModifier[POWER_TYPE_MANA];
        setFloatValue(PLAYER_FIELD_MOD_MANA_REGEN, BCamt + m_ModInterrMRegen / 5.0f);
        setFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT, amt * m_ModInterrMRegenPCT / 100.0f + m_ModInterrMRegen / 5.0f);
#endif
    }

    // Spell haste rating
    float haste = 1.0f + CalcRating(PCR_SPELL_HASTE) / 100.0f;
    if (haste != SpellHasteRatingBonus)
    {
        float value = getModCastSpeed() * SpellHasteRatingBonus / haste; // remove previous mod and apply current

        setModCastSpeed(value);
        SpellHasteRatingBonus = haste;    // keep value for next run
    }

    // Shield Block
    Item* shield = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (shield != nullptr && shield->getItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        float block_multiplier = (100.0f + m_modblockabsorbvalue) / 100.0f;
        if (block_multiplier < 1.0f)
            block_multiplier = 1.0f;

        int32 blockable_damage = float2int32((shield->getItemProperties()->Block + m_modblockvaluefromspells + getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + (str / 2.0f) - 1.0f) * block_multiplier);
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
    {
        if (m_auras[x] != nullptr)
        {
            if (m_auras[x]->HasModType(SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT) || m_auras[x]->HasModType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT))
                m_auras[x]->UpdateModifiers();
        }
    }

    UpdateChances();
    CalcDamage();
}

uint32 Player::SubtractRestXP(uint32 amount)
{
    if (getLevel() >= getMaxLevel()) // Save CPU, don't waste time on this if you've reached max_level
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
    uint32 xp_to_lvl = getNextLevelXp();

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
    if (m_restAmount && getLevel() < getMaxLevel())
        m_restState = RESTSTATE_RESTED;
    else
        m_restState = RESTSTATE_NORMAL;

    // Update RestState 100%/200%
    setRestState(m_restState);

    //update needle (weird, works at 1/2 rate)
    setUInt32Value(PLAYER_REST_STATE_EXPERIENCE, m_restAmount >> 1);
}

void Player::ApplyPlayerRestState(bool apply)
{
    if (apply)
    {
        m_restState = RESTSTATE_RESTED;
        m_isResting = true;
        addPlayerFlags(PLAYER_FLAG_RESTING);    //put zZz icon
    }
    else
    {
        m_isResting = false;
        removePlayerFlags(PLAYER_FLAG_RESTING);    //remove zZz icon
    }
    UpdateRestState();
}

void Player::addToInRangeObjects(Object* pObj)
{
    //Send taxi move if we're on a taxi
    if (m_CurrentTaxiPath && pObj->isPlayer())
    {
        uint32 ntime = Util::getMSTime();

        if (ntime > m_taxi_ride_time)
            m_CurrentTaxiPath->SendMoveForTime(this, static_cast< Player* >(pObj), ntime - m_taxi_ride_time);
        /*else
            m_CurrentTaxiPath->SendMoveForTime(this, TO< Player* >(pObj), m_taxi_ride_time - ntime);*/
    }

    Unit::addToInRangeObjects(pObj);

    //if the object is a unit send a move packet if they have a destination
    if (pObj->isCreature())
        static_cast< Creature* >(pObj)->GetAIInterface()->SendCurrentMove(this);
}

void Player::onRemoveInRangeObject(Object* pObj)
{
    //object was deleted before reaching here
    if (pObj == nullptr)
        return;

    if (IsVisible(pObj->getGuid()))
    {
        getUpdateMgr().pushOutOfRangeGuid(pObj->GetNewGUID());
    }

    m_visibleObjects.erase(pObj->getGuid());
    Unit::onRemoveInRangeObject(pObj);

    if (pObj->getGuid() == m_CurrentCharm)
    {
        Unit* p = GetMapMgr()->GetUnit(m_CurrentCharm);
        if (!p)
            return;

        UnPossess();
        if (isCastingSpell())
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

    if (pObj->getGuid() == getSummonGuid())
        sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::RemoveFieldSummon, EVENT_SUMMON_EXPIRE, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);//otherwise Creature::Update() will access free'd memory
}

void Player::clearInRangeSets()
{
    m_visibleObjects.clear();
    Unit::clearInRangeSets();
}

void Player::EventCannibalize(uint32 amount)
{
    if (getChannelSpellId() != 20577)
    {
        sEventMgr.RemoveEvents(this, EVENT_CANNIBALIZE);
        cannibalize = false;
        cannibalizeCount = 0;
        return;
    }

    uint32 amt = (getMaxHealth() * amount) / 100;

    uint32 newHealth = getHealth() + amt;
    if (newHealth <= getMaxHealth())
        setHealth(newHealth);
    else
        setHealth(getMaxHealth());

    cannibalizeCount++;
    if (cannibalizeCount == 5)
        setEmoteState(EMOTE_ONESHOT_NONE);

    SendPeriodicHealAuraLog(GetNewGUID(), GetNewGUID(), 20577, amt, 0, false);
}

// The player sobers by 256 every 10 seconds
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
    uint32 oldDrunkenState = GetDrunkenstateByValue(m_drunk);

    m_drunk = newDrunkenValue;
    setDrunkValue(m_drunk);

    uint32 newDrunkenState = GetDrunkenstateByValue(m_drunk);

    if (newDrunkenState == oldDrunkenState)
        return;

    // special drunk invisibility detection
    if (newDrunkenState >= DRUNKEN_DRUNK)
        modInvisibilityDetection(INVIS_FLAG_DRUNK, 100);
    else
        modInvisibilityDetection(INVIS_FLAG_DRUNK, -getInvisibilityDetection(INVIS_FLAG_DRUNK));

    UpdateVisibility();

    sendNewDrunkStatePacket(newDrunkenState, itemId);
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
    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (m_questlog[i] != nullptr)
        {
            QuestProperties const* qst = m_questlog[i]->GetQuest();

            // Check the item_quest_association table for an entry related to this item
            QuestAssociationList* tempList = QuestMgr::getSingleton().GetQuestAssociationListForItemId(itemid);
            if (tempList != nullptr)
            {
                QuestAssociationList::iterator it;
                for (it = tempList->begin(); it != tempList->end(); ++it)
                {
                    if (((*it)->qst == qst) && (getItemInterface()->GetItemCount(itemid) < (*it)->item_count))
                    {
                        return true;
                    } // end if
                } // end for
            } // end if

            // No item_quest association found, check the quest requirements
            if (!qst->count_required_item)
                continue;

            for (uint32 j = 0; j < MAX_REQUIRED_QUEST_ITEM; ++j)
                if (qst->required_item[j] == itemid && (getItemInterface()->GetItemCount(itemid) < qst->required_itemcount[j]))
                    return true;
        }
    }
    return false;
}

void Player::EventAllowTiggerPort(bool enable)
{
    m_AllowAreaTriggerPort = enable;
}

uint32 Player::CalcTalentResetCost(uint32 resetnum)
{

    if (resetnum == 0)
        return  10000;

    if (resetnum > 10)
        return  500000;

    return resetnum * 50000;
}

int32 Player::CanShootRangedWeapon(uint32 spellid, Unit* target, bool autoshot)
{
    SpellInfo const* spell_info = sSpellMgr.getSpellInfo(spellid);
    if (spell_info == nullptr)
        return -1;

    LogDebugFlag(LF_SPELL, "Canshootwithrangedweapon!?!? spell: [%u] %s" , spell_info->getId() , spell_info->getName().c_str());

    // Player has clicked off target. Fail spell.
    if (m_curSelection != getCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets.m_unitTarget)
        return SPELL_FAILED_INTERRUPTED;

    // Supalosa - The hunter ability Auto Shot is using Shoot range, which is 5 yards shorter.
    // So we'll use 114, which is the correct 35 yard range used by the other Hunter abilities (arcane shot, concussive shot...)
    uint8 fail = 0;
    uint32 rIndex = autoshot ? 114 : spell_info->getRangeIndex();

    auto spell_range = sSpellRangeStore.LookupEntry(rIndex);
    float minrange = GetMinRange(spell_range);
    float dist = CalcDistance(this, target);
    float maxr = GetMaxRange(spell_range) + 2.52f;

    spellModFlatFloatValue(this->SM_FRange, &maxr, spell_info->getSpellFamilyFlags());
    spellModPercentageFloatValue(this->SM_PRange, &maxr, spell_info->getSpellFamilyFlags());

    //float bonusRange = 0;
    // another hackfix: bonus range from hunter talent hawk eye: +2/4/6 yard range to ranged weapons
    //if (autoshot)
    //spellModFlatFloatValue(SM_FRange, &bonusRange, sSpellMgr.getSpellInfo(75)->SpellGroupType); // HORRIBLE hackfixes :P
    // Partha: +2.52yds to max range, this matches the range the client is calculating.
    // see extra/supalosa_range_research.txt for more info
    //bonusRange = 2.52f;
    //LogDefault("Bonus range = %f" , bonusRange);

    // Check for too close
    if (spellid != SPELL_RANGED_WAND)  //no min limit for wands
        if (minrange > dist)
            fail = SPELL_FAILED_TOO_CLOSE;

    if (dist > maxr)
    {
        LogDebug("Auto shot failed: out of range (Maxr: %f, Dist: %f)" , maxr , dist);
        fail = SPELL_FAILED_OUT_OF_RANGE;
    }

    if (fail == 0)
        fail = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->canCast(true, 0, 0);

    return fail;
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
    for (SpellSet::iterator iter = mSpells.begin(); iter != mSpells.end();)
    {
        SpellSet::iterator it = iter++;
        uint32 SpellID = *it;
        SpellInfo const* e = sSpellMgr.getSpellInfo(SpellID);
        if (e->custom_NameHash == hash)
        {
            if (info->spell_list.find(e->getId()) != info->spell_list.end())
                continue;

            RemoveAura(SpellID, getGuid());

            m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);

            mSpells.erase(it);
        }
    }

    for (SpellSet::iterator iter = mDeletedSpells.begin(); iter != mDeletedSpells.end();)
    {
        SpellSet::iterator it = iter++;
        uint32 SpellID = *it;
        SpellInfo const* e = sSpellMgr.getSpellInfo(SpellID);
        if (e->custom_NameHash == hash)
        {
            if (info->spell_list.find(e->getId()) != info->spell_list.end())
                continue;

            RemoveAura(SpellID, getGuid());

            m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);

            mDeletedSpells.erase(it);
        }
    }
}

bool Player::removeSpell(uint32 SpellID, bool MoveToDeleted, bool SupercededSpell, uint32 SupercededSpellID)
{
    SpellSet::iterator iter = mSpells.find(SpellID);
    if (iter != mSpells.end())
    {
        mSpells.erase(iter);
        RemoveAura(SpellID, getGuid());
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

    // Dual Wield skills
    // these must be set false here instead because this function is called from many different places
    // and player can end up being without dual wield but still able to dual wield
    const auto spellInfo = sSpellMgr.getSpellInfo(SpellID);
    if (spellInfo->hasEffect(SPELL_EFFECT_DUAL_WIELD))
        setDualWield(false);

    if (spellInfo->hasEffect(SPELL_EFFECT_DUAL_WIELD_2H))
        setDualWield2H(false);

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
        spelllist.push_back((*itr));

    for (std::list<uint32>::iterator itr = spelllist.begin(); itr != spelllist.end(); ++itr)
        removeSpell((*itr), false, false, 0);

    for (std::set<uint32>::iterator sp = playerCreateInfo->spell_list.begin(); sp != playerCreateInfo->spell_list.end(); ++sp)
    {
        if (*sp)
            addSpell(*sp);
    }

    // cebernic ResetAll ? don't forget DeletedSpells
    mDeletedSpells.clear();
}

void Player::Reset_AllTalents()
{
    uint8 originalspec = m_talentActiveSpec;
    resetTalents();

    if (originalspec == SPEC_PRIMARY)
        activateTalentSpec(SPEC_SECONDARY);
    else
        activateTalentSpec(SPEC_PRIMARY);

    resetTalents();
    activateTalentSpec(originalspec);
}

void Player::CalcResistance(uint8_t type)
{
    ARCEMU_ASSERT(type < 7);
    int32 pos = (BaseResistance[type] * BaseResistanceModPctPos[type]) / 100;
    int32 neg = (BaseResistance[type] * BaseResistanceModPctNeg[type]) / 100;

    pos += FlatResistanceModifierPos[type];
    neg += FlatResistanceModifierNeg[type];
    int32 res = BaseResistance[type] + pos - neg;
    if (type == 0)
        res += getStat(STAT_AGILITY) * 2; //fix armor from agi
    if (res < 0)
        res = 0;
    pos += (res * ResistanceModPctPos[type]) / 100;
    neg += (res * ResistanceModPctNeg[type]) / 100;
    res = pos - neg + BaseResistance[type];
    if (type == 0)
        res += getStat(STAT_AGILITY) * 2; //fix armor from agi

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
    setResistance(type, res > 0 ? res : 0);

    std::list<Pet*> summons = GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        (*itr)->CalcResistance(type);  //Re-calculate pet's too.

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
    for (const auto& itr : getInRangeObjectsSet())
    {
        Object* obj = itr;
        if (obj && obj->isGameObject())
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
                            if (qle->GetQuest()->required_mob_or_go[i] == static_cast<int32>(go->getEntry()) &&
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
                            if (getItemInterface()->GetItemCount(it2->first) < it2->second)
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
            if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            {
                GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(go);

                if (go_quest_giver->HasQuests() && go_quest_giver->NumOfQuests() > 0)
                {
                    for (std::list<QuestRelation*>::iterator itr2 = go_quest_giver->QuestsBegin(); itr2 != go_quest_giver->QuestsEnd(); ++itr2)
                    {
                        QuestRelation* qr = (*itr2);

                        uint32 status = sQuestMgr.CalcQuestStatus(nullptr, this, qr->qst, qr->type, false);
                        if (status == QuestStatus::AvailableChat
                            || (qr->type & QUESTGIVER_QUEST_START && (status == QuestStatus::Available || status == QuestStatus::Repeatable))
                            || (qr->type & QUESTGIVER_QUEST_END && status == QuestStatus::Finished))
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
                EventDeActivateGameObject(static_cast<GameObject*>(itr));
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

    if (m_currentVehicle != nullptr)
        m_currentVehicle->EjectPassenger(this);

    //also remove morph spells
    if (getDisplayId() != getNativeDisplayId())
    {
        RemoveAllAuraType(SPELL_AURA_TRANSFORM);
        RemoveAllAuraType(SPELL_AURA_MOD_SHAPESHIFT);
    }

    DismissActivePets();

    setMountDisplayId(modelid);
    addUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
    addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

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
#if VERSION_STRING >= Cata
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
    if (!pathnode)
        return;

    modCoinage(-(int32)path->GetPrice());

    SetTaxiState(false);
    SetTaxiPath(nullptr);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    setMountDisplayId(0);
    removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
    removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

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

void Player::CalcStat(uint8_t type)
{
    ARCEMU_ASSERT(type < 5);

    int32 pos = (int32)((int32)BaseStats[type] * (int32)StatModPctPos[type]) / 100 + (int32)FlatStatModPos[type];
    int32 neg = (int32)((int32)BaseStats[type] * (int32)StatModPctNeg[type]) / 100 + (int32)FlatStatModNeg[type];
    int32 res = pos + (int32)BaseStats[type] - neg;
    if (res <= 0)
        res = 1;

    pos += (res * (int32)static_cast< Player* >(this)->TotalStatModPctPos[type]) / 100;
    neg += (res * (int32)static_cast< Player* >(this)->TotalStatModPctNeg[type]) / 100;
    res = pos + BaseStats[type] - neg;
    if (res <= 0)
        res = 1;

#if VERSION_STRING != Classic
    setPosStat(type, pos);

    if (neg < 0)
        setNegStat(type, -neg);
    else
        setNegStat(type, neg);
#endif

    setStat(type, res);
    if (type == STAT_AGILITY)
        CalcResistance(0);

    if (type == STAT_STAMINA || type == STAT_INTELLECT)
    {
        std::list<Pet*> summons = GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
            (*itr)->CalcStat(type);  //Re-calculate pet's too
    }
}

void Player::RegenerateMana(bool is_interrupted)
{
    uint32 cur = getPower(POWER_TYPE_MANA);
    uint32 mm = getMaxPower(POWER_TYPE_MANA);
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

    if (amt <= 1.0 && amt > 0) //this fixes regen like 0.98
    {
        if (is_interrupted)
            return;
        ++cur;
    }
    else
        cur += float2int32(amt);

    //    Unit::setPower() will call Object::SetUInt32Value(), which will (for players) call SendPowerUpdate(),
    //    which can be slightly out-of-sync with client regeneration [with latency] (and wastes packets since client can handle this on its own)
    if (wrate != 1.0) // our config has custom regen rate, so send new amount to player's client
    {
        setPower(POWER_TYPE_MANA, cur);
    }
    else // let player's own client handle normal regen rates.
    {
        setPower(POWER_TYPE_MANA, ((cur >= mm) ? mm : cur));
        SendPowerUpdate(false); // send update to other in-range players
    }
}

void Player::RegenerateHealth(bool inCombat)
{
    uint32 cur = getHealth();
    uint32 mh = getMaxHealth();

    if (cur == 0)
        return;   // cebernic: bugfix dying but regenerated?

    if (cur >= mh)
        return;

#if VERSION_STRING < Cata
    auto HPRegenBase = sGtRegenHPPerSptStore.LookupEntry(getLevel() - 1 + (getClass() - 1) * 100);
    if (HPRegenBase == nullptr)
        HPRegenBase = sGtRegenHPPerSptStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (getClass() - 1) * 100);

    auto HPRegen = sGtOCTRegenHPStore.LookupEntry(getLevel() - 1 + (getClass() - 1) * 100);
    if (HPRegen == nullptr)
        HPRegen = sGtOCTRegenHPStore.LookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (getClass() - 1) * 100);
#endif

    uint32 basespirit = getStat(STAT_SPIRIT);
    uint32 extraspirit = 0;

    if (basespirit > 50)
    {
        extraspirit = basespirit - 50;
        basespirit = 50;
    }

#if VERSION_STRING < Cata
    float amt = basespirit * HPRegen->ratio + extraspirit * HPRegenBase->ratio;
#else
    float amt = static_cast<float>(basespirit * 200 + extraspirit * 200);
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

            setHealth((cur >= mh) ? mh : cur);
        }
        else
            DealDamage(this, float2int32(-amt), 0, 0, 0);
    }
}

void Player::LooseRage(int32 decayValue)
{
    //Rage is lost at a rate of 3 rage every 3 seconds.
    //The Anger Management talent changes this to 2 rage every 3 seconds.
    uint32 cur = getPower(POWER_TYPE_RAGE);
    uint32 newrage = ((int)cur <= decayValue) ? 0 : cur - decayValue;
    if (newrage > 1000)
        newrage = 1000;

    // Object::SetUInt32Value() will (for players) call SendPowerUpdate(),
    // which can be slightly out-of-sync with client rage loss
    // config file rage rate is rage gained, not lost, so we don't need that here
    //    SetUInt32Value(UNIT_FIELD_POWER2,newrage);
    setPower(POWER_TYPE_RAGE, newrage);
    SendPowerUpdate(false); // send update to other in-range players
}

void Player::RegenerateEnergy()
{
    uint32 cur = getPower(POWER_TYPE_ENERGY);
    uint32 mh = getMaxPower(POWER_TYPE_ENERGY);
    if (cur >= mh)
        return;

    float wrate = worldConfig.getFloatRate(RATE_POWER4);
    float amt = PctPowerRegenModifier[POWER_TYPE_ENERGY];
    amt *= wrate * 20.0f;

    cur += float2int32(amt);

#if VERSION_STRING > TBC
    //    Object::SetUInt32Value() will (for players) call SendPowerUpdate(),
    //    which can be slightly out-of-sync with client regeneration [latency] (and wastes packets since client can handle this on its own)
    if (wrate != 1.0f) // our config has custom regen rate, so send new amount to player's client
    {
        setPower(POWER_TYPE_ENERGY, (cur >= mh) ? mh : cur);
    }
    else // let player's own client handle normal regen rates.
    {
        m_uint32Values[UNIT_FIELD_POWER4] = (cur >= mh) ? mh : cur;
        SendPowerUpdate(false); // send update to other in-range players
    }
#elif VERSION_STRING == TBC
    setPower(POWER_TYPE_ENERGY, (cur >= mh) ? mh : cur);
#endif
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
    CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), pet_number);
}

void Player::_Relocate(uint32 mapid, const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id)
{
    //this func must only be called when switching between maps!
    WorldPacket data(41);
    if (sendpending && mapid != m_mapId && force_new_world)
    {
        data.SetOpcode(SMSG_TRANSFER_PENDING);

#if VERSION_STRING >= Cata
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
            RemoveFromWorld();

        m_session->SendPacket(SmsgNewWorld(mapid, v).serialise().get());

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
        SendTeleportPacket(v.x, v.y, v.z, v.o);

    SpeedCheatReset();

    z_axisposition = 0.0f;
}

#ifdef AE_TBC
void Player::AddItemsToWorld()
{
    for (uint8 i = 0; i < INVENTORY_KEYRING_END; ++i)
    {
        if (const auto pItem = getItemInterface()->GetInventoryItem(i))
        {
            pItem->PushToWorld(m_mapMgr);

            if (i < INVENTORY_SLOT_BAG_END)      // only equipment slots get mods.
                _ApplyItemMods(pItem, i, true, false, true);

            if (i >= CURRENCYTOKEN_SLOT_START && i < CURRENCYTOKEN_SLOT_END)
                UpdateKnownCurrencies(pItem->getEntry(), true);

            if (pItem->isContainer() && getItemInterface()->IsBagSlot(i))
            {
                for (uint32 e = 0; e < pItem->getItemProperties()->ContainerSlots; ++e)
                {
                    Item* item = (static_cast< Container* >(pItem))->GetItem(static_cast<int16>(e));
                    if (item)
                        item->PushToWorld(m_mapMgr);
                }
            }
        }
    }

    UpdateStats();
}
#else
void Player::AddItemsToWorld()
{
    for (uint8 i = 0; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (Item* pItem = getItemInterface()->GetInventoryItem(i))
        {
            pItem->PushToWorld(m_mapMgr);

            if (i < INVENTORY_SLOT_BAG_END)      // only equipment slots get mods.
                _ApplyItemMods(pItem, i, true, false, true);

            if (i >= CURRENCYTOKEN_SLOT_START && i < CURRENCYTOKEN_SLOT_END)
                UpdateKnownCurrencies(pItem->getEntry(), true);

            if (pItem->isContainer() && getItemInterface()->IsBagSlot(i))
            {
                for (uint32 e = 0; e < pItem->getItemProperties()->ContainerSlots; ++e)
                {
                    Item* item = (static_cast< Container* >(pItem))->GetItem(static_cast<int16>(e));
                    if (item)
                        item->PushToWorld(m_mapMgr);
                }
            }
        }
    }

    UpdateStats();
}
#endif

void Player::RemoveItemsFromWorld()
{
    for (uint8 i = 0; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (Item* pItem = getItemInterface()->GetInventoryItem((int8)i))
        {
            if (pItem->IsInWorld())
            {
                if (i < INVENTORY_SLOT_BAG_END)      // only equipment+bags slots get mods.
                    _ApplyItemMods(pItem, static_cast<int16>(i), false, false, true);

                pItem->RemoveFromWorld();
            }

            if (pItem->isContainer() && getItemInterface()->IsBagSlot(static_cast<int16>(i)))
            {
                for (uint32 e = 0; e < pItem->getItemProperties()->ContainerSlots; e++)
                {
                    Item* item = (static_cast< Container* >(pItem))->GetItem(static_cast<int16>(e));
                    if (item && item->IsInWorld())
                        item->RemoveFromWorld();
                }
            }
        }
    }

    UpdateStats();
}

uint32 Player::buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    int count = 0;
    if (target == this)
    {
        // we need to send create objects for all items.
        count += getItemInterface()->m_CreateForPlayer(data);
    }
    count += Unit::buildCreateUpdateBlockForPlayer(data, target);
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
            m_KickDelay = 0;
        else
            m_KickDelay -= 1000;

        sChatHandler.BlueSystemMessage(GetSession(), "You will be removed from the server in %u seconds.", (uint32)(m_KickDelay / 1000));
    }
}

void Player::ClearCooldownForSpell(uint32 spell_id)
{
    WorldPacket data(SMSG_CLEAR_COOLDOWN, 12);
    data << uint32_t(spell_id);
    data << uint64_t(getGuid());
    GetSession()->SendPacket(&data);

    if (const auto spellInfo = sSpellMgr.getSpellInfo(spell_id))
    {
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
}

void Player::ClearCooldownsOnLine(uint32 skill_line, uint32 called_from)
{
    // found an easier way.. loop spells, check skill line
    for (SpellSet::const_iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
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
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
        {
            ClearCooldownsOnLine(770, guid);
            ClearCooldownsOnLine(771, guid);
            ClearCooldownsOnLine(772, guid);
        }
        break;
#endif
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
            break;
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
#if VERSION_STRING < Cata
    m_session->OutPacket(SMSG_COMPRESSED_UPDATE_OBJECT, (uint16)stream.total_out + 4, buffer);
#else
    m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)stream.total_out + 4, buffer);
#endif

    // cleanup memory
    delete[] buffer;

    return true;
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

    return false;
}

void Player::ZoneUpdate(uint32 ZoneId)
{
    uint32 oldzone = m_zoneId;
    if (m_zoneId != ZoneId)
    {
        SetZoneId(ZoneId);
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_AREA);
    }

    // how the f*ck is this happening
    if (m_playerInfo == nullptr)
    {
        m_playerInfo = objmgr.GetPlayerInfo(getGuidLow());
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
        Unit* pUnit = (GetSelection() == 0) ? nullptr : (m_mapMgr ? m_mapMgr->GetUnit(GetSelection()) : nullptr);
        if (pUnit && DuelingWith != pUnit)
        {
            EventAttackStop();
            smsg_AttackStop(pUnit);
        }

        if (isCastingSpell())
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

#if VERSION_STRING < Cata
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

    for (std::set<Channel*>::iterator i = m_channels.begin(); i != m_channels.end();)
    {
        Channel* c = *i;
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
        {
            channelname += AreaName;
        }

        Channel* chn = channelmgr.GetCreateChannel(channelname.c_str(), this, c->m_id);
        if (chn != nullptr && !chn->HasMember(this))
        {
            c->Part(this);
            chn->AttemptJoin(this, nullptr);
        }
    }
}

#if VERSION_STRING < Cata
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
        if (Item* pItem = mTradeItems[Index])
        {
            count++;
            ItemProperties const* pProto = pItem->getItemProperties();
            ARCEMU_ASSERT(pProto != NULL);

            data << uint8(Index);

            data << uint32(pProto->ItemId);
            data << uint32(pProto->DisplayInfoID);
            data << uint32(pItem->getStackCount());    // Amount           OK

            // Enchantment stuff
            data << uint32(0);                                            // unknown
            data << uint64(pItem->getGiftCreatorGuid());    // gift creator     OK
            data << uint32(pItem->getEnchantmentId(0));    // Item Enchantment OK
            for (uint8 i = 2; i < 5; i++)                                // Gem enchantments
            {
                if (pItem->GetEnchantment(i) != NULL && pItem->GetEnchantment(i)->Enchantment != NULL)
                    data << uint32(pItem->GetEnchantment(i)->Enchantment->Id);
                else
                    data << uint32(0);
            }
            data << uint64(pItem->getCreatorGuid());        // item creator     OK
            data << uint32(pItem->getSpellCharges(0));    // Spell Charges    OK
            data << uint32(pItem->getPropertySeed());   // seems like time stamp or something like that
            data << uint32(pItem->getRandomPropertiesId());
            data << uint32(pProto->LockId);                                        // lock ID          OK
            data << uint32(pItem->getMaxDurability());
            data << uint32(pItem->getDurability());
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
    pGameObj->setCreatedByGuid(getGuid());
    pGameObj->SetFaction(getFactionTemplate());
    pGameObj->setLevel(getLevel());

    //Assign the Flag
    setDuelArbiter(pGameObj->getGuid());
    pTarget->setDuelArbiter(pGameObj->getGuid());

    pGameObj->PushToWorld(m_mapMgr);

    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->getGuid();
    data << getGuid();
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
        setPower(POWER_TYPE_RAGE, 0);
        DuelingWith->setPower(POWER_TYPE_RAGE, 0);

        //Give the players a Team
        DuelingWith->setDuelTeam(1);  // Duel Requester
        setDuelTeam(2);

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

    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());

    GameObject* pGameObject = GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
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
    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());

    if (m_duelState == DUEL_STATE_FINISHED)
    {
        //if loggingout player requested a duel then we have to make the cleanups
        if (wowGuid.getGuidLowPart())
        {
            GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart()) : 0;
            if (arbiter != nullptr)
            {
                arbiter->RemoveFromWorld(true);
                delete arbiter;
            }

            //we do not wish to lock the other player in duel state
            DuelingWith->setDuelArbiter(0);
            DuelingWith->setDuelTeam(0);
            setDuelArbiter(0);
            setDuelTeam(0);
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
    SendMessageToSet(SmsgDuelWinner(WinCondition, getName(), DuelingWith->getName()).serialise().get(), true);

    WorldPacket data(SMSG_DUEL_COMPLETE, 1);
    data << uint8(1);
    SendMessageToSet(&data, true);

    //Send hook OnDuelFinished

    if (WinCondition != 0)
        sHookInterface.OnDuelFinished(DuelingWith, this);
    else
        sHookInterface.OnDuelFinished(this, DuelingWith);

    //Clear Duel Related Stuff

    GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart()) : 0;

    if (arbiter != nullptr)
    {
        arbiter->RemoveFromWorld(true);
        delete arbiter;
    }

    setDuelArbiter(0);
    setDuelTeam(0);
    DuelingWith->setDuelArbiter(0);
    DuelingWith->setDuelTeam(0);

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
            sendStopMirrorTimerPacket(Type);

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
    setHealth(Info->HP);
    setMaxHealth(Info->HP);
    setMaxPower(POWER_TYPE_MANA, Info->Mana);
    setPower(POWER_TYPE_MANA, Info->Mana);

    if (Level > PreviousLevel)
    {
        if (Level > 9)
            setInitialTalentPoints();
    }
    else
    {
        if (Level != PreviousLevel)
            Reset_AllTalents();
    }

    // Set base fields
    setBaseHealth(Info->HP);
    setBaseMana(Info->Mana);

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

    LOG_DETAIL("Player %s set parameters to level %u", getName().c_str(), Level);
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

    return (rating / combat_rating_entry->val);
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O)
{
    return SafeTeleport(MapID, InstanceID, LocationVector(X, Y, Z, O));
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, const LocationVector & vec)
{
#if VERSION_STRING < Cata
    // Checking if we have a unit whose waypoints are shown
    // If there is such, then we "unlink" it
    // Failing to do so leads to a crash if we try to show some other Unit's wps, after the map was shut down
    if (waypointunit != NULL)
        waypointunit->hideWayPoints(this);

    waypointunit = NULL;

    SpeedCheatDelay(10000);

    if (isOnTaxi())
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TELEPORT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_DISMOUNT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_INTERPOLATE);
        SetTaxiState(false);
        SetTaxiPath(NULL);
        UnSetTaxiPos();
        m_taxi_ride_time = 0;
        setMountDisplayId(0);
        removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
        removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
    }

    if (obj_movement_info.transport_data.transportGuid)
    {
        Transporter* pTrans = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_data.transportGuid));
        if (pTrans)
        {
            pTrans->RemovePassenger(this);
            obj_movement_info.transport_data.transportGuid = 0;
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
    if (getVehicleBase() != NULL)
        getVehicleBase()->getVehicleComponent()->EjectPassenger(this);

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

    if (isOnTaxi())
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TELEPORT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_DISMOUNT);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TAXI_INTERPOLATE);
        SetTaxiState(false);
        SetTaxiPath(nullptr);
        UnSetTaxiPos();
        m_taxi_ride_time = 0;
        setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
        removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
    }

    if (obj_movement_info.getTransportGuid())
    {
        Transporter* pTrans = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.getTransportGuid()));
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
    if (getVehicleBase() != nullptr)
        getVehicleBase()->getVehicleComponent()->EjectPassenger(this);

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
    if (!GetMapMgr())
        return;

    auto at = this->GetArea();
    if (!at)
        return;

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

    GetSession()->SendPacket(SmsgNewWorld(mgr->GetMapId(), vec).serialise().get());

    SetPlayerStatus(TRANSFER_PENDING);
    m_sentTeleportPosition = vec;
    SetPosition(vec);
    SpeedCheatReset();
    ForceZoneUpdate();
}

Guild* Player::GetGuild()
{
    return getGuildId() ? sGuildMgr.getGuildById(getGuildId()) : nullptr;
}

uint32 Player::GetGuildIdFromDB(uint64 guid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT guildId, playerGuid FROM guild_members WHERE playerid = %u", WoWGuid::getGuidLowPartFromUInt64(guid));
    if (result)
    {
        Field* fields = result->Fetch();
        return fields[0].GetUInt32();
    }

    return 0;
}

int8 Player::GetRankFromDB(uint64 guid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT playerid, guildRank FROM guild_members WHERE playerid = %u", WoWGuid::getGuidLowPartFromUInt64(guid));
    if (result)
    {
        Field* fields = result->Fetch();
        return fields[1].GetUInt8();
    }

    return -1;
}

std::string Player::GetGuildName()
{
    return getGuildId() ? sGuildMgr.getGuildById(getGuildId())->getName() : "";
}

void Player::UpdatePvPArea()
{
    auto at = this->GetArea();
    if (at == nullptr)
        return;

    if (hasPlayerFlags(PLAYER_FLAG_GM))
    {
        if (isPvpFlagSet())
            removePvpFlag();
        else
            StopPvPTimer();

        removeFfaPvpFlag();
        return;
    }

    // This is where all the magic happens :P
    if ((at->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (at->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
    {
        if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
        {
            // I'm flagged and I just walked into a zone of my type. Start the 5min counter.
            ResetPvPTimer();
        }
    }
    else
    {
        //Enemy city check
        if (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY)
        {
            if ((at->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (at->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
            {
                if (!isPvpFlagSet())
                    setPvpFlag();
                else
                    StopPvPTimer();
                return;
            }
        }

        //fix for zone areas.
        if (at->zone)
        {
            auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
            if (at2 && ((at2->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && isTeamHorde())))
            {
                if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
                {
                    // I'm flagged and I just walked into a zone of my type. Start the 5min counter.
                    ResetPvPTimer();
                }
                return;
            }
            //enemy territory check
            if (at2 && (at2->flags & AREA_CITY_AREA || at2->flags & AREA_CITY))
            {
                if ((at2->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (at2->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
                {
                    if (!isPvpFlagSet())
                        setPvpFlag();
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
            if (isPvpFlagSet())
                removePvpFlag();
            else
                StopPvPTimer();

            removeFfaPvpFlag();
            setSanctuaryFlag();
        }
        else
        {
            // if we are not in a sanctuary we don't need this flag
            removeSanctuaryFlag();

            //contested territory
            if (sLogonCommHandler.getRealmType() == REALMTYPE_PVP || sLogonCommHandler.getRealmType() == REALMTYPE_RPPVP)
            {
                //automatically sets pvp flag on contested territories.
                if (!isPvpFlagSet())
                    setPvpFlag();
                else
                    StopPvPTimer();
            }

            if (sLogonCommHandler.getRealmType() == REALMTYPE_NORMAL || sLogonCommHandler.getRealmType() == REALMTYPE_RP)
            {
                if (hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE))
                {
                    if (!isPvpFlagSet())
                        setPvpFlag();
                }
                else if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE) && isPvpFlagSet() && !m_pvpTimer)
                {
                    ResetPvPTimer();
                }
            }

            if (at->flags & AREA_PVP_ARENA)            /* ffa pvp arenas will come later */
            {
                if (!isPvpFlagSet())
                    setPvpFlag();

                setFfaPvpFlag();
            }
            else
            {
                removeFfaPvpFlag();
            }
        }
    }
}

void Player::BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag)
{
    for (const auto& iter : getInRangeObjectsSet())
    {
        if (iter)
        {
            Object* curObj = iter;
            if (curObj->isPlayer())
            {
                Group* pGroup = static_cast<Player*>(curObj)->GetGroup();
                if (!pGroup && pGroup != GetGroup())
                {
                    BuildFieldUpdatePacket(static_cast<Player*>(curObj), index, flag);
                }
            }
        }
    }
}

void Player::LoginPvPSetup()
{
    // Make sure we know our area ID.
    _EventExploration();

    auto at = this->GetArea();

    if (at != nullptr && isAlive() && (at->team == AREAC_CONTESTED || (isTeamAlliance() && at->team == AREAC_HORDE_TERRITORY) || (isTeamHorde() && at->team == AREAC_ALLIANCE_TERRITORY)))
        castSpell(this, PLAYER_HONORLESS_TARGET_SPELL, true);

}

void Player::PvPToggle()
{
    if (sLogonCommHandler.getRealmType() == REALMTYPE_NORMAL || sLogonCommHandler.getRealmType() == REALMTYPE_RP)
    {
        if (m_pvpTimer > 0)
        {
            // Means that we typed /pvp while we were "cooling down". Stop the timer.
            StopPvPTimer();

            addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
            removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

            if (!isPvpFlagSet())
                setPvpFlag();
        }
        else
        {
            if (isPvpFlagSet())
            {
                auto at = this->GetArea();
                if (at && (at->flags & AREA_CITY_AREA || at->flags & AREA_CITY))
                {
                    if ((at->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (at->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
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
                removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
            }
            else
            {
                // Move into PvP state.
                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

                StopPvPTimer();
                setPvpFlag();
            }
        }
    }
    else if (sLogonCommHandler.getRealmType() == REALMTYPE_PVP || sLogonCommHandler.getRealmType() == REALMTYPE_RPPVP)
    {
        auto at = this->GetArea();
        if (at == nullptr)
            return;

        // This is where all the magic happens :P
        if ((at->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (at->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
        {
            if (m_pvpTimer > 0)
            {
                // Means that we typed /pvp while we were "cooling down". Stop the timer.
                StopPvPTimer();

                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

                if (!isPvpFlagSet())
                    setPvpFlag();
            }
            else
            {
                if (isPvpFlagSet())
                {
                    // Start the "cooldown" timer.
                    ResetPvPTimer();

                    removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                    addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
                }
                else
                {
                    // Move into PvP state.
                    addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                    removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

                    StopPvPTimer();
                    setPvpFlag();
                }
            }
        }
        else
        {
            if (at->zone)
            {
                auto at2 = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);
                if (at2 && ((at2->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && isTeamHorde())))
                {
                    if (m_pvpTimer > 0)
                    {
                        // Means that we typed /pvp while we were "cooling down". Stop the timer.
                        StopPvPTimer();

                        addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                        removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

                        if (!isPvpFlagSet())
                            setPvpFlag();
                    }
                    else
                    {
                        if (isPvpFlagSet())
                        {
                            // Start the "cooldown" timer.
                            ResetPvPTimer();

                            removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                            addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
                        }
                        else
                        {
                            // Move into PvP state.
                            addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                            removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

                            StopPvPTimer();
                            setPvpFlag();
                        }
                    }
                    return;
                }
            }

            if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE))
            {
                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
            }
            else
            {
                removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
                addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
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
#if VERSION_STRING < Cata
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
#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    this->setArenaCurrency(this->m_arenaPoints);
#endif
#endif

    this->UpdateKnownCurrencies(43307, true);
}

void Player::ResetPvPTimer()
{
    m_pvpTimer = worldConfig.getIntRate(INTRATE_PVPTIMER);
}

void Player::CalculateBaseStats()
{
    if (!lvlinfo)
        return;

    memcpy(BaseStats, lvlinfo->Stat, sizeof(uint32) * 5);

    LevelInfo* levelone = objmgr.GetLevelInfo(this->getRace(), this->getClass(), 1);
    if (levelone == nullptr)
    {
        LOG_ERROR("%s (%d): NULL pointer", __FUNCTION__, __LINE__);
        return;
    }
    setMaxHealth(lvlinfo->HP);
    setBaseHealth(lvlinfo->HP - (lvlinfo->Stat[2] - levelone->Stat[2]) * 10);

    if (getPowerType() == POWER_TYPE_MANA)
    {
        setBaseMana(lvlinfo->Mana - (lvlinfo->Stat[3] - levelone->Stat[3]) * 15);
        setMaxMana(lvlinfo->Mana);
    }
}

void Player::CompleteLoading()
{
    SpellCastTargets targets;
    targets.m_unitTarget = this->getGuid();
    targets.m_targetMask = TARGET_FLAG_UNIT;

    // warrior has to have battle stance
    if (getClass() == WARRIOR)
    {
        // battle stance passive
        castSpell(this, sSpellMgr.getSpellInfo(2457), true);
    }

    for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(*itr);

        if (spellInfo != nullptr
            && (spellInfo->isPassive())  // passive
            && !(spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET))
        {
            if (spellInfo->getRequiredShapeShift())
            {
                if (!(getShapeShiftMask() & spellInfo->getRequiredShapeShift()))
                    continue;
            }

            // Check aurastate
            if (spellInfo->getCasterAuraState() != 0 && !hasAuraState(AuraState(spellInfo->getCasterAuraState()), spellInfo, this))
                continue;

            Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
            spell->prepare(&targets);
        }
    }

    for (std::list<LoginAura>::iterator i = loginauras.begin(); i != loginauras.end(); ++i)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo((*i).id);
        if (sp != nullptr && sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
            continue; //do not load auras that only exist while pet exist. We should recast these when pet is created anyway

        Aura* aura = sSpellMgr.newAura(sp, (*i).dur, this, this, false);
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
                Aura* a = sSpellMgr.newAura(sp, (*i).dur, this, this, false);
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
    if (getHealth() <= 0 && !hasPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        setDeathState(CORPSE);
    }
    else if (hasPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        // Check if we have an existing corpse.
        Corpse* corpse = objmgr.GetCorpseByOwner(getGuidLow());
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

    if (isDead())
    {
        if (myCorpseInstanceId != 0)
        {
            // cebernic: tempfix. This send a counter for player with just logging in.
            //\todo counter will be follow with death time.
            Corpse* myCorpse = objmgr.GetCorpseByOwner(getGuidLow());
            if (myCorpse != nullptr)
                myCorpse->ResetDeathClock();

            GetSession()->SendPacket(SmsgCorpseReclaimDelay(CORPSE_RECLAIM_TIME_MS).serialise().get());
        }
        //RepopRequestedPlayer();
        //sEventMgr.AddEvent(this, &Player::RepopRequestedPlayer, EVENT_PLAYER_CHECKFORCHEATS, 2000, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    if (!IsMounted())
        SpawnActivePet();

#if VERSION_STRING > TBC
    // useless logon spell
    Spell* logonspell = sSpellMgr.newSpell(this, sSpellMgr.getSpellInfo(836), false, nullptr);
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

        castSpell(this, glyph_properties->SpellID, true);
    }

    //sEventMgr.AddEvent(this,&Player::SendAllAchievementData,EVENT_SEND_ACHIEVEMNTS_TO_PLAYER,ACHIEVEMENT_SEND_DELAY,1,0);
    sEventMgr.AddEvent(static_cast< Unit* >(this), &Unit::UpdatePowerAmm, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, LOGIN_CIENT_SEND_DELAY, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
#endif
}

void Player::OnWorldPortAck()
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());
    if (pMapinfo)
    {
        //only resurrect if player is porting to a instance portal
        if (isDead() && pMapinfo->type != INSTANCE_NULL)
            ResurrectPlayer();

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
            modMaxPower(POWER_TYPE_MANA, val);
            m_manafromitems += val;
        }
        break;
        case HEALTH:
        {
            modMaxHealth(val);
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
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_SKILL, val);
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_MAIN_HAND_SKILL, val);   // melee main hand
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_OFF_HAND_SKILL, val);   // melee off hand
        }
        break;
        case DEFENSE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_DEFENCE, val);
        }
        break;
        case DODGE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_DODGE, val);
        }
        break;
        case PARRY_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_PARRY, val);
        }
        break;
        case SHIELD_BLOCK_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK, val);
        }
        break;
        case MELEE_HIT_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT, val);
        }
        break;
        case RANGED_HIT_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT, val);
        }
        break;
        case SPELL_HIT_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT, val);
        }
        break;
        case MELEE_CRITICAL_STRIKE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT, val);
        }
        break;
        case RANGED_CRITICAL_STRIKE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT, val);
        }
        break;
        case SPELL_CRITICAL_STRIKE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT, val);
        }
        break;
        case MELEE_HIT_AVOIDANCE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT_AVOIDANCE, val);
        }
        break;
        case RANGED_HIT_AVOIDANCE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT_AVOIDANCE, val);
        }
        break;
        case SPELL_HIT_AVOIDANCE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT_AVOIDANCE, val);
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
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HASTE, val);  //melee
        }
        break;
        case RANGED_HASTE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HASTE, val);  //ranged
        }
        break;
        case SPELL_HASTE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HASTE, val);  //spell
        }
        break;
        case HIT_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT, val);  //melee
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT, val);  //ranged
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT, val);   //Spell
        }
        break;
        case CRITICAL_STRIKE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT, val);  //melee
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT, val);  //ranged
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT, val);   //spell
        }
        break;
        case HIT_AVOIDANCE_RATING:// this is guessed based on layout of other fields
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HIT_AVOIDANCE, val);  //melee
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HIT_AVOIDANCE, val);  //ranged
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HIT_AVOIDANCE, val);  //spell
        }
        break;
        case CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case EXPERTISE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_EXPERTISE, val);
        }
        break;
        case RESILIENCE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_CRIT_RESILIENCE, val);  //melee
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_CRIT_RESILIENCE, val);  //ranged
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_CRIT_RESILIENCE, val);  //spell
        }
        break;
        case HASTE_RATING:
        {
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_HASTE, val);  //melee
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_HASTE, val);  //ranged
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_SPELL_HASTE, val);   // Spell
        }
        break;
        case ATTACK_POWER:
        {
            modAttackPowerMods(val);
            modRangedAttackPowerMods(val);
        }
        break;
        case RANGED_ATTACK_POWER:
        {
            modRangedAttackPowerMods(val);
        }
        break;
        case FERAL_ATTACK_POWER:
        {
            modAttackPowerMods(val);
        }
        break;
#if VERSION_STRING > Classic
        case SPELL_HEALING_DONE:
        {
            for (uint8 school = 1; school < SCHOOL_COUNT; ++school)
            {
                HealDoneMod[school] += val;
            }
            modModHealingDone(val);
        }
        break;
#endif
        case SPELL_DAMAGE_DONE:
        {
            for (uint8 school = 1; school < SCHOOL_COUNT; ++school)
            {
                modModDamageDonePositive(school, val);
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
            modUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_ARMOR_PENETRATION_RATING, val);
        }
        break;
        case SPELL_POWER:
        {
            for (uint8 school = 1; school < 7; ++school)
            {
                modModDamageDonePositive(school, val);
                HealDoneMod[school] += val;
            }
#if VERSION_STRING > Classic
            modModHealingDone(val);
#endif
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

#if VERSION_STRING < Cata
    if (charter->CharterType == CHARTER_TYPE_GUILD && IsInGuild())
        return false;
#else
    if (charter->CharterType == CHARTER_TYPE_GUILD && requester->GetGuild())
        return false;
#endif

    if (m_charters[charter->CharterType] || requester->getTeam() != getTeam() || this == requester)
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
    uint8 old_ss = getShapeShiftForm();
    setShapeShiftForm(ss);

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
    SpellCastTargets t(getGuid());

    for (std::set<uint32>::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo(*itr);
        if (sp == nullptr)
            continue;

        if (sp->custom_apply_on_shapeshift_change || sp->getAttributes() & ATTRIBUTES_PASSIVE)        // passive/talent
        {
            if (sp->getRequiredShapeShift() && ((uint32)1 << (ss - 1)) & sp->getRequiredShapeShift())
            {
                Spell* spe = sSpellMgr.newSpell(this, sp, true, nullptr);
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
    for (std::set<uint32>::iterator itr = mShapeShiftSpells.begin(); itr != mShapeShiftSpells.end(); ++itr)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo(*itr);
        if (sp->getRequiredShapeShift() && ((uint32)1 << (ss - 1)) & sp->getRequiredShapeShift())
        {
            Spell* spe = sSpellMgr.newSpell(this, sp, true, nullptr);
            spe->prepare(&t);
        }
    }

    UpdateStats();
    UpdateChances();
}

void Player::CalcDamage()
{
    float r;
    int ss = getShapeShiftForm();
    /////////////////MAIN HAND
    float ap_bonus = GetAP() / 14000.0f;
    float delta = (float)getModDamageDonePositive(SCHOOL_NORMAL) - (float)getModDamageDoneNegative(SCHOOL_NORMAL);

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
            if (lev < 42)
                x = lev - 1;
            else if (lev < 46)
                x = lev;
            else if (lev < 49)
                x = 2 * lev - 45;
            else if (lev < 60)
                x = lev + 4;
            else
            x = 64;

            // 3rd grade polinom for calculating blue two-handed weapon dps based on itemlevel (from Hyzenthlei)
            if (x <= 28)
                feral_damage = 1.563e-03f * x * x * x - 1.219e-01f * x * x + 3.802e+00f * x - 2.227e+01f;
            else if (x <= 41)
                feral_damage = -3.817e-03f * x * x * x + 4.015e-01f * x * x - 1.289e+01f * x + 1.530e+02f;
            else
                feral_damage = 1.829e-04f * x * x * x - 2.692e-02f * x * x + 2.086e+00f * x - 1.645e+01f;

            r = feral_damage * 0.79f + delta + ap_bonus * 1000.0f;
            r *= tmp;
            setMinDamage(r > 0 ? r : 0);

            r = feral_damage * 1.21f + delta + ap_bonus * 1000.0f;
            r *= tmp;
            setMaxDamage(r > 0 ? r : 0);
        }
        else // Bear or Dire Bear Form
        {
            if (ss == FORM_BEAR)
                x = lev;
            else
                x = lev + 5; // DIRE_BEAR dps is slightly better than bear dps

            if (x > 70)
                x = 70;

            // 3rd grade polinom for calculating green two-handed weapon dps based on itemlevel (from Hyzenthlei)
            if (x <= 30)
                feral_damage = 7.638e-05f * x * x * x + 1.874e-03f * x * x + 4.967e-01f * x + 1.906e+00f;
            else if (x <= 44)
                feral_damage = -1.412e-03f * x * x * x + 1.870e-01f * x * x - 7.046e+00f * x + 1.018e+02f;
            else
                feral_damage = 2.268e-04f * x * x * x - 3.704e-02f * x * x + 2.784e+00f * x - 3.616e+01f;

            feral_damage *= 2.5f; // Bear Form attack speed

            r = feral_damage * 0.79f + delta + ap_bonus * 2500.0f;
            r *= tmp;
            setMinDamage(r > 0 ? r : 0);

            r = feral_damage * 1.21f + delta + ap_bonus * 2500.0f;
            r *= tmp;
            setMaxDamage(r > 0 ? r : 0);
        }

        return;
    }

    //////no druid ss
    uint32 speed = 2000;
    Item* it = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (!disarmed)
    {
        if (it)
            speed = it->getItemProperties()->Delay;
    }

    float bonus = ap_bonus * speed;
    float tmp = 1;
    for (std::map<uint32, WeaponModifier>::iterator i = damagedone.begin(); i != damagedone.end(); ++i)
    {
        if ((i->second.wclass == (uint32)-1) || //any weapon
            (it && ((1 << it->getItemProperties()->SubClass) & i->second.subclass)))
            tmp += i->second.value;
    }

    r = BaseDamage[0] + delta + bonus;
    r *= tmp;
    setMinDamage(r > 0 ? r : 0);

    r = BaseDamage[1] + delta + bonus;
    r *= tmp;
    setMaxDamage(r > 0 ? r : 0);

    uint32 cr = 0;
    if (it)
    {
        if (static_cast< Player* >(this)->m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    //\todo investigate
#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_MAIN_HAND_SKILL, cr);
#endif
    /////////////// MAIN HAND END

    /////////////// OFF HAND START
    cr = 0;
    it = static_cast< Player* >(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it)
    {
        if (!disarmed)
        {
            speed = it->getItemProperties()->Delay;
        }
        else speed = 2000;

        bonus = ap_bonus * speed;
        
        tmp = 1;
        for (std::map<uint32, WeaponModifier>::iterator i = damagedone.begin(); i != damagedone.end(); ++i)
        {
            if ((i->second.wclass == (uint32)-1) || //any weapon
                (((1 << it->getItemProperties()->SubClass) & i->second.subclass))
                )
                tmp += i->second.value;
        }

        r = (BaseOffhandDamage[0] + delta + bonus) * offhand_dmg_mod;
        r *= tmp;
        setMinOffhandDamage(r > 0 ? r : 0);
        r = (BaseOffhandDamage[1] + delta + bonus) * offhand_dmg_mod;
        r *= tmp;
        setMaxOffhandDamage(r > 0 ? r : 0);
        if (m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    //\todo investigate
#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_MELEE_OFF_HAND_SKILL, cr);
#endif
    /////////////second hand end
    ///////////////////////////RANGED
    cr = 0;
    if ((it = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)) != nullptr)
    {
        tmp = 1;
        for (std::map<uint32, WeaponModifier>::iterator i = damagedone.begin(); i != damagedone.end(); ++i)
        {
            if ((i->second.wclass == (uint32)-1) || //any weapon
                (((1 << it->getItemProperties()->SubClass) & i->second.subclass)))
            {
                tmp += i->second.value;
            }
        }

#if VERSION_STRING < Cata
        if (it->getItemProperties()->SubClass != 19)//wands do not have bonuses from RAP & ammo
        {
            //                ap_bonus = (getRangedAttackPower()+(int32)getRangedAttackPowerMods())/14000.0;
            //modified by Zack : please try to use premade functions if possible to avoid forgetting stuff
            ap_bonus = GetRAP() / 14000.0f;
            bonus = ap_bonus * it->getItemProperties()->Delay;

            if (getAmmoId() && !m_requiresNoAmmo)
            {
                ItemProperties const* xproto = sMySQLStore.getItemProperties(getAmmoId());
                if (xproto)
                {
                    bonus += ((xproto->Damage[0].Min + xproto->Damage[0].Max) * it->getItemProperties()->Delay) / 2000.0f;
                }
            }
        }
        else
#endif
            bonus = 0;

        r = BaseRangedDamage[0] + delta + bonus;
        r *= tmp;
        setMinRangedDamage(r > 0 ? r : 0);

        r = BaseRangedDamage[1] + delta + bonus;
        r *= tmp;
        setMaxRangedDamage(r > 0 ? r : 0);


        if (m_wratings.size())
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find(it->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }

    }
    //\todo investigate
#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_RANGED_SKILL, cr);
#endif
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
    int ss = getShapeShiftForm();
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
    Item* it = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (!disarmed)
    {
        if (it)
            speed = it->getItemProperties()->Delay;
    }
    r = ap_bonus * speed;
    return float2int32(r);
}

void Player::EventPortToGM(Player* p)
{
    SafeTeleport(p->GetMapId(), p->GetInstanceID(), p->GetPosition());
}

void Player::AddComboPoints(uint64 target, int8 count)
{
    // GetTimeLeft() checked in SpellAura, so we won't lose points
    RemoveAllAuraType(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (m_comboTarget == target)
    {
        m_comboPoints += count;
    }
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
        if (!target || target->isDead() || GetSelection() != m_comboTarget)
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

    m_session->SendPacket(SmsgAreaTriggerMessage(0, msg, 0).serialise().get());
}

void Player::removeSoulStone()
{
    if (!this->SoulStone)
        return;

    uint32 sSoulStone = 0;
    switch (this->SoulStone)
    {
        case 3026:
            sSoulStone = 20707;
        break;
        case 20758:
            sSoulStone = 20762;
        break;
        case 20759:
            sSoulStone = 20763;
        break;
        case 20760:
            sSoulStone = 20764;
        break;
        case 20761:
            sSoulStone = 20765;
        break;
        case 27240:
            sSoulStone = 27239;
        break;
        case 47882:
            sSoulStone = 47883;
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

//\todo: find another solution for this
void Player::SetNoseLevel()
{
    // Set the height of the player
    switch (getRace())
    {
        case RACE_HUMAN:
            // female
            if (getGender())
                m_noseLevel = 1.72f;
            // male
            else
                m_noseLevel = 1.78f;
            break;
        case RACE_ORC:
            if (getGender())
                m_noseLevel = 1.82f;
            else
                m_noseLevel = 1.98f;
            break;
        case RACE_DWARF:
            if (getGender())
                m_noseLevel = 1.27f;
            else
                m_noseLevel = 1.4f;
            break;
        case RACE_NIGHTELF:
            if (getGender())
                m_noseLevel = 1.84f;
            else
                m_noseLevel = 2.13f;
            break;
        case RACE_UNDEAD:
            if (getGender())
                m_noseLevel = 1.61f;
            else
                m_noseLevel = 1.8f;
            break;
        case RACE_TAUREN:
            if (getGender())
                m_noseLevel = 2.48f;
            else
                m_noseLevel = 2.01f;
            break;
        case RACE_GNOME:
            if (getGender())
                m_noseLevel = 1.06f;
            else
                m_noseLevel = 1.04f;
            break;
#if VERSION_STRING >= Cata
        case RACE_GOBLIN:
            if (getGender())
                m_noseLevel = 1.06f;
            else
                m_noseLevel = 1.04f;
            break;
#endif
        case RACE_TROLL:
            if (getGender())
                m_noseLevel = 2.02f;
            else
                m_noseLevel = 1.93f;
            break;
#if VERSION_STRING > Classic
        case RACE_BLOODELF:
            if (getGender())
                m_noseLevel = 1.83f;
            else
                m_noseLevel = 1.93f;
            break;
        case RACE_DRAENEI:
            if (getGender())
                m_noseLevel = 2.09f;
            else
                m_noseLevel = 2.36f;
            break;
#endif
#if VERSION_STRING >= Cata
        case RACE_WORGEN:
            if (getGender())
                m_noseLevel = 1.72f;
            else
                m_noseLevel = 1.78f;
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

    m_session->SendPacket(SmsgSummonRequest(Requestor, ZoneID, 120000).serialise().get());
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

    //Add to proficiency
    if (ItemProf* prof1 = (ItemProf*)GetProficiencyBySkill(SkillLine))
    {
        if (prof1->itemclass == 4)
        {
            armor_proficiency |= prof1->subclass;
            sendSetProficiencyPacket(prof1->itemclass, armor_proficiency);
        }
        else
        {
            weapon_proficiency |= prof1->subclass;
            sendSetProficiencyPacket(prof1->itemclass, weapon_proficiency);
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
#if VERSION_STRING < Cata
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

#if VERSION_STRING < Cata
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
#if VERSION_STRING >= Cata
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
#if VERSION_STRING < Cata
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
    uint32 removeSpellId = 0;
    for (uint32 idx = 0; idx < sSkillLineAbilityStore.GetNumRows(); ++idx)
    {
        auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(idx);
        if (skill_line_ability == nullptr)
            continue;

        // add new "automatic-acquired" spell
        if ((skill_line_ability->skilline == SkillLine) && (skill_line_ability->acquireMethod == 1))
        {
            SpellInfo const* sp = sSpellMgr.getSpellInfo(skill_line_ability->spell);
            if (sp && (curr_sk >= skill_line_ability->minSkillLineRank))
            {
                // Player is able to learn this spell; check if they already have it, or a higher rank (shouldn't, but just in case)
                bool addThisSpell = true;
                for (SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
                {
                    SpellInfo const* se = sSpellMgr.getSpellInfo(*itr);
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
                    if (sp->isPassive())
                    {
                        SpellCastTargets targets;
                        targets.m_unitTarget = this->getGuid();
                        targets.m_targetMask = TARGET_FLAG_UNIT;
                        Spell* spell = sSpellMgr.newSpell(this, sp, true, nullptr);
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
        auto level_bound_skill = itr->second.Skill->type == SKILL_TYPE_WEAPON || itr->second.Skill->id == SKILL_LOCKPICKING;
#if VERSION_STRING <= TBC
        level_bound_skill = level_bound_skill || itr->second.Skill->id == SKILL_POISONS;
#endif
        if (level_bound_skill)
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
#if VERSION_STRING >= Cata
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

            PlayerSkill sk;
            sk.Reset(skills[i]);
            sk.MaximumValue = sk.CurrentValue = 300;
            m_skills.insert(std::make_pair(skills[i], sk));
            if (uint32 spell_id = ::GetSpellForLanguage(skills[i]))
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
                    PlayerSkill sk;
                    sk.Reset(itr->skillid);
                    sk.MaximumValue = sk.CurrentValue = 300;
                    m_skills.insert(std::make_pair(itr->skillid, sk));
                    if (uint32 spell_id = ::GetSpellForLanguage(itr->skillid))
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

    uint32 SS = getShapeShiftForm();

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
        setAttackPowerMultiplier(getAttackPowerMultiplier() + tval / 200.0f);
        setRangedAttackPowerMultiplier(getRangedAttackPowerMultiplier() + tval / 200.0f);
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
    for (uint8 i = 0; i < MAX_QUEST_SLOT; i++)
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
    getUpdateMgr().queueDelayedPacket(new WorldPacket(*data));
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

    for (SpellSet::iterator iter = mSpells.begin(); iter != mSpells.end();)
    {
        SpellSet::iterator it = iter++;
        uint32 SpellID = *it;
        const auto spellInfo = sSpellMgr.getSpellInfo(SpellID);
        if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER)
        {
            this->removeAllAurasByIdForGuid(SpellID, this->getGuid());   //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets(this->getGuid());
            Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
        if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET)
        {
            this->removeAllAurasByIdForGuid(SpellID, this->getGuid());   //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets(new_pet->getGuid());
            Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);    //we cast it as a proc spell, maybe we should not !
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
    SpellInfo const* sp = sSpellMgr.getSpellInfo(id);
    mShapeShiftSpells.insert(id);

    if (sp->getRequiredShapeShift() && getShapeShiftMask() & sp->getRequiredShapeShift())
    {
        Spell* spe = sSpellMgr.newSpell(this, sp, true, nullptr);
        SpellCastTargets t(this->getGuid());
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

    if (ItemProperties const* proto = sMySQLStore.getItemProperties(m_lastPotionId))
    {
        for (uint8 i = 0; i < 5; ++i)
        {
            if (proto->Spells[i].Id && proto->Spells[i].Trigger == USE)
            {
                const auto spellInfo = sSpellMgr.getSpellInfo(proto->Spells[i].Id);
                if (spellInfo != nullptr)
                {
                    Cooldown_AddItem(proto, i);
                    sendSpellCooldownEventPacket(spellInfo->getId());
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
        SpellInfo const *sp = sSpellMgr.getSpellInfo(*itr);

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

void Player::Cooldown_Add(SpellInfo const* pSpell, Item* pItemCaster)
{
    uint32 mstime = Util::getMSTime();
    int32 cool_time;
    uint32 spell_id = pSpell->getId();
    uint32 category_id = pSpell->getCategory();

    uint32 spell_category_recovery_time = pSpell->getCategoryRecoveryTime();
    if (spell_category_recovery_time > 0 && category_id)
    {
        spellModFlatIntValue(SM_FCooldownTime, &cool_time, pSpell->getSpellFamilyFlags());
        spellModPercentageIntValue(SM_PCooldownTime, &cool_time, pSpell->getSpellFamilyFlags());

        AddCategoryCooldown(category_id, spell_category_recovery_time + mstime, spell_id, pItemCaster ? pItemCaster->getItemProperties()->ItemId : 0);
    }

    uint32 spell_recovery_t = pSpell->getRecoveryTime();
    if (spell_recovery_t > 0)
    {
        spellModFlatIntValue(SM_FCooldownTime, &cool_time, pSpell->getSpellFamilyFlags());
        spellModPercentageIntValue(SM_PCooldownTime, &cool_time, pSpell->getSpellFamilyFlags());

        _Cooldown_Add(COOLDOWN_TYPE_SPELL, spell_id, spell_recovery_t + mstime, spell_id, pItemCaster ? pItemCaster->getItemProperties()->ItemId : 0);
    }
}

void Player::Cooldown_AddStart(SpellInfo const* pSpell)
{
    if (pSpell->getStartRecoveryTime() == 0)
        return;

    uint32 mstime = Util::getMSTime();
    int32 atime; // = float2int32(float(pSpell->StartRecoveryTime) / SpellHasteRatingBonus);

    if (getModCastSpeed() >= 1.0f)
        atime = pSpell->getStartRecoveryTime();
    else
        atime = float2int32(pSpell->getStartRecoveryTime() * getModCastSpeed());

    spellModFlatIntValue(SM_FGlobalCooldown, &atime, pSpell->getSpellFamilyFlags());
    spellModPercentageIntValue(SM_PGlobalCooldown, &atime, pSpell->getSpellFamilyFlags());

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

bool Player::Cooldown_CanCast(SpellInfo const* pSpell)
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
            
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(pSpell->getId());
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (mstime < itr->second.ExpireTime)
            return false;
        
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    if (pSpell->getStartRecoveryTime() && m_globalCooldown && !this->m_cheats.CooldownCheat /* cebernic:GCD also cheat :D */)            /* gcd doesn't affect spells without a cooldown it seems */
    {
        if (mstime < m_globalCooldown)
            return false;
        
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
            
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(isp->Id);
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (mstime < itr->second.ExpireTime)
            return false;
        
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    return true;
}

#define COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN 10000

void Player::_SavePlayerCooldowns(QueryBuffer* buf)
{
    uint32 mstime = Util::getMSTime();

    // clear them (this should be replaced with an update queue later)
    if (buf != nullptr)
        buf->AddQuery("DELETE FROM playercooldowns WHERE player_guid = %u", getGuidLow());        // 0 is guid always
    else
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", getGuidLow());        // 0 is guid always

    for (uint32 i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (PlayerCooldownMap::iterator itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            PlayerCooldownMap::iterator itr2 = itr++;

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

            uint32 seconds = (itr2->second.ExpireTime - mstime) / 1000;
            // this shouldn't ever be nonzero because of our check before, so no check needed

            if (buf != nullptr)
            {
                buf->AddQuery("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", getGuidLow(),
                              i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId);
            }
            else
            {
                CharacterDatabase.Execute("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", getGuidLow(),
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

    do
    {
        uint32 type = result->Fetch()[0].GetUInt32();
        uint32 misc = result->Fetch()[1].GetUInt32();
        uint32 rtime = result->Fetch()[2].GetUInt32();
        uint32 spellid = result->Fetch()[3].GetUInt32();
        uint32 itemid = result->Fetch()[4].GetUInt32();

        if (type >= NUM_COOLDOWN_TYPES)
            continue;

        if ((uint32)UNIXTIME > rtime)
            continue;

        rtime -= (uint32)UNIXTIME;

        if (rtime < 10)
            continue;

        uint32 realtime = mstime + ((rtime) * 1000);

        // apply it back into cooldown map
        PlayerCooldown cd;
        cd.ExpireTime = realtime;
        cd.ItemId = itemid;
        cd.SpellId = spellid;
        m_cooldownMap[type].insert(std::make_pair(misc, cd));

    }
    while (result->NextRow());
}

void Player::Social_AddFriend(const char* name, const char* note)
{
    // lookup the player
    PlayerInfo* playerInfo = objmgr.GetPlayerInfoByName(name);
    PlayerCache* playerCache = objmgr.GetPlayerCache(name, false);

    if (playerInfo == nullptr || (playerCache != nullptr && playerCache->HasFlag(CACHE_PLAYER_FLAGS, PLAYER_FLAG_GM)))
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());

        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    // team check
    if (playerInfo->team != getInitialTeam() && m_session->permissioncount == 0 && !worldConfig.player.isInterfactionFriendsEnabled)
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_ENEMY, playerInfo->guid).serialise().get());
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    // are we ourselves?
    if (playerCache != nullptr && playerCache->GetUInt32Value(CACHE_PLAYER_LOWGUID) == getGuidLow())
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_SELF, getGuid()).serialise().get());
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    if (m_cache->CountValue64(CACHE_SOCIAL_FRIENDLIST, playerInfo->guid))
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_ALREADY, playerInfo->guid).serialise().get());
        if (playerCache != nullptr)
            playerCache->DecRef();
        return;
    }

    if (playerCache != nullptr)   //hes online if he has a cache
    {
        playerCache->InsertValue64(CACHE_SOCIAL_HASFRIENDLIST, getGuidLow());
        m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_ONLINE, playerCache->GetUInt32Value(CACHE_PLAYER_LOWGUID), note ? note : "", 1,
            playerInfo->m_loggedInPlayer->GetZoneId(), playerInfo->lastLevel, playerInfo->cl).serialise().get());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_OFFLINE, playerInfo->guid).serialise().get());
    }

    char* notedup = note == nullptr ? NULL : strdup(note);
    m_cache->InsertValue64(CACHE_SOCIAL_FRIENDLIST, playerInfo->guid, notedup);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')",
                              getGuidLow(), playerInfo->guid, note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "");

    if (playerCache != nullptr)
        playerCache->DecRef();
}

void Player::Social_RemoveFriend(uint32 guid)
{
    // are we ourselves?
    if (guid == getGuidLow())
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_SELF, getGuid()).serialise().get());
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

    PlayerCache* cache = objmgr.GetPlayerCache((uint32)guid);
    if (cache != nullptr)
    {
        cache->RemoveValue64(CACHE_SOCIAL_HASFRIENDLIST, getGuidLow());
        cache->DecRef();
    }

    m_session->SendPacket(SmsgFriendStatus(FRIEND_REMOVED, guid).serialise().get());

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u",
                              getGuidLow(), (uint32)guid);
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
                              note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "", getGuidLow(), guid);
}

void Player::Social_AddIgnore(const char* name)
{
    // lookup the player
    PlayerInfo* playerInfo = objmgr.GetPlayerInfoByName(name);
    if (playerInfo == nullptr)
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
        return;
    }

    // are we ourselves?
    if (playerInfo == m_playerInfo)
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_SELF, getGuid()).serialise().get());
        return;
    }

    if (m_cache->CountValue64(CACHE_SOCIAL_IGNORELIST, playerInfo->guid) > 0)
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_ALREADY, playerInfo->guid).serialise().get());
        return;
    }

    m_cache->InsertValue64(CACHE_SOCIAL_IGNORELIST, playerInfo->guid);
    m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_ADDED, playerInfo->guid).serialise().get());

    // dump into db
    CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", getGuidLow(), playerInfo->guid);
}

void Player::Social_RemoveIgnore(uint32 guid)
{
    // are we ourselves?
    if (guid == getGuidLow())
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_SELF, getGuid()).serialise().get());
        return;
    }

    m_cache->RemoveValue64(CACHE_SOCIAL_IGNORELIST, guid);
    m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_REMOVED, guid).serialise().get());

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u",
                              getGuidLow(), (uint32)guid);
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

    m_cache->AcquireLock64(CACHE_SOCIAL_HASFRIENDLIST);
    for (PlayerCacheMap::iterator itr = m_cache->Begin64(CACHE_SOCIAL_HASFRIENDLIST); itr != m_cache->End64(CACHE_SOCIAL_HASFRIENDLIST); ++itr)
    {
        PlayerCache* cache = objmgr.GetPlayerCache(uint32(itr->first));
        if (cache != nullptr)
        {
            cache->SendPacket(SmsgFriendStatus(FRIEND_ONLINE, getGuid(), "", 1, GetAreaID(), getLevel(), getClass()).serialise().get());
            cache->DecRef();
        }
    }
    m_cache->ReleaseLock64(CACHE_SOCIAL_HASFRIENDLIST);
}

void Player::Social_TellFriendsOffline()
{
    if (m_cache->GetSize64(CACHE_SOCIAL_HASFRIENDLIST) == 0)
        return;

    m_cache->AcquireLock64(CACHE_SOCIAL_HASFRIENDLIST);
    for (PlayerCacheMap::iterator itr = m_cache->Begin64(CACHE_SOCIAL_HASFRIENDLIST); itr != m_cache->End64(CACHE_SOCIAL_HASFRIENDLIST); ++itr)
    {
        PlayerCache* cache = objmgr.GetPlayerCache(uint32(itr->first));
        if (cache != nullptr)
        {
            cache->SendPacket(SmsgFriendStatus(FRIEND_OFFLINE, getGuid()).serialise().get());
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

#if VERSION_STRING == Mop
        data << uint32(0);
        data << uint32(0);
#endif
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

#if VERSION_STRING == Mop
        data << uint32(0);
        data << uint32(0);
#endif

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

    ARCEMU_ASSERT(m_playerInfo != NULL);

    for (int i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
    {
        if (m_arenaTeams[i] != nullptr)
        {
            if (ArenaTeamMember* m = m_arenaTeams[i]->GetMemberByGuid(m_playerInfo->guid))
            {
                if (m->PersonalRating > maxrating)
                    maxrating = m->PersonalRating;
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
    if (isDead())
        ResurrectPlayer();

    setHealth(getMaxHealth());
    setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
    setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
}

/**********************************************
* Remove all temporary enchants from all items
**********************************************/
void Player::RemoveTempEnchantsOnArena()
{
    ItemInterface* itemi = getItemInterface();

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
            if (it->isContainer())
            {
                Container* bag = static_cast< Container* >(it);
                for (uint32 ci = 0; ci < bag->getItemProperties()->ContainerSlots; ++ci)
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

void Player::UpdatePowerAmm()
{
#if VERSION_STRING > TBC
    SendMessageToSet(SmsgPowerUpdate(GetNewGUID(), getPowerType(), getUInt32Value(UNIT_FIELD_POWER1 + getPowerType())).serialise().get(), true);
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
    setGlyphsEnabled(glyphMask[level]);
}
#endif

#if VERSION_STRING >= Cata
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
            setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_GLYPH_SLOTS_1 + slot++), glyphSlot->Id);
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
        slotMask = (GS_MASK_LEVEL_25 | GS_MASK_LEVEL_50);
    }
    if (level >= 75)
    {
        slotMask = (GS_MASK_LEVEL_25 | GS_MASK_LEVEL_50 | GS_MASK_LEVEL_75);
    }

    setGlyphsEnabled(slotMask);
}
#endif

// Fills fields from firstField to firstField+fieldsNum-1 with integers from the string
void Player::LoadFieldsFromString(const char* string, uint16 firstField, uint32 fieldsNum)
{
    if (string == nullptr)
        return;

    char* start = (char*)string;
    for (uint16 Counter = 0; Counter < fieldsNum; Counter++)
    {
        char* end = strchr(start, ',');
        if (!end)
            break;
        *end = 0;
        setExploredZone(Counter, atol(start));
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

    m_session->SendPacket(SmsgTitleEarned(title, set ? 1 : 0).serialise().get());
}

uint32 Player::GetInitialFactionId()
{
    if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
        return raceEntry->faction_id;

    return 0;
}

void Player::CalcExpertise()
{
    int32 modifier = 0;

#if VERSION_STRING != Classic
    setUInt32Value(PLAYER_EXPERTISE, 0);
    setUInt32Value(PLAYER_OFFHAND_EXPERTISE, 0);
#endif

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] != nullptr && m_auras[x]->HasModType(SPELL_AURA_EXPERTISE))
        {
            SpellInfo const* entry = m_auras[x]->m_spellInfo;
            int32 val = m_auras[x]->GetModAmountByMod();

            if (entry->getEquippedItemSubClass() != 0)
            {
                auto item_mainhand = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                auto item_offhand = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                uint32 reqskillMH = 0;
                uint32 reqskillOH = 0;

                if (item_mainhand)
                    reqskillMH = entry->getEquippedItemSubClass() & (((uint32)1) << item_mainhand->getItemProperties()->SubClass);
                if (item_offhand)
                    reqskillOH = entry->getEquippedItemSubClass() & (((uint32)1) << item_offhand->getItemProperties()->SubClass);

                if (reqskillMH != 0 || reqskillOH != 0)
                    modifier = +val;
            }
            else
                modifier += val;
        }
    }

#if VERSION_STRING != Classic
    modUInt32Value(PLAYER_EXPERTISE, (int32)CalcRating(PCR_EXPERTISE) + modifier);
    modUInt32Value(PLAYER_OFFHAND_EXPERTISE, (int32)CalcRating(PCR_EXPERTISE) + modifier);
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
    this->getItemInterface()->SafeFullRemoveItemByGuid(GUID);
}

void Player::SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active)
{
    if (active)
    {
        if (!shapeshift_form)
            return;

        WorldPacket data(SMSG_PET_SPELLS, 8 * 4 + 20);
        data << getGuid();
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

void Player::HandleSpellLoot(uint32 itemid)
{
    Loot loot1;

    lootmgr.FillItemLoot(&loot1, itemid);

    for (std::vector<__LootItem>::iterator itr = loot1.items.begin(); itr != loot1.items.end(); ++itr)
    {
        uint32 looteditemid = itr->item.itemproto->ItemId;
        uint32 count = itr->iItemsCount;

        getItemInterface()->AddItemById(looteditemid, count, 0);
    }
}

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
    data << getGuid();
    data << uint8(0x0);

    SpellSet::iterator sitr;
    for (sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        uint32 SpellId = (*sitr);

        const auto spellInfo = sSpellMgr.getSpellInfo(SpellId);

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
    for (std::list<Item*>::iterator itr = m_GarbageItems.begin(); itr != m_GarbageItems.end(); ++itr)
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

    for (const auto& itr : getInRangePlayersSet())
    {
        if (itr)
        {
            Player* p = static_cast<Player*>(itr);
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
        uint32 myteam = getTeam();

        if (data->GetOpcode() != SMSG_MESSAGECHAT)
        {
            for (const auto& itr : getInRangePlayersSet())
            {
                if (itr)
                {
                    Player* p = static_cast<Player*>(itr);
                    if (gminvis && ((p->GetSession() == nullptr) || (p->GetSession()->GetPermissionCount() <= 0)))
                        continue;

                    if (p->getTeam() == myteam && (p->GetPhase() & myphase) != 0 && p->IsVisible(getGuid()))
                        p->SendPacket(data);
                }
            }
        }
        else
        {
            for (const auto& itr : getInRangePlayersSet())
            {
                if (itr)
                {
                    Player* p = static_cast<Player*>(itr);
                    if (p->GetSession() && p->getTeam() == myteam && !p->Social_IsIgnoring(getGuidLow()) && (p->GetPhase() & myphase) != 0)
                        p->SendPacket(data);
                }
            }
        }
    }
    else
    {
        if (data->GetOpcode() != SMSG_MESSAGECHAT)
        {
            for (const auto& itr : getInRangePlayersSet())
            {
                if (itr)
                {
                    Player* p = static_cast<Player*>(itr);
                    if (gminvis && (p->GetSession() == nullptr || p->GetSession()->GetPermissionCount() <= 0))
                        continue;

                    if ((p->GetPhase() & myphase) != 0 && p->IsVisible(getGuid()))
                        p->SendPacket(data);
                }
            }
        }
        else
        {
            for (const auto& itr : getInRangePlayersSet())
            {
                if (itr)
                {
                    Player* p = static_cast<Player*>(itr);
                    if (p->GetSession() && !p->Social_IsIgnoring(getGuidLow()) && (p->GetPhase() & myphase) != 0)
                        p->SendPacket(data);
                }
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
        sendReportToGmMessage(getName(), dmglog.str());

    return dmg;
}

void Player::DealDamage(Unit* pVictim, uint32 damage, uint32 /*targetEvent*/, uint32 /*unitEvent*/, uint32 spellId, bool no_remove_auras)
{
    if (!pVictim || !pVictim->isAlive() || !pVictim->IsInWorld() || !IsInWorld())
        return;
    if (pVictim->isPlayer() && static_cast< Player* >(pVictim)->m_cheats.GodModeCheat == true)
        return;
    if (pVictim->bInvincible)
        return;
    if (pVictim->isCreature() && static_cast<Creature*>(pVictim)->isSpiritHealer())
        return;

    if (this != pVictim)
    {
        if (!GetSession()->HasPermissions() && worldConfig.limit.isLimitSystemEnabled != 0)
            damage = CheckDamageLimits(damage, spellId);

        CombatStatus.OnDamageDealt(pVictim);

        if (pVictim->isPvpFlagSet())
        {
            if (!isPvpFlagSet())
                PvPToggle();

            AggroPvPGuards();
        }
    }

    pVictim->setStandState(STANDSTATE_STAND);

    if (CombatStatus.IsInCombat())
        sHookInterface.OnEnterCombat(this, this);

    ///////////////////////////////////////////////////// Hackatlon ///////////////////////////////////////////////////////////

    //the black sheep , no actually it is paladin : Ardent Defender
    if (DamageTakenPctModOnHP35 && hasAuraState(AURASTATE_FLAG_HEALTH35))
        damage = damage - float2int32(damage * DamageTakenPctModOnHP35) / 100;

    if (pVictim->isCreature() && pVictim->IsTaggable())
    {
        pVictim->Tag(getGuid());
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
    if (pVictim->isPlayer() && DuelingWith != nullptr && DuelingWith->getGuid() == pVictim->getGuid())
    {
        if (pVictim->getHealth() <= damage)
        {
            uint32 NewHP = pVictim->getMaxHealth() / 100;
            if (NewHP < 5)
                NewHP = 5;

            pVictim->setHealth(NewHP);
            EndDuel(DUEL_WINNER_KNOCKOUT);
            pVictim->Emote(EMOTE_ONESHOT_BEG);
            return;
        }
    }

    if (pVictim->getHealth() <= damage)
    {
        if (pVictim->isTrainingDummy())
        {
            pVictim->setHealth(1);
            return;
        }

        if (m_bg != nullptr)
        {
            m_bg->HookOnUnitKill(this, pVictim);

            if (pVictim->isPlayer())
                m_bg->HookOnPlayerKill(this, static_cast< Player* >(pVictim));
        }

        if (pVictim->isPlayer())
        {
            Player* playerVictim = static_cast<Player*>(pVictim);
            sHookInterface.OnKillPlayer(this, playerVictim);

            bool setAurastateFlag = false;

            if (getLevel() >= (pVictim->getLevel() - 8) && (getGuid() != pVictim->getGuid()))
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
                addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);

                if (!sEventMgr.HasEvent(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                    sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);
            }
        }
        else
        {
            if (pVictim->isCreature())
            {
                Reputation_OnKilledUnit(pVictim, false);

#if VERSION_STRING > TBC
                GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, GetMapId(), 0, 0);
#endif

                if (pVictim->getLevel() >= (getLevel() - 8) && (getGuid() != pVictim->getGuid()))
                {
                    HandleProc(PROC_ON_GAIN_EXPIERIENCE, this, nullptr);
                    m_procCounter = 0;
                }
            }
        }

        EventAttackStop();

        sendPartyKillLogPacket(pVictim->getGuid());

        if (pVictim->isPvpFlagSet())
        {
            uint32 team = getTeam();
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
            Player* tagger = GetMapMgr()->GetPlayer(WoWGuid::getGuidLowPartFromUInt64(pVictim->GetTaggerGUID()));

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

        if (pVictim->getCreatedByGuid() == 0 && !pVictim->isPet() && pVictim->IsTagged())
        {
            auto unit_tagger = pVictim->GetMapMgr()->GetUnit(pVictim->GetTaggerGUID());

            if (unit_tagger != nullptr)
            {
                Player* player_tagger = nullptr;

                if (unit_tagger->isPlayer())
                    player_tagger = static_cast<Player*>(unit_tagger);

                if ((unit_tagger->isPet() || unit_tagger->isSummon()) && unit_tagger->getPlayerOwner())
                    player_tagger = static_cast<Player*>(unit_tagger->getPlayerOwner());

                if (player_tagger != nullptr)
                {
                    if (player_tagger->InGroup())
                    {
                        player_tagger->GiveGroupXP(pVictim, player_tagger);
                    }
                    else if (isCreatureOrPlayer())
                    {
                        uint32 xp = CalculateXpToGive(pVictim, unit_tagger);

                        if (xp > 0)
                        {
                            player_tagger->GiveXP(xp, pVictim->getGuid(), true);

                            addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);

                            if (!sEventMgr.HasEvent(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                                sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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

                    if (pVictim->isCreature())
                    {
                        sQuestMgr.OnPlayerKill(player_tagger, static_cast<Creature*>(pVictim), true);
#if VERSION_STRING > TBC
                        ///////////////////////////////////////////////// Kill creature/creature type Achievements /////////////////////////////////////////////////////////////////////
                        if (player_tagger->InGroup())
                        {
                            auto player_group = player_tagger->GetGroup();

                            player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->getEntry(), 1, 0);
                            player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                        }
                        else
                        {
                            player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->getEntry(), 1, 0);
                            player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
                        }
#endif
                    }
                }
            }
        }

#if VERSION_STRING > TBC
        if (pVictim->isCritter())
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->getEntry(), 1, 0);
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, getGuidHigh(), getGuidLow(), 0);
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
    if (getPowerType() == POWER_TYPE_RAGE && pAttacker != this)
    {
        float level = static_cast<float>(getLevel());
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        float val = 2.5f * damage / c;
        uint32 rage = getPower(POWER_TYPE_RAGE);

        if (rage + float2int32(val) > 1000)
            val = 1000.0f - static_cast<float>(getPower(POWER_TYPE_RAGE));

        val *= 10.0;
        modPower(POWER_TYPE_RAGE, static_cast<int32>(val));
    }

    // Cannibalize, when we are hit we need to stop munching that nice fresh corpse
    {
        if (cannibalize)
        {
            sEventMgr.RemoveEvents(this, EVENT_CANNIBALIZE);
            setEmoteState(EMOTE_ONESHOT_NONE);
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

    modHealth(-1 * static_cast<int32>(damage));
}

void Player::Die(Unit* pAttacker, uint32 /*damage*/, uint32 spellid)
{
    if (getVehicleComponent() != nullptr)
    {
        getVehicleComponent()->RemoveAccessories();
        getVehicleComponent()->EjectAllPassengers();
    }

#if VERSION_STRING > TBC
    // A Player has died
    if (isPlayer())
    {
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH, 1, 0, 0);
        GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP, GetMapId(), 1, 0);

        // A Player killed a Player
        if (pAttacker->isPlayer())
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER, 1, 0, 0);
        else if (pAttacker->isCreature()) // A Creature killed a Player
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE, 1, 0, 0);
    }
#endif

    //general hook for die
    if (!sHookInterface.OnPreUnitDie(pAttacker, this))
        return;

    // on die and an target die proc
    {
        SpellInfo const* killerspell;
        if (spellid)
            killerspell = sSpellMgr.getSpellInfo(spellid);
        else
            killerspell = nullptr;

        HandleProc(PROC_ON_DIE, this, killerspell);
        m_procCounter = 0;
        pAttacker->HandleProc(PROC_ON_TARGET_DIE, this, killerspell);
        pAttacker->m_procCounter = 0;
    }

    if (!pAttacker->isPlayer())
        DeathDurabilityLoss(0.10);

    if (getChannelObjectGuid() != 0)
    {
        Spell* spl = getCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (spl != nullptr)
        {
            for (uint8 i = 0; i < 3; i++)
            {
                if (spl->getSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64 guid = getChannelObjectGuid();
                    DynamicObject* dObj = GetMapMgr()->GetDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (!dObj)
                        continue;

                    dObj->Remove();
                }
            }

            if (spl->getSpellInfo()->getChannelInterruptFlags() == 48140)
                interruptSpell(spl->getSpellInfo()->getId());
        }
    }

    // Stop players from casting
    for (const auto& itr : getInRangePlayersSet())
    {
        Unit* attacker = static_cast<Unit*>(itr);
        if (attacker && attacker->isCastingSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                if (attacker->getCurrentSpell(CurrentSpellType(i)) == nullptr)
                    continue;

                if (attacker->getCurrentSpell(CurrentSpellType(i))->m_targets.m_unitTarget == getGuid())
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
                SpellInfo const* m_reincarnSpellInfo = sSpellMgr.getSpellInfo(20608);
                if (Cooldown_CanCast(m_reincarnSpellInfo))
                {
                    uint32 ankh_count = getItemInterface()->GetItemCount(17030);
                    if (ankh_count)
                        self_res_spell = 21169;
                }
            }
        }

        setSelfResurrectSpell(self_res_spell);
        setMountDisplayId(0);
    }

    // Wipe our attacker set on death
    CombatStatus.Vanished();

    CALL_SCRIPT_EVENT(pAttacker, OnTargetDied)(this);
    pAttacker->smsg_AttackStop(this);

    m_UnderwaterTime = 0;
    m_UnderwaterState = 0;

    summonhandler.RemoveAllSummons();
    DismissActivePets();

    setHealth(0);

    //check for spirit of Redemption
    if (HasSpell(20711))
    {
        SpellInfo const* sorInfo = sSpellMgr.getSpellInfo(27827);
        if (sorInfo != nullptr)
        {
            Spell* sor = sSpellMgr.newSpell(this, sorInfo, true, nullptr);
            SpellCastTargets targets;
            targets.m_unitTarget = getGuid();
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

    GetSession()->SendPacket(SmsgMoveKnockBack(GetNewGUID(), Util::getMSTime(), cos, sin, horizontal, -vertical).serialise().get());

    blinked = true;
    SpeedCheatDelay(10000);
}

void Player::RemoveIfVisible(uint64 obj)
{
    std::set< uint64 >::iterator itr = m_visibleObjects.find(obj);
    if (itr == m_visibleObjects.end())
        return;

    m_visibleObjects.erase(obj);
    getUpdateMgr().pushOutOfRangeGuid(obj);
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
        ObjectGuid guid = getGuid();
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
    Item* it = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it == nullptr || it->getItemProperties()->InventoryType != INVTYPE_SHIELD)
        return 0;

    float block_multiplier = (100.0f + this->m_modblockabsorbvalue) / 100.0f;
    if (block_multiplier < 1.0f)
        block_multiplier = 1.0f;

    return float2int32((it->getItemProperties()->Block + this->m_modblockvaluefromspells + this->getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + this->getStat(STAT_STRENGTH) / 2.0f - 1.0f) * block_multiplier);
}

void Player::ApplyFeralAttackPower(bool apply, Item* item)
{
    float FeralAP = 0.0f;

    Item* it = item;
    if (it == nullptr)
        it = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (it != nullptr)
    {
        float delay = (float)it->getItemProperties()->Delay / 1000.0f;
        delay = std::max(1.0f, delay);
        float dps = ((it->getItemProperties()->Damage[0].Min + it->getItemProperties()->Damage[0].Max) / 2) / delay;
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

    WorldPacket* data = sChatHandler.FillMessageData(type, lang, msg, getGuid());
    SendMessageToSet(data, true);
    delete data;
}

void Player::SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr)
{
    if (plr == nullptr)
        return;
    WorldPacket* data = sChatHandler.FillMessageData(type, lang, msg, getGuid());
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
    WoWGuid wowGuid;
    wowGuid.Init(guid);

    if (wowGuid.isUnit())
    {
        Creature* quest_giver = m_mapMgr->GetCreature(wowGuid.getGuidLowPart());
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
    else if (wowGuid.isGameObject())
    {
        GameObject* quest_giver = m_mapMgr->GetGameObject(wowGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }
    else if (wowGuid.isItem())
    {
        Item* quest_giver = getItemInterface()->GetItemByGUID(guid);
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        bSkipLevelCheck = true;
        qst = sMySQLStore.getQuestProperties(quest_id);
    }
    else if (wowGuid.isPlayer())
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

    if (qst_giver->isCreature() && static_cast< Creature* >(qst_giver)->m_escorter != nullptr)
    {
        m_session->SystemMessage("You cannot accept this quest at this time.");
        return;
    }

    // Check the player hasn't already taken this quest, or
    // it isn't available.
    uint32 status = sQuestMgr.CalcQuestStatus(qst_giver, this, qst, 3, bSkipLevelCheck);

    if ((!sQuestMgr.IsQuestRepeatable(qst) && HasFinishedQuest(qst->id))
        || (status != QuestStatus::Available && status != QuestStatus::Repeatable && status != QuestStatus::AvailableChat)
        || !hasquest)
    {
        // We've got a hacker. Disconnect them.
        //sCheatLog.writefromsession(this, "tried to accept incompatible quest %u from %u.", qst->id, qst_giver->getEntry());
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

        if (getItemInterface()->CalculateFreeSlots(nullptr) < slots_required)
        {
            getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
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

            if (!getItemInterface()->AddItemToFreeSlot(item))
            {
                item->DeleteMe();
            }
            else
                sendItemPushResultPacket(false, true, false,
                getItemInterface()->LastSearchItemBagSlot(), getItemInterface()->LastSearchItemSlot(),
                1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
        }
    }

    if (qst->srcitem && qst->srcitem != qst->receive_items[0])
    {
        if (!qst_giver->isItem() || (qst_giver->getEntry() != qst->srcitem))
        {
            Item *item = objmgr.CreateItem(qst->srcitem, this);
            if (item != nullptr)
            {
                item->setStackCount(qst->srcitemcount ? qst->srcitemcount : 1);
                if (!getItemInterface()->AddItemToFreeSlot(item))
                    item->DeleteMe();
            }
        }
    }

    if (qst->count_required_item || qst_giver->isGameObject())    // gameobject quests deactivate
        UpdateNearbyGameObjects();

    // Some spells applied at quest activation
    SpellAreaForQuestMapBounds saBounds = sSpellMgr.getSpellAreaForQuestMapBounds(quest_id, true);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second->autoCast && itr->second->fitsToRequirements(this, GetZoneId(), GetAreaID()))
                if (!HasAura(itr->second->spellId))
                    castSpell(this, itr->second->spellId, true);
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

    do {
        Field* field = result->Fetch();

        uint32 id = field[0].GetUInt32();
        uint32 flag = field[1].GetUInt32();
        int32 basestanding = field[2].GetInt32();
        int32 standing = field[3].GetInt32();

        DBC::Structures::FactionEntry const* faction = sFactionStore.LookupEntry(id);
        if (faction == nullptr || faction->RepListId < 0)
            continue;

        ReputationMap::iterator itr = m_reputation.find(id);
        if (itr != m_reputation.end())
            delete itr->second;

        FactionReputation* reputation = new FactionReputation;
        reputation->baseStanding = basestanding;
        reputation->standing = standing;
        reputation->flag = static_cast<uint8>(flag);
        m_reputation[id] = reputation;
        reputationByListId[faction->RepListId] = reputation;

    } while (result->NextRow());

    if (m_reputation.empty())
        _InitialReputation();

    return true;
}

bool Player::SaveReputations(bool NewCharacter, QueryBuffer *buf)
{
    if (!NewCharacter && (buf == nullptr))
        return false;

    std::stringstream ds;
    uint32 guid = getGuidLow();

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
        ss << getGuidLow() << "','";
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

    do
    {
        Field* fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();

        SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);
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
    uint32 guid = getGuidLow();

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
    do
    {
        Field* fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();

        SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);
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
    uint32 guid = getGuidLow();

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

    do
    {
        Field* fields = result->Fetch();

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
    uint32 guid = getGuidLow();

    ds << "DELETE FROM playerskills WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!NewCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
#if VERSION_STRING < Cata
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

void Player::addVehicleComponent(uint32 creature_entry, uint32 vehicleid)
{
    if (mountvehicleid == 0)
    {
        LOG_ERROR("Tried to add a vehicle component with 0 as vehicle id for player %u (%s)", getGuidLow(), getName().c_str());
        return;
    }

    if (m_vehicle != nullptr)
    {
        LOG_ERROR("Tried to add a vehicle component, but there's already one for player %u (%s)", getGuidLow(), getName().c_str());
        return;
    }

    m_vehicle = new Vehicle();
    m_vehicle->Load(this, creature_entry, vehicleid);
}

void Player::removeVehicleComponent()
{
    delete m_vehicle;
    m_vehicle = nullptr;
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
#if VERSION_STRING < Cata
    WorldPacket data2(MSG_MOVE_TELEPORT, 38);
    data2.append(GetNewGUID());
    BuildMovementPacket(&data2, x, y, z, o);
    SendMessageToSet(&data2, false);
#else
    LocationVector oldPos = LocationVector(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    LocationVector pos = LocationVector(x, y, z, o);

    if (getObjectTypeId() == TYPEID_UNIT)
        SetPosition(pos);

    ObjectGuid guid = getGuid();

    WorldPacket data(SMSG_MOVE_UPDATE_TELEPORT, 38);
    movement_info.writeMovementInfo(data, SMSG_MOVE_UPDATE_TELEPORT);

    if (getObjectTypeId() == TYPEID_PLAYER)
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

    if (getObjectTypeId() == TYPEID_PLAYER)
        SetPosition(pos);
    else
        SetPosition(oldPos);

    SendMessageToSet(&data, false);
#endif
}

void Player::SendTeleportAckPacket(float x, float y, float z, float o)
{
    SetPlayerStatus(TRANSFER_PENDING);

#if VERSION_STRING < WotLK
    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetNewGUID();
    data << uint32(2);
    data << uint32(0);
    data << uint8(0);

    data << float(0);
    data << x;
    data << y;
    data << z;
    data << o;
    data << uint16(2);
    data << uint8(0);
#else
    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetNewGUID();
    data << uint32(0);
    BuildMovementPacket(&data, x, y, z, o);
#endif
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
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.getSpellAreaForAreaMapBounds(AreaId);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        if (itr->second->autoCast && itr->second->fitsToRequirements(this, ZoneId, AreaId))
            if (!HasAura(itr->second->spellId))
                castSpell(this, itr->second->spellId, true);


    // Some spells applied at enter into zone (with subzones)
    SpellAreaForAreaMapBounds szBounds = sSpellMgr.getSpellAreaForAreaMapBounds(ZoneId);
    for (SpellAreaForAreaMap::const_iterator itr = szBounds.first; itr != szBounds.second; ++itr)
        if (itr->second->autoCast && itr->second->fitsToRequirements(this, ZoneId, AreaId))
            if (!HasAura(itr->second->spellId))
                castSpell(this, itr->second->spellId, true);


    //Remove of Spells
    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != nullptr)
        {
            if (sSpellMgr.checkLocation(m_auras[i]->GetSpellInfo(), ZoneId, AreaId, this) == false)
            {
                SpellAreaMapBounds sab = sSpellMgr.getSpellAreaMapBounds(m_auras[i]->GetSpellId());
                if (sab.first != sab.second)
                    RemoveAura(m_auras[i]->GetSpellId());
            }
        }
    }
}

void Player::SetGroupUpdateFlags(uint32 flags)
{
	if (GetGroup() == nullptr)
		return;
	GroupUpdateFlags = flags;
}

void Player::AddGroupUpdateFlag(uint32 flag)
{
	if (GetGroup() == nullptr)
		return;
	GroupUpdateFlags |= flag;
}

uint16 Player::GetGroupStatus()
{
    uint16 status = MEMBER_STATUS_ONLINE;
    if (isPvpFlagSet())
        status |= MEMBER_STATUS_PVP;
    if (getDeathState() == CORPSE)
        status |= MEMBER_STATUS_DEAD;
    else if (isDead())
        status |= MEMBER_STATUS_GHOST;
    if (isFfaPvpFlagSet())
        status |= MEMBER_STATUS_PVP_FFA;
    if (hasPlayerFlags(PLAYER_FLAG_AFK))
        status |= MEMBER_STATUS_AFK;
    if (hasPlayerFlags(PLAYER_FLAG_DND))
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

void Player::SendCinematicCamera(uint32 id)
{
    GetMapMgr()->ChangeObjectLocation(this);
    SetPosition(float(GetPositionX() + 0.01), float(GetPositionY() + 0.01), float(GetPositionZ() + 0.01), GetOrientation());
    GetSession()->OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &id);
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
        if ((*itr)->getGuid() == pet->getGuid())
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

    for (std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); ++itr)
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

bool Player::IsMounted()
{
    if (m_MountSpellId != 0)
        return true;
    return false;
}

#if VERSION_STRING < Cata
Player* Player::GetTradeTarget()
{
    if (!IsInWorld())
        return nullptr;
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
    setHealth(getMaxHealth());
}

void Player::SetMover(Unit* target)
{
    GetSession()->m_MoverWoWGuid.Init(target->getGuid());
}

//////////////////////////////////////////////////////////////////////////////////////////
// old functions from PlayerPacketWrapper.cpp

void Player::SendWorldStateUpdate(uint32 WorldState, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);

#if VERSION_STRING == Mop
    data.writeBit(0);
#endif
    data << uint32(WorldState);
    data << uint32(Value);

    m_session->SendPacket(&data);
}

/*Loot type MUST be
1-corpse, go
2-skinning/herbalism/minning
3-Fishing
*/
void Player::SendLoot(uint64 guid, uint8 loot_type, uint32 mapid)
{
    if (!IsInWorld())
        return;

    Loot* pLoot = NULL;

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    int8 loot_method;

    Group* m_Group = m_playerInfo->m_Group;
    if (m_Group != NULL)
        loot_method = m_Group->GetMethod();
    else
        loot_method = PARTY_LOOT_FFA;

    if (wowGuid.isUnit())
    {
        Creature* pCreature = GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
        if (!pCreature)return;
        pLoot = &pCreature->loot;
        m_currentLoot = pCreature->getGuid();

    }
    else if (wowGuid.isGameObject())
    {
        GameObject* pGO = GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
        if (!pGO)
            return;

        if (!pGO->IsLootable())
            return;

        GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pGO);
        pLGO->setState(0);
        pLoot = &pLGO->loot;
        m_currentLoot = pLGO->getGuid();
    }
    else if (wowGuid.isPlayer())
    {
        Player* p = GetMapMgr()->GetPlayer((uint32)guid);
        if (!p)
            return;

        pLoot = &p->loot;
        m_currentLoot = p->getGuid();
    }
    else if (wowGuid.isCorpse())
    {
        Corpse* pCorpse = objmgr.GetCorpse((uint32)guid);
        if (!pCorpse)
            return;

        pLoot = &pCorpse->loot;
        m_currentLoot = pCorpse->getGuid();
    }
    else if (wowGuid.isItem())
    {
        Item* pItem = getItemInterface()->GetItemByGUID(guid);
        if (!pItem)
            return;
        pLoot = pItem->loot;
        m_currentLoot = pItem->getGuid();
    }

    if (!pLoot)
    {
        // something whack happened.. damn cheaters..
        return;
    }

    // add to looter set
    pLoot->looters.insert(getGuidLow());

    WorldPacket data, data2(32);
    data.SetOpcode(SMSG_LOOT_RESPONSE);

    m_lootGuid = guid;

    data << uint64(guid);
    data << uint8(loot_type);  //loot_type;
    data << uint32(pLoot->gold);
    data << uint8(0);   //loot size reserve
#if VERSION_STRING >= Cata
    data << uint8(0);
#endif

    std::vector<__LootItem>::iterator iter = pLoot->items.begin();
    uint32 count = 0;
    uint8 slottype = 0;

    for (uint32 x = 0; iter != pLoot->items.end(); ++iter, x++)
    {
        if (iter->iItemsCount == 0)
            continue;

        LooterSet::iterator itr = iter->has_looted.find(getGuidLow());
        if (iter->has_looted.end() != itr)
            continue;

        ItemProperties const* itemProto = iter->item.itemproto;
        if (!itemProto)
            continue;

        // check if it's on ML if so only quest items and ffa loot should be shown based on mob
        if (loot_method == PARTY_LOOT_MASTER && m_Group && m_Group->GetLooter() != m_playerInfo)
            // pass on all ffa_loot and the grey / white items
            if (!iter->ffa_loot && !(itemProto->Quality < m_Group->GetThreshold()))
                continue;

        // team check
        if (itemProto->HasFlag2(ITEM_FLAG2_HORDE_ONLY) && isTeamAlliance())
            continue;

        if (itemProto->HasFlag2(ITEM_FLAG2_ALLIANCE_ONLY) && isTeamHorde())
            continue;

        //quest items check. type 4/5
        //quest items that don't start quests.
        if ((itemProto->Bonding == ITEM_BIND_QUEST) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;
        if ((itemProto->Bonding == ITEM_BIND_QUEST2) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        //quest items that start quests need special check to avoid drops all the time.
        if ((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && HasQuest(itemProto->QuestId))
            continue;
        if ((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && HasQuest(itemProto->QuestId))
            continue;

        if ((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;
        if ((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        //check for starting item quests that need questlines.
        if ((itemProto->QuestId && itemProto->Bonding != ITEM_BIND_QUEST && itemProto->Bonding != ITEM_BIND_QUEST2))
        {
            QuestProperties const* pQuest = sMySQLStore.getQuestProperties(itemProto->QuestId);
            if (pQuest)
            {
                uint32 finishedCount = 0;

                //check if its a questline.
                for (uint32 i = 0; i < pQuest->count_requiredquests; ++i)
                {
                    if (pQuest->required_quests[i])
                    {
                        if (!HasFinishedQuest(pQuest->required_quests[i]) || HasQuest(pQuest->required_quests[i]))
                        {

                        }
                        else
                        {
                            finishedCount++;
                        }
                    }
                }
            }
        }

        slottype = 0;
        if (m_Group != NULL && loot_type < 2)
        {
            switch (loot_method)
            {
                case PARTY_LOOT_MASTER:
                    slottype = 2;
                    break;
                case PARTY_LOOT_GROUP:
                case PARTY_LOOT_RR:
                case PARTY_LOOT_NBG:
                    slottype = 1;
                    break;
                default:
                    slottype = 0;
                    break;
            }
            // only quality items are distributed
            if (itemProto->Quality < m_Group->GetThreshold())
                slottype = 0;

            // if all people passed anyone can loot it? :P
            if (iter->passed)
                slottype = 0;					// All players passed on the loot

            //if it is ffa loot and not an masterlooter
            if (iter->ffa_loot)
                slottype = 0;
        }

        data << uint8(x);
        data << uint32(itemProto->ItemId);
        data << uint32(iter->iItemsCount);  //nr of items of this type
        data << uint32(iter->item.displayid);

        if (iter->iRandomSuffix)
        {
            data << uint32(Item::GenerateRandomSuffixFactor(itemProto));
            data << uint32(-int32(iter->iRandomSuffix->id));
        }
        else if (iter->iRandomProperty)
        {
            data << uint32(0);
            data << uint32(iter->iRandomProperty->ID);
        }
        else
        {
            data << uint32(0);
            data << uint32(0);
        }

        data << slottype;   // "still being rolled for" flag

        if (slottype == 1)
        {
            if (iter->roll == NULL && !iter->passed)
            {
                int32 ipid = 0;
                uint32 factor = 0;
                if (iter->iRandomProperty)
                {
                    ipid = iter->iRandomProperty->ID;
                }
                else if (iter->iRandomSuffix)
                {
                    ipid = -int32(iter->iRandomSuffix->id);
                    factor = Item::GenerateRandomSuffixFactor(iter->item.itemproto);
                }

                if (iter->item.itemproto)
                {
                    iter->roll = new LootRoll(60000, m_Group->MemberCount(), guid, x, itemProto->ItemId, factor, uint32(ipid), GetMapMgr());

                    data2.Initialize(SMSG_LOOT_START_ROLL);
                    data2 << guid;
                    data2 << uint32(mapid);
                    data2 << uint32(x);
                    data2 << uint32(itemProto->ItemId);
                    data2 << uint32(factor);

                    if (iter->iRandomProperty)
                        data2 << uint32(iter->iRandomProperty->ID);
                    else if (iter->iRandomSuffix)
                        data2 << uint32(ipid);
                    else
                        data2 << uint32(0);

                    data2 << uint32(iter->iItemsCount);
                    data2 << uint32(60000);	// countdown
                    data2 << uint8(7);		// some sort of flags that require research
                }

                if (Group* pGroup = m_playerInfo->m_Group)
                {
                    pGroup->Lock();
                    for (uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
                    {
                        for (GroupMembersSet::iterator itr2 = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr2 != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr2)
                        {
                            PlayerInfo* pinfo = *itr2;
                            if (pinfo->m_loggedInPlayer && pinfo->m_loggedInPlayer->getItemInterface()->CanReceiveItem(itemProto, iter->iItemsCount) == 0)
                            {
                                if (pinfo->m_loggedInPlayer->m_passOnLoot)
                                    iter->roll->PlayerRolled(pinfo->m_loggedInPlayer, 3);		// passed
                                else
                                    pinfo->m_loggedInPlayer->SendPacket(&data2);
                            }
                        }
                    }
                    pGroup->Unlock();
                }
                else
                {
                    m_session->SendPacket(&data2);
                }
            }
        }
        count++;
    }
    data.wpos(13);
    data << uint8(count);

    m_session->SendPacket(&data);

    addUnitFlags(UNIT_FLAG_LOOTING);
}

#ifdef AE_TBC
// MIT Start
// TODO: Move to own file
void Player::SendInitialLogonPackets()
{
    // SMSG_SET_REST_START
    m_session->OutPacket(SMSG_SET_REST_START, 4, &m_timeLogoff); // Seem to be unused by client

    m_session->SendPacket(SmsgBindPointUpdate(m_bind_pos_x, m_bind_pos_y, m_bind_pos_z, m_bind_mapid, m_bind_zoneid).serialise().get());
    m_session->SendPacket(SmsgSetProficiency(4, armor_proficiency).serialise().get());
    m_session->SendPacket(SmsgSetProficiency(2, weapon_proficiency).serialise().get());

    std::vector<uint32_t> tutorials;
    for (auto tutorial : m_Tutorials)
        tutorials.push_back(tutorial);

    m_session->SendPacket(SmsgTutorialFlags(tutorials).serialise().get());
    // Normal difficulty - TODO implement
    //m_session->SendPacket(SmsgInstanceDifficulty(0).serialise().get());

    smsg_InitialSpells();

    // MIT End

    SendInitialActions();
    smsg_InitialFactions();

    StackWorldPacket<32> data(SMSG_BINDPOINTUPDATE);
    data.Initialize(SMSG_LOGIN_SETTIMESPEED);

    data << uint32(Util::getGameTime());
    data << float(0.0166666669777748f);    // Normal Game Speed

    m_session->SendPacket(&data);

    UpdateSpeed();
    LOG_DETAIL("WORLD: Sent initial logon packets for %s.", getName().c_str());
}
#endif

#ifndef AE_TBC
void Player::SendInitialLogonPackets()
{
    // Initial Packets... they seem to be re-sent on port.
    //m_session->OutPacket(SMSG_SET_REST_START_OBSOLETE, 4, &m_timeLogoff); // Seem to be unused by client

    StackWorldPacket<32> data(SMSG_BINDPOINTUPDATE);

#if VERSION_STRING != Mop
    data << float(m_bind_pos_x);
    data << float(m_bind_pos_y);
    data << float(m_bind_pos_z);
    data << uint32(m_bind_mapid);
    data << uint32(m_bind_zoneid);
#elif VERSION_STRING == Mop
    data << float(m_bind_pos_x);
    data << float(m_bind_pos_z);
    data << float(m_bind_pos_y);
    data << uint32(m_bind_zoneid);
    data << uint32(m_bind_mapid);
#endif

    m_session->SendPacket(&data);

    //Proficiencies
    sendSetProficiencyPacket(4, armor_proficiency);
    sendSetProficiencyPacket(2, weapon_proficiency);

    //Tutorial Flags
    WorldPacket datab(SMSG_TUTORIAL_FLAGS, 4 * 8);
    for (int i = 0; i < 8; ++i)
        datab << uint32_t(m_Tutorials[i]);

    m_session->SendPacket(&datab);

#if VERSION_STRING > TBC
    smsg_TalentsInfo(false);
#endif

    smsg_InitialSpells();

#if VERSION_STRING > TBC
#if VERSION_STRING == Mop
    WorldPacket unlearnPacket(SMSG_SEND_UNLEARN_SPELLS, 4);
    unlearnPacket.writeBits(0, 22); // Count
    unlearnPacket.flushBits();
    GetSession()->SendPacket(&unlearnPacket);
#elif VERSION_STRING < Mop
    data.Initialize(SMSG_SEND_UNLEARN_SPELLS);
    data << uint32(0); // count, for (count) uint32;
    GetSession()->SendPacket(&data);
#endif
#endif

    SendInitialActions();
    smsg_InitialFactions();

    data.Initialize(SMSG_LOGIN_SETTIMESPEED);

#if VERSION_STRING != Mop
    data << uint32(Util::getGameTime());
    data << float(0.0166666669777748f);    // Normal Game Speed
#if VERSION_STRING > TBC
    data << uint32(0);   // 3.1.2
#endif
#elif VERSION_STRING == Mop
    data << uint32_t(0);
    data << uint32_t(Util::getGameTime());
    data << uint32_t(0);
    data << uint32_t(Util::getGameTime());
    data << float(0.0166666669777748f);    // Normal Game Speed
#endif

    m_session->SendPacket(&data);

    UpdateSpeed();

#if VERSION_STRING > TBC
    WorldPacket ArenaSettings(SMSG_UPDATE_WORLD_STATE, 16);

    ArenaSettings << uint32(0xC77);
    ArenaSettings << uint32(worldConfig.arena.arenaProgress);
    ArenaSettings << uint32(0xF3D);
    ArenaSettings << uint32(worldConfig.arena.arenaSeason);

    m_session->SendPacket(&ArenaSettings);
#endif
    LOG_DETAIL("WORLD: Sent initial logon packets for %s.", getName().c_str());
}
#endif

void Player::SendLootUpdate(Object* o)
{
    if (!IsVisible(o->getGuid()))
        return;

    if (o->isCreatureOrPlayer())
    {
        // Build the actual update.
        ByteBuffer buf(500);

        uint32 Flags = static_cast<Unit*>(o)->getDynamicFlags();

        Flags |= U_DYN_FLAG_LOOTABLE;
        Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

        o->BuildFieldUpdatePacket(&buf, UNIT_DYNAMIC_FLAGS, Flags);

        getUpdateMgr().pushUpdateData(&buf, 1);
    }
}

void Player::SendUpdateDataToSet(ByteBuffer* groupbuf, ByteBuffer* nongroupbuf, bool sendtoself)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // first case we need to send to both grouped and ungrouped players in the set
    if (groupbuf != nullptr && nongroupbuf != nullptr)
    {
        for (const auto& itr : getInRangePlayersSet())
        {
            Player* p = static_cast<Player*>(itr);

            if (p->GetGroup() != nullptr && GetGroup() != nullptr && p->GetGroup()->GetID() == GetGroup()->GetID())
                p->getUpdateMgr().pushUpdateData(groupbuf, 1);
            else
                p->getUpdateMgr().pushUpdateData(nongroupbuf, 1);
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        //second case we send to group only
        if (groupbuf != nullptr && nongroupbuf == nullptr)
        {
            for (const auto& itr : getInRangePlayersSet())
            {
                Player* p = static_cast<Player*>(itr);
                if (p && p->GetGroup() != nullptr && GetGroup() != nullptr && p->GetGroup()->GetID() == GetGroup()->GetID())
                    p->getUpdateMgr().pushUpdateData(groupbuf, 1);
            }
        }
        else
        {
            //////////////////////////////////////////////////////////////////////////////////////////
            //Last case we send to nongroup only
            if (groupbuf == nullptr && nongroupbuf != nullptr)
            {
                for (const auto& itr : getInRangePlayersSet())
                {
                    if (itr)
                    {
                        Player* p = static_cast<Player*>(itr);
                        if (p->GetGroup() == nullptr || p->GetGroup()->GetID() != GetGroup()->GetID())
                            p->getUpdateMgr().pushUpdateData(nongroupbuf, 1);
                    }
                }
            }
        }
    }

    if (sendtoself && groupbuf != nullptr)
        getUpdateMgr().pushUpdateData(groupbuf, 1);
}

void Player::TagUnit(Object* o)
{
    if (o->isCreatureOrPlayer())
    {
        // For new players who get a create object
        uint32 Flags = static_cast<Unit*>(o)->getDynamicFlags();
        Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

        // Update existing players.
        ByteBuffer buf(500);
        ByteBuffer buf1(500);

        o->BuildFieldUpdatePacket(&buf1, UNIT_DYNAMIC_FLAGS, Flags);
        o->BuildFieldUpdatePacket(&buf, UNIT_DYNAMIC_FLAGS, o->getUInt32Value(UNIT_DYNAMIC_FLAGS));

        SendUpdateDataToSet(&buf1, &buf, true);
    }
}

void Player::SendEquipmentSetList()
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_EQUIPMENT_SET_LIST, 1000);

    getItemInterface()->m_EquipmentSets.FillEquipmentSetListPacket(data);
    m_session->SendPacket(&data);

    LOG_DEBUG("Sent SMSG_EQUIPMENT_SET_LIST.");
#endif
}

void Player::SendEquipmentSetSaved(uint32 setID, uint32 setGUID)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_EQUIPMENT_SET_SAVED, 12);
    data << uint32(setID);
    data << WoWGuid(uint64(setGUID));

    m_session->SendPacket(&data);

    LOG_DEBUG("Sent SMSG_EQUIPMENT_SET_SAVED.");
#endif
}

void Player::SendInitialWorldstates()
{
#if VERSION_STRING < Cata
    WorldPacket data(SMSG_INIT_WORLD_STATES, 100);

    m_mapMgr->GetWorldStatesHandler().BuildInitWorldStatesForZone(m_zoneId, m_AreaID, data);
    m_session->SendPacket(&data);
#endif
}
// end L15420 12/11/2018 Zyres
