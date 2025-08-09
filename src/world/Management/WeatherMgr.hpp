/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/EventableObject.h"

class Player;
class WeatherInfo;

class SERVER_DECL WeatherMgr
{
private:
    WeatherMgr() = default;
    ~WeatherMgr() = default;

public:
    static WeatherMgr& getInstance();
    void finalize();

    WeatherMgr(WeatherMgr&&) = delete;
    WeatherMgr(WeatherMgr const&) = delete;
    WeatherMgr& operator=(WeatherMgr&&) = delete;
    WeatherMgr& operator=(WeatherMgr const&) = delete;

    void loadFromDB();
    void sendWeather(Player* plr);

    void sendWeatherForZone(uint32_t type, float density, uint32_t zoneId);
    void sendWeatherForPlayer(uint32_t type, float density, Player* player);

    uint32_t getSound(uint32_t effect, float density);

private:
    std::map<uint32_t, std::unique_ptr<WeatherInfo>> m_zoneWeathers;
};

class WeatherInfo : public EventableObject
{
    friend class WeatherMgr;

public:
    WeatherInfo();
    ~WeatherInfo();

    void buildUp();
    void update();
    void sendUpdate() const;
    void sendUpdate(Player* player) const;

protected:
    void _generateWeather();

    uint32_t m_zoneId;

    uint32_t m_totalTime;
    uint32_t m_currentTime;

    float m_maxDensity;
    float m_currentDensity;

    uint32_t m_currentEffect;
    std::map<uint32_t, uint32_t> m_effectValues;
    const float m_densityUpdate = 0.05f;
};

#define sWeatherMgr WeatherMgr::getInstance()
