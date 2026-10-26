/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>

// brings the characters last played with an older build to the build of the server, once at the start
class CharacterBuildUpdater
{
    CharacterBuildUpdater() = default;
    ~CharacterBuildUpdater() = default;

public:
    static CharacterBuildUpdater& getInstance();

    CharacterBuildUpdater(CharacterBuildUpdater&&) = delete;
    CharacterBuildUpdater(CharacterBuildUpdater const&) = delete;
    CharacterBuildUpdater& operator=(CharacterBuildUpdater&&) = delete;
    CharacterBuildUpdater& operator=(CharacterBuildUpdater const&) = delete;

    // every step a character crossed since its last login, then the build of the server as its last login
    void run();

    struct OldCharacter
    {
        uint32_t guid = 0;
        uint32_t accountId = 0;
        uint32_t lastLoginBuild = 0;
        uint32_t level = 0;
        uint32_t mapId = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool onTransport = false;
    };

private:
    // the reshaped terrain of 4.0: the character stands on the ground of the current map data
    void placeOnCurrentTerrain(std::vector<OldCharacter> const& characters, uint32_t serverBuild);

    // the level squish of 9.0: the old level becomes the new one, the experience starts over
    void squishLevels(std::vector<OldCharacter> const& characters);
};

#define sCharacterBuildUpdater CharacterBuildUpdater::getInstance()
