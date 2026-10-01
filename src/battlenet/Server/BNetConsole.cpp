/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetConsole.hpp"

#include "Master.hpp"

#include "Cryptography/BNetSrpV1.hpp"
#include "Database/Database.hpp"
#include "Database/Field.hpp"
#include "Logging/Logger.hpp"
#include "Threading/Thread.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

namespace AscEmu::Battlenet
{
    namespace
    {
        std::string trim(std::string value)
        {
            const auto notSpace = [](unsigned char c) { return !std::isspace(c); };

            value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
            value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
            return value;
        }

        std::string lower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        std::string makeDefaultBattleTag(const std::string& email)
        {
            const size_t at = email.find('@');
            std::string tag = at == std::string::npos ? email : email.substr(0, at);

            if (tag.empty())
                tag = "Player";

            return tag + "#1";
        }

        bool readToken(std::istringstream& stream, std::string& value)
        {
            value.clear();
            stream >> value;
            return !value.empty();
        }

        bool getBattleNetAccountId(const std::string& email, uint32_t& id)
        {
            id = 0;
            if (!sBNetLogonSQL)
                return false;

            const std::string escapedEmail = sBNetLogonSQL->escapeString(email);
            auto result = sBNetLogonSQL->query(
                "SELECT id FROM battlenet_accounts WHERE UPPER(email) = UPPER('%s') LIMIT 1",
                escapedEmail.c_str());

            if (!result)
                return false;

            id = result->fetch()[0].asUint32();
            return id != 0;
        }

        bool getGameAccountId(const std::string& accountName, uint32_t& id)
        {
            id = 0;
            if (!sBNetLogonSQL)
                return false;

            const std::string escapedName = sBNetLogonSQL->escapeString(accountName);
            auto result = sBNetLogonSQL->query(
                "SELECT id FROM accounts WHERE UPPER(acc_name) = UPPER('%s') LIMIT 1",
                escapedName.c_str());

            if (!result)
                return false;

            id = result->fetch()[0].asUint32();
            return id != 0;
        }
    }

    BNetConsole& BNetConsole::getInstance()
    {
        static BNetConsole instance;
        return instance;
    }

    void BNetConsole::run(AscEmu::Threading::AEThread& thread)
    {
        m_running.store(true);

        sLogger.info("BNet Console: ready. Type 'help' for commands.");

#ifndef WIN32
        fd_set fds;
        timeval timeout{};
#endif

        while (m_running.load() && !thread.isKilled())
        {
#ifndef WIN32
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            const int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout);
            if (ready <= 0)
                continue;
#endif

            std::string input;
            if (!std::getline(std::cin, input))
            {
                if (!m_running.load() || thread.isKilled())
                    break;

                std::cin.clear();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            input = trim(input);
            if (!input.empty())
                processCommand(input);
        }

        sLogger.info("BNet Console: stopped.");
    }

