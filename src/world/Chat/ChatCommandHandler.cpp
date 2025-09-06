/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ChatCommandHandler.hpp"

#include "ChatCommand.hpp"
#include "CommandTableStorage.hpp"
#include "Exceptions/PlayerExceptions.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemInterface.h"
#include "Management/SkillNameMgr.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Storage/MySQLDataStore.hpp"
#include <cstdarg>

using namespace AscEmu::Packets;

ChatCommandHandler::ChatCommandHandler() = default;
ChatCommandHandler::~ChatCommandHandler() = default;

ChatCommandHandler& ChatCommandHandler::getInstance()
{
    static ChatCommandHandler mInstance;
    return mInstance;
}

void ChatCommandHandler::initialize()
{
    SkillNameManager = std::make_unique<SkillNameMgr>();
}

void ChatCommandHandler::finalize()
{
}

void ChatCommandHandler::SendMultilineMessage(WorldSession* m_session, const char* str)
{
    char* start = (char*)str, *end;
    for (;;)
    {
        end = strchr(start, '\n');
        if (!end)
            break;

        *end = '\0';
        systemMessage(m_session, start);
        start = end + 1;
    }
    if (*start != '\0')
        systemMessage(m_session, start);
}

// normalize command input once while parsing that should be enough (was in 4 places before)
std::optional<std::string_view> ChatCommandHandler::normalizeCommandInput(const char* raw)
{
    if (!raw)
        return std::nullopt;

    // skip leading whitespace
    const char* p = raw;
    while (*p && std::isspace(static_cast<unsigned char>(*p)))
        ++p;

    // must start with '.' or '!'
    if (*p != '.' && *p != '!')
        return std::nullopt;

    // reject ".."
    if (p[1] == '.')
        return std::nullopt;

    // skip the sigil
    ++p;

    // spaces after sigil
    while (*p && std::isspace(static_cast<unsigned char>(*p)))
        ++p;

    if (*p == '\0')
        return std::nullopt;

    // build string_view to the end
    return std::string_view{ p, std::strlen(p) };
}

// Resolve the 1st token to a unique top-level command word (case-insensitive, abbreviation).
// Returns true and writes 'outTop' on unique match (with permission), otherwise false.
// Top-level == registry entries whose command has exactly one word.
bool ChatCommandHandler::resolveTopLevelAbbrev(std::string_view tok0, WorldSession* s, std::string& outTop) const
{
    auto &reg = sCommandTableStorage.getCommandRegistry();

    auto lo = [](char c){
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        };
    auto istarts_with = [&](std::string_view a, std::string_view b){
        if (b.size() > a.size())
            return false;

        for (size_t i=0; i<b.size(); ++i)
            if (lo(a[i]) != lo(b[i]))
                return false;

        return true;
    };

    std::vector<std::string> matches;

    for (const auto& e : reg)
    {
        if (e.command.empty())
            continue;

        // split command into words (top-level == exactly 1 word)
        std::istringstream is(e.command);
        std::string w0, more;
        if (!(is >> w0) || (is >> more))
            continue; // not exactly one word

        const char perm = e.commandPermission.empty() ? '0' : e.commandPermission[0];
        if (perm != '0' && !s->CanUseCommand(perm))
            continue;

        // exact (case-insensitive) wins immediately
        if (w0.size() == tok0.size())
        {
            bool eq = true;
            for (size_t i=0; i<w0.size(); ++i)
            {
                if (lo(w0[i]) != lo(tok0[i]))
                {
                    eq = false;
                    break;
                }
            }

            if (eq)
            {
                outTop = w0;
                return true;
            }
        }

        // abbreviation candidate
        if (istarts_with(w0, tok0))
            matches.push_back(std::move(w0));
    }

    if (matches.size() == 1)
    {
        outTop = matches[0];
        return true;
    }

    return false; // unknown or ambiguous
}

