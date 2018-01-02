/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Config/Config.h"
#include "Server/Packets/Handlers/HonorHandler.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Objects/ObjectMgr.h"
#include "Spell/Customization/SpellCustomizations.hpp"

#if VERSION_STRING != Cata
void WorldSession::HandleSetVisibleRankOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 4);
    uint32_t ChosenRank;
    recvData >> ChosenRank;
    if (ChosenRank == 0xFFFFFFFF)
        _player->SetChosenTitle(0);
    else if (_player->HasTitle(static_cast<RankTitles>(ChosenRank)))
        _player->SetChosenTitle(ChosenRank);
}

void HonorHandler::AddHonorPointsToPlayer(Player* pPlayer, uint32 uAmount)
{
    pPlayer->AddHonor(uAmount, true);
}

int32 HonorHandler::CalculateHonorPointsForKill(uint32 playerLevel, uint32 victimLevel)
{
    uint32 kLevel = playerLevel;
    uint32 vLevel = victimLevel;

    uint32 kGrey;

    if (kLevel > 5 && kLevel < 40)
    {
        kGrey = kLevel - 5 - float2int32(std::floor(((float)kLevel) / 10.0f));
    }
    else
    {
        kGrey = kLevel - 1 - float2int32(std::floor(((float)kLevel) / 5.0f));
    }

    if (vLevel <= kGrey)
        return 0;

    float honor_points = -0.53177f + 0.59357f * exp((kLevel + 23.54042f) / 26.07859f);
    honor_points *= World::getSingleton().settings.getFloatRate(RATE_HONOR);
    return float2int32(honor_points);
}

