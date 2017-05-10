/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Player.h"
#include "Server/Packets/Opcode.h"
#include "Chat/ChatDefines.hpp"
#include "Server/World.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING != Cata
void Player::sendForceMovePaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(50);

    switch (speed_type)
    {
        case TYPE_WALK:
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            break;
        case TYPE_RUN:
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            break;
        case TYPE_SWIM:
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            break;
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            break;
#endif
    }

    data << GetNewGUID();
    data << m_speedChangeCounter++;

    if (speed_type == TYPE_RUN)
        data << uint8_t(1);

    data << float(speed);

    SendMessageToSet(&data, true);
}
#else
void Player::sendForceMovePaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(60);
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
        } break;
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
        } break;
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
        } break;
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_TURN_RATE:
        {
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_TURN_RATE_CHANGE, speed);
        } break;
        case TYPE_FLY:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
        } break;
        case TYPE_FLY_BACK:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_PITCH_RATE:
        {
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_PITCH_RATE_CHANGE, speed);
        } break;
    }

    SendMessageToSet(&data, true);
}
#endif

#if VERSION_STRING != Cata
void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(45);

    switch (speed_type)
    {
        case TYPE_WALK:
            data.Initialize(MSG_MOVE_SET_WALK_SPEED);
            break;
        case TYPE_RUN:
            data.Initialize(MSG_MOVE_SET_RUN_SPEED);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED);
            break;
        case TYPE_SWIM:
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(MSG_MOVE_SET_TURN_RATE);
            break;
        case TYPE_FLY:
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(MSG_MOVE_SET_PITCH_RATE);
            break;
#endif
    }

    data << GetNewGUID();
    BuildMovementPacket(&data);
    data << float(speed);

    SendMessageToSet(&data, false);
}
#else
void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data;
    ObjectGuid guid = GetGUID();

    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(MSG_MOVE_SET_WALK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_WALK_SPEED, speed);
        } break;
        case TYPE_RUN:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_SPEED, speed);
        } break;
        case TYPE_RUN_BACK:
        {
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_BACK_SPEED, speed);
        } break;
        case TYPE_SWIM:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_SPEED, speed);
        } break;
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_BACK_SPEED, speed);
        } break;
        case TYPE_TURN_RATE:
        {
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_TURN_RATE, speed);
        } break;
        case TYPE_FLY:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_SPEED, speed);
        } break;
        case TYPE_FLY_BACK:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_BACK_SPEED, speed);
        } break;
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_PITCH_RATE, speed);
        } break;
    }

    SendMessageToSet(&data, true);
}
#endif

void Player::handleFall(MovementInfo const& movement_info)
{
#if VERSION_STRING == Cata
    if (!z_axisposition)
        z_axisposition = movement_info.getPosition()->z;

    uint32 falldistance = float2int32(z_axisposition - movement_info.getPosition()->z);
    if (z_axisposition <= movement_info.getPosition()->z)
        falldistance = 1;

    if ((int)falldistance > m_safeFall)
        falldistance -= m_safeFall;
    else
        falldistance = 1;


    if (isAlive() && !bInvincible && (falldistance > 12) && !m_noFallDamage && ((!GodModeCheat && (UNIXTIME >= m_fallDisabledUntil))))
    {
        uint32_t health_loss = static_cast<uint32_t>(GetHealth() * (falldistance - 12) * 0.017f);

        if (health_loss >= GetHealth())
            health_loss = GetHealth();
        else if ((falldistance >= 65))
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(GetDrunkValue()), 0);
        }

        SendEnvironmentalDamageLog(GetGUID(), DAMAGE_FALL, health_loss);
        DealDamage(this, health_loss, 0, 0, 0);
    }
    z_axisposition = 0.0f;
#endif
}

bool Player::isPlayerJumping(MovementInfo const& movement_info, uint16_t opcode)
{
#if VERSION_STRING == Cata
    if (opcode == MSG_MOVE_FALL_LAND || movement_info.hasMovementFlag(MOVEFLAG_SWIMMING))
    {
        jumping = false;
        return false;
    }

    if (!jumping && (opcode == MSG_MOVE_JUMP || movement_info.hasMovementFlag(MOVEFLAG_FALLING)))
    {
        jumping = true;
        return true;
    }
#endif
    return false;
}

