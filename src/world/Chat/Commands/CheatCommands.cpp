/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

//.cheat list
bool ChatHandler::HandleCheatListCommand(const char* /*args*/, WorldSession* m_session)
{
    const auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target->m_cheats.hasCooldownCheat || player_target->m_cheats.hasCastTimeCheat ||
        player_target->m_cheats.hasGodModeCheat || player_target->m_cheats.hasPowerCheat ||
        player_target->m_cheats.hasFlyCheat || player_target->m_cheats.hasAuraStackCheat ||
        player_target->m_cheats.hasItemStackCheat || player_target->m_cheats.hasTriggerpassCheat ||
        player_target->m_isGmInvisible || player_target->m_isInvincible
        )
    {
        SystemMessage(m_session, "Player %s has the following cheats activated:", player_target->getName().c_str());

        if (player_target->m_cheats.hasCooldownCheat)
            SystemMessage(m_session, "-- Cooldown is active.");
        if (player_target->m_cheats.hasCastTimeCheat)
            SystemMessage(m_session, "-- CastTime is active.");
        if (player_target->m_cheats.hasGodModeCheat)
            SystemMessage(m_session, "-- GodMode is active.");
        if (player_target->m_cheats.hasPowerCheat)
            SystemMessage(m_session, "-- PowerCheat is active.");
        if (player_target->m_cheats.hasFlyCheat)
            SystemMessage(m_session, "-- FlyCheat is active.");
        if (player_target->m_cheats.hasAuraStackCheat)
            SystemMessage(m_session, "-- AuraStack is active.");
        if (player_target->m_cheats.hasItemStackCheat)
            SystemMessage(m_session, "-- ItemStack is active.");
        if (player_target->m_cheats.hasTriggerpassCheat)
            SystemMessage(m_session, "-- TriggerPass is active.");
        if (player_target->m_cheats.hasTaxiCheat)
            SystemMessage(m_session, "-- TaxiCheat is active.");
        if (player_target->m_isGmInvisible)
            SystemMessage(m_session, "-- Invisibility is active.");
        if (player_target->m_isInvincible)
            SystemMessage(m_session, "-- Invincibility is active.");
    }
    else
    {
        if (player_target == m_session->GetPlayer())
            SystemMessage(m_session, "You have no active cheats!");
        else
            SystemMessage(m_session, "Player %s has no active cheats!", player_target->getName().c_str());
    }

    return true;
}

//.cheat taxi
bool ChatHandler::HandleCheatTaxiCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasTaxiCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "You can now use all taxi nodes.");
        }
        else
        {
            GreenSystemMessage(m_session, "%s can now use all taxi nodes", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated taxi cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated TaxiCheat on Player: %s", player_target->getName().c_str());

        }

        player_target->m_cheats.hasTaxiCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "You can just use discovered taxi nodes from now.");
        }
        else
        {
            GreenSystemMessage(m_session, "%s can just use discovered taxi nodes from now.", player_target->getName().c_str());
            SystemMessage(player_target->getSession(), "%s has deactivated taxi cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated TaxiCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTaxiCheat = false;
    }

    return true;
}

//.cheat cooldown
bool ChatHandler::HandleCheatCooldownCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasCooldownCheat)
    {
        player_target->resetAllCooldowns();

        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Cooldown cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the cooldown cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated cooldown cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated CooldownCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCooldownCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Cooldown cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the cooldown cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated cooldown cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated CooldownCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCooldownCheat = false;
    }

    return true;
}

//.cheat casttime
bool ChatHandler::HandleCheatCastTimeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasCastTimeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "CastTime cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the casttime cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated casttime cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated CastTimeCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCastTimeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "CastTime cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the casttime cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated casttime cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated CastTimeCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCastTimeCheat = false;
    }

    return true;
}

//.cheat power
bool ChatHandler::HandleCheatPowerCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasPowerCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Power cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the power cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated power cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated PowerCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasPowerCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Power cheat is now deactivated for you.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the power cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated power cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated PowerCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasPowerCheat = false;
    }

    return true;
}

//.cheat god
bool ChatHandler::HandleCheatGodCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasGodModeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "God cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the god cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated god cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated GodCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasGodModeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "God cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the god cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated god cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated GodCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasGodModeCheat = false;
    }

    return true;
}

//.cheat fly
bool ChatHandler::HandleCheatFlyCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasFlyCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Fly cheat is now activated for you.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the fly cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated fly cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated FlyCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasFlyCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Fly cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the fly cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated fly cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated FlyCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasFlyCheat = false;
    }

    player_target->setMoveCanFly(player_target->m_cheats.hasFlyCheat);

    return true;
}

//.cheat aurastack
bool ChatHandler::HandleCheatAuraStackCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasAuraStackCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "AuraStack cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the aurastack cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated aurastack cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated AuraStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasAuraStackCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "AuraStack cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the aurastack cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated aurastack cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated AuraStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasAuraStackCheat = false;
    }

    return true;
}

//.cheat itemstack
bool ChatHandler::HandleCheatItemStackCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasItemStackCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "ItemStack cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the itemstack cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated itemstack cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated ItemStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasItemStackCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "ItemStack cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the itemstack cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated itemstack cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated ItemStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasItemStackCheat = false;
    }

    return true;
}

//.cheat triggerpass
bool ChatHandler::HandleCheatTriggerpassCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasTriggerpassCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Triggerpass cheat activated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Activated the triggerpass cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has activated triggerpass cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has activated TriggerpassCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTriggerpassCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            GreenSystemMessage(m_session, "Triggerpas cheat deactivated.");
        }
        else
        {
            GreenSystemMessage(m_session, "Deactivated the triggerpass cheat on %s.", player_target->getName().c_str());
            SystemMessage(m_session, "%s has deactivated triggerpass cheat on you.", m_session->GetPlayer()->getName().c_str());
            sGMLog.writefromsession(m_session, "has deactivated TriggerpassCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTriggerpassCheat = false;
    }

    return true;
}
