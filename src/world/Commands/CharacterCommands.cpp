/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.character clearcooldowns
bool ChatHandler::HandleCharClearCooldownsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target != m_session->GetPlayer())
    {
        sGMLog.writefromsession(m_session, "Cleared all cooldowns for player %s", player_target->GetName());
    }

    uint64 guid = player_target->GetGUID();
    switch (player_target->getClass())
    {
        case WARRIOR:
        {
            player_target->ClearCooldownsOnLine(26, guid);
            player_target->ClearCooldownsOnLine(256, guid);
            player_target->ClearCooldownsOnLine(257, guid);
            BlueSystemMessage(m_session, "Cleared all Warrior cooldowns.");
            break;
        }
        case PALADIN:
        {
            player_target->ClearCooldownsOnLine(594, guid);
            player_target->ClearCooldownsOnLine(267, guid);
            player_target->ClearCooldownsOnLine(184, guid);
            BlueSystemMessage(m_session, "Cleared all Paladin cooldowns.");
            break;
        }
        case HUNTER:
        {
            player_target->ClearCooldownsOnLine(50, guid);
            player_target->ClearCooldownsOnLine(51, guid);
            player_target->ClearCooldownsOnLine(163, guid);
            BlueSystemMessage(m_session, "Cleared all Hunter cooldowns.");
            break;
        }
        case ROGUE:
        {
            player_target->ClearCooldownsOnLine(253, guid);
            player_target->ClearCooldownsOnLine(38, guid);
            player_target->ClearCooldownsOnLine(39, guid);
            BlueSystemMessage(m_session, "Cleared all Rogue cooldowns.");
            break;
        }
        case PRIEST:
        {
            player_target->ClearCooldownsOnLine(56, guid);
            player_target->ClearCooldownsOnLine(78, guid);
            player_target->ClearCooldownsOnLine(613, guid);
            BlueSystemMessage(m_session, "Cleared all Priest cooldowns.");
            break;
        }
        case DEATHKNIGHT:
        {
            player_target->ClearCooldownsOnLine(770, guid);
            player_target->ClearCooldownsOnLine(771, guid);
            player_target->ClearCooldownsOnLine(772, guid);
            BlueSystemMessage(m_session, "Cleared all Death Knight cooldowns.");
            break;
        }
        case SHAMAN:
        {
            player_target->ClearCooldownsOnLine(373, guid);
            player_target->ClearCooldownsOnLine(374, guid);
            player_target->ClearCooldownsOnLine(375, guid);
            BlueSystemMessage(m_session, "Cleared all Shaman cooldowns.");
            break;
        }
        case MAGE:
        {
            player_target->ClearCooldownsOnLine(6, guid);
            player_target->ClearCooldownsOnLine(8, guid);
            player_target->ClearCooldownsOnLine(237, guid);
            BlueSystemMessage(m_session, "Cleared all Mage cooldowns.");
            break;
        }
        case WARLOCK:
        {
            player_target->ClearCooldownsOnLine(355, guid);
            player_target->ClearCooldownsOnLine(354, guid);
            player_target->ClearCooldownsOnLine(593, guid);
            BlueSystemMessage(m_session, "Cleared all Warlock cooldowns.");
            break;
        }
        case DRUID:
        {
            player_target->ClearCooldownsOnLine(573, guid);
            player_target->ClearCooldownsOnLine(574, guid);
            player_target->ClearCooldownsOnLine(134, guid);
            BlueSystemMessage(m_session, "Cleared all Druid cooldowns.");
            break;
        }
    }
 
    return true;
}

//.character demorph
bool ChatHandler::HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->DeMorph();

    return true;
}

