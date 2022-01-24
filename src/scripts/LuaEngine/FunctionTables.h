/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// Script function tables

RegType<Item> ItemMethods[] =
{
    { "GossipCreateMenu", &luaItem::GossipCreateMenu },
    { "GossipMenuAddItem", &luaItem::GossipMenuAddItem },
    { "GossipSendMenu", &luaItem::GossipSendMenu },
    { "GossipComplete", &luaItem::GossipComplete },
    { "GossipSendPOI", &luaItem::GossipSendPOI },
    { "GossipSendQuickMenu", &luaItem::GossipSendQuickMenu },
    { "GetOwner", &luaItem::GetOwner },
    { "AddEnchantment", &luaItem::AddEnchantment },
    { "RemoveEnchantment", &luaItem::RemoveEnchantment },
    { "GetEntryId", &luaItem::GetEntryId },
    { "GetName", &luaItem::GetName },
    { "GetSpellId", &luaItem::GetSpellId },
    { "GetSpellTrigger", &luaItem::GetSpellTrigger },
    { "GetGuid", &luaItem::GetGUID },
    { "AddLoot", &luaItem::AddLoot},
    { "SetByteValue", &luaItem::SetByteValue },
    { "GetByteValue", &luaItem::GetByteValue },
    { "GetItemLink", &luaItem::GetItemLink },
    { "GetItemLevel", &luaItem::GetItemLevel },
    { "GetRequiredLevel", &luaItem::GetRequiredLevel },
    { "GetBuyPrice", &luaItem::GetBuyPrice },
    { "GetSellPrice", &luaItem::GetSellPrice },
    { "RepairItem", &luaItem::RepairItem },
    { "GetMaxDurability", &luaItem::GetMaxDurability },
    { "GetDurability", &luaItem::GetDurability },
    { "HasEnchantment", &luaItem::HasEnchantment },
    { "ModifyEnchantmentTime", &luaItem::ModifyEnchantmentTime },
    { "SetStackCount", &luaItem::SetStackCount },
    { "HasFlag", &luaItem::HasFlag },
    { "IsSoulbound", &luaItem::IsSoulbound },
    { "IsAccountbound", &luaItem::IsAccountbound },
    { "IsContainer", &luaItem::IsContainer },
    { "GetContainerItemCount", &luaItem::GetContainerItemCount },
    { "GetEquippedSlot", &luaItem::GetEquippedSlot },
    { "GetObjectType", &luaItem::GetObjectType },
    { "Remove", &luaItem::Remove },
    { "Create", &luaItem::Create },
    { "ModUInt32Value", &luaItem::ModUInt32Value },
    { "ModFloatValue", &luaItem::ModFloatValue },
    { "SetUInt32Value", &luaItem::SetUInt32Value },
    { "SetUInt64Value", &luaItem::SetUInt64Value },
    { "SetFloatValue", &luaItem::SetFloatValue },
    { "GetUInt32Value", &luaItem::GetUInt32Value },
    { "GetUInt64Value", &luaItem::GetUInt64Value },
    { "GetFloatValue", &luaItem::GetFloatValue },
    { "RemoveFlag", &luaItem::RemoveFlag },
    { "SetFlag", &luaItem::SetFlag },
    { nullptr, nullptr },
};

