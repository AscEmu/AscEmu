/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <zlib.h>

// Attribute lists of the game utilities calls the 12.x engine sends through the world connection.
namespace AscEmu::BattlenetRpc
{
    constexpr uint32_t GameUtilitiesService = 0x5DBB51C2;
    constexpr uint32_t ClientRequestMethod = 1;
    constexpr uint32_t MethodIdMask = 0x3FFFFFFF;

    // field numbers inside the value of an attribute
    constexpr uint32_t ValueInt = 2;
    constexpr uint32_t ValueBlob = 5;
    constexpr uint32_t ValueUInt = 6;

    struct Field
    {
        uint32_t number = 0;
        uint32_t wireType = 0;
        uint64_t value = 0;             // wire types 0, 1 and 5
        const uint8_t* data = nullptr;  // wire type 2
        size_t size = 0;
    };

    inline bool readVarInt(const uint8_t*& cursor, const uint8_t* end, uint64_t& value)
    {
        value = 0;
        for (uint32_t shift = 0; shift < 64 && cursor < end; shift += 7)
        {
            const uint8_t byte = *cursor++;
            value |= static_cast<uint64_t>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0)
                return true;
        }
        return false;
    }

    // the next field of a message, false at its end or on malformed data
    inline bool readField(const uint8_t*& cursor, const uint8_t* end, Field& field)
    {
        uint64_t key = 0;
        if (cursor >= end || !readVarInt(cursor, end, key))
            return false;

        field = Field{};
        field.number = static_cast<uint32_t>(key >> 3);
        field.wireType = static_cast<uint32_t>(key & 7);

        switch (field.wireType)
        {
            case 0:
                return readVarInt(cursor, end, field.value);
            case 1:
            case 5:
            {
                const size_t width = field.wireType == 1 ? 8 : 4;
                if (static_cast<size_t>(end - cursor) < width)
                    return false;
                for (size_t i = 0; i < width; ++i)
                    field.value |= static_cast<uint64_t>(cursor[i]) << (8 * i);
                cursor += width;
                return true;
            }
            case 2:
            {
                uint64_t length = 0;
                if (!readVarInt(cursor, end, length) || length > static_cast<uint64_t>(end - cursor))
                    return false;
                field.data = cursor;
                field.size = static_cast<size_t>(length);
                cursor += field.size;
                return true;
            }
            default:
                return false;
        }
    }

    struct Request
    {
        std::string command;
        std::optional<uint64_t> contentSet;
    };

    // the command name and the content set filter of a client request
    inline Request readRequest(const std::vector<uint8_t>& data)
    {
        Request request;
        const uint8_t* cursor = data.data();
        const uint8_t* const end = cursor + data.size();

        Field attribute;
        while (readField(cursor, end, attribute))
        {
            if (attribute.number != 1 || attribute.wireType != 2)
                continue;

            std::string_view name;
            Field value;
            Field part;
            const uint8_t* inner = attribute.data;
            const uint8_t* const innerEnd = inner + attribute.size;
            while (readField(inner, innerEnd, part))
            {
                if (part.number == 1 && part.wireType == 2)
                    name = std::string_view(reinterpret_cast<const char*>(part.data), part.size);
                else if (part.number == 2 && part.wireType == 2)
                    value = part;
            }

            if (name.starts_with("Command_"))
            {
                request.command = name;
            }
            else if (name == "Param_ContentSetIDFilter" && value.data != nullptr)
            {
                const uint8_t* number = value.data;
                while (readField(number, value.data + value.size, part))
                {
                    if (part.wireType == 0 && (part.number == ValueInt || part.number == ValueUInt))
                        request.contentSet = part.value;
                }
            }
        }
        return request;
    }

    inline void writeVarInt(std::vector<uint8_t>& out, uint64_t value)
    {
        while (value >= 0x80)
        {
            out.push_back(static_cast<uint8_t>(value | 0x80));
            value >>= 7;
        }
        out.push_back(static_cast<uint8_t>(value));
    }

    inline void writeBytesField(std::vector<uint8_t>& out, uint32_t number, const uint8_t* data, size_t size)
    {
        writeVarInt(out, (static_cast<uint64_t>(number) << 3) | 2);
        writeVarInt(out, size);
        out.insert(out.end(), data, data + size);
    }

    // an attribute is its name and a value message
    inline void writeAttribute(std::vector<uint8_t>& out, std::string_view name, const std::vector<uint8_t>& value)
    {
        std::vector<uint8_t> attribute;
        writeBytesField(attribute, 1, reinterpret_cast<const uint8_t*>(name.data()), name.size());
        writeBytesField(attribute, 2, value.data(), value.size());
        writeBytesField(out, 1, attribute.data(), attribute.size());
    }

    inline void writeBlobAttribute(std::vector<uint8_t>& out, std::string_view name, const std::vector<uint8_t>& blob)
    {
        std::vector<uint8_t> value;
        writeBytesField(value, ValueBlob, blob.data(), blob.size());
        writeAttribute(out, name, value);
    }

    inline void writeIntAttribute(std::vector<uint8_t>& out, std::string_view name, uint64_t number)
    {
        std::vector<uint8_t> value;
        writeVarInt(value, static_cast<uint64_t>(ValueInt) << 3);
        writeVarInt(value, number);
        writeAttribute(out, name, value);
    }

    // the form of the json values: size of the zero terminated text, then its zlib stream
    inline std::vector<uint8_t> compressText(std::string_view text)
    {
        const std::string terminated(text);
        const uLong sourceSize = static_cast<uLong>(terminated.size() + 1);
        uLongf compressedSize = compressBound(sourceSize);

        std::vector<uint8_t> result(4 + compressedSize);
        for (uint32_t i = 0; i < 4; ++i)
            result[i] = static_cast<uint8_t>(sourceSize >> (8 * i));

        if (compress(result.data() + 4, &compressedSize, reinterpret_cast<const Bytef*>(terminated.c_str()), sourceSize) != Z_OK)
            return {};

        result.resize(4 + compressedSize);
        return result;
    }
}
