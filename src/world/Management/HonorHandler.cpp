/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Management/HonorHandler.h"

#include "Group.h"
#include "Objects/Item.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.hpp"
#include "Server/World.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/SpellMgr.hpp"
#include "Utilities/Narrow.hpp"

void HonorHandler::AddHonorPointsToPlayer(Player* pPlayer, uint32_t uAmount)
{
    pPlayer->addHonor(uAmount, true);
}

int32_t HonorHandler::CalculateHonorPointsForKill(uint32_t playerLevel, uint32_t victimLevel)
{
    uint32_t kLevel = playerLevel;
    uint32_t vLevel = victimLevel;

    uint32_t kGrey;

    if (kLevel > 5 && kLevel < 40)
    {
        kGrey = kLevel - 5 - Util::float2int32(std::floor(((float)kLevel) / 10.0f));
    }
    else
    {
        kGrey = kLevel - 1 - Util::float2int32(std::floor(((float)kLevel) / 5.0f));
    }

    if (vLevel <= kGrey)
        return 0;

    float honor_points = -0.53177f + 0.59357f * exp((kLevel + 23.54042f) / 26.07859f);
    honor_points *= sWorld.settings.getFloatRate(RATE_HONOR);
    return Util::float2int32(honor_points);
}

void HonorHandler::OnPlayerKilled(Player* pPlayer, Player* pVictim)
{
    if (pVictim == nullptr)
        return;

    if (pVictim->getHonorless())
        return;

    if (pPlayer->getBattleground())
    {
        if (pVictim->getBgTeam() == pPlayer->getBgTeam())
            return;

        // patch 2.4, players killed >50 times in battlegrounds won't be worth honor for the rest of that bg
        if (pVictim->m_bgScore.Deaths >= 50)
            return;
    }
    else
    {
        if (pPlayer->getTeam() == pVictim->getTeam())
            return;
    }

    // Calculate points
    int32_t points = 0;
    if (pPlayer != pVictim)
        points = CalculateHonorPointsForKill(pPlayer->getLevel(), pVictim->getLevel());

    if (points > 0)
    {
        if (pPlayer->getBattleground())
        {
            std::lock_guard<std::recursive_mutex> lock(pPlayer->getBattleground()->GetMutex());

            // hackfix for battlegrounds (since the groups there are disabled, we need to do this manually)
            std::vector<Player*> toadd;
            uint32_t t = pPlayer->getBgTeam();
            toadd.reserve(15);        // shouldn't have more than this
            std::set<Player*> * s = &pPlayer->getBattleground()->m_players[t];

            for (std::set<Player*>::iterator itr = s->begin(); itr != s->end(); ++itr)
            {
                // Also check that the player is in range, and the player is alive.
                if ((*itr) == pPlayer || ((*itr)->isAlive() && (*itr)->isInRange(pPlayer, 100.0f)))
                    toadd.push_back(*itr);
            }

            if (toadd.size() > 0)
            {
                uint32_t pts = points / (uint32_t)toadd.size();
                for (std::vector<Player*>::iterator vtr = toadd.begin(); vtr != toadd.end(); ++vtr)
                {
                    AddHonorPointsToPlayer(*vtr, pts);

                    (*vtr)->incrementKills();
                    pPlayer->getBattleground()->HookOnHK(*vtr);

                    // Send PVP credit
                    uint32_t pvppoints = pts * 10;
                    (*vtr)->sendPvpCredit(pvppoints, pVictim->getGuid(), pVictim->getPvpRank());
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
                if (!itr || !itr->isPlayer())
                    continue;

                bool added = false;
                Player* plr = static_cast<Player*>(itr);
                if (pVictim->getCombatHandler().isInCombatWithPlayer(plr))
                {
                    added = true;
                    contributors.insert(plr);
                }

                if (added && plr->getGroup())
                {
                    const auto group = plr->getGroup();
                    uint32_t groups = group->GetSubGroupCount();
                    for (uint32_t i = 0; i < groups; i++)
                    {
                        SubGroup* sg = group->GetSubGroup(i);
                        if (!sg)
                            continue;

                        for (const auto itr2 : sg->getGroupMembers())
                        {
                            Player* gm = sObjectMgr.getPlayer(itr2->guid);
                            if (!gm)
                                continue;

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

                pAffectedPlayer->incrementKills();
                if (pAffectedPlayer->getBattleground())
                    pAffectedPlayer->getBattleground()->HookOnHK(pAffectedPlayer);

                int32_t contributorpts = points / (int32_t)contributors.size();
                AddHonorPointsToPlayer(pAffectedPlayer, contributorpts);

                sHookInterface.OnHonorableKill(pAffectedPlayer, pVictim);

                uint32_t pvppoints = contributorpts * 10; // Why *10?

                pAffectedPlayer->sendPvpCredit(pvppoints, pVictim->getGuid(), pVictim->getPvpRank());

                const auto PvPToken = worldConfig.player.enablePvPToken;
                if (PvPToken)
                {
                    const auto PvPTokenID = worldConfig.player.pvpTokenId;
                    if (PvPTokenID > 0)
                    {
                        auto PvPTokenItem = sObjectMgr.createItem(PvPTokenID, pAffectedPlayer);
                        if (PvPTokenItem)
                        {
                            PvPTokenItem->addFlags(ITEM_FLAG_SOULBOUND);
                            pAffectedPlayer->getItemInterface()->AddItemToFreeSlot(std::move(PvPTokenItem));
                        }
                    }
                }
                if (pAffectedPlayer->getZoneId() == 3518)
                {
                    // Add Halaa Battle Token
                    SpellInfo const* pvp_token_spell = sSpellMgr.getSpellInfo(pAffectedPlayer->isTeamHorde() ? 33004 : 33005);
                    pAffectedPlayer->castSpell(pAffectedPlayer, pvp_token_spell, true);
                }
                // If we are in Hellfire Peninsula <http://www.wowwiki.com/Hellfire_Peninsula#World_PvP_-_Hellfire_Fortifications>
                if (pAffectedPlayer->getZoneId() == 3483)
                {
                    // Hellfire Horde Controlled Towers
                    /*if (pAffectedPlayer->getWorldMap()->GetWorldState(2478) != 3 && pAffectedPlayer->getTeam() == TEAM_HORDE)
                        return;

                        // Hellfire Alliance Controlled Towers
                        if (pAffectedPlayer->getWorldMap()->GetWorldState(2476) != 3 && pAffectedPlayer->getTeam() == TEAM_ALLIANCE)
                        return;
                        */

                    // Add Mark of Thrallmar/Honor Hold
                    SpellInfo const* pvp_token_spell = sSpellMgr.getSpellInfo(pAffectedPlayer->isTeamHorde() ? 32158 : 32155);
                    pAffectedPlayer->castSpell(pAffectedPlayer, pvp_token_spell, true);
                }
            }
        }
    }
}

void HonorHandler::RecalculateHonorFields(Player* pPlayer)
{
    if (pPlayer != nullptr)
        pPlayer->updatePvPCurrencies();
}
