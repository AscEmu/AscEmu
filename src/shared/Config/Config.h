/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

class SERVER_DECL ConfigFile
{
    public:

        ConfigFile();
        ~ConfigFile();

        struct ConfigValueSetting
        {
            std::string asString;
            bool asBool;
            int asInt;
            float asFloat;
        };

        bool openAndLoadConfigFile(std::string configFileName);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Parser
        bool parseConfigValues(std::string fileBufferString);
        void removeSpacesInString(std::string& str);
        void removeAllSpacesInString(std::string& str);
        bool isComment(std::string& lineString, bool* isInMultilineComment);
        void applySettingToStore(std::string& str, ConfigValueSetting& setting);

        uint32_t getSettingHash(std::string settingString);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Get functions
        ConfigValueSetting* getSavedSetting(std::string sectionName, std::string confName);

        std::string getStringDefault(std::string sectionName, std::string confName, std::string defaultString);
        bool getBoolDefault(std::string sectionName, std::string confName, bool defaultBool);
        int getIntDefault(std::string sectionName, std::string confName, int defaultInt);
        float getFloatDefault(std::string sectionName, std::string confName, float defaultFloat);

    private:

        typedef std::map<uint32_t, ConfigValueSetting> ConfigSection;
        std::map<uint32_t, ConfigSection> mSettings;
};
