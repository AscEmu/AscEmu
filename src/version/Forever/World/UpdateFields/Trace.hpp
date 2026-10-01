/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "FieldDefinition.hpp"
#include "Logging/Logger.hpp"

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace AscEmu::Version::Forever::UpdateFields
{
    template <typename Definition, typename Owner>
    void traceChangedFields(std::string_view scope, Owner const& owner)
    {
        if (!owner.changes.any())
            return;

        Definition::traceChangedFields(owner, [scope](std::string_view name, std::string_view referenceName, FieldVerification verification, std::string_view wireType, std::size_t bit, int32_t index, auto value)
        {
            using ValueType = std::remove_cvref_t<decltype(value)>;
            if constexpr (std::is_floating_point_v<ValueType>)
            {
                if (referenceName.empty())
                    sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} type={} value={}", scope, bit, index, name, verificationName(verification), wireType, static_cast<double>(value));
                else
                    sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} reference={} type={} value={}", scope, bit, index, name, verificationName(verification), referenceName, wireType, static_cast<double>(value));
            }
            else
            {
                if (referenceName.empty())
                    sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} type={} value={}", scope, bit, index, name, verificationName(verification), wireType, static_cast<int64_t>(value));
                else
                    sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} reference={} type={} value={}", scope, bit, index, name, verificationName(verification), referenceName, wireType, static_cast<int64_t>(value));
            }
        });
    }

    inline void traceManualField(std::string_view scope, std::size_t bit, int32_t index, std::string_view name, FieldVerification verification, std::string_view referenceName, std::string_view wireType, int64_t value)
    {
        if (referenceName.empty())
            sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} type={} value={}", scope, bit, index, name, verificationName(verification), wireType, value);
        else
            sLogger.info("[ForeverDebug][UF][Field] scope={} bit={} index={} name={} status={} reference={} type={} value={}", scope, bit, index, name, verificationName(verification), referenceName, wireType, value);
    }
}
