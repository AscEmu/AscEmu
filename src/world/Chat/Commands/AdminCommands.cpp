/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Spell/SpellMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"
#include "Spell/Definitions/SpellEffects.h"
#include "Spell/SpellAuras.h"

//.admin castall
bool ChatHandler::HandleAdminCastAllCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "No spellid specified.");
        return true;
    }

    uint32 spell_id = atol(args);
    auto spell_entry = sSpellMgr.getSpellInfo(spell_id);
    if (!spell_entry)
    {
        RedSystemMessage(m_session, "Spell %u is not a valid spell!", spell_id);
        return true;
    }

    for (uint8 i = 0; i < 3; ++i)
    {
        if (spell_entry->getEffect(i) == SPELL_EFFECT_LEARN_SPELL)
        {
            sGMLog.writefromsession(m_session, "used learn spell stopped %u", spell_id);
            RedSystemMessage(m_session, "Learn spell specified.");
            return true;
        }
    }

    sGMLog.writefromsession(m_session, "used castall command, spellid %u", spell_id);

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        Player* player = itr->second;
        if (player->GetSession() && player->IsInWorld())
        {
            if (player->GetMapMgr() != m_session->GetPlayer()->GetMapMgr())
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
    objmgr._playerslock.ReleaseReadLock();

    BlueSystemMessage(m_session, "Casted spell %u on all players!", spell_id);
    return true;
}

//.admin dispellall
bool ChatHandler::HandleAdminDispelAllCommand(const char* args, WorldSession* m_session)
{
    uint32 pos = 0;
    if (*args)
        pos = atoi(args);

    sGMLog.writefromsession(m_session, "used dispelall command, pos %u", pos);

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        Player* player = itr->second;
        if (player->GetSession() && player->IsInWorld())
        {
            if (player->GetMapMgr() != m_session->GetPlayer()->GetMapMgr())
            {
                sEventMgr.AddEvent(static_cast< Unit* >(player), &Unit::DispelAll, pos ? true : false, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                player->DispelAll(pos ? true : false);
            }
        }
    }
    sGMLog.writefromsession(m_session, "used mass dispel");
    objmgr._playerslock.ReleaseReadLock();

    BlueSystemMessage(m_session, "Dispel action done.");
    return true;
}

//.admin masssummon
bool ChatHandler::HandleAdminMassSummonCommand(const char* args, WorldSession* m_session)
{
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();

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

    uint32 summon_count = 0;
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        Player* plr = itr->second;
        if (plr->GetSession() && plr->IsInWorld())
        {
            if (faction > -1 && plr->getTeam() == static_cast<uint32>(faction))
            {
                plr->SummonRequest(summon_player->getGuidLow(), summon_player->GetZoneId(), summon_player->GetMapId(), summon_player->GetInstanceID(), summon_player->GetPosition());
                ++summon_count;
            }
            else if (faction == -1)
            {
                plr->SummonRequest(summon_player->getGuidLow(), summon_player->GetZoneId(), summon_player->GetMapId(), summon_player->GetInstanceID(), summon_player->GetPosition());
                ++summon_count;
            }

        }
    }

    sGMLog.writefromsession(m_session, "requested a mass summon of %u players.", summon_count);

    objmgr._playerslock.ReleaseReadLock();

    return true;
}

//.admin playall
bool ChatHandler::HandleAdminPlayGlobalSoundCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32 sound_id = atoi(args);
    if (sound_id == 0)
        return false;

    sWorld.playSoundToAllPlayers(sound_id);

    BlueSystemMessage(m_session, "Broadcasted sound %u to server.", sound_id);

    sGMLog.writefromsession(m_session, "used play all command soundid %u", sound_id);

    return true;
}
