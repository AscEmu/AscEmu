/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Units/Players/Player.h"
#include "Server/World.Legacy.h"
#include "Server/World.h"
#include "Spell/Definitions/AuraInterruptFlags.h"

void Player::sendForceMovePacket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(60);
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_TURN_RATE_CHANGE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_PITCH_RATE_CHANGE, speed);
            break;
        }
    }

    SendMessageToSet(&data, true);
}

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
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_SPEED, speed);
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_BACK_SPEED, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_SPEED, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_BACK_SPEED, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_TURN_RATE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_SPEED, speed);
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_BACK_SPEED, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_PITCH_RATE, speed);
            break;
        }
    }

    SendMessageToSet(&data, true);
}


void Player::handleFall(MovementInfo const& movement_info)
{
    if (!z_axisposition)
    {
        z_axisposition = movement_info.getPosition()->z;
    }

    uint32 falldistance = float2int32(z_axisposition - movement_info.getPosition()->z);
    if (z_axisposition <= movement_info.getPosition()->z)
    {
        falldistance = 1;
    }

    if (static_cast<int>(falldistance) > m_safeFall)
    {
        falldistance -= m_safeFall;
    }
    else
    {
        falldistance = 1;
    }

    if (isAlive() && !bInvincible && (falldistance > 12) && !m_noFallDamage && ((!GodModeCheat && (UNIXTIME >= m_fallDisabledUntil))))
    {
        auto health_loss = static_cast<uint32_t>(GetHealth() * (falldistance - 12) * 0.017f);

        if (health_loss >= GetHealth())
        {
            health_loss = GetHealth();
        }
        else if ((falldistance >= 65))
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, GetDrunkenstateByValue(GetDrunkValue()), 0);
        }

        SendEnvironmentalDamageLog(GetGUID(), DAMAGE_FALL, health_loss);
        DealDamage(this, health_loss, 0, 0, 0);
    }

    z_axisposition = 0.0f;
}

bool Player::isPlayerJumping(MovementInfo const& movement_info, uint16_t opcode)
{
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

    return false;
}

void Player::handleBreathing(MovementInfo& movement_info, WorldSession* session)
{
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
}

static const uint32_t LanguageSkills[NUM_LANGUAGES] =
{
    0,              // UNIVERSAL          0x00
    109,            // ORCISH             0x01
    113,            // DARNASSIAN         0x02
    115,            // TAURAHE            0x03
    0,                // -                0x04
    0,                // -                0x05
    111,            // DWARVISH           0x06
    98,             // COMMON             0x07
    139,            // DEMON TONGUE       0x08
    140,            // TITAN              0x09
    137,            // THALSSIAN          0x0A
    138,            // DRACONIC           0x0B
    0,              // KALIMAG            0x0C
    313,            // GNOMISH            0x0D
    315,            // TROLL              0x0E
    0,                // -                0x0F
    0,                // -                0x10
    0,                // -                0x11
    0,                // -                0x12
    0,                // -                0x13
    0,                // -                0x14
    0,                // -                0x15
    0,                // -                0x16
    0,                // -                0x17
    0,                // -                0x18
    0,                // -                0x19
    0,                // -                0x1A
    0,                // -                0x1B
    0,                // -                0x1C
    0,                // -                0x1D
    0,                // -                0x1E
    0,                // -                0x1F
    0,                // -                0x20
    673,            // GUTTERSPEAK        0x21
    0,                // -                0x22
    759,            // DRAENEI            0x23
    0,              // ZOMBIE             0x24
    0,              // GNOMISH_BINAR      0x25
    0,              // GOBLIN_BINARY      0x26
    791,            // WORGEN             0x27
    792,            // GOBLIN             0x28
};

bool Player::hasSkilledSkill(uint32_t skill)
{
    return (m_skills.find(skill) != m_skills.end());
}

bool Player::hasLanguage(uint32_t language)
{
    return hasSkilledSkill(LanguageSkills[language]);
}

WorldPacket Player::buildChatMessagePacket(Player* targetPlayer, uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag)
{
    Player* senderPlayer = objmgr.GetPlayer(guid);

    uint32_t messageLength = (uint32_t)strlen(message) + 1;
    WorldPacket data(SMSG_MESSAGECHAT, messageLength + 60);
    data << uint8_t(type);

    if (targetPlayer->hasLanguage(language)
        || targetPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM)
        || (senderPlayer && senderPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
        || worldConfig.player.isInterfactionChatEnabled)
    {
        data << 0;
    }
    else
    {
        data << language;
    }

    data << guid;
    data << uint32_t(0);

    data << targetPlayer->GetGUID();

    data << messageLength;
    data << message;

    data << uint8_t(flag);
    return data;
}

void Player::sendChatPacket(uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag)
{
    if (IsInWorld() == false)
    {
        return;
    }

    uint32_t senderPhase = GetPhase();

    for (std::set<Object*>::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Player* p = static_cast<Player*>(*itr);
        if (p->GetSession() && !p->Social_IsIgnoring(GetLowGUID()) && (p->GetPhase() & senderPhase) != 0)
        {
            WorldPacket data = p->buildChatMessagePacket(p, type, language, message, guid, flag);
            p->SendPacket(&data);
        }
    }

    // send to self
    WorldPacket selfData = buildChatMessagePacket(this, type, language, message, guid, flag);
    this->SendPacket(&selfData);
}
