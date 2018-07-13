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
#include "Management/Item.h"
#include "Storage/WorldStrings.h"
#include "Management/ItemInterface.h"
#include "Management/ArenaTeam.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Management/Guild/GuildDefinitions.h"
#include "Server/Packets/CmsgGuildInvite.h"
#include "Server/Packets/CmsgGuildInfoText.h"
#include "Server/Packets/CmsgGuildPromote.h"
#include "Server/Packets/CmsgGuildDemote.h"
#include "Server/Packets/CmsgGuildRemove.h"
#include "Server/Packets/CmsgGuildLeader.h"
#include "Server/Packets/CmsgGuildMotd.h"
#include "Server/Packets/CmsgGuildAddRank.h"
#include "Server/Packets/CmsgGuildSetPublicNote.h"
#include "Server/Packets/CmsgGuildSetOfficerNote.h"
#include "Server/Packets/MsgSaveGuildEmblem.h"
#include "Server/Packets/CmsgPetitionBuy.h"
#include "Server/Packets/CmsgPetitionShowSignatures.h"
#include "Server/Packets/CmsgPetitionQuery.h"
#include "Server/Packets/CmsgOfferPetition.h"
#include "Server/Packets/CmsgPetitionSign.h"
#include "Server/Packets/SmsgPetitionSignResult.h"
#include "Server/Packets/MsgPetitionDecline.h"
#include "Server/Packets/CmsgTurnInPetition.h"
#include "Server/Packets/MsgPetitionRename.h"
#include "Server/Packets/CmsgGuildBankBuyTab.h"
#include "Server/Packets/CmsgGuildBankUpdateTab.h"
#include "Server/Packets/CmsgGuildBankWithdrawMoney.h"
#include "Server/Packets/CmsgGuildBankDepositMoney.h"
#include "Server/Packets/CmsgGuildBankerActivate.h"
#include "Server/Packets/CmsgGuildBankQueryTab.h"
#include "Server/Packets/MsgGuildBankLogQuery.h"
#include "Server/Packets/MsgQueryGuildBankText.h"
#include "Server/Packets/CmsgSetGuildBankText.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Management/GuildMgr.h"

using namespace AscEmu::Packets;

#if VERSION_STRING != Cata

// Charter part
void WorldSession::HandleCharterBuy(WorldPacket& recv_data)
{
    CmsgPetitionBuy recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Creature* crt = _player->GetMapMgr()->GetCreature(recv_packet.creatureGuid.getGuidLow());
    if (!crt)
    {
        Disconnect();
        return;
    }

    if (!crt->isTabardDesigner())
    {
        uint32 arena_type = recv_packet.arenaIndex - 1;
        if (arena_type > 2)
            return;

        if (_player->m_arenaTeams[arena_type])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(71));
            return;
        }

        ArenaTeam* t = objmgr.GetArenaTeamByName(recv_packet.name, arena_type);
        if (t != nullptr)
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (objmgr.GetCharterByName(recv_packet.name, static_cast<CharterTypes>(recv_packet.arenaIndex)))
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (_player->m_charters[recv_packet.arenaIndex])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(73));
            return;
        }

        if (_player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
        {
            //\todo Replace by LocalizedWorldSrv(..)
            SendNotification("You must be at least level %u to buy Arena charter", PLAYER_ARENA_MIN_LEVEL);
            return;
        }

        static uint32 item_ids[] = { CharterEntry::TwoOnTwo, CharterEntry::ThreeOnThree, CharterEntry::FiveOnFive };
        static uint32 costs[] = { CharterCost::TwoOnTwo, CharterCost::ThreeOnThree, CharterCost::FiveOnFive };

        if (!_player->HasGold(costs[arena_type]))
            return;            // error message needed here

        ItemProperties const* ip = sMySQLStore.getItemProperties(item_ids[arena_type]);
        ARCEMU_ASSERT(ip != NULL);
        SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(ip);
        if (res.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        uint8 error = _player->GetItemInterface()->CanReceiveItem(ip, 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            // Create the item and charter
            Item* i = objmgr.CreateItem(item_ids[arena_type], _player);
            Charter* c = objmgr.CreateCharter(_player->getGuidLow(), static_cast<CharterTypes>(recv_packet.arenaIndex));
            if (i == NULL || c == NULL)
                return;

            c->GuildName = recv_packet.name;
            c->ItemGuid = i->getGuid();

            c->PetitionSignerCount = recv_packet.signerCount;

            i->setStackCount(1);
            i->addFlags(ITEM_FLAG_SOULBOUND);
            i->setEnchantmentId(0, c->GetID());
            i->setPropertySeed(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(i))
            {
                c->Destroy();
                i->DeleteMe();
                return;
            }

            c->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1, i->getEntry(), i->getPropertySeed(), i->getRandomPropertiesId(), i->getStackCount());

            _player->ModGold(-(int32)costs[arena_type]);
            _player->m_charters[recv_packet.arenaIndex] = c;
            _player->SaveToDB(false);
        }
    }
    else
    {
        if (!_player->HasGold(1000))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* g = sGuildMgr.getGuildByName(recv_packet.name);
        Charter* c = objmgr.GetCharterByName(recv_packet.name, CHARTER_TYPE_GUILD);
        if (g != nullptr || c != nullptr)
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(74));
            return;
        }

        if (_player->m_charters[CHARTER_TYPE_GUILD])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(75));
            return;
        }

        ItemProperties const* ip = sMySQLStore.getItemProperties(CharterEntry::Guild);
        if (ip == nullptr)
            return;

        SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(ip);
        if (res.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        uint8 error = _player->GetItemInterface()->CanReceiveItem(sMySQLStore.getItemProperties(CharterEntry::Guild), 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            _player->PlaySoundToPlayer(recv_packet.creatureGuid, 6594);

            // Create the item and charter
            Item* i = objmgr.CreateItem(CharterEntry::Guild, _player);
            c = objmgr.CreateCharter(_player->getGuidLow(), CHARTER_TYPE_GUILD);
            if (i == NULL || c == NULL)
                return;

            c->GuildName = recv_packet.name;
            c->ItemGuid = i->getGuid();

            c->PetitionSignerCount = recv_packet.signerCount;

            i->setStackCount(1);
            i->addFlags(ITEM_FLAG_SOULBOUND);
            i->setEnchantmentId(0, c->GetID());
            i->setPropertySeed(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(i))
            {
                c->Destroy();
                i->DeleteMe();
                return;
            }

            c->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1, i->getEntry(), i->getPropertySeed(), i->getRandomPropertiesId(), i->getStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = c;
            _player->ModGold(-1000);
            _player->SaveToDB(false);
        }
    }
}

