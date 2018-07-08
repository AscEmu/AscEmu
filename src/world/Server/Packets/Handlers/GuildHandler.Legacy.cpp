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


void WorldSession::HandleGuildAccept(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    Player* plyr = GetPlayer();
    if (!plyr)
        return;

    Player* inviter = objmgr.GetPlayer(plyr->GetGuildInvitersGuid());
    plyr->UnSetGuildInvitersGuid();

    if (!inviter)
        return;

    Guild* pGuild = inviter->m_playerInfo->guild;
    if (!pGuild)
        return;

    pGuild->getLock().Acquire();
    uint32 memberCount = static_cast<uint32>(pGuild->GetNumMembers());
    pGuild->getLock().Release();

    if (memberCount >= MAX_GUILD_MEMBERS)
    {
        plyr->UnSetGuildInvitersGuid();
        SystemMessage("That guild is full.");
        return;
    }

    pGuild->AddGuildMember(plyr->m_playerInfo, NULL);
}

void WorldSession::HandleGuildDecline(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    Player* plyr = GetPlayer();
    if (!plyr)
        return;

    Player* inviter = objmgr.GetPlayer(plyr->GetGuildInvitersGuid());
    plyr->UnSetGuildInvitersGuid();

    if (!inviter)
        return;

    WorldPacket data;
    data.Initialize(SMSG_GUILD_DECLINE);
    data << plyr->getName().c_str();
    inviter->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetGuildInformation(WorldPacket& recv_data)
{
    CmsgGuildInfoText recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Guild* pGuild = _player->m_playerInfo->guild;
    if (!pGuild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    pGuild->SetGuildInformation(recv_packet.text.c_str(), this);
}

void WorldSession::HandleGuildInfo(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (_player->GetGuild() != nullptr)
        _player->GetGuild()->SendGuildInfo(this);
}

void WorldSession::HandleGuildRoster(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (!_player->m_playerInfo->guild)
        return;

    _player->m_playerInfo->guild->SendGuildRoster(this);
}

void WorldSession::HandleGuildPromote(WorldPacket& recv_data)
{
    CmsgGuildPromote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    PlayerInfo* dstplr = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (dstplr == NULL)
        return;

    _player->m_playerInfo->guild->PromoteGuildMember(dstplr, this);
}

void WorldSession::HandleGuildDemote(WorldPacket& recv_data)
{
    CmsgGuildDemote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    PlayerInfo* dstplr = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (dstplr == NULL)
        return;

    _player->m_playerInfo->guild->DemoteGuildMember(dstplr, this);
}

void WorldSession::HandleGuildLeave(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    _player->m_playerInfo->guild->RemoveGuildMember(_player->m_playerInfo, this);
}

void WorldSession::HandleGuildRemove(WorldPacket& recv_data)
{
    CmsgGuildRemove recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    PlayerInfo* dstplr = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (dstplr == NULL)
        return;

    _player->m_playerInfo->guild->RemoveGuildMember(dstplr, this);
}

void WorldSession::HandleGuildDisband(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    if (_player->m_playerInfo->guild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    _player->m_playerInfo->guild->disband();
}

void WorldSession::HandleGuildLeader(WorldPacket& recv_data)
{
    CmsgGuildLeader recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    PlayerInfo* dstplr = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (dstplr == NULL)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, recv_packet.name, GC_ERROR_PLAYER_NOT_FOUND_S).serialise().get());
        return;
    }

    _player->m_playerInfo->guild->ChangeGuildMaster(dstplr, this);
}

void WorldSession::HandleGuildMotd(WorldPacket& recv_data)
{
    CmsgGuildMotd recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    _player->m_playerInfo->guild->SetMOTD(recv_packet.message.c_str(), this);
}

