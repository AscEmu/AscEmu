/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/AddonMgr.h"

#include <cstdint>
#include <list>

namespace AscEmu::Packets
{
    class SmsgAddonInfo : public ManagedPacket
    {
    public:
        std::list<AddonEntry>* addonList {nullptr};
        BannedAddonList const* bannedAddons {nullptr};
        uint32_t accountId {0};

        SmsgAddonInfo() : SmsgAddonInfo(nullptr, nullptr, 0)
        {
        }

        SmsgAddonInfo(std::list<AddonEntry>* addonList, BannedAddonList const* bannedAddons, uint32_t accountId) :
            ManagedPacket(SMSG_ADDON_INFO, 100),
            addonList(addonList),
            bannedAddons(bannedAddons),
            accountId(accountId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (addonList == nullptr)
                return 0;

            size_t size = 4 + addonList->size() * 270;    // per-addon block, including a possible 265-byte public key
            if (bannedAddons != nullptr)
                size += bannedAddons->size() * 40;         // per-banned-addon block

            return size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (addonList == nullptr)
                return false;


            if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                for (auto& itr : *addonList)
                {
                    if (itr.crc != STANDARD_ADDON_CRC)
                        packet.append(PublicKey, 264);
                    else
                        packet << uint8_t(2) << uint8_t(1) << uint8_t(0) << uint32_t(0) << uint8_t(0);
                }

                if (m_protocol.isTbc())
                    packet << uint32_t(0);

                return true;
            }
            else if (m_protocol.isWotlk())
            {
                for (auto& itr : *addonList)
                {
                    uint8_t unk;
                    uint8_t unk1;
                    uint8_t unk2;

                    unk = (itr.state ? 2 : 1);
                    packet << unk;

                    unk1 = (itr.state ? 1 : 0);
                    packet << unk1;

                    if (unk1)
                    {
                        if (itr.crc != STANDARD_ADDON_CRC)
                        {
                            packet << uint8_t(1);
                            packet.append(PublicKey, 264);
                        }
                        else
                        {
                            packet << uint8_t(0);
                        }

                        packet << uint32_t(0);
                    }

                    unk2 = (itr.state ? 0 : 1);
                    packet << unk2;

                    if (unk2)
                        packet << uint8_t(0);
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                for (auto& itr : *addonList)
                {
                    packet << uint8_t(itr.state);

                    uint8_t crcpub = itr.usePublicKeyOrCRC;
                    packet << uint8_t(crcpub);
                    if (crcpub)
                    {
                        uint8_t usepk = (itr.crc != STANDARD_ADDON_CRC);    // standard addon CRC
                        packet << uint8_t(usepk);
                        if (usepk)                                          // add public key if crc is wrong
                        {
                            sLogger.debug("AddOn: {}: CRC checksum mismatch: got 0x{:x} - expected 0x{:x} - sending pubkey to accountID {}",
                                itr.name, itr.crc, STANDARD_ADDON_CRC, accountId);

                            packet.append(PublicKey, sizeof(PublicKey));
                        }

                        packet << uint32_t(0);
                    }

                    packet << uint8_t(0);
                }

                packet << uint32_t(bannedAddons ? static_cast<uint32_t>(bannedAddons->size()) : 0);
                if (bannedAddons)
                {
                    for (auto itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
                    {
                        packet << uint32_t(itr->id);
                        packet.append(itr->nameMD5, sizeof(itr->nameMD5));
                        packet.append(itr->versionMD5, sizeof(itr->versionMD5));
                        packet << uint32_t(itr->timestamp);
                        packet << uint32_t(1); // banned?
                    }
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(bannedAddons ? static_cast<uint32_t>(bannedAddons->size()) : 0, 18);
                packet.writeBits(static_cast<uint32_t>(addonList->size()), 23);

                for (auto& itr : *addonList)
                {
                    packet.writeBit(0); // Has URL
                    packet.writeBit(itr.enabled);
                    packet.writeBit(!itr.usePublicKeyOrCRC);
                }

                packet.flushBits();

                for (auto& itr : *addonList)
                {
                    if (!itr.usePublicKeyOrCRC)
                    {
                        const size_t pos = packet.wpos();
                        for (int i = 0; i < 256; i++)
                            packet << uint8_t(0);

                        for (int i = 0; i < 256; i++)
                            packet.put<uint8_t>(pos + publicKeyOrder[i], PublicKey[i]);
                    }

                    if (itr.enabled)
                    {
                        packet << itr.enabled;
                        packet << static_cast<uint32_t>(0);
                    }

                    packet << itr.state;
                }

                if (bannedAddons)
                {
                    for (auto itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
                    {
                        packet << uint32_t(itr->id);
                        packet << uint32_t(1); // banned?

                        for (int32_t i = 0; i < 8; i++)
                            packet << uint32_t(0);

                        packet << uint32_t(itr->timestamp);
                    }
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
