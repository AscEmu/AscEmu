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


uint16 GetItemIDFromLink(const char* itemlink, uint32* itemid)
{
    if (itemlink == NULL)
    {
        *itemid = 0;
        return 0;
    }
    uint16 slen = (uint16)strlen(itemlink);
    const char* ptr = strstr(itemlink, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }
    ptr += 7;                       // item id is just past "|Hitem:" (7 bytes)
    *itemid = atoi(ptr);
    ptr = strstr(itemlink, "|r");   // the end of the item link
    if (ptr == NULL)                // item link was invalid
    {
        *itemid = 0;
        return slen;
    }
    ptr += 2;
    return (ptr - itemlink) & 0x0000ffff;
}

bool ChatHandler::HandleAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args || strlen(args) < 4 || strchr(args, '%'))
    {
        m_session->SystemMessage("Announces cannot contain the %% character and must be at least 4 characters.");
        return true;
    }
    char msg[1024];
    std::string input2;
    input2 = sWorld.ann_tagcolor;
    input2 += "[";
    input2 += sWorld.announce_tag;
    input2 += "]";
    input2 += sWorld.ann_gmtagcolor;
    if (sWorld.GMAdminTag)
    {
        if (m_session->CanUseCommand('z')) input2 += "<Admin>";
        else if (m_session->GetPermissionCount()) input2 += "<GM>";
    }
    if (sWorld.NameinAnnounce)
    {
        input2 += "|r" + sWorld.ann_namecolor + "|Hplayer:";
        input2 += m_session->GetPlayer()->GetName();
        input2 += "|h[";
        input2 += m_session->GetPlayer()->GetName();
        input2 += "]|h:|r " + sWorld.ann_msgcolor;
    }
    else if (!sWorld.NameinAnnounce) { input2 += ": "; input2 += sWorld.ann_msgcolor; }
    snprintf((char*)msg, 1024, "%s%s", input2.c_str(), args);
    sWorld.SendWorldText(msg);
    sGMLog.writefromsession(m_session, "used announce command, [%s]", args);
    return true;
}

bool ChatHandler::HandleWAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    char pAnnounce[1024];
    std::string input3;
    input3 = sWorld.ann_tagcolor;
    input3 += "[";
    input3 += sWorld.announce_tag;
    input3 += "]";
    input3 += sWorld.ann_gmtagcolor;
    if (sWorld.GMAdminTag)
    {
        if (m_session->CanUseCommand('z')) input3 += "<Admin>";
        else if (m_session->GetPermissionCount()) input3 += "<GM>";
    }
    if (sWorld.NameinWAnnounce)
    {
        input3 += "|r" + sWorld.ann_namecolor + "[";
        input3 += m_session->GetPlayer()->GetName();
        input3 += "]:|r " + sWorld.ann_msgcolor;
    }
    else if (!sWorld.NameinWAnnounce) { input3 += ": "; input3 += sWorld.ann_msgcolor; }
    snprintf((char*)pAnnounce, 1024, "%s%s", input3.c_str(), args);
    sWorld.SendWorldWideScreenText(pAnnounce);
    sGMLog.writefromsession(m_session, "used wannounce command [%s]", args);
    return true;
}

bool ChatHandler::HandleGPSCommand(const char* args, WorldSession* m_session)
{
    Object* obj;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char buf[400];
    auto at = obj->GetArea();
    if (!at)
    {
        snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f|r",
                 (unsigned int)obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
        SystemMessage(m_session, buf);
        return true;
    }
    auto out_map_id = obj->GetMapId();
    auto out_zone_id = at->zone; // uint32 at_old->ZoneId
    auto out_area_id = at->id; // uint32 at_old->AreaId
    auto out_phase = obj->GetPhase();
    auto out_x = obj->GetPositionX();
    auto out_y = obj->GetPositionY();
    auto out_z = obj->GetPositionZ();
    auto out_o = obj->GetOrientation();
    auto out_area_name = at->area_name[0]; // enUS, hardcoded until locale is implemented properly
    snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffZone: |cff00ff00%u |cffffffffArea: |cff00ff00%u |cffffffffPhase: |cff00ff00%u |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f |cffffffffArea Name: |cff00ff00%s |r",
             out_map_id, out_zone_id, out_area_id, out_phase, out_x, out_y, out_z, out_o, out_area_name);
    SystemMessage(m_session, buf);
    if (obj->obj_movement_info.IsOnTransport())
    {
        SystemMessage(m_session, "Position on Transport:");
        SystemMessage(m_session, "  tX: %f  tY: %f  tZ: %f  tO: %f", obj->GetTransPositionX(), obj->GetTransPositionY(), obj->GetTransPositionZ(), obj->GetTransPositionO());
    }

    // ".gps 1" will save gps info to file logs/gps.log - This probably isn't very multithread safe so don't have many gms spamming it!
    if (args != NULL && *args == '1')
    {
        FILE* gpslog = fopen(FormatOutputString("logs", "gps", false).c_str(), "at");
        if (gpslog)
        {
            fprintf(gpslog, "%d, %u, %u, %f, %f, %f, %f, \'%s\'", out_map_id, out_zone_id, out_area_id, out_x, out_y, out_z, out_o, out_area_name);
            // ".gps 1 comment" will save comment after the gps data
            if (*(args + 1) == ' ')
                fprintf(gpslog, ",%s\n", args + 2);
            else
                fprintf(gpslog, "\n");
            fclose(gpslog);
        }
    }
    return true;
}

