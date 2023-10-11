/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

extern "C"
{
#include <lua/lua.h>
}

class Unit;

class LuaUnit
{
public:
    static int GetDisplay(lua_State* L, Unit* ptr);
    static int GetNativeDisplay(lua_State* L, Unit* ptr);
    static int GossipCreateMenu(lua_State* L, Unit* ptr);
    static int GossipMenuAddItem(lua_State* L, Unit* /*ptr*/);
    static int GossipSendMenu(lua_State* L, Unit* /*ptr*/);
    static int GossipSendPOI(lua_State* L, Unit* ptr);
    static int GossipSendQuickMenu(lua_State* L, Unit* ptr);
    static int GossipAddQuests(lua_State* L, Unit* ptr);
    static int GossipComplete(lua_State* /*L*/, Unit* ptr);
    static int IsPlayer(lua_State* L, Unit* ptr);
    static int IsCreature(lua_State* L, Unit* ptr);
    static int Emote(lua_State* L, Unit* ptr);
    static int GetName(lua_State* L, Unit* ptr);
    static int PhaseSet(lua_State* L, Unit* ptr);
    static int PhaseAdd(lua_State* L, Unit* ptr);
    static int PhaseDelete(lua_State* L, Unit* ptr);
    static int GetPhase(lua_State* L, Unit* ptr);
    static int SendChatMessage(lua_State* L, Unit* ptr);
    static int PlayerSendChatMessage(lua_State* L, Unit* ptr);
    static int AggroWithInRangeFriends(lua_State* /*L*/, Unit* ptr);
    static int MoveTo(lua_State* L, Unit* ptr);
    static int MoveRandomArea(lua_State* L, Unit* ptr);
    static int GetX(lua_State* L, Unit* ptr);
    static int GetY(lua_State* L, Unit* ptr);
    static int GetZ(lua_State* L, Unit* ptr);
    static int GetO(lua_State* L, Unit* ptr);
    static int CastSpell(lua_State* L, Unit* ptr);
    static int FullCastSpell(lua_State* L, Unit* ptr);
    static int FullCastSpellOnTarget(lua_State* L, Unit* ptr);
    static int CastSpellOnTarget(lua_State* L, Unit* ptr);
    static int SpawnCreature(lua_State* L, Unit* ptr);
    static int SpawnGameObject(lua_State* L, Unit* ptr);
    static int RegisterEvent(lua_State* L, Unit* ptr);
    static int CreateLuaEvent(lua_State* L, Unit* ptr);
    static int RemoveEvents(lua_State* L, Unit* ptr);
    static int SetFaction(lua_State* L, Unit* ptr);
    static int GetNativeFaction(lua_State* L, Unit* ptr);
    static int SetStandState(lua_State* L, Unit* ptr);   //states 0..8
    static int IsInCombat(lua_State* L, Unit* ptr);
    static int SetScale(lua_State* L, Unit* ptr);
    static int SetModel(lua_State* L, Unit* ptr);
    static int SetNPCFlags(lua_State* L, Unit* ptr);
    static int SetMount(lua_State* L, Unit* ptr);
    static int RemoveItem(lua_State* L, Unit* ptr);
    static int AddItem(lua_State* L, Unit* ptr);
    static int GetInstanceID(lua_State* L, Unit* ptr);
    static int GetClosestPlayer(lua_State* L, Unit* ptr);
    static int GetRandomPlayer(lua_State* L, Unit* ptr);
    static int GetRandomFriend(lua_State* L, Unit* ptr);
    static int GetRandomEnemy(lua_State* L, Unit* ptr);
    static int StopMovement(lua_State* L, Unit* ptr);
    static int RemoveAura(lua_State* L, Unit* ptr);
    static int CanAttack(lua_State* L, Unit* ptr);
    static int PlaySoundToSet(lua_State* L, Unit* ptr);
    static int PlaySoundToPlayer(lua_State* L, Unit* ptr);
    static int GetUnitBySqlId(lua_State* L, Unit* ptr);
    static int GetInventoryItem(lua_State* L, Unit* ptr);
    static int GetInventoryItemById(lua_State* L, Unit* ptr);
    static int SetZoneWeather(lua_State* L, Unit* /*ptr*/);
    static int SetPlayerWeather(lua_State* L, Unit* ptr);
    static int Despawn(lua_State* L, Unit* ptr);
    static int GetInRangeFriends(lua_State* L, Unit* ptr);
    static int GetInRangeEnemies(lua_State* L, Unit* ptr);
    static int GetInRangeUnits(lua_State* L, Unit* ptr);
    static int getHealthPct(lua_State* L, Unit* ptr);
    static int SetHealthPct(lua_State* L, Unit* ptr);
    static int GetItemCount(lua_State* L, Unit* ptr);
    static int GetMainTank(lua_State* L, Unit* ptr);
    static int GetAddTank(lua_State* L, Unit* ptr);
    static int ClearThreatList(lua_State* /*L*/, Unit* ptr);
    static int SetTauntedBy(lua_State* L, Unit* ptr);
    static int ModThreat(lua_State* L, Unit* ptr);
    static int GetThreatByPtr(lua_State* L, Unit* ptr);
    static int ChangeTarget(lua_State* L, Unit* ptr);
    static int HasFinishedQuest(lua_State* L, Unit* ptr);
    static int FinishQuest(lua_State* L, Unit* ptr);
    static int StartQuest(lua_State* L, Unit* ptr);
    static int UnlearnSpell(lua_State* L, Unit* ptr);
    static int LearnSpell(lua_State* L, Unit* ptr);
    static int LearnSpells(lua_State* L, Unit* ptr);
    static int MarkQuestObjectiveAsComplete(lua_State* L, Unit* ptr);
    static int SendAreaTriggerMessage(lua_State* L, Unit* ptr);
    static int SendBroadcastMessage(lua_State* L, Unit* ptr);
    static int TeleportUnit(lua_State* L, Unit* ptr);
    static int GetHealth(lua_State* L, Unit* ptr);
    static int GetMaxHealth(lua_State* L, Unit* ptr);
    static int SetHealth(lua_State* L, Unit* ptr);
    static int SetMaxHealth(lua_State* L, Unit* ptr);
    static int WipeHateList(lua_State* /*L*/, Unit* ptr);
    static int WipeTargetList(lua_State* /*L*/, Unit* ptr);
    static int WipeCurrentTarget(lua_State* /*L*/, Unit* ptr);
    static int GetPlayerClass(lua_State* L, Unit* ptr);
    static int ClearHateList(lua_State* /*L*/, Unit* ptr);
    static int SetMana(lua_State* L, Unit* ptr);
    static int SetMaxMana(lua_State* L, Unit* ptr);
    static int GetPlayerRace(lua_State* L, Unit* ptr);
    static int SetFlying(lua_State* /*L*/, Unit* ptr);
    static int Land(lua_State* /*L*/, Unit* ptr);
    static int HasAura(lua_State* L, Unit* ptr);
    static int ReturnToSpawnPoint(lua_State* /*L*/, Unit* ptr);
    static int GetGUID(lua_State* L, Unit* ptr);
    static int GetDistance(lua_State* L, Unit* ptr);
    static int GetDistanceYards(lua_State* L, Unit* ptr);
    static int GetDuelState(lua_State* L, Unit* ptr);
    static int GetCreatureNearestCoords(lua_State* L, Unit* ptr);
    static int GetGameObjectNearestCoords(lua_State* L, Unit* ptr);
    static int SetPosition(lua_State* L, Unit* ptr);
    static int GetLandHeight(lua_State* L, Unit* ptr);
    static int IsInPhase(lua_State* L, Unit* ptr);
    static int HasFlag(lua_State* L, Unit* ptr);
    static int QuestAddStarter(lua_State* L, Unit* ptr);
    static int QuestAddFinisher(lua_State* L, Unit* ptr);
    static int castSpellLoc(lua_State* L, Unit* ptr);
    static int FullCastSpellAoF(lua_State* L, Unit* ptr);
    static int SetInFront(lua_State* L, Unit* ptr);
    static int RemoveAllAuras(lua_State* /*L*/, Unit* ptr);
    static int CancelSpell(lua_State* /*L*/, Unit* ptr);
    static int IsAlive(lua_State* L, Unit* ptr);
    static int IsDead(lua_State* L, Unit* ptr);
    static int IsInWorld(lua_State* L, Unit* ptr);
    static int GetZoneId(lua_State* L, Unit* ptr);
    static int Root(lua_State* /*L*/, Unit* ptr);
    static int Unroot(lua_State* /*L*/, Unit* ptr);
    static int SetOutOfCombatRange(lua_State* L, Unit* ptr);
    static int ModifyRunSpeed(lua_State* L, Unit* ptr);
    static int ModifyWalkSpeed(lua_State* L, Unit* ptr);
    static int ModifyFlySpeed(lua_State* L, Unit* ptr);
    static int isFlying(lua_State* L, Unit* ptr);
    static int SendAIReaction(lua_State* /*L*/, Unit* ptr);
    static int SetOrientation(lua_State* L, Unit* ptr);
    static int GetSpawnX(lua_State* L, Unit* ptr);
    static int GetSpawnY(lua_State* L, Unit* ptr);
    static int GetSpawnZ(lua_State* L, Unit* ptr);
    static int GetSpawnO(lua_State* L, Unit* ptr);
    static int GetInRangePlayersCount(lua_State* L, Unit* ptr);
    static int GetEntry(lua_State* L, Unit* ptr);
    static int HandleEvent(lua_State* L, Unit* ptr);
    static int GetCurrentSpellId(lua_State* L, Unit* ptr);
    static int GetCurrentSpell(lua_State* L, Unit* ptr);
    static int GetFloatValue(lua_State* /*L*/, Unit* /*ptr*/);
    static int SendPacket(lua_State* L, Unit* ptr);
    static int SendPacketToGroup(lua_State* L, Unit* ptr);
    static int SendPacketToPlayer(lua_State* L, Unit* ptr);
    static int ModUInt32Value(lua_State* /*L*/, Unit* /*ptr*/);
    static int ModFloatValue(lua_State* /*L*/, Unit* /*ptr*/);
    static int SetUInt32Value(lua_State* /*L*/, Unit* /*ptr*/);
    static int SetUInt64Value(lua_State* /*L*/, Unit* /*ptr*/);
    static int RemoveFlag(lua_State* /*L*/, Unit* /*ptr*/);
    static int SetFlag(lua_State* /*L*/, Unit* /*ptr*/);
    static int SetFloatValue(lua_State* /*L*/, Unit* /*ptr*/);
    static int GetUInt32Value(lua_State* /*L*/, Unit* /*ptr*/);
    static int GetUInt64Value(lua_State* /*L*/, Unit* /*ptr*/);
    static int AdvanceQuestObjective(lua_State* L, Unit* ptr);
    static int Heal(lua_State* L, Unit* ptr);
    static int Energize(lua_State* L, Unit* ptr);
    static int SendChatMessageAlternateEntry(lua_State* L, Unit* ptr);
    static int SendChatMessageToPlayer(lua_State* L, Unit* ptr);
    static int GetManaPct(lua_State* L, Unit* ptr);
    static int GetPowerPct(lua_State* L, Unit* ptr);
    static int GetMana(lua_State* L, Unit* ptr);
    static int GetPower(lua_State* L, Unit* ptr);
    static int GetMaxMana(lua_State* L, Unit* ptr);
    static int GetMaxPower(lua_State* L, Unit* ptr);
    static int SetPowerType(lua_State* L, Unit* ptr);
    static int SetMaxPower(lua_State* L, Unit* ptr);
    static int SetPower(lua_State* L, Unit* ptr);
    static int SetPowerPct(lua_State* L, Unit* ptr);
    static int GetPowerType(lua_State* L, Unit* ptr);
    static int Strike(lua_State* L, Unit* ptr);
    static int SetAttackTimer(lua_State* L, Unit* ptr);
    static int Kill(lua_State* L, Unit* ptr);
    static int DealDamage(lua_State* L, Unit* ptr);
    static int setCurrentTarget(lua_State* L, Unit* ptr);
    static int getCurrentTarget(lua_State* L, Unit* ptr);
    static int SetPetOwner(lua_State* L, Unit* ptr);
    static int DismissPet(lua_State* /*L*/, Unit* ptr);
    static int IsPet(lua_State* L, Unit* ptr);
    static int GetPetOwner(lua_State* L, Unit* ptr);
    static int SetUnitToFollow(lua_State* L, Unit* ptr);
    static int IsInFront(lua_State* L, Unit* ptr);
    static int IsInBack(lua_State* L, Unit* ptr);
    static int IsPacified(lua_State* L, Unit* ptr);
    static int SetPacified(lua_State* L, Unit* ptr);
    static int IsFeared(lua_State* L, Unit* ptr);
    static int IsStunned(lua_State* L, Unit* ptr);
    static int CreateGuardian(lua_State* L, Unit* ptr);
    static int IsInArc(lua_State* L, Unit* ptr);
    static int IsInWater(lua_State* L, Unit* ptr);
    static int GetAITargetsCount(lua_State* L, Unit* ptr);
    static int GetUnitByGUID(lua_State* L, Unit* ptr);
    static int GetAITargets(lua_State* L, Unit* ptr);
    static int GetInRangeObjectsCount(lua_State* L, Unit* ptr);
    static int GetInRangePlayers(lua_State* L, Unit* ptr);
    static int GetGroupPlayers(lua_State* L, Unit* ptr);
    static int GetDungeonDifficulty(lua_State* L, Unit* ptr);
    static int IsGroupFull(lua_State* L, Unit* ptr);
    static int GetGroupLeader(lua_State* L, Unit* ptr);
    static int SetGroupLeader(lua_State* L, Unit* ptr);
    static int AddGroupMember(lua_State* L, Unit* ptr);
    static int SetDungeonDifficulty(lua_State* L, Unit* ptr);
    static int ExpandToRaid(lua_State* /*L*/, Unit* ptr);
    static int GetInRangeGameObjects(lua_State* L, Unit* ptr);
    static int HasInRangeObjects(lua_State* L, Unit* ptr);
    static int SetFacing(lua_State* L, Unit* ptr);
    static int CalcToDistance(lua_State* L, Unit* ptr);
    static int CalcAngle(lua_State* L, Unit* ptr);
    static int CalcRadAngle(lua_State* L, Unit* ptr);
    static int IsInvisible(lua_State* /*L*/, Unit* ptr);   //THIS IS NOT "IS" IT'S SET!
    static int MoveFly(lua_State* L, Unit* ptr);
    static int IsInvincible(lua_State* L, Unit* ptr);   //THIS IS NOT "IS" IT'S SET!
    static int ResurrectPlayer(lua_State* /*L*/, Unit* ptr);
    static int KickPlayer(lua_State* L, Unit* ptr);
    static int CanCallForHelp(lua_State* L, Unit* ptr);
    static int CallForHelpHp(lua_State* L, Unit* ptr);
    static int SetDeathState(lua_State* L, Unit* ptr);
    static int SetCreatureName(lua_State* /*L*/, Unit* ptr);
    static int SetBindPoint(lua_State* L, Unit* ptr);
    static int SoftDisconnect(lua_State* /*L*/, Unit* ptr);
    static int Possess(lua_State* L, Unit* ptr);
    static int Unpossess(lua_State* /*L*/, Unit* ptr);
    static int RemoveFromWorld(lua_State* /*L*/, Unit* ptr);
    static int GetFaction(lua_State* L, Unit* ptr);
    static int SpellNonMeleeDamageLog(lua_State* L, Unit* ptr);
    static int NoRespawn(lua_State* L, Unit* ptr);
    static int GetMapId(lua_State* L, Unit* ptr);
    static int AttackReaction(lua_State* L, Unit* ptr);
    static int eventCastSpell(lua_State* L, Unit* ptr);
    static int IsPlayerMoving(lua_State* L, Unit* ptr);
    static int IsPlayerAttacking(lua_State* L, Unit* ptr);
    static int GetFactionStanding(lua_State* L, Unit* ptr);
    static int SetPlayerAtWar(lua_State* L, Unit* ptr);
    static int SetPlayerStanding(lua_State* L, Unit* ptr);
    static int SetPlayerSpeed(lua_State* L, Unit* ptr);
    static int GiveHonor(lua_State* L, Unit* ptr);
    static int TakeHonor(lua_State* L, Unit* ptr);
    static int GetStanding(lua_State* L, Unit* ptr);
    static int RemoveThreatByPtr(lua_State* L, Unit* ptr);
    static int HasItem(lua_State* L, Unit* ptr);
    static int PlaySpellVisual(lua_State* L, Unit* ptr);
    static int GetLevel(lua_State* L, Unit* ptr);
    static int SetLevel(lua_State* L, Unit* ptr);
    static int AddSkill(lua_State* L, Unit* ptr);
    static int RemoveSkill(lua_State* L, Unit* ptr);
    static int FlyCheat(lua_State* L, Unit* ptr);
    static int AdvanceSkill(lua_State* L, Unit* ptr);
    static int RemoveAurasByMechanic(lua_State* L, Unit* ptr);
    static int RemoveAurasType(lua_State* L, Unit* ptr);
    static int AddAura(lua_State* L, Unit* ptr);
    static int SetStealth(lua_State* /*L*/, Unit* ptr);
    static int GetStealthLevel(lua_State* L, Unit* ptr);
    static int IsStealthed(lua_State* L, Unit* ptr);
    static int RemoveStealth(lua_State* /*L*/, Unit* ptr);
    static int InterruptSpell(lua_State* /*L*/, Unit* ptr);
    static int IsPoisoned(lua_State* L, Unit* ptr);
    static int ModifyAIUpdateEvent(lua_State* L, Unit* ptr);
    static int RemoveAIUpdateEvent(lua_State* /*L*/, Unit* ptr);
    static int DealGoldCost(lua_State* L, Unit* ptr);
    static int DealGoldMerit(lua_State* L, Unit* ptr);
    static int DeMorph(lua_State* /*L*/, Unit* ptr);
    static int Attack(lua_State* L, Unit* ptr);
    static int CanUseCommand(lua_State* L, Unit* ptr);
    static int GetSelection(lua_State* L, Unit* ptr);
    static int GetSelectedGO(lua_State* L, Unit* ptr);
    static int SetSelectedGO(lua_State* L, Unit* ptr);
    static int RepairAllPlayerItems(lua_State* L, Unit* ptr);
    static int SetKnownTitle(lua_State* L, Unit* ptr);
    static int UnsetKnownTitle(lua_State* L, Unit* ptr);
    static int LifeTimeKills(lua_State* L, Unit* ptr);
    static int HasTitle(lua_State* L, Unit* ptr);
    static int GetMaxSkill(lua_State* L, Unit* ptr);
    static int GetCurrentSkill(lua_State* L, Unit* ptr);
    static int HasSkill(lua_State* L, Unit* ptr);
    static int GetGuildName(lua_State* L, Unit* ptr);
    static int ClearCooldownForSpell(lua_State* L, Unit* ptr);
    static int HasSpell(lua_State* L, Unit* ptr);
    static int ClearAllCooldowns(lua_State* /*L*/, Unit* ptr);
    static int ResetAllTalents(lua_State* /*L*/, Unit* ptr);
    static int GetAccountName(lua_State* L, Unit* ptr);
    static int GetGmRank(lua_State* L, Unit* ptr);
    static int IsGm(lua_State* L, Unit* ptr);
    static int SavePlayer(lua_State* /*L*/, Unit* ptr);
    static int HasQuest(lua_State* L, Unit* ptr);
    static int CreatureHasQuest(lua_State* L, Unit* ptr);
    static int RemovePvPFlag(lua_State* /*L*/, Unit* ptr);
    static int RemoveNegativeAuras(lua_State* /*L*/, Unit* ptr);
    static int GossipMiscAction(lua_State* L, Unit* ptr);
    static int SendVendorWindow(lua_State* L, Unit* ptr);
    static int SendTrainerWindow(lua_State* L, Unit* ptr);
    static int SendInnkeeperWindow(lua_State* L, Unit* ptr);
    static int SendBankWindow(lua_State* L, Unit* ptr);
    static int SendAuctionWindow(lua_State* L, Unit* ptr);
    static int SendBattlegroundWindow(lua_State* L, Unit* ptr);
    static int SendLootWindow(lua_State* L, Unit* ptr);
    static int AddLoot(lua_State* L, Unit* ptr);
    static int VendorAddItem(lua_State* L, Unit* ptr);
    static int VendorRemoveItem(lua_State* L, Unit* ptr);
    static int VendorRemoveAllItems(lua_State* /*L*/, Unit* ptr);
    static int EquipWeapons(lua_State* L, Unit* ptr);
    static int Dismount(lua_State* /*L*/, Unit* ptr);
    static int GiveXp(lua_State* L, Unit* ptr);
    static int AdvanceAllSkills(lua_State* L, Unit* ptr);
    static int GetTeam(lua_State* L, Unit* ptr); // returns 0 for alliance, 1 for horde.
    static int StartTaxi(lua_State* L, Unit* ptr);
    static int IsOnTaxi(lua_State* L, Unit* ptr);
    static int GetTaxi(lua_State* L, Unit* ptr);
    static int SetPlayerLock(lua_State* L, Unit* ptr);
    static int MovePlayerTo(lua_State* L, Unit* ptr);
    static int ChannelSpell(lua_State* L, Unit* ptr);
    static int StopChannel(lua_State* /*L*/, Unit* ptr);

