/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <string>

struct GossipMenuItem
{
    uint32 Id;
    uint32 IntId;
    uint8 Icon;
    uint8 Extra;
    std::string Text;
    uint32 m_gSender;
    uint32 m_gAction;
    std::string m_gBoxMessage;
    uint32 m_gBoxMoney;
};

enum GossipIcons
{
    GOSSIP_ICON_CHAT = 0,              // chat bubble
    GOSSIP_ICON_VENDOR = 1,            // vendor
    GOSSIP_ICON_FLIGHTMASTER = 2,      // flightmaster
    GOSSIP_ICON_TRAINER = 3,           // book
    GOSSIP_ICON_INTERACT_1 = 4,        // interaction wheel
    GOSSIP_ICON_INTERACT_2 = 5,        // interaction wheel
    GOSSIP_ICON_MONEY_BAG = 6,         // brown bag with yellow dot
    GOSSIP_ICON_TALK = 7,              // white chat bubble with black dots
    GOSSIP_ICON_TABARD = 8,            // tabard
    GOSSIP_ICON_BATTLE = 9,            // two swords
    GOSSIP_ICON_DOT = 10,              // yellow dot
    GOSSIP_ICON_CHAT_11 = 11,          // This and below are most the same visual as GOSSIP_ICON_CHAT
    GOSSIP_ICON_CHAT_12 = 12,          // but are still used for unknown reasons.
    GOSSIP_ICON_CHAT_13 = 13,
    GOSSIP_ICON_CHAT_14 = 14,          // probably invalid
    GOSSIP_ICON_CHAT_15 = 15,          // probably invalid
    GOSSIP_ICON_CHAT_16 = 16,
    GOSSIP_ICON_CHAT_17 = 17,
    GOSSIP_ICON_CHAT_18 = 18,
    GOSSIP_ICON_CHAT_19 = 19,
    GOSSIP_ICON_CHAT_20 = 20,
    GOSSIP_ICON_MAX = 21
};

const unsigned TrainerTalentResetMinLevel = 10;
const unsigned DefaultGossipTextId = 2;

enum MenuItemOptions
{
    VENDOR = 1,                         // GT_ "I would like to browse your goods."
    ISEEK,                              // GI_ "I seek"
    GI_MAGE,                            // GI_ "mage"
    GI_SHAMAN,                          // GI_ "shaman"
    GI_WARRIOR,                         // GI_ "warrior"
    GI_PALADIN,                         // GI_ "paladin"
    GI_WARLOCK,                         // GI_ "warlock"
    GI_HUNTER,                          // GI_ "hunter"
    GI_ROGUE,                           // GI_ "rogue"
    GI_DRUID,                           // GI_ "druid"
    GI_PRIEST,                          // GI_ "priest"
    TRAINING,                           // GI_ "training"
    BEASTTRAINING,                      // GI_ "Train me in the ways of the beast."
    FLIGHTMASTER,                       // GI_ "Give me a ride."
    AUCTIONEER,                         // GI_ "I would like to make a bid."
    INNKEEPER,                          // GI_ "Make this inn your home."
    BANKER,                             // GI_ "I would like to check my deposit box."
    // GI_ "Bring me back to life."
    // GI_ "How do I create a guild/arena team?"
    TABARD = 20,                        // GI_ "I want to create a guild crest."
    BATTLEMASTER,                       // GI_ "I would like to go to the battleground."
    CLASSTRAINER_TALENTRESET,           // GI_ "I would like to reset my talents."
    PETTRAINER_TALENTRESET,             // GI_ "I wish to untrain my pet."
    CLASSTRAINER_TALENTCONFIRM,         // GI_ "I understand, continue."
    PETTRAINER_TALENTCONFIRM,           // GI_ "Yes, please do."
    GI_DEATHKNIGHT,                     // "deathknight"