void SendShowSignatures(Charter* c, uint64 i, Player* p)
{
    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, 100);
    data << i;
    data << (uint64)c->GetLeader();
    data << c->GetID();
    data << uint8(c->SignatureCount);
    for (uint32 j = 0; j < c->Slots; ++j)
    {
        if (c->Signatures[j] == 0) continue;
        data << uint64(c->Signatures[j]) << uint32(1);
    }
    data << uint8(0);
    p->GetSession()->SendPacket(&data);
}

void WorldSession::HandleCharterShowSignatures(WorldPacket& recv_data)
{
    CmsgPetitionShowSignatures recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* pCharter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (pCharter != nullptr)
        SendShowSignatures(pCharter, recv_packet.itemGuid, _player);
}

void WorldSession::HandleCharterQuery(WorldPacket& recv_data)
{
    CmsgPetitionQuery recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* c = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (c == nullptr)
        return;

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, 100);
    data << recv_packet.charterId;
    data << (uint64)c->LeaderGuid;
    data << c->GuildName << uint8(0);
    if (c->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(9) << uint32(9);
    else
        data << uint32(c->Slots) << uint32(c->Slots);

    data << uint32(0);                                      // 4
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field

    if (c->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32(1);                                  // 10 min level to sign a guild charter
        data << uint32(80);                                    // 11 max level to sign a guild charter
    }
    else
    {
        data << uint32(80);                                 // 10 min level to sign an arena charter
        data << uint32(80);                                    // 11 max level to sign an arena charter
    }

    data << uint32(0);                                      // 12
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    data << uint32(0);
    data << uint16(0);

    if (c->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(0);
    else
        data << uint32(1);

    SendPacket(&data);
}

void WorldSession::HandleCharterOffer(WorldPacket& recv_data)
{
    CmsgOfferPetition recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Player* pTarget = _player->GetMapMgr()->GetPlayer(recv_packet.playerGuid.getGuidLow());
    Charter* pCharter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (pCharter != nullptr)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(76));
        return;
    }

    if (pTarget == nullptr || pTarget->GetTeam() != _player->GetTeam() || (pTarget == _player && !worldConfig.player.isInterfactionGuildEnabled))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(77));
        return;
    }

    if (!pTarget->CanSignCharter(pCharter, _player))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(78));
        return;
    }

    SendShowSignatures(pCharter, recv_packet.itemGuid, pTarget);
}

namespace PetitionSignResult
{
    enum
    {
        OK = 0,
        AlreadySigned = 1
    };
}

void WorldSession::HandleCharterSign(WorldPacket& recv_data)
{
    CmsgPetitionSign recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* c = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (c == nullptr)
        return;

    for (uint32 i = 0; i < c->SignatureCount; ++i)
    {
        if (c->Signatures[i] == _player->getGuid())
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(79));
            SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, _player->getGuid(), PetitionSignResult::AlreadySigned).serialise().get());
            return;
        }
    }

    if (c->IsFull())
        return;

    c->AddSignature(_player->getGuidLow());
    c->SaveToDB();
    _player->m_charters[c->CharterType] = c;
    _player->SaveToDB(false);

    Player* l = _player->GetMapMgr()->GetPlayer(c->GetLeader());
    if (l == nullptr)
        return;

    l->SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, _player->getGuid(), PetitionSignResult::OK).serialise().get());
    SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, uint64_t(c->GetLeader()), PetitionSignResult::OK).serialise().get());
}

