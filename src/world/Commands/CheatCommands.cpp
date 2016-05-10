/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.cheat list
bool ChatHandler::HandleCheatListCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target->CooldownCheat || player_target->CastTimeCheat ||
        player_target->GodModeCheat || player_target->PowerCheat ||
        player_target->FlyCheat || player_target->AuraStackCheat ||
        player_target->ItemStackCheat || player_target->TriggerpassCheat ||
        player_target->m_isGmInvisible || player_target->bInvincible
        )
    {
        auto player_name = player_target->GetName();
        SystemMessage(m_session, "Player %s has the following cheats activated:", player_name);


        if (player_target->CooldownCheat)
            SystemMessage(m_session, "-- Cooldown is active.", player_name);
        if (player_target->CastTimeCheat)
            SystemMessage(m_session, "-- CastTime is active.", player_name);
        if (player_target->GodModeCheat)
            SystemMessage(m_session, "-- GodMode is active.", player_name);
        if (player_target->PowerCheat)
            SystemMessage(m_session, "-- PowerCheat is active.", player_name);
        if (player_target->FlyCheat)
            SystemMessage(m_session, "-- FlyCheat is active.", player_name);
        if (player_target->AuraStackCheat)
            SystemMessage(m_session, "-- AuraStack is active.", player_name);
        if (player_target->ItemStackCheat)
            SystemMessage(m_session, "-- ItemStack is active.", player_name);
        if (player_target->TriggerpassCheat)
            SystemMessage(m_session, "-- TriggerPass is active.", player_name);
        if (player_target->TaxiCheat)
            SystemMessage(m_session, "-- TaxiCheat is active.", player_name);
        if (player_target->m_isGmInvisible)
            SystemMessage(m_session, "-- Invisibility is active.", player_name);
        if (player_target->bInvincible)
            SystemMessage(m_session, "-- Invincibility is active.", player_name);
    }
    else
    {
        if (player_target == m_session->GetPlayer())
            SystemMessage(m_session, "You have no active cheats!", player_target->GetName());
        else
            SystemMessage(m_session, "Player %s has no active cheats!", player_target->GetName());
    }

    return true;
}

//.cheat taxi
bool ChatHandler::HandleCheatTaxiCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->TaxiCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "You can now use all taxi nodes.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "%s can now use all taxi nodes", player_target->GetName());
            SystemMessage(m_session, "%s has activated taxi cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated TaxiCheat on Player: %s", player_target->GetName());

        }

        player_target->TaxiCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "You can just use discovered taxi nodes from now.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "%s can just use discovered taxi nodes from now.", player_target->GetName());
            SystemMessage(player_target->GetSession(), "%s has deactivated taxi cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated TaxiCheat on Player: %s", player_target->GetName());
        }

        player_target->TaxiCheat = false;
    }

    return true;
}

//.cheat cooldown
bool ChatHandler::HandleCheatCooldownCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->CooldownCheat)
    {
        for (SpellSet::const_iterator itr = player_target->mSpells.begin(); itr != player_target->mSpells.end(); ++itr)
            player_target->ClearCooldownForSpell((*itr));

        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Cooldown cheat activated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the cooldown cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has activated cooldown cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated CooldownCheat on Player: %s", player_target->GetName());
        }

        player_target->CooldownCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Cooldown cheat deactivated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the cooldown cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has deactivated cooldown cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated CooldownCheat on Player: %s", player_target->GetName());
        }

        player_target->CooldownCheat = false;
    }

    return true;
}

//.cheat casttime
bool ChatHandler::HandleCheatCastTimeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->CastTimeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "CastTime cheat activated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the casttime cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has activated casttime cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated CastTimeCheat on Player: %s", player_target->GetName());
        }

        player_target->CastTimeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "CastTime cheat deactivated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the casttime cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has deactivated casttime cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated CastTimeCheat on Player: %s", player_target->GetName());
        }

        player_target->CastTimeCheat = false;
    }

    return true;
}

//.cheat power
bool ChatHandler::HandleCheatPowerCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->CastTimeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Power cheat activated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the power cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has activated power cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated PowerCheat on Player: %s", player_target->GetName());
        }

        player_target->PowerCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Power cheat deactivated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the power cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has deactivated power cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated PowerCheat on Player: %s", player_target->GetName());
        }

        player_target->PowerCheat = false;
    }

    return true;
}

//.cheat god
bool ChatHandler::HandleCheatGodCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->CastTimeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "God cheat activated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the god cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has activated god cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated GodCheat on Player: %s", player_target->GetName());
        }

        player_target->GodModeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "God cheat deactivated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the god cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has deactivated god cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated GodCheat on Player: %s", player_target->GetName());
        }

        player_target->GodModeCheat = false;
    }

    return true;
}

//.cheat fly
bool ChatHandler::HandleCheatFlyCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->FlyCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Fly cheat activated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the fly cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has activated fly cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has activated FlyCheat on Player: %s", player_target->GetName());
        }

        player_target->EnableFlight();
        player_target->m_setflycheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Fly cheat deactivated.", player_target->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the fly cheat on %s.", player_target->GetName());
            SystemMessage(m_session, "%s has deactivated fly cheat on you.", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "has deactivated FlyCheat on Player: %s", player_target->GetName());
        }

        player_target->DisableFlight();
        player_target->m_setflycheat = false;
    }

    return true;
}
