#include "world/Server/WorldSocket.hpp"
#include "world/Server/WorldSession.h"
#include "world/Objects/Units/Players/Player.hpp"
#include "world/Spell/SpellMgr.hpp"
#include "world/Spell/SpellInfo.hpp"
#include "Logging/Logger.hpp"
#include "Utilities/Util.hpp"

#include <cstring>
#include <string>
#include <unordered_set>

bool WorldSocket::handleForeverCastSpellOpcode(WorldPacket& packet)
{
    if (m_session == nullptr || m_session->GetPlayer() == nullptr)
        return false;

    Player* const player = m_session->GetPlayer();
    const uint8_t* const data = packet.contents() + packet.rpos();
    const size_t size = packet.remaining();
    std::string candidates;
    std::unordered_set<uint32_t> seen;

    for (size_t offset = 0; offset + sizeof(uint32_t) <= size; ++offset)
    {
        uint32_t spellId = 0;
        std::memcpy(&spellId, data + offset, sizeof(spellId));
        if (spellId == 0 || !seen.emplace(spellId).second || sSpellMgr.getSpellInfo(spellId) == nullptr || !player->hasSpell(spellId))
            continue;

        if (!candidates.empty())
            candidates += ", ";
        candidates += std::to_string(spellId) + "@" + std::to_string(offset);
    }

    sLogger.info("WorldSocket::Forever: CMSG_CAST_SPELL probe size={} knownSpellCandidates=[{}] hex=[{}].", size, candidates, Util::ByteArrayToHexString(data, static_cast<uint32_t>(size)));
    return true;
}
