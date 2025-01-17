/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// Pre-made TargetTypes
#define Target_Self TargetType()
#define Target_Current TargetType(TargetGen_Current)
#define Target_SecondMostHated TargetType(TargetGen_SecondMostHated)
#define Target_Destination TargetType(TargetGen_Destination)
#define Target_Predefined TargetType(TargetGen_Predefined)
#define Target_RandomPlayer TargetType(TargetGen_RandomPlayer)
#define Target_RandomPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_NotCurrent)
#define Target_RandomPlayerDestination TargetType(TargetGen_RandomPlayerDestination)
#define Target_RandomPlayerApplyAura TargetType(TargetGen_RandomPlayerApplyAura)
#define Target_RandomUnit TargetType(TargetGen_RandomUnit)
#define Target_RandomUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_NotCurrent)
#define Target_RandomDestination TargetType(TargetGen_RandomUnitDestination)
#define Target_RandomUnitApplyAura TargetType(TargetGen_RandomUnitApplyAura)
#define Target_RandomFriendly TargetType(TargetGen_RandomUnit, TargetFilter_Friendly)
#define Target_WoundedPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Wounded)
#define Target_WoundedUnit TargetType(TargetGen_RandomUnit, TargetFilter_Wounded)
#define Target_WoundedFriendly TargetType(TargetGen_RandomUnit, TargetFilter_WoundedFriendly)
#define Target_ClosestPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Closest)
#define Target_ClosestPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_ClosestNotCurrent)
#define Target_ClosestUnit TargetType(TargetGen_RandomUnit, TargetFilter_Closest)
#define Target_ClosestUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_ClosestNotCurrent)
#define Target_ClosestFriendly TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendly)
#define Target_ClosestCorpse TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendlyCorpse)
#define Target_RandomCorpse TargetType(TargetGen_RandomUnit, TargetFilter_FriendlyCorpse)