RegType<Unit> UnitMethods[] =
{
    { "GossipCreateMenu", &LuaUnit::GossipCreateMenu },
    { "GossipMenuAddItem", &LuaUnit::GossipMenuAddItem },
    { "GossipSendMenu", &LuaUnit::GossipSendMenu },
    { "GossipComplete", &LuaUnit::GossipComplete },
    { "GossipSendPOI", &LuaUnit::GossipSendPOI },
    { "GossipSendQuickMenu", &LuaUnit::GossipSendQuickMenu },
    { "GossipAddQuests", &LuaUnit::GossipAddQuests },
    { "GetName", &LuaUnit::GetName },
    { "SendChatMessage", &LuaUnit::SendChatMessage },
    { "MoveTo", &LuaUnit::MoveTo },
    { "CastSpell", &LuaUnit::CastSpell },
    { "SpawnCreature", &LuaUnit::SpawnCreature },
    { "SpawnGameObject", &LuaUnit::SpawnGameObject },
    { "GetX", &LuaUnit::GetX },
    { "GetY", &LuaUnit::GetY },
    { "GetZ", &LuaUnit::GetZ },
    { "GetO", &LuaUnit::GetO },
    { "IsPlayer", &LuaUnit::IsPlayer },
    { "IsCreature", &LuaUnit::IsCreature },
    { "RegisterEvent", &LuaUnit::RegisterEvent },
    { "RemoveEvents", &LuaUnit::RemoveEvents },
    { "SendBroadcastMessage", &LuaUnit::SendBroadcastMessage },
    { "SendAreaTriggerMessage", &LuaUnit::SendAreaTriggerMessage },
    { "MarkQuestObjectiveAsComplete", &LuaUnit::MarkQuestObjectiveAsComplete },
    { "LearnSpell", &LuaUnit::LearnSpell },
    { "UnlearnSpell", &LuaUnit::UnlearnSpell },
    { "HasFinishedQuest", &LuaUnit::HasFinishedQuest },
    { "GetItemCount", &LuaUnit::GetItemCount },
    { "IsInCombat", &LuaUnit::IsInCombat },
    { "GetMainTank", &LuaUnit::GetMainTank },
    { "GetAddTank", &LuaUnit::GetAddTank },
    { "ClearThreatList", &LuaUnit::ClearThreatList },
    { "SetTauntedBy", &LuaUnit::SetTauntedBy },
    { "ChangeTarget", &LuaUnit::ChangeTarget },
    { "GetHealthPct", &LuaUnit::getHealthPct },
    { "SetHealthPct", &LuaUnit::SetHealthPct },
    { "GetManaPct", &LuaUnit::GetManaPct },
    { "Despawn", &LuaUnit::Despawn },
    { "GetUnitBySqlId", &LuaUnit::GetUnitBySqlId },
    { "PlaySoundToSet", &LuaUnit::PlaySoundToSet },
    { "RemoveAura", &LuaUnit::RemoveAura },
    { "StopMovement", &LuaUnit::StopMovement },
    { "Emote", &LuaUnit::Emote },
    { "GetInstanceID", &LuaUnit::GetInstanceID },
    { "GetClosestPlayer", &LuaUnit::GetClosestPlayer },
    { "GetRandomPlayer", &LuaUnit::GetRandomPlayer },
    { "GetRandomFriend", &LuaUnit::GetRandomFriend },
    { "GetRandomEnemy", &LuaUnit::GetRandomEnemy },
    { "AddItem", &LuaUnit::AddItem },
    { "RemoveItem", &LuaUnit::RemoveItem },
    { "SetCombatCapable", &LuaUnit::DisableCombat },
    { "SetCombatMeleeCapable", &LuaUnit::DisableMelee },
    { "SetCombatRangedCapable", &LuaUnit::DisableRanged },
    { "SetCombatSpellCapable", &LuaUnit::DisableSpells },
    { "SetCombatTargetingCapable", &LuaUnit::DisableTargeting },
    { "SetNPCFlags", &LuaUnit::SetNPCFlags },
    { "SetModel", &LuaUnit::SetModel },
    { "SetScale", &LuaUnit::SetScale },
    { "SetFaction", &LuaUnit::SetFaction },
    { "SetStandState", &LuaUnit::SetStandState },
    { "Teleport" , &LuaUnit::TeleportUnit },
    { "GetPlayerClass", &LuaUnit::GetPlayerClass },
    { "ClearThreatList", &LuaUnit::ClearHateList },
    { "WipeThreatList", &LuaUnit::WipeHateList },
    { "WipeTargetList", &LuaUnit::WipeTargetList },
    { "WipeCurrentTarget", &LuaUnit::WipeCurrentTarget },
    { "GetHealth", &LuaUnit::GetHealth },
    { "GetMaxHealth", &LuaUnit::GetMaxHealth },
    { "SetHealth", &LuaUnit::SetHealth },
    { "SetMaxHealth", &LuaUnit::SetMaxHealth },
    { "HasAura", &LuaUnit::HasAura },
    { "Land", &LuaUnit::Land },
    { "SetFlying", &LuaUnit::SetFlying },
    { "SetMana", &LuaUnit::SetMana },
    { "SetMaxMana", &LuaUnit::SetMaxMana },
    { "GetDistance", &LuaUnit::GetDistance },
    { "GetGuid", &LuaUnit::GetGUID },
    { "GetCreatureNearestCoords", &LuaUnit::GetCreatureNearestCoords },
    { "CastSpellAoF", &LuaUnit::castSpellLoc },
    { "GetGameObjectNearestCoords", &LuaUnit::GetGameObjectNearestCoords },
    { "SetInFront", &LuaUnit::SetInFront },
    { "RemoveAllAuras", &LuaUnit::RemoveAllAuras },
    { "ReturnToSpawnPoint", &LuaUnit::ReturnToSpawnPoint },
    { "CancelSpell", &LuaUnit::CancelSpell },
    { "IsAlive", &LuaUnit::IsAlive },
    { "IsDead", &LuaUnit::IsDead },
    { "IsInWorld", &LuaUnit::IsInWorld },
    { "GetZoneId", &LuaUnit::GetZoneId },
    { "GetMana", &LuaUnit::GetMana },
    { "GetMaxMana", &LuaUnit::GetMaxMana },
    { "Root", &LuaUnit::Root },
    { "Unroot", &LuaUnit::Unroot },
    { "SetOutOfCombatRange", &LuaUnit::SetOutOfCombatRange },
    { "ModifyRunSpeed", &LuaUnit::ModifyRunSpeed },
    { "ModifyWalkSpeed", &LuaUnit::ModifyWalkSpeed },
    { "ModifyFlySpeed" , &LuaUnit::ModifyFlySpeed },
    { "GetCurrentSpell", &LuaUnit::GetCurrentSpell },
    { "IsFlying", &LuaUnit::isFlying },
    { "SendAIReaction", &LuaUnit::SendAIReaction },
    { "SetOrientation", &LuaUnit::SetOrientation },
    { "GetSpawnX", &LuaUnit::GetSpawnX },
    { "GetSpawnY", &LuaUnit::GetSpawnY },
    { "GetSpawnZ", &LuaUnit::GetSpawnZ },
    { "GetSpawnO", &LuaUnit::GetSpawnO },
    { "GetInRangePlayersCount", &LuaUnit::GetInRangePlayersCount },
    { "GetEntry", &LuaUnit::GetEntry },
    { "GetAIState", &LuaUnit::GetAIState },
    { "ModUInt32Value", &LuaUnit::ModUInt32Value },
    { "ModFloatValue", &LuaUnit::ModFloatValue },
    { "SetUInt32Value", &LuaUnit::SetUInt32Value },
    { "SetUInt64Value", &LuaUnit::SetUInt64Value },
    { "SetFloatValue", &LuaUnit::SetFloatValue },
    { "GetUInt32Value", &LuaUnit::GetUInt32Value },
    { "GetUInt64Value", &LuaUnit::GetUInt64Value },
    { "GetFloatValue", &LuaUnit::GetFloatValue },
    { "SendPacket", &LuaUnit::SendPacket },
    { "AdvanceQuestObjective", &LuaUnit::AdvanceQuestObjective },
    { "Heal", &LuaUnit::Heal },
    { "Energize", &LuaUnit::Energize },
    { "SendChatMessageAlternateEntry", &LuaUnit::SendChatMessageAlternateEntry },
    { "SendChatMessageToPlayer", &LuaUnit::SendChatMessageToPlayer },
    { "SetPowerType", &LuaUnit::SetPowerType },
    { "Strike", &LuaUnit::Strike },
    { "SetAttackTimer", &LuaUnit::SetAttackTimer },
    { "Kill", &LuaUnit::Kill },
    { "DealDamage", &LuaUnit::DealDamage },
    { "IsInFront", &LuaUnit::IsInFront },
    { "IsInBack", &LuaUnit::IsInBack },
    { "IsPacified", &LuaUnit::IsPacified },
    { "IsStunned", &LuaUnit::IsStunned },
    { "IsFeared", &LuaUnit::IsFeared },
    { "CreateGuardian", &LuaUnit::CreateGuardian },
    { "HandleEvent", &LuaUnit::HandleEvent },
    { "HasInRangeObjects", &LuaUnit::HasInRangeObjects },
    { "IsInWater", &LuaUnit::IsInWater },
    { "IsInArc", &LuaUnit::IsInArc },
    { "GetInRangeObjects", &LuaUnit::GetInRangeGameObjects },
    { "GetInRangeObjectsCount", &LuaUnit::GetInRangeObjectsCount },
    { "GetAITargetsCount", &LuaUnit::GetAITargetsCount },
    { "SetUnitToFollow", &LuaUnit::SetUnitToFollow },
    { "DismissPet", &LuaUnit::DismissPet },
    { "IsPet", &LuaUnit::IsPet },
    { "SetPetOwner", &LuaUnit::SetPetOwner },
    { "GetPetOwner", &LuaUnit::GetPetOwner },
    { "CalcToDistance", &LuaUnit::CalcToDistance },
    { "CalcAngle", &LuaUnit::CalcAngle },
    { "CalcRadAngle", &LuaUnit::CalcRadAngle },
    { "SetFacing", &LuaUnit::SetFacing },
    { "SetDeathState", &LuaUnit::SetDeathState },
    { "SetInvisible", &LuaUnit::IsInvisible },
    { "SetInvincible", &LuaUnit::IsInvincible },
    { "ResurrectPlayer", &LuaUnit::ResurrectPlayer },
    { "KickPlayer", &LuaUnit::KickPlayer },
    { "CanCallForHelp", &LuaUnit::CanCallForHelp },
    { "CallForHelpHp", &LuaUnit::CallForHelpHp },
    { "SetCreatureNameById", &LuaUnit::SetCreatureName },
    { "GetAITargets", &LuaUnit::GetAITargets },
    { "GetInRangePlayers", &LuaUnit::GetInRangePlayers },
    { "GetUnitByGUID", &LuaUnit::GetUnitByGUID },
    { "RemoveFromWorld", &LuaUnit::RemoveFromWorld },
    { "GetFaction", &LuaUnit::GetFaction },
    { "EnableMoveFly", &LuaUnit::MoveFly },
    { "SpellNonMeleeDamageLog", &LuaUnit::SpellNonMeleeDamageLog },
    { "DisableRespawn", &LuaUnit::NoRespawn },
    { "ModThreat", &LuaUnit::ModThreat },
    { "GetThreat", &LuaUnit::GetThreatByPtr },
    { "GetInRangeFriends", &LuaUnit::GetInRangeFriends },
    { "GetPowerType", &LuaUnit::GetPowerType },
    { "GetMapId", &LuaUnit::GetMapId },
    { "AttackReaction", &LuaUnit::AttackReaction },
    { "EventCastSpell", &LuaUnit::eventCastSpell },
    { "IsPlayerMoving", &LuaUnit::IsPlayerMoving },
    { "IsPlayerAttacking", &LuaUnit::IsPlayerAttacking },
    { "RemoveThreat", &LuaUnit::RemoveThreatByPtr },
    { "SetPlayerAtWar", &LuaUnit::SetPlayerAtWar },
    { "GetFactionStanding", &LuaUnit::GetFactionStanding },
    { "PlaySpellVisual", &LuaUnit::PlaySpellVisual },
    { "GetPlayerLevel", &LuaUnit::GetLevel },
    { "GetLevel", &LuaUnit::GetLevel },
    { "SetPlayerLevel", &LuaUnit::SetLevel },
    { "SetLevel", &LuaUnit::SetLevel },
    { "SetStanding", &LuaUnit::SetPlayerStanding },
    { "GetStanding", &LuaUnit::GetStanding },
    { "HasItem", &LuaUnit::HasItem },
    { "AdvanceSkill", &LuaUnit::AdvanceSkill },
    { "AddSkill", &LuaUnit::AddSkill },
    { "RemoveSkill", &LuaUnit::RemoveSkill },
    { "EnableFlyCheat", &LuaUnit::FlyCheat },
    { "GetCurrentSpellId", &LuaUnit::GetCurrentSpellId },
    { "GetPlayerRace", &LuaUnit::GetPlayerRace },
    { "RemoveAurasByMechanic", &LuaUnit::RemoveAurasByMechanic },
    { "RemoveAurasType", &LuaUnit::RemoveAurasType },
    { "AddAura", &LuaUnit::AddAura },
    { "SetAIState", &LuaUnit::SetAIState },
    { "InterruptSpell", &LuaUnit::InterruptSpell },
    { "RemoveStealth", &LuaUnit::RemoveStealth },
    { "IsPoisoned", &LuaUnit::IsPoisoned },
    { "SetStealthLevel", &LuaUnit::SetStealth },
    { "GetStealthLevel", &LuaUnit::GetStealthLevel },
    { "IsStealthed", &LuaUnit::IsStealthed },
    { "RemoveFlag", &LuaUnit::RemoveFlag },
    { "ModifyAIUpdateEvent", &LuaUnit::ModifyAIUpdateEvent },
    { "RemoveAIUpdateEvent", &LuaUnit::RemoveAIUpdateEvent },
    { "DealGoldCost", &LuaUnit::DealGoldCost },
    { "DealGoldMerit", &LuaUnit::DealGoldMerit },
    { "CanUseCommand", &LuaUnit::CanUseCommand },
    { "DeMorph", &LuaUnit::DeMorph },
    { "Attack", &LuaUnit::Attack },
    { "GetSelection", &LuaUnit::GetSelection },
    { "SetMount", &LuaUnit::SetMount },
    { "StartQuest", &LuaUnit::StartQuest },
    { "FinishQuest", &LuaUnit::FinishQuest },
    { "RepairAllPlayerItems", &LuaUnit::RepairAllPlayerItems },
    { "SetKnownTitle", &LuaUnit::SetKnownTitle },
    { "LifeTimeKills", &LuaUnit::LifeTimeKills },
    { "HasTitle", &LuaUnit::HasTitle },
    { "GetMaxSkill", &LuaUnit::GetMaxSkill },
    { "GetCurrentSkill", &LuaUnit::GetCurrentSkill },
    { "HasSkill", &LuaUnit::HasSkill },
    { "GetGuildName", &LuaUnit::GetGuildName },
    { "ClearCooldownForSpell", &LuaUnit::ClearCooldownForSpell },
    { "HasSpell", &LuaUnit::HasSpell },
    { "ClearAllCooldowns", &LuaUnit::ClearAllCooldowns },
    { "ResetAllTalents", &LuaUnit::ResetAllTalents },
    { "GetAccountName", &LuaUnit::GetAccountName },
    { "SavePlayer", &LuaUnit::SavePlayer },
    { "HasQuest", &LuaUnit::HasQuest },
    { "RemovePvPFlag", &LuaUnit::RemovePvPFlag },
    { "RemoveNegativeAuras", &LuaUnit::RemoveNegativeAuras },
    { "GossipMiscAction", &LuaUnit::GossipMiscAction },
    { "EquipWeapons", &LuaUnit::EquipWeapons },
    { "Dismount", &LuaUnit::Dismount },
    { "AdvanceAllSkills", &LuaUnit::AdvanceAllSkills },
    { "GetTeam", &LuaUnit::GetTeam },
    { "Possess", &LuaUnit::Possess },
    { "Unpossess", &LuaUnit::Unpossess },
    { "StartTaxi", &LuaUnit::StartTaxi },
    { "ChannelSpell", &LuaUnit::ChannelSpell },
    { "StopChannel", &LuaUnit::StopChannel },
    { "EnableFlight", &LuaUnit::EnableFlight },
    { "GetCoinage", &LuaUnit::GetCoinage },
    { "FlagPvP", &LuaUnit::FlagPvP },
    { "GetDisplay", &LuaUnit::GetDisplay },
    { "GetNativeDisplay", &LuaUnit::GetNativeDisplay },
    { "IsMounted", &LuaUnit::IsMounted },
    { "PlaySoundToPlayer", &LuaUnit::PlaySoundToPlayer },
    { "GetDuelState", &LuaUnit::GetDuelState },
    { "SetPosition", &LuaUnit::SetPosition},
    { "CastSpellOnTarget", &LuaUnit::CastSpellOnTarget},
    { "GetLandHeight", &LuaUnit::GetLandHeight},
    { "QuestAddStarter", &LuaUnit::QuestAddStarter},
    { "QuestAddFinisher", &LuaUnit::QuestAddFinisher},
    { "SetPlayerSpeed", &LuaUnit::SetPlayerSpeed},
    { "GiveHonor", &LuaUnit::GiveHonor},
    { "SetBindPoint", &LuaUnit::SetBindPoint},
    { "SoftDisconnect", &LuaUnit::SoftDisconnect},
    { "SetZoneWeather", &LuaUnit::SetZoneWeather},
    { "SetPlayerWeather", &LuaUnit::SetPlayerWeather},
    { "SendPacketToPlayer", &LuaUnit::SendPacketToPlayer},
    { "PlayerSendChatMessage", &LuaUnit::PlayerSendChatMessage},
    { "GetDistanceYards", &LuaUnit::GetDistanceYards},
    { "VendorAddItem", &LuaUnit::VendorAddItem},
    { "VendorRemoveItem", &LuaUnit::VendorRemoveItem},
    { "VendorRemoveAllItems", &LuaUnit::VendorRemoveAllItems},
    { "CreatureHasQuest", &LuaUnit::CreatureHasQuest},
    { "SendVendorWindow", &LuaUnit::SendVendorWindow},
    { "SendTrainerWindow", &LuaUnit::SendTrainerWindow},
    { "SendInnkeeperWindow", &LuaUnit::SendInnkeeperWindow},
    { "SendBankWindow", &LuaUnit::SendBankWindow},
    { "SendAuctionWindow", &LuaUnit::SendAuctionWindow},
    { "SendBattlegroundWindow", &LuaUnit::SendBattlegroundWindow},
    { "GetInventoryItem", &LuaUnit::GetInventoryItem},
    { "GetInventoryItemById", &LuaUnit::GetInventoryItemById},
    { "PhaseSet", &LuaUnit::PhaseSet},
    { "PhaseAdd", &LuaUnit::PhaseAdd},
    { "PhaseDelete", &LuaUnit::PhaseDelete},
    { "GetPhase", &LuaUnit::GetPhase},
    { "AggroWithInRangeFriends", &LuaUnit::AggroWithInRangeFriends},
    { "MoveRandomArea", &LuaUnit::MoveRandomArea},
    { "SendLootWindow", &LuaUnit::SendLootWindow},
    { "AddLoot", &LuaUnit::AddLoot},
    { "SetPacified", &LuaUnit::SetPacified},
    { "SetPlayerLock", &LuaUnit::SetPlayerLock},
    { "GetGroupPlayers", &LuaUnit::GetGroupPlayers},
    { "IsGm", &LuaUnit::IsGm},
    { "GetDungeonDifficulty", &LuaUnit::GetDungeonDifficulty},
    { "GetGroupLeader", &LuaUnit::GetGroupLeader},
    { "SetGroupLeader", &LuaUnit::SetGroupLeader},
    { "AddGroupMember", &LuaUnit::AddGroupMember},
    { "SetDungeonDifficulty", &LuaUnit::SetDungeonDifficulty},
    { "ExpandToRaid", &LuaUnit::ExpandToRaid},
    { "SendPacketToGroup", &LuaUnit::SendPacketToGroup},
    { "IsGroupFull", &LuaUnit::IsGroupFull},
    { "IsGroupedWith", &LuaUnit::IsGroupedWith},
    { "GetTotalHonor", &LuaUnit::GetTotalHonor},
    { "GetHonorToday", &LuaUnit::GetHonorToday},
    { "GetHonorYesterday", &LuaUnit::GetHonorYesterday},
    { "GetArenaPoints", &LuaUnit::GetArenaPoints},
    { "AddArenaPoints", &LuaUnit::AddArenaPoints},
    { "AddLifetimeKills", &LuaUnit::AddLifetimeKills},
    { "GetGender", &LuaUnit::GetGender},
    { "SetGender", &LuaUnit::SetGender},
    { "GetGroupType", &LuaUnit::GetGroupType},
    { "SendPacketToGuild", &LuaUnit::SendPacketToGuild },
    { "GetGuildId", &LuaUnit::GetGuildId },
    { "GetGuildRank", &LuaUnit::GetGuildRank },
    { "SetGuildRank", &LuaUnit::SetGuildRank },
    { "IsInGuild", &LuaUnit::IsInGuild },
    { "SendGuildInvite", &LuaUnit::SendGuildInvite },
    { "DemoteGuildMember", &LuaUnit::DemoteGuildMember },
    { "PromoteGuildMember", &LuaUnit::PromoteGuildMember },
    { "SetGuildMotd", &LuaUnit::SetGuildMotd },
    { "GetGuildMotd", &LuaUnit::GetGuildMotd },
    { "SetGuildInformation", &LuaUnit::SetGuildInformation },
    { "AddGuildMember", &LuaUnit::AddGuildMember },
    { "RemoveGuildMember", &LuaUnit::RemoveGuildMember },
    { "SetPublicNote", &LuaUnit::SetPublicNote },
    { "SetOfficerNote", &LuaUnit::SetOfficerNote },
    { "DisbandGuild", &LuaUnit::DisbandGuild },
    { "ChangeGuildMaster", &LuaUnit::ChangeGuildMaster },
    { "SendGuildChatMessage", &LuaUnit::SendGuildChatMessage },
    { "SendGuildLog", &LuaUnit::SendGuildLog },
    { "GuildBankDepositMoney", &LuaUnit::GuildBankDepositMoney },
    { "GuildBankWithdrawMoney", &LuaUnit::GuildBankWithdrawMoney },
    { "GetGmRank", &LuaUnit::GetGmRank },
    { "SetByteValue", &LuaUnit::SetByteValue },
    { "GetByteValue", &LuaUnit::GetByteValue },
    { "IsPvPFlagged", &LuaUnit::IsPvPFlagged },
    { "IsFFAPvPFlagged", &LuaUnit::IsFFAPvPFlagged },
    { "IsFFAFlagged", &LuaUnit::IsFFAPvPFlagged },
    { "GetGuildLeader", &LuaUnit::GetGuildLeader },
    { "GetGuildMemberCount", &LuaUnit::GetGuildMemberCount },
    { "CanAttack", &LuaUnit::CanAttack },
    { "GetInRangeUnits", &LuaUnit::GetInRangeUnits },
    { "GetInRangeEnemies", &LuaUnit::GetInRangeEnemies },
    { "IsFriendly", &LuaUnit::IsFriendly },
    { "MovePlayerTo", &LuaUnit::MovePlayerTo },
    { "IsInChannel", &LuaUnit::IsInChannel },
    { "JoinChannel", &LuaUnit::JoinChannel },
    { "LeaveChannel", &LuaUnit::LeaveChannel },
    { "SetChannelName", &LuaUnit::SetChannelName },
    { "SetChannelPassword", &LuaUnit::SetChannelPassword },
    { "GetChannelPassword", &LuaUnit::GetChannelPassword },
    { "KickFromChannel", &LuaUnit::KickFromChannel },
    { "BanFromChannel", &LuaUnit::BanFromChannel },
    { "UnbanFromChannel", &LuaUnit::UnbanFromChannel },
    { "GetChannelMemberCount", &LuaUnit::GetChannelMemberCount },
    { "GetPlayerMovementVector", &LuaUnit::GetPlayerMovementVector},
    { "UnsetKnownTitle", &LuaUnit::UnsetKnownTitle},
    { "IsInPhase", &LuaUnit::IsInPhase},
    { "HasFlag", &LuaUnit::HasFlag },
    { "Repop", &LuaUnit::Repop },
    { "SetMovementFlags", &LuaUnit::SetMovementFlags },
    { "GetSpawnId", &LuaUnit::GetSpawnId },
    { "ResetTalents", &LuaUnit::ResetTalents },
    { "SetTalentPoints", &LuaUnit::SetTalentPoints },
    { "GetTalentPoints", &LuaUnit::GetTalentPoints },
    { "EventChat", &LuaUnit::EventChat },
    { "GetEquippedItemBySlot", &LuaUnit::GetEquippedItemBySlot },
    { "GetGuildMembers", &LuaUnit::GetGuildMembers },
    { "AddAchievement", &LuaUnit::AddAchievement },
    { "RemoveAchievement", &LuaUnit::RemoveAchievement },
    { "HasAchievement", &LuaUnit::HasAchievement },
    { "RemoveArenaPoints", &LuaUnit::RemoveArenaPoints},
    { "TakeHonor", &LuaUnit::TakeHonor},
    { "SetPhase", &LuaUnit::PhaseSet},
    { "DeletePhase", &LuaUnit::PhaseDelete},
    { "AddToPhase", &LuaUnit::PhaseAdd},
    { "GetAreaId", &LuaUnit::GetAreaId},
    { "ResetPetTalents", &LuaUnit::ResetPetTalents},
    { "IsDazed", &LuaUnit::IsDazed },
    { "IsRooted", &LuaUnit::IsRooted },
    { "HasAuraWithMechanic", &LuaUnit::HasAuraWithMechanic },
    { "HasNegativeAura", &LuaUnit::HasNegativeAura },
    { "HasPositiveAura", &LuaUnit::HasPositiveAura },
    { "GetClosestEnemy", &LuaUnit::GetClosestEnemy },
    { "GetClosestFriend", &LuaUnit::GetClosestFriend },
    { "IsOnTaxi", &LuaUnit::IsOnTaxi },
    { "GetTaxi", &LuaUnit::GetTaxi },
    { "GetObjectType", &LuaUnit::GetObjectType },
    { "GiveXp", &LuaUnit::GiveXp },
    { "GetPower", &LuaUnit::GetPower },
    { "GetPowerPct", &LuaUnit::GetPowerPct },
    { "GetMaxPower", &LuaUnit::GetMaxPower },
    { "SetPower", &LuaUnit::SetPower },
    { "SetPowerPct", &LuaUnit::SetPowerPct },
    { "SetMaxPower", &LuaUnit::SetMaxPower },
    { "LearnSpells", &LuaUnit::LearnSpells },
    { "GetSelectedGO", &LuaUnit::GetSelectedGO },
    { "FullCastSpell", &LuaUnit::FullCastSpell },
    { "FullCastSpellOnTarget", &LuaUnit::FullCastSpellOnTarget },
    { "DisableMelee", &LuaUnit::DisableMelee },
    { "DisableRanged", &LuaUnit::DisableRanged },
    { "DisableSpells", &LuaUnit::DisableSpells },
    { "DisableCombat", &LuaUnit::DisableCombat },
    { "DisableTargeting", &LuaUnit::DisableTargeting },
    { "IsInGroup", &LuaUnit::IsInGroup },
    { "GetLocation", &LuaUnit::GetLocation },
    { "GetSpawnLocation", &LuaUnit::GetSpawnLocation },
    { "GetPlayerMovementFlags", &LuaUnit::GetPlayerMovementFlags},
    { "GetObject", &LuaUnit::GetObject },
    { "GetSecondHated", &LuaUnit::GetSecondHated },
    { "UseAI", &LuaUnit::UseAI },
    { "FlagFFA", &LuaUnit::FlagFFA },
    { "TeleportCreature", &LuaUnit::TeleportCreature },
    { "IsInDungeon", &LuaUnit::IsInDungeon },
    { "IsInRaid", &LuaUnit::IsInRaid },
    { "CreateLuaEvent", &LuaUnit::CreateLuaEvent },
    { "IsHostile", &LuaUnit::IsHostile },
    { "IsAttackable", &LuaUnit::IsAttackable },
    { "GetQuestLogSlot", &LuaUnit::GetQuestLogSlot },
    { "GetAuraStackCount", &LuaUnit::GetAuraStackCount },
    { "AddAuraObject", &LuaUnit::AddAuraObject },
    { "GetAuraObjectById", &LuaUnit::GetAuraObjectById },
    { "GetNativeFaction", &LuaUnit::GetNativeFaction },
    { "StopPlayerAttack", &LuaUnit::StopPlayerAttack },
    { "GetQuestObjectiveCompletion", &LuaUnit::GetQuestObjectiveCompletion },
    { "FullCastSpellAoF", &LuaUnit::FullCastSpellAoF },
    { "GetClosestUnit", &LuaUnit::GetClosestUnit },
    { "FullCastSpellAoE", &LuaUnit::FullCastSpellAoF },
    { "CastSpellAoE", &LuaUnit::castSpellLoc },
    { "SetFlag", &LuaUnit::SetFlag },
    { "SetSelectedGO", &LuaUnit::SetSelectedGO },
    { "IsOnVehicle", &LuaUnit::IsOnVehicle },
    { "SpawnAndEnterVehicle", &LuaUnit::SpawnAndEnterVehicle },
    { "DismissVehicle", &LuaUnit::DismissVehicle },
    { "AddVehiclePassenger", &LuaUnit::AddVehiclePassenger },
    { "HasEmptyVehicleSeat", &LuaUnit::HasEmptyVehicleSeat },
    { "EnterVehicle", &LuaUnit::EnterVehicle },
    { "ExitVehicle", &LuaUnit::ExitVehicle },
    { "GetVehicleBase", &LuaUnit::GetVehicleBase },
    { "EjectAllVehiclePassengers", &LuaUnit::EjectAllVehiclePassengers },
    { "EjectVehiclePassengerFromSeat", &LuaUnit::EjectVehiclePassengerFromSeat },
    { "MoveVehiclePassengerToSeat", &LuaUnit::MoveVehiclePassengerToSeat },
    { "GetWorldStateForZone", &LuaUnit::GetWorldStateForZone },
    { "SetWorldStateForZone", &LuaUnit::SetWorldStateForZone },
    { "SetWorldStateForPlayer", &LuaUnit::SetWorldStateForPlayer },
    { nullptr, nullptr },
};

