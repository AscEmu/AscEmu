/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"
#include <world/Spell/Definitions/PowerType.h>
#include <world/Units/Stats.h>


//.modify hp
bool ChatHandler::HandleModifyHp(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldHealth = unitTarget->getHealth();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify health from %u to %u on %s (%u)", oldHealth, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the health of %s from %u to %u.", player->getName().c_str(), oldHealth, value);
        GreenSystemMessage(player->GetSession(), "%s modify your health from %u to %u.", session->GetPlayer()->getName().c_str(), oldHealth, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify health from %u to %u on %s (%u)", oldHealth, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the health of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldHealth, value);
    }

    unitTarget->setHealth(value);

    return false;
}

//.modify mana
bool ChatHandler::HandleModifyMana(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldMana = unitTarget->getPower(POWER_TYPE_MANA);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (mana) from %u to %u on %s (%u)", oldMana, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the power (mana) of %s from %u to %u.", player->getName().c_str(), oldMana, value);
        GreenSystemMessage(player->GetSession(), "%s modify your power (mana) from %u to %u.", session->GetPlayer()->getName().c_str(), oldMana, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (mana) from %u to %u on %s (%u)", oldMana, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the power (mana) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldMana, value);
    }

    unitTarget->setPower(POWER_TYPE_MANA, value);

    return false;
}

//.modify rage
bool ChatHandler::HandleModifyRage(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldRage = unitTarget->getPower(POWER_TYPE_RAGE);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (rage) from %u to %u on %s (%u)", oldRage, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the power (rage) of %s from %u to %u.", player->getName().c_str(), oldRage, value);
        GreenSystemMessage(player->GetSession(), "%s modify your power (rage) from %u to %u.", session->GetPlayer()->getName().c_str(), oldRage, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (rage) from %u to %u on %s (%u)", oldRage, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the power (rage) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldRage, value);
    }

    unitTarget->setPower(POWER_TYPE_RAGE, value);

    return false;
}

//.modify energy
bool ChatHandler::HandleModifyEnergy(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldEnergy = unitTarget->getPower(POWER_TYPE_ENERGY);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (energy) from %u to %u on %s (%u)", oldEnergy, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the power (energy) of %s from %u to %u.", player->getName().c_str(), oldEnergy, value);
        GreenSystemMessage(player->GetSession(), "%s modify your power (energy) from %u to %u.", session->GetPlayer()->getName().c_str(), oldEnergy, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (energy) from %u to %u on %s (%u)", oldEnergy, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the power (energy) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldEnergy, value);
    }

    unitTarget->setPower(POWER_TYPE_ENERGY, value);

    return false;
}

//.modify runicpower
bool ChatHandler::HandleModifyRunicpower(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

#if VERSION_STRING == WotLK
    const uint32_t value = atol(args);
    const uint32_t oldRunic = unitTarget->getPower(POWER_TYPE_RUNIC_POWER);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (runic) from %u to %u on %s (%u)", oldRunic, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the power (runic) of %s from %u to %u.", player->getName().c_str(), oldRunic, value);
        GreenSystemMessage(player->GetSession(), "%s modify your power (runic) from %u to %u.", session->GetPlayer()->getName().c_str(), oldRunic, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify power (runic) from %u to %u on %s (%u)", oldRunic, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the power (runic) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldRunic, value);
    }

    unitTarget->setPower(POWER_TYPE_RUNIC_POWER, value);
#endif

    return false;
}

//.modify strength
bool ChatHandler::HandleModifyStrength(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldStrength = unitTarget->getStat(STAT_STRENGTH);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (strength) from %u to %u on %s (%u)", oldStrength, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the stat (strength) of %s from %u to %u.", player->getName().c_str(), oldStrength, value);
        GreenSystemMessage(player->GetSession(), "%s modify your stat (strength) from %u to %u.", session->GetPlayer()->getName().c_str(), oldStrength, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (strength) from %u to %u on %s (%u)", oldStrength, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the stat (strength) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldStrength, value);
    }

    unitTarget->setStat(STAT_STRENGTH, value);

    return false;
}