    void BNetConsole::stop()
    {
        m_running.store(false);

#ifdef WIN32
        // std::getline() blocks on the Windows console. Inject Enter so the
        // dedicated console thread can observe m_running and terminate.
        DWORD written = 0;
        INPUT_RECORD records[2]{};

        records[0].EventType = KEY_EVENT;
        records[0].Event.KeyEvent.bKeyDown = TRUE;
        records[0].Event.KeyEvent.wRepeatCount = 1;
        records[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
        records[0].Event.KeyEvent.uChar.UnicodeChar = L'\r';

        records[1] = records[0];
        records[1].Event.KeyEvent.bKeyDown = FALSE;

        WriteConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), records, 2, &written);
#endif
    }

    void BNetConsole::processCommand(const std::string& input)
    {
        const std::string lowered = lower(input);

        struct Command
        {
            const char* name;
            void (BNetConsole::*handler)(const std::string&);
        };

        static constexpr Command commands[] =
        {
            { "bnetaccount create", &BNetConsole::createBattleNetAccount },
            { "gameaccount create", &BNetConsole::createGameAccount },
            { "account unlink", &BNetConsole::unlinkGameAccount },
            { "account link", &BNetConsole::linkGameAccount },
            { "account list", &BNetConsole::listGameAccounts },
        };

        if (lowered == "help" || lowered == "?")
        {
            printHelp();
            return;
        }

        if (lowered == "shutdown" || lowered == "exit" || lowered == "quit")
        {
            Master::getInstance().stop();
            return;
        }

        for (const Command& command : commands)
        {
            const std::string_view name(command.name);
            if (lowered == name || (lowered.size() > name.size() &&
                lowered.compare(0, name.size(), name) == 0 &&
                std::isspace(static_cast<unsigned char>(lowered[name.size()]))))
            {
                const std::string args = trim(input.substr(name.size()));
                (this->*command.handler)(args);
                return;
            }
        }

        sLogger.info("BNet Console: unknown command '{}'. Type 'help'.", input);
    }

    void BNetConsole::printHelp() const
    {
        sLogger.info("BNet Console commands:");
        sLogger.info("  bnetaccount create <email> <password> [battleTag] [country]");
        sLogger.info("      Creates a Battle.net login and SRP-v1 credentials.");
        sLogger.info("      Example: bnetaccount create test@test.local Secret123 Test#1 CH");
        sLogger.info("  gameaccount create <name> <password> [email]");
        sLogger.info("      Creates a legacy WoW game account in `accounts`.");
        sLogger.info("      Example: gameaccount create TEST Secret123 test@test.local");
        sLogger.info("  account link <bnet-email> <game-account>");
        sLogger.info("      Links an existing WoW account to a Battle.net account.");
        sLogger.info("  account unlink <bnet-email> <game-account>");
        sLogger.info("      Removes that link.");
        sLogger.info("  account list <bnet-email>");
        sLogger.info("      Lists linked WoW game accounts.");
        sLogger.info("  shutdown | exit | quit");
    }

    void BNetConsole::createBattleNetAccount(const std::string& args)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet Console: logon database is unavailable.");
            return;
        }

        std::istringstream stream(args);
        std::string email;
        std::string password;
        std::string battleTag;
        std::string country;

        if (!readToken(stream, email) || !readToken(stream, password))
        {
            sLogger.info("Usage: bnetaccount create <email> <password> [battleTag] [country]");
            return;
        }

        stream >> battleTag >> country;

        if (battleTag.empty())
            battleTag = makeDefaultBattleTag(email);
        if (country.empty())
            country = "CH";

        if (country.size() != 2)
        {
            sLogger.failure("BNet Console: country must be a two-letter code, e.g. CH.");
            return;
        }

        uint32_t existingId = 0;
        if (getBattleNetAccountId(email, existingId))
        {
            sLogger.failure("BNet Console: Battle.net account '{}' already exists with id {}.", email, existingId);
            return;
        }

        AscEmu::Cryptography::BNetSrpV1::RegistrationData registration;
        if (!AscEmu::Cryptography::BNetSrpV1::makeRegistrationData(email, password, registration))
        {
            sLogger.failure("BNet Console: failed to generate SRP-v1 credentials for '{}'.", email);
            return;
        }

        const std::string escapedEmail = sBNetLogonSQL->escapeString(email);
        const std::string escapedTag = sBNetLogonSQL->escapeString(battleTag);
        const std::string escapedCountry = sBNetLogonSQL->escapeString(country);
        const std::string escapedSalt = sBNetLogonSQL->escapeString(registration.saltHex);
        const std::string escapedVerifier = sBNetLogonSQL->escapeString(registration.verifierHex);

        std::stringstream query;
        query << "INSERT INTO `battlenet_accounts` "
              << "(`email`, `srp_version`, `srp_salt`, `srp_verifier`, `battle_tag`, `country`) VALUES ('"
              << escapedEmail << "', 1, '"
              << escapedSalt << "', '"
              << escapedVerifier << "', '"
              << escapedTag << "', '"
              << escapedCountry << "')";

        if (!sBNetLogonSQL->waitExecuteNA(query.str().c_str()))
        {
            sLogger.failure("BNet Console: failed to create Battle.net account '{}'.", email);
            return;
        }

        uint32_t accountId = 0;
        getBattleNetAccountId(email, accountId);

        sLogger.info(
            "BNet Console: created Battle.net account '{}' id={} BattleTag='{}'.",
            email, accountId, battleTag);
    }

    void BNetConsole::createGameAccount(const std::string& args)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet Console: logon database is unavailable.");
            return;
        }

        std::istringstream stream(args);
        std::string name;
        std::string password;
        std::string email;

        if (!readToken(stream, name) || !readToken(stream, password))
        {
            sLogger.info("Usage: gameaccount create <name> <password> [email]");
            return;
        }

        stream >> email;

        uint32_t existingId = 0;
        if (getGameAccountId(name, existingId))
        {
            sLogger.failure("BNet Console: game account '{}' already exists with id {}.", name, existingId);
            return;
        }

        const std::string escapedName = sBNetLogonSQL->escapeString(name);
        const std::string escapedEmail = sBNetLogonSQL->escapeString(email);
        const std::string passwordSource = name + ":" + password;
        const std::string escapedPasswordSource = sBNetLogonSQL->escapeString(passwordSource);

        // Keep the same legacy credential format used by the existing
        // logonserver account-create path.
        std::stringstream query;
        query << "INSERT INTO `accounts` "
              << "(`acc_name`, `encrypted_password`, `banned`, `email`, `flags`, `banreason`) VALUES ('"
              << escapedName << "', SHA(UPPER('"
              << escapedPasswordSource << "')), '0', '"
              << escapedEmail << "', '24', '')";

        if (!sBNetLogonSQL->waitExecuteNA(query.str().c_str()))
        {
            sLogger.failure("BNet Console: failed to create game account '{}'.", name);
            return;
        }

        uint32_t gameAccountId = 0;
        getGameAccountId(name, gameAccountId);

        sLogger.info(
            "BNet Console: created WoW game account '{}' id={}.",
            name, gameAccountId);
    }

    void BNetConsole::linkGameAccount(const std::string& args)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet Console: logon database is unavailable.");
            return;
        }

        std::istringstream stream(args);
        std::string email;
        std::string gameAccountName;

        if (!readToken(stream, email) || !readToken(stream, gameAccountName))
        {
            sLogger.info("Usage: account link <bnet-email> <game-account>");
            return;
        }

        uint32_t battleNetAccountId = 0;
        if (!getBattleNetAccountId(email, battleNetAccountId))
        {
            sLogger.failure("BNet Console: Battle.net account '{}' was not found.", email);
            return;
        }

        uint32_t gameAccountId = 0;
        if (!getGameAccountId(gameAccountName, gameAccountId))
        {
            sLogger.failure("BNet Console: game account '{}' was not found.", gameAccountName);
            return;
        }

        auto existingLink = sBNetLogonSQL->query(
            "SELECT battlenet_account_id "
            "FROM battlenet_game_accounts "
            "WHERE game_account_id = %u LIMIT 1",
            gameAccountId);

        if (existingLink)
        {
            const uint32_t linkedBattleNetId = existingLink->fetch()[0].asUint32();
            if (linkedBattleNetId == battleNetAccountId)
            {
                sLogger.info(
                    "BNet Console: game account '{}' is already linked to '{}'.",
                    gameAccountName, email);
            }
            else
            {
                sLogger.failure(
                    "BNet Console: game account '{}' is already linked to Battle.net account id {}. "
                    "Unlink it first.",
                    gameAccountName, linkedBattleNetId);
            }
            return;
        }

        std::stringstream query;
        query << "INSERT INTO `battlenet_game_accounts` "
              << "(`battlenet_account_id`, `game_account_id`) VALUES ("
              << battleNetAccountId << ", " << gameAccountId << ")";

        if (!sBNetLogonSQL->waitExecuteNA(query.str().c_str()))
        {
            sLogger.failure(
                "BNet Console: failed to link '{}' to '{}'.",
                gameAccountName, email);
            return;
        }

        sLogger.info(
            "BNet Console: linked game account '{}' (id={}) to '{}' (id={}).",
            gameAccountName, gameAccountId, email, battleNetAccountId);
    }

    void BNetConsole::unlinkGameAccount(const std::string& args)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet Console: logon database is unavailable.");
            return;
        }

        std::istringstream stream(args);
        std::string email;
        std::string gameAccountName;

        if (!readToken(stream, email) || !readToken(stream, gameAccountName))
        {
            sLogger.info("Usage: account unlink <bnet-email> <game-account>");
            return;
        }

        uint32_t battleNetAccountId = 0;
        if (!getBattleNetAccountId(email, battleNetAccountId))
        {
            sLogger.failure("BNet Console: Battle.net account '{}' was not found.", email);
            return;
        }

        uint32_t gameAccountId = 0;
        if (!getGameAccountId(gameAccountName, gameAccountId))
        {
            sLogger.failure("BNet Console: game account '{}' was not found.", gameAccountName);
            return;
        }

        std::stringstream query;
        query << "DELETE FROM `battlenet_game_accounts` "
              << "WHERE `battlenet_account_id` = " << battleNetAccountId
              << " AND `game_account_id` = " << gameAccountId;

        if (!sBNetLogonSQL->waitExecuteNA(query.str().c_str()))
        {
            sLogger.failure(
                "BNet Console: failed to unlink '{}' from '{}'.",
                gameAccountName, email);
            return;
        }

        sLogger.info(
            "BNet Console: unlinked game account '{}' from '{}'.",
            gameAccountName, email);
    }

    void BNetConsole::listGameAccounts(const std::string& args)
    {
        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet Console: logon database is unavailable.");
            return;
        }

        const std::string email = trim(args);
        if (email.empty())
        {
            sLogger.info("Usage: account list <bnet-email>");
            return;
        }

        uint32_t battleNetAccountId = 0;
        if (!getBattleNetAccountId(email, battleNetAccountId))
        {
            sLogger.failure("BNet Console: Battle.net account '{}' was not found.", email);
            return;
        }

        auto result = sBNetLogonSQL->query(
            "SELECT a.id, a.acc_name "
            "FROM battlenet_game_accounts bga "
            "INNER JOIN accounts a ON a.id = bga.game_account_id "
            "WHERE bga.battlenet_account_id = %u "
            "ORDER BY a.id",
            battleNetAccountId);

        if (!result)
        {
            sLogger.info("BNet Console: '{}' has no linked game accounts.", email);
            return;
        }

        sLogger.info(
            "BNet Console: linked game accounts for '{}' (id={}):",
            email, battleNetAccountId);

        do
        {
            Field* fields = result->fetch();
            sLogger.info(
                "  id={} name='{}'",
                fields[0].asUint32(),
                fields[1].asCString() != nullptr ? fields[1].asCString() : "");
        }
        while (result->nextRow());
    }
}