bool ChatHandler::HandleSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    // Summon Blocking
    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            BlueSystemMessage(m_session, "Summon blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableSummon(true);
            GreenSystemMessage(m_session, "Summon blocking is now enabled");
        }
        return true;
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            m_session->GetPlayer()->DisableSummon(false);
            GreenSystemMessage(m_session, "Summon blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Summon blocking is already disabled");
        }
        return true;
    }

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        // send message to user
        char buf[256];
        if (!m_session->CanUseCommand('z') && chr->IsSummonDisabled())
        {
            snprintf((char*)buf, 256, "%s has blocked other GMs from summoning them.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            snprintf((char*)buf, 256, "%s is already being teleported.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        snprintf((char*)buf, 256, "You are summoning %s.", chr->GetName());
        SystemMessage(m_session, buf);
        // Don't summon the dead, lol, I see dead people. :P
        // If you do, we better bring them back to life
        if (chr->getDeathState() == 1) // Just died
            chr->RemoteRevive();
        if (chr->getDeathState() != 0) // Not alive
            chr->ResurrectPlayer();
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "You are being summoned by %s.", m_session->GetPlayer()->GetName());
        }
        Player* plr = m_session->GetPlayer();
        if (plr->GetMapMgr() == chr->GetMapMgr())
            chr->_Relocate(plr->GetMapId(), plr->GetPosition(), false, false, plr->GetInstanceID());
        else
        {
            sEventMgr.AddEvent(chr, &Player::EventPortToGM, plr, 0, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
    else
    {
        PlayerInfo* pinfo = objmgr.GetPlayerInfoByName(args);
        if (!pinfo)
        {
            char buf[256];
            snprintf((char*)buf, 256, "Player (%s) does not exist.", args);
            SystemMessage(m_session, buf);
            return true;
        }
        else
        {
            Player* pPlayer = m_session->GetPlayer();
            char query[512];
            snprintf((char*)&query, 512, "UPDATE characters SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetZoneId(), pinfo->guid);
            CharacterDatabase.Execute(query);
            char buf[256];
            snprintf((char*)buf, 256, "(Offline) %s has been summoned.", pinfo->name);
            SystemMessage(m_session, buf);
            return true;
        }
    }
    sGMLog.writefromsession(m_session, "summoned %s on map %u, %f %f %f", args, m_session->GetPlayer()->GetMapId(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    return true;
}

bool ChatHandler::HandleAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    // Appear Blocking
    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            BlueSystemMessage(m_session, "Appear blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableAppear(true);
            GreenSystemMessage(m_session, "Appear blocking is now enabled");
        }
        return true;
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            m_session->GetPlayer()->DisableAppear(false);
            GreenSystemMessage(m_session, "Appear blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Appear blocking is already disabled");
        }
        return true;
    }

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        if (!m_session->CanUseCommand('z') && chr->IsAppearDisabled())
        {
            SystemMessage(m_session, "%s has blocked other GMs from appearing to them.", chr->GetName());
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            SystemMessage(m_session, "%s is already being teleported.", chr->GetName());
            return true;
        }
        SystemMessage(m_session, "Appearing at %s's location.", chr->GetName());
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "%s is appearing to your location.", m_session->GetPlayer()->GetName());
        }
        //m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        //If the GM is on the same map as the player, use the normal safeteleport method
        if (m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID())
            m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        else
            m_session->GetPlayer()->SafeTeleport(chr->GetMapMgr(), chr->GetPosition());
        //The player and GM are not on the same map. We use this method so we can port to BG's (Above method doesn't support them)
    }
    else
    {
        SystemMessage(m_session, "Player (%s) does not exist or is not logged in.", args);
    }
    return true;
}

/// DGM: Get skill level command for getting information about a skill
bool ChatHandler::HandleGetSkillLevelCommand(const char* args, WorldSession* m_session)
{
    uint32 skill = 0;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) return false;
    if (skill > SkillNameManager->maxskill)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    char* SkillName = SkillNameManager->SkillNames[skill];
    if (SkillName == 0)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    if (!plr->_HasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }
    uint32 nobonus = plr->_GetSkillLineCurrent(skill, false);
    uint32 bonus = plr->_GetSkillLineCurrent(skill, true) - nobonus;
    uint32 max = plr->_GetSkillLineMax(skill);
    BlueSystemMessage(m_session, "Player's %s skill has level: %u maxlevel: %u. (+ %u bonus)", SkillName, nobonus, max, bonus);
    return true;
}

