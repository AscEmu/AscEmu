/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Units/Players/Player.h"

void Player::sendAuctionCommandResult(Auction* auction, uint32_t action, uint32_t errorCode, uint32_t bidError)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32_t(auction ? auction->Id : 0);
    data << uint32_t(action);
    data << uint32_t(errorCode);

    switch (errorCode)
    {
        case AUCTION_ERR_NONE:
        {
            if (action == AUCTION_BID)
            {
                data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            }
            break;
        }
        case AUCTION_ERR_INVENTORY:
        {
            data << uint32_t(bidError);
            break;
        }
        case AUCTION_ERR_HIGHER_BID:
        {
            data << uint64_t(auction->HighestBidder);
            data << uint64_t(auction->HighestBid);
            data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            break;
        }
        default: break;
    }

    SendPacket(&data);
}
