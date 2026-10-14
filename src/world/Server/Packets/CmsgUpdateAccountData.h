/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <zlib.h>

namespace AscEmu::Packets
{
    struct CharacterListOrderEntry
    {
        uint64_t guidLow = 0;
        uint32_t listPosition = 0;
    };

    class CmsgUpdateAccountData : public ManagedPacket
    {
    public:
        uint32_t uiId;
        uint64_t uiTimestamp;
        uint32_t uiDecompressedSize;
        std::vector<uint8_t> compressedData;

        CmsgUpdateAccountData() : CmsgUpdateAccountData(0, 0, 0)
        {
        }

        CmsgUpdateAccountData(uint32_t uiId, uint64_t uiTimestamp, uint32_t uiDecompressedSize) :
            ManagedPacket(CMSG_UPDATE_ACCOUNT_DATA, 4 + 4),
            uiId(uiId),
            uiTimestamp(uiTimestamp),
            uiDecompressedSize(uiDecompressedSize)
        {
        }

        bool parseCharacterListOrder(std::vector<CharacterListOrderEntry>& entries) const
        {
            entries.clear();

            if (!m_protocol.isForever() || uiId != 16)
                return false;

            std::string data;
            if (uiDecompressedSize != 0)
            {
                data.resize(uiDecompressedSize);
                uLongf actualSize = static_cast<uLongf>(uiDecompressedSize);
                const int zResult = ::uncompress(reinterpret_cast<Bytef*>(data.data()), &actualSize, compressedData.data(), static_cast<uLong>(compressedData.size()));
                if (zResult != Z_OK || actualSize != uiDecompressedSize)
                    return false;
            }

            std::istringstream stream(data);
            std::string line;
            bool firstLine = true;

            while (std::getline(stream, line))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                if (firstLine)
                {
                    firstLine = false;
                    if (line == "Version: 2")
                        continue;
                }

                std::istringstream lineStream(line);
                uint32_t realmToken = 0;
                std::string guidToken;
                uint32_t oneBasedPosition = 0;
                if (!(lineStream >> realmToken >> guidToken >> oneBasedPosition) || oneBasedPosition == 0)
                    continue;

                const size_t dash = guidToken.rfind('-');
                if (dash == std::string::npos || dash + 1 >= guidToken.size())
                    continue;

                const std::string_view hex(guidToken.data() + dash + 1, guidToken.size() - dash - 1);
                uint64_t guidLow = 0;
                const auto [ptr, ec] = std::from_chars(hex.data(), hex.data() + hex.size(), guidLow, 16);
                if (ec != std::errc{} || ptr != hex.data() + hex.size())
                    continue;

                entries.push_back({ guidLow, std::min<uint32_t>(oneBasedPosition - 1U, 0xFFFFU) });
            }

            return true;
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Forever 69913+ layout:
                // uint64 time, uint32 decompressedSize, packed ObjectGuid,
                // int32 dataType, uint32 compressedSize, compressed payload.
                if (packet.remaining() < sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(int32_t) + sizeof(uint32_t))
                    return false;

                packet >> uiTimestamp;
                packet >> uiDecompressedSize;

                WoWGuid ownerGuid;
                size_t consumed = 0;
                if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), ownerGuid, consumed))
                    return false;
                packet.rpos(packet.rpos() + consumed);

                int32_t dataType = -1;
                uint32_t compressedSize = 0;
                packet >> dataType;
                packet >> compressedSize;

                if (dataType < 0 || compressedSize > packet.remaining())
                    return false;

                uiId = static_cast<uint32_t>(dataType);
                compressedData.resize(compressedSize);
                if (compressedSize != 0)
                {
                    std::memcpy(compressedData.data(), packet.contents() + packet.rpos(), compressedSize);
                    packet.rpos(packet.rpos() + compressedSize);
                }

                return packet.remaining() == 0;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> uiId;
                if (m_protocol.expansion == WoW::Expansion::_Cata)
                {
                    packet >> uiTimestamp;
                }
                packet >> uiDecompressedSize;

                if (uiDecompressedSize >= 0xFFFF)
                {
                    packet.rfinish();
                    return false;
                }
            }
            else // Mop
            {
                packet >> uiDecompressedSize;
                packet >> uiTimestamp;
                packet >> uiDecompressedSize;
                uiId = packet.readBits(3);

                if (uiDecompressedSize >= 0xFFFF)
                {
                    packet.rfinish();
                    return false;
                }

                packet.rfinish();
            }

            return true;
        }
    };
}