//.modify agility
bool ChatHandler::HandleModifyAgility(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldAgility = unitTarget->getStat(STAT_AGILITY);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (agility) from %u to %u on %s (%u)", oldAgility, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the stat (agility) of %s from %u to %u.", player->getName().c_str(), oldAgility, value);
        GreenSystemMessage(player->GetSession(), "%s modify your stat (agility) from %u to %u.", session->GetPlayer()->getName().c_str(), oldAgility, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (agility) from %u to %u on %s (%u)", oldAgility, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the stat (agility) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldAgility, value);
    }

    unitTarget->setStat(STAT_AGILITY, value);

    return false;
}

//.modify intelligence
bool ChatHandler::HandleModifyIntelligence(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldIntellect = unitTarget->getStat(STAT_INTELLECT);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (intellect) from %u to %u on %s (%u)", oldIntellect, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the stat (intellect) of %s from %u to %u.", player->getName().c_str(), oldIntellect, value);
        GreenSystemMessage(player->GetSession(), "%s modify your stat (intellect) from %u to %u.", session->GetPlayer()->getName().c_str(), oldIntellect, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (intellect) from %u to %u on %s (%u)", oldIntellect, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the stat (intellect) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldIntellect, value);
    }

    unitTarget->setStat(STAT_INTELLECT, value);

    return false;
}

//.modify spirit
bool ChatHandler::HandleModifySpirit(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldSpirit = unitTarget->getStat(STAT_SPIRIT);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (spirit) from %u to %u on %s (%u)", oldSpirit, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the stat (spirit) of %s from %u to %u.", player->getName().c_str(), oldSpirit, value);
        GreenSystemMessage(player->GetSession(), "%s modify your stat (spirit) from %u to %u.", session->GetPlayer()->getName().c_str(), oldSpirit, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify stat (spirit) from %u to %u on %s (%u)", oldSpirit, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the stat (spirit) of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldSpirit, value);
    }

    unitTarget->setStat(STAT_SPIRIT, value);

    return false;
}

//.modify armor
bool ChatHandler::HandleModifyArmor(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldArmor = unitTarget->getResistance(0);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify armor from %u to %u on %s (%u)", oldArmor, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the armor of %s from %u to %u.", player->getName().c_str(), oldArmor, value);
        GreenSystemMessage(player->GetSession(), "%s modify your armor from %u to %u.", session->GetPlayer()->getName().c_str(), oldArmor, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify armor from %u to %u on %s (%u)", oldArmor, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the armor of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldArmor, value);
    }

    unitTarget->setResistance(0, value);

    return false;
}

//.modify holy
bool ChatHandler::HandleModifyHoly(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldHoly = unitTarget->getResistance(1);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify holy from %u to %u on %s (%u)", oldHoly, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the holy of %s from %u to %u.", player->getName().c_str(), oldHoly, value);
        GreenSystemMessage(player->GetSession(), "%s modify your holy from %u to %u.", session->GetPlayer()->getName().c_str(), oldHoly, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify holy from %u to %u on %s (%u)", oldHoly, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the holy of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldHoly, value);
    }

    unitTarget->setResistance(1, value);

    return false;
}

//.modify fire
bool ChatHandler::HandleModifyFire(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldFire = unitTarget->getResistance(2);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify fire from %u to %u on %s (%u)", oldFire, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the fire of %s from %u to %u.", player->getName().c_str(), oldFire, value);
        GreenSystemMessage(player->GetSession(), "%s modify your fire from %u to %u.", session->GetPlayer()->getName().c_str(), oldFire, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify fire from %u to %u on %s (%u)", oldFire, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the fire of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldFire, value);
    }

    unitTarget->setResistance(2, value);

    return false;
}

//.modify nature
bool ChatHandler::HandleModifyNature(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldNature = unitTarget->getResistance(3);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify nature from %u to %u on %s (%u)", oldNature, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the nature of %s from %u to %u.", player->getName().c_str(), oldNature, value);
        GreenSystemMessage(player->GetSession(), "%s modify your nature from %u to %u.", session->GetPlayer()->getName().c_str(), oldNature, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify nature from %u to %u on %s (%u)", oldNature, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the nature of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldNature, value);
    }

    unitTarget->setResistance(3, value);

    return false;
}

