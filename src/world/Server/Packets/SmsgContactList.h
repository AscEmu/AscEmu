/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgContactListMember
    {
        uint64_t guid = 0;
        uint32_t flag = 0;
        std::string note;

        uint8_t isOnline = 0;
        uint32_t zoneId = 0;
        uint32_t level = 0;
        uint32_t playerClass = 0;
    };

    class SmsgContactList : public ManagedPacket
    {
    public:
        uint32_t socialFlag;
        uint32_t listCount;
        std::vector<SmsgContactListMember> contactMemberList;

        SmsgContactList() : SmsgContactList(0, {})
        {
        }

        SmsgContactList(uint32_t socialFlag, const std::vector<SmsgContactListMember> contactMemberList) :
            ManagedPacket(SMSG_CONTACT_LIST, 500),
            socialFlag(socialFlag),
            listCount(static_cast<uint32_t>(contactMemberList.size())),
            contactMemberList(contactMemberList)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA())
            {
                // flags, count, then every contact with its realms, status and note
                packet << socialFlag;
                packet.writeBits(listCount, 8);

                for (const auto& listMember : contactMemberList)
                {
                    packet << WoWGuid(listMember.guid).toGuid128(m_protocol.realmId, 0);
                    packet << WoWGuid128();                                 // account of the contact
                    packet << uint32_t(m_protocol.getVirtualRealmAddress());
                    packet << uint32_t(m_protocol.getVirtualRealmAddress());
                    packet << listMember.flag;
                    packet << uint8_t(listMember.isOnline);
                    packet << listMember.zoneId;
                    packet << listMember.level;
                    packet << listMember.playerClass;
                    packet.writeBits(static_cast<uint32_t>(listMember.note.length()), 10);
                    if (m_protocol.isBfA())
                        packet.writeBit(false);         // mobile
                    packet.flushBits();
                    packet.writeString(listMember.note);
                }

                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Classic)
            {
                packet << static_cast<uint8_t>(listCount);
                for (const auto& listMember : contactMemberList)
                {
                    packet << listMember.guid;
                    packet << listMember.isOnline;
                    if (listMember.isOnline)
                        packet << listMember.zoneId << listMember.level << listMember.playerClass;
                }
            }

            if (m_protocol.expansion > WoW::Expansion::_Classic)
            {
                packet << socialFlag << listCount;

                for (const auto& listMember : contactMemberList)
                {
                    packet << listMember.guid;
                    if (m_protocol.expansion == WoW::Expansion::_Mop)
                    {
                        packet << static_cast<uint32_t>(0) << static_cast<uint32_t>(0);
                    }
                    packet << listMember.flag << listMember.note;

                    if (listMember.flag & 0x1)
                    {
                        packet << listMember.isOnline;
                        if (listMember.isOnline)
                            packet << listMember.zoneId << listMember.level << listMember.playerClass;
                    }
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
