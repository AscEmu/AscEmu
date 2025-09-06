/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

//.cheat list
bool ChatCommandHandler::HandleCheatListCommand(const char* /*args*/, WorldSession* m_session)
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
        systemMessage(m_session, "Player %s has the following cheats activated:", player_target->getName().c_str());

        if (player_target->m_cheats.hasCooldownCheat)
            systemMessage(m_session, "-- Cooldown is active.");
        if (player_target->m_cheats.hasCastTimeCheat)
            systemMessage(m_session, "-- CastTime is active.");
        if (player_target->m_cheats.hasGodModeCheat)
            systemMessage(m_session, "-- GodMode is active.");
        if (player_target->m_cheats.hasPowerCheat)
            systemMessage(m_session, "-- PowerCheat is active.");
        if (player_target->m_cheats.hasFlyCheat)
            systemMessage(m_session, "-- FlyCheat is active.");
        if (player_target->m_cheats.hasAuraStackCheat)
            systemMessage(m_session, "-- AuraStack is active.");
        if (player_target->m_cheats.hasItemStackCheat)
            systemMessage(m_session, "-- ItemStack is active.");
        if (player_target->m_cheats.hasTriggerpassCheat)
            systemMessage(m_session, "-- TriggerPass is active.");
        if (player_target->m_cheats.hasTaxiCheat)
            systemMessage(m_session, "-- TaxiCheat is active.");
        if (player_target->m_isGmInvisible)
            systemMessage(m_session, "-- Invisibility is active.");
        if (player_target->m_isInvincible)
            systemMessage(m_session, "-- Invincibility is active.");
    }
    else
    {
        if (player_target == m_session->GetPlayer())
            systemMessage(m_session, "You have no active cheats!");
        else
            systemMessage(m_session, "Player {} has no active cheats!", player_target->getName());
    }

    return true;
}

//.cheat taxi
bool ChatCommandHandler::HandleCheatTaxiCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasTaxiCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "You can now use all taxi nodes.");
        }
        else
        {
            greenSystemMessage(m_session, "{} can now use all taxi nodes", player_target->getName());
            systemMessage(m_session, "{} has activated taxi cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated TaxiCheat on Player: %s", player_target->getName().c_str());

        }

        player_target->m_cheats.hasTaxiCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "You can just use discovered taxi nodes from now.");
        }
        else
        {
            greenSystemMessage(m_session, "{} can just use discovered taxi nodes from now.", player_target->getName());
            systemMessage(player_target->getSession(), "{} has deactivated taxi cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated TaxiCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTaxiCheat = false;
    }

    return true;
}

//.cheat cooldown
bool ChatCommandHandler::HandleCheatCooldownCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasCooldownCheat)
    {
        player_target->resetAllCooldowns();

        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Cooldown cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the cooldown cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated cooldown cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated CooldownCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCooldownCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Cooldown cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the cooldown cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated cooldown cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated CooldownCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCooldownCheat = false;
    }

    return true;
}

//.cheat casttime
bool ChatCommandHandler::HandleCheatCastTimeCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasCastTimeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "CastTime cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the casttime cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated casttime cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated CastTimeCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCastTimeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "CastTime cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the casttime cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated casttime cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated CastTimeCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasCastTimeCheat = false;
    }

    return true;
}

//.cheat power
bool ChatCommandHandler::HandleCheatPowerCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasPowerCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Power cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the power cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated power cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated PowerCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasPowerCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Power cheat is now deactivated for you.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the power cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated power cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated PowerCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasPowerCheat = false;
    }

    return true;
}

//.cheat god
bool ChatCommandHandler::HandleCheatGodCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasGodModeCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "God cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the god cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated god cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated GodCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasGodModeCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "God cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the god cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated god cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated GodCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasGodModeCheat = false;
    }

    return true;
}

//.cheat fly
bool ChatCommandHandler::HandleCheatFlyCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasFlyCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Fly cheat is now activated for you.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the fly cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated fly cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated FlyCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasFlyCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Fly cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the fly cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated fly cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated FlyCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasFlyCheat = false;
    }

    player_target->setMoveCanFly(player_target->m_cheats.hasFlyCheat);

    return true;
}

//.cheat aurastack
bool ChatCommandHandler::HandleCheatAuraStackCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasAuraStackCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "AuraStack cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the aurastack cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated aurastack cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated AuraStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasAuraStackCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "AuraStack cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the aurastack cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated aurastack cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated AuraStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasAuraStackCheat = false;
    }

    return true;
}

//.cheat itemstack
bool ChatCommandHandler::HandleCheatItemStackCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasItemStackCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "ItemStack cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the itemstack cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated itemstack cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated ItemStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasItemStackCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "ItemStack cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the itemstack cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated itemstack cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated ItemStack on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasItemStackCheat = false;
    }

    return true;
}

//.cheat triggerpass
bool ChatCommandHandler::HandleCheatTriggerpassCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (!player_target->m_cheats.hasTriggerpassCheat)
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Triggerpass cheat activated.");
        }
        else
        {
            greenSystemMessage(m_session, "Activated the triggerpass cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has activated triggerpass cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has activated TriggerpassCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTriggerpassCheat = true;
    }
    else
    {
        if (player_target == m_session->GetPlayer())
        {
            greenSystemMessage(m_session, "Triggerpas cheat deactivated.");
        }
        else
        {
            greenSystemMessage(m_session, "Deactivated the triggerpass cheat on {}.", player_target->getName());
            systemMessage(m_session, "{} has deactivated triggerpass cheat on you.", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "has deactivated TriggerpassCheat on Player: %s", player_target->getName().c_str());
        }

        player_target->m_cheats.hasTriggerpassCheat = false;
    }

    return true;
}
