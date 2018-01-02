/*
 Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
