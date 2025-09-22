/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"

//.pet create
bool ChatCommandHandler::HandlePetCreateCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32_t entry = std::stoul(args);
    CreatureProperties const* creature_proto = sMySQLStore.getCreatureProperties(entry);
    if (creature_proto == nullptr)
    {
        redSystemMessage(m_session, "Creature Entry: {} is not a valid endtry!", entry);
        return true;
    }

    if (selected_player->getPet() != nullptr)
    {
        redSystemMessage(m_session, "You must dismiss your current pet.");
        return true;
    }

    if (!selected_player->findFreeActivePetSlot().has_value())
    {
#if VERSION_STRING < Cata
        if (selected_player->isClassHunter())
            redSystemMessage(m_session, "You must put your current pet to stables.");
        else
            redSystemMessage(m_session, "You must dismiss your inactive pet.");
#else
        redSystemMessage(m_session, "You must have one free active pet slot.");
#endif
        return true;
    }

    float followangle = -M_PI_FLOAT * 2;
    LocationVector vector(selected_player->GetPosition());
    vector.x += (3 * (cosf(followangle + selected_player->GetOrientation())));
    vector.y += (3 * (sinf(followangle + selected_player->GetOrientation())));

    const auto pet = sObjectMgr.createPet(entry, nullptr);
    if (!pet->createAsSummon(creature_proto, nullptr, selected_player, vector, 0, nullptr, 0, PET_TYPE_HUNTER))
    {
        pet->DeleteMe();
        return true;
    }

    if (selected_player != m_session->GetPlayer())
    {
        sGMLog.writefromsession(m_session, "used created pet with entry %u for player %s", entry, selected_player->getName().c_str());
        blueSystemMessage(m_session, "Pet with entry {} created for player {}.", entry, selected_player->getName());
        blueSystemMessage(selected_player->getSession(), "{} created a pet with entry {} for you.", m_session->GetPlayer()->getName(), entry);

    }
    else
    {
        blueSystemMessage(m_session, "Pet with entry {} created.", entry);
    }

    return true;
}

//.pet dismiss
bool ChatCommandHandler::HandlePetDismissCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    Pet* selected_pet = nullptr;
    Creature* selected_creature = nullptr;
    if (selected_player != nullptr)
    {
        if (selected_player->getPet() == nullptr)
        {
            redSystemMessage(m_session, "Player has no pet.");
            return true;
        }
        selected_player->getPet()->unSummon();
    }
    else
    {
        // no player selected, see if it is a pet or if creature has pet
        selected_creature = GetSelectedCreature(m_session, false);
        if (selected_creature == nullptr)
            return false;

        if (selected_creature->isPet())
        {
            selected_pet = dynamic_cast<Pet*>(selected_creature);
            auto* const petOwner = selected_pet->getUnitOwner();
            if (petOwner == nullptr)
                return false;

            selected_player = petOwner->isPlayer() ? selected_pet->getPlayerOwner() : nullptr;
            selected_creature = petOwner->isCreature() ? dynamic_cast<Creature*>(petOwner) : nullptr;
        }
        else
        {
            if (selected_creature->getPet() == nullptr)
                return false;

            // Target is creature but not a pet
            selected_pet = selected_creature->getPet();
            selected_player = selected_pet->getPlayerOwner();
        }

        if (selected_pet == nullptr)
            return false;

        selected_pet->unSummon();
    }

    if (selected_creature != nullptr)
    {
        greenSystemMessage(m_session, "Dismissed {}'s pet.", selected_creature->GetCreatureProperties()->Name);
        sGMLog.writefromsession(m_session, "used dismiss pet command on creature %s", selected_creature->GetCreatureProperties()->Name.c_str());
    }
    else if (selected_player != nullptr && selected_player != m_session->GetPlayer())
    {
        greenSystemMessage(m_session, "Dismissed {}'s pet.", selected_player->getName());
        systemMessage(selected_player->getSession(), "{} dismissed your pet.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "used dismiss pet command on player %s", selected_player->getName().c_str());
    }
    else
    {
        greenSystemMessage(m_session, "Your pet is dismissed.");
    }

    return true;
}