void HonorHandler::OnPlayerKilled(Player* pPlayer, Player* pVictim)
{
    if (pVictim == nullptr)
        return;

    if (pVictim->m_honorless)
        return;

    if (pPlayer->m_bg)
    {
        if (pVictim->m_bgTeam == pPlayer->m_bgTeam)
            return;

        // patch 2.4, players killed >50 times in battlegrounds won't be worth honor for the rest of that bg
        if (pVictim->m_bgScore.Deaths >= 50)
            return;
    }
    else
    {
        if (pPlayer->GetTeam() == pVictim->GetTeam())
            return;
    }

    // Calculate points
    int32_t points = 0;
    if (pPlayer != pVictim)
        points = CalculateHonorPointsForKill(pPlayer->getLevel(), pVictim->getLevel());

    if (points > 0)
    {
        if (pPlayer->m_bg)
        {
            std::lock_guard<std::recursive_mutex> lock(pPlayer->m_bg->GetMutex());

            // hackfix for battlegrounds (since the groups there are disabled, we need to do this manually)
            std::vector<Player*> toadd;
            uint32 t = pPlayer->m_bgTeam;
            toadd.reserve(15);        // shouldn't have more than this
            std::set<Player*> * s = &pPlayer->m_bg->m_players[t];

            for (std::set<Player*>::iterator itr = s->begin(); itr != s->end(); ++itr)
            {
                // Also check that the player is in range, and the player is alive.
                if ((*itr) == pPlayer || ((*itr)->isAlive() && (*itr)->isInRange(pPlayer, 100.0f)))
                    toadd.push_back(*itr);
            }

            if (toadd.size() > 0)
            {
                uint32 pts = points / (uint32)toadd.size();
                for (std::vector<Player*>::iterator vtr = toadd.begin(); vtr != toadd.end(); ++vtr)
                {
                    AddHonorPointsToPlayer(*vtr, pts);

                    (*vtr)->m_killsToday++;
                    (*vtr)->m_killsLifetime++;
                    pPlayer->m_bg->HookOnHK(*vtr);
                    if (pVictim)
                    {
                        // Send PVP credit
                        WorldPacket data(SMSG_PVP_CREDIT, 12);
                        uint32 pvppoints = pts * 10;
                        data << pvppoints;
                        data << pVictim->GetGUID();
                        data << uint32(pVictim->GetPVPRank());
                        (*vtr)->GetSession()->SendPacket(&data);
                    }
                }
            }
        }
        else
        {
            std::set<Player*> contributors;
            // First loop: Get all the people in the attackermap.
            pVictim->updateInRangeOppositeFactionSet();
            for (const auto& itr : pVictim->getInRangeOppositeFactionSet())
            {
                if (!itr || !itr->IsPlayer())
                    continue;

                bool added = false;
                Player* plr = static_cast<Player*>(itr);
                if (pVictim->CombatStatus.m_attackers.find(plr->GetGUID()) != pVictim->CombatStatus.m_attackers.end())
                {
                    added = true;
                    contributors.insert(plr);
                }

                if (added && plr->GetGroup())
                {
                    Group* pGroup = plr->GetGroup();
                    uint32 groups = pGroup->GetSubGroupCount();
                    for (uint32 i = 0; i < groups; i++)
                    {
                        SubGroup* sg = pGroup->GetSubGroup(i);
                        if (!sg) continue;

                        for (GroupMembersSet::iterator itr2 = sg->GetGroupMembersBegin(); itr2 != sg->GetGroupMembersEnd(); ++itr2)
                        {
                            PlayerInfo* pi = (*itr2);
                            Player* gm = objmgr.GetPlayer(pi->guid);
                            if (!gm) continue;

                            if (gm->isInRange(pVictim, 100.0f))
                                contributors.insert(gm);
                        }
                    }
                }
            }

            for (std::set<Player*>::iterator itr = contributors.begin(); itr != contributors.end(); ++itr)
            {
                Player* pAffectedPlayer = (*itr);
                if (!pAffectedPlayer) continue;

                pAffectedPlayer->m_killsToday++;
                pAffectedPlayer->m_killsLifetime++;
                if (pAffectedPlayer->m_bg)
                    pAffectedPlayer->m_bg->HookOnHK(pAffectedPlayer);

                int32 contributorpts = points / (int32)contributors.size();
                AddHonorPointsToPlayer(pAffectedPlayer, contributorpts);

                sHookInterface.OnHonorableKill(pAffectedPlayer, pVictim);

                WorldPacket data(SMSG_PVP_CREDIT, 12);
                uint32 pvppoints = contributorpts * 10; // Why *10?
                data << pvppoints;
                data << pVictim->GetGUID();
                data << uint32(pVictim->GetPVPRank());
                pAffectedPlayer->GetSession()->SendPacket(&data);

                int PvPToken = Config.MainConfig.getIntDefault("Player", "EnablePvPToken", 0);
                if (PvPToken > 0)
                {
                    int PvPTokenID = Config.MainConfig.getIntDefault("Player", "PvPTokenID", 0);
                    if (PvPTokenID > 0)
                    {
                        Item* PvPTokenItem = objmgr.CreateItem(PvPTokenID, pAffectedPlayer);
                        if (PvPTokenItem)
                        {
                            PvPTokenItem->SoulBind();
                            if (!pAffectedPlayer->GetItemInterface()->AddItemToFreeSlot(PvPTokenItem))
                                PvPTokenItem->DeleteMe();
                        }
                    }
                }
                if (pAffectedPlayer->GetZoneId() == 3518)
                {
                    // Add Halaa Battle Token
                    SpellInfo* pvp_token_spell = sSpellCustomizations.GetSpellInfo(pAffectedPlayer->IsTeamHorde() ? 33004 : 33005);
                    pAffectedPlayer->CastSpell(pAffectedPlayer, pvp_token_spell, true);
                }
                // If we are in Hellfire Peninsula <http://www.wowwiki.com/Hellfire_Peninsula#World_PvP_-_Hellfire_Fortifications>
                if (pAffectedPlayer->GetZoneId() == 3483)
                {
                    // Hellfire Horde Controlled Towers
                    /*if (pAffectedPlayer->GetMapMgr()->GetWorldState(2478) != 3 && pAffectedPlayer->GetTeam() == 1)
                        return;

                        // Hellfire Alliance Controlled Towers
                        if (pAffectedPlayer->GetMapMgr()->GetWorldState(2476) != 3 && pAffectedPlayer->GetTeam() == 0)
                        return;
                        */

                    // Add Mark of Thrallmar/Honor Hold
                    SpellInfo* pvp_token_spell = sSpellCustomizations.GetSpellInfo(pAffectedPlayer->IsTeamHorde() ? 32158 : 32155);
                    pAffectedPlayer->CastSpell(pAffectedPlayer, pvp_token_spell, true);
                }
            }
        }
    }
}

void HonorHandler::RecalculateHonorFields(Player* pPlayer)
{
    if (pPlayer != nullptr)
        pPlayer->UpdatePvPCurrencies();
}
#endif
