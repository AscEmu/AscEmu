/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"


//.kill - kills target or player with <name>
bool ChatHandler::HandleKillCommand(const char* args, WorldSession* m_session)
{
    bool is_name_set = false;
    auto unit_target = GetSelectedUnit(m_session);

    if (*args)
        is_name_set = true;

    if (is_name_set)
    {
        auto named_player = objmgr.GetPlayer(args, false);
        if (named_player == nullptr)
        {
            RedSystemMessage(m_session, "Player %s is not online or does not exist!", args);
            return true;
        }
        else
        {
            named_player->SetHealth(0);
            named_player->KillPlayer();
            RedSystemMessageToPlr(named_player, "You were killed by %s with a GM command.", m_session->GetPlayer()->GetName());
            GreenSystemMessage(m_session, "Killed player %s.", args);
            sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid:  " I64FMT " ", m_session->GetPlayer()->GetName(), named_player->GetGUID(), named_player->GetNameString());
        }
    }
    else
    {
        if (unit_target != nullptr)
        {
            switch (unit_target->GetTypeId())
            {
                case TYPEID_UNIT:
                {
                    auto creature = static_cast<Creature*>(unit_target);
                    auto kill_spell = dbcSpell.LookupEntryForced(5);
                    if (kill_spell == nullptr)
                        return true;

                    SpellCastTargets targets(unit_target->GetGUID());
                    Spell* spell = sSpellFactoryMgr.NewSpell(m_session->GetPlayer(), kill_spell, true, 0);
                    spell->prepare(&targets);

                    GreenSystemMessage(m_session, "Killed Creature %s.", creature->GetCreatureInfo()->Name);
                    sGMLog.writefromsession(m_session, "used kill command on Creature %u [%s], spawn ID: %u", creature->GetEntry(), creature->GetCreatureInfo()->Name, creature->spawnid);
                    break;
                }
                case TYPEID_PLAYER:
                {
                    auto player = static_cast<Player*>(unit_target);
                    if (player == nullptr)
                        return true;

                    player->SetHealth(0);
                    player->KillPlayer();
                    RedSystemMessageToPlr(player, "You were killed by %s with a GM command.", m_session->GetPlayer()->GetName());
                    GreenSystemMessage(m_session, "Killed player %s.", player->GetName());
                    sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid:  " I64FMT " ", m_session->GetPlayer()->GetName(), player->GetGUID());
                    break;
                }
                default:
                    RedSystemMessage(m_session, "Something went wrong while killing a unit with this command!");
                    break;

            }
        }
        else
        {
            RedSystemMessage(m_session, "No selected target or player name set.");
            RedSystemMessage(m_session, "Use .kill on an selected unit (player/creature)");
            RedSystemMessage(m_session, "Use .kill <playername> to kill players");
            return true;
        }

    }

    return true;
}

