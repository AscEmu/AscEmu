/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

    uint32_t pos = static_cast<uint32_t>(recv_data.rpos());

    ByteBuffer unpackedInfo;
    unpackedInfo.resize(recvSize);

    if (uncompress(unpackedInfo.contents(), &uSize, recv_data.contents() + pos, static_cast<uLong>(recv_data.size() - pos)) == Z_OK)
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

            LOG_DEBUG("AddOn: %s (CRC: 0x%x) - enabled: 0x%x - Unknown2: 0x%x", addonName.c_str(), crc, enabledState, unknown);

            AddonEntry addon(addonName, enabledState, crc, 2, true);

            SavedAddon const* savedAddon = sAddonMgr.getAddonInfoForAddonName(addonName);
            if (savedAddon)
            {
                if (addon.crc != savedAddon->crc)
                {
                    LOG_DEBUG("Addon: %s: modified (CRC: 0x%x) - accountID %d)", addon.name.c_str(), savedAddon->crc, GetAccountId());
                }
                else
                {
                    LOG_DEBUG("Addon: %s: validated (CRC: 0x%x) - accountID %d", addon.name.c_str(), savedAddon->crc, GetAccountId());
                }
            }
            else
            {
                sAddonMgr.SaveAddon(addon);
                LOG_DEBUG("Addon: %s: unknown (CRC: 0x%x) - accountId %d (storing addon name and checksum to database)", addon.name.c_str(), addon.crc, GetAccountId());
            }

            m_addonList.push_back(addon);
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
        data << uint8_t(itr->state);

        uint8_t crcpub = itr->usePublicKeyOrCRC;
        data << uint8_t(crcpub);
        if (crcpub)
        {
            uint8_t usepk = (itr->crc != STANDARD_ADDON_CRC); // standard addon CRC
            data << uint8_t(usepk);
            if (usepk)                                      // add public key if crc is wrong
            {
                LOG_DEBUG("AddOn: %s: CRC checksum mismatch: got 0x%x - expected 0x%x - sending pubkey to accountID %d",
                    itr->name.c_str(), itr->crc, STANDARD_ADDON_CRC, GetAccountId());

                data.append(PublicKey, sizeof(PublicKey));
            }

            data << uint32_t(0);
        }

        data << uint8_t(0);
    }

    m_addonList.clear();

    BannedAddonList const* bannedAddons = sAddonMgr.getBannedAddonsList();
    data << uint32_t(bannedAddons->size());
    for (BannedAddonList::const_iterator itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
    {
        data << uint32_t(itr->id);
        data.append(itr->nameMD5, sizeof(itr->nameMD5));
        data.append(itr->versionMD5, sizeof(itr->versionMD5));
        data << uint32_t(itr->timestamp);
        data << uint32_t(1);  // banned?
    }

    SendPacket(&data);
}

bool WorldSession::isAddonRegistered(const std::string& addon_name) const
{
    if (!isAddonMessageFiltered)
    {
        return true;
    }

    if (mRegisteredAddonPrefixesVector.empty())
    {
        return false;
    }

    std::vector<std::string>::const_iterator itr = std::find(mRegisteredAddonPrefixesVector.begin(), mRegisteredAddonPrefixesVector.end(), addon_name);
    return itr != mRegisteredAddonPrefixesVector.end();
}

void WorldSession::HandleUnregisterAddonPrefixesOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_UNREGISTER_ALL_ADDON_PREFIXES received");

    mRegisteredAddonPrefixesVector.clear();
}

void WorldSession::HandleAddonRegisteredPrefixesOpcode(WorldPacket& recv_data)
{
    uint32_t addonCount = recv_data.readBits(25);

    if (addonCount > REGISTERED_ADDON_PREFIX_SOFTCAP)
    {
        isAddonMessageFiltered = false;
        recv_data.rfinish();
        return;
    }

    std::vector<uint8_t> nameLengths(addonCount);
    for (uint32_t i = 0; i < addonCount; ++i)
    {
        nameLengths[i] = static_cast<uint8_t>(recv_data.readBits(5));
    }

    for (uint32_t i = 0; i < addonCount; ++i)
    {
        mRegisteredAddonPrefixesVector.push_back(recv_data.ReadString(nameLengths[i]));
    }

    if (mRegisteredAddonPrefixesVector.size() > REGISTERED_ADDON_PREFIX_SOFTCAP)
    {
        isAddonMessageFiltered = false;
        return;
    }

    isAddonMessageFiltered = true;
}