//.modify frost
bool ChatHandler::HandleModifyFrost(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldFrost = unitTarget->getResistance(4);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify frost from %u to %u on %s (%u)", oldFrost, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the frost of %s from %u to %u.", player->getName().c_str(), oldFrost, value);
        GreenSystemMessage(player->GetSession(), "%s modify your frost from %u to %u.", session->GetPlayer()->getName().c_str(), oldFrost, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify frost from %u to %u on %s (%u)", oldFrost, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the frost of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldFrost, value);
    }

    unitTarget->setResistance(4, value);

    return false;
}

//.modify shadow
bool ChatHandler::HandleModifyShadow(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldShadow = unitTarget->getResistance(5);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify shadow from %u to %u on %s (%u)", oldShadow, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the shadow of %s from %u to %u.", player->getName().c_str(), oldShadow, value);
        GreenSystemMessage(player->GetSession(), "%s modify your shadow from %u to %u.", session->GetPlayer()->getName().c_str(), oldShadow, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify shadow from %u to %u on %s (%u)", oldShadow, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the shadow of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldShadow, value);
    }

    unitTarget->setResistance(5, value);

    return false;
}

//.modify arcane
bool ChatHandler::HandleModifyArcane(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldArcane = unitTarget->getResistance(5);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify arcane from %u to %u on %s (%u)", oldArcane, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the arcane of %s from %u to %u.", player->getName().c_str(), oldArcane, value);
        GreenSystemMessage(player->GetSession(), "%s modify your arcane from %u to %u.", session->GetPlayer()->getName().c_str(), oldArcane, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify arcane from %u to %u on %s (%u)", oldArcane, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the arcane of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldArcane, value);
    }

    unitTarget->setResistance(6, value);

    return false;
}

//.modify damage
bool ChatHandler::HandleModifyDamage(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const float value = static_cast<float>(atof(args));
    const float oldDamage = unitTarget->getMinDamage();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify arcane from %f to %f on %s (%u)", oldDamage, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the arcane of %s from %f to %f.", player->getName().c_str(), oldDamage, value);
        GreenSystemMessage(player->GetSession(), "%s modify your arcane from %f to %f.", session->GetPlayer()->getName().c_str(), oldDamage, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify arcane from %f to %f on %s (%u)", oldDamage, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the arcane of %s from %f to %f.", creature->GetCreatureProperties()->Name.c_str(), oldDamage, value);
    }


    unitTarget->setMinDamage(value);
    unitTarget->setMaxDamage(value);

    return false;
}

//.modify ap
bool ChatHandler::HandleModifyAp(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldAttackPower = unitTarget->getAttackPower();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify ap from %u to %u on %s (%u)", oldAttackPower, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the ap of %s from %u to %u.", player->getName().c_str(), oldAttackPower, value);
        GreenSystemMessage(player->GetSession(), "%s modify your ap from %u to %u.", session->GetPlayer()->getName().c_str(), oldAttackPower, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify ap from %u to %u on %s (%u)", oldAttackPower, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the ap of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldAttackPower, value);
    }

    unitTarget->setAttackPower(value);

    return false;
}

//.modify rangeap
bool ChatHandler::HandleModifyRangeap(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldRangedAp = unitTarget->getRangedAttackPower();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify rangeap from %u to %u on %s (%u)", oldRangedAp, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the rangeap of %s from %u to %u.", player->getName().c_str(), oldRangedAp, value);
        GreenSystemMessage(player->GetSession(), "%s modify your rangeap from %u to %u.", session->GetPlayer()->getName().c_str(), oldRangedAp, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify rangeap from %u to %u on %s (%u)", oldRangedAp, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the rangeap of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldRangedAp, value);
    }

    unitTarget->setRangedAttackPower(value);

    return false;
}

//.modify scale
bool ChatHandler::HandleModifyScale(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const float value = static_cast<float>(atof(args));
    const float oldScale = unitTarget->getScale();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify scale from %f to %f on %s (%u)", oldScale, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the scale of %s from %f to %f.", player->getName().c_str(), oldScale, value);
        GreenSystemMessage(player->GetSession(), "%s modify your scale from %f to %f.", session->GetPlayer()->getName().c_str(), oldScale, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify scale from %f to %f on %s (%u)", oldScale, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the scale of %s from %f to %f.", creature->GetCreatureProperties()->Name.c_str(), oldScale, value);
    }

    unitTarget->setScale(value);

    return false;
}

