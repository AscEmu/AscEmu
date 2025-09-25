/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <zlib.h>

#include "Player.hpp"

#include "TradeData.hpp"
#include "Chat/ChatDefines.hpp"
#include "Data/WoWPlayer.hpp"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Macros/CorpseMacros.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/AuctionHouse.h"
#include "Management/Charter.hpp"
#include "Management/Group.h"
#include "Management/HonorHandler.h"
#include "Management/Battleground/Battleground.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ItemInterface.h"
#include "Management/Loot/LootMgr.hpp"
#include "Management/MailMgr.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Skill.hpp"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/GameObject.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Management/TaxiMgr.hpp"
#include "Management/WeatherMgr.hpp"
#include "Management/Tickets/TicketMgr.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Container.hpp"
#include "Objects/DynamicObject.hpp"
#include "Server/Opcodes.hpp"
#include "Server/Packets/SmsgActivateTaxiReply.h"
#include "Server/Packets/SmsgTaxinodeStatus.h"
#include "Server/Packets/MsgTalentWipeConfirm.h"
#include "Server/Packets/SmsgPetUnlearnConfirm.h"
#include "Server/Packets/MsgSetDungeonDifficulty.h"
#include "Server/Packets/MsgSetRaidDifficulty.h"
#include "Server/Packets/SmsgInstanceDifficulty.h"
#include "Server/Packets/SmsgCrossedInebriationThreshold.h"
#include "Server/Packets/SmsgSetProficiency.h"
#include "Server/Packets/SmsgPartyKillLog.h"
#include "Server/Packets/SmsgEquipmentSetUseResult.h"
#include "Server/Packets/SmsgTotemCreated.h"
#include "Server/Packets/SmsgGossipPoi.h"
#include "Server/Packets/SmsgStopMirrorTimer.h"
#include "Server/Packets/SmsgMeetingstoneSetQueue.h"
#include "Server/Packets/SmsgPlayObjectSound.h"
#include "Server/Packets/SmsgPlaySound.h"
#include "Server/Packets/SmsgExplorationExperience.h"
#include "Server/Packets/SmsgCooldownEvent.h"
#include "Server/Packets/SmsgSetFlatSpellModifier.h"
#include "Server/Packets/SmsgSetPctSpellModifier.h"
#include "Server/Packets/SmsgLoginVerifyWorld.h"
#include "Server/Packets/SmsgMountResult.h"
#include "Server/Packets/SmsgDismountResult.h"
#include "Server/Packets/SmsgLogXpGain.h"
#include "Server/Packets/SmsgCastFailed.h"
#include "Server/Packets/SmsgLevelupInfo.h"
#include "Server/Packets/SmsgItemPushResult.h"
#include "Server/Packets/SmsgClientControlUpdate.h"
#include "Server/Packets/SmsgGuildEvent.h"
#include "Server/Packets/SmsgDestoyObject.h"
#include "Server/Packets/SmsgNewWorld.h"
#include "Server/Packets/SmsgPvpCredit.h"
#include "Server/Packets/SmsgRaidGroupOnly.h"
#include "Server/Packets/SmsgAuctionCommandResult.h"
#include "Server/Packets/SmsgClearCooldown.h"
#include "Server/Packets/SmsgLootReleaseResponse.h"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Server/Packets/SmsgInstanceReset.h"
#include "Server/Packets/SmsgStableResult.h"
#include "Server/Packets/SmsgPetSpells.h"
#include "Server/World.h"
#include "Server/WorldSocket.h"
#include "Server/Packets/SmsgContactList.h"
#include "Server/Packets/SmsgFriendStatus.h"
#include "Spell/Definitions/AuraInterruptFlags.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/Spec.hpp"
#include "Spell/Definitions/SpellDamageType.hpp"
#include "Spell/Definitions/SpellFailure.hpp"
#include "Spell/Definitions/SpellIsFlags.hpp"
#include "Spell/Definitions/SummonTypes.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellDefines.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/UnitDefines.hpp"
#include "Server/Definitions.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Packets/SmsgAreaTriggerMessage.h"
#include "Server/Packets/SmsgAttackSwingBadFacing.h"
#include "Server/Packets/SmsgAttackSwingNotInRange.h"
#include "Server/Packets/SmsgBindPointUpdate.h"
#include "Server/Packets/SmsgCancelCombat.h"
#include "Server/Packets/SmsgCharacterLoginFailed.h"
#include "Server/Packets/SmsgCorpseReclaimDelay.h"
#include "Server/Packets/SmsgDeathReleaseLoc.h"
#include "Server/Packets/SmsgDuelComplete.h"
#include "Server/Packets/SmsgDuelInbounds.h"
#include "Server/Packets/SmsgDuelOutOfBounds.h"
#include "Server/Packets/SmsgDuelRequested.h"
#include "Server/Packets/SmsgDuelWinner.h"
#include "Server/Packets/SmsgDurabilityDamageDeath.h"
#include "Server/Packets/SmsgSendKnownSpells.h"
#include "Server/Packets/SmsgLearnedSpell.h"
#include "Server/Packets/SmsgLoginSetTimeSpeed.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Server/Packets/SmsgPreResurrect.h"
#include "Server/Packets/SmsgRemovedSpell.h"
#include "Server/Packets/SmsgSendUnlearnSpells.h"
#include "Server/Packets/SmsgSetFactionStanding.h"
#include "Server/Packets/SmsgSetFactionVisible.h"
#include "Server/Packets/SmsgPhaseShiftChange.h"
#include "Server/Packets/SmsgTriggerMovie.h"
#include "Server/Packets/SmsgTriggerCinematic.h"
#include "Server/Packets/SmsgSpellCooldown.h"
#include "Server/Packets/SmsgStartMirrorTimer.h"
#include "Server/Packets/SmsgSummonRequest.h"
#include "Server/Packets/SmsgSupercededSpell.h"
#include "Server/Packets/SmsgTimeSyncRequest.h"
#include "Server/Packets/SmsgTitleEarned.h"
#include "Server/Packets/SmsgTransferAborted.h"
#include "Server/Packets/SmsgTransferPending.h"
#include "Server/Packets/SmsgTutorialFlags.h"
#include "Server/Packets/SmsgUpdateWorldState.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Warden/SpeedDetector.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/WorldStrings.h"
#include "Utilities/Strings.hpp"
#include "Objects/Transporter.hpp"
#include "Movement/MovementGenerators/FlightPathMovementGenerator.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include <cstdarg>

#include "Utilities/Narrow.hpp"

#if VERSION_STRING > TBC
    #include "Management/AchievementMgr.h"
#endif

using namespace AscEmu::Packets;
using namespace MapManagement::AreaManagement;
using namespace InstanceDifficulty;

CachedCharacterInfo::CachedCharacterInfo() = default;

CachedCharacterInfo::CachedCharacterInfo(Field const* fields)
{
    guid = fields[0].asUint32();

    std::string characterNameDB = fields[1].asCString();
    AscEmu::Util::Strings::capitalize(characterNameDB);

    name = characterNameDB;
    race = fields[2].asUint8();
    cl = fields[3].asUint8();
    lastLevel = fields[4].asUint32();
    gender = fields[5].asUint8();
    lastZone = fields[6].asUint32();
    lastOnline = fields[7].asUint32();
    acct = fields[8].asUint32();
    m_Group = nullptr;
    subGroup = 0;
    m_guild = 0;
    guildRank = GUILD_RANK_NONE;
    team = getSideByRace(race);
}

CachedCharacterInfo::~CachedCharacterInfo()
{
    if (m_Group != nullptr)
        m_Group->RemovePlayer(this);
}

Player::Player(uint32_t guid) :
    m_updateMgr(this, static_cast<size_t>(worldConfig.server.compressionThreshold), 40000, 30000, 1000),
    m_nextSave(Util::getMSTime() + worldConfig.getIntRate(INTRATE_SAVE)),
    m_mailBox(std::make_unique<Mailbox>(guid)),
    m_speedCheatDetector(std::make_unique<SpeedCheatDetector>()),
    m_groupUpdateFlags(GROUP_UPDATE_FLAG_NONE),
    m_TradeData(nullptr)
{
    //////////////////////////////////////////////////////////////////////////
    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;
    m_valuesCount = getSizeOfStructure(WoWPlayer);
    //////////////////////////////////////////////////////////////////////////

    //\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (getSizeOfStructure(WoWPlayer)) * sizeof(uint32_t));
    m_updateMask.SetCount(getSizeOfStructure(WoWPlayer));

    setObjectType(TYPEID_PLAYER);
    setGuidLow(guid);

#if VERSION_STRING >= WotLK
    setRuneRegen(0, 0.100000f);
    setRuneRegen(1, 0.100000f);
    setRuneRegen(2, 0.100000f);
    setRuneRegen(3, 0.100000f);
#endif

    m_playerControler = this;

    setAttackPowerMultiplier(0.f);
    setRangedAttackPowerMultiplier(0.f);

    m_sentTeleportPosition.ChangeCoords({ 999999.0f, 999999.0f, 999999.0f });

    // Zyres: initialise here because ItemInterface needs the guid from object data
    m_itemInterface = std::make_unique<ItemInterface>(this);
#if VERSION_STRING > TBC
    // make_unique does not work with private ctor -Appled
    m_achievementMgr = std::unique_ptr<AchievementMgr>(new AchievementMgr(this));
#endif
    m_taxi = std::make_unique<TaxiPath>();

    m_underwaterLastDamage = Util::getMSTime();
    m_explorationTimer = Util::getMSTime();

    std::fill(m_questlog.begin(), m_questlog.end(), nullptr);
#if VERSION_STRING >= Cata
    std::fill(_voidStorageItems.begin(), _voidStorageItems.end(), nullptr);
#endif

    // Override initialization from Unit class
    getThreatManager().initialize();
}

Player::~Player()
{
    if (!m_isReadyToBeRemoved)
    {
        sLogger.failure("Player deleted from non-logout player!");
        sObjectMgr.removePlayer(this);
    }

    if (m_session)
    {
        m_session->SetPlayer(nullptr);
        if (!m_isReadyToBeRemoved)
            m_session->Disconnect();
    }

    if (m_TradeData != nullptr)
        cancelTrade(false);

    if (Player* inviterPlayer = sObjectMgr.getPlayer(getGroupInviterId()))
        inviterPlayer->setGroupInviterId(0);

    if (m_duelPlayer != nullptr)
        m_duelPlayer->m_duelPlayer = nullptr;

    m_duelPlayer = nullptr;

    m_cachedPets.clear();
    removeGarbageItems();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Essential functions
void Player::Update(unsigned long time_passed)
{
    if (getSession() && !getSession()->GetSocket())
    {
        getSession()->LogoutPlayer(false);
        return;
    }

    if (!IsInWorld())
        return;

    Unit::Update(time_passed);
    uint32_t mstime = Util::getMSTime();

    if (m_attacking)
    {
        // Check attack timer.
        if (isAttackReady(MELEE))
            _eventAttack(false);

        if (hasOffHandWeapon() && isAttackReady(OFFHAND))
            _eventAttack(true);
    }

    // Breathing
    if (m_underwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        // keep subtracting timer
        if (m_underwaterTime)
        {
            // not taking dmg yet
            if (time_passed >= m_underwaterTime)
                m_underwaterTime = 0;
            else
                m_underwaterTime -= time_passed;
        }

        if (!m_underwaterTime)
        {
            // check last damage dealt timestamp, and if enough time has elapsed deal damage
            if (mstime >= m_underwaterLastDamage)
            {
                uint32_t damage = getMaxHealth() / 10;

                sendEnvironmentalDamageLogPacket(getGuid(), DAMAGE_DROWNING, damage);
                addSimpleEnvironmentalDamageBatchEvent(DAMAGE_DROWNING, damage);
                m_underwaterLastDamage = mstime + 1000;
            }
        }
    }
    else
    {
        // check if we're not on a full breath timer
        if (m_underwaterTime < m_underwaterMaxTime)
        {
            // regenning
            m_underwaterTime += (time_passed * 10);

            if (m_underwaterTime >= m_underwaterMaxTime)
            {
                m_underwaterTime = m_underwaterMaxTime;
                sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
            }
        }
    }

    // Lava Damage
    if (m_underwaterState & UNDERWATERSTATE_LAVA)
    {
        // check last damage dealt timestamp, and if enough time has elapsed deal damage
        if (mstime >= m_underwaterLastDamage)
        {
            uint32_t damage = getMaxHealth() / 5;

            sendEnvironmentalDamageLogPacket(getGuid(), DAMAGE_LAVA, damage);
            addSimpleEnvironmentalDamageBatchEvent(DAMAGE_LAVA, damage);
            m_underwaterLastDamage = mstime + 1000;
        }
    }

    // Autosave
    if (mstime >= m_nextSave)
        saveToDB(false);

    // Exploration
    if (mstime >= m_explorationTimer)
    {
        eventExploration();
        m_explorationTimer = mstime + 3000;
    }

    // Autocast Spells in Area
    if (time_passed >= m_spellAreaUpdateTimer)
    {
        _castSpellArea();
        m_spellAreaUpdateTimer = 1000;
    }
    else
    {
        m_spellAreaUpdateTimer -= static_cast<uint16_t>(time_passed);
    }

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

    indoorCheckUpdate(mstime);

    if (m_serversideDrunkValue > 0)
    {
        m_drunkTimer += time_passed;

        if (m_drunkTimer > 10000)
            handleSobering();
    }

    // Instance Hourly Limit
    if (!m_instanceResetTimes.empty())
    {
        time_t now = Util::getTimeNow();
        for (InstanceTimeMap::iterator itr = m_instanceResetTimes.begin(); itr != m_instanceResetTimes.end();)
        {
            if (itr->second < now)
                m_instanceResetTimes.erase(itr++);
            else
                ++itr;
        }
    }

    // Instance Binds
    if (hasPendingBind())
    {
        if (m_pendingBindTimer <= time_passed)
        {
            // Player left the instance
            if (m_pendingBindId == static_cast<uint32_t>(GetInstanceID()))
                bindToInstance();
            setPendingBind(0, 0);
        }
        else
        {
            m_pendingBindTimer -= time_passed;
        }
    }

    if (m_timeSyncTimer > 0)
    {
        if (time_passed >= m_timeSyncTimer)
            sendTimeSync();
        else
            m_timeSyncTimer -= time_passed;
    }

    if (time_passed >= m_partyUpdateTimer)
    {
        sendUpdateToOutOfRangeGroupMembers();
        m_partyUpdateTimer = 1000;
    }
    else
    {
        m_partyUpdateTimer -= static_cast<uint16_t>(time_passed);
    }

    // Update items
    if (m_itemUpdateTimer >= 1000)
    {
        removeGarbageItems();
        getItemInterface()->update(m_itemUpdateTimer);
        m_itemUpdateTimer = 0;
    }
    else
    {
        m_itemUpdateTimer += time_passed;
    }
}

void Player::AddToWorld()
{
    if (auto transport = this->GetTransport())
    {
        this->SetPosition(transport->GetPositionX() + GetTransOffsetX(),
            transport->GetPositionY() + GetTransOffsetY(),
            transport->GetPositionZ() + GetTransOffsetZ(),
            GetOrientation(), false);
    }

    // If we join an invalid instance and get booted out, this will prevent our stats from doubling :P
    if (IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld();

    if (m_WorldMap == nullptr)
    {
        m_beingPushed = false;
        ejectFromInstance();
        return;
    }

    if (m_session)
        m_session->SetInstance(m_WorldMap->getInstanceId());

#if VERSION_STRING > TBC
    sendInstanceDifficultyPacket(m_WorldMap->getDifficulty());
#endif
}

void Player::AddToWorld(WorldMap* pMapMgr)
{
    if (auto transport = this->GetTransport())
    {
        auto t_loc = transport->GetPosition();
        this->SetPosition(t_loc.x + this->GetTransOffsetX(),
            t_loc.y + this->GetTransOffsetY(),
            t_loc.z + this->GetTransOffsetZ(),
            this->GetOrientation(), false);
    }

    // If we join an invalid instance and get booted out, this will prevent our stats from doubling :P
    if (IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld(pMapMgr);

    if (m_WorldMap == nullptr)
    {
        m_beingPushed = false;
        ejectFromInstance();
        return;
    }

    if (m_session)
        m_session->SetInstance(m_WorldMap->getInstanceId());

#if VERSION_STRING > TBC
    sendInstanceDifficultyPacket(m_WorldMap->getDifficulty());
#endif
}

void Player::OnPrePushToWorld()
{
    sendInitialLogonPackets();
#if VERSION_STRING > TBC
    m_achievementMgr->sendAllAchievementData(this);
#endif

    // Send initial power regen modifiers before push
    updateManaRegeneration(true);
    updateRageRegeneration(true);
    updateFocusRegeneration(true);
    updateEnergyRegeneration(true);
#if VERSION_STRING >= WotLK
    updateRunicPowerRegeneration(true);
#endif
}

void Player::OnPushToWorld()
{
    uint8_t class_ = getClass();
    uint8_t startlevel = 1;

    // Process create packet
    processPendingUpdates();

    if (m_teleportState == 2)   // Worldport Ack
        onWorldPortAck();

    speedCheatReset();
    m_beingPushed = false;
    addItemsToWorld();

    // set fly if cheat is active
    setMoveCanFly(m_cheats.hasFlyCheat);

    getMovementManager()->initialize();

    // Update PVP Situation
    setupPvPOnLogin();

    if (m_playerInfo->lastOnline + 900 < UNIXTIME)    // did we logged out for more than 15 minutes?
        getItemInterface()->RemoveAllConjured();

    Unit::OnPushToWorld();

    sHookInterface.OnEnterWorld(this);

    if (m_WorldMap && m_WorldMap->getScript())
    {
        m_WorldMap->getScript()->OnZoneChange(this, m_zoneId, 0);
        m_WorldMap->getScript()->OnPlayerEnter(this);
    }

    if (m_teleportState == 1 || m_enteringWorld)        // First world enter
        completeLoading();

    m_enteringWorld = false;
    m_teleportState = 0;

    // can only fly in outlands or northrend (northrend requires cold weather flying)
    if (m_flyingAura && ((m_mapId != 530) && (m_mapId != 571 || !hasSpell(54197) && getDeathState() == ALIVE)))
    {
        removeAllAurasById(m_flyingAura);
        m_flyingAura = 0;
    }

    // send weather
    sWeatherMgr.sendWeather(this);

    setHealth(m_loadHealth > getMaxHealth() ? getMaxHealth() : m_loadHealth);
    if (getPowerType() == POWER_TYPE_MANA)
        setPower(POWER_TYPE_MANA, (m_loadMana > getMaxPower(POWER_TYPE_MANA) ? getMaxPower(POWER_TYPE_MANA) : m_loadMana));

    if (m_firstLogin)
    {
        if (class_ == DEATHKNIGHT)
            startlevel = static_cast<uint8_t>(std::max(55, worldConfig.player.playerStartingLevel));
        else
            startlevel = static_cast<uint8_t>(worldConfig.player.playerStartingLevel);

        applyLevelInfo(startlevel);

        setHealthPct(100);

        // Sometimes power types aren't initialized - so initialize it again
        switch (getClass())
        {
            case WARRIOR:
                setMaxPower(POWER_TYPE_RAGE, 1000);
                setPower(POWER_TYPE_RAGE, 0);
                break;
            case ROGUE:
                setMaxPower(POWER_TYPE_ENERGY, 100);
                setPower(POWER_TYPE_ENERGY, 100);
                break;
#if VERSION_STRING >= WotLK
            case DEATHKNIGHT:
                setMaxPower(POWER_TYPE_RUNES, 8);
                setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
                setPower(POWER_TYPE_RUNES, 8);
                break;
#endif
#if VERSION_STRING >= Cata
            case HUNTER:
                setPower(POWER_TYPE_FOCUS, 0);
                setMaxPower(POWER_TYPE_FOCUS, 100);
#endif
            default:
                setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
                break;
        }
        m_firstLogin = false;

        for (uint32_t spellId : m_playerCreateInfo->spell_cast_list)
            castSpell(this, spellId, true);
    }

    if (!getSession()->HasGMPermissions())
        getItemInterface()->CheckAreaItems();

    if (m_WorldMap->getBaseMap()->isBattlegroundOrArena())
    {
        if (m_WorldMap && reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground() != nullptr && m_bg != reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground())
            reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground()->portPlayer(this, true);
    }

    if (m_bg != nullptr)
    {
        m_bg->OnAddPlayer(this);   // add buffs and so, must be after zone update and related aura removal
        m_bg->onPlayerPushed(this);
    }

    m_changingMaps = false;
    sendFullAuraUpdate();

    getItemInterface()->HandleItemDurations();
    // Send enchant durations for unequipped items
    getItemInterface()->sendEnchantDurations();

    sendInitialWorldstates();

    if (m_resetTalents)
    {
        resetTalents();
        m_resetTalents = false;
    }

    summonTemporarilyUnsummonedPet();

#if VERSION_STRING == Mop
    updateVisibility();

    WorldPacket data(SMSG_LOAD_CUF_PROFILES, 1);
    data.writeBits(0, 20);
    data.flushBits();
    sendPacket(&data);

    data.Initialize(SMSG_BATTLE_PET_JOURNAL);
    data.writeBits(0, 19);
    data.writeBit(1);
    data.writeBits(0, 25);
    data.flushBits();
    data << uint16_t(0);
    sendPacket(&data);

    data.Initialize(SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED);
    sendPacket(&data);

#endif

    sendTaxiNodeStatusMultiple();
    continueTaxiFlight();
}

void Player::removeFromWorld()
{
    if (m_sendOnlyRaidgroup)
        event_RemoveEvents(EVENT_PLAYER_EJECT_FROM_INSTANCE);

    m_loadHealth = getHealth();
    m_loadMana = getPower(POWER_TYPE_MANA);

    if (m_bg)
        m_bg->removePlayer(this, true);

    // Cancel trade if it's active.
    if (m_TradeData != nullptr)
        cancelTrade(false);

    //stop dueling
    if (m_duelPlayer != nullptr)
        m_duelPlayer->endDuel(DUEL_WINNER_RETREAT);

    //clear buyback
    getItemInterface()->EmptyBuyBack();

    // Keep current pet active, Unit::RemoveFromWorld removes other summons
    unSummonPetTemporarily();

    if (m_summonedObject)
    {
        if (m_summonedObject->GetInstanceID() != GetInstanceID())
        {
            m_summonedObject->ToGameObject()->despawn(100, 0);
        }
        else
        {
            if (m_summonedObject->IsInWorld())
                m_summonedObject->RemoveFromWorld(true);

            delete m_summonedObject;
        }
        m_summonedObject = nullptr;
    }

    if (IsInWorld())
    {
        removeItemsFromWorld();
        Unit::RemoveFromWorld(false);
    }

    m_changingMaps = true;
    m_playerInfo->lastOnline = UNIXTIME; // don't destroy conjured items yet
}

//////////////////////////////////////////////////////////////////////////////////////////
// Data
uint64_t Player::getDuelArbiter() const { return playerData()->duel_arbiter; }
void Player::setDuelArbiter(uint64_t guid) { write(playerData()->duel_arbiter, guid); }

uint32_t Player::getPlayerFlags() const { return playerData()->player_flags; }
void Player::setPlayerFlags(uint32_t flags)
{
    write(playerData()->player_flags, flags);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update player flags also to group
    if (!IsInWorld() || getGroup() == nullptr)
        return;

    addGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);
}
void Player::addPlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() | flags); }
void Player::removePlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() & ~flags); }
bool Player::hasPlayerFlags(uint32_t flags) const { return (getPlayerFlags() & flags) != 0; }

uint32_t Player::getGuildId() const
{
#if VERSION_STRING < Cata
    return playerData()->guild_id;
#else
    return static_cast<uint32_t>(objectData()->data);
#endif
}
void Player::setGuildId(uint32_t guildId)
{
#if VERSION_STRING < Cata
    write(playerData()->guild_id, guildId);
#else
    write(objectData()->data, WoWGuid(guildId, 0, HIGHGUID_TYPE_GUILD).getRawGuid());

    if (guildId)
        addPlayerFlags(PLAYER_FLAG_GUILD_LVL_ENABLED);
    else
        removePlayerFlags(PLAYER_FLAG_GUILD_LVL_ENABLED);

    write(objectData()->field_type.parts.guild_id, static_cast<uint16_t>(guildId != 0 ? 1 : 0));
#endif
}

uint32_t Player::getGuildRank() const { return playerData()->guild_rank; }
void Player::setGuildRank(uint32_t guildRank) { write(playerData()->guild_rank, guildRank); }

#if VERSION_STRING >= Cata
uint32_t Player::getGuildLevel() const { return playerData()->guild_level; }
void Player::setGuildLevel(uint32_t guildLevel) { write(playerData()->guild_level, guildLevel); }
#endif

//bytes begin
uint32_t Player::getPlayerBytes() const { return playerData()->player_bytes.raw; }
void Player::setPlayerBytes(uint32_t bytes) { write(playerData()->player_bytes.raw, bytes); }

uint8_t Player::getSkinColor() const { return playerData()->player_bytes.s.skin_color; }
void Player::setSkinColor(uint8_t color) { write(playerData()->player_bytes.s.skin_color, color); }

uint8_t Player::getFace() const { return playerData()->player_bytes.s.face; }
void Player::setFace(uint8_t face) { write(playerData()->player_bytes.s.face, face); }

uint8_t Player::getHairStyle() const { return playerData()->player_bytes.s.hair_style; }
void Player::setHairStyle(uint8_t style) { write(playerData()->player_bytes.s.hair_style, style); }

uint8_t Player::getHairColor() const { return playerData()->player_bytes.s.hair_color; }
void Player::setHairColor(uint8_t color) { write(playerData()->player_bytes.s.hair_color, color); }
//bytes end

//bytes2 begin
uint32_t Player::getPlayerBytes2() const { return playerData()->player_bytes_2.raw; }
void Player::setPlayerBytes2(uint32_t bytes2) { write(playerData()->player_bytes_2.raw, bytes2); }

uint8_t Player::getFacialFeatures() const { return playerData()->player_bytes_2.s.facial_hair; }
void Player::setFacialFeatures(uint8_t feature) { write(playerData()->player_bytes_2.s.facial_hair, feature); }

uint8_t Player::getBankSlots() const { return playerData()->player_bytes_2.s.bank_slots; }
void Player::setBankSlots(uint8_t slots) { write(playerData()->player_bytes_2.s.bank_slots, slots); }

uint8_t Player::getRestState() const { return playerData()->player_bytes_2.s.rest_state; }
void Player::setRestState(uint8_t state) { write(playerData()->player_bytes_2.s.rest_state, state); }
//bytes2 end

//bytes3 begin
uint32_t Player::getPlayerBytes3() const { return playerData()->player_bytes_3.raw; }
void Player::setPlayerBytes3(uint32_t bytes3) { write(playerData()->player_bytes_3.raw, bytes3); }

uint8_t Player::getPlayerGender() const { return playerData()->player_bytes_3.s.gender; }
void Player::setPlayerGender(uint8_t gender) { write(playerData()->player_bytes_3.s.gender, gender); }

uint8_t Player::getDrunkValue() const { return playerData()->player_bytes_3.s.drunk_value; }
void Player::setDrunkValue(uint8_t value) { write(playerData()->player_bytes_3.s.drunk_value, value); }

uint8_t Player::getPvpRank() const { return playerData()->player_bytes_3.s.pvp_rank; }
void Player::setPvpRank(uint8_t rank) { write(playerData()->player_bytes_3.s.pvp_rank, rank); }

#if VERSION_STRING >= TBC
uint8_t Player::getArenaFaction() const { return playerData()->player_bytes_3.s.arena_faction; }
void Player::setArenaFaction(uint8_t faction) { write(playerData()->player_bytes_3.s.arena_faction, faction); }
#endif
//bytes3 end

uint32_t Player::getDuelTeam() const { return playerData()->duel_team; }
void Player::setDuelTeam(uint32_t team) { write(playerData()->duel_team, team); }

uint32_t Player::getGuildTimestamp() const { return playerData()->guild_timestamp; }
void Player::setGuildTimestamp(uint32_t timestamp) { write(playerData()->guild_timestamp, timestamp); }

//QuestLog start
uint32_t Player::getQuestLogEntryForSlot(uint8_t slot) const { return playerData()->quests[slot].quest_id; }
void Player::setQuestLogEntryBySlot(uint8_t slot, uint32_t questEntry) { write(playerData()->quests[slot].quest_id, questEntry); }

#if VERSION_STRING > Classic
uint32_t Player::getQuestLogStateForSlot(uint8_t slot) const { return playerData()->quests[slot].state; }
void Player::setQuestLogStateBySlot(uint8_t slot, uint32_t state) { write(playerData()->quests[slot].state, state); }
#else
uint32_t Player::getQuestLogStateForSlot(uint8_t slot) const
{
    //\todo: get last 1*8 bits as state
    return playerData()->quests[slot].required_count_state;
}

void Player::setQuestLogStateBySlot(uint8_t slot, uint32_t state)
{
    //\todo: write last 1*8 bits as state
    write(playerData()->quests[slot].required_count_state, state);
}
#endif

#if VERSION_STRING > TBC
uint64_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const { return playerData()->quests[slot].required_mob_or_go; }
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint64_t mobOrGoCount) { write(playerData()->quests[slot].required_mob_or_go, mobOrGoCount); }
#elif VERSION_STRING == TBC
uint32_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const { return playerData()->quests[slot].required_mob_or_go; }
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint32_t mobOrGoCount) { write(playerData()->quests[slot].required_mob_or_go, mobOrGoCount); }
#else
uint32_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const
{
    //\todo: get first 4*6 bits as required count
    return playerData()->quests[slot].required_count_state;
}
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint32_t mobOrGoCount)
{
    //\todo: write first 4*6 bits as required count
    write(playerData()->quests[slot].required_count_state, mobOrGoCount);
}
#endif

uint32_t Player::getQuestLogExpireTimeForSlot(uint8_t slot) const { return playerData()->quests[slot].expire_time; }
void Player::setQuestLogExpireTimeBySlot(uint8_t slot, uint32_t expireTime) { write(playerData()->quests[slot].expire_time, expireTime); }
//QuestLog end

//VisibleItem start
uint32_t Player::getVisibleItemEntry(uint32_t slot) const { return playerData()->visible_items[slot].entry; }
void Player::setVisibleItemEntry(uint32_t slot, uint32_t entry) { write(playerData()->visible_items[slot].entry, entry); }

#if VERSION_STRING > TBC
uint16_t Player::getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const
{
    if (pos > TEMP_ENCHANTMENT_SLOT)
        return 0;

    return playerData()->visible_items[slot].enchantment.raw[pos];
}
void Player::setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint16_t enchantment)
{
    if (pos > TEMP_ENCHANTMENT_SLOT)
        return;

    write(playerData()->visible_items[slot].enchantment.raw[pos], enchantment);
}
#else
uint32_t Player::getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const { return playerData()->visible_items[slot].enchantment[pos]; }
void Player::setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint32_t enchantment)  { write(playerData()->visible_items[slot].enchantment[pos], enchantment); }
#endif
//VisibleItem end

uint64_t Player::getInventorySlotItemGuid(uint8_t slot) const { return playerData()->inventory_slot[slot]; }
void Player::setInventorySlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->inventory_slot[slot], guid); }

uint64_t Player::getPackSlotItemGuid(uint8_t slot) const { return playerData()->pack_slot[slot]; }
void Player::setPackSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->pack_slot[slot], guid); }

uint64_t Player::getBankSlotItemGuid(uint8_t slot) const { return playerData()->bank_slot[slot]; }
void Player::setBankSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->bank_slot[slot], guid); }

uint64_t Player::getBankBagSlotItemGuid(uint8_t slot) const { return playerData()->bank_bag_slot[slot]; }
void Player::setBankBagSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->bank_bag_slot[slot], guid); }

uint64_t Player::getVendorBuybackSlot(uint8_t slot) const { return playerData()->vendor_buy_back_slot[slot]; }
void Player::setVendorBuybackSlot(uint8_t slot, uint64_t guid) { write(playerData()->vendor_buy_back_slot[slot], guid); }

#if VERSION_STRING < Cata
uint64_t Player::getKeyRingSlotItemGuid(uint8_t slot) const { return playerData()->key_ring_slot[slot]; }
void Player::setKeyRingSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->key_ring_slot[slot], guid); }
#endif

#if VERSION_STRING == TBC
uint64_t Player::getVanityPetSlotItemGuid(uint8_t slot) const { return playerData()->vanity_pet_slot[slot]; }
void Player::setVanityPetSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->vanity_pet_slot[slot], guid); }
#endif

#if VERSION_STRING == WotLK
uint64_t Player::getCurrencyTokenSlotItemGuid(uint8_t slot) const { return playerData()->currencytoken_slot[slot]; }
void Player::setCurrencyTokenSlotItemGuid(uint8_t slot, uint64_t guid) { write(playerData()->currencytoken_slot[slot], guid); }
#endif

uint64_t Player::getFarsightGuid() const { return playerData()->farsight_guid; }
void Player::setFarsightGuid(uint64_t farsightGuid) { write(playerData()->farsight_guid, farsightGuid); }

#if VERSION_STRING > Classic
uint64_t Player::getKnownTitles(uint8_t index) const { return playerData()->field_known_titles[index]; }
void Player::setKnownTitles(uint8_t index, uint64_t title) { write(playerData()->field_known_titles[index], title); }
#endif

#if VERSION_STRING > Classic
uint32_t Player::getChosenTitle() const { return playerData()->chosen_title; }
void Player::setChosenTitle(uint32_t title) { write(playerData()->chosen_title, title); }
#endif

#if VERSION_STRING == WotLK
uint64_t Player::getKnownCurrencies() const { return playerData()->field_known_currencies; }
void Player::setKnownCurrencies(uint64_t currencies) { write(playerData()->field_known_currencies, currencies); }
#endif

uint32_t Player::getXp() const { return playerData()->xp; }
void Player::setXp(uint32_t xp) { write(playerData()->xp, xp); }
void Player::addXP(uint32_t xp) { write(playerData()->xp, getXp() + xp); }

uint32_t Player::getNextLevelXp() const { return playerData()->next_level_xp; }
void Player::setNextLevelXp(uint32_t xp) { write(playerData()->next_level_xp, xp); }

#if VERSION_STRING < Cata
uint16_t Player::getSkillInfoId(uint32_t index) const { return playerData()->skill_info[index].id; }
uint16_t Player::getSkillInfoStep(uint32_t index) const { return playerData()->skill_info[index].step; }
uint16_t Player::getSkillInfoCurrentValue(uint32_t index) const { return playerData()->skill_info[index].current_value; }
uint16_t Player::getSkillInfoMaxValue(uint32_t index) const { return playerData()->skill_info[index].max_value; }
uint16_t Player::getSkillInfoBonusTemporary(uint32_t index) const { return playerData()->skill_info[index].bonus_temporary; }
uint16_t Player::getSkillInfoBonusPermanent(uint32_t index) const { return playerData()->skill_info[index].bonus_permanent; }
void Player::setSkillInfoId(uint32_t index, uint16_t id) { write(playerData()->skill_info[index].id, id); }
void Player::setSkillInfoStep(uint32_t index, uint16_t step) { write(playerData()->skill_info[index].step, step); }
void Player::setSkillInfoCurrentValue(uint32_t index, uint16_t current) { write(playerData()->skill_info[index].current_value, current); }
void Player::setSkillInfoMaxValue(uint32_t index, uint16_t max) { write(playerData()->skill_info[index].max_value, max); }
void Player::setSkillInfoBonusTemporary(uint32_t index, uint16_t bonus) { write(playerData()->skill_info[index].bonus_temporary, bonus); }
void Player::setSkillInfoBonusPermanent(uint32_t index, uint16_t bonus) { write(playerData()->skill_info[index].bonus_permanent, bonus); }
#else
uint16_t Player::getSkillInfoId(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_line[index]) + offset); }
uint16_t Player::getSkillInfoStep(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_step[index]) + offset); }
uint16_t Player::getSkillInfoCurrentValue(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_rank[index]) + offset); }
uint16_t Player::getSkillInfoMaxValue(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_max_rank[index]) + offset); }
uint16_t Player::getSkillInfoBonusTemporary(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_mod[index]) + offset); }
uint16_t Player::getSkillInfoBonusPermanent(uint32_t index, uint8_t offset) const { return *(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_talent[index]) + offset); }
uint32_t Player::getProfessionSkillLine(uint32_t index) const { return playerData()->profession_skill_line[index]; }
void Player::setSkillInfoId(uint32_t index, uint8_t offset, uint16_t id) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_line[index]) + offset), id); }
void Player::setSkillInfoStep(uint32_t index, uint8_t offset, uint16_t step) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_step[index]) + offset), step); }
void Player::setSkillInfoCurrentValue(uint32_t index, uint8_t offset, uint16_t current) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_rank[index]) + offset), current); }
void Player::setSkillInfoMaxValue(uint32_t index, uint8_t offset, uint16_t max) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_max_rank[index]) + offset), max); }
void Player::setSkillInfoBonusTemporary(uint32_t index, uint8_t offset, uint16_t bonus) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_mod[index]) + offset), bonus); }
void Player::setSkillInfoBonusPermanent(uint32_t index, uint8_t offset, uint16_t bonus) { write(*(((uint16_t*)&playerData()->field_skill_info.skill_info_parts.skill_talent[index]) + offset), bonus); }
void Player::setProfessionSkillLine(uint32_t index, uint32_t value) { write(playerData()->profession_skill_line[index], value); }
#endif

uint32_t Player::getFreeTalentPoints() const
{
#if VERSION_STRING < Cata
    return playerData()->character_points_1;
#else
    return m_specs[m_talentActiveSpec].getTalentPoints();
#endif
}

#if VERSION_STRING < Cata
void Player::setFreeTalentPoints(uint32_t points) { write(playerData()->character_points_1, points); }
#endif

uint32_t Player::getFreePrimaryProfessionPoints() const
{
#if VERSION_STRING < Cata
    return playerData()->character_points_2;
#else
    return playerData()->character_points_1;
#endif
}

void Player::setFreePrimaryProfessionPoints(uint32_t points)
{
    if (points > worldConfig.player.maxProfessions)
        points = worldConfig.player.maxProfessions;

#if VERSION_STRING < Cata
    write(playerData()->character_points_2, points);
#else
    write(playerData()->character_points_1, points);
#endif
}

void Player::modFreePrimaryProfessionPoints(int32_t amount)
{
    int32_t value = getFreePrimaryProfessionPoints();
    value += amount;

    if (value < 0)
        value = 0;

    setFreePrimaryProfessionPoints(value);
}

uint32_t Player::getTrackCreature() const { return playerData()->track_creatures; }
void Player::setTrackCreature(uint32_t id) { write(playerData()->track_creatures, id); }

uint32_t Player::getTrackResource() const { return playerData()->track_resources; }
void Player::setTrackResource(uint32_t id) { write(playerData()->track_resources, id); }

float Player::getBlockPercentage() const { return playerData()->block_pct; }
void Player::setBlockPercentage(float value) { write(playerData()->block_pct, value); }

float Player::getDodgePercentage() const { return playerData()->dodge_pct; }
void Player::setDodgePercentage(float value) { write(playerData()->dodge_pct, value); }

float Player::getParryPercentage() const { return playerData()->parry_pct; }
void Player::setParryPercentage(float value) { write(playerData()->parry_pct, value); }

#if VERSION_STRING >= TBC
uint32_t Player::getExpertise() const { return playerData()->expertise; }
void Player::setExpertise(uint32_t value) { write(playerData()->expertise, value); }
void Player::modExpertise(int32_t value) { setExpertise(getExpertise() + value); }

uint32_t Player::getOffHandExpertise() const { return playerData()->offhand_expertise; }
void Player::setOffHandExpertise(uint32_t value) { write(playerData()->offhand_expertise, value); }
void Player::modOffHandExpertise(int32_t value) { setOffHandExpertise(getOffHandExpertise() + value); }
#endif

float Player::getMeleeCritPercentage() const { return playerData()->crit_pct; }
void Player::setMeleeCritPercentage(float value) { write(playerData()->crit_pct, value); }

float Player::getRangedCritPercentage() const { return playerData()->ranged_crit_pct; }
void Player::setRangedCritPercentage(float value) { write(playerData()->ranged_crit_pct, value); }

#if VERSION_STRING >= TBC
float Player::getOffHandCritPercentage() const { return playerData()->offhand_crit_pct; }
void Player::setOffHandCritPercentage(float value) { write(playerData()->offhand_crit_pct, value); }

float Player::getSpellCritPercentage(uint8_t school) const { return playerData()->spell_crit_pct[school]; }
void Player::setSpellCritPercentage(uint8_t school, float value) { write(playerData()->spell_crit_pct[school], value); }

uint32_t Player::getShieldBlock() const { return playerData()->shield_block; }
void Player::setShieldBlock(uint32_t value) { write(playerData()->shield_block, value); }
#endif

#if VERSION_STRING >= WotLK
float Player::getShieldBlockCritPercentage() const { return playerData()->shield_block_crit_pct; }
void Player::setShieldBlockCritPercentage(float value) { write(playerData()->shield_block_crit_pct, value); }
#endif

uint32_t Player::getExploredZone(uint32_t idx) const
{
    if (idx < WOWPLAYER_EXPLORED_ZONES_COUNT)
        return playerData()->explored_zones[idx];
    return 0;
}

void Player::setExploredZone(uint32_t idx, uint32_t data)
{
    if (idx < WOWPLAYER_EXPLORED_ZONES_COUNT)
        write(playerData()->explored_zones[idx], data);
}

uint32_t Player::getSelfResurrectSpell() const { return playerData()->self_resurrection_spell; }
void Player::setSelfResurrectSpell(uint32_t spell) { write(playerData()->self_resurrection_spell, spell); }

uint32_t Player::getWatchedFaction() const { return playerData()->field_watched_faction_idx; }
void Player::setWatchedFaction(uint32_t factionId) { write(playerData()->field_watched_faction_idx, factionId); }

#if VERSION_STRING == TBC
float Player::getManaRegeneration() const { return playerData()->field_mod_mana_regen; }
void Player::setManaRegeneration(float value) { write(playerData()->field_mod_mana_regen, value); }

float Player::getManaRegenerationWhileCasting() const { return playerData()->field_mod_mana_regen_interrupt; }
void Player::setManaRegenerationWhileCasting(float value) { write(playerData()->field_mod_mana_regen_interrupt, value); }
#endif

uint32_t Player::getMaxLevel() const
{
#if VERSION_STRING > Classic
    return playerData()->field_max_level;
#else
    return m_classicMaxLevel;
#endif
}

void Player::setMaxLevel(uint32_t level)
{
#if VERSION_STRING > Classic
    write(playerData()->field_max_level, level);
#else
    m_classicMaxLevel = level;
#endif 
}

#if VERSION_STRING >= WotLK
float Player::getRuneRegen(uint8_t rune) const { return playerData()->rune_regen[rune]; }
void Player::setRuneRegen(uint8_t rune, float regen) { write(playerData()->rune_regen[rune], regen); }
#endif

uint32_t Player::getRestStateXp() const { return playerData()->rest_state_xp; }
void Player::setRestStateXp(uint32_t xp)  { write(playerData()->rest_state_xp, xp); }

#if VERSION_STRING < Cata
uint32_t Player::getCoinage() const { return playerData()->field_coinage; }
void Player::setCoinage(uint32_t coinage) { write(playerData()->field_coinage, coinage); }
bool Player::hasEnoughCoinage(uint32_t coinage) const { return getCoinage() >= coinage; }
void Player::modCoinage(int32_t coinage)
{
    setCoinage(getCoinage() + coinage);
}
#else
uint64_t Player::getCoinage() const { return playerData()->field_coinage; }
void Player::setCoinage(uint64_t coinage) { write(playerData()->field_coinage, coinage); }
bool Player::hasEnoughCoinage(uint64_t coinage) const { return getCoinage() >= coinage; }
void Player::modCoinage(int64_t coinage)
{
    setCoinage(getCoinage() + coinage);
}
#endif

#if VERSION_STRING == Classic
uint32_t Player::getResistanceBuffModPositive(uint8_t type) const { return playerData()->resistance_buff_mod_positive[type]; }
void Player::setResistanceBuffModPositive(uint8_t type, uint32_t value) { write(playerData()->resistance_buff_mod_positive[type], value); }

uint32_t Player::getResistanceBuffModNegative(uint8_t type) const { return playerData()->resistance_buff_mod_negative[type]; }
void Player::setResistanceBuffModNegative(uint8_t type, uint32_t value) { write(playerData()->resistance_buff_mod_negative[type], value); }
#endif

uint32_t Player::getModDamageDonePositive(uint16_t school) const { return playerData()->field_mod_damage_done_positive[school]; }
void Player::setModDamageDonePositive(uint16_t school, uint32_t value) { write(playerData()->field_mod_damage_done_positive[school], value); }
void Player::modModDamageDonePositive(uint16_t school, int32_t value) { setModDamageDonePositive(school, getModDamageDonePositive(school) + value); }

uint32_t Player::getModDamageDoneNegative(uint16_t school) const { return playerData()->field_mod_damage_done_negative[school]; }
void Player::setModDamageDoneNegative(uint16_t school, uint32_t value) { write(playerData()->field_mod_damage_done_negative[school], value); }
void Player::modModDamageDoneNegative(uint16_t school, int32_t value) { setModDamageDoneNegative(school, getModDamageDoneNegative(school) + value); }

float Player::getModDamageDonePct(uint8_t shool) const { return playerData()->field_mod_damage_done_pct[shool]; }
void Player::setModDamageDonePct(float damagePct, uint8_t shool) { write(playerData()->field_mod_damage_done_pct[shool], damagePct); }

#if VERSION_STRING >= TBC
uint32_t Player::getModHealingDone() const { return playerData()->field_mod_healing_done; }
void Player::setModHealingDone(uint32_t value) { write(playerData()->field_mod_healing_done, value); }
void Player::modModHealingDone(int32_t value) { setModHealingDone(getModHealingDone() + value); }

uint32_t Player::getModTargetResistance() const { return playerData()->field_mod_target_resistance; }
void Player::setModTargetResistance(uint32_t value) { write(playerData()->field_mod_target_resistance, value); }
void Player::modModTargetResistance(int32_t value) { setModTargetResistance(getModTargetResistance() + value); }

uint32_t Player::getModTargetPhysicalResistance() const { return playerData()->field_mod_target_physical_resistance; }
void Player::setModTargetPhysicalResistance(uint32_t value) { write(playerData()->field_mod_target_physical_resistance, value); }
void Player::modModTargetPhysicalResistance(int32_t value) { setModTargetPhysicalResistance(getModTargetPhysicalResistance() + value); }
#endif

uint32_t Player::getPlayerFieldBytes() const { return playerData()->player_field_bytes.raw; }
void Player::setPlayerFieldBytes(uint32_t bytes) { write(playerData()->player_field_bytes.raw, bytes); }

uint8_t Player::getPlayerFieldBytesMiscFlag() const { return playerData()->player_field_bytes.s.misc_flags; }
void Player::setPlayerFieldBytesMiscFlag(uint8_t miscFlag) { write(playerData()->player_field_bytes.s.misc_flags, miscFlag); }
void Player::addPlayerFieldBytesMiscFlag(uint8_t miscFlag) { setPlayerFieldBytesMiscFlag(getPlayerFieldBytesMiscFlag() | miscFlag); }
void Player::removePlayerFieldBytesMiscFlag(uint8_t miscFlag) { setPlayerFieldBytesMiscFlag(getPlayerFieldBytesMiscFlag() & ~miscFlag); }

uint8_t Player::getEnabledActionBars() const { return playerData()->player_field_bytes.s.enabled_action_bars; }
void Player::setEnabledActionBars(uint8_t actionBarId) { write(playerData()->player_field_bytes.s.enabled_action_bars, actionBarId); }

#if VERSION_STRING < Cata
uint32_t Player::getAmmoId() const { return playerData()->ammo_id; }
void Player::setAmmoId(uint32_t id) { write(playerData()->ammo_id, id); }
#endif

uint32_t Player::getBuybackPriceSlot(uint8_t slot) const { return playerData()->field_buy_back_price[slot]; }
void Player::setBuybackPriceSlot(uint8_t slot, uint32_t price) { write(playerData()->field_buy_back_price[slot], price); }

uint32_t Player::getBuybackTimestampSlot(uint8_t slot) const { return playerData()->field_buy_back_timestamp[slot]; }
void Player::setBuybackTimestampSlot(uint8_t slot, uint32_t timestamp) { write(playerData()->field_buy_back_timestamp[slot], timestamp); }

#if VERSION_STRING > Classic
uint32_t Player::getFieldKills() const { return playerData()->field_kills.raw; }
void Player::setFieldKills(uint32_t kills) { write(playerData()->field_kills.raw, kills); }
#endif

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    uint32_t Player::getContributionToday() const { return playerData()->field_contribution_today; }
    void Player::setContributionToday(uint32_t contribution) { write(playerData()->field_contribution_today, contribution); }

    uint32_t Player::getContributionYesterday() const { return playerData()->field_contribution_yesterday; }
    void Player::setContributionYesterday(uint32_t contribution) { write(playerData()->field_contribution_yesterday, contribution); }
#endif
#endif

uint32_t Player::getLifetimeHonorableKills() const { return playerData()->field_lifetime_honorable_kills; }
void Player::setLifetimeHonorableKills(uint32_t kills) { write(playerData()->field_lifetime_honorable_kills, kills); }

#if VERSION_STRING != Mop
uint32_t Player::getPlayerFieldBytes2() const { return playerData()->player_field_bytes_2.raw; }
void Player::setPlayerFieldBytes2(uint32_t bytes) { write(playerData()->player_field_bytes_2.raw, bytes); }

uint8_t Player::getAuraVision() const { return playerData()->player_field_bytes_2.s.aura_vision; }
void Player::setAuraVision(uint8_t auraVision) { write(playerData()->player_field_bytes_2.s.aura_vision, auraVision); }
void Player::addAuraVision(uint8_t auraVision) { setAuraVision(getAuraVision() | auraVision); }
void Player::removeAuraVision(uint8_t auraVision) { setAuraVision(getAuraVision() & ~auraVision); }
#endif

uint32_t Player::getCombatRating(uint8_t combatRating) const { return playerData()->field_combat_rating[combatRating]; }
void Player::setCombatRating(uint8_t combatRating, uint32_t value) { write(playerData()->field_combat_rating[combatRating], value); }
void Player::modCombatRating(uint8_t combatRating, int32_t value) { setCombatRating(combatRating, getCombatRating(combatRating) + value); }

#if VERSION_STRING > Classic
    // field_arena_team_info start
uint32_t Player::getArenaTeamId(uint8_t teamSlot) const { return playerData()->field_arena_team_info[teamSlot].team_id; }
void Player::setArenaTeamId(uint8_t teamSlot, uint32_t teamId) { write(playerData()->field_arena_team_info[teamSlot].team_id, teamId); }

uint32_t Player::getArenaTeamMemberRank(uint8_t teamSlot) const { return playerData()->field_arena_team_info[teamSlot].member_rank; }
void Player::setArenaTeamMemberRank(uint8_t teamSlot, uint32_t rank) { write(playerData()->field_arena_team_info[teamSlot].member_rank, rank); }
    // field_arena_team_info end
#endif

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
uint32_t Player::getHonorCurrency() const { return playerData()->field_honor_currency; }
void Player::setHonorCurrency(uint32_t amount) { write(playerData()->field_honor_currency, amount); }
void Player::modHonorCurrency(int32_t value) { setArenaCurrency(getArenaCurrency() + value); }

uint32_t Player::getArenaCurrency() const { return playerData()->field_arena_currency; }
void Player::setArenaCurrency(uint32_t amount) { write(playerData()->field_arena_currency, amount); }
void Player::modArenaCurrency(int32_t value) { setArenaCurrency(getArenaCurrency() + value); }
#endif
#endif

#if VERSION_STRING >= WotLK
uint32_t Player::getNoReagentCost(uint8_t index) const { return playerData()->no_reagent_cost[index]; }
void Player::setNoReagentCost(uint8_t index, uint32_t value) { write(playerData()->no_reagent_cost[index], value); }

uint32_t Player::getGlyphSlot(uint16_t slot) const { return playerData()->field_glyph_slots[slot]; }
void Player::setGlyphSlot(uint16_t slot, uint32_t glyph) { write(playerData()->field_glyph_slots[slot], glyph); }

uint32_t Player::getGlyph(uint16_t slot) const { return playerData()->field_glyphs[slot]; }
void Player::setGlyph(uint16_t slot, uint32_t glyph) { write(playerData()->field_glyphs[slot], glyph); }

uint32_t Player::getGlyphsEnabled() const { return playerData()->glyphs_enabled; }
void Player::setGlyphsEnabled(uint32_t glyphs) { write(playerData()->glyphs_enabled, glyphs); }
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING >= Cata
void Player::sendForceMovePacket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(60);
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_TURN_RATE_CHANGE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_PITCH_RATE_CHANGE, speed);
            break;
        }
    }

    sendMessageToSet(&data, true);
}

void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data;
    ObjectGuid guid = getGuid();

    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(MSG_MOVE_SET_WALK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_WALK_SPEED, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED, 1 + 8 + 4 + 4);
#if VERSION_STRING == Mop
            data.writeBit(guid[1]);
            data.writeBit(guid[7]);
            data.writeBit(guid[4]);
            data.writeBit(guid[2]);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(guid[6]);
            data.writeBit(guid[0]);

            data.WriteByteSeq(guid[1]);

            data << uint32_t(0);

            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);

            data << float(speed);

            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
#else
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_SPEED, speed);
#endif
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_BACK_SPEED, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_SPEED, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_BACK_SPEED, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_TURN_RATE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 1 + 8 + 4 + 4);
#if VERSION_STRING == Mop
            data << float(speed);
            data << uint32_t(0);

            data.writeBit(guid[6]);
            data.writeBit(guid[5]);
            data.writeBit(guid[0]);
            data.writeBit(guid[4]);
            data.writeBit(guid[1]);
            data.writeBit(guid[7]);
            data.writeBit(guid[3]);
            data.writeBit(guid[2]);

            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
#else
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_SPEED, speed);
#endif
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_BACK_SPEED, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_PITCH_RATE, speed);
            break;
        }
    }

    sendMessageToSet(&data, true);
}
#endif

void Player::resendSpeed()
{
    if (m_resendSpeed)
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true), true);
        setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, true), true);
        m_resendSpeed = false;
    }
}
bool Player::isMoving() const { return m_isMoving; }

uint32_t Player::getMountSpellId() const { return m_mountSpellId; }
void Player::setMountSpellId(uint32_t id) { m_mountSpellId = id; }

bool Player::isOnVehicle() const { return m_mountVehicleId ? true : false; }
uint32_t Player::getMountVehicleId() const { return m_mountVehicleId; }
void Player::setMountVehicleId(uint32_t id) { m_mountVehicleId = id; }

void Player::handleAuraInterruptForMovementFlags(MovementInfo const& movementInfo)
{
    uint32_t auraInterruptFlags = 0;
    if (movementInfo.hasMovementFlag(MOVEFLAG_MOTION_MASK))
        auraInterruptFlags |= AURA_INTERRUPT_ON_MOVEMENT;

    if (!(movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING)) || movementInfo.hasMovementFlag(MOVEFLAG_FALLING))
        auraInterruptFlags |= AURA_INTERRUPT_ON_LEAVE_WATER;

    if (movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING))
        auraInterruptFlags |= AURA_INTERRUPT_ON_ENTER_WATER;

    if ((movementInfo.hasMovementFlag(MOVEFLAG_TURNING_MASK)) || m_isTurning)
        auraInterruptFlags |= AURA_INTERRUPT_ON_TURNING;

    removeAllAurasByAuraInterruptFlag(auraInterruptFlags);
}

bool Player::isInCity() const
{
    if (const auto at = GetArea())
    {
        ::WDB::Structures::AreaTableEntry const* zt = nullptr;
        if (at->zone)
            zt = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);

        bool areaIsCity = at->flags & MapManagement::AreaManagement::AREA_CITY_AREA || at->flags & MapManagement::AreaManagement::AREA_CITY;
        bool zoneIsCity = zt && (zt->flags & MapManagement::AreaManagement::AREA_CITY_AREA || zt->flags & MapManagement::AreaManagement::AREA_CITY);

        return (areaIsCity || zoneIsCity);
    }

    return false;
}

void Player::handleBreathing(MovementInfo const& movementInfo, WorldSession* session)
{
    if (!worldConfig.server.enableBreathing || m_cheats.hasFlyCheat || m_isWaterBreathingEnabled || !isAlive() || m_cheats.hasGodModeCheat)
    {
        if (m_underwaterState & UNDERWATERSTATE_SWIMMING)
            m_underwaterState &= ~UNDERWATERSTATE_SWIMMING;

        if (m_underwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            sendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, -1);
        }

        if (session->m_bIsWLevelSet)
        {
            if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
            {
                removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);
                session->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    if (movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && !(m_underwaterState & UNDERWATERSTATE_SWIMMING))
    {
        removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        if (!session->m_bIsWLevelSet)
        {
            session->m_wLevel = movementInfo.getPosition()->z + m_noseLevel * 0.95f;
            session->m_bIsWLevelSet = true;
        }

        m_underwaterState |= UNDERWATERSTATE_SWIMMING;
    }

#if VERSION_STRING <= WotLK
    if (!movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && movementInfo.flags != MOVEFLAG_MOVE_STOP && m_underwaterState & UNDERWATERSTATE_SWIMMING)
#else
    if (!movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && movementInfo.flags != MOVEFLAG_NONE && m_underwaterState & UNDERWATERSTATE_SWIMMING)
#endif
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);
            session->m_bIsWLevelSet = false;

            m_underwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    if (m_underwaterState & UNDERWATERSTATE_SWIMMING && !(m_underwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        if (movementInfo.getPosition()->z + m_noseLevel < session->m_wLevel)
        {
            m_underwaterState |= UNDERWATERSTATE_UNDERWATER;
            sendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, -1);
        }
    }

    if (m_underwaterState & UNDERWATERSTATE_SWIMMING && m_underwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            sendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, 10);
        }
    }

    if (!(m_underwaterState & UNDERWATERSTATE_SWIMMING) && m_underwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            sendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, 10);
        }
    }
}

//\todo: find another solution for this
void Player::initialiseNoseLevel()
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

void Player::handleKnockback(Object* object, float horizontal, float vertical)
{
    if (object == nullptr)
        object = this;

    float angle = calcRadAngle(object->GetPositionX(), object->GetPositionY(), GetPositionX(), GetPositionY());
    if (object == this)
        angle = static_cast<float>(M_PI + GetOrientation());

    float sin = sinf(angle);
    float cos = cosf(angle);

    getSession()->SendPacket(SmsgMoveKnockBack(GetNewGUID(), Util::getMSTime(), cos, sin, horizontal, -vertical).serialise().get());

    m_blinked = true;
    speedCheatDelay(10000);
}

bool Player::teleport(const LocationVector& vec, WorldMap* map)
{
    if (map)
    {
        if (map->getPlayer(this->getGuidLow()))
        {
            this->SetPosition(vec);
        }
        else
        {
            if (map->getBaseMap()->getMapId() == 530 && !this->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_01))
                return false;

            if (map->getBaseMap()->getMapId() == 571 && !this->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
                return false;

            this->safeTeleport(map, vec);
        }

        return true;
    }

    return false;
}

void Player::eventTeleport(uint32_t mapId, LocationVector position, uint32_t instanceId)
{
    safeTeleport(mapId, instanceId, position);
}

bool Player::safeTeleport(uint32_t mapId, uint32_t instanceId, const LocationVector& vec)
{
    // do not teleport to an unallowed mapId
    if (const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId))
    {
        if (mapInfo->flags & WMI_INSTANCE_XPACK_01 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            sendChatMessage(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, getSession()->LocalizedWorldSrv(SS_MUST_HAVE_BC));
            return false;
        }

        if (mapInfo->flags & WMI_INSTANCE_XPACK_02 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            sendChatMessage(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, getSession()->LocalizedWorldSrv(SS_MUST_HAVE_WOTLK));
            return false;
        }
    }

    // hide waypoints otherwise it will crash when trying to unload map
    if (m_aiInterfaceWaypoint != nullptr)
        m_aiInterfaceWaypoint->hideWayPoints(this);

    m_aiInterfaceWaypoint = nullptr;

    speedCheatDelay(10000);

    if (m_taxi->getCurrentTaxiPath())
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_TELEPORT);
        setMountDisplayId(0);

        removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
        removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true), true);
    }

    if (obj_movement_info.transport_guid)
    {
        if (const auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_guid)))
        {
            transporter->RemovePassenger(this);
            obj_movement_info.transport_guid = 0;
        }
    }

    bool instance = false;
    if (instanceId && static_cast<uint32_t>(m_instanceId) != instanceId)
    {
        instance = true;
        this->SetInstanceID(instanceId);
    }
    else if (m_mapId != mapId)
    {
        instance = true;
    }

    // make sure player does not drown when teleporting from under water
    if (m_underwaterState & UNDERWATERSTATE_UNDERWATER)
        m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;

    // can only fly in outlands or northrend (northrend requires cold weather flying)
    if (m_flyingAura && ((m_mapId != 530) && (m_mapId != 571 || !hasSpell(54197) && getDeathState() == ALIVE)))
    {
        removeAllAurasById(m_flyingAura);
        m_flyingAura = 0;
    }

#ifdef FT_VEHICLES
    callExitVehicle();
#endif

    if (m_bg && m_bg->getWorldMap() && getWorldMap() && getWorldMap()->getBaseMap()->getMapInfo()->mapid != mapId)
    {
        m_bg->removePlayer(this, false);
    }

    _Relocate(mapId, vec, true, instance, instanceId);

    speedCheatReset();

    forceZoneUpdate();

    return true;
}

void Player::safeTeleport(WorldMap* mgr, const LocationVector& vec)
{
    if (mgr)
    {
        speedCheatDelay(10000);

        // can only fly in outlands or northrend (northrend requires cold weather flying)
        if (m_flyingAura && ((m_mapId != 530) && (m_mapId != 571 || !hasSpell(54197) && getDeathState() == ALIVE)))
        {
            removeAllAurasById(m_flyingAura);
            m_flyingAura = 0;
        }

        if (IsInWorld())
            removeFromWorld();

        m_mapId = mgr->getBaseMap()->getMapId();
        m_instanceId = mgr->getInstanceId();

        getSession()->SendPacket(SmsgTransferPending(mgr->getBaseMap()->getMapId()).serialise().get());
        getSession()->SendPacket(SmsgNewWorld(mgr->getBaseMap()->getMapId(), vec).serialise().get());

        setTransferStatus(TRANSFER_PENDING);
        m_sentTeleportPosition = vec;
        SetPosition(vec);

        speedCheatReset();
        forceZoneUpdate();
    }
}

void Player::setTransferStatus(uint8_t status) { m_transferStatus = status; }
uint8_t Player::getTransferStatus() const { return m_transferStatus; }
bool Player::isTransferPending() const { return getTransferStatus() == TRANSFER_PENDING; }

uint32_t Player::getTeleportState() const { return m_teleportState; }

void Player::sendTeleportPacket(LocationVector position)
{
#if VERSION_STRING < Cata
    WorldPacket data2(MSG_MOVE_TELEPORT, 38);
    data2.append(GetNewGUID());
    buildMovementPacket(&data2, position.x, position.y, position.z, position.o);
    sendMessageToSet(&data2, false);
    SetPosition(position);
#else
    LocationVector oldPos = LocationVector(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    LocationVector pos = position;

    if (getObjectTypeId() == TYPEID_UNIT)
        SetPosition(pos);

    ObjectGuid guid = getGuid();

    WorldPacket data(SMSG_MOVE_UPDATE_TELEPORT, 38);
    obj_movement_info.writeMovementInfo(data, SMSG_MOVE_UPDATE_TELEPORT);

    if (getObjectTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data2(MSG_MOVE_TELEPORT, 38);
        data2.writeBit(guid[6]);
        data2.writeBit(guid[0]);
        data2.writeBit(guid[3]);
        data2.writeBit(guid[2]);
        data2.writeBit(0); // unk
        //\TODO add transport
        data2.writeBit(uint64_t(0)); // transport guid
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
        sendPacket(&data2);
    }

    if (getObjectTypeId() == TYPEID_PLAYER)
        SetPosition(pos);
    else
        SetPosition(oldPos);

    sendMessageToSet(&data, false);
#endif
}

void Player::sendTeleportAckPacket(LocationVector position)
{
    setTransferStatus(TRANSFER_PENDING);

#if VERSION_STRING < TBC
    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetNewGUID();
    data << uint32_t(2);
    data << uint32_t(0);
    data << uint8_t(0);

    data << float(0);
    data << position.x;
    data << position.y;
    data << position.z;
    data << position.o;
    data << uint16_t(2);
    data << uint8_t(0);
#else
    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetNewGUID();
    data << uint32_t(0);
    buildMovementPacket(&data, position.x, position.y, position.z, position.o);
#endif
    getSession()->SendPacket(&data);

#if VERSION_STRING == TBC
    sendTeleportPacket(position);
#endif

}

void Player::onWorldPortAck()
{
    WDB::Structures::MapEntry const* mEntry = sMapStore.lookupEntry(GetMapId());
    //only resurrect if player is porting to a instance portal
    if (mEntry->isInstanceMap() && isDead())
        resurrect();

    if (mEntry->isInstanceMap())
    {
        // check if this instance has a reset time and send it to player if so
        InstanceDifficulty::Difficulties diff = getDifficulty(mEntry->isRaid());
        if (WDB::Structures::MapDifficulty const* mapDiff = getMapDifficultyData(mEntry->id, diff))
        {
            if (mapDiff->resetTime)
            {
                if (time_t timeReset = sInstanceMgr.getResetTimeFor(static_cast<uint16_t>(mEntry->id), diff))
                {
                    const auto now = Util::getTimeNow();

                    uint32_t timeleft = static_cast<uint32_t>(timeReset - now);
                    sendInstanceResetWarning(mEntry->id, diff, timeleft, true);
                }
            }
        }
    }

    speedCheatReset();
}

void Player::eventPortToGm(Player* gmPlayer)
{
    safeTeleport(gmPlayer->GetMapId(), gmPlayer->GetInstanceID(), gmPlayer->GetPosition());
}

void Player::indoorCheckUpdate(uint32_t time)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (time >= m_indoorCheckTimer)
        {
            if (!isOutdoors())
            {
                // this is duplicated check, but some mount auras comes w/o this flag set, maybe due to spellfixes.cpp line:663
                if (isMounted() && !m_taxi->getCurrentTaxiPath())
                    dismount();

                for (uint16_t x = AuraSlots::POSITIVE_SLOT_START; x < AuraSlots::POSITIVE_SLOT_END; ++x)
                {
                    auto* const aur = getAuraWithAuraSlot(x);
                    if (aur && aur->getSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS)
                        aur->removeAura();
                }
            }
            m_indoorCheckTimer = time + COLLISION_INDOOR_CHECK_INTERVAL;
        }
    }
}

time_t Player::getFallDisabledUntil() const { return m_fallDisabledUntil; }
void Player::setFallDisabledUntil(time_t time) { m_fallDisabledUntil = time; }

void Player::setMapEntryPoint(uint32_t mapId)
{
    if (IS_INSTANCE(GetMapId()))
        return;

    if (MySQLStructure::MapInfo const* mapInfo = sMySQLStore.getWorldMapInfo(mapId))
        setBGEntryPoint(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz, GetOrientation(), mapInfo->repopmapid, GetInstanceID());
    else
        setBGEntryPoint(0, 0, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance, Zone, Area, Phase
void Player::setPhase(uint8_t command, uint32_t newPhase)
{
    Unit::setPhase(command, newPhase);

    if (getSession())
    {
#if VERSION_STRING == WotLK
        sendPacket(SmsgPhaseShiftChange(newPhase, getGuid()).serialise().get());
#elif VERSION_STRING > WotLK

        uint32_t phaseFlags = 0;

        for (uint32_t i = 0; i < sPhaseStore.getNumRows(); ++i)
        {
            if (WDB::Structures::PhaseEntry const* phase = sPhaseStore.lookupEntry(i))
            {
                if (phase->PhaseShift == newPhase)
                {
                    phaseFlags = phase->Flags;
                    break;
                }
            }
        }

        sendPacket(SmsgPhaseShiftChange(newPhase, getGuid(), phaseFlags, GetMapId()).serialise().get());
#endif
    }

    getSummonInterface()->setPhase(command, newPhase);

    if (Unit* charm = m_WorldMap->getUnit(getCharmGuid()))
        charm->setPhase(command, newPhase);
}

void Player::zoneUpdate(uint32_t zoneId)
{
    uint32_t oldzone = m_zoneId;
    if (m_zoneId != zoneId)
    {
        setZoneId(zoneId);
        removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_LEAVE_AREA);
    }

    if (m_playerInfo)
    {
        m_playerInfo->lastZone = zoneId;
        sHookInterface.OnZone(this, zoneId, oldzone);

        if (m_WorldMap && m_WorldMap->getScript())
            m_WorldMap->getScript()->OnZoneChange(this, zoneId, oldzone);

        auto at = GetArea();
        if (at && (at->team == AREAC_SANCTUARY || at->flags & AREA_SANCTUARY))
        {
            Unit* pUnit = (getTargetGuid() == 0) ? nullptr : (m_WorldMap ? m_WorldMap->getUnit(getTargetGuid()) : nullptr);
            if (pUnit && m_duelPlayer != pUnit)
            {
                eventAttackStop();
                smsg_AttackStop(pUnit);
            }

            if (isCastingSpell())
            {
                for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
                {
                    if (getCurrentSpell(CurrentSpellType(i)) != nullptr)
                    {
                        Unit* target = getCurrentSpell(CurrentSpellType(i))->getUnitTarget();
                        if (target != nullptr && target != m_duelPlayer && target != this)
                        {
                            interruptSpellWithSpellType(CurrentSpellType(i));
                        }
                    }
                }
            }
        }

        sendInitialWorldstates();

        updateChannels();
    }
    else
    {
        sLogger.failure("Player with invalid player_info tries to call Player::zoneUpdate()!");
        m_session->Disconnect();
    }
}

void Player::forceZoneUpdate()
{
    if (!m_WorldMap)
        return;

    if (auto areaTableEntry = this->GetArea())
    {
        if (areaTableEntry->zone && areaTableEntry->zone != m_zoneId)
            zoneUpdate(areaTableEntry->zone);

        sendInitialWorldstates();
    }
}

bool Player::hasAreaExplored(::WDB::Structures::AreaTableEntry const* areaTableEntry)
{
    if (areaTableEntry)
    {
        uint16_t offset = static_cast<uint16_t>(areaTableEntry->explore_flag / 32);

        uint32_t val = (uint32_t)(1 << (areaTableEntry->explore_flag % 32));
        uint32_t currFields = getExploredZone(offset);

        return (currFields & val) != 0;
    }

    return false;
}

bool Player::hasOverlayUncovered(uint32_t overlayId)
{
    if (auto overlay = sWorldMapOverlayStore.lookupEntry(overlayId))
    {
        if (overlay->areaID && hasAreaExplored(AreaStorage::GetAreaById(overlay->areaID)))
            return true;

        if (overlay->areaID_2 && hasAreaExplored(AreaStorage::GetAreaById(overlay->areaID_2)))
            return true;

        if (overlay->areaID_3 && hasAreaExplored(AreaStorage::GetAreaById(overlay->areaID_3)))
            return true;

        if (overlay->areaID_4 && hasAreaExplored(AreaStorage::GetAreaById(overlay->areaID_4)))
            return true;
    }

    return false;
}

void Player::eventExploration()
{
    if (isDead())
        return;

    if (!IsInWorld())
        return;

    if (m_position.x > Map::Terrain::_maxX || m_position.x < Map::Terrain::_minX || m_position.y > Map::Terrain::_maxY || m_position.y < Map::Terrain::_minY)
        return;

    if (getWorldMap()->getCellByCoords(GetPositionX(), GetPositionY()) == nullptr)
        return;

    if (auto areaTableEntry = this->GetArea())
    {
        uint16_t offset = static_cast<uint16_t>(areaTableEntry->explore_flag / 32);
        uint32_t val = (uint32_t)(1 << (areaTableEntry->explore_flag % 32));
        uint32_t currFields = getExploredZone(offset);

        if (areaTableEntry->id != m_areaId)
        {
            m_areaId = areaTableEntry->id;

            updatePvPArea();
            addGroupUpdateFlag(GROUP_UPDATE_FULL);

            if (getGroup())
                getGroup()->UpdateOutOfRangePlayer(this, true, nullptr);
        }

        if (areaTableEntry->zone == 0 && m_zoneId != areaTableEntry->id)
            zoneUpdate(areaTableEntry->id);
        else if (areaTableEntry->zone != 0 && m_zoneId != areaTableEntry->zone)
            zoneUpdate(areaTableEntry->zone);


        if (areaTableEntry->zone != 0 && m_zoneId != areaTableEntry->zone)
            zoneUpdate(areaTableEntry->zone);

        bool rest_on = false;

        if (areaTableEntry->flags & AREA_CITY_AREA || areaTableEntry->flags & AREA_CITY)
        {
            // check faction
            if (areaTableEntry->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance() || (areaTableEntry->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
                rest_on = true;
            else if (areaTableEntry->team != AREAC_ALLIANCE_TERRITORY && areaTableEntry->team != AREAC_HORDE_TERRITORY)
                rest_on = true;
        }
        else
        {
            //second AT check for subzones.
            if (areaTableEntry->zone)
            {
                auto at2 = AreaStorage::GetAreaById(areaTableEntry->zone);
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
                applyPlayerRestState(true);
        }
        else
        {
            if (m_isResting)
                applyPlayerRestState(false);
        }

        if (!(currFields & val) && !m_taxi->getCurrentTaxiPath() && !obj_movement_info.transport_guid)
        {
            setExploredZone(offset, currFields | val);

            uint32_t explore_xp = areaTableEntry->area_level * 10;
            explore_xp *= Util::float2int32(worldConfig.getFloatRate(RATE_EXPLOREXP));

#if VERSION_STRING > TBC
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA, getAreaId());
#endif

            if (getLevel() < getMaxLevel() && explore_xp > 0)
            {
                sendExploreExperiencePacket(areaTableEntry->id, explore_xp);
                giveXp(explore_xp, 0, false);
            }
            else
            {
                sendExploreExperiencePacket(areaTableEntry->id, 0);
            }
        }
    }
}

void Player::ejectFromInstance()
{
    if (getBGEntryPosition().isSet() && !IS_INSTANCE(getBGEntryMapId()))
        if (safeTeleport(getBGEntryMapId(), getBGEntryInstanceId(), getBGEntryPosition()))
            return;

    safeTeleport(getBindMapId(), 0, getBindPosition());
}

bool Player::exitInstance()
{
    if (getBGEntryPosition().isSet())
    {
        removeFromWorld();
        safeTeleport(getBGEntryMapId(), getBGEntryInstanceId(), getBGEntryPosition());

        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Commands
void Player::disableSummoning(bool disable) { m_disableSummoning = disable; }
bool Player::isSummoningDisabled() const { return m_disableSummoning; }
void Player::disableAppearing(bool disable) { m_disableAppearing = disable; }
bool Player::isAppearingDisabled() const { return m_disableAppearing; }

bool Player::isBanned() const
{
    if (m_banned)
    {
        if (m_banned < 100 || static_cast<uint32_t>(UNIXTIME) < m_banned)
            return true;
    }
    return false;
}

void Player::setBanned(uint32_t timestamp /*= 4*/, std::string Reason /*= ""*/) { m_banned = timestamp; m_banreason = Reason; }
void Player::unsetBanned() { m_banned = 0; }
std::string Player::getBanReason() const { return m_banreason; }

GameObject* Player::getSelectedGo() const
{
    if (m_GMSelectedGO)
        return getWorldMap()->getGameObject(static_cast<uint32_t>(m_GMSelectedGO));

    return nullptr;
}

void Player::setSelectedGo(uint64_t guid) { m_GMSelectedGO = guid; }

void Player::kickFromServer(uint32_t delay)
{
    if (delay)
    {
        m_kickDelay = delay;
        sEventMgr.AddEvent(this, &Player::eventKickFromServer, EVENT_PLAYER_KICK, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        m_kickDelay = 0;
        eventKickFromServer();
    }
}

void Player::eventKickFromServer()
{
    if (m_kickDelay)
    {
        if (m_kickDelay < 1500)
            m_kickDelay = 0;
        else
            m_kickDelay -= 1000;

        getSession()->systemMessage("You will be removed from the server in {} seconds.", m_kickDelay / 1000);
    }
    else
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_KICK);
        getSession()->LogoutPlayer(true);
    }
}

void Player::sendSummonRequest(uint32_t requesterId, uint32_t zoneId, uint32_t mapId, uint32_t instanceId, const LocationVector& position)
{
    m_summonData.instanceId = instanceId;
    m_summonData.position = position;
    m_summonData.summonerId = requesterId;
    m_summonData.mapId = mapId;

    m_session->SendPacket(SmsgSummonRequest(requesterId, zoneId, 120000).serialise().get());
}

void Player::setAFKReason(std::string reason) { afkReason = reason; }
std::string Player::getAFKReason() const { return afkReason; }

void Player::addToGMTargetList(uint32_t guid)
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    m_gmPlayerTargetList.push_back(guid);
}

void Player::removeFromGMTargetList(uint32_t guid)
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    m_gmPlayerTargetList.erase(std::remove(m_gmPlayerTargetList.begin(), m_gmPlayerTargetList.end(), guid), m_gmPlayerTargetList.end());
}

bool Player::isOnGMTargetList(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    if (std::find(m_gmPlayerTargetList.begin(), m_gmPlayerTargetList.end(), guid) != m_gmPlayerTargetList.end())
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Basic
bool Player::create(CharCreate& charCreateContent)
{
    m_name = charCreateContent.name;
    AscEmu::Util::Strings::capitalize(m_name);

    //\todo: Zyres: we already have a method to check if the race/class combination is valid (somewhere in the char list generation).
    m_playerCreateInfo = sMySQLStore.getPlayerCreateInfo(charCreateContent._race, charCreateContent._class);
    if (m_playerCreateInfo == nullptr)
    {
        m_session->Disconnect();
#if VERSION_STRING > TBC
        if (charCreateContent._class == DEATHKNIGHT)
            sLogger.failure("Account Name: {} tried to create a deathknight, however your playercreateinfo table does not support this class, please update your database.", m_session->GetAccountName());
        else
#endif
            sLogger.failure("Account Name: {} tried to create an invalid character with race {} and class {}, if this is intended please update your playercreateinfo table inside your database.", m_session->GetAccountName(), charCreateContent._race, charCreateContent._class);
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
        sLogger.failure("Account {} tried to create a DeathKnight, but Account flag is {}!", m_session->GetAccountName(), m_session->_accountFlags);
        m_session->Disconnect();
        return false;
    }
#endif

    m_mapId = m_playerCreateInfo->mapId;
    setZoneId(m_playerCreateInfo->zoneId);
    m_position.ChangeCoords({ m_playerCreateInfo->positionX, m_playerCreateInfo->positionY, m_playerCreateInfo->positionZ, m_playerCreateInfo->orientation });

    setBindPoint(m_playerCreateInfo->positionX, m_playerCreateInfo->positionY, m_playerCreateInfo->positionZ, m_playerCreateInfo->orientation, m_playerCreateInfo->mapId, m_playerCreateInfo->zoneId);
    m_isResting = 0;
    m_restAmount = 0;
    m_restState = 0;

    // set race dbc
    m_dbcRace = sChrRacesStore.lookupEntry(charCreateContent._race);
    m_dbcClass = sChrClassesStore.lookupEntry(charCreateContent._class);
    if (!m_dbcRace || !m_dbcClass)
    {
        // information not found
        sCheatLog.writefromsession(m_session, "tried to create invalid player with race %u and class %u, dbc m_playerCreateInfo not found", charCreateContent._race, charCreateContent._class);
        m_session->Disconnect();
        return false;
    }

    if (m_dbcRace->team_id == 7)
        m_team = 0;
    else
        m_team = 1;

    // Automatically add the race's taxi hub to the character's taximask at creation time (1 << (taxi_node_id-1))
    // this is defined in table playercreateinfo, field taximask
    //memcpy(m_taxiMask, m_playerCreateInfo->taximask, sizeof(m_taxiMask));

    if (auto playerClassLevelStats = sMySQLStore.getPlayerClassLevelStats(1, charCreateContent._class))
        setMaxHealth(playerClassLevelStats->health);
    else
        sLogger.failure("No class levelstats found!");

    if (const auto raceEntry = sChrRacesStore.lookupEntry(charCreateContent._race))
        setFaction(raceEntry->faction_id);
    else
        setFaction(0);

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

    setRace(charCreateContent._race);
    setClass(charCreateContent._class);
    setGender(charCreateContent.gender);

    initialiseNoseLevel();
    setInitialDisplayIds(charCreateContent.gender, charCreateContent._race);

    eventModelChange();

    // PLAYER_BYTES
    setSkinColor(charCreateContent.skin);
    setFace(charCreateContent.face);
    setHairStyle(charCreateContent.hairStyle);
    setHairColor(charCreateContent.hairColor);

    // PLAYER_BYTES_2
    setPlayerBytes2(0);
    setFacialFeatures(charCreateContent.facialHair);
    setRestState(RESTSTATE_NORMAL);

    // PLAYER_BYTES_3
    setPlayerBytes3(0);
    setPlayerGender(charCreateContent.gender);

    addPlayerFieldBytesMiscFlag(PLAYER_MISC_FLAG_SHOW_RELEASE_TIME);

    // Gold Starting Amount
    setCoinage(worldConfig.player.startGoldAmount);

    // Default value is -1
    setWatchedFaction(std::numeric_limits<uint32_t>::max());

    // Profession points
    setFreePrimaryProfessionPoints(worldConfig.player.maxProfessions);

    m_stableSlotCount = 0;

    m_firstLogin = true;

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
                sLogger.debug("StartOutfit - Item with entry {} not in item_properties table but in CharStartOutfit.dbc!", itemId);
                continue;
            }

            auto item = sObjectMgr.createItem(itemId, this);
            if (item)
            {
                item->setStackCount(1);

                int8_t itemSlot = 0;

                //shitty db lets check for dbc/db2 values
                if (itemProperties->InventoryType == 0)
                {
                    if (const auto itemDB2Properties = sItemStore.lookupEntry(itemId))
                        itemSlot = getItemInterface()->GetItemSlotByType(itemDB2Properties->InventoryType);
                }
                else
                {
                    itemSlot = getItemInterface()->GetItemSlotByType(itemProperties->InventoryType);
                }

                //use safeadd only for equipmentset items... all other items will go to a free bag slot.
                if (itemSlot < INVENTORY_SLOT_BAG_END && (itemProperties->Class == ITEM_CLASS_ARMOR || itemProperties->Class == ITEM_CLASS_WEAPON || itemProperties->Class == ITEM_CLASS_CONTAINER || itemProperties->Class == ITEM_CLASS_QUIVER))
                {
                    const auto [addResult, _] = getItemInterface()->SafeAddItem(std::move(item), INVENTORY_SLOT_NOT_SET, itemSlot);
                    if (!addResult)
                    {
                        sLogger.debug("StartOutfit - Item with entry {} can not be added safe to slot {}!", itemId, static_cast<uint32_t>(itemSlot));
                    }
                }
                else
                {
                    item->setStackCount(itemProperties->MaxCount);
                    const auto [addResult, _] = getItemInterface()->AddItemToFreeSlot(std::move(item));
                    if (!addResult)
                    {
                        sLogger.debug("StartOutfit - Item with entry {} can not be added to a free slot!", itemId);
                    }
                }
            }
        }
    }

    for (std::list<CreateInfo_ItemStruct>::const_iterator is = m_playerCreateInfo->items.begin(); is != m_playerCreateInfo->items.end(); ++is)
    {
        if ((*is).id != 0)
        {
            auto item = sObjectMgr.createItem((*is).id, this);
            if (item)
            {
                item->setStackCount((*is).amount);
                if ((*is).slot < INVENTORY_SLOT_BAG_END)
                {
                    getItemInterface()->SafeAddItem(std::move(item), INVENTORY_SLOT_NOT_SET, (*is).slot);
                }
                else
                {
                    getItemInterface()->AddItemToFreeSlot(std::move(item));
                }
            }
        }
    }

    sHookInterface.OnCharacterCreate(this);
    m_loadHealth = getMaxHealth();
    m_loadMana = getMaxPower(POWER_TYPE_MANA);
    return true;
}

WDB::Structures::ChrRacesEntry const* Player::getDbcRaceEntry() { return m_dbcRace; };
WDB::Structures::ChrClassesEntry const* Player::getDbcClassEntry() { return m_dbcClass; };

utf8_string Player::getName() const { return m_name; }
void Player::setName(utf8_string name) { m_name = name; }

uint32_t Player::getLoginFlag() const { return m_loginFlag; }
void Player::setLoginFlag(uint32_t flag) { m_loginFlag = flag; }

void Player::setInitialDisplayIds(uint8_t gender, uint8_t race)
{
    if (const auto raceEntry = sChrRacesStore.lookupEntry(race))
    {
        switch (gender)
        {
            case GENDER_MALE:
                setDisplayId(raceEntry->model_male);
                setNativeDisplayId(raceEntry->model_male);
                break;
            case GENDER_FEMALE:
                setDisplayId(raceEntry->model_female);
                setNativeDisplayId(raceEntry->model_female);
                break;
            default:
                sLogger.failure("Gender {} is not valid for Player charecters!", gender);
        }
    }
    else
    {
        sLogger.failure("Race {} is not supported by this AEVersion ({})", race, getAEVersion());
    }
}

void Player::applyLevelInfo(uint32_t newLevel)
{
    const auto previousLevel = getLevel();

    if (!m_firstLogin)
    {
        const auto previousLevelInfo = m_levelInfo;

        m_levelInfo = sObjectMgr.getLevelInfo(getRace(), getClass(), newLevel);
        if (m_levelInfo == nullptr)
            return;

        if (isDead())
            resurrect();

        setLevel(newLevel);

        setBaseHealth(m_levelInfo->HP);
        setBaseMana(m_levelInfo->Mana);

        for (uint8_t i = 0; i < STAT_COUNT; ++i)
        {
            m_baseStats[i] = m_levelInfo->Stat[i];
            calcStat(i);
        }

#if VERSION_STRING >= WotLK
        for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
            if (Item* pItem = getItemInterface()->GetInventoryItem(i))
                applyItemMods(pItem, i, true, false, true);
#endif
        updateStats();

        setHealth(getMaxHealth());

        setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
        setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
        setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
#if VERSION_STRING >= WotLK
        setPower(POWER_TYPE_RUNES, getMaxPower(POWER_TYPE_RUNES));
#endif

        sendLevelupInfoPacket(
            newLevel,
            m_levelInfo->HP - previousLevelInfo->HP,
            m_levelInfo->Mana - previousLevelInfo->Mana,
            m_levelInfo->Stat[STAT_STRENGTH] - previousLevelInfo->Stat[STAT_STRENGTH],
            m_levelInfo->Stat[STAT_AGILITY] - previousLevelInfo->Stat[STAT_AGILITY],
            m_levelInfo->Stat[STAT_STAMINA] - previousLevelInfo->Stat[STAT_STAMINA],
            m_levelInfo->Stat[STAT_INTELLECT] - previousLevelInfo->Stat[STAT_INTELLECT],
            m_levelInfo->Stat[STAT_SPIRIT] - previousLevelInfo->Stat[STAT_SPIRIT]);
    }

#if VERSION_STRING >= TBC
    // Classic does not have any level dependant flight paths
    initTaxiNodesForLevel();
#endif

    updateSkillMaximumValues();

    if (newLevel > previousLevel || m_firstLogin)
        setInitialTalentPoints();
    else if (newLevel != previousLevel)
        resetAllTalents();

    m_playerInfo->lastLevel = previousLevel;

#if VERSION_STRING >= WotLK
    updateGlyphs();
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
#endif

    if (m_firstLogin)
        sHookInterface.OnFirstEnterWorld(this);
    else
        sHookInterface.OnPostLevelUp(this);

    if (getClass() == WARLOCK)
    {
        const auto pet = getPet();
        if (pet != nullptr && pet->IsInWorld() && pet->isAlive())
        {
            pet->setLevel(newLevel);
            pet->applyStatsForLevel();
            pet->updateSpellList();
        }
    }

    smsg_TalentsInfo(false);

    m_playedTime[0] = 0;
}

bool Player::isClassMage() const { return false; }
bool Player::isClassDeathKnight() const { return false; }
bool Player::isClassPriest() const { return false; }
bool Player::isClassRogue() const { return false; }
bool Player::isClassShaman() const { return false; }
bool Player::isClassHunter() const { return false; }
bool Player::isClassWarlock() const { return false; }
bool Player::isClassWarrior() const { return false; }
bool Player::isClassPaladin() const { return false; }
bool Player::isClassMonk() const { return false; }
bool Player::isClassDruid() const { return false; }

PlayerTeam Player::getTeam() const { return m_team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
PlayerTeam Player::getBgTeam() const { return m_bgTeam == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
void Player::setTeam(uint32_t team) { m_team = team; m_bgTeam = team; }
void Player::setBgTeam(uint32_t team) { m_bgTeam = team; }

uint32_t Player::getInitialTeam() const { return m_dbcRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE; }

void Player::resetTeam()
{
    m_team = m_dbcRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE;
    m_bgTeam = m_team;
}

bool Player::isTeamHorde() const { return getTeam() == TEAM_HORDE; }
bool Player::isTeamAlliance() const { return getTeam() == TEAM_ALLIANCE; }

Unit* Player::getUnitOwner()
{
    if (getCharmedByGuid() != 0)
        return getWorldMapUnit(getCharmedByGuid());

    return nullptr;
}

Unit const* Player::getUnitOwner() const
{
    if (getCharmedByGuid() != 0)
        return getWorldMapUnit(getCharmedByGuid());

    return nullptr;
}

Unit* Player::getUnitOwnerOrSelf()
{
    if (auto* const unitOwner = getUnitOwner())
        return unitOwner;

    return this;
}

Unit const* Player::getUnitOwnerOrSelf() const
{
    if (auto* const unitOwner = getUnitOwner())
        return unitOwner;

    return this;
}

Player* Player::getPlayerOwner()
{
    if (getCharmedByGuid() != 0)
        return getWorldMapPlayer(getCharmedByGuid());

    return nullptr;
}

Player const* Player::getPlayerOwner() const
{
    if (getCharmedByGuid() != 0)
        return getWorldMapPlayer(getCharmedByGuid());

    return nullptr;
}

Player* Player::getPlayerOwnerOrSelf()
{
    if (auto* const plrOwner = getPlayerOwner())
        return plrOwner;

    return this;
}

Player const* Player::getPlayerOwnerOrSelf() const
{
    if (auto* const plrOwner = getPlayerOwner())
        return plrOwner;

    return this;
}

void Player::toggleAfk()
{
    if (hasPlayerFlags(PLAYER_FLAG_AFK))
    {
        removePlayerFlags(PLAYER_FLAG_AFK);
        if (worldConfig.getKickAFKPlayerTime())
            sEventMgr.RemoveEvents(this, EVENT_PLAYER_SOFT_DISCONNECT);
    }
    else
    {
        addPlayerFlags(PLAYER_FLAG_AFK);

        if (m_bg)
            m_bg->removePlayer(this, false);

        if (worldConfig.getKickAFKPlayerTime())
            sEventMgr.AddEvent(this, &Player::softDisconnect, EVENT_PLAYER_SOFT_DISCONNECT,
                worldConfig.getKickAFKPlayerTime(), 1, 0);
    }
}

void Player::toggleDnd()
{
    if (hasPlayerFlags(PLAYER_FLAG_DND))
        removePlayerFlags(PLAYER_FLAG_DND);
    else
        addPlayerFlags(PLAYER_FLAG_DND);
}

uint32_t* Player::getPlayedTime() { return m_playedTime; }

CachedCharacterInfo* Player::getPlayerInfo() const { return m_playerInfo; }

void Player::changeLooks(uint64_t guid, uint8_t gender, uint8_t skin, uint8_t face, uint8_t hairStyle, uint8_t hairColor, uint8_t facialHair)
{
    auto result = CharacterDatabase.Query("SELECT bytes2 FROM `characters` WHERE guid = '%u'", static_cast<uint32_t>(guid));
    if (!result)
        return;

    Field* fields = result->Fetch();

    uint32_t player_bytes2 = fields[0].asUint32();
    player_bytes2 &= ~0xFF;
    player_bytes2 |= facialHair;

    CharacterDatabase.Execute("UPDATE `characters` SET gender = '%u', bytes = '%u', bytes2 = '%u' WHERE guid = '%u'", gender, skin | (face << 8) | (hairStyle << 16) | (hairColor << 24), player_bytes2, (uint32_t)guid);
}

void Player::changeLanguage(uint64_t guid, uint8_t race)
{
    const auto getSpellIdForLanguage = [](uint16_t skillId) -> uint32_t
    {
        switch (skillId)
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
                return 7340;
            case SKILL_LANG_TROLL:
                return 7341;
            case SKILL_LANG_GUTTERSPEAK:
                return 17737;
#if VERSION_STRING >= TBC
            case SKILL_LANG_DRAENEI:
                return 29932;
#endif
#if VERSION_STRING >= Cata
            case SKILL_LANG_GOBLIN:
                return 69269;
            case SKILL_LANG_GILNEAN:
                return 69270;
#endif
#if VERSION_STRING >= Mop
            case SKILL_LANG_PANDAREN_NEUTRAL:
                return 108127;
            case SKILL_LANG_PANDAREN_ALLIANCE:
                return 108130;
            case SKILL_LANG_PANDAREN_HORDE:
                return 108131;
#endif
        }

        return 0;
    };

#if VERSION_STRING < TBC
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u');", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH), getSpellIdForLanguage(SKILL_LANG_TAURAHE), getSpellIdForLanguage(SKILL_LANG_TROLL), getSpellIdForLanguage(SKILL_LANG_GUTTERSPEAK), getSpellIdForLanguage(SKILL_LANG_THALASSIAN), getSpellIdForLanguage(SKILL_LANG_COMMON), getSpellIdForLanguage(SKILL_LANG_DARNASSIAN), getSpellIdForLanguage(SKILL_LANG_DWARVEN), getSpellIdForLanguage(SKILL_LANG_GNOMISH));
#elif VERSION_STRING < Cata
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u', '%u');", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH), getSpellIdForLanguage(SKILL_LANG_TAURAHE), getSpellIdForLanguage(SKILL_LANG_TROLL), getSpellIdForLanguage(SKILL_LANG_GUTTERSPEAK), getSpellIdForLanguage(SKILL_LANG_THALASSIAN), getSpellIdForLanguage(SKILL_LANG_COMMON), getSpellIdForLanguage(SKILL_LANG_DARNASSIAN), getSpellIdForLanguage(SKILL_LANG_DRAENEI), getSpellIdForLanguage(SKILL_LANG_DWARVEN), getSpellIdForLanguage(SKILL_LANG_GNOMISH));
#elif VERSION_STRING == Cata
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u', '%u');", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH), getSpellIdForLanguage(SKILL_LANG_TAURAHE), getSpellIdForLanguage(SKILL_LANG_TROLL), getSpellIdForLanguage(SKILL_LANG_GUTTERSPEAK), getSpellIdForLanguage(SKILL_LANG_THALASSIAN), getSpellIdForLanguage(SKILL_LANG_COMMON), getSpellIdForLanguage(SKILL_LANG_DARNASSIAN), getSpellIdForLanguage(SKILL_LANG_DRAENEI), getSpellIdForLanguage(SKILL_LANG_DWARVEN), getSpellIdForLanguage(SKILL_LANG_GNOMISH), getSpellIdForLanguage(SKILL_LANG_GILNEAN), getSpellIdForLanguage(SKILL_LANG_GOBLIN));
#elif VERSION_STRING == Mop
    CharacterDatabase.Execute("DELETE FROM `playerspells` WHERE GUID = '%u' AND SpellID IN ('%u', '%u', '%u', '%u', '%u','%u', '%u', '%u', '%u', '%u');", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH), getSpellIdForLanguage(SKILL_LANG_TAURAHE), getSpellIdForLanguage(SKILL_LANG_TROLL), getSpellIdForLanguage(SKILL_LANG_GUTTERSPEAK), getSpellIdForLanguage(SKILL_LANG_THALASSIAN), getSpellIdForLanguage(SKILL_LANG_COMMON), getSpellIdForLanguage(SKILL_LANG_DARNASSIAN), getSpellIdForLanguage(SKILL_LANG_DRAENEI), getSpellIdForLanguage(SKILL_LANG_DWARVEN), getSpellIdForLanguage(SKILL_LANG_GNOMISH), getSpellIdForLanguage(SKILL_LANG_GILNEAN), getSpellIdForLanguage(SKILL_LANG_GOBLIN), getSpellIdForLanguage(SKILL_LANG_PANDAREN_NEUTRAL), getSpellIdForLanguage(SKILL_LANG_PANDAREN_ALLIANCE), getSpellIdForLanguage(SKILL_LANG_PANDAREN_HORDE));
#endif
    switch (race)
    {
        case RACE_DWARF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_DWARVEN));
            break;
#if VERSION_STRING > Classic
        case RACE_DRAENEI:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_DRAENEI));
            break;
#endif
        case RACE_GNOME:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_GNOMISH));
            break;
        case RACE_NIGHTELF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_DARNASSIAN));
            break;
        case RACE_UNDEAD:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_GUTTERSPEAK));
            break;
        case RACE_TAUREN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_TAURAHE));
            break;
        case RACE_TROLL:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_TROLL));
            break;
#if VERSION_STRING > Classic
        case RACE_BLOODELF:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_THALASSIAN));
            break;
#endif
#if VERSION_STRING >= Cata
        case RACE_WORGEN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_GILNEAN));
            break;
        case RACE_GOBLIN:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_GOBLIN));
            break;
#endif
#if VERSION_STRING >= Mop
        case RACE_PANDAREN_NEUTRAL:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_PANDAREN_NEUTRAL));
            break;
        case RACE_PANDAREN_ALLIANCE:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_COMMON));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_PANDAREN_ALLIANCE));
            break;
        case RACE_PANDAREN_HORDE:
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_ORCISH));
            CharacterDatabase.Execute("INSERT INTO `playerspells` (GUID, SpellID) VALUES ('%u', '%u')", static_cast<uint32_t>(guid), getSpellIdForLanguage(SKILL_LANG_PANDAREN_HORDE));
            break;
#endif
    }
}

void Player::sendInitialLogonPackets()
{
    sLogger.debug("Player {} gets prepared for login.", getName());

#if VERSION_STRING == Mop
    m_session->SendPacket(SmsgBindPointUpdate(getBindPosition(), getBindMapId(), getBindZoneId()).serialise().get());

    smsg_TalentsInfo(false);

    WorldPacket data(SMSG_WORLD_SERVER_INFO, 4 + 4 + 1 + 1);
    data.writeBit(0);
    data.writeBit(0);
    data.writeBit(0);
    data.writeBit(0);
    data.flushBits();

    data << uint8_t(0);
    data << uint32_t(0);       // reset weekly quest time
    data << uint32_t(0);
    getSession()->SendPacket(&data);

    sendSmsgInitialSpells();

    m_session->SendPacket(SmsgSendUnlearnSpells().serialise().get());

    sendActionBars(0);

    sendSmsgInitialFactions();

    data.Initialize(SMSG_LOAD_EQUIPMENT_SET);
    data.writeBits(0, 19);
    getSession()->SendPacket(&data);

    m_session->SendPacket(SmsgLoginSetTimeSpeed(Util::getGameTime(), 0.0166666669777748f).serialise().get());

    data.Initialize(SMSG_SET_FORCED_REACTIONS, 1 + 4 + 4);
    data.writeBits(0, 6);
    data.flushBits();
    getSession()->SendPacket(&data);

    data.Initialize(SMSG_SETUP_CURRENCY, 3 + 1 + 4 + 4 + 4 + 4);
    data.writeBits(0, 21);
    getSession()->SendPacket(&data);

    ObjectGuid guid = getGuid();
    data.Initialize(SMSG_MOVE_SET_ACTIVE_MOVER);
    data.writeBit(guid[5]);
    data.writeBit(guid[1]);
    data.writeBit(guid[4]);
    data.writeBit(guid[2]);
    data.writeBit(guid[3]);
    data.writeBit(guid[7]);
    data.writeBit(guid[0]);
    data.writeBit(guid[6]);

    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[1]);
    getSession()->SendPacket(&data);

#else
    m_session->SendPacket(SmsgBindPointUpdate(getBindPosition(), getBindMapId(), getBindZoneId()).serialise().get());

    std::vector<uint32_t> tutorials;
    for (auto tutorial : m_tutorials)
        tutorials.push_back(tutorial);

    m_session->SendPacket(SmsgTutorialFlags(tutorials).serialise().get());

#if VERSION_STRING > TBC
    smsg_TalentsInfo(false);
#endif

    sendSmsgInitialSpells();

    m_session->SendPacket(SmsgSendUnlearnSpells().serialise().get());

    sendActionBars(0);
    sendSmsgInitialFactions();

    m_session->SendPacket(SmsgLoginSetTimeSpeed(Util::getGameTime(), 0.0166666669777748f).serialise().get());

    updateSpeed();

#if VERSION_STRING > TBC
    m_session->SendPacket(SmsgUpdateWorldState(0xC77, worldConfig.arena.arenaProgress, 0xF3D, worldConfig.arena.arenaSeason).serialise().get());
#endif
#endif

    sLogger.info("WORLD: Sent initial logon packets for {}.", getName());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Session & Packets
WorldSession* Player::getSession() const { return m_session; }
void Player::setSession(WorldSession* session) { m_session = session; }

void Player::removePendingPlayer()
{
    if (m_session)
    {
        uint8_t respons = E_CHAR_LOGIN_NO_CHARACTER;
        sendPacket(SmsgCharacterLoginFailed(respons).serialise().get());
        m_session->m_loggingInPlayer = nullptr;
    }

    m_isReadyToBeRemoved = true;
    delete this;
}

void Player::softDisconnect()
{
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_SOFT_DISCONNECT);

    if (m_session)
        m_session->LogoutPlayer(true);
}

void Player::outPacket(uint16_t opcode, uint16_t length, const void* data)
{
    if (m_session)
        m_session->OutPacket(opcode, length, data);
}

void Player::sendPacket(WorldPacket* packet)
{
    if (m_session)
        m_session->SendPacket(packet);
}

void Player::outPacketToSet(uint16_t opcode, uint16_t length, const void* data, bool sendToSelf)
{
    if (!IsInWorld())
        return;

    if (sendToSelf)
        outPacket(opcode, length, data);

    for (const auto& objectPlayer : getInRangePlayersSet())
    {
        if (Player* player = static_cast<Player*>(objectPlayer))
        {
            if (m_isGmInvisible)
            {
                if (player->getSession()->hasPermissions())
                    player->outPacket(opcode, length, data);
            }
            else
            {
                player->outPacket(opcode, length, data);
            }
        }
    }
}

void Player::sendMessageToSet(WorldPacket* data, bool sendToSelf, bool sendToOwnTeam)
{
    if (!IsInWorld())
        return;

    if (sendToSelf)
        sendPacket(data);

    for (const auto& objectPlayer : getInRangePlayersSet())
    {
        if (Player* player = static_cast<Player*>(objectPlayer))
        {
            if (player->getSession() == nullptr)
                continue;

            if (sendToOwnTeam && player->getTeam() != getTeam())
                continue;

            if ((player->GetPhase() & GetPhase()) == 0)
                continue;

            if (data->GetOpcode() != SMSG_MESSAGECHAT)
            {
                if (m_isGmInvisible && !player->getSession()->hasPermissions())
                    continue;

                if (player->isVisibleObject(getGuid()))
                    player->sendPacket(data);
            }
            else
            {
                if (!player->isIgnored(getGuidLow()))
                    player->sendPacket(data);
            }
        }
    }
}

void Player::sendDelayedPacket(WorldPacket* data, bool deleteDataOnSend)
{
    if (data == nullptr)
        return;

    if (m_session)
        m_session->SendPacket(data);

    if (deleteDataOnSend)
        delete data;
}

bool Player::compressAndSendUpdateBuffer(uint32_t size, const uint8_t* update_buffer)
{
    uint32_t destsize = size + size / 10 + 16;
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
        sLogger.failure("deflateInit failed.");
        return false;
    }

    auto buffer = std::make_unique<uint8_t[]>(destsize);

    // set up stream pointers
    stream.next_out = (Bytef*)buffer.get() + 4;
    stream.avail_out = destsize;
    stream.next_in = (Bytef*)update_buffer;
    stream.avail_in = size;

    // call the actual process
    if (deflate(&stream, Z_NO_FLUSH) != Z_OK ||
        stream.avail_in != 0)
    {
        sLogger.failure("deflate failed.");
        return false;
    }

    // finish the deflate
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        sLogger.failure("deflate failed: did not end stream");
        return false;
    }

    // finish up
    if (deflateEnd(&stream) != Z_OK)
    {
        sLogger.failure("deflateEnd failed.");
        return false;
    }

    // fill in the full size of the compressed stream
    *(uint32_t*)&buffer[0] = size;

#if VERSION_STRING < Cata
    m_session->OutPacket(SMSG_COMPRESSED_UPDATE_OBJECT, static_cast<uint16_t>(stream.total_out) + 4, buffer.get());
#else
    m_session->OutPacket(SMSG_UPDATE_OBJECT, static_cast<uint16_t>(stream.total_out) + 4, buffer.get());
#endif

    return true;
}

uint32_t Player::buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    uint32_t count = 0;
    if (target == this)
        count += getItemInterface()->m_CreateForPlayer(data);

    count += Unit::buildCreateUpdateBlockForPlayer(data, target);

    return count;
}

UpdateMask Player::m_visibleUpdateMask;

void Player::initVisibleUpdateBits()
{
#if VERSION_STRING == Mop
    Player::m_visibleUpdateMask.SetCount(getSizeOfStructure(WoWPlayer));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, guid) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, data));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, data) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, field_type.raw));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, entry));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, dynamic_field));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, scale_x));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, charm_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, charm_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, summon_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, summon_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, charmed_by_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, charmed_by_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, target_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, target_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, health));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_4));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_5));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_health));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_4));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_5));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 2));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, level));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, faction_template));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, unit_flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, unit_flags_2));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, base_attack_time, 0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, base_attack_time, 1) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, bounding_radius));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, combat_reach));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, native_display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, mount_display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, pet_number));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, pet_name_timestamp));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_spell));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, mod_cast_speed));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, npc_flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, hover_height));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_timestamp));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_team));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_arbiter));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_arbiter) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_rank));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_level));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, base_mana));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, aura_state));

    for (uint16_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        uint32_t offset = i * 2;

        Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, visible_items) + offset);
        Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, visible_items) + 1 + offset);
    }

    uint16_t questIdOffset = 5;
    for (uint16_t i = getOffsetForStructuredField(WoWPlayer, quests); i < getOffsetForStructuredField(WoWPlayer, visible_items); i += questIdOffset)
        Player::m_visibleUpdateMask.SetBit(i);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, chosen_title));

#else
    Player::m_visibleUpdateMask.SetCount(getSizeOfStructure(WoWPlayer));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, guid) + 1);
#if VERSION_STRING < Cata
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, type));
#else
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, field_type.raw));
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, entry));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWObject, scale_x));

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, summon_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, summon_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, target_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, target_guid) + 1);

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, health));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_4));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_5));
#if VERSION_STRING == WotLK
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_6));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, power_7));
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_health));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_4));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_5));
#if VERSION_STRING == WotLK
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_6));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, max_power_7));
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_slot_display, 2));

#if VERSION_STRING <= TBC
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 0) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 1) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, virtual_item_info, 2) + 1);
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, level));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, faction_template));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, unit_flags));
#if VERSION_STRING != Classic
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, unit_flags_2));
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, base_attack_time, 0));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredArrayField(WoWUnit, base_attack_time, 1) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, bounding_radius));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, combat_reach));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, native_display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, mount_display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_1));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, pet_number));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, pet_name_timestamp));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_object_guid) + 1);
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, channel_spell));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, dynamic_flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, npc_flags));
#if VERSION_STRING > TBC
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, hover_height));
#endif

    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, player_bytes_3));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_timestamp));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_team));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_arbiter));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, duel_arbiter) + 1);
#if VERSION_STRING < Cata
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_id));
#endif
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, guild_rank));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, base_mana));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, field_bytes_2));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWUnit, aura_state));

#if VERSION_STRING == TBC
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, display_id));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, flags));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, state));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, level));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, art_kit));
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWGameObject, animation_progress));
#endif

    for (uint16_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
#if VERSION_STRING > TBC
        uint32_t offset = i * 2;
#else
        uint32_t offset = i * 16;
#endif
        // visible_items includes creator guid, so add + 2 since we are not sending that as update field
        Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, visible_items) + 2 + offset);
        Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, visible_items) + 2 + 1 + offset);
    }

#if VERSION_STRING == Classic
    uint16_t questIdOffset = 3;
#elif VERSION_STRING == TBC
    uint16_t questIdOffset = 4;
#else
    uint16_t questIdOffset = 5;
#endif

    for (uint16_t i = getOffsetForStructuredField(WoWPlayer, quests); i < getOffsetForStructuredField(WoWPlayer, visible_items); i += questIdOffset)
        Player::m_visibleUpdateMask.SetBit(i);

#if VERSION_STRING != Classic
    Player::m_visibleUpdateMask.SetBit(getOffsetForStructuredField(WoWPlayer, chosen_title));
#endif
#endif
}

void Player::copyAndSendDelayedPacket(WorldPacket* data) { m_updateMgr.queueDelayedPacket(std::make_unique<WorldPacket>(*data)); }

void Player::setEnteringToWorld() { m_enteringWorld = true; }

UpdateManager& Player::getUpdateMgr() { return m_updateMgr; }

void Player::setCreateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::setCreateBits(updateMask, target);
    }
    else
    {
        for (uint32_t index = 0; index < m_valuesCount; index++)
        {
            if (m_uint32Values[index] != 0 && Player::m_visibleUpdateMask.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}

void Player::setUpdateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::setUpdateBits(updateMask, target);
    }
    else
    {
        Object::setUpdateBits(updateMask, target);
        *updateMask &= Player::m_visibleUpdateMask;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Visiblility
void Player::addVisibleObject(uint64_t guid) { m_visibleObjects.insert(guid); }
void Player::removeVisibleObject(uint64_t guid)
{
    if (isVisibleObject(guid))
    {
        m_visibleObjects.erase(guid);
#if VERSION_STRING <= TBC
        if (WoWGuid(guid).isGameObject() && !WoWGuid(guid).isTransport() && !WoWGuid(guid).isTransporter())
            sendDestroyObjectPacket(guid);
#endif
    }
}
bool Player::isVisibleObject(uint64_t guid) { return m_visibleObjects.contains(guid); }

void Player::removeIfVisiblePushOutOfRange(uint64_t guid)
{
    if (m_visibleObjects.contains(guid))
    {
        m_visibleObjects.erase(guid);
#if VERSION_STRING <= TBC
        if (WoWGuid(guid).isGameObject() && !WoWGuid(guid).isTransport() && !WoWGuid(guid).isTransporter())
            sendDestroyObjectPacket(guid);
#endif
        getUpdateMgr().pushOutOfRangeGuid(guid);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Stats
void Player::setInitialPlayerData()
{
    if (m_levelInfo != nullptr)
    {
        setBaseHealth(m_levelInfo->HP);
        setBaseMana(m_levelInfo->Mana);
    }
    else
    {
        sLogger.failure("Major error in Player::setInitialPlayerData : No LevelInfo for player (level {}, race {}, class {})!", getLevel(), getRace(), getClass());

        setBaseHealth(1);
        setBaseMana(1);
    }

    // Set max health and powers
    setMaxHealth(getBaseHealth());

    // First initialize all power fields to 0
    for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
        setMaxPower(static_cast<PowerType>(power), 0);

    // Next set correct power for each class
    switch (getClass())
    {
        case WARRIOR:
        {
            setMaxPower(POWER_TYPE_RAGE, 1000);
        } break;
#if VERSION_STRING >= Cata
        case HUNTER:
        {
            setMaxPower(POWER_TYPE_FOCUS, 100);
        } break;
#endif
        case ROGUE:
        {
            setMaxPower(POWER_TYPE_ENERGY, 100);
        } break;
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
        {
            setMaxPower(POWER_TYPE_RUNES, 8);
            setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
        } break;
#endif
        default:
        {
#if VERSION_STRING >= Cata
            // Another switch case to set secondary powers
            switch (getClass())
            {
                case PALADIN:
                    setMaxPower(POWER_TYPE_HOLY_POWER, 3);
                    break;
                case WARLOCK:
                    setMaxPower(POWER_TYPE_SOUL_SHARDS, 3);
                    break;
                case DRUID:
                    setMaxPower(POWER_TYPE_ECLIPSE, 100);
                    break;
                default:
                    break;
            }
#endif
            setMaxPower(POWER_TYPE_MANA, getBaseMana());
        } break;
    }

    setMinDamage(0.0f);
    setMaxDamage(0.0f);
    setMinOffhandDamage(0.0f);
    setMaxOffhandDamage(0.0f);
    setMinRangedDamage(0.0f);
    setMaxRangedDamage(0.0f);

    setBaseAttackTime(MELEE, 2000);
    setBaseAttackTime(OFFHAND, 2000);
    setBaseAttackTime(RANGED, 2000);

    setAttackPower(0);
    setAttackPowerMods(0);
    setAttackPowerMultiplier(0.0f);
    setRangedAttackPower(0);
    setRangedAttackPowerMods(0);
    setRangedAttackPowerMultiplier(0.0f);

    setBlockPercentage(0.0f);
    setDodgePercentage(0.0f);
    setParryPercentage(0.0f);
    setMeleeCritPercentage(0.0f);
    setRangedCritPercentage(0.0f);
#if VERSION_STRING >= TBC
    setExpertise(0);
    setOffHandExpertise(0);
    setOffHandCritPercentage(0.0f);
    setShieldBlock(0);
#endif
#if VERSION_STRING >= WotLK
    setShieldBlockCritPercentage(0.0f);
#endif

#if VERSION_STRING >= TBC
    setModHealingDone(0);

    setModTargetResistance(0);
    setModTargetPhysicalResistance(0);
#endif

    setModCastSpeed(1.0f);

    for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
    {
        setModDamageDonePositive(i, 0);
        setModDamageDoneNegative(i, 0);
        setModDamageDonePct(1.0f, i);

#if VERSION_STRING >= TBC
        setSpellCritPercentage(i, 0.0f);
#endif
        setResistance(i, 0);

        setPowerCostModifier(i, 0);
        setPowerCostMultiplier(i, 0.0f);
    }

#if VERSION_STRING >= WotLK
    for (uint8_t i = 0; i < WOWPLAYER_NO_REAGENT_COST_COUNT; ++i)
    {
        setNoReagentCost(i, 0);
    }
#endif

    for (uint8_t i = 0; i < MAX_PCR; ++i)
        setCombatRating(i, 0);

    for (uint8_t i = 0; i < STAT_COUNT; ++i)
    {
        m_baseStats[i] = m_levelInfo->Stat[i];
        calcStat(i);
    }

    updateStats();

    setMaxLevel(worldConfig.player.playerLevelCap);

    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
#if VERSION_STRING == TBC
    setPositiveAuraLimit(POS_AURA_LIMIT_PVP_ATTACKABLE);
#elif VERSION_STRING >= WotLK
    addUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

    // Set current health and power after stats are loaded
    setHealth(getMaxHealth());
    setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
    setPower(POWER_TYPE_RAGE, 0);
    setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
    setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
#if VERSION_STRING >= WotLK
    setPower(POWER_TYPE_RUNES, getMaxPower(POWER_TYPE_RUNES));
    setPower(POWER_TYPE_RUNIC_POWER, 0);
#endif
}

void Player::regeneratePlayerPowers(uint16_t diff)
{
#if VERSION_STRING < WotLK
    // Rage
    m_rageRegenerateTimer += diff;
    if (m_rageRegenerateTimer >= REGENERATION_INTERVAL_RAGE)
    {
        regeneratePower(POWER_TYPE_RAGE, m_rageRegenerateTimer);
        m_rageRegenerateTimer = 0;
    }
#endif

#if VERSION_STRING >= Cata
    // Holy Power
    if (isClassPaladin())
    {
        m_holyPowerRegenerateTimer += diff;
        if (m_holyPowerRegenerateTimer >= REGENERATION_INTERVAL_HOLY_POWER)
        {
            regeneratePower(POWER_TYPE_HOLY_POWER, m_holyPowerRegenerateTimer);
            m_holyPowerRegenerateTimer = 0;
        }
    }
#endif

    // Food/Drink visual effect
    // Confirmed from sniffs that the timer keeps going on even when there is no food/drink aura
    if (diff >= m_foodDrinkSpellVisualTimer)
    {
        // Find food/drink aura
        const auto findFoodOrDrinkAura = [this](AuraEffect auraEffect) -> bool
        {
            for (const auto& aurEff : getAuraEffectList(auraEffect))
            {
                if (aurEff->getAura()->IsPassive() || aurEff->getAura()->isNegative())
                    continue;
                if (aurEff->getAura()->getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                    return true;
            }
            return false;
        };

        // Food takes priority over drink
        if (findFoodOrDrinkAura(SPELL_AURA_MOD_HEALTH_REGEN) || findFoodOrDrinkAura(SPELL_AURA_PERIODIC_HEAL_PCT))
            playSpellVisual(SPELL_VISUAL_FOOD, 0);
        else if (findFoodOrDrinkAura(SPELL_AURA_MOD_POWER_REGEN) || findFoodOrDrinkAura(SPELL_AURA_PERIODIC_POWER_PCT))
            playSpellVisual(SPELL_VISUAL_DRINK, 0);

        m_foodDrinkSpellVisualTimer = 5000;
    }
    else
    {
        m_foodDrinkSpellVisualTimer -= diff;
    }
}

#if VERSION_STRING >= Cata
void Player::resetHolyPowerTimer()
{
    m_holyPowerRegenerateTimer = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Database stuff
bool Player::loadSpells(QueryResult* result)
{
    // Add initial spells on first login
    if (m_firstLogin)
    {
        for (const auto& spellId : m_playerCreateInfo->spell_list)
            addSpell(spellId);

        return true;
    }

    if (result == nullptr)
        return false;

    do
    {
        const auto fields = result->Fetch();
        const auto spellId = fields[0].asUint32();

        // addSpell will validate spell id
        addSpell(spellId);
    } while (result->NextRow());

    return true;
}

bool Player::loadSkills(QueryResult* result)
{
    if (result == nullptr)
        return false;

    do
    {
        const auto fields = result->Fetch();

        const auto skillid = fields[0].asUint16();
        const auto currval = fields[1].asUint16();
        const auto maxval = fields[2].asUint16();

        addSkillLine(skillid, currval, maxval);
    } while (result->NextRow());

    return true;
}

bool Player::loadReputations(QueryResult* result)
{
    // Add initial reputations on first login
    if (m_firstLogin)
    {
        initialiseReputation();
        return true;
    }

    if (result == nullptr)
        return false;

    do
    {
        const auto field = result->Fetch();

        const auto id = field[0].asUint32();
        const auto flag = field[1].asUint8();
        const auto basestanding = field[2].asInt32();
        const auto standing = field[3].asInt32();

        const auto faction = sFactionStore.lookupEntry(id);
        if (faction == nullptr || faction->RepListId < 0)
            continue;

        const auto [repItr, _] = m_reputation.insert_or_assign(id, std::make_unique<FactionReputation>(standing, flag, basestanding));
        m_reputationByListId[faction->RepListId] = repItr->second.get();
    } while (result->NextRow());

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells and skills
bool Player::hasSpell(uint32_t spellId) const
{
    return m_spellSet.find(spellId) != m_spellSet.cend();
}

bool Player::hasDeletedSpell(uint32_t spellId) const
{
    return m_deletedSpellSet.find(spellId) != m_deletedSpellSet.cend();
}

void Player::addSpell(uint32_t spellId, uint16_t fromSkill/* = 0*/)
{
    _addSpell(spellId, fromSkill, false, false);
}

void Player::addDeletedSpell(uint32_t spellId)
{
    m_deletedSpellSet.emplace(spellId);
}

bool Player::removeSpell(uint32_t spellId, bool moveToDeleted)
{
    return _removeSpell(spellId, moveToDeleted, false, false, false);
}

bool Player::removeDeletedSpell(uint32_t spellId)
{
    const auto itr = std::as_const(m_deletedSpellSet).find(spellId);
    if (itr == m_deletedSpellSet.cend())
        return false;

    m_deletedSpellSet.erase(itr);
    return true;
}

SpellSet const& Player::getSpellSet() const
{
    return m_spellSet;
}

SpellSet const& Player::getDeletedSpellSet() const
{
    return m_deletedSpellSet;
}

void Player::sendSmsgInitialSpells()
{
    auto smsgInitialSpells = SmsgSendKnownSpells();

    uint32_t mstime = Util::getMSTime();

    for (const auto& spellId : m_spellSet)
    {
        smsgInitialSpells.addSpellIds(spellId);
    }

    for (auto itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end();)
    {
        auto itr2 = itr++;

        if (itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000)
        {
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr2);
            continue;
        }

        sLogger.debug("InitialSpells sending spell cooldown for spell {} to {} ms", itr2->first, itr2->second.ExpireTime - mstime);

        smsgInitialSpells.addSpellCooldown(itr2->first, itr2->second.ItemId, 0, itr2->second.ExpireTime - mstime, 0);
    }

    for (auto itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end();)
    {
        PlayerCooldownMap::iterator itr2 = itr++;

        if (itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000)
        {
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr2);
            continue;
        }

        sLogger.debug("InitialSpells sending category cooldown for cat {} to {} ms", itr2->first, itr2->second.ExpireTime - mstime);

        smsgInitialSpells.addSpellCooldown(itr2->first, itr2->second.ItemId, static_cast<uint16_t>(itr2->first), 0, itr2->second.ExpireTime - mstime);
    }

    getSession()->SendPacket(smsgInitialSpells.serialise().get());
}

void Player::sendPreventSchoolCast(uint32_t spellSchool, uint32_t timeMs)
{
    std::vector<SmsgSpellCooldownMap> spellCoodlownMap;

    for (const auto& SpellId : m_spellSet)
    {
        if (const auto* spellInfo = sSpellMgr.getSpellInfo(SpellId))
        {
            // Not send cooldown for this spells
            if (spellInfo->getAttributes() & ATTRIBUTES_TRIGGER_COOLDOWN)
                continue;

            if (spellInfo->getFirstSchoolFromSchoolMask() == spellSchool)
            {
                SmsgSpellCooldownMap smsgSpellCooldownMap;
                smsgSpellCooldownMap.spellId = SpellId;
                smsgSpellCooldownMap.duration = timeMs;

                spellCoodlownMap.push_back(smsgSpellCooldownMap);
            }
        }
    }
    getSession()->SendPacket(SmsgSpellCooldown(getGuid(), 0x0, spellCoodlownMap).serialise().get());
}

void Player::resetSpells()
{
    if (PlayerCreateInfo const* playerCreateInfo = sMySQLStore.getPlayerCreateInfo(getRace(), getClass()))
    {
        std::list<uint32_t> spelllist;

        for (const auto& spellId : m_spellSet)
            spelllist.push_back(spellId);

        for (std::list<uint32_t>::iterator itr = spelllist.begin(); itr != spelllist.end(); ++itr)
            removeSpell((*itr), false);

        m_deletedSpellSet.clear();

        for (std::set<uint32_t>::iterator sp = playerCreateInfo->spell_list.begin(); sp != playerCreateInfo->spell_list.end(); ++sp)
        {
            if (*sp)
                addSpell(*sp);
        }
    }
}

void Player::addShapeShiftSpell(uint32_t spellId)
{
    SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(spellId);
    m_shapeshiftSpells.emplace(spellId);

    if (spellInfo->getRequiredShapeShift() && getShapeShiftMask() & spellInfo->getRequiredShapeShift())
    {
        Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
        SpellCastTargets spellCastTargets(this->getGuid());
        spell->prepare(&spellCastTargets);
    }
}

void Player::removeShapeShiftSpell(uint32_t spellId)
{
    m_shapeshiftSpells.erase(spellId);
    removeAllAurasById(spellId);
}

SpellSet const& Player::getShapeshiftSpells() const
{
    return m_shapeshiftSpells;
}

void Player::sendAvailSpells(WDB::Structures::SpellShapeshiftFormEntry const* shapeshiftFormEntry, bool active)
{
    if (active)
    {
        if (!shapeshiftFormEntry)
            return;

        // Send the spells
        SmsgPetActionsArray actions{};
        for (uint8_t i = 0; i < 8; ++i)
        {
#if VERSION_STRING >= TBC
            actions[i] = packPetActionButtonData(shapeshiftFormEntry->spells[i], PET_SPELL_STATE_DEFAULT);
#else
            actions[i] = 0;
#endif
        }
        actions[8] = 0;
        actions[9] = 0;

        getSession()->SendPacket(SmsgPetSpells(getGuid(), 0, 0, 0, 0, 0, std::move(actions), SmsgPetSpellsVector()).serialise().get());
    }
    else
    {
        sendEmptyPetSpellList();
    }
}

bool Player::isInFeralForm()
{
    //\todo shapeshiftform is never negative.
    int s = getShapeShiftForm();
    if (s <= 0)
        return false;

    // Fight forms that do not use player's weapon
    return (s == FORM_BEAR || s == FORM_DIREBEAR || s == FORM_CAT);     //Shady: actually ghostwolf form doesn't use weapon too.
}

#if VERSION_STRING >= TBC
bool Player::isInDisallowedMountForm() const
{
    if (auto form = UnitBytes_ShapeshiftForm(getShapeShiftForm()))
    {
        WDB::Structures::SpellShapeshiftFormEntry const* shapeshift = sSpellShapeshiftFormStore.lookupEntry(form);
        if (!shapeshift)
            return true;

        if (!(shapeshift->Flags & 0x1))
            return true;
    }

    if (getDisplayId() == getNativeDisplayId())
        return false;

    WDB::Structures::CreatureDisplayInfoEntry const* display = sCreatureDisplayInfoStore.lookupEntry(getDisplayId());
    if (!display)
        return true;

    WDB::Structures::CreatureDisplayInfoExtraEntry const* displayExtra = sCreatureDisplayInfoExtraStore.lookupEntry(display->ExtendedDisplayInfoID);
    if (!displayExtra)
        return true;

    WDB::Structures::CreatureModelDataEntry const* model = sCreatureModelDataStore.lookupEntry(display->ModelID);
    WDB::Structures::ChrRacesEntry const* race = sChrRacesStore.lookupEntry(displayExtra->Race);

    if (model && !(model->Flags & 0x80))
        if (race && !(race->flags & 0x4))
            return true;

    return false;
}
#else
bool Player::isInDisallowedMountForm() const
{
    auto form = UnitBytes_ShapeshiftForm(getShapeShiftForm());
    return form != FORM_NORMAL && form != FORM_BATTLESTANCE && form != FORM_BERSERKERSTANCE && form != FORM_DEFENSIVESTANCE &&
        form != FORM_SHADOW && form != FORM_STEALTH;
}
#endif

void Player::updateAutoRepeatSpell()
{
    // Get the autorepeat spell
    const auto autoRepeatSpell = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL);

    // If player is moving or casting a spell, interrupt wand casting and delay auto shot
    const auto isAutoShot = autoRepeatSpell->getSpellInfo()->getId() == 75;
    if (m_isMoving || isCastingSpell(false, true, isAutoShot))
    {
        if (!isAutoShot)
        {
            interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        }
        m_FirstCastAutoRepeat = true;
        return;
    }

    // Apply delay to wand shooting
    if (m_FirstCastAutoRepeat && (getAttackTimer(RANGED) - Util::getMSTime() < 500) && !isAutoShot)
    {
        setAttackTimer(RANGED, 500);
    }
    m_FirstCastAutoRepeat = false;

    if (isAttackReady(RANGED))
    {
        const auto canCastAutoRepeatSpell = autoRepeatSpell->canCast(true, 0, 0);
        if (canCastAutoRepeatSpell != SPELL_CAST_SUCCESS)
        {
            if (!isAutoShot)
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            else if (isPlayer())
                autoRepeatSpell->sendCastResult(canCastAutoRepeatSpell);
            return;
        }

        // Cast the spell with triggered flag
        const auto newAutoRepeatSpell = sSpellMgr.newSpell(this, autoRepeatSpell->getSpellInfo(), true, nullptr);
        newAutoRepeatSpell->prepare(&autoRepeatSpell->m_targets);

        setAttackTimer(RANGED, getBaseAttackTime(RANGED));
    }
}

bool Player::canUseFlyingMountHere()
{
#if VERSION_STRING == Classic
    return false;
#else
    auto areaEntry = GetArea();
    if (areaEntry == nullptr)
        // If area is null, try finding any area from the zone with zone id
        areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(getZoneId());
    if (areaEntry == nullptr)
        return false;

    // Not flyable areas (such as Dalaran in wotlk)
    if (areaEntry->flags & MapManagement::AreaManagement::AreaFlags::AREA_FLAG_NO_FLY_ZONE)
        return false;

    // Get continent map id
    auto mapId = GetMapId();
    if (mapId == 530 || mapId == 571)
    {
        const auto worldMapEntry = sWorldMapAreaStore.lookupEntry(getZoneId());
        if (worldMapEntry != nullptr)
            mapId = worldMapEntry->continentMapId >= 0 ? worldMapEntry->continentMapId : worldMapEntry->mapId;
    }

    switch (mapId)
    {
        // Eastern Kingdoms
        case 0:
        // Kalimdor
        case 1:
            // Flight Master's License
            if (!hasSpell(90267))
                return false;
            break;
        // Outland
        case 530:
            return true;
        // Northrend
        case 571:
            // Cold Weather Flying
            if (!hasSpell(54197))
                return false;
            break;
        default:
            return false;
    }
    return true;
#endif
}

bool Player::canDualWield2H() const
{
    return m_canDualWield2H;
}

void Player::setDualWield2H(bool enable)
{
    m_canDualWield2H = enable;
}

bool Player::isSpellFitByClassAndRace(uint32_t spell_id) const
{
    const auto spellSkillRange = sSpellMgr.getSkillEntryRangeForSpell(spell_id);

    // If spell does not exist in sSkillLineAbilityStore assume it fits for player
    if (spellSkillRange.empty())
        return true;

    const auto raceMask = getRaceMask();
    const auto classMask = getClassMask();

    for (const auto& [_, skillEntry] : spellSkillRange)
    {
        // skip wrong race skills
        if (skillEntry->race_mask > 0 && !(skillEntry->race_mask & raceMask))
            continue;

        // skip wrong class skills
        if (skillEntry->class_mask > 0 && !(skillEntry->class_mask & classMask))
            continue;

        return true;
    }

    return false;
}

bool Player::hasSpellOnCooldown(SpellInfo const* spellInfo)
{
    const auto curTime = Util::getMSTime();

    // Check category cooldown
    if (spellInfo->getCategory() > 0)
    {
        const auto itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(spellInfo->getCategory());
        if (itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
        {
            if (curTime < itr->second.ExpireTime)
                return true;

            // Cooldown has expired
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    // Check spell cooldown
    const auto itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellInfo->getId());
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (curTime < itr->second.ExpireTime)
            return true;

        // Cooldown has expired
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    return false;
}

bool Player::hasSpellGlobalCooldown(SpellInfo const* spellInfo)
{
    const auto curTime = Util::getMSTime();

    // Check for cooldown cheat as well
    if (spellInfo->getStartRecoveryTime() > 0 && m_globalCooldown > 0 && !m_cheats.hasCooldownCheat)
    {
        if (curTime < m_globalCooldown)
            return true;

        // Global cooldown has expired
        m_globalCooldown = 0;
    }

    return false;
}

void Player::addSpellCooldown(SpellInfo const* spellInfo, Item const* itemCaster, Spell* castingSpell/* = nullptr*/, int32_t cooldownTime/* = 0*/)
{
    const auto curTime = Util::getMSTime();
    const auto spellId = spellInfo->getId();

    // Set category cooldown
    int32_t spellCategoryCooldown = static_cast<int32_t>(spellInfo->getCategoryRecoveryTime());
    if (spellCategoryCooldown > 0 && spellInfo->getCategory() > 0)
    {
        // Add cooldown modifiers
        if (castingSpell != nullptr)
            applySpellModifiers(SPELLMOD_COOLDOWN_DECREASE, &spellCategoryCooldown, spellInfo, castingSpell);

        _addCategoryCooldown(spellInfo->getCategory(), spellCategoryCooldown + curTime, spellId, itemCaster != nullptr ? itemCaster->getEntry() : 0);
    }

    // Set spell cooldown
    int32_t spellCooldown = cooldownTime == 0 ? static_cast<int32_t>(spellInfo->getRecoveryTime()) : cooldownTime;
    if (spellCooldown > 0)
    {
        // Add cooldown modifers
        if (castingSpell != nullptr)
            applySpellModifiers(SPELLMOD_COOLDOWN_DECREASE, &spellCooldown, spellInfo, castingSpell);

        _addCooldown(COOLDOWN_TYPE_SPELL, spellId, spellCooldown + curTime, spellId, itemCaster != nullptr ? itemCaster->getEntry() : 0);
    }

    // Send cooldown packet
    sendSpellCooldownPacket(spellInfo, spellCooldown > spellCategoryCooldown ? spellCooldown : spellCategoryCooldown, false);
}

void Player::addGlobalCooldown(SpellInfo const* spellInfo, Spell* castingSpell, const bool sendPacket/* = false*/)
{
    if (spellInfo->getStartRecoveryTime() == 0 && spellInfo->getStartRecoveryCategory() == 0)
        return;

    const auto curTime = Util::getMSTime();
    auto gcdDuration = static_cast<int32_t>(spellInfo->getStartRecoveryTime());

    // Apply global cooldown modifiers
    applySpellModifiers(SPELLMOD_GLOBAL_COOLDOWN, &gcdDuration, spellInfo, castingSpell);

    // Apply haste modifier only to magic spells
    if (spellInfo->getStartRecoveryCategory() == 133 && spellInfo->getDmgClass() == SPELL_DMG_TYPE_MAGIC &&
        !(spellInfo->getAttributes() & ATTRIBUTES_RANGED) && !(spellInfo->getAttributes() & ATTRIBUTES_ABILITY))
    {
        gcdDuration = static_cast<int32_t>(gcdDuration * getModCastSpeed());

        // Global cooldown cannot be shorter than 1 second or longer than 1.5 seconds
        gcdDuration = std::max(gcdDuration, 1000);
        gcdDuration = std::min(gcdDuration, 1500);
    }

    if (gcdDuration <= 0)
        return;

    m_globalCooldown = curTime + gcdDuration;

    if (sendPacket)
        sendSpellCooldownPacket(spellInfo, 0, true);
}

void Player::sendSpellCooldownPacket(SpellInfo const* spellInfo, const uint32_t duration, const bool isGcd)
{
    std::vector<SmsgSpellCooldownMap> spellMap;

    SmsgSpellCooldownMap mapMembers;
    mapMembers.spellId = spellInfo->getId();
    mapMembers.duration = duration;

    spellMap.push_back(mapMembers);

    sendMessageToSet(SmsgSpellCooldown(GetNewGUID(), isGcd, spellMap).serialise().get(), true);
}

void Player::clearCooldownForSpell(uint32_t spellId)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    // Send cooldown clear packet
    getSession()->SendPacket(SmsgClearCooldown(spellId, getGuid()).serialise().get());

    for (uint8_t i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (auto itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            auto cooldown = (*itr);
            if ((i == COOLDOWN_TYPE_CATEGORY && cooldown.first == spellInfo->getCategory()) ||
                (i == COOLDOWN_TYPE_SPELL && cooldown.first == spellInfo->getId()))
            {
                itr = m_cooldownMap[i].erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
}

void Player::clearGlobalCooldown()
{
    m_globalCooldown = Util::getMSTime();
}

void Player::resetAllCooldowns()
{
    // Clear spell cooldowns
    for (const auto& spell : m_spellSet)
        clearCooldownForSpell(spell);

    // Clear global cooldown
    clearGlobalCooldown();

    // Clear other cooldowns, like items
    for (uint8_t i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (auto itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            auto spellId = (*itr).second.SpellId;
            getSession()->SendPacket(SmsgClearCooldown(spellId, getGuid()).serialise().get());
            itr = m_cooldownMap[i].erase(itr);
        }
    }

    // Clear proc cooldowns
    clearProcCooldowns();
}

void Player::cooldownAddItem(ItemProperties const* itemProp, uint32_t spellIndex)
{
    if (itemProp->Spells[spellIndex].CategoryCooldown <= 0 && itemProp->Spells[spellIndex].Cooldown <= 0)
        return;

    if (m_cheats.hasCooldownCheat)
        return;

    ItemSpell const* itemSpell = &itemProp->Spells[spellIndex];
    uint32_t mstime = Util::getMSTime();

    uint32_t itemSpellId = itemSpell->Id;

    uint32_t categoryId = itemSpell->Category;
    int32_t categoryCooldownTime = itemSpell->CategoryCooldown;

    if (itemSpell->CategoryCooldown > 0)
        _addCategoryCooldown(categoryId, categoryCooldownTime + mstime, itemSpellId, itemProp->ItemId);

    int32_t cooldownTime = itemSpell->Cooldown;
    if (cooldownTime > 0)
        _addCooldown(COOLDOWN_TYPE_SPELL, itemSpellId, cooldownTime + mstime, itemSpellId, itemProp->ItemId);
}

bool Player::cooldownCanCast(ItemProperties const* itemProp, uint32_t spellIndex)
{
    PlayerCooldownMap::iterator cooldownMapItr;
    ItemSpell const* itemSpell = &itemProp->Spells[spellIndex];
    uint32_t mstime = Util::getMSTime();

    if (itemSpell->Category)
    {
        cooldownMapItr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(itemSpell->Category);
        if (cooldownMapItr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
        {
            if (mstime < cooldownMapItr->second.ExpireTime)
                return false;

            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(cooldownMapItr);
        }
    }

    cooldownMapItr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(itemSpell->Id);
    if (cooldownMapItr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (mstime < cooldownMapItr->second.ExpireTime)
            return false;

        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(cooldownMapItr);
    }

    return true;
}

void Player::updatePotionCooldown()
{
    if (m_lastPotionId == 0 || getCombatHandler().isInCombat())
        return;

    if (const auto itemProperties = sMySQLStore.getItemProperties(m_lastPotionId))
    {
        for (uint8_t spellIndex = 0; spellIndex < MAX_ITEM_PROTO_SPELLS; ++spellIndex)
        {
            if (itemProperties->Spells[spellIndex].Id != 0 && itemProperties->Spells[spellIndex].Trigger == USE)
            {
                if (const auto* const spellInfo = sSpellMgr.getSpellInfo(itemProperties->Spells[spellIndex].Id))
                {
                    cooldownAddItem(itemProperties, spellIndex);
                    sendSpellCooldownEventPacket(spellInfo->getId());
                }
            }
        }
    }

    m_lastPotionId = 0;
}

bool Player::hasSpellWithAuraNameAndBasePoints(uint32_t auraName, uint32_t basePoints)
{
    for (const auto& spellId : m_spellSet)
    {
        SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(spellId);

        for (uint8_t effectIndex = 0; effectIndex < 3; ++effectIndex)
        {
            if (spellInfo->getEffect(effectIndex) == SPELL_EFFECT_APPLY_AURA)
            {
                if (spellInfo->getEffectApplyAuraName(effectIndex) == auraName && spellInfo->getEffectBasePoints(effectIndex) == static_cast<int32_t>(basePoints - 1))
                    return true;
            }
        }

    }

    return false;
}

void Player::_addCategoryCooldown(uint32_t categoryId, uint32_t time, uint32_t SpellId, uint32_t ItemId)
{
    PlayerCooldownMap::iterator cooldownItr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(categoryId);
    if (cooldownItr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
    {
        auto& playerCooldown = cooldownItr->second;
        if (playerCooldown.ExpireTime < time)
        {
            playerCooldown.ExpireTime = time;
            playerCooldown.ItemId = ItemId;
            playerCooldown.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown playerCooldown;
        playerCooldown.ExpireTime = time;
        playerCooldown.ItemId = ItemId;
        playerCooldown.SpellId = SpellId;

        m_cooldownMap[COOLDOWN_TYPE_CATEGORY].insert(std::make_pair(categoryId, playerCooldown));
    }

    sLogger.debug("Player::_addCategoryCooldown added cooldown for COOLDOWN_TYPE_CATEGORY category_type {} time {} item {} spell {}", categoryId, time - Util::getMSTime(), ItemId, SpellId);
}

void Player::_addCooldown(uint32_t type, uint32_t mis, uint32_t time, uint32_t SpellId, uint32_t ItemId)
{
    PlayerCooldownMap::iterator cooldownItr = m_cooldownMap[type].find(mis);
    if (cooldownItr != m_cooldownMap[type].end())
    {
        auto& playerCooldown = cooldownItr->second;
        if (playerCooldown.ExpireTime < time)
        {
            playerCooldown.ExpireTime = time;
            playerCooldown.ItemId = ItemId;
            playerCooldown.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown playerCooldown;
        playerCooldown.ExpireTime = time;
        playerCooldown.ItemId = ItemId;
        playerCooldown.SpellId = SpellId;

        m_cooldownMap[type].insert(std::make_pair(mis, playerCooldown));
    }

    sLogger.debug("Player::_addCooldown added cooldown for type {} misc {} time {} item {} spell {}", type, mis, time - Util::getMSTime(), ItemId, SpellId);
}

void Player::_loadPlayerCooldowns(QueryResult* result)
{
    if (result == nullptr)
        return;

    uint32_t mstime = Util::getMSTime();

    do
    {
        uint32_t type = result->Fetch()[0].asUint32();
        uint32_t misc = result->Fetch()[1].asUint32();
        uint32_t rtime = result->Fetch()[2].asUint32();
        uint32_t spellid = result->Fetch()[3].asUint32();
        uint32_t itemid = result->Fetch()[4].asUint32();

        if (type >= NUM_COOLDOWN_TYPES)
            continue;

        if ((uint32_t)UNIXTIME > rtime)
            continue;

        rtime -= (uint32_t)UNIXTIME;

        if (rtime < 10)
            continue;

        uint32_t realtime = mstime + ((rtime) * 1000);

        // apply it back into cooldown map
        PlayerCooldown playerCooldown;
        playerCooldown.ExpireTime = realtime;
        playerCooldown.ItemId = itemid;
        playerCooldown.SpellId = spellid;
        m_cooldownMap[type].insert(std::make_pair(misc, playerCooldown));

    } while (result->NextRow());
}

void Player::_savePlayerCooldowns(QueryBuffer* buf)
{
    uint32_t mstime = Util::getMSTime();

    if (buf != nullptr)
        buf->AddQuery("DELETE FROM playercooldowns WHERE player_guid = %u", getGuidLow());
    else
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", getGuidLow());

    for (uint32_t index = 0; index < NUM_COOLDOWN_TYPES; ++index)
    {
        for (PlayerCooldownMap::iterator cooldownItr = m_cooldownMap[index].begin(); cooldownItr != m_cooldownMap[index].end();)
        {
            PlayerCooldownMap::iterator nextCooldownItr = cooldownItr++;

            if (mstime >= nextCooldownItr->second.ExpireTime)
            {
                m_cooldownMap[index].erase(nextCooldownItr);
                continue;
            }

            if ((nextCooldownItr->second.ExpireTime - mstime) < COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN)
                continue;

            uint32_t seconds = (nextCooldownItr->second.ExpireTime - mstime) / 1000;

            if (buf != nullptr)
            {
                buf->AddQuery("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", getGuidLow(),
                    index, nextCooldownItr->first, seconds + (uint32_t)UNIXTIME, nextCooldownItr->second.SpellId, nextCooldownItr->second.ItemId);
            }
            else
            {
                CharacterDatabase.Execute("INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", getGuidLow(),
                    index, nextCooldownItr->first, seconds + (uint32_t)UNIXTIME, nextCooldownItr->second.SpellId, nextCooldownItr->second.ItemId);
            }
        }
    }
}

void Player::advanceAllSkills(uint16_t amount/* = 1*/)
{
    for (const auto& itr : m_skills)
    {
        advanceSkillLine(itr.first, amount);
    }
}

void Player::advanceSkillLine(uint16_t skillLine, uint16_t amount/* = 1*/)
{
    if (skillLine == 0)
        return;

    auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end() || itr->second.CurrentValue == 0)
    {
        // Add the skill line to player
        // addSkillLine will set correct maximum value
        addSkillLine(skillLine, amount, 0);
        sHookInterface.OnAdvanceSkillLine(this, skillLine, amount);
        return;
    }

    uint16_t skillStep = 0;
    const auto currentValue = itr->second.CurrentValue;

    const uint16_t newValue = currentValue + amount;
    itr->second.CurrentValue = std::min(newValue, itr->second.MaximumValue);

    // Skill value did not change
    if (itr->second.CurrentValue == currentValue)
        return;

    // Get skill step
    if (itr->second.Skill->type == SKILL_TYPE_PROFESSION || itr->second.Skill->type == SKILL_TYPE_SECONDARY)
        skillStep = itr->second.MaximumValue / 75U;

    // Update skill fields
#if VERSION_STRING >= Cata
    if (itr->second.Skill->type != SKILL_TYPE_WEAPON)
#endif
    {
        _updateSkillFieldOnValueChange(itr->second.FieldPosition, skillStep, itr->second.CurrentValue, itr->second.MaximumValue);
    }

    sHookInterface.OnAdvanceSkillLine(this, skillLine, itr->second.CurrentValue);

#ifdef FT_ACHIEVEMENTS
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, skillLine, skillStep);
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, skillLine, itr->second.CurrentValue);
#endif

    learnSkillSpells(skillLine, itr->second.CurrentValue);
}

void Player::addSkillLine(uint16_t skillLine, uint16_t currentValue, uint16_t maxValue, bool noSpellLearning/* = false*/, bool initializeProfession/* = false*/)
{
    if (skillLine == 0)
        return;

    const auto skillEntry = sSkillLineStore.lookupEntry(skillLine);
    if (skillEntry == nullptr)
        return;

    uint16_t skillStep = 0;
    currentValue = currentValue > DBC_PLAYER_SKILL_MAX ? DBC_PLAYER_SKILL_MAX : currentValue;
    maxValue = maxValue > DBC_PLAYER_SKILL_MAX ? DBC_PLAYER_SKILL_MAX : maxValue;

    if (!initializeProfession)
        currentValue = currentValue < 1 ? 1U : currentValue;

    const auto onLearnedNewSkill = [&](uint16_t curVal, uint16_t skillStep, bool isPrimaryProfession) -> void
    {
#if VERSION_STRING >= Cata
        // Profession skill line
        if (isPrimaryProfession)
        {
            if (getProfessionSkillLine(0) == 0 && getProfessionSkillLine(1) != skillLine)
                setProfessionSkillLine(0, skillLine);
            else if (getProfessionSkillLine(1) == 0 && getProfessionSkillLine(0) != skillLine)
                setProfessionSkillLine(1, skillLine);
        }
#endif
        // Set profession points
        if (isPrimaryProfession)
            modFreePrimaryProfessionPoints(-1);

        // Reapply skill passive auras
        for (const auto& aura : getAuraList())
        {
            if (aura == nullptr)
                continue;

            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                const auto aurEff = aura->getAuraEffect(i);
                if (aurEff->getAuraEffectType() == SPELL_AURA_NONE)
                    continue;

                if (aurEff->getAuraEffectType() != SPELL_AURA_MOD_SKILL &&
                    aurEff->getAuraEffectType() != SPELL_AURA_MOD_SKILL_TALENT
#if VERSION_STRING >= TBC
                    && aurEff->getAuraEffectType() != SPELL_AURA_MOD_ALL_WEAPON_SKILLS
#endif
                    )
                    continue;

                const auto effType = aurEff->getAuraEffectType();
                if (aurEff->getEffectMiscValue() == skillLine)
                    aura->applyModifiers(true, effType);
            }
        }

        if (!noSpellLearning)
            learnSkillSpells(skillLine, curVal);

#ifdef FT_ACHIEVEMENTS
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, skillLine, skillStep, 0);
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, skillLine, currentValue, 0);
#endif
    };

    auto itr = m_skills.find(skillLine);
    if (itr != m_skills.end())
    {
        _verifySkillValues(skillEntry, &currentValue, &maxValue, &skillStep);

        if (!((currentValue > itr->second.CurrentValue && maxValue >= itr->second.MaximumValue) || (currentValue == itr->second.CurrentValue && maxValue > itr->second.MaximumValue)))
            return;

        itr->second.CurrentValue = currentValue;
        itr->second.MaximumValue = maxValue;

        // Update skill fields
        _updateSkillFieldOnValueChange(itr->second.FieldPosition, skillStep, itr->second.CurrentValue, itr->second.MaximumValue);

        if (itr->second.CurrentValue > 0)
            onLearnedNewSkill(itr->second.CurrentValue, skillStep, itr->second.Skill->type == SKILL_TYPE_PROFESSION);
    }
    else
    {
        // Find a skill field position for skill
        auto foundPosition = false;
        PlayerSkillFieldPosition fieldPosition;
        for (uint16_t i = 0; i < WOWPLAYER_SKILL_INFO_COUNT; ++i)
        {
#if VERSION_STRING >= Cata
            const uint16_t field = i / 2;
            const uint8_t offset = i & 1;
            if (getSkillInfoId(field, offset) == 0)
            {
                fieldPosition.field = field;
                fieldPosition.offset = offset;
                foundPosition = true;
                break;
            }
#else
            if (getSkillInfoId(i) == 0)
            {
                fieldPosition.index = i;
                foundPosition = true;
                break;
            }
#endif
        }

        if (!foundPosition)
        {
            sLogger.failure("Player::addSkillLine : Could not add skill line {} to player (guid {}), skill fields are full!", skillLine, getGuidLow());
            return;
        }

        PlayerSkill newSkill;
        newSkill.Skill = skillEntry;
        newSkill.CurrentValue = currentValue;
        newSkill.MaximumValue = maxValue;
        newSkill.FieldPosition = fieldPosition;

        if (!initializeProfession)
            _verifySkillValues(skillEntry, &newSkill.CurrentValue, &newSkill.MaximumValue, &skillStep);

        m_skills.insert(std::make_pair(skillLine, newSkill));

        // Update skill fields
#if VERSION_STRING < Cata
        // field 0
        setSkillInfoId(fieldPosition.index, skillLine);
        setSkillInfoStep(fieldPosition.index, skillStep);
        // field 1
        setSkillInfoCurrentValue(fieldPosition.index, newSkill.CurrentValue);
        setSkillInfoMaxValue(fieldPosition.index, newSkill.MaximumValue);
#else
        // field 0
        setSkillInfoId(fieldPosition.field, fieldPosition.offset, skillLine);
        setSkillInfoStep(fieldPosition.field, fieldPosition.offset, skillStep);
        // field 1
        setSkillInfoCurrentValue(fieldPosition.field, fieldPosition.offset, newSkill.CurrentValue);
        setSkillInfoMaxValue(fieldPosition.field, fieldPosition.offset, newSkill.MaximumValue);
#endif
        // field 2
        _updateSkillBonusFields(fieldPosition, 0, 0);

        if (newSkill.CurrentValue > 0)
            onLearnedNewSkill(newSkill.CurrentValue, skillStep, newSkill.Skill->type == SKILL_TYPE_PROFESSION);
    }
}

bool Player::hasSkillLine(uint16_t skillLine, bool strict/* = false*/) const
{
    if (skillLine == 0)
        return false;

    const auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end())
        return false;

    // Skip initialized only skills
    if (itr->second.CurrentValue == 0 && !strict)
        return false;

    return true;
}

uint16_t Player::getSkillLineCurrent(uint16_t skillLine, bool includeBonus/* = true*/) const
{
    if (skillLine == 0)
        return 0;

    const auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end())
        return 0;

    if (!includeBonus)
        return itr->second.CurrentValue;

    const auto result = static_cast<int32_t>(itr->second.CurrentValue) + itr->second.PermanentBonusValue + itr->second.TemporaryBonusValue;
    return result < 0 ? 0U : static_cast<uint16_t>(result);
}

uint16_t Player::getSkillLineMax(uint16_t skillLine) const
{
    if (skillLine == 0)
        return 0;

    const auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end())
        return 0;

    return itr->second.MaximumValue;
}

void Player::learnInitialSkills()
{
    for (const auto& skill : m_playerCreateInfo->skills)
    {
        if (skill.skillid == 0)
            continue;

        const auto skillLine = sSkillLineStore.lookupEntry(skill.skillid);
        if (skillLine == nullptr)
            continue;

        // Set current skill values for Death Knight's weapon skills
        auto curVal = skill.currentval;
        if (isClassDeathKnight() && skillLine->type == SKILL_TYPE_WEAPON && skillLine->id != SKILL_DUAL_WIELD)
            curVal = static_cast<uint16_t>((std::min(55U, getLevel()) - 1) * 5);

        addSkillLine(skill.skillid, curVal, 0);
    }
}

void Player::learnSkillSpells(uint16_t skillLine, uint16_t currentValue)
{
    const auto raceMask = getRaceMask();
    const auto classMask = getClassMask();

    const auto skillRange = sSpellMgr.getSkillEntryRangeForSkill(skillLine);
    for (const auto& [_, skillEntry] : skillRange)
    {
        if (skillEntry->acquireMethod != 1 && skillEntry->acquireMethod != 2)
            continue;

        // Check race mask
        if (skillEntry->race_mask != 0 && !(skillEntry->race_mask & raceMask))
            continue;

        // Check class mask
        if (skillEntry->class_mask != 0 && !(skillEntry->class_mask & classMask))
            continue;

        // Check skill value
        if (currentValue < skillEntry->minSkillLineRank)
            continue;

        // Add automatically acquired spells
        addSpell(skillEntry->spell, skillLine);
    }
}

void Player::modifySkillBonus(uint16_t skillLine, int16_t amount, bool permanentBonus)
{
    if (skillLine == 0)
        return;

    auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end() || itr->second.CurrentValue == 0)
        return;

    if (permanentBonus)
        itr->second.PermanentBonusValue += amount;
    else
        itr->second.TemporaryBonusValue += amount;

    // Bonuses can be negative but client still wants them in unsigned
    _updateSkillBonusFields(itr->second.FieldPosition, static_cast<uint16_t>(itr->second.TemporaryBonusValue), static_cast<uint16_t>(itr->second.PermanentBonusValue));
}

void Player::modifySkillMaximum(uint16_t skillLine, uint16_t maxValue)
{
    if (skillLine == 0)
        return;

    auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end())
        return;

    const auto oldCurValue = itr->second.CurrentValue;
    const auto oldMaxValue = itr->second.MaximumValue;
    itr->second.MaximumValue = maxValue;

    auto valuesChanged = false;
    uint16_t skillStep = 0;

    _verifySkillValues(itr->second.Skill, &itr->second.CurrentValue, &itr->second.MaximumValue, &skillStep, &valuesChanged);

    if (oldMaxValue != itr->second.MaximumValue || valuesChanged)
    {
        // Update skill fields
#if VERSION_STRING >= Cata
        if (itr->second.Skill->type != SKILL_TYPE_WEAPON)
#endif
        {
            _updateSkillFieldOnValueChange(itr->second.FieldPosition, skillStep, itr->second.CurrentValue, itr->second.MaximumValue);
        }

#ifdef FT_ACHIEVEMENTS
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, skillLine, skillStep, 0);
#endif

        // Current skill value did not change
        if (oldCurValue == itr->second.CurrentValue)
            return;

        sHookInterface.OnAdvanceSkillLine(this, skillLine, itr->second.CurrentValue);

#ifdef FT_ACHIEVEMENTS
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, skillLine, itr->second.CurrentValue, 0);
#endif

        learnSkillSpells(skillLine, itr->second.CurrentValue);
    }
}

void Player::removeSkillLine(uint16_t skillLine)
{
    if (skillLine == 0)
        return;

    auto itr = m_skills.find(skillLine);
    if (itr == m_skills.end())
        return;

    const auto fieldPosition = itr->second.FieldPosition;

    if (itr->second.Skill->type == SKILL_TYPE_PROFESSION)
        modFreePrimaryProfessionPoints(1);

#if VERSION_STRING >= Cata
    // Clear profession skill line
    if (getProfessionSkillLine(0) == itr->second.Skill->id)
        setProfessionSkillLine(0, 0);
    else if (getProfessionSkillLine(1) == itr->second.Skill->id)
        setProfessionSkillLine(1, 0);

    // Since cata, profession skills are never actually removed, they are only "deactivated"
    if (itr->second.Skill->type == SKILL_TYPE_PROFESSION || itr->second.Skill->type == SKILL_TYPE_SECONDARY)
    {
        itr->second.MaximumValue = 75;
        itr->second.CurrentValue = 0;
        itr->second.TemporaryBonusValue = 0;
        itr->second.PermanentBonusValue = 0;
    }
    else
#endif
    {
        m_skills.erase(itr);
    }

    // Update skill fields
#if VERSION_STRING < Cata
    // field 0
    setSkillInfoId(fieldPosition.index, 0);
    setSkillInfoStep(fieldPosition.index, 0);
    // field 1
    setSkillInfoCurrentValue(fieldPosition.index, 0);
    setSkillInfoMaxValue(fieldPosition.index, 0);
#else
    // field 0
    setSkillInfoStep(fieldPosition.field, fieldPosition.offset, 0);
    // field 1
    setSkillInfoCurrentValue(fieldPosition.field, fieldPosition.offset, 0);
    setSkillInfoMaxValue(fieldPosition.field, fieldPosition.offset, 0);
#endif
    // field 2
    _updateSkillBonusFields(fieldPosition, 0, 0);

    // Deactivate passive skill bonus auras
    for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_SKILL))
    {
        if (aurEff->getEffectMiscValue() == skillLine)
        {
            auto modifiableAurEff = aurEff->getAura()->getModifiableAuraEffect(aurEff->getEffectIndex());
            modifiableAurEff->setEffectActive(false);
        }
    }

    // Deactivate passive skill bonus auras
    for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_SKILL_TALENT))
    {
        if (aurEff->getEffectMiscValue() == skillLine)
        {
            auto modifiableAurEff = aurEff->getAura()->getModifiableAuraEffect(aurEff->getEffectIndex());
            modifiableAurEff->setEffectActive(false);
        }
    }

    // Remove skill spells
    removeSkillSpells(skillLine);
}

void Player::removeSkillSpells(uint16_t skillLine)
{
    const auto skillRange = sSpellMgr.getSkillEntryRangeForSkill(skillLine);
    for (const auto& [_, skillEntry] : skillRange)
    {
        // Check also from deleted spells
        if (!removeSpell(skillEntry->spell, false))
            removeDeletedSpell(skillEntry->spell);
    }
}

void Player::removeAllSkills()
{
    for (auto itr = m_skills.cbegin(); itr != m_skills.cend();)
    {
        const auto itr2 = itr;
        ++itr;

        // Skill is not necessarily erased from skill map
        removeSkillLine(itr2->first);
    }
}

void Player::updateSkillMaximumValues()
{
    for (auto& itr : m_skills)
    {
        // Skip initialized only values
        if (itr.second.CurrentValue == 0)
            continue;

        auto valuesChanged = false;
        uint16_t skillStep = 0;

        _verifySkillValues(itr.second.Skill, &itr.second.CurrentValue, &itr.second.MaximumValue, &skillStep, &valuesChanged);

        // Update skill fields
#if VERSION_STRING < Cata
        if (valuesChanged)
#else
        if (valuesChanged && itr.second.Skill->type != SKILL_TYPE_WEAPON)
#endif
            _updateSkillFieldOnValueChange(itr.second.FieldPosition, skillStep, itr.second.CurrentValue, itr.second.MaximumValue);
    }
}

float Player::getSkillUpChance(uint16_t id)
{
    SkillMap::iterator itr = m_skills.find(id);
    if (itr == m_skills.end())
        return 0.0f;

    return itr->second.GetSkillUpChance();
}

#if VERSION_STRING >= Cata
void Player::setInitialPlayerProfessions()
{
    // Since cata player must have profession skills initialized even if the player does not have them
#if VERSION_STRING == Cata
    for (uint16_t skillId = SKILL_FROST; skillId != SKILL_PET_HYDRA; ++skillId)
#elif VERSION_STRING == Mop
    for (uint16_t skillId = SKILL_SWORDS; skillId != SKILL_DIREHORN; ++skillId)
#endif
    {
        const auto skillLine = sSkillLineStore.lookupEntry(skillId);
        if (skillLine == nullptr)
            continue;

        if (skillLine->type != SKILL_TYPE_PROFESSION && skillLine->type != SKILL_TYPE_SECONDARY)
            continue;

        if (!hasSkillLine(skillId, true))
            addSkillLine(skillId, 0, 0, false, true);
    }
}
#endif

uint32_t Player::getArmorProficiency() const
{
    return armorProficiency;
}

void Player::addArmorProficiency(uint32_t proficiency)
{
    armorProficiency |= proficiency;
}

void Player::removeArmorProficiency(uint32_t proficiency)
{
    armorProficiency &= ~proficiency;
}

uint32_t Player::getWeaponProficiency() const
{
    return weaponProficiency;
}

void Player::addWeaponProficiency(uint32_t proficiency)
{
    weaponProficiency |= proficiency;
}

void Player::removeWeaponProficiency(uint32_t proficiency)
{
    weaponProficiency &= ~proficiency;
}

void Player::applyItemProficienciesFromSpell(SpellInfo const* spellInfo, bool apply)
{
    if (spellInfo == nullptr)
        return;

    uint16_t skillId = 0;
    const auto skill_line_ability = sSpellMgr.getFirstSkillEntryForSpell(spellInfo->getId());
    if (skill_line_ability != nullptr)
        skillId = static_cast<uint16_t>(skill_line_ability->skilline);

    const auto skill_line = sSkillLineStore.lookupEntry(skillId);
    if (skill_line == nullptr)
        return;

    const auto subclass = spellInfo->getEquippedItemSubClass();
    if (apply)
    {
        // Add the skill to player if player does not have it
        // addSkillLine will set correct maximum skill value
        if (!hasSkillLine(skillId))
            addSkillLine(skillId, 1, 0);

        if (spellInfo->getEquippedItemClass() == ITEM_CLASS_ARMOR && !(getArmorProficiency() & subclass))
        {
            addArmorProficiency(subclass);
            sendSetProficiencyPacket(ITEM_CLASS_ARMOR, getArmorProficiency());
        }
        else if (spellInfo->getEquippedItemClass() == ITEM_CLASS_WEAPON && !(getWeaponProficiency() & subclass))
        {
            addWeaponProficiency(subclass);
            sendSetProficiencyPacket(ITEM_CLASS_WEAPON, getWeaponProficiency());
        }
    }
    else
    {
        if (hasSkillLine(skillId))
            removeSkillLine(skillId);

        if (spellInfo->getEquippedItemClass() == ITEM_CLASS_ARMOR && getArmorProficiency() & subclass)
        {
            removeArmorProficiency(subclass);
            sendSetProficiencyPacket(ITEM_CLASS_ARMOR, getArmorProficiency());
        }
        else if (spellInfo->getEquippedItemClass() == ITEM_CLASS_WEAPON && getWeaponProficiency() & subclass)
        {
            removeWeaponProficiency(subclass);
            sendSetProficiencyPacket(ITEM_CLASS_WEAPON, getWeaponProficiency());
        }
    }
}

#ifdef FT_GLYPHS
void Player::updateGlyphs()
{
#if VERSION_STRING == WotLK
    for (uint32_t i = 0; i < sGlyphSlotStore.getNumRows(); ++i)
    {
        const auto glyphSlot = sGlyphSlotStore.lookupEntry(i);
        if (glyphSlot == nullptr)
            continue;

        if (glyphSlot->Slot > 0)
            setGlyphSlot(static_cast<uint16_t>(glyphSlot->Slot - 1), glyphSlot->Id);
    }
#else
    uint16_t slot = 0;
    for (uint32_t i = 0; i < sGlyphSlotStore.getNumRows(); ++i)
    {
        const auto glyphSlot = sGlyphSlotStore.lookupEntry(i);
        if (glyphSlot != nullptr)
            setGlyphSlot(slot++, glyphSlot->Id);
    }
#endif

    const auto level = getLevel();
    uint32_t slotMask = 0;

#if VERSION_STRING == WotLK
    if (level >= 15)
        slotMask |= (GS_MASK_1 | GS_MASK_2);
    if (level >= 30)
        slotMask |= GS_MASK_3;
    if (level >= 50)
        slotMask |= GS_MASK_4;
    if (level >= 70)
        slotMask |= GS_MASK_5;
    if (level >= 80)
        slotMask |= GS_MASK_6;
#elif VERSION_STRING == Cata
    if (level >= 25)
        slotMask |= GS_MASK_LEVEL_25;
    if (level >= 50)
        slotMask |= GS_MASK_LEVEL_50;
    if (level >= 75)
        slotMask |= GS_MASK_LEVEL_75;
#elif VERSION_STRING == Mop
    // TODO
#endif

    setGlyphsEnabled(slotMask);
}
#endif

uint64_t Player::getComboPointTarget() const
{
    return m_comboTarget;
}

int8_t Player::getComboPoints() const
{
    return m_comboPoints;
}

void Player::addComboPoints(uint64_t targetGuid, int8_t points)
{
    // Remove combo point retain auras
    // This will not clear points created by retain aura, remove code checks for duration
    if (points > 0)
        removeAllAurasByAuraEffect(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (getComboPointTarget() == targetGuid)
    {
        m_comboPoints += points;
    }
    else
    {
        // Clear points when switching combo target
        m_comboTarget = targetGuid;
        m_comboPoints = points;
    }

    updateComboPoints();
}

void Player::updateComboPoints()
{
    if (getComboPoints() > 5)
        m_comboPoints = 5;

    if (getComboPoints() < 0)
        m_comboPoints = 0;

    // todo: I think there should be a better way to do this, copypasting from legacy method now -Appled
    unsigned char buffer[10];
    uint16_t length = 2;

    if (getComboPointTarget() != 0)
    {
        const auto* const target = getWorldMapUnit(getComboPointTarget());
        if (target == nullptr || target->isDead() || getTargetGuid() != getComboPointTarget())
        {
            buffer[0] = buffer[1] = 0;
        }
        else
        {
            length = static_cast<uint16_t>(FastGUIDPack(getComboPointTarget(), buffer, 0));
            buffer[length++] = getComboPoints();
        }
    }
    else
    {
        buffer[0] = buffer[1] = 0;
    }

    m_session->OutPacket(SMSG_UPDATE_COMBO_POINTS, length, buffer);
}

void Player::clearComboPoints()
{
    m_comboTarget = 0;
    m_comboPoints = 0;

    // Remove combo point retain auras when combo points have been used
    removeAllAurasByAuraEffect(SPELL_AURA_RETAIN_COMBO_POINTS);

    updateComboPoints();
}

void Player::_addSpell(uint32_t spellId, uint16_t fromSkill/* = 0*/, bool learningPreviousRanks/* = false*/, bool ignorePreviousRanks/* = false*/)
{
    const auto* spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    if (sSpellMgr.isSpellDisabled(spellId))
        return;

    // Check if player already knows this spell
    if (hasSpell(spellId))
        return;

    if (spellInfo->hasSpellRanks())
    {
        if (!learningPreviousRanks)
        {
            // Check if player has at one point known a higher rank of this spell
            const auto* higherRankInfo = spellInfo->getRankInfo()->getLastSpell();
            const auto isSingleRankAbility = spellInfo->canKnowOnlySingleRank();
            do
            {
                // If player can know only one rank of this spell rank chain, try find a existing higher rank
                // Possible lower ranks are removed when a higher rank is added to spell map
                if (isSingleRankAbility && hasSpell(higherRankInfo->getId()))
                    return;

                if (removeDeletedSpell(higherRankInfo->getId()))
                {
                    // Possibly found a deleted higher rank, add it to player instead
                    break;
                }

                if (higherRankInfo->getId() == spellId)
                    break;

                higherRankInfo = higherRankInfo->getRankInfo()->getPreviousSpell();
            } while (higherRankInfo != nullptr);

            if (higherRankInfo != nullptr)
                spellInfo = higherRankInfo;
        }
        else
        {
            // When learning previous ranks or talents make sure they are also deleted from deleted spells
            removeDeletedSpell(spellInfo->getId());
        }

        if (!ignorePreviousRanks && !spellInfo->isTalent() && !spellInfo->canKnowOnlySingleRank())
        {
            // Add all previous ranks to player
            if (const auto* const previousSpell = spellInfo->getRankInfo()->getPreviousSpell())
                _addSpell(previousSpell->getId(), fromSkill, true);
        }
    }
    else
    {
        // Check if spell was deleted from player
        removeDeletedSpell(spellId);
    }

    uint32_t supercededSpellId = 0;
    if (spellInfo->hasSpellRanks() && (spellInfo->canKnowOnlySingleRank() || spellInfo->isTalent()))
    {
        // If spell can have only one rank known move all previous ranks to deleted spells
        const auto* previousRank = spellInfo->getRankInfo()->getPreviousSpell();
        const auto moveToDeleted = !spellInfo->isTalent();
        const auto silently = !spellInfo->isTalent();
        while (previousRank != nullptr)
        {
            if (_removeSpell(previousRank->getId(), moveToDeleted, silently, true))
            {
                if (!spellInfo->isTalent())
                    supercededSpellId = previousRank->getId();
                break;
            }

            previousRank = previousRank->getRankInfo()->getPreviousSpell();
        }
    }

    m_spellSet.emplace(spellInfo->getId());

    if (IsInWorld())
    {
        if (!ignorePreviousRanks)
        {
            // If previous rank was found overwrite it in client with smsg_superceded packet
            if (supercededSpellId > 0)
                getSession()->SendPacket(SmsgSupercededSpell(supercededSpellId, spellInfo->getId()).serialise().get());
            else
                getSession()->SendPacket(SmsgLearnedSpell(spellInfo->getId()).serialise().get());
        }

        // Cast talents and auto castable spells with learn spell effect
        if ((spellInfo->isTalent() || spellInfo->getAttributesEx() & ATTRIBUTESEX_AUTOCASTED_AT_SPELL_LEARN) && spellInfo->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        {
            castSpell(getGuid(), spellInfo, true);
        }
        // Cast passive spells only if player has proper shapeshift form
        else if (spellInfo->isPassive())
        {
            if (spellInfo->getRequiredShapeShift() == 0 ||
                (getShapeShiftMask() != 0 && (spellInfo->getRequiredShapeShift() & getShapeShiftMask())) ||
                (getShapeShiftMask() == 0 && (spellInfo->getAttributesExB() & ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT)))
            {
                // TODO: temporarily check for this custom flag, will be removed when spell system handles pets properly!
                if (((spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) == 0) || (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET && getPet() != nullptr))
                    castSpell(getGuid(), spellInfo, true);
            }
        }
    }

    // Add spell's skill line to player
    if (fromSkill == 0)
    {
        const auto teachesProfession = spellInfo->hasEffect(SPELL_EFFECT_SKILL) || spellInfo->hasEffect(SPELL_EFFECT_TRADE_SKILL);
        const auto raceMask = getRaceMask();
        const auto classMask = getClassMask();

        const auto spellSkillRange = sSpellMgr.getSkillEntryRangeForSpell(spellId);
        for (const auto& [_, skillEntry] : spellSkillRange)
        {
            if (skillEntry->race_mask > 0 && !(skillEntry->race_mask & raceMask))
                continue;

            if (skillEntry->class_mask > 0 && !(skillEntry->class_mask & classMask))
                continue;

            const auto skillLine = static_cast<uint16_t>(skillEntry->skilline);
            if (hasSkillLine(skillLine))
                continue;

            // Do not learn skill default spells if spell does not teach profession skills
            // This allows to make starting spells fully customizable
            // If skill default spells would be taught, then all default starting spells from DBC files are taught on first login
            addSkillLine(skillLine, 1, 0, !teachesProfession);
        }
    }

#ifdef FT_ACHIEVEMENTS
    if (!IsInWorld())
        return;

    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL, spellId, 1, 0);
    if (spellInfo->getMechanicsType() == MECHANIC_MOUNTED) // Mounts
    {
        // miscvalue1==777 for mounts, 778 for pets
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS, 777, 0, 0);
    }
    else if (spellInfo->getEffect(0) == SPELL_EFFECT_SUMMON) // Companion pet?
    {
        // miscvalue1==777 for mounts, 778 for pets
        // make sure it's a companion pet, not some other summon-type spell
        if (const auto summonProperties = sSummonPropertiesStore.lookupEntry(spellInfo->getEffectMiscValueB(0)))
        {
            if (summonProperties->Slot == 5 || (summonProperties->Type == SUMMON_TYPE_COMPANION && summonProperties->Slot != 6))
                updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS, 778, 0, 0);
        }
    }
#endif
}

bool Player::_removeSpell(uint32_t spellId, bool moveToDeleted, bool silently/* = false*/, bool removingPreviousRank/* = false*/, bool forceRemoveHigherRanks/* = false*/)
{
    const auto itr = std::as_const(m_spellSet).find(spellId);
    if (itr == m_spellSet.cend())
    {
        // When resetting talents if player can know single rank of this spell and the first rank is a talent,
        // other ranks are never removed since player does not have first rank active anymore
        if (forceRemoveHigherRanks)
        {
            const auto* const spellInfo = sSpellMgr.getSpellInfo(spellId);
            if (spellInfo == nullptr || !spellInfo->hasSpellRanks() || !spellInfo->canKnowOnlySingleRank())
                return false;

            const auto* higherRankInfo = spellInfo->getRankInfo()->getNextSpell();
            while (higherRankInfo != nullptr)
            {
                if (_removeSpell(higherRankInfo->getId(), true, silently, true))
                {
                    // Removed a higher ranked spell from single rank chain, safe to exit
                    return true;
                }

                higherRankInfo = higherRankInfo->getRankInfo()->getNextSpell();
            }
        }

        return false;
    }

    m_spellSet.erase(itr);
    removeAllAurasByIdForGuid(spellId, getGuid());

    if (moveToDeleted)
        m_deletedSpellSet.emplace(spellId);

    const auto* const spellInfo = sSpellMgr.getSpellInfo(spellId);
    auto activatedPreviousRank = false;
    if (spellInfo->hasSpellRanks())
    {
        // If player can know single rank of this spell rank chain, activate previous rank in spell map
        if (!removingPreviousRank && spellInfo->canKnowOnlySingleRank())
        {
            const auto* previousSpell = spellInfo->getRankInfo()->getPreviousSpell();
            while (previousSpell != nullptr)
            {
                if (hasDeletedSpell(previousSpell->getId()))
                {
                    _addSpell(previousSpell->getId(), 0, true, true);
                    activatedPreviousRank = true;
                    break;
                }
                previousSpell = previousSpell->getRankInfo()->getPreviousSpell();
            }

            if (IsInWorld() && !silently && activatedPreviousRank)
                getSession()->SendPacket(SmsgSupercededSpell(spellId, previousSpell->getId()).serialise().get());
        }
    }

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        const auto spellEff = spellInfo->getEffect(i);
        if (spellEff == SPELL_EFFECT_NULL)
            continue;

        switch (spellEff)
        {
            case SPELL_EFFECT_LEARN_SPELL:
                // If spell teaches another spell, remove it recursively as well
                if (const auto taughtSpellId = spellInfo->getEffectTriggerSpell(i))
                    _removeSpell(taughtSpellId, false, false, true);
                break;
            case SPELL_EFFECT_DUAL_WIELD:
                setDualWield(false);
                break;
            case SPELL_EFFECT_PROFICIENCY:
                applyItemProficienciesFromSpell(spellInfo, false);
                break;
            case SPELL_EFFECT_TRIGGER_SPELL:
                if (const auto triggerSpellId = spellInfo->getEffectTriggerSpell(i))
                    removeAllAurasByIdForGuid(triggerSpellId, getGuid());
                break;
#if VERSION_STRING >= WotLK
            case SPELL_EFFECT_DUAL_WIELD_2H:
                setDualWield2H(false);
                break;
#endif
            default:
                break;
        }
    }

    if (IsInWorld() && !silently && !activatedPreviousRank)
        getSession()->SendPacket(SmsgRemovedSpell(spellId).serialise().get());

    if (spellInfo->hasSpellRanks())
    {
        // Remove higher ranks from spell map as well
        if (const auto* const nextSpell = spellInfo->getRankInfo()->getNextSpell())
            _removeSpell(nextSpell->getId(), true, false, true);
    }

    return true;
}


void Player::_verifySkillValues(WDB::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep, bool* requireUpdate)
{
    auto level_bound_skill = skillEntry->type == SKILL_TYPE_WEAPON && skillEntry->id != SKILL_DUAL_WIELD;
#if VERSION_STRING <= WotLK
    level_bound_skill = level_bound_skill || skillEntry->id == SKILL_LOCKPICKING;
#endif
#if VERSION_STRING <= TBC
    level_bound_skill = level_bound_skill || skillEntry->id == SKILL_POISONS;
#endif

    uint16_t newMaximum = 0;
    auto isCurrentValueMaxed = false;
    *requireUpdate = false;

    if (level_bound_skill)
    {
        newMaximum = static_cast<uint16_t>(5 * getLevel());
#if VERSION_STRING >= Cata
        // In cata all weapon skills are always maxed
        isCurrentValueMaxed = true;
#endif
    }
    else if (skillEntry->type == SKILL_TYPE_LANGUAGE)
    {
        newMaximum = 300;
        isCurrentValueMaxed = true;
    }
    else if (skillEntry->type == SKILL_TYPE_PROFESSION || skillEntry->type == SKILL_TYPE_SECONDARY)
    {
        newMaximum = *maxValue;

        if (newMaximum == 0)
        {
            newMaximum = 75;
            while (newMaximum < *currentValue && newMaximum < DBC_PLAYER_SKILL_MAX)
            {
                newMaximum += 75;
            }
        }

        if (skillEntry->id == SKILL_RIDING)
            isCurrentValueMaxed = true;
    }
    else
    {
        newMaximum = 1;
        isCurrentValueMaxed = true;
    }

    // force to be within limits
    if (newMaximum > DBC_PLAYER_SKILL_MAX)
        newMaximum = DBC_PLAYER_SKILL_MAX;

    if (*maxValue != newMaximum)
    {
        *requireUpdate = true;
        *maxValue = newMaximum;
    }
    if (*currentValue > newMaximum)
    {
        *requireUpdate = true;
        *currentValue = newMaximum;
    }

    // These are at max value all the time
    if (isCurrentValueMaxed && *currentValue != newMaximum)
    {
        *requireUpdate = true;
        *currentValue = newMaximum;
    }

    if (skillEntry->type == SKILL_TYPE_PROFESSION || skillEntry->type == SKILL_TYPE_SECONDARY)
        *skillStep = *maxValue / 75U;
    else
        *skillStep = 0;
}

void Player::_verifySkillValues(WDB::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep)
{
    auto requireUpdate = false;
    _verifySkillValues(skillEntry, currentValue, maxValue, skillStep, &requireUpdate);
}

void Player::_updateSkillFieldOnValueChange(const PlayerSkillFieldPosition fieldPosition, uint16_t skillStep, uint16_t currentValue, uint16_t maxValue)
{
#if VERSION_STRING < Cata
    // field 0
    setSkillInfoStep(fieldPosition.index, skillStep);
    // field 1
    setSkillInfoCurrentValue(fieldPosition.index, currentValue);
    setSkillInfoMaxValue(fieldPosition.index, maxValue);
#else
    // field 0
    setSkillInfoStep(fieldPosition.field, fieldPosition.offset, skillStep);
    // field 1
    setSkillInfoCurrentValue(fieldPosition.field, fieldPosition.offset, currentValue);
    setSkillInfoMaxValue(fieldPosition.field, fieldPosition.offset, maxValue);
#endif
}

void Player::_updateSkillBonusFields(const PlayerSkillFieldPosition fieldPosition, uint16_t tempBonus, uint16_t permBonus)
{
#if VERSION_STRING < Cata
    // field 2
    setSkillInfoBonusTemporary(fieldPosition.index, tempBonus);
    setSkillInfoBonusPermanent(fieldPosition.index, permBonus);
#else
    // field 2
    setSkillInfoBonusTemporary(fieldPosition.field, fieldPosition.offset, tempBonus);
    setSkillInfoBonusPermanent(fieldPosition.field, fieldPosition.offset, permBonus);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Talents
void Player::learnTalent(uint32_t talentId, uint32_t talentRank)
{
    auto curTalentPoints = getActiveSpec().getTalentPoints();
    if (curTalentPoints == 0)
        return;

    if (talentRank > 4)
        return;

    auto talentInfo = sTalentStore.lookupEntry(talentId);
    if (talentInfo == nullptr)
        return;
#if VERSION_STRING < Mop
    if (sSpellMgr.isSpellDisabled(talentInfo->RankID[talentRank]))
    {
        if (IsInWorld())
            sendCastFailedPacket(talentInfo->RankID[talentRank], SPELL_FAILED_SPELL_UNAVAILABLE, 0, 0);
        return;
    }

    // Check if player already has the talent with same or higher rank
    for (auto i = talentRank; i <= 4; ++i)
    {
        if (talentInfo->RankID[i] != 0 && hasSpell(talentInfo->RankID[i]))
            return;
    }

    // Check if talent tree is for player's class
    auto talentTreeInfo = sTalentTabStore.lookupEntry(talentInfo->TalentTree);
    if (talentTreeInfo == nullptr || !(getClassMask() & talentTreeInfo->ClassMask))
        return;

#if VERSION_STRING >= Cata
    // Check if enough talent points are spent in the primary talent tree before unlocking other trees
    if (talentInfo->TalentTree != m_FirstTalentTreeLock && m_FirstTalentTreeLock != 0)
    {
        auto pointsUsed = 0;
        for (const auto& [talentId, rank] : getActiveSpec().getTalents())
        {
            pointsUsed += rank + 1;
        }

        // You need to spent 31 points in the primary tree before you're able to unlock other trees
        if (pointsUsed < 31)
            return;
    }
#endif

    // Check if talent requires another talent
    if (talentInfo->DependsOn > 0)
    {
        auto dependsOnTalent = sTalentStore.lookupEntry(talentInfo->DependsOn);
        if (dependsOnTalent != nullptr)
        {
            auto hasEnoughRank = false;
            for (auto i = 0; i <= 4; ++i)
            {
                if (dependsOnTalent->RankID[i] != 0)
                {
                    if (hasSpell(dependsOnTalent->RankID[i]))
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

    auto spellId = talentInfo->RankID[talentRank];
    if (spellId == 0)
    {
        sLogger.info("Player::learnTalent: Player tried to learn talent {} (rank {}) but talent's spell id is 0.", talentId, talentRank);
        return;
    }

    // Check can player yet access this talent
    uint32_t spentPoints = 0;
    if (talentInfo->Row > 0)
    {
        // Loop through player's talents
        for (const auto& [talent, rank] : getActiveSpec().getTalents())
        {
            auto tmpTalent = sTalentStore.lookupEntry(talent);
            if (tmpTalent == nullptr)
                continue;
            // Skip talents from other trees
            if (tmpTalent->TalentTree != talentInfo->TalentTree)
                continue;
            spentPoints += rank + 1;
        }
    }

    if (spentPoints < (talentInfo->Row * 5))
        return;

    // Get current talent rank
    uint8_t curTalentRank = 0;
    auto isMultiRankTalent = false;
    for (int8_t _talentRank = 4; _talentRank >= 0; --_talentRank)
    {
        if (talentInfo->RankID[_talentRank] != 0)
        {
            if (_talentRank > 0)
                isMultiRankTalent = true;

            if (hasSpell(talentInfo->RankID[_talentRank]))
            {
                curTalentRank = _talentRank + 1;
                break;
            }
        }
    }

    // Check does player have enough talent points
    auto requiredTalentPoints = (talentRank + 1) - curTalentRank;
    if (curTalentPoints < requiredTalentPoints)
        return;

    // Check if player already knows this or higher rank
    if (curTalentRank >= (talentRank + 1))
        return;

    // Check if player already has the talent spell
    if (hasSpell(spellId))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    _addSpell(spellId, 0, isMultiRankTalent);

#if VERSION_STRING >= Cata
    // Set primary talent tree and lock others
    if (m_FirstTalentTreeLock == 0)
    {
        m_FirstTalentTreeLock = talentInfo->TalentTree;
        // TODO: learning Mastery and spec spells
        // also need to handle them in talent reset
    }
#endif

    // Add the new talent to player talent map
    getActiveSpec().addTalent(talentId, static_cast<uint8_t>(talentRank));
    setTalentPoints(curTalentPoints - requiredTalentPoints, false);
#endif
}

void Player::resetTalents()
{
#if VERSION_STRING < Mop
    // Loop through player's talents
    for (const auto& [talentId, rank] : getActiveSpec().getTalents())
    {
        if (const auto* tmpTalent = sTalentStore.lookupEntry(talentId))
            _removeSpell(tmpTalent->RankID[rank], false, false, true, true);
    }

    // Unsummon current pet or set temporarily unsummoned pet offline
    if (getPet() != nullptr)
        getPet()->unSummon();
    else
        setTemporarilyUnsummonedPetsOffline();

    // Check offhand
    unEquipOffHandIfRequired();

    // Clear talents
    getActiveSpec().clearTalents();
#if VERSION_STRING >= Cata
    m_FirstTalentTreeLock = 0;
#endif

    // Reset talent point amount
    setInitialTalentPoints(true);
#endif
}

void Player::resetAllTalents()
{
    if (m_talentSpecsCount == 1)
    {
        resetTalents();
        return;
    }

    const auto activeSpec = m_talentActiveSpec;
    resetTalents();

    if (activeSpec == SPEC_PRIMARY)
        activateTalentSpec(SPEC_SECONDARY);
    else
        activateTalentSpec(SPEC_PRIMARY);

    resetTalents();
    // Change back to the original spec
    activateTalentSpec(activeSpec);
}

void Player::setTalentPoints(uint32_t talentPoints, bool forBothSpecs /*= true*/)
{
    if (!forBothSpecs)
        getActiveSpec().setTalentPoints(talentPoints);
    else
    {
#ifndef FT_DUAL_SPEC
        getActiveSpec().setTalentPoints(talentPoints);
#else
        m_specs[SPEC_PRIMARY].setTalentPoints(talentPoints);
        m_specs[SPEC_SECONDARY].setTalentPoints(talentPoints);
#endif
    }

#if VERSION_STRING < Cata
    // Send talent points also to client
    setFreeTalentPoints(talentPoints);
#endif
}

void Player::addTalentPoints(uint32_t talentPoints, bool forBothSpecs /*= true*/)
{
    if (!forBothSpecs)
        setTalentPoints(getActiveSpec().getTalentPoints() + talentPoints);
    else
    {
#ifndef FT_DUAL_SPEC
        setTalentPoints(getActiveSpec().getTalentPoints() + talentPoints);
#else
        m_specs[SPEC_PRIMARY].setTalentPoints(m_specs[SPEC_PRIMARY].getTalentPoints() + talentPoints);
        m_specs[SPEC_SECONDARY].setTalentPoints(m_specs[SPEC_SECONDARY].getTalentPoints() + talentPoints);

#if VERSION_STRING < Cata
        setFreeTalentPoints(getFreeTalentPoints() + talentPoints);
#endif
#endif
    }
}

void Player::setInitialTalentPoints(bool talentsResetted /*= false*/)
{
    if (getLevel() < 10)
    {
        setTalentPoints(0);
        return;
    }

    // Calculate initial talent points based on level
    uint32_t talentPoints = 0;
#if VERSION_STRING >= Cata
    auto talentPointsAtLevel = sNumTalentsAtLevel.lookupEntry(getLevel());
    if (talentPointsAtLevel != nullptr)
        talentPoints = static_cast<uint32_t>(talentPointsAtLevel->talentPoints);
#else
    talentPoints = getLevel() - 9;
#endif

#ifdef FT_DEATH_KNIGHT
    if (getClass() == DEATHKNIGHT)
    {
        if (GetMapId() == 609)
        {
            // If Death Knight is in the instanced Ebon Hold (map 609), talent points are calculated differently,
            // because Death Knights receive their talent points from their starting quest chain.
            // However if Death Knight is not in the instanced Ebon Hold, it is safe to assume that
            // the player has completed the DK starting quest chain and normal calculation can be used.
            uint32_t dkTalentPoints = 0;
#if VERSION_STRING >= Cata
            auto dkBaseTalentPoints = sNumTalentsAtLevel.lookupEntry(55);
            if (dkBaseTalentPoints != nullptr)
                dkTalentPoints = getLevel() < 55 ? 0 : talentPoints - static_cast<uint32_t>(dkBaseTalentPoints->talentPoints);
#else
            dkTalentPoints = getLevel() < 55 ? 0 : getLevel() - 55;
#endif
            // Add talent points from quests
            dkTalentPoints += m_talentPointsFromQuests;

            if (dkTalentPoints < talentPoints)
                talentPoints = dkTalentPoints;
        }

        // Add extra talent points if any is set in config files
        talentPoints += worldConfig.player.deathKnightStartTalentPoints;
    }
#endif

    // If player's level is increased, player's already spent talent points must be subtracted from initial talent points
    uint32_t usedTalentPoints = 0;
    if (!talentsResetted)
    {
#ifdef FT_DUAL_SPEC
        if (m_talentSpecsCount == 2)
        {
            auto inactiveSpec = m_talentActiveSpec == SPEC_PRIMARY ? SPEC_SECONDARY : SPEC_PRIMARY;
            if (m_specs[inactiveSpec].getTalents().size() > 0)
            {
                uint32_t usedTalentPoints2 = 0;
                for (const auto& [talentId, rank] : m_specs[inactiveSpec].getTalents())
                {
                    usedTalentPoints2 += rank + 1;
                }

                if (usedTalentPoints2 > talentPoints)
                    usedTalentPoints2 = talentPoints;

                m_specs[inactiveSpec].setTalentPoints(talentPoints - usedTalentPoints2);
            }
        }
#endif
        if (getActiveSpec().getTalents().size() > 0)
        {
            for (const auto& [talentId, rank] : getActiveSpec().getTalents())
            {
                usedTalentPoints += rank + 1;
            }

            if (usedTalentPoints > talentPoints)
                usedTalentPoints = talentPoints;
        }
    }

    setTalentPoints(talentPoints - usedTalentPoints, false);
    smsg_TalentsInfo(false);
}

uint32_t Player::getTalentPointsFromQuests() const
{
    return m_talentPointsFromQuests;
}

void Player::setTalentPointsFromQuests(uint32_t talentPoints)
{
    m_talentPointsFromQuests = talentPoints;
}

void Player::smsg_TalentsInfo([[maybe_unused]]bool SendPetTalents)
{
    // TODO: classic and tbc
#if VERSION_STRING < Mop
#if VERSION_STRING >= WotLK
    WorldPacket data(SMSG_UPDATE_TALENT_DATA, 1000);
    data << uint8_t(SendPetTalents ? 1 : 0);
    if (SendPetTalents)
    {
        if (getPet() != nullptr)
            getPet()->SendTalentsToOwner();
        return;
    }
    else
    {
        data << uint32_t(getActiveSpec().getTalentPoints()); // Free talent points
        data << uint8_t(m_talentSpecsCount); // How many specs player has
        data << uint8_t(m_talentActiveSpec); // Which spec is active right now

        if (m_talentSpecsCount > MAX_SPEC_COUNT)
            m_talentSpecsCount = MAX_SPEC_COUNT;

        // Loop through specs
        for (uint8_t specId = 0; specId < m_talentSpecsCount; ++specId)
        {
            PlayerSpec spec = m_specs[specId];

#if VERSION_STRING >= Cata
            // Send primary talent tree
            data << uint32_t(m_FirstTalentTreeLock);
#endif

            // How many talents player has learnt
            data << uint8_t(spec.getTalents().size());
            for (const auto& [talentId, rank] : spec.getTalents())
            {
                data << uint32_t(talentId);
                data << uint8_t(rank);
            }

            // What kind of glyphs player has
            data << uint8_t(GLYPHS_COUNT);
            for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
            {
                data << uint16_t(getGlyph(specId, i));
            }
        }
    }
    getSession()->SendPacket(&data);
#endif
#else
    WorldPacket data(SMSG_UPDATE_TALENT_DATA, 50);
    data << uint8_t(m_talentActiveSpec); // Which spec is active right now
    data.writeBits(m_talentSpecsCount, 19);

    auto wpos = std::make_unique<size_t[]>(m_talentSpecsCount);
    for (int i = 0; i < m_talentSpecsCount; ++i)
    {
        wpos[i] = data.bitwpos();
        data.writeBits(0, 23);
    }

    data.flushBits();

    for (auto specId = 0; specId < m_talentSpecsCount; ++specId)
    {
        PlayerSpec spec = m_specs[specId];

        for (uint8_t i = 0; i < 6; ++i)
            data << uint16_t(getGlyph(specId, i));

        int32_t talentCount = 0;
        for (const auto& [talentId, rank] : spec.getTalents())
        {
            data << uint16_t(talentId);
            talentCount++;
        }
        data.PutBits(wpos[specId], talentCount, 23);
        data << uint32_t(spec.getTalentPoints());
    }

    getSession()->SendPacket(&data);
#endif
}

void Player::activateTalentSpec([[maybe_unused]]uint8_t specId)
{
#if VERSION_STRING < Mop
#ifndef FT_DUAL_SPEC
    return;
#else
    if (specId >= MAX_SPEC_COUNT || m_talentActiveSpec >= MAX_SPEC_COUNT || m_talentActiveSpec == specId)
        return;

    // Dismiss current pet or set temporarily unsummoned pet offline
    if (getPet() != nullptr)
        getPet()->unSummon();
    else
        setTemporarilyUnsummonedPetsOffline();

    // Reset action buttons on client
    sendActionBars(2);

    // Remove old glyphs
    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
    {
        const auto glyphProperties = sGlyphPropertiesStore.lookupEntry(m_specs[m_talentActiveSpec].getGlyph(i));
        if (glyphProperties != nullptr)
            removeAllAurasById(glyphProperties->SpellID);
    }

    // Remove old talents and move them to deleted spells
    for (const auto& [talentId, rank] : m_specs[m_talentActiveSpec].getTalents())
    {
        const auto talentInfo = sTalentStore.lookupEntry(talentId);
        if (talentInfo != nullptr)
            _removeSpell(talentInfo->RankID[rank], true, false, true, true);
    }

    m_talentActiveSpec = specId;

    // Add new glyphs
    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
    {
        const auto glyphProperties = sGlyphPropertiesStore.lookupEntry(m_specs[m_talentActiveSpec].getGlyph(i));
        if (glyphProperties != nullptr)
            castSpell(this, glyphProperties->SpellID, true);
    }

    // Add new talents
    for (const auto& [talentId, rank] : m_specs[m_talentActiveSpec].getTalents())
    {
        const auto talentInfo = sTalentStore.lookupEntry(talentId);
        if (talentInfo == nullptr)
            continue;
        auto isSingleRankTalent = rank == 0;
        if (isSingleRankTalent)
        {
            for (uint8_t talentRank = 1; talentRank < 5; ++talentRank)
            {
                if (talentInfo->RankID[talentRank] != 0)
                {
                    isSingleRankTalent = false;
                    break;
                }
            }
        }
        _addSpell(talentInfo->RankID[rank], 0, !isSingleRankTalent);
    }

    // Set action buttons from new spec
    sendActionBars(1);

    // Reset power
    setPower(getPowerType(), 0);
    sendPowerUpdate(false);

    // Check offhand
    unEquipOffHandIfRequired();

    // Send talent points
    setInitialTalentPoints();
#endif
#endif
}

uint32_t Player::getTalentResetsCount() const { return m_talentResetsCount; }
void Player::setTalentResetsCount(uint32_t value) { m_talentResetsCount = value; }

uint32_t Player::calcTalentResetCost(uint32_t resetnum) const
{
    if (resetnum == 0)
        return  10000;

    if (resetnum > 10)
        return  500000;

    return resetnum * 50000;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tutorials
uint32_t Player::getTutorialValueById(uint8_t id)
{
    if (id < 8)
        return m_tutorials[id];
    return 0;
}

void Player::setTutorialValueForId(uint8_t id, uint32_t value)
{
    if (id < 8)
    {
        m_tutorials[id] = value;
        m_tutorialsDirty = true;
    }
}

void Player::loadTutorials()
{
    if (auto result = CharacterDatabase.Query("SELECT * FROM tutorials WHERE playerId = %u", getGuidLow()))
    {
        auto* const fields = result->Fetch();
        for (uint8_t id = 0; id < 8; ++id)
            m_tutorials[id] = fields[id + 1].asUint32();
    }
    m_tutorialsDirty = false;
}

void Player::saveTutorials()
{
    if (m_tutorialsDirty)
    {
        CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerid = %u;", getGuidLow());
        CharacterDatabase.Execute("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", getGuidLow(), m_tutorials[0], m_tutorials[1], m_tutorials[2], m_tutorials[3], m_tutorials[4], m_tutorials[5], m_tutorials[6], m_tutorials[7]);

        m_tutorialsDirty = false;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Actionbar
void Player::setActionButton(uint8_t button, uint32_t action, uint8_t type, uint8_t misc)
{
    if (button >= PLAYER_ACTION_BUTTON_COUNT)
        return;

    getActiveSpec().getActionButton(button).Action = action;
    getActiveSpec().getActionButton(button).Misc = misc;
    getActiveSpec().getActionButton(button).Type = type;
}

void Player::sendActionBars([[maybe_unused]]uint8_t action)
{
#if VERSION_STRING < Mop
    WorldPacket data(SMSG_UPDATE_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);

#if VERSION_STRING == WotLK
    data << uint8_t(action);
#endif

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        // TODO: this needs investigation
        // action, as in spell id, can be and will be over uint16_t max (65535) on wotlk and cata
        // but if I send action in uint32_t, client ignores the button completely and leaves an empty button slot, or corrupts other slots as well
        // however casting the action to uint16_t seems to somehow work. I tested it with a spell id over 65535.
        // but this is not a solution and can cause undefined behaviour... (previously ActionButton::Action was stored in uint16_t)
        // I believe client accepts at most 4 bytes per button -Appled
        data << uint16_t(getActiveSpec().getActionButton(i).Action);
#if VERSION_STRING < WotLK
        data << getActiveSpec().getActionButton(i).Type;
        data << getActiveSpec().getActionButton(i).Misc;
#else
        // Since Wotlk misc needs to be sent before type
        data << getActiveSpec().getActionButton(i).Misc;
        data << getActiveSpec().getActionButton(i).Type;
#endif
    }
#else
    WorldPacket data(SMSG_UPDATE_ACTION_BUTTONS, (PLAYER_ACTION_BUTTON_COUNT * 8) + 1);

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
#endif

#if VERSION_STRING >= Cata
    data << uint8_t(action);
#endif

    getSession()->SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Auction
void Player::sendAuctionCommandResult(Auction* auction, uint32_t action, uint32_t errorCode, uint32_t bidError)
{
    const auto auctionId = auction ? auction->Id : 0;

#if VERSION_STRING >= Cata
    uint64_t outBid = 0;
    uint64_t highestBid = 0;
#else
    uint32_t outBid = 0;
    uint32_t highestBid = 0;
#endif
    uint64_t highestBidderGuid = 0;

    if (auction)
    {
        outBid = auction->highestBid ? auction->getAuctionOutBid() : 0;
        highestBid = auction->highestBid;
        highestBidderGuid = auction->highestBidderGuid;
    }

    sendPacket(SmsgAuctionCommandResult(auctionId, action, errorCode, outBid, highestBid, bidError, highestBidderGuid).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Trade
Player* Player::getTradeTarget() const
{
    return m_TradeData != nullptr ? m_TradeData->getTradeTarget() : nullptr;
}

TradeData* Player::getTradeData() const
{
    return m_TradeData.get();
}

void Player::cancelTrade(bool sendToSelfAlso, bool silently /*= false*/)
{
    // TODO: for some reason client sends multiple trade cancel packets which at some point leads to nullptr trade data
    // investigate why client sends so many packets but use mutex for now to prevent crashes -Appled
    std::scoped_lock<std::mutex> guard(m_tradeMutex);

    if (m_TradeData != nullptr)
    {
        if (sendToSelfAlso)
            getSession()->sendTradeResult(TRADE_STATUS_CANCELLED);

        if (auto* tradeTarget = m_TradeData->getTradeTarget())
        {
            std::scoped_lock<std::mutex> targetGuard(tradeTarget->m_tradeMutex);
            if (!silently)
                tradeTarget->getSession()->sendTradeResult(TRADE_STATUS_CANCELLED);

            tradeTarget->m_TradeData = nullptr;
        }

        m_TradeData = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Messages
void Player::sendReportToGmMessage(std::string playerName, std::string damageLog)
{
    std::string gm_ann(MSG_COLOR_GREEN);

    gm_ann += "|HPlayer:";
    gm_ann += playerName;
    gm_ann += "|h[";
    gm_ann += playerName;
    gm_ann += "]|h: ";
    gm_ann += MSG_COLOR_YELLOW;
    gm_ann += damageLog;

    sWorld.sendMessageToOnlineGms(gm_ann);
}

void Player::broadcastMessage(const char* Format, ...)
{
    va_list list;
    va_start(list, Format);
    char Message[1024];
    vsnprintf(Message, 1024, Format, list);
    va_end(list);

    m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(Message)).serialise().get());
}

void Player::sendAreaTriggerMessage(const char* message, ...)
{
    va_list list;
    va_start(list, message);
    char msg[500];
    vsnprintf(msg, 500, message, list);
    va_end(list);

    m_session->SendPacket(SmsgAreaTriggerMessage(0, msg, 0).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Items
void Player::unEquipOffHandIfRequired()
{
    auto offHandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandWeapon == nullptr)
        return;

    auto needToRemove = true;
    // Check if player has a two-handed weapon in offhand
    if (offHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
    {
        needToRemove = !canDualWield2H();
    }
    else
    {
        // Player has something in offhand, check if main hand is a two-handed weapon
        const auto mainHandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (mainHandWeapon != nullptr && mainHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            needToRemove = !canDualWield2H();
        else
        {
            // Main hand nor offhand is a two-handed weapon, check if player can dual wield one-handed weapons
            if (offHandWeapon->isWeapon())
                needToRemove = !canDualWield();
            else
                needToRemove = false; // Offhand is not a weapon
        }
    }

    if (!needToRemove)
        return;

    // Unequip offhand and find a bag slot for it
    auto offHandWeaponHolder = getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
    auto result = getItemInterface()->FindFreeInventorySlot(offHandWeapon->getItemProperties());
    if (!result.Result)
    {
        // Player has no free slots in inventory, send it by mail
        offHandWeapon->removeFromWorld();
        offHandWeapon->setOwner(nullptr);
        offHandWeapon->saveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, getGuid(), getGuid(), "There were troubles with your item.", "There were troubles storing your item into your inventory.", 0, 0, offHandWeapon->getGuidLow(), MAIL_STATIONERY_GM);
    }
    else
    {
        auto [addResult, returnedItem] = getItemInterface()->SafeAddItem(std::move(offHandWeaponHolder), result.ContainerSlot, result.Slot);
        if (!addResult)
        {
            // TODO: if add fails, should item be sent in mail? now it's destroyed
            getItemInterface()->AddItemToFreeSlot(std::move(returnedItem));
        }
    }
}

bool Player::hasOffHandWeapon() const
{
    if (!canDualWield())
        return false;

    const auto offHandItem = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandItem == nullptr)
        return false;

    return offHandItem->isWeapon();
}

bool Player::hasItem(uint32_t itemId, uint32_t amount /*= 1*/, bool checkBankAlso /*= false*/) const
{
    return getItemInterface()->GetItemCount(itemId, checkBankAlso) >= amount;
}

#if VERSION_STRING == WotLK
void Player::calculateHeirloomBonus(ItemProperties const* proto, int16_t slot, bool apply)
{
    WDB::Structures::ScalingStatDistributionEntry const* ssd = getScalingStatDistributionFor(*proto);
    WDB::Structures::ScalingStatValuesEntry const* ssvrow = getScalingStatValuesFor(*proto);

    if (!ssd || !ssvrow)
        return;

    std::map<uint32_t, int32_t> tempStats;

    // Loop through 10 proto stats and try to find matching scaling stats
    for (uint32_t id = 0; id < MAX_ITEM_PROTO_STATS; ++id)
    {
        uint32_t statType = 0;
        int32_t  val = 0;

        if (ssd && ssvrow)
        {
            if (ssd->stat[id] < 0)
                continue;
            statType = ssd->stat[id];
            val = (ssvrow->getScalingStatDistributionMultiplier(proto->ScalingStatsFlag) * ssd->statmodifier[id]) / 10000;

            tempStats[statType] = val;
        }
    }

    // Loop through general stats from db and add all types not found
    for (auto generalStats : proto->generalStatsMap)
    {
        if (tempStats.size() < MAX_ITEM_PROTO_STATS)
        {
            if (tempStats.find(generalStats.first) == tempStats.end())
                tempStats[generalStats.first] = generalStats.second;
        }
    }

    // Loop through all collected stats and apply them
    auto it = tempStats.begin();
    for (uint32_t id = 0; id < MAX_ITEM_PROTO_STATS; ++id)
    {
        uint32_t statType = it->first;
        int32_t val = it->second;

        if (val == 0)
        {
            ++it;
            continue;
        }

        switch (statType)
        {
            case ITEM_MOD_MANA:
                modifyBonuses(ITEM_MOD_MANA, val, apply);
                break;
            case ITEM_MOD_HEALTH:                           // modify HP
                modifyBonuses(ITEM_MOD_HEALTH, val, apply);
                break;
            case ITEM_MOD_AGILITY:                          // modify agility
                modifyBonuses(ITEM_MOD_AGILITY, val, apply);
                break;
            case ITEM_MOD_STRENGTH:                         //modify strength
                modifyBonuses(ITEM_MOD_STRENGTH, val, apply);
                break;
            case ITEM_MOD_INTELLECT:                        //modify intellect
                modifyBonuses(ITEM_MOD_INTELLECT, val, apply);
                break;
            case ITEM_MOD_SPIRIT:                           //modify spirit
                modifyBonuses(ITEM_MOD_SPIRIT, val, apply);
                break;
            case ITEM_MOD_STAMINA:                          //modify stamina
                modifyBonuses(ITEM_MOD_STAMINA, val, apply);
                break;
            case ITEM_MOD_DEFENSE_RATING:
                modifyBonuses(ITEM_MOD_DEFENSE_RATING, val, apply);
                break;
            case ITEM_MOD_DODGE_RATING:
                modifyBonuses(ITEM_MOD_DODGE_RATING, val, apply);
                break;
            case ITEM_MOD_PARRY_RATING:
                modifyBonuses(ITEM_MOD_PARRY_RATING, val, apply);
                break;
            case ITEM_MOD_SHIELD_BLOCK_RATING:
                modifyBonuses(ITEM_MOD_SHIELD_BLOCK_RATING, val, apply);
                break;
            case ITEM_MOD_MELEE_HIT_RATING:
                modifyBonuses(ITEM_MOD_MELEE_HIT_RATING, val, apply);
                break;
            case ITEM_MOD_RANGED_HIT_RATING:
                modifyBonuses(ITEM_MOD_RANGED_HIT_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_HIT_RATING:
                modifyBonuses(ITEM_MOD_SPELL_HIT_RATING, val, apply);
                break;
            case ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING:
                modifyBonuses(ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING, val, apply);
                break;
            case ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING:
                modifyBonuses(ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING:
                modifyBonuses(ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING, val, apply);
                break;
            case ITEM_MOD_MELEE_HIT_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_MELEE_HIT_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_RANGED_HIT_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_RANGED_HIT_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_HIT_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_SPELL_HIT_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_MELEE_CRITICAL_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_MELEE_CRITICAL_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_RANGED_CRITICAL_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_RANGED_CRITICAL_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_CRITICAL_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_SPELL_CRITICAL_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_MELEE_HASTE_RATING:
                modifyBonuses(ITEM_MOD_MELEE_HASTE_RATING, val, apply);
                break;
            case ITEM_MOD_RANGED_HASTE_RATING:
                modifyBonuses(ITEM_MOD_RANGED_HASTE_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_HASTE_RATING:
                modifyBonuses(ITEM_MOD_SPELL_HASTE_RATING, val, apply);
                break;
            case ITEM_MOD_HIT_RATING:
                modifyBonuses(ITEM_MOD_HIT_RATING, val, apply);
                break;
            case ITEM_MOD_CRITICAL_STRIKE_RATING:
                modifyBonuses(ITEM_MOD_CRITICAL_STRIKE_RATING, val, apply);
                break;
            case ITEM_MOD_HIT_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_HIT_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_CRITICAL_AVOIDANCE_RATING:
                modifyBonuses(ITEM_MOD_CRITICAL_AVOIDANCE_RATING, val, apply);
                break;
            case ITEM_MOD_RESILIENCE_RATING:
                modifyBonuses(ITEM_MOD_RESILIENCE_RATING, val, apply);
                break;
            case ITEM_MOD_HASTE_RATING:
                modifyBonuses(ITEM_MOD_HASTE_RATING, val, apply);
                break;
            case ITEM_MOD_EXPERTISE_RATING:
                modifyBonuses(ITEM_MOD_EXPERTISE_RATING, val, apply);
                break;
            case ITEM_MOD_ATTACK_POWER:
                modifyBonuses(ITEM_MOD_ATTACK_POWER, val, apply);
                break;
            case ITEM_MOD_RANGED_ATTACK_POWER:
                modifyBonuses(ITEM_MOD_RANGED_ATTACK_POWER, val, apply);
                break;
            case ITEM_MOD_MANA_REGENERATION:
                modifyBonuses(ITEM_MOD_MANA_REGENERATION, val, apply);
                break;
            case ITEM_MOD_ARMOR_PENETRATION_RATING:
                modifyBonuses(ITEM_MOD_ARMOR_PENETRATION_RATING, val, apply);
                break;
            case ITEM_MOD_SPELL_POWER:
                modifyBonuses(ITEM_MOD_SPELL_POWER, val, apply);
                break;
            case ITEM_MOD_HEALTH_REGEN:
                modifyBonuses(ITEM_MOD_HEALTH_REGEN, val, apply);
                break;
            case ITEM_MOD_SPELL_PENETRATION:
                modifyBonuses(ITEM_MOD_SPELL_PENETRATION, val, apply);
                break;
            case ITEM_MOD_BLOCK_VALUE:
                modifyBonuses(ITEM_MOD_BLOCK_VALUE, val, apply);
                break;
                // deprecated item mods
            case ITEM_MOD_SPELL_HEALING_DONE:
            case ITEM_MOD_SPELL_DAMAGE_DONE:
                modifyBonuses(ITEM_MOD_SPELL_HEALING_DONE, val, apply);
                modifyBonuses(ITEM_MOD_SPELL_DAMAGE_DONE, val, apply);
                break;
            default:
                break;
        }

        ++it;
    }

    // Apply Spell Power from ScalingStatValue if set
    if (ssvrow)
        if (int32_t spellbonus = ssvrow->getSpellBonus(proto->ScalingStatsEntry))
            modifyBonuses(ITEM_MOD_SPELL_POWER, spellbonus, apply);

    // If set ScalingStatValue armor get it or use item armor
    uint32_t armor = proto->Armor;
    if (ssvrow)
    {
        if (uint32_t ssvarmor = ssvrow->getArmorMod(proto->ScalingStatsEntry))
            armor = ssvarmor;
    }
    else if (armor && proto->ArmorDamageModifier)
    {
        armor -= uint32_t(proto->ArmorDamageModifier);
    }

    if (armor)
    {
        if (apply)
            m_baseResistance[0] += armor;
        else
            m_baseResistance[0] -= armor;
    }

    /* Calculating the damages correct for our level and applying it */
    if (ssvrow)
    {
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            float minDamage = proto->Damage[i].Min;
            float maxDamage = proto->Damage[i].Max;

            // If set dpsMod in ScalingStatValue use it for min (70% from average), max (130% from average) damage
            if (i == 0) // scaling stats only for first damage
            {
                int32_t extraDPS = ssvrow->getDPSMod(proto->ScalingStatsFlag);
                if (extraDPS)
                {
                    float average = extraDPS * proto->Delay / 1000.0f;
                    minDamage = 0.7f * average;
                    maxDamage = 1.3f * average;
                }

                if (proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT || proto->InventoryType == INVTYPE_THROWN)
                {
                    m_baseRangedDamage[0] += apply ? minDamage : -minDamage;
                    m_baseRangedDamage[1] += apply ? maxDamage : -maxDamage;
                }
                else
                {
                    if (slot == EQUIPMENT_SLOT_OFFHAND)
                    {
                        m_baseOffhandDamage[0] = apply ? minDamage : 0;
                        m_baseOffhandDamage[1] = apply ? maxDamage : 0;
                    }
                    else
                    {
                        m_baseDamage[0] = apply ? minDamage : 0;
                        m_baseDamage[1] = apply ? maxDamage : 0;
                    }
                }
            }
        }
    }
}
#endif

#if VERSION_STRING > WotLK
void Player::calculateHeirloomBonus(ItemProperties const* proto, int16_t slot, bool apply)
{
    // Todo CATA/MOP
}
#endif

#if VERSION_STRING > TBC
WDB::Structures::ScalingStatDistributionEntry const* Player::getScalingStatDistributionFor(ItemProperties const& itemProto) const
{
    if (!itemProto.ScalingStatsEntry)
        return nullptr;

    return sScalingStatDistributionStore.lookupEntry(itemProto.ScalingStatsEntry);
}

WDB::Structures::ScalingStatValuesEntry const* Player::getScalingStatValuesFor(ItemProperties const& itemProto) const
{
    if (!itemProto.ScalingStatsFlag)
        return nullptr;

    WDB::Structures::ScalingStatDistributionEntry const* ssd = getScalingStatDistributionFor(itemProto);
    if (!ssd)
        return nullptr;

    // req. check at equip, but allow use for extended range if range limit max level, set proper level
    uint32_t const ssd_level = std::min(uint32_t(getLevel()), ssd->maxlevel);
    return sScalingStatValuesStore.lookupEntry(ssd_level);
}
#endif

ItemInterface* Player::getItemInterface() const
{
    return m_itemInterface.get();
}

void Player::removeTempItemEnchantsOnArena()
{
    ItemInterface* itemInterface = getItemInterface();

    for (uint32_t x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
        if (Item* item = itemInterface->GetInventoryItem(static_cast<int16_t>(x)))
            item->removeAllEnchantments(true);

    for (uint32_t x = INVENTORY_SLOT_BAG_START; x < INVENTORY_SLOT_BAG_END; ++x)
    {
        if (Item* item = itemInterface->GetInventoryItem(static_cast<int16_t>(x)))
        {
            if (item->isContainer())
            {
                Container* bag = static_cast<Container*>(item);
                for (uint32_t ci = 0; ci < bag->getItemProperties()->ContainerSlots; ++ci)
                {
                    if (auto* const bagItem = bag->getItem(static_cast<int16_t>(ci)))
                        bagItem->removeAllEnchantments(true);
                }
            }
        }
    }

    for (uint32_t x = INVENTORY_SLOT_ITEM_START; x < INVENTORY_SLOT_ITEM_END; ++x)
        if (Item* item = itemInterface->GetInventoryItem(static_cast<int16_t>(x)))
            item->removeAllEnchantments(true);
}

void Player::addGarbageItem(std::unique_ptr<Item> item) { m_GarbageItems.push_back(std::move(item)); }

void Player::removeGarbageItems() { m_GarbageItems.clear(); }

void Player::applyItemMods(Item* item, int16_t slot, bool apply, bool justBrokedown /* = false */, bool skipStatApply /* = false  */)
{
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (!item)
        return;

    ItemProperties const* itemProperties = item->getItemProperties();
    if (!itemProperties)
        return;

    if (!item->isContainer() && !item->getDurability() && item->getMaxDurability() && justBrokedown == false)
        return;

    item->applyRandomProperties(true);

    int32_t setId = 0;
    if (itemProperties->ItemSet < 0)
    {
        if (sMySQLStore.getItemSetLinkedBonus(itemProperties->ItemSet) != 0)
            setId = sMySQLStore.getItemSetLinkedBonus(itemProperties->ItemSet);
    }
    else
    {
        setId = itemProperties->ItemSet;
    }

    if (setId != 0)
    {
        if (auto itemSetEntry = sItemSetStore.lookupEntry(setId))
        {
            auto itemSetListMember = std::find_if(m_itemSets.begin(), m_itemSets.end(),
                [setId](ItemSet const& itemset) { return itemset.setid == setId; });

            if (apply)
            {
                ItemSet* itemSet = nullptr;

                // create new itemset if item has itemsetentry but not generated set stats
                if (itemSetListMember == m_itemSets.cend())
                {
                    // push to m_itemSets if it was not available before.
                    auto& newItemSet = m_itemSets.emplace_back(setId, 1);
                    itemSet = &newItemSet;
                }
                else
                {
                    itemSet = &(*itemSetListMember);
                    itemSet->itemscount++;
                }

                // apply spells from dbc for set
                if (!itemSetEntry->RequiredSkillID || (getSkillLineCurrent(static_cast<uint16_t>(itemSetEntry->RequiredSkillID), true) >= itemSetEntry->RequiredSkillAmt))
                {
                    for (uint8_t itemIndex = 0; itemIndex < 8; ++itemIndex)
                    {
                        if (itemSet->itemscount == itemSetEntry->itemscount[itemIndex])
                        {
                            const auto spellInfo = sSpellMgr.getSpellInfo(itemSetEntry->SpellID[itemIndex]);
                            Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
                            SpellCastTargets targets(getGuid());
                            spell->prepare(&targets);
                        }
                    }
                }
            }
            else
            {
                if (itemSetListMember != m_itemSets.cend())
                {
                    auto* itemSet = &(*itemSetListMember);
                    for (uint8_t itemIndex = 0; itemIndex < 8; ++itemIndex)
                        if (itemSet->itemscount == itemSetEntry->itemscount[itemIndex])
                            removeAllAurasByIdForGuid(itemSetEntry->SpellID[itemIndex], getGuid());

                    if (!(--itemSet->itemscount))
                        m_itemSets.erase(itemSetListMember);
                }
            }
        }
        else
        {
            sLogger.failure("Item {} has wrong ItemSet {}", itemProperties->ItemId, setId);
        }
    }

    for (auto resistanceStat : itemProperties->resistanceStatsMap)
    {
        uint8_t spellSchool = SCHOOL_NORMAL;
        switch (resistanceStat.first)
        {
            case ITEM_MOD_HOLY_RESISTANCE:
                spellSchool = SCHOOL_HOLY;
                break;
            case ITEM_MOD_FIRE_RESISTANCE:
                spellSchool = SCHOOL_FIRE;
                break;
            case ITEM_MOD_NATURE_RESISTANCE:
                spellSchool = SCHOOL_NATURE;
                break;
            case ITEM_MOD_FROST_RESISTANCE:
                spellSchool = SCHOOL_FROST;
                break;
            case ITEM_MOD_SHADOW_RESISTANCE:
                spellSchool = SCHOOL_SHADOW;
                break;
            case ITEM_MOD_ARCANE_RESISTANCE:
                spellSchool = SCHOOL_ARCANE;
                break;
            default:
                continue;
        }

        if (apply)
            m_flatResistanceModifierPos[spellSchool] += resistanceStat.second;
        else
            m_flatResistanceModifierPos[spellSchool] -= resistanceStat.second;

        calcResistance(spellSchool);
    }

#if VERSION_STRING > TBC
    if (itemProperties->ScalingStatsEntry != 0)
    {
        calculateHeirloomBonus(itemProperties, slot, apply);
    }
    else
#endif
    {
        // apply general stat mods
        for (auto stats : itemProperties->generalStatsMap)
            modifyBonuses(stats.first, stats.second, apply);

        if (itemProperties->Armor)
        {
            if (apply)
                m_baseResistance[0] += itemProperties->Armor;
            else
                m_baseResistance[0] -= itemProperties->Armor;
            calcResistance(0);
        }

        if (itemProperties->Damage[0].Min)
        {
            if (itemProperties->InventoryType == INVTYPE_RANGED || itemProperties->InventoryType == INVTYPE_RANGEDRIGHT || itemProperties->InventoryType == INVTYPE_THROWN)
            {
                m_baseRangedDamage[0] += apply ? itemProperties->Damage[0].Min : -itemProperties->Damage[0].Min;
                m_baseRangedDamage[1] += apply ? itemProperties->Damage[0].Max : -itemProperties->Damage[0].Max;
            }
            else
            {
                if (slot == EQUIPMENT_SLOT_OFFHAND)
                {
                    m_baseOffhandDamage[0] = apply ? itemProperties->Damage[0].Min : 0;
                    m_baseOffhandDamage[1] = apply ? itemProperties->Damage[0].Max : 0;
                }
                else
                {
                    m_baseDamage[0] = apply ? itemProperties->Damage[0].Min : 0;
                    m_baseDamage[1] = apply ? itemProperties->Damage[0].Max : 0;
                }
            }
        }
    }

    if (this->getClass() == DRUID && slot == EQUIPMENT_SLOT_MAINHAND)
    {
        uint8_t shapeShiftForm = getShapeShiftForm();
        if (shapeShiftForm == FORM_MOONKIN || shapeShiftForm == FORM_CAT || shapeShiftForm == FORM_BEAR || shapeShiftForm == FORM_DIREBEAR)
            this->applyFeralAttackPower(apply, item);
    }

    if (apply)
    {
        item->applyAllEnchantmentBonuses();

        for (auto itemSpell : item->getItemProperties()->Spells)
        {
            if (itemSpell.Id == 0)
                continue;

            if (auto spellInfo = sSpellMgr.getSpellInfo(itemSpell.Id))
            {
                if (itemSpell.Trigger == ON_EQUIP)
                {
                    if (spellInfo->getRequiredShapeShift())
                    {
                        addShapeShiftSpell(spellInfo->getId());
                        continue;
                    }

                    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
                    SpellCastTargets targets(getGuid());
                    spell->castedItemId = item->getEntry();
                    spell->prepare(&targets);

                }
                else if (itemSpell.Trigger == CHANCE_ON_HIT)
                {
                    // Calculate proc chance equivalent of 1 PPM
                    // On average 'chance on hit' effects on items seem to have 1 proc-per-minute
                    const auto procChance = Util::float2int32((item->getItemProperties()->Delay * 0.001f / 60.0f) * 100.0f);
                    switch (slot)
                    {
                        // 'Chance on hit' in main hand should only proc from main hand hits
                        case EQUIPMENT_SLOT_MAINHAND:
                            addProcTriggerSpell(spellInfo, nullptr, getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT), EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                            break;
                        // 'Chance on hit' in off hand should only proc from off hand hits
                        case EQUIPMENT_SLOT_OFFHAND:
                            addProcTriggerSpell(spellInfo, nullptr, getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT | PROC_ON_DONE_OFFHAND_ATTACK), EXTRA_PROC_ON_OFF_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                            break;
                        // 'Chance on hit' in ranged slot should only proc from ranged attacks
                        case EQUIPMENT_SLOT_RANGED:
                            addProcTriggerSpell(spellInfo, nullptr, getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_RANGED_HIT | PROC_ON_DONE_RANGED_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                            break;
                        // In any other slot, proc on any melee or ranged hit
                        default:
                            addProcTriggerSpell(spellInfo, nullptr, getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT | PROC_ON_DONE_RANGED_HIT | PROC_ON_DONE_RANGED_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                            break;
                    }
                }
            }
        }
    }
    else
    {
        item->removeAllEnchantmentBonuses();
        for (auto itemSpell : item->getItemProperties()->Spells)
        {
            if (itemSpell.Trigger == ON_EQUIP)
            {
                if (auto spellInfo = sSpellMgr.getSpellInfo(itemSpell.Id))
                {
                    if (spellInfo->getRequiredShapeShift())
                        removeShapeShiftSpell(spellInfo->getId());
                    else
                        removeAllAurasById(itemSpell.Id);
                }
            }
            else if (itemSpell.Trigger == CHANCE_ON_HIT)
            {
                this->removeProcTriggerSpell(itemSpell.Id);
            }
        }
    }

    if (!apply)
    {
        for (uint16_t posIndex = AuraSlots::POSITIVE_SLOT_START; posIndex < AuraSlots::POSITIVE_SLOT_END; ++posIndex)
        {
            if (auto* const m_aura = this->getAuraWithAuraSlot(posIndex))
                if (m_aura->m_castedItemId && m_aura->m_castedItemId == itemProperties->ItemId)
                    m_aura->removeAura();
        }
    }

    if (!skipStatApply)
        updateStats();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Difficulty
void Player::setDungeonDifficulty(uint8_t diff)
{
    m_dungeonDifficulty = diff;
}

uint8_t Player::getDungeonDifficulty()
{
    return m_dungeonDifficulty;
}

void Player::setRaidDifficulty(uint8_t diff)
{
    m_raidDifficulty = diff;
}

uint8_t Player::getRaidDifficulty()
{
    return m_raidDifficulty;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Die, Corpse & Repop
void Player::die(Unit* unitAttacker, uint32_t /*damage*/, uint32_t /*spellId*/)
{
    if (getDeathState() != ALIVE)
        return;

#ifdef FT_VEHICLES
    callExitVehicle();
#endif

#if VERSION_STRING > TBC
    if (unitAttacker != nullptr)
    {
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH, 1, 0, 0);
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP, GetMapId(), 1, 0);

        if (unitAttacker->isPlayer())
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER, 1, 0, 0);
        else if (unitAttacker->isCreature())
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE, 1, 0, 0);
    }
#endif

    if (unitAttacker != nullptr && !sHookInterface.OnPreUnitDie(unitAttacker, this))
        return;

    if (unitAttacker != nullptr && !unitAttacker->isPlayer())
        calcDeathDurabilityLoss(0.10);

    setDeathState(JUST_DIED);
    eventDeath();

    if (getChannelObjectGuid() != 0)
    {
        if (const auto spell = getCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            for (uint8_t i = 0; i < 3; i++)
            {
                if (spell->getSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    const uint64_t guid = getChannelObjectGuid();
                    DynamicObject* dynamicObject = getWorldMap()->getDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (!dynamicObject)
                        continue;

                    dynamicObject->remove();
                }
            }

            if (spell->getSpellInfo()->getChannelInterruptFlags() == 48140)
                interruptSpell(spell->getSpellInfo()->getId());
        }
    }

    for (const auto& inRangePlayer : getInRangePlayersSet())
    {
        Unit* attacker = dynamic_cast<Unit*>(inRangePlayer);
        if (attacker && attacker->isCastingSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                if (attacker->getCurrentSpell(static_cast<CurrentSpellType>(i)) == nullptr)
                    continue;

                if (attacker->getCurrentSpell(static_cast<CurrentSpellType>(i))->m_targets.getUnitTargetGuid() == getGuid())
                    attacker->interruptSpellWithSpellType(static_cast<CurrentSpellType>(i));
            }
        }
    }

    smsg_AttackStop(unitAttacker);
    eventAttackStop();

    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    setDynamicFlags(0);

    m_session->SendPacket(SmsgCancelCombat().serialise().get());

    WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 8);
    data << GetNewGUID();
    sendMessageToSet(&data, false);

    if (unitAttacker != nullptr && m_WorldMap && m_WorldMap->getScript())
        m_WorldMap->getScript()->OnPlayerDeath(this, unitAttacker);

    uint32_t selfResSpellId = 0;
    if (!m_bg || m_bg && !m_bg->isArena())
    {
        selfResSpellId = getSelfResurrectSpell();

        if (selfResSpellId == 0 && m_reincarnation)
        {
            SpellInfo const* m_reincarnSpellInfo = sSpellMgr.getSpellInfo(20608);
            if (!hasSpellOnCooldown(m_reincarnSpellInfo))
            {
                uint32_t ankhCount = getItemInterface()->GetItemCount(17030);
                if (ankhCount)
                    selfResSpellId = 21169;
            }
        }
    }

    setSelfResurrectSpell(selfResSpellId);
    setMountDisplayId(0);

    if (unitAttacker != nullptr)
    {
        if (unitAttacker->IsInWorld() && unitAttacker->isCreature() && static_cast<Creature*>(unitAttacker)->GetScript())
            static_cast<Creature*>(unitAttacker)->GetScript()->OnTargetDied(this);

        unitAttacker->getAIInterface()->eventOnTargetDied(this);
        unitAttacker->smsg_AttackStop(this);
    }

    getCombatHandler().clearCombat();

    m_underwaterTime = 0;
    m_underwaterState = 0;

    setMoveRoot(true);
    sendStopMirrorTimerPacket(MIRROR_TYPE_FATIGUE);
    sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
    sendStopMirrorTimerPacket(MIRROR_TYPE_FIRE);

    getSummonInterface()->removeAllSummons();

    // On player death set all pets offline
    // If player was i.e. mounted with pet inactive when they died its possible to get pet stuck in weird state
    setTemporarilyUnsummonedPetsOffline();

    setHealth(0);

    //check for spirit of Redemption
    if (hasSpell(20711))
    {
        SpellInfo const* sorInfo = sSpellMgr.getSpellInfo(27827);
        if (sorInfo != nullptr)
        {
            Spell* sor = sSpellMgr.newSpell(this, sorInfo, true, nullptr);
            SpellCastTargets targets(getGuid());
            sor->prepare(&targets);
        }
    }

    clearHealthBatch();

    if (getClass() == WARRIOR)
        setPower(POWER_TYPE_RAGE, 0);
#if VERSION_STRING == WotLK
    else if (getClass() == DEATHKNIGHT)
        setPower(POWER_TYPE_RUNIC_POWER, 0);
#endif

    if (m_bg)
    {
        m_bg->HookOnUnitDied(this);
        m_bg->HookOnPlayerDeath(this);
    }

    sHookInterface.OnDeath(this);
}

void Player::setCorpseData(LocationVector position, int32_t instanceId)
{
    m_corpseData.location = position;
    m_corpseData.instanceId = instanceId;
}

LocationVector Player::getCorpseLocation() const
{
    return m_corpseData.location;
}

int32_t Player::getCorpseInstanceId() const
{
    return m_corpseData.instanceId;
}

void Player::setAllowedToCreateCorpse(bool allowed)
{
    m_isCorpseCreationAllowed = allowed;
}

bool Player::isAllowedToCreateCorpse() const
{
    return m_isCorpseCreationAllowed;
}

void Player::createCorpse()
{
    sObjectMgr.delinkCorpseForPlayer(this);

    if (!isAllowedToCreateCorpse())
    {
        setAllowedToCreateCorpse(true);
        return;
    }

    const auto corpse = sObjectMgr.createCorpse();
    corpse->SetInstanceID(GetInstanceID());
    corpse->create(this, GetMapId(), GetPosition());

    corpse->setZoneId(getZoneId());

    corpse->setRace(getRace());
    corpse->setSkinColor(getSkinColor());

    corpse->setFace(getFace());
    corpse->setHairStyle(getHairStyle());
    corpse->setHairColor(getHairColor());
    corpse->setFacialFeatures(getFacialFeatures());

    corpse->setFlags(CORPSE_FLAG_UNK1);

    corpse->setDisplayId(getDisplayId());

    if (m_bg)
    {
        removeDynamicFlags(U_DYN_FLAG_LOOTABLE);
        removeUnitFlags(UNIT_FLAG_SKINNABLE);

        loot.gold = 0;

        corpse->generateLoot();
        if (m_lootableOnCorpse)
            corpse->setDynamicFlags(1);
        else
            corpse->setFlags(CORPSE_FLAG_UNK1 | CORPSE_FLAG_HIDDEN_HELM | CORPSE_FLAG_HIDDEN_CLOAK | CORPSE_FLAG_LOOT);

        m_lootableOnCorpse = false;
    }
    else
    {
        corpse->loot.gold = 0;
    }

    for (uint8_t slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (Item* item = getItemInterface()->GetInventoryItem(slot))
        {
            const uint32_t displayId = item->getItemProperties()->DisplayInfoID;
            const auto inventoryType = static_cast<uint16_t>(item->getItemProperties()->InventoryType);

            const uint32_t itemId = static_cast<uint16_t>(displayId) | inventoryType << 24;
            corpse->setItem(slot, itemId);
        }
    }

    corpse->saveToDB();
}

void Player::spawnCorpseBody()
{
    if (const auto corpse = sObjectMgr.getCorpseByOwner(this->getGuidLow()))
    {
        if (!corpse->IsInWorld())
        {
            if (m_lootableOnCorpse && corpse->getDynamicFlags() != 1)
                corpse->setDynamicFlags(1);

            if (m_WorldMap == nullptr)
                corpse->AddToWorld();
            else
                corpse->PushToWorld(m_WorldMap);
        }

        setCorpseData(corpse->GetPosition(), corpse->GetInstanceID());
    }
    else
    {
        setCorpseData({ 0, 0, 0, 0 }, 0);
    }
}

void Player::spawnCorpseBones()
{
    setCorpseData({ 0, 0, 0, 0 }, 0);

    if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
    {
        if (corpse->IsInWorld() && corpse->getCorpseState() == CORPSE_STATE_BODY)
        {
            corpse->spawnBones();
            sObjectMgr.addCorpseDespawnTime(corpse);
        }
    }
}

void Player::repopRequest()
{
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHECKFORCHEATS);
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FORCED_RESURRECT);

    if (m_corpseData.instanceId != 0)
    {
        if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
            corpse->resetDeathClock();

        resurrect();
        repopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return;
    }

    if (auto transport = this->GetTransport())
    {
        transport->RemovePassenger(this);
        this->obj_movement_info.clearTransportData();

        repopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return;
    }

    setDeathState(CORPSE);

    updateVisibility();

    removeUnitFlags(UNIT_FLAG_SKINNABLE);

    const bool hasCorpse = m_bg ? m_bg->CreateCorpse(this) : true;
    if (hasCorpse)
        createCorpse();

    buildRepop();

    if (!m_bg || m_bg && m_bg->hasStarted())
    {
        if (const auto mapInfo = sMySQLStore.getWorldMapInfo(GetMapId()))
        {
            if (mapInfo->isWorldMap() || mapInfo->isBattlegroundOrArena())
                repopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
            else
                repopAtGraveyard(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz, mapInfo->repopmapid);

            switch (mapInfo->mapid)
            {
                case 533: // Naxx
                case 550: // The Eye
                case 552: // The Arcatraz
                case 553: // The Botanica
                case 554: // The Mechanar
                    resurrect();
                    return;
                default:
                    break;
            }
        }
        else
        {
            repopAtGraveyard(getBindPosition().x, getBindPosition().y, getBindPosition().z, getBindMapId());
        }
    }

    if (hasCorpse)
    {
        spawnCorpseBody();

        if (m_corpseData.instanceId != 0)
            if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
                corpse->resetDeathClock();

        m_session->SendPacket(SmsgDeathReleaseLoc(m_mapId, m_position).serialise().get());
        m_session->SendPacket(SmsgCorpseReclaimDelay(CORPSE_RECLAIM_TIME_MS).serialise().get());
    }
}

void Player::repopAtGraveyard(float ox, float oy, float oz, uint32_t mapId)
{
#if VERSION_STRING >= WotLK
    if (hasAuraWithAuraEffect(SPELL_AURA_PREVENT_RESURRECTION))
        return;
#endif

    bool first = true;

    LocationVector currentLocation(ox, oy, oz);
    LocationVector finalDestination;
    LocationVector temp;

    if (!m_bg || !m_bg->HookHandleRepop(this))
    {
        float closestDistance = 999999.0f;

        MySQLStructure::Graveyards const* graveyard = nullptr;
        for (const auto& graveyardStore : *sMySQLStore.getGraveyardsStore())
        {
            graveyard = sMySQLStore.getGraveyard(graveyardStore.second.id);
            if (graveyard->mapId == mapId && (graveyard->factionId == getTeam() || graveyard->factionId == 3))
            {
                temp.ChangeCoords({ graveyard->position_x, graveyard->position_y, graveyard->position_z });
                const float distance = currentLocation.distanceSquare(temp);
                if (first || distance < closestDistance)
                {
                    first = false;
                    closestDistance = distance;
                    finalDestination = temp;
                }
            }
        }

        if (first && graveyard)
        {
            finalDestination.ChangeCoords({ graveyard->position_x, graveyard->position_y, graveyard->position_z });
            first = false;
        }
    }
    else
    {
        return;
    }

    if (sHookInterface.OnRepop(this) && !first)
        safeTeleport(mapId, 0, finalDestination);
}

void Player::resurrect()
{
    if (!sHookInterface.OnResurrect(this))
        return;

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FORCED_RESURRECT);

    if (m_resurrectHealth)
        setHealth(std::min(m_resurrectHealth, getMaxHealth()));

    if (m_resurrectMana)
        setPower(POWER_TYPE_MANA, m_resurrectMana);

    m_resurrectHealth = m_resurrectMana = 0;

    spawnCorpseBones();

    removeAllNegativeAuras();

    uint32_t AuraIds[] = { 20584, 9036, 8326, 55164, 0 };
    removeAllAurasById(AuraIds);

    removePlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE);
    setDeathState(ALIVE);

    updateVisibility();

    if (m_resurrecter && IsInWorld() && m_resurrectInstanceID == static_cast<uint32_t>(GetInstanceID()))
        safeTeleport(m_resurrectMapId, m_resurrectInstanceID, m_resurrectPosition);

    m_resurrecter = 0;
    setMoveLandWalk();

    for (uint8_t i = 0; i < 7; ++i)
        m_schoolImmunityList[i] = 0;

    if (m_bg)
        m_bg->HookOnPlayerResurrect(this);
}

void Player::buildRepop()
{
#if VERSION_STRING > TBC
    getSession()->SendPacket(SmsgPreResurrect(getGuid()).serialise().get());
#endif

    uint32_t AuraIds[] = { 20584, 9036, 8326, 0 };
    removeAllAurasById(AuraIds);

    setHealth(1);

    SpellCastTargets target(getGuid());

    if (getRace() == RACE_NIGHTELF)
    {
        SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(9036);
        Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
        spell->prepare(&target);
    }
    else
    {
        SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(8326);
        Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
        spell->prepare(&target);
    }

    sendStopMirrorTimerPacket(MIRROR_TYPE_FATIGUE);
    sendStopMirrorTimerPacket(MIRROR_TYPE_BREATH);
    sendStopMirrorTimerPacket(MIRROR_TYPE_FIRE);

    addPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE);

    setMoveRoot(false);
    setMoveWaterWalk();
}

void Player::calcDeathDurabilityLoss(double percent)
{
    sendPacket(SmsgDurabilityDamageDeath(static_cast<uint32_t>(percent)).serialise().get());

    for (uint8_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* item = getItemInterface()->GetInventoryItem(i))
        {
            const uint32_t maxDurability = item->getMaxDurability();
            const uint32_t durability = item->getDurability();
            if (durability)
            {
                int32_t newDurability = static_cast<uint32_t>(maxDurability * percent);
                newDurability = durability - newDurability;
                if (newDurability < 0)
                    newDurability = 0;

                if (newDurability <= 0)
                    applyItemMods(item, i, false, true);

                item->setDurability(static_cast<uint32_t>(newDurability));
                item->m_isDirty = true;
            }
        }
    }
}

void Player::setResurrecterGuid(uint64_t guid) { m_resurrecter = guid; }
void Player::setResurrectHealth(uint32_t health) { m_resurrectHealth = health; }
void Player::setResurrectMana(uint32_t mana) { m_resurrectMana = mana; }
void Player::setResurrectInstanceId(uint32_t id) { m_resurrectInstanceID = id; }
void Player::setResurrectMapId(uint32_t id) { m_resurrectMapId = id; }
void Player::setResurrectPosition(LocationVector position) { m_resurrectPosition = position; }

uint64_t Player::getAreaSpiritHealerGuid() const { return m_areaSpiritHealerGuid; }
void Player::setAreaSpiritHealerGuid(uint64_t guid) { m_areaSpiritHealerGuid = guid; }

void Player::setFullHealthMana()
{
    if (isDead())
        resurrect();

    setHealth(getMaxHealth());
    setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
    setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
    setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
}

void Player::setResurrect()
{
    resurrect();

    setMoveRoot(false);
    setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, false), true);
    setSpeedRate(TYPE_SWIM, getSpeedRate(TYPE_SWIM, false), true);
    setMoveLandWalk();

    setFullHealthMana();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Bind
void Player::setBindPoint(float x, float y, float z, float o, uint32_t mapId, uint32_t zoneId)
{
    m_bindData.location = { x, y, z, o };
    m_bindData.mapId = mapId;
    m_bindData.zoneId = zoneId;
}

LocationVector Player::getBindPosition() const { return m_bindData.location; }
uint32_t Player::getBindMapId() const { return m_bindData.mapId; }
uint32_t Player::getBindZoneId() const { return m_bindData.zoneId; }

//////////////////////////////////////////////////////////////////////////////////////////
// Battleground Entry
void Player::setBGEntryPoint(float x, float y, float z, float o, uint32_t mapId, int32_t instanceId)
{
    m_bgEntryData.location = { x, y, z, o };
    m_bgEntryData.mapId = mapId;
    m_bgEntryData.instanceId = instanceId;
}

LocationVector Player::getBGEntryPosition() const { return m_bgEntryData.location; }
uint32_t Player::getBGEntryMapId() const { return m_bgEntryData.mapId; }
int32_t Player::getBGEntryInstanceId() const { return m_bgEntryData.instanceId; }

//////////////////////////////////////////////////////////////////////////////////////////
// Charter
void Player::unsetCharter(uint8_t charterType) { m_charters[charterType] = nullptr; }
Charter const* Player::getCharter(uint8_t charterType) { return m_charters[charterType]; }

bool Player::canSignCharter(Charter const* charter, Player* requester)
{
    if (charter == nullptr || requester == nullptr)
        return false;

    if (charter->getCharterType() >= CHARTER_TYPE_ARENA_2V2 && getArenaTeam(charter->getCharterType() - 1U) != nullptr)
        return false;

    if (charter->getCharterType() == CHARTER_TYPE_GUILD && isInGuild())
        return false;

    if (m_charters[charter->getCharterType()] || requester->getTeam() != getTeam() || this == requester)
        return false;

    return true;
}

void Player::initialiseCharters()
{
    for (uint8_t i = 0; i < NUM_CHARTER_TYPES; ++i)
        m_charters[i] = sObjectMgr.getCharterByGuid(getGuid(), static_cast<CharterTypes>(i));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Guild
void Player::setInvitedByGuildId(uint32_t GuildId) { m_invitedByGuildId = GuildId; }
uint32_t Player::getInvitedByGuildId() const { return m_invitedByGuildId; }
Guild* Player::getGuild() const { return getGuildId() ? sGuildMgr.getGuildById(getGuildId()) : nullptr; }
bool Player::isInGuild() { return getGuild() != nullptr; }

uint32_t Player::getGuildRankFromDB()
{
    if (auto result = CharacterDatabase.Query("SELECT playerid, guildRank FROM guild_members WHERE playerid = %u", WoWGuid::getGuidLowPartFromUInt64(getGuid())))
    {
        Field* fields = result->Fetch();
        return fields[1].asUint32();
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Group
void Player::setGroupInviterId(uint32_t inviterId) { m_grouIdpInviterId = inviterId; }
uint32_t Player::getGroupInviterId() const { return m_grouIdpInviterId; }
bool Player::isAlreadyInvitedToGroup() const { return m_grouIdpInviterId != 0; }

bool Player::isInGroup() const { return m_playerInfo && m_playerInfo->m_Group; }

Group* Player::getGroup() { return m_playerInfo ? m_playerInfo->m_Group : nullptr; }
bool Player::isGroupLeader() const
{
    if (m_playerInfo->m_Group != nullptr)
    {
        if (m_playerInfo->m_Group->GetLeader() == m_playerInfo)
            return true;
    }
    return false;
}

int8_t Player::getSubGroupSlot() const { return m_playerInfo->subGroup; }

uint32_t Player::getGroupUpdateFlags() const { return m_groupUpdateFlags; }

void Player::setGroupUpdateFlags(uint32_t flags)
{
    if (getGroup())
        m_groupUpdateFlags = flags;
}

void Player::addGroupUpdateFlag(uint32_t flag)
{
    if (getGroup())
        m_groupUpdateFlags |= flag;
}

uint16_t Player::getGroupStatus()
{
    uint16_t status = MEMBER_STATUS_ONLINE;
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

void Player::sendUpdateToOutOfRangeGroupMembers()
{
    if (m_groupUpdateFlags == GROUP_UPDATE_FLAG_NONE)
        return;

    if (auto group = getGroup())
        group->UpdateOutOfRangePlayer(this, true, nullptr);

    m_groupUpdateFlags = GROUP_UPDATE_FLAG_NONE;

    if (Pet* pet = getPet())
        pet->resetAuraUpdateMaskForRaid();
}

void Player::eventGroupFullUpdate()
{
    if (m_playerInfo->m_Group)
        m_playerInfo->m_Group->UpdateAllOutOfRangePlayersFor(this);
}

bool Player::isSendOnlyRaidgroupSet() const { return m_sendOnlyRaidgroup; }
void Player::setSendOnlyRaidgroup(bool set) { m_sendOnlyRaidgroup = set; }

LocationVector Player::getLastGroupPosition() const { return m_lastGroupPosition; }

//////////////////////////////////////////////////////////////////////////////////////////
// Channels
void Player::joinedChannel(Channel* channel)
{
    if (channel == nullptr)
        return;

    std::lock_guard<std::mutex> guard(m_mutexChannel);
    m_channels.insert(channel);
}

void Player::leftChannel(Channel* channel)
{
    if (channel == nullptr)
        return;

    std::lock_guard<std::mutex> guard(m_mutexChannel);
    m_channels.erase(channel);
}

void Player::updateChannels()
{
    auto areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(getZoneId());

#if VERSION_STRING < WotLK
    // TODO: verify if this is needed anymore in < wotlk
    // Correct zone for Hall of Legends
    if (GetMapId() == 450)
        areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(2917);
    // Correct zone for Champions' Hall
    else if (GetMapId() == 449)
        areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(2918);
#endif

    // Update only default channels
    for (uint8_t i = 0; i < sChatChannelsStore.getNumRows(); ++i)
    {
        const auto channelDbc = sChatChannelsStore.lookupEntry(i);
        if (channelDbc == nullptr)
            continue;

        Channel* oldChannel = nullptr;

        m_mutexChannel.lock();
        for (auto _channel : m_channels)
        {
            if (_channel->getChannelId() == i)
            {
                // Found same channel
                oldChannel = _channel;
                break;
            }
        }
        m_mutexChannel.unlock();

        if (sChannelMgr.canPlayerJoinDefaultChannel(this, areaEntry, channelDbc))
        {
            auto channelName = sChannelMgr.generateChannelName(channelDbc, areaEntry);

            auto newChannel = sChannelMgr.getOrCreateChannel(channelName, this, channelDbc->id);
            if (newChannel == nullptr)
            {
                // should not happen
                sLogger.failure("Player::updateChannels : Could not create new channel {} with name {}", channelDbc->id, channelName);
                continue;
            }

            if (newChannel != oldChannel && !newChannel->hasMember(this))
            {
                // Join new channel
                newChannel->attemptJoin(this, "", true);
                // Leave old channel if it exists
                if (oldChannel != nullptr)
                    oldChannel->leaveChannel(this, false);
            }
        }
        else
        {
            // Leave old channel if it exists
            if (oldChannel != nullptr)
                oldChannel->leaveChannel(this);
        }
    }
}

void Player::removeAllChannels()
{
    std::set<Channel*> removeList;
    m_mutexChannel.lock();

    for (const auto& channel : m_channels)
        removeList.insert(channel);

    m_mutexChannel.unlock();

    auto itr = removeList.begin();
    while (itr != removeList.end())
    {
        (*itr)->leaveChannel(this);
        itr = removeList.erase(itr);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// ArenaTeam
void Player::setArenaTeam(uint8_t type, ArenaTeam* arenaTeam)
{
    m_arenaTeams[type] = arenaTeam;

    if (arenaTeam)
        getSession()->SystemMessage("You are now a member of the arena team'%s'.", arenaTeam->m_name.c_str());
}
ArenaTeam* Player::getArenaTeam(uint8_t type) { return m_arenaTeams[type]; }

bool Player::isInArenaTeam(uint8_t type) const { return m_arenaTeams[type] != nullptr; }
void Player::initialiseArenaTeam()
{
    for (uint8_t i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
    {
        m_arenaTeams[i] = sObjectMgr.getArenaTeamByGuid(getGuidLow(), i);
        if (m_arenaTeams[i] != nullptr)
        {
#if VERSION_STRING != Classic
            setArenaTeamId(i, m_arenaTeams[i]->m_id);

            if (m_arenaTeams[i]->m_leader == getGuidLow())
                setArenaTeamMemberRank(i, 0);
            else
                setArenaTeamMemberRank(i, 1);
#endif
        }
    }
}

void Player::addArenaPoints(uint32_t arenaPoints, bool sendUpdate)
{
    this->m_arenaPoints += arenaPoints;
    if (this->m_arenaPoints > worldConfig.limit.maxArenaPoints)
        this->m_arenaPoints = worldConfig.limit.maxArenaPoints;

    if (sendUpdate)
        this->updateArenaPoints();
}

uint32_t Player::getArenaPoints() const { return m_arenaPoints; }

void Player::removeArenaPoints(uint32_t arenaPoints, bool sendUpdate)
{
    int32_t newPoints = this->m_arenaPoints;
    newPoints -= arenaPoints;
    if (newPoints < 0)
        newPoints = 0;

    this->m_arenaPoints = newPoints;

    if (sendUpdate)
        this->updateArenaPoints();
}

void Player::updateArenaPoints()
{
#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    this->setArenaCurrency(this->m_arenaPoints);
#endif
#endif

    this->updateKnownCurrencies(43307, true);
}

void Player::setInviteArenaTeamId(uint32_t id) { m_inviteArenaTeamId = id; }
uint32_t Player::getInviteArenaTeamId() const { return m_inviteArenaTeamId; }

//////////////////////////////////////////////////////////////////////////////////////////
// Honor
void Player::addHonor(uint32_t honorPoints, bool sendUpdate)
{
    if (this->GetMapId() == 559 || this->GetMapId() == 562 || this->GetMapId() == 572)
        return;

    this->m_honorPoints += honorPoints;
    this->m_honorToday += honorPoints;
    if (this->m_honorPoints > worldConfig.limit.maxHonorPoints)
        this->m_honorPoints = worldConfig.limit.maxHonorPoints;

    if (sendUpdate)
        this->updateHonor();
}

uint32_t Player::getHonor() const { return m_honorPoints; }

void Player::removeHonor(uint32_t honorPoints, bool sendUpdate)
{
    int32_t newPoints = this->m_honorPoints;
    newPoints -= honorPoints;
    if (newPoints < 0)
        newPoints = 0;

    this->m_honorPoints = newPoints;

    if (sendUpdate)
        this->updateHonor();
}

void Player::updateHonor()
{
#if VERSION_STRING != Classic
    this->setFieldKills((this->m_killsToday | this->m_killsYesterday << 16));
#if VERSION_STRING < Cata
    this->setContributionToday(this->m_honorToday);
    this->setContributionYesterday(this->m_honorYesterday);

    this->setHonorCurrency(this->m_honorPoints);
#endif
#endif
    this->setLifetimeHonorableKills(this->m_killsLifetime);

    this->updateKnownCurrencies(43308, true);
}

void Player::rolloverHonor()
{
    uint32_t current_val = (g_localTime.tm_year << 16) | g_localTime.tm_yday;
    if (current_val != m_honorRolloverTime)
    {
        m_honorRolloverTime = current_val;
        m_honorYesterday = m_honorToday;
        m_killsYesterday = m_killsToday;
        m_honorToday = m_killsToday = 0;
    }
}

uint32_t Player::getHonorToday() const { return m_honorToday; }
uint32_t Player::getHonorYesterday() const { return m_honorYesterday; }
uint32_t Player::getHonorless() const { return m_honorless; }
void Player::incrementHonorless() { m_honorless++; }
void Player::decrementHonorless() { m_honorless > 0 ? m_honorless-- : m_honorless = 0; }

void Player::incrementKills(uint32_t count)
{
    if (count)
    {
        m_killsToday += count;
        m_killsLifetime += count;
        return;
    }

    m_killsToday++;
    m_killsLifetime++;
}

uint32_t Player::getKillsToday() const { return m_killsToday; }
uint32_t Player::getKillsLifetime() const { return m_killsLifetime; }
uint32_t Player::getKillsYesterday() const { return m_killsYesterday; }

//////////////////////////////////////////////////////////////////////////////////////////
// PvP
void Player::resetPvPTimer() { m_pvpTimer = worldConfig.getIntRate(INTRATE_PVPTIMER); }
void Player::stopPvPTimer() { m_pvpTimer = 0; }

void Player::setupPvPOnLogin()
{
    eventExploration();

    const auto areaTableEntry = this->GetArea();

    if (areaTableEntry != nullptr && isAlive() && 
        (areaTableEntry->team == AREAC_CONTESTED ||
            (isTeamAlliance() && areaTableEntry->team == AREAC_HORDE_TERRITORY) ||
            (isTeamHorde() && areaTableEntry->team == AREAC_ALLIANCE_TERRITORY)))
        castSpell(this, PLAYER_HONORLESS_TARGET_SPELL, true);
}

void Player::updatePvPArea()
{
    auto areaTableEntry = this->GetArea();
    if (areaTableEntry == nullptr)
        return;

    if (hasPlayerFlags(PLAYER_FLAG_GM))
    {
        if (isPvpFlagSet())
            removePvpFlag();
        else
            stopPvPTimer();

        removeFfaPvpFlag();
        return;
    }

    if ((areaTableEntry->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (areaTableEntry->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
    {
        if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
            resetPvPTimer();
    }
    else
    {
        if (areaTableEntry->flags & AREA_CITY_AREA || areaTableEntry->flags & AREA_CITY)
        {
            if ((areaTableEntry->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (areaTableEntry->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
            {
                if (!isPvpFlagSet())
                    setPvpFlag();
                else
                    stopPvPTimer();
                return;
            }
        }

        if (areaTableEntry->zone)
        {
            if (auto at2 = AreaStorage::GetAreaById(areaTableEntry->zone))
            {
                if ((at2->team == AREAC_ALLIANCE_TERRITORY && isTeamAlliance()) || (at2->team == AREAC_HORDE_TERRITORY && isTeamHorde()))
                {
                    if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
                        resetPvPTimer();

                    return;
                }

                if (at2->flags & AREA_CITY_AREA || at2->flags & AREA_CITY)
                {
                    if ((at2->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (at2->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
                    {
                        if (!isPvpFlagSet())
                            setPvpFlag();
                        else
                            stopPvPTimer();
                        return;
                    }
                }
            }
        }

        if (areaTableEntry->team == AREAC_SANCTUARY || areaTableEntry->flags & AREA_SANCTUARY)
        {
            if (isPvpFlagSet())
                removePvpFlag();
            else
                stopPvPTimer();

            removeFfaPvpFlag();
            setSanctuaryFlag();
        }
        else
        {
            removeSanctuaryFlag();

            if (sLogonCommHandler.getRealmType() == REALMTYPE_PVP || sLogonCommHandler.getRealmType() == REALMTYPE_RPPVP)
            {
                if (!isPvpFlagSet())
                    setPvpFlag();
                else
                    stopPvPTimer();
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
                    resetPvPTimer();
                }
            }

            if (areaTableEntry->flags & AREA_PVP_ARENA)
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

void Player::togglePvP()
{
    if (sLogonCommHandler.getRealmType() == REALMTYPE_NORMAL || sLogonCommHandler.getRealmType() == REALMTYPE_RP)
    {
        if (m_pvpTimer > 0)
        {
            stopPvPTimer();

            addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
            removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

            if (!isPvpFlagSet())
                setPvpFlag();
        }
        else
        {
            if (isPvpFlagSet())
            {
                auto areaTableEntry = this->GetArea();
                if (areaTableEntry && (areaTableEntry->flags & AREA_CITY_AREA || areaTableEntry->flags & AREA_CITY))
                {
                    if ((areaTableEntry->team == AREAC_ALLIANCE_TERRITORY && isTeamHorde()) || (areaTableEntry->team == AREAC_HORDE_TERRITORY && isTeamAlliance()))
                    {
                    }
                    else
                    {
                        resetPvPTimer();
                    }
                }
                else
                {
                    resetPvPTimer();
                }

                removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif
            }
            else
            {
                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

                stopPvPTimer();
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
                stopPvPTimer();

                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

                if (!isPvpFlagSet())
                    setPvpFlag();
            }
            else
            {
                if (isPvpFlagSet())
                {
                    // Start the "cooldown" timer.
                    resetPvPTimer();

                    removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                    addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif
                }
                else
                {
                    // Move into PvP state.
                    addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                    removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

                    stopPvPTimer();
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
                        stopPvPTimer();

                        addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                        removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

                        if (!isPvpFlagSet())
                            setPvpFlag();
                    }
                    else
                    {
                        if (isPvpFlagSet())
                        {
                            // Start the "cooldown" timer.
                            resetPvPTimer();

                            removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                            addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif
                        }
                        else
                        {
                            // Move into PvP state.
                            addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                            removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif

                            stopPvPTimer();
                            setPvpFlag();
                        }
                    }
                    return;
                }
            }

            if (!hasPlayerFlags(PLAYER_FLAG_PVP_TOGGLE))
            {
                addPlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif
            }
            else
            {
                removePlayerFlags(PLAYER_FLAG_PVP_TOGGLE);
#if VERSION_STRING >= WotLK
                addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
#endif
            }
        }
    }
}

void Player::updatePvPCurrencies()
{
    this->updateHonor();
    this->updateArenaPoints();
}

bool Player::hasPvPTitle(RankTitles title)
{
#if VERSION_STRING > Classic
    const auto index = static_cast<uint8_t>(title / 32);

    return (getKnownTitles(index) & 1ULL << static_cast<uint64_t>((title % 32))) != 0;
#else
    return false;
#endif
}

void Player::setKnownPvPTitle(RankTitles title, bool set)
{
#if VERSION_STRING > Classic
    if (!set && !hasPvPTitle(title))
        return;

    const auto index = static_cast<uint8_t>(title / 32);
    const uint64_t current = getKnownTitles(index);

    if (set)
        setKnownTitles(index, current | 1ULL << static_cast<uint64_t>((title % 32)));
    else
        setKnownTitles(index, current & ~1 << (title % 32));

    m_session->SendPacket(SmsgTitleEarned(title, set ? 1 : 0).serialise().get());
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Battleground
Battleground* Player::getBattleground() const { return m_bg; }
void Player::setBattleground(Battleground* bg) { m_bg = bg; }

Battleground* Player::getPendingBattleground() const { return m_pendingBattleground; }
void Player::setPendingBattleground(Battleground* bg) { m_pendingBattleground = bg; }

bool Player::isQueuedForBg() const { return m_isQueuedForBg; }
void Player::setIsQueuedForBg(bool set) { m_isQueuedForBg = set; }

bool Player::hasQueuedBgInstanceId() const { return m_queuedBgInstanceId != 0; }
uint32_t Player::getQueuedBgInstanceId() const { return m_queuedBgInstanceId; }
void Player::setQueuedBgInstanceId(uint32_t id) { m_queuedBgInstanceId = id; }

bool Player::isQueuedForRbg() const { return this->m_isQueuedForRbg; }
void Player::setIsQueuedForRbg(bool value) { this->m_isQueuedForRbg = value; }

void Player::removeFromBgQueue()
{
    if (!m_pendingBattleground)
        return;

    m_pendingBattleground->removePendingPlayer(this);
    m_session->systemMessage(getSession()->LocalizedWorldSrv(ServerString::SS_BG_REMOVE_QUEUE_INF));
}

bool Player::hasWonRbgToday() const { return this->m_hasWonRbgToday; }
void Player::setHasWonRbgToday(bool value) { this->m_hasWonRbgToday = value; }

void Player::setBgQueueType(uint32_t type) { this->m_bgQueueType = type; }
uint32_t Player::getBgQueueType() const { return this->m_bgQueueType; }

bool Player::hasBgFlag() const { return m_bgHasFlag; }
void Player::setHasBgFlag(bool set) { m_bgHasFlag = set; }

void Player::setRoles(uint8_t role) { m_roles = role; }
uint8_t Player::retRoles() const { return m_roles; }

void Player::fillRandomBattlegroundReward(bool wonBattleground, uint32_t& honorPoints, uint32_t& arenaPoints)
{
    auto honorForSingleKill = HonorHandler::CalculateHonorPointsForKill(this->getLevel(), this->getLevel());

    if (wonBattleground)
    {
        if (this->m_hasWonRbgToday)
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

void Player::applyRandomBattlegroundReward(bool wonBattleground)
{
    uint32_t honorPoints, arenaPoints;
    this->fillRandomBattlegroundReward(wonBattleground, honorPoints, arenaPoints);
    this->addHonor(honorPoints, false);
    this->addArenaPoints(arenaPoints, false);
    this->updatePvPCurrencies();
}

uint32_t Player::getLevelGrouping()
{
    uint32_t level = getLevel();

    if (level < 10)
        return 0;
    if (level < 20)
        return 1;
    if (level < 30)
        return 2;
    if (level < 40)
        return 3;
    if (level < 50)
        return 4;
    if (level < 60)
        return 5;
    if (level < 70)
        return 6;
    if (level < 80)
        return 7;

    return 8;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Quests
void Player::acceptQuest(uint64_t guid, uint32_t quest_id)
{
    bool bValid = false;
    bool hasquest = true;
    bool bSkipLevelCheck = false;

    QuestProperties const* questProperties = nullptr;

    Object* qst_giver = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    if (wowGuid.isUnit())
    {
        Creature* quest_giver = m_WorldMap->getCreature(wowGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        hasquest = quest_giver->HasQuest(quest_id, 1);
        if (quest_giver->isQuestGiver())
        {
            bValid = true;
            questProperties = sMySQLStore.getQuestProperties(quest_id);
        }
    }
    else if (wowGuid.isGameObject())
    {
        GameObject* quest_giver = m_WorldMap->getGameObject(wowGuid.getGuidLowPart());
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        questProperties = sMySQLStore.getQuestProperties(quest_id);
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
        questProperties = sMySQLStore.getQuestProperties(quest_id);
    }
    else if (wowGuid.isPlayer())
    {
        Player* quest_giver = m_WorldMap->getPlayer(static_cast<uint32_t>(guid));
        if (quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = true;
        questProperties = sMySQLStore.getQuestProperties(quest_id);
    }

    if (!qst_giver)
    {
        sLogger.debug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || questProperties == nullptr)
    {
        sLogger.debug("WORLD: Creature is not a questgiver.");
        return;
    }

    if (hasQuestInQuestLog(questProperties->id))
        return;

    if (qst_giver->isCreature() && dynamic_cast<Creature*>(qst_giver)->m_escorter != nullptr)
    {
        m_session->SystemMessage("You cannot accept this quest at this time.");
        return;
    }

    // Check the player hasn't already taken this quest, or it isn't available.
    const uint32_t status = sQuestMgr.CalcQuestStatus(qst_giver, this, questProperties, 3, bSkipLevelCheck);

    if ((!sQuestMgr.IsQuestRepeatable(questProperties) && hasQuestFinished(questProperties->id))
        || (status != QuestStatus::Available && status != QuestStatus::Repeatable && status != QuestStatus::AvailableChat)
        || !hasquest)
    {
        return;
    }

    const uint8_t log_slot = getFreeQuestSlot();
    if (log_slot > MAX_QUEST_SLOT)
    {
        sQuestMgr.SendQuestLogFull(this);
        return;
    }

    if ((questProperties->time != 0) && hasTimedQuestInQuestSlot())
    {
        sQuestMgr.SendQuestInvalid(INVALID_REASON_HAVE_TIMED_QUEST, this);
        return;
    }

    if (questProperties->count_receiveitems || questProperties->srcitem)
    {
        const uint32_t slots_required = questProperties->count_receiveitems;

        if (getItemInterface()->CalculateFreeSlots(nullptr) < slots_required)
        {
            getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
            sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, questProperties, this);
            return;
        }
    }

    auto* questLogEntry = createQuestLogInSlot(questProperties, log_slot);
    questLogEntry->updatePlayerFields();

    // If the quest should give any items on begin, give them the items.
    for (uint32_t receive_item : questProperties->receive_items)
    {
        if (receive_item)
        {
            if (auto itemHolder = sObjectMgr.createItem(receive_item, this))
            {
                auto* item = itemHolder.get();
                const auto [addResult, _] = getItemInterface()->AddItemToFreeSlot(std::move(itemHolder));
                if (addResult == ADD_ITEM_RESULT_OK)
                {
                    sendItemPushResultPacket(false, true, false,
                        getItemInterface()->LastSearchItemBagSlot(), getItemInterface()->LastSearchItemSlot(),
                        1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
                }
            }
        }
    }

    if (questProperties->srcitem && questProperties->srcitem != questProperties->receive_items[0])
    {
        if (!qst_giver->isItem() || (qst_giver->getEntry() != questProperties->srcitem))
        {
            if (auto item = sObjectMgr.createItem(questProperties->srcitem, this))
            {
                item->setStackCount(questProperties->srcitemcount ? questProperties->srcitemcount : 1);
                getItemInterface()->AddItemToFreeSlot(std::move(item));
            }
        }
    }

    updateNearbyQuestGameObjects();

    const SpellAreaForQuestMapBounds saBounds = { sSpellMgr.mSpellAreaForQuestMap.lower_bound(quest_id), sSpellMgr.mSpellAreaForQuestMap.upper_bound(quest_id) };
    if (saBounds.first != saBounds.second)
    {
        for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second->autoCast && itr->second->fitsToRequirements(this, getZoneId(), getAreaId()))
                if (!hasAurasWithId(itr->second->spellId))
                    castSpell(this, itr->second->spellId, true);
        }
    }

    sQuestMgr.OnQuestAccepted(this, questProperties, qst_giver);

    // Hook to Creature Script
    if (qst_giver->ToCreature() && qst_giver->ToCreature()->GetScript())
        qst_giver->ToCreature()->GetScript()->onQuestAccept(this, questProperties);

    sLogger.debug("WORLD: Added new QLE.");
    sHookInterface.OnQuestAccept(this, questProperties, qst_giver);
}

QuestLogEntry* Player::createQuestLogInSlot(QuestProperties const* questProperties, uint8_t slotId)
{
    if (slotId >= MAX_QUEST_LOG_SIZE)
        return nullptr;

    if (questProperties == nullptr)
    {
        m_questlog[slotId] = nullptr;
        return nullptr;
    }

    m_questlog[slotId] = std::make_unique<QuestLogEntry>(questProperties, this, slotId);
    return m_questlog[slotId].get();
}

bool Player::hasAnyQuestInQuestSlot() const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr)
            return true;

    return false;
}

bool Player::hasQuestInQuestLog(uint32_t questId) const
{
    if (getQuestLogByQuestId(questId))
        return true;

    return false;
}

uint8_t Player::getFreeQuestSlot() const
{
    for (uint8_t slotId = 0; slotId < MAX_QUEST_LOG_SIZE; ++slotId)
        if (m_questlog[slotId] == nullptr)
            return slotId;

    return MAX_QUEST_LOG_SIZE + 1;
}

QuestLogEntry* Player::getQuestLogByQuestId(uint32_t questId) const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr)
            if (questlogSlot->getQuestProperties()->id == questId)
                return questlogSlot.get();

    return nullptr;
}

QuestLogEntry* Player::getQuestLogBySlotId(uint32_t slotId) const
{
    if (slotId < MAX_QUEST_LOG_SIZE)
        return m_questlog[slotId].get();

    return nullptr;
}

void Player::addQuestIdToFinishedDailies(uint32_t questId)
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    m_finishedDailies.insert(questId);
}
std::set<uint32_t> Player::getFinishedDailies() const
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    return m_finishedDailies;
}
bool Player::hasQuestInFinishedDailies(uint32_t questId) const
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    return m_finishedDailies.find(questId) != m_finishedDailies.end();
}
void Player::resetFinishedDailies()
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    m_finishedDailies.clear();
}

bool Player::hasTimedQuestInQuestSlot() const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr && questlogSlot->getQuestProperties()->time != 0)
            return true;

    return false;
}

void Player::eventTimedQuestExpire(uint32_t questId)
{
    if (QuestLogEntry* questLogEntry = this->getQuestLogByQuestId(questId))
    {
        QuestProperties const* qst = questLogEntry->getQuestProperties();

        sQuestMgr.SendQuestUpdateFailedTimer(qst, this);

        if (const auto questScript = questLogEntry->getQuestScript())
            questScript->OnQuestCancel(this);

        questLogEntry->sendQuestFailed(true);
    }
}

uint32_t Player::getQuestSharerByDbId() const { return m_questSharer; }
void Player::setQuestSharerDbId(uint32_t id) { m_questSharer = id; }

void Player::addQuestToRemove(uint32_t questId) { m_removequests.insert(questId); }

void Player::addQuestToFinished(uint32_t questId)
{
    if (m_finishedQuests.find(questId) != m_finishedQuests.end())
        return;

    m_finishedQuests.insert(questId);
}

bool Player::hasQuestFinished(uint32_t questId) const
{
    return m_finishedQuests.find(questId) != m_finishedQuests.cend();
}

void Player::areaExploredQuestEvent(uint32_t questId)
{
    sQuestMgr.AreaExplored(this, questId);
}

void Player::clearQuest(uint32_t questId)
{
    m_finishedQuests.erase(questId);
    m_finishedDailies.erase(questId);
}

bool Player::hasQuestForItem(uint32_t itemId) const
{
    for (const auto& questLogEntry : m_questlog)
    {
        if (questLogEntry != nullptr)
        {
            QuestProperties const* questProperties = questLogEntry->getQuestProperties();

            // Check the item_quest_association table for an entry related to this item
            if (const auto* tempList = sQuestMgr.GetQuestAssociationListForItemId(itemId))
            {
                for (auto questAssiciation = tempList->cbegin(); questAssiciation != tempList->cend(); ++questAssiciation)
                    if ((*questAssiciation)->qst == questProperties && (getItemInterface()->GetItemCount(itemId) < (*questAssiciation)->item_count))
                        return true;
            }

            // No item_quest association found, check the quest requirements
            if (!questProperties->count_required_item)
                continue;

            for (uint8_t j = 0; j < MAX_REQUIRED_QUEST_ITEM; ++j)
                if (questProperties->required_item[j] == itemId && getItemInterface()->GetItemCount(itemId) < questProperties->required_itemcount[j])
                    return true;
        }
    }
    return false;
}

void Player::addQuestSpell(uint32_t spellId) { quest_spells.insert(spellId); }

//Only for Cast Quests
bool Player::hasQuestSpell(uint32_t spellId)
{
    if (!quest_spells.empty() && quest_spells.find(spellId) != quest_spells.end())
        return true;

    return false;
}

//Only for Cast Quests
void Player::removeQuestSpell(uint32_t spellId)
{
    if (!quest_spells.empty())
        quest_spells.erase(spellId);
}

void Player::addQuestMob(uint32_t entry) { quest_mobs.insert(entry); }

//Only for Kill Quests
bool Player::hasQuestMob(uint32_t entry)
{
    if (!quest_mobs.empty() && quest_mobs.find(entry) != quest_mobs.end())
        return true;

    return false;
}

//Only for Kill Quests
void Player::removeQuestMob(uint32_t entry)
{
    if (!quest_mobs.empty())
        quest_mobs.erase(entry);
}

void Player::addQuestKill(uint32_t questId, uint8_t reqId, uint32_t delay)
{
    if (!hasQuestInQuestLog(questId))
        return;

    if (delay)
    {
        sEventMgr.AddEvent(this, &Player::addQuestKill, questId, reqId, static_cast<uint32_t>(0), EVENT_PLAYER_UPDATE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    if (QuestLogEntry* questLogEntry = getQuestLogByQuestId(questId))
    {
        if (QuestProperties const* quest = questLogEntry->getQuestProperties())
        {
            if (questLogEntry->getMobCountByIndex(reqId) >= quest->required_mob_or_go_count[reqId])
                return;

            questLogEntry->incrementMobCountForIndex(reqId);
            questLogEntry->sendUpdateAddKill(reqId);
            questLogEntry->updatePlayerFields();

            if (questLogEntry->canBeFinished())
                questLogEntry->sendQuestComplete();
        }
    }
}

void Player::updateNearbyQuestGameObjects()
{
    for (const auto& itr : getInRangeObjectsSet())
    {
        auto* obj = itr;
        if (obj == nullptr || !obj->isGameObject() || obj->isTransporter())
            continue;

        if (const auto gameobject = dynamic_cast<GameObject*>(obj))
        {
            const auto gobProperties = gameobject->GetGameObjectProperties();

            // Update dynamic flags for gameobjects with quests or item loot
            if (gameobject->isQuestGiver() || !gobProperties->itemMap.empty() || !gobProperties->goMap.empty())
            {
#if VERSION_STRING < Mop
                gameobject->forceBuildUpdateValueForField(getOffsetForStructuredField(WoWGameObject, dynamic), this);
#else
                gameobject->forceBuildUpdateValueForField(getOffsetForStructuredField(WoWObject, dynamic_field), this);
#endif
            }
        }
    }
}

std::set<uint32_t> Player::getFinishedQuests() const { return m_finishedQuests; }

//////////////////////////////////////////////////////////////////////////////////////////
// Social
void Player::loadFriendList()
{
    if (auto result = CharacterDatabase.Query("SELECT * FROM social_friends WHERE character_guid = %u", getGuidLow()))
    {
        do
        {
            SocialFriends socialFriend;

            auto* const socialField = result->Fetch();
            socialFriend.friendGuid = socialField[1].asUint32();
            socialFriend.note = socialField[2].asCString();

            m_socialIFriends.push_back(socialFriend);

        } while (result->NextRow());
    }
}

void Player::loadFriendedByOthersList()
{
    if (auto result = CharacterDatabase.Query("SELECT character_guid FROM social_friends WHERE friend_guid = %u", getGuidLow()))
    {
        do
        {
            auto* const socialField = result->Fetch();
            uint32_t friendedByGuid= socialField[0].asUint32();

            m_socialFriendedByGuids.push_back(friendedByGuid);

        } while (result->NextRow());
    }
}

void Player::loadIgnoreList()
{
    if (auto result = CharacterDatabase.Query("SELECT * FROM social_ignores WHERE character_guid = %u", getGuidLow()))
    {
        do
        {
            auto* const ignoreField = result->Fetch();
            uint32_t ignoreGuid = ignoreField[1].asUint32();

            m_socialIgnoring.push_back(ignoreGuid);

        } while (result->NextRow());
    }
}

void Player::addToFriendList(std::string name, std::string note)
{
    if (auto* targetPlayer = sObjectMgr.getPlayer(name.c_str()))
    {
        // we can not add us ;)
        if (targetPlayer->getGuidLow() == getGuidLow())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_SELF, getGuid()).serialise().get());
            return;
        }

        if (targetPlayer->isGMFlagSet())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
            return;
        }

        if (isFriended(targetPlayer->getGuidLow()))
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ALREADY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        if (targetPlayer->getPlayerInfo()->team != getInitialTeam() && !m_session->hasPermissions() && !worldConfig.player.isInterfactionFriendsEnabled)
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ENEMY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        if (targetPlayer->getSession())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_ONLINE, targetPlayer->getGuidLow(), note, 1,
                targetPlayer->getZoneId(), targetPlayer->getLevel(), targetPlayer->getClass()).serialise().get());
        }
        else
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_OFFLINE, targetPlayer->getGuidLow()).serialise().get());
        }

        // todo: missing FRIEND_LIST_FULL when friend list is full

        CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')",
            getGuidLow(), targetPlayer->getGuidLow(), !note.empty() ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "");

        SocialFriends socialFriend;
        socialFriend.friendGuid = targetPlayer->getGuidLow();
        socialFriend.note = note;

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        m_socialIFriends.push_back(socialFriend);
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
    }
}

void Player::removeFromFriendList(uint32_t guid)
{
    if (isFriended(guid))
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_REMOVED, guid).serialise().get());

        CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u", getGuidLow(), guid);

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        m_socialIFriends.erase(std::remove_if(m_socialIFriends.begin(), m_socialIFriends.end(), [&](SocialFriends const& friends) {
                return friends.friendGuid == guid;
            }),
            m_socialIFriends.end());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
    }
}

void Player::addNoteToFriend(uint32_t guid, std::string note)
{
    std::lock_guard<std::mutex> guard(m_mutexFriendList);
    for (const auto friends : m_socialIFriends)
    {
        if (friends.friendGuid == guid)
        {
            friends.note = note;
            CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u",
                !note.empty() ? CharacterDatabase.EscapeString(note).c_str() : "", getGuidLow(), guid);
        }
    }
}

bool Player::isFriended(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_mutexFriendList);
    for (const auto friends : m_socialIFriends)
    {
        if (friends.friendGuid == guid)
            return true;
    }
    return false;
}

void Player::sendFriendStatus(bool comesOnline)
{
    std::lock_guard<std::mutex> guard(m_mutexFriendedBy);
    if (!m_socialFriendedByGuids.empty())
    {
        for (auto friendedGuids : m_socialFriendedByGuids)
        {
            if (auto* targetPlayer = sObjectMgr.getPlayer(friendedGuids))
            {
                if (targetPlayer->getSession())
                {
                    if (comesOnline)
                        targetPlayer->sendPacket(SmsgFriendStatus(FRIEND_ONLINE, getGuid(), "", 1, getAreaId(), getLevel(), getClass()).serialise().get());
                    else
                        targetPlayer->sendPacket(SmsgFriendStatus(FRIEND_OFFLINE, getGuid()).serialise().get());
                }
            }
        }
    }
}

void Player::sendFriendLists(uint32_t flags)
{
    std::vector<SmsgContactListMember> contactMemberList;

    if (flags & 0x01)    // friend
    {
        uint32_t maxCount = 0;

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        for (auto friends : m_socialIFriends)
        {
            SmsgContactListMember friendListMember;
            friendListMember.guid = friends.friendGuid;
            friendListMember.flag = 0x01;
            friendListMember.note = friends.note;

            if (auto* plr = sObjectMgr.getPlayer(friends.friendGuid))
            {
                friendListMember.isOnline = 1;
                friendListMember.zoneId = plr->getZoneId();
                friendListMember.level = plr->getLevel();
                friendListMember.playerClass = plr->getClass();
            }
            else
            {
                friendListMember.isOnline = 0;
            }

            contactMemberList.push_back(friendListMember);
            ++maxCount;

            if (maxCount >= 50)
                break;
        }

    }

    if (flags & 0x02)    // ignore
    {
        uint32_t maxCount = 0;

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        for (auto ignoredGuid : m_socialIgnoring)
        {
            SmsgContactListMember ignoreListMember;
            ignoreListMember.guid = ignoredGuid;
            ignoreListMember.flag = 0x02;
            ignoreListMember.note = "";

            contactMemberList.push_back(ignoreListMember);
            ++maxCount;

            if (maxCount >= 50)
                break;
        }
    }

    sendPacket(SmsgContactList(flags, contactMemberList).serialise().get());
}

void Player::addToIgnoreList(std::string name)
{
    if (auto* targetPlayer = sObjectMgr.getPlayer(name.c_str()))
    {
        // we can not add us ;)
        if (targetPlayer->getGuidLow() == getGuidLow())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_SELF, getGuid()).serialise().get());
            return;
        }

        if (targetPlayer->isGMFlagSet())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
            return;
        }

        if (isIgnored(targetPlayer->getGuidLow()))
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_ALREADY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        // todo: missing FRIEND_IGNORE_FULL when ignore list is full

        CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", getGuidLow(), targetPlayer->getGuidLow());

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        m_socialIgnoring.push_back(targetPlayer->getGuidLow());

        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_ADDED, targetPlayer->getGuidLow()).serialise().get());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
    }
}

void Player::removeFromIgnoreList(uint32_t guid)
{
    if (isIgnored(guid))
    {
        CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", getGuidLow(), guid);

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        m_socialIgnoring.erase(std::remove(m_socialIgnoring.begin(), m_socialIgnoring.end(), guid), m_socialIgnoring.end());

        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_REMOVED, guid).serialise().get());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
    }
}

bool Player::isIgnored(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
    if (std::find(m_socialIgnoring.begin(), m_socialIgnoring.end(), guid) != m_socialIgnoring.end())
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Hack/Cheat Detection
void Player::speedCheatDelay(uint32_t delay)
{
    m_speedCheatDetector->SkipSamplingUntil(Util::getMSTime() + delay + getSession()->GetLatency() * 2 + 2000);
}

void Player::speedCheatReset()
{
    m_speedCheatDetector->EventSpeedChange();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Player::isGMFlagSet() const
{
    return hasPlayerFlags(PLAYER_FLAG_GM);
}

void Player::sendMovie([[maybe_unused]]uint32_t movieId)
{
#if VERSION_STRING > TBC
    m_session->SendPacket(SmsgTriggerMovie(movieId).serialise().get());
#endif
}

PlayerSpec& Player::getActiveSpec()
{
#ifdef FT_DUAL_SPEC
    return m_specs[m_talentActiveSpec];
#else
    return m_spec;
#endif
}

void Player::logIntoBattleground()
{
    const auto mapMgr = sMapMgr.findWorldMap(GetMapId(), GetInstanceID());
    if (mapMgr && mapMgr->getBaseMap()->isBattlegroundOrArena())
    {
        const auto battleground = reinterpret_cast<BattlegroundMap*>(mapMgr)->getBattleground();
        if (battleground->hasEnded() && battleground->hasFreeSlots(getInitialTeam(), battleground->getType()))
        {
            if (!IS_INSTANCE(getBGEntryMapId()))
            {
                m_position.ChangeCoords(getBGEntryPosition());
                m_mapId = getBGEntryMapId();
            }
            else
            {
                m_position.ChangeCoords(getBindPosition());
                m_mapId = getBindMapId();
            }
        }
    }
}

bool Player::logOntoTransport()
{
    bool success = true;
    if (obj_movement_info.transport_guid != 0)
    {
        const auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_guid));
        if (transporter)
        {
            if (isDead())
            {
                resurrect();
                setHealth(getMaxHealth());
                setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
            }

            const float c_tposx = transporter->GetPositionX() + GetTransOffsetX();
            const float c_tposy = transporter->GetPositionY() + GetTransOffsetY();
            const float c_tposz = transporter->GetPositionZ() + GetTransOffsetZ();

            const LocationVector positionOnTransport = LocationVector(c_tposx, c_tposy, c_tposz, GetOrientation());

            if (GetMapId() != transporter->GetMapId())
            {
                SetMapId(transporter->GetMapId());
                sendPacket(AscEmu::Packets::SmsgNewWorld(transporter->GetMapId(), positionOnTransport).serialise().get());

                success = false;
            }

            SetPosition(positionOnTransport.x, positionOnTransport.y, positionOnTransport.z, positionOnTransport.o, false);
            transporter->AddPassenger(this);
        }
    }

    return success;
}

void Player::setLoginPosition()
{
    bool startOnGMIsland = false;
    if (m_session->HasGMPermissions() && m_firstLogin && sWorld.settings.gm.isStartOnGmIslandEnabled)
        startOnGMIsland = true;

    uint32_t mapId = 1;
    float orientation = 0;
    float position_x = 16222.6f;
    float position_y = 16265.9f;
    float position_z = 14.2085f;

    if (startOnGMIsland)
    {
        m_position.ChangeCoords({ position_x, position_y, position_z, orientation });
        m_mapId = mapId;

        setBindPoint(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation(), GetMapId(), getZoneId());
    }
    else
    {
        mapId = GetMapId();
        orientation = GetOrientation();
        position_x = GetPositionX();
        position_y = GetPositionY();
        position_z = GetPositionZ();
    }

    sendLoginVerifyWorldPacket(mapId, position_x, position_y, position_z, orientation);
}

void Player::setPlayerInfoIfNeeded()
{
    m_playerInfo = sObjectMgr.getCachedCharacterInfo(getGuidLow());
    if (m_playerInfo == nullptr)
    {
        auto playerInfo = std::make_unique<CachedCharacterInfo>();
        playerInfo->cl = getClass();
        playerInfo->gender = getGender();
        playerInfo->guid = getGuidLow();
        std::string name = getName();
        AscEmu::Util::Strings::capitalize(name);
        playerInfo->name = name;
        playerInfo->lastLevel = getLevel();
        playerInfo->lastOnline = UNIXTIME;
        playerInfo->lastZone = getZoneId();
        playerInfo->race = getRace();
        playerInfo->team = getTeam();
        playerInfo->guildRank = GUILD_RANK_NONE;
        playerInfo->m_Group = nullptr;
        playerInfo->subGroup = 0;

        m_playerInfo = sObjectMgr.addCachedCharacterInfo(std::move(playerInfo));
    }
}

void Player::setGuildAndGroupInfo()
{
    if (getPlayerInfo()->m_guild)
    {
        if (const auto guild = sGuildMgr.getGuildById(getPlayerInfo()->m_guild))
        {
            setGuildId(getPlayerInfo()->m_guild);
            setGuildRank(getPlayerInfo()->guildRank);
            guild->sendLoginInfo(getSession());
#if VERSION_STRING >= Cata
            setGuildLevel(guild->getLevel());
#endif
        }
    }

    if (getPlayerInfo()->m_Group)
        getPlayerInfo()->m_Group->Update();
}

void Player::sendCinematicOnFirstLogin()
{
    if (m_firstLogin && !worldConfig.player.skipCinematics)
    {
#if VERSION_STRING > TBC
        if (const auto charEntry = sChrClassesStore.lookupEntry(getClass()))
        {
            if (charEntry->cinematic_id != 0)
                sendPacket(SmsgTriggerCinematic(charEntry->cinematic_id).serialise().get());
            else if (const auto raceEntry = sChrRacesStore.lookupEntry(getRace()))
                sendPacket(SmsgTriggerCinematic(raceEntry->cinematic_id).serialise().get());
        }
#else
        if (const auto raceEntry = sChrRacesStore.lookupEntry(getRace()))
            sendPacket(SmsgTriggerCinematic(raceEntry->cinematic_id).serialise().get());
#endif
    }
}

void Player::sendTalentResetConfirmPacket()
{
    m_session->SendPacket(MsgTalentWipeConfirm(getGuid(), calcTalentResetCost(getTalentResetsCount())).serialise().get());
}

void Player::sendPetUnlearnConfirmPacket()
{
    if (getPet() == nullptr)
        return;

#if VERSION_STRING < Mop
    m_session->SendPacket(SmsgPetUnlearnConfirm(getPet()->getGuid(), getPet()->getUntrainCost()).serialise().get());
#else
    m_session->SendPacket(SmsgPetUnlearnConfirm(getPet()->getGuid(), 0).serialise().get());
#endif
}

void Player::sendDungeonDifficultyPacket()
{
    m_session->SendPacket(MsgSetDungeonDifficulty(m_dungeonDifficulty, 1, isInGroup()).serialise().get());
}

void Player::sendRaidDifficultyPacket()
{
#if VERSION_STRING > TBC
    m_session->SendPacket(MsgSetRaidDifficulty(m_raidDifficulty, 1, isInGroup()).serialise().get());
#endif
}

void Player::sendResetFailedNotify(uint32_t mapid)
{
    WorldPacket data(SMSG_RESET_FAILED_NOTIFY, 4);
    data << uint32_t(mapid);
    sendPacket(&data);
}

void Player::sendInstanceDifficultyPacket(uint8_t difficulty)
{
    m_session->SendPacket(SmsgInstanceDifficulty(difficulty).serialise().get());
}

void Player::sendNewDrunkStatePacket(uint32_t state, uint32_t itemId)
{
    sendMessageToSet(SmsgCrossedInebriationThreshold(getGuid(), state, itemId).serialise().get(), true);
}

void Player::sendSetProficiencyPacket(uint8_t itemClass, uint32_t proficiency)
{
    m_session->SendPacket(SmsgSetProficiency(itemClass, proficiency).serialise().get());
}

void Player::sendPartyKillLogPacket(uint64_t killedGuid)
{
    sendMessageToSet(SmsgPartyKillLog(getGuid(), killedGuid).serialise().get(), true);
}

void Player::sendDestroyObjectPacket(uint64_t destroyedGuid)
{
    m_session->SendPacket(SmsgDestroyObject(destroyedGuid).serialise().get());
}

void Player::sendEquipmentSetUseResultPacket(uint8_t result)
{
#if VERSION_STRING > TBC
    m_session->SendPacket(SmsgEquipmentSetUseResult(result).serialise().get());
#endif
}

void Player::sendTotemCreatedPacket(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId)
{
    m_session->SendPacket(SmsgTotemCreated(slot, guid, duration, spellId).serialise().get());
}

void Player::sendPetTameFailure(uint8_t result) const
{
    WorldPacket data(SMSG_PET_TAME_FAILURE, 1);
    data << uint8_t(result);
    m_session->SendPacket(&data);
}

void Player::sendGossipPoiPacket(float posX, float posY, uint32_t icon, uint32_t flags, uint32_t data, std::string name)
{
    m_session->SendPacket(SmsgGossipPoi(flags, posX, posY, icon, data, name).serialise().get());
}

void Player::sendPoiById(uint32_t id)
{
    if (const auto pPoi = sMySQLStore.getPointOfInterest(id))
    {
        const auto loc = (m_session->language > 0) ? sMySQLStore.getLocalizedPointsOfInterest(id, m_session->language) : nullptr;
        const auto name = loc ? loc->iconName : pPoi->iconName;

        sendGossipPoiPacket(pPoi->x, pPoi->y, pPoi->icon, pPoi->flags, pPoi->data, name);
    }
}

void Player::sendStopMirrorTimerPacket(MirrorTimerTypes type)
{
    m_session->SendPacket(SmsgStopMirrorTimer(type).serialise().get());
}

void Player::sendMeetingStoneSetQueuePacket(uint32_t dungeonId, uint8_t status)
{
    m_session->SendPacket(SmsgMeetingstoneSetQueue(dungeonId, status).serialise().get());
}

void Player::sendPlayObjectSoundPacket(uint64_t objectGuid, uint32_t soundId)
{
    sendMessageToSet(SmsgPlayObjectSound(soundId, objectGuid).serialise().get(), true);
}

void Player::sendPlaySoundPacket(uint32_t soundId)
{
    m_session->SendPacket(SmsgPlaySound(soundId).serialise().get());
}

void Player::sendExploreExperiencePacket(uint32_t areaId, uint32_t experience)
{
    m_session->SendPacket(SmsgExplorationExperience(areaId, experience).serialise().get());
}

void Player::sendSpellCooldownEventPacket(uint32_t spellId)
{
    m_session->SendPacket(SmsgCooldownEvent(spellId, getGuid()).serialise().get());
}

#if VERSION_STRING < Cata
void Player::sendSpellModifierPacket(uint8_t spellGroup, uint8_t spellType, int32_t modifier, bool isPct)
{
    if (isPct)
        m_session->SendPacket(SmsgSetPctSpellModifier(spellGroup, spellType, modifier).serialise().get());
    else
        m_session->SendPacket(SmsgSetFlatSpellModifier(spellGroup, spellType, modifier).serialise().get());
}
#else
void Player::sendSpellModifierPacket(uint8_t spellType, std::vector<std::pair<uint8_t, float>> modValues, bool isPct)
{
    if (isPct)
        m_session->SendPacket(SmsgSetPctSpellModifier(spellType, modValues).serialise().get());
    else
        m_session->SendPacket(SmsgSetFlatSpellModifier(spellType, modValues).serialise().get());
}
#endif

void Player::sendLoginVerifyWorldPacket(uint32_t mapId, float posX, float posY, float posZ, float orientation)
{
    m_session->SendPacket(SmsgLoginVerifyWorld(mapId, LocationVector(posX, posY, posZ, orientation)).serialise().get());
}

void Player::sendMountResultPacket(uint32_t result)
{
    m_session->SendPacket(SmsgMountResult(result).serialise().get());
}

void Player::sendDismountResultPacket(uint32_t result)
{
    m_session->SendPacket(SmsgDismountResult(result).serialise().get());
}

void Player::sendCastFailedPacket(uint32_t spellId, uint8_t errorMessage, uint8_t multiCast, uint32_t extra1, uint32_t extra2 /*= 0*/)
{
    m_session->SendPacket(SmsgCastFailed(multiCast, spellId, errorMessage, extra1, extra2).serialise().get());
}

void Player::sendLevelupInfoPacket(uint32_t level, uint32_t hp, uint32_t mana, uint32_t stat0, uint32_t stat1, uint32_t stat2, uint32_t stat3, uint32_t stat4)
{
    m_session->SendPacket(SmsgLevelupInfo(level, hp, mana, stat0, stat1, stat2, stat3, stat4).serialise().get());
}

void Player::sendItemPushResultPacket(bool created, bool recieved, bool sendtoset, uint8_t destbagslot, uint32_t destslot, uint32_t count, uint32_t entry, uint32_t suffix, uint32_t randomprop, uint32_t stack)
{
    if (sendtoset && isInGroup())
        getGroup()->SendPacketToAll(SmsgItemPushResult(getGuid(), recieved, created, destbagslot, destslot, entry, suffix, randomprop, count, stack).serialise().get());
    else
        m_session->SendPacket(SmsgItemPushResult(getGuid(), recieved, created, destbagslot, destslot, entry, suffix, randomprop, count, stack).serialise().get());
}

void Player::sendClientControlPacket(Unit* target, uint8_t allowMove)
{
    sendPacket(SmsgClientControlUpdate(target->GetNewGUID(), allowMove).serialise().get());

    if (target == this)
        setMover(this);
}

void Player::sendGuildMotd()
{
    if (!getGuild())
        return;

    sendPacket(SmsgGuildEvent(GE_MOTD, { getGuild()->getMOTD() }, 0).serialise().get());
}

void Player::sendEquipmentSetList()
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_EQUIPMENT_SET_LIST, 1000);
    getItemInterface()->m_EquipmentSets.FillEquipmentSetListPacket(data);
    m_session->SendPacket(&data);
#endif
}

void Player::sendEquipmentSetSaved(uint32_t setId, uint32_t setGuid)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_EQUIPMENT_SET_SAVED, 12);
    data << uint32_t(setId);
    data << WoWGuid(uint64_t(setGuid));
    m_session->SendPacket(&data);
#endif
}

void Player::sendEmptyPetSpellList()
{
    m_session->SendPacket(SmsgPetSpells(0).serialise().get());
}

void Player::sendInitialWorldstates()
{
#if VERSION_STRING < Cata
    WorldPacket data(SMSG_INIT_WORLD_STATES, 100);
    m_WorldMap->getWorldStatesHandler().BuildInitWorldStatesForZone(m_zoneId, m_areaId, data);
    m_session->SendPacket(&data);
#endif
}

bool Player::isPvpFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_PVP;
#else
    return getUnitFlags() & UNIT_FLAG_PVP;
#endif
}

void Player::setPvpFlag()
{
    stopPvPTimer();
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_PVP);
    addPlayerFlags(PLAYER_FLAG_PVP_TIMER);
#else
    addUnitFlags(UNIT_FLAG_PVP);
#endif

    getSummonInterface()->setPvPFlags(true);

    if (getCombatHandler().isInCombat())
        addPlayerFlags(PLAYER_FLAG_PVP_GUARD_ATTACKABLE);
}

void Player::removePvpFlag()
{
    stopPvPTimer();
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_PVP);
    removePlayerFlags(PLAYER_FLAG_PVP_TIMER);
#else
    removeUnitFlags(UNIT_FLAG_PVP);
#endif

    getSummonInterface()->setPvPFlags(false);
}

bool Player::isFfaPvpFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_FFA_PVP;
#else
    return hasPlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);
#endif
}

void Player::setFfaPvpFlag()
{
    stopPvPTimer();
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_FFA_PVP);
#else
    addPlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);
#endif

    getSummonInterface()->setFFAPvPFlags(true);
}

void Player::removeFfaPvpFlag()
{
    stopPvPTimer();
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_FFA_PVP);
#else
    removePlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);
#endif

    getSummonInterface()->setFFAPvPFlags(false);
}

bool Player::isSanctuaryFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_SANCTUARY;
#elif VERSION_STRING == TBC
    return hasPlayerFlags(PLAYER_FLAG_SANCTUARY);
#elif VERSION_STRING == Classic
    return false;
#endif
}

void Player::setSanctuaryFlag()
{
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_SANCTUARY);
#elif VERSION_STRING == TBC
    addPlayerFlags(PLAYER_FLAG_SANCTUARY);
#endif

    getSummonInterface()->setSanctuaryFlags(true);
}

void Player::removeSanctuaryFlag()
{
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_SANCTUARY);
#elif VERSION_STRING == TBC
    removePlayerFlags(PLAYER_FLAG_SANCTUARY);
#endif

    getSummonInterface()->setSanctuaryFlags(false);
}

void Player::sendPvpCredit(uint32_t honor, uint64_t victimGuid, uint32_t victimRank)
{
    this->sendPacket(SmsgPvpCredit(honor, victimGuid, victimRank).serialise().get());
}

void Player::sendRaidGroupOnly(uint32_t timeInMs, uint32_t type)
{
    this->sendPacket(SmsgRaidGroupOnly(timeInMs, type).serialise().get());
}

void Player::setVisibleItemFields(uint32_t slot, Item* item)
{
    if (item)
    {
        setVisibleItemEntry(slot, item->getVisibleEntry());
#if VERSION_STRING > TBC
        setVisibleItemEnchantment(slot, PERM_ENCHANTMENT_SLOT, item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
        setVisibleItemEnchantment(slot, TEMP_ENCHANTMENT_SLOT, item->getEnchantmentId(TEMP_ENCHANTMENT_SLOT));
#else
        for (uint8_t i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            setVisibleItemEnchantment(slot, i, item->getEnchantmentId(i));
#endif
    }
    else
    {
        setVisibleItemEntry(slot, 0);
#if VERSION_STRING > TBC
        setVisibleItemEnchantment(slot, PERM_ENCHANTMENT_SLOT, 0);
        setVisibleItemEnchantment(slot, TEMP_ENCHANTMENT_SLOT, 0);
#else
        for (uint8_t i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            setVisibleItemEnchantment(slot, i, 0);
#endif
    }
}

#if VERSION_STRING == Cata
void Player::applyReforgeEnchantment(Item* item, bool apply)
{
    if (!item)
        return;

    WDB::Structures::ItemReforgeEntry const* reforge = sItemReforgeStore.lookupEntry(item->getEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
    if (!reforge)
        return;

    auto removeValue = static_cast<int32_t>(item->getReforgableStat(ItemModType(reforge->SourceStat)) * reforge->SourceMultiplier);
    auto addValue = static_cast<int32_t>(removeValue * reforge->FinalMultiplier);

    switch (reforge->SourceStat)
    {
    case ITEM_MOD_MANA:
        modifyBonuses(ITEM_MOD_MANA, -removeValue, apply);
        break;
    case ITEM_MOD_HEALTH:
        modifyBonuses(ITEM_MOD_HEALTH, -removeValue, apply);
        break;
    case ITEM_MOD_AGILITY:
        modifyBonuses(ITEM_MOD_AGILITY, -removeValue, apply);
        break;
    case ITEM_MOD_STRENGTH:
        modifyBonuses(ITEM_MOD_STRENGTH, -removeValue, apply);
        break;
    case ITEM_MOD_INTELLECT:
        modifyBonuses(ITEM_MOD_INTELLECT, -removeValue, apply);
        break;
    case ITEM_MOD_SPIRIT:
        modifyBonuses(ITEM_MOD_SPIRIT, -removeValue, apply);
        break;
    case ITEM_MOD_STAMINA:
        modifyBonuses(ITEM_MOD_STAMINA, -removeValue, apply);
        break;
    case ITEM_MOD_DEFENSE_RATING:
        modifyBonuses(ITEM_MOD_DEFENSE_RATING, -removeValue, apply);
        break;
    case  ITEM_MOD_DODGE_RATING:
        modifyBonuses(ITEM_MOD_DODGE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_PARRY_RATING:
        modifyBonuses(ITEM_MOD_PARRY_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_SHIELD_BLOCK_RATING:
        modifyBonuses(ITEM_MOD_SHIELD_BLOCK_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_MELEE_HIT_RATING:
        modifyBonuses(ITEM_MOD_MELEE_HIT_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_RANGED_HIT_RATING:
        modifyBonuses(ITEM_MOD_RANGED_HIT_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_SPELL_HIT_RATING:
        modifyBonuses(ITEM_MOD_SPELL_HIT_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_SPELL_HASTE_RATING:
        modifyBonuses(ITEM_MOD_SPELL_HASTE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_HIT_RATING:
        modifyBonuses(ITEM_MOD_HIT_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_CRITICAL_STRIKE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_RESILIENCE_RATING:
        modifyBonuses(ITEM_MOD_RESILIENCE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_HASTE_RATING:
        modifyBonuses(ITEM_MOD_HASTE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_EXPERTISE_RATING:
        modifyBonuses(ITEM_MOD_EXPERTISE_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_ATTACK_POWER:
        modifyBonuses(ITEM_MOD_ATTACK_POWER, -removeValue, apply);
        break;
    case ITEM_MOD_RANGED_ATTACK_POWER:
        modifyBonuses(ITEM_MOD_RANGED_ATTACK_POWER, -removeValue, apply);
        break;
    case ITEM_MOD_MANA_REGENERATION:
        modifyBonuses(ITEM_MOD_MANA_REGENERATION , -removeValue, apply);
        break;
    case ITEM_MOD_ARMOR_PENETRATION_RATING:
        modifyBonuses(ITEM_MOD_ARMOR_PENETRATION_RATING, -removeValue, apply);
        break;
    case ITEM_MOD_SPELL_POWER:
        modifyBonuses(ITEM_MOD_SPELL_POWER , -removeValue, apply);
        break;
    /*case ITEM_MOD_HEALTH_REGEN:   // todo dunno where these are handled
        -int32_t(removeValue)
        break;
    case ITEM_MOD_SPELL_PENETRATION:
        -int32_t(removeValue)
        break;
    case ITEM_MOD_BLOCK_VALUE:
        -removeValue
        break;*/
    }

    switch (reforge->FinalStat)
    {
    case ITEM_MOD_MANA:
        modifyBonuses(ITEM_MOD_MANA, addValue, apply);
        break;
    case ITEM_MOD_HEALTH:
        modifyBonuses(ITEM_MOD_HEALTH, addValue, apply);
        break;
    case ITEM_MOD_AGILITY:
        modifyBonuses(ITEM_MOD_AGILITY, addValue, apply);
        break;
    case ITEM_MOD_STRENGTH:
        modifyBonuses(ITEM_MOD_STRENGTH, addValue, apply);
        break;
    case ITEM_MOD_INTELLECT:
        modifyBonuses(ITEM_MOD_INTELLECT, addValue, apply);
        break;
    case ITEM_MOD_SPIRIT:
        modifyBonuses(ITEM_MOD_SPIRIT, addValue, apply);
        break;
    case ITEM_MOD_STAMINA:
        modifyBonuses(ITEM_MOD_STAMINA, addValue, apply);
        break;
    case ITEM_MOD_DEFENSE_RATING:
        modifyBonuses(ITEM_MOD_DEFENSE_RATING, addValue, apply);
        break;
    case  ITEM_MOD_DODGE_RATING:
        modifyBonuses(ITEM_MOD_DODGE_RATING, addValue, apply);
        break;
    case ITEM_MOD_PARRY_RATING:
        modifyBonuses(ITEM_MOD_PARRY_RATING, addValue, apply);
        break;
    case ITEM_MOD_SHIELD_BLOCK_RATING:
        modifyBonuses(ITEM_MOD_SHIELD_BLOCK_RATING, addValue, apply);
        break;
    case ITEM_MOD_MELEE_HIT_RATING:
        modifyBonuses(ITEM_MOD_MELEE_HIT_RATING, addValue, apply);
        break;
    case ITEM_MOD_RANGED_HIT_RATING:
        modifyBonuses(ITEM_MOD_RANGED_HIT_RATING, addValue, apply);
        break;
    case ITEM_MOD_SPELL_HIT_RATING:
        modifyBonuses(ITEM_MOD_SPELL_HIT_RATING, addValue, apply);
        break;
    case ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING, addValue, apply);
        break;
    case ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING, addValue, apply);
        break;
    case ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING, addValue, apply);
        break;
    case ITEM_MOD_SPELL_HASTE_RATING:
        modifyBonuses(ITEM_MOD_SPELL_HASTE_RATING, addValue, apply);
        break;
    case ITEM_MOD_HIT_RATING:
        modifyBonuses(ITEM_MOD_HIT_RATING, addValue, apply);
        break;
    case ITEM_MOD_CRITICAL_STRIKE_RATING:
        modifyBonuses(ITEM_MOD_CRITICAL_STRIKE_RATING, addValue, apply);
        break;
    case ITEM_MOD_RESILIENCE_RATING:
        modifyBonuses(ITEM_MOD_RESILIENCE_RATING, addValue, apply);
        break;
    case ITEM_MOD_HASTE_RATING:
        modifyBonuses(ITEM_MOD_HASTE_RATING, addValue, apply);
        break;
    case ITEM_MOD_EXPERTISE_RATING:
        modifyBonuses(ITEM_MOD_EXPERTISE_RATING, addValue, apply);
        break;
    case ITEM_MOD_ATTACK_POWER:
        modifyBonuses(ITEM_MOD_ATTACK_POWER, addValue, apply);
        break;
    case ITEM_MOD_RANGED_ATTACK_POWER:
        modifyBonuses(ITEM_MOD_RANGED_ATTACK_POWER, addValue, apply);
        break;
    case ITEM_MOD_MANA_REGENERATION:
        modifyBonuses(ITEM_MOD_MANA_REGENERATION, addValue, apply);
        break;
    case ITEM_MOD_ARMOR_PENETRATION_RATING:
        modifyBonuses(CR_ARMOR_PENETRATION, addValue, apply);
        break;
    case ITEM_MOD_SPELL_POWER:
        modifyBonuses(ITEM_MOD_SPELL_POWER, addValue, apply);
        break;
    /*case ITEM_MOD_HEALTH_REGEN:   // todo dunno where these are handled
        int32_t(addValue)
        break;
    case ITEM_MOD_SPELL_PENETRATION:
        int32_t(addValue)
        break;
    case ITEM_MOD_BLOCK_VALUE:
        addValue
        break;*/
    }

    updateStats();
}
#elif VERSION_STRING > Cata
void Player::applyReforgeEnchantment(Item* item, bool apply)
{
    // TODO mop
}
#endif

bool Player::isAtGroupRewardDistance(Object* pRewardSource)
{
    if (!pRewardSource)
        return false;

    Object* player = nullptr;
    const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow());
    if (corpse)
        player = sObjectMgr.getPlayer(static_cast<uint32_t>(corpse->getOwnerGuid()));

    if (!player || isAlive())
        player = this;

    if (player->GetMapId() != pRewardSource->GetMapId() || player->GetInstanceID() != pRewardSource->GetInstanceID())
        return false;

    return pRewardSource->getDistance(player) <= 75.0f;
}

void Player::tagUnit(Object* object)
{
    if (object->isCreatureOrPlayer())
    {
#if VERSION_STRING < Mop
        object->forceBuildUpdateValueForField(getOffsetForStructuredField(WoWUnit, dynamic_flags), this);
#else
        object->forceBuildUpdateValueForField(getOffsetForStructuredField(WoWObject, dynamic_field), this);
#endif
    }
}

#if VERSION_STRING > TBC
AchievementMgr* Player::getAchievementMgr() { return m_achievementMgr.get(); }
#endif

void Player::sendUpdateDataToSet(ByteBuffer* groupBuf, ByteBuffer* nonGroupBuf, bool sendToSelf)
{
    if (groupBuf && nonGroupBuf)
    {
        for (const auto& object : getInRangePlayersSet())
        {
            if (Player* player = static_cast<Player*>(object))
            {
                if (player->getGroup() && getGroup() && player->getGroup()->GetID() == getGroup()->GetID())
                    player->getUpdateMgr().pushUpdateData(groupBuf, 1);
                else
                    player->getUpdateMgr().pushUpdateData(nonGroupBuf, 1);
            }
        }
    }
    else
    {
        if (groupBuf && nonGroupBuf == nullptr)
        {
            for (const auto& object : getInRangePlayersSet())
            {
                if (Player* player = static_cast<Player*>(object))
                    if (player->getGroup() && getGroup() && player->getGroup()->GetID() == getGroup()->GetID())
                        player->getUpdateMgr().pushUpdateData(groupBuf, 1);
            }
        }
        else
        {
            if (groupBuf == nullptr && nonGroupBuf)
            {
                for (const auto& object : getInRangePlayersSet())
                {
                    if (Player* player = static_cast<Player*>(object))
                        if (player->getGroup() == nullptr || player->getGroup()->GetID() != getGroup()->GetID())
                            player->getUpdateMgr().pushUpdateData(nonGroupBuf, 1);
                }
            }
        }
    }

    if (sendToSelf && groupBuf != nullptr)
        getUpdateMgr().pushUpdateData(groupBuf, 1);
}

void Player::sendWorldStateUpdate(uint32_t worldState, uint32_t value)
{
    m_session->SendPacket(SmsgUpdateWorldState(worldState, value).serialise().get());
}

bool Player::canBuyAt(MySQLStructure::VendorRestrictions const* vendor)
{
    if (vendor == nullptr)
        return true;

    if (vendor->flags == RESTRICTION_CHECK_ALL)
    {
        if ((vendor->racemask > 0) && !(getRaceMask() & vendor->racemask))
            return false;

        if ((vendor->classmask > 0) && !(getClassMask() & vendor->classmask))
            return false;

        if (vendor->reqrepfaction)
        {
            uint32_t plrep = getFactionStanding(vendor->reqrepfaction);
            if (plrep < vendor->reqrepvalue)
                return false;
        }
    }
    else if (vendor->flags == RESTRICTION_CHECK_MOUNT_VENDOR)
    {
        if ((vendor->racemask > 0) && (vendor->reqrepfaction))
        {
            uint32_t plrep = getFactionStanding(vendor->reqrepfaction);
            if (!(getRaceMask() & vendor->racemask) && (plrep < vendor->reqrepvalue))
                return false;
        }
        else
        {
            sLogger.failure("VendorRestrictions: Mount vendor specified, but not enough m_playerCreateInfo for creature {}", vendor->entry);
        }
    }

    return true;
}

bool Player::canTrainAt(Trainer const* trainer)
{
    if (!trainer)
        return false;

    if ((trainer->RequiredClass && this->getClass() != trainer->RequiredClass) ||
        ((trainer->RequiredRace && this->getRace() != trainer->RequiredRace) &&
            ((trainer->RequiredRepFaction && trainer->RequiredRepValue) &&
                this->getFactionStanding(trainer->RequiredRepFaction) != static_cast<int32_t>(trainer->RequiredRepValue))) ||
        (trainer->RequiredSkill && !this->hasSkillLine(trainer->RequiredSkill)) ||
        (trainer->RequiredSkillLine && this->getSkillLineCurrent(trainer->RequiredSkill) < trainer->RequiredSkillLine))
    {
        return false;
    }

    return true;
}

void Player::sendCinematicCamera(uint32_t id)
{
    m_WorldMap->changeObjectLocation(this);
    SetPosition(float(GetPositionX() + 0.01), float(GetPositionY() + 0.01), float(GetPositionZ() + 0.01), GetOrientation());
    m_session->SendPacket(SmsgTriggerCinematic(id).serialise().get());
}

void Player::setMover(Unit* target)
{
    m_session->m_MoverWoWGuid.Init(target->getGuid());
    m_controledUnit = target;

#if VERSION_STRING > WotLK
    ObjectGuid guid = target->getGuid();

    WorldPacket data(SMSG_MOVE_SET_ACTIVE_MOVER, 9);
    data.writeBit(guid[5]);
    data.writeBit(guid[7]);
    data.writeBit(guid[3]);
    data.writeBit(guid[6]);
    data.writeBit(guid[0]);
    data.writeBit(guid[4]);
    data.writeBit(guid[1]);
    data.writeBit(guid[2]);

    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[4]);

    sendPacket(&data);
#endif
}

void Player::resetTimeSync()
{
    m_timeSyncCounter = 0;
    m_timeSyncTimer = 0;
    m_timeSyncClient = 0;
    m_timeSyncServer = Util::getMSTime();
}

void Player::sendTimeSync()
{
    getSession()->SendPacket(SmsgTimeSyncRequest(m_timeSyncCounter++).serialise().get());

    // Schedule next sync in 10 sec
    m_timeSyncTimer = 10000;
    m_timeSyncServer = Util::getMSTime();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Void Storage
#if VERSION_STRING > WotLK
void Player::loadVoidStorage()
{
    auto result = CharacterDatabase.Query("SELECT itemid, itemEntry, slot, creatorGuid, randomProperty, suffixFactor FROM character_void_storage WHERE playerGuid = %u", getGuidLow());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint64_t itemId = fields[0].asUint64();
        uint32_t itemEntry = fields[1].asUint32();
        uint8_t slot = fields[2].asUint8();
        uint32_t creatorGuid = fields[3].asUint32();
        uint32_t randomProperty = fields[4].asUint32();
        uint32_t suffixFactor = fields[5].asUint32();

        if (!itemId)
        {
            sLogger.debug("Player::loadVoidStorage - Player (GUID: {}, name: {}) has an item with an invalid id (item id: %I64u, entry: {}).", getGuidLow(), getName(), itemId, itemEntry);
            continue;
        }

        if (!sMySQLStore.getItemProperties(itemEntry))
        {
            sLogger.debug("Player::loadVoidStorage - Player (GUID: {}, name: {}) has an item with an invalid entry (item id: %I64u, entry: {}).", getGuidLow(), getName(), itemId, itemEntry);
            continue;
        }

        if (slot >= VOID_STORAGE_MAX_SLOT)
        {
            sLogger.debug("Player::loadVoidStorage - Player (GUID: {}, name: {}) has an item with an invalid slot (item id: %I64u, entry: {}, slot: {}).", getGuidLow(), getName(), itemId, itemEntry, slot);
            continue;
        }

        if (!sObjectMgr.getPlayer(creatorGuid))
        {
            sLogger.debug("Player::loadVoidStorage - Player (GUID: {}, name: {}) has an item with an invalid creator guid, set to 0 (item id: %I64u, entry: {}, creatorGuid: {}).", getGuidLow(), getName(), itemId, itemEntry, creatorGuid);
            creatorGuid = 0;
        }

        _voidStorageItems[slot] = std::make_unique<VoidStorageItem>(itemId, itemEntry, creatorGuid, randomProperty, suffixFactor);
    } while (result->NextRow());
}

void Player::saveVoidStorage()
{
    uint32_t lowGuid = getGuidLow();

    for (uint8_t slot = 0; slot < VOID_STORAGE_MAX_SLOT; ++slot)
    {
        if (!_voidStorageItems[slot]) // unused item
        {
            // DELETE FROM void_storage WHERE slot = ? AND playerGuid = ?
            CharacterDatabase.Execute("DELETE FROM character_void_storage WHERE playerGuid = %u AND slot = %u ", lowGuid, slot);
        }
        else
        {
            std::stringstream ss;
            ss << "REPLACE INTO character_void_storage VALUES(";
            ss << _voidStorageItems[slot]->itemId << ",";
            ss << lowGuid << ",";
            ss << uint32_t(_voidStorageItems[slot]->itemEntry) << ",";
            ss << int(slot) << ",";
            ss << _voidStorageItems[slot]->creatorGuid << ",";
            ss << _voidStorageItems[slot]->itemRandomPropertyId << ",";
            ss << _voidStorageItems[slot]->itemSuffixFactor;
            ss << ")";
            CharacterDatabase.Execute(ss.str().c_str());
        }
    }
}

bool Player::isVoidStorageUnlocked() const { return hasPlayerFlags(PLAYER_FLAG_VOID_STORAGE_UNLOCKED); }
void Player::unlockVoidStorage() { addPlayerFlags(PLAYER_FLAG_VOID_STORAGE_UNLOCKED); }
void Player::lockVoidStorage() { removePlayerFlags(PLAYER_FLAG_VOID_STORAGE_UNLOCKED); }

uint8_t Player::getNextVoidStorageFreeSlot() const
{
    for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
        if (!_voidStorageItems[i]) // unused item
            return i;

    return VOID_STORAGE_MAX_SLOT;
}

uint8_t Player::getNumOfVoidStorageFreeSlots() const
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
        if (!_voidStorageItems[i])
            count++;

    return count;
}

uint8_t Player::addVoidStorageItem(const VoidStorageItem& item)
{
    int8_t slot = getNextVoidStorageFreeSlot();

    if (slot >= VOID_STORAGE_MAX_SLOT)
    {
        getSession()->sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_FULL);
        return 255;
    }

    _voidStorageItems[slot] = std::make_unique<VoidStorageItem>(item.itemId, item.itemEntry,
        item.creatorGuid, item.itemRandomPropertyId, item.itemSuffixFactor);
    return slot;
}

void Player::addVoidStorageItemAtSlot(uint8_t slot, const VoidStorageItem& item)
{
    if (slot >= VOID_STORAGE_MAX_SLOT)
    {
        getSession()->sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_FULL);
        return;
    }

    if (_voidStorageItems[slot])
    {
        sLogger.debug("Player::addVoidStorageItemAtSlot - Player (GUID: {}, name: {}) tried to add an item to an used slot (item id: {}, entry: {}, slot: {}).", getGuidLow(), getName(), _voidStorageItems[slot]->itemId, _voidStorageItems[slot]->itemEntry, slot);
        getSession()->sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    _voidStorageItems[slot] = std::make_unique<VoidStorageItem>(item.itemId, item.itemEntry,
        item.creatorGuid, item.itemRandomPropertyId, item.itemSuffixFactor);
}

void Player::deleteVoidStorageItem(uint8_t slot)
{
    if (slot >= VOID_STORAGE_MAX_SLOT)
    {
        getSession()->sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    _voidStorageItems[slot] = nullptr;
}

bool Player::swapVoidStorageItem(uint8_t oldSlot, uint8_t newSlot)
{
    if (oldSlot >= VOID_STORAGE_MAX_SLOT || newSlot >= VOID_STORAGE_MAX_SLOT || oldSlot == newSlot)
        return false;

    std::swap(_voidStorageItems[newSlot], _voidStorageItems[oldSlot]);
    return true;
}

VoidStorageItem* Player::getVoidStorageItem(uint8_t slot) const
{
    if (slot >= VOID_STORAGE_MAX_SLOT)
    {
        getSession()->sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return nullptr;
    }

    return _voidStorageItems[slot] != nullptr ? _voidStorageItems[slot].get() : nullptr;
}

VoidStorageItem* Player::getVoidStorageItem(uint64_t id, uint8_t& slot) const
{
    for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
    {
        if (_voidStorageItems[i] && _voidStorageItems[i]->itemId == id)
        {
            slot = i;
            return _voidStorageItems[i].get();
        }
    }

    return nullptr;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Taxi
bool Player::activateTaxiPathTo(std::vector<uint32_t> const& nodes, Creature* npc /*= nullptr*/, uint32_t spellid /*= 0*/)
{
    if (nodes.size() < 2)
        return false;

    // not let cheating with start flight in time of logout process || while in combat || has type state: stunned || has type state: root
    if (getSession()->IsLoggingOut() || isInCombat() || hasUnitStateFlag(UNIT_STATE_STUNNED) || hasUnitStateFlag(UNIT_STATE_ROOTED))
    {
        getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiPlayerBusy).serialise().get());
        return false;
    }

    if (hasUnitFlags(UNIT_FLAG_LOCK_PLAYER))
        return false;

    // taximaster case
    if (npc)
    {
        // not let cheating with start flight mounted
        if (isMounted())
        {
            getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiPlayerAlreadyMounted).serialise().get());
            return false;
        }

        if (isInDisallowedMountForm())
        {
            getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiPlayerShapeshifted).serialise().get());;
            return false;
        }
    }
    // cast case or scripted call case
    else
    {
        removeAllAurasByAuraEffect(SPELL_AURA_MOUNTED);

        if (isInDisallowedMountForm())
            removeAllAurasByAuraEffect(SPELL_AURA_MOD_SHAPESHIFT);

        if (Spell* spell = getCurrentSpell(CURRENT_GENERIC_SPELL))
            if (spell->getSpellInfo()->getId() != spellid)
                interruptSpell(CURRENT_GENERIC_SPELL, false);

        interruptSpell(CURRENT_AUTOREPEAT_SPELL, false);

        if (Spell* spell = getCurrentSpell(CURRENT_CHANNELED_SPELL))
            if (spell->getSpellInfo()->getId() != spellid)
                interruptSpell(CURRENT_CHANNELED_SPELL, true);
    }

    uint32_t sourcenode = nodes[0];

    // starting node too far away (cheat?)
    WDB::Structures::TaxiNodesEntry const* node = sTaxiNodesStore.lookupEntry(sourcenode);
    if (!node)
    {
        getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_NoDirectPath).serialise().get());
        return false;
    }

    // Prepare to flight start now
#if VERSION_STRING > TBC
    exitVehicle();
#endif

    // stop trade (client cancel trade at taxi map open but cheating tools can be used for reopen it)
    cancelTrade(true);

    // clean not finished taxi path if any
    m_taxi->clearTaxiDestinations();

    // 0 element current node
    m_taxi->addTaxiDestination(sourcenode);

    // fill destinations path tail
    uint32_t sourcepath = 0;
    uint32_t totalcost = 0;
    uint32_t firstcost = 0;

    uint32_t prevnode = sourcenode;
    uint32_t lastnode;

    for (uint32_t i = 1; i < nodes.size(); ++i)
    {
        uint32_t path, cost;

        lastnode = nodes[i];
        sTaxiMgr.getTaxiPath(prevnode, lastnode, path, cost);

        if (!path)
        {
            m_taxi->clearTaxiDestinations();
            return false;
        }

        totalcost += cost;
        if (i == 1)
            firstcost = cost;

        if (prevnode == sourcenode)
            sourcepath = path;

        m_taxi->addTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    // get mount model (in case non taximaster (npc == nullptr) allow more wide lookup)
    uint32_t mount_display_id = sTaxiMgr.getTaxiMountDisplayId(sourcenode, GetTeam(), npc == nullptr || (sourcenode == 315 && getClass() == DEATHKNIGHT));

    // in spell case allow 0 model
    if ((mount_display_id == 0 && spellid == 0) || sourcepath == 0)
    {
        getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_UnspecificError).serialise().get());
        m_taxi->clearTaxiDestinations();
        return false;
    }

    uint64_t money = getCoinage();

    if (npc)
    {
        // Disocunting todo
        float discount = 1.0f;
        totalcost = uint32_t(ceil(totalcost * discount));
        firstcost = uint32_t(round(firstcost * discount));
    }

    if (money < totalcost)
    {
        getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_NotEnoughMoney).serialise().get());
        m_taxi->clearTaxiDestinations();
        return false;
    }

    //Checks and preparations done, DO FLIGHT
#if VERSION_STRING > TBC
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN, 1);
#endif

    // prevent stealth flight
    modCoinage(-(int64_t)firstcost);
#if VERSION_STRING > TBC
    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, firstcost);
#endif
    getSession()->SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_Ok).serialise().get());
    getSession()->sendDoFlight(mount_display_id, sourcepath);
    return true;
}

bool Player::activateTaxiPathTo(uint32_t taxi_path_id, uint32_t spellid /*= 0*/)
{
    WDB::Structures::TaxiPathEntry const* entry = sTaxiPathStore.lookupEntry(taxi_path_id);
    if (!entry)
        return false;

    std::vector<uint32_t> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    return activateTaxiPathTo(nodes, nullptr, spellid);
}

bool Player::activateTaxiPathTo(uint32_t taxi_path_id, Creature* npc)
{
    WDB::Structures::TaxiPathEntry const* entry = sTaxiPathStore.lookupEntry(taxi_path_id);
    if (!entry)
        return false;

    std::vector<uint32_t> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    return activateTaxiPathTo(nodes, npc);
}

void Player::cleanupAfterTaxiFlight()
{
    m_taxi->clearTaxiDestinations();        // not destinations, clear source node
    dismount();
    removeUnitFlags(UNIT_FLAG_LOCK_PLAYER | UNIT_FLAG_MOUNTED_TAXI);
}

void Player::continueTaxiFlight() const
{
    uint32_t sourceNode = m_taxi->getTaxiSource();
    if (!sourceNode)
        return;

    uint32_t mountDisplayId = sTaxiMgr.getTaxiMountDisplayId(sourceNode, getTeam(), true);
    if (!mountDisplayId)
        return;

    uint32_t path = m_taxi->getCurrentTaxiPath();

    // search appropriate start path node
    uint32_t startNode = m_taxi->nodeAfterTeleport;

    TaxiPathNodeList const& nodeList = sTaxiPathNodesByPath[path];

    float distPrev;
    float distNext = getExactDistSq(nodeList[0]->x, nodeList[0]->y, nodeList[0]->z);

    for (uint32_t i = 1; i < nodeList.size(); ++i)
    {
        WDB::Structures::TaxiPathNodeEntry const* node = nodeList[i];
        WDB::Structures::TaxiPathNodeEntry const* prevNode = nodeList[i - 1];

        // skip nodes at another map
        if (node->mapid != GetMapId())
            continue;

        distPrev = distNext;

        distNext = getExactDistSq(node->x, node->y, node->z);

        float distNodes =
            (node->x - prevNode->x) * (node->x - prevNode->x) +
            (node->y - prevNode->y) * (node->y - prevNode->y) +
            (node->z - prevNode->z) * (node->z - prevNode->z);

        if (distNext + distPrev < distNodes)
        {
            startNode = i;
            break;
        }
    }

    getSession()->sendDoFlight(mountDisplayId, path, startNode);
}

void Player::sendTaxiNodeStatusMultiple()
{
    for (const auto& itr : getInRangeObjectsSet())
    {
        if (!itr->isCreature())
            continue;

        Creature* creature = itr->ToCreature();
        if (!creature || creature->isHostileTo(this))
            continue;

        if (!creature->isTaxi())
            continue;

        const auto nearestNode = sTaxiMgr.getNearestTaxiNode(creature->GetPosition(), creature->GetMapId(), GetTeam());
        if (nearestNode == 0)
            continue;

        getSession()->SendPacket(SmsgTaxinodeStatus(creature->getGuid(), m_taxi->isTaximaskNodeKnown(nearestNode)).serialise().get());
    }
}

bool Player::isInFlight() const
{
    return hasUnitStateFlag(UNIT_STATE_IN_FLIGHT);
}

bool Player::isOnTaxi() const
{
    return !m_taxi->empty();
}

void Player::initTaxiNodesForLevel()
{
    m_taxi->initTaxiNodesForLevel(getRace(), getClass(), getLevel());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Loot
const uint64_t& Player::getLootGuid() const { return m_lootGuid; }
void Player::setLootGuid(const uint64_t& guid) { m_lootGuid = guid; }

//\note: Types 1 corpse/go; 2 skinning/herbalism/minning; 3 fishing
void Player::sendLoot(uint64_t guid, uint8_t loot_type, uint32_t mapId)
{
    if (!IsInWorld())
        return;

    Loot* pLoot = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    if (wowGuid.isUnit())
    {
        Creature* pCreature = getWorldMap()->getCreature(wowGuid.getGuidLowPart());
        if (!pCreature)return;
        pLoot = &pCreature->loot;
        m_currentLoot = pCreature->getGuid();

    }
    else if (wowGuid.isGameObject())
    {
        GameObject* go = getWorldMap()->getGameObject(wowGuid.getGuidLowPart());

        if (!go)
        {
            SmsgLootReleaseResponse(guid, 1);
            return;
        }

        if (loot_type == LOOT_SKINNING)
        {
            // Disarm Trap
            if (!go->IsWithinDistInMap(this, 20.f))
            {
                SmsgLootReleaseResponse(guid, 1);
                return;
            }
        }
        else
        {
            if (loot_type != LOOT_FISHINGHOLE && ((loot_type != LOOT_FISHING && loot_type != LOOT_FISHING_JUNK) || go->getCreatedByGuid() != getGuid()) && !go->IsWithinDistInMap(this, 30.0f))
            {
                SmsgLootReleaseResponse(guid, 1);
                return;
            }

            if (loot_type == LOOT_CORPSE && go->getRespawnTime() && go->isSpawnedByDefault())
            {
                SmsgLootReleaseResponse(guid, 1);
                return;
            }
        }

        GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(go);
        pLoot = &pLGO->loot;

        // loot was generated and respawntime has passed since then, allow to recreate loot
        // to avoid bugs, this rule covers spawned gameobjects only
        // Don't allow to regenerate chest loot inside instances and raids
        if (go->isSpawnedByDefault() && go->getLootState() == GO_ACTIVATED && !pLGO->loot.isLooted() && !go->getWorldMap()->getBaseMap()->isInstanceableMap() && pLGO->getLootGenerationTime() + go->getRespawnDelay() < Util::getTimeNow())
            go->setLootState(GO_READY);

        if (go->getLootState() == GO_READY)
        {
            uint32_t lootid = go->GetGameObjectProperties()->getLootId();
            if (lootid)
            {
                pLoot->clear();

                auto group = getGroup();
                bool groupRules = (group && go->GetGameObjectProperties()->type == GAMEOBJECT_TYPE_CHEST && go->GetGameObjectProperties()->chest.group_loot_rules);

                // check current RR player and get next if necessary
                if (groupRules)
                    group->updateLooterGuid(go);

                pLoot->fillLoot(lootid, sLootMgr.getGameobjectLoot(), this, false, static_cast<uint8_t>(pLGO->getLootMode()));
                pLGO->setLootGenerationTime();

                // get next RR player (for next loot)
                if (groupRules && !pLoot->empty())
                    group->updateLooterGuid(go);
            }

            if (loot_type == LOOT_FISHING || loot_type == LOOT_FISHING_JUNK)
                pLGO->getFishLoot(this, loot_type == LOOT_FISHING_JUNK);

            go->setLootState(GO_ACTIVATED, this);

            // set Current Looter
            m_currentLoot = pLGO->getGuid();
        }
    }
    else if (wowGuid.isPlayer())
    {
        Player* p = getWorldMap()->getPlayer((uint32_t)guid);
        if (!p)
            return;

        pLoot = &p->loot;
        m_currentLoot = p->getGuid();
    }
    else if (wowGuid.isCorpse())
    {
        if (const auto corpse = sObjectMgr.getCorpseByGuid(static_cast<uint32_t>(guid)))
        {
            pLoot = &corpse->loot;
            m_currentLoot = corpse->getGuid();
        }
    }
    else if (wowGuid.isItem())
    {
        Item* pItem = getItemInterface()->GetItemByGUID(guid);
        if (!pItem)
            return;
        pLoot = pItem->m_loot.get();
        m_currentLoot = pItem->getGuid();
    }

    if (!pLoot)
    {
        // something whack happened.. damn cheaters..
        return;
    }

    // add to looter set
    pLoot->addLooter(getGuidLow());

    // Group case
    PartyLootMethod loot_method;

    // Send Roll packets
    if (getGroup())
    {
        loot_method = PartyLootMethod(getGroup()->GetMethod());

        switch (loot_method)
        {
        case PARTY_LOOT_GROUP:
            getGroup()->sendGroupLoot(pLoot, getWorldMap()->getObject(m_currentLoot), this, mapId);
            break;
        case PARTY_LOOT_NEED_BEFORE_GREED:
        case PARTY_LOOT_MASTER_LOOTER:
        case PARTY_LOOT_FREE_FOR_ALL:
        case PARTY_LOOT_ROUND_ROBIN:
            break;
        }
    }
    else
    {
        loot_method = PARTY_LOOT_FREE_FOR_ALL;
    }

    m_lootGuid = guid;

    WorldPacket data;
    data.SetOpcode(SMSG_LOOT_RESPONSE);
    data << uint64_t(guid);
    data << uint8_t(loot_type);     //loot_type;


    data << uint32_t(pLoot->gold);  // gold
    data << uint8_t(0);             //loot size reserve
#if VERSION_STRING >= Cata
    data << uint8_t(0);             // currency count reserve
#endif

    uint32_t maxItemsCount = 0;

    // Non Personal Items
    auto item = pLoot->items.begin();
    for (uint32_t nonpersonalItemsCount = 0; item != pLoot->items.end(); ++item, nonpersonalItemsCount++)
    {
        if (item->is_looted)
            continue;

        if (item->is_ffa)
            continue;

        uint8_t slottype = LOOT_SLOT_TYPE_ALLOW_LOOT;
        if (loot_type < 2)
        {
            switch (loot_method)
            {
            case PARTY_LOOT_MASTER_LOOTER:
            {
                if (!item->is_looted && !item->is_ffa && item->isAllowedForPlayer(this))
                    slottype = LOOT_SLOT_TYPE_MASTER;
                else
                    // dont show item
                    continue;
            }
            break;
            case PARTY_LOOT_NEED_BEFORE_GREED:
            case PARTY_LOOT_GROUP:
            {
                if (item->is_blocked)
                    slottype = LOOT_SLOT_TYPE_ROLL_ONGOING;
                else if (pLoot->roundRobinPlayer == 0 || !item->is_underthreshold || getGuid() == pLoot->roundRobinPlayer)
                    slottype = LOOT_SLOT_TYPE_ALLOW_LOOT;
                else
                    // dont show Item.
                    continue;
            }
            break;
            case PARTY_LOOT_ROUND_ROBIN:
            {
                if (!item->is_looted && !item->is_ffa && item->isAllowedForPlayer(this))
                {
                    if (pLoot->roundRobinPlayer != 0 && getGuid() != pLoot->roundRobinPlayer)
                        // dont show Item.
                        continue;
                }
            }
            break;
            default:
                slottype = LOOT_SLOT_TYPE_ALLOW_LOOT;
                break;
            }
        }

        data << uint8_t(nonpersonalItemsCount);
        data << uint32_t(item->itemproto->ItemId);
        data << uint32_t(item->count);  //nr of items of this type
        data << uint32_t(item->itemproto->DisplayInfoID);

        if (item->iRandomSuffix)
        {
            data << uint32_t(Item::generateRandomSuffixFactor(item->itemproto));
            data << uint32_t(-int32_t(item->iRandomSuffix->id));
        }
        else if (item->iRandomProperty)
        {
            data << uint32_t(0);
            data << uint32_t(item->iRandomProperty->ID);
        }
        else
        {
            data << uint32_t(0);
            data << uint32_t(0);
        }

        data << slottype;   // "still being rolled for" flag

        maxItemsCount++;
    }

    uint32_t personalItemsCount = maxItemsCount;

    // Quest Loot
    PersonaltemMap const& lootPlayerQuestItems = pLoot->getPlayerQuestItems();
    PersonaltemMap::const_iterator q_itr = lootPlayerQuestItems.find(getGuidLow());
    if (q_itr != lootPlayerQuestItems.end())
    {
        const auto& q_list = q_itr->second;
        for (auto qi = q_list->cbegin(); qi != q_list->cend(); ++qi, personalItemsCount++)
        {
            uint8_t slottype = LOOT_SLOT_TYPE_ALLOW_LOOT;

            LootItem& questItem = pLoot->quest_items[qi->index];
            if (!qi->is_looted && !questItem.is_looted && questItem.isAllowedForPlayer(this))
            {
                data << uint8_t(pLoot->items.size() + (qi - q_list->cbegin()));
                data << uint32_t(questItem.itemproto->ItemId);
                data << uint32_t(questItem.count);  //nr of items of this type
                data << uint32_t(questItem.itemproto->DisplayInfoID);

                if (questItem.iRandomSuffix)
                {
                    data << uint32_t(Item::generateRandomSuffixFactor(questItem.itemproto));
                    data << uint32_t(-int32_t(questItem.iRandomSuffix->id));
                }
                else if (questItem.iRandomProperty)
                {
                    data << uint32_t(0);
                    data << uint32_t(questItem.iRandomProperty->ID);
                }
                else
                {
                    data << uint32_t(0);
                    data << uint32_t(0);
                }

                data << slottype;   // "still being rolled for" flag
            }
            maxItemsCount++;
        }
    }

    uint32_t ffaItemsCount = maxItemsCount;

    // Free for All
    PersonaltemMap const& lootPlayerFFAItems = pLoot->getPlayerFFAItems();
    PersonaltemMap::const_iterator ffa_itr = lootPlayerFFAItems.find(getGuidLow());
    if (ffa_itr != lootPlayerFFAItems.end())
    {
        const auto& ffa_list = ffa_itr->second;
        for (auto fi = ffa_list->cbegin(); fi != ffa_list->cend(); ++fi, ffaItemsCount++)
        {
            uint8_t slottype = LOOT_SLOT_TYPE_ALLOW_LOOT;

            LootItem& ffaItem = pLoot->items[fi->index];
            if (!fi->is_looted && !ffaItem.is_looted && ffaItem.isAllowedForPlayer(this))
            {
                data << uint8_t(fi->index);
                data << uint32_t(ffaItem.itemproto->ItemId);
                data << uint32_t(ffaItem.count);  //nr of items of this type
                data << uint32_t(ffaItem.itemproto->DisplayInfoID);

                if (ffaItem.iRandomSuffix)
                {
                    data << uint32_t(Item::generateRandomSuffixFactor(ffaItem.itemproto));
                    data << uint32_t(-int32_t(ffaItem.iRandomSuffix->id));
                }
                else if (ffaItem.iRandomProperty)
                {
                    data << uint32_t(0);
                    data << uint32_t(ffaItem.iRandomProperty->ID);
                }
                else
                {
                    data << uint32_t(0);
                    data << uint32_t(0);
                }

                data << slottype;   // "still being rolled for" flag
            }
            maxItemsCount++;
        }
    }

    data.wpos(13);
    data << uint8_t(maxItemsCount);

    m_session->SendPacket(&data);

    addUnitFlags(UNIT_FLAG_LOOTING);
}

void Player::sendLootUpdate(Object* object)
{
    if (!isVisibleObject(object->getGuid()))
        return;

    if (object->isCreatureOrPlayer())
    {
        // Build the actual update.
        ByteBuffer buffer(500);

        uint32_t flags = dynamic_cast<Unit*>(object)->getDynamicFlags();

        flags |= U_DYN_FLAG_LOOTABLE;
        flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

#if VERSION_STRING < Mop
        object->BuildFieldUpdatePacket(&buffer, getOffsetForStructuredField(WoWUnit, dynamic_flags), flags);
#else
        object->BuildFieldUpdatePacket(&buffer, getOffsetForStructuredField(WoWObject, dynamic_field), flags);
#endif

        getUpdateMgr().pushUpdateData(&buffer, 1);
    }
}

void Player::sendLooter(Creature* creature)
{
    WorldPacket data(SMSG_LOOT_LIST, 8 + 1 + 1);
    data << uint64_t(creature->getGuid());
    data << uint8_t(0); // unk1
    data << uint8_t(0); // no group looter
    sendMessageToSet(&data, true);
}

Item* Player::storeNewLootItem(uint8_t slot, Loot* _loot)
{
    Personaltem* questItem = nullptr;
    Personaltem* ffaItem = nullptr;

    // Pick our loot from Loot Store
    LootItem* item = _loot->lootItemInSlot(slot, this, &questItem, &ffaItem);

    if (!item)
    {
        getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
        return nullptr;
    }

    // questitems use the blocked field for other purposes
    if (!questItem && item->is_blocked)
    {
        sendPacket(SmsgLootReleaseResponse(getLootGuid(), 1).serialise().get());
        return nullptr;
    }

    // Add our Item
    Item* newItem = storeItem(item);
    if (!newItem)
        return nullptr;

    if (questItem)
    {
        questItem->is_looted = true;
        //freeforall is 1 if everyone's supposed to get the quest item.
        if (item->is_ffa || _loot->getPlayerQuestItems().size() == 1)
            getSession()->SendPacket(SmsgLootRemoved(slot).serialise().get());
        else
            _loot->itemRemoved(questItem->index);
    }
    else
    {
        if (ffaItem)
        {
            //freeforall case, notify only one player of the removal
            ffaItem->is_looted = true;
            getSession()->SendPacket(SmsgLootRemoved(slot).serialise().get());
        }
        else
        {
            _loot->itemRemoved(slot);
        }
    }

    //if only one person is supposed to loot the item, then set it to looted
    if (!item->is_ffa)
        item->is_looted = true;

    --_loot->unlootedCount;

    return newItem;
}

Item* Player::storeItem(LootItem const* lootItem)
{
    auto add = getItemInterface()->FindItemLessMax(lootItem->itemId, lootItem->count, false);

    // Can we Store our New item?
    if (const uint8_t error = getItemInterface()->CanReceiveItem(lootItem->itemproto, lootItem->count) && !add)
    {
        getItemInterface()->buildInventoryChangeError(nullptr, nullptr, error, lootItem->itemId);
        return nullptr;
    }

    const auto slotResult = getItemInterface()->FindFreeInventorySlot(lootItem->itemproto);
    if (!slotResult.Result && !add)
    {
        getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        return nullptr;
    }

    // Players which should be able to receive the item after its looted while tradeable
    LooterSet looters = lootItem->getAllowedLooters();

    if (add == nullptr)
    {
        // Create the Item
        auto newItemHolder = sObjectMgr.createItem(lootItem->itemId, this);
        if (newItemHolder == nullptr)
            return nullptr;

        auto* newItem = newItemHolder.get();
        newItem->setStackCount(lootItem->count);
        newItem->setOwnerGuid(getGuid());

        if (lootItem->iRandomProperty != nullptr)
        {
            newItem->setRandomPropertiesId(lootItem->iRandomProperty->ID);
            newItem->applyRandomProperties(false);
        }
        else if (lootItem->iRandomSuffix != nullptr)
        {
            newItem->setRandomSuffix(lootItem->iRandomSuffix->id);
            newItem->applyRandomProperties(false);
        }

        const auto [addResult, _] = getItemInterface()->SafeAddItem(std::move(newItemHolder), slotResult.ContainerSlot, slotResult.Slot);
        if (addResult)
        {
            sendItemPushResultPacket(false, true, true, slotResult.ContainerSlot, slotResult.Slot, lootItem->count, newItem->getEntry(), newItem->getPropertySeed(), newItem->getRandomPropertiesId(), newItem->getStackCount());
            sQuestMgr.OnPlayerItemPickup(this, newItem);
#if VERSION_STRING > TBC
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, newItem->getEntry(), lootItem->count, 0);
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE, newItem->getEntry(), lootItem->count, 0);
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM, newItem->getEntry(), lootItem->count);
#endif
        }
        else
        {
            return nullptr;
        }

#if VERSION_STRING >= WotLK
        // Soulbound Tradeable
        if (looters.size() > 1 && lootItem->itemproto->MaxCount == 1 && newItem->isSoulbound())
        {
            newItem->setSoulboundTradeable(looters);

            uint32_t* played = getPlayedTime();
            newItem->setCreatePlayedTime(played[1]);
            getItemInterface()->addTradeableItem(newItem);
        }
#endif

        return newItem;
    }
    else
    {
        add->setStackCount(add->getStackCount() + lootItem->count);
        add->m_isDirty = true;
        add->setOwnerGuid(getGuid());

        sendItemPushResultPacket(false, true, true, slotResult.ContainerSlot, slotResult.Slot, lootItem->count, add->getEntry(), add->getPropertySeed(), add->getRandomPropertiesId(), add->getStackCount());
        sQuestMgr.OnPlayerItemPickup(this, add);
#if VERSION_STRING > TBC
        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, add->getEntry(), 1, 0);
#endif
        return add;
    }
}

bool Player::isLootableOnCorpse() const { return m_lootableOnCorpse; }
void Player::setLootableOnCorpse(bool lootable) { m_lootableOnCorpse = lootable; }

/////////////////////////////////////////////////////////////////////////////////////////
// Reputation
inline bool CanToggleAtWar(uint8_t flag) { return (flag & FACTION_FLAG_DISABLE_ATWAR) == 0; }
inline bool AtWar(uint8_t flag) { return (flag & FACTION_FLAG_AT_WAR) != 0; }
inline bool ForcedInvisible(uint8_t flag) { return (flag & FACTION_FLAG_FORCED_INVISIBLE) != 0; }
inline bool Visible(uint8_t flag) { return (flag & FACTION_FLAG_VISIBLE) != 0; }
inline bool Hidden(uint8_t flag) { return (flag & FACTION_FLAG_HIDDEN) != 0; }
inline bool Inactive(uint8_t flag) { return (flag & FACTION_FLAG_INACTIVE) != 0; }

inline bool SetFlagAtWar(uint8_t& flag, bool set)
{
    if (set && !AtWar(flag))
        flag |= FACTION_FLAG_AT_WAR;
    else if (!set && AtWar(flag))
        flag &= ~FACTION_FLAG_AT_WAR;
    else
        return false;

    return true;
}

inline bool SetFlagVisible(uint8_t& flag, bool set)
{
    if (ForcedInvisible(flag) || Hidden(flag))
        return false;
    else if (set && !Visible(flag))
        flag |= FACTION_FLAG_VISIBLE;
    else if (!set && Visible(flag))
        flag &= ~FACTION_FLAG_VISIBLE;
    else
        return false;

    return true;
}

inline bool SetFlagInactive(uint8_t& flag, bool set)
{
    if (set && !Inactive(flag))
        flag |= FACTION_FLAG_INACTIVE;
    else if (!set && Inactive(flag))
        flag &= ~FACTION_FLAG_INACTIVE;
    else
        return false;

    return true;
}

inline bool RankChanged(int32_t Standing, int32_t Change)
{
    return Player::getReputationRankFromStanding(Standing) != Player::getReputationRankFromStanding(Standing + Change);
}

inline bool RankChangedFlat(int32_t Standing, int32_t NewStanding)
{
    return Player::getReputationRankFromStanding(Standing) != Player::getReputationRankFromStanding(NewStanding);
}

void Player::setFactionStanding(uint32_t faction, int32_t value)
{
    WDB::Structures::FactionEntry const* factionEntry = sFactionStore.lookupEntry(faction);
    if (!factionEntry || factionEntry->RepListId < 0)
        return;

    const int32_t minReputation = -42000;      //   0/36000 Hated
    const int32_t exaltedReputation = 42000;   //   0/1000  Exalted
    const int32_t maxReputation = 42999;       // 999/1000  Exalted

    int32_t newValue = value;
    if (newValue < minReputation)
        newValue = minReputation;
    else if (newValue > maxReputation)
        newValue = maxReputation;

    auto reputation = m_reputation.find(faction);
    if (reputation == m_reputation.end())
    {
        if (!addNewFaction(factionEntry, newValue, false))
            return;

        reputation = m_reputation.find(faction);

#if VERSION_STRING > TBC
        if (reputation->second->standing >= 42000)
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);

        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, factionEntry->ID, reputation->second->standing, 0);
#endif

        updateInrangeSetsBasedOnReputation();
        onModStanding(factionEntry, reputation->second.get());
    }
    else
    {
        if (RankChangedFlat(reputation->second->standing, newValue))
        {

#if VERSION_STRING > TBC
            if (reputation->second->standing - newValue >= exaltedReputation)
               updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, -1, 0, 0);
            else if (newValue >= exaltedReputation)
                updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);
#endif

            reputation->second->standing = newValue;
            updateInrangeSetsBasedOnReputation();

#if VERSION_STRING > TBC
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, factionEntry->ID, value, 0);
#endif

        }
        else
        {
            reputation->second->standing = newValue;
        }

        onModStanding(factionEntry, reputation->second.get());
    }
}

int32_t Player::getFactionStanding(uint32_t faction)
{
    const ReputationMap::iterator itr = m_reputation.find(faction);
    if (itr != m_reputation.end())
        return itr->second->standing;
    return 0;
}

int32_t Player::getBaseFactionStanding(uint32_t faction)
{
    const ReputationMap::iterator itr = m_reputation.find(faction);
    if (itr != m_reputation.end())
        return itr->second->baseStanding;
    return 0;
}

void Player::modFactionStanding(uint32_t faction, int32_t value)
{
    WDB::Structures::FactionEntry const* factionEntry = sFactionStore.lookupEntry(faction);
    if (factionEntry == nullptr || factionEntry->RepListId < 0)
        return;

    const int32_t minReputation = -42000;      //   0/36000 Hated
    const int32_t exaltedReputation = 42000;   //   0/1000  Exalted
    const int32_t maxReputation = 42999;       // 999/1000  Exalted

    if ((getWorldMap()->getBaseMap()->getMapInfo()->minlevel == 80 ||
        (getWorldMap()->getDifficulty() == InstanceDifficulty::DUNGEON_HEROIC && getWorldMap()->getBaseMap()->getMapInfo()->minlevel_heroic == 80)) &&
        m_championingFactionId != 0)
        faction = m_championingFactionId;

    int32_t newValue = value;
    if (newValue < minReputation)
        newValue = minReputation;
    else if (newValue > maxReputation)
        newValue = maxReputation;

    ReputationMap::iterator itr = m_reputation.find(faction);
    if (itr == m_reputation.end())
    {
        if (!addNewFaction(factionEntry, newValue, false))
            return;

        itr = m_reputation.find(faction);

#if VERSION_STRING > TBC
        if (itr->second->standing >= 42000)
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);

        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, factionEntry->ID, itr->second->standing, 0);
#endif

        updateInrangeSetsBasedOnReputation();
        onModStanding(factionEntry, itr->second.get());
    }
    else
    {
        if (m_pctReputationMod > 0)
            newValue = value + (value * m_pctReputationMod / 100);

        if (RankChanged(itr->second->standing, newValue))
        {
            itr->second->standing += newValue;
            updateInrangeSetsBasedOnReputation();

#if VERSION_STRING > TBC
            updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, factionEntry->ID, itr->second->standing, 0);
            if (itr->second->standing >= exaltedReputation) 
                updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);
            else if (itr->second->standing - newValue >= exaltedReputation)
                updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, -1, 0, 0);
#endif

        }
        else
        {
            itr->second->standing += newValue;
        }

        if (itr->second->standing < minReputation)
            itr->second->standing = minReputation;
        else if (itr->second->standing > maxReputation)
            itr->second->standing = maxReputation;
        onModStanding(factionEntry, itr->second.get());
    }
}

Standing Player::getFactionStandingRank(uint32_t faction)
{
    return getReputationRankFromStanding(getFactionStanding(faction));
}

Standing Player::getReputationRankFromStanding(int32_t value)
{
    if (value >= 42000)
        return STANDING_EXALTED;
    if (value >= 21000)
        return STANDING_REVERED;
    if (value >= 9000)
        return STANDING_HONORED;
    if (value >= 3000)
        return STANDING_FRIENDLY;
    if (value >= 0)
        return STANDING_NEUTRAL;
    if (value > -3000)
        return STANDING_UNFRIENDLY;
    if (value > -6000)
        return STANDING_HOSTILE;

    return STANDING_HATED;
}

void Player::applyForcedReaction(uint32_t faction_id, Standing rank, bool apply)
{
    if (apply)
        m_forcedReactions[faction_id] = rank;
    else
        m_forcedReactions.erase(faction_id);
}

Standing const* Player::getForcedReputationRank(WDB::Structures::FactionTemplateEntry const* factionTemplateEntry) const
{
    const auto itr = m_forcedReactions.find(factionTemplateEntry->Faction);
    return itr != m_forcedReactions.end() ? &itr->second : nullptr;
}

void Player::setFactionAtWar(uint32_t faction, bool set)
{
    if (faction >= 128)
        return;

    FactionReputation* factionReputation = m_reputationByListId[faction];
    if (!factionReputation)
        return;

    if (getReputationRankFromStanding(factionReputation->standing) <= STANDING_HOSTILE && !set)
        return;

    if (!CanToggleAtWar(factionReputation->flag))
        return;

    if (SetFlagAtWar(factionReputation->flag, set))
        updateInrangeSetsBasedOnReputation();
}

bool Player::isFactionAtWar(WDB::Structures::FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return false;

    FactionReputation const* factionState = m_reputationByListId[factionEntry->RepListId];
    if (factionState == nullptr)
        return false;

        return AtWar(factionState->flag);

    return false;
}

bool Player::isHostileBasedOnReputation(WDB::Structures::FactionEntry const* factionEntry)
{
    if (!factionEntry)
        return false;

    if (factionEntry->RepListId < 0 || factionEntry->RepListId >= 128)
        return false;

    FactionReputation* factionReputation = m_reputationByListId[factionEntry->RepListId];
    if (factionReputation == nullptr)
        return false;

    const auto itr = m_forcedReactions.find(factionEntry->ID);
    if (itr != m_forcedReactions.end())
        return itr->second <= STANDING_HOSTILE;

    return AtWar(factionReputation->flag) || getReputationRankFromStanding(factionReputation->standing) <= STANDING_HOSTILE;
}

void Player::updateInrangeSetsBasedOnReputation()
{
    for (const auto& object : getInRangeObjectsSet())
    {
        if (!object->isCreatureOrPlayer())
            continue;

        if (const auto unit = dynamic_cast<Unit*>(object))
        {
            if (unit->m_factionEntry == nullptr || unit->m_factionEntry->RepListId < 0)
                continue;

            bool isHostile = isHostileBasedOnReputation(unit->m_factionEntry);
            bool currentHostileObject = isObjectInInRangeOppositeFactionSet(unit);

            if (isHostile && !currentHostileObject)
                addInRangeOppositeFaction(unit);
            else if (!isHostile && currentHostileObject)
                addInRangeOppositeFaction(unit);
        }
    }
}

void Player::onKillUnitReputation(Unit* unit, bool innerLoop)
{
    if (!unit)
        return;

    if (!unit->isCreature() || unit->isPet() || unit->isCritter())
        return;

    if (auto m_Group = getGroup())
    {
        if (!innerLoop)
        {
            m_Group->getLock().lock();

            for (uint32_t i = 0; i < m_Group->GetSubGroupCount(); ++i)
                for (auto groupMember : m_Group->GetSubGroup(i)->getGroupMembers())
                    if (auto player = sObjectMgr.getPlayer(groupMember->guid))
                        if (player->isInRange(this, 100.0f))
                            player->onKillUnitReputation(unit, true);

            m_Group->getLock().unlock();

            return;
        }
    }

    const uint32_t team = getTeam();
    auto modifier = sObjectMgr.getReputationModifier(unit->getEntry(), unit->m_factionEntry->ID);
    if (modifier != nullptr)
    {
        for (auto& mod : modifier->mods)
        {
            if (!mod->faction[team])
                continue;

            if (!IS_INSTANCE(GetMapId()) || (IS_INSTANCE(GetMapId()) && this->m_dungeonDifficulty != InstanceDifficulty::DUNGEON_HEROIC))
                if (mod->replimit)
                    if (getFactionStanding(mod->faction[team]) >= static_cast<int32_t>(mod->replimit))
                        continue;

            modFactionStanding(mod->faction[team], Util::float2int32(mod->value * worldConfig.getFloatRate(RATE_KILLREPUTATION)));
        }
    }
    else
    {
        if (IS_INSTANCE(GetMapId()) && sObjectMgr.handleInstanceReputationModifiers(this, unit))
            return;

        if (unit->m_factionEntry->RepListId < 0)
            return;

        const int32_t change = static_cast<int32_t>(-5.0f * worldConfig.getFloatRate(RATE_KILLREPUTATION));
        modFactionStanding(unit->m_factionEntry->ID, change);
    }
}

void Player::onTalkReputation(WDB::Structures::FactionEntry const* factionEntry)
{
    if (!factionEntry || factionEntry->RepListId < 0)
        return;

    FactionReputation* factionReputation = m_reputationByListId[factionEntry->RepListId];
    if (factionReputation == nullptr)
        return;

    if (SetFlagVisible(factionReputation->flag, true) && IsInWorld())
        sendPacket(SmsgSetFactionVisible(factionEntry->RepListId).serialise().get());
}

void Player::setFactionInactive(uint32_t faction, bool /*set*/)
{
    FactionReputation* factionReputation = m_reputationByListId[faction];
    if (!factionReputation)
        return;
}

bool Player::addNewFaction(WDB::Structures::FactionEntry const* factionEntry, int32_t standing, bool base)
{
    if (!factionEntry || factionEntry->RepListId < 0)
        return false;

    for (uint8_t i = 0; i < 4; ++i)
    {
        if ((factionEntry->RaceMask[i] & getRaceMask() || 
            factionEntry->RaceMask[i] == 0 && factionEntry->ClassMask[i] != 0) && 
            (factionEntry->ClassMask[i] & getClassMask() || factionEntry->ClassMask[i] == 0))
        {
            const auto flag = static_cast<uint8_t>(factionEntry->repFlags[i]);
            const auto baseStanding = factionEntry->baseRepValue[i];
            const auto m_standing = (base) ? factionEntry->baseRepValue[i] : standing;

            const auto [repItr, _] = m_reputation.insert_or_assign(factionEntry->ID, std::make_unique<FactionReputation>(m_standing, flag, baseStanding));
            m_reputationByListId[factionEntry->RepListId] = repItr->second.get();

            return true;
        }
    }
    return false;
}

void Player::onModStanding(WDB::Structures::FactionEntry const* factionEntry, FactionReputation* reputation)
{
    if (!factionEntry || !reputation)
        return;

    if (SetFlagVisible(reputation->flag, true) && IsInWorld())
        sendPacket(SmsgSetFactionVisible(factionEntry->RepListId).serialise().get());

    SetFlagAtWar(reputation->flag, (getReputationRankFromStanding(reputation->standing) <= STANDING_HOSTILE));

    if (Visible(reputation->flag) && IsInWorld())
        sendPacket(SmsgSetFactionStanding(factionEntry->RepListId, reputation->CalcStanding()).serialise().get());
}

uint32_t Player::getExaltedCount()
{
    uint32_t exaltedCount = 0;

    auto itr = m_reputation.begin();
    while (itr != m_reputation.end())
    {
        const int32_t exaltedReputation = 42000;
        if (itr->second->standing >= exaltedReputation)
            ++exaltedCount;
        ++itr;
    }
    return exaltedCount;
}

void Player::sendSmsgInitialFactions()
{
#if VERSION_STRING == Mop
    const uint16_t factionCount = 256;
    ByteBuffer buffer;
    uint32_t a = 0;

    WorldPacket data(SMSG_INITIALIZE_FACTIONS, factionCount * (1 + 4) + 32);
    for (; a != factionCount; ++a)
    {
        data << uint8_t(0);
        data << uint32_t(0);
        buffer.writeBit(0);
    }

    buffer.flushBits();

    data.append(buffer);
#else
    WorldPacket data(SMSG_INITIALIZE_FACTIONS, 764);
    data << uint32_t(128);

    for (auto& i : m_reputationByListId)
    {
        FactionReputation* factionReputation = i;
        if (!factionReputation)
        {
            data << uint8_t(0);
            data << uint32_t(0);
        }
        else
        {
            data << factionReputation->flag;
            data << factionReputation->CalcStanding();
        }
    }

#endif
    m_session->SendPacket(&data);
}

void Player::initialiseReputation()
{
    for (uint32_t i = 0; i < sFactionStore.getNumRows(); ++i)
    {
        WDB::Structures::FactionEntry const* factionEntry = sFactionStore.lookupEntry(i);
        addNewFaction(factionEntry, 0, true);
    }
}

uint32_t Player::getInitialFactionId()
{
    if (const auto raceEntry = sChrRacesStore.lookupEntry(getRace()))
        return raceEntry->faction_id;

    return 0;
}

int32_t Player::getPctReputationMod() const { return m_pctReputationMod; }
void Player::setPctReputationMod(int32_t value) { m_pctReputationMod = value; }

void Player::setChampioningFaction(uint32_t factionId) { m_championingFactionId = factionId; }

/////////////////////////////////////////////////////////////////////////////////////////
// Drunk system
uint16_t Player::getServersideDrunkValue() const { return m_serversideDrunkValue; }

void Player::setServersideDrunkValue(uint16_t newDrunkenValue, uint32_t itemId)
{
    const uint32_t oldDrunkenState = getDrunkStateByValue(m_serversideDrunkValue);

    m_serversideDrunkValue = newDrunkenValue;
    setDrunkValue(static_cast<uint8_t>(m_serversideDrunkValue));

    const uint32_t newDrunkenState = getDrunkStateByValue(m_serversideDrunkValue);

    if (newDrunkenState == oldDrunkenState)
        return;

    if (newDrunkenState >= DRUNKEN_DRUNK)
        modInvisibilityDetection(INVIS_FLAG_DRUNK, 100);
    else
        modInvisibilityDetection(INVIS_FLAG_DRUNK, -getInvisibilityDetection(INVIS_FLAG_DRUNK));

    updateVisibility();

    sendNewDrunkStatePacket(newDrunkenState, itemId);
}

PlayerBytes3_DrunkValue Player::getDrunkStateByValue(uint16_t value)
{
    if (value >= 23000)
        return DRUNKEN_SMASHED;

    if (value >= 12800)
        return DRUNKEN_DRUNK;

    if (value & 0xFFFE)
        return DRUNKEN_TIPSY;

    return DRUNKEN_SOBER;
}

void Player::handleSobering()
{
    m_drunkTimer = 0;

    setDrunkValue((m_serversideDrunkValue <= 256) ? 0U : static_cast<uint8_t>(m_serversideDrunkValue - 256));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Duel
Player* Player::getDuelPlayer() const { return m_duelPlayer; }
void Player::requestDuel(Player* target)
{
    if (m_duelPlayer != nullptr)
        return;

    if (m_duelState != DUEL_STATE_FINISHED)
        return;

    setDuelState(DUEL_STATE_REQUESTED);

    target->m_duelPlayer = this;
    m_duelPlayer = target;

    // flag position
    const float distance = CalcDistance(target) * 0.5f;
    const float x = (GetPositionX() + target->GetPositionX() * distance) / (1 + distance) + cos(GetOrientation() + (M_PI_FLOAT / 2)) * 2;
    const float y = (GetPositionY() + target->GetPositionY() * distance) / (1 + distance) + sin(GetOrientation() + (M_PI_FLOAT / 2)) * 2;
    const float z = (GetPositionZ() + target->GetPositionZ() * distance) / (1 + distance);

    // create flag
    if (GameObject* goFlag = getWorldMap()->createGameObject(21680))
    {
        goFlag->create(21680, m_WorldMap, GetPhase(), LocationVector(x, y, z, GetOrientation()), QuaternionData(), GO_STATE_CLOSED);

        goFlag->setCreatedByGuid(getGuid());
        goFlag->SetFaction(getFactionTemplate());
        goFlag->setLevel(getLevel());

        setDuelArbiter(goFlag->getGuid());
        target->setDuelArbiter(goFlag->getGuid());

        goFlag->PushToWorld(m_WorldMap);

        addGameObject(goFlag);

        target->getSession()->SendPacket(SmsgDuelRequested(goFlag->getGuid(), getGuid()).serialise().get());
    }
}

void Player::testDuelBoundary()
{
    if (!IsInWorld())
        return;

    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());

    if (GameObject* goFlag = getWorldMap()->getGameObject(wowGuid.getGuidLowPart()))
    {
        if (CalcDistance(goFlag) > 75.0f)
        {
            if (m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
            {
                m_duelCountdownTimer -= 500;
                if (m_duelCountdownTimer == 0)
                    m_duelPlayer->endDuel(DUEL_WINNER_RETREAT);
            }
            else
            {
                m_duelCountdownTimer = 10000;

                sendPacket(SmsgDuelOutOfBounds(m_duelCountdownTimer).serialise().get());
                m_duelStatus = DUEL_STATUS_OUTOFBOUNDS;
            }
        }
        else
        {
            if (m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
            {
                sendPacket(SmsgDuelInbounds().serialise().get());
                m_duelStatus = DUEL_STATUS_INBOUNDS;
            }
        }
    }
    else
    {
        endDuel(DUEL_WINNER_RETREAT);
    }
}

void Player::endDuel(uint8_t condition)
{
    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());

    if (m_duelState == DUEL_STATE_FINISHED)
    {
        if (wowGuid.getGuidLowPart())
        {
            GameObject* arbiter = m_WorldMap ? getWorldMap()->getGameObject(wowGuid.getGuidLowPart()) : nullptr;
            if (arbiter)
            {
                arbiter->RemoveFromWorld(true);
                delete arbiter;
            }

            m_duelPlayer->setDuelArbiter(0);
            setDuelArbiter(0);

            m_duelPlayer->setDuelTeam(0);
            setDuelTeam(0);

            sEventMgr.RemoveEvents(m_duelPlayer, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);
            sEventMgr.RemoveEvents(m_duelPlayer, EVENT_PLAYER_DUEL_COUNTDOWN);

            m_duelPlayer->m_duelPlayer = nullptr;
            m_duelPlayer = nullptr;
        }

        return;
    }

    sEventMgr.RemoveEvents(this, EVENT_PLAYER_DUEL_COUNTDOWN);
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);

    for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
    {
        auto* const aur = getAuraWithAuraSlot(x);
        if (aur == nullptr)
            continue;

        if (aur->WasCastInDuel())
            aur->removeAura();
    }

    m_duelState = DUEL_STATE_FINISHED;

    if (m_duelPlayer == nullptr)
        return;

    sEventMgr.RemoveEvents(m_duelPlayer, EVENT_PLAYER_DUEL_BOUNDARY_CHECK);
    sEventMgr.RemoveEvents(m_duelPlayer, EVENT_PLAYER_DUEL_COUNTDOWN);

    for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
    {
        auto* const aur = m_duelPlayer->getAuraWithAuraSlot(x);
        if (aur == nullptr)
            continue;
        if (aur->WasCastInDuel())
            aur->removeAura();
    }

    m_duelPlayer->m_duelState = DUEL_STATE_FINISHED;

    sendMessageToSet(SmsgDuelWinner(condition, getName(), m_duelPlayer->getName()).serialise().get(), true);
    sendMessageToSet(SmsgDuelComplete(1).serialise().get(), true);

    if (condition != 0)
        sHookInterface.OnDuelFinished(m_duelPlayer, this);
    else
        sHookInterface.OnDuelFinished(this, m_duelPlayer);

    GameObject* goFlag = m_WorldMap ? getWorldMap()->getGameObject(wowGuid.getGuidLowPart()) : nullptr;
    if (goFlag)
    {
        goFlag->RemoveFromWorld(true);
        delete goFlag;
    }

    setDuelArbiter(0);
    m_duelPlayer->setDuelArbiter(0);

    setDuelTeam(0);
    m_duelPlayer->setDuelTeam(0);

    eventAttackStop();
    m_duelPlayer->eventAttackStop();

    for (auto& summon : getSummonInterface()->getSummons())
    {
        summon->getCombatHandler().clearCombat();
        summon->getAIInterface()->setPetOwner(this);
        summon->getAIInterface()->handleEvent(EVENT_FOLLOWOWNER, summon, 0);
        summon->getThreatManager().clearAllThreat();
        summon->getThreatManager().removeMeFromThreatLists();
    }

    for (auto& duelingWithSummon : m_duelPlayer->getSummonInterface()->getSummons())
    {
        duelingWithSummon->getCombatHandler().clearCombat();
        duelingWithSummon->getAIInterface()->setPetOwner(this);
        duelingWithSummon->getAIInterface()->handleEvent(EVENT_FOLLOWOWNER, duelingWithSummon, 0);
        duelingWithSummon->getThreatManager().clearAllThreat();
        duelingWithSummon->getThreatManager().removeMeFromThreatLists();
    }

    m_session->SendPacket(SmsgCancelCombat().serialise().get());
    m_duelPlayer->m_session->SendPacket(SmsgCancelCombat().serialise().get());

    smsg_AttackStop(m_duelPlayer);
    m_duelPlayer->smsg_AttackStop(this);

    m_duelPlayer->m_duelCountdownTimer = 0;
    m_duelCountdownTimer = 0;

    m_duelPlayer->m_duelPlayer = nullptr;
    m_duelPlayer = nullptr;
}

void Player::cancelDuel()
{
    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());

    const auto goFlag = getWorldMap()->getGameObject(wowGuid.getGuidLowPart());
    if (goFlag)
        goFlag->RemoveFromWorld(true);

    setDuelArbiter(0);
    m_duelPlayer->setDuelArbiter(0);

    m_duelPlayer->m_duelState = DUEL_STATE_FINISHED;
    m_duelState = DUEL_STATE_FINISHED;

    m_duelPlayer->setDuelTeam(0);
    setDuelTeam(0);

    m_duelPlayer->m_duelCountdownTimer = 0;
    m_duelCountdownTimer = 0;

    m_duelPlayer->m_duelPlayer = nullptr;
    m_duelPlayer = nullptr;
}

void Player::handleDuelCountdown()
{
    if (m_duelPlayer == nullptr)
        return;

    m_duelCountdownTimer -= 1000;

    if (static_cast<int32_t>(m_duelCountdownTimer) < 0)
        m_duelCountdownTimer = 0;

    if (m_duelCountdownTimer == 0)
    {
        setPower(POWER_TYPE_RAGE, 0);
        m_duelPlayer->setPower(POWER_TYPE_RAGE, 0);

        m_duelPlayer->setDuelTeam(1);
        setDuelTeam(2);

        setDuelState(DUEL_STATE_STARTED);
        m_duelPlayer->setDuelState(DUEL_STATE_STARTED);

        sEventMgr.AddEvent(this, &Player::testDuelBoundary, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        sEventMgr.AddEvent(m_duelPlayer, &Player::testDuelBoundary, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Player::setDuelStatus(uint8_t status) { m_duelStatus = status; }
uint8_t Player::getDuelStatus() const { return m_duelStatus; }

void Player::setDuelState(uint8_t state) { m_duelState = state; }
uint8_t Player::getDuelState() const { return m_duelState; }

/////////////////////////////////////////////////////////////////////////////////////////
// Resting/Experience XP
void Player::giveXp(uint32_t xp, const uint64_t& guid, bool allowBonus)
{
    if (xp < 1)
        return;

#if VERSION_STRING >= Cata
    //this is new since 403. As we gain XP we also gain XP with our guild
    if (m_playerInfo && m_playerInfo->m_guild)
    {
        uint32_t guild_share = xp / 100;

        if (Guild* guild = sGuildMgr.getGuildById(m_playerInfo->m_guild))
            guild->giveXP(guild_share, this);
    }
#endif

    if (!m_isXpGainAllowed)
        return;

    if (getLevel() >= getMaxLevel())
        return;

    uint32_t restXp = xp;

    if (m_restState == RESTSTATE_RESTED && allowBonus)
    {
        restXp = subtractRestXp(xp);
        xp += restXp;
    }

    updateRestState();

    sendLogXpGainPacket(guid, xp, restXp, guid == 0 ? true : false);

    int32_t newXp = getXp() + xp;
    int32_t nextLevelXp = getNextLevelXp();
    uint32_t level = getLevel();
    bool levelup = false;

    while (newXp >= nextLevelXp && newXp > 0)
    {
        ++level;
        if (sObjectMgr.getLevelInfo(getRace(), getClass(), level))
        {
            newXp -= nextLevelXp;
            nextLevelXp = sMySQLStore.getPlayerXPForLevel(level);
            levelup = true;
            if (level >= getMaxLevel())
                break;
        }
        else
        {
            return;
        }
    }

    if (level > getMaxLevel())
        level = getMaxLevel();

    if (levelup)
        applyLevelInfo(level);

    setXp(newXp);
}

void Player::sendLogXpGainPacket(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool type)
{
    m_session->SendPacket(SmsgLogXpGain(guid, normalXp, restedXp, type).serialise().get());
}

void Player::toggleXpGain() { m_isXpGainAllowed ? m_isXpGainAllowed = false : m_isXpGainAllowed = true; }
bool Player::canGainXp() const { return m_isXpGainAllowed; }

uint32_t Player::subtractRestXp(uint32_t amount)
{
    if (getLevel() >= getMaxLevel())
        amount = 0;

    const int32_t restAmount = m_restAmount - (amount << 1);
    if (restAmount < 0)
        m_restAmount = 0;
    else
        m_restAmount = restAmount;

    sLogger.debug("Subtracted {} rest XP to a total of {}", amount, m_restAmount);

    updateRestState();

    return amount;
}

void Player::addCalculatedRestXp(uint32_t seconds)
{
    const uint32_t nextLevelXp = getNextLevelXp();

    const float restXpRate = worldConfig.getFloatRate(RATE_RESTXP);

    auto restXp = static_cast<uint32_t>(0.05f * nextLevelXp * (seconds / (3600 * (8 / restXpRate))));

    if (m_isResting)
        restXp <<= 2;

    m_restAmount += restXp;

    if (m_restAmount > nextLevelXp + static_cast<uint32_t>(static_cast<float>(nextLevelXp >> 1) * restXpRate))
        m_restAmount = nextLevelXp + static_cast<uint32_t>(static_cast<float>(nextLevelXp >> 1) * restXpRate);

    sLogger.debug("Add {} rest XP to a total of {}, RestState {}", restXp, m_restAmount, m_isResting);

    updateRestState();
}

void Player::applyPlayerRestState(bool apply)
{
    if (apply)
    {
        m_restState = RESTSTATE_RESTED;
        m_isResting = true;
        addPlayerFlags(PLAYER_FLAG_RESTING);
    }
    else
    {
        m_isResting = false;
        removePlayerFlags(PLAYER_FLAG_RESTING);
    }

    updateRestState();
}

void Player::updateRestState()
{
    if (m_restAmount && getLevel() < getMaxLevel())
        m_restState = RESTSTATE_RESTED;
    else
        m_restState = RESTSTATE_NORMAL;

    setRestState(m_restState);
    setRestStateXp(m_restAmount >> 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Pets/Summons

PetCache const* Player::getPetCache(uint8_t petId) const
{
    const auto itr = m_cachedPets.find(petId);
    if (itr != m_cachedPets.cend())
        return itr->second.get();

    return nullptr;
}

PetCache* Player::getModifiablePetCache(uint8_t petId) const
{
    const auto itr = m_cachedPets.find(petId);
    if (itr != m_cachedPets.cend())
        return itr->second.get();

    return nullptr;
}

PetCacheMap const& Player::getPetCacheMap() const
{
    return m_cachedPets;
}

std::map<uint8_t, uint8_t> const& Player::getPetCachedSlotMap() const
{
    return m_cachedPetSlots;
}

void Player::addPetCache(std::unique_ptr<PetCache> pet, uint8_t index)
{
    m_cachedPetSlots.emplace(pet->slot, index);
    m_cachedPets.emplace(index, std::move(pet));
}

void Player::removePetCache(uint8_t petId)
{
    const auto itr = std::as_const(m_cachedPets).find(petId);
    if (itr != m_cachedPets.cend())
    {
        std::erase_if(m_cachedPetSlots, [petId](const auto& slotItr) { return slotItr.second == petId; });
        m_cachedPets.erase(itr);
    }

    // Pet will be deleted from playerpets table when player is saved
    CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), petId);
}

uint8_t Player::getPetCount() const { return static_cast<uint8_t>(m_cachedPets.size()); }

uint8_t Player::getFreePetNumber()
{
    for (uint8_t i = 1; i < m_maxPetNumber; ++i)
        if (!m_cachedPets.contains(i))
            return i;

    m_maxPetNumber += 1;
    return m_maxPetNumber;
}

std::optional<uint8_t> Player::getPetIdFromSlot(uint8_t slot) const
{
    const auto itr = m_cachedPetSlots.find(slot);
    if (itr != m_cachedPetSlots.cend())
        return itr->second;

    return std::nullopt;
}

bool Player::hasPetInSlot(uint8_t slot) const
{
    return m_cachedPetSlots.contains(slot);
}

std::optional<uint8_t> Player::findFreeActivePetSlot() const
{
    std::optional<uint8_t> foundSlot = std::nullopt;
    for (uint8_t i = PET_SLOT_FIRST_ACTIVE_SLOT; i < PET_SLOT_MAX_ACTIVE_SLOT; ++i)
    {
        if (!hasPetInSlot(i))
        {
            foundSlot = i;
            break;
        }
    }
    return foundSlot;
}

std::optional<uint8_t> Player::findFreeStablePetSlot() const
{
    std::optional<uint8_t> foundSlot = std::nullopt;
    for (uint8_t i = PET_SLOT_FIRST_STABLE_SLOT; i < PET_SLOT_LAST_STABLE_SLOT; ++i)
    {
        if (m_stableSlotCount <= (i - PET_SLOT_FIRST_STABLE_SLOT))
            break;

        if (!hasPetInSlot(i))
        {
            foundSlot = i;
            break;
        }
    }
    return foundSlot;
}

bool Player::tryPutPetToSlot(uint8_t petId, uint8_t newSlot, bool sendErrors/* = true*/)
{
    const auto petItr = std::as_const(m_cachedPets).find(petId);
    if (petItr == m_cachedPets.cend())
    {
        if (sendErrors)
            sendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());

        return false;
    }

    // Check if pet is being tried to move to same slot where it is already
    if (petItr->second->slot == newSlot)
    {
        if (sendErrors)
            sendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());

        return false;
    }

    auto* oldSlotPet = petItr->second.get();
    // Pet that possibly exists in new slot
    PetCache* newSlotPet = nullptr;

    const auto slotItr = std::as_const(m_cachedPetSlots).find(newSlot);
    if (slotItr != m_cachedPetSlots.cend())
    {
        const auto existingPetItr = std::as_const(m_cachedPets).find(slotItr->second);
        if (existingPetItr != m_cachedPets.cend())
            newSlotPet = existingPetItr->second.get();
    }

    const auto isOldSlotActiveSlot = oldSlotPet->slot < PET_SLOT_MAX_ACTIVE_SLOT;
    const auto isNewSlotActiveSlot = newSlot < PET_SLOT_MAX_ACTIVE_SLOT;

#if VERSION_STRING >= WotLK
    if (isNewSlotActiveSlot)
    {
        // Check if player can have exotic pets when taking pet from stables
        if (const auto creatureProperties = sMySQLStore.getCreatureProperties(oldSlotPet->entry))
        {
            if (creatureProperties->isExotic() && !hasAuraWithAuraEffect(SPELL_AURA_ALLOW_TAME_PET_TYPE))
            {
                if (sendErrors)
                    sendPacket(SmsgStableResult(PetStableResult::ExoticNotAvailable).serialise().get());

                return false;
            }
        }
    }

    if (isOldSlotActiveSlot && !isNewSlotActiveSlot && newSlotPet != nullptr)
    {
        // Check also if pet in new slot is exotic if player is swapping pet slots
        if (const auto creatureProperties = sMySQLStore.getCreatureProperties(newSlotPet->entry))
        {
            if (creatureProperties->isExotic() && !hasAuraWithAuraEffect(SPELL_AURA_ALLOW_TAME_PET_TYPE))
            {
                if (sendErrors)
                    sendPacket(SmsgStableResult(PetStableResult::ExoticNotAvailable).serialise().get());

                return false;
            }
        }
    }
#endif

    if (!isNewSlotActiveSlot)
    {
        // Must be hunter pet
        if (oldSlotPet->type != PET_TYPE_HUNTER)
        {
            if (sendErrors)
                sendPacket(SmsgStableResult(PetStableResult::Error).serialise().get());

            return false;
        }
    }

    m_cachedPetSlots.insert_or_assign(newSlot, petId);
    if (newSlotPet != nullptr)
        m_cachedPetSlots.insert_or_assign(oldSlotPet->slot, newSlotPet->number);
    else
        m_cachedPetSlots.erase(oldSlotPet->slot);

    // Update only slot in pet cache, full update is done in possible summon/unsummon
    if (newSlotPet != nullptr)
        newSlotPet->slot = oldSlotPet->slot;
    oldSlotPet->slot = newSlot;

    auto requiresPetSave = false;
    if (isOldSlotActiveSlot && !isNewSlotActiveSlot)
    {
        // Active pet is put to stables
        auto* const currentPet = getPet();
        if (currentPet != nullptr && currentPet->getPetId() == petId)
        {
            currentPet->unSummon();
            if (newSlotPet != nullptr && !isPetRequiringTemporaryUnsummon() && newSlotPet->alive)
                _spawnPet(newSlotPet);
        }
        else
        {
            // If this pet was temporary unsummoned make the other pet active as well
            if (newSlotPet != nullptr)
                newSlotPet->active = oldSlotPet->active;
            oldSlotPet->active = false;
            requiresPetSave = true;
        }
    }
    else if (!isOldSlotActiveSlot && isNewSlotActiveSlot)
    {
        // Pet is taken from stables
        if (newSlotPet != nullptr)
        {
            auto* const currentPet = getPet();
            // Only summon it if it's put to same slot as current summoned pet
            if (currentPet != nullptr && currentPet->getPetId() == newSlotPet->number)
            {
                currentPet->unSummon();
                if (!isPetRequiringTemporaryUnsummon() && oldSlotPet->alive)
                    _spawnPet(oldSlotPet);
            }
            else
            {
                // If pet from new slot was temporary unsummoned make this pet active as well
                oldSlotPet->active = newSlotPet->active;
                newSlotPet->active = false;
                requiresPetSave = true;
            }
        }
        else
        {
            requiresPetSave = true;
        }
    }
    else if ((!isOldSlotActiveSlot && !isNewSlotActiveSlot) || (isOldSlotActiveSlot && isNewSlotActiveSlot))
    {
        // Pet is either moved inside stables or its active slot was changed
        // In either case unsummon is not required, just save pets to database
        requiresPetSave = true;
    }

    if (requiresPetSave)
    {
        // Save only slot and active fields
        CharacterDatabase.Execute("UPDATE playerpets SET slot = %u, active = %u WHERE ownerguid = %u AND petnumber = %u",
            oldSlotPet->slot, oldSlotPet->active, getGuidLow(), oldSlotPet->number);
        if (newSlotPet != nullptr)
        {
            CharacterDatabase.Execute("UPDATE playerpets SET slot = %u, active = %u WHERE ownerguid = %u AND petnumber = %u",
                newSlotPet->slot, newSlotPet->active, getGuidLow(), newSlotPet->number);
        }
    }

    return true;
}

void Player::spawnPet(uint8_t petId)
{
    const auto itr = m_cachedPets.find(petId);
    if (itr == m_cachedPets.cend())
    {
        sLogger.failure("Player::spawnPet : {} tried to load invalid pet {}", std::to_string(getGuid()), petId);
        return;
    }

    if (itr->second.get()->slot >= PET_SLOT_MAX_ACTIVE_SLOT)
    {
        sLogger.debug("Player::spawnPet : {} tried to spawn pet from stable slot {}", std::to_string(getGuid()), std::to_string(itr->second.get()->slot));
        return;
    }

    _spawnPet(itr->second.get());
}

void Player::summonTemporarilyUnsummonedPet()
{
    if (getPet() != nullptr)
        return;

    if (isPetRequiringTemporaryUnsummon())
        return;

    for (const auto& [slot, petId] : std::as_const(m_cachedPetSlots))
    {
        // Just check active slots
        if (slot >= PET_SLOT_MAX_ACTIVE_SLOT)
            break;

        auto* const petCache = getModifiablePetCache(petId);
        if (petCache != nullptr && petCache->active)
        {
            // If active pet is not alive it cant be summoned now
            // Player must explicitly summon and revive it
            if (petCache->alive)
                _spawnPet(petCache);
            else
                petCache->active = false;

            return;
        }
    }
}

void Player::unSummonPetTemporarily()
{
    if (getPet() == nullptr)
        return;

    getPet()->unSummonTemporarily();
}

bool Player::isPetRequiringTemporaryUnsummon() const
{
    if (!IsInWorld() || !isAlive())
        return true;

    if (isOnTaxi())
        return true;

    // In classic pets were not despawned when mounted
    // In tbc and wotlk they despawned, but this was again changed around patch 4.1
#if VERSION_STRING == TBC || VERSION_STRING == WotLK
#ifdef FT_VEHICLES
    if (isMounted() || isOnVehicle())
#else
    if (isMounted())
#endif
    {
        if (const auto* const pet = getPet())
        {
            if (!pet->isAlive())
                return false;

            if (!pet->isPermanentSummon())
                return false;

            if (m_bg != nullptr && m_bg->isArena())
                return false;

            // For some reason permanent water elemental is not despawned
            if (pet->getEntry() == PET_WATER_ELEMENTAL_NEW)
                return false;
        }

        return true;
    }
#endif

    return false;
}

void Player::setTemporarilyUnsummonedPetsOffline()
{
    const auto copiedCachedSlots = m_cachedPetSlots;
    for (const auto& [slot, petId] : std::as_const(copiedCachedSlots))
    {
        // Just check active slots
        if (slot >= PET_SLOT_MAX_ACTIVE_SLOT)
            break;

        auto* const petCache = getModifiablePetCache(petId);
        if (petCache == nullptr)
            continue;

        if (petCache->active)
        {
            // Summoned pets can be completely deleted since they can be resummoned anyway
            if (petCache->type == PET_TYPE_HUNTER)
                petCache->active = false;
            else
                removePetCache(petCache->number);
        }
    }
}

void Player::setLastBattlegroundPetId(uint8_t petId) { m_battlegroundLastPetId = petId; }
uint8_t Player::getLastBattlegroundPetId() const { return m_battlegroundLastPetId; }
void Player::setLastBattlegroundPetSpell(uint32_t petSpell) { m_battlegroundLastPetSpell = petSpell; }
uint32_t Player::getLastBattlegroundPetSpell() const { return m_battlegroundLastPetSpell; }

void Player::setStableSlotCount(uint8_t count) { m_stableSlotCount = count; }
uint8_t Player::getStableSlotCount() const { return m_stableSlotCount; }

void Player::eventSummonPet(Pet* summonPet)
{
    if (summonPet)
    {
        for (const auto& spellId : m_spellSet)
        {
            if (const auto spellInfo = sSpellMgr.getSpellInfo(spellId))
            {
                if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER)
                {
                    this->removeAllAurasByIdForGuid(spellId, this->getGuid());
                    SpellCastTargets targets(this->getGuid());
                    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
                    spell->prepare(&targets);
                }

                if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET)
                {
                    this->removeAllAurasByIdForGuid(spellId, this->getGuid());
                    SpellCastTargets targets(summonPet->getGuid());
                    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
                    spell->prepare(&targets);
                }
            }
        }

        for (const auto& aura : getAuraList())
            if (aura && aura->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_ON_PET)
                aura->removeAura();
    }
}

void Player::eventDismissPet()
{
    for (const auto& aura : getAuraList())
        if (aura && aura->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
            aura->removeAura();
}

Object* Player::getSummonedObject() const { return m_summonedObject; }
void Player::setSummonedObject(Object* summonedObject) { m_summonedObject = summonedObject; }

void Player::_spawnPet(PetCache const* petCache)
{
    const auto pet = sObjectMgr.createPet(petCache->entry, nullptr);
    if (!pet->loadFromDB(this, petCache))
    {
        pet->DeleteMe();
        return;
    }

    // TODO: find a better way to handle these -Appled
    if (petCache->spellid)
    {
        removeAllAurasById(18789);
        removeAllAurasById(18790);
        removeAllAurasById(18791);
        removeAllAurasById(18792);
        removeAllAurasById(35701);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void Player::loadBoundInstances()
{
    for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
        m_boundInstances[i].clear();

    auto group = getGroup();

    //                                             0          1    2           3            4          5
    auto result = CharacterDatabase.Query("SELECT id, permanent, map, difficulty, extendState, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid =  %u", getGuidLow());
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            bool perm = fields[1].asBool();
            uint32_t mapId = fields[2].asUint16();
            uint32_t instanceId = fields[0].asUint32();
            uint8_t difficulty = fields[3].asUint8();
            BindExtensionState extendState = BindExtensionState(fields[4].asUint8());

            time_t resetTime = time_t(fields[5].asUint64());
            bool deleteInstance = false;

            WDB::Structures::MapEntry const* mapEntry = sMapStore.lookupEntry(mapId);
#if VERSION_STRING > WotLK
            std::string mapname = mapEntry ? mapEntry->map_name[0] : "Unknown";
#else
            std::string mapname = mapEntry ? mapEntry->map_name[sWorld.getDbcLocaleLanguageId()] : "Unknown";
#endif

            if (!mapEntry || !mapEntry->isInstanceMap())
            {
                sLogger.failure("Player::loadBoundInstances: Player '{}' ({}) has bind to not existed or not dungeon map {} ({})",
                    getName(), getGuid(), mapId, mapname);
                deleteInstance = true;
            }
            else if (difficulty >= InstanceDifficulty::MAX_DIFFICULTY)
            {
                sLogger.failure("entities.player", "Player::loadBoundInstances: player '{}' ({}) has bind to not existed difficulty {} instance for map {} ({})",
                    getName(), getGuid(), difficulty, mapId, mapname);
                deleteInstance = true;
            }
            else
            {
                WDB::Structures::MapDifficulty const* mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(difficulty));
                if (!mapDiff)
                {
                    sLogger.failure("entities.player", "Player::loadBoundInstances: player '{}' ({}) has bind to not existed difficulty {} instance for map {} ({})",
                        getName(), getGuid(), difficulty, mapId, mapname);
                    deleteInstance = true;
                }
                else if (!perm && group)
                {
                    sLogger.failure("entities.player", "Player::loadBoundInstances: player '{}' ({}) is in group {} but has a non-permanent character bind to map {} ({}), {}, {}",
                        getName(), getGuid(), group->GetGUID(), mapId, mapname, instanceId, difficulty);
                    deleteInstance = true;
                }
            }

            if (deleteInstance)
            {
                CharacterDatabase.Execute("DELETE FROM character_instance WHERE guid = %u AND instance = %u", getGuidLow(), instanceId);
                continue;
            }

            // since non permanent binds are always solo bind, they can always be reset
            if (InstanceSaved* save = sInstanceMgr.addInstanceSave(mapId, instanceId, InstanceDifficulty::Difficulties(difficulty), resetTime, !perm, true))
                bindToInstance(save, perm, extendState, true);

        } while (result->NextRow());
    }
}

InstancePlayerBind* Player::getBoundInstance(uint32_t mapId, InstanceDifficulty::Difficulties difficulty, bool withExpired)
{
#if VERSION_STRING > TBC
    // some instances only have one difficulty
    auto const* mapDiff = getDownscaledMapDifficultyData(mapId, difficulty);
    if (!mapDiff)
        return nullptr;
#endif

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapId);
    if (itr != m_boundInstances[difficulty].end())
        if (itr->second.extendState || withExpired)
            return &itr->second;
    return nullptr;
}

InstanceSaved* Player::getInstanceSave(uint32_t mapId, bool isRaid)
{
    InstancePlayerBind* pBind = getBoundInstance(mapId, getDifficulty(isRaid));
    InstanceSaved* pSave = pBind ? pBind->save : nullptr;
    if (!pBind || !pBind->perm)
        if (auto group = getGroup())
            if (InstanceGroupBind* groupBind = group->getBoundInstance(getDifficulty(isRaid), mapId))
                pSave = groupBind->save;

    return pSave;
}

void Player::unbindInstance(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    unbindInstance(itr, difficulty, unload);
}

void Player::unbindInstance(BoundInstancesMap::iterator& itr, InstanceDifficulty::Difficulties difficulty, bool unload)
{
    if (itr != m_boundInstances[difficulty].end())
    {
        if (!unload)
        {
            CharacterDatabase.Execute("DELETE FROM character_instance WHERE guid = %u AND instance = %u", getGuidLow(), itr->second.save->getInstanceId());
        }

#if VERSION_STRING > TBC
        if (itr->second.perm)
            getSession()->sendCalendarRaidLockout(itr->second.save, false);
#endif
        itr->second.save->removePlayer(this);               // save can become invalid
        m_boundInstances[difficulty].erase(itr++);
    }
}

InstancePlayerBind* Player::bindToInstance(InstanceSaved* save, bool permanent, BindExtensionState extendState, bool load)
{
    if (save)
    {
        InstancePlayerBind& bind = m_boundInstances[save->getDifficulty()][save->getMapId()];
        if (extendState == EXTEND_STATE_KEEP) // special flag, keep the player's current extend state when updating for new boss down
        {
            if (save == bind.save)
                extendState = bind.extendState;
            else
                extendState = EXTEND_STATE_NORMAL;
        }
        if (!load)
        {
            if (bind.save)
            {
                // update the save when the group kills a boss
                if (permanent != bind.perm || save != bind.save || extendState != bind.extendState)
                {
                    CharacterDatabase.Execute("UPDATE character_instance SET instance = %u, permanent = %u, extendState = %u WHERE guid = %u AND instance = %u", save->getInstanceId(), permanent, extendState, getGuidLow(), bind.save->getInstanceId());
                }
            }
            else
            {
                CharacterDatabase.Execute("INSERT INTO character_instance (guid, instance, permanent, extendState) VALUES (%u, %u, %u, %u)", getGuidLow(), save->getInstanceId(), permanent, extendState);
            }
        }

        if (bind.save != save)
        {
            if (bind.save)
                bind.save->removePlayer(this);

            save->addPlayer(this);
        }

        if (permanent)
            save->setCanReset(false);

        bind.save = save;
        bind.perm = permanent;
        bind.extendState = extendState;
        return &bind;
    }

    return nullptr;
}

void Player::bindToInstance()
{
    InstanceSaved* mapSave = sInstanceMgr.getInstanceSave(m_pendingBindId);
    if (!mapSave)
        return;

    WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
    data << uint32_t(0);
    sendPacket(&data);
    if (!isGMFlagSet())
    {
        bindToInstance(mapSave, true, EXTEND_STATE_KEEP);
#if VERSION_STRING > TBC
        getSession()->sendCalendarRaidLockout(mapSave, true);
#endif
    }
}

void Player::setPendingBind(uint32_t instanceId, uint32_t bindTimer)
{
    m_pendingBindId = instanceId;
    m_pendingBindTimer = bindTimer;
}

void Player::sendRaidInfo()
{
    uint32_t counter = 0;

    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);

    size_t p_counter = data.wpos();
    data << uint32_t(counter);

    const auto now = Util::getTimeNow();

    for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            InstancePlayerBind const& bind = itr->second;
            if (bind.perm)
            {
                InstanceSaved* save = bind.save;
                data << uint32_t(save->getMapId());
#if VERSION_STRING > TBC
                data << uint32_t(save->getDifficulty());
                data << uint64_t(save->getInstanceId());
                data << uint8_t(bind.extendState != EXTEND_STATE_EXPIRED);
                data << uint8_t(bind.extendState == EXTEND_STATE_EXTENDED);

                time_t nextReset = save->getResetTime();
                if (bind.extendState == EXTEND_STATE_EXTENDED)
                    nextReset = sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime());

                data << uint32_t(nextReset - now);
#else
                time_t nextReset = save->getResetTime();
                if (bind.extendState == EXTEND_STATE_EXTENDED)
                    nextReset = sInstanceMgr.getSubsequentResetTime(save->getMapId(), save->getDifficulty(), save->getResetTime());

                data << uint32_t(nextReset - now);

                data << uint32_t(save->getInstanceId());
                data << uint32_t(counter);
#endif

                ++counter;
            }
        }
    }
    data.put<uint32_t>(p_counter, counter);
    sendPacket(&data);
}

void Player::sendSavedInstances()
{
    bool hasBeenSaved = false;
    WorldPacket data;

    for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            if (itr->second.perm)
            {
                hasBeenSaved = true;
                break;
            }
        }
    }

    // true or false means, whether you have current raid/heroic instances
    data.Initialize(SMSG_UPDATE_INSTANCE_OWNERSHIP);
    data << uint32_t(hasBeenSaved);
    sendPacket(&data);

    if (!hasBeenSaved)
        return;

    for (uint8_t i = 0; i < InstanceDifficulty::MAX_DIFFICULTY; ++i)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            if (itr->second.perm)
            {
                data.Initialize(SMSG_UPDATE_LAST_INSTANCE);
                data << uint32_t(itr->second.save->getMapId());
                sendPacket(&data);
            }
        }
    }
}

void Player::sendInstanceResetWarning(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, uint32_t time, bool welcome)
{
    // type of warning, based on the time remaining until reset
    uint32_t type;
    if (welcome)
        type = RAID_INSTANCE_WELCOME;
    else if (time > 21600)
        type = RAID_INSTANCE_WELCOME;
    else if (time > 3600)
        type = RAID_INSTANCE_WARNING_HOURS;
    else if (time > 300)
        type = RAID_INSTANCE_WARNING_MIN;
    else
        type = RAID_INSTANCE_WARNING_MIN_SOON;

    WorldPacket data(SMSG_RAID_INSTANCE_MESSAGE, 4 + 4 + 4 + 4);
    data << uint32_t(type);
    data << uint32_t(mapid);
    data << uint32_t(difficulty);   // difficulty
    data << uint32_t(time);

    if (type == RAID_INSTANCE_WELCOME)
    {
        data << uint8_t(0); // is locked
        data << uint8_t(0); // is extended, ignored if prev field is 0
    }

    sendPacket(&data);
}

void Player::resetInstances(uint8_t method, bool isRaid)
{
    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_JOIN

    // we assume that when the difficulty changes, all instances that can be reset will be
    InstanceDifficulty::Difficulties diff = getDifficulty(isRaid);

    for (BoundInstancesMap::iterator itr = m_boundInstances[diff].begin(); itr != m_boundInstances[diff].end();)
    {
        InstanceSaved* p = itr->second.save;
        WDB::Structures::MapEntry const* entry = sMapStore.lookupEntry(itr->first);
        if (!entry || entry->isRaid() != isRaid || !p->canReset())
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->map_type == MAP_RAID || diff == InstanceDifficulty::Difficulties::DUNGEON_HEROIC)
            {
                ++itr;
                continue;
            }
        }

        // if the map is loaded, reset it
        WorldMap* map = sMapMgr.findWorldMap(p->getMapId(), p->getInstanceId());
        if (map && map->getBaseMap()->isInstanceMap())
            if (!reinterpret_cast<InstanceMap*>(map)->reset(method))
            {
                ++itr;
                continue;
            }

        // since this is a solo instance there should not be any players inside
        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
            sendPacket(SmsgInstanceReset(p->getMapId()).serialise().get());

        p->deleteFromDB();
        m_boundInstances[diff].erase(itr++);

        // the following should remove the instance save from the manager and delete it as well
        p->removePlayer(this);
    }
}

void Player::sendResetInstanceFailed(uint32_t reason, uint32_t MapId)
{
    // reasons for instance reset failure:
    // 0: There are players inside the instance.
    // 1: There are players offline in your party.
    // 2: There are players in your party attempting to zone into an instance.
    WorldPacket data(SMSG_INSTANCE_RESET_FAILED, 4);
    data << uint32_t(reason);
    data << uint32_t(MapId);
    sendPacket(&data);
}

void Player::loadInstanceTimeRestrictions()
{
    auto result = CharacterDatabase.Query("SELECT instanceId, releaseTime FROM account_instance_times WHERE accountId = %u", getSession()->GetAccountId());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        m_instanceResetTimes.insert(InstanceTimeMap::value_type(fields[0].asUint32(), fields[1].asUint64()));
    } while (result->NextRow());
}

bool Player::checkInstanceCount(uint32_t instanceId) const
{
    //\todo: Max Instances Per Hour is per Default 5 add these to Configs
    if (m_instanceResetTimes.size() < 5)
        return true;

    return m_instanceResetTimes.find(instanceId) != m_instanceResetTimes.end();
}

void Player::addInstanceEnterTime(uint32_t instanceId, time_t enterTime)
{
    if (m_instanceResetTimes.find(instanceId) == m_instanceResetTimes.end())
        m_instanceResetTimes.insert(InstanceTimeMap::value_type(instanceId, enterTime + HOUR));
}

void Player::saveInstanceTimeRestrictions()
{
    if (m_instanceResetTimes.empty())
        return;

    CharacterDatabase.Execute("DELETE FROM account_instance_times WHERE accountId = %u", getSession()->GetAccountId());

    for (InstanceTimeMap::const_iterator itr = m_instanceResetTimes.begin(); itr != m_instanceResetTimes.end(); ++itr)
    {
        CharacterDatabase.Execute("INSERT INTO account_instance_times (accountId, instanceId, releaseTime) VALUES (%u, %u, %u)", getSession()->GetAccountId(), itr->first, itr->second);
    }
}


uint32_t Player::getMaxPersonalRating()
{
    uint32_t maxRating = 0;

    if (m_playerInfo)
    {
        for (uint8_t index = 0; index < NUM_ARENA_TEAM_TYPES; index++)
        {
            if (m_arenaTeams[index] != nullptr)
            {
                if (ArenaTeamMember* arenaTeamMember = m_arenaTeams[index]->getMemberByGuid(m_playerInfo->guid))
                {
                    if (arenaTeamMember->PersonalRating > maxRating)
                        maxRating = arenaTeamMember->PersonalRating;
                }
                else
                {
                    sLogger.failure("{}: GetMemberByGuid returned NULL for player guid = {}", __FUNCTION__, m_playerInfo->guid);
                }
            }
        }
    }
    return maxRating;
}

// Fills fields from firstField to firstField+fieldsNum-1 with integers from the string
void Player::loadFieldsFromString(const char* string, uint16_t /*firstField*/, uint32_t fieldsNum)
{
    if (string == nullptr)
        return;

    char* start = (char*)string;
    for (uint16_t Counter = 0; Counter < fieldsNum; Counter++)
    {
        char* end = strchr(start, ',');
        if (!end)
            break;

        *end = 0;
        setExploredZone(Counter, std::stoul(start));
        start = end + 1;
    }
}

void Player::calcExpertise()
{
    int32_t modifier = 0;

#if VERSION_STRING != Classic
    setExpertise(0);
    setOffHandExpertise(0);

    for (const auto& aurEff : getAuraEffectList(SPELL_AURA_EXPERTISE))
    {
        SpellInfo const* spellInfo = aurEff->getAura()->getSpellInfo();
        int32_t val = aurEff->getEffectDamage();

        if (spellInfo->getEquippedItemSubClass() != 0)
        {
            auto item_mainhand = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            auto item_offhand = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

            uint32_t reqskillMH = 0;
            uint32_t reqskillOH = 0;

            if (item_mainhand)
                reqskillMH = spellInfo->getEquippedItemSubClass() & (((uint32_t)1) << item_mainhand->getItemProperties()->SubClass);

            if (item_offhand)
                reqskillOH = spellInfo->getEquippedItemSubClass() & (((uint32_t)1) << item_offhand->getItemProperties()->SubClass);

            if (reqskillMH != 0 || reqskillOH != 0)
                modifier = +val;
        }
        else
            modifier += val;
    }

    modExpertise((int32_t)calcRating(CR_EXPERTISE) + modifier);
    modOffHandExpertise((int32_t)calcRating(CR_EXPERTISE) + modifier);
#endif
    updateStats();
}

void Player::updateKnownCurrencies(uint32_t itemId, bool apply)
{
#if VERSION_STRING == WotLK
    if (auto const* currency_type_entry = sCurrencyTypesStore.lookupEntry(itemId))
    {
        if (apply)
        {
            uint64_t oldval = getKnownCurrencies();
            uint64_t newval = oldval | (1LL << (currency_type_entry->bit_index - 1));
            setKnownCurrencies(newval);
        }
        else
        {
            uint64_t oldval = getKnownCurrencies();
            uint64_t newval = oldval & ~(1LL << (currency_type_entry->bit_index - 1));
            setKnownCurrencies(newval);
        }
    }
#else
    if (itemId == 0 || apply) { return; }
#endif
}

void Player::handleSpellLoot(uint32_t itemId)
{
    Loot loot1;
    sLootMgr.fillItemLoot(this, &loot1, itemId, 0);

    for (const auto& item : loot1.items)
    {
        uint32_t looteditemid = item.itemproto->ItemId;
        uint32_t count = item.count;

        getItemInterface()->AddItemById(looteditemid, count, 0);
    }
}

void Player::displayDataStateList()
{
    if (InstanceMap* instance = sMapMgr.findInstanceMap(GetInstanceID()))
        if (instance->getScript())
            instance->getScript()->displayDataStateList(this);
}

void Player::displayTimerList()
{
    if (InstanceMap* instance = sMapMgr.findInstanceMap(GetInstanceID()))
        if (instance->getScript())
            instance->getScript()->displayTimerList(this);
}
void Player::displayCreatureSetForEntry(uint32_t _creatureEntry)
{
    if (InstanceMap* instance = sMapMgr.findInstanceMap(GetInstanceID()))
        if (instance->getScript())
            instance->getScript()->getCreatureSetForEntry(_creatureEntry, true, this);
}

uint32_t Player::checkDamageLimits(uint32_t damage, uint32_t spellId)
{
    std::stringstream dmglog;

    if ((spellId != 0) && (worldConfig.limit.maxSpellDamageCap > 0))
    {
        if (damage > worldConfig.limit.maxSpellDamageCap)
        {
            dmglog << "Dealt " << damage << " with spell " << spellId;

            sCheatLog.writefromsession(m_session, dmglog.str().c_str());

            if (worldConfig.limit.disconnectPlayerForExceedingLimits != 0)
                m_session->Disconnect();

            damage = worldConfig.limit.maxSpellDamageCap;
        }
    }
    else if ((worldConfig.limit.maxAutoAttackDamageCap > 0) && (damage > worldConfig.limit.maxAutoAttackDamageCap))
    {
        dmglog << "Dealt " << damage << " with auto attack";
        sCheatLog.writefromsession(m_session, dmglog.str().c_str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits != 0)
            m_session->Disconnect();

        damage = worldConfig.limit.maxAutoAttackDamageCap;
    }

    if (worldConfig.limit.broadcastMessageToGmOnExceeding != 0)
        sendReportToGmMessage(getName(), dmglog.str());

    return damage;
}

uint32_t Player::getBlockDamageReduction()
{
    Item* item = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (item == nullptr || item->getItemProperties()->InventoryType != INVTYPE_SHIELD)
        return 0;

    float block_multiplier = (100.0f + this->m_modBlockAbsorbValue) / 100.0f;
    if (block_multiplier < 1.0f)
        block_multiplier = 1.0f;

    return Util::float2int32((item->getItemProperties()->Block + this->m_modBlockValueFromSpells + this->getCombatRating(CR_BLOCK) + this->getStat(STAT_STRENGTH) / 2.0f - 1.0f) * block_multiplier);
}

void Player::applyFeralAttackPower(bool apply, Item* item)
{
    float feralAttackPower = 0.0f;

    Item* applyItem = item;
    if (applyItem == nullptr)
        applyItem = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (applyItem)
    {
        float delay = static_cast<float>(applyItem->getItemProperties()->Delay) / 1000.0f;
        delay = std::max(1.0f, delay);
        float dps = ((applyItem->getItemProperties()->Damage[0].Min + applyItem->getItemProperties()->Damage[0].Max) / 2) / delay;
        if (dps > 54.8f)
            feralAttackPower = (dps - 54.8f) * 14;
    }
    modifyBonuses(ITEM_MOD_FERAL_ATTACK_POWER, static_cast<int32_t>(feralAttackPower), apply);
}

bool Player::saveReputations(bool newCharacter, QueryBuffer* buf)
{
    if (!newCharacter && (buf == nullptr))
        return false;

    std::stringstream ds;
    uint32_t guid = getGuidLow();

    ds << "DELETE FROM playerreputations WHERE guid = '";
    ds << guid;
    ds << "';";

    if (!newCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (ReputationMap::iterator itr = m_reputation.begin(); itr != m_reputation.end(); ++itr)
    {
        std::stringstream ss;

        ss << "INSERT INTO playerreputations VALUES('";
        ss << getGuidLow() << "','";
        ss << itr->first << "','";
        ss << uint32_t(itr->second->flag) << "','";
        ss << itr->second->baseStanding << "','";
        ss << itr->second->standing << "');";

        if (!newCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::saveSpells(bool newCharacter, QueryBuffer* buf)
{
    if (!newCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32_t guid = getGuidLow();

    ds << "DELETE FROM playerspells WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!newCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (const auto& spellid : m_spellSet)
    {
        std::stringstream ss;

        ss << "INSERT INTO playerspells VALUES('";
        ss << guid << "','";
        ss << spellid << "');";

        if (!newCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::loadDeletedSpells(QueryResult* result)
{
    if (result == nullptr)
        return false;

    do
    {
        Field* fields = result->Fetch();
        uint32_t spellid = fields[0].asUint32();

        const auto* const spellInfo = sSpellMgr.getSpellInfo(spellid);
        if (spellInfo == nullptr)
            continue;

        if (sSpellMgr.isSpellDisabled(spellid))
            continue;

        m_deletedSpellSet.emplace(spellid);
    } while (result->NextRow());

    return true;
}

bool Player::saveDeletedSpells(bool newCharacter, QueryBuffer* buf)
{
    if (!newCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32_t guid = getGuidLow();

    ds << "DELETE FROM playerdeletedspells WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!newCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (const auto& spellid : m_deletedSpellSet)
    {
        std::stringstream ss;

        ss << "INSERT INTO playerdeletedspells VALUES('";
        ss << guid << "','";
        ss << spellid << "');";

        if (!newCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

bool Player::saveSkills(bool newCharacter, QueryBuffer* buf)
{
    if (!newCharacter && buf == nullptr)
        return false;

    std::stringstream ds;
    uint32_t guid = getGuidLow();

    ds << "DELETE FROM playerskills WHERE GUID = '";
    ds << guid;
    ds << "';";

    if (!newCharacter)
        buf->AddQueryStr(ds.str());
    else
        CharacterDatabase.ExecuteNA(ds.str().c_str());

    for (SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); ++itr)
    {
        uint32_t skillid = itr->first;
        uint32_t currval = itr->second.CurrentValue;
        uint32_t maxval = itr->second.MaximumValue;

        // Skip only initialized values
        if (currval == 0)
            continue;

        std::stringstream ss;

        ss << "INSERT INTO playerskills VALUES('";
        ss << guid << "','";
        ss << skillid << "','";
        ss << currval << "','";
        ss << maxval << "');";

        if (!newCharacter)
            buf->AddQueryStr(ss.str());
        else
            CharacterDatabase.ExecuteNA(ss.str().c_str());
    }

    return true;
}

void Player::_castSpellArea()
{
    if (!IsInWorld())
        return;

    if (m_position.x > Map::Terrain::_maxX || m_position.x < Map::Terrain::_minX || m_position.y > Map::Terrain::_maxY || m_position.y < Map::Terrain::_minY)
        return;

    if (getWorldMap()->getCellByCoords(GetPositionX(), GetPositionY()) == nullptr)
        return;

    uint32_t AreaId = 0;
    uint32_t ZoneId = 0;

    getWorldMap()->getZoneAndAreaId(GetPhase(), ZoneId, AreaId, GetPosition());

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Cheks for Casting a Spell in Specified Area / Zone :D

    // Spells get Casted in specified Area
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.getSpellAreaForAreaMapBounds(AreaId);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        if (itr->second->autoCast && itr->second->fitsToRequirements(this, ZoneId, AreaId))
            if (!hasAurasWithId(itr->second->spellId))
                castSpell(this, itr->second->spellId, true);


    // Some spells applied at enter into zone (with subzones)
    SpellAreaForAreaMapBounds szBounds = sSpellMgr.getSpellAreaForAreaMapBounds(ZoneId);
    for (SpellAreaForAreaMap::const_iterator itr = szBounds.first; itr != szBounds.second; ++itr)
        if (itr->second->autoCast && itr->second->fitsToRequirements(this, ZoneId, AreaId))
            if (!hasAurasWithId(itr->second->spellId))
                castSpell(this, itr->second->spellId, true);


    // Remove of Spells
    for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
    {
        if (auto* const aur = getAuraWithAuraSlot(i))
        {
            if (sSpellMgr.checkLocation(aur->getSpellInfo(), ZoneId, AreaId, this) == false)
            {
                SpellAreaMapBounds sab = sSpellMgr.getSpellAreaMapBounds(aur->getSpellId());
                if (sab.first != sab.second)
                    removeAllAurasById(aur->getSpellId());
            }
        }
    }
}

void Player::processPendingUpdates()
{
    m_updateMgr.processPendingUpdates();
}

void Player::eventTalentHearthOfWildChange(bool apply)
{
    if (!m_hearthOfWildPct)
        return;

    int tval;
    if (apply)
        tval = m_hearthOfWildPct;
    else
        tval = -m_hearthOfWildPct;

    uint32_t shapeShiftForm = getShapeShiftForm();

    if (shapeShiftForm == FORM_BEAR || shapeShiftForm == FORM_DIREBEAR)
    {
        m_totalStatModPctPos[STAT_STAMINA] += tval;
        calcStat(STAT_STAMINA);
        updateStats();

#if VERSION_STRING >= TBC
        updateChances();
#endif
    }
    else if (shapeShiftForm == FORM_CAT)
    {
        setAttackPowerMultiplier(getAttackPowerMultiplier() + tval / 200.0f);
        setRangedAttackPowerMultiplier(getRangedAttackPowerMultiplier() + tval / 200.0f);
        updateStats();
    }
}


void Player::_eventAttack(bool offhand)
{
    if (isCastingSpell())
    {
        setAttackTimer(offhand == true ? OFFHAND : MELEE, 100);
        return;
    }

    if (isFeared() || isStunned())
        return;

    Unit* pVictim = nullptr;
    if (getTargetGuid())
        pVictim = getWorldMap()->getUnit(getTargetGuid());

    if (!pVictim)
    {
        sLogger.info("Player::Update: No valid current selection to attack, stopping attack");
        interruptHealthRegeneration(5000); //prevent clicking off creature for a quick heal
        eventAttackStop();
        return;
    }

    if (!this->isValidTarget(pVictim))
    {
        interruptHealthRegeneration(5000);
        eventAttackStop();
        return;
    }

    if (!canReachWithAttack(pVictim))
    {
        if (m_AttackMsgTimer != 1)
        {
#if VERSION_STRING < Mop
            sendPacket(SmsgAttackSwingNotInRange().serialise().get());
#endif
            m_AttackMsgTimer = 1;
        }
        setAttackTimer(offhand == true ? OFFHAND : MELEE, 300);
    }
    else if (!isInFront(pVictim))
    {
        // We still have to do this one.
        if (m_AttackMsgTimer != 2)
        {
#if VERSION_STRING < Mop
            sendPacket(SmsgAttackSwingBadFacing().serialise().get());
#endif
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
            if (static_cast<Player*>(pVictim)->m_cannibalize)
            {
                sEventMgr.RemoveEvents(pVictim, EVENT_CANNIBALIZE);
                pVictim->setEmoteState(EMOTE_ONESHOT_NONE);
                static_cast<Player*>(pVictim)->m_cannibalize = false;
            }
        }

        if (this->isStealthed())
            removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        if (getOnMeleeSpell() == 0 || offhand)
            strike(pVictim, (offhand ? OFFHAND : MELEE), nullptr, 0, 0, 0, false, false);
        else
            castOnMeleeSpell();
    }
}

void Player::eventCharmAttack()
{
    if (!getCharmGuid())
        return;

    if (!IsInWorld())
    {
        setCharmGuid(0);
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    if (getTargetGuid() == 0)
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    Unit* pVictim = getWorldMap()->getUnit(getTargetGuid());
    if (!pVictim)
    {
        sLogger.failure("WORLD: {} doesn't exist.", std::to_string(getTargetGuid()));
        sLogger.info("Player::Update: No valid current selection to attack, stopping attack");
        this->interruptHealthRegeneration(5000); //prevent clicking off creature for a quick heal
        // todo
        //removeUnitStateFlag(UNIT_STATE_ATTACKING);
        eventAttackStop();
    }
    else
    {
        Unit* currentCharm = getWorldMap()->getUnit(getCharmGuid());
        if (!currentCharm)
            return;

        if (!currentCharm->canReachWithAttack(pVictim))
        {
            if (m_AttackMsgTimer == 0)
                m_AttackMsgTimer = 2000;

            sEventMgr.ModifyEventTimeLeft(this, EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else if (!currentCharm->isInFront(pVictim))
        {
            if (m_AttackMsgTimer == 0)
            {
#if VERSION_STRING < Mop
                sendPacket(SmsgAttackSwingBadFacing().serialise().get());
#endif
                m_AttackMsgTimer = 2000; // 2 sec till next msg.
            }

            sEventMgr.ModifyEventTimeLeft(this, EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else
        {
            if (!currentCharm->getOnMeleeSpell())
            {
                currentCharm->strike(pVictim, MELEE, nullptr, 0, 0, 0, false, false);
            }
            else
            {
                const auto spellInfo = sSpellMgr.getSpellInfo(currentCharm->getOnMeleeSpell());
                currentCharm->setOnMeleeSpell(0);
                Spell* spell = sSpellMgr.newSpell(currentCharm, spellInfo, true, nullptr);
                SpellCastTargets targets(getTargetGuid());
                spell->prepare(&targets);
            }
        }
    }
}

void Player::eventAttackStart()
{
    m_attacking = true;
    dismount();
}

void Player::eventAttackStop()
{
    if (getCharmGuid() != 0)
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_CHARM_ATTACK);

    m_attacking = false;
}

void Player::eventDeath()
{
    if (!IS_INSTANCE(GetMapId()) && !sEventMgr.HasEvent(this, EVENT_PLAYER_FORCED_RESURRECT)) //Should never be true
        sEventMgr.AddEvent(this, &Player::repopRequest, EVENT_PLAYER_FORCED_RESURRECT, forcedResurrectInterval, 1, 0); //in case he forgets to release spirit (afk or something)

    removeAllNegativeAuras();

    setServersideDrunkValue(0);
}

void Player::_savePet(QueryBuffer* buf, bool updateCurrentPetCache/* = false*/, Pet* currentPet/* = nullptr*/)
{
    // Remove any existing m_playerCreateInfo
    if (buf == nullptr)
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", getGuidLow());
    else
        buf->AddQuery("DELETE FROM playerpets WHERE ownerguid = %u", getGuidLow());

    const auto* summon = currentPet != nullptr ? currentPet : getPet();
    if (summon && summon->IsInWorld())    // update PlayerPets array with current pet's m_playerCreateInfo
    {
        if (updateCurrentPetCache)
        {
            const auto playerPetCache = getPetCache(summon->getPetId());
            if (playerPetCache != nullptr && playerPetCache->active)
                summon->updatePetInfo(false);
            else
                summon->updatePetInfo(true);
        }

        if (summon->isHunterPet())       // is a pet
        {
            // save pet spellz
            auto pn = summon->getPetId();
            if (buf == nullptr)
                CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), pn);
            else
                buf->AddQuery("DELETE FROM playerpetspells WHERE ownerguid=%u AND petnumber=%u", getGuidLow(), pn);

            for (const auto& [spell, state] : summon->getSpellMap())
            {
                if (buf == nullptr)
                    CharacterDatabase.Execute("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", getGuidLow(), pn, spell, state);
                else
                    buf->AddQuery("INSERT INTO playerpetspells VALUES(%u, %u, %u, %u)", getGuidLow(), pn, spell, state);
            }
        }
    }

    std::stringstream ss;

    ss.rdbuf()->str("");

    std::optional<uint8_t> currentPetId = std::nullopt;
    if (getPet() != nullptr && getPet()->isPermanentSummon())
        currentPetId = getPet()->getPetId();

    std::vector<uint8_t> savedPetIds;
    savedPetIds.reserve(m_cachedPets.size());

    auto foundActivePet = false;
    for (auto itr = m_cachedPets.cbegin(); itr != m_cachedPets.cend();)
    {
        auto* petCache = itr->second.get();

        // Do some clean up to pet cache before save
        if (currentPetId.has_value())
        {
            // Set all other pets expect current pet to offline
            if (petCache->active && petCache->number != currentPetId.value())
                petCache->active = false;

            // Remove all other cached pets from non-hunters expect current pet
            if (!isClassHunter() && getPetCount() > 1)
            {
                if (petCache->number != currentPetId.value())
                {
                    std::erase_if(m_cachedPetSlots, [&petCache](const auto& slotItr) { return slotItr.second == petCache->number; });
                    itr = m_cachedPets.erase(itr);
                    continue;
                }
            }
        }
        else
        {
            // There can be only one active pet
            if (petCache->active)
            {
                if (petCache->slot >= PET_SLOT_FIRST_STABLE_SLOT)
                    petCache->active = false;
                else if (foundActivePet)
                    petCache->active = false;
                else
                    foundActivePet = true;
            }

            // Only hunters can have multiple pets saved
            if (!isClassHunter() && getPetCount() > 1)
            {
                if (!petCache->active)
                {
                    std::erase_if(m_cachedPetSlots, [&petCache](const auto& slotItr) { return slotItr.second == petCache->number; });
                    itr = m_cachedPets.erase(itr);
                    continue;
                }
            }
        }

        ss.rdbuf()->str("");

        ss << "REPLACE INTO playerpets VALUES('"
            << getGuidLow() << "','"
            << std::to_string(petCache->number) << "','"
            << std::to_string(petCache->type) << "','"
            << petCache->name << "','"
            << petCache->entry << "','"
            << petCache->model << "','"
            << petCache->level << "','"
            << petCache->xp << "','"
            << std::to_string(petCache->slot) << "','"
            << petCache->active << "','"
            << petCache->alive << "','"
            << petCache->actionbar << "','"
            << static_cast<long>(petCache->reset_time) << "','"
            << petCache->reset_cost << "','"
            << petCache->spellid << "','"
            << std::to_string(petCache->petstate) << "','"
            << petCache->talentpoints << "','"
            << petCache->current_power << "','"
            << petCache->current_hp << "','"
            << petCache->current_happiness << "','"
            << petCache->renamable << "')";

        if (buf == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryStr(ss.str());

        savedPetIds.push_back(petCache->number);
        ++itr;
    }

    // Cleanup as well pet spell table by removing spells from non existant pets
    ss.rdbuf()->str("");
    ss << "DELETE FROM playerpetspells WHERE ownerguid=" << getGuidLow();
    if (!savedPetIds.empty())
    {
        ss << " AND petnumber NOT IN (";
        for (auto itr = savedPetIds.cbegin(); itr != savedPetIds.cend();)
        {
            ss << std::to_string(*itr);
            if (++itr != savedPetIds.cend())
                ss << ", ";
            else
                ss << ")";
        }
    }

    if (buf == nullptr)
        CharacterDatabase.ExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());
}

void Player::_savePetSpells(QueryBuffer* buf)
{
    // Remove any existing
    if (buf == nullptr)
        CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid=%u", getGuidLow());
    else
        buf->AddQuery("DELETE FROM playersummonspells WHERE ownerguid=%u", getGuidLow());

    // Save summon spells
    for (std::map<uint32_t, std::set<uint32_t> >::iterator itr = m_summonSpells.begin(); itr != m_summonSpells.end(); ++itr)
    {
        for (std::set<uint32_t>::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            if (buf == nullptr)
                CharacterDatabase.Execute("INSERT INTO playersummonspells VALUES(%u, %u, %u)", getGuidLow(), itr->first, (*it));
            else
                buf->AddQuery("INSERT INTO playersummonspells VALUES(%u, %u, %u)", getGuidLow(), itr->first, (*it));
        }
    }
}

void Player::addSummonSpell(uint32_t entry, uint32_t spellId)
{
    SpellInfo const* sp = sSpellMgr.getSpellInfo(spellId);
    std::map<uint32_t, std::set<uint32_t> >::iterator itr = m_summonSpells.find(entry);
    if (itr == m_summonSpells.end())
    {
        m_summonSpells[entry].insert(spellId);
    }
    else
    {
        if (sp->hasSpellRanks())
        {
            std::set<uint32_t>::iterator it3;
            for (std::set<uint32_t>::iterator it2 = itr->second.begin(); it2 != itr->second.end();)
            {
                it3 = it2++;
                const auto se = sSpellMgr.getSpellInfo(*it3);
                if (se == nullptr || !se->hasSpellRanks())
                    continue;

                if (sp->getRankInfo()->isSpellPartOfThisSpellRankChain(se))
                    itr->second.erase(it3);
            }
        }
        itr->second.insert(spellId);
    }
}

void Player::removeSummonSpell(uint32_t entry, uint32_t spellId)
{
    std::map<uint32_t, std::set<uint32_t> >::iterator itr = m_summonSpells.find(entry);
    if (itr != m_summonSpells.end())
    {
        itr->second.erase(spellId);
        if (itr->second.size() == 0)
            m_summonSpells.erase(itr);
    }
}

std::set<uint32_t>* Player::getSummonSpells(uint32_t entry)
{
    std::map<uint32_t, std::set<uint32_t> >::iterator itr = m_summonSpells.find(entry);
    if (itr != m_summonSpells.end())
        return &itr->second;

    return nullptr;
}

void Player::_loadPet(QueryResult* result)
{
    m_maxPetNumber = 0;
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        auto pet = std::make_unique<PetCache>();
        pet->number = fields[1].asUint8();
        pet->type = fields[2].asUint8();
        pet->name = fields[3].asCString();
        pet->entry = fields[4].asUint32();

        // Check that creature properties exist
        const auto creatureProperties = sMySQLStore.getCreatureProperties(pet->entry);
        if (creatureProperties == nullptr)
            continue;

        pet->model = fields[5].asUint32();
        pet->level = fields[6].asUint32();
        pet->xp = fields[7].asUint32();
        pet->slot = fields[8].asUint8();
        pet->active = fields[9].asBool();
        pet->alive = fields[10].asBool();
        pet->actionbar = fields[11].asCString();
        pet->reset_time = fields[12].asUint32();
        pet->reset_cost = fields[13].asUint32();
        pet->spellid = fields[14].asUint32();
        pet->petstate = fields[15].asUint8();
        pet->talentpoints = fields[16].asUint32();
        pet->current_power = fields[17].asUint32();
        pet->current_hp = fields[18].asUint32();
        pet->current_happiness = fields[19].asUint32();
        pet->renamable = fields[20].asBool();

        // Check if there are pets using same slot
        // Also check for invalid pet slot in classic - wotlk
        // Could happen when server changes from cata to wotlk
        if (m_cachedPetSlots.contains(pet->slot)
#if VERSION_STRING < Cata
            || (pet->slot >= PET_SLOT_MAX_ACTIVE_SLOT && pet->slot < PET_SLOT_FIRST_STABLE_SLOT)
#endif
            )
        {
            if (pet->type != PET_TYPE_HUNTER)
            {
                // If other than hunter pet has invalid slot or is in duplicate slot, just remove it
                // They can be resummoned anyway
                continue;
            }

#if VERSION_STRING >= Cata
            auto foundNewSlot = false;
            if (pet->slot < PET_SLOT_FIRST_STABLE_SLOT)
            {
                // Pet is in active slot, try find another active slot
                const auto freeActiveSlot = findFreeActivePetSlot();
                if (freeActiveSlot.has_value())
                {
                    pet->slot = freeActiveSlot.value();
                    foundNewSlot = true;
                }
            }

            if (!foundNewSlot)
#endif
            {
                // Next try find free stable slot
                const auto freeStableSlot = findFreeStablePetSlot();
                if (!freeStableSlot.has_value())
                {
                    // There were no free slots left, remove pet
                    continue;
                }

                pet->slot = freeStableSlot.value();
            }
        }

        if (pet->type != PET_TYPE_HUNTER)
        {
            // Skip dead or inactive summoned pets
            // They should not be saved anyway
            if (!pet->active || !pet->alive)
                continue;
        }

        // Pet in stables cannot be active
        if (pet->slot >= PET_SLOT_FIRST_STABLE_SLOT && pet->active)
            pet->active = false;

        if (pet->number > m_maxPetNumber)
            m_maxPetNumber = pet->number;

        addPetCache(std::move(pet), pet->number);
    } while (result->NextRow());
}

void Player::_loadPetSpells(QueryResult* result)
{
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32_t entry = fields[1].asUint32();
            uint32_t spell = fields[2].asUint32();
            addSummonSpell(entry, spell);
        } while (result->NextRow());
    }
}

void Player::saveToDB(bool newCharacter /* =false */)
{
    bool in_arena = false;
    std::unique_ptr<QueryBuffer> bufPtr = nullptr;
    QueryBuffer* buf = nullptr;
    if (!newCharacter)
    {
        bufPtr = std::make_unique<QueryBuffer>();
        buf = bufPtr.get();
    }

    if (m_bg != nullptr && m_bg->isArena())
        in_arena = true;

    //Calc played times
    uint32_t playedt = (uint32_t)UNIXTIME - m_playedTime[2];
    m_playedTime[0] += playedt;
    m_playedTime[1] += playedt;
    m_playedTime[2] += playedt;

    // active cheats
    uint32_t active_cheats = PLAYER_CHEAT_NONE;
    if (m_cheats.hasCooldownCheat)
        active_cheats |= PLAYER_CHEAT_COOLDOWN;
    if (m_cheats.hasCastTimeCheat)
        active_cheats |= PLAYER_CHEAT_CAST_TIME;
    if (m_cheats.hasGodModeCheat)
        active_cheats |= PLAYER_CHEAT_GOD_MODE;
    if (m_cheats.hasPowerCheat)
        active_cheats |= PLAYER_CHEAT_POWER;
    if (m_cheats.hasFlyCheat)
        active_cheats |= PLAYER_CHEAT_FLY;
    if (m_cheats.hasAuraStackCheat)
        active_cheats |= PLAYER_CHEAT_AURA_STACK;
    if (m_cheats.hasItemStackCheat)
        active_cheats |= PLAYER_CHEAT_ITEM_STACK;
    if (m_cheats.hasTriggerpassCheat)
        active_cheats |= PLAYER_CHEAT_TRIGGERPASS;
    if (m_cheats.hasTaxiCheat)
        active_cheats |= PLAYER_CHEAT_TAXI;

    std::stringstream ss;

    ss << "REPLACE INTO characters VALUES (" << getGuidLow() << ", " << getSession()->GetAccountId() << ", " << "'" << m_name << "', "
        << uint32_t(getRace()) << ", " << uint32_t(getClass()) << ", " << uint32_t(getGender()) << ", " << getFactionTemplate() << ", ";

    ss << uint32_t(getLevel()) << ", " << getXp() << ", " << active_cheats << ", ";

    // exploration data
    ss << "'";
    for (uint8_t i = 0; i < WOWPLAYER_EXPLORED_ZONES_COUNT; ++i)
        ss << getExploredZone(i) << ",";
    ss << "', ";

    saveSkills(newCharacter, buf);

    ss << getWatchedFaction() << ", "
#if VERSION_STRING > Classic
        << getChosenTitle() << ", "
#else
        << uint32_t(0) << ", "
#endif

#if VERSION_STRING > Classic
        << getKnownTitles(0) << ", "
#else
        << uint32_t(0) << ", "
#endif

#if VERSION_STRING < WotLK
        << uint32_t(0) << ", " << uint32_t(0) << ", "
#else
        << getKnownTitles(1) << ", " << getKnownTitles(2) << ", "
#endif
        << getCoinage() << ", ";

    if (getClass() == MAGE || getClass() == PRIEST || (getClass() == WARLOCK))
        ss << uint32_t(0) << ", "; // make sure ammo slot is 0 for these classes, otherwise it can mess up wand shoot
    else
#if VERSION_STRING < Cata
        ss << getAmmoId() << ", ";
#else
        ss << uint32_t(0) << ", ";
#endif

    ss << getFreePrimaryProfessionPoints() << ", ";

    ss << m_loadHealth << ", " << m_loadMana << ", " << uint32_t(getPvpRank()) << ", " << getPlayerBytes() << ", " << getPlayerBytes2() << ", ";

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

#if VERSION_STRING < WotLK
    if (hasPlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP))
        removePlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);
#endif

#if VERSION_STRING >= WotLK
    if (hasPlayerFlags(PLAYER_FLAG_DEVELOPER))
        removePlayerFlags(PLAYER_FLAG_DEVELOPER);
#endif

#if VERSION_STRING == TBC
    if (hasPlayerFlags(PLAYER_FLAG_SANCTUARY))
        removePlayerFlags(PLAYER_FLAG_SANCTUARY);
#endif

    ss << getPlayerFlags() << ", " << std::to_string(getEnabledActionBars()) << ", ";

    // if its an arena, save the entry coords instead of the normal position
    if (in_arena)
        ss << getBGEntryPosition().x << ", " << getBGEntryPosition().y << ", " << getBGEntryPosition().z << ", " << getBGEntryPosition().o << ", " << getBGEntryMapId() << ", ";
    else
        ss << m_position.x << ", " << m_position.y << ", " << m_position.z << ", " << m_position.o << ", " << m_mapId << ", ";

    ss << m_zoneId << ", ";

    // taxi mask
    ss << "'";

    ss << m_taxi->saveTaximaskNodeToString();

    ss << "', ";

    ss << m_banned << ", '" << CharacterDatabase.EscapeString(m_banreason) << "', " << uint32_t(UNIXTIME) << ", ";

    //online state
    if (getSession()->_loggingOut || newCharacter)
        ss << "0, ";
    else
        ss << "1, ";

    ss << getBindPosition().x << ", " << getBindPosition().y << ", " << getBindPosition().z << ", " << getBindPosition().o << ", " << getBindMapId() << ", " << getBindZoneId() << ", ";

    ss << uint32_t(m_isResting) << ", " << uint32_t(m_restState) << ", " << uint32_t(m_restAmount) << ", ";

    ss << "'" << uint32_t(m_playedTime[0]) << " " << uint32_t(m_playedTime[1]) << " " << uint32_t(playedt) << "', ";

    ss << uint32_t(m_deathState) << ", " << m_talentResetsCount << ", " << m_firstLogin << ", " << m_loginFlag << ", " << m_arenaPoints << ", " << (uint32_t)m_stableSlotCount << ", ";

    // instances
    if (in_arena)
        ss << getBGEntryInstanceId() << ", ";
    else
        ss << m_instanceId << ", ";

    ss << getBGEntryMapId() << ", " << getBGEntryPosition().x << ", " << getBGEntryPosition().y << ", " << getBGEntryPosition().z << ", " << getBGEntryPosition().o << ", " << getBGEntryInstanceId() << ", ";

    // taxi destination
    ss << "'";
    ss << m_taxi->saveTaxiDestinationsToString();
    ss << "', ";

    // last node
    if (FlightPathMovementGenerator* flight = dynamic_cast<FlightPathMovementGenerator*>(getMovementManager()->getCurrentMovementGenerator()))
        ss << flight->getCurrentNode() << ", ";
    else
        ss << uint32_t(0) << ", ";

    const auto transport = this->GetTransport();
    if (!transport)
        ss << uint32_t(0) << ",'0','0','0','0'" << ", ";
    else
        ss << transport->getEntry() << ",'" << GetTransOffsetX() << "','" << GetTransOffsetY() << "','" << GetTransOffsetZ() << "','" << GetTransOffsetO() << "'" << ", ";

    saveSpells(newCharacter, buf);

    saveDeletedSpells(newCharacter, buf);

    saveReputations(newCharacter, buf);

    // Add player action bars
#ifdef FT_DUAL_SPEC
    for (uint8_t s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        ss << "'";
        for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        {
            ss << uint32_t(m_specs[s].getActionButton(i).Action) << ","
                << uint32_t(m_specs[s].getActionButton(i).Type) << ","
                << uint32_t(m_specs[s].getActionButton(i).Misc) << ",";
        }
        ss << "'" << ", ";
    }
#else
    ss << "'";
    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        ss << uint32_t(m_spec.getActionButton(i).Action) << ","
            << uint32_t(m_spec.getActionButton(i).Type) << ","
            << uint32_t(m_spec.getActionButton(i).Misc) << ",";
    }
    ss << "'" << ", " << "''" << ", ";
#endif

    if (!newCharacter)
    {
        saveAuras(ss);
        ss << ", ";
    }
    else
    {
        ss << "''" << ", ";
    }

    // Add player finished quests
    ss << "'";
    for (auto finishedQuests = m_finishedQuests.begin(); finishedQuests != m_finishedQuests.end(); ++finishedQuests)
        ss << (*finishedQuests) << ",";
    ss << "'" << ", ";

    // add finished dailies
    ss << "'";
    for (auto finishedDailies : getFinishedDailies())
        ss << finishedDailies << ",";
    ss << "'" << ", ";

    ss << m_honorRolloverTime << ", " << m_killsToday << ", " << m_killsYesterday << ", " << m_killsLifetime << ", " << m_honorToday << ", " << m_honorYesterday << ", " << m_honorPoints << ", ";

    ss << uint32_t(getDrunkValue()) << ", ";

    // TODO Remove
#ifdef FT_DUAL_SPEC
    for (uint8_t s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        ss << "'";
        for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
            ss << uint32_t(m_specs[s].getGlyph(i)) << ",";

        ss << "', '";
        for (const auto& [talentId, rank] : m_specs[s].getTalents())
            ss << uint32_t(talentId) << "," << uint32_t(rank) << ",";

        ss << "'" << ", ";
    }
#else
    ss << "'', '";
    for (const auto& [talentId, rank] : m_spec.getTalents())
        ss << talentId << "," << rank << ",";

    ss << "', '', '', ";
#endif

    ss << uint32_t(m_talentSpecsCount) << ", " << uint32_t(m_talentActiveSpec) << ", ";

    ss << "'";
#ifdef FT_DUAL_SPEC
    ss << uint32_t(m_specs[SPEC_PRIMARY].getTalentPoints()) << " " << uint32_t(m_specs[SPEC_SECONDARY].getTalentPoints());
#else
    ss << uint32_t(m_spec.getTalentPoints()) << " 0";
#endif
    ss << "'" << ", ";

#if VERSION_STRING < Cata
    ss << "'" << uint32_t(0) << "', ";
#else
    ss << "'" << uint32_t(m_FirstTalentTreeLock) << "', ";
#endif

    ss << "'" << m_phase << "', ";

    uint32_t xpfield = 0;

    if (m_isXpGainAllowed)
        xpfield = 1;

    ss << "'" << xpfield << "'" << ", ";

    const bool saveData = worldConfig.server.saveExtendedCharData;
    if (saveData)
    {
        ss << "'";
        for (uint32_t offset = getSizeOfStructure(WoWObject); offset < getSizeOfStructure(WoWPlayer); offset++)
            ss << uint32_t(m_uint32Values[offset]) << ";";
        ss << "'" << ", ";
    }
    else
    {
        ss << "'', ";
    }

    if (m_resetTalents)
        ss << uint32_t(1);
    else
        ss << uint32_t(0);

    ss << ", ";

    ss << uint32_t(this->hasWonRbgToday()) << ", " << uint32_t(m_dungeonDifficulty) << ", " << uint32_t(m_raidDifficulty);
    ss << ")";

    if (newCharacter)
        CharacterDatabase.WaitExecuteNA(ss.str().c_str());
    else
        buf->AddQueryNA(ss.str().c_str());

    // Save Other related player stuff

    // Inventory
    getItemInterface()->mSaveItemsToDatabase(newCharacter, buf);

    getItemInterface()->m_EquipmentSets.SavetoDB(buf);

    // save quest progress
    _saveQuestLogEntry(buf);

    // Tutorials
    saveTutorials();

    // GM Ticket
    //\todo Is this really necessary? Tickets will always be saved on creation, update and so on...
    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(getGuid());
    if (ticket != nullptr)
        sTicketMgr.saveGMTicket(ticket, buf);

    // Cooldown Items
    _savePlayerCooldowns(buf);

    // Instance Timed Lockout
    saveInstanceTimeRestrictions();

    // Pets
    if (getClass() == HUNTER || getClass() == WARLOCK)
    {
        _savePet(buf, true);
        _savePetSpells(buf);
    }
    m_nextSave = Util::getMSTime() + worldConfig.getIntRate(INTRATE_SAVE);
#if VERSION_STRING > TBC
    m_achievementMgr->saveToDb(buf);
#endif

    if (buf)
        CharacterDatabase.AddQueryBuffer(std::move(bufPtr));
}

void Player::_saveQuestLogEntry(QueryBuffer* buf)
{
    for (uint32_t removeableQuestId : m_removequests)
    {
        if (buf == nullptr)
            CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", getGuidLow(), removeableQuestId);
        else
            buf->AddQuery("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", getGuidLow(), removeableQuestId);
    }

    m_removequests.clear();

    for (auto& questlogEntry : m_questlog)
    {
        if (questlogEntry != nullptr)
            questlogEntry->saveToDB(buf);
    }
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

bool Player::loadFromDB(uint32_t guid)
{
    auto q = std::make_unique<AsyncQuery>(std::make_unique<SQLClassCallbackP0<Player>>(this, &Player::loadFromDBProc));

    q->AddQuery("SELECT * FROM characters WHERE guid = %u AND login_flags = %u", guid, (uint32_t)LOGIN_NO_FLAG); // 0
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
    CharacterDatabase.QueueAsyncQuery(std::move(q));
    return true;
}

void Player::loadFromDBProc(QueryResultVector& results)
{
    auto startTime = Util::TimeNow();

    if (getSession() == nullptr || results.size() < 8)        // should have 8 queryresults for aplayer load.
    {
        removePendingPlayer();
        return;
    }

    QueryResult* result = results[PlayerQuery::LoginFlags].result.get();
    if (!result)
    {
        sLogger.failure("Player login query failed! guid = {}", getGuidLow());
        removePendingPlayer();
        return;
    }

    const uint32_t fieldcount = 95;
    if (result->GetFieldCount() != fieldcount)
    {
        sLogger.failure("Expected {} fields from the database, but received {}!  You may need to update your character database.", fieldcount, uint32_t(result->GetFieldCount()));
        removePendingPlayer();
        return;
    }

    Field* field = result->Fetch();
    if (field[1].asUint32() != m_session->GetAccountId())
    {
        sCheatLog.writefromsession(m_session, "player tried to load character not belonging to them (guid %u, on account %u)",
            field[0].asUint32(), field[1].asUint32());
        removePendingPlayer();
        return;
    }

    uint32_t banned = field[34].asUint32();
    if (banned && (banned < 100 || banned >(uint32_t)UNIXTIME))
    {
        removePendingPlayer();
        return;
    }

    m_name = field[2].asCString();

    // Load race/class from fields
    setRace(field[3].asUint8());
    setClass(field[4].asUint8());
    setGender(field[5].asUint8());
    uint32_t cfaction = field[6].asUint32();

    // set race dbc
    m_dbcRace = sChrRacesStore.lookupEntry(getRace());
    m_dbcClass = sChrClassesStore.lookupEntry(getClass());
    if (!m_dbcClass || !m_dbcRace)
    {
        // bad character
        sLogger.failure("guid {} failed to login, no race or class dbc found. (race {} class {})", getGuidLow(), (unsigned int)getRace(), (unsigned int)getClass());
        removePendingPlayer();
        return;
    }

    if (m_dbcRace->team_id == 7)
        m_bgTeam = m_team = 0;
    else
        m_bgTeam = m_team = 1;

    initialiseNoseLevel();

    // set power type
    setPowerType(static_cast<uint8_t>(m_dbcClass->power_type));

    // obtain player create m_playerCreateInfo
    m_playerCreateInfo = sMySQLStore.getPlayerCreateInfo(getRace(), getClass());
    if (m_playerCreateInfo == nullptr)
    {
        sLogger.failure("player guid {} has no playerCreateInfo!", getGuidLow());
        removePendingPlayer();
        return;
    }

    // set level
    setLevel(field[7].asUint32());

    // obtain level/stats information
    m_levelInfo = sObjectMgr.getLevelInfo(getRace(), getClass(), getLevel());

    if (!m_levelInfo)
    {
        sLogger.failure("guid {} level {} class {} race {} levelinfo not found!", getGuidLow(), getLevel(), (unsigned int)getClass(), (unsigned int)getRace());
        removePendingPlayer();
        return;
    }

#if VERSION_STRING > TBC
    // load achievements before anything else otherwise skills would complete achievements already in the DB, leading to duplicate achievements and criterias(like achievement=126).
    m_achievementMgr->loadFromDb(results[PlayerQuery::Achievements].result.get(), results[PlayerQuery::AchievementProgress].result.get());
#endif

    setInitialPlayerData();

    // set xp
    setXp(field[8].asUint32());

    // Load active cheats
    uint32_t active_cheats = field[9].asUint32();
    if (active_cheats & PLAYER_CHEAT_COOLDOWN)
        m_cheats.hasCooldownCheat = true;
    if (active_cheats & PLAYER_CHEAT_CAST_TIME)
        m_cheats.hasCastTimeCheat = true;
    if (active_cheats & PLAYER_CHEAT_GOD_MODE)
        m_cheats.hasGodModeCheat = true;
    if (active_cheats & PLAYER_CHEAT_POWER)
        m_cheats.hasPowerCheat = true;
    if (active_cheats & PLAYER_CHEAT_FLY)
        m_cheats.hasFlyCheat = true;
    if (active_cheats & PLAYER_CHEAT_AURA_STACK)
        m_cheats.hasAuraStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_ITEM_STACK)
        m_cheats.hasItemStackCheat = true;
    if (active_cheats & PLAYER_CHEAT_TRIGGERPASS)
        m_cheats.hasTriggerpassCheat = true;
    if (active_cheats & PLAYER_CHEAT_TAXI)
        m_cheats.hasTaxiCheat = true;

    // Process exploration data.
    loadFieldsFromString(field[10].asCString(), getOffsetForStructuredField(WoWPlayer, explored_zones), WOWPLAYER_EXPLORED_ZONES_COUNT); //10

    loadSkills(results[PlayerQuery::Skills].result.get());

    if (m_firstLogin || m_skills.empty())
    {
        // no skills - reset to defaults
        learnInitialSkills();
    }

#if VERSION_STRING >= Cata
    setInitialPlayerProfessions();
#endif

    // set the rest of the stuff
    setWatchedFaction(field[11].asUint32());
#if VERSION_STRING > Classic
    setChosenTitle(field[12].asUint32());
    setKnownTitles(0, field[13].asUint64());
#if VERSION_STRING > TBC
    setKnownTitles(1, field[14].asUint64());
    setKnownTitles(2, field[15].asUint64());
#endif
#endif

    setCoinage(field[16].asUint32());

#if VERSION_STRING < Cata
    setAmmoId(field[17].asUint32());
#endif

    setFreePrimaryProfessionPoints(field[18].asUint32());

    m_loadHealth = field[19].asUint32();
    m_loadMana = field[20].asUint32();
    setHealth(m_loadHealth);

    sLogger.debug("Player level {}, health {}, mana {} loaded from db!", getLevel(), m_loadHealth, m_loadMana);

    setPvpRank(field[21].asUint8());

    setPlayerBytes(field[22].asUint32());
    setPlayerBytes2(field[23].asUint32());

    setPlayerGender(getGender());

    setPlayerFlags(field[24].asUint32());
    setEnabledActionBars(field[25].asUint8());

    m_position.x = field[26].asFloat();
    m_position.y = field[27].asFloat();
    m_position.z = field[28].asFloat();
    m_position.o = field[29].asFloat();

    m_mapId = field[30].asUint32();
    m_zoneId = field[31].asUint32();
    setZoneId(m_zoneId);

    // Initialize 'normal' fields
    setScale(1.0f);
#if VERSION_STRING > TBC
    setHoverHeight(1.0f);
#endif

    setBoundingRadius(0.388999998569489f);
    setCombatReach(1.5f);

    setInitialDisplayIds(getGender(), getRace());

    eventModelChange();

    if (const auto raceEntry = sChrRacesStore.lookupEntry(getRace()))
        setFaction(raceEntry->faction_id);
    else
        setFaction(0);

    if (cfaction)
    {
        setFaction(cfaction);
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

    // Load Taxis From Database
    m_taxi->loadTaxiMask(field[32].asCString());
    initTaxiNodesForLevel();

    m_banned = field[33].asUint32();      //Character ban
    m_banreason = field[34].asCString();
    m_timeLogoff = field[35].asUint32();
    //field[36].GetUInt32();    online

    setBindPoint(field[37].asFloat(), field[38].asFloat(), field[39].asFloat(), field[40].asFloat(), field[41].asUint32(), field[42].asUint32());

    m_isResting = field[43].asUint8();
    m_restState = field[44].asUint8();
    m_restAmount = field[45].asUint32();


    std::string tmpStr = field[46].asCString();
    m_playedTime[0] = (uint32_t)atoi(strtok((char*)tmpStr.c_str(), " "));
    m_playedTime[1] = (uint32_t)atoi(strtok(nullptr, " "));

    m_deathState = (DeathState)field[47].asUint32();
    m_talentResetsCount = field[48].asUint32();
    m_firstLogin = field[49].asBool();
    m_loginFlag = field[50].asUint32();
    m_arenaPoints = field[51].asUint32();
    if (m_arenaPoints > worldConfig.limit.maxArenaPoints)
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxArenaPoints << " arena points " << m_arenaPoints;
        sCheatLog.writefromsession(m_session, dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)          // report to online GMs
            sendReportToGmMessage(getName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
        {
            m_session->Disconnect();
        }
        m_arenaPoints = worldConfig.limit.maxArenaPoints;
    }

    initialiseCharters();

    initialiseArenaTeam();

    m_stableSlotCount = static_cast<uint8_t>(field[52].asUint32());
    m_instanceId = field[53].asUint32();

    setBGEntryPoint(field[55].asFloat(), field[56].asFloat(), field[57].asFloat(), field[58].asFloat(), field[54].asUint32(), field[59].asUint32());

    std::string taxi_nodes = field[60].asCString();
    uint32_t taxi_currentNode = field[61].asInt32();

    uint32_t transportGuid = field[62].asUint32();
    float transportX = field[63].asFloat();
    float transportY = field[64].asFloat();
    float transportZ = field[65].asFloat();
    float transportO = field[66].asFloat();

    if (transportGuid != 0)
        obj_movement_info.setTransportData(transportGuid, transportX, transportY, transportZ, transportO, 0, 0);
    else
        obj_movement_info.clearTransportData();

    loadDeletedSpells(results[PlayerQuery::DeletedSpells].result.get());

    loadSpells(results[PlayerQuery::Spells].result.get());

    loadReputations(results[PlayerQuery::Reputation].result.get());

    // Load saved actionbars
    uint32_t Counter = 0;
    char* start = nullptr;
    char* end = nullptr;
#if VERSION_STRING > TBC
    for (uint8_t s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        start = (char*)field[67 + s].asCString();
        Counter = 0;
        while (Counter < PLAYER_ACTION_BUTTON_COUNT)
        {
            if (start == nullptr)
                break;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[0 + s].getActionButton(Counter).Action = std::stoul(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[0 + s].getActionButton(Counter).Type = static_cast<uint8_t>(std::stoul(start));
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            m_specs[0 + s].getActionButton(Counter).Misc = static_cast<uint8_t>(std::stoul(start));
            start = end + 1;

            Counter++;
        }
    }
#else
    {
        auto& spec = m_spec;

        start = (char*)field[67].asCString();
        Counter = 0;
        while (Counter < PLAYER_ACTION_BUTTON_COUNT)
        {
            if (start == nullptr)
                break;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            spec.getActionButton(Counter).Action = (uint32_t)std::stoul(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            spec.getActionButton(Counter).Type = (uint8_t)std::stoul(start);
            start = end + 1;
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            spec.getActionButton(Counter).Misc = (uint8_t)std::stoul(start);
            start = end + 1;

            Counter++;
        }
    }
#endif

    if (m_firstLogin)
    {
        for (const auto itr : m_playerCreateInfo->actionbars)
            setActionButton(itr.button, itr.action, itr.type, itr.misc);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Parse saved buffs
    std::istringstream savedPlayerBuffsStream(field[69].asCString());
    std::string auraId, auraDuration, auraPositivValue, auraCharges;

    while (std::getline(savedPlayerBuffsStream, auraId, ','))
    {
        LoginAura la;
        la.id = std::stoul(auraId.c_str());

        std::getline(savedPlayerBuffsStream, auraDuration, ',');
        la.dur = std::stoul(auraDuration.c_str());

        std::getline(savedPlayerBuffsStream, auraPositivValue, ',');
        la.positive = auraPositivValue.empty() ? false : true;

        std::getline(savedPlayerBuffsStream, auraCharges, ',');
        la.charges = std::stoul(auraCharges.c_str());

        m_loginAuras.push_back(la);
    }

    // Load saved finished quests

    start = (char*)field[70].asCString();
    while (true)
    {
        end = strchr(start, ',');
        if (!end)break;
        *end = 0;
        const uint32_t questEntry = std::stoul(start);
        m_finishedQuests.insert(questEntry);

        // Load talent points from finished quests
        auto questProperties = sMySQLStore.getQuestProperties(questEntry);
        if (questProperties != nullptr && questProperties->rewardtalents > 0)
            m_talentPointsFromQuests += questProperties->rewardtalents;

        start = end + 1;
    }

    start = (char*)field[71].asCString();
    while (true)
    {
        end = strchr(start, ',');
        if (!end) break;
        *end = 0;
        m_finishedDailies.insert(std::stoul(start));
        start = end + 1;
    }

    m_honorRolloverTime = field[72].asUint32();
    m_killsToday = field[73].asUint32();
    m_killsYesterday = field[74].asUint32();
    m_killsLifetime = field[75].asUint32();

    m_honorToday = field[76].asUint32();
    m_honorYesterday = field[77].asUint32();
    m_honorPoints = field[78].asUint32();
    if (m_honorPoints > worldConfig.limit.maxHonorPoints)
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxHonorPoints << " honor points " << m_honorPoints;

        sCheatLog.writefromsession(m_session, dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)
            sendReportToGmMessage(getName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
            m_session->Disconnect();

        m_honorPoints = worldConfig.limit.maxHonorPoints;
    }

    rolloverHonor();

    // Load drunk value and calculate sobering. after 15 minutes logged out, the player will be sober again
    uint32_t timediff = (uint32_t)UNIXTIME - m_timeLogoff;
    uint32_t soberFactor;
    if (timediff > 900)
        soberFactor = 0;
    else
        soberFactor = 1 - timediff / 900;

    setServersideDrunkValue(uint16_t(soberFactor * field[79].asUint32()));

#if VERSION_STRING > TBC
    for (uint8_t s = 0; s < MAX_SPEC_COUNT; ++s)
    {
        start = (char*)field[80 + 2 * s].asCString();
        uint8_t glyphid = 0;
        while (glyphid < GLYPHS_COUNT)
        {
            end = strchr(start, ',');
            if (!end)break;
            *end = 0;
            m_specs[s].setGlyph(static_cast<uint16_t>(std::stoul(start)), glyphid);
            ++glyphid;
            start = end + 1;
        }

        //Load talents for spec
        start = (char*)field[81 + 2 * s].asCString();
        while (end != nullptr)
        {
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint32_t talentid = std::stoul(start);
            start = end + 1;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint8_t rank = static_cast<uint8_t>(std::stoul(start));
            start = end + 1;

            m_specs[s].addTalent(talentid, rank);
        }
    }
#else
    {
        auto& spec = m_spec;

        //Load talents for spec	
        start = (char*)field[81].asCString();  // talents1
        while (end != nullptr)
        {
            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint32_t talentid = std::stoul(start);
            start = end + 1;

            end = strchr(start, ',');
            if (!end)
                break;
            *end = 0;
            uint8_t rank = static_cast<uint8_t>(std::stoul(start));
            start = end + 1;

            spec.addTalent(talentid, rank);
        }
    }
#endif

    m_talentSpecsCount = field[84].asUint8();
    m_talentActiveSpec = field[85].asUint8();

#if VERSION_STRING > TBC
    {
        if (auto talentPoints = field[86].asCString())
        {
            uint32_t tps[2] = { 0,0 };

            auto talentPointsVector = AscEmu::Util::Strings::split(talentPoints, " ");
            for (uint8_t i = 0; i < 2; ++i)
                tps[i] = std::stoi(talentPointsVector[i]);

            m_specs[SPEC_PRIMARY].setTalentPoints(tps[0]);
            m_specs[SPEC_SECONDARY].setTalentPoints(tps[1]);
        }
#if VERSION_STRING < Cata
        setFreeTalentPoints(getActiveSpec().getTalentPoints());
#endif
    }
#else
    {
        if (auto talentPoints = field[86].asCString())
        {
            uint32_t tps[2] = { 0,0 };

            auto talentPointsVector = AscEmu::Util::Strings::split(talentPoints, " ");
            for (uint8_t i = 0; i < 2; ++i)
                tps[i] = std::stoi(talentPointsVector[i]);

            m_spec.setTalentPoints(tps[0]);
        }

        setFreeTalentPoints(getActiveSpec().getTalentPoints());
    }
#endif

#if VERSION_STRING >= Cata
    m_FirstTalentTreeLock = field[87].asUint32(); // Load First Set Talent Tree
#endif

    m_phase = field[88].asUint32(); //Load the player's last phase

    uint32_t xpfield = field[89].asUint32();

    if (xpfield == 0)
        m_isXpGainAllowed = false;
    else
        m_isXpGainAllowed = true;

    //field[90].GetString();    //skipping data

    if (field[91].asUint32() == 1)
        m_resetTalents = true;
    else
        m_resetTalents = false;

    // Load player's RGB daily data
    if (field[92].asUint32() == 1)
        m_hasWonRbgToday = true;
    else
        m_hasWonRbgToday = false;

    m_dungeonDifficulty = field[93].asUint8();
    m_raidDifficulty = field[94].asUint8();

    HonorHandler::RecalculateHonorFields(this);

#if VERSION_STRING > TBC
    updateGlyphs();

    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
        setGlyph(i, m_specs[m_talentActiveSpec].getGlyph(i));
#endif

    //class fixes
    switch (getClass())
    {
        case WARLOCK:
        case HUNTER:
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
        case MAGE:
#endif
            _loadPet(results[PlayerQuery::Pets].result.get());
            _loadPetSpells(results[PlayerQuery::SummonSpells].result.get());
            break;
    }

    if (getGuildId())
        setGuildTimestamp(static_cast<uint32_t>(UNIXTIME));

    // load properties
    loadTutorials();
    _loadPlayerCooldowns(results[PlayerQuery::Cooldowns].result.get());
    _loadQuestLogEntry(results[PlayerQuery::Questlog].result.get());
    getItemInterface()->mLoadItemsFromDatabase(results[PlayerQuery::Items].result.get());
    getItemInterface()->m_EquipmentSets.LoadfromDB(results[PlayerQuery::EquipmentSets].result.get());

#if VERSION_STRING > WotLK
    loadVoidStorage();
#endif

    m_mailBox->Load(results[PlayerQuery::Mailbox].result.get());

    // Saved Instances
    loadBoundInstances();
    loadInstanceTimeRestrictions();

    // Create Instance when needed
    if (sMapMgr.findBaseMap(GetMapId()) && sMapMgr.findBaseMap(GetMapId())->isInstanceableMap())
    {
        // No Instance Found Lets Create it
        if (!sMapMgr.findWorldMap(GetMapId(), GetInstanceID()))
            sMapMgr.createInstanceForPlayer(GetMapId(), this, GetInstanceID());
    }

    // SOCIAL
    loadFriendList();
    loadFriendedByOthersList();
    loadIgnoreList();
    // END SOCIAL

    // Check skills that player shouldn't have
    if (hasSkillLine(SKILL_DUAL_WIELD) && !hasSpell(674))
        removeSkillLine(SKILL_DUAL_WIELD);

#if VERSION_STRING > TBC
    // update achievements before adding player to World, otherwise we'll get a nice race condition.
    //move CheckAllAchievementCriteria() after FullLogin(this) and i'll cut your b***s.
    m_achievementMgr->updateAllAchievementCriteria();
#endif

    m_session->fullLogin(this);
    m_session->m_loggingInPlayer = nullptr;

    if (!isAlive())
    {
        if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
            setCorpseData(corpse->GetPosition(), corpse->GetInstanceID());
    }

#if VERSION_STRING > Classic
    uint32_t uniques[64];
    int nuniques = 0;

    for (uint8_t x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        ItemInterface* itemi = getItemInterface();
        Item* it = itemi->GetInventoryItem(x);

        if (it != nullptr)
        {
            for (uint8_t count = 0; count < it->getSocketSlotCount(); count++)
            {
                const auto enchantmentSlot = static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count);
                EnchantmentInstance* ei = it->getEnchantment(enchantmentSlot);

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
                                it->removeEnchantment(enchantmentSlot);
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
#endif

    // Continue Our Taxi Path
    if (!taxi_nodes.empty())
    {
        // Not finish taxi flight path
        if (!m_taxi->loadTaxiDestinationsFromString(taxi_nodes, GetTeam()))
        {
            // problems with taxi path loading
            WDB::Structures::TaxiNodesEntry const* nodeEntry = nullptr;
            if (uint32_t node_id = m_taxi->getTaxiSource())
                nodeEntry = sTaxiNodesStore.lookupEntry(node_id);

            if (!nodeEntry) // don't know taxi start node, teleport to homebind
            {
                safeTeleport(getBindMapId(), 0, getBindPosition());
            }
            else // has start node, teleport to it
            {
                safeTeleport(nodeEntry->mapid, 0, LocationVector(nodeEntry->x, nodeEntry->y, nodeEntry->z, 0.0f));
            }
            m_taxi->clearTaxiDestinations();
        }
        
        m_taxi->setNodeAfterTeleport(taxi_currentNode);
        // flight will started later
    }

    auto timeToNow = Util::GetTimeDifferenceToNow(startTime);
    sLogger.info("Time for playerloading: {} ms", static_cast<uint32_t>(timeToNow));
}

void Player::_loadQuestLogEntry(QueryResult* result)
{
    for (uint8_t slot = 0; slot < MAX_QUEST_SLOT; ++slot)
    {
        setQuestLogEntryBySlot(slot, 0);
        setQuestLogStateBySlot(slot, 0);
        setQuestLogRequiredMobOrGoBySlot(slot, 0);
        setQuestLogExpireTimeBySlot(slot, 0);
    }

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32_t questid = fields[1].asUint32();
            uint8_t slot = fields[2].asUint8();

            QuestProperties const* questProperties = sMySQLStore.getQuestProperties(questid);
            if (!questProperties)
            {
                m_removequests.insert(questid);
                continue;
            }

            if (m_questlog[slot] != nullptr)
                continue;

            auto* questLogEntry = createQuestLogInSlot(questProperties, slot);
            questLogEntry->loadFromDB(fields);
            questLogEntry->updatePlayerFields();

        } while (result->NextRow());
    }
}

#if VERSION_STRING >= TBC
float Player::getDefenseChance(uint32_t opLevel)
{
    float chance = getSkillLineCurrent(SKILL_DEFENSE, true) - (opLevel * 5.0f);
    chance += calcRating(CR_DEFENSE_SKILL);
    chance = floorf(chance) * 0.04f;   // defense skill is treated as an integer on retail

    return chance;
}

float Player::getDodgeChance()
{
    const float crit_to_dodge[MAX_PLAYER_CLASSES] =
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

    uint32_t playerClass = getClass();
    float chance = 0.0f;
    uint32_t level = getLevel();

    if (level > worldConfig.player.playerGeneratedInformationByLevelCap)
        level = worldConfig.player.playerGeneratedInformationByLevelCap;

    // Base dodge + dodge from agility
    auto baseCrit = sGtChanceToMeleeCritBaseStore.lookupEntry(playerClass - 1);
    auto critPerAgi = sGtChanceToMeleeCritStore.lookupEntry(level - 1 + (playerClass - 1) * 100);
    uint32_t agi = getStat(STAT_AGILITY);

    float tmp = 100.0f * (baseCrit->val + agi * critPerAgi->val);
    tmp *= crit_to_dodge[playerClass];
    chance += tmp;

    chance += calcRating(CR_DODGE);
    chance += getDodgeFromSpell();

    return std::max(chance, 0.0f); // Make sure we don't have a negative chance
}

float Player::getBlockChance()
{
    float chance = BASE_BLOCK_CHANCE;
    chance += calcRating(CR_BLOCK);
    chance += getBlockFromSpell();

    return std::max(chance, 0.0f);   // Make sure we don't have a negative chance
}

float Player::getParryChance()
{
    float chance = BASE_PARRY_CHANCE;
    chance += calcRating(CR_PARRY);
    chance += getParryFromSpell();

    return std::max(chance, 0.0f);   // Make sure we don't have a negative chance
}

void Player::updateChances()
{
    uint32_t playerClass = getClass();
    uint32_t playerLevel = (getLevel() > DBC_PLAYER_LEVEL_CAP) ? DBC_PLAYER_LEVEL_CAP : getLevel();

    float tmp = 0;
    float defence_contribution = 0;

    // Avoidance from defense skill
    defence_contribution = getDefenseChance(playerLevel);

    // Dodge
    tmp = getDodgeChance();
    tmp += defence_contribution;
    tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    setDodgePercentage(tmp);

    // Block
    Item* it = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (it != nullptr && it->getItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        tmp = getBlockChance();
        tmp += defence_contribution;
        tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    }
    else
    {
        tmp = 0.0f;
    }

    setBlockPercentage(tmp);

    // Parry (can only parry with something in main hand)
    it = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (it != nullptr)
    {
        tmp = getParryChance();
        tmp += defence_contribution;
        tmp = std::min(std::max(tmp, 0.0f), 95.0f);
    }
    else
    {
        tmp = 0.0f;
    }

    setParryPercentage(tmp);

    // Critical
    auto baseCrit = sGtChanceToMeleeCritBaseStore.lookupEntry(playerClass - 1);

    auto CritPerAgi = sGtChanceToMeleeCritStore.lookupEntry(playerLevel - 1 + (playerClass - 1) * 100);
    if (CritPerAgi == nullptr)
        CritPerAgi = sGtChanceToMeleeCritStore.lookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (playerClass - 1) * 100);

    tmp = 100 * (baseCrit->val + getStat(STAT_AGILITY) * CritPerAgi->val);

    float melee_bonus = 0;
    float ranged_bonus = 0;

    if (m_toCritChance.size() > 0)
    {
        Item* tItemMelee = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        Item* tItemRanged = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);

        //-1 = any weapon
        for (std::map< uint32_t, WeaponModifier >::iterator itr = m_toCritChance.begin(); itr != m_toCritChance.end(); ++itr)
        {
            if (itr->second.wclass == (uint32_t)-1 || (tItemMelee != nullptr && (1 << tItemMelee->getItemProperties()->SubClass & itr->second.subclass)))
                melee_bonus += itr->second.value;

            if (itr->second.wclass == (uint32_t)-1 || (tItemRanged != nullptr && (1 << tItemRanged->getItemProperties()->SubClass & itr->second.subclass)))
                ranged_bonus += itr->second.value;
        }
    }

    float cr = tmp + calcRating(CR_CRIT_MELEE) + melee_bonus;
    setMeleeCritPercentage(std::min(cr, 95.0f));

    float rcr = tmp + calcRating(CR_CRIT_RANGED) + ranged_bonus;
    setRangedCritPercentage(std::min(rcr, 95.0f));

    auto SpellCritBase = sGtChanceToSpellCritBaseStore.lookupEntry(playerClass - 1);

    auto SpellCritPerInt = sGtChanceToSpellCritStore.lookupEntry(playerLevel - 1 + (playerClass - 1) * 100);
    if (SpellCritPerInt == nullptr)
        SpellCritPerInt = sGtChanceToSpellCritStore.lookupEntry(DBC_PLAYER_LEVEL_CAP - 1 + (playerClass - 1) * 100);

    m_spellCritPercentage = 100 * (SpellCritBase->val + getStat(STAT_INTELLECT) * SpellCritPerInt->val) +
        this->getSpellCritFromSpell() +
        this->calcRating(CR_CRIT_SPELL);

    updateChanceFields();
}
#endif

void Player::updateChanceFields()
{
#if VERSION_STRING != Classic
    for (uint8_t school = 0; school < 7; ++school)
        setSpellCritPercentage(school, m_spellCritChanceSchool[school] + m_spellCritPercentage);
#endif
}

void Player::updateAttackSpeed()
{
    uint32_t speed = 2000;

    if (getShapeShiftForm() == FORM_CAT)
        speed = 1000;
    else if (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR)
        speed = 2500;
    else if (!m_isDisarmed)
        if (const auto* itemWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
            speed = itemWeapon->getItemProperties()->Delay;

    setBaseAttackTime(MELEE, static_cast<uint32_t>(static_cast<float>(speed) / (getAttackSpeedModifier(MELEE) * (1.0f + calcRating(CR_HASTE_MELEE) / 100.0f))));

    const auto* offhandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offhandWeapon && offhandWeapon->getItemProperties()->Class == ITEM_CLASS_WEAPON)
    {
        speed = offhandWeapon->getItemProperties()->Delay;
        setBaseAttackTime(OFFHAND, static_cast<uint32_t>(static_cast<float>(speed) / (getAttackSpeedModifier(OFFHAND) * (1.0f + calcRating(CR_HASTE_MELEE) / 100.0f))));
    }

    if (const auto* rangedWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
    {
        speed = rangedWeapon->getItemProperties()->Delay;
        setBaseAttackTime(RANGED, static_cast<uint32_t>(static_cast<float>(speed) / (getAttackSpeedModifier(RANGED) * (1.0f + calcRating(CR_HASTE_RANGED) / 100.0f))));
    }
}

void Player::updateStats()
{
    updateAttackSpeed();

    // Formulas from wowwiki
    int32_t attackPower = 0;
    int32_t rangedAttackPower = 0;
    int32_t hpdelta = 128;
    int32_t manadelta = 128;

    uint32_t str = getStat(STAT_STRENGTH);
    uint32_t agi = getStat(STAT_AGILITY);
    uint32_t lev = getLevel();

    // Attack power
    uint32_t playerClass = getClass();
    switch (playerClass)
    {
        case DRUID:
            //(Strength x 2) - 20
            attackPower = str * 2 - 20;
            //Agility - 10
            rangedAttackPower = agi - 10;

            if (getShapeShiftForm() == FORM_MOONKIN)
            {
                //(Strength x 2) + (Character Level x 1.5) - 20
                attackPower += Util::float2int32(static_cast<float>(lev) * 1.5f);
            }
            if (getShapeShiftForm() == FORM_CAT)
            {
                //(Strength x 2) + Agility + (Character Level x 2) - 20
                attackPower += agi + (lev * 2);
            }
            if (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR)
            {
                //(Strength x 2) + (Character Level x 3) - 20
                attackPower += (lev * 3);
            }
            break;

        case ROGUE:
            //Strength + Agility + (Character Level x 2) - 20
            attackPower = str + agi + (lev * 2) - 20;
            //Character Level + Agility - 10
            rangedAttackPower = lev + agi - 10;

            break;

        case HUNTER:
            //Strength + Agility + (Character Level x 2) - 20
            attackPower = str + agi + (lev * 2) - 20;
            //(Character Level x 2) + Agility - 10
            rangedAttackPower = (lev * 2) + agi - 10;

            break;

        case SHAMAN:
            //(Strength) + (Agility) + (Character Level x 2) - 20
            attackPower = str + agi + (lev * 2) - 20;
            //Agility - 10
            rangedAttackPower = agi - 10;

            break;

        case PALADIN:
            //(Strength x 2) + (Character Level x 3) - 20
            attackPower = (str * 2) + (lev * 3) - 20;
            //Agility - 10
            rangedAttackPower = agi - 10;

            break;

        case WARRIOR:
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
            //(Strength x 2) + (Character Level x 3) - 20
            attackPower = (str * 2) + (lev * 3) - 20;
            //Character Level + Agility - 10
            rangedAttackPower = lev + agi - 10;

            break;
#endif
        default:    //mage,priest,warlock
            attackPower = agi - 10;
    }

    /* modifiers */
    rangedAttackPower += m_rapModPct * getStat(STAT_INTELLECT) / 100;

    if (rangedAttackPower < 0)
        rangedAttackPower = 0;

    if (attackPower < 0)
        attackPower = 0;

    setAttackPower(attackPower);
    setRangedAttackPower(rangedAttackPower);

    const auto* levelInfo = sObjectMgr.getLevelInfo(this->getRace(), this->getClass(), lev);
    if (levelInfo != nullptr)
    {
        hpdelta = levelInfo->Stat[2] * 10;
        manadelta = levelInfo->Stat[3] * 15;
    }

    levelInfo = sObjectMgr.getLevelInfo(this->getRace(), this->getClass(), 1);
    if (levelInfo != nullptr)
    {
        hpdelta -= levelInfo->Stat[2] * 10;
        manadelta -= levelInfo->Stat[3] * 15;
    }

    uint32_t hp = getBaseHealth();

#if VERSION_STRING != Classic
    int32_t stat_bonus = getPosStat(STAT_STAMINA) - getNegStat(STAT_STAMINA);
#else
    int32_t stat_bonus = 0;
#endif
    if (stat_bonus < 0)
        stat_bonus = 0; // Avoid of having negative health
    int32_t bonus = stat_bonus * 10 + m_healthFromSpell + m_healthFromItems;

    uint32_t res = hp + bonus + hpdelta;
    uint32_t oldmaxhp = getMaxHealth();

    if (res < hp)
        res = hp;

    if (worldConfig.limit.isLimitSystemEnabled && (worldConfig.limit.maxHealthCap > 0) && (res > worldConfig.limit.maxHealthCap) && !getSession()->hasPermissions())   //hacker?
    {
        std::stringstream dmgLog;
        dmgLog << "has over " << worldConfig.limit.maxArenaPoints << " health " << res;

        sCheatLog.writefromsession(getSession(), dmgLog.str().c_str());

        if (worldConfig.limit.broadcastMessageToGmOnExceeding)
            sendReportToGmMessage(getName(), dmgLog.str());

        if (worldConfig.limit.disconnectPlayerForExceedingLimits)
            getSession()->Disconnect();
        else // no disconnect, set it to the cap instead
            res = worldConfig.limit.maxHealthCap;
    }
    setMaxHealth(res);

    if (getHealth() > res)
    {
        setHealth(res);
    }
    else if (playerClass == DRUID && (getShapeShiftForm() == FORM_BEAR || getShapeShiftForm() == FORM_DIREBEAR))
    {
        res = getMaxHealth() * getHealth() / oldmaxhp;
        setHealth(res);
    }

    if (playerClass != WARRIOR && playerClass != ROGUE
#if VERSION_STRING > TBC
        && playerClass != DEATHKNIGHT
#endif
        )
    {
        // MP
        uint32_t mana = getBaseMana();
#if VERSION_STRING != Classic
        stat_bonus = getPosStat(STAT_INTELLECT) - getNegStat(STAT_INTELLECT);
#endif
        if (stat_bonus < 0)
            stat_bonus = 0; // Avoid of having negative mana
        bonus = stat_bonus * 15 + m_manaFromSpell + m_manaFromItems;

        res = mana + bonus + manadelta;
        if (res < mana)
            res = mana;

        if (worldConfig.limit.isLimitSystemEnabled && (worldConfig.limit.maxManaCap > 0) && (res > worldConfig.limit.maxManaCap) && !getSession()->hasPermissions())   //hacker?
        {
            char logmsg[256];
            snprintf(logmsg, 256, "has over %u mana (%i)", worldConfig.limit.maxManaCap, res);
            sCheatLog.writefromsession(getSession(), logmsg);

            if (worldConfig.limit.broadcastMessageToGmOnExceeding) // send m_playerCreateInfo to online GM
                sendReportToGmMessage(getName(), logmsg);

            if (worldConfig.limit.disconnectPlayerForExceedingLimits)
                getSession()->Disconnect();
            else // no disconnect, set it to the cap instead
                res = worldConfig.limit.maxManaCap;
        }
        setMaxPower(POWER_TYPE_MANA, res);

        if (getPower(POWER_TYPE_MANA) > res)
            setPower(POWER_TYPE_MANA, res);

        updateManaRegeneration();
    }

    // Spell haste rating
    float haste = 1.0f + calcRating(CR_HASTE_SPELL) / 100.0f;
    if (haste != m_spellHasteRatingBonus)
    {
        float value = getModCastSpeed() * m_spellHasteRatingBonus / haste; // remove previous mod and apply current

        setModCastSpeed(value);
        m_spellHasteRatingBonus = haste;    // keep value for next run
    }

    // Shield Block
    Item* itemShield = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (itemShield != nullptr && itemShield->getItemProperties()->InventoryType == INVTYPE_SHIELD)
    {
        float block_multiplier = (100.0f + m_modBlockAbsorbValue) / 100.0f;
        if (block_multiplier < 1.0f)
            block_multiplier = 1.0f;

        int32_t blockable_damage = Util::float2int32((itemShield->getItemProperties()->Block + m_modBlockValueFromSpells + getCombatRating(CR_BLOCK) + (str / 2.0f) - 1.0f) * block_multiplier);
#if VERSION_STRING != Classic
        setShieldBlock(blockable_damage);
#endif
    }
    else
    {
#if VERSION_STRING != Classic
        setShieldBlock(0);
#endif
    }

    // Dynamic aura application, auras 212, 268
#if VERSION_STRING >= WotLK
    for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT))
        aurEff->getAura()->updateModifiers();
#endif
#if VERSION_STRING >= TBC
    for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT))
        aurEff->getAura()->updateModifiers();

    updateChances();
#endif

    calculateDamage();
}

void Player::addToInRangeObjects(Object* object)
{
    Unit::addToInRangeObjects(object);
}

void Player::onRemoveInRangeObject(Object* object)
{
    if (object == nullptr)
        return;

    if (isVisibleObject(object->getGuid()))
    {
        getUpdateMgr().pushOutOfRangeGuid(object->GetNewGUID());
    }

    m_visibleObjects.erase(object->getGuid());
    Unit::onRemoveInRangeObject(object);

    if (object->getGuid() == getCharmGuid())
    {
        Unit* unit = getWorldMap()->getUnit(getCharmGuid());
        if (!unit)
            return;

        unPossess();

        if (isCastingSpell())
            interruptSpell();

        setCharmGuid(0);
    }
}

void Player::clearInRangeSets()
{
    m_visibleObjects.clear();
    Unit::clearInRangeSets();
}

void Player::eventCannibalize(uint32_t amount)
{
    if (getChannelSpellId() != 20577)
    {
        sEventMgr.RemoveEvents(this, EVENT_CANNIBALIZE);
        m_cannibalize = false;
        m_cannibalizeCount = 0;
        return;
    }

    uint32_t amt = (getMaxHealth() * amount) / 100;

    uint32_t newHealth = getHealth() + amt;
    if (newHealth <= getMaxHealth())
        setHealth(newHealth);
    else
        setHealth(getMaxHealth());

    m_cannibalizeCount++;
    if (m_cannibalizeCount == 5)
        setEmoteState(EMOTE_ONESHOT_NONE);

    sendPeriodicAuraLog(GetNewGUID(), GetNewGUID(), sSpellMgr.getSpellInfo(20577), amt, 0, 0, 0, SPELL_AURA_PERIODIC_HEAL_PCT, false);
}

void Player::calcResistance(uint8_t type)
{
    if (type < 7)
    {
        int32_t pos = (m_baseResistance[type] * m_baseResistanceModPctPos[type]) / 100;
        int32_t neg = (m_baseResistance[type] * m_baseResistanceModPctNeg[type]) / 100;

        pos += m_flatResistanceModifierPos[type];
        neg += m_flatResistanceModifierNeg[type];
        int32_t res = m_baseResistance[type] + pos - neg;

        if (type == 0)
            res += getStat(STAT_AGILITY) * 2; //fix armor from agi

        if (res < 0)
            res = 0;

        pos += (res * m_resistanceModPctPos[type]) / 100;
        neg += (res * m_resistanceModPctNeg[type]) / 100;
        res = pos - neg + m_baseResistance[type];

        if (type == 0)
            res += getStat(STAT_AGILITY) * 2; //fix armor from agi

#if VERSION_STRING >= WotLK
        // Dynamic aura 285 application, removing bonus
        for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR))
        {
            auto modifiableAurEff = aurEff->getAura()->getModifiableAuraEffect(aurEff->getEffectIndex());
            aurEff->getAura()->SpellAuraModAttackPowerOfArmor(modifiableAurEff, false);
        }
#endif

        if (res < 0)
            res = 1;

#if VERSION_STRING > Classic
        setResistanceBuffModPositive(type, pos);
        setResistanceBuffModNegative(type, -neg);
#endif
        setResistance(type, res > 0 ? res : 0);

        if (auto* const pet = getPet())
            pet->CalcResistance(type);  //Re-calculate pet's too.

#if VERSION_STRING >= WotLK
        // Dynamic aura 285 application, adding bonus
        for (const auto& aurEff : getAuraEffectList(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR))
        {
            auto modifiableAurEff = aurEff->getAura()->getModifiableAuraEffect(aurEff->getEffectIndex());
            aurEff->getAura()->SpellAuraModAttackPowerOfArmor(modifiableAurEff, true);
        }
#endif
    }
}

void Player::calcStat(uint8_t type)
{
    if (type < 5)
    {
        int32_t pos = (int32_t)m_baseStats[type] * (int32_t)m_statModPctPos[type] / 100 + (int32_t)m_flatStatModPos[type];
        int32_t neg = (int32_t)m_baseStats[type] * (int32_t)m_statModPctNeg[type] / 100 + (int32_t)m_flatStatModNeg[type];
        int32_t res = pos + (int32_t)m_baseStats[type] - neg;
        if (res <= 0)
            res = 1;

        pos += (res * (int32_t)this->m_totalStatModPctPos[type]) / 100;
        neg += (res * (int32_t)this->m_totalStatModPctNeg[type]) / 100;
        res = pos + m_baseStats[type] - neg;
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
            calcResistance(0);

        if (type == STAT_STAMINA || type == STAT_INTELLECT)
        {
            if (auto* const pet = getPet())
                pet->CalcStat(type);  //Re-calculate pet's too
        }
    }
}

void Player::regenerateHealth(bool inCombat)
{
    const auto currentHealth = getHealth();
    if (currentHealth == 0)
        return;

    const auto maxHealth = getMaxHealth();
    if (currentHealth >= maxHealth)
        return;

    float_t amt = 0.0f;

    // While polymorphed health is regenerated rapidly
    // Exact value is yet unknown but it's roughly 10% of health per sec
    if (hasUnitStateFlag(UNIT_STATE_POLYMORPHED))
        amt = getMaxHealth() * 0.10f;
    else
        amt = calculateHealthRegenerationValue(inCombat);

    modHealth(static_cast<int32_t>(std::ceil(amt)));
}

void Player::_Relocate(uint32_t mapid, const LocationVector& v, bool sendpending, bool force_new_world, uint32_t instance_id)
{
    // this func must only be called when switching between maps!
    if (sendpending && mapid != m_mapId && force_new_world)
        m_session->SendPacket(SmsgTransferPending(mapid).serialise().get());

    bool sendpacket = (mapid == m_mapId);
    // Dismount before teleport and before being removed from world,
    // otherwise we may spawn the active pet while not being in world.
    dismount(false);

    MySQLStructure::AreaTrigger const* areaTrigger = nullptr;
    bool check = false;

    if (!sendpacket || force_new_world)
    {
        WorldMap* map = sMapMgr.createMap(mapid, this, instance_id);
        if (!map)
        {
            m_session->SendPacket(SmsgTransferAborted(mapid, INSTANCE_ABORT_NOT_FOUND).serialise().get());
            return;
        }
        else if (map->getBaseMap()->isInstanceMap())
        {
            if (auto state = map->cannotEnter(this))
            {
                switch (state)
                {
                    case CANNOT_ENTER_DIFFICULTY_UNAVAILABLE:
                        m_session->SendPacket(SmsgTransferAborted(mapid, INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE).serialise().get());
                        break;
                    case CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
                        m_session->systemMessage("Another group is already inside this instance of the dungeon.");
                        break;
                    case CANNOT_ENTER_TOO_MANY_INSTANCES:
                        m_session->SendPacket(SmsgTransferAborted(mapid, INSTANCE_ABORT_TOO_MANY).serialise().get());
                        break;
                    case CANNOT_ENTER_MAX_PLAYERS:
                        m_session->SendPacket(SmsgTransferAborted(mapid, INSTANCE_ABORT_FULL).serialise().get());
                        break;
                    case CANNOT_ENTER_ENCOUNTER:
                        m_session->SendPacket(SmsgTransferAborted(mapid, INSTANCE_ABORT_ENCOUNTER).serialise().get());
                        break;
                    default:
                        break;
                }
                areaTrigger = sMySQLStore.getMapGoBackTrigger(mapid);
                check = true;
            }
            else if (instance_id && !sInstanceMgr.getInstanceSave(instance_id)) // ... and instance is reseted then look for entrance.
            {
                areaTrigger = sMySQLStore.getMapEntranceTrigger(mapid);
                check = true;
            }
        }

        // Special Cases
        if (check)
        {
            if (areaTrigger)
            {
                // our Instance got reset, port us to the entrance
                sendTeleportAckPacket(LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
                if (mapid != areaTrigger->mapId)
                {
                    mapid = areaTrigger->mapId;
                    map = sMapMgr.createMap(mapid, this);
                }
            }
            else
            {
                return;
            }
        }

        if (IsInWorld())
            removeFromWorld();

        m_session->SendPacket(SmsgNewWorld(mapid, v).serialise().get());

        SetMapId(mapid);
        SetInstanceID(map->getInstanceId());
    }
    else
    {
        sendTeleportAckPacket(v);
    }

    setTransferStatus(TRANSFER_PENDING);
    m_sentTeleportPosition = v;
    SetPosition(v);

    if (sendpacket)
        sendTeleportPacket(v);

    speedCheatReset();

    m_zAxisPosition = 0.0f;
}

#ifdef AE_TBC
void Player::addItemsToWorld()
{
    for (uint8_t slotIndex = 0; slotIndex < INVENTORY_KEYRING_END; ++slotIndex)
    {
        if (const auto inventoryItem = getItemInterface()->GetInventoryItem(slotIndex))
        {
            inventoryItem->PushToWorld(m_WorldMap);

            if (slotIndex < INVENTORY_SLOT_BAG_END)      // only equipment slots get mods.
                applyItemMods(inventoryItem, slotIndex, true, false, true);

            if (slotIndex >= CURRENCYTOKEN_SLOT_START && slotIndex < CURRENCYTOKEN_SLOT_END)
                updateKnownCurrencies(inventoryItem->getEntry(), true);

            if (inventoryItem->isContainer() && getItemInterface()->IsBagSlot(slotIndex))
            {
                for (uint32_t containerSlot = 0; containerSlot < inventoryItem->getItemProperties()->ContainerSlots; ++containerSlot)
                {
                    if (Item* item = (static_cast<Container*>(inventoryItem))->getItem(static_cast<int16_t>(containerSlot)))
                        item->PushToWorld(m_WorldMap);
                }
            }
        }
    }

    updateStats();
}
#else
void Player::addItemsToWorld()
{
    for (uint8_t slotIndex = 0; slotIndex < CURRENCYTOKEN_SLOT_END; ++slotIndex)
    {
        if (Item* inventoryItem = getItemInterface()->GetInventoryItem(slotIndex))
        {
            inventoryItem->PushToWorld(m_WorldMap);

            if (slotIndex < INVENTORY_SLOT_BAG_END)
                applyItemMods(inventoryItem, slotIndex, true, false, true);

            if (slotIndex >= CURRENCYTOKEN_SLOT_START)
                updateKnownCurrencies(inventoryItem->getEntry(), true);

            if (inventoryItem->isContainer() && getItemInterface()->IsBagSlot(slotIndex))
            {
                for (uint32_t containerSlot = 0; containerSlot < inventoryItem->getItemProperties()->ContainerSlots; ++containerSlot)
                {
                    if (Item* item = (static_cast<Container*>(inventoryItem))->getItem(static_cast<int16_t>(containerSlot)))
                        item->PushToWorld(m_WorldMap);
                }
            }
        }
    }

    updateStats();
}
#endif

void Player::removeItemsFromWorld()
{
    for (uint8_t slotIndex = 0; slotIndex < CURRENCYTOKEN_SLOT_END; ++slotIndex)
    {
        if (Item* inventoryItem = getItemInterface()->GetInventoryItem((int8_t)slotIndex))
        {
            if (inventoryItem->IsInWorld())
            {
                if (slotIndex < INVENTORY_SLOT_BAG_END)
                    applyItemMods(inventoryItem, static_cast<int16_t>(slotIndex), false, false, true);

                inventoryItem->removeFromWorld();
            }

            if (inventoryItem->isContainer() && getItemInterface()->IsBagSlot(static_cast<int16_t>(slotIndex)))
            {
                for (uint32_t containerSlot = 0; containerSlot < inventoryItem->getItemProperties()->ContainerSlots; ++containerSlot)
                {
                    Item* item = (static_cast<Container*>(inventoryItem))->getItem(static_cast<int16_t>(containerSlot));
                    if (item && item->IsInWorld())
                        item->removeFromWorld();
                }
            }
        }
    }

    updateStats();
}

void Player::clearCooldownsOnLine(uint32_t skillLine, uint32_t calledFrom)
{
    for (const auto& spellId : m_spellSet)
    {
        if (spellId == calledFrom)
            continue;

        const auto spellSkillRange = sSpellMgr.getSkillEntryRangeForSpell(spellId);
        for (const auto& [_, skill_line_ability] : spellSkillRange)
        {
            if (skill_line_ability->skilline == skillLine)
                clearCooldownForSpell(spellId);
        }
    }
}

void Player::sendMirrorTimer(MirrorTimerTypes mirrorType, uint32_t max, uint32_t current, int32_t regen)
{
    if (static_cast<int>(max) == -1)
    {
        if (static_cast<int>(current) != -1)
            sendStopMirrorTimerPacket(mirrorType);

        return;
    }

    getSession()->SendPacket(SmsgStartMirrorTimer(mirrorType, current, max, regen).serialise().get());
}

float Player::calcRating(PlayerCombatRating index)
{
    uint32_t level = getLevel();
    if (level > 100)
        level = 100;

    uint32_t rating = getCombatRating(index);

    WDB::Structures::GtCombatRatingsEntry const* combatRatingsEntry = sGtCombatRatingsStore.lookupEntry(index * 100 + level - 1);
    if (combatRatingsEntry == nullptr)
        return float(rating);

    return (rating / combatRatingsEntry->val);
}

void Player::buildFlagUpdateForNonGroupSet(uint32_t index, uint32_t flag)
{
    for (const auto& inRangeObject : getInRangeObjectsSet())
    {
        if (inRangeObject && inRangeObject->isPlayer())
        {
            auto group = static_cast<Player*>(inRangeObject)->getGroup();
            if (!group && group != getGroup())
            {
                BuildFieldUpdatePacket(static_cast<Player*>(inRangeObject), index, flag);
            }
        }
    }
}

void Player::completeLoading()
{
    setGuildAndGroupInfo();

    SpellCastTargets targets(getGuid());

    if (getClass() == WARRIOR)
        castSpell(this, sSpellMgr.getSpellInfo(2457), true);

    for (const auto& spellId : m_spellSet)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(spellId);

        if (spellInfo != nullptr
            && (spellInfo->isPassive())
            && !(spellInfo->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET))
        {
            if (spellInfo->getRequiredShapeShift())
            {
                if (!(getShapeShiftMask() & spellInfo->getRequiredShapeShift()))
                    continue;
            }

            if (spellInfo->getCasterAuraState() != 0 && !hasAuraState(static_cast<AuraState>(spellInfo->getCasterAuraState()), spellInfo, this))
                continue;

            Spell* newSpell = sSpellMgr.newSpell(this, spellInfo, true, nullptr);
            newSpell->prepare(&targets);
        }
    }

    for (auto& loginaura : m_loginAuras)
    {
        if (SpellInfo const* sp = sSpellMgr.getSpellInfo(loginaura.id))
        {
            if (sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
                continue; //do not load auras that only exist while pet exist. We should recast these when pet is created anyway

            auto aura = sSpellMgr.newAura(sp, loginaura.dur, this, this, false);
            for (uint8_t x = 0; x < 3; x++)
            {
                if (sp->getEffect(x) == SPELL_EFFECT_APPLY_AURA)
                {
                    aura->addAuraEffect(static_cast<AuraEffect>(sp->getEffectApplyAuraName(x)), sp->getEffectBasePoints(x) + 1, sp->getEffectMiscValue(x), 1.0f, false, x);
                }
            }

            if (sp->getProcCharges() > 0 && loginaura.charges > 0)
                aura->setCharges(static_cast<uint16_t>(loginaura.charges), false);

            this->addAura(std::move(aura));
        }
    }

    if (getHealth() <= 0 && !hasPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        setDeathState(CORPSE);
    }
    else if (hasPlayerFlags(PLAYER_FLAG_DEATH_WORLD_ENABLE))
    {
        if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
            setDeathState(CORPSE);
        else
            sEventMgr.AddEvent(this, &Player::repopAtGraveyard, GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), EVENT_PLAYER_CHECKFORCHEATS, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    if (isDead())
    {
        if (getCorpseInstanceId() != 0)
        {
            if (const auto corpse = sObjectMgr.getCorpseByOwner(getGuidLow()))
                corpse->resetDeathClock();

            getSession()->SendPacket(SmsgCorpseReclaimDelay(CORPSE_RECLAIM_TIME_MS).serialise().get());
        }
    }

#if VERSION_STRING > TBC
    // useless logon spell
    Spell* logonspell = sSpellMgr.newSpell(this, sSpellMgr.getSpellInfo(836), false, nullptr);
    logonspell->prepare(&targets);
#endif

    if (isBanned())
    {
        kickFromServer(10000);
        broadcastMessage(getSession()->LocalizedWorldSrv(ServerString::SS_NOT_ALLOWED_TO_PLAY));
        broadcastMessage(getSession()->LocalizedWorldSrv(ServerString::SS_BANNED_FOR_TIME), getBanReason().c_str());
    }

    if (m_playerInfo->m_Group)
    {
        sEventMgr.AddEvent(this, &Player::eventGroupFullUpdate, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    if (m_sendOnlyRaidgroup)
    {
        sendRaidGroupOnly(0xFFFFFFFF, 0);
        m_sendOnlyRaidgroup = false;
    }

#if VERSION_STRING > TBC
    // add glyphs
    for (uint8_t j = 0; j < GLYPHS_COUNT; ++j)
    {
        auto glyph_properties = sGlyphPropertiesStore.lookupEntry(m_specs[m_talentActiveSpec].getGlyph(j));
        if (glyph_properties == nullptr)
            continue;

        castSpell(this, glyph_properties->SpellID, true);
    }

    //sEventMgr.AddEvent(this,&Player::SendAllAchievementData,EVENT_SEND_ACHIEVEMNTS_TO_PLAYER,ACHIEVEMENT_SEND_DELAY,1,0);
    sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::sendPowerUpdate, true, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, LOGIN_CIENT_SEND_DELAY, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
#endif
}

void Player::modifyBonuses(uint32_t type, int32_t val, bool apply)
{
    // Added some updateXXXX calls so when an item modifies a stat they get updated
    // also since this is used by auras now it will handle it for those
    int32_t _val = val;
    if (!apply)
        val = -val;

    switch (type)
    {
        case ITEM_MOD_MANA:
        {
            modMaxPower(POWER_TYPE_MANA, val);
            m_manaFromItems += val;
        }
        break;
        case ITEM_MOD_HEALTH:
        {
            modMaxHealth(val);
            m_healthFromItems += val;
        }
        break;
        case ITEM_MOD_AGILITY:       // modify agility
        case ITEM_MOD_STRENGTH:      // modify strength
        case ITEM_MOD_INTELLECT:     // modify intellect
        case ITEM_MOD_SPIRIT:        // modify spirit
        case ITEM_MOD_STAMINA:       // modify stamina
        {
            uint8_t convert[] = { 1, 0, 3, 4, 2 };
            if (_val > 0)
                m_flatStatModPos[convert[type - 3]] += val;
            else
                m_flatStatModNeg[convert[type - 3]] -= val;
            calcStat(convert[type - 3]);
        }
        break;
        case ITEM_MOD_WEAPON_SKILL_RATING:
        {
            modCombatRating(CR_WEAPON_SKILL_RANGED, val);
            modCombatRating(CR_WEAPON_SKILL_MAINHAND, val); // melee main hand
            modCombatRating(CR_WEAPON_SKILL_OFFHAND, val); // melee off hand
        }
        break;
        case ITEM_MOD_DEFENSE_RATING:
        {
            modCombatRating(CR_DEFENSE_SKILL, val);
        }
        break;
        case ITEM_MOD_DODGE_RATING:
        {
            modCombatRating(CR_DODGE, val);
        }
        break;
        case ITEM_MOD_PARRY_RATING:
        {
            modCombatRating(CR_PARRY, val);
        }
        break;
        case ITEM_MOD_SHIELD_BLOCK_RATING:
        {
            modCombatRating(CR_BLOCK, val);
        }
        break;
        case ITEM_MOD_MELEE_HIT_RATING:
        {
            modCombatRating(CR_HIT_MELEE, val);
        }
        break;
        case ITEM_MOD_RANGED_HIT_RATING:
        {
            modCombatRating(CR_HIT_RANGED, val);
        }
        break;
        case ITEM_MOD_SPELL_HIT_RATING:
        {
            modCombatRating(CR_HIT_SPELL, val);
        }
        break;
        case ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING:
        {
            modCombatRating(CR_CRIT_MELEE, val);
        }
        break;
        case ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING:
        {
            modCombatRating(CR_CRIT_RANGED, val);
        }
        break;
        case ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING:
        {
            modCombatRating(CR_CRIT_SPELL, val);
        }
        break;
        case ITEM_MOD_MELEE_HIT_AVOIDANCE_RATING:
        {
            modCombatRating(CR_HIT_TAKEN_MELEE, val);
        }
        break;
        case ITEM_MOD_RANGED_HIT_AVOIDANCE_RATING:
        {
            modCombatRating(CR_HIT_TAKEN_RANGED, val);
        }
        break;
        case ITEM_MOD_SPELL_HIT_AVOIDANCE_RATING:
        {
            modCombatRating(CR_HIT_TAKEN_SPELL, val);
        }
        break;
        case ITEM_MOD_MELEE_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case ITEM_MOD_RANGED_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case ITEM_MOD_SPELL_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case ITEM_MOD_MELEE_HASTE_RATING:
        {
            modCombatRating(CR_HASTE_MELEE, val); // melee
        }
        break;
        case ITEM_MOD_RANGED_HASTE_RATING:
        {
            modCombatRating(CR_HASTE_RANGED, val); // ranged
        }
        break;
        case ITEM_MOD_SPELL_HASTE_RATING:
        {
            modCombatRating(CR_HASTE_SPELL, val); // spell
        }
        break;
        case ITEM_MOD_HIT_RATING:
        {
            modCombatRating(CR_HIT_MELEE, val); // melee
            modCombatRating(CR_HIT_RANGED, val); // ranged
            modCombatRating(CR_HIT_SPELL, val); // spell
        }
        break;
        case ITEM_MOD_CRITICAL_STRIKE_RATING:
        {
            modCombatRating(CR_CRIT_MELEE, val);  // melee
            modCombatRating(CR_CRIT_RANGED, val);  // ranged
            modCombatRating(CR_CRIT_SPELL, val);   // spell
        }
        break;
        case ITEM_MOD_HIT_AVOIDANCE_RATING: // this is guessed based on layout of other fields
        {
            modCombatRating(CR_HIT_TAKEN_MELEE, val); // melee
            modCombatRating(CR_HIT_TAKEN_RANGED, val); // ranged
            modCombatRating(CR_HIT_TAKEN_SPELL, val); // spell
        }
        break;
        case ITEM_MOD_CRITICAL_AVOIDANCE_RATING:
        {

        } break;
        case ITEM_MOD_EXPERTISE_RATING:
        {
            modCombatRating(CR_EXPERTISE, val);
        }
        break;
        case ITEM_MOD_RESILIENCE_RATING:
        {
#if VERSION_STRING >= Cata
            modCombatRating(CR_RESILIENCE_CRIT_TAKEN, val); // melee
            modCombatRating(CR_RESILIENCE_PLAYER_DAMAGE_TAKEN, val); // ranged
#else
            modCombatRating(CR_CRIT_TAKEN_MELEE, val); // melee
            modCombatRating(CR_CRIT_TAKEN_RANGED, val); // ranged
#endif
            modCombatRating(CR_CRIT_TAKEN_SPELL, val); // spell
        }
        break;
        case ITEM_MOD_HASTE_RATING:
        {
            modCombatRating(CR_HASTE_MELEE, val); // melee
            modCombatRating(CR_HASTE_RANGED, val); // ranged
            modCombatRating(CR_HASTE_SPELL, val); // spell
        }
        break;
        case ITEM_MOD_ATTACK_POWER:
        {
            modAttackPowerMods(val);
            modRangedAttackPowerMods(val);
        }
        break;
        case ITEM_MOD_RANGED_ATTACK_POWER:
        {
            modRangedAttackPowerMods(val);
        }
        break;
        case ITEM_MOD_FERAL_ATTACK_POWER:
        {
            modAttackPowerMods(val);
        }
        break;
#if VERSION_STRING > Classic
        case ITEM_MOD_SPELL_HEALING_DONE:
        {
            for (uint8_t school = 1; school < TOTAL_SPELL_SCHOOLS; ++school)
            {
                m_healDoneMod[school] += val;
            }
            modModHealingDone(val);
        }
        break;
#endif
        case ITEM_MOD_SPELL_DAMAGE_DONE:
        {
            for (uint8_t school = 1; school < TOTAL_SPELL_SCHOOLS; ++school)
            {
                modModDamageDonePositive(school, val);
            }
        }
        break;
        case ITEM_MOD_MANA_REGENERATION:
        {
            m_manaFromItems += val;
        }
        break;
#if VERSION_STRING >= WotLK
        case ITEM_MOD_ARMOR_PENETRATION_RATING:
        {
            modCombatRating(CR_ARMOR_PENETRATION, val);
        }
        break;
#endif
        case ITEM_MOD_SPELL_POWER:
        {
            for (uint8_t school = 1; school < 7; ++school)
            {
                modModDamageDonePositive(school, val);
                m_healDoneMod[school] += val;
            }
#if VERSION_STRING > Classic
            modModHealingDone(val);
#endif
        }
        break;
    }
}

void Player::saveAuras(std::stringstream& ss)
{
    ss << "'";
    uint32_t charges = 0;
    uint16_t prevX = 0;

    // save all auras why only just positive?
    for (uint16_t x = AuraSlots::REMOVABLE_SLOT_START; x < AuraSlots::REMOVABLE_SLOT_END; x++)
    {
        auto* const aur = getAuraWithAuraSlot(x);
        if (aur != nullptr && aur->getTimeLeft() > 3000)
        {
            for (uint8_t i = 0; i < 3; ++i)
                if (aur->getSpellInfo()->getEffect(i) == SPELL_EFFECT_APPLY_GROUP_AREA_AURA || aur->getSpellInfo()->getEffect(i) == SPELL_EFFECT_APPLY_RAID_AREA_AURA || aur->getSpellInfo()->getEffect(i) == SPELL_EFFECT_ADD_FARSIGHT)
                    continue;

            if (aur->pSpellId)
                continue; // these auras were gained due to some proc. We do not save these either to avoid exploits of not removing them

            if (aur->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET)
                continue;

            // we are going to cast passive spells anyway on login so no need to save auras for them
            if (aur->IsPassive() && !(aur->getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
                continue;

            auto* const prevAura = getAuraWithAuraSlot(prevX);
            if (charges > 0 && aur->getSpellId() != prevAura->getSpellId())
            {
                ss << prevAura->getSpellId() << "," << prevAura->getTimeLeft() << "," << !prevAura->isNegative() << "," << charges << ",";
                charges = 0;
            }

            if (aur->getSpellInfo()->getProcCharges() == 0)
                ss << aur->getSpellId() << "," << aur->getTimeLeft() << "," << !aur->isNegative() << "," << uint32_t(0) << ",";
            else
                charges++;

            prevX = x;
        }
    }

    auto* const prevAura = getAuraWithAuraSlot(prevX);
    if (charges > 0 && prevAura)
        ss << prevAura->getSpellId() << "," << prevAura->getTimeLeft() << "," << !prevAura->isNegative() << "," << charges << ",";

    ss << "'";
}

void Player::calculateDamage()
{
    float rangeDamage;
    
    // Mainhand
    float attackPowerBonus = getCalculatedAttackPower() / 14000.0f;
    float deltaDone = (float)getModDamageDonePositive(SCHOOL_NORMAL) - (float)getModDamageDoneNegative(SCHOOL_NORMAL);

    if (isInFeralForm())
    {
        float damageMod = 1;
        for (std::map<uint32_t, WeaponModifier>::iterator i = m_damageDone.begin(); i != m_damageDone.end(); ++i)
        {
            if (i->second.wclass == (uint32_t)-1)  // applying only "any weapon" modifiers
                damageMod += i->second.value;
        }

        uint32_t level = getLevel();
        float averageFeralDamage;
        uint32_t itemLeven; // the two hand weapon with dps equal to cat or bear dps

        uint8_t shapeShiftForm = getShapeShiftForm();
        if (shapeShiftForm == FORM_CAT)
        {
            if (level < 42)
                itemLeven = level - 1;
            else if (level < 46)
                itemLeven = level;
            else if (level < 49)
                itemLeven = 2 * level - 45;
            else if (level < 60)
                itemLeven = level + 4;
            else
                itemLeven = 64;

            // 3rd grade polinom for calculating blue two-handed weapon dps based on itemlevel
            if (itemLeven <= 28)
                averageFeralDamage = 1.563e-03f * itemLeven * itemLeven * itemLeven - 1.219e-01f * itemLeven * itemLeven + 3.802e+00f * itemLeven - 2.227e+01f;
            else if (itemLeven <= 41)
                averageFeralDamage = -3.817e-03f * itemLeven * itemLeven * itemLeven + 4.015e-01f * itemLeven * itemLeven - 1.289e+01f * itemLeven + 1.530e+02f;
            else
                averageFeralDamage = 1.829e-04f * itemLeven * itemLeven * itemLeven - 2.692e-02f * itemLeven * itemLeven + 2.086e+00f * itemLeven - 1.645e+01f;

            rangeDamage = averageFeralDamage * 0.79f + deltaDone + attackPowerBonus * 1000.0f;
            rangeDamage *= damageMod;
            setMinDamage(rangeDamage > 0 ? rangeDamage : 0);

            rangeDamage = averageFeralDamage * 1.21f + deltaDone + attackPowerBonus * 1000.0f;
            rangeDamage *= damageMod;
            setMaxDamage(rangeDamage > 0 ? rangeDamage : 0);
        }
        else // Bear or Dire Bear Form
        {
            if (shapeShiftForm == FORM_BEAR)
                itemLeven = level;
            else
                itemLeven = level + 5; // DIRE_BEAR dps is slightly better than bear dps

            if (itemLeven > 70)
                itemLeven = 70;

            // 3rd grade polinom for calculating green two-handed weapon dps based on itemlevel
            if (itemLeven <= 30)
                averageFeralDamage = 7.638e-05f * itemLeven * itemLeven * itemLeven + 1.874e-03f * itemLeven * itemLeven + 4.967e-01f * itemLeven + 1.906e+00f;
            else if (itemLeven <= 44)
                averageFeralDamage = -1.412e-03f * itemLeven * itemLeven * itemLeven + 1.870e-01f * itemLeven * itemLeven - 7.046e+00f * itemLeven + 1.018e+02f;
            else
                averageFeralDamage = 2.268e-04f * itemLeven * itemLeven * itemLeven - 3.704e-02f * itemLeven * itemLeven + 2.784e+00f * itemLeven - 3.616e+01f;

            averageFeralDamage *= 2.5f; // Bear Form attack speed

            rangeDamage = averageFeralDamage * 0.79f + deltaDone + attackPowerBonus * 2500.0f;
            rangeDamage *= damageMod;
            setMinDamage(rangeDamage > 0 ? rangeDamage : 0);

            rangeDamage = averageFeralDamage * 1.21f + deltaDone + attackPowerBonus * 2500.0f;
            rangeDamage *= damageMod;
            setMaxDamage(rangeDamage > 0 ? rangeDamage : 0);
        }

        return;
    }

    // no druid shapeShift
    uint32_t speed = 2000;
    Item* item = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if (item && !m_isDisarmed)
        speed = item->getItemProperties()->Delay;

    float bonus = attackPowerBonus * speed;
    float damageMod = 1;
    for (std::map<uint32_t, WeaponModifier>::iterator weaponMod = m_damageDone.begin(); weaponMod != m_damageDone.end(); ++weaponMod)
    {
        if ((weaponMod->second.wclass == (uint32_t)-1) || //any weapon
            (item && ((1 << item->getItemProperties()->SubClass) & weaponMod->second.subclass)))
            damageMod += weaponMod->second.value;
    }

    rangeDamage = m_baseDamage[0] + deltaDone + bonus;
    rangeDamage *= damageMod;
    setMinDamage(rangeDamage > 0 ? rangeDamage : 0);

    rangeDamage = m_baseDamage[1] + deltaDone + bonus;
    rangeDamage *= damageMod;
    setMaxDamage(rangeDamage > 0 ? rangeDamage : 0);

    uint32_t cr = 0;
    if (item)
    {
        if (this->m_wratings.size())
        {
            std::map<uint32_t, uint32_t>::iterator itr = m_wratings.find(item->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    //\todo investigate
#if VERSION_STRING != Classic
    setCombatRating(CR_WEAPON_SKILL_MAINHAND, cr);
#endif
    // Mainhand END

    // Offhand START
    cr = 0;
    item = this->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (item)
    {
        if (!m_isDisarmed)
            speed = item->getItemProperties()->Delay;
        else
            speed = 2000;

        bonus = attackPowerBonus * speed;

        damageMod = 1;
        for (std::map<uint32_t, WeaponModifier>::iterator i = m_damageDone.begin(); i != m_damageDone.end(); ++i)
        {
            if ((i->second.wclass == (uint32_t)-1) || //any weapon
                (((1 << item->getItemProperties()->SubClass) & i->second.subclass))
                )
                damageMod += i->second.value;
        }

        rangeDamage = (m_baseOffhandDamage[0] + deltaDone + bonus) * m_offhandDmgMod;
        rangeDamage *= damageMod;
        setMinOffhandDamage(rangeDamage > 0 ? rangeDamage : 0);

        rangeDamage = (m_baseOffhandDamage[1] + deltaDone + bonus) * m_offhandDmgMod;
        rangeDamage *= damageMod;
        setMaxOffhandDamage(rangeDamage > 0 ? rangeDamage : 0);

        if (m_wratings.size())
        {
            std::map<uint32_t, uint32_t>::iterator itr = m_wratings.find(item->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }
    }
    //\todo investigate
#if VERSION_STRING != Classic
    setCombatRating(CR_WEAPON_SKILL_OFFHAND, cr);
#endif
    // Offhand END
    // Ranged
    cr = 0;
    item = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (item)
    {
        damageMod = 1;
        for (std::map<uint32_t, WeaponModifier>::iterator weaponMod = m_damageDone.begin(); weaponMod != m_damageDone.end(); ++weaponMod)
        {
            if ((weaponMod->second.wclass == (uint32_t)-1) || //any weapon
                (((1 << item->getItemProperties()->SubClass) & weaponMod->second.subclass)))
            {
                damageMod += weaponMod->second.value;
            }
        }

#if VERSION_STRING < Cata
        if (item->getItemProperties()->SubClass != 19)//wands do not have bonuses from RAP & ammo
        {
            //                ap_bonus = (getRangedAttackPower()+(int32_t)getRangedAttackPowerMods())/14000.0;
            attackPowerBonus = getCalculatedRangedAttackPower() / 14000.0f;
            bonus = attackPowerBonus * item->getItemProperties()->Delay;

            if (getAmmoId() && !m_requiresNoAmmo)
            {
                ItemProperties const* xproto = sMySQLStore.getItemProperties(getAmmoId());
                if (xproto)
                {
                    bonus += ((xproto->Damage[0].Min + xproto->Damage[0].Max) * item->getItemProperties()->Delay) / 2000.0f;
                }
            }
        }
        else
#endif
            bonus = 0;

        rangeDamage = m_baseRangedDamage[0] + deltaDone + bonus;
        rangeDamage *= damageMod;
        setMinRangedDamage(rangeDamage > 0 ? rangeDamage : 0);

        rangeDamage = m_baseRangedDamage[1] + deltaDone + bonus;
        rangeDamage *= damageMod;
        setMaxRangedDamage(rangeDamage > 0 ? rangeDamage : 0);

        if (m_wratings.size())
        {
            std::map<uint32_t, uint32_t>::iterator itr = m_wratings.find(item->getItemProperties()->SubClass);
            if (itr != m_wratings.end())
                cr = itr->second;
        }

    }
    //\todo investigate
#if VERSION_STRING != Classic
    setCombatRating(CR_WEAPON_SKILL_RANGED, cr);
#endif
    // Ranged END
    if (auto* const pet = getPet())
        pet->calculateDamage();//Re-calculate pet's too
}

uint32_t Player::getMainMeleeDamage(uint32_t attackPowerOverride)
{
    float result;

    float attackPowerBonus;
    if (attackPowerOverride)
        attackPowerBonus = attackPowerOverride / 14000.0f;
    else
        attackPowerBonus = getCalculatedAttackPower() / 14000.0f;

    if (isInFeralForm())
    {
        if (getShapeShiftForm() == FORM_CAT)
            result = attackPowerBonus * 1000.0f;
        else
            result = attackPowerBonus * 2500.0f;

        return Util::float2int32(result);
    }

    // no druid shapeShift
    uint32_t speed = 2000;
    Item* item = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (item && !m_isDisarmed)
        speed = item->getItemProperties()->Delay;

    result = attackPowerBonus * speed;
    return Util::float2int32(result);
}

#if VERSION_STRING > TBC
void Player::updateAchievementCriteria(AchievementCriteriaTypes type, int32_t miscValue1 /*= 0*/, int32_t miscValue2 /*= 0*/, uint32_t miscValue3 /*= 0*/, Unit* unit /*= nullptr*/)
{
    m_achievementMgr->updateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, unit);
    Guild* guild = sGuildMgr.getGuildById(getGuildId());
    if (!guild)
        return;

    // Update only individual achievement criteria here, otherwise we may get multiple updates
    // from a single boss kill
    if (m_achievementMgr->isGroupCriteriaType(type))
        return;

    // ToDo Cata Has Guild Achievements
    //guild->updateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, unit, this);
}
#endif

Creature* Player::getCreatureWhenICanInteract(WoWGuid const& guid, uint32_t npcflagmask)
{
    // unit checks
    if (!guid)
        return nullptr;

    if (!IsInWorld())
        return nullptr;

    if (isInFlight())
        return nullptr;

    Creature* creature = getWorldMapCreature(guid.getRawGuid());
    if (!creature)
        return nullptr;

    // Deathstate checks
    if (!isAlive() && !(creature->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_GHOST))
        return nullptr;

    // alive or spirit healer
    if (!creature->isAlive() && !(creature->GetCreatureProperties()->typeFlags & CREATURE_FLAG1S_DEAD_INTERACT))
        return nullptr;

    // appropriate npc type
    if (npcflagmask && !(creature->getNpcFlags() & npcflagmask))
        return nullptr;

    // not allow interaction under control, but allow with own pets
    if (creature->getCharmGuid())
        return nullptr;

    // not unfriendly/hostile
    if (this->isHostileTo(creature))
        return nullptr;

    // not too far
    if (!creature->IsWithinDistInMap(this, creature->getCombatReach() + 4.0f))
        return nullptr;

    return creature;
}
