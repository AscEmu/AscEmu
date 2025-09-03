/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Stats.h"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Definitions/PowerType.hpp"

//.modify hp
bool ChatHandler::HandleModifyHp(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = std::stoul(args);
    const uint32_t oldHealth = unitTarget->getHealth();

    sendModifySystemMessage(session, unitTarget, "health", value, oldHealth);

    if (value > unitTarget->getMaxHealth())
        unitTarget->setMaxHealth(value);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldMana = unitTarget->getPower(POWER_TYPE_MANA);

    sendModifySystemMessage(session, unitTarget, "power (mana)", value, oldMana);

    if (value > unitTarget->getMaxPower(POWER_TYPE_MANA))
        unitTarget->setMaxPower(POWER_TYPE_MANA, value);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldRage = unitTarget->getPower(POWER_TYPE_RAGE);

    sendModifySystemMessage(session, unitTarget, "power (rage)", value, oldRage);

    if (value > unitTarget->getMaxPower(POWER_TYPE_RAGE))
        unitTarget->setMaxPower(POWER_TYPE_RAGE, value);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldEnergy = unitTarget->getPower(POWER_TYPE_ENERGY);

    sendModifySystemMessage(session, unitTarget, "power (energy)", value, oldEnergy);

    if (value > unitTarget->getMaxPower(POWER_TYPE_ENERGY))
        unitTarget->setMaxPower(POWER_TYPE_ENERGY, value);

    unitTarget->setPower(POWER_TYPE_ENERGY, value);

    return false;
}

#if VERSION_STRING >= WotLK
//.modify runicpower
bool ChatHandler::HandleModifyRunicpower(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = std::stoul(args);
    const uint32_t oldRunic = unitTarget->getPower(POWER_TYPE_RUNIC_POWER);

    sendModifySystemMessage(session, unitTarget, "stat (runic)", value, oldRunic);

    if (value > unitTarget->getMaxPower(POWER_TYPE_RUNIC_POWER))
        unitTarget->setMaxPower(POWER_TYPE_RUNIC_POWER, value);

    unitTarget->setPower(POWER_TYPE_RUNIC_POWER, value);

    return false;
}
#endif

//.modify strength
bool ChatHandler::HandleModifyStrength(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = std::stoul(args);
    const uint32_t oldStrength = unitTarget->getStat(STAT_STRENGTH);

    sendModifySystemMessage(session, unitTarget, "stat (strength)", value, oldStrength);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldAgility = unitTarget->getStat(STAT_AGILITY);

    sendModifySystemMessage(session, unitTarget, "stat (agility)", value, oldAgility);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldIntellect = unitTarget->getStat(STAT_INTELLECT);

    sendModifySystemMessage(session, unitTarget, "stat (intellect)", value, oldIntellect);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldSpirit = unitTarget->getStat(STAT_SPIRIT);

    sendModifySystemMessage(session, unitTarget, "stat (spirit)", value, oldSpirit);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldArmor = unitTarget->getResistance(0);

    sendModifySystemMessage(session, unitTarget, "armor", value, oldArmor);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldHoly = unitTarget->getResistance(1);

    sendModifySystemMessage(session, unitTarget, "holy", value, oldHoly);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldFire = unitTarget->getResistance(2);

    sendModifySystemMessage(session, unitTarget, "fire", value, oldFire);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldNature = unitTarget->getResistance(3);

    sendModifySystemMessage(session, unitTarget, "nature", value, oldNature);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldFrost = unitTarget->getResistance(4);

    sendModifySystemMessage(session, unitTarget, "frost", value, oldFrost);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldShadow = unitTarget->getResistance(5);

    sendModifySystemMessage(session, unitTarget, "shadow", value, oldShadow);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldArcane = unitTarget->getResistance(5);

    sendModifySystemMessage(session, unitTarget, "arcane", value, oldArcane);

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

    sendModifySystemMessage(session, unitTarget, "damage", value, oldDamage);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldAttackPower = unitTarget->getAttackPower();

    sendModifySystemMessage(session, unitTarget, "ap", value, oldAttackPower);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldRangedAp = unitTarget->getRangedAttackPower();

    sendModifySystemMessage(session, unitTarget, "rangeap", value, oldRangedAp);

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

    sendModifySystemMessage(session, unitTarget, "scale", value, oldScale);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldDisplayId = unitTarget->getNativeDisplayId();

    sendModifySystemMessage(session, unitTarget, "nativedisplayid", value, oldDisplayId);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldDisplayId = unitTarget->getDisplayId();

    sendModifySystemMessage(session, unitTarget, "displayid", value, oldDisplayId);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldUnitFlags = unitTarget->getUnitFlags();

    sendModifySystemMessage(session, unitTarget, "flags", value, oldUnitFlags);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldFaction = unitTarget->getFactionTemplate();

    sendModifySystemMessage(session, unitTarget, "faction", value, oldFaction);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldDynamicFlags = unitTarget->getDynamicFlags();

    sendModifySystemMessage(session, unitTarget, "dynamicflags", value, oldDynamicFlags);

    unitTarget->setDynamicFlags(value);

    return false;
}

