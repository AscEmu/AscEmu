/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/DamageInfo.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAttackerstateupdate : public ManagedPacket
    {
    public:
        WoWGuid attackerGuid;
        WoWGuid victimGuid;
        HitStatus hitStatus;
        uint32_t damage;
        uint32_t overKill;
        DamageInfo damageInfo;
        uint32_t absorbedDamage;
        VisualState visualState;
        uint32_t blockedDamage;
        uint32_t rageGain;

        SmsgAttackerstateupdate() :
            SmsgAttackerstateupdate(WoWGuid(), WoWGuid(), HitStatus(0), 0, 0, DamageInfo(), 0, VisualState::MISS, 0, 0)
        {
        }

        SmsgAttackerstateupdate(WoWGuid attackerGuid, WoWGuid victimGuid, HitStatus hitStatus, uint32_t damage, uint32_t overKill,
            DamageInfo damageInfo, uint32_t absorbedDamage, VisualState visualState, uint32_t blockedDamage, uint32_t rageGain) :
            ManagedPacket(SMSG_ATTACKERSTATEUPDATE, 114),
            attackerGuid(attackerGuid), victimGuid(victimGuid), hitStatus(hitStatus), damage(damage), overKill(overKill),
            damageInfo(damageInfo), absorbedDamage(absorbedDamage), visualState(visualState), blockedDamage(blockedDamage), rageGain(rageGain)
        {
        }

    protected:
        size_t expectedSize() const override { return 114; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
#if VERSION_STRING == Mop
                ByteBuffer buff;

                buff << uint32_t(hitStatus);
                buff << attackerGuid;
                buff << victimGuid;

                buff << uint32_t(damage);                                     // full damage
                buff << uint32_t(overKill);                                   // overkill

                buff << uint8_t(1);                                           // sub damage count

                buff << uint32_t(damageInfo.schoolMask);                      // school of sub damage
                buff << float(damage);                                       // sub damage
                buff << uint32_t(damage);                                     // sub damage

                if (hitStatus & HITSTATUS_ABSORBED)
                    buff << uint32_t(absorbedDamage);

                if (hitStatus & HITSTATUS_RESIST)
                    buff << uint32_t(damageInfo.resistedDamage);

                buff << uint8_t(visualState);
                buff << uint32_t(0);                                         // unk, can be 0, 1000 or -1
                buff << uint32_t(0);                                         // unk, probably GetMeleeSpell

                if (hitStatus & HITSTATUS_BLOCK)
                    buff << uint32_t(blockedDamage);

                // HITSTATUS_RAGE_GAIN only exists in the post-TBC HitStatus enum.
                if (hitStatus & HITSTATUS_RAGE_GAIN)
                    buff << uint32_t(0);                                     // real client never reads this as rage amount

                if (hitStatus & HITSTATUS_UNK_00)                            // debug information
                {
                    buff << uint32_t(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);
                    buff << float(0);

                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        buff << float(0);
                        buff << float(0);
                    }
                    buff << uint32_t(0);
                }

                // HITSTATUS_UNK_04 only exists in the post-TBC HitStatus enum.
                if (hitStatus & (HITSTATUS_BLOCK | HITSTATUS_UNK_04))
                    buff << float(0);


                packet.writeBit(0);                                          // hasSpellCastLogData
                packet.flushBits();
                packet << uint32_t(buff.size());
                packet.append(buff);

                return true;
#endif
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                // School type in classic, school mask in tbc+
                const uint32_t school = damageInfo.schoolMask;

                packet << uint32_t(hitStatus);
                packet << attackerGuid;
                packet << victimGuid;

                packet << uint32_t(damage);                                   // real damage

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << uint32_t(overKill);

                packet << uint8_t(1);                                         // damage counter

                packet << uint32_t(school);                                   // damage school
                packet << float(1.0f);                                        // some sort of damage coefficient
                packet << uint32_t(damage);                                   // full damage in int

                if (hitStatus & HITSTATUS_ABSORBED)
                    packet << uint32_t(absorbedDamage);

                if (hitStatus & HITSTATUS_RESIST)
                    packet << uint32_t(damageInfo.resistedDamage);

                packet << uint8_t(visualState);
                packet << uint32_t(0);                                        // unk, can be 0, 1000 or -1
                packet << uint32_t(0);                                        // unk, probably GetMeleeSpell

                if (hitStatus & HITSTATUS_BLOCK)
                    packet << uint32_t(blockedDamage);

                // HITSTATUS_RAGE_GAIN only exists in the post-TBC HitStatus enum.
                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    if (hitStatus & HITSTATUS_RAGE_GAIN)
                        packet << uint32_t(rageGain);
                }

                if (hitStatus & HITSTATUS_UNK_00)                             // debug information
                {
                    packet << uint32_t(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);

                    packet << float(0);                                       // Found in loop
                    packet << float(0);                                       // Found in loop
                    packet << uint32_t(0);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                // School type in classic, school mask in tbc+
                const uint32_t school = m_protocol.isClassic() ? damageInfo.getSchoolTypeFromMask() : damageInfo.schoolMask;

                packet << uint32_t(hitStatus);
                packet << attackerGuid;
                packet << victimGuid;
                packet << uint32_t(damage);                         // real damage

                packet << uint8_t(1);                               // damage counter

                packet << uint32_t(school);                         // damage school
                packet << float(damageInfo.fullDamage);
                packet << uint32_t(damageInfo.fullDamage);
                packet << uint32_t(absorbedDamage);
                packet << int32_t(damageInfo.resistedDamage);

                packet << uint32_t(visualState);
                packet << uint32_t(0);
                packet << uint32_t(0);

                packet << uint32_t(blockedDamage);

                if (hitStatus & HITSTATUS_UNK_00)
                {
                    packet << uint32_t(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);
                    packet << float(0);

                    for (uint8_t i = 0; i < 5; ++i)
                    {
                        packet << float(0);
                        packet << float(0);
                    }
                    packet << uint32_t(0);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
