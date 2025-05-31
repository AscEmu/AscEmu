/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/WeatherMgr.hpp"

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/Packets/SmsgWeather.h"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

enum WeatherTypes
{
    WEATHER_TYPE_NORMAL         = 0,
    WEATHER_TYPE_FOG            = 1,
    WEATHER_TYPE_RAIN           = 2,
    WEATHER_TYPE_HEAVY_RAIN     = 4,
    WEATHER_TYPE_SNOW           = 8,
    WEATHER_TYPE_SANDSTORM      = 16
};

enum WeatherSounds
{
    WEATHER_NOSOUND             = 0,
    WEATHER_RAINLIGHT           = 8533,
    WEATHER_RAINMEDIUM          = 8534,
    WEATHER_RAINHEAVY           = 8535,
    WEATHER_SNOWLIGHT           = 8536,
    WEATHER_SNOWMEDIUM          = 8537,
    WEATHER_SNOWHEAVY           = 8538,
    WEATHER_SANDSTORMLIGHT      = 8556,
    WEATHER_SANDSTORMMEDIUM     = 8557,
    WEATHER_SANDSTORMHEAVY      = 8558
};

uint32_t WeatherMgr::getSound(uint32_t effect, float density)
{
    if (density <= 0.30f)
        return WEATHER_NOSOUND;

    uint32_t sound;

    switch (effect)
    {
        case WEATHER_TYPE_RAIN:
        case WEATHER_TYPE_HEAVY_RAIN:
        {
            if (density < 0.40f)
                sound = WEATHER_RAINLIGHT;
            else if (density < 0.70f)
                sound = WEATHER_RAINMEDIUM;
            else
                sound = WEATHER_RAINHEAVY;
        } break;
        case WEATHER_TYPE_SNOW:
        {
            if (density < 0.40f)
                sound = WEATHER_SNOWLIGHT;
            else if (density < 0.70f)
                sound = WEATHER_SNOWMEDIUM;
            else
                sound = WEATHER_SNOWHEAVY;
        } break;
        case WEATHER_TYPE_SANDSTORM:
        {
            if (density < 0.40f)
                sound = WEATHER_SANDSTORMLIGHT;
            else if (density < 0.70f)
                sound = WEATHER_SANDSTORMMEDIUM;
            else
                sound = WEATHER_SANDSTORMHEAVY;
        } break;
        default:
            sound = WEATHER_NOSOUND;
            break;
    }
    return sound;
}

WeatherMgr& WeatherMgr::getInstance()
{
    static WeatherMgr mInstance;
    return mInstance;
}

void WeatherMgr::finalize()
{
    m_zoneWeathers.clear();
}

void WeatherMgr::loadFromDB()
{
    sLogger.info("Loading Weather...");
    auto result = WorldDatabase.Query("SELECT zoneId,high_chance,high_type,med_chance,med_type,low_chance,low_type FROM weather");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        auto weatherInfo = std::make_unique<WeatherInfo>();
        weatherInfo->m_zoneId = fields[0].asUint32();
        weatherInfo->m_effectValues[0] = fields[1].asUint32();  // high_chance
        weatherInfo->m_effectValues[1] = fields[2].asUint32();  // high_type
        weatherInfo->m_effectValues[2] = fields[3].asUint32();  // med_chance
        weatherInfo->m_effectValues[3] = fields[4].asUint32();  // med_type
        weatherInfo->m_effectValues[4] = fields[5].asUint32();  // low_chance
        weatherInfo->m_effectValues[5] = fields[6].asUint32();  // low_type
        const auto [itr, _] = m_zoneWeathers.try_emplace(fields[0].asUint32(), std::move(weatherInfo));

        itr->second->_generateWeather();
    }
    while (result->NextRow());
    sLogger.info("WeatherMgr : Loaded weather information for {} zones.", result->GetRowCount());
}

void WeatherMgr::sendWeather(Player* plr)
{
    auto zoneWeatherItr = m_zoneWeathers.find(plr->getZoneId());
    if (zoneWeatherItr == m_zoneWeathers.end())
    {
        plr->getSession()->SendPacket(AscEmu::Packets::SmsgWeather(0, 0, 0).serialise().get());
        plr->m_lastSeenWeather = 0;
    }
    else
    {
        zoneWeatherItr->second->sendUpdate(plr);
    }
}

