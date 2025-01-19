/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum ServerString
{
    // 0                                                                                        // "Are you sure you wish to purchase a Dual Talent Specialization."
    // 1                                                                                        // "I wish to learn Dual Specialization."
    SS_INSTANCE_UNAVAILABLE = 26,                                                               // "This instance is unavailable."
    SS_MUST_HAVE_BC = 27,                                                                       // "You must have The Burning Crusade Expansion to access this content."
    // 28                                                                                       // "Heroic mode unavailable for this instance."
    // 29                                                                                       // "You must be in a raid group to pass through here."
    // 30                                                                                       // "You do not have the required attunement to pass through here."
    SS_MUST_BE_LEVEL_X = 31,                                                                    // "You must be at least level %u to pass through here."
    // 32                                                                                       // "You must be in a party to pass through here."
    // 33                                                                                       // "You must be level 70 to enter heroic mode."
    // 34                                                                                       // "-"
    SS_MUST_HAVE_ITEM = 35,                                                                     // "You must have the item, `%s` to pass through here."
    // 35                                                                                       // "You must have the item, `%s` to pass through here."
    // 36                                                                                       // "You must have the item, UNKNOWN to pass through here."
    SS_WHAT_CAN_I_TEACH_YOU = 37,                                                               // "What can I teach you, $N?"
    SS_BATTLE_BEGIN_ONE_MINUTE = 46,                                                            // "One minute until the battle for %s begins!"
    SS_THIRTY_SECONDS_UNTIL_THE_BATTLE = 47,                                                    // "Thirty seconds until the battle for %s begins!"
    SS_FIFTEEN_SECONDS_UNTIL_THE_BATTLE = 48,                                                   // "Fifteen seconds until the battle for %s begins!"
    SS_THE_BATTLE_FOR_HAS_BEGUN = 49,                                                           // "The battle for %s has begun!"
    // 50                                                                                       // "You must have the item, UNKNOWN to pass through here."
    SS_JOIN_INVALID_INSTANCE = 51,                                                              // "You have tried to join an invalid instance id."
    SS_QUEUE_BG_INSTANCE_ID_NO_VALID_DELETED = 52,                                              // "Your queue on battleground instance id %u is no longer valid. Reason: Instance Deleted."
    SS_YOU_CANNOT_JOIN_BG_AS_IT_HAS_ALREADY_ENDED = 53,                                         // "You cannot join this battleground as it has already ended."
    SS_QUEUE_BG_INSTANCE_ID_NO_VALID_LONGER_EXISTS = 54,                                        // "Your queue on battleground instance %u is no longer valid, the instance no longer exists."
    SS_SORRY_RAID_GROUPS_JOINING_BG_ARE_UNSUPPORTED = 55,                                       // "Sorry, raid groups joining battlegrounds are currently unsupported."
    SS_MUST_BE_PARTY_LEADER_TO_ADD_GROUP_AN_ARENA = 56,                                         // "You must be the party leader to add a group to an arena."
    // 57,                                                                                      // "You must be in a team to join rated arena."
    SS_TOO_MANY_PLAYERS_PARTY_TO_JOIN_OF_ARENA = 58,                                            // "You have too many players in your party to join this type of arena."
    SS_SORRY_SOME_OF_PARTY_MEMBERS_ARE_NOT_LVL_70 = 59,                                         // "Sorry, some of your party members are not level 70."
    SS_ONE_OR_MORE_OF_PARTY_MEMBERS_ARE_ALREADY_QUEUED_OR_INSIDE_BG = 60,                       // "One or more of your party members are already queued or inside a battleground."
    SS_ONE_OR_MORE_OF_YOUR_PARTY_MEMBERS_ARE_NOT_MEMBERS_OF_YOUR_TEAM = 61,                     // "One or more of your party members are not members of your team."
    SS_INSTANCE_WELCOME = 62,                                                                   // "Welcome to"
    SS_INSTANCE_RESET_INF = 66,                                                                 // "This instance is scheduled to reset on"
    // 67                                                                                       // "Auto loot passing is now %s"
    // 68                                                                                       // "On"
    // 69                                                                                       // "Off"
    SS_HEY_HOW_CAN_I_HELP_YOU = 70,                                                             // Hey there, $N. How can I help you?
    SS_ALREADY_ARENA_TEAM = 71,                                                                 // "You are already in an arena team."
    SS_PETITION_NAME_ALREADY_USED = 72,                                                         // "That name is already in use."
    SS_ALREADY_ARENA_CHARTER = 73,                                                              // "You already have an arena charter."
    SS_GUILD_NAME_ALREADY_IN_USE = 74,                                                          // "A guild with that name already exists."
    SS_ALREADY_GUILD_CHARTER = 75,                                                              // "You already have a guild charter."
    SS_ITEM_NOT_FOUND = 76,                                                                     // "Item not found."
    SS_TARGET_WRONG_FACTION = 77,                                                               // "Target is of the wrong faction."
    SS_CANNOT_SIGN_MORE_REASONS = 78,                                                           // "Target player cannot sign your charter for one or more reasons."
    SS_ALREADY_SIGNED_CHARTER = 79,                                                             // "You have already signed that charter."
                                                                                                // "You don't have the required amount of signatures to turn in this petition."
    SS_MUST_HAVE_QUEST = 81,                                                                    // "You must have the quest, '%s' completed to pass through here."
    // SHATT_ZEPH_KOT = 397,                                                                    // "You need to be Revered with the faction Keepers of Time!"

    // New strings beginns with 500!
    SS_MUST_HAVE_WOTLK = 500,                                                                   // "You must have Wrath of the Lich King Expansion to access this content."
    SS_NOT_ALLOWED_TO_PLAY = 501,                                                               // "This character is not allowed to play."
    SS_BANNED_FOR_TIME = 502,                                                                   // "You have been banned for: %s"
    SS_BG_REMOVE_QUEUE_INF = 503,                                                               // "You were removed from the queue for the battleground for not joining after 1 minute 20 seconds."
};
