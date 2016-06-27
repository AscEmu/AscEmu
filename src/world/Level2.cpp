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


bool ChatHandler::HandleResetReputationCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        SystemMessage(m_session, "Select a player or yourself first.");
        return true;
    }

    plr->_InitialReputation();
    SystemMessage(m_session, "Done. Relog for changes to take effect.");
    sGMLog.writefromsession(m_session, "used reset reputation for %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleInvincibleCommand(const char* args, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    char msg[100];
    if (chr)
    {
        chr->bInvincible = !chr->bInvincible;
        snprintf(msg, 100, "Invincibility is now %s", chr->bInvincible ? "ON. You may have to leave and re-enter this zone for changes to take effect." : "OFF. Exit and re-enter this zone for this change to take effect.");
    }
    else
    {
        snprintf(msg, 100, "Select a player or yourself first.");
    }
    if (chr != m_session->GetPlayer() && chr)
        sGMLog.writefromsession(m_session, "toggled invincibility on %s", chr->GetName());
    SystemMessage(m_session, msg);
    return true;
}

bool ChatHandler::HandleInvisibleCommand(const char* args, WorldSession* m_session)
{
    char msg[256];
    Player* pChar = m_session->GetPlayer();

    snprintf(msg, 256, "Invisibility and Invincibility are now ");
    if (pChar->m_isGmInvisible)
    {
        pChar->m_isGmInvisible = false;
        pChar->m_invisible = false;
        pChar->bInvincible = false;
        pChar->Social_TellFriendsOnline();
        if (pChar->m_bg)
        {
            pChar->m_bg->RemoveInvisGM();
        }
        snprintf(msg, 256, "%s OFF.", msg);
    }
    else
    {
        pChar->m_isGmInvisible = true;
        pChar->m_invisible = true;
        pChar->bInvincible = true;
        pChar->Social_TellFriendsOffline();
        if (pChar->m_bg)
        {
            pChar->m_bg->AddInvisGM();
        }
        snprintf(msg, 256, "%s ON.", msg);
    }

    pChar->UpdateVisibility();

    snprintf(msg, 256, "%s You may have to leave and re-enter this zone for changes to take effect.", msg);

    GreenSystemMessage(m_session, (const char*)msg);
    return true;
}

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

bool ChatHandler::HandleGOSelect(const char* args, WorldSession* m_session)
{
    GameObject* GObj = NULL;
    GameObject* GObjs = m_session->GetPlayer()->GetSelectedGo();

    std::set<Object*>::iterator Itr = m_session->GetPlayer()->GetInRangeSetBegin();
    std::set<Object*>::iterator Itr2 = m_session->GetPlayer()->GetInRangeSetEnd();
    float cDist = 9999.0f;
    float nDist = 0.0f;
    bool bUseNext = false;

    if (args)
    {
        if (args[0] == '1')
        {
            if (GObjs == NULL)
                bUseNext = true;

            for (;; ++Itr)
            {
                if (Itr == Itr2 && GObj == NULL && bUseNext)
                    Itr = m_session->GetPlayer()->GetInRangeSetBegin();
                else if (Itr == Itr2)
                    break;

                if ((*Itr)->IsGameObject())
                {
                    // Find the current go, move to the next one
                    if (bUseNext)
                    {
                        // Select the first.
                        GObj = static_cast< GameObject* >(*Itr);
                        break;
                    }
                    else
                    {
                        if (((*Itr) == GObjs))
                        {
                            // Found him. Move to the next one, or beginning if we're at the end
                            bUseNext = true;
                        }
                    }
                }
            }
        }
    }
    if (!GObj)
    {
        for (; Itr != Itr2; ++Itr)
        {
            if ((*Itr)->IsGameObject())
            {
                if ((nDist = m_session->GetPlayer()->CalcDistance(*Itr)) < cDist)
                {
                    cDist = nDist;
                    nDist = 0.0f;
                    GObj = static_cast<GameObject*>(*Itr);
                }
            }
        }
    }


    if (GObj == NULL)
    {
        RedSystemMessage(m_session, "No inrange GameObject found.");
        return true;
    }

    m_session->GetPlayer()->m_GM_SelectedGO = GObj->GetGUID();

    //reset last rotation values on selecting a new go.
    m_session->GetPlayer()->go_last_x_rotation = 0.0f;
    m_session->GetPlayer()->go_last_y_rotation = 0.0f;

    GreenSystemMessage(m_session, "Selected GameObject [ %s ] which is %.3f meters away from you.",
        sMySQLStore.GetGameObjectProperties(GObj->GetEntry())->name.c_str(), m_session->GetPlayer()->CalcDistance(GObj));

    return true;
}

