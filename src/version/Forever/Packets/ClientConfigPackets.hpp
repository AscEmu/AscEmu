#pragma once

#include "version/Forever/Packets/Packet.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Version::Forever::Packets
{
    struct CharacterOrderEntry
    {
        uint64_t guidLow{0};
        uint16_t position{0}; // zero based
    };

    struct UpdateAccountDataRequest
    {
        uint64_t time{0};
        uint32_t decompressedSize{0};
        int32_t dataType{-1};
        std::vector<uint8_t> compressedData;
        std::string decompressedData;
        std::vector<CharacterOrderEntry> characterOrder;
    };

    bool parseUpdateAccountData(Packet& packet, UpdateAccountDataRequest& request);
}