void Player::handleBreathing(MovementInfo& movement_info, WorldSession* session)
{
#if VERSION_STRING == Cata
    if (!worldConfig.server.enableBreathing || FlyCheat || m_bUnlimitedBreath || !isAlive() || GodModeCheat)
    {
        if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING)
            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;

        if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }

        if (session->m_bIsWLevelSet)
        {
            if ((movement_info.getPosition()->z + m_noseLevel) > session->m_wLevel)
            {
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

                session->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    if (movement_info.hasMovementFlag(MOVEFLAG_SWIMMING) && !(m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        if (!session->m_bIsWLevelSet)
        {
            session->m_wLevel = movement_info.getPosition()->z + m_noseLevel * 0.95f;
            session->m_bIsWLevelSet = true;
        }

        m_UnderwaterState |= UNDERWATERSTATE_SWIMMING;
    }

    if (!(movement_info.hasMovementFlag(MOVEFLAG_SWIMMING)) && (movement_info.hasMovementFlag(MOVEFLAG_NONE)) && (m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        if ((movement_info.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

            session->m_bIsWLevelSet = false;

            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && !(m_UnderwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        if ((movement_info.getPosition()->z + m_noseLevel) < session->m_wLevel)
        {
            m_UnderwaterState |= UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movement_info.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }

    if (!(m_UnderwaterState & UNDERWATERSTATE_SWIMMING) && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movement_info.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

bool Player::isSpellFitByClassAndRace(uint32_t spell_id)
{
#if VERSION_STRING == Cata
    uint32_t racemask = getRaceMask();
    uint32_t classmask = getClassMask();

    SkillLineAbilityMapBounds bounds = objmgr.GetSkillLineAbilityMapBounds(spell_id);
    if (bounds.first == bounds.second)
        return true;

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        // skip wrong race skills
        if (_spell_idx->second->race_mask && (_spell_idx->second->race_mask & racemask) == 0)
            continue;

        // skip wrong class skills
        if (_spell_idx->second->class_mask && (_spell_idx->second->class_mask & classmask) == 0)
            continue;

        return true;
    }
#endif
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Auction

void Player::sendAuctionCommandResult(Auction* auction, uint32_t action, uint32_t errorCode, uint32_t bidError)
{
#if VERSION_STRING == Cata
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32_t(auction ? auction->Id : 0);
    data << uint32_t(action);
    data << uint32_t(errorCode);

    switch (errorCode)
    {
        case AUCTION_ERR_NONE:
            if (action == AUCTION_BID)
                data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            break;
        case AUCTION_ERR_INVENTORY:
            data << uint32_t(bidError);
            break;
        case AUCTION_ERR_HIGHER_BID:
            data << uint64_t(auction->HighestBidder);
            data << uint64_t(auction->HighestBid);
            data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            break;
    }

    SendPacket(&data);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Trade
#if VERSION_STRING == Cata
void Player::cancelTrade(bool sendback)
{
    if (m_TradeData)
    {
        Player* trade_target = m_TradeData->getTradeTarget();

        if (sendback || trade_target == nullptr)
        {
            GetSession()->sendTradeCancel();
            delete m_TradeData;
            m_TradeData = nullptr;
        }
        else
        {
            trade_target->GetSession()->sendTradeCancel();
            delete trade_target->m_TradeData;
            trade_target->m_TradeData = nullptr;
        }
    }
}

TradeData* TradeData::getTargetTradeData() const
{
    return m_tradeTarget->getTradeData();
}

Item* TradeData::getTradeItem(TradeSlots slot) const
{
    return m_items[slot] ? m_player->GetItemInterface()->GetItemByGUID(m_items[slot]) : nullptr;
}

bool TradeData::hasTradeItem(uint64 item_guid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
        if (m_items[i] == item_guid)
            return true;
    return false;
}

Item* TradeData::getSpellCastItem() const
{
    return m_spellCastItem ? m_player->GetItemInterface()->GetItemByGUID(m_spellCastItem) : nullptr;
}

void TradeData::setItem(TradeSlots slot, Item* item)
{
    ObjectGuid itemGuid;

    if (item)
        itemGuid = item->GetGUID();
    else
        itemGuid = ObjectGuid();

    if (m_items[slot] == itemGuid)
        return;

    m_items[slot] = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade();

    //\todo
    /*if (slot == TRADE_SLOT_NONTRADED)
    GetTargetTradeData()->SetSpell(0);*/

    //\todo
    //SetSpell(0);
}

void TradeData::setSpell(uint32_t spell_id, Item* cast_item /*= nullptr*/)
{
    ObjectGuid itemGuid;
    if (cast_item)
        itemGuid = cast_item->GetGUID();
    else
        itemGuid = ObjectGuid();

    if (m_spell == spell_id && m_spellCastItem == itemGuid)
        return;

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade(true);                   // spell info to owner
    updateTrade(false);                  // spell info to caster
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Messages
void Player::sendReportToGmMessage(std::string playerName, std::string damageLog)
{
    std::string gm_ann(MSG_COLOR_GREEN);

    gm_ann += "|HPlayer:";
    gm_ann += playerName;
    gm_ann += "|h[";
    gm_ann += playerName;
    gm_ann += "]|h: ";
    gm_ann += MSG_COLOR_YELLOW;
    gm_ann += damageLog;

    sWorld.sendMessageToOnlineGms(gm_ann.c_str());
}
