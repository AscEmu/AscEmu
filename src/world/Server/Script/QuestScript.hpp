/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <cstdint>

class QuestLogEntry;
class Player;

class SERVER_DECL QuestScript
{
public:
    QuestScript() {}
    virtual ~QuestScript() {}

    virtual void OnQuestStart(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
    virtual void OnQuestComplete(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
    virtual void OnQuestCancel(Player* /*mTarget*/) {}
    virtual void OnGameObjectActivate(uint32_t /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
    virtual void OnCreatureKill(uint32_t /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
    virtual void OnExploreArea(uint32_t /*areaId*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
    virtual void OnPlayerItemPickup(uint32_t /*itemId*/, uint32_t /*totalCount*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
};