bool ChatCommandHandler::executeCommandFlat(std::string_view text, WorldSession* m_session)
{
    if (text.empty())
        return false;

    // split input into tokens (views)
    std::vector<std::string_view> tokens;
    {
        size_t p = 0;
        while (p < text.size())
        {
            while (p < text.size() && std::isspace(static_cast<unsigned char>(text[p])))
                ++p;

            if (p >= text.size())
                break;

            size_t q = p;
            while (q < text.size() && !std::isspace(static_cast<unsigned char>(text[q])))
                ++q;

            tokens.emplace_back(text.substr(p, q - p));
            p = q;
        }
    }
    if (tokens.empty())
        return false;

    // resolve only the first token to a top-level command
    std::string top;
    if (!resolveTopLevelAbbrev(tokens[0], m_session, top))
        return false; // unknown or ambiguous top-level

    auto lo = [](char c){
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        };
    auto istarts_with = [&](std::string_view a, std::string_view b){
        if (b.size() > a.size())
            return false;

        for (size_t i=0; i<b.size(); ++i)
            if (lo(a[i]) != lo(b[i]))
                return false;

        return true;
    };

    // find deepest matching command (multi-token abbrev)
    const ChatCommand* chosen = nullptr;
    std::string           chosenCmd;
    size_t                matchedDepth = 0;

    const auto& reg = sCommandTableStorage.getCommandRegistry();

    for (const auto& e : reg)
    {
        if (e.command.empty())
            continue;

        // split command into words
        std::vector<std::string> words;
        {
            std::istringstream is(e.command);
            for (std::string w; is >> w; ) words.push_back(std::move(w));
        }
        if (words.empty())
            continue;

        // cannot be deeper than what the user typed
        if (words.size() > tokens.size())
            continue;

        // first word must equal the resolved top-level exactly (case-insensitive)
        if (words[0].size() != top.size()) {
            // fast-path size check (kept to mirror your structure)
        }
        {
            bool eq = true;
            for (size_t i=0; i<top.size(); ++i)
            {
                if (lo(words[0][i]) != lo(top[i]))
                {
                    eq = false;
                    break;
                }
            }
            if (!eq)
                continue;
        }

        // remaining words (if any) are matched by abbreviation
        bool ok = true;
        for (size_t i = 1; i < words.size(); ++i)
        {
            if (!istarts_with(words[i], tokens[i]))
            {
                ok = false;
                break;
            }
        }

        if (!ok)
            continue;

        const char perm = e.commandPermission.empty() ? '0' : e.commandPermission[0];
        if (perm != '0' && !m_session->CanUseCommand(perm))
            continue;

        // prefer deepest match
        if (words.size() > matchedDepth)
        {
            chosen     = &e;
            chosenCmd  = e.command;
            matchedDepth = words.size();
        }
    }

    // If there is no matching entry, treat it as a node prefix: show its children.
    if (!chosen)
    {
        // Build the prefix string: <top> + remaining typed tokens (as text),
        // but do NOT auto-append anything else.
        std::string prefix = top;
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            prefix.push_back(' ');
            prefix.append(tokens[i].data(), tokens[i].size());
        }
        std::string prefixWithSpace = prefix + ' ';

        // List subcommands under this prefix the user can use.
        bool any = false;
        for (const auto& sub : reg)
        {
            if (sub.command.size() <= prefixWithSpace.size())
                continue;

            // case-insensitive startswith on the whole prefix string
            bool starts = true;
            for (size_t k = 0; k < prefixWithSpace.size(); ++k)
            {
                if (lo(sub.command[k]) != lo(prefixWithSpace[k]))
                {
                    starts = false;
                    break;
                }
            }

            if (!starts)
                continue;

            const char p = sub.commandPermission.empty() ? '0' : sub.commandPermission[0];
            if (p != '0' && !m_session->CanUseCommand(p))
                continue;

            auto help = sub.help.empty() ? "No Help Available" : sub.help;
            blueSystemMessage(m_session, " {} - {}", sub.command.substr(prefixWithSpace.size()), help);

            any = true;
        }

        if (!any)
            systemMessage(m_session, "There is no help for that command");
        else
            greenSystemMessage(m_session, "Available Subcommands:");

        return true;
    }

    // Exact/deepest entry found → either node (no handler) or leaf (handler)

    if (!chosen->handler)
    {
        if (!chosen->help.empty())
            SendMultilineMessage(m_session, chosen->help.c_str());
        else
            greenSystemMessage(m_session, "Available Subcommands:");

        const std::string prefix = chosenCmd + ' ';
        bool any = false;

        for (const auto& sub : reg)
        {
            if (sub.command.size() <= prefix.size())
                continue;

            bool starts = true;
            for (size_t k = 0; k < prefix.size(); ++k)
            {
                if (lo(sub.command[k]) != lo(prefix[k]))
                {
                    starts = false;
                    break;
                }
            }

            if (!starts)
                continue;

            const char p = sub.commandPermission.empty() ? '0' : sub.commandPermission[0];
            if (p != '0' && !m_session->CanUseCommand(p))
                continue;

            auto help = sub.help.empty() ? "No Help Available" : sub.help;
            blueSystemMessage(m_session, " {} - {}", sub.command.substr(prefix.size()), help);
            any = true;
        }

        if (!any && chosen->help.empty())
            systemMessage(m_session, "There is no help for that command");

        return true;
    }

    // pass "args" = everything after the matchedDepth words (not after all tokens!)
    std::string_view args = text;
    {
        size_t consumed = 0, seen = 0;
        while (consumed < text.size() && seen < matchedDepth)
        {
            while (consumed < text.size() && std::isspace(static_cast<unsigned char>(text[consumed])))
                ++consumed;

            while (consumed < text.size() && !std::isspace(static_cast<unsigned char>(text[consumed])))
                ++consumed;

            ++seen;
        }
        while (consumed < text.size() && std::isspace(static_cast<unsigned char>(text[consumed])))
            ++consumed;
        args = text.substr(consumed);
    }

    // check if min argument count was reached
    const size_t argc = countWords(args);

    if (argc < chosen->minArgCount)
    {
        if (!chosen->help.empty())
            SendMultilineMessage(m_session, chosen->help.c_str());
        else
            redSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", chosenCmd.c_str());

        return true;
    }

    // actually execute the command
    const bool ok = chosen->handler(this, args, m_session);
    if (!ok)
    {
        if (!chosen->help.empty())
            SendMultilineMessage(m_session, chosen->help.c_str());
        else
            redSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", chosenCmd.c_str());
    }

    return true;
}