void WorldSession::HandleGuildRank(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 9);

    if (!_player->m_playerInfo->guild)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    if (GetPlayer()->getGuidLow() != _player->m_playerInfo->guild->GetGuildLeader())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    uint32 rankId;
    std::string newName;

    recv_data >> rankId;

    GuildRank* pRank = _player->GetGuild()->GetGuildRank(rankId);
    if (pRank == NULL)
        return;

    recv_data >> pRank->iRights;    // no
    recv_data >> newName;

    if (newName.length() < 2)
        newName = std::string(pRank->szRankName);

    if (strcmp(newName.c_str(), pRank->szRankName) != 0)
    {
        // name changed
        char* pTmp = pRank->szRankName;
        pRank->szRankName = strdup(newName.c_str());
        free(pTmp);
    }

    int32 gold_limit;
    recv_data >> gold_limit;

    // do not touch guild masters withdraw limit
    if (pRank->iGoldLimitPerDay != -1 || rankId != 0)
        pRank->iGoldLimitPerDay = gold_limit;


    for (uint8 i = 0; i < MAX_GUILD_BANK_TABS; ++i)
    {
        recv_data >> pRank->iTabPermissions[i].iFlags;
        recv_data >> pRank->iTabPermissions[i].iStacksPerDay;
    }

    uint32 guildID = _player->GetGuildId();
    uint32 rankID = pRank->iId;

    CharacterDatabase.Execute("DELETE FROM guild_ranks WHERE guildid = %u AND rankid = %u;", guildID, rankID);

    CharacterDatabase.Execute("INSERT INTO guild_ranks VALUES(%u, %u, \'%s\', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                              guildID, rankID, CharacterDatabase.EscapeString(newName).c_str(),
                              pRank->iRights, pRank->iGoldLimitPerDay,
                              pRank->iTabPermissions[0].iFlags, pRank->iTabPermissions[0].iStacksPerDay,
                              pRank->iTabPermissions[1].iFlags, pRank->iTabPermissions[1].iStacksPerDay,
                              pRank->iTabPermissions[2].iFlags, pRank->iTabPermissions[2].iStacksPerDay,
                              pRank->iTabPermissions[3].iFlags, pRank->iTabPermissions[3].iStacksPerDay,
                              pRank->iTabPermissions[4].iFlags, pRank->iTabPermissions[4].iStacksPerDay,
                              pRank->iTabPermissions[5].iFlags, pRank->iTabPermissions[5].iStacksPerDay);

    _player->GetGuild()->SendGuildQuery(nullptr);
    _player->GetGuild()->SendGuildRoster(this);
}

void WorldSession::HandleGuildAddRank(WorldPacket& recv_data)
{
    CmsgGuildAddRank recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    Guild* pGuild = _player->GetGuild();
    if (pGuild == NULL)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    if (pGuild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    if (recv_packet.name.size() < 2)
        return;

    pGuild->CreateGuildRank(recv_packet.name.c_str(), GR_RIGHT_DEFAULT, false);

    // there is probably a command result for this. need to find it.
    pGuild->SendGuildQuery(NULL);
    pGuild->SendGuildRoster(this);
}

void WorldSession::HandleGuildDelRank(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    Guild* pGuild = _player->GetGuild();
    if (pGuild == nullptr)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    if (pGuild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    pGuild->RemoveGuildRank(this);

    // there is probably a command result for this. need to find it.
    pGuild->SendGuildQuery(NULL);
    pGuild->SendGuildRoster(this);
}

void WorldSession::HandleGuildSetPublicNote(WorldPacket& recv_data)
{
    CmsgGuildSetPublicNote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    PlayerInfo* pTarget = objmgr.GetPlayerInfoByName(recv_packet.targetName.c_str());
    if (pTarget == NULL)
        return;

    if (!pTarget->guild)
        return;

    pTarget->guild->SetPublicNote(pTarget, recv_packet.note.c_str(), this);
}

void WorldSession::HandleGuildSetOfficerNote(WorldPacket& recv_data)
{
    CmsgGuildSetOfficerNote recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    PlayerInfo* pTarget = objmgr.GetPlayerInfoByName(recv_packet.targetName.c_str());
    if (pTarget == NULL)
        return;

    if (!pTarget->guild)
        return;

    pTarget->guild->SetOfficerNote(pTarget, recv_packet.note.c_str(), this);
}

void WorldSession::HandleSaveGuildEmblem(WorldPacket& recv_data)
{
    MsgSaveGuildEmblem recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    CHECK_GUID_EXISTS(recv_packet.guid);

    Guild* pGuild = _player->GetGuild();
    if (pGuild == nullptr)
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOGUILD).serialise().get());
        return;
    }

    if (pGuild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTGUILDMASTER).serialise().get());
        return;
    }

    if (!_player->HasGold(EMBLEM_PRICE))
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTENOUGHMONEY).serialise().get());
        return;
    }

    SendPacket(MsgSaveGuildEmblem(GEM_ERROR_SUCCESS).serialise().get());

    pGuild->setEmblemInfo(recv_packet.emblemInfo);

    // update all clients (probably is an event for this, again.)
    pGuild->SendGuildQuery(nullptr);
}

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

        if (objmgr.GetCharterByName(recv_packet.name, (CharterTypes)recv_packet.arenaIndex))
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
            Charter* c = objmgr.CreateCharter(_player->getGuidLow(), (CharterTypes)recv_packet.arenaIndex);
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
    {
        data << uint32(0);
    }
    else
    {
        data << uint32(1);
    }

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
            Guild::SendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        // don't know hacky or not but only solution for now
        // If everything is fine create guild

        Guild* pGuild = Guild::Create();
        pGuild->CreateFromCharter(gc, this);

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
            Guild::SendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
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

    Guild::SendTurnInPetitionResult(this, PETITION_ERROR_OK);
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
    Charter* c = objmgr.GetCharterByName(recv_packet.name, (CharterTypes)pCharter->CharterType);
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