bool ChatHandler::HandleGODelete(const char* args, WorldSession* m_session)
{
    GameObject* GObj = m_session->GetPlayer()->GetSelectedGo();
    if (GObj == NULL)
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    if (GObj->IsInBg())
    {
        RedSystemMessage(m_session, "GameObjects can't be deleted in Battlegrounds");
        return true;
    }

    if (GObj->m_spawn != NULL && GObj->m_spawn->entry == GObj->GetEntry())
    {
        uint32 cellx = uint32(((_maxX - GObj->m_spawn->position_x) / _cellSize));
        uint32 celly = uint32(((_maxY - GObj->m_spawn->position_y) / _cellSize));

        if (cellx < _sizeX && celly < _sizeY)
        {
            CellSpawns* sp = GObj->GetMapMgr()->GetBaseMap()->GetSpawnsList(cellx, celly);
            if (sp != NULL)
            {
                for (GameobjectSpawnList::iterator itr = sp->GameobjectSpawns.begin(); itr != sp->GameobjectSpawns.end(); ++itr)
                    if ((*itr) == GObj->m_spawn)
                    {
                        sp->GameobjectSpawns.erase(itr);
                        break;
                    }
            }
            GObj->DeleteFromDB();
            delete GObj->m_spawn;
            GObj->m_spawn = NULL;
        }
    }
    sGMLog.writefromsession(m_session, "deleted game object entry %u on map %u at X:%f Y:%f Z:%f Name %s", GObj->GetEntry(), GObj->GetMapId(), GObj->GetPositionX(), GObj->GetPositionY(), GObj->GetPositionZ(), sMySQLStore.GetGameObjectProperties(GObj->GetEntry())->name.c_str());
    GObj->Despawn(0, 0); // We do not need to delete the object because GameObject::Despawn with no time => ExpireAndDelete() => _Expire() => delete GObj;

    m_session->GetPlayer()->m_GM_SelectedGO = 0;

    return true;
}