RegType<GameObject> GOMethods[] =
{
    { "GetGuid", &LuaGameObject::GetGUID },
    { "GetName", &LuaGameObject::GetName },
    { "GetCreatureNearestCoords", &LuaGameObject::GetCreatureNearestCoords },
    { "GetAreaId", &LuaGameObject::GetAreaId },
    { "GetGameObjectNearestCoords", &LuaGameObject::GetGameObjectNearestCoords },
    { "GetZoneId", &LuaGameObject::GetZoneId },
    { "GetClosestPlayer", &LuaGameObject::GetClosestPlayer },
    { "SpawnCreature", &LuaGameObject::SpawnCreature },
    { "SpawnGameObject", &LuaGameObject::SpawnGameObject },
    { "IsInWorld", &LuaGameObject::IsInWorld },
    { "GetSpawnX", &LuaGameObject::GetSpawnX },
    { "GetSpawnY", &LuaGameObject::GetSpawnY },
    { "GetSpawnZ", &LuaGameObject::GetSpawnZ },
    { "GetSpawnO", &LuaGameObject::GetSpawnO },
    { "GetInRangePlayersCount", &LuaGameObject::GetInRangePlayersCount },
    { "GetEntry", &LuaGameObject::GetEntry },
    { "SetOrientation", &LuaGameObject::SetOrientation },
    { "GetX", &LuaGameObject::GetX },
    { "GetY", &LuaGameObject::GetY },
    { "GetZ", &LuaGameObject::GetZ },
    { "GetO", &LuaGameObject::GetO },
    { "RemoveFromWorld", &LuaGameObject::RemoveFromWorld },
    { "CalcRadAngle", &LuaGameObject::CalcRadAngle },
    { "GetInstanceID", &LuaGameObject::GetInstanceID },
    { "GetInRangePlayers", &LuaGameObject::GetInRangePlayers },
    { "GetInRangeObjects", &LuaGameObject::GetInRangeGameObjects },
    { "IsInBack", &LuaGameObject::IsInBack },
    { "IsInFront", &LuaGameObject::IsInFront },
    { "GetMapId", &LuaGameObject::GetMapId },
    { "SetUInt32Value", &LuaGameObject::SetUInt32Value },
    { "SetUInt64Value", &LuaGameObject::SetUInt64Value },
    { "SetFloatValue", &LuaGameObject::SetFloatValue },
    { "GetUInt32Value", &LuaGameObject::GetUInt32Value },
    { "GetUInt64Value", &LuaGameObject::GetUInt64Value },
    { "GetFloatValue", &LuaGameObject::GetFloatValue },
    { "ModUInt32Value", &LuaGameObject::ModUInt32Value },
    { "CastSpell", &LuaGameObject::CastSpell },
    { "CastSpellOnTarget", &LuaGameObject::CastSpellOnTarget },
    { "GossipObjectCreateMenu", &LuaGameObject::GossipCreateMenu },
    { "GossipObjectMenuAddItem", &LuaGameObject::GossipMenuAddItem },
    { "GossipObjectSendMenu", &LuaGameObject::GossipSendMenu },
    { "GossipObjectComplete", &LuaGameObject::GossipComplete },
    { "GossipObjectSendPOI", &LuaGameObject::GossipSendPOI },
    { "GossipCreateMenu", &LuaGameObject::GossipCreateMenu },
    { "GossipMenuAddItem", &LuaGameObject::GossipMenuAddItem },
    { "GossipSendMenu", &LuaGameObject::GossipSendMenu },
    { "GossipComplete", &LuaGameObject::GossipComplete },
    { "GossipSendPOI", &LuaGameObject::GossipSendPOI },
    { "GossipSendQuickMenu", &LuaGameObject::GossipSendQuickMenu },
    { "RegisterAIUpdateEvent", &LuaGameObject::RegisterAIUpdate },
    { "ModifyAIUpdateEvent", &LuaGameObject::ModAIUpdate },
    { "RemoveAIUpdateEvent", &LuaGameObject::RemoveAIUpdate },
    { "Activate", &LuaGameObject::Activate },
    { "IsActive", &LuaGameObject::IsActive },
    { "Despawn", &LuaGameObject::DespawnObject },
    { "GetLandHeight", &LuaGameObject::GetLandHeight},
    { "SetZoneWeather", &LuaGameObject::SetZoneWeather},
    { "PhaseSet", &LuaGameObject::PhaseSet},
    { "PhaseAdd", &LuaGameObject::PhaseAdd},
    { "PhaseDelete", &LuaGameObject::PhaseDelete},
    { "GetPhase", &LuaGameObject::GetPhase},
    { "SendPacket", LuaGameObject::SendPacket },
    { "AddLoot", &LuaGameObject::AddLoot},
    { "Update", &LuaGameObject::Update}, //sadikum
    { "GetDungeonDifficulty", &LuaGameObject::GetDungeonDifficulty },
    { "SetDungeonDifficulty", &LuaGameObject::SetDungeonDifficulty },
    { "HasFlag", &LuaGameObject::HasFlag },
    { "IsInPhase", &LuaGameObject::IsInPhase},
    { "SetPhase", &LuaGameObject::PhaseSet},
    { "DeletePhase", &LuaGameObject::PhaseDelete},
    { "AddToPhase", &LuaGameObject::PhaseAdd},
    { "GetAreaId", &LuaGameObject::GetAreaId},
    { "SetPosition", &LuaGameObject::SetPosition},
    { "GetObjectType", &LuaGameObject::GetObjectType},
    { "PlaySoundToSet", &LuaGameObject::PlaySoundToSet },
    { "GetDistance", &LuaGameObject::GetDistance },
    { "GetDistanceYards", &LuaGameObject::GetDistanceYards },
    { "GetSpawnId", &LuaGameObject::GetSpawnId },
    { "ChangeScale", &LuaGameObject::ChangeScale },
    { "GetByte", &LuaGameObject::GetByte },
    { "GetByteValue", &LuaGameObject::GetByte },
    { "SetByte", &LuaGameObject::SetByte },
    { "SetByteValue", &LuaGameObject::SetByte },
    { "FullCastSpell", &LuaGameObject::FullCastSpell },
    { "FullCastSpellOnTarget", &LuaGameObject::FullCastSpellOnTarget },
    { "CustomAnimate", &LuaGameObject::CustomAnimate },
    { "GetLocation", &LuaGameObject::GetLocation },
    { "GetSpawnLocation", &LuaGameObject::GetSpawnLocation },
    { "GetObject", &LuaGameObject::GetWoWObject },
    { "GetClosestPlayer", &LuaGameObject::GetClosestPlayer },
    { "CreateLuaEvent", &LuaGameObject::RegisterEvent },
    { "RemoveEvents", &LuaGameObject::RemoveEvents },
    { "SetScale", &LuaGameObject::SetScale },
    { "GetScale", &LuaGameObject::GetScale },
    { "GetInRangeUnits", &LuaGameObject::GetInRangeUnits },
    { "GetClosestUnit", &LuaGameObject::GetClosestUnit },
    { "SetFlag", &LuaGameObject::SetFlag },
    { "RemoveFlag", &LuaGameObject::RemoveFlag },
    { "Damage", &LuaGameObject::Damage },
    { "Rebuild", &LuaGameObject::Rebuild },
    { "GetHP", &LuaGameObject::GetHP },
    { "GetMaxHP", &LuaGameObject::GetMaxHP },
    { "GetWorldStateForZone", &LuaGameObject::GetWorldStateForZone },
    { "SetWorldStateForZone", &LuaGameObject::SetWorldStateForZone },
    { nullptr, nullptr },
};

