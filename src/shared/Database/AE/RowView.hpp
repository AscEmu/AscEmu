/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../Database.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace AscEmu::AE::Database
{
    class RowView final
    {
    public:
        RowView(const Field* fields, std::size_t fieldCount)
            : m_fields(fields), m_fieldCount(fieldCount)
        {
        }

        bool isSet(std::size_t index) const
        {
            return index < m_fieldCount && m_fields[index].isSet();
        }

        std::string_view getStringView(std::size_t index) const
        {
            return index < m_fieldCount ? m_fields[index].asStringView() : std::string_view{};
        }

        template <typename T>
        T get(std::size_t index) const
        {
            static_assert(sizeof(T) == 0, "Unsupported RowView::get<T>().");
        }

    private:
        const Field* m_fields = nullptr;
        std::size_t m_fieldCount = 0;
    };

    template <>
    inline uint8_t RowView::get<uint8_t>(std::size_t index) const
    {
        return m_fields[index].asUint8();
    }

    template <>
    inline uint16_t RowView::get<uint16_t>(std::size_t index) const
    {
        return m_fields[index].asUint16();
    }

    template <>
    inline uint32_t RowView::get<uint32_t>(std::size_t index) const
    {
        return m_fields[index].asUint32();
    }

    template <>
    inline uint64_t RowView::get<uint64_t>(std::size_t index) const
    {
        return m_fields[index].asUint64();
    }

    template <>
    inline int8_t RowView::get<int8_t>(std::size_t index) const
    {
        return m_fields[index].asInt8();
    }

    template <>
    inline int16_t RowView::get<int16_t>(std::size_t index) const
    {
        return m_fields[index].asInt16();
    }

    template <>
    inline int32_t RowView::get<int32_t>(std::size_t index) const
    {
        return m_fields[index].asInt32();
    }

    template <>
    inline int64_t RowView::get<int64_t>(std::size_t index) const
    {
        return m_fields[index].asInt64();
    }

    template <>
    inline float RowView::get<float>(std::size_t index) const
    {
        return m_fields[index].asFloat();
    }

    template <>
    inline bool RowView::get<bool>(std::size_t index) const
    {
        return m_fields[index].asBool();
    }

    template <>
    inline std::string RowView::get<std::string>(std::size_t index) const
    {
        const auto view = m_fields[index].asStringView();
        return std::string(view.begin(), view.end());
    }

    template <typename T, typename Mapper>
    std::vector<T> queryMany(::Database& db, std::string_view sql, Mapper&& mapper)
    {
        std::string ownedSql(sql);
        auto result = db.QueryNA(ownedSql.c_str());

        std::vector<T> out;
        if (!result)
            return out;

        out.reserve(result->GetRowCount());

        do
        {
            out.push_back(std::forward<Mapper>(mapper)(RowView(result->Fetch(), result->GetFieldCount())));
        } while (result->NextRow());

        return out;
    }

    template <typename T, typename Mapper>
    std::optional<T> queryOne(::Database& db, std::string_view sql, Mapper&& mapper)
    {
        std::string ownedSql(sql);
        auto result = db.QueryNA(ownedSql.c_str());
        if (!result)
            return std::nullopt;

        return std::forward<Mapper>(mapper)(RowView(result->Fetch(), result->GetFieldCount()));
    }
}
