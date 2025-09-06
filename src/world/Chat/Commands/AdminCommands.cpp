/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Logging/Log.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"

//.admin castall
bool ChatCommandHandler::HandleAdminCastAllCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "No spellid specified.");
        return true;
    }

    uint32_t spell_id = std::stoul(args);
    SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(spell_id);
    if (!spell_entry)
    {
        redSystemMessage(m_session, "Spell {} is not a valid spell!", spell_id);
        return true;
    }

    for (uint8_t i = 0; i < 3; ++i)
    {
        if (spell_entry->getEffect(i) == SPELL_EFFECT_LEARN_SPELL)
        {
            sGMLog.writefromsession(m_session, "used learn spell stopped %u", spell_id);
            redSystemMessage(m_session, "Learn spell specified.");
            return true;
        }
    }

    sGMLog.writefromsession(m_session, "used castall command, spellid %u", spell_id);

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession() && player->IsInWorld())
        {
            if (player->getWorldMap() != m_session->GetPlayer()->getWorldMap())
            {
                sEventMgr.AddEvent(static_cast< Unit* >(player), &Unit::eventCastSpell, static_cast< Unit* >(player), spell_entry, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                Spell* spell = sSpellMgr.newSpell(player, spell_entry, true, 0);
                SpellCastTargets targets(player->getGuid());
                spell->prepare(&targets);
            }
        }
    }

    blueSystemMessage(m_session, "Casted spell {} on all players!", spell_id);
    return true;
}

//.admin dispellall
bool ChatCommandHandler::HandleAdminDispelAllCommand(const char* args, WorldSession* m_session)
{
    uint32_t pos = 0;
    if (*args)
        pos = atoi(args);

    sGMLog.writefromsession(m_session, "used dispelall command, pos %u", pos);

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession() && player->IsInWorld())
        {
            if (player->getWorldMap() != m_session->GetPlayer()->getWorldMap())
            {
                if (pos)
                    sEventMgr.AddEvent(static_cast<Unit*>(player), &Unit::removeAllPositiveAuras, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                else
                    sEventMgr.AddEvent(static_cast<Unit*>(player), &Unit::removeAllNegativeAuras, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                if (pos)
                    player->removeAllPositiveAuras();
                else
                    player->removeAllNegativeAuras();
            }
        }
    }
    sGMLog.writefromsession(m_session, "used mass dispel");

    blueSystemMessage(m_session, "Dispel action done.");
    return true;
}

//.admin masssummon
bool ChatCommandHandler::HandleAdminMassSummonCommand(const char* args, WorldSession* m_session)
{
    sObjectMgr.m_playerLock.lock();

    Player* summon_player = m_session->GetPlayer();

    int faction = -1;
    char Buffer[170];

    if (*args == 'a')
    {
        faction = 0;
        snprintf(Buffer, 170, "%s%s Has requested a mass summon of all Alliance players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->getName().c_str());

    }
    else if (*args == 'h')
    {
        faction = 1;
        snprintf(Buffer, 170, "%s%s Has requested a mass summon of all Horde players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->getName().c_str());
    }
    else
    {
        snprintf(Buffer, 170, "%s%s Has requested a mass summon of all players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->getName().c_str());
    }

    uint32_t summon_count = 0;
    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession() && player->IsInWorld())
        {
            if (faction > -1 && player->getTeam() == static_cast<uint32_t>(faction))
            {
                player->sendSummonRequest(summon_player->getGuidLow(), summon_player->getZoneId(), summon_player->GetMapId(), summon_player->GetInstanceID(), summon_player->GetPosition());
                ++summon_count;
            }
            else if (faction == -1)
            {
                player->sendSummonRequest(summon_player->getGuidLow(), summon_player->getZoneId(), summon_player->GetMapId(), summon_player->GetInstanceID(), summon_player->GetPosition());
                ++summon_count;
            }

        }
    }

    sGMLog.writefromsession(m_session, "requested a mass summon of %u players.", summon_count);

    return true;
}

//.admin playall
bool ChatCommandHandler::HandleAdminPlayGlobalSoundCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32_t sound_id = atoi(args);
    if (sound_id == 0)
        return false;

    sWorld.playSoundToAllPlayers(sound_id);

    blueSystemMessage(m_session, "Broadcasted sound {} to server.", sound_id);

    sGMLog.writefromsession(m_session, "used play all command soundid %u", sound_id);

    return true;
}