void WorldSession::HandleGuildLog(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (!_player->m_playerInfo->guild)
        return;

    _player->m_playerInfo->guild->SendGuildLog(this);
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket& recv_data)
{
    //\todo not used.
    CmsgGuildBankBuyTab recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->IsInWorld())
        return;

    if (!_player->m_playerInfo->guild)
        return;

    if (_player->m_playerInfo->guild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_GUILD_CHAT, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    if (_player->m_playerInfo->guild->GetBankTabCount() < MAX_GUILD_BANK_TABS)
    {
        //                                        tab1, tab2, tab3, tab4, tab5, tab6
        int32 cost = GOLD * _GetGuildBankTabPrice(_player->m_playerInfo->guild->GetBankTabCount());

        if (!_player->HasGold((uint32)cost))
            return;

        _player->ModGold(-cost);
        _player->m_playerInfo->guild->BuyBankTab(this);
        _player->m_playerInfo->guild->LogGuildEvent(GE_BANK_TAB_PURCHASED, 1, "");
    }
}

void WorldSession::HandleGuildBankGetAvailableAmount(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    if (_player->m_playerInfo->guildMember == nullptr)
        return;

    uint64 money = _player->m_playerInfo->guild->GetBankBalance();
    uint32 avail = _player->m_playerInfo->guildMember->CalculateAvailableAmount();

    WorldPacket data(MSG_GUILD_BANK_MONEY_WITHDRAWN, 4);
    data << uint32(money > avail ? avail : money);
    SendPacket(&data);
}

