/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetSocket.hpp"
#include "BattleNetComm.hpp"

#include "BNetAuthTicketStore.hpp"
#include "BNetConfig.hpp"
#include "BNetTlsContext.hpp"
#include "BNetProtocol.hpp"
#include "Master.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <zlib.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace AscEmu::Battlenet
{
    extern std::unique_ptr<Database> sBNetCharacterSQL;

    namespace
    {
        constexpr uint32_t BNET_SOCKET_BUFFER_SIZE = 64 * 1024;
        constexpr size_t TLS_IO_BUFFER_SIZE = 16 * 1024;
        constexpr size_t HEX_DUMP_BYTES_PER_LINE = 16;

        std::atomic<uint64_t> nextConnectionId{ 1 };

        constexpr size_t MAX_RPC_HEADER_SIZE = 4 * 1024;
        constexpr size_t MAX_RPC_PAYLOAD_SIZE = 16 * 1024 * 1024;
        constexpr size_t MAX_DIAGNOSTIC_DUMP_SIZE = 2 * 1024;

        // Protocol service/method identifiers live in BNetProtocol.hpp.
        // Keep this translation unit focused on request decoding and session behavior.

        constexpr char WEB_AUTH_URL[] =
            "https://bnet.ascemu.local:8081/bnetserver/login/";

        uint32_t getServerEpochSeconds()
        {
            static const uint32_t epoch = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            return epoch;
        }

        bool extractRealmListClientSecret(const uint8_t* payload, size_t payloadSize, std::array<uint8_t, 32>& secret)
        {
            if (payload == nullptr || payloadSize == 0)
                return false;

            const std::string_view text(reinterpret_cast<const char*>(payload), payloadSize);
            size_t cursor = text.find("\"secret\"");
            if (cursor == std::string_view::npos)
                return false;

            cursor = text.find('[', cursor);
            if (cursor == std::string_view::npos)
                return false;
            ++cursor;

            for (size_t index = 0; index < secret.size(); ++index)
            {
                while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor])) != 0)
                    ++cursor;

                if (cursor >= text.size() || !std::isdigit(static_cast<unsigned char>(text[cursor])))
                    return false;

                uint32_t value = 0;
                do
                {
                    value = value * 10u + static_cast<uint32_t>(text[cursor] - '0');
                    if (value > 255u)
                        return false;
                    ++cursor;
                } while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0);

                secret[index] = static_cast<uint8_t>(value);

                while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor])) != 0)
                    ++cursor;

                if (index + 1 < secret.size())
                {
                    if (cursor >= text.size() || text[cursor] != ',')
                        return false;
                    ++cursor;
                }
            }

            while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor])) != 0)
                ++cursor;

            return cursor < text.size() && text[cursor] == ']';
        }

        void appendVarInt(std::vector<uint8_t>& output, uint64_t value)
        {
            while (value >= 0x80u)
            {
                output.push_back(static_cast<uint8_t>((value & 0x7Fu) | 0x80u));
                value >>= 7;
            }
            output.push_back(static_cast<uint8_t>(value));
        }

        void appendVarIntField(std::vector<uint8_t>& output, uint32_t fieldNumber, uint64_t value)
        {
            appendVarInt(output, static_cast<uint64_t>(fieldNumber) << 3);
            appendVarInt(output, value);
        }

        void appendMessageField(std::vector<uint8_t>& output, uint32_t fieldNumber, const std::vector<uint8_t>& message)
        {
            appendVarInt(output, (static_cast<uint64_t>(fieldNumber) << 3) | 2u);
            appendVarInt(output, message.size());
            output.insert(output.end(), message.begin(), message.end());
        }

        void appendStringField(std::vector<uint8_t>& output, uint32_t fieldNumber, const std::string& value)
        {
            appendVarInt(output, (static_cast<uint64_t>(fieldNumber) << 3) | 2u);
            appendVarInt(output, value.size());
            output.insert(output.end(), value.begin(), value.end());
        }

        void appendFixed32Field(std::vector<uint8_t>& output, uint32_t fieldNumber, uint32_t value)
        {
            appendVarInt(output, (static_cast<uint64_t>(fieldNumber) << 3) | 5u);
            output.push_back(static_cast<uint8_t>(value & 0xFFu));
            output.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
            output.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
            output.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
        }

        std::string buildClientInstanceId(uint32_t serverLabel, uint32_t serverEpoch, uint32_t clientLabel, uint32_t clientEpoch)
        {
            std::ostringstream stream;
            stream << std::uppercase << std::hex << std::setfill('0')
                   << std::setw(8) << serverLabel
                   << std::setw(8) << serverEpoch
                   << '-'
                   << std::setw(8) << clientLabel
                   << std::setw(8) << clientEpoch;
            return stream.str();
        }

        std::vector<uint8_t> buildProcessId(uint32_t label, uint32_t epoch)
        {
            std::vector<uint8_t> result;
            result.reserve(12);
            appendVarIntField(result, 1, label);
            appendVarIntField(result, 2, epoch);
            return result;
        }

        struct RpcHeaderDiagnostic
        {
            bool valid = true;
            bool hasServiceId = false;
            uint32_t serviceId = 0;
            bool hasMethodId = false;
            uint32_t methodId = 0;
            bool hasToken = false;
            uint32_t token = 0;
            bool hasSize = false;
            uint32_t size = 0;
            bool hasStatus = false;
            uint32_t status = 0;
            bool hasIsResponse = false;
            bool isResponse = false;
            bool hasServiceHash = false;
            uint32_t serviceHash = 0;
        };

        bool readVarInt(const uint8_t*& cursor, const uint8_t* end, uint64_t& value)
        {
            value = 0;
            uint32_t shift = 0;

            while (cursor < end && shift < 64)
            {
                const uint8_t byte = *cursor++;
                value |= static_cast<uint64_t>(byte & 0x7Fu) << shift;
                if ((byte & 0x80u) == 0)
                    return true;
                shift += 7;
            }

            return false;
        }

        bool skipProtobufField(uint32_t wireType, const uint8_t*& cursor, const uint8_t* end)
        {
            switch (wireType)
            {
                case 0:
                {
                    uint64_t ignored = 0;
                    return readVarInt(cursor, end, ignored);
                }
                case 1:
                    if (static_cast<size_t>(end - cursor) < 8)
                        return false;
                    cursor += 8;
                    return true;
                case 2:
                {
                    uint64_t length = 0;
                    if (!readVarInt(cursor, end, length) || length > static_cast<uint64_t>(end - cursor))
                        return false;
                    cursor += static_cast<size_t>(length);
                    return true;
                }
                case 5:
                    if (static_cast<size_t>(end - cursor) < 4)
                        return false;
                    cursor += 4;
                    return true;
                default:
                    return false;
            }
        }

        bool readLengthDelimited(const uint8_t*& cursor, const uint8_t* end, const uint8_t*& data, size_t& size)
        {
            uint64_t length = 0;
            if (!readVarInt(cursor, end, length) || length > static_cast<uint64_t>(end - cursor))
                return false;

            data = cursor;
            size = static_cast<size_t>(length);
            cursor += size;
            return true;
        }

        std::vector<uint8_t> makeGameUtilitiesStringVariantList(const std::vector<std::string>& values)
        {
            // bgs.protocol.game_utilities.v1.GetAllValuesForAttributeResponse:
            // repeated Variant attribute_value = field 1
            // Variant.string_value = field 4
            std::vector<uint8_t> response;

            for (const std::string& value : values)
            {
                std::vector<uint8_t> variant;
                appendStringField(variant, 4, value);
                appendMessageField(response, 1, variant);
            }

            return response;
        }

        std::string jsonEscape(const std::string& value)
        {
            std::ostringstream out;
            for (const unsigned char c : value)
            {
                switch (c)
                {
                    case '\\': out << "\\\\"; break;
                    case '"': out << "\\\""; break;
                    case '\b': out << "\\b"; break;
                    case '\f': out << "\\f"; break;
                    case '\n': out << "\\n"; break;
                    case '\r': out << "\\r"; break;
                    case '\t': out << "\\t"; break;
                    default:
                        if (c < 0x20u)
                            out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned>(c) << std::dec;
                        else
                            out << static_cast<char>(c);
                        break;
                }
            }
            return out.str();
        }

        std::string makeCompactHex(const uint8_t* data, size_t size, size_t maxBytes = MAX_DIAGNOSTIC_DUMP_SIZE)
        {
            if (data == nullptr || size == 0)
                return "<empty>";

            const size_t dumpSize = std::min(size, maxBytes);
            std::ostringstream out;
            out << std::hex << std::uppercase << std::setfill('0');
            for (size_t index = 0; index < dumpSize; ++index)
            {
                if (index != 0)
                    out << ' ';
                out << std::setw(2) << static_cast<unsigned>(data[index]);
            }

            if (dumpSize < size)
                out << " ... (" << std::dec << (size - dumpSize) << " more byte(s))";

            return out.str();
        }

        std::vector<uint8_t> compressGameUtilitiesJson(const std::string& json)
        {
            // Blizzard's realm-list JSON blobs start with a little-endian uint32
            // containing the uncompressed byte count including the trailing NUL,
            // followed by a zlib stream of the NUL-terminated JSON text.
            const uLong sourceSize = static_cast<uLong>(json.size() + 1u);
            uLongf compressedSize = compressBound(sourceSize);

            std::vector<uint8_t> result(4u + compressedSize);
            const uint32_t uncompressedSize = static_cast<uint32_t>(sourceSize);
            result[0] = static_cast<uint8_t>(uncompressedSize & 0xFFu);
            result[1] = static_cast<uint8_t>((uncompressedSize >> 8) & 0xFFu);
            result[2] = static_cast<uint8_t>((uncompressedSize >> 16) & 0xFFu);
            result[3] = static_cast<uint8_t>((uncompressedSize >> 24) & 0xFFu);

            const int zResult = compress(reinterpret_cast<Bytef*>(result.data() + 4u), &compressedSize, reinterpret_cast<const Bytef*>(json.c_str()), sourceSize);

            if (zResult != Z_OK)
                return {};

            result.resize(4u + compressedSize);
            return result;
        }

        void appendGameUtilitiesBlobAttribute(std::vector<uint8_t>& response, const std::string& name, const std::vector<uint8_t>& blob)
        {
            std::vector<uint8_t> variant;
            appendVarInt(variant, (static_cast<uint64_t>(5) << 3) | 2u);
            appendVarInt(variant, blob.size());
            variant.insert(variant.end(), blob.begin(), blob.end());

            std::vector<uint8_t> attribute;
            appendStringField(attribute, 1, name);
            appendMessageField(attribute, 2, variant);
            appendMessageField(response, 1, attribute);
        }

        void appendGameUtilitiesUInt64Attribute(std::vector<uint8_t>& response, const std::string& name, uint64_t value)
        {
            // bgs.protocol.Variant field 2 = unsigned/integer value.
            std::vector<uint8_t> variant;
            appendVarInt(variant, (static_cast<uint64_t>(2) << 3) | 0u);
            appendVarInt(variant, value);

            std::vector<uint8_t> attribute;
            appendStringField(attribute, 1, name);
            appendMessageField(attribute, 2, variant);
            appendMessageField(response, 1, attribute);
        }

        void appendGameUtilitiesStringAttribute(std::vector<uint8_t>& response, const std::string& name, const std::string& value)
        {
            // bgs.protocol.Variant field 4 = string.
            std::vector<uint8_t> variant;
            appendStringField(variant, 4, value);

            std::vector<uint8_t> attribute;
            appendStringField(attribute, 1, name);
            appendMessageField(attribute, 2, variant);
            appendMessageField(response, 1, attribute);
        }

        bool commandMatches(std::string_view command, std::string_view semanticPrefix)
        {
            return command.size() >= semanticPrefix.size() &&
                   command.compare(0, semanticPrefix.size(), semanticPrefix) == 0;
        }

        [[nodiscard]] constexpr bool usesForever1601Transport(uint32_t build) { return build == 69893u || build == 70009u; }

        struct ClientVersionParts
        {
            uint32_t major;
            uint32_t minor;
            uint32_t revision;
        };

        ClientVersionParts getClientVersionParts(uint32_t build)
        {
            switch (build)
            {
                case 69722u: return { 1u, 15u, 9u };
                case 69795u: return { 2u, 5u, 6u };
                case 69585u: return { 5u, 5u, 4u };
                case 69814u: return { 12u, 1u, 0u };
                case 69893u: return { 1u, 60u, 1u };
                case 70009u: return { 1u, 60u, 1u };
                default:     return { 0u, 0u, 0u };
            }
        }

        uint32_t makeRealmAddress(uint8_t region, uint8_t site, uint16_t realmId)
        {
            return (static_cast<uint32_t>(region) << 24) |
                   (static_cast<uint32_t>(site) << 16) |
                   static_cast<uint32_t>(realmId);
        }

        struct ForeverSuperDistrictProfile
        {
            uint32_t build = 0;
            uint32_t collectionId = 0;
            uint32_t superDistrictSetId = 0;

            uint32_t pveAvailableSuperDistrictId = 0;
            uint32_t pvpAvailableSuperDistrictId = 0;
            uint32_t roleplayAvailableSuperDistrictId = 0;
            uint32_t hardcoreAvailableSuperDistrictId = 0;

            uint32_t currentCfgContentSetId = 0;
            bool contentSetIdKnown = false;
        };


        ForeverSuperDistrictProfile getForeverSuperDistrictProfile(uint32_t clientBuild)
        {
            // Build 70009 currently exposes four Forever rulesets:
            //
            //   PvE
            //   PvP
            //   Roleplay
            //   Hardcore
            //
            // The SuperDistrict DB2 relationships stored here are diagnostic metadata
            // only. These values must not be serialized as invented Battle.net fields.
            //
            // The previously verified relationship was:
            //
            //   SuperDistrictSetCollection 1 -> SuperDistrictSet 36
            //   AvailableSuperDistrict 2 -> PvP
            //   AvailableSuperDistrict 3 -> PvE / Normal
            //
            // Roleplay, Hardcore and the ContentSet selector for build 70009 still
            // need to be verified from the extracted 70009 DB2 data. Keep them unset
            // rather than guessing IDs.
            if (clientBuild == 70009u)
            {
                return {
                    .build = 70009u,
                    .collectionId = 1u,
                    .superDistrictSetId = 36u,
                    .pveAvailableSuperDistrictId = 3u,
                    .pvpAvailableSuperDistrictId = 2u,
                    .roleplayAvailableSuperDistrictId = 0u,
                    .hardcoreAvailableSuperDistrictId = 0u,
                    .currentCfgContentSetId = 0u,
                    .contentSetIdKnown = false,
                };
            }

            return {};
        }

        uint32_t getRealmCfgTimezonesId(uint32_t clientBuild)
        {
            // Forever 1.60.1.69893 test value.
            //
            // The official Camelot capture confirmed the sub-region "70-1-70",
            // but it did not expose cfgTimezonesID in clear text.  Keep this
            // isolated by build while testing the client-side SuperDistrict
            // mapping used by the play-style picker.
            if (clientBuild == 69893u)
                return 1u;

            return 1u;
        }

        bool readGameUtilitiesBlobAttribute(const uint8_t* payload, size_t payloadSize, const std::string& wantedName, std::vector<uint8_t>& value)
        {
            value.clear();
            if (payload == nullptr || payloadSize == 0)
                return false;

            const uint8_t* cursor = payload;
            const uint8_t* end = payload + payloadSize;

            while (cursor < end)
            {
                uint64_t key = 0;
                if (!readVarInt(cursor, end, key))
                    return false;

                const uint32_t fieldNumber = static_cast<uint32_t>(key >> 3);
                const uint32_t wireType = static_cast<uint32_t>(key & 0x07u);
                if (fieldNumber != 1u || wireType != 2u)
                {
                    if (!skipProtobufField(wireType, cursor, end))
                        return false;
                    continue;
                }

                const uint8_t* attributeData = nullptr;
                size_t attributeSize = 0;
                if (!readLengthDelimited(cursor, end, attributeData, attributeSize))
                    return false;

                std::string name;
                const uint8_t* variantData = nullptr;
                size_t variantSize = 0;
                const uint8_t* attributeCursor = attributeData;
                const uint8_t* attributeEnd = attributeData + attributeSize;

                while (attributeCursor < attributeEnd)
                {
                    uint64_t attributeKey = 0;
                    if (!readVarInt(attributeCursor, attributeEnd, attributeKey))
                        return false;

                    const uint32_t attributeField = static_cast<uint32_t>(attributeKey >> 3);
                    const uint32_t attributeWire = static_cast<uint32_t>(attributeKey & 0x07u);
                    if (attributeField == 1u && attributeWire == 2u)
                    {
                        const uint8_t* data = nullptr;
                        size_t size = 0;
                        if (!readLengthDelimited(attributeCursor, attributeEnd, data, size))
                            return false;
                        name.assign(reinterpret_cast<const char*>(data), size);
                    }
                    else if (attributeField == 2u && attributeWire == 2u)
                    {
                        if (!readLengthDelimited(attributeCursor, attributeEnd, variantData, variantSize))
                            return false;
                    }
                    else if (!skipProtobufField(attributeWire, attributeCursor, attributeEnd))
                    {
                        return false;
                    }
                }

                if (name != wantedName || variantData == nullptr)
                    continue;

                const uint8_t* variantCursor = variantData;
                const uint8_t* variantEnd = variantData + variantSize;
                while (variantCursor < variantEnd)
                {
                    uint64_t variantKey = 0;
                    if (!readVarInt(variantCursor, variantEnd, variantKey))
                        return false;

                    const uint32_t variantField = static_cast<uint32_t>(variantKey >> 3);
                    const uint32_t variantWire = static_cast<uint32_t>(variantKey & 0x07u);
                    // bgs.protocol.Variant blob_value = field 5.
                    if (variantField == 5u && variantWire == 2u)
                    {
                        const uint8_t* data = nullptr;
                        size_t size = 0;
                        if (!readLengthDelimited(variantCursor, variantEnd, data, size))
                            return false;
                        value.assign(data, data + size);
                        return true;
                    }

                    if (!skipProtobufField(variantWire, variantCursor, variantEnd))
                        return false;
                }
            }

            return false;
        }

        bool extractRealmListIdentityGameAccountId(const uint8_t* payload, size_t payloadSize, uint32_t& gameAccountId)
        {
            gameAccountId = 0;
            std::vector<uint8_t> identity;
            if (!readGameUtilitiesBlobAttribute(payload, payloadSize, "Param_Identity", identity) || identity.empty())
                return false;

            const std::string_view text(reinterpret_cast<const char*>(identity.data()), identity.size());

            // reference implementation serializes JSON::RealmList::RealmListTicketIdentity with the
            // proto field name `gameAccountID`. Do not depend on the exact casing
            // here; Blizzard clients/builds have used ID/Id variants over time.
            std::string normalizedIdentity(text);
            std::transform(normalizedIdentity.begin(), normalizedIdentity.end(), normalizedIdentity.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

            size_t cursor = normalizedIdentity.find("gameaccountid");
            if (cursor == std::string::npos)
                return false;

            cursor = normalizedIdentity.find(':', cursor);
            if (cursor == std::string_view::npos)
                return false;
            ++cursor;

            while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor])) != 0)
                ++cursor;

            uint64_t value = 0;
            bool haveDigit = false;
            while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
            {
                haveDigit = true;
                value = value * 10u + static_cast<uint64_t>(text[cursor] - '0');
                if (value > std::numeric_limits<uint32_t>::max())
                    return false;
                ++cursor;
            }

            if (!haveDigit || value == 0)
                return false;

            gameAccountId = static_cast<uint32_t>(value);
            return true;
        }

        bool readGameUtilitiesUintAttribute(const uint8_t* payload, size_t payloadSize, const std::string& wantedName, uint64_t& value)
        {
            if (payload == nullptr || payloadSize == 0)
                return false;

            const uint8_t* cursor = payload;
            const uint8_t* end = payload + payloadSize;

            while (cursor < end)
            {
                uint64_t key = 0;
                if (!readVarInt(cursor, end, key))
                    return false;

                const uint32_t fieldNumber = static_cast<uint32_t>(key >> 3);
                const uint32_t wireType = static_cast<uint32_t>(key & 0x07u);
                if (fieldNumber != 1u || wireType != 2u)
                {
                    if (!skipProtobufField(wireType, cursor, end))
                        return false;
                    continue;
                }

                const uint8_t* attributeData = nullptr;
                size_t attributeSize = 0;
                if (!readLengthDelimited(cursor, end, attributeData, attributeSize))
                    return false;

                std::string name;
                const uint8_t* variantData = nullptr;
                size_t variantSize = 0;
                const uint8_t* attributeCursor = attributeData;
                const uint8_t* attributeEnd = attributeData + attributeSize;

                while (attributeCursor < attributeEnd)
                {
                    uint64_t attributeKey = 0;
                    if (!readVarInt(attributeCursor, attributeEnd, attributeKey))
                        return false;

                    const uint32_t attributeField = static_cast<uint32_t>(attributeKey >> 3);
                    const uint32_t attributeWire = static_cast<uint32_t>(attributeKey & 0x07u);
                    if (attributeField == 1u && attributeWire == 2u)
                    {
                        const uint8_t* data = nullptr;
                        size_t size = 0;
                        if (!readLengthDelimited(attributeCursor, attributeEnd, data, size))
                            return false;
                        name.assign(reinterpret_cast<const char*>(data), size);
                    }
                    else if (attributeField == 2u && attributeWire == 2u)
                    {
                        if (!readLengthDelimited(attributeCursor, attributeEnd, variantData, variantSize))
                            return false;
                    }
                    else if (!skipProtobufField(attributeWire, attributeCursor, attributeEnd))
                    {
                        return false;
                    }
                }

                if (name != wantedName || variantData == nullptr)
                    continue;

                const uint8_t* variantCursor = variantData;
                const uint8_t* variantEnd = variantData + variantSize;
                while (variantCursor < variantEnd)
                {
                    uint64_t variantKey = 0;
                    if (!readVarInt(variantCursor, variantEnd, variantKey))
                        return false;

                    const uint32_t variantField = static_cast<uint32_t>(variantKey >> 3);
                    const uint32_t variantWire = static_cast<uint32_t>(variantKey & 0x07u);
                    // bgs.protocol.Variant uint_value = field 6.
                    if (variantField == 6u && variantWire == 0u)
                        return readVarInt(variantCursor, variantEnd, value);

                    if (!skipProtobufField(variantWire, variantCursor, variantEnd))
                        return false;
                }
            }

            return false;
        }

        std::vector<uint8_t> makeRealmJoinResponse(uint32_t clientBuild, const std::string& gameAccountName, uint32_t realmAddress, const std::string& worldHost, uint32_t worldPort, std::string& realmJoinTicket, std::array<uint8_t, 32>& joinSecret, uint32_t& localRealmId)
        {
            if (gameAccountName.empty() || worldHost.empty() || worldPort == 0u || worldPort > 65535u)
                return {};

            const uint8_t region = static_cast<uint8_t>((realmAddress >> 24) & 0xFFu);
            const uint8_t site = static_cast<uint8_t>((realmAddress >> 16) & 0xFFu);
            const uint16_t realmId = static_cast<uint16_t>(realmAddress & 0xFFFFu);

            // Forever/Camelot does not use the legacy Battle.net region byte here.
            // The beta capture selects Normal with realm address 0x46010002:
            //   region = 70, site = 1, realm = 2.
            const bool validRegion =
                region == Protocol::WoW::EuropeRegion ||
                (usesForever1601Transport(clientBuild) && region == 70u);

            if (!validRegion || site != 1u || realmId == 0u)
                return {};

            if (!sBNetLogonSQL)
                return {};

            localRealmId = 0u;

            // The low 16 bits of the Camelot address are part of Blizzard's
            // external realm address (Normal currently arrives as 0x46010002).
            // They are not AscEmu's local realms.id.  Route Forever joins to the
            // first enabled local realm instead of requiring realms.id == 2.
            std::unique_ptr<QueryResult> realmResult;
            if (usesForever1601Transport(clientBuild) && region == 70u)
            {
                realmResult = sBNetLogonSQL->query("SELECT id, status FROM realms WHERE status <> 0 ORDER BY id LIMIT 1");
            }
            else
            {
                realmResult = sBNetLogonSQL->query("SELECT id, status FROM realms WHERE id = %u LIMIT 1", static_cast<uint32_t>(realmId));
            }

            if (!realmResult)
                return {};

            Field* realmFields = realmResult->fetch();
            if (realmFields == nullptr || realmFields[1].asUint8() == 0u)
                return {};

            localRealmId = realmFields[0].asUint32();
            if (localRealmId == 0u)
                return {};

            std::ostringstream serverAddressesJson;
            serverAddressesJson
                << "JSONRealmListServerIPAddresses:{\"families\":[{"
                << "\"family\":1,\"addresses\":[{"
                << "\"ip\":\"" << jsonEscape(worldHost) << "\","
                << "\"port\":" << worldPort
                << "}]}]}";

            const std::vector<uint8_t> serverAddresses =
                compressGameUtilitiesJson(serverAddressesJson.str());
            if (serverAddresses.empty())
                return {};

            // RealmJoinTicket is opaque to the game client and is passed to the
            // world connection. Keep the same JSON contract used by modern WoW
            // realm joins so the world-side authentication can consume it later.
            std::ostringstream joinTicketJson;
            joinTicketJson
                << "{\"gameAccount\":\"" << jsonEscape(gameAccountName) << "\","
                << "\"platform\":\"Win\","
                << "\"clientArch\":\"x64\","
                << "\"type\":\"wow_classic\"}";
            realmJoinTicket = joinTicketJson.str();

            if (RAND_bytes(joinSecret.data(), static_cast<int>(joinSecret.size())) != 1)
                return {};

            std::vector<uint8_t> joinTicketBlob(realmJoinTicket.begin(), realmJoinTicket.end());
            std::vector<uint8_t> serverSecretBlob(joinSecret.begin(), joinSecret.end());

            std::vector<uint8_t> response;
            appendGameUtilitiesBlobAttribute(response, "Param_RealmJoinTicket", joinTicketBlob);
            appendGameUtilitiesBlobAttribute(response, "Param_ServerAddresses", serverAddresses);

            // RealmJoin returns the server half of the 64-byte world-auth key as
            // Param_JoinSecret. Param_BnetSessionKey belongs to the request side
            // and contains the Battle.net/client session material; returning the
            // generated server secret under that name makes the client discard
            // the join result before opening the world TCP connection.
            appendGameUtilitiesBlobAttribute(response, "Param_JoinSecret", serverSecretBlob);
            return response;
        }

        std::vector<uint8_t> makeRealmListResponse(uint32_t clientBuild, uint32_t accountId)
        {
            struct RealmRow
            {
                uint32_t id = 0;
                uint8_t status = 0;
            };

            std::vector<RealmRow> realms;
            if (sBNetLogonSQL)
            {
                if (auto result = sBNetLogonSQL->query("SELECT id, status FROM realms ORDER BY id"))
                {
                    do
                    {
                        Field* fields = result->fetch();
                        if (!fields)
                            continue;

                        realms.push_back({ fields[0].asUint32(), fields[1].asUint8() });
                    } while (result->nextRow());
                }
            }

            const ForeverSuperDistrictProfile foreverProfile = getForeverSuperDistrictProfile(clientBuild);

            std::ostringstream realmJson;
            realmJson << "JSONRealmListUpdates:{\"updates\":[";

            bool first = true;
            for (const RealmRow& realm : realms)
            {
                if (!first)
                    realmJson << ',';
                first = false;

                const uint32_t address = makeRealmAddress(static_cast<uint8_t>(Protocol::WoW::EuropeRegion), 1u, static_cast<uint16_t>(realm.id));
                const bool online = realm.status != 0;
                const uint32_t flags = online ? 0u : 2u;
                const std::string name = realms.size() == 1u
                    ? "AscEmu"
                    : "AscEmu " + std::to_string(realm.id);

                const ClientVersionParts version = getClientVersionParts(clientBuild);

                realmJson
                    << "{\"update\":{" 
                    << "\"wowRealmAddress\":" << address << ','
                    << "\"cfgTimezonesID\":" << getRealmCfgTimezonesId(clientBuild) << ','
                    << "\"populationState\":" << (online ? 1u : 0u) << ','
                    << "\"cfgCategoriesID\":1,"
                    << "\"version\":{" 
                    << "\"versionMajor\":" << version.major << ','
                    << "\"versionMinor\":" << version.minor << ','
                    << "\"versionRevision\":" << version.revision << ','
                    << "\"versionBuild\":" << clientBuild
                    << "},"
                    << "\"cfgRealmsID\":" << realm.id << ','
                    << "\"flags\":" << flags << ','
                    << "\"name\":\"" << jsonEscape(name) << "\","
                    << "\"cfgConfigsID\":1,"
                    << "\"cfgLanguagesID\":1,"
                    << "\"cfgContentSetID\":" << (foreverProfile.build != 0u ? foreverProfile.currentCfgContentSetId : 0u) << ','
                    << "\"useBleepChance\":0.0"
                    << "},\"deleting\":false}";
            }
            realmJson << "]}";

            std::ostringstream countJson;
            countJson << "JSONRealmCharacterCountList:{\"counts\":[";

            bool firstCount = true;
            if (accountId != 0u && sBNetCharacterSQL)
            {
                for (const RealmRow& realm : realms)
                {
                    // AscEmu currently uses one configured character database for this
                    // Battle.net process.  Emit a count only for realms represented by
                    // that database.  In the normal single-realm setup this maps realm 1
                    // to the same ascemu_char database used by world.
                    uint32_t characterCount = 0u;
                    if (auto countResult = sBNetCharacterSQL->query("SELECT COUNT(*) FROM characters WHERE acct = %u", accountId))
                    {
                        if (Field* countFields = countResult->fetch())
                            characterCount = countFields[0].asUint32();
                    }

                    const uint32_t address = makeRealmAddress(static_cast<uint8_t>(Protocol::WoW::EuropeRegion), 1u, static_cast<uint16_t>(realm.id));

                    if (!firstCount)
                        countJson << ',';
                    firstCount = false;

                    countJson
                        << "{\"wowRealmAddress\":" << address << ','
                        << "\"count\":" << characterCount << '}';

                    sLogger.debug("BNet: RealmList character count account={} realm={} address=0x{:08X} count={}", accountId, realm.id, address, characterCount);
                }
            }

            countJson << "]}";

            sLogger.debug("BNet: RealmList JSON account={} realms={} realm_json='{}' character_count_json='{}'", accountId, realms.size(), realmJson.str(), countJson.str());

            const std::vector<uint8_t> realmList = compressGameUtilitiesJson(realmJson.str());
            const std::vector<uint8_t> characterCounts = compressGameUtilitiesJson(countJson.str());
            if (realmList.empty() || characterCounts.empty())
                return {};

            std::vector<uint8_t> response;
            appendGameUtilitiesBlobAttribute(response, "Param_RealmList", realmList);
            appendGameUtilitiesBlobAttribute(response, "Param_CharacterCountList", characterCounts);
            return response;
        }

        std::vector<uint8_t> makeGameUtilitiesBlobAttribute(const std::string& name, const std::string& blob)
        {
            // bgs.protocol.Variant
            // field 5 = blob_value
            std::vector<uint8_t> variant;
            appendStringField(variant, 5, blob);

            // bgs.protocol.Attribute
            // field 1 = name
            // field 2 = value
            std::vector<uint8_t> attribute;
            appendStringField(attribute, 1, name);
            appendMessageField(attribute, 2, variant);

            // bgs.protocol.game_utilities.v1.ClientResponse
            // repeated Attribute attribute = field 1
            std::vector<uint8_t> response;
            appendMessageField(response, 1, attribute);
            return response;
        }

        std::string findCommandName(const uint8_t* payload, size_t payloadSize)
        {
            if (payload == nullptr || payloadSize == 0)
                return {};

            static constexpr char prefix[] = "Command_";
            constexpr size_t prefixLength = sizeof(prefix) - 1;

            for (size_t i = 0; i + prefixLength <= payloadSize; ++i)
            {
                if (std::memcmp(payload + i, prefix, prefixLength) != 0)
                    continue;

                size_t end = i;
                while (end < payloadSize)
                {
                    const unsigned char character = payload[end];
                    if (!(std::isalnum(character) || character == '_' || character == '-'))
                    {
                        break;
                    }

                    ++end;
                }

                if (end > i)
                {
                    return std::string(reinterpret_cast<const char*>(payload + i), end - i);
                }
            }

            return {};
        }

        bool parseSingleStringFieldOne(const uint8_t* payload, size_t payloadSize, std::string& value)
        {
            value.clear();

            if (payload == nullptr)
                return false;

            const uint8_t* cursor = payload;
            const uint8_t* const end = payload + payloadSize;

            while (cursor < end)
            {
                uint64_t tag = 0;
                if (!readVarInt(cursor, end, tag))
                    return false;

                const uint32_t fieldNumber = static_cast<uint32_t>(tag >> 3);
                const uint32_t wireType = static_cast<uint32_t>(tag & 0x07u);

                if (fieldNumber == 1 && wireType == 2)
                {
                    const uint8_t* data = nullptr;
                    size_t size = 0;
                    if (!readLengthDelimited(cursor, end, data, size))
                        return false;

                    value.assign(reinterpret_cast<const char*>(data), size);
                    return true;
                }

                if (!skipProtobufField(wireType, cursor, end))
                    return false;
            }

            return false;
        }

        struct AuthenticationV2LogonRequestDiagnostic
        {
            bool valid = true;
            uint32_t titleId = 0;
            std::string platform;
            std::string locale;
            uint32_t applicationVersion = 0;
            std::string deviceId;
            std::string authToken;
        };

        AuthenticationV2LogonRequestDiagnostic parseAuthenticationV2LogonRequest(const uint8_t* data, size_t size)
        {
            AuthenticationV2LogonRequestDiagnostic result;
            const uint8_t* cursor = data;
            const uint8_t* const end = data + size;

            while (cursor < end)
            {
                uint64_t tag = 0;
                if (!readVarInt(cursor, end, tag))
                {
                    result.valid = false;
                    break;
                }

                const uint32_t fieldNumber = static_cast<uint32_t>(tag >> 3);
                const uint32_t wireType = static_cast<uint32_t>(tag & 0x07u);

                if (fieldNumber == 1 && wireType == 0)
                {
                    uint64_t value = 0;
                    if (!readVarInt(cursor, end, value) || value > UINT32_MAX)
                    {
                        result.valid = false;
                        break;
                    }
                    result.titleId = static_cast<uint32_t>(value);
                    continue;
                }

                if ((fieldNumber == 2 || fieldNumber == 3) && wireType == 2)
                {
                    const uint8_t* valueData = nullptr;
                    size_t valueSize = 0;
                    if (!readLengthDelimited(cursor, end, valueData, valueSize))
                    {
                        result.valid = false;
                        break;
                    }

                    std::string value(reinterpret_cast<const char*>(valueData), valueSize);
                    if (fieldNumber == 2)
                        result.platform = std::move(value);
                    else
                        result.locale = std::move(value);
                    continue;
                }

                if (fieldNumber == 4 && wireType == 0)
                {
                    uint64_t value = 0;
                    if (!readVarInt(cursor, end, value) || value > UINT32_MAX)
                    {
                        result.valid = false;
                        break;
                    }
                    result.applicationVersion = static_cast<uint32_t>(value);
                    continue;
                }

                // WoW 5.5.4 uses field 10 for LogonOptions.
                if (fieldNumber == 10 && wireType == 2)
                {
                    const uint8_t* optionsData = nullptr;
                    size_t optionsSize = 0;
                    if (!readLengthDelimited(cursor, end, optionsData, optionsSize))
                    {
                        result.valid = false;
                        break;
                    }

                    const uint8_t* optionsCursor = optionsData;
                    const uint8_t* const optionsEnd = optionsData + optionsSize;

                    while (optionsCursor < optionsEnd)
                    {
                        uint64_t optionsTag = 0;
                        if (!readVarInt(optionsCursor, optionsEnd, optionsTag))
                        {
                            result.valid = false;
                            break;
                        }

                        const uint32_t optionsField = static_cast<uint32_t>(optionsTag >> 3);
                        const uint32_t optionsWire = static_cast<uint32_t>(optionsTag & 0x07u);

                        if ((optionsField == 3 || optionsField == 4) && optionsWire == 2)
                        {
                            const uint8_t* valueData = nullptr;
                            size_t valueSize = 0;
                            if (!readLengthDelimited(optionsCursor, optionsEnd, valueData, valueSize))
                            {
                                result.valid = false;
                                break;
                            }

                            std::string value(reinterpret_cast<const char*>(valueData), valueSize);

                            if (optionsField == 3)
                                result.deviceId = std::move(value);
                            else
                                result.authToken = std::move(value);

                            continue;
                        }

                        if (!skipProtobufField(optionsWire, optionsCursor, optionsEnd))
                        {
                            result.valid = false;
                            break;
                        }
                    }

                    if (!result.valid)
                        break;

                    continue;
                }

                if (!skipProtobufField(wireType, cursor, end))
                {
                    result.valid = false;
                    break;
                }
            }

            return result;
        }

        bool parseConnectUseBindlessRpc(const uint8_t* data, size_t size)
        {
            bool useBindlessRpc = true;
            const uint8_t* cursor = data;
            const uint8_t* const end = data + size;

            while (cursor < end)
            {
                uint64_t tag = 0;
                if (!readVarInt(cursor, end, tag))
                    break;

                const uint32_t fieldNumber = static_cast<uint32_t>(tag >> 3);
                const uint32_t wireType = static_cast<uint32_t>(tag & 0x07u);
                if (fieldNumber == 3 && wireType == 0)
                {
                    uint64_t value = 0;
                    if (readVarInt(cursor, end, value))
                        useBindlessRpc = value != 0;
                    break;
                }

                if (!skipProtobufField(wireType, cursor, end))
                    break;
            }

            return useBindlessRpc;
        }

        RpcHeaderDiagnostic parseRpcHeader(const uint8_t* data, size_t size)
        {
            RpcHeaderDiagnostic result;
            const uint8_t* cursor = data;
            const uint8_t* const end = data + size;

            while (cursor < end)
            {
                uint64_t tag = 0;
                if (!readVarInt(cursor, end, tag))
                {
                    result.valid = false;
                    break;
                }

                const uint32_t fieldNumber = static_cast<uint32_t>(tag >> 3);
                const uint32_t wireType = static_cast<uint32_t>(tag & 0x07u);

                auto readUInt32 = [&](uint32_t& target, bool& present) -> bool
                {
                    if (wireType != 0)
                        return false;
                    uint64_t value = 0;
                    if (!readVarInt(cursor, end, value) || value > UINT32_MAX)
                        return false;
                    target = static_cast<uint32_t>(value);
                    present = true;
                    return true;
                };

                bool handled = true;
                switch (fieldNumber)
                {
                    case 1:
                        handled = readUInt32(result.serviceId, result.hasServiceId);
                        break;
                    case 2:
                        handled = readUInt32(result.methodId, result.hasMethodId);
                        break;
                    case 3:
                        handled = readUInt32(result.token, result.hasToken);
                        break;
                    case 5:
                        handled = readUInt32(result.size, result.hasSize);
                        break;
                    case 6:
                        handled = readUInt32(result.status, result.hasStatus);
                        break;
                    case 9:
                    {
                        uint32_t value = 0;
                        handled = readUInt32(value, result.hasIsResponse);
                        result.isResponse = value != 0;
                        break;
                    }
                    case 11:
                        if (wireType != 5 || static_cast<size_t>(end - cursor) < 4)
                        {
                            handled = false;
                            break;
                        }
                        result.serviceHash = static_cast<uint32_t>(cursor[0]) |
                            (static_cast<uint32_t>(cursor[1]) << 8) |
                            (static_cast<uint32_t>(cursor[2]) << 16) |
                            (static_cast<uint32_t>(cursor[3]) << 24);
                        result.hasServiceHash = true;
                        cursor += 4;
                        break;
                    default:
                        handled = skipProtobufField(wireType, cursor, end);
                        break;
                }

                if (!handled)
                {
                    result.valid = false;
                    break;
                }
            }

            return result;
        }

        std::string makeHexDump(const uint8_t* data, size_t size)
        {
            std::ostringstream output;
            output << std::hex << std::setfill('0');

            for (size_t offset = 0; offset < size; offset += HEX_DUMP_BYTES_PER_LINE)
            {
                const size_t lineSize = std::min(HEX_DUMP_BYTES_PER_LINE, size - offset);

                output << std::setw(4) << offset << ": ";

                for (size_t index = 0; index < HEX_DUMP_BYTES_PER_LINE; ++index)
                {
                    if (index < lineSize)
                        output << std::setw(2) << static_cast<unsigned int>(data[offset + index]) << ' ';
                    else
                        output << "   ";
                }

                output << " |";
                for (size_t index = 0; index < lineSize; ++index)
                {
                    const unsigned char value = data[offset + index];
                    output << (std::isprint(value) != 0 ? static_cast<char>(value) : '.');
                }
                output << '|';

                if (offset + lineSize < size)
                    output << '\n';
            }

            return output.str();
        }
    }

    BNetSocket::BNetSocket(SOCKET fd)
        : Socket(fd, BNET_SOCKET_BUFFER_SIZE, BNET_SOCKET_BUFFER_SIZE)
    {
    }

    BNetSocket::~BNetSocket()
    {
        releaseTls();
    }

    void BNetSocket::onConnect()
    {
        m_connectionId = nextConnectionId.fetch_add(1, std::memory_order_relaxed);
        m_receivedBytes = 0;
        m_sentBytes = 0;
        m_connectedAt = std::chrono::steady_clock::now();
        m_plainTextBuffer.clear();

        sLogger.info("BNet: connection #{} from {}:{}", m_connectionId, getRemoteIp(), getRemotePort());

        if (!initializeTls())
        {
            sLogger.failure("BNet: connection #{} could not initialize TLS", m_connectionId);
            disconnect();
            return;
        }

        sLogger.info("BNet: connection #{} TLS handshake started", m_connectionId);
    }

    bool BNetSocket::initializeTls()
    {
        SSL_CTX* context = BNetTlsContext::getInstance().getContext();
        if (context == nullptr)
            return false;

        m_ssl = SSL_new(context);
        if (m_ssl == nullptr)
            return false;

        BIO* readBio = BIO_new(BIO_s_mem());
        BIO* writeBio = BIO_new(BIO_s_mem());
        if (readBio == nullptr || writeBio == nullptr)
        {
            if (readBio != nullptr)
                BIO_free(readBio);
            if (writeBio != nullptr)
                BIO_free(writeBio);
            releaseTls();
            return false;
        }

        // SSL owns both BIOs after this call.
        SSL_set_bio(m_ssl, readBio, writeBio);
        SSL_set_accept_state(m_ssl);
        return true;
    }

    void BNetSocket::onRead()
    {
        const size_t available = readBuffer.GetSize();
        if (available == 0 || m_ssl == nullptr)
            return;

        std::vector<uint8_t> encryptedData(available);
        if (!readBuffer.Read(encryptedData.data(), encryptedData.size()))
        {
            sLogger.failure("BNet: connection #{} failed to read {} byte(s) from socket buffer", m_connectionId, available);
            disconnect();
            return;
        }

        m_receivedBytes += encryptedData.size();

        BIO* readBio = SSL_get_rbio(m_ssl);
        const int written = BIO_write(readBio, encryptedData.data(), static_cast<int>(encryptedData.size()));
        if (written <= 0 || static_cast<size_t>(written) != encryptedData.size())
        {
            sLogger.failure("BNet: connection #{} failed to pass {} TLS byte(s) to OpenSSL", m_connectionId, encryptedData.size());
            disconnect();
            return;
        }

        if (!processTls())
        {
            disconnect();
            return;
        }

        // ConnectionService::RequestDisconnect is allowed to finish the entire
        // current TLS/read stack before the underlying socket is armed for a
        // delayed close. delayedDisconnect() may complete synchronously when
        // the socket write queue is already empty, and onDisconnect() frees
        // m_ssl. Calling it from processRpcFrames()/processTls() would therefore
        // invalidate the TLS object while those functions are still using it.
        if (m_delayCloseAfterReadCallback)
        {
            m_delayCloseAfterReadCallback = false;
            sLogger.debug("BNet: connection #{} current TLS read completed; arming delayed socket close.", m_connectionId);
            delayedDisconnect();
        }
    }

    bool BNetSocket::processTls()
    {
        if (!m_tlsHandshakeComplete)
        {
            const int result = SSL_do_handshake(m_ssl);
            if (!flushTlsOutput())
                return false;

            if (result == 1)
            {
                m_tlsHandshakeComplete = true;
                const char* serverName = SSL_get_servername(m_ssl, TLSEXT_NAMETYPE_host_name);
                const unsigned char* alpn = nullptr;
                unsigned int alpnLength = 0;
                SSL_get0_alpn_selected(m_ssl, &alpn, &alpnLength);

                const std::string sni = serverName != nullptr ? serverName : "<none>";
                const std::string alpnProtocol =
                    (alpn != nullptr && alpnLength > 0)
                        ? std::string(reinterpret_cast<const char*>(alpn), alpnLength)
                        : "<none>";

                sLogger.info("BNet: connection #{} TLS handshake complete (version: {}, cipher: {}, SNI: '{}', ALPN: '{}')", m_connectionId, SSL_get_version(m_ssl), SSL_get_cipher_name(m_ssl), sni, alpnProtocol);
            }
            else
            {
                const int error = SSL_get_error(m_ssl, result);
                if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
                    return true;

                logTlsError("handshake", result);
                return false;
            }
        }

        std::array<uint8_t, TLS_IO_BUFFER_SIZE> plainText{};

        while (m_tlsHandshakeComplete)
        {
            const int result = SSL_read(m_ssl, plainText.data(), static_cast<int>(plainText.size()));
            if (result > 0)
            {
                if (!processPlainText(plainText.data(), static_cast<size_t>(result)))
                    return false;

                // A RequestDisconnect frame has been fully retired from the
                // plaintext parser. Do not perform another SSL_read() from this
                // callback; flush any TLS output below, return to onRead(), and
                // only then arm the delayed socket close.
                if (m_delayCloseAfterReadCallback)
                    break;

                continue;
            }

            const int error = SSL_get_error(m_ssl, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
                break;

            if (error == SSL_ERROR_ZERO_RETURN)
            {
                sLogger.info("BNet: connection #{} received TLS close_notify", m_connectionId);
                return false;
            }

            logTlsError("read", result);
            return false;
        }

        return flushTlsOutput();
    }

    bool BNetSocket::processPlainText(const uint8_t* data, size_t size)
    {
        sLogger.debug("BNet: connection #{} decrypted RX {} byte(s)\n{}", m_connectionId, size, makeHexDump(data, std::min(size, MAX_DIAGNOSTIC_DUMP_SIZE)));

        if (size > MAX_DIAGNOSTIC_DUMP_SIZE)
        {
            sLogger.debug("BNet: connection #{} decrypted RX dump truncated ({} of {} byte(s) shown)", m_connectionId, MAX_DIAGNOSTIC_DUMP_SIZE, size);
        }

        m_plainTextBuffer.insert(m_plainTextBuffer.end(), data, data + size);
        return processRpcFrames();
    }

    bool BNetSocket::processRpcFrames()
    {
        while (m_plainTextBuffer.size() >= 2)
        {
            // Battle.net RPC prefixes the protobuf Header with a two-byte, big-endian header length.
            const size_t headerSize =
                (static_cast<size_t>(m_plainTextBuffer[0]) << 8) |
                static_cast<size_t>(m_plainTextBuffer[1]);

            if (headerSize == 0 || headerSize > MAX_RPC_HEADER_SIZE)
            {
                sLogger.failure("BNet: connection #{} invalid RPC header length {} (buffered: {} byte(s))", m_connectionId, headerSize, m_plainTextBuffer.size());
                return false;
            }

            if (m_plainTextBuffer.size() < 2 + headerSize)
                return true;

            const uint8_t* const headerData = m_plainTextBuffer.data() + 2;
            const RpcHeaderDiagnostic header = parseRpcHeader(headerData, headerSize);
            if (!header.valid)
            {
                sLogger.failure("BNet: connection #{} could not decode {} byte RPC protobuf header\n{}", m_connectionId, headerSize, makeHexDump(headerData, headerSize));
                return false;
            }

            const size_t payloadSize = header.hasSize ? static_cast<size_t>(header.size) : 0;
            if (payloadSize > MAX_RPC_PAYLOAD_SIZE)
            {
                sLogger.failure("BNet: connection #{} RPC payload too large: {} byte(s)", m_connectionId, payloadSize);
                return false;
            }

            const size_t frameSize = 2 + headerSize + payloadSize;
            if (m_plainTextBuffer.size() < frameSize)
                return true;

            if (header.hasServiceHash && header.hasMethodId)
            {
                sLogger.debug("BNet RX {}::{} size={} token={}", Protocol::getServiceName(header.serviceHash), Protocol::getMethodName(header.serviceHash, header.methodId), payloadSize, header.hasToken ? header.token : 0u);
            }
            else
            {
                sLogger.debug("BNet: connection #{} incomplete RPC header: service_hash={}, method_id={}, token={}, payload={} byte(s)", m_connectionId, header.hasServiceHash ? "present" : "missing", header.hasMethodId ? "present" : "missing", header.hasToken ? "present" : "missing", payloadSize);
            }

            if (payloadSize > 0)
            {
                const uint8_t* const payload = m_plainTextBuffer.data() + 2 + headerSize;
                const size_t dumpSize = std::min(payloadSize, MAX_DIAGNOSTIC_DUMP_SIZE);
                sLogger.debug("BNet RX payload service=0x{:08X} method={} size={}\n{}", header.hasServiceHash ? header.serviceHash : 0u, header.hasMethodId ? header.methodId : 0u, payloadSize, makeHexDump(payload, dumpSize));
            }

            const uint8_t* const payload = payloadSize > 0
                ? m_plainTextBuffer.data() + 2 + headerSize
                : nullptr;

            if (!header.hasServiceHash || !header.hasMethodId || !header.hasToken)
            {
                sLogger.info("BNet: connection #{} RPC frame cannot be dispatched: service_hash={}, method_id={}, token={}", m_connectionId, header.hasServiceHash ? "present" : "missing", header.hasMethodId ? "present" : "missing", header.hasToken ? "present" : "missing");
            }
            else
            {
                m_currentRpcServiceHash = header.serviceHash;
                m_currentRpcMethodId = header.methodId;
                const bool dispatched = dispatchRpcRequest(header.serviceHash, header.methodId, header.token, payload, payloadSize);
                m_currentRpcServiceHash = 0;
                m_currentRpcMethodId = 0;
                if (!dispatched)
                    return false;
            }

            m_plainTextBuffer.erase(m_plainTextBuffer.begin(), m_plainTextBuffer.begin() + static_cast<std::ptrdiff_t>(frameSize));

            // A RequestDisconnect handler marks the connection for a delayed
            // close. The current frame has now been safely removed, so stop
            // parsing further frames from this read. The actual delayedDisconnect()
            // is intentionally deferred until onRead(), after processTls() has
            // returned and no code on this stack can touch m_ssl anymore.
            if (m_delayCloseAfterReadCallback)
                return true;
        }

        return true;
    }

    bool BNetSocket::dispatchRpcRequest(uint32_t serviceHash, uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        if (serviceHash == Protocol::ConnectionService::Hash)
        {
            if (methodId == Protocol::ConnectionService::Connect)
                return handleConnectionConnect(token, payload, payloadSize);
            if (methodId == Protocol::ConnectionService::KeepAlive)
                return handleConnectionKeepAlive(token, payload, payloadSize);
            if (methodId == Protocol::ConnectionService::RequestDisconnect)
                return handleConnectionRequestDisconnect(token, payload, payloadSize);
        }
        else if (serviceHash == Protocol::AuthenticationServiceV2::Hash)
        {
            if (methodId == Protocol::AuthenticationServiceV2::Logon)
                return handleAuthenticationV2Logon(token, payload, payloadSize);
            if (methodId == Protocol::AuthenticationServiceV2::VerifyWebCredentials)
                return handleAuthenticationV2VerifyWebCredentials(token, payload, payloadSize);
        }
        else if (serviceHash == Protocol::AccountServiceV2::Hash)
        {
            if (methodId == Protocol::AccountServiceV2::GetAccountInfo || methodId == Protocol::AccountServiceV2::GetRestriction || methodId == Protocol::AccountServiceV2::GetGameAccountLinks || methodId == Protocol::AccountServiceV2::GetGameAccountInfo || methodId == Protocol::AccountServiceV2::GetGameAccountRestriction)
            {
                return handleAccountServiceV2Request(methodId, token, payload, payloadSize);
            }
        }
        else if (serviceHash == Protocol::GameUtilitiesService::Hash)
        {
            if (methodId == Protocol::GameUtilitiesService::ProcessClientRequest || methodId == Protocol::GameUtilitiesService::GetAllValuesForAttribute)
            {
                return handleGameUtilitiesRequest(methodId, token, payload, payloadSize);
            }
        }

        logUnhandledRpc(serviceHash, methodId, token, payload, payloadSize);
        return true;
    }

    void BNetSocket::logUnhandledRpc(uint32_t serviceHash, uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize) const
    {
        const std::string_view serviceName = Protocol::getServiceName(serviceHash);
        const std::string_view methodName = Protocol::getMethodName(serviceHash, methodId);

        sLogger.info("BNet UNHANDLED {}::{} size={} token={} (service=0x{:08X}, method={})", serviceName, methodName, payloadSize, token, serviceHash, methodId);

        if (payload != nullptr && payloadSize != 0)
        {
            const size_t dumpSize = std::min(payloadSize, MAX_DIAGNOSTIC_DUMP_SIZE);
            sLogger.debug("BNet: connection #{} unhandled RPC payload\n{}", m_connectionId, makeHexDump(payload, dumpSize));
        }
    }

    bool BNetSocket::handleConnectionConnect(uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        const bool useBindlessRpc = payload != nullptr
            ? parseConnectUseBindlessRpc(payload, payloadSize)
            : true;

        const auto now = std::chrono::system_clock::now();
        const uint64_t serverTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

        // ConnectionService.ConnectResponse identifies the server process separately
        // from the client/session instance. Current BGS clients also accept a CIID
        // derived from these two ProcessIds.
        constexpr uint32_t serverLabel = 1u;
        const uint32_t serverEpoch = getServerEpochSeconds();

        const uint32_t clientLabel = static_cast<uint32_t>(m_connectionId & std::numeric_limits<uint32_t>::max());
        const uint32_t clientEpoch = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());

        const std::string clientInstanceId = buildClientInstanceId(serverLabel, serverEpoch, clientLabel, clientEpoch);

        std::vector<uint8_t> response;
        response.reserve(96);
        appendMessageField(response, 1, buildProcessId(serverLabel, serverEpoch));
        appendMessageField(response, 2, buildProcessId(clientLabel, clientEpoch));
        appendVarIntField(response, 6, serverTime);
        appendVarIntField(response, 7, useBindlessRpc ? 1u : 0u);

        // Current BGS connection.proto: ConnectResponse.ciid.
        // This field was added after the older public protocol snapshots.
        appendStringField(response, 9, clientInstanceId);

        sLogger.debug("BNet: connection #{} ConnectionService.Connect -> status=0, token={}, bindless={}, " "server_id={:08X}:{:08X}, client_id={:08X}:{:08X}, ciid='{}', server_time={}", m_connectionId, token, useBindlessRpc ? "true" : "false", serverLabel, serverEpoch, clientLabel, clientEpoch, clientInstanceId, serverTime);

        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleConnectionKeepAlive(uint32_t token, const uint8_t* /*payload*/, size_t payloadSize)
    {
        if (payloadSize != 0)
        {
            sLogger.debug("BNet: connection #{} ConnectionService.KeepAlive received {} payload byte(s)", m_connectionId, payloadSize);
        }

        sLogger.debug("BNet: connection #{} ConnectionService.KeepAlive -> status=0, token={}", m_connectionId, token);

        const std::vector<uint8_t> noData;
        return sendRpcResponse(token, noData);
    }

    bool BNetSocket::handleConnectionRequestDisconnect(uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        uint64_t errorCode = 0;
        if (payload != nullptr && payloadSize != 0)
        {
            const uint8_t* cursor = payload;
            const uint8_t* const end = payload + payloadSize;
            uint64_t tag = 0;
            uint64_t value = 0;
            if (readVarInt(cursor, end, tag) && (tag >> 3) == 1 && (tag & 7u) == 0 && readVarInt(cursor, end, value))
                errorCode = value;
        }

        sLogger.debug("BNet: connection #{} ConnectionService.RequestDisconnect -> token={}, error_code={}, payload={} byte(s)", m_connectionId, token, errorCode, payloadSize);

        // Match the modern protocol reference's ConnectionService::HandleRequestDisconnect:
        // first issue ForceDisconnect(DisconnectNotification), then return the
        // RequestDisconnect response, and only close after queued writes drain.
        std::vector<uint8_t> notification;
        appendVarIntField(notification, 1, errorCode);

        constexpr uint32_t forceDisconnectServerToken = 2u;
        if (!sendRpcRequest(Protocol::ConnectionService::Hash, Protocol::ConnectionService::ForceDisconnect, forceDisconnectServerToken, notification))
        {
            sLogger.failure("BNet: connection #{} failed to send ConnectionService.ForceDisconnect", m_connectionId);
            return false;
        }

        if (!sendRpcResponse(token, std::vector<uint8_t>{}))
            return false;

        sLogger.debug("BNet: connection #{} RequestDisconnect response queued; delayed close will be armed after the current TLS read callback finishes.", m_connectionId);

        // Do not call delayedDisconnect() from inside the RPC/TLS stack. If the
        // socket write queue is already empty it may close synchronously, and
        // onDisconnect()/releaseTls() would free m_ssl while processTls() still
        // needs it. onRead() consumes this flag only after processTls() returns.
        m_delayCloseAfterReadCallback = true;
        return true;
    }

    bool BNetSocket::handleAuthenticationV2Logon(uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        if (payload == nullptr)
        {
            sLogger.failure("BNet: connection #{} AuthenticationServiceV2.Logon has no payload", m_connectionId);
            return false;
        }

        const AuthenticationV2LogonRequestDiagnostic request =
            parseAuthenticationV2LogonRequest(payload, payloadSize);

        if (!request.valid)
        {
            sLogger.failure("BNet: connection #{} could not decode AuthenticationServiceV2.LogonRequest", m_connectionId);
            return false;
        }

        m_clientBuild = request.applicationVersion;

        std::string title;
        title.push_back(static_cast<char>(request.titleId & 0xFFu));
        title.push_back(static_cast<char>((request.titleId >> 8) & 0xFFu));
        title.push_back(static_cast<char>((request.titleId >> 16) & 0xFFu));

        sLogger.debug("BNet: connection #{} AuthenticationServiceV2.Logon -> " "title='{}' (0x{:08X}), platform='{}', locale='{}', build={}, " "device_id={} byte(s), cached_auth_token={}", m_connectionId, title, request.titleId, request.platform, request.locale, request.applicationVersion, request.deviceId.size(), request.authToken.empty() ? "no" : "yes");

        if (!request.deviceId.empty())
        {
            const size_t maxLength = 1024;
            sLogger.debug("BNet: connection #{} AuthenticationServiceV2 device_id: {}", m_connectionId, request.deviceId.substr(0, maxLength));
        }

        const std::vector<uint8_t> noData;
        if (!sendRpcResponse(token, noData))
            return false;

        return sendExternalChallenge();
    }

    bool BNetSocket::handleAuthenticationV2VerifyWebCredentials(uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        std::string loginTicket;
        if (!parseSingleStringFieldOne(payload, payloadSize, loginTicket))
        {
            sLogger.failure("BNet: connection #{} AuthenticationServiceV2 method 2 " "could not decode login ticket", m_connectionId);
            return false;
        }

        std::string login;
        const bool valid = consumeWebAuthTicket(loginTicket, login);

        sLogger.debug("BNet: connection #{} AuthenticationServiceV2 method 2 -> " "login_ticket='{}', valid={}, login='{}'", m_connectionId, loginTicket, valid ? "true" : "false", valid ? login : std::string());

        if (!valid)
        {
            // Keep this stage diagnostic: do not forge a successful auth state.
            // Close the RPC with a generic non-zero status so the client cannot
            // continue on an unknown/replayed ticket.
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        const std::vector<uint8_t> noData;
        if (!sendRpcResponse(token, noData))
            return false;

        sLogger.debug("BNet: connection #{} AuthenticationServiceV2 method 2 accepted; " "sending AuthenticationListenerV2.OnLogonComplete", m_connectionId);

        return sendLogonComplete(login, loginTicket);
    }

    bool BNetSocket::sendLogonComplete(const std::string& login, const std::string& loginTicket)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet: connection #{} cannot build OnLogonComplete: " "logon database is unavailable", m_connectionId);
            return false;
        }

        const std::string escapedLogin = sBNetLogonSQL->escapeString(login);
        auto result = sBNetLogonSQL->query("SELECT ba.id, ba.email, ba.battle_tag, ba.country, a.id, a.acc_name " "FROM battlenet_accounts ba " "INNER JOIN battlenet_game_accounts bga ON bga.battlenet_account_id = ba.id " "INNER JOIN accounts a ON a.id = bga.game_account_id " "WHERE UPPER(ba.email) = UPPER('%s') " "ORDER BY a.id", escapedLogin.c_str());

        if (!result)
        {
            sLogger.failure("BNet: connection #{} cannot build OnLogonComplete: " "Battle.net account '{}' has no linked WoW game accounts", m_connectionId, login);
            return false;
        }

        m_linkedGameAccounts.clear();
        m_selectedGameAccountId = 0;
        m_selectedGameAccountName.clear();

        uint32_t battleNetAccountId = 0;
        std::string battleTag;
        std::string country = "CH";

        do
        {
            Field* fields = result->fetch();
            if (fields == nullptr)
                continue;

            if (battleNetAccountId == 0)
            {
                battleNetAccountId = fields[0].asUint32();
                battleTag = fields[2].asCString() != nullptr ? fields[2].asCString() : "";
                if (fields[3].asCString() != nullptr && fields[3].asCString()[0] != '\0')
                    country = fields[3].asCString();
            }

            LinkedGameAccount gameAccount;
            gameAccount.id = fields[4].asUint32();
            gameAccount.name = fields[5].asCString() != nullptr ? fields[5].asCString() : "";
            if (gameAccount.id != 0)
                m_linkedGameAccounts.push_back(std::move(gameAccount));
        } while (result->nextRow());

        if (battleNetAccountId == 0 || m_linkedGameAccounts.empty())
            return false;

        m_battleNetAccountId = battleNetAccountId;

        std::array<uint8_t, 64> sessionKey{};
        if (RAND_bytes(sessionKey.data(), static_cast<int>(sessionKey.size())) != 1)
        {
            sLogger.failure("BNet: connection #{} could not generate BNet session key", m_connectionId);
            return false;
        }

        std::ostringstream sessionKeyHex;

        for (size_t i = 0; i < sessionKey.size(); ++i)
        {
            if (i != 0)
                sessionKeyHex << ' ';

            sessionKeyHex << std::format("{:02X}", sessionKey[i]);
        }

        sLogger.info("BNet: connection #{} Param_BnetSessionKey={}", m_connectionId, sessionKeyHex.str());

        // bgs.protocol.authentication.v2.client.LogonRecord
        std::vector<uint8_t> record;
        appendVarIntField(record, 1, battleNetAccountId);

        for (const LinkedGameAccount& linked : m_linkedGameAccounts)
        {
            // bgs.protocol.account.v2.GameAccountHandle
            std::vector<uint8_t> gameAccount;
            appendVarIntField(gameAccount, 1, linked.id);
            appendVarIntField(gameAccount, 2, Protocol::WoW::TitleId);
            appendVarIntField(gameAccount, 3, Protocol::WoW::EuropeRegion);
            appendMessageField(record, 2, gameAccount);
        }

        if (!battleTag.empty())
            appendStringField(record, 3, battleTag);

        if (!country.empty())
            appendStringField(record, 4, country);

        appendVarInt(record, (static_cast<uint64_t>(5) << 3) | 2u);
        appendVarInt(record, sessionKey.size());
        record.insert(record.end(), sessionKey.begin(), sessionKey.end());

        appendVarIntField(record, 6, 0); // employee_only_mode = false
        appendStringField(record, 7, loginTicket);

        std::vector<uint8_t> notification;
        appendVarIntField(notification, 1, 0); // error_code = 0
        appendMessageField(notification, 2, record);

        constexpr uint32_t serverToken = 2u;

        std::ostringstream accountList;
        for (size_t index = 0; index < m_linkedGameAccounts.size(); ++index)
        {
            if (index != 0)
                accountList << ',';
            accountList << m_linkedGameAccounts[index].id << ':' << m_linkedGameAccounts[index].name;
        }

        sLogger.info("BNet: connection #{} AuthenticationListenerV2.OnLogonComplete -> " "token={}, battlenet_account_id={}, game_accounts=[{}], title_id={}, region={}, " "battle_tag='{}', country='{}', session_key={} byte(s)", m_connectionId, serverToken, battleNetAccountId, accountList.str(), Protocol::WoW::TitleId, Protocol::WoW::EuropeRegion, battleTag, country, sessionKey.size());

        return sendRpcRequest(Protocol::AuthenticationListenerV2::Hash, Protocol::AuthenticationListenerV2::OnLogonComplete, serverToken, notification);
    }

    bool BNetSocket::handleGameUtilitiesRequest(uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        const std::string commandName = findCommandName(payload, payloadSize);

        sLogger.debug("BNet: connection #{} GameUtilities method={}, token={}, command='{}', payload={} byte(s)", m_connectionId, methodId, token, commandName.empty() ? std::string("<unnamed>") : commandName, payloadSize);

        if (methodId == Protocol::GameUtilitiesService::GetAllValuesForAttribute)
            return handleGameUtilitiesGetAllValues(token, commandName, payload, payloadSize);

        if (methodId != Protocol::GameUtilitiesService::ProcessClientRequest)
        {
            logUnhandledRpc(Protocol::GameUtilitiesService::Hash, methodId, token, payload, payloadSize);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::RealmJoinPrefix))
            return handleRealmJoinRequest(token, payload, payloadSize);

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::RealmListPrefix))
            return handleRealmListRequest(token, commandName);

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::RealmListTicketPrefix))
            return handleRealmListTicketRequest(token, commandName, payload, payloadSize);

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::FetchBleepProxiesPrefix))
            return handleFetchBleepProxiesRequest(token, commandName);

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::SuperDistrictListPrefix))
            return handleSuperDistrictListRequest(token, commandName);

        if (commandMatches(commandName, Protocol::GameUtilitiesCommands::LastCharPlayedPrefix))
            return handleLastCharPlayedRequest(token, commandName);

        sLogger.debug("BNet: connection #{} GameUtilitiesService.ProcessClientRequest unhandled command='{}', token={}, payload={} byte(s); replying NoData", m_connectionId, commandName.empty() ? std::string("<unnamed>") : commandName, token, payloadSize);
        return sendRpcResponse(token, std::vector<uint8_t>{});
    }

    bool BNetSocket::handleGameUtilitiesGetAllValues(uint32_t token, const std::string& attributeName, const uint8_t* /*payload*/, size_t payloadSize)
    {
        if (commandMatches(attributeName, Protocol::GameUtilitiesCommands::RealmListPrefix))
        {
            // Forever/Camelot 1.60.1.69893 uses "70-1-70" for the realm-list
            // command variant/subregion. Keep the legacy value for other builds.
            const std::string_view subRegion = usesForever1601Transport(m_clientBuild)
                ? std::string_view{ "70-1-70" }
                : std::string_view{ "2-1-0" };

            const std::vector<std::string> subRegions{ std::string(subRegion) };
            const std::vector<uint8_t> response = makeGameUtilitiesStringVariantList(subRegions);

            sLogger.debug("BNet: connection #{} GameUtilitiesService.GetAllValuesForAttribute -> token={}, key='{}', subregion='{}', response={} byte(s)", m_connectionId, token, attributeName, subRegion, response.size());
            return sendRpcResponse(token, response);
        }

        sLogger.debug("BNet: connection #{} GameUtilitiesService.GetAllValuesForAttribute unknown key='{}', token={}, payload={} byte(s); replying NoData", m_connectionId, attributeName.empty() ? std::string("<unnamed>") : attributeName, token, payloadSize);
        return sendRpcResponse(token, std::vector<uint8_t>{});
    }


    bool BNetSocket::handleFetchBleepProxiesRequest(uint32_t token, const std::string& commandName)
    {
        // Camelot/Forever asks for Bleep proxy discovery during the login flow.
        // A private server does not need Blizzard's Bleep relay service, but the
        // client expects a syntactically valid JSON list instead of an empty RPC.
        const std::vector<uint8_t> blob =
            compressGameUtilitiesJson("JSONBleepProxyList:{\"proxies\":[]}");

        std::vector<uint8_t> response;
        if (!blob.empty())
            appendGameUtilitiesBlobAttribute(response, "Param_BleepProxyList", blob);

        sLogger.debug("BNet: connection #{} FetchBleepProxies -> token={}, command='{}', proxies=0, response={} byte(s)", m_connectionId, token, commandName, response.size());

        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleSuperDistrictListRequest(uint32_t token, const std::string& commandName)
    {
        (void)commandName;
        std::ostringstream json;

        if (m_clientBuild == 70009u)
        {
            sLogger.warning("BNet: Forever 70009 SuperDistrict mapping is not verified yet; Roleplay/Hardcore IDs must be captured before serialization.");
            json
            << "JSONSuperDistrictList:{\"superDistricts\":["
                << "{\"superDistrictID\":2,\"disallowLogin\":false},"
                << "{\"superDistrictID\":1,\"disallowLogin\":false},"
                << "{\"superDistrictID\":5,\"disallowLogin\":false}"
                << "]}";
        }
        else if (m_clientBuild == 69893u)
        {
            // Forever/Camelot 1.60.1.69893 sniff-based compatibility test.
            // The play-style selector expects SuperDistrict IDs here; realm
            // address/cfg metadata is carried later by RealmList entries.
            json
                << "JSONSuperDistrictList:{\"superDistricts\":["
                << "{\"superDistrictID\":2,\"disallowLogin\":false},"
                << "{\"superDistrictID\":1,\"disallowLogin\":false},"
                << "{\"superDistrictID\":5,\"disallowLogin\":false}"
                << "]}";
        }
        else
        {
            struct SuperDistrictRealm
            {
                uint32_t id = 0;
                uint8_t status = 0;
            };

            std::vector<SuperDistrictRealm> realms;
            if (sBNetLogonSQL)
            {
                if (auto result = sBNetLogonSQL->query("SELECT id, status FROM realms ORDER BY id"))
                {
                    do
                    {
                        Field* fields = result->fetch();
                        if (!fields)
                            continue;

                        realms.push_back({ fields[0].asUint32(), fields[1].asUint8() });
                    } while (result->nextRow());
                }
            }

            json << "JSONSuperDistrictList:{\"superDistricts\":[";

            bool first = true;
            for (const SuperDistrictRealm& realm : realms)
            {
                if (realm.status == 0u)
                    continue;

                if (!first)
                    json << ',';
                first = false;

                const uint32_t address = makeRealmAddress(static_cast<uint8_t>(Protocol::WoW::EuropeRegion), 1u, static_cast<uint16_t>(realm.id));

                json
                    << "{\"wowRealmAddress\":" << address << ','
                    << "\"useBleepChance\":0.0,"
                    << "\"cfgTimezonesID\":" << getRealmCfgTimezonesId(m_clientBuild) << '}';

            }

            json << "]}";
        }

        const std::vector<uint8_t> blob = compressGameUtilitiesJson(json.str());

        std::vector<uint8_t> response;
        if (!blob.empty())
            appendGameUtilitiesBlobAttribute(response, "Param_SuperDistrictList", blob);

        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleLastCharPlayedRequest(uint32_t token, const std::string& commandName)
    {
        (void)commandName;
        if (usesForever1601Transport(m_clientBuild))
        {
            // Forever 1.60.1 transport. The 70009 PvE LastCharPlayed shape is provisionally inherited from the verified 69893 capture until a 70009 capture confirms the ruleset-specific values:
            //
            // The official beta service returns a non-empty LastCharPlayed
            // response even for an account without characters.  After Normal
            // is selected the client requests ContentSetID 137 and expects a
            // RealmEntry/LastPlayedTime/UtilityInfo context before continuing
            // to RealmListRequest.
            //
            // Values below mirror the observed Classic Beta PvE context.
            const std::string realmEntryJson =
                "JamJSONRealmEntry:{"
                "\"wowRealmAddress\":1174470658,"
                "\"useBleepChance\":0.0,"
                "\"cfgTimezonesID\":1,"
                "\"populationState\":2,"
                "\"cfgCategoriesID\":26,"
                "\"version\":{"
                    "\"versionMajor\":1,"
                    "\"versionBuild\":69800,"
                    "\"versionMinor\":60,"
                    "\"versionRevision\":1"
                "},"
                "\"cfgRealmsID\":4618,"
                "\"gameServiceRegionId\":98,"
                "\"flags\":0,"
                "\"name\":\"Classic Beta PvE\","
                "\"cfgConfigsID\":1,"
                "\"cfgContentSetID\":137,"
                "\"cfgLanguagesID\":1,"
                "\"superDistrictID\":2"
                "}";

            const std::string utilityInfoJson =
                "JSONUtilityInfo:{"
                "\"realmPermissions\":3,"
                "\"loginLicenses\":[0]"
                "}";

            const std::vector<uint8_t> realmEntry = compressGameUtilitiesJson(realmEntryJson);
            const std::vector<uint8_t> utilityInfo = compressGameUtilitiesJson(utilityInfoJson);

            std::vector<uint8_t> response;
            if (!realmEntry.empty())
                appendGameUtilitiesBlobAttribute(response, "Param_RealmEntry", realmEntry);

            // Exact no-character shape observed in the Forever beta capture:
            //   Param_CharacterName = ""
            //   Param_CharacterGUID = blob { 0x00, 0x00 }
            //   Param_LastPlayedTime = 1
            appendGameUtilitiesStringAttribute(response, "Param_CharacterName", "");

            const std::vector<uint8_t> emptyCharacterGuid{ 0x00u, 0x00u };
            appendGameUtilitiesBlobAttribute(response, "Param_CharacterGUID", emptyCharacterGuid);

            appendGameUtilitiesUInt64Attribute(response, "Param_LastPlayedTime", 1u);

            if (!utilityInfo.empty())
                appendGameUtilitiesBlobAttribute(response, "Param_UtilityInfo", utilityInfo);


            return sendRpcResponse(token, response);
        }


        return sendRpcResponse(token, std::vector<uint8_t>{});
    }


    bool BNetSocket::handleRealmJoinRequest(uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        uint64_t realmAddressValue = 0;
        if (!readGameUtilitiesUintAttribute(payload, payloadSize, "Param_RealmAddress", realmAddressValue) || realmAddressValue > std::numeric_limits<uint32_t>::max())
        {
            sLogger.failure("BNet: connection #{} RealmJoin request is missing a valid Param_RealmAddress", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        const uint32_t realmAddress = static_cast<uint32_t>(realmAddressValue);
        const uint8_t region = static_cast<uint8_t>((realmAddress >> 24) & 0xFFu);
        const uint8_t site = static_cast<uint8_t>((realmAddress >> 16) & 0xFFu);
        const uint16_t realmId = static_cast<uint16_t>(realmAddress & 0xFFFFu);

        if (m_selectedGameAccountId == 0 || m_selectedGameAccountName.empty())
        {
            sLogger.failure("BNet: connection #{} RealmJoin attempted before a WoW game account was selected by RealmListTicket", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        if (!m_realmListClientSecretValid)
        {
            sLogger.failure("BNet: connection #{} RealmJoin has no captured RealmList client secret; refusing to create unusable world auth KeyData", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        std::string realmJoinTicket;
        std::array<uint8_t, 32> joinSecret{};
        uint32_t localRealmId = 0u;
        const std::vector<uint8_t> response = makeRealmJoinResponse(m_clientBuild, m_selectedGameAccountName, realmAddress, bnetConfig.world.host, bnetConfig.world.port, realmJoinTicket, joinSecret, localRealmId);

        bool sessionQueued = false;
        if (!response.empty())
        {
            PendingWorldSession pending;
            pending.accountId = m_battleNetAccountId;
            pending.gameAccountId = m_selectedGameAccountId;
            pending.realmId = localRealmId;
            pending.clientBuild = m_clientBuild;
            pending.region = region;
            pending.expiresAt = static_cast<uint64_t>(UNIXTIME) + 60u;
            pending.gameAccountName = m_selectedGameAccountName;
            pending.realmJoinTicket = realmJoinTicket;
            std::copy(m_realmListClientSecret.begin(), m_realmListClientSecret.end(), pending.worldAuthKeyData.begin());
            std::copy(joinSecret.begin(), joinSecret.end(), pending.worldAuthKeyData.begin() + m_realmListClientSecret.size());
            pending.joinSecret = joinSecret;
            sessionQueued = sBattleNetCommManager.sendPendingSession(pending);
        }

        sLogger.debug("BNet: connection #{} RealmJoin -> token={}, realm_address=0x{:08X} (region={}, site={}, external_realm={}), local_realm={}, world={}:{}, game_account='{}', pending_session={}, response={} byte(s)", m_connectionId, token, realmAddress, region, site, realmId, localRealmId, bnetConfig.world.host, bnetConfig.world.port, m_selectedGameAccountName, sessionQueued ? "queued" : "missing-world", response.size());

        if (response.empty())
        {
            sLogger.failure("BNet: connection #{} failed to build RealmJoin response", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        if (!sessionQueued)
        {
            sLogger.failure("BNet: connection #{} cannot complete RealmJoin: local realm {} (external realm {}) has no authenticated BattleNetComm world connection", m_connectionId, localRealmId, realmId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleRealmListRequest(uint32_t token, const std::string& commandName)
    {
        if (m_selectedGameAccountId == 0)
        {
            sLogger.failure("BNet: connection #{} RealmList requested before RealmListTicket selected a WoW game account", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        // Realm list framing is shared; the advertised version must match the
        // authenticated client build instead of a fixed development build.
        const uint32_t clientBuild = m_clientBuild;
        const std::vector<uint8_t> response = makeRealmListResponse(clientBuild, m_selectedGameAccountId);

        sLogger.debug("BNet: connection #{} RealmList -> token={}, command='{}', build={}, account={}, response={} byte(s)", m_connectionId, token, commandName, clientBuild, m_selectedGameAccountId, response.size());

        if (response.empty())
            sLogger.failure("BNet: connection #{} failed to build RealmList response", m_connectionId);

        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleRealmListTicketRequest(uint32_t token, const std::string& commandName, const uint8_t* payload, size_t payloadSize)
    {
        uint32_t requestedGameAccountId = 0;
        if (!extractRealmListIdentityGameAccountId(payload, payloadSize, requestedGameAccountId))
        {
            std::vector<uint8_t> identityDiagnostic;
            std::string identityText = "<missing>";
            if (readGameUtilitiesBlobAttribute(payload, payloadSize, "Param_Identity", identityDiagnostic) && !identityDiagnostic.empty())
                identityText.assign(reinterpret_cast<const char*>(identityDiagnostic.data()), identityDiagnostic.size());

            sLogger.failure("BNet: connection #{} RealmListTicket request did not contain a valid Param_Identity gameAccountID; identity='{}'", m_connectionId, identityText);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        const auto linkedAccount = std::find_if(m_linkedGameAccounts.begin(), m_linkedGameAccounts.end(), [requestedGameAccountId](const LinkedGameAccount& account) { return account.id == requestedGameAccountId; });

        if (linkedAccount == m_linkedGameAccounts.end())
        {
            sLogger.failure("BNet: connection #{} RealmListTicket requested game account {} which is not linked to Battle.net account {}", m_connectionId, requestedGameAccountId, m_battleNetAccountId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        m_selectedGameAccountId = linkedAccount->id;
        m_selectedGameAccountName = linkedAccount->name;

        m_realmListClientSecretValid = extractRealmListClientSecret(payload, payloadSize, m_realmListClientSecret);
        if (!m_realmListClientSecretValid)
        {
            sLogger.failure("BNet: connection #{} RealmListTicket request did not contain a valid 32-byte client secret", m_connectionId);
            return sendRpcResponse(token, std::vector<uint8_t>{});
        }

        const std::vector<uint8_t> response = makeGameUtilitiesBlobAttribute("Param_RealmListTicket", "AuthRealmListTicket");

        sLogger.debug("BNet: connection #{} RealmListTicket -> token={}, command='{}', battlenet_account={}, selected_game_account={} ('{}'), response={} byte(s), client_secret=32 byte(s)", m_connectionId, token, commandName, m_battleNetAccountId, m_selectedGameAccountId, m_selectedGameAccountName, response.size());
        return sendRpcResponse(token, response);
    }

    bool BNetSocket::handleAccountServiceV2Request(uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize)
    {
        auto parseGameAccountHandle = [payload, payloadSize](uint32_t& accountId, uint32_t& titleId, uint32_t& region) -> bool
        {
            accountId = 0;
            titleId = 0;
            region = 0;

            if (payload == nullptr || payloadSize == 0)
                return false;

            // GetGameAccountInfo/GetGameAccountRestriction request:
            // field 1 = embedded GameAccountHandle { id=1, title_id=2, region=3 }.
            const uint8_t* cursor = payload;
            const uint8_t* const end = payload + payloadSize;
            uint64_t outerTag = 0;
            if (!readVarInt(cursor, end, outerTag) || (outerTag >> 3) != 1u || (outerTag & 0x07u) != 2u)
                return false;

            const uint8_t* handleData = nullptr;
            size_t handleSize = 0;
            if (!readLengthDelimited(cursor, end, handleData, handleSize))
                return false;

            const uint8_t* handleCursor = handleData;
            const uint8_t* const handleEnd = handleData + handleSize;
            while (handleCursor < handleEnd)
            {
                uint64_t tag = 0;
                if (!readVarInt(handleCursor, handleEnd, tag))
                    return false;

                const uint32_t fieldNumber = static_cast<uint32_t>(tag >> 3);
                const uint32_t wireType = static_cast<uint32_t>(tag & 0x07u);
                if (wireType != 0u)
                {
                    if (!skipProtobufField(wireType, handleCursor, handleEnd))
                        return false;
                    continue;
                }

                uint64_t value = 0;
                if (!readVarInt(handleCursor, handleEnd, value) || value > std::numeric_limits<uint32_t>::max())
                    return false;

                if (fieldNumber == 1u)
                    accountId = static_cast<uint32_t>(value);
                else if (fieldNumber == 2u)
                    titleId = static_cast<uint32_t>(value);
                else if (fieldNumber == 3u)
                    region = static_cast<uint32_t>(value);
            }

            return accountId != 0;
        };

        switch (methodId)
        {
            case Protocol::AccountServiceV2::GetAccountInfo:
            {
                // GetAccountInfoResponse.info -> AccountInfo
                // reference implementation currently emits account_id and FLAG_IS_HIDDEN_FROM_FRIEND_FINDER.
                std::vector<uint8_t> info;
                appendVarIntField(info, 1, m_battleNetAccountId);
                appendVarIntField(info, 14, 7u);

                std::vector<uint8_t> response;
                appendMessageField(response, 1, info);

                sLogger.debug("BNet: connection #{} AccountServiceV2.GetAccountInfo -> token={}, battlenet_account={}, response={} byte(s)", m_connectionId, token, m_battleNetAccountId, response.size());
                return sendRpcResponse(token, response);
            }

            case Protocol::AccountServiceV2::GetRestriction:
            {
                // No account-level restrictions for local development accounts.
                sLogger.debug("BNet: connection #{} AccountServiceV2.GetRestriction -> token={}, restrictions=0", m_connectionId, token);
                return sendRpcResponse(token, std::vector<uint8_t>{});
            }

            case Protocol::AccountServiceV2::GetGameAccountLinks:
            {
                // GetGameAccountLinksResponse.links -> GameAccountLinks.handles[]
                std::vector<uint8_t> links;
                for (const LinkedGameAccount& linked : m_linkedGameAccounts)
                {
                    std::vector<uint8_t> handle;
                    appendVarIntField(handle, 1, linked.id);
                    appendVarIntField(handle, 2, Protocol::WoW::TitleId);
                    appendVarIntField(handle, 3, Protocol::WoW::EuropeRegion);
                    appendMessageField(links, 1, handle);
                }

                std::vector<uint8_t> response;
                appendMessageField(response, 1, links);

                sLogger.debug("BNet: connection #{} AccountServiceV2.GetGameAccountLinks -> token={}, game_accounts={}, response={} byte(s)", m_connectionId, token, m_linkedGameAccounts.size(), response.size());
                return sendRpcResponse(token, response);
            }

            case Protocol::AccountServiceV2::GetGameAccountInfo:
            {
                uint32_t accountId = 0;
                uint32_t titleId = 0;
                uint32_t region = 0;
                if (!parseGameAccountHandle(accountId, titleId, region))
                {
                    sLogger.failure("BNet: connection #{} AccountServiceV2.GetGameAccountInfo could not decode GameAccountHandle", m_connectionId);
                    return sendRpcResponse(token, std::vector<uint8_t>{});
                }

                const auto linked = std::find_if(m_linkedGameAccounts.begin(), m_linkedGameAccounts.end(), [accountId](const LinkedGameAccount& account) { return account.id == accountId; });

                if (linked == m_linkedGameAccounts.end() || titleId != Protocol::WoW::TitleId || region != Protocol::WoW::EuropeRegion)
                {
                    sLogger.failure("BNet: connection #{} AccountServiceV2.GetGameAccountInfo rejected handle id={}, title_id={}, region={}", m_connectionId, accountId, titleId, region);
                    return sendRpcResponse(token, std::vector<uint8_t>{});
                }

                std::string displayName = linked->name;
                if (const size_t hashPos = displayName.find('#'); hashPos != std::string::npos)
                    displayName = std::string("WoW") + displayName.substr(hashPos + 1);

                // GetGameAccountInfoResponse.info -> GameAccountInfo
                std::vector<uint8_t> info;
                appendVarIntField(info, 1, linked->id);
                appendStringField(info, 2, displayName);

                std::vector<uint8_t> response;
                appendMessageField(response, 1, info);

                sLogger.debug("BNet: connection #{} AccountServiceV2.GetGameAccountInfo -> token={}, game_account={}, name='{}', response={} byte(s)", m_connectionId, token, linked->id, displayName, response.size());
                return sendRpcResponse(token, response);
            }

            case Protocol::AccountServiceV2::GetGameAccountRestriction:
            {
                uint32_t accountId = 0;
                uint32_t titleId = 0;
                uint32_t region = 0;
                if (!parseGameAccountHandle(accountId, titleId, region))
                {
                    sLogger.failure("BNet: connection #{} AccountServiceV2.GetGameAccountRestriction could not decode GameAccountHandle", m_connectionId);
                    return sendRpcResponse(token, std::vector<uint8_t>{});
                }

                const bool linked = std::any_of(m_linkedGameAccounts.begin(), m_linkedGameAccounts.end(), [accountId](const LinkedGameAccount& account) { return account.id == accountId; });

                if (!linked || titleId != Protocol::WoW::TitleId || region != Protocol::WoW::EuropeRegion)
                {
                    sLogger.failure("BNet: connection #{} AccountServiceV2.GetGameAccountRestriction rejected handle id={}, title_id={}, region={}", m_connectionId, accountId, titleId, region);
                    return sendRpcResponse(token, std::vector<uint8_t>{});
                }

                // Empty response means no restrictions, matching reference implementation for an unbanned account.
                sLogger.debug("BNet: connection #{} AccountServiceV2.GetGameAccountRestriction -> token={}, game_account={}, restrictions=0", m_connectionId, token, accountId);
                return sendRpcResponse(token, std::vector<uint8_t>{});
            }

            default:
                break;
        }

        sLogger.debug("BNet: connection #{} AccountServiceV2 method {} -> token={}, payload={} byte(s); replying NoData", m_connectionId, methodId, token, payloadSize);
        return sendRpcResponse(token, std::vector<uint8_t>{});
    }

    bool BNetSocket::sendExternalChallenge()
    {
        std::vector<uint8_t> challenge;
        challenge.reserve(96);

        // bgs.protocol.challenge.v1.ChallengeExternalRequest
        // field 1 (request_token) is optional and intentionally omitted.
        appendStringField(challenge, 2, "web_auth_url");
        appendStringField(challenge, 3, WEB_AUTH_URL);

        // Server-side RPC token space. This is currently our first
        // server-initiated request on a connection.
        constexpr uint32_t serverToken = 1u;

        sLogger.debug("BNet: connection #{} AuthenticationListenerV2.OnExternalChallenge -> " "token={}, payload_type='web_auth_url', payload='{}'", m_connectionId, serverToken, WEB_AUTH_URL);

        return sendRpcRequest(Protocol::AuthenticationListenerV2::Hash, Protocol::AuthenticationListenerV2::OnExternalChallenge, serverToken, challenge);
    }

    bool BNetSocket::sendRpcRequest(uint32_t serviceHash, uint32_t methodId, uint32_t token, const std::vector<uint8_t>& payload)
    {
        std::vector<uint8_t> header;
        header.reserve(24);

        appendVarIntField(header, 1, Protocol::BindlessServiceId);
        appendVarIntField(header, 2, methodId);
        appendVarIntField(header, 3, token);

        if (!payload.empty())
            appendVarIntField(header, 5, payload.size());

        appendFixed32Field(header, 11, serviceHash);

        if (header.empty() || header.size() > UINT16_MAX)
            return false;

        std::vector<uint8_t> frame;
        frame.reserve(2 + header.size() + payload.size());
        frame.push_back(static_cast<uint8_t>((header.size() >> 8) & 0xFFu));
        frame.push_back(static_cast<uint8_t>(header.size() & 0xFFu));
        frame.insert(frame.end(), header.begin(), header.end());
        frame.insert(frame.end(), payload.begin(), payload.end());

        sLogger.debug("BNet TX {}::{} size={} token={}", Protocol::getServiceName(serviceHash), Protocol::getMethodName(serviceHash, methodId), payload.size(), token);
        sLogger.debug("BNet TX frame service=0x{:08X} method={} header={} payload={}\n{}", serviceHash, methodId, header.size(), payload.size(), makeHexDump(frame.data(), std::min(frame.size(), MAX_DIAGNOSTIC_DUMP_SIZE)));

        return writeTlsPlainText(frame.data(), frame.size());
    }

    bool BNetSocket::sendRpcResponse(uint32_t token, const std::vector<uint8_t>& payload)
    {
        std::vector<uint8_t> header;
        header.reserve(16);
        appendVarIntField(header, 1, Protocol::ResponseServiceId);
        appendVarIntField(header, 3, token);
        if (!payload.empty())
            appendVarIntField(header, 5, payload.size());

        if (header.empty() || header.size() > UINT16_MAX)
            return false;

        std::vector<uint8_t> frame;
        frame.reserve(2 + header.size() + payload.size());
        frame.push_back(static_cast<uint8_t>((header.size() >> 8) & 0xFFu));
        frame.push_back(static_cast<uint8_t>(header.size() & 0xFFu));
        frame.insert(frame.end(), header.begin(), header.end());
        frame.insert(frame.end(), payload.begin(), payload.end());

        if (m_currentRpcServiceHash != 0)
        {
            sLogger.debug("BNet TX {}::{}Response size={} token={}", Protocol::getServiceName(m_currentRpcServiceHash), Protocol::getMethodName(m_currentRpcServiceHash, m_currentRpcMethodId), payload.size(), token);
        }
        else
        {
            sLogger.debug("BNet TX Response size={} token={}", payload.size(), token);
        }

        sLogger.debug("BNet TX response frame header={} payload={} token={}\n{}", header.size(), payload.size(), token, makeHexDump(frame.data(), std::min(frame.size(), MAX_DIAGNOSTIC_DUMP_SIZE)));

        return writeTlsPlainText(frame.data(), frame.size());
    }

    bool BNetSocket::writeTlsPlainText(const uint8_t* data, size_t size)
    {
        if (m_ssl == nullptr || !m_tlsHandshakeComplete || data == nullptr || size == 0)
            return false;

        size_t offset = 0;
        while (offset < size)
        {
            const size_t remaining = size - offset;
            const int chunkSize = static_cast<int>(std::min(remaining, static_cast<size_t>(INT_MAX)));
            const int result = SSL_write(m_ssl, data + offset, chunkSize);
            if (result > 0)
            {
                offset += static_cast<size_t>(result);
                if (!flushTlsOutput())
                    return false;
                continue;
            }

            const int error = SSL_get_error(m_ssl, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
            {
                if (!flushTlsOutput())
                    return false;
                continue;
            }

            logTlsError("write", result);
            return false;
        }

        return flushTlsOutput();
    }

    bool BNetSocket::flushTlsOutput()
    {
        if (m_ssl == nullptr)
            return false;

        BIO* writeBio = SSL_get_wbio(m_ssl);
        if (writeBio == nullptr)
            return false;

        std::array<uint8_t, TLS_IO_BUFFER_SIZE> encryptedData{};

        while (BIO_ctrl_pending(writeBio) > 0)
        {
            const int bytesRead = BIO_read(writeBio, encryptedData.data(), static_cast<int>(encryptedData.size()));
            if (bytesRead <= 0)
                return false;

            if (!send(encryptedData.data(), static_cast<uint32_t>(bytesRead)))
            {
                sLogger.failure("BNet: connection #{} failed to queue {} TLS byte(s) for sending", m_connectionId, bytesRead);
                return false;
            }

            m_sentBytes += static_cast<size_t>(bytesRead);
        }

        return true;
    }

    void BNetSocket::logTlsError(const char* operation, int result) const
    {
        const int sslError = SSL_get_error(m_ssl, result);
        const unsigned long openSslError = ERR_get_error();

        if (openSslError == 0)
        {
            sLogger.failure("BNet: connection #{} TLS {} failed (SSL error {})", m_connectionId, operation, sslError);
            return;
        }

        char errorBuffer[256]{};
        ERR_error_string_n(openSslError, errorBuffer, sizeof(errorBuffer));
        sLogger.failure("BNet: connection #{} TLS {} failed (SSL error {}): {}", m_connectionId, operation, sslError, errorBuffer);
    }

    void BNetSocket::releaseTls()
    {
        if (m_ssl == nullptr)
            return;

        SSL_free(m_ssl);
        m_ssl = nullptr;
        m_tlsHandshakeComplete = false;
        m_plainTextBuffer.clear();
    }

    void BNetSocket::onDisconnect()
    {
        const auto connectedFor = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_connectedAt);

        sLogger.info("BNet: connection #{} disconnected {}:{} after {} ms (received: {} byte(s), sent: {} byte(s))", m_connectionId, getRemoteIp(), getRemotePort(), connectedFor.count(), m_receivedBytes, m_sentBytes);

        releaseTls();
    }
}
