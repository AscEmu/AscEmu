/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Units/Creatures/Pet.h"

//.pet create
bool ChatHandler::HandlePetCreateCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32 entry = atol(args);
    CreatureProperties const* creature_proto = sMySQLStore.getCreatureProperties(entry);
    if (creature_proto == nullptr)
    {
        RedSystemMessage(m_session, "Creature Entry: %u is not a valid endtry!", entry);
        return true;
    }

    selected_player->DismissActivePets();
    selected_player->RemoveFieldSummon();

    float followangle = -M_PI_FLOAT * 2;
    LocationVector vector(selected_player->GetPosition());
    vector.x += (3 * (cosf(followangle + selected_player->GetOrientation())));
    vector.y += (3 * (sinf(followangle + selected_player->GetOrientation())));

    Pet* pet = objmgr.CreatePet(entry);
    if (!pet->CreateAsSummon(entry, creature_proto, nullptr, selected_player, nullptr, 1, 0, &vector, true))
    {
        pet->DeleteMe();
        return true;
    }

    pet->GetAIInterface()->SetUnitToFollowAngle(followangle);

    if (selected_player != m_session->GetPlayer())
    {
        sGMLog.writefromsession(m_session, "used created pet with entry %u for player %s", entry, selected_player->GetName());
        BlueSystemMessage(m_session, "Pet with entry %u created for player %s.", entry, selected_player->GetName());
        BlueSystemMessage(selected_player->GetSession(), "%s created a pet with entry %u for you.", m_session->GetPlayer()->GetName(), entry);

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
    if (selected_player != nullptr)
    {
        if (selected_player->GetSummon() == nullptr)
        {
            RedSystemMessage(m_session, "Player has no pet.");
            return true;
        }
        else
        {
            selected_player->DismissActivePets();
        }
    }
    else
    {
        // no player selected, see if it is a pet
        Creature* selected_creature = GetSelectedCreature(m_session, false);
        if (selected_creature == nullptr)
            return false;

        if (!selected_creature->IsPet())
            return false;

        selected_pet = static_cast< Pet* >(selected_creature);

        selected_player = selected_pet->GetPetOwner();
        selected_pet->Dismiss();
    }

    if (selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(m_session, "Dismissed %s's pet.", selected_player->GetName());
        SystemMessage(selected_player->GetSession(), "%s dismissed your pet.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "used dismiss pet command on player %s", selected_player->GetName());
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

    Pet* selected_pet = selected_player->GetSummon();
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
        GreenSystemMessage(m_session, "Renamed %s's pet to %s.", selected_player->GetName(), args);
        SystemMessage(selected_player->GetSession(), "%s renamed your pet to %s.", m_session->GetPlayer()->GetName(), args);
        sGMLog.writefromsession(m_session, "renamed %s's pet to %s", selected_player->GetName(), args);
    }
    else
    {
        GreenSystemMessage(m_session, "You renamed you pet to %s.", args);
    }

    selected_pet->Rename(args);

    return true;
}

//.pet addspell
bool ChatHandler::HandlePetAddSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->GetSummon() == nullptr)
    {
        RedSystemMessage(m_session, "%s has no pet.", selected_player->GetName());
        return true;
    }

    if (!*args)
        return false;

    uint32 SpellId = atol(args);
    SpellInfo* spell_entry = sSpellCustomizations.GetSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell id %u.", SpellId);
        return true;
    }

    std::list<Pet*> summons = selected_player->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->AddSpell(spell_entry, true);
    }

    GreenSystemMessage(m_session, "Added spell %u to %s's pet.", SpellId, selected_player->GetName());

    return true;
}

//.pet removespell
bool ChatHandler::HandlePetRemoveSpellCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->GetSummon() == nullptr)
    {
        RedSystemMessage(m_session, "%s has no pet.", selected_player->GetName());
        return true;
    }

    if (!*args)
        return false;

    uint32 SpellId = atol(args);
    SpellInfo* spell_entry = sSpellCustomizations.GetSpellInfo(SpellId);
    if (spell_entry == nullptr)
    {
        RedSystemMessage(m_session, "Invalid spell id requested.");
        return true;
    }

    std::list<Pet*> summons = selected_player->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->RemoveSpell(SpellId);
    }

    GreenSystemMessage(m_session, "Removed spell %u from %s's pet.", SpellId, selected_player->GetName());

    return true;
}

//.pet setlevel
bool ChatHandler::HandlePetSetLevelCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;


    int32 newLevel = atol(args);
    if (newLevel < 1)
        return false;

    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    Pet* selected_pet = nullptr;
    if (selected_player != nullptr)
    {
        selected_pet = selected_player->GetSummon();
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

        if (!selected_creature->IsPet())
            return false;

        selected_pet = static_cast< Pet* >(selected_creature);

        selected_player = selected_pet->GetPetOwner();
    }

    if ((uint32)newLevel > selected_player->getLevel())
    {
        RedSystemMessage(m_session, "You can not set a pet level higher than thew player level!");
        newLevel = selected_player->getLevel();
    }

    selected_pet->setLevel(newLevel);
    selected_pet->setUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
    selected_pet->setUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, selected_pet->GetNextLevelXP(newLevel));
    selected_pet->ApplyStatsForLevel();
    selected_pet->UpdateSpellList();

    if (selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(m_session, "Set %s's pet to level %lu.", selected_player->GetName(), newLevel);
        SystemMessage(selected_player->GetSession(), "%s set your pet to level %lu.", m_session->GetPlayer()->GetName(), newLevel);
        sGMLog.writefromsession(m_session, "leveled %s's pet to %lu", selected_player->GetName(), newLevel);
    }
    else
    {
        GreenSystemMessage(m_session, "You set your pet to level %lu.", newLevel);
    }

    return true;
}