//.character levelup
bool ChatHandler::HandleCharLevelUpCommand(const char* args, WorldSession* m_session)
{
    uint32 levels = atoi(args);

    if (levels == 0 || levels < 0)
    {
        RedSystemMessage(m_session, "Command must be in format: .character levelup <level>.");
        RedSystemMessage(m_session, "A negative/0 level is not allowed.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    sGMLog.writefromsession(m_session, "used level up command on %s, with %u levels", player_target->GetName(), levels);

    levels += player_target->getLevel();

    if (levels > PLAYER_LEVEL_CAP)
        levels = PLAYER_LEVEL_CAP;

    auto level_info = objmgr.GetLevelInfo(player_target->getRace(), player_target->getClass(), levels);
    if (level_info == nullptr)
    {
        RedSystemMessage(m_session, "No LevelInfo for Leve: %u, Race: %u, Class: %u", levels, player_target->getRace(), player_target->getClass());
        return true;
    }

    player_target->ApplyLevelInfo(level_info, levels);

    if (player_target->getClass() == WARLOCK)
    {
        std::list<Pet*> summons = player_target->GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            if ((*itr)->IsInWorld() && (*itr)->isAlive())
            {
                (*itr)->setLevel(levels);
                (*itr)->ApplyStatsForLevel();
                (*itr)->UpdateSpellList();
            }
        }
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "%s leveled up to level: %u", player_target->GetName(), levels);
        BlueSystemMessage(player_target->GetSession(), "%s leveled you up to %u.", m_session->GetPlayer()->GetName(), levels);
        sGMLog.writefromsession(m_session, "leveled player %s to level %u", player_target->GetName(), levels);
    }
    else
    {
        BlueSystemMessage(m_session, "You leveled yourself to %u", levels);
    }

    player_target->Social_TellFriendsOnline();

    return true;
}

//.character removeauras
bool ChatHandler::HandleCharRemoveAurasCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "Removing all auras...");
    for (uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
    {
        if (player_target->m_auras[i] != 0)
            player_target->m_auras[i]->Remove();
    }

    if (player_target != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Removed all of %s's auras.", player_target->GetName());

    return true;
}

//.character removesickness
bool ChatHandler::HandleCharRemoveSickessCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->RemoveAura(15007);

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from %s", player_target->GetName());
        BlueSystemMessage(player_target->GetSession(), "%s removed your resurection sickness.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "removed resurrection sickness from player %s", player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from you");
    }

    return true;
}

//.character setallexplored
bool ChatHandler::HandleCharSetAllExploredCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    SystemMessage(m_session, "%s has explored all zones now.", player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s sets all areas as explored for you.", m_session->GetPlayer()->GetName());
    sGMLog.writefromsession(m_session, "sets all areas as explored for player %s", player_target->GetName());

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_LENGTH; ++i)
    {
        player_target->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0xFFFFFFFF);
    }

#ifdef ENABLE_ACHIEVEMENTS
    player_target->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA); // update
#endif
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character add commands
//.character add honorpoints
bool ChatHandler::HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session)
{
    uint32 honor_amount = args ? atol(args) : 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor points added to Player %s.", honor_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor points to your character.", m_session->GetPlayer()->GetName(), honor_amount);
    sGMLog.writefromsession(m_session, "added %u honor points to character %s", honor_amount, player_target->GetName());

    HonorHandler::AddHonorPointsToPlayer(player_target, honor_amount);

    return true;
}


//.character add honorkill
bool ChatHandler::HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session)
{
    uint32 kill_amount = args ? atol(args) : 1;
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor kill points added to Player %s.", kill_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor kill points to your character.", m_session->GetPlayer()->GetName(), kill_amount);
    sGMLog.writefromsession(m_session, "added %u honor kill points to character %s", kill_amount, player_target->GetName());

    player_target->m_killsToday += kill_amount;
    player_target->m_killsLifetime += kill_amount;
    player_target->SetUInt32Value(PLAYER_FIELD_KILLS, uint16(player_target->m_killsToday) | (player_target->m_killsYesterday << 16));
    player_target->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS, player_target->m_killsLifetime);

    return true;
}

//.character add item
bool ChatHandler::HandleCharAddItemCommand(const char* args, WorldSession* m_session)
{
    uint32 itemid = 0;
    uint32 count = 1;
    int32 randomprop = 0;
    int32 numadded = 0;

    if (sscanf(args, "%u %u %d", &itemid, &count, &randomprop) < 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .character add item <itemID>.");
        RedSystemMessage(m_session, "Optional: .character add item <itemID> <amount> <randomprop>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    auto item_proto = ItemPrototypeStorage.LookupEntry(itemid);
    if (item_proto != nullptr)
    {
        numadded -= player_target->GetItemInterface()->GetItemCount(itemid);
        bool result = player_target->GetItemInterface()->AddItemById(itemid, count, randomprop);
        numadded += player_target->GetItemInterface()->GetItemCount(itemid);
        if (result == true)
        {
            if (count == 0)
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u, to %s", item_proto->ItemId, item_proto->Name1, numadded, player_target->GetName());
            }
            else
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u (only %lu added due to full inventory), to %s", item_proto->ItemId, item_proto->Name1, numadded, numadded, player_target->GetName());
            }

            SystemMessage(m_session, "Added item %s (id: %u), quantity %u, to %s's inventory.", GetItemLinkByProto(item_proto, m_session->language).c_str(), item_proto->ItemId, numadded, player_target->GetName());
            SystemMessage(player_target->GetSession(), "%s added item %s, quantity %u, to your inventory.", m_session->GetPlayer()->GetName(), GetItemLinkByProto(item_proto, player_target->GetSession()->language).c_str(), numadded);
        }
        else
        {
            SystemMessage(player_target->GetSession(), "Failed to add item.");
        }
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Item %u is not a valid item!", itemid);
        return true;
    }
}