void WorldSession::HandleGuildBankModifyTab(WorldPacket& recv_data)
{
    CmsgGuildBankUpdateTab recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (_player->m_playerInfo->guild == NULL)
        return;

    GuildBankTab* pTab = _player->m_playerInfo->guild->GetBankTab(recv_packet.slot);
    if (pTab == NULL)
        return;

    if (_player->m_playerInfo->guild->GetGuildLeader() != _player->getGuidLow())
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_GUILD_CHAT, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    char* ptmp;
    if (!recv_packet.tabName.empty())
    {
        if (!(pTab->szTabName && strcmp(pTab->szTabName, recv_packet.tabName.c_str()) == 0))
        {
            ptmp = pTab->szTabName;
            pTab->szTabName = strdup(recv_packet.tabName.c_str());
            if (ptmp)
                free(ptmp);

            CharacterDatabase.Execute("UPDATE guild_banktabs SET tabName = \'%s\' WHERE guildId = %u AND tabId = %u",
                                      CharacterDatabase.EscapeString(recv_packet.tabName).c_str(), _player->m_playerInfo->guild->getGuildId(), static_cast<uint32>(recv_packet.slot));
        }
    }
    else
    {
        if (pTab->szTabName)
        {
            ptmp = pTab->szTabName;
            pTab->szTabName = NULL;
            if (ptmp)
                free(ptmp);

            CharacterDatabase.Execute("UPDATE guild_banktabs SET tabName = '' WHERE guildId = %u AND tabId = %u", _player->m_playerInfo->guild->getGuildId(), static_cast<uint32>(recv_packet.slot));
        }
    }

    if (!recv_packet.tabIcon.empty())
    {
        if (!(pTab->szTabIcon && strcmp(pTab->szTabIcon, recv_packet.tabIcon.c_str()) == 0))
        {
            ptmp = pTab->szTabIcon;
            pTab->szTabIcon = strdup(recv_packet.tabIcon.c_str());
            if (ptmp)
                free(ptmp);

            CharacterDatabase.Execute("UPDATE guild_banktabs SET tabIcon = \'%s\' WHERE guildId = %u AND tabId = %u",
                                      CharacterDatabase.EscapeString(recv_packet.tabIcon).c_str(), _player->m_playerInfo->guild->getGuildId(), static_cast<uint32>(recv_packet.slot));
        }
    }
    else
    {
        if (pTab->szTabIcon)
        {
            ptmp = pTab->szTabIcon;
            pTab->szTabIcon = NULL;
            if (ptmp)
                free(ptmp);

            CharacterDatabase.Execute("UPDATE guild_banktabs SET tabIcon = '' WHERE guildId = %u AND tabId = %u", _player->m_playerInfo->guild->getGuildId(), static_cast<uint32>(recv_packet.slot));
        }
    }

    _player->m_playerInfo->guild->SendGuildBankInfo(this);
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket& recv_data)
{
    CmsgGuildBankWithdrawMoney recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (_player->m_playerInfo->guild == NULL)
        return;

    _player->m_playerInfo->guild->WithdrawMoney(this, recv_packet.money);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket& recv_data)
{
    CmsgGuildBankDepositMoney recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (_player->m_playerInfo->guild == NULL)
        return;

    _player->m_playerInfo->guild->DepositMoney(this, recv_packet.money);
}

