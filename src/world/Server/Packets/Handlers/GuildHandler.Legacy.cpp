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

#endif