//.character add itemset
bool ChatHandler::HandleCharAddItemSetCommand(const char* args, WorldSession* m_session)
{
    int32 setid = atoi(args);
    if (!setid)
    {
        RedSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    auto player = GetSelectedPlayer(m_session, true, true);
    if (player == nullptr)
        return true;

    auto item_set_list = objmgr.GetListForItemSet(setid);
    if (!item_set_list)
    {
        RedSystemMessage(m_session, "Invalid item set.");
        return true;
    }

    BlueSystemMessage(m_session, "Searching item set %u...", setid);
    sGMLog.writefromsession(m_session, "used add item set command, set %u, target %s", setid, player->GetName());

    for (std::list<ItemPrototype*>::iterator itr = item_set_list->begin(); itr != item_set_list->end(); ++itr)
    {
        auto item = objmgr.CreateItem((*itr)->ItemId, m_session->GetPlayer());
        if (!item)
            continue;

        if (item->GetProto()->Bonding == ITEM_BIND_ON_PICKUP)
        {
            if (item->GetProto()->Flags & ITEM_FLAG_ACCOUNTBOUND)
                item->AccountBind();
            else
                item->SoulBind();
        }

        if (!player->GetItemInterface()->AddItemToFreeSlot(item))
        {
            m_session->SendNotification("No free slots left!");
            item->DeleteMe();
            return true;
        }
        else
        {
            SystemMessage(m_session, "Added item: %s [%u]", (*itr)->Name1, (*itr)->ItemId);
            SlotResult* le = player->GetItemInterface()->LastSearchResult();
            player->SendItemPushResult(false, true, false, true, le->ContainerSlot, le->Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
        }
    }
    GreenSystemMessage(m_session, "Added set to inventory complete.");
    return true;
}

//.character add copper
bool ChatHandler::HandleCharAddCopperCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many copper you will add.");
        RedSystemMessage(m_session, "10000 = 1 gold, 1000 = 1 silver, 1 = 1 copper.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args);

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);
    uint32 silver = (uint32)std::floor(((float)int32abs(total) / 100.0f)) % 100;
    uint32 copper = int32abs2uint32(total) % 100;

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold, %u silver, %u copper to %s's backpack...", gold, silver, copper, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold, %u silver, %u copper to your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver, %u copper to %s's backpack.", gold, silver, copper , player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold, %u silver, %u copper from %s's backpack...", gold, silver, copper, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold, %u silver, %u copper from your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver, %u copper from %s's backpack.", gold, silver, copper, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}

//.character add silver
bool ChatHandler::HandleCharAddSilverCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many silver you will add.");
        RedSystemMessage(m_session, "1000 = 1 gold, 1 = 1 silver");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args) * 100;

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);
    uint32 silver = (uint32)std::floor(((float)int32abs(total) / 100.0f)) % 100;

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold, %u silver to %s's backpack...", gold, silver, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold, %u silver to your backpack.", m_session->GetPlayer()->GetName(), gold, silver);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver to %s's backpack.", gold, silver, player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold, %u silver from %s's backpack...", gold, silver, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold, %u silver from your backpack.", m_session->GetPlayer()->GetName(), gold, silver);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver from %s's backpack.", gold, silver, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}

//.character add gold
bool ChatHandler::HandleCharAddGoldCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many gold you will add.");
        RedSystemMessage(m_session, "1 = 1 gold.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args) * 10000;

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold to %s's backpack...", gold, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold to your backpack.", m_session->GetPlayer()->GetName(), gold);
            sGMLog.writefromsession(m_session, "added %u gold to %s's backpack.", gold, player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold from %s's backpack...", gold, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold from your backpack.", m_session->GetPlayer()->GetName(), gold);
            sGMLog.writefromsession(m_session, "took %u gold from %s's backpack.", gold, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}