bool ChatCommandHandler::executeCommand(std::string_view text, WorldSession* m_session)
{
    return executeCommandFlat(text, m_session);
}

int ChatCommandHandler::ParseCommands(const char* text, WorldSession* session)
{
    if (!session)
        return 0;

    if (!*text)
        return 0;

    if (!session->HasGMPermissions() && worldConfig.server.requireGmForCommands)
        return 0;

    auto normalized = normalizeCommandInput(text);
    if (!normalized)
        return 0;

    try
    {
        bool success = executeCommand(*normalized, session);
        if (!success)
        {
            systemMessage(session, "There is no such command, or you do not have access to it.");
        }
    }
    catch (AscEmu::Exception::PlayerNotFoundException& e)
    {
        // TODO: Handle this properly (what do we do when we're running commands with no player object?)
        sLogger.failure("PlayerNotFoundException occurred when processing command [{}]. Exception: {}", text, e.AEwhat());
    }

    return 1;
}

Player* ChatCommandHandler::GetSelectedPlayer(WorldSession* m_session, bool showerror, bool auto_self)
{
    if (m_session == nullptr)
        return nullptr;

    bool is_creature = false;
    Player* player_target = nullptr;
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    switch (wowGuid.getHigh())
    {
        case HighGuid::Pet:
        case HighGuid::Unit:
        case HighGuid::Vehicle:
        {
            is_creature = true;
            break;
        }
        default:
            break;
    }

    if (guid == 0 || is_creature)
    {
        if (auto_self)
        {
            m_session->systemMessage("Auto-targeting self.");
            player_target = m_session->GetPlayer();
        }
        else
        {
            if (showerror)
                m_session->systemMessage("This command requires a selected player.");

            return nullptr;
        }
    }
    else
    {
        player_target = m_session->GetPlayer()->getWorldMap()->getPlayer((uint32_t)guid);
    }

    return player_target;
}