void WorldSession::HandleGuildBankDepositItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    uint8 source_isfrombank;
    uint32 wtf;
    uint8 wtf2;
    uint32 i;

    Guild* pGuild = _player->m_playerInfo->guild;
    GuildMember* pMember = _player->m_playerInfo->guildMember;

    if (pGuild == NULL || pMember == NULL)
        return;

    recv_data >> guid;
    recv_data >> source_isfrombank;

    if (source_isfrombank)
    {
        uint8 dest_bank;
        uint8 dest_bankslot;
        uint8 source_bank;
        uint8 source_bankslot;

        recv_data >> dest_bank;
        recv_data >> dest_bankslot;
        recv_data >> wtf;
        recv_data >> source_bank;
        recv_data >> source_bankslot;

        if (source_bankslot >= MAX_GUILD_BANK_SLOTS || dest_bankslot >= MAX_GUILD_BANK_SLOTS
            || source_bank >= MAX_GUILD_BANK_TABS || dest_bank >= MAX_GUILD_BANK_TABS)
            return;

        if (!pMember->pRank->CanPerformBankCommand(GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS, dest_bank) ||
            !pMember->pRank->CanPerformBankCommand(GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS, source_bank))
            return;

        /* locate the tabs */
        GuildBankTab* pSourceTab = pGuild->GetBankTab(source_bank);
        GuildBankTab* pDestTab = pGuild->GetBankTab(dest_bank);
        if (pSourceTab == NULL || pDestTab == NULL)
            return;

        Item* pSourceItem = pSourceTab->pSlots[source_bankslot];
        Item* pDestItem = pDestTab->pSlots[dest_bankslot];
        if (pSourceItem == NULL && pDestItem == NULL)
            return;

        /* perform the actual swap */
        pSourceTab->pSlots[source_bankslot] = pDestItem;
        pDestTab->pSlots[dest_bankslot] = pSourceItem;

        /* update the client */
        if (pSourceTab == pDestTab)
        {
            /* send both slots in the packet */
            pGuild->SendGuildBank(this, pSourceTab, source_bankslot, dest_bankslot);
        }
        else
        {
            /* send a packet for each different bag */
            pGuild->SendGuildBank(this, pSourceTab, source_bankslot, -1);
            pGuild->SendGuildBank(this, pDestTab, dest_bankslot, -1);
        }

        /* update in sql */
        if (pDestItem == NULL)
        {
            /* this means the source slot is no longer being used. */
            CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u",
                                      pGuild->getGuildId(), (uint32)pSourceTab->iTabId, (uint32)source_bankslot);
        }
        else
        {
            /* insert the new item */
            CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u",
                                      pGuild->getGuildId(), pSourceTab->iTabId, source_bankslot);
            CharacterDatabase.Execute("INSERT INTO guild_bankitems VALUES(%u, %u, %u, %u)",
                                      pGuild->getGuildId(), (uint32)pSourceTab->iTabId, (uint32)source_bankslot, pDestItem->getGuidLow());
        }

        if (pSourceItem == NULL)
        {
            /* this means the destination slot is no longer being used. */
            CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u",
                                      pGuild->getGuildId(), (uint32)pDestTab->iTabId, (uint32)dest_bankslot);
        }
        else
        {
            /* insert the new item */
            CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u",
                                      pGuild->getGuildId(), pDestTab->iTabId, dest_bankslot);
            CharacterDatabase.Execute("INSERT INTO guild_bankitems VALUES(%u, %u, %u, %u)",
                                      pGuild->getGuildId(), (uint32)pDestTab->iTabId, (uint32)dest_bankslot, pSourceItem->getGuidLow());
        }
    }
    else
    {
        uint8 source_bagslot;
        uint8 source_slot;
        uint8 dest_bank;
        uint8 dest_bankslot;
        uint8 withdraw_stack = 0;
        uint8 deposit_stack = 0;
        GuildBankTab* pTab;
        Item* pSourceItem;
        Item* pDestItem;
        Item* pSourceItem2;

        /* read packet */
        recv_data >> dest_bank;
        recv_data >> dest_bankslot;
        recv_data >> wtf;
        recv_data >> wtf2;

        if (wtf2)
            recv_data >> withdraw_stack;

        recv_data >> source_bagslot;
        recv_data >> source_slot;

        if (!(source_bagslot == 1 && source_slot == 0))
        {
            recv_data >> wtf2;
            recv_data >> deposit_stack;
        }

        /* sanity checks to avoid overflows */
        if (dest_bank >= MAX_GUILD_BANK_TABS)
        {
            return;
        }

        /* make sure we have permissions */
        if (!pMember->pRank->CanPerformBankCommand(GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS, dest_bank))
            return;

        /* get tab */
        pTab = pGuild->GetBankTab(dest_bank);
        if (pTab == NULL)
            return;

        // check if we are autoassigning
        if (dest_bankslot == 0xff)
        {
            for (i = 0; i < MAX_GUILD_BANK_SLOTS; ++i)
            {
                if (pTab->pSlots[i] == NULL)
                {
                    dest_bankslot = (uint8)i;
                    break;
                }
            }

            if (dest_bankslot == 0xff)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_BAG_FULL);
                return;
            }
        }

        // another check here
        if (dest_bankslot >= MAX_GUILD_BANK_SLOTS)
            return;

        // check if we're pulling an item from the bank, make sure we're not cheating.
        pDestItem = pTab->pSlots[dest_bankslot];

        // grab the source/destination item
        if (source_bagslot == 1 && source_slot == 0)
        {
            // find a free bag slot
            if (pDestItem == NULL)
            {
                // dis is fucked up mate
                return;
            }

            SlotResult sr = _player->GetItemInterface()->FindFreeInventorySlot(pDestItem->getItemProperties());
            if (!sr.Result)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_BAG_FULL);
                return;
            }

            source_bagslot = sr.ContainerSlot;
            source_slot = sr.Slot;
        }

        if (source_bagslot == 0xff && source_slot < INVENTORY_SLOT_ITEM_START && pDestItem != NULL)
        {
            sCheatLog.writefromsession(this, "Tried to equip an item from the guild bank (WPE HACK)");
            SystemMessage("You don't have permission to do that.");
            return;
        }

        if (pDestItem != NULL)
        {
            if (pMember->pRank->iTabPermissions[dest_bank].iStacksPerDay == 0)
            {
                SystemMessage("You don't have permission to do that.");
                return;
            }

            if (pMember->pRank->iTabPermissions[dest_bank].iStacksPerDay > 0)
            {
                if (pMember->CalculateAllowedItemWithdraws(dest_bank) == 0)
                {
                    // a "no permissions" notice would probably be better here
                    SystemMessage("You have withdrawn the maximum amount for today.");
                    return;
                }

                /* reduce his count by one */
                pMember->OnItemWithdraw(dest_bank);
            }
        }

        pSourceItem = _player->GetItemInterface()->GetInventoryItem(source_bagslot, source_slot);

        // make sure that both aren't null - wtf ?
        if (pSourceItem == NULL && pDestItem == NULL)
            return;

        if (pSourceItem != NULL)
        {
            // make sure its not a soulbound item
            if (pSourceItem->isSoulbound() || pSourceItem->getItemProperties()->Class == ITEM_CLASS_QUEST)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_CANT_DROP_SOULBOUND);
                return;
            }

            // pull the item from the slot
            if (deposit_stack && pSourceItem->getStackCount() > deposit_stack)
            {
                pSourceItem2 = pSourceItem;
                pSourceItem = objmgr.CreateItem(pSourceItem2->getEntry(), _player);
                if (pSourceItem == NULL)
                    return;

                pSourceItem->setStackCount(deposit_stack);
                pSourceItem->setCreatorGuid(pSourceItem2->getCreatorGuid());
                pSourceItem2->modStackCount(-(int32)deposit_stack);
                pSourceItem2->m_isDirty = true;
            }
            else
            {
                if (!_player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(source_bagslot, source_slot, false))
                    return;

                pSourceItem->RemoveFromWorld();
            }
        }

        // perform the swap.
        // pSourceItem = Source item from players backpack coming into guild bank
        if (pSourceItem == NULL)
        {
            // splitting
            if (pDestItem != NULL && deposit_stack > 0 && pDestItem->getStackCount() > deposit_stack)
            {
                pSourceItem2 = pDestItem;

                pSourceItem2->modStackCount(-(int32)deposit_stack);
                pSourceItem2->SaveToDB(0, 0, true, NULL);

                pDestItem = objmgr.CreateItem(pSourceItem2->getEntry(), _player);
                if (pDestItem == NULL)
                    return;

                pDestItem->setStackCount(deposit_stack);
                pDestItem->setCreatorGuid(pSourceItem2->getCreatorGuid());
            }
            else
            {
                // that slot in the bank is now empty.
                pTab->pSlots[dest_bankslot] = NULL;
                CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u", pGuild->getGuildId(), (uint32)pTab->iTabId, (uint32)dest_bankslot);
            }
        }
        else
        {
            // there is a new item in that slot.
            pTab->pSlots[dest_bankslot] = pSourceItem;

            CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE guildId = %u AND tabId = %u AND slotId = %u", pGuild->getGuildId(), pTab->iTabId, dest_bankslot);
            CharacterDatabase.Execute("INSERT INTO guild_bankitems VALUES(%u, %u, %u, %u)", pGuild->getGuildId(), (uint32)pTab->iTabId, (uint32)dest_bankslot, pSourceItem->getGuidLow());

            // remove the item's association with the player
            pSourceItem->setOwner(nullptr);
            pSourceItem->SaveToDB(0, 0, true, NULL);

            // log it
            pGuild->LogGuildBankAction(GB_LOG_DEPOSIT_ITEM, _player->getGuidLow(), pSourceItem->getEntry(),
                                       (uint8)pSourceItem->getStackCount(), pTab);
        }

        // pDestItem = Item from bank coming into players backpack
        if (pDestItem == NULL)
        {
            // the item has already been removed from the players backpack at this stage, there isn't really much to do at this point.
        }
        else
        {
            // the guild was robbed by some n00b! :O
            pDestItem->setOwner(_player);
            pDestItem->SaveToDB(source_bagslot, source_slot, true, NULL);

            // add it to him in game
            if (!_player->GetItemInterface()->SafeAddItem(pDestItem, source_bagslot, source_slot))
            {
                // this *really* shouldn't happen.
                if (!_player->GetItemInterface()->AddItemToFreeSlot(pDestItem))
                {
                    //pDestItem->DeleteFromDB();
                    pDestItem->DeleteMe();
                }
            }
            else
            {
                // log it
                pGuild->LogGuildBankAction(GB_LOG_WITHDRAW_ITEM, _player->getGuidLow(), pDestItem->getEntry(),
                                           (uint8)pDestItem->getStackCount(), pTab);
            }
        }

        // update the clients view of the bank tab
        pGuild->SendGuildBank(this, pTab, dest_bankslot);
    }
}