bool ChatHandler::HandleGOInfo(const char* args, WorldSession* m_session)
{
    GameObjectProperties const* gameobject_info = nullptr;
    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (!gameobject)
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    SystemMessage(m_session, "%s Information:", MSG_COLOR_SUBWHITE);
    SystemMessage(m_session, "%s SpawnID:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->m_spawn != NULL ? gameobject->m_spawn->id : 0);
    SystemMessage(m_session, "%s Entry:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetEntry());
    SystemMessage(m_session, "%s GUID:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetLowGUID());
    SystemMessage(m_session, "%s Model:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetDisplayId());
    SystemMessage(m_session, "%s State:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetState());
    SystemMessage(m_session, "%s flags:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetFlags());
    SystemMessage(m_session, "%s dynflags:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetUInt32Value(GAMEOBJECT_DYNAMIC));
    SystemMessage(m_session, "%s faction:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetFaction());
    SystemMessage(m_session, "%s phase:%s%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetPhase());

    char gotypetxt[50];
    switch (gameobject->GetType())
    {
        case GAMEOBJECT_TYPE_DOOR:
            strcpy(gotypetxt, "Door");
            break;
        case GAMEOBJECT_TYPE_BUTTON:
            strcpy(gotypetxt, "Button");
            break;
        case GAMEOBJECT_TYPE_QUESTGIVER:
            strcpy(gotypetxt, "Quest Giver");
            break;
        case GAMEOBJECT_TYPE_CHEST:
            strcpy(gotypetxt, "Chest");
            break;
        case GAMEOBJECT_TYPE_BINDER:
            strcpy(gotypetxt, "Binder");
            break;
        case GAMEOBJECT_TYPE_GENERIC:
            strcpy(gotypetxt, "Generic");
            break;
        case GAMEOBJECT_TYPE_TRAP:
            strcpy(gotypetxt, "Trap");
            break;
        case GAMEOBJECT_TYPE_CHAIR:
            strcpy(gotypetxt, "Chair");
            break;
        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            strcpy(gotypetxt, "Spell Focus");
            break;
        case GAMEOBJECT_TYPE_TEXT:
            strcpy(gotypetxt, "Text");
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            strcpy(gotypetxt, "Goober");
            break;
        case GAMEOBJECT_TYPE_TRANSPORT:
            strcpy(gotypetxt, "Transport");
            break;
        case GAMEOBJECT_TYPE_AREADAMAGE:
            strcpy(gotypetxt, "Area Damage");
            break;
        case GAMEOBJECT_TYPE_CAMERA:
            strcpy(gotypetxt, "Camera");
            break;
        case GAMEOBJECT_TYPE_MAP_OBJECT:
            strcpy(gotypetxt, "Map Object");
            break;
        case GAMEOBJECT_TYPE_MO_TRANSPORT:
            strcpy(gotypetxt, "Mo Transport");
            break;
        case GAMEOBJECT_TYPE_DUEL_ARBITER:
            strcpy(gotypetxt, "Duel Arbiter");
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            strcpy(gotypetxt, "Fishing Node");
            break;
        case GAMEOBJECT_TYPE_RITUAL:
            strcpy(gotypetxt, "Ritual");
            break;
        case GAMEOBJECT_TYPE_MAILBOX:
            strcpy(gotypetxt, "Mailbox");
            break;
        case GAMEOBJECT_TYPE_AUCTIONHOUSE:
            strcpy(gotypetxt, "Auction House");
            break;
        case GAMEOBJECT_TYPE_GUARDPOST:
            strcpy(gotypetxt, "Guard Post");
            break;
        case GAMEOBJECT_TYPE_SPELLCASTER:
            strcpy(gotypetxt, "Spell Caster");
            break;
        case GAMEOBJECT_TYPE_MEETINGSTONE:
            strcpy(gotypetxt, "Meeting Stone");
            break;
        case GAMEOBJECT_TYPE_FLAGSTAND:
            strcpy(gotypetxt, "Flag Stand");
            break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
            strcpy(gotypetxt, "Fishing Hole");
            break;
        case GAMEOBJECT_TYPE_FLAGDROP:
            strcpy(gotypetxt, "Flag Drop");
            break;
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            strcpy(gotypetxt, "Destructible Building");
            break;
        default:
            strcpy(gotypetxt, "Unknown.");
            break;
    }
    SystemMessage(m_session, "%s Type:%s%u -- %s", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetType(), gotypetxt);

    SystemMessage(m_session, "%s Distance:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->CalcDistance(m_session->GetPlayer()));

    gameobject_info = sMySQLStore.GetGameObjectProperties(gameobject->GetEntry());
    if (!gameobject_info)
    {
        RedSystemMessage(m_session, "This GameObject doesn't have template, you won't be able to get some information nor to spawn a GO with this entry.");
        return true;
    }


    SystemMessage(m_session, "%s Name:%s%s", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject_info->name.c_str());

    SystemMessage(m_session, "%s Size:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetScale());
    SystemMessage(m_session, "%s Orientation:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetOrientation());
    SystemMessage(m_session, "%s Rotation 0:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetParentRotation(0));
    SystemMessage(m_session, "%s Rotation 1:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetParentRotation(1));
    SystemMessage(m_session, "%s Rotation 2:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetParentRotation(2));
    SystemMessage(m_session, "%s Rotation 3:%s%f", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetParentRotation(3));

    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameobject);

    if (gameobject_info->type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        SystemMessage(m_session, "%s HP:%s%u/%u", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, dgo->GetHP(), dgo->GetMaxHP());
    }

    return true;
}

bool ChatHandler::HandleGOEnable(const char* args, WorldSession* m_session)
{
    GameObject* GObj = m_session->GetPlayer()->GetSelectedGo();
    if (!GObj)
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }
    if (GObj->IsActive())
    {
        // Deactivate
        GObj->Deactivate();
        BlueSystemMessage(m_session, "Gameobject deactivated.");
    }
    else
    {
        // /Activate
        GObj->Activate();
        BlueSystemMessage(m_session, "Gameobject activated.");
    }
    sGMLog.writefromsession(m_session, "activated/deactivated gameobject %s, entry %u", sMySQLStore.GetGameObjectProperties(GObj->GetEntry())->name.c_str(), GObj->GetEntry());
    return true;
}

bool ChatHandler::HandleMountCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "No model specified");
        return true;
    }
    uint32 modelid = atol(args);
    if (!modelid)
    {
        RedSystemMessage(m_session, "No model specified");
        return true;
    }

    Unit* m_target = NULL;
    Player* m_plyr = GetSelectedPlayer(m_session, false, true);
    if (m_plyr)
        m_target = m_plyr;
    else
    {
        Creature* m_crt = GetSelectedCreature(m_session, false);
        if (m_crt)
            m_target = m_crt;
    }

    if (!m_target)
    {
        RedSystemMessage(m_session, "No target found.");
        return true;
    }

    if (m_target->GetMount() != 0)
    {
        RedSystemMessage(m_session, "Target is already mounted.");
        return true;
    }

    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, modelid);
    //m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);

    BlueSystemMessage(m_session, "Now mounted with model %d.", modelid);
    sGMLog.writefromsession(m_session, "used mount command to model %u", modelid);
    return true;
}

bool ChatHandler::HandleGOExport(const char* args, WorldSession* m_session)
{
    /*if (!m_session->GetPlayer()->m_GM_SelectedGO)
        return false;

        std::stringstream name;
        if (*args)
        {
        name << "GO_" << args << ".sql";
        }
        else
        {
        name << "GO_" << m_session->GetPlayer()->m_GM_SelectedGO->GetEntry() << ".sql";
        }

        m_session->GetPlayer()->m_GM_SelectedGO->SaveToFile(name);

        BlueSystemMessage(m_session, "Go saved to: %s", name.str().c_str());*/

    Creature* pCreature = GetSelectedCreature(m_session, true);
    if (!pCreature) return true;
    WorldDatabase.WaitExecute("INSERT INTO creature_names_export SELECT * FROM creature_names WHERE entry = %u", pCreature->GetEntry());
    WorldDatabase.WaitExecute("INSERT INTO creature_proto_export SELECT * FROM creature_proto WHERE entry = %u", pCreature->GetEntry());
    WorldDatabase.WaitExecute("INSERT INTO vendors_export SELECT * FROM vendors WHERE entry = %u", pCreature->GetEntry());
    QueryResult* qr = WorldDatabase.Query("SELECT * FROM vendors WHERE entry = %u", pCreature->GetEntry());
    if (qr != NULL)
    {
        do
        {
            WorldDatabase.WaitExecute("INSERT INTO items_export SELECT * FROM items WHERE entry = %u", qr->Fetch()[1].GetUInt32());
        }
        while (qr->NextRow());
        delete qr;
    }
    return true;
}