#ifdef ENABLE_ACHIEVEMENTS
/**
Handles .achieve complete
.achieve complete id : completes achievement "id" (can be an achievement link) for the selected player
*/
bool ChatHandler::HandleAchievementCompleteCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    uint32 achievement_id = atol(args);
    if (achievement_id == 0)
    {
        achievement_id = GetAchievementIDFromLink(args);
        if (achievement_id == 0)
        {
            if (stricmp(args, "all") == 0)
            {
                plr->GetAchievementMgr().GMCompleteAchievement(m_session, -1);
                SystemMessage(m_session, "All achievements have now been completed for that player.");
                sGMLog.writefromsession(m_session, "completed all achievements for player %s", plr->GetName());
                return true;
            }
            return false;
        }
    }
    if (plr->GetAchievementMgr().GMCompleteAchievement(m_session, achievement_id))
    {
        SystemMessage(m_session, "The achievement has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement %u for player %s", achievement_id, plr->GetName());
    }
    return true;
}

/**
Handles .achieve criteria
.achieve criteria id : completes achievement criteria "id" for the selected player
*/
bool ChatHandler::HandleAchievementCriteriaCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    uint32 criteria_id = atol(args);
    if (criteria_id == 0)
    {
        if (stricmp(args, "all") == 0)
        {
            plr->GetAchievementMgr().GMCompleteCriteria(m_session, -1);
            SystemMessage(m_session, "All achievement criteria have now been completed for that player.");
            sGMLog.writefromsession(m_session, "completed all achievement criteria for player %s", plr->GetName());
            return true;
        }
        return false;
    }
    if (plr->GetAchievementMgr().GMCompleteCriteria(m_session, criteria_id))
    {
        SystemMessage(m_session, "The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement criteria %u for player %s", criteria_id, plr->GetName());
    }
    return true;
}

/**
Handles .achieve reset
.achieve reset id : removes achievement "id" (can be an achievement link) from the selected player
.achieve reset criteria id : removes achievement criteria "id" from the selected player
.achieve reset all : removes all achievement and criteria data from the selected player
*/
bool ChatHandler::HandleAchievementResetCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    bool resetAch = true, resetCri = false;
    int32 achievement_id;
    if (strnicmp(args, "criteria ", 9) == 0)
    {
        achievement_id = atol(args + 9);
        if (achievement_id == 0)
        {
            if (stricmp(args + 9, "all") != 0)
            {
                return false;
            }
            achievement_id = -1;
        }
        resetCri = true;
        resetAch = false;
    }
    else if (stricmp(args, "all") == 0)
    {
        achievement_id = -1;
        resetCri = true;
    }
    else
    {
        achievement_id = atol(args);
        if (achievement_id == 0)
        {
            achievement_id = GetAchievementIDFromLink(args);
            if (achievement_id == 0)
                return false;
        }
    }
    if (resetAch)
        plr->GetAchievementMgr().GMResetAchievement(achievement_id);
    if (resetCri)
        plr->GetAchievementMgr().GMResetCriteria(achievement_id);
    return true;
}
#endif

bool ChatHandler::HandleVehicleEjectPassengerCommand(const char *args, WorldSession* session)
{
    uint32 seat = 0;
    std::stringstream ss(args);
    ss >> seat;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify a seat number.");
        return false;
    }
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->EjectPassengerFromSeat(seat);
    return true;
}

bool ChatHandler::HandleVehicleEjectAllPassengersCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->EjectAllPassengers();
    return true;
}

bool ChatHandler::HandleVehicleInstallAccessoriesCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->InstallAccessories();
    return true;
}

bool ChatHandler::HandleVehicleRemoveAccessoriesCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->RemoveAccessories();
    return true;
}

bool ChatHandler::HandleVehicleAddPassengerCommand(const char *args, WorldSession* session)
{
    std::stringstream ss(args);
    uint32 creature_entry;
    ss >> creature_entry;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify a creature id.");
        return false;
    }
    if (session->GetPlayer()->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = session->GetPlayer()->GetMapMgr()->GetUnit(session->GetPlayer()->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (!u->GetVehicleComponent()->HasEmptySeat())
    {
        RedSystemMessage(session, "That vehicle has no more empty seats.");
        return false;
    }

    CreatureProperties const* cp = sMySQLStore.GetCreatureProperties(creature_entry);
    if (cp == nullptr)
    {
        RedSystemMessage(session, "Creature %u doesn't exist in the database", creature_entry);
        return false;
    }
    Creature* c = u->GetMapMgr()->CreateCreature(creature_entry);
    c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
    c->PushToWorld(u->GetMapMgr());
    c->EnterVehicle(u->GetGUID(), 1);
    return true;
}