    INNKEEPERASK,                       // "What can I do at an inn?"
    STABLE_MY_PET,                      // "I'd like to stable my pet here."
    FOUND_GUILD,                        // "How do I create a guild?"
    FOUND_ARENATEAM,                    // "How do I create a arena team?"
    LEARN_DUAL_TS,                      // "Learn about Dual Talent Specialization."
    // Class Trainers
    MENU_ITEM_DRUID = 64,               // "Druid"
    MENU_ITEM_HUNTER,                   // "Hunter"
    MENU_ITEM_MAGE,                     // "Mage"
    MENU_ITEM_PALADIN,                  // "Paladin"
    MENU_ITEM_PRIEST,                   // "Priest"
    MENU_ITEM_ROGUE,                    // "Rogue"
    MENU_ITEM_SHAMAN,                   // "Shaman"
    MENU_ITEM_WARLOCK,                  // "Warlock"
    MENU_ITEM_WARRIOR,                  // "Warrior"
    MENU_ITEM_DEATHKNIGHT,              // "Deathknight"
    // ProfessionTrainers
    MENU_ITEM_ALCHEMY = 81,             // "Alchemy"
    MENU_ITEM_BSMITHING,                // "Blacksmithing"
    MENU_ITEM_COOKING,                  // "Cooking"
    MENU_ITEM_ENCHANTING,               // "Enchanting" 
    MENU_ITEM_ENGINEERING,              // "Engineering"
    MENU_ITEM_FIRST_AID,                // "First Aid"
    MENU_ITEM_FISHING,                  // "Fishing" 
    MENU_ITEM_HERBALISM,                // "Herbalism"
    MENU_ITEM_INSCRIPTION,              // "Inscription"
    MENU_ITEM_LEATHER_W,                // "Leatherworking"
    MENU_ITEM_MINING,                   // "Mining"
    MENU_ITEM_SKINNING,                 // "Skinning"
    MENU_ITEM_TAILORING,                // "Tailoring"
    MENU_ITEM_JUWELCRAFTING,            // "Jewelcrafting"
    MENU_ITEM_CW_FLYING,                // "Cold Weather Flying Trainer"
    MENU_ITEM_PORTAL,                   // "Portal Trainer"
    // Gossip 1st level items starts 150
    GI_THE_BANK = 150,                  // "The bank"
    GI_THE_BANK2,                       // "Bank" 
    GI_THE_WIND_R_M,                    // "The wind rider master"
    GI_THE_FLIGHT_M,                    // "Flight Master"
    GI_THE_GRYPHON_M,                   // "Gryphon Master"
    GI_THE_HYPPOGRYPH_M,                // "Hippogryph Master"
    GI_DRAGONHAWK_M,                    // "Dragonhawk Master"
    GI_THE_GUILD_M,                     // "The guild master"
    GI_THE_GUILD2_M,                    // "Guild Master"
    GI_THE_INN,                         // "The inn"
    GI_THE_INN2,                        // "Inn"
    GI_THE_MAILBOX,                     // "The mailbox"
    GI_MAILBOX,                         // "Mailbox"
    GI_THE_AH,                          // "The auction house"
    GI_THE_AH2,                         // "Auction House"
    GI_THE_WEAPON_M,                    // "The weapon master"
    GI_THE_WEAPON2_M,                   // "Weapon Master"
    GI_THE_WEAPON3_M,                   // "Weapons Trainer"
    GI_THE_STABLE_M,                    // "The stable master"
    GI_THE_STABLE2_M,                   // "Stable Master"
    GI_STABLE,                          // "Stable"
    GI_THE_BATTLE_M,                    // "The battlemaster"
    GI_THE_BATTLE2_M,                   // "Battlemasters"
    GI_THE_BATTLE3_M,                   // "Battlemaster"
    GI_A_CLASS_T,                       // "A class trainer"
    GI_A_CLASS2_T,                      // "Class Trainer"
    GI_A_CLASS3_T,                      // "Trainers"
    GI_A_PROFESSION_T,                  // "A profession trainer"
    GI_A_PROFESSION2_T,                 // "Profession Trainer"
    GI_THE_ZEPPELIN_M,                  // "The zeppelin master"
    GI_LOCKSMITH,                       // "Locksmith"
    GI_THE_BAT_H,                       // "The bat handler"
    GI_THE_BARBER,                      // "Barber"
    GI_THE_ARENA,                       // "Arena"
    GI_THE_VENDORS,                     // "Vendors"
    GI_OFFICERS_LOUNGE,                 // "Officers' Lounge"
    GI_LOOKING_SOMTH_ELSE,              // "I was looking for something else."
    GI_BAT_HANDLER,                     // "Bat Handler"
    GI_BREW_OTM,                        // "I'd like to buy this month's brew."
    // VariousItems
    GI_THE_OFFICERS = 225,              // "The officers' lounge"
    GI_BANK_OF_IRONFORGE,               // "Bank of Ironforge"
    GI_BANK_OF_STORMWIND,               // "Bank of Stormwind"
    GI_STORMWIND_HARBOUR,               // "Stormwind Harbor"
    GI_DEEPRUN_TRAM,                    // "Deeprun Tram"
    GI_LEXICON_OF_POWER,                // "Lexicon of Power"
    GI_MANA_LOOM,                       // "Mana Loom"
    GI_POI,                             // "Points of Interest"
    GI_CAPITAL_PORTS,                   // "Capital Portals"
    GI_TO_THE_WEST,                     // "To the west."
    GI_THE_WEST,                        // "The west."
    GI_TO_THE_EAST,                     // "To the east."
    GI_THE_EAST,                        // "The east."
    GI_SILVERMOON_C_INN,                // "The Silvermoon City Inn."
    GI_THE_WAYFARERS_TAV,               // "The Wayfarer's Rest tavern."
    GI_WORLDS_END_TAV,                  // "World's End Tavern"
    GI_ALCHEMIE_LAB,                    // "Alchemy Lab"
    GI_GEM_MERCHANT,                    // "Gem Merchant"
    GI_ALDOR_BANK,                      // "Aldor Bank"
    GI_ALDOR_INN,                       // "Aldor inn"
    GI_ALDOR_STABLE,                    // "Aldor Stable"
    GI_ALDOR_GEM_MERCHANT,              // "Aldor Gem Merchant"
    GI_SCYERS_BANK,                     // "Scryers Bank"
    GI_SCYERS_INN,                      // "Scryers inn"
    GI_SVYERS_STABLE,                   // "Scryers Stable"
    GI_SCYER_GEM_MERCHANT,              // "Scryer Gem Merchant"
    GI_A_BATTLEMASTERS,                 // "Alliance Battlemasters"
    GI_H_ARENA_BATTLEMASTERS,           // "Horde & Arena Battlemasters"
    GI_EAST_SEW_ENTR,                   // "Eastern Sewer Entrance"
    GI_WEST_SEW_ENTR,                   // "Western Sewer Entrance"
    GI_WELL_ENTR,                       // "Well Entrance"
    GI_THE_A_QUART,                     // "The Alliance Quarter"
    GI_A_INN,                           // "Alliance Inn"
    GI_THE_H_QUART,                     // "The Horde Quarter"
    GI_THE_H_INN,                       // "Horde Inn"
    GI_NORTH_BANK,                      // "Northern Bank"
    GI_SOUTH_BANK,                      // "Southern Bank"
    GI_SEWERS,                          // "Sewers"
    GI_KRASUS_LAND,                     // "Krasus' Landing"
    GI_VIOLET_CITADEL,                  // "The Violet Citadel"
    GI_VIOLET_HOLD,                     // "The Violet Hold"
    GI_TRADE_DISTRICT,                  // "Trade District"
    GI_ANTONIDAS_MEMORIAL,              // "Antonidas Memorial"
    GI_RUNEWEAV_SQUARE,                 // "Runeweaver Square"
    GI_THE_EVENTIDE,                    // "The Eventide"
    GI_CEMETARY,                        // "Cemetary"
    GI_ARMOR,                           // "Armor"
    GI_CLOTHING,                        // "Clothing"
    GI_EMBLEM_GEAR,                     // "Emblem Gear"
    GI_FLOWERS,                         // "Flowers"
    GI_FRUIT,                           // "Fruit"
    GI_GENERAL_GOODS,                   // "General Goods"
    GI_JEWELRY,                         // "Jewelry"
    GI_PET_SUBS_EX_MOUNTS,              // "Pet Supplies & Exotic Mounts"
    GI_PIE_PASTRY_CAKES,                // "Pie, Pastry & Cakes"
    GI_REAGENTS_MAG_GOODS,              // "Reagents & Magical Goods"
    GI_TOYS,                            // "Toys"
    GI_TRADE_SUP,                       // "Trade Supplies"
    GI_TRINKETS_REL_OFF,                // "Trinkets. Relics & Off-hand items"
    GI_WEAPONS,                         // "Weapons"
    GI_WINE_CHEESE,                     // "Wine & Cheese"
    GI_CLOTH_ARMOR,                     // "Cloth Armor" 
    GI_LEATHER_ARMOR,                   // "Leather Armor"
    GI_MAIL_ARMOR,                      // "Mail Armor"
    GI_PLATE_ARMOR,                     // "Plate Armor"
    GI_SHIELDS,                         // "Shields"
    GI_MELEE_WEAPONS,                   // "Melee Weapons"
    GI_RANGE_THROW_WEAPONS,             // "Ranged & Thrown Weapons"
    GI_STAVES_WANDS,                    // "Staves & Wands"
    GI_TRICK_TREAT,                     // "Trick or Treat!"
    // Not GuardGossips starts with 350
    GI_ULTRA_CANNON = 352,              // "Tell me how to use the Blastenheimer 5000 Ultra Cannon."
    GI_BROWS_GOODS,                     // "Let me browse your goods."
    GI_TELL_ME_DARKMOON_CARDS,          // "Tell me more about these Darkmoon Cards."
    GI_TELL_BEAST_DECK,                 // "Tell me about the Beasts Deck."
    GI_TELL_PORTAL_DECK,                // "Tell me about the Portals Deck."
    GI_TELL_ELEMENTALS_DECK,            // "Tell me about the Elementals Deck."
    GI_TELL_WARLORDS_DECK,              // "Tell me about the Warlords Deck."
    GI_TELL_FURIES_DECK,                // "Tell me about the Furies Deck."
    GI_TELL_LUNACY_DECK,                // "Tell me about the Lunacy Deck."
    GI_TELL_BLESSINGS_DECK,             // "Tell me about the Blessings Deck."
    GI_TELL_STORMS_DECK,                // "Tell me about the Storms Deck."
    GI_READY_DISC_FORTUNE,              // "I am ready to discover where my fortune lies!"
    GI_DF_ANSWER_1_1,                   // "I slay the man on the spot as my liege would expect me to do, as he is nothing more than a thief and a liar."
    GI_DF_ANSWER_1_2,                   // "I turn over the man to my liege for punishment, as he has broken the law of the land and it is my sworn duty to enforce it."
    GI_DF_ANSWER_1_3,                   // "I confiscate the corn he has stolen, warn him that stealing is a path towards doom and destruction, but I let him go to return to his family."
    GI_DF_ANSWER_1_4,                   // "I allow the man to take enough corn to feed his family for a couple of days, encouraging him to leave the land."
    GI_DF_ANSWER_2_1,                   // "I execute him as per my liege's instructions, and do it in such a manner that he suffers painfully before he dies a retribution for his crimes against my people."
    GI_DF_ANSWER_2_2,                   // "I execute him as per my liege's instructions, but doing so in as painless of a way as possible. Justice must be served, but I can show some compassion."
    GI_DF_ANSWER_2_3,                   // "I risk my own life and free him so that he may prove his innocence. If I can, I'll help him in any way."
    GI_DF_ANSWER_3_1,                   // "I confront the ruler on his malicious behavior, upholding my liege's honor at risk of any future diplomacy."
    GI_DF_ANSWER_3_2,                   // "I not-so-quietly ignore the insult, hoping to instill a fear in the ruler that he may have gaffed. I then inform my liege of the insult when I return."
    GI_DF_ANSWER_3_3,                   // "I quietly ignore the insult. I will not tell my liege, as I am to secure peace at all costs. It's only an insult - not a declaration of war."
    GI_DF_ANSWER_4_1,                   // "I would speak against my brother joining the order, rushing a permanent breech in our relationship. He would be a danger to himself and those around him, and that is too great a risk hoping he would improve over time."
    GI_DF_ANSWER_4_2,                   // "I would speak for my brother joining the order, potentially risking the safety of the order. I could help him with the order's regimens, and I'd have faith in his ability to adapt and learn."
    GI_DF_ANSWER_4_3,                   // "I would create some surreptitious means to keep my brother out of the order. I can keep him out without him being any bit wiser, thereby saving our familial bonds."
    GI_DF_ANSWER_5_1,                   // "I would show my liege the beast's ear and claim the beast's death as my own, taking the reward for my own use. It is wrong to claim a deed as your own that someone else in fact did."
    GI_DF_ANSWER_5_2,                   // "I would show my liege the beast's ear and claim the beast's death as my own - after all, I did slay it. I would then offer some of the reward to the destitute knight to help his family."
    GI_DF_ANSWER_5_3,                   // "I would remain silent about the kill and allow the knight to claim the reward to aid his family."
    GI_DF_WRITTEN_FORTUNES,             // "I'd love to get one of those written fortunes you mentioned! I've got the space in my inventory for it."
    GI_DF_NOT_ENOUGH_SLOTS,             // "You do not have enough free slots for your fortune!"
    GI_DF_WHAT_PURCHASE,                // "What can I purchase?"
    GI_DF_FAIRE_PRIZE,                  // "What are Darkmoon Faire Prize Tickets and how do I get them?"
    GI_DF_WHAT_ARE_DARKMOON,            // "What are Darkmoon Cards?"
    GI_DF_THINGS_FAIRE,                 // "What other things can I do at the faire?"
    GI_DF_MORE,                         // "<more>"
    GI_DF_TONK_CONTROLS,                // "What are these Tonk Control Consoles?"
    GI_DF_ABOUT_CANON,                  // "Tell me about the cannon."
    GI_DF_ASK_PROFIT,                   // "Silas, why is most everything at the fair free? How do you make a profit?"
    GI_MG_FLY_VILLAGE,                  // "I'd like to fly to Rut'theran Village."
    GI_MG_FLY_THUNDER_BLUFF,            // "I'd like to fly to Thunder Bluff."
    GI_MULGORE_STORY_SKORN,             // "Tell me a story, Skorn."
    GI_SHATT_EXARCH_NASUUN_1,           // "Nasuun, do you know how long until we have an alchemy lab at the Sun's Reach Harbor?"
    GI_SHATT_EXARCH_NASUUN_2,           // "What do you know about the magical gates at the Sunwell Plateau being brought down?"
    GI_SHATT_EXARCH_NASUUN_3,           // "I have something else to ask you about."
    GI_SHATT_ZEPH_COT,                  // "Bring me to Caverns of Time!"
    GI_SW_ARCHMAGE_JAINA = 398,         // "Can you send me to Theramore? I have an urgent message for Lady Jaina from Highlord Bolvar."
    GI_SW_HARBOR_FLY_YES,               // "Yes, please."
    GI_SW_HARBOR_FLY_NO,                // "No, thank you."
    GI_TANARIS_CRANK_HIPPO,             // "Please tell me more about the hippogryphs."
    GI_TANARIS_CRANK_GORDUNNI,          // "Please tell me more about the Gordunni ogres."
    GI_TANARIS_TELL_TRENTON,            // "Tell me more, Trenton."
    GI_TELDSASSIL_HIPPO,                // "Tell me more about these hippogryphs."
    GI_THERAMORE_CROMSONWING,           // "Lady Jaina told me to speak to you about using a gryphon to survey Alcaz Island."
    GI_THERAMORE_SHADY_REST,            // "What have you heard of the Shady Rest Inn?"
    GI_T_HAMMERSMITH_LEARN,             // "Please teach me how to become a hammersmith, Lilith."
    GI_T_HAMMERSMITH_UNLEARN,           // "I wish to unlearn Hammersmithing!"
    GI_T_SWORDSMITH_LEARN,              // "Please teach me how to become a swordsmith, Seril."
    GI_T_SWORDSMITH_UNLEARN,            // "I wish to unlearn Swordsmithing!"
    GI_T_AXESMITH_LEARN,                // "Please teach me how to become a axesmith, Kilram."
    GI_T_AXESMITH_UNLEARN,              // "I wish to unlearn Axesmithing!"
    GI_DISABLE_XP_GAIN = 419,           // "I no longer wish to gain experience."
    GI_ENABLE_XP_GAIN,                  // "I wish to start gaining experience again"
    GI_BOXMSG_DISABLE_XP_GAIN,          // "Are you certain you wish to stop gaining experience?"
    GI_BOXMSG_ENABLE_XP_GAIN,           // "Are you certain you wish to start gaining experience again?"
};
