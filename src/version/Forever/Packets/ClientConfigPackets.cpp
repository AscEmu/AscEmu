#include "version/Forever/Packets/ClientConfigPackets.hpp"
#include "Network/ByteBuffer.hpp"

#include <algorithm>
#include <bit>
#include <charconv>
#include <sstream>
#include <string_view>

#include <zlib.h>

namespace AscEmu::Version::Forever::Packets
{
    namespace
    {
        bool skipPackedGuid(ByteBuffer& packet)
        {
            if (packet.remaining() < sizeof(uint16_t))
                return false;

            uint16_t mask = 0;
            packet >> mask;

            const size_t byteCount = static_cast<size_t>(std::popcount(mask));
            if (packet.remaining() < byteCount)
                return false;

            packet.rpos(packet.rpos() + byteCount);
            return true;
        }

        bool parseHexGuidLow(std::string_view token, uint64_t& guidLow)
        {
            // Forever's character-order account data uses entries such as:
            //   137 4618-00B70B31 1
            //   137 2-00000001 1
            //
            // The low counter is the hexadecimal component after the final '-'.
            const size_t dash = token.rfind('-');
            if (dash == std::string_view::npos || dash + 1 >= token.size())
                return false;

            const std::string_view hex = token.substr(dash + 1);
            guidLow = 0;
            const auto [ptr, ec] = std::from_chars(hex.data(), hex.data() + hex.size(), guidLow, 16);

            return ec == std::errc{} && ptr == hex.data() + hex.size();
        }

        void parseCharacterOrder(std::string const& data, std::vector<CharacterOrderEntry>& order)
        {
            order.clear();

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

                if (!(lineStream >> realmToken >> guidToken >> oneBasedPosition))
                    continue;

                uint64_t guidLow = 0;
                if (!parseHexGuidLow(guidToken, guidLow) || oneBasedPosition == 0)
                    continue;

                order.push_back(CharacterOrderEntry{ guidLow, static_cast<uint16_t>(std::min<uint32_t>(oneBasedPosition - 1U, 0xFFFFU)) });
            }
        }
    }

    bool parseUpdateAccountData(ByteBuffer& packet, UpdateAccountDataRequest& request)
    {
        if (packet.remaining() < sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(int32_t) + sizeof(uint32_t))
            return false;

        packet >> request.time;
        packet >> request.decompressedSize;

        if (!skipPackedGuid(packet))
            return false;

        packet >> request.dataType;

        uint32_t compressedSize = 0;
        packet >> compressedSize;

        if (compressedSize > packet.remaining())
            return false;

        request.compressedData.resize(compressedSize);
        if (compressedSize != 0)
        {
            std::memcpy(request.compressedData.data(), packet.contents() + packet.rpos(), compressedSize);
            packet.rpos(packet.rpos() + compressedSize);
        }

        request.decompressedData.clear();
        request.characterOrder.clear();

        if (request.decompressedSize == 0)
            return true;

        request.decompressedData.resize(request.decompressedSize);
        uLongf actualSize = static_cast<uLongf>(request.decompressedSize);

        const int zResult = ::uncompress(reinterpret_cast<Bytef*>(request.decompressedData.data()), &actualSize, reinterpret_cast<const Bytef*>(request.compressedData.data()), static_cast<uLong>(request.compressedData.size()));

        if (zResult != Z_OK || actualSize != request.decompressedSize)
            return false;

        // Observed Forever 69913: data type 16 is the character-list ordering
        // blob. Keep generic account-data parsing separate from this semantic.
        if (request.dataType == 16)
            parseCharacterOrder(request.decompressedData, request.characterOrder);

        return true;
    }
}
