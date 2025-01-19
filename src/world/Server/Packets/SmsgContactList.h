/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

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
#if VERSION_STRING > Classic
            packet << socialFlag << listCount;
#else
            packet << uint8_t(listCount);
#endif

            for (const auto listMember : contactMemberList)
            {
                packet << listMember.guid;
#if VERSION_STRING == Mop
                packet << uint32_t(0) << uint32_t(0);
#endif
#if VERSION_STRING > Classic
                packet << listMember.flag << listMember.note;

                if (listMember.flag & 0x1)
                {
#endif
                    packet << listMember.isOnline;
                    if (listMember.isOnline)
                        packet << listMember.zoneId << listMember.level << listMember.playerClass;
#if VERSION_STRING > Classic
                }
#endif
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