void WeatherMgr::sendWeatherForZone(uint32_t type, float density, uint32_t zoneId)
{
    const uint32_t sound = getSound(type, density);
    sWorld.sendZoneMessage(AscEmu::Packets::SmsgWeather(type, density, sound).serialise().get(), zoneId);
}

void WeatherMgr::sendWeatherForPlayer(uint32_t type, float density, Player* player)
{
    if (player != nullptr)
    {
        const uint32_t sound = getSound(type, density);
        player->sendPacket(AscEmu::Packets::SmsgWeather(type, density, sound).serialise().get());
    }
}

WeatherInfo::WeatherInfo()
{
    m_currentDensity = 0;
    m_currentEffect = 0;
    m_currentTime = 0;
    m_maxDensity = 0;
    m_totalTime = 0;
    m_zoneId = 0;

    m_holder = sEventMgr.GetEventHolder(WORLD_INSTANCE);
}

WeatherInfo::~WeatherInfo()
{}

void WeatherInfo::_generateWeather()
{
    m_currentTime = 0;
    m_currentEffect = 0;
    m_currentDensity = 0.30f;

    const float randomFloat = Util::getRandomFloat(1);
    m_maxDensity = randomFloat + 1;
    m_totalTime = (Util::getRandomUInt(11) + 5) * 1000 * 120;

    const uint32_t randomUInt = Util::getRandomUInt(100);

    if (randomUInt <= m_effectValues[4])        // low chance
        m_currentEffect = m_effectValues[5];    // low type
    else if (randomUInt <= m_effectValues[2])   // med chance
        m_currentEffect = m_effectValues[3];    // med type
    else if (randomUInt <= m_effectValues[0])   // high chance
        m_currentEffect = m_effectValues[1];    // high type

    sendUpdate();

    sEventMgr.AddEvent(this, &WeatherInfo::buildUp, EVENT_WEATHER_UPDATE, static_cast<uint32_t>(m_totalTime / ceil(m_maxDensity / m_densityUpdate) * 2), 0, 0);
    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "Forecast for zone:{} new type:{} new interval:{} ms", m_zoneId, m_currentEffect, static_cast<uint32_t>(m_totalTime / ceil(m_maxDensity / m_densityUpdate) * 2));
}

void WeatherInfo::buildUp()
{
    if (m_currentDensity >= 0.50f)
    {
        sEventMgr.RemoveEvents(this, EVENT_WEATHER_UPDATE);
        sEventMgr.AddEvent(this, &WeatherInfo::update, EVENT_WEATHER_UPDATE, static_cast<uint32_t>(m_totalTime / ceil(m_maxDensity / m_densityUpdate) * 4), 0, 0);
    }
    else
    {
        m_currentDensity += m_densityUpdate;
        sendUpdate();
    }
}

void WeatherInfo::update()
{
    if (m_currentEffect == 0 || Util::getRandomUInt(100) < 66)
    {
        m_currentDensity -= m_densityUpdate;
        if (m_currentDensity < 0.30f)
        {
            m_currentDensity = 0.0f;
            m_currentEffect = 0;
            sEventMgr.RemoveEvents(this, EVENT_WEATHER_UPDATE);
            _generateWeather();
            return;
        }
    }
    else
    {
        m_currentDensity += m_densityUpdate;
        if (m_currentDensity >= m_maxDensity)
        {
            m_currentDensity = m_maxDensity;
            return;
        }
    }
    sendUpdate();
}

void WeatherInfo::sendUpdate() const
{
    const uint32_t sound = sWeatherMgr.getSound(m_currentEffect, m_currentDensity);
    sWorld.sendZoneMessage(AscEmu::Packets::SmsgWeather(m_currentEffect, m_currentDensity, sound).serialise().get(), m_zoneId);
}

void WeatherInfo::sendUpdate(Player* player) const
{
    if (player->m_lastSeenWeather == m_currentEffect)
        return;

    player->m_lastSeenWeather = m_currentEffect;

    const uint32_t sound = sWeatherMgr.getSound(m_currentEffect, m_currentDensity);
    player->getSession()->SendPacket(AscEmu::Packets::SmsgWeather(m_currentEffect, m_currentDensity, sound).serialise().get());
}