void WorldSession::HandleGuildBankOpenVault(WorldPacket& recv_data)
{
    CmsgGuildBankerActivate recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (!_player->IsInWorld() || _player->m_playerInfo->guild == NULL)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    GameObject* pObj = _player->GetMapMgr()->GetGameObject(recv_packet.guid.getGuidLow());
    if (pObj == NULL)
        return;

    _player->m_playerInfo->guild->SendGuildBankInfo(this);
}

void WorldSession::HandleGuildBankViewTab(WorldPacket& recv_data)
{
    CmsgGuildBankQueryTab recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    // maybe last uint8 is "show additional info" such as tab names? *shrug*
    Guild* pGuild = _player->m_playerInfo->guild;
    if (pGuild == NULL)
        return;

    GuildBankTab* pTab = pGuild->GetBankTab(recv_packet.tabId);
    if (pTab == NULL)
        return;

    pGuild->SendGuildBank(this, pTab);
}

void WorldSession::HandleGuildGetFullPermissions(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(MSG_GUILD_PERMISSIONS, 61);
    GuildRank* pRank = _player->GetGuildRankS();

    if (_player->GetGuild() == nullptr)
        return;

    data << pRank->iId;
    data << pRank->iRights;
    data << pRank->iGoldLimitPerDay;
    data << _player->GetGuild()->GetBankTabCount();

    for (uint8 i = 0; i < MAX_GUILD_BANK_TABS; ++i)
    {
        data << pRank->iTabPermissions[i].iFlags;
        data << pRank->iTabPermissions[i].iStacksPerDay;
    }

    SendPacket(&data);
}