//.modify nativedisplayid
bool ChatHandler::HandleModifyNativedisplayid(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldDisplayId = unitTarget->getNativeDisplayId();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify nativedisplayid from %u to %u on %s (%u)", oldDisplayId, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the nativedisplayid of %s from %u to %u.", player->getName().c_str(), oldDisplayId, value);
        GreenSystemMessage(player->GetSession(), "%s modify your nativedisplayid from %u to %u.", session->GetPlayer()->getName().c_str(), oldDisplayId, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify nativedisplayid from %u to %u on %s (%u)", oldDisplayId, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the nativedisplayid of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldDisplayId, value);
    }

    unitTarget->setNativeDisplayId(value);

    return false;
}

//.modify displayid
bool ChatHandler::HandleModifyDisplayid(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldDisplayId = unitTarget->getDisplayId();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify displayid from %u to %u on %s (%u)", oldDisplayId, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the displayid of %s from %u to %u.", player->getName().c_str(), oldDisplayId, value);
        GreenSystemMessage(player->GetSession(), "%s modify your displayid from %u to %u.", session->GetPlayer()->getName().c_str(), oldDisplayId, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify displayid from %u to %u on %s (%u)", oldDisplayId, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the displayid of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldDisplayId, value);
    }

    unitTarget->setDisplayId(value);

    return false;
}

//.modify flags
bool ChatHandler::HandleModifyFlags(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldUnitFlags = unitTarget->getUnitFlags();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify flags from %u to %u on %s (%u)", oldUnitFlags, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the flags of %s from %u to %u.", player->getName().c_str(), oldUnitFlags, value);
        GreenSystemMessage(player->GetSession(), "%s modify your flags from %u to %u.", session->GetPlayer()->getName().c_str(), oldUnitFlags, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify flags from %u to %u on %s (%u)", oldUnitFlags, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the flags of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldUnitFlags, value);
    }

    unitTarget->setUnitFlags(value);

    return false;
}

//.modify faction
bool ChatHandler::HandleModifyFaction(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldFaction = unitTarget->getFactionTemplate();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify faction from %u to %u on %s (%u)", oldFaction, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the faction of %s from %u to %u.", player->getName().c_str(), oldFaction, value);
        GreenSystemMessage(player->GetSession(), "%s modify your faction from %u to %u.", session->GetPlayer()->getName().c_str(), oldFaction, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify faction from %u to %u on %s (%u)", oldFaction, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the faction of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldFaction, value);
    }

    unitTarget->setFactionTemplate(value);

    return false;
}

//.modify dynamicflags
bool ChatHandler::HandleModifyDynamicflags(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldDynamicFlags = unitTarget->getDynamicFlags();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify dynamicflags from %u to %u on %s (%u)", oldDynamicFlags, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the dynamicflags of %s from %u to %u.", player->getName().c_str(), oldDynamicFlags, value);
        GreenSystemMessage(player->GetSession(), "%s modify your dynamicflags from %u to %u.", session->GetPlayer()->getName().c_str(), oldDynamicFlags, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify dynamicflags from %u to %u on %s (%u)", oldDynamicFlags, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the dynamicflags of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldDynamicFlags, value);
    }

    unitTarget->setDynamicFlags(value);

    return false;
}

//.modify happiness
bool ChatHandler::HandleModifyHappiness(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldHappiness = unitTarget->getPower(POWER_TYPE_HAPPINESS);

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify happiness from %u to %u on %s (%u)", oldHappiness, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the happiness of %s from %u to %u.", player->getName().c_str(), oldHappiness, value);
        GreenSystemMessage(player->GetSession(), "%s modify your happiness from %u to %u.", session->GetPlayer()->getName().c_str(), oldHappiness, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify happiness from %u to %u on %s (%u)", oldHappiness, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the happiness of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldHappiness, value);
    }

    unitTarget->setPower(POWER_TYPE_HAPPINESS, value);

    return false;
}

