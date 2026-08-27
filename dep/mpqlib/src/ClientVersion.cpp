/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ClientVersion.hpp"

namespace mpqlib
{
    std::optional<ClientVersion> clientVersionFromBuild(uint32_t build)
    {
        // The exact build number a real client reports (e.g. via
        // component.wow-<locale>.txt) is whatever patch level that client
        // happens to be - not necessarily one of the five reference builds
        // AEVersion.hpp uses per expansion. Build numbers increase
        // monotonically release-over-release with no overlap between
        // expansions, so classify by range against those same five
        // thresholds rather than requiring an exact match.
        if (build == 0)
            return std::nullopt;

        if (build < static_cast<uint32_t>(ClientVersion::BurningCrusade))
            return ClientVersion::Vanilla;
        if (build < static_cast<uint32_t>(ClientVersion::WrathOfTheLichKing))
            return ClientVersion::BurningCrusade;
        if (build < static_cast<uint32_t>(ClientVersion::Cataclysm))
            return ClientVersion::WrathOfTheLichKing;
        if (build < static_cast<uint32_t>(ClientVersion::MistsOfPandaria))
            return ClientVersion::Cataclysm;

        return ClientVersion::MistsOfPandaria;
    }
}