    //////////////////////////////////////////////////////////////////////////////////////////
    // WORLDSTATES/WORLD PVP NOT SUPPORTED
    //static int SetWorldState(lua_State * L, Unit * ptr);

    static int EnableFlight(lua_State* L, Unit* ptr);
    static int GetCoinage(lua_State* L, Unit* ptr);
    static int FlagPvP(lua_State* /*L*/, Unit* ptr);
    static int IsMounted(lua_State* L, Unit* ptr);
    static int IsGroupedWith(lua_State* L, Unit* ptr);
    static int GetGroupType(lua_State* L, Unit* ptr);
    static int GetTotalHonor(lua_State* L, Unit* ptr); // I loathe typing "honour" like "honor".
    static int GetHonorToday(lua_State* L, Unit* ptr);
    static int GetHonorYesterday(lua_State* L, Unit* ptr);
    static int GetArenaPoints(lua_State* L, Unit* ptr);
    static int AddArenaPoints(lua_State* L, Unit* ptr);
    static int RemoveArenaPoints(lua_State* L, Unit* ptr);
    static int AddLifetimeKills(lua_State* L, Unit* ptr);
    static int GetGender(lua_State* L, Unit* ptr);
    static int SetGender(lua_State* L, Unit* ptr);
    static int SendPacketToGuild(lua_State* L, Unit* ptr);
    static int GetGuildId(lua_State* L, Unit* ptr);
    static int GetGuildRank(lua_State* L, Unit* ptr);
    static int SetGuildRank(lua_State* L, Unit* ptr);
    static int IsInGuild(lua_State* L, Unit* ptr);
    static int SendGuildInvite(lua_State* L, Unit* ptr);
    static int DemoteGuildMember(lua_State* L, Unit* ptr);
    static int PromoteGuildMember(lua_State* L, Unit* ptr);
    static int SetGuildMotd(lua_State* L, Unit* ptr);
    static int GetGuildMotd(lua_State* L, Unit* ptr);
    static int SetGuildInformation(lua_State* L, Unit* ptr);
    static int AddGuildMember(lua_State* L, Unit* ptr);
    static int RemoveGuildMember(lua_State* L, Unit* ptr);
    static int SetPublicNote(lua_State* L, Unit* ptr);
    static int SetOfficerNote(lua_State* L, Unit* ptr);
    static int DisbandGuild(lua_State* L, Unit* ptr);
    static int ChangeGuildMaster(lua_State* L, Unit* ptr);
    static int SendGuildChatMessage(lua_State* L, Unit* ptr);
    static int SendGuildLog(lua_State* /*L*/, Unit* ptr);
    static int GuildBankDepositMoney(lua_State* L, Unit* ptr);
    static int GuildBankWithdrawMoney(lua_State* L, Unit* ptr);
    static int SetByteValue(lua_State* /*L*/, Unit* ptr);
    static int GetByteValue(lua_State* /*L*/, Unit* ptr);
    static int IsPvPFlagged(lua_State* L, Unit* ptr);
    static int IsFFAPvPFlagged(lua_State* L, Unit* ptr);
    static int GetGuildLeader(lua_State* L, Unit* ptr);
    static int GetGuildMemberCount(lua_State* L, Unit* ptr);
    static int IsFriendly(lua_State* L, Unit* ptr);
    static int IsInChannel(lua_State* L, Unit* ptr);
    static int JoinChannel(lua_State* L, Unit* ptr);
    static int LeaveChannel(lua_State* L, Unit* ptr);
    static int SetChannelName(lua_State* L, Unit* ptr);
    static int SetChannelPassword(lua_State* L, Unit* ptr);
    static int GetChannelPassword(lua_State* L, Unit* ptr);
    static int KickFromChannel(lua_State* L, Unit* ptr);
    static int BanFromChannel(lua_State* L, Unit* ptr);
    static int UnbanFromChannel(lua_State* L, Unit* ptr);
    static int GetChannelMemberCount(lua_State* L, Unit* ptr);
    static int GetPlayerMovementVector(lua_State* L, Unit* ptr);
    static int GetPlayerMovementFlags(lua_State* L, Unit* ptr);
    static int Repop(lua_State* /*L*/, Unit* ptr);
    static int SetMovementFlags(lua_State* L, Unit* ptr);
    static int GetSpawnId(lua_State* L, Unit* ptr);
    static int ResetTalents(lua_State* /*L*/, Unit* ptr);
    static int SetTalentPoints(lua_State* L, Unit* ptr);
    static int GetTalentPoints(lua_State* L, Unit* ptr);
    static int EventChat(lua_State* L, Unit* ptr);
    static int GetEquippedItemBySlot(lua_State* L, Unit* ptr);
    static int GetGuildMembers(lua_State* L, Unit* ptr);
    static int AddAchievement(lua_State* L, Unit* ptr);
    static int RemoveAchievement(lua_State* L, Unit* ptr);
    static int HasAchievement(lua_State* L, Unit* ptr);
    static int GetAreaId(lua_State* L, Unit* ptr);
    static int ResetPetTalents(lua_State* /*L*/, Unit* ptr);
    static int IsDazed(lua_State* L, Unit* ptr);
    static int IsRooted(lua_State* L, Unit* ptr);
    static int HasAuraWithMechanic(lua_State* L, Unit* ptr);
    static int HasNegativeAura(lua_State* L, Unit* ptr);
    static int HasPositiveAura(lua_State* L, Unit* ptr);
    static int GetClosestEnemy(lua_State* L, Unit* ptr);
    static int GetClosestFriend(lua_State* L, Unit* ptr);
    static int GetClosestUnit(lua_State* L, Unit* ptr);
    static int GetObjectType(lua_State* L, Unit* ptr);
    static int DisableMelee(lua_State* L, Unit* ptr);
    static int DisableSpells(lua_State* L, Unit* ptr);
    static int DisableRanged(lua_State* L, Unit* ptr);
    static int DisableCombat(lua_State* L, Unit* ptr);
    static int DisableTargeting(lua_State* L, Unit* ptr);
    static int IsInGroup(lua_State* L, Unit* ptr);
    static int GetLocation(lua_State* L, Unit* ptr);
    static int GetByte(lua_State* L, Unit* ptr);
    static int SetByte(lua_State* L, Unit* ptr);
    static int GetSpawnLocation(lua_State* L, Unit* ptr);
    static int GetObjectByGuid(lua_State* L, Unit* ptr);
    static int GetSecondHated(lua_State* L, Unit* ptr);
    static int UseAI(lua_State* L, Unit* ptr);
    static int FlagFFA(lua_State* L, Unit* ptr);
    static int TeleportCreature(lua_State* L, Unit* ptr);
    static int IsInDungeon(lua_State* L, Unit* ptr);
    static int IsInRaid(lua_State* L, Unit* ptr);
    static int IsHostile(lua_State* L, Unit* ptr);
    static int IsAttackable(lua_State* L, Unit* ptr);
    static int GetQuestLogSlot(lua_State* L, Unit* ptr);
    static int GetAuraStackCount(lua_State* L, Unit* ptr);
    static int AddAuraObject(lua_State* L, Unit* ptr);
    static int GetAuraObjectById(lua_State* L, Unit* ptr);
    static int StopPlayerAttack(lua_State* /*L*/, Unit* ptr);
    static int GetQuestObjectiveCompletion(lua_State* L, Unit* ptr);
    static int IsOnVehicle(lua_State* L, Unit* ptr);
    static int SpawnAndEnterVehicle(lua_State* L, Unit* ptr);
    static int DismissVehicle(lua_State* /*L*/, Unit* ptr);
    static int AddVehiclePassenger(lua_State* L, Unit* ptr);
    static int HasEmptyVehicleSeat(lua_State* L, Unit* ptr);
    static int EnterVehicle(lua_State* L, Unit* ptr);
    static int ExitVehicle(lua_State* /*L*/, Unit* ptr);
    static int GetVehicleBase(lua_State* L, Unit* ptr);
    static int EjectAllVehiclePassengers(lua_State* /*L*/, Unit* ptr);
    static int EjectVehiclePassengerFromSeat(lua_State* L, Unit* ptr);
    static int MoveVehiclePassengerToSeat(lua_State* L, Unit* ptr);
    static int SendCinematic(lua_State* L, Unit* ptr);
    static int GetWorldStateForZone(lua_State* L, Unit* ptr);
    static int SetWorldStateForZone(lua_State* L, Unit* ptr);
    static int SetWorldStateForPlayer(lua_State* L, Unit* ptr);
};