//.modify boundingradius
bool ChatHandler::HandleModifyBoundingradius(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const float value = static_cast<float>(atof(args));
    const float oldBounding = unitTarget->getBoundingRadius();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify boundingradius from %f to %f on %s (%u)", oldBounding, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the boundingradius of %s from %f to %f.", player->getName().c_str(), oldBounding, value);
        GreenSystemMessage(player->GetSession(), "%s modify your boundingradius from %f to %f.", session->GetPlayer()->getName().c_str(), oldBounding, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify boundingradius from %f to %f on %s (%u)", oldBounding, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the boundingradius of %s from %f to %f.", creature->GetCreatureProperties()->Name.c_str(), oldBounding, value);
    }

    unitTarget->setBoundingRadius(value);

    return false;
}

//.modify combatreach
bool ChatHandler::HandleModifyCombatreach(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const float value = static_cast<float>(atof(args));
    const float oldReach = unitTarget->getCombatReach();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify combatreach from %f to %f on %s (%u)", oldReach, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the combatreach of %s from %f to %f.", player->getName().c_str(), oldReach, value);
        GreenSystemMessage(player->GetSession(), "%s modify your combatreach from %f to %f.", session->GetPlayer()->getName().c_str(), oldReach, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify combatreach from %f to %f on %s (%u)", oldReach, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the combatreach of %s from %f to %f.", creature->GetCreatureProperties()->Name.c_str(), oldReach, value);
    }

    unitTarget->setCombatReach(value);

    return false;
}

//.modify emotestate
bool ChatHandler::HandleModifyEmotestate(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldEmote = unitTarget->getEmoteState();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify emotestate from %u to %u on %s (%u)", oldEmote, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the emotestate of %s from %u to %u.", player->getName().c_str(), oldEmote, value);
        GreenSystemMessage(player->GetSession(), "%s modify your emotestate from %u to %u.", session->GetPlayer()->getName().c_str(), oldEmote, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify emotestate from %u to %u on %s (%u)", oldEmote, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the emotestate of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldEmote, value);
    }

    unitTarget->setEmoteState(value);

    return false;
}

//.modify bytes0
bool ChatHandler::HandleModifyBytes0(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldBytes = unitTarget->getBytes0();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes0 from %u to %u on %s (%u)", oldBytes, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the bytes0 of %s from %u to %u.", player->getName().c_str(), oldBytes, value);
        GreenSystemMessage(player->GetSession(), "%s modify your bytes0 from %u to %u.", session->GetPlayer()->getName().c_str(), oldBytes, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes0 from %u to %u on %s (%u)", oldBytes, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the bytes0 of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldBytes, value);
    }

    unitTarget->setBytes0(value);

    return false;
}

//.modify bytes1
bool ChatHandler::HandleModifyBytes1(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldBytes = unitTarget->getBytes1();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes1 from %u to %u on %s (%u)", oldBytes, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the bytes1 of %s from %u to %u.", player->getName().c_str(), oldBytes, value);
        GreenSystemMessage(player->GetSession(), "%s modify your bytes1 from %u to %u.", session->GetPlayer()->getName().c_str(), oldBytes, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes1 from %u to %u on %s (%u)", oldBytes, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the bytes1 of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldBytes, value);
    }

    unitTarget->setBytes1(value);

    return false;
}

//.modify bytes2
bool ChatHandler::HandleModifyBytes2(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = atol(args);
    const uint32_t oldBytes = unitTarget->getBytes2();

    if (unitTarget->isPlayer())
    {
        const auto player = dynamic_cast<Player*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes2 from %u to %u on %s (%u)", oldBytes, value, player->getName().c_str(), player->getGuidLow());

        BlueSystemMessage(session, "You modify the bytes2 of %s from %u to %u.", player->getName().c_str(), oldBytes, value);
        GreenSystemMessage(player->GetSession(), "%s modify your bytes2 from %u to %u.", session->GetPlayer()->getName().c_str(), oldBytes, value);
    }
    else if (unitTarget->isCreature())
    {
        auto creature = dynamic_cast<Creature*>(unitTarget);
        sGMLog.writefromsession(session, "used modify bytes2 from %u to %u on %s (%u)", oldBytes, value, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

        BlueSystemMessage(session, "You modify the bytes2 of %s from %u to %u.", creature->GetCreatureProperties()->Name.c_str(), oldBytes, value);
    }

    unitTarget->setBytes2(value);

    return false;
}