RegType<WorldPacket> LuaPacketMethods[] =
{
    {"CreatePacket", &luPacket::CreatePacket },
    { "GetOpcode", &luPacket::GetOpcode },
    { "GetSize", &luPacket::GetSize },

    //////////////////////////////////////////////////////////////////////////////////////////
    // Read operations
    {"ReadByte", &luPacket::ReadByte },
    {"ReadUByte", &luPacket::ReadUByte },
    {"ReadShort", &luPacket::ReadShort },
    {"ReadUShort", &luPacket::ReadUShort },
    {"ReadLong", &luPacket::ReadLong },
    {"ReadULong", &luPacket::ReadULong },
    {"ReadFloat", &luPacket::ReadFloat },
    {"ReadDouble", &luPacket::ReadDouble },
    {"ReadGUID", &luPacket::ReadGUID },
    {"ReadWoWGuid", &luPacket::ReadWoWGuid },
    {"ReadString", &luPacket::ReadString },

    //////////////////////////////////////////////////////////////////////////////////////////
    // Write operations
    {"WriteUByte", &luPacket::WriteUByte },
    {"WriteByte", &luPacket::WriteByte },
    {"WriteShort", &luPacket::WriteShort },
    {"WriteUShort", &luPacket::WriteUShort },
    {"WriteLong", &luPacket::WriteLong },
    {"WriteULong", &luPacket::WriteULong },
    {"WriteFloat", &luPacket::WriteFloat },
    {"WriteDouble", &luPacket::WriteDouble },
    {"WriteGUID", &luPacket::WriteGUID },
    {"WriteWoWGuid", &luPacket::WriteWoWGuid },
    {"WriteString", &luPacket::WriteString },
    {"GetObjectType", &luPacket::GetObjectType},
    {nullptr, nullptr},
};

