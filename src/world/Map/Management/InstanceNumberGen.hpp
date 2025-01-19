/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>

template<typename T>
class custom_priority_queue_ascend : public std::priority_queue<T, std::vector<T>, std::greater<int32_t>>
{
public:
    bool remove(const T& value) 
    {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) 
        {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }

        return false;
    }
};

template<typename T>
class custom_priority_queue_descend : public std::priority_queue<T, std::vector<T>>
{
public:
    bool remove(const T& value)
    {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end())
        {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }

        return false;
    }
};

class UniqueNumberPool
{
private:
    int32_t min = 0;
    int32_t max = 0;

    custom_priority_queue_ascend<int32_t> freeIds;
    std::vector<int32_t> usedIds;

public:
    void fill(int32_t minValue, int32_t maxValue)
    {
        usedIds.clear();

        min = minValue;
        max = maxValue;

        for (int32_t i = minValue; i < maxValue; ++i)
            freeIds.push(i);
    }

    int32_t generateId()
    {
        if (!freeIds.empty())
        {
            const int32_t id = freeIds.top();
            freeIds.pop();
            usedIds.push_back(id);
            return id;
        }

        const auto error = abs(min) + abs(max);
        std::cout << "We run out of Available Unique Ids will return " << error << " \n";
        return error;
    }

    void freeUsedId(int32_t id)
    {
        if (id < min || id > max)
        {
            std::cout << "Tried to free Id " << id << " but is not in range of min " << min << " and max " << max << " \n";
            return;
        }

        usedIds.erase(std::ranges::remove(usedIds, id).begin(), usedIds.end());
        freeIds.emplace(id);
    }

    void addUsedValue(int32_t id)
    {
        if (freeIds.remove(id))
            usedIds.push_back(id);
    }
};