void WorldSession::HandleGuildBankViewLog(WorldPacket& recv_data)
{
    MsgGuildBankLogQuery recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (_player->GetGuild() == NULL)
        return;

    _player->GetGuild()->SendGuildBankLog(this, recv_packet.slotId);
}

void WorldSession::HandleGuildBankQueryText(WorldPacket& recv_data)
{
    MsgQueryGuildBankText recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    if (_player->GetGuild() == nullptr)
        return;

    GuildBankTab* tab = _player->GetGuild()->GetBankTab(recv_packet.tabId);
    if (tab == nullptr)
        return;

    SendPacket(MsgQueryGuildBankText(recv_packet.tabId, tab->szTabInfo).serialise().get());
}

void WorldSession::HandleSetGuildBankText(WorldPacket& recv_data)
{
    GuildMember* pMember = _player->m_playerInfo->guildMember;
    if (_player->GetGuild() == NULL || pMember == NULL)
        return;

    CmsgSetGuildBankText recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    GuildBankTab* tab = _player->GetGuild()->GetBankTab(recv_packet.tabId);
    if (tab != NULL &&
        pMember->pRank->CanPerformBankCommand(GR_RIGHT_GUILD_BANK_CHANGE_TABTXT, recv_packet.tabId))
    {
        tab->szTabInfo = strdup(recv_packet.text.c_str());
        WorldPacket data(SMSG_GUILD_EVENT, 4);
        data << uint8(GE_BANK_TEXT_CHANGED);
        data << uint8(1);
        data << uint16(0x30 + recv_packet.tabId);
        SendPacket(&data);

        CharacterDatabase.Execute("UPDATE guild_banktabs SET tabInfo = \'%s\' WHERE guildId = %u AND tabId = %u",
                                  CharacterDatabase.EscapeString(recv_packet.text).c_str(), _player->m_playerInfo->guild->getGuildId(), static_cast<uint32>(recv_packet.tabId));
    }
}
#endif