RegType<TaxiPath> LuaTaxiMethods[] =
{
    { "CreateTaxi", &LuaTaxi::CreateTaxi },
    { "GetNodeCount", &LuaTaxi::GetNodeCount },
    { "AddPathNode", &LuaTaxi::AddPathNode },
    //{ "GetNodeX", &LuaTaxi::GetNodeX },
    //{ "GetNodeY", &LuaTaxi::GetNodeY },
    //{ "GetNodeZ", &LuaTaxi::GetNodeZ },
    //{ "GetNodeMapId", &LuaTaxi::GetNodeMapId },
    { "GetId", &LuaTaxi::GetId },
    { "GetObjectType", &LuaTaxi::GetObjectType},
    {nullptr, nullptr},
};

RegType<Spell> SpellMethods[] =
{
    { "GetCaster", &LuaSpell::GetCaster },
    { "GetEntry", &LuaSpell::GetEntry },
    { "IsDuelSpell", &LuaSpell::IsDuelSpell },
    { "GetSpellType", &LuaSpell::GetSpellType },
    { "GetSpellState", &LuaSpell::GetSpellState },
    { "Cancel", &LuaSpell::Cancel },
    { "Cast", &LuaSpell::Cast },
    { "CanCast", &LuaSpell::CanCast },
    { "Finish", &LuaSpell::Finish },
    { "GetTarget", &LuaSpell::GetTarget },
    { "IsStealthSpell", &LuaSpell::IsStealthSpell },
    { "IsInvisibilitySpell", &LuaSpell::IsInvisibilitySpell },
    { "GetPossibleEnemy", &LuaSpell::GetPossibleEnemy },
    { "GetPossibleFriend", &LuaSpell::GetPossibleFriend },
    { "HasPower", &LuaSpell::HasPower },
    { "IsAspect", &LuaSpell::IsAspect },
    { "IsSeal", &LuaSpell::IsSeal },
    { "GetObjectType", &LuaSpell::GetObjectType},
    { "SetVar", &LuaSpell::SetVar},
    { "GetVar", &LuaSpell::GetVar},
    { "ResetVar", &LuaSpell::ResetVar},
    { "ResetAllVars", &LuaSpell::ResetAllVars},
    { "GetCastedItemId", &LuaSpell::GetCastedItemId},
    {nullptr, nullptr},
};
RegType<QueryResult> QResultMethods[] =
{
    {"GetColumn", &luaSql::GetColumn },
    {"NextRow", &luaSql::NextRow },
    {"GetColumnCount", &luaSql::GetColumnCount },
    {"GetRowCount", &luaSql::GetRowCount },
    { nullptr, nullptr},
};
RegType<Field> SQLFieldMethods[] =
{
    {"GetByte", &luaSql::GetByte },
    {"GetUByte", &luaSql::GetUByte },
    {"GetShort", &luaSql::GetShort },
    {"GetUShort", &luaSql::GetUShort},
    {"GetLong", &luaSql::GetLong },
    {"GetULong", &luaSql::GetULong },
    {"GetString", &luaSql::GetString },
    {"GetGuid", &luaSql::GetGUID },
    {"GetFloat", &luaSql::GetFloat },
    {"GetBool", &luaSql::GetBool },
    {nullptr, nullptr},
};

