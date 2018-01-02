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

#ifndef WORLD_STRINGS_H
#define WORLD_STRINGS_H

enum ServerString
{
    SS_INSTANCE_UNAVAILABLE = 26,       // "This instance is unavailable."
    SS_MUST_HAVE_BC,                    // "You must have The Burning Crusade Expansion to access this content."
                                        // "Heroic mode unavailable for this instance."
                                        // "You must be in a raid group to pass through here."
                                        // "You do not have the required attunement to pass through here."
                                        // "You must be at least level %u to pass through here."
                                        // "You must be in a party to pass through here."
                                        // "You must be level 70 to enter heroic mode."
                                        // "-"
    SS_MUST_HAVE_ITEM = 35,             // "You must have the item, `%s` to pass through here."
    SS_BATTLE_BEGIN_ONE_MINUTE = 46,    // "One minute until the battle for %s begins!"
                                        // "Thirty seconds until the battle for %s begins!"
                                        // "Fifteen seconds until the battle for %s begins!"
                                        // "The battle for %s has begun!"
                                        // "You must have the item, UNKNOWN to pass through here."
    SS_JOIN_INVALID_INSTANCE = 51,      // "You have tried to join an invalid instance id."
                                        // "Your queue on battleground instance id %u is no longer valid. Reason: Instance Deleted."
                                        // "You cannot join this battleground as it has already ended."
                                        // "Your queue on battleground instance %u is no longer valid, the instance no longer exists."
                                        // "Sorry, raid groups joining battlegrounds are currently unsupported."
                                        // "You must be the party leader to add a group to an arena."
                                        // "You must be in a team to join rated arena."
                                        // "You have too many players in your party to join this type of arena."
                                        // "Sorry, some of your party members are not level 70."
                                        // "One or more of your party members are already queued or inside a battleground."
                                        // "One or more of your party members are not members of your team."
    SS_INSTANCE_WELCOME = 62,           // "Welcome to"
    SS_INSTANCE_RESET_INF = 66,         // "This instance is scheduled to reset on"
                                        // "Auto loot passing is now %s"
                                        // "On"
                                        // "Off"
    SS_ALREADY_ARENA_TEAM = 71,         // "You are already in an arena team."
                                        // "That name is already in use."
                                        // "You already have an arena charter."
                                        // "A guild with that name already exists."
                                        // "You already have a guild charter."
                                        // "Item not found."
                                        // "Target is of the wrong faction."
                                        // "Target player cannot sign your charter for one or more reasons."
                                        // "You have already signed that charter."
                                        // "You don't have the required amount of signatures to turn in this petition."
    SS_MUST_HAVE_QUEST = 81,            // "You must have the quest, '%s' completed to pass through here."
    SHATT_ZEPH_KOT = 397,               // "You need to be Revered with the faction Keepers of Time!"

    // New strings beginns with 500!
    SS_MUST_HAVE_WOTLK = 500,           // "You must have Wrath of the Lich King Expansion to access this content."
    SS_NOT_ALLOWED_TO_PLAY,             // "This character is not allowed to play."
    SS_BANNED_FOR_TIME,                 // "You have been banned for: %s"


    SS_BG_REMOVE_QUEUE_INF,             // "You were removed from the queue for the battleground for not joining after 1 minute 20 seconds."

};


#endif      //WORLD_STRINGS_H