//.pet rename
bool ChatCommandHandler::HandlePetRenameCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    Pet* selected_pet = selected_player->getPet();
    if (selected_pet == nullptr)
    {
        redSystemMessage(m_session, "You have no pet.");
        return true;
    }

    if (strlen(args) < 2)
    {
        redSystemMessage(m_session, "You must specify a name.");
        return true;
    }

    if (selected_player != m_session->GetPlayer())
    {
        greenSystemMessage(m_session, "Renamed {}'s pet to {}.", selected_player->getName(), args);
        systemMessage(selected_player->getSession(), "{} renamed your pet to {}.", m_session->GetPlayer()->getName(), args);
        sGMLog.writefromsession(m_session, "renamed %s's pet to %s", selected_player->getName().c_str(), args);
    }
    else
    {
        greenSystemMessage(m_session, "You renamed your pet to {}.", args);
    }

    selected_pet->rename(args);

    return true;
}

//.pet addspell
bool ChatCommandHandler::HandlePetAddSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    auto* const pet = selected_player->getPet();
    if (pet == nullptr)
    {
        redSystemMessage(m_session, "{} has no pet.", selected_player->getName());
        return true;
    }

    if (!*args)
        return false;

    uint32_t SpellId = std::stoul(args);
    SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        redSystemMessage(m_session, "Invalid spell id {}.", SpellId);
        return true;
    }

    pet->addSpell(spell_entry);

    greenSystemMessage(m_session, "Added spell {} to {}'s pet.", SpellId, selected_player->getName());

    return true;
}

//.pet removespell
bool ChatCommandHandler::HandlePetRemoveSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    auto* const pet = selected_player->getPet();
    if (pet == nullptr)
    {
        redSystemMessage(m_session, "{} has no pet.", selected_player->getName());
        return true;
    }

    if (!*args)
        return false;

    uint32_t SpellId = std::stoul(args);
    SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        redSystemMessage(m_session, "Invalid spell id requested.");
        return true;
    }

    pet->removeSpell(SpellId);

    greenSystemMessage(m_session, "Removed spell {} from {}'s pet.", SpellId, selected_player->getName());

    return true;
}

//.pet setlevel
bool ChatCommandHandler::HandlePetSetLevelCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;


    int32_t newLevel = std::stoul(args);
    if (newLevel < 1)
        return false;

    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    Pet* selected_pet = nullptr;
    if (selected_player != nullptr)
    {
        selected_pet = selected_player->getPet();
        if (selected_pet == nullptr)
        {
            redSystemMessage(m_session, "Player has no pet.");
            return true;
        }
    }
    else
    {
        Creature* selected_creature = GetSelectedCreature(m_session, false);
        if (selected_creature == nullptr)
            return false;

        if (!selected_creature->isPet())
            return false;

        selected_pet = dynamic_cast< Pet* >(selected_creature);

        selected_player = selected_pet->getPlayerOwner();
    }

    if (static_cast<uint32_t>(newLevel) > selected_player->getLevel())
    {
        redSystemMessage(m_session, "You can not set a pet level higher than thew player level!");
        newLevel = selected_player->getLevel();
    }

    selected_pet->setLevel(newLevel);
    selected_pet->setPetExperience(0);
    selected_pet->setPetNextLevelExperience(selected_pet->getNextLevelXp(newLevel));
    selected_pet->applyStatsForLevel();
    selected_pet->updateSpellList();

    if (selected_player != m_session->GetPlayer())
    {
        greenSystemMessage(m_session, "Set {}'s pet to level {}.", selected_player->getName(), static_cast<uint32_t>(newLevel));
        systemMessage(selected_player->getSession(), "{} set your pet to level {}.", m_session->GetPlayer()->getName(), newLevel);
        sGMLog.writefromsession(m_session, "leveled %s's pet to %u", selected_player->getName().c_str(), static_cast<uint32_t>(newLevel));
    }
    else
    {
        greenSystemMessage(m_session, "You set your pet to level {}.", static_cast<uint32_t>(newLevel));
    }

    return true;
}