RegType<Aura> AuraMethods[] =
{
    {"GetObjectType", &LuaAura::GetObjectType},
    {"GetSpellId", &LuaAura::GetSpellId},
    {"GetCaster", &LuaAura::GetCaster},
    {"GetTarget", &LuaAura::GetTarget},
    {"GetDuration", &LuaAura::GetDuration},
    {"SetDuration", &LuaAura::SetDuration},
    {"GetTimeLeft", &LuaAura::GetTimeLeft},
    {"Remove", &LuaAura::Remove},
    {"SetVar", &LuaAura::SetVar},
    {"GetVar", &LuaAura::GetVar},
    {"GetAuraSlot", &LuaAura::GetAuraSlot},
    {"SetAuraSlot", &LuaAura::SetAuraSlot},
    {nullptr, nullptr},
};

template<typename T> RegType<T>* GetMethodTable() { return nullptr; }
template<> RegType<Unit>* GetMethodTable<Unit>() { return UnitMethods; }
template<> RegType<Item>* GetMethodTable<Item>() { return ItemMethods; }
template<> RegType<GameObject>* GetMethodTable<GameObject>() { return GOMethods; }
template<> RegType<WorldPacket>* GetMethodTable<WorldPacket>() { return LuaPacketMethods; }
template<> RegType<TaxiPath>* GetMethodTable<TaxiPath>() { return LuaTaxiMethods; }
template<> RegType<Spell>* GetMethodTable<Spell>() { return SpellMethods; }
template<> RegType<QueryResult>* GetMethodTable<QueryResult>() { return QResultMethods; }
template<> RegType<Field> * GetMethodTable<Field>() { return SQLFieldMethods; }
template<> RegType<Aura> * GetMethodTable<Aura>() { return AuraMethods; }