#if VERSION_STRING < Cata
//.modify happiness
bool ChatHandler::HandleModifyHappiness(const char* args, WorldSession* session)
{
    if (!args)
        return true;

    const auto unitTarget = GetSelectedUnit(session, true);
    if (unitTarget == nullptr)
        return true;

    const uint32_t value = std::stoul(args);
    const uint32_t oldHappiness = unitTarget->getPower(POWER_TYPE_HAPPINESS);

    sendModifySystemMessage(session, unitTarget, "happiness", value, oldHappiness);

    unitTarget->setPower(POWER_TYPE_HAPPINESS, value);

    return false;
}
#endif

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

    sendModifySystemMessage(session, unitTarget, "boundingradius", value, oldBounding);

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

    sendModifySystemMessage(session, unitTarget, "combatreach", value, oldReach);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldEmote = unitTarget->getEmoteState();

    sendModifySystemMessage(session, unitTarget, "emotestates", value, oldEmote);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldBytes = unitTarget->getBytes0();

    sendModifySystemMessage(session, unitTarget, "bytes0", value, oldBytes);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldBytes = unitTarget->getBytes1();

    sendModifySystemMessage(session, unitTarget, "bytes1", value, oldBytes);

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

    const uint32_t value = std::stoul(args);
    const uint32_t oldBytes = unitTarget->getBytes2();

    sendModifySystemMessage(session, unitTarget, "bytes2", value, oldBytes);

    unitTarget->setBytes2(value);

    return false;
}

void ChatHandler::sendModifySystemMessage(WorldSession* session, Unit* unitTarget, std::string modType, uint32_t newValue, uint32_t oldValue)
{
    if (unitTarget->isPlayer())
    {
        if (const auto player = dynamic_cast<Player*>(unitTarget))
        {
            sGMLog.writefromsession(session, "used modify %s from %u to %u on %s (%u)", modType.c_str(), oldValue, newValue, player->getName().c_str(), player->getGuidLow());

            blueSystemMessage(session, "You modify '{}' of {} from {} to {}.", modType, player->getName(), oldValue, newValue);
            GreenSystemMessage(player->getSession(), "%s modify your %s from %u to %u.", modType.c_str(), session->GetPlayer()->getName().c_str(), oldValue, newValue);
        }
    }
    else if (unitTarget->isCreature())
    {
        if (auto creature = dynamic_cast<Creature*>(unitTarget))
        {
            sGMLog.writefromsession(session, "used modify %s from %u to %u on %s (%u)", modType.c_str(), oldValue, newValue, creature->GetCreatureProperties()->Name.c_str(), creature->GetCreatureProperties()->Id);

            blueSystemMessage(session, "You modify '{}' of {} from {} to {}.", modType, creature->GetCreatureProperties()->Name, oldValue, newValue);
        }
    }
}
