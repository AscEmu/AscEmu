/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Server/WorldSession.h"
#include "Log.hpp"
#include "zlib.h"

void WorldSession::readAddonInfoPacket(ByteBuffer &recv_data)
{
    if (recv_data.rpos() + 4 > recv_data.size())
        return;

    uint32_t recvSize;
    recv_data >> recvSize;

    if (!recvSize)
        return;

    if (recvSize > 0xFFFFF)
    {
        LOG_DEBUG("recvSize %u too bog", recvSize);
        return;
    }

    uLongf uSize = recvSize;

    uint32_t pos = recv_data.rpos();

    ByteBuffer unpackedInfo;
    unpackedInfo.resize(recvSize);

    if (uncompress(unpackedInfo.contents(), &uSize, recv_data.contents() + pos, recv_data.size() - pos) == Z_OK)
    {
        uint32_t addonsCount;
        unpackedInfo >> addonsCount;

        for (uint32_t i = 0; i < addonsCount; ++i)
        {
            std::string addonName;
            uint8_t enabledState;
            uint32_t crc;
            uint32_t unknown;

            if (unpackedInfo.rpos() + 1 > unpackedInfo.size())
                return;

            unpackedInfo >> addonName;

            unpackedInfo >> enabledState;
            unpackedInfo >> crc;
            unpackedInfo >> unknown;

            LOG_DEBUG("Decompression of %s - enabled: 0x%x - unknown: 0x%x", addonName.c_str(), enabledState, unknown);

            AddonEntry addonEntry(addonName, enabledState, crc, 2, true);

            m_addonList.push_back(addonEntry);
        }

        uint32_t addonTime;
        unpackedInfo >> addonTime;
    }
    else
    {
        LOG_ERROR("Decompression of addon section of CMSG_AUTH_SESSION failed.");
    }
}

void WorldSession::sendAddonInfo()
{
    WorldPacket data(SMSG_ADDON_INFO, 4);
    for (AddonsList::iterator itr = m_addonList.begin(); itr != m_addonList.end(); ++itr)
    {
        data << uint8_t(itr->status);

        uint8_t crcpub = itr->isNew;
        data << uint8_t(crcpub);
        if (crcpub)
        {
            uint8_t packedState = (itr->crc != STANDARD_ADDON_CRC);
            data << uint8_t(packedState);
            if (packedState)
                data.append(PublicKey, sizeof(PublicKey));

            data << uint32_t(0);
        }
        data << uint8_t(0);
    }
    m_addonList.clear();

    SendPacket(&data);
}
