/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define WEATHER_DENSITY_UPDATE 0.05f

#include "Singleton.h"
#include "Server/EventableObject.h"
#include "Server/Packets/SmsgWeather.h"

class WorldPacket;
class WeatherInfo;

uint32 GetSound(uint32 Effect, float Density);

class SERVER_DECL WeatherMgr : public Singleton<WeatherMgr>
{
    public:

        WeatherMgr();
        ~WeatherMgr();

        void LoadFromDB();
        void SendWeather(Player* plr);

        void sendWeatherForZone(uint32_t type, float density, uint32_t zoneId);
        void sendWeatherForPlayer(uint32_t type, float density, Player* player);

    private:

        std::map<uint32, WeatherInfo*> m_zoneWeathers;
};

class WeatherInfo : public EventableObject
{
    friend class WeatherMgr;

    public:

        WeatherInfo();
        ~WeatherInfo();

        void BuildUp();
        void Update();
        void SendUpdate();
        void SendUpdate(Player* plr);

    protected:

        void _GenerateWeather();

        uint32 m_zoneId;

        uint32 m_totalTime;
        uint32 m_currentTime;

        float m_maxDensity;
        float m_currentDensity;

        uint32 m_currentEffect;
        std::map<uint32, uint32> m_effectValues;
};

#define sWeatherMgr WeatherMgr::getSingleton()
