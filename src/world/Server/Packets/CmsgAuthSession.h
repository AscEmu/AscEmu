/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAuthSession : public ManagedPacket
    {
    public:
        uint32_t accountNameLength = 0;
        std::string accountName;
        uint32_t addonSize = 0;
        uint32_t clientSeed = 0;
        uint32_t clientBuild = 0;
        uint8_t authDigest[20]{0};
        ByteBuffer addonInfoBuffer;

        std::string errorMsg;

        CmsgAuthSession() : ManagedPacket(CMSG_AUTH_SESSION, 2)
        {
        }

    protected:

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.read<uint32_t>();
                packet.read<uint32_t>();
                packet >> authDigest[18];
                packet >> authDigest[14];
                packet >> authDigest[3];
                packet >> authDigest[4];
                packet >> authDigest[0];
                packet.read<uint32_t>();
                packet >> authDigest[11];
                packet >> clientSeed;
                packet >> authDigest[19];
                packet.read<uint8_t>();
                packet.read<uint8_t>();
                packet >> authDigest[2];
                packet >> authDigest[9];
                packet >> authDigest[12];
                packet.read<uint64_t>();
                packet.read<uint32_t>();
                packet >> authDigest[16];
                packet >> authDigest[5];
                packet >> authDigest[6];
                packet >> authDigest[8];

                uint16_t build;
                packet >> build;
                clientBuild = static_cast<uint32_t>(build);

                packet >> authDigest[17];
                packet >> authDigest[7];
                packet >> authDigest[13];
                packet >> authDigest[15];
                packet >> authDigest[1];
                packet >> authDigest[10];

                packet >> addonSize;

                if (addonSize)
                {
                    if (packet.rpos() + addonSize > packet.size())
                    {
                        errorMsg = "Addon size overflow packet size!";
                        return false;
                    }
                    addonInfoBuffer.resize(addonSize);
                    packet.read(static_cast<uint8_t*>(addonInfoBuffer.contents()), addonSize);
                }

                packet.readBit();

                accountNameLength = packet.readBits(11);
                accountName = packet.readString(accountNameLength);

                return true;
            }

            if (m_protocol.isCata())
            {
                packet.read<uint32_t>();
                packet.read<uint32_t>();
                packet.read<uint8_t>();

                packet >> authDigest[10];
                packet >> authDigest[18];
                packet >> authDigest[12];
                packet >> authDigest[5];

                packet.read<uint64_t>();

                packet >> authDigest[15];
                packet >> authDigest[9];
                packet >> authDigest[19];
                packet >> authDigest[4];
                packet >> authDigest[7];
                packet >> authDigest[16];
                packet >> authDigest[3];

                uint16_t build;
                packet >> build;
                clientBuild = static_cast<uint32_t>(build);

                packet >> authDigest[8];

                packet.read<uint32_t>();
                packet.read<uint8_t>();

                packet >> authDigest[17];
                packet >> authDigest[6];
                packet >> authDigest[0];
                packet >> authDigest[1];
                packet >> authDigest[11];

                packet >> clientSeed;

                packet >> authDigest[2];

                packet.read<uint32_t>();

                packet >> authDigest[14];
                packet >> authDigest[13];

                packet >> addonSize;
                if (addonSize)
                {
                    if (packet.rpos() + addonSize > packet.size())
                    {
                        errorMsg = "Addon size overflow packet size!";
                        return false;
                    }
                    addonInfoBuffer.resize(addonSize);
                    packet.read(static_cast<uint8_t*>(addonInfoBuffer.contents()), addonSize);
                }

                packet.readBit();

                accountNameLength = static_cast<uint32_t>(packet.readBits(12));
                accountName = packet.readString(accountNameLength);

                return true;
            }
            
            if (m_protocol.isWotlk())
            {
                packet >> clientBuild;
                packet.read<uint32_t>();
                packet >> accountName;
                packet.read<uint32_t>();
                packet >> clientSeed;
                packet.read<uint64_t>();
                packet.read<uint32_t>();
                packet.read<uint32_t>();
                packet.read<uint32_t>();

                packet.read(authDigest, 20);

                // Unlike Cata/Mop there is no separate addon-size field before this point in the
                // packet - the remaining bytes are [uint32_t compressed size][zlib blob], and that
                // leading size is parsed later by readAddonInfoPacket(), so we just hand over
                // everything that's left here.
                addonSize = static_cast<uint32_t>(packet.size() - packet.rpos());
                if (addonSize)
                {
                    addonInfoBuffer.resize(addonSize);
                    packet.read(static_cast<uint8_t*>(addonInfoBuffer.contents()), addonSize);
                }

                return true;
            }

            if (m_protocol.isTbc() || m_protocol.isClassic())
            {
                packet >> clientBuild;
                packet.read<uint32_t>();
                packet >> accountName;
                packet >> clientSeed;

                packet.read(authDigest, 20);

                // Same reasoning as WotLK above - no separate addon-size field, the remaining
                // bytes are [uint32_t compressed size][zlib blob] handed to readAddonInfoPacket().
                addonSize = static_cast<uint32_t>(packet.size() - packet.rpos());
                if (addonSize)
                {
                    addonInfoBuffer.resize(addonSize);
                    packet.read(static_cast<uint8_t*>(addonInfoBuffer.contents()), addonSize);
                }

                return true;
            }

            return false;
        }
    };
}
