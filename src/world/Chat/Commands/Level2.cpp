/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

bool ChatHandler::CreateGuildCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* ptarget = GetSelectedPlayer(m_session, true, true);
    if (!ptarget) return false;

    if (ptarget->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", ptarget->GetName());
        return true;
    }

    if (strlen((char*)args) > 75)
    {
        // send message to user
        char buf[256];
        snprintf((char*)buf, 256, "The name was too long by %u", (uint32)strlen(args) - 75);
        SystemMessage(m_session, buf);
        return true;
    }

    for (uint32 i = 0; i < strlen(args); i++)
    {
        if (!isalpha(args[i]) && args[i] != ' ')
        {
            SystemMessage(m_session, "Error, name can only contain chars A-Z and a-z.");
            return true;
        }
    }

    Guild* pGuild = NULL;
    pGuild = objmgr.GetGuildByGuildName(std::string(args));

    if (pGuild)
    {
        RedSystemMessage(m_session, "Guild name is already taken.");
        return true;
    }

    Charter tempCharter(0, ptarget->GetLowGUID(), CHARTER_TYPE_GUILD);
    tempCharter.SignatureCount = 0;
    tempCharter.GuildName = std::string(args);

    pGuild = Guild::Create();
    pGuild->CreateFromCharter(&tempCharter, ptarget->GetSession());
    GreenSystemMessage(m_session, "Guild created");
    sGMLog.writefromsession(m_session, "Created guild '%s'", args);
    return true;
}

bool ChatHandler::HandleItemCommand(const char* args, WorldSession* m_session)
{
    char* pitem = strtok((char*)args, " ");
    if (!pitem)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 item = atoi(pitem);
    int amount = -1;

    char* pamount = strtok(NULL, " ");
    if (pamount)
        amount = atoi(pamount);

    if (amount == -1)
    {
        SystemMessage(m_session, "You need to specify an amount.");
        return true;
    }

    uint32 costid = 0;
    char* pcostid = strtok(NULL, " ");
    if (pcostid)
        costid = atoi(pcostid);

    auto item_extended_cost = (costid > 0) ? sItemExtendedCostStore.LookupEntry(costid) : NULL;
    if (costid > 0 && sItemExtendedCostStore.LookupEntry(costid) == NULL)
    {
        SystemMessage(m_session, "You've entered invalid extended cost id.");
        return true;
    }

    ItemProperties const* tmpItem = sMySQLStore.GetItemProperties(item);

    std::stringstream sstext;
    if (tmpItem)
    {
        std::stringstream ss;
        ss << "INSERT INTO vendors VALUES ('" << pCreature->GetEntry() << "', '" << item << "', '" << amount << "', 0, 0, " << costid << ")" << '\0';
        WorldDatabase.Execute(ss.str().c_str());

        pCreature->AddVendorItem(item, amount, item_extended_cost);

        sstext << "Item '" << item << "' '" << tmpItem->Name.c_str() << "' Added to list";
        if (costid > 0)
            sstext << "with extended cost " << costid;
        sstext << '\0';
    }
    else
    {
        sstext << "Item '" << item << "' Not Found in Database." << '\0';
    }

    sGMLog.writefromsession(m_session, "added item %u to vendor %u", item, pCreature->GetEntry());
    SystemMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleItemRemoveCommand(const char* args, WorldSession* m_session)
{
    char* iguid = strtok((char*)args, " ");
    if (!iguid)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 itemguid = atoi(iguid);
    int slot = pCreature->GetSlotByItemId(itemguid);

    std::stringstream sstext;
    if (slot != -1)
    {
        uint32 creatureId = pCreature->GetEntry();

        std::stringstream ss;
        ss << "DELETE FROM vendors WHERE entry = " << creatureId << " AND item = " << itemguid << '\0';
        WorldDatabase.Execute(ss.str().c_str());

        pCreature->RemoveVendorItem(itemguid);
        ItemProperties const* tmpItem = sMySQLStore.GetItemProperties(itemguid);
        if (tmpItem)
        {
            sstext << "Item '" << itemguid << "' '" << tmpItem->Name.c_str() << "' Deleted from list" << '\0';
        }
        else
        {
            sstext << "Item '" << itemguid << "' Deleted from list" << '\0';
        }
        sGMLog.writefromsession(m_session, "removed item %u from vendor %u", itemguid, creatureId);
    }
    else
    {
        sstext << "Item '" << itemguid << "' Not Found in List." << '\0';
    }

    SystemMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleCastSpellCommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellEntry* spellentry = dbcSpell.LookupEntryForced(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellFactoryMgr.NewSpell(caster, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    sp->prepare(&targets);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellid, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSpellNECommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellId = atol(args);
    SpellEntry* spellentry = dbcSpell.LookupEntryForced(spellId);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellId);

    WorldPacket data;

    data.Initialize(SMSG_SPELL_START);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0);
    data << uint16(0);
    data << uint32(0);
    data << uint16(2);
    data << target->GetGUID();
    //        WPARCEMU_ASSERT(  data.size() == 36);
    m_session->SendPacket(&data);

    data.Initialize(SMSG_SPELL_GO);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0) << uint8(1) << uint8(1);
    data << target->GetGUID();
    data << uint8(0);
    data << uint16(2);
    data << target->GetGUID();
    //        WPARCEMU_ASSERT(  data.size() == 42);
    m_session->SendPacket(&data);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellId, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellId, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSelfCommand(const char* args, WorldSession* m_session)
{
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellEntry* spellentry = dbcSpell.LookupEntryForced(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellFactoryMgr.NewSpell(target, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Target is casting spell %d on himself.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    sp->prepare(&targets);

    switch (target->GetTypeId())
    {
        case TYPEID_PLAYER:
            if (m_session->GetPlayer() != target)
                sGMLog.writefromsession(m_session, "used castself with spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->GetName());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "used castself with spell %d on CREATURE %u [%s], sqlid %u", spellid, static_cast< Creature* >(target)->GetEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}
