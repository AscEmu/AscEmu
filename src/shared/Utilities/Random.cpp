/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Random.hpp"

namespace Util
{
    std::mt19937& getRandomEngine()
    {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        return mt;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end)
    {
        return getRandomUInt(0, end);
    }

    int getRandomInt(int start, int end)
    {
        auto& engine = getRandomEngine();
        std::uniform_int_distribution<int> dist(start, end);
        return dist(engine);
    }

    uint32_t getRandomUInt(uint32_t end)
    {
        return getRandomUInt(0, end);
    }

    uint32_t getRandomUInt(uint32_t start, uint32_t end)
    {
        auto& engine = getRandomEngine();
        std::uniform_int_distribution<uint32_t> dist(start, end);
        return dist(engine);
    }

    float getRandomFloat(float end)
    {
        return getRandomFloat(0.0f, end);
    }

    float getRandomFloat(float start, float end)
    {
        auto& engine = getRandomEngine();
        std::uniform_real_distribution<float> dist(start, end);
        return dist(engine);
    }

    bool checkChance(uint32_t val)
    {
        if (val >= 100)
            return true;
        if (val == 0)
            return false;

        return val >= getRandomUInt(100);
    }

    bool checkChance(int32_t val)
    {
        if (val >= 100)
            return true;
        if (val <= 0)
            return false;

        return val >= getRandomInt(100);
    }

    bool checkChance(float val)
    {
        if (val >= 100.0f)
            return true;
        if (val <= 0.0f)
            return false;

        return static_cast<uint32_t>(val * 100) >= getRandomUInt(100 * 100);
    }
}