void WorldSession::HandleCharterDecline(WorldPacket& recv_data)
{
    MsgPetitionDecline recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* c = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (c == nullptr)
        return;

    Player* owner = objmgr.GetPlayer(c->GetLeader());
    if (owner)
        owner->GetSession()->SendPacket(MsgPetitionDecline(_player->getGuid()).serialise().get());
}


void WorldSession::HandleCharterTurnInCharter(WorldPacket& recv_data)
{
    CmsgTurnInPetition recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* pCharter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (pCharter == nullptr)
        return;

    if (pCharter->CharterType == CHARTER_TYPE_GUILD)
    {
        Charter* gc = _player->m_charters[CHARTER_TYPE_GUILD];
        if (gc == nullptr)
            return;

        if (gc->SignatureCount < 9 && worldConfig.server.requireAllSignatures)
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        // don't know hacky or not but only solution for now
        // If everything is fine create guild

        Guild* pGuild = new Guild;

        if (!pGuild->create(_player, gc->GuildName))
        {
            delete pGuild;
            return;
        }

        // Destroy the charter
        _player->m_charters[CHARTER_TYPE_GUILD] = 0;
        gc->Destroy();

        _player->GetItemInterface()->RemoveItemAmt(CharterEntry::Guild, 1);
        sHookInterface.OnGuildCreate(_player, pGuild);
    }
    else
    {
        //\todo Arena charter -Replace with correct messages
        uint16_t type;
        uint32 i;
        uint32 icon;
        uint32 iconcolor;
        uint32 bordercolor;
        uint32 border;
        uint32 background;

        recv_data >> iconcolor;
        recv_data >> icon;
        recv_data >> bordercolor;
        recv_data >> border;
        recv_data >> background;

        switch (pCharter->CharterType)
        {
            case CHARTER_TYPE_ARENA_2V2:
                type = ARENA_TEAM_TYPE_2V2;
                break;

            case CHARTER_TYPE_ARENA_3V3:
                type = ARENA_TEAM_TYPE_3V3;
                break;

            case CHARTER_TYPE_ARENA_5V5:
                type = ARENA_TEAM_TYPE_5V5;
                break;

            default:
                SendNotification("Internal Error");
                return;
        }

        if (_player->m_arenaTeams[pCharter->CharterType - 1] != NULL)
        {
            sChatHandler.SystemMessage(this, LocalizedWorldSrv(SS_ALREADY_ARENA_TEAM));
            return;
        }

        if (pCharter->SignatureCount < pCharter->GetNumberOfSlotsByType() && worldConfig.server.requireAllSignatures)
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        ArenaTeam* team = new ArenaTeam(type, objmgr.GenerateArenaTeamId());
        team->m_name = pCharter->GuildName;
        team->m_emblemColour = iconcolor;
        team->m_emblemStyle = icon;
        team->m_borderColour = bordercolor;
        team->m_borderStyle = border;
        team->m_backgroundColour = background;
        team->m_leader = _player->getGuidLow();
        team->m_stat_rating = 1500;

        objmgr.AddArenaTeam(team);
        objmgr.UpdateArenaTeamRankings();
        team->AddMember(_player->m_playerInfo);

        for (i = 0; i < pCharter->SignatureCount; ++i)
        {
            PlayerInfo* info = objmgr.GetPlayerInfo(pCharter->Signatures[i]);
            if (info)
                team->AddMember(info);
        }

        _player->GetItemInterface()->SafeFullRemoveItemByGuid(recv_packet.itemGuid);
        _player->m_charters[pCharter->CharterType] = NULL;
        pCharter->Destroy();
    }

    Guild::sendTurnInPetitionResult(this, PETITION_ERROR_OK);
}

void WorldSession::HandleCharterRename(WorldPacket& recv_data)
{
    MsgPetitionRename recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Charter* pCharter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (pCharter == nullptr)
        return;

    Guild* g = sGuildMgr.getGuildByName(recv_packet.name);
    Charter* c = objmgr.GetCharterByName(recv_packet.name, static_cast<CharterTypes>(pCharter->CharterType));
    if (c || g)
    {
        SendNotification("That name is in use by another guild.");
        return;
    }

    c = pCharter;
    c->GuildName = recv_packet.name;
    c->SaveToDB();

    SendPacket(MsgPetitionRename(recv_packet.itemGuid, recv_packet.name).serialise().get());
}

#endif
