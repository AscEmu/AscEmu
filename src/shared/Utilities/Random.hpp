/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <random>
#include <vector>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end);
    int getRandomInt(int start, int end);

    uint32_t getRandomUInt(uint32_t end);
    uint32_t getRandomUInt(uint32_t start, uint32_t end);

    float getRandomFloat(float end);
    float getRandomFloat(float start, float end);

    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(uint32_t val);
    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(int32_t val);
    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(float val);

    template <class T>
    inline T square(T x) { return x * x; }

    // Percentage calculation
    template <class T, class U>
    inline T calculatePct(T base, U pct)
    {
        return T(base * static_cast<float>(pct) / 100.0f);
    }

    template <class T, class U>
    inline T addPct(T& base, U pct)
    {
        return base += calculatePct(base, pct);
    }

    template <class T, class U>
    inline T applyPct(T& base, U pct)
    {
        return base = calculatePct(base, pct);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Container helper functions

    template<typename T>
    inline void randomShuffleVector(std::vector<T>* vector)
    {
        std::random_device rd;
        std::mt19937 mt(rd());

        std::shuffle(vector->begin(), vector->end(), mt);
    }

    template <class C>
    typename C::value_type const& selectRandomContainerElement(C const& container)
    {
        typename C::const_iterator it = container.begin();
        std::advance(it, Util::getRandomUInt(0, static_cast<uint32_t>(container.size() - 1)));
        return *it;
    }
}
