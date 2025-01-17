/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"

//.pet create
bool ChatHandler::HandlePetCreateCommand(const char* args, WorldSession* m_session)
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
        RedSystemMessage(m_session, "Creature Entry: %u is not a valid endtry!", entry);
        return true;
    }

    if (selected_player->getPet() != nullptr)
    {
        RedSystemMessage(m_session, "You must dismiss your current pet.");
        return true;
    }

    if (!selected_player->findFreeActivePetSlot().has_value())
    {
#if VERSION_STRING < Cata
        if (selected_player->isClassHunter())
            RedSystemMessage(m_session, "You must put your current pet to stables.");
        else
            RedSystemMessage(m_session, "You must dismiss your inactive pet.");
#else
        RedSystemMessage(m_session, "You must have one free active pet slot.");
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
        BlueSystemMessage(m_session, "Pet with entry %u created for player %s.", entry, selected_player->getName().c_str());
        BlueSystemMessage(selected_player->getSession(), "%s created a pet with entry %u for you.", m_session->GetPlayer()->getName().c_str(), entry);

    }
    else
    {
        BlueSystemMessage(m_session, "Pet with entry %u created.", entry);
    }

    return true;
}

//.pet dismiss
bool ChatHandler::HandlePetDismissCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    Pet* selected_pet = nullptr;
    Creature* selected_creature = nullptr;
    if (selected_player != nullptr)
    {
        if (selected_player->getPet() == nullptr)
        {
            RedSystemMessage(m_session, "Player has no pet.");
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
        GreenSystemMessage(m_session, "Dismissed %s's pet.", selected_creature->GetCreatureProperties()->Name.c_str());
        sGMLog.writefromsession(m_session, "used dismiss pet command on creature %s", selected_creature->GetCreatureProperties()->Name.c_str());
    }
    else if (selected_player != nullptr && selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(m_session, "Dismissed %s's pet.", selected_player->getName().c_str());
        SystemMessage(selected_player->getSession(), "%s dismissed your pet.", m_session->GetPlayer()->getName().c_str());
        sGMLog.writefromsession(m_session, "used dismiss pet command on player %s", selected_player->getName().c_str());
    }
    else
    {
        GreenSystemMessage(m_session, "Your pet is dismissed.");
    }

    return true;
}

//.pet rename
bool ChatHandler::HandlePetRenameCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    Pet* selected_pet = selected_player->getPet();
    if (selected_pet == nullptr)
    {
        RedSystemMessage(m_session, "You have no pet.");
        return true;
    }

    if (strlen(args) < 2)
    {
        RedSystemMessage(m_session, "You must specify a name.");
        return true;
    }

    if (selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(m_session, "Renamed %s's pet to %s.", selected_player->getName().c_str(), args);
        SystemMessage(selected_player->getSession(), "%s renamed your pet to %s.", m_session->GetPlayer()->getName().c_str(), args);
        sGMLog.writefromsession(m_session, "renamed %s's pet to %s", selected_player->getName().c_str(), args);
    }
    else
    {
        GreenSystemMessage(m_session, "You renamed you pet to %s.", args);
    }

    selected_pet->rename(args);

    return true;
}

//.pet addspell
bool ChatHandler::HandlePetAddSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    auto* const pet = selected_player->getPet();
    if (pet == nullptr)
    {
        RedSystemMessage(m_session, "%s has no pet.", selected_player->getName().c_str());
        return true;
    }

    if (!*args)
        return false;

    uint32_t SpellId = std::stoul(args);
    SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell id %u.", SpellId);
        return true;
    }

    pet->AddSpell(spell_entry, true);

    GreenSystemMessage(m_session, "Added spell %u to %s's pet.", SpellId, selected_player->getName().c_str());

    return true;
}

//.pet removespell
bool ChatHandler::HandlePetRemoveSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    auto* const pet = selected_player->getPet();
    if (pet == nullptr)
    {
        RedSystemMessage(m_session, "%s has no pet.", selected_player->getName().c_str());
        return true;
    }

    if (!*args)
        return false;

    uint32_t SpellId = std::stoul(args);
    SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell id requested.");
        return true;
    }

    pet->RemoveSpell(SpellId);

    GreenSystemMessage(m_session, "Removed spell %u from %s's pet.", SpellId, selected_player->getName().c_str());

    return true;
}

//.pet setlevel
bool ChatHandler::HandlePetSetLevelCommand(const char* args, WorldSession* m_session)
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
            RedSystemMessage(m_session, "Player has no pet.");
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
        RedSystemMessage(m_session, "You can not set a pet level higher than thew player level!");
        newLevel = selected_player->getLevel();
    }

    selected_pet->setLevel(newLevel);
    selected_pet->setPetExperience(0);
    selected_pet->setPetNextLevelExperience(selected_pet->getNextLevelXp(newLevel));
    selected_pet->applyStatsForLevel();
    selected_pet->UpdateSpellList();

    if (selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(m_session, "Set %s's pet to level %u.", selected_player->getName().c_str(), static_cast<uint32_t>(newLevel));
        SystemMessage(selected_player->getSession(), "%s set your pet to level %u.", m_session->GetPlayer()->getName().c_str(), static_cast<uint32_t>(newLevel));
        sGMLog.writefromsession(m_session, "leveled %s's pet to %u", selected_player->getName().c_str(), static_cast<uint32_t>(newLevel));
    }
    else
    {
        GreenSystemMessage(m_session, "You set your pet to level %u.", static_cast<uint32_t>(newLevel));
    }

    return true;
}