Creature* ChatCommandHandler::GetSelectedCreature(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr)
        return nullptr;

    Creature* creature = nullptr;
    bool is_invalid_type = false;
    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    switch(wowGuid.getHigh())
    {
        case HighGuid::Pet:
            creature = reinterpret_cast<Creature*>(m_session->GetPlayer()->getWorldMap()->getPet(wowGuid.getGuidLowPart()));
            break;

        case HighGuid::Unit:
        case HighGuid::Vehicle:
            creature = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
            break;
        default:
            is_invalid_type = true;
            break;
    }

    if (creature == nullptr || is_invalid_type)
    {
        if (showerror)
            redSystemMessage(m_session, "This command requires a selected a creature.");

        return nullptr;
    }

    return creature;
}

Unit* ChatCommandHandler::GetSelectedUnit(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr || m_session->GetPlayer() == nullptr)
        return nullptr;

    uint64_t guid = m_session->GetPlayer()->getTargetGuid();

    Unit* unit = m_session->GetPlayer()->getWorldMap()->getUnit(guid);
    if (unit == nullptr)
    {
        if (showerror)
            redSystemMessage(m_session, "You need to select a unit!");
        return nullptr;
    }

    return unit;
}

uint32_t ChatCommandHandler::GetSelectedWayPointId(WorldSession* m_session)
{
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (guid == 0)
    {
        systemMessage(m_session, "No selection.");
        return 0;
    }

    if (!wowGuid.isWaypoint())
    {
        systemMessage(m_session, "You should select a Waypoint.");
        return 0;
    }

    return WoWGuid::getGuidLowPartFromUInt64(guid);
}

const char* ChatCommandHandler::GetMapTypeString(uint8_t type)
{
    switch (type)
    {
        case INSTANCE_NULL:
            return "Continent";
        case INSTANCE_RAID:
            return "Raid";
        case INSTANCE_DUNGEON:
            return "Non-Raid";
        case INSTANCE_BATTLEGROUND:
            return "PvP";
        case INSTANCE_MULTIMODE:
            return "MultiMode";
        default:
            return "Unknown";
    }
}

const char* ChatCommandHandler::GetDifficultyString(uint8_t difficulty)
{
    switch (difficulty)
    {
        case InstanceDifficulty::DUNGEON_NORMAL:
            return "normal";
        case InstanceDifficulty::DUNGEON_HEROIC:
            return "heroic";
        default:
            return "unknown";
    }
}

const char* ChatCommandHandler::GetRaidDifficultyString(uint8_t diff)
{
    switch (diff)
    {
        case InstanceDifficulty::RAID_10MAN_NORMAL:
            return "normal 10man";
        case InstanceDifficulty::RAID_25MAN_NORMAL:
            return "normal 25man";
        case InstanceDifficulty::RAID_10MAN_HEROIC:
            return "heroic 10man";
        case InstanceDifficulty::RAID_25MAN_HEROIC:
            return "heroic 25man";
        default:
            return "unknown";
    }
}

void ChatCommandHandler::sendSystemMessagePacket(WorldSession* _session, std::string& _message)
{
    if (_session != nullptr)
        _session->SendPacket(SmsgMessageChat(SystemMessagePacket(_message)).serialise().get());
}

void ChatCommandHandler::SendHighlightedName(WorldSession* m_session, const char* prefix, const char* full_name, std::string & lowercase_name, std::string & highlight, uint32_t id)
{
    char message[1024];
    char start[50];
    start[0] = 0;
    message[0] = 0;

    snprintf(start, 50, "%s %u: %s", prefix, id, MSG_COLOR_WHITE);

    auto highlight_length = highlight.length();
    std::string fullname = std::string(full_name);
    size_t offset = lowercase_name.find(highlight);
    auto remaining = fullname.size() - offset - highlight_length;

    strcat(message, start);
    strncat(message, fullname.c_str(), offset);
    strcat(message, MSG_COLOR_LIGHTRED);
    strncat(message, (fullname.c_str() + offset), highlight_length);
    strcat(message, MSG_COLOR_WHITE);
    if (remaining > 0)
        strncat(message, (fullname.c_str() + offset + highlight_length), remaining);

    systemMessage(m_session, message);
}